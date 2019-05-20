#pragma once
#ifndef DISABLE_CPPRESTSDK

#include "HakeSystemConfigurations.h"
#include "HakeMessage/HakeMessages.h"
#include "HakeMessage/KahaMessages.h"
#include "HakeMessage/Configurer.h"
#include "HakeMessage/Logger.h"
#include "HakeMessage/Io.h"
#include "HakeMessage/QueryDataMaker.h"

#include "NFmiQueryData.h"

#include <webclient/CppRestClient.h>
#include <cppback/background-manager.h>

#include <functional>
#include <vector>
#include <chrono>
#include <future>

namespace HakeMessage
{
    class Main
    {
        std::shared_ptr<Web::Client> client_;

        std::unique_ptr<Io> io_;
        std::unique_ptr<QueryDataMaker> queryDataMaker_;

        std::unique_ptr<HakeMessages> hakeMessages_;
        std::unique_ptr<KahaMessages> kahaMessages_;
        std::unique_ptr<NFmiQueryData> hakeData_;
        std::unique_ptr<NFmiQueryData> kahaData_;
        std::unique_ptr<Configurer> configurer_;

        std::function<void()> updateApplicationCallback_;
        std::unique_ptr<Logger> logger_;
        std::shared_ptr<cppback::BackgroundManager> bManager_;

        std::unique_ptr<HakeLegacySupport::HakeSystemConfigurations> legacyData_;
    public:
        Main();
        
        void initialize();
        void goToWorkAfter(std::chrono::milliseconds delay);
        bool isThereAnyWorkToDo() const;

        void setLogger(std::unique_ptr<Logger> logger);
        void setUpdateApplicationCallback(std::function<void()> updateApplicationCallback);

        std::vector<HakeMsg> getHakeMessages(const NFmiMetTime &startTime, const NFmiMetTime &endTime, const NFmiArea &area) const;
        std::vector<KahaMsg> getKahaMessages(const NFmiMetTime &startTime, const NFmiMetTime &endTime, const NFmiArea &area) const;

        void createHakeQueryData();
        void createKahaQueryData();
        std::unique_ptr<NFmiQueryData> getHakeQueryData();
        std::unique_ptr<NFmiQueryData> getKahaQueryData();

        void kill();
        bool isDead(std::chrono::milliseconds wait);

        HakeLegacySupport::HakeSystemConfigurations& getLegacyData();
    private:
        void handleHakeMessages(unsigned int maxNumberOfMessagesToRead);
        void handleKahaMessages();
        size_t handleHakeJson(unsigned int maxNumberOfMessagesToRead);
        size_t handleHakeXml(unsigned int maxNumberOfMessagesToRead);
        size_t handleKahaJson();
        void doUpdateApplicationCallback();
    };
}

#endif // DISABLE_CPPRESTSDK
