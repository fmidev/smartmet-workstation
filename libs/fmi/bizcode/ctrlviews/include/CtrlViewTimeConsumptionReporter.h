#pragma once

#include <string>
#include <chrono>

class NFmiCtrlView;

namespace CtrlViewUtils
{
    class CtrlViewTimeConsumptionReporter
    {
    public:
        CtrlViewTimeConsumptionReporter(NFmiCtrlView *ctrlView, const std::string &eventName);
        ~CtrlViewTimeConsumptionReporter();

        static void makeSeparateTraceLogging(const std::string &message, NFmiCtrlView *ctrlView);
        static std::string makeCtrlViewIdentifier(NFmiCtrlView *ctrlView);
        static void setCurrentUpdateId(size_t currentUpdateId);
        static std::string makeCurrentUpdateIdString();
        static void increaseCurrentUpdateId();
    private:
        std::string makeReadableDurationString(const std::chrono::duration<float> &durationValue);

        NFmiCtrlView *ctrlView_;
        std::string eventName_;
        std::string identifier_;
        std::chrono::system_clock::time_point startTime_;
        // Lasketaan update counter indeksi‰, jonka avulla voidaan lokiviesteist‰ hakea tietyn update kierroksen lokituksia.
        // Ei aloiteta lukua 1:st‰, koska jos etsii vain lukua 1 tai 2-9, tulee liian paljon osumia v‰‰rist‰ lokiviesteist‰
        static size_t currentUpdateId_;
    };

}
