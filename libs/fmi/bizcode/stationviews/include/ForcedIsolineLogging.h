#pragma once

#include "boost/shared_ptr.hpp"

class NFmiDrawParam;
class NFmiIsoLineData;

namespace ForcedLogging
{
    void IsolineDrawingInfo(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData, bool stationData, bool toolMasterUsed, bool mapViewCase);
}
