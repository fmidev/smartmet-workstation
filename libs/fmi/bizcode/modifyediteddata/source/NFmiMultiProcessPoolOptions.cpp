
#include "NFmiMultiProcessPoolOptions.h"
#include "NFmiSettings.h"
#include "process_helpers.h"

NFmiMultiProcessPoolOptions::NFmiMultiProcessPoolOptions(void)
:itsMultiProcessPoolOptions()
,itsLogLevel(logging::trivial::info)
,itsBaseNameSpace()
{
}

NFmiMultiProcessPoolOptions::~NFmiMultiProcessPoolOptions(void)
{
}

void NFmiMultiProcessPoolOptions::InitFromSettings(const std::string &theBaseNameSpace)
{
	itsBaseNameSpace = theBaseNameSpace;

    itsMultiProcessPoolOptions.total_wait_time_limit_in_seconds = NFmiSettings::Optional<double>(std::string(itsBaseNameSpace + "::TotalWaitTimeLimitInSeconds"), itsMultiProcessPoolOptions.total_wait_time_limit_in_seconds);
    itsLogLevel = static_cast<logging::trivial::severity_level>(NFmiSettings::Optional<int>(std::string(itsBaseNameSpace + "::LogLevel"), itsLogLevel));
    double defaultTaskQueueSizeInMB = itsMultiProcessPoolOptions.task_queue_size_in_bytes / (double)process_helpers::gMegaByte;
    itsMultiProcessPoolOptions.task_queue_size_in_bytes = static_cast<size_t>(NFmiSettings::Optional<double>(std::string(itsBaseNameSpace + "::TaskQueueSizeInMB"), defaultTaskQueueSizeInMB) * process_helpers::gMegaByte);
    double defaultResultQueueSizeInMB = itsMultiProcessPoolOptions.result_queue_size_in_bytes / (double)process_helpers::gMegaByte;
    itsMultiProcessPoolOptions.result_queue_size_in_bytes = static_cast<size_t>(NFmiSettings::Optional<double>(std::string(itsBaseNameSpace + "::ResultQueueSizeInMB"), defaultResultQueueSizeInMB) * process_helpers::gMegaByte);
    itsMultiProcessPoolOptions.verbose_logging = NFmiSettings::Optional<bool>(std::string(itsBaseNameSpace + "::VerboseLogging"), itsMultiProcessPoolOptions.verbose_logging);
}
