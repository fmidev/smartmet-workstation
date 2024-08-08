#pragma once

#include "NFmiHelpDataInfo.h"
#include <mutex>

// Tätä luokkaa käytetään QueryDataToLocalCacheLoaderThread käyttämissä rutiineissa.
// Jos käytettyihin HelpDataSystem asetuksiin tulee muutoksia käyttäjän tekemillä
// asetuksien muutoksilla, niitä otetaan käyttöön tämän systeemin avulla.
// Tärkein motiivi tälle on että muutokset saadaan toisistaan riippumattomiin työ-säikeisiin
// mukaan thread turvallisesti.
// Siksi gWorkerHelpDataSystem luodaan dynaamisesti shared_ptr olioon ja sen kopioita jaetaan
// sitten niitä tarvitseville käyttöön. Kun uusi gWorkerHelpDataSystem olio luodaan, voivat
// eri työ-säikeet käyttää sen vanhaa versiota kunnes nämä yhden datantiedoston kopioivat
// threadit lopettavat toimintansa.
class LocalCacheHelpDataSystem
{
    // Tämän olion avulla working thread osaa lukea/kopioida haluttuja datoja
    std::shared_ptr<NFmiHelpDataInfoSystem> workerHelpDataSystemPtr;
    // Tämän avulla päivitetään datan luku asetuksia thread safetysti
    std::mutex settingsMutex;
public:
    LocalCacheHelpDataSystem();
    void InitHelpDataInfoSystem(const NFmiHelpDataInfoSystem& helpDataInfoSystem);
    void UpdateHelpDataInfoSystem(NFmiHelpDataInfoSystem& helpDataInfoSystem);
    std::shared_ptr<NFmiHelpDataInfoSystem> GetHelpDataInfoSystemPtr();
};
