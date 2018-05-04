#include "stdafx.h"
#include "DynamicSplitterHelper.h"

#include "boost\math\special_functions.hpp"

CDynamicSplitterHelper::CDynamicSplitterHelper(CWnd *parentWnd, int horizontalCtrlId, int verticalCtrlId, double splitRatio, int useHorizontalSplitterCtrlId, int topOrLeftCtrlId, int bottomOrRightCtrlId, int totalSplitterAreaCtrlId)
    :itsParentWnd(parentWnd)
    , itsSplitterHorizontalCtrlId(horizontalCtrlId)
    , itsSplitterHorizontal()
    , itsSplitterVerticalCtrlId(verticalCtrlId)
    , itsSplitterVertical()
    , itsSplitRatio(splitRatio)
    , itsUseHorizontalSplitterCtrlId(useHorizontalSplitterCtrlId)
    , fUseHorizontalSplitter(TRUE)
    , itsTopOrLeftCtrlId(topOrLeftCtrlId)
    , itsBottomOrRightCtrlId(bottomOrRightCtrlId)
    , itsTotalSplitterAreaCtrlId(totalSplitterAreaCtrlId)
    , fParentWndInitialized(false)
{}

void CDynamicSplitterHelper::SetUpDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, itsSplitterHorizontalCtrlId, itsSplitterHorizontal);
    DDX_Control(pDX, itsSplitterVerticalCtrlId, itsSplitterVertical);
    DDX_Check(pDX, itsUseHorizontalSplitterCtrlId, fUseHorizontalSplitter);
}

void CDynamicSplitterHelper::ShowHideSplitters()
{
    CWnd *splitterHorizontal = itsParentWnd->GetDlgItem(itsSplitterHorizontalCtrlId);
    splitterHorizontal->ShowWindow(fUseHorizontalSplitter ? SW_SHOW : SW_HIDE);

    CWnd *splitterVertical = itsParentWnd->GetDlgItem(itsSplitterVerticalCtrlId);
    splitterVertical->ShowWindow(fUseHorizontalSplitter ? SW_HIDE : SW_SHOW);
}

// Splitter systeemille pitää antaa alkuarvot ennen kuin tehdään muita säätöjä.
void CDynamicSplitterHelper::PreResizerHookingSetup()
{
    // IDC_BUTTON_SPLITTER_SMARTTOOL kontrolli määrää splitterin alkuasennon
    // IDC_STATIC_SPLITTER_CONTROL kontrolli määrää koko hökötyksen originaali koon ja sijainnin 
    // eli näiden avulla lasketaan koot ja sijainnit IDC_RICHEDIT_MACRO_TEXT- ja IDC_RICHEDIT_VIEW_ONLY_MACRO_TEXT -kontrolleille.
    CRect dividerRect(GetSplitterDividerRect());
    CRect totalSplitterRect(GetTotalSplitterRect());

    // Lasketaan halutun ration mukainen splitter-nappulan paikka ja siirretään se siihen.
    // Splitter-nappulalla on minimi korkeus/leveys, lasketaan keskipisteen suhteen siirtymät.
    if(fUseHorizontalSplitter)
    {
        int wantedDividerCenterYpos = boost::math::iround(itsSplitRatio * totalSplitterRect.Height() + totalSplitterRect.top);
        int moveSplitterVertically = wantedDividerCenterYpos - dividerRect.CenterPoint().y;
        dividerRect.bottom += moveSplitterVertically;
        dividerRect.top += moveSplitterVertically;
    }
    else
    {
        int wantedDividerCenterXpos = boost::math::iround(itsSplitRatio * totalSplitterRect.Width() + totalSplitterRect.left);
        int moveSplitterHorizontally = wantedDividerCenterXpos - dividerRect.CenterPoint().x;
        dividerRect.left += moveSplitterHorizontally;
        dividerRect.right += moveSplitterHorizontally;
    }

    CWnd *splitterDivider = itsParentWnd->GetDlgItem(GetUsedSplitterControlId());
    splitterDivider->MoveWindow(dividerRect);

    CRect editMacroControlRect;
    if(fUseHorizontalSplitter)
        editMacroControlRect = CRect(totalSplitterRect.left, totalSplitterRect.top, totalSplitterRect.right, dividerRect.top);
    else
        editMacroControlRect = CRect(totalSplitterRect.left, totalSplitterRect.top, dividerRect.left, totalSplitterRect.bottom);
    CWnd *editMacroControl = itsParentWnd->GetDlgItem(itsTopOrLeftCtrlId);
    editMacroControl->MoveWindow(editMacroControlRect);

    CRect viewMacroControlRect;
    if(fUseHorizontalSplitter)
        viewMacroControlRect = CRect(totalSplitterRect.left, dividerRect.bottom, totalSplitterRect.right, totalSplitterRect.bottom);
    else
        viewMacroControlRect = CRect(dividerRect.right, totalSplitterRect.top, totalSplitterRect.right, totalSplitterRect.bottom);
    CWnd *viewMacroControl = itsParentWnd->GetDlgItem(itsBottomOrRightCtrlId);
    viewMacroControl->MoveWindow(viewMacroControlRect);

    itsSplitterHorizontal.SetMaxRect(totalSplitterRect);
    itsSplitterVertical.SetMaxRect(totalSplitterRect);
}

