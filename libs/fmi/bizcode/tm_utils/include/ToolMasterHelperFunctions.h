#pragma once

#include "NFmiDataMatrix.h"
#include "boost/shared_ptr.hpp"

class CDC;
class CRect;
class NFmiDrawParam;
class NFmiIsoLineData;
class CWnd;

namespace Toolmaster
{
    void SetToolMastersDC(CDC* theDC, const CRect &theClientRect);
    void DoToolMasterGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, const NFmiRect &theRelativeRect, int theGriddingFunction, float theObservationRadiusRelative, const NFmiDataMatrix<float> &gridData, std::vector<float> &toolMasterGridValuesOut);
    void FillChangingColorIndicesForSimpleIsoline(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData *theIsoLineData, float step, float startValue, float endValue);
    std::string MakeAvsToolmasterVersionString();
    void CloseToolMaster();
    int InitToolMaster(CWnd *mainWindow, bool useToolMasterIfAvailable);
    void InitToolMasterColors(bool fToolMasterAvailable);
    bool DoToolMasterInitialization(CWnd *mainWindow, bool useToolMasterIfAvailable);
}
