#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiRect.h"
#include "boost/shared_ptr.hpp"

class CDC;
class CRect;
class NFmiDrawParam;
class NFmiIsoLineData;
class CWnd;
class NFmiGriddingProperties;

namespace Toolmaster
{
#ifndef DISABLE_UNIRAS_TOOLMASTER
    // Here are normal function declarations if Uniras ToolMaster is supported
    void SetToolMastersDC(CDC* theDC, const CRect &theClientRect);
    void DoToolMasterGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, const NFmiRect &theRelativeRect, const NFmiGriddingProperties &griddingProperties, float theObservationRadiusRelative, const NFmiDataMatrix<float> &gridData, std::vector<float> &toolMasterGridValuesOut);
    std::string MakeAvsToolmasterVersionString();
    void CloseToolMaster();
    int InitToolMaster(CWnd *mainWindow, bool useToolMasterIfAvailable);
    void InitToolMasterColors(bool fToolMasterAvailable);
    bool DoToolMasterInitialization(CWnd *mainWindow, bool useToolMasterIfAvailable);
#else
    // Here are dymmy versions of functions with dummy definitions, if Uniras ToolMaster isn't supported (must be inline because VC++ linker...)
    inline void SetToolMastersDC(CDC* theDC, const CRect &theClientRect)
    {}
    inline void DoToolMasterGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, const NFmiRect &theRelativeRect, int theGriddingFunction, float theObservationRadiusRelative, const NFmiDataMatrix<float> &gridData, std::vector<float> &toolMasterGridValuesOut)
    {}
    inline std::string MakeAvsToolmasterVersionString()
    { return ""; }
    inline void CloseToolMaster()
    {}
    inline int InitToolMaster(CWnd *mainWindow, bool useToolMasterIfAvailable)
    { return -1; }
    inline void InitToolMasterColors(bool fToolMasterAvailable)
    {}
    inline bool DoToolMasterInitialization(CWnd *mainWindow, bool useToolMasterIfAvailable)
    { 
        InitToolMaster(mainWindow, false);
        return false;
    }
#endif // DISABLE_UNIRAS_TOOLMASTER
}
