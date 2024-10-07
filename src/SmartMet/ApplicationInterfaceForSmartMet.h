#pragma once

#include "ApplicationInterface.h"

class CSmartMetView;
class CSmartMetDoc;

// Interface that is meant to be used by NFmiEditMapGeneralDataDoc class by using functionalities from SmartMet application.
class ApplicationInterfaceForSmartMet : public ApplicationInterface
{
public:
    ApplicationInterfaceForSmartMet();
	~ApplicationInterfaceForSmartMet() = default;

    void ParameterSelectionSystemUpdateTimerStart(int waitTimeInSeconds) override; // CMainFrame (CMainFrame*)AfxGetMainWnd()
    void SetNotificationMessage(const std::string &theNotificationMsgStr, const std::string &theNotificationTitle, int theStyle, int theTimeout, bool fNoSound) override; // CSmartMetView
    CSmartMetDoc* GetDocument() override; // returns CSmartMetDoc
    CDocument* GetDocumentAsCDocument() override; // returns CSmartMetDoc but as CDocument (needed with classes that don't know about CSmartMetView)
    void PostMessageToDialog(SmartMetViewId dialogId, unsigned int theMessage) override; // CSmartMetDoc
    CWnd* GetView(int theWantedDescTopIndex) override; // CSmartMetDoc
    void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty = false, bool fClearCache = false, int theWantedMapViewDescTop = -1) override;  // CSmartMetView
    void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, SmartMetViewId updatedViewsFlag, bool fMakeAreaViewDirty = false, bool fClearCache = false, int theWantedMapViewDescTop = -1) override;
    void StoreViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro) override;  // CSmartMetView
    void LoadViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro) override;  // CSmartMetView
    void UpdateMapView(unsigned int theDescTopIndex) override;  // CSmartMetView
    void UpdateOnlyExtraMapViews(bool updateMap1, bool updateMap2) override; // CSmartMetDoc
    void DrawOverBitmapThings(NFmiToolBox *theGTB) override;  // CSmartMetView
    void ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews) override;  // CSmartMetView
    void UpdateTempView() override; // CSmartMetDoc
    void UpdateCrossSectionView() override; // CSmartMetDoc
    void ActivateParameterSelectionDlg(unsigned int theDescTopIndex = 0) override; // CSmartMetDoc
    void ActivateZoomDialog(int theWantedDescTopIndex) override;  // CSmartMetView
    std::string GetToolTipString(unsigned int commandID, const std::string &theMagickWordU_) override; // CSmartMetApp
    //{
    //    CSmartMetApp* app = (CSmartMetApp*)AfxGetApp();
    //    CString strU_ = app->GetToolTipString(commandID, CString(CA2T(theMagickWord.c_str())));
    //    std::string tmp = CT2A(strU_);
    //    return tmp;
    //}
    void InvalidateMainMapView(bool bErase = true) override;  // CSmartMetView
    void SetMacroErrorText(const std::string &theErrorStr) override;  // CSmartMetView
    void CaseStudyLoadingActions(const NFmiMetTime &theUsedTime, const std::string &updateReasonText) override; // CSmartMetDoc
    void CaseStudyToNormalModeActions() override; // CSmartMetDoc
    void PutWarningFlagTimerOn() override;  // CSmartMetView
    void CreateBetaProductDialog(SmartMetDocumentInterface *smartMetDocumentInterface) override; // CSmartMetDoc
    void SetAllViewIconsDynamically() override; // CSmartMetDoc
    void DoOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex, CBitmap &theDrawedScreenBitmapOut) override; // CSmartMetDoc
    void UpdateViewForOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex) override; // CSmartMetDoc
    void UpdateMainFrameTitle() override; // CMainFrame (CMainFrame*)AfxGetMainWnd()
    void OpenLocationFinderDialog(CWnd *parentView) override;
    NFmiApplicationWinRegistry& ApplicationWinRegistry() override;
    void ApplyUpdatedViewsFlag(SmartMetViewId updatedViewsFlag) override;
    std::pair<HICON, HICON> GetUsedIcons() override;
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
    void UpdateAllViewsAndDialogs(const std::string& reasonForUpdate, bool fUpdateOnlyMapViews = false) override;
    void OpenMacroParamInSmarttoolDialog(const std::string& absoluteMacroParamPath) override;
};
