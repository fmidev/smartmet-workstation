#include "NFmiCombinedMapModeState.h"

NFmiCombinedMapModeState::NFmiCombinedMapModeState() = default;

void NFmiCombinedMapModeState::initialize(int localMapSize, int wmsMapSize)
{
    localMapSize_ = localMapSize;
    wmsMapSize_ = wmsMapSize;
    totalMapSize_ = localMapSize_ + wmsMapSize_;
}

void NFmiCombinedMapModeState::next()
{
    combinedModeMapIndex_++;
    checkIndexOverFlow();
}

void NFmiCombinedMapModeState::previous()
{
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

bool NFmiCombinedMapModeState::isLocalMapInUse() const
{
    if(combinedModeMapIndex_ < localMapSize_)
        return true;
    else
        return false;
}

int NFmiCombinedMapModeState::currentMapSectionIndex() const
{
    if(isLocalMapInUse())
        return combinedModeMapIndex_;
    else
        return combinedModeMapIndex_ - localMapSize_;
}

void NFmiCombinedMapModeState::checkIndexUnderFlow()
{
    if(combinedModeMapIndex_ < 0)
        combinedModeMapIndex_ = totalMapSize_ - 1; // jos meni alle, mennään loppuun
}

void NFmiCombinedMapModeState::checkIndexOverFlow()
{
    if(combinedModeMapIndex_ >= totalMapSize_)
        combinedModeMapIndex_ = 0; // jos meni yli, palataan alkuun
}
