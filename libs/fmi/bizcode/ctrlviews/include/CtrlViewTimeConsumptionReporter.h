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
    private:
        std::string makeReadableDurationString(const std::chrono::duration<float> &durationValue);

        NFmiCtrlView *ctrlView_;
        std::string eventName_;
        std::string identifier_;
        std::chrono::system_clock::time_point startTime_;
        static size_t currentUpdateId_;
    };

}
