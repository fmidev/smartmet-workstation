#include "HakeMessage/Main.h"
#ifndef DISABLE_CPPRESTSDK

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
#include "NFmiFileString.h"
#include "cppback/background-manager.h"
#include "cppback/loop-signal-checker.h"

namespace
{
    std::vector<HakeMessage::HakeMsg> filterMessages(const std::multiset<HakeMessage::HakeMsg> msgsToFilter, const NFmiMetTime& startTime, const NFmiMetTime& endTime, const NFmiArea& area, HakeLegacySupport::HakeSystemConfigurations &hakeSystemConfigurations)
    {
        auto msgs = std::vector<HakeMessage::HakeMsg>{};

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
        timeRangeString += ::timePointToString(startTime, timeFormat);
        timeRangeString += "&endtime=";
        timeRangeString += ::timePointToString(currentTime, timeFormat);
        return timeRangeString;
    }

    void makeMessageRelatedErrorLog(const std::string &baseMessage, std::exception &e, CatLog::Severity severity)
    {
        std::string logMessage = baseMessage;
        logMessage += ": ";
        logMessage += e.what();
        CatLog::logMessage(logMessage, severity, CatLog::Category::Data);
    }

    void makeMessageReadLogMessage(const std::string &baseMessage, size_t messagesRead)
    {
        std::string logMessage = baseMessage;
        logMessage += ": ";
        logMessage += std::to_string(messagesRead);
        CatLog::logMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::Data);
    }

    void makeNoMessagesReadTraceLogMessage(const std::string &message)
    {
        if(CatLog::doTraceLevelLogging())
        {
            CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Data);
        }
    }
}


namespace HakeMessage
{
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

    void Main::initialize(const std::string& usedAbsoluteBaseDirectory)
    {
        configurer_->configure(usedAbsoluteBaseDirectory);
        legacyData_->InitializeFromSettings(configurer_->baseConfigurationNamespace);
        // Legacydataan pit‰‰ myˆs asettaa sanomien lukutiheys minuuteissa
        legacyData_->UpdateTimeStepInMinutes(milliSecondsToMinutes(configurer_->checkForNewMessagesDelay));
    }

    using namespace std::chrono_literals;

    void Main::goIntoCaseStudyMode(const std::string& usedAbsoluteCaseStudyHakeDirectory)
    {
        if(isThereAnyWorkToDo())
        {
            doCaseStudyModeChangePreparations();
            // Originaali konffeista saa ottaa vain 1. kerran kopion
            if(!originalConfigurer_)
            {
                originalConfigurer_ = std::make_unique<Configurer>(*configurer_);
            }
            configurer_->jsonPath = usedAbsoluteCaseStudyHakeDirectory;
            configurer_->xmlPath = usedAbsoluteCaseStudyHakeDirectory;

            pauseWorking_ = false;
            wakeUpWorker();
        }
    }

    void Main::doCaseStudyModeChangePreparations()
    {
        pauseWorking_ = true;
        // Odotetaan v‰h‰n aikaa ja toivotaan ett‰ mahdolliset tyˆt ovat loppuneet ja systeemi on pause moodissa
        std::this_thread::sleep_for(500ms);
        hakeMessages_->clearMessages();
        hakeData_.reset(nullptr);
        io_->clearReadFiles();
    }

    void Main::goIntoNormalModeFromStudyMode()
    {
        if(isThereAnyWorkToDo())
        {
            doCaseStudyModeChangePreparations();
            // Tehd‰‰n originaalista konffista takaisin kopio
            if(originalConfigurer_)
                configurer_ = std::make_unique<Configurer>(*originalConfigurer_);

            pauseWorking_ = false;
            wakeUpWorker();
        }
    }

    void Main::wakeUpWorker()
    {
        if(bManager_)
            bManager_->doForcedWakeUp();
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
        // Jos kaikki datapolut ovat tyhji‰, ei pid‰ alkaa tekem‰‰n mit‰‰n (esim. kansainv‰liset konfiguraatiot)
        return !configurer_->xmlPath.empty() || !configurer_->jsonPath.empty();
    }

    std::vector<HakeMsg> Main::getHakeMessages(const NFmiMetTime &startTime, const NFmiMetTime &endTime, const NFmiArea &area) const
    {
        return ::filterMessages(hakeMessages_->peekMessages(), startTime, endTime, area, *legacyData_);
    }

