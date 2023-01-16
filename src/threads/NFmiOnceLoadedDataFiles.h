#pragma once

#include "NFmiMilliSecondTimer.h"
#include <mutex> 
#include <set> 
#include <string> 

struct NFmiCachedDataFileInfo;

class NFmiOnceLoadedDataFiles
{
    // Aletaan pit‰m‰‰n kirjaa mitk‰ tiedostot on jo kerran kopioitu (tai yritetty kopioida) serverilt‰ lokaali cacheen.
    // On tilanteita, miss‰ samaa tiedostoa yritet‰‰n ladata uudestaan ja uudestaan cacheen ja n‰in kulutetaan turhaan
    // verkkoa ja mahdollisesti puretaan zipattua tiedostoa jolloin kulutetaan turhaan CPU:ta.
    std::set<std::string> onceLoadedFilePathsOnServer_;
    // T‰m‰n avulla k‰ytet‰‰n ja t‰ytet‰‰n onceLoadedFilePathsOnServer_ containeria thread turvallisesti
    std::mutex onceLoadedFilePathsOnServerMutex_;
    NFmiNanoSecondTimer onceLoadedFilePathsOnServerTimer_;
public:
    NFmiOnceLoadedDataFiles();

    bool checkIfFileHasBeenLoadedEarlier(NFmiCachedDataFileInfo& cachedDataFileInfo);
private:
    void doClearanceChecks();
};
