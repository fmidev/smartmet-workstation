
#include "MultiProcessClientData.h"

MultiProcessClientData::MultiProcessClientData(void)
:itsMultiProcessClientData()
,itsMultiProcessLogPath(default_logging_file)
,itsMultiProcessPoolOptions()
,itsMppLogLevel(logging::trivial::info)
,fUseTcpMasterProcess(true)
{
}

MultiProcessClientData::MultiProcessClientDataType& MultiProcessClientData::GetMultiProcessClientData(void) 
{
    if(!itsMultiProcessClientData)
    {
        init_logger("SmartMet", itsMppLogLevel, itsMultiProcessLogPath);
        work_queue_verbose_logging(itsMultiProcessPoolOptions.verbose_logging); // laitetaan my�s work_queue:n verbose-log tila p��lle
        itsMultiProcessClientData = MultiProcessClientDataType(new MultiProcessClientDataType::element_type(itsMultiProcessPoolOptions));
    }

    return itsMultiProcessClientData;
}

void MultiProcessClientData::PresetMultiProcessPoolOptions(const process_helpers::multi_process_pool_options &theMppOptions, logging::trivial::severity_level theMppLogLevel)
{
    itsMultiProcessPoolOptions = theMppOptions;
    itsMppLogLevel = theMppLogLevel;
}

bool MultiProcessClientData::IsMultiProcessClientDataInitialized(void) const
{
    return (itsMultiProcessClientData != 0);
}
