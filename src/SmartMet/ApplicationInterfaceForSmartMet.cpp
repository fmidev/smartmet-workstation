#include "stdafx.h"
#include "ApplicationInterfaceForSmartMet.h"
#include "SmartMet.h"
#include "SmartMetDoc.h"
#include "SmartMetView.h"
#include "MainFrm.h"
#include "CloneBitmap.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiBetaProductSystem.h"

namespace
{
    class IconHandler
    {
        HICON normalSmallIcon_ = nullptr;
        HICON normalBigIcon_ = nullptr;
        HICON betaProductSmallIcon_ = nullptr;
        HICON betaProductBigIcon_ = nullptr;
        HICON caseStudySmallIcon_ = nullptr;
        HICON caseStudyBigIcon_ = nullptr;
    public:
        IconHandler()
        {
            normalSmallIcon_ = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
            normalBigIcon_ = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), 0);
            betaProductSmallIcon_ = CCloneBitmap::BitmapToIcon(IDB_BITMAP_FMI_LOGO_BETA_PRODUCT_MODE, ColorPOD(160, 160, 164));
            betaProductBigIcon_ = CCloneBitmap::BitmapToIcon(IDB_BITMAP_FMI_LOGO_BETA_PRODUCT_MODE, ColorPOD(160, 160, 164));
            caseStudySmallIcon_ = CCloneBitmap::BitmapToIcon(IDB_BITMAP_FMI_LOGO_CASE_STUDY, ColorPOD(160, 160, 164));
            caseStudyBigIcon_ = CCloneBitmap::BitmapToIcon(IDB_BITMAP_FMI_LOGO_CASE_STUDY, ColorPOD(160, 160, 164));
        }

        ~IconHandler()
        {
            DestroyIcon(normalSmallIcon_);
            DestroyIcon(normalBigIcon_);
            DestroyIcon(betaProductSmallIcon_);
            DestroyIcon(betaProductBigIcon_);
            DestroyIcon(caseStudySmallIcon_);
            DestroyIcon(caseStudyBigIcon_);
        }

        std::pair<HICON, HICON> getUsedIcons(bool betaProductAutomationOn, bool caseStudyModeOn)
        {
            if(betaProductAutomationOn)
            {
                return std::make_pair(betaProductSmallIcon_, betaProductBigIcon_);
            }
            else if(caseStudyModeOn)
            {
                return std::make_pair(caseStudySmallIcon_, caseStudyBigIcon_);
            }
            else
            {
                return std::make_pair(normalSmallIcon_, normalBigIcon_);
            }
        }
    };


    CMainFrame* GetMainFrame()
    {
        return (CMainFrame*)AfxGetMainWnd();
    }

    CSmartMetApp* GetApplication()
    {
        return (CSmartMetApp*)AfxGetApp();
    }

    std::pair<HICON, HICON> GetUsedIcons(NFmiEditMapGeneralDataDoc& generalDataDocument)
    {
        static IconHandler iconHandler;

        return iconHandler.getUsedIcons(generalDataDocument.BetaProductionSystem().AutomationModeOn(), generalDataDocument.CaseStudyModeOn());
    }
}

ApplicationInterfaceForSmartMet::ApplicationInterfaceForSmartMet()
{
}

void ApplicationInterfaceForSmartMet::ParameterSelectionSystemUpdateTimerStart(int waitTimeInSeconds)
{
    ::GetMainFrame()->ParameterSelectionSystemUpdateTimerStart(waitTimeInSeconds);
}

void ApplicationInterfaceForSmartMet::SetNotificationMessage(const std::string &theNotificationMsgStr, const std::string &theNotificationTitle, int theStyle, int theTimeout, bool fNoSound)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->SetNotificationMessage(theNotificationMsgStr, theNotificationTitle, theStyle, theTimeout, fNoSound);
}

CSmartMetDoc* ApplicationInterfaceForSmartMet::GetDocument()
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        return view->GetDocument();
    else
        return nullptr;
}

CDocument* ApplicationInterfaceForSmartMet::GetDocumentAsCDocument()
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        return view->GetDocument();
    else
        return nullptr;
}

