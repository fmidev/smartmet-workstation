#ifdef UNIX

#include "qt_application_interface.h"

#include "NFmiApplicationWinRegistry.h"
#include "catlog/catlog.h"

#include <set>

namespace
{
    // Logs the first call to each unimplemented entry point. Repeating it on every
    // document update would drown the log.
    void logOnce(const char* theFunctionName)
    {
        static std::set<std::string> alreadyLogged;
        if(alreadyLogged.insert(theFunctionName).second)
        {
            CatLog::logMessage(std::string("ApplicationInterface::") + theFunctionName +
                                   " is not implemented in the Qt port yet",
                               CatLog::Severity::Debug, CatLog::Category::Visualization);
        }
    }
}

QtApplicationInterface::QtApplicationInterface() = default;
QtApplicationInterface::~QtApplicationInterface() = default;

QtApplicationInterface& QtApplicationInterface::install()
{
    static QtApplicationInterface instance;
    ApplicationInterface::GetApplicationInterfaceImplementation =
        []() -> ApplicationInterface* { return &instance; };
    return instance;
}

void QtApplicationInterface::requestRepaint(const std::string& reason)
{
    if(itsRepaintCallback)
        itsRepaintCallback();
    else
        CatLog::logMessage("Repaint requested before the window was ready: " + reason,
                           CatLog::Severity::Debug, CatLog::Category::Visualization);
}

// ---- redraw requests --------------------------------------------------------

void QtApplicationInterface::RefreshApplicationViewsAndDialogs(
    const std::string& reasonForUpdate, bool, bool, int)
{
    requestRepaint(reasonForUpdate);
}

void QtApplicationInterface::RefreshApplicationViewsAndDialogs(
    const std::string& reasonForUpdate, SmartMetViewId, bool, bool, int)
{
    requestRepaint(reasonForUpdate);
}

void QtApplicationInterface::UpdateMapView(unsigned int)
{
    requestRepaint("UpdateMapView");
}

void QtApplicationInterface::UpdateAllViewsAndDialogs(const std::string& reasonForUpdate, bool)
{
    requestRepaint(reasonForUpdate);
}

void QtApplicationInterface::InvalidateMainMapView(bool)
{
    requestRepaint("InvalidateMainMapView");
}

void QtApplicationInterface::UpdateOnlyExtraMapViews(bool, bool)
{
    // The Qt port has a single map view, so there are no extra views to update.
}

// ---- not implemented yet ----------------------------------------------------

void QtApplicationInterface::ParameterSelectionSystemUpdateTimerStart(int) { logOnce("ParameterSelectionSystemUpdateTimerStart"); }
void QtApplicationInterface::SetNotificationMessage(const std::string& theNotificationMsgStr, const std::string&, int, int, bool)
{
    CatLog::logMessage("Notification: " + theNotificationMsgStr,
                       CatLog::Severity::Info, CatLog::Category::Visualization);
}
void QtApplicationInterface::PostMessageToDialog(SmartMetViewId, unsigned int) { logOnce("PostMessageToDialog"); }
void QtApplicationInterface::StoreViewMacroWindowsSettings(NFmiViewSettingMacro&) { logOnce("StoreViewMacroWindowsSettings"); }
void QtApplicationInterface::LoadViewMacroWindowsSettings(NFmiViewSettingMacro&) { logOnce("LoadViewMacroWindowsSettings"); }
void QtApplicationInterface::DrawOverBitmapThings(NFmiToolBox*) { logOnce("DrawOverBitmapThings"); }
void QtApplicationInterface::ForceDrawOverBitmapThings(unsigned int, bool, bool) { logOnce("ForceDrawOverBitmapThings"); }
void QtApplicationInterface::UpdateTempView() { logOnce("UpdateTempView"); }
void QtApplicationInterface::UpdateCrossSectionView() { logOnce("UpdateCrossSectionView"); }
void QtApplicationInterface::ActivateParameterSelectionDlg(unsigned int) { logOnce("ActivateParameterSelectionDlg"); }
void QtApplicationInterface::ActivateZoomDialog(int) { logOnce("ActivateZoomDialog"); }
std::string QtApplicationInterface::GetToolTipString(unsigned int, const std::string&) { return ""; }
void QtApplicationInterface::SetMacroErrorText(const std::string& theErrorStr, std::shared_ptr<NFmiDrawParam>&)
{
    if(!theErrorStr.empty())
        CatLog::logMessage("Macro error: " + theErrorStr,
                           CatLog::Severity::Warning, CatLog::Category::Macro);
}
void QtApplicationInterface::CaseStudyLoadingActions(const NFmiMetTime&, const std::string&) { logOnce("CaseStudyLoadingActions"); }
void QtApplicationInterface::CaseStudyToNormalModeActions() { logOnce("CaseStudyToNormalModeActions"); }
void QtApplicationInterface::PutWarningFlagTimerOn() { logOnce("PutWarningFlagTimerOn"); }
void QtApplicationInterface::CreateBetaProductDialog(SmartMetDocumentInterface*) { logOnce("CreateBetaProductDialog"); }
void QtApplicationInterface::SetAllViewIconsDynamically() { logOnce("SetAllViewIconsDynamically"); }
void QtApplicationInterface::UpdateViewForOffScreenDraw(BetaProductViewIndex) { logOnce("UpdateViewForOffScreenDraw"); }
void QtApplicationInterface::UpdateMainFrameTitle() { logOnce("UpdateMainFrameTitle"); }
void QtApplicationInterface::ApplyUpdatedViewsFlag(SmartMetViewId) { logOnce("ApplyUpdatedViewsFlag"); }
void QtApplicationInterface::OpenVisualizationsettingsDialog() { logOnce("OpenVisualizationsettingsDialog"); }
void QtApplicationInterface::OpenLogViewer() { logOnce("OpenLogViewer"); }
void QtApplicationInterface::OpenTimeSerialView() { logOnce("OpenTimeSerialView"); }
void QtApplicationInterface::OpenCrossSectionView() { logOnce("OpenCrossSectionView"); }
void QtApplicationInterface::OpenSoundingView() { logOnce("OpenSoundingView"); }
void QtApplicationInterface::OpenMainMapView() { logOnce("OpenMainMapView"); }
void QtApplicationInterface::OpenSideMapView(unsigned int) { logOnce("OpenSideMapView"); }
void QtApplicationInterface::OpenSmarttoolsDialog() { logOnce("OpenSmarttoolsDialog"); }
void QtApplicationInterface::OpenViewMacroDialog() { logOnce("OpenViewMacroDialog"); }
void QtApplicationInterface::SetAllMapViewTooltipDelays(bool, int) { logOnce("SetAllMapViewTooltipDelays"); }
void QtApplicationInterface::OpenMacroParamInSmarttoolDialog(const std::string&) { logOnce("OpenMacroParamInSmarttoolDialog"); }
bool QtApplicationInterface::IsCrossSectionMacroParamOk(std::shared_ptr<NFmiDrawParam>&) { return false; }

NFmiApplicationWinRegistry& QtApplicationInterface::ApplicationWinRegistry()
{
    if(!itsApplicationWinRegistry)
        itsApplicationWinRegistry = std::make_unique<NFmiApplicationWinRegistry>();
    return *itsApplicationWinRegistry;
}

#endif // UNIX
