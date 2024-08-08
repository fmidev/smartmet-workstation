#pragma once

#include "NFmiHelpDataInfo.h"
#include <mutex>

// T�t� luokkaa k�ytet��n QueryDataToLocalCacheLoaderThread k�ytt�miss� rutiineissa.
// Jos k�ytettyihin HelpDataSystem asetuksiin tulee muutoksia k�ytt�j�n tekemill�
// asetuksien muutoksilla, niit� otetaan k�ytt��n t�m�n systeemin avulla.
// T�rkein motiivi t�lle on ett� muutokset saadaan toisistaan riippumattomiin ty�-s�ikeisiin
// mukaan thread turvallisesti.
// Siksi gWorkerHelpDataSystem luodaan dynaamisesti shared_ptr olioon ja sen kopioita jaetaan
// sitten niit� tarvitseville k�ytt��n. Kun uusi gWorkerHelpDataSystem olio luodaan, voivat
// eri ty�-s�ikeet k�ytt�� sen vanhaa versiota kunnes n�m� yhden datantiedoston kopioivat
// threadit lopettavat toimintansa.
class LocalCacheHelpDataSystem
{
    // T�m�n olion avulla working thread osaa lukea/kopioida haluttuja datoja
    std::shared_ptr<NFmiHelpDataInfoSystem> workerHelpDataSystemPtr;
    // T�m�n avulla p�ivitet��n datan luku asetuksia thread safetysti
    std::mutex settingsMutex;
public:
    LocalCacheHelpDataSystem();
    void InitHelpDataInfoSystem(const NFmiHelpDataInfoSystem& helpDataInfoSystem);
    void UpdateHelpDataInfoSystem(NFmiHelpDataInfoSystem& helpDataInfoSystem);
    std::shared_ptr<NFmiHelpDataInfoSystem> GetHelpDataInfoSystemPtr();
};