void ApplicationInterfaceForSmartMet::PostMessageToDialog(SmartMetViewId dialogId, unsigned int theMessage)
{
    auto doc = GetDocument();
    if(doc)
        doc->PostMessageToDialog(dialogId, theMessage);
}

CWnd* ApplicationInterfaceForSmartMet::GetView(int theWantedDescTopIndex)
{
    auto doc = GetDocument();
    if(doc)
        return doc->GetView(theWantedDescTopIndex);
    else
        return nullptr;
}

void ApplicationInterfaceForSmartMet::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty, bool fClearCache, int theWantedMapViewDescTop)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->RefreshApplicationViewsAndDialogs(reasonForUpdate, fMakeAreaViewDirty, fClearCache, theWantedMapViewDescTop);
}

void ApplicationInterfaceForSmartMet::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, SmartMetViewId updatedViewsFlag, bool fMakeAreaViewDirty, bool fClearCache, int theWantedMapViewDescTop)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->RefreshApplicationViewsAndDialogs(reasonForUpdate, updatedViewsFlag, fMakeAreaViewDirty, fClearCache, theWantedMapViewDescTop);
}

void ApplicationInterfaceForSmartMet::StoreViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->StoreViewMacroWindowsSettings(theViewMacro);
}

void ApplicationInterfaceForSmartMet::LoadViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->LoadViewMacroWindowsSettings(theViewMacro);
}

void ApplicationInterfaceForSmartMet::UpdateMapView(unsigned int theDescTopIndex)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->UpdateMapView(theDescTopIndex);
}

void ApplicationInterfaceForSmartMet::UpdateOnlyExtraMapViews(bool updateMap1, bool updateMap2)
{
    auto doc = GetDocument();
    if(doc)
        doc->UpdateOnlyExtraMapViews(updateMap1, updateMap2);
}

void ApplicationInterfaceForSmartMet::DrawOverBitmapThings(NFmiToolBox *theGTB)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->DrawOverBitmapThings(theGTB);
}

void ApplicationInterfaceForSmartMet::ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->ForceDrawOverBitmapThings(originalCallerDescTopIndex, doOriginalView, doAllOtherMapViews);
}

void ApplicationInterfaceForSmartMet::UpdateTempView()
{
    auto doc = GetDocument();
    if(doc)
        doc->UpdateTempView();
}

void ApplicationInterfaceForSmartMet::UpdateCrossSectionView()
{
    auto doc = GetDocument();
    if(doc)
        doc->UpdateCrossSectionView();
}

void ApplicationInterfaceForSmartMet::ActivateParameterSelectionDlg(unsigned int theDescTopIndex)
{
    auto doc = GetDocument();
    if(doc)
        doc->ActivateParameterSelectionDlg(theDescTopIndex);
}

void ApplicationInterfaceForSmartMet::ActivateZoomDialog(int theWantedDescTopIndex)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->ActivateZoomDialog(theWantedDescTopIndex);
}

std::string ApplicationInterfaceForSmartMet::GetToolTipString(unsigned int commandID, const std::string &theMagickWord)
{
    CString strU_ = ::GetApplication()->GetToolTipString(commandID, CString(CA2T(theMagickWord.c_str())));
    std::string tmp = CT2A(strU_);
    return tmp;
}

void ApplicationInterfaceForSmartMet::InvalidateMainMapView(bool bErase)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->Invalidate(bErase);
}

void ApplicationInterfaceForSmartMet::SetMacroErrorText(const std::string &theErrorStr)
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->SetMacroErrorText(theErrorStr);
}

void ApplicationInterfaceForSmartMet::CaseStudyLoadingActions(const NFmiMetTime &theUsedTime, const std::string &updateReasonText)
{
    auto doc = GetDocument();
    if(doc)
        doc->CaseStudyLoadingActions(theUsedTime, updateReasonText);
}

void ApplicationInterfaceForSmartMet::CaseStudyToNormalModeActions()
{
    auto doc = GetDocument();
    if(doc)
        doc->CaseStudyToNormalModeActions();
}

void ApplicationInterfaceForSmartMet::PutWarningFlagTimerOn()
{
    auto view = ApplicationInterface::GetSmartMetView();
    if(view)
        view->PutWarningFlagTimerOn();
}

