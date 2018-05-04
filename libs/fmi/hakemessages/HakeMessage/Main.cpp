#include "HakeMessage/Main.h"
#include "HakeMessage/HakeMsg.h"
#include "HakeMessage/MsgParser.h"
#include "HakeMessage/Io.h"
#include "HakeMessage/HakeMessages.h"
#include "HakeMessage/Configurer.h"
#include "HakeMessage/QueryDataMaker.h"
#include "HakeMessage/Common.h"
#include "HakeMessage/json.hpp"
#include "HakeMessage/HakeSystemConfigurations.h"

#include "NFmiArea.h"
#include "NFmiMetTime.h"
#include "NFmiQueryData.h"
#include "NFmiProducerName.h"
#include "cppback/background-manager.h"
#include "cppback/loop-signal-checker.h"


namespace HakeMessage
{
    namespace
    {
        std::vector<HakeMsg> filterMessages(const std::multiset<HakeMsg> msgsToFilter, const NFmiMetTime& startTime, const NFmiMetTime& endTime, const NFmiArea& area, HakeLegacySupport::HakeSystemConfigurations &hakeSystemConfigurations)
        {
            auto msgs = std::vector<HakeMsg>{};

            std::copy_if(msgsToFilter.cbegin(), msgsToFilter.cend(), std::back_inserter(msgs), [&startTime, &endTime, &area, &hakeSystemConfigurations](const auto& message)
            {
                return startTime.IsLessThan(message.StartTime())
                    && message.StartTime().IsLessThan(endTime)
                    && area.IsInside(message.LatlonPoint())
                    && hakeSystemConfigurations.IsMessageShown(message);
            });

            return msgs;
        }

        std::string timePointToString(const std::chrono::system_clock::time_point &timePoint, const std::string &timeFormat)
        {
            auto time_t1 = std::chrono::system_clock::to_time_t(timePoint);
            std::stringstream out;
#pragma warning(push)
#pragma warning(disable:4996)
            out << std::put_time(std::gmtime(&time_t1), timeFormat.data());
#pragma warning(pop)
            return out.str();
        }

        // This is wanted tiem range string made here:
        // "?starttime=2017-06-20 00:00&endtime=2017-06-21 00:00"
        std::string makeKahaTimeRangeString(int hourRange)
        {
            // Had to add space between day and hour using url encoded character %20 which has to be given to 
            // std::put_time function (which uses it finally) with %-escape character. So space between day and hour is %%20.
            const std::string timeFormat = "%Y-%m-%d%%20%H:%M";
            auto currentTime = std::chrono::system_clock::now();
            auto startTime = currentTime;
            startTime -= std::chrono::hours(hourRange);
            std::string timeRangeString = "&starttime=";
            timeRangeString += timePointToString(startTime, timeFormat);
            timeRangeString += "&endtime=";
            timeRangeString += timePointToString(currentTime, timeFormat);
            return timeRangeString;
        }
    }

    Main::Main()
    {
        bManager_ = std::make_shared<cppback::BackgroundManager>();
        client_ = std::make_shared<Web::CppRestClient>(bManager_);
        kahaMessages_ = std::make_unique<KahaMessages>(client_);
        hakeMessages_ = std::make_unique<HakeMessages>();
        io_ = std::make_unique<Io>(bManager_);
        queryDataMaker_ = std::make_unique<QueryDataMaker>(bManager_);
        configurer_ = std::make_unique<Configurer>();

        legacyData_ = std::make_unique<HakeLegacySupport::HakeSystemConfigurations>();

        logger_ = nullptr;

        hakeData_ = nullptr;
        kahaData_ = nullptr;
    }

    void Main::initialize()
    {
        configurer_->configure();
        legacyData_->InitializeFromSettings(configurer_->baseConfigurationNamespace);
        // Legacydataan pitää myös asettaa sanomien lukutiheys minuuteissa
        legacyData_->UpdateTimeStepInMinutes(milliSecondsToMinutes(configurer_->checkForNewMessagesDelay));
    }

    void Main::setUpdateApplicationCallback(std::function<void()> updateApplicationCallback)
    {
        updateApplicationCallback_ = updateApplicationCallback;
    }

    void Main::setLogger(std::unique_ptr<Logger> logger)
    {
        logger_ = std::move(logger);
    }

    bool Main::isThereAnyWorkToDo() const
    {
        // Jos kaikki datapolut ovat tyhjiä, ei pidä alkaa tekemään mitään (esim. kansainväliset konfiguraatiot)
        return !configurer_->xmlPath.empty() || !configurer_->jsonPath.empty();
    }

    std::vector<HakeMsg> Main::getHakeMessages(const NFmiMetTime &startTime, const NFmiMetTime &endTime, const NFmiArea &area) const
    {
        return filterMessages(hakeMessages_->peekMessages(), startTime, endTime, area, *legacyData_);
    }

    std::vector<KahaMsg> Main::getKahaMessages(const NFmiMetTime &startTime, const NFmiMetTime &endTime, const NFmiArea &area) const
    {
        return filterMessages(kahaMessages_->peekMessages(), startTime, endTime, area, *legacyData_);
    }

