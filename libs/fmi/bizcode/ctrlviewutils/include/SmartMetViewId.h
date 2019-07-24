#pragma once

#include <string>
#include <math.h>

// Here are view ids for each SmartMet views.
// Each view is represented by one bit in 64-bit number (size_t type).
// First 5 bits are reserved for different map-views (there are only 3 in present system).
// These enums are used e.g. to tell system which views need to be updated.
enum class SmartMetViewId : size_t
{
    NoViews = 0,
    MainMapView = 1 << 0,
    MapView2 = 1 << 1,
    MapView3 = 1 << 2,
//    MapView4 = 1 << 3,
//    MapView5 = 1 << 4,
    TimeSerialView = 1 << 5,
    SoundingView = 1 << 6,
    CrossSectionView = 1 << 7,
    SmartToolDlg = 1 << 8,
    TrajectoryView = 1 << 9,
    StationDataTableView = 1 << 10,
    DataFilterToolDlg = 1 << 11,
    BrushToolDlg = 1 << 12,
    LogViewer = 1 << 13,
    ParameterSelectionDlg = 1 << 14,
    DataQualityCheckerView = 1 << 15,
    ViewMacroDlg = 1 << 16,
    ZoomDlg = 1 << 17,
    SynopPlotSettingsDlg = 1 << 18,
    WarningCenterDlg = 1 << 19,
    SeaIcingDlg = 1 << 20,
    WindTableDlg = 1 << 21,
    IgnoreStationsDlg = 1 << 22,
    CaseStudyDlg = 1 << 23,
    BetaProductionDlg = 1 << 24,

    LastViewId = BetaProductionDlg, // set the last view id here also
    // Special combo view ids
    AllMapViews = MainMapView | MapView2 | MapView3, // | MapView4 | MapView5,
    // Laitetaan kaikki bitit päälle kun size_t:hen (unsigned __int64/long long) asetetaan -1
    AllViews = -1 // kaikkien näyttöjen päivitys
};

bool SmartMetViewIdFlagCheck(SmartMetViewId viewsFlag, SmartMetViewId checkedSingleViewId);
std::string SmartMetViewIdGetName(SmartMetViewId singleViewId);
std::string SmartMetViewIdGetNameList(SmartMetViewId viewsFlag);
SmartMetViewId GetWantedMapViewIdFlag(int theMapViewDescTopIndex);

inline SmartMetViewId operator | (SmartMetViewId lhs, SmartMetViewId rhs)
{
    using T = std::underlying_type_t <SmartMetViewId>;
    return static_cast<SmartMetViewId>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
