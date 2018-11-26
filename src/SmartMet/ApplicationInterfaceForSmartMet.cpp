#include "ApplicationInterfaceForSmartMet.h"
#include "SmartMet.h"
#include "SmartMetDoc.h"
#include "SmartMetView.h"
#include "MainFrm.h"

namespace
{
    CMainFrame* GetMainFrame()
    {
        return (CMainFrame*)AfxGetMainWnd();
    }

    CSmartMetApp* GetApplication()
    {
        return (CSmartMetApp*)AfxGetApp();
    }
}

ApplicationInterfaceForSmartMet::ApplicationInterfaceForSmartMet()
{
}

void ApplicationInterfaceForSmartMet::ParamAddingSystemUpdateTimerStart(int waitTimeInSeconds)
{
    ::GetMainFrame()->ParamAddingSystemUpdateTimerStart(waitTimeInSeconds);
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

void ApplicationInterfaceForSmartMet::RestartHistoryDataCacheThread()
{
    ::GetMainFrame()->RestartHistoryDataCacheThread();
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

void ApplicationInterfaceForSmartMet::ActivateParamAddingDlg()
{
    auto doc = GetDocument();
    if(doc)
        doc->ActivateParamAddingDlg();
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

void ApplicationInterfaceForSmartMet::DoOffScreenDraw(unsigned int theMapViewDescTopIndex, CBitmap &theDrawedScreenBitmapOut)
{
    auto doc = GetDocument();
    if(doc)
        doc->DoOffScreenDraw(theMapViewDescTopIndex, theDrawedScreenBitmapOut);
}

void ApplicationInterfaceForSmartMet::UpdateViewForOffScreenDraw(unsigned int theMapViewDescTopIndex)
{
    auto doc = GetDocument();
    if(doc)
        doc->UpdateViewForOffScreenDraw(theMapViewDescTopIndex);
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
