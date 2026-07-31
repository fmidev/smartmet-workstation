#pragma once
// ApplicationInterface implementation for the Linux/Qt build.
//
// NFmiEditMapGeneralDataDoc calls back into the hosting application through this
// interface. On Windows ApplicationInterfaceForSmartMet forwards to the MFC frame,
// dialogs and views. The Qt port has no dialogs yet, so most entry points are
// deliberately inert; the ones that ask for a redraw are forwarded to a repaint
// callback the window installs.
//
// Anything not implemented yet logs once through CatLog rather than silently doing
// nothing, so it is visible which parts of the document actually get exercised.

#ifdef UNIX

#include "ApplicationInterface.h"

#include <functional>
#include <memory>

class NFmiApplicationWinRegistry;

class QtApplicationInterface : public ApplicationInterface
{
public:
    QtApplicationInterface();
    ~QtApplicationInterface() override;

    // Installs this implementation as the one ApplicationInterface hands out.
    // Must be called before NFmiEditMapGeneralDataDoc::Init().
    static QtApplicationInterface& install();

    // Called whenever the document asks for the map views to be redrawn.
    void setRepaintCallback(std::function<void()> callback) { itsRepaintCallback = std::move(callback); }

    // ---- redraw requests, forwarded to the repaint callback ----
    void RefreshApplicationViewsAndDialogs(const std::string& reasonForUpdate, bool fMakeAreaViewDirty = false, bool fClearCache = false, int theWantedMapViewDescTop = -1) override;
    void RefreshApplicationViewsAndDialogs(const std::string& reasonForUpdate, SmartMetViewId updatedViewsFlag, bool fMakeAreaViewDirty = false, bool fClearCache = false, int theWantedMapViewDescTop = -1) override;
    void UpdateMapView(unsigned int theDescTopIndex) override;
    void UpdateAllViewsAndDialogs(const std::string& reasonForUpdate, bool fUpdateOnlyMapViews = false) override;
    void InvalidateMainMapView(bool bErase = true) override;
    void UpdateOnlyExtraMapViews(bool updateMap1, bool updateMap2) override;

    // ---- no-ops: no Qt equivalent exists yet ----
    void ParameterSelectionSystemUpdateTimerStart(int waitTimeInSeconds) override;
    void SetNotificationMessage(const std::string& theNotificationMsgStr, const std::string& theNotificationTitle, int theStyle, int theTimeout, bool fNoSound) override;
    void PostMessageToDialog(SmartMetViewId dialogId, unsigned int theMessage) override;
    void StoreViewMacroWindowsSettings(NFmiViewSettingMacro& theViewMacro) override;
    void LoadViewMacroWindowsSettings(NFmiViewSettingMacro& theViewMacro) override;
    void DrawOverBitmapThings(NFmiToolBox* theGTB) override;
    void ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews) override;
    void UpdateTempView() override;
    void UpdateCrossSectionView() override;
    void ActivateParameterSelectionDlg(unsigned int theDescTopIndex) override;
    void ActivateZoomDialog(int theWantedDescTopIndex) override;
    std::string GetToolTipString(unsigned int commandID, const std::string& theMagickWordU_) override;
    void SetMacroErrorText(const std::string& theErrorStr, std::shared_ptr<NFmiDrawParam>& triggerDrawParam) override;
    void CaseStudyLoadingActions(const NFmiMetTime& theUsedTime, const std::string& updateReasonText) override;
    void CaseStudyToNormalModeActions() override;
    void PutWarningFlagTimerOn() override;
    void CreateBetaProductDialog(SmartMetDocumentInterface* smartMetDocumentInterface) override;
    void SetAllViewIconsDynamically() override;
    void UpdateViewForOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex) override;
    void UpdateMainFrameTitle() override;
    NFmiApplicationWinRegistry& ApplicationWinRegistry() override;
    void ApplyUpdatedViewsFlag(SmartMetViewId updatedViewsFlag) override;
    void OpenVisualizationsettingsDialog() override;
    void OpenLogViewer() override;
    void OpenTimeSerialView() override;
    void OpenCrossSectionView() override;
    void OpenSoundingView() override;
    void OpenMainMapView() override;
    void OpenSideMapView(unsigned int theMapViewDescTopIndex) override;
    void OpenSmarttoolsDialog() override;
    void OpenViewMacroDialog() override;
    void SetAllMapViewTooltipDelays(bool doRestoreAction, int delayInMilliSeconds) override;
    void OpenMacroParamInSmarttoolDialog(const std::string& absoluteMacroParamPath) override;
    bool IsCrossSectionMacroParamOk(std::shared_ptr<NFmiDrawParam>& theUsedDrawParam) override;

private:
    void requestRepaint(const std::string& reason);

    std::function<void()> itsRepaintCallback;
    std::unique_ptr<NFmiApplicationWinRegistry> itsApplicationWinRegistry;
};

#endif // UNIX
