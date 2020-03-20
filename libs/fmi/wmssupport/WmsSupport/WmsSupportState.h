#pragma once
#include "WmsSupport/Setup.h"

#include <map>

namespace Wms
{
    class WmsSupportState
    {
        int layerGroupIndex_;
        int overlayIndex_;
        int backgroundsLength_;
        int overlaysLenght_;
    public:
        WmsSupportState(const Setup& setup);

        int getCurrentBackgroundIndex() const;
        int getCurrentOverlayIndex() const;
        void setBackgroundIndex(int index);
        void setOverlayIndex(int index);
        int getBackgroundsLength() const { return backgroundsLength_; }
        int getOverlaysLenght() const { return overlaysLenght_; }
    };
}