#pragma once

#include "NFmiDataMatrix.h"

class CDC;
class CRect;

namespace Toolmaster
{
    void SetToolMastersDC(CDC* theDC, const CRect &theClientRect);
    void DoToolMasterGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, const NFmiRect &theRelativeRect, int theGriddingFunction, float theObservationRadiusRelative, const NFmiDataMatrix<float> &gridData, std::vector<float> &toolMasterGridValuesOut);
}