    std::vector<KahaMsg> Main::getKahaMessages(const NFmiMetTime &startTime, const NFmiMetTime &endTime, const NFmiArea &area) const
    {
        return ::filterMessages(kahaMessages_->peekMessages(), startTime, endTime, area, *legacyData_);
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
                if(!pauseWorking_)
                {
                    handleHakeMessages(configurer_->maxNumberOfMessagesReadAtOnce);
                    handleKahaMessages();
                }
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
        size_t messagesRead = 0;
        // Loopissa luetaan sanomia niin kauan kuin lˆytyy tarpeeksi luettuja uusia sanomia
        do
        {
            messagesRead = 0;
            messagesRead += handleHakeXml(maxNumberOfMessagesToRead);
            messagesRead += handleHakeJson(maxNumberOfMessagesToRead);
            if(messagesRead > 0)
            {
                ::makeMessageReadLogMessage("Hake messages read", messagesRead);
                doUpdateApplicationCallback();
            }
            else
                ::makeNoMessagesReadTraceLogMessage("No more Hake messages to read");
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

        if(messagesRead > 0)
            ::makeMessageReadLogMessage("KaHa messages read", messagesRead);
        else
            ::makeNoMessagesReadTraceLogMessage("No more KaHa messages to read");

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
            catch(std::exception &e)
            {
                // Ei viitti laittaa n‰ihin korkeata severity tasoa, koska virheellisi‰ viestej‰ on aika paljon
                ::makeMessageRelatedErrorLog("Hake xml type message parsing failed", e, CatLog::Severity::Debug);
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
            catch(std::exception &e)
            {
                // Ei viitti laittaa n‰ihin korkeata severity tasoa, koska virheellisi‰ viestej‰ on aika paljon
                ::makeMessageRelatedErrorLog("Hake json type message parsing failed", e, CatLog::Severity::Debug);
            }
        }
        return jsonMsgs.size();
    }

    size_t Main::handleKahaJson()
    {
        try
        {
            std::string kahaRequestString = "/mobile/interfaces/crowd/map.php?smartmet=1";
            kahaRequestString += ::makeKahaTimeRangeString(7 * 24);
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
                catch(std::exception &e)
                {
                    // Ei viitti laittaa n‰ihin korkeata severity tasoa, koska virheellisi‰ viestej‰ on aika paljon
                    ::makeMessageRelatedErrorLog("KaHa type message parsing failed", e, CatLog::Severity::Debug);
                }
            }
            return jsons.size();
        }
        catch(const std::exception&)
        {
            return 0;
        }
    }

    void Main::doUpdateApplicationCallback()
    {
        if(updateApplicationCallback_)
        {
            updateApplicationCallback_();
        }
    }

    void Main::createHakeQueryData()
    {
        bManager_->addTask([&] {
            auto msgs = hakeMessages_->peekMessages();
            if(msgs.empty())
                return;
            try
            {
                auto hakeProducer = NFmiProducer{ kFmiHakeMessages, "HAKE" };
                hakeData_ = queryDataMaker_->createQueryDataFrom(msgs, hakeProducer);
                doUpdateApplicationCallback();
            }
            catch(std::exception &e)
            {
                ::makeMessageRelatedErrorLog("Hake based queryData generation failed", e, CatLog::Severity::Warning);
            }
        });
    }

    void Main::createKahaQueryData()
    {
        bManager_->addTask([&] {
            auto msgs = kahaMessages_->peekMessages();
            if(msgs.empty())
                return;
            try
            {
                auto kahaProducer = NFmiProducer{ kFmiKaHaMessages, "KaHa" };
            kahaData_ = queryDataMaker_->createQueryDataFrom(msgs, kahaProducer);
            doUpdateApplicationCallback();
            }
            catch(std::exception &e)
            {
                ::makeMessageRelatedErrorLog("KaHa based queryData generation failed", e, CatLog::Severity::Warning);
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

    std::string Main::getHakeMessageAbsoluteFileFilter() const
    {
        // 1. Palauttaa vain json:eihin liittyv‰n polun, koska xml versiota ei ole en‰‰ k‰ytˆss‰.
        auto finalPath = configurer_->jsonPath;
        // 2. jsonPath:in per‰‰n pit‰‰ laittaa tarvittaessa hakemistoerotin
        if(!finalPath.empty() && finalPath.back() != '\\' && finalPath.back() != '/')
            finalPath += '\\';
        // 3. jsonFilter:ia pit‰‰ viritell‰, koska siin‰ on mukana merkki, joka kuuluu regex syntaksiin (1. '.' merkki)
        auto finalFileFilter = configurer_->jsonFilter;
        if(!finalFileFilter.empty() && finalFileFilter.front() == '.')
            finalFileFilter.erase(0, 1);
        NFmiFileString fileString = finalPath + finalFileFilter;
        fileString.NormalizeDelimiter();
        return std::string(fileString);
    }

}

#endif // DISABLE_CPPRESTSDK
