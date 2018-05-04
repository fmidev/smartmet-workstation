#pragma once

#include "logging.h"
#include "process_helpers.h"

class NFmiMultiProcessPoolOptions
{
public:
	NFmiMultiProcessPoolOptions(void);
	~NFmiMultiProcessPoolOptions(void);

	void InitFromSettings(const std::string &theBaseNameSpace);
//	void StoreToSettings(void) const;

    process_helpers::multi_process_pool_options& MultiProcessPoolOptions(void) {return itsMultiProcessPoolOptions;}
    logging::trivial::severity_level LogLevel(void) const {return itsLogLevel;}
    void LogLevel(logging::trivial::severity_level newValue) {itsLogLevel = newValue;}

private:
    process_helpers::multi_process_pool_options itsMultiProcessPoolOptions;
    logging::trivial::severity_level itsLogLevel;

	std::string itsBaseNameSpace;
};
