#include "WmsSupport/WmsSupportState.h"

namespace Wms
{
    WmsSupportState::WmsSupportState(const Setup& setup)
    {
        backgroundsLength_ = static_cast<int>(setup.background.parsedServers.size());
        overlaysLenght_ = static_cast<int>(setup.overlay.parsedServers.size());
        layerGroupIndex_ = 0;
        overlayIndex_ = -1;
    }

    int WmsSupportState::getCurrentBackgroundIndex() const
    {
        return layerGroupIndex_;
    }

    int WmsSupportState::getCurrentOverlayIndex() const
    {
        return overlayIndex_;
    }

    void WmsSupportState::setBackgroundIndex(int index)
    {
        if(index < 0)
        {
            layerGroupIndex_ = backgroundsLength_ - 1;
        }
        else if(index > (backgroundsLength_ - 1))
        {
            layerGroupIndex_ = 0;
        }
        else
        {
            layerGroupIndex_ = index;
        }
    }

    void WmsSupportState::setOverlayIndex(int index)
    {
        if(index < -1)
        {
            overlayIndex_ = overlaysLenght_ - 1;
        }
        else if(index > (overlaysLenght_ - 1))
        {
            overlayIndex_ = -1;
        }
        else
        {
            overlayIndex_ = index;
        }
    }

}
