#pragma once

#include <string>
#include <chrono>

class LoggingProfilerHelper
{
public:
    LoggingProfilerHelper(const std::string& eventName);
    ~LoggingProfilerHelper();

    void stopProfiling();
    std::string makeCurrentUpdateIdString();
private:
    std::string makeReadableDurationString(const std::chrono::duration<float>& durationValue);
    void doStopProfilingLogging();

    std::string eventName_;
    std::chrono::system_clock::time_point startTime_;
    size_t localUpdateId_;
    static size_t globalUpdateId_;
    bool profilingHasStopped_ = false;
};

