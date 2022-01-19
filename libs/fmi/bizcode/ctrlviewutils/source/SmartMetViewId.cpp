#include "SmartMetViewId.h"
#include "SpecialDesctopIndex.h"

bool SmartMetViewIdFlagCheck(SmartMetViewId viewsFlag, SmartMetViewId checkedSingleViewId)
{
    return ((static_cast<size_t>(viewsFlag) & static_cast<size_t>(checkedSingleViewId)) == static_cast<size_t>(checkedSingleViewId));
}

std::string SmartMetViewIdGetName(SmartMetViewId singleViewId)
{
    switch(singleViewId)
    {
    case SmartMetViewId::MainMapView:
        return "MainMapView";
    case SmartMetViewId::MapView2:
        return "MapView2";
    case SmartMetViewId::MapView3:
        return "MapView3";
    //case SmartMetViewId::MapView4:
    //    return "MapView4";
    //case SmartMetViewId::MapView5:
    //    return "MapView5";
    case SmartMetViewId::TimeSerialView:
        return "TimeSerialView";
    case SmartMetViewId::SoundingView:
        return "SoundingView";
    case SmartMetViewId::CrossSectionView:
        return "CrossSectionView";
    case SmartMetViewId::SmartToolDlg:
        return "SmartToolDlg";
    case SmartMetViewId::TrajectoryView:
        return "TrajectoryView";
    case SmartMetViewId::StationDataTableView:
        return "StationDataTableView";
    case SmartMetViewId::DataFilterToolDlg:
        return "DataFilterToolDlg";
    case SmartMetViewId::BrushToolDlg:
        return "BrushToolDlg";
    case SmartMetViewId::LogViewer:
        return "LogViewer";
    case SmartMetViewId::ParameterSelectionDlg:
        return "ParameterSelectionDlg";
    case SmartMetViewId::DataQualityCheckerView:
        return "DataQualityCheckerView";
    case SmartMetViewId::ViewMacroDlg:
        return "ViewMacroDlg";
    case SmartMetViewId::ZoomDlg:
        return "ZoomDlg";
    case SmartMetViewId::SynopPlotSettingsDlg:
        return "SynopPlotSettingsDlg";
    case SmartMetViewId::WarningCenterDlg:
        return "WarningCenterDlg";
    case SmartMetViewId::WindTableDlg:
        return "WindTableDlg";
    case SmartMetViewId::IgnoreStationsDlg:
        return "IgnoreStationsDlg";
    case SmartMetViewId::CaseStudyDlg:
        return "CaseStudyDlg";
    case SmartMetViewId::BetaProductionDlg:
        return "BetaProductionDlg";
    default:
        return "unknown-view";
    }
}

std::string SmartMetViewIdGetNameList(SmartMetViewId viewsFlag)
{
    std::string nameList;
    int counter = 0;
    SmartMetViewId checkedViewId = SmartMetViewId::MainMapView;
    for(;;)
    {
        if(SmartMetViewIdFlagCheck(viewsFlag, checkedViewId))
        {
            if(counter++)
                nameList += ",";
            nameList += SmartMetViewIdGetName(checkedViewId);
        }
        if(checkedViewId == SmartMetViewId::LastViewId)
            break;
        checkedViewId = static_cast<SmartMetViewId>(static_cast<size_t>(checkedViewId) << 1);
    }
    return nameList;
}

SmartMetViewId GetWantedMapViewIdFlag(int theMapViewDescTopIndex)
{
    if((theMapViewDescTopIndex >= 0 && theMapViewDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex) || 
		theMapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView || 
		theMapViewDescTopIndex == CtrlViewUtils::kFmiTimeSerialView)
    {
        switch(theMapViewDescTopIndex)
        {
        case 0:
            return SmartMetViewId::MainMapView;
        case 1:
            return SmartMetViewId::MapView2;
        case 2:
            return SmartMetViewId::MapView3;
		case 98:
			return SmartMetViewId::CrossSectionView;
		case 99:
			return SmartMetViewId::TimeSerialView;
        default:
            break;
        }
    }
    return SmartMetViewId::NoViews;
}

SmartMetViewId GetUpdatedViewIdMaskForEditingData()
{
    SmartMetViewId updatedViewIds = SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView | SmartMetViewId::StationDataTableView | SmartMetViewId::WindTableDlg;
    return updatedViewIds;
}