    void Main::goToWorkAfter(std::chrono::milliseconds delay)
    {
        bManager_->addTask([&, delay, bManager = bManager_]
        {
            auto tmp = delay;
            static bool firstRound = true;

            while(true)
            {
                using namespace std::literals;
                bManager_->sleepInIntervals(tmp, 500ms, "hakeMessageFetch");
                handleKahaMessages();
                handleHakeMessages(configurer_->maxNumberOfMessagesReadAtOnce);
                if(firstRound)
                {
                    tmp = configurer_->checkForNewMessagesDelay;
                    firstRound = false;
                }
            }
        });
    };

    void Main::handleHakeMessages(unsigned int maxNumberOfMessagesToRead)
    {
        auto messagesInitSize = hakeMessages_->getSize();
        size_t messagesRead;
        do
        {
            messagesRead = 0;
            messagesRead += handleHakeXml(maxNumberOfMessagesToRead / 2);
            messagesRead += handleHakeJson(maxNumberOfMessagesToRead / 2);
            if(updateApplicationCallback_)
            {
                updateApplicationCallback_();
            }
        } while(messagesRead > maxNumberOfMessagesToRead / 3);

        if(hakeMessages_->getSize() != messagesInitSize)
        {
            createHakeQueryData();
        }
    }

    void Main::handleKahaMessages()
    {
        auto messagesInitSize = kahaMessages_->getSize();
        size_t messagesRead = handleKahaJson();

        if(kahaMessages_->getSize() != messagesInitSize)
        {
            createKahaQueryData();
        }
    }

    size_t Main::handleHakeXml(unsigned int maxNumberOfMessagesToRead)
    {
        auto checker = cppback::LoopSignalChecker(maxNumberOfMessagesToRead / 10, "hakeMessageFetch");

        auto xmlMsgs = io_->readFromFolderWithFilter(configurer_->xmlPath, configurer_->xmlFilter, maxNumberOfMessagesToRead);
        for(const std::string &xmlMsg : xmlMsgs)
        {
            checker.check(*bManager_);
            try
            {
                hakeMessages_->addMessage(MsgParser::parseXmlToHakeMessage(xmlMsg));
            }
            catch(...)
            {

            }
        }
        return xmlMsgs.size();
    }

    size_t Main::handleHakeJson(unsigned int maxNumberOfMessagesToRead)
    {
        auto checker = cppback::LoopSignalChecker(maxNumberOfMessagesToRead / 10, "hakeMessageFetch");
        auto jsonMsgs = io_->readFromFolderWithFilter(configurer_->jsonPath, configurer_->jsonFilter, maxNumberOfMessagesToRead);
        for(const std::string &jsonMsg : jsonMsgs)
        {
            checker.check(*bManager_);
            try
            {
                hakeMessages_->addMessage(MsgParser::parseJsonToHakeMessage(jsonMsg));
            }
            catch(...)
            {

            }
        }
        return jsonMsgs.size();
    }

    size_t Main::handleKahaJson()
    {
        try
        {
            std::string kahaRequestString = "/mobile/interfaces/crowd/map.php?smartmet=1";
            kahaRequestString += makeKahaTimeRangeString(7 * 24);
            auto jsonFut = client_->queryFor("http://m.fmi.fi", kahaRequestString);

            jsonFut.wait();

            auto json = nlohmann::json::parse(jsonFut.get());

            auto jsons = json["observations"]["features"];
            auto checker = cppback::LoopSignalChecker(jsons.size() / 10, "kahaMessageFetch");
            for(const auto &json : jsons)
            {
                checker.check(*bManager_);
                try
                {
                    kahaMessages_->addMessage(MsgParser::parseJsonToKahaMessage(json.dump()));
                }
                catch(...)
                {

                }
            }
            return jsons.size();
        }
        catch(const std::exception&)
        {
            return 0;
        }
    }

    void Main::createHakeQueryData()
    {
        bManager_->addTask([&] {
            auto msgs = hakeMessages_->peekMessages();
            try
            {
                auto hakeProducer = NFmiProducer{ kFmiHakeMessages, "HAKE" };
                hakeData_ = queryDataMaker_->createQueryDataFrom(msgs, hakeProducer);
                if(updateApplicationCallback_)
                {
                    updateApplicationCallback_();
                }
            }
            catch(const std::exception&)
            {
            }
        });
    }

    void Main::createKahaQueryData()
    {
        bManager_->addTask([&] {
            auto msgs = kahaMessages_->peekMessages();
            auto kahaProducer = NFmiProducer{ kFmiKaHaMessages, "KaHa" };
            kahaData_ = queryDataMaker_->createQueryDataFrom(msgs, kahaProducer);
            if(updateApplicationCallback_)
            {
                updateApplicationCallback_();
            }
        });
    }

    std::unique_ptr<NFmiQueryData> Main::getHakeQueryData()
    {
        return std::move(hakeData_);
    }

    std::unique_ptr<NFmiQueryData> Main::getKahaQueryData()
    {
        return std::move(kahaData_);
    }

    bool Main::isDead(std::chrono::milliseconds wait)
    {
        return bManager_->isDead(wait);
    }

    void Main::kill()
    {
        client_->cancel();
        bManager_->kill();
    }

    HakeLegacySupport::HakeSystemConfigurations& Main::getLegacyData()
    {
        return *legacyData_;
    }
}
