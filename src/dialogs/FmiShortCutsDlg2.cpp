// FmiShortCutsDlg2.cpp : implementation file
//

#include "stdafx.h"
#include "FmiShortCutsDlg2.h"
#include "NFmiDictionaryFunction.h"
#include "CloneBitmap.h"
#include "FmiWin32Helpers.h"

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

    CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
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
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + C Copy active row parameters to clipboard").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgSpaceOut").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgShowPoints").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- SHIFT + E Change data point marker color").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + E Change data point marker size").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeMapImage").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgStoreGrid").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgHideParamBox").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgOpenViewMacroDlg").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + L Opens Log viewer").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + M Copy zoomed area information to clipboard").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- SHIFT + M Copy calculationpoint information to clipboard").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + M Copy data file paths information to clipboard").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgPrint").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeBorderLineColor").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeBorderLineWidth").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeProjectionLineDraw").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgSave").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgChangeTimeDraw").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + U Change map-view time mode (works only in map-view 2+3)").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + V Paste parameters from clipboard to active row").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleMask").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- X Opens Location finder").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgRedo").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- Z Toggle Animation mode").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + Z Undo editing data action").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + Z Redo editing data action").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgUndo").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMoveTimeBack").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + <- key Pan map to left").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + <- key Set active map-view row's all model data to previous model run").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMoveTimeForward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + -> key Pan map to right").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + -> key Set active map-view row's all model data to next model run").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMoveMapRowUp").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + Up key Pan map upward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- SHIFT + Up key Zoom map in").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + Up key Scroll 10 rows upward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgMoveMapRowDown").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + Down key Pan map downward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- SHIFT + Down key Zoom map out").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + Down key Scroll 10 rows downward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- Space Toggle between zoomed area - total area").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgSelectMapRow").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgOpenShortCuts").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgObsComparisonSetMode").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgObsComparisonSetSymbol").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgObsComparisonSetSymbolSize").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgObsComparisonToggleSymbolBorderDraw").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgOpenSynopPlotDlg").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + F3 Opens Ignore stations").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- Home Centers time control view to wallclock time").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- PageUp Move selected time 6 hours backward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + PageUp Move selected time 24 hours backward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- SHIFT + PageUp Move selected time 1 week backward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + PageUp Move selected time 1 month backward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- PageDown Move selected time 6 hours forward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + PageDown Move selected time 24 hours forward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- SHIFT + PageDown Move selected time 1 week forward").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + PageDown Move selected time 1 month forward").c_str()) + CString(_TEXT("\r\n"));

    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleCrossSectionMode").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgRefreshAllViews").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + SHIFT + F5 Reload all dynamic data and update all views").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleHelpCursors").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- F8 Opens Parameter selection").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + F8 Opens Case study").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- SHIFT + F8 Opens Beta production").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgHAKEWarnings").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleKeepMapRatio").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("ShortCutsDlgToggleTooltip").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- F12 Apply backup viewMacro").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + F12 Apply crash backup viewMacro").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- SHIFT + F12 Apply startup viewMacro").c_str()) + CString(_TEXT("\r\n"));

    itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + K Toggle Virtual-Time mode on all views (map, time-serial, sounding, cross-section)").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- In time-control-views left click on the purple bar to set Virtual-Time (all views when visible)").c_str()) + CString(_TEXT("\r\n"));

    itsTextU_ += _TEXT("\r\n");
    itsTextU_ += CA2T(::GetDictionaryString("- Y  Distance measure mode on/off").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- To measure distance on map: Start left mouse down - drag to destination - left mouse up").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- Just change start point: CTRL key down - left mouse down+up").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- CTRL + Y  Toggle Lock mode (locks start/end points and allows normal data selection/viewing)").c_str()) + CString(_TEXT("\r\n"));
    itsTextU_ += CA2T(::GetDictionaryString("- SHIFT + Y  Change colour (white -> black -> red -> green -> blue -> magenta -> yellow -> cyan)").c_str()) + CString(_TEXT("\r\n"));

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
