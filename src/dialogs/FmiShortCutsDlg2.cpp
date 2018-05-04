// FmiShortCutsDlg2.cpp : implementation file
//

#include "stdafx.h"
#include "FmiShortCutsDlg2.h"
#include "NFmiDictionaryFunction.h"
#include "CloneBitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiShortCutsDlg2 dialog


CFmiShortCutsDlg2::CFmiShortCutsDlg2(CWnd* pParent /*=NULL*/)
	: CDialog(CFmiShortCutsDlg2::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFmiShortCutsDlg2)
    itsTextU_ = _T("");
	//}}AFX_DATA_INIT
}

void CFmiShortCutsDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiShortCutsDlg2)
    DDX_Text(pDX, IDC_EDIT_TEXT, itsTextU_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiShortCutsDlg2, CDialog)
	//{{AFX_MSG_MAP(CFmiShortCutsDlg2)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_WM_CLOSE()
ON_COMMAND(ID_SELECT_ALL_SHORT_CUTS, OnSelectAll)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiShortCutsDlg2 message handlers

BOOL CFmiShortCutsDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	InitDialogTexts();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiShortCutsDlg2::OnSelectAll()
{
	CEdit *edit = (CEdit *)GetDlgItem(IDC_EDIT_TEXT);
	if(edit)
		edit->SetSel(0, -1, TRUE);
}

// Tämä funktio on tehty siksi että edit-box jostaqin syystä valitsee 
// kaiken tekstin minkä laitan sinne. Eli tämä desectoi kaiken.
// Tätä pitää kutsua vain 1. kerran kun näytetään tämä dialogi.
void CFmiShortCutsDlg2::DeselectTexts(void)
{
	CEdit *edit = (CEdit *)GetDlgItem(IDC_EDIT_TEXT);
	if(edit)
		edit->SetSel(-1, -1, TRUE);
}

void CFmiShortCutsDlg2::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CWnd *win = GetDlgItem(IDC_EDIT_TEXT);
	if(win)
	{
		CRect rect;
		GetClientRect(rect);
		win->MoveWindow(rect, FALSE);
	}
}

void CFmiShortCutsDlg2::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose();
}

void CFmiShortCutsDlg2::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiShortCutsDlg2::DoWhenClosing(void)
{
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiShortCutsDlg2::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("ShortCutsDlgTitle").c_str()));

	itsTextU_ = _TEXT("");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMapViewStr").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgSelectAll").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgDeSelectAll").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleOverMapImage").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgOverMapImageOverData").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCopy").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgSpaceOut").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgShowPoints").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeMapImage").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgStoreGrid").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgHideParamBox").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgOpenViewMacroDlg").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleAreaTextLanguage").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMoveParamBox").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgPrint").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeBorderLineColor").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeBorderLineWidth").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeProjectionLineDraw").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgSave").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeTimeDraw").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgPaste").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleMask").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgUndo").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgRedo").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMoveTimeBack").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMoveTimeForward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMoveMapRowUp").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMoveMapRowDown").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgSelectMapRow").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgOpenShortCuts").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgObsComparisonSetMode").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgObsComparisonSetSymbol").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgObsComparisonSetSymbolSize").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgObsComparisonToggleSymbolBorderDraw").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgOpenSynopPlotDlg").c_str()) + CString(_TEXT("\r\n"));

    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleCrossSectionMode").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgRefreshAllViews").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleHelpCursors").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgHAKEWarnings").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleKeepMapRatio").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleTooltip").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMouseClickStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMouseClickStr2").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMouseClickStr3").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMouseClickStr4").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMouseClickStr5").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMouseClickStr6").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMouseClickStr7").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMouseClickStr8").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr2").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr3").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr4").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr5").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr6").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr7").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr8").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgParamBoxStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgParamBoxStr2").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgParamBoxStr3").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMaskBoxStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMaskBoxStr2").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMaskBoxStr3").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgZoomStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgZoomStr2").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgZoomStr3").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgZoomStr4").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgZoomStr5").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgZoomStr6").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgZoomStr7").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr2").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr3").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr4").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr5").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr6").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr7").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr8").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr9").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr10").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialStr11").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialTimeControlStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialTimeControlStr2").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialTimeControlStr3").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialTimeControlStr4").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialTimeControlStr5").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgTimeSerialTimeControlStr6").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr2").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr3").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr4").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr5").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr6").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr7").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr8").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr9").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr10").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr11").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr12").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr13").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr14").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgModifToolStr15").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCPToolStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCPToolStr2").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCPToolStr3").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCPToolStr4").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCPToolStr5").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCPToolStr6").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCPToolStr7").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr1").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr2").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr3").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr4").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr5").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr6").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr7").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr8").c_str()) + CString(_TEXT("\r\n"));

	itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr9").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr10").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr11").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgCrossSectionViewStr12").c_str()) + CString(_TEXT("\r\n"));
}
