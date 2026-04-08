
#pragma once

#include <string>

#ifndef UNIX
#include "process_helpers.h"
#include "logging.h"

// T�m�n luokan ainoa tarkoitus oli poistaa �lyt�n m��r� riippuvuuksia MultiProcessTools-kirjaston template koodien
// ja NFmiEditMapGeneralDataDoc -luokan v�lilt� (ja liian moni muu kirjasto+luokka on taas riippuvainen GenDataDoc-luokasta).
class MultiProcessClientData
{
public:
    MultiProcessClientData(void);

    const std::string& MultiProcessLogPath(void) const {return itsMultiProcessLogPath;}
    void MultiProcessLogPath(const std::string &newPath) {itsMultiProcessLogPath = newPath;}
    // HUOM! t�t� pit�� kutsua ennen ensimm�ist� GetMultiProcessClientData -kutsua
    void PresetMultiProcessPoolOptions(const process_helpers::multi_process_pool_options &theMppOptions, logging::trivial::severity_level theMppLogLevel);
    process_helpers::multi_process_pool_options& MultiProcessPoolOptions(void) {return itsMultiProcessPoolOptions;}
    logging::trivial::severity_level MppLogLevel(void) const {return itsMppLogLevel;}

private:

    std::string itsMultiProcessLogPath;
    process_helpers::multi_process_pool_options itsMultiProcessPoolOptions;
    logging::trivial::severity_level itsMppLogLevel;
};
#else // UNIX
class MultiProcessClientData
{
public:
    MultiProcessClientData(void) = default;
    const std::string& MultiProcessLogPath(void) const {return itsMultiProcessLogPath;}
    void MultiProcessLogPath(const std::string &newPath) {itsMultiProcessLogPath = newPath;}
private:
    std::string itsMultiProcessLogPath;
};
#endif // UNIX
