#pragma once

#include "source/rootData/RootData.h"

#include <functional>
#include <vector>
#include <utility>

class NFmiPoint;

namespace SmartMetDataUtilities 
{
    using StepFunction = std::function<bool()>;
    using StepperWithPossibilityToDoChecking = std::function<bool(NFmiPoint latlon)>;
    using SamplePoints = std::vector<NFmiPoint>;
    using PointPair = std::pair<NFmiPoint, NFmiPoint>;
}