void ApplicationInterfaceForSmartMet::CreateBetaProductDialog(SmartMetDocumentInterface *smartMetDocumentInterface)
{
    auto doc = GetDocument();
    if(doc)
        doc->CreateBetaProductDialog(smartMetDocumentInterface);
}

void ApplicationInterfaceForSmartMet::SetAllViewIconsDynamically()
{
    auto doc = GetDocument();
    if(doc)
        doc->SetAllViewIconsDynamically();
}

void ApplicationInterfaceForSmartMet::DoOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex, CBitmap &theDrawedScreenBitmapOut)
{
    auto doc = GetDocument();
    if(doc)
        doc->DoOffScreenDraw(selectedViewRadioButtonIndex, theDrawedScreenBitmapOut);
}

void ApplicationInterfaceForSmartMet::UpdateViewForOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex)
{
    auto doc = GetDocument();
    if(doc)
        doc->UpdateViewForOffScreenDraw(selectedViewRadioButtonIndex);
}

void ApplicationInterfaceForSmartMet::UpdateMainFrameTitle()
{
    ((CMainFrame*)AfxGetMainWnd())->OnUpdateFrameTitle(TRUE);
}

void ApplicationInterfaceForSmartMet::OpenLocationFinderDialog(CWnd *parentView)
{
    auto doc = GetDocument();
    if(doc)
        doc->OpenLocationFinderTool(parentView);
}

NFmiApplicationWinRegistry& ApplicationInterfaceForSmartMet::ApplicationWinRegistry()
{
    return GetDocument()->ApplicationWinRegistry();
}

void ApplicationInterfaceForSmartMet::ApplyUpdatedViewsFlag(SmartMetViewId updatedViewsFlag)
{
    GetDocument()->ApplyUpdatedViewsFlag(updatedViewsFlag);
}

std::pair<HICON, HICON> ApplicationInterfaceForSmartMet::GetUsedIcons()
{
    auto *generalDataDocument = GetApplication()->GeneralDocData();
    if(generalDataDocument)
        return ::GetUsedIcons(*generalDataDocument);
    else
        return std::pair<HICON, HICON>();
}

void ApplicationInterfaceForSmartMet::SetToDoFirstTimeWmsDataBasedUpdate()
{
    CMainFrame::SetToDoFirstTimeWmsDataBasedUpdate();
}

void ApplicationInterfaceForSmartMet::OpenVisualizationsettingsDialog()
{
    GetDocument()->OnEditVisualizationsettings();
}

void ApplicationInterfaceForSmartMet::OpenLogViewer()
{
    GetDocument()->OpenLogViewer();
}

void ApplicationInterfaceForSmartMet::OpenTimeSerialView()
{
    GetDocument()->OnButtonTimeEditValuesDlg();
}

void ApplicationInterfaceForSmartMet::OpenCrossSectionView()
{
    GetDocument()->OnButtonShowCrossSection();
}

void ApplicationInterfaceForSmartMet::OpenSoundingView()
{
    GetDocument()->OnButtonTempDlg();
}

void ApplicationInterfaceForSmartMet::OpenMainMapView()
{
    auto *mainWindow = AfxGetMainWnd();
    if(mainWindow)
    {
        mainWindow->ShowWindow(SW_RESTORE);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
        mainWindow->SetActiveWindow();
    }
}

void ApplicationInterfaceForSmartMet::OpenSideMapView(unsigned int theMapViewDescTopIndex)
{
    if(theMapViewDescTopIndex == 1)
        GetDocument()->OnExtraMapView1();
    else
        GetDocument()->OnExtraMapView2();
}

void ApplicationInterfaceForSmartMet::OpenSmarttoolsDialog()
{
    GetDocument()->OnButtonOpenSmartToolDlg();
}

void ApplicationInterfaceForSmartMet::OpenViewMacroDialog()
{
    GetDocument()->OnAcceleratorStoreViewMacro();
}

void ApplicationInterfaceForSmartMet::SetAllMapViewTooltipDelays(bool doRestoreAction, int delayInMilliSeconds)
{
    GetDocument()->SetAllMapViewTooltipDelays(doRestoreAction, delayInMilliSeconds);
}