void CDynamicSplitterHelper::PostResizerHookingSetup()
{
    ShowHideSplitters();

    // Pitää täyttää molemmat splitterit (horisontaali ja vertikaali), mutta vain toinen niistä näytetään
    itsSplitterHorizontal.SetType(CControlSplitter::CS_HORZ);
    itsSplitterHorizontal.AddToTopOrLeftCtrls(itsTopOrLeftCtrlId);
    itsSplitterHorizontal.AddToBottomOrRightCtrls(itsBottomOrRightCtrlId);

    itsSplitterVertical.SetType(CControlSplitter::CS_VERT);
    itsSplitterVertical.AddToTopOrLeftCtrls(itsTopOrLeftCtrlId);
    itsSplitterVertical.AddToBottomOrRightCtrls(itsBottomOrRightCtrlId);

    fParentWndInitialized = true;
}

void CDynamicSplitterHelper::AdjustSplitterControl()
{
    if(IsParentWndFullyOperational())
        PreResizerHookingSetup();
}

int CDynamicSplitterHelper::GetUsedSplitterControlId()
{
    return fUseHorizontalSplitter ? itsSplitterHorizontalCtrlId : itsSplitterVerticalCtrlId;
}

CRect CDynamicSplitterHelper::GetSplitterDividerRect()
{
    CRect dividerRect;
    CWnd *splitterDivider = itsParentWnd->GetDlgItem(GetUsedSplitterControlId());
    splitterDivider->GetWindowRect(dividerRect);
    itsParentWnd->ScreenToClient(dividerRect);
    return dividerRect;
}

CRect CDynamicSplitterHelper::GetTotalSplitterRect()
{
    CRect totalSplitterRect;
    CWnd *totalSplitter = itsParentWnd->GetDlgItem(itsTotalSplitterAreaCtrlId);
    totalSplitter->GetWindowRect(totalSplitterRect);
    itsParentWnd->ScreenToClient(totalSplitterRect);
    return totalSplitterRect;
}

void CDynamicSplitterHelper::ReCalculateNewSplitFactor()
{
    CRect dividerRect(GetSplitterDividerRect());
    CRect totalSplitterRect(GetTotalSplitterRect());
    if(fUseHorizontalSplitter)
        itsSplitRatio = (dividerRect.CenterPoint().y - totalSplitterRect.top) / (totalSplitterRect.Height() * 1.);
    else
        itsSplitRatio = (dividerRect.CenterPoint().x - totalSplitterRect.left) / (totalSplitterRect.Width() * 1.);
}

void CDynamicSplitterHelper::OnSplitterOrientationCheckboxClicked()
{
    itsParentWnd->UpdateData(TRUE); // haetaan dialogista uusi orientaation arvo

    ShowHideSplitters();
    AdjustSplitterControl();
}

bool CDynamicSplitterHelper::IsParentWndFullyOperational()
{
    if(itsParentWnd->GetSafeHwnd() != NULL && fParentWndInitialized)
        return true;
    else
        return false;
}
