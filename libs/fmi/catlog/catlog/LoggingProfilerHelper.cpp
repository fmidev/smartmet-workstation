#include "LoggingProfilerHelper.h"
#include "catlog/catlog.h"

// Ei aloiteta globaalia laskuria 1:st‰, koska lukujen 1-20 haku lokiviesteista voi saada paljon v‰‰ri‰ tuloksia
size_t LoggingProfilerHelper::globalUpdateId_ = 201;

std::string LoggingProfilerHelper::makeCurrentUpdateIdString()
{
    return std::string("(") + std::to_string(localUpdateId_) + ")";
}

LoggingProfilerHelper::LoggingProfilerHelper(const std::string& eventName)
    : eventName_(eventName)
    , startTime_(std::chrono::system_clock::now())
    , localUpdateId_(globalUpdateId_++)
{
    if(CatLog::doTraceLevelLogging())
    {
        std::string message = makeCurrentUpdateIdString() + " " + eventName_;
        message += " starts...";
        CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization);
    }
}

LoggingProfilerHelper::~LoggingProfilerHelper()
{
    doStopProfilingLogging();
}

// Jos k‰ytt‰j‰ haluaa lopettaa profiloinnin ennen destruktoria eli 
// ennen kuin tulee ulos siit‰ lokaalista scopesta, miss‰ profilointi olio on luotu.
void LoggingProfilerHelper::stopProfiling()
{
    doStopProfilingLogging();
}

void LoggingProfilerHelper::doStopProfilingLogging()
{
    if(!profilingHasStopped_)
    {
        profilingHasStopped_ = true;
        if(CatLog::doTraceLevelLogging())
        {
            auto endTime(std::chrono::system_clock::now());
            auto durationValue = endTime - startTime_;
            std::string message = makeCurrentUpdateIdString() + " Operation lasted ";
            message += makeReadableDurationString(durationValue);
            message += " for " + eventName_;
            CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization);
        }
    }
}

std::string LoggingProfilerHelper::makeReadableDurationString(const std::chrono::duration<float>& durationValue)
{
    std::string durationString;
    auto secondsValue = std::chrono::duration_cast<std::chrono::seconds>(durationValue);
    durationString += std::to_string(secondsValue.count());
    durationString += ".";
    auto milliSecondsPart = std::chrono::duration_cast<std::chrono::milliseconds>(durationValue).count() % 1000;
    if(milliSecondsPart < 10)
        durationString += "00";
    else if(milliSecondsPart < 100)
        durationString += "0";
    durationString += std::to_string(milliSecondsPart) + "s";

    return durationString;
}


