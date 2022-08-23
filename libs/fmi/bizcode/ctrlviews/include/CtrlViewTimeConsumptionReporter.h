#pragma once

#include "catlog/catlog.h"
#include <string>
#include <chrono>

class NFmiCtrlView;

namespace CtrlViewUtils
{
    class CtrlViewTimeConsumptionReporter
    {
    public:
        CtrlViewTimeConsumptionReporter(NFmiCtrlView *ctrlView, const std::string &eventName, CatLog::Severity logLevel = CatLog::Severity::Trace);
        ~CtrlViewTimeConsumptionReporter();

        static void makeSeparateTraceLogging(const std::string &message, NFmiCtrlView *ctrlView);
        static std::string makeCtrlViewIdentifier(NFmiCtrlView *ctrlView);
        static void setCurrentUpdateId(size_t currentUpdateId);
        static std::string makeCurrentUpdateIdString();
        static void increaseCurrentUpdateId();
    private:
        std::string makeReadableDurationString(const std::chrono::duration<float> &durationValue);
        bool doLogging() const;
        bool addUpdateId() const;

        NFmiCtrlView *ctrlView_;
        std::string eventName_;
        std::string identifier_;
        std::chrono::system_clock::time_point startTime_;
        CatLog::Severity logLevel_ = CatLog::Severity::Trace;
        // Lasketaan update counter indeksi‰, jonka avulla voidaan lokiviesteist‰ hakea tietyn update kierroksen lokituksia.
        // Ei aloiteta lukua 1:st‰, koska jos etsii vain lukua 1 tai 2-9, tulee liian paljon osumia v‰‰rist‰ lokiviesteist‰
        static size_t currentUpdateId_;
    };

}
