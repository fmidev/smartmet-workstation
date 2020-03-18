#include "NFmiCombinedMapModeState.h"

NFmiCombinedMapModeState::NFmiCombinedMapModeState() = default;

void NFmiCombinedMapModeState::initialize(int localMapSize, int wmsMapSize, LocalOnlyMapModeUsedFunction & localOnlyMapModeUsedFunction)
{
    localMapSize_ = localMapSize;
    wmsMapSize_ = wmsMapSize;
    totalMapSize_ = localMapSize_ + wmsMapSize_;
    localOnlyMapModeUsedFunction_ = localOnlyMapModeUsedFunction;
}

void NFmiCombinedMapModeState::next()
{
    mapIndexIsAboutToChangeChecks();
    combinedModeMapIndex_++;
    checkIndexOverFlow();
}

void NFmiCombinedMapModeState::previous()
{
    mapIndexIsAboutToChangeChecks();
    combinedModeMapIndex_--;
    checkIndexUnderFlow();
}

int NFmiCombinedMapModeState::combinedModeMapIndex() const
{
    return combinedModeMapIndex_;
}

void NFmiCombinedMapModeState::combinedModeMapIndex(int index)
{
    combinedModeMapIndex_ = index;
    checkIndexOverFlow();
    checkIndexUnderFlow();
}

bool NFmiCombinedMapModeState::isLocalOnlyMapModeInUse() const
{
    if(localOnlyMapModeUsedFunction_)
        return localOnlyMapModeUsedFunction_();
    else
        return true;

}

bool NFmiCombinedMapModeState::isLocalMapCurrentlyInUse() const
{
    if(isLocalOnlyMapModeInUse())
        return true;
    else if(isMapIndexInLocalSection())
        return true;
    else
        return false;
}

bool NFmiCombinedMapModeState::isMapIndexInLocalSection() const
{
    return combinedModeMapIndex_ < localMapSize_;
}

// T�m�n funktion on teht�v� varmistaa ett� jos ollaan local-only moodissa,
// ja k�ytt�j� haluaa muuttaa karttaindeksi� (esim. next/previous)
// ett� combinedModeMapIndex_ ja lastUsedLocalModeMapIndex_ ovat samoja, ennen 
// kuin itse combinedModeMapIndex_:ia aletaan taas muuttamaan.
void NFmiCombinedMapModeState::mapIndexIsAboutToChangeChecks()
{
    if(isLocalOnlyMapModeInUse())
    {
        if(combinedModeMapIndex_ != lastUsedLocalModeMapIndex_)
            combinedModeMapIndex_ = lastUsedLocalModeMapIndex_;
    }
}

int NFmiCombinedMapModeState::currentMapSectionIndex() const
{
    if(isLocalOnlyMapModeInUse())
        return lastUsedLocalModeMapIndex_; // Palautetaan lokaalin kartan indeksi 'erillismuistista'
    else if(isMapIndexInLocalSection())
        return combinedModeMapIndex_; // Ollaan combine moodissa, mutta lokaali sectiossa, palautetaan normi indeksi
    else
        return combinedModeMapIndex_ - localMapSize_; // Palautetaan wms sectioon osoittava indeksi muutettuna sen omaksi 0-kantaiseksi indeksiksi
}

void NFmiCombinedMapModeState::checkIndexUnderFlow()
{
    if(combinedModeMapIndex_ < 0)
        combinedModeMapIndex_ = totalMapSize_ - 1; // jos meni alle, menn��n loppuun
    updateLastUsedLocalModeMapIndex();
}

void NFmiCombinedMapModeState::checkIndexOverFlow()
{
    if(combinedModeMapIndex_ >= totalMapSize_)
        combinedModeMapIndex_ = 0; // jos meni yli, palataan alkuun
    updateLastUsedLocalModeMapIndex();
}

void NFmiCombinedMapModeState::updateLastUsedLocalModeMapIndex()
{
    if(isMapIndexInLocalSection())
        lastUsedLocalModeMapIndex_ = combinedModeMapIndex_;
}
