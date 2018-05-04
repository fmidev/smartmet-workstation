
#pragma once

#include "process_helpers.h"

// T‰m‰n luokan ainoa tarkoitus oli poistaa ‰lytˆn m‰‰r‰ riippuvuuksia MultiProcessTools-kirjaston template koodien
// ja NFmiEditMapGeneralDataDoc -luokan v‰lilt‰ (ja liian moni muu kirjasto+luokka on taas riippuvainen GenDataDoc-luokasta).
class MultiProcessClientData
{
public:
    typedef boost::shared_ptr<process_helpers::master_process_stuff<task_queue_t, result_queue_t> > MultiProcessClientDataType;

    MultiProcessClientData(void);

    MultiProcessClientDataType& GetMultiProcessClientData(void);
    const std::string& MultiProcessLogPath(void) const {return itsMultiProcessLogPath;}
    void MultiProcessLogPath(const std::string &newPath) {itsMultiProcessLogPath = newPath;}
    // HUOM! t‰t‰ pit‰‰ kutsua ennen ensimm‰ist‰ GetMultiProcessClientData -kutsua
    void PresetMultiProcessPoolOptions(const process_helpers::multi_process_pool_options &theMppOptions, logging::trivial::severity_level theMppLogLevel);
    bool IsMultiProcessClientDataInitialized(void) const;
    process_helpers::multi_process_pool_options& MultiProcessPoolOptions(void) {return itsMultiProcessPoolOptions;}
    bool UseTcpMasterProcess(void) const {return fUseTcpMasterProcess;}
    void UseTcpMasterProcess(bool newValue) {fUseTcpMasterProcess = newValue;}
    logging::trivial::severity_level MppLogLevel(void) const {return itsMppLogLevel;}

private:

    MultiProcessClientDataType itsMultiProcessClientData;
    std::string itsMultiProcessLogPath;
    process_helpers::multi_process_pool_options itsMultiProcessPoolOptions; // t‰m‰ asetetaan etuk‰teen, ennen kuin itsMultiProcessClientData -dataosio alustetaan, halusin viivytt‰‰ itsMultiProcessClientData:n alustusta mahd. pitk‰‰n, siksi monimutkaisia alustus virityksi‰
    logging::trivial::severity_level itsMppLogLevel;
    bool fUseTcpMasterProcess; // k‰ytet‰‰nkˆ vanhaa CP muokkaus MasterProcess-systeemi‰ (false), vai uutta Tcp-pohjaista systeemi‰ (true)
};
