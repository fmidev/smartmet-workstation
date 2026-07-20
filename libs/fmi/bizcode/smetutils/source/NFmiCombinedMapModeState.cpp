#include "NFmiCombinedMapModeState.h"

NFmiCombinedMapModeState::NFmiCombinedMapModeState() = default;

void NFmiCombinedMapModeState::initialize(int localMapSize, int wmsMapSize, LocalOnlyMapModeUsedFunction & localOnlyMapModeUsedFunction, bool backgroundCase)
{
    localMapSize_ = localMapSize;
    wmsMapSize_ = wmsMapSize;
    totalMapSize_ = localMapSize_ + wmsMapSize_;
    localOnlyMapModeUsedFunction_ = localOnlyMapModeUsedFunction;
    backgroundCase_ = backgroundCase;
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

void NFmiCombinedMapModeState::changeLayer(bool goForward)
{
    if(goForward)
        next();
    else
        previous();
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

// This function must ensure that if we are in local-only mode,
// and the user wants to change the map index (e.g. next/previous),
// that combinedModeMapIndex_ and lastUsedLocalModeMapIndex_ are the same, before
// combinedModeMapIndex_ itself starts being changed again.
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
        // Return the local map index from the 'separate memory'
        return lastUsedLocalModeMapIndex_;
    else if(isMapIndexInLocalSection())
        // We are in combine mode, but in the local section, return the normal index
        return combinedModeMapIndex_;
    else
        // Return the index pointing to the wms section, converted to its own 0-based index
        return combinedModeMapIndex_ - localMapSize_;
}

void NFmiCombinedMapModeState::checkIndexUnderFlow()
{
    if(combinedModeMapIndex_ < getMinimumIndex())
    {
        if(isLocalOnlyMapModeInUse())
            // If it went below, go to the end of the local maps
            combinedModeMapIndex_ = localMapSize_ - 1;
        else
            // If it went below, go to the end of the total maps
            combinedModeMapIndex_ = totalMapSize_ - 1;
    }
    updateLastUsedLocalModeMapIndex();
}

void NFmiCombinedMapModeState::checkIndexOverFlow()
{
    if(isLocalOnlyMapModeInUse())
    {
        if(combinedModeMapIndex_ >= localMapSize_)
            // If it went over, return to the beginning
            combinedModeMapIndex_ = getMinimumIndex();
    }
    else if(combinedModeMapIndex_ >= totalMapSize_)
        // If it went over, return to the beginning
        combinedModeMapIndex_ = getMinimumIndex();
    updateLastUsedLocalModeMapIndex();
}

void NFmiCombinedMapModeState::updateLastUsedLocalModeMapIndex()
{
    if(isMapIndexInLocalSection())
        lastUsedLocalModeMapIndex_ = combinedModeMapIndex_;
}

int NFmiCombinedMapModeState::getMinimumIndex() const
{
    if(backgroundCase_)
        return 0;
    else
        return -1;
}
