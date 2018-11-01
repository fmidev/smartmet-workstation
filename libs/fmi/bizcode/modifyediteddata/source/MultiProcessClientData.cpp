
#include "MultiProcessClientData.h"

MultiProcessClientData::MultiProcessClientData(void)
:itsMultiProcessLogPath(default_logging_file)
,itsMultiProcessPoolOptions()
,itsMppLogLevel(logging::trivial::info)
{
}

void MultiProcessClientData::PresetMultiProcessPoolOptions(const process_helpers::multi_process_pool_options &theMppOptions, logging::trivial::severity_level theMppLogLevel)
{
    itsMultiProcessPoolOptions = theMppOptions;
    itsMppLogLevel = theMppLogLevel;
}
