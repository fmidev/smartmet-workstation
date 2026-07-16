#pragma once

#include <functional>

// This class knows the map counts of the different modes (local and wms).
// You can move to the next or previous map, taking the combined map count into account.
// You can directly set the wanted combined index.
// You can ask whether the current index points to a local or a wms map (isLocalMapInUse).
// You can ask the mode-specific (local/wms) index of the current combined index (currentMapSectionIndex method).
class NFmiCombinedMapModeState
{
    using LocalOnlyMapModeUsedFunction = std::function<bool()>;

    int localMapSize_ = 0;
    int wmsMapSize_ = 0;
    int totalMapSize_ = 0;
    // Zero-based index that points to some map limited by the totalMapSize_ size.
    int combinedModeMapIndex_ = 0;
    // Zero-based index that points to some map limited by the localMapSize_ size.
    // This is meant to be updated (alongside combinedModeMapIndex_) whenever we are in local-only mode and combinedModeMapIndex_
    // is within the range of local indices. Its purpose is to be a short index memory, e.g. when the user jumps from local mode to combined mode and back.
    int lastUsedLocalModeMapIndex_ = 0;
    // This is used to ask whether we are in local or combined-map mode.
    LocalOnlyMapModeUsedFunction localOnlyMapModeUsedFunction_;
    // Whether we are in background or overlay state. In overlay state the index can also be -1, in which case nothing is drawn.
    bool backgroundCase_ = true;
public:
    NFmiCombinedMapModeState();
    void initialize(int localMapSize, int wmsMapSize, LocalOnlyMapModeUsedFunction &localOnlyMapModeUsedFunction, bool backgroundCase);

    void next();
    void previous();
    void changeLayer(bool goForward);
    int combinedModeMapIndex() const;
    void combinedModeMapIndex(int index);
    bool isLocalMapCurrentlyInUse() const;
    int currentMapSectionIndex() const;
    bool isLocalOnlyMapModeInUse() const;

private:
    void checkIndexUnderFlow();
    void checkIndexOverFlow();
    void mapIndexIsAboutToChangeChecks();
    bool isMapIndexInLocalSection() const;
    void updateLastUsedLocalModeMapIndex();
    int getMinimumIndex() const;
};
