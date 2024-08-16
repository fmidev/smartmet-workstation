#pragma once

#include "SmartMetViewId.h"
#include <string>
#include <functional>

class CView;
class CWnd;
class NFmiViewSettingMacro;
class NFmiToolBox;
class NFmiMetTime;
class SmartMetDocumentInterface;
class CBitmap;
class CSmartMetView;
class CSmartMetDoc;
class CDocument;
class NFmiApplicationWinRegistry;
enum class BetaProductViewIndex;

// Interface that is meant to be used by NFmiEditMapGeneralDataDoc class by using functionalities from SmartMet application.
class ApplicationInterface
{
    static CSmartMetView *itsSmartMetView;
    static CView *itsSmartMetViewAsCView;
public:
    using GetApplicationInterfaceImplementationCallBackType = std::function<ApplicationInterface*(void)>;
    // Tämä pitää asettaa johonkin konkreettiseen funktioon, jotta käyttäjä koodi saa käyttöönsä halutun interface toteutuksen
    static GetApplicationInterfaceImplementationCallBackType GetApplicationInterfaceImplementation;

	virtual ~ApplicationInterface() = default;

    // SmartMetView is complicated, there is problems when it's created and initialized and when you are supposed to use it through this interface class.
    // Basicly set SmartMetView pointers in CSmartMetView::OnInitialUpdate method right after CView::OnInitialUpdate method call.
    static void SetSmartMetView(CSmartMetView *view);
    static void SetSmartMetViewAsCView(CView *view);
    static CSmartMetView* GetSmartMetView();
    static CView* GetSmartMetViewAsCView();

    virtual void ParameterSelectionSystemUpdateTimerStart(int waitTimeInSeconds) = 0;
    virtual void SetNotificationMessage(const std::string &theNotificationMsgStr, const std::string &theNotificationTitle, int theStyle, int theTimeout, bool fNoSound) = 0;
    virtual CSmartMetDoc* GetDocument() = 0; // returns CSmartMetDoc
    virtual CDocument* GetDocumentAsCDocument() = 0; // returns CSmartMetDoc but as CDocument (needed with classes that don't know about CSmartMetView)
    virtual void PostMessageToDialog(SmartMetViewId dialogId, unsigned int theMessage) = 0;
    virtual CWnd* GetView(int theWantedDescTopIndex) = 0;
    virtual void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty = false, bool fClearCache = false, int theWantedMapViewDescTop = -1) = 0;
    virtual void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, SmartMetViewId updatedViewsFlag, bool fMakeAreaViewDirty = false, bool fClearCache = false, int theWantedMapViewDescTop = -1) = 0;
    virtual void StoreViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro) = 0;
    virtual void LoadViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro) = 0;
    virtual void UpdateMapView(unsigned int theDescTopIndex) = 0;
    virtual void UpdateOnlyExtraMapViews(bool updateMap1, bool updateMap2) = 0;
    virtual void DrawOverBitmapThings(NFmiToolBox *theGTB) = 0;
    virtual void ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews) = 0;
    virtual void UpdateTempView() = 0;
    virtual void UpdateCrossSectionView() = 0;
    virtual void ActivateParameterSelectionDlg(unsigned int theDescTopIndex) = 0;
    virtual void ActivateZoomDialog(int theWantedDescTopIndex) = 0;
    virtual std::string GetToolTipString(unsigned int commandID, const std::string &theMagickWordU_) = 0;
    virtual void InvalidateMainMapView(bool bErase = true) = 0;
    virtual void SetMacroErrorText(const std::string &theErrorStr) = 0;
    virtual void CaseStudyLoadingActions(const NFmiMetTime &theUsedTime, const std::string &updateReasonText) = 0;
    virtual void CaseStudyToNormalModeActions() = 0;
    virtual void PutWarningFlagTimerOn() = 0;
    virtual void CreateBetaProductDialog(SmartMetDocumentInterface *smartMetDocumentInterface) = 0;
    virtual void SetAllViewIconsDynamically() = 0;
    virtual void DoOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex, CBitmap &theDrawedScreenBitmapOut) = 0;
    virtual void UpdateViewForOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex) = 0;
    virtual void UpdateMainFrameTitle() = 0;
    virtual void OpenLocationFinderDialog(CWnd *parentView) = 0;
    virtual NFmiApplicationWinRegistry& ApplicationWinRegistry() = 0;
    virtual void ApplyUpdatedViewsFlag(SmartMetViewId updatedViewsFlag) = 0;
    virtual std::pair<HICON, HICON> GetUsedIcons() = 0;
    virtual void SetHatchingToolmasterEpsilonFactor(float newEpsilonFactor);
    virtual void SetHatchingDebuggingPolygonIndex(int action);
    virtual void OpenVisualizationsettingsDialog() = 0;
    virtual void OpenLogViewer() = 0;
    virtual void OpenTimeSerialView() = 0;
    virtual void OpenCrossSectionView() = 0;
    virtual void OpenSoundingView() = 0;
    virtual void OpenMainMapView() = 0;
    virtual void OpenSideMapView(unsigned int theMapViewDescTopIndex) = 0;
    virtual void OpenSmarttoolsDialog() = 0;
    virtual void OpenViewMacroDialog() = 0;
    virtual void SetAllMapViewTooltipDelays(bool doRestoreAction, int delayInMilliSeconds) = 0;
    virtual void UpdateAllViewsAndDialogs(const std::string& reasonForUpdate, bool fUpdateOnlyMapViews = false) = 0;

private:
    void AddToHatchingToolmasterEpsilonFactor(float addedValue);
};
