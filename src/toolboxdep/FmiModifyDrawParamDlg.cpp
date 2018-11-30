// FmiModifyDrawParamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiModifyDrawParamDlg.h"
#include "NFmiDrawParamList.h"
#include "NFmiValueString.h"
#include "FmiTMColorIndexDlg.h"
#include "NFmiSettings.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiFileString.h"
#include "SmartMetDocumentInterface.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "NFmiFixedDrawParamSystem.h"
#include "CtrlViewWin32Functions.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewFunctions.h"
#include "ToolMasterColorCube.h"
#include "PERSIST2.H"
#include "CtrlViewGdiPlusFunctions.h"

#include <direct.h> // working directory juttuja varten

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
/////////////////////////////////////////////////////////////////////////////
// CFmiModifyDrawParamDlg dialog

CFmiModifyDrawParamDlg::CFmiModifyDrawParamDlg(SmartMetDocumentInterface *smartMetDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const std::string &theDrawParamPath, bool modifyMapViewParam, bool modifyCrossSectionViewParam, unsigned int theDescTopIndex, CWnd* pParent)
:CDialog(CFmiModifyDrawParamDlg::IDD, pParent)
,itsDrawParam(new NFmiDrawParam())
,itsBackupDrawParam(new NFmiDrawParam())
,itsOrigDrawParam(theDrawParam)
,fModifyMapViewParam(modifyMapViewParam)
,fModifyCrossSectionViewParam(modifyCrossSectionViewParam)
,itsDrawParamPath(theDrawParamPath)
,itsSymbolFillBitmap(0)
,itsSymbolBitmap(0)
,itsIsoLineLabelBitmap(0)
,itsIsoLineBitmap(0)
,itsIsoLineLabelBoxFillBitmap(0)
,itsSymbolMidBitmap(0)
,itsSymbolLowBitmap(0)
,itsSymbolHighBitmap(0)
,itsSimpleIsoLineMidBitmap(0)
,itsSimpleIsoLineLowBitmap(0)
,itsSimpleIsoLineHighBitmap(0)
,itsSimpleColorContourMidBitmap(0)
,itsSimpleColorContourLowBitmap(0)
,itsSimpleColorContourHighBitmap(0)
,itsSimpleColorContourHigh2Bitmap(0)
,itsHatch1Bitmap(0)
,itsHatch2Bitmap(0)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsDescTopIndex(theDescTopIndex)
, itsDrawParamFileNameStrU_(_T(""))
, fUseIsoLineGabWithCustomContours(FALSE)
, itsContourGap(0)
,fRefreshPressed(false)
,fSkipreadingSpecialClassColorIndices(false)
, itsAlpha(100)
,fSpecialClassesHaveInvalidValues(false)
,fFixedDrawParamSelectorInitialized(false)
, itsOneSymbolHorizontalOffset_NEW(0)
, itsOneSymbolVerticalOffset_NEW(0)
, itsIsoLineZeroValue_NEW(0)
, itsSimpleClassMiddleValue_NEW(0)
, fUseTransparentLabelBoxFillColor(TRUE)
, fDoSparseDataSymbolVisualization(FALSE)
{
	if(theDrawParam)
	{
		itsOrigDrawParam = theDrawParam;
		itsDrawParam->Init(itsOrigDrawParam);
		itsBackupDrawParam->Init(itsOrigDrawParam);
	}
	else
		throw std::runtime_error("Error using CFmiModifyDrawParamDlg, drawParam pointer was NULL, error in application...");

	//{{AFX_DATA_INIT(CFmiModifyDrawParamDlg)
	fUSeChangingColorsWithSymbols = FALSE;
	fUSeColorScaleWithSimpleIsoLines = FALSE;
	fUseIsoLineFeathering = FALSE;
	fUseHatch1 = FALSE;
	fDrawLabelBox = FALSE;
	fDrawOnlyOverMask = FALSE;
	fUseSpecialClasses = FALSE;
	itsHatch1Style = 0;
	itsHatch1EndValue = 0.0;
	itsHatch1StartValue = 0.0;
	itsIsoLineGap = 0.0;
	itsContourGap = 0.0;
	itsIsoLineDecimals = 0;
	itsIsoLineLabelHeight = 0.0;
	itsIsoLineSmoothFactor = 0.0f;
	itsIsoLineStyle = 0;
	itsIsoLineWidth = 0.0f;
	itsModifyingStep = 0.0;
	itsOneSymbolHeight = 0.0;
	itsOneSymbolWidth = 0.0;
	itsParamAbsolutValueMax = 0.0;
	itsParamAbsolutValueMin = 0.0;
	itsParamAbbreviationStrU_ = _T("");
    itsParamNameStrU_ = _T("");
    itsProducerStrU_ = _T("");
	itsGridDataDrawStyle = -1;
	itsSimpleClassCount = 0;
	itsSimpleClassEndValue = 0.0f;
	itsSimpleClassStartValue = 0.0f;
	itsSymbolsWithColorsClassCount = 0;
	itsSymbolsWithColorsEndValue = 0.0f;
	itsSymbolsWithColorsMiddleValue = 0.0f;
	itsSymbolsWithColorsStartValue = 0.0f;
	itsSpecialClassCount = 0;
    itsSpecialClassColorIndicesStrU_ = _T("");
    itsSpecialClassLabelColorIndicesStrU_ = _T("");
    itsSpecialClassLineStylesStrU_ = _T("");
    itsSpecialClassLineWidthStrU_ = _T("");
    itsSpecialClassValuesStrU_ = _T("");
	itsTimeSeriesModifyLimit = 0.0;
	itsTimeSeriesScaleMax = 0.0;
	itsTimeSeriesScaleMin = 0.0;
    itsParamUnitStrU_ = _T("");
	fIsHidden = FALSE;
	fUseHatch2 = FALSE;
	itsHatch2Style = 0.0;
	itsHatch2EndValue = 0.0;
	itsHatch2StartValue = 0.0;
	itsSimpleColorContourClassEnd2Value = 0.0f;
	itsSimpleColorContourClassEndValue = 0.0f;
	itsSimpleColorContourClassMiddleValue = 0.0f;
	itsSimpleColorContourClassStartValue = 0.0f;
	fUSeSeparatingLinesBetweenColorContourClasses = FALSE;
	//}}AFX_DATA_INIT
}

CFmiModifyDrawParamDlg::~CFmiModifyDrawParamDlg(void)
{
	CtrlView::DestroyBitmap(&itsSymbolFillBitmap);
    CtrlView::DestroyBitmap(&itsSymbolBitmap);
    CtrlView::DestroyBitmap(&itsIsoLineLabelBitmap);
    CtrlView::DestroyBitmap(&itsIsoLineBitmap);
    CtrlView::DestroyBitmap(&itsIsoLineLabelBoxFillBitmap);
    CtrlView::DestroyBitmap(&itsSymbolMidBitmap);
    CtrlView::DestroyBitmap(&itsSymbolLowBitmap);
    CtrlView::DestroyBitmap(&itsSymbolHighBitmap);
    CtrlView::DestroyBitmap(&itsSimpleIsoLineMidBitmap);
    CtrlView::DestroyBitmap(&itsSimpleIsoLineLowBitmap);
    CtrlView::DestroyBitmap(&itsSimpleIsoLineHighBitmap);
    CtrlView::DestroyBitmap(&itsHatch1Bitmap);
    CtrlView::DestroyBitmap(&itsHatch2Bitmap);
    CtrlView::DestroyBitmap(&itsSimpleColorContourMidBitmap);
    CtrlView::DestroyBitmap(&itsSimpleColorContourLowBitmap);
    CtrlView::DestroyBitmap(&itsSimpleColorContourHighBitmap);
    CtrlView::DestroyBitmap(&itsSimpleColorContourHigh2Bitmap);
}


void CFmiModifyDrawParamDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFmiModifyDrawParamDlg)
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_MID, itsSimpleColorContourMidColor);
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_LOW, itsSimpleColorContourLowColor);
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_HIGH, itsSimpleColorContourHighColor);
    DDX_Control(pDX, IDC_BUTTON__HATCH2_COLOR, itsHatch2Color);
    DDX_Control(pDX, IDC_BUTTON_SYMBOL_FILL_COLOR, itsSymbolFillColor);
    DDX_Control(pDX, IDC_BUTTON_SYMBOL_COLOR, itsSymbolColor);
    DDX_Control(pDX, IDC_BUTTON_ISOLINE_LABEL_COLOR, itsIsoLineLabelColor);
    DDX_Control(pDX, IDC_BUTTON_ISOLINE_COLOR, itsIsoLineColor);
    DDX_Control(pDX, IDC_BUTTON_ISOLINE_LABEL_BOX_FILL_COLOR, itsIsolineLabelBoxFillColor);
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SYMB_MID, itsSymbolMidColor);
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SYMB_LOW, itsSymbolLowColor);
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SYMB_HIGH, itsSymbolHighColor);
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_MID, itsSimpleIsoLineMidColor);
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_LOW, itsSimpleIsoLineLowColor);
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_HIGH, itsSimpleIsoLineHighColor);
    DDX_Control(pDX, IDC_BUTTON__HATCH1_COLOR, itsHatch1Color);
    DDX_Check(pDX, IDC_CHECK_SHOW_SYMBOL_CHANGING_COLORS, fUSeChangingColorsWithSymbols);
    DDX_Check(pDX, IDC_CHECK_USE_COLOR_SCALE_WITH_SIMPLE_ISOLINES, fUSeColorScaleWithSimpleIsoLines);
    DDX_Check(pDX, IDC_CHECK_USE_FEATHERING, fUseIsoLineFeathering);
    DDX_Check(pDX, IDC_CHECK_USE_HACTH1, fUseHatch1);
    DDX_Check(pDX, IDC_CHECK_USE_LABEL_BOX, fDrawLabelBox);
    DDX_Check(pDX, IDC_CHECK_USE_REGIONS, fDrawOnlyOverMask);
    DDX_Check(pDX, IDC_CHECK_USE_SPECIAL_CLASSES, fUseSpecialClasses);
    DDX_Text(pDX, IDC_HATCH1_HATCH_STYLE, itsHatch1Style);
    DDX_Text(pDX, IDC_HATCH1_VALUE_END, itsHatch1EndValue);
    DDX_Text(pDX, IDC_HATCH1_VALUE_START, itsHatch1StartValue);
    DDX_Text(pDX, IDC_ISOLINE_GAP, itsIsoLineGap);
    DDX_Text(pDX, IDC_ISOLINE_LABEL_DECIMALS, itsIsoLineDecimals);
    DDX_Text(pDX, IDC_ISOLINE_LABEL_HEIGHT, itsIsoLineLabelHeight);
    DDX_Text(pDX, IDC_ISOLINE_SMOOTH_FACTOR, itsIsoLineSmoothFactor);
    DDX_Text(pDX, IDC_ISOLINE_STYLE, itsIsoLineStyle);
    DDX_Text(pDX, IDC_ISOLINE_WIDTH, itsIsoLineWidth);
    DDV_MinMaxFloat(pDX, itsIsoLineWidth, 0.f, 10.f);
    DDX_Text(pDX, IDC_MODIFYING_STEP, itsModifyingStep);
    DDX_Text(pDX, IDC_ONE_RELATIVE_HEIGHT, itsOneSymbolHeight);
    DDX_Text(pDX, IDC_ONE_RELATIVE_WIDTH, itsOneSymbolWidth);
    DDX_Text(pDX, IDC_PARAMETER_ABSOLUTE_VALUE_MAX, itsParamAbsolutValueMax);
    DDX_Text(pDX, IDC_PARAMETER_ABSOLUTE_VALUE_MIN, itsParamAbsolutValueMin);
    DDX_Text(pDX, IDC_PARAM_ABBREVIATION, itsParamAbbreviationStrU_);
    DDX_Text(pDX, IDC_PARAM, itsParamNameStrU_);
    DDX_Text(pDX, IDC_PRODUCER, itsProducerStrU_);
    DDX_Radio(pDX, IDC_RADIO_GRID_DATA_DRAW_STYLE, itsGridDataDrawStyle);
    DDX_Text(pDX, IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_CLASS_COUNT, itsSimpleClassCount);
    DDX_Text(pDX, IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_END_VALUE, itsSimpleClassEndValue);
    DDX_Text(pDX, IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_START_VALUE, itsSimpleClassStartValue);
    DDX_Text(pDX, IDC_SHOW_SYMBOL_WITH_COLORS_CLASS_COUNT, itsSymbolsWithColorsClassCount);
    DDX_Text(pDX, IDC_SHOW_SYMBOL_WITH_COLORS_END_VALUE, itsSymbolsWithColorsEndValue);
    DDX_Text(pDX, IDC_SHOW_SYMBOL_WITH_COLORS_MIDDLE_VALUE, itsSymbolsWithColorsMiddleValue);
    DDX_Text(pDX, IDC_SHOW_SYMBOL_WITH_COLORS_START_VALUE, itsSymbolsWithColorsStartValue);
    DDX_Text(pDX, IDC_SPECIAL_CLASSES_COUNT, itsSpecialClassCount);
    DDX_Text(pDX, IDC_SPECIAL_CLASSES_LINE_COLOR_INDICES, itsSpecialClassColorIndicesStrU_);
    DDX_Text(pDX, IDC_SPECIAL_CLASSES_LINE_LABEL_COLOR_INDICES, itsSpecialClassLabelColorIndicesStrU_);
    DDX_Text(pDX, IDC_SPECIAL_CLASSES_LINE_STYLES, itsSpecialClassLineStylesStrU_);
    DDX_Text(pDX, IDC_SPECIAL_CLASSES_LINE_WIDTH, itsSpecialClassLineWidthStrU_);
    DDX_Text(pDX, IDC_SPECIAL_CLASSES_VALUES, itsSpecialClassValuesStrU_);
    DDX_Text(pDX, IDC_TIME_SERIES_MOD_LIMIT, itsTimeSeriesModifyLimit);
    DDX_Text(pDX, IDC_TIME_SERIES_SCALE_MAX, itsTimeSeriesScaleMax);
    DDX_Text(pDX, IDC_TIME_SERIES_SCALE_MIN, itsTimeSeriesScaleMin);
    DDX_Text(pDX, IDC_UNIT, itsParamUnitStrU_);
    DDX_Check(pDX, IDC_F_HIDDEN, fIsHidden);
    DDX_Check(pDX, IDC_CHECK_USE_HACTH2, fUseHatch2);
    DDX_Text(pDX, IDC_HATCH2_HATCH_STYLE, itsHatch2Style);
    DDX_Text(pDX, IDC_HATCH2_VALUE_END, itsHatch2EndValue);
    DDX_Text(pDX, IDC_HATCH2_VALUE_START, itsHatch2StartValue);
    DDX_Text(pDX, IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_CLASS_COUNT, itsSimpleColorContourClassEnd2Value);
    DDX_Text(pDX, IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_END_VALUE, itsSimpleColorContourClassEndValue);
    DDX_Text(pDX, IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_MIDDLE_VALUE, itsSimpleColorContourClassMiddleValue);
    DDX_Text(pDX, IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_START_VALUE, itsSimpleColorContourClassStartValue);
    DDX_Check(pDX, IDC_CHECK_USE_SEPARATOR_LINES_BETWEEN_COLORCONTOUR_CLASSES, fUSeSeparatingLinesBetweenColorContourClasses);
    //}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_STATIC_DRAW_PARAM_FILE_NAME, itsDrawParamFileNameStrU_);
    DDX_Control(pDX, IDC_COMBO_DRAW_PARAM_STATION_DATA_VIEW_SELECTOR, itsStationDataViewSelector);
    DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_HIGH2, itsSimpleColorContourHigh2Color);
    DDX_Check(pDX, IDC_CHECK_DRAW_PARAM_USE_STEPS_WITH_CUSTOM_CONTOURS, fUseIsoLineGabWithCustomContours);
    DDX_Text(pDX, IDC_CONTOUR_GAP, itsContourGap);
    DDX_Text(pDX, IDC_CONTOUR_ALPHA, itsAlpha);
    DDV_MinMaxFloat(pDX, itsAlpha, NFmiDrawParam::itsMinAlpha, 100);
    DDX_Control(pDX, IDC_COMBO_FIXED_DRAW_PARAM_SELECTOR, itsFixedDrawParamSelector);
    DDX_Text(pDX, IDC_ONE_HORIZONTAL_RELATIVE_POS_OFFSET_NEW, itsOneSymbolHorizontalOffset_NEW);
    DDX_Text(pDX, IDC_ONE_VERTICAL_RELATIVE_POS_OFFSET_NEW, itsOneSymbolVerticalOffset_NEW);
    DDX_Text(pDX, IDC_ISOLINE_ZERO_VALUE_POINT_NEW, itsIsoLineZeroValue_NEW);
    DDX_Text(pDX, IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_MIDDLE_VALUE_NEW, itsSimpleClassMiddleValue_NEW);
    DDX_Check(pDX, IDC_CHECK_DRAW_PARAM_USE_TRANSPARENT_LABEL_FILL_COLOR, fUseTransparentLabelBoxFillColor);
    DDX_Check(pDX, IDC_CHECK_DO_SPARSE_SYMBOL_VISUALIZATION, fDoSparseDataSymbolVisualization);
}


BEGIN_MESSAGE_MAP(CFmiModifyDrawParamDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiModifyDrawParamDlg)
	ON_BN_CLICKED(IDC_BUTTON__HATCH1_COLOR, OnButtonHatch1Color)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_HIGH, OnButtonColorShowSimpleIsolineHigh)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_LOW, OnButtonColorShowSimpleIsolineLow)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_MID, OnButtonColorShowSimpleIsolineMid)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SYMB_HIGH, OnButtonColorShowSymbHigh)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SYMB_LOW, OnButtonColorShowSymbLow)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SYMB_MID, OnButtonColorShowSymbMid)
	ON_BN_CLICKED(IDC_BUTTON_ISOLINE_COLOR, OnButtonIsolineColor)
	ON_BN_CLICKED(IDC_BUTTON_ISOLINE_LABEL_BOX_FILL_COLOR, OnBnClickedButtonIsolineLabelBoxFillColor)
	ON_BN_CLICKED(IDC_BUTTON_ISOLINE_LABEL_COLOR, OnButtonIsolineLabelColor)
	ON_BN_CLICKED(IDC_BUTTON_SYMBOL_COLOR, OnButtonSymbolColor)
	ON_BN_CLICKED(IDC_BUTTON_SYMBOL_FILL_COLOR, OnButtonSymbolFillColor)
	ON_BN_CLICKED(IDC_SAVE_AS_BUTTON, OnSaveAsButton)
	ON_BN_CLICKED(IDC_SAVE_BUTTON, OnSaveButton)
	ON_BN_CLICKED(IDC_SHOW_COLOR_INDEX_DLG, OnShowColorIndexDlg)
	ON_BN_CLICKED(IDC_BUTTON__HATCH2_COLOR, OnButtonHatch2Color)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_HIGH, OnButtonColorShowSimpleColorcontourHigh)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_LOW, OnButtonColorShowSimpleColorcontourLow)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_MID, OnButtonColorShowSimpleColorcontourMid)
	ON_BN_CLICKED(IDC_BUTTON_RESET_DRAW_PARAM, OnButtonResetDrawParam)
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_MODIFY_DRW_PARAM_REFRESH, OnBnClickedModifyDrwParamRefresh)
	ON_BN_CLICKED(IDC_DRAW_PARAM_LOAD_FROM, OnBnClickedDrawParamLoadFrom)
	ON_BN_CLICKED(IDC_MODIFY_DRW_PARAM_USE_WITH_ALL, OnBnClickedModifyDrwParamUseWithAll)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_HIGH2, OnBnClickedButtonColorShowSimpleColorcontourHigh2)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_SPECIAL_CLASSES_VALUES, &CFmiModifyDrawParamDlg::OnEnChangeSpecialClassesValues)
    ON_CBN_SELCHANGE(IDC_COMBO_FIXED_DRAW_PARAM_SELECTOR, &CFmiModifyDrawParamDlg::OnCbnSelchangeComboFixedDrawParamSelector)
    ON_BN_CLICKED(IDC_BUTTON_RELOAD_ORIGINAL, &CFmiModifyDrawParamDlg::OnBnClickedButtonReloadOriginal)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiModifyDrawParamDlg message handlers

BOOL CFmiModifyDrawParamDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	InitDialogFromDrawParam();

    if(NFmiDrawParam::IsMacroParamCase(itsDrawParam->DataType())) // jos macroParam kyseessä, ei lyhennettä voi antaa editoida, koska se on tunniste Metkun editorille
	{
		CWnd *win = GetDlgItem(IDC_PARAM_ABBREVIATION);
		if(win)
			win->EnableWindow(FALSE);
	}

    AdjustStartingPosition();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiModifyDrawParamDlg::AdjustStartingPosition()
{
    // Want to move dialog over parent view with two things in mind:
    // 1. Put bottom-right corner over parent's bottom-right corner
    // 2. If dialog's top is higher than parent's top, set top to parent's top
    auto parentWindow = GetParent();
    if(parentWindow)
    {
        CRect parentRect;
        parentWindow->GetWindowRect(&parentRect);
        CRect windowRect;
        GetWindowRect(&windowRect);
        POINT place = parentRect.BottomRight() - windowRect.Size();
        if(place.y < parentRect.top)
            place.y = parentRect.top;
        CRect wantedWindowRect(place, SIZE(windowRect.Size()));
        Persist2::DoWindowMovements(this, true, wantedWindowRect);
    }
}

void CFmiModifyDrawParamDlg::InitDialogFromDrawParam(void)
{
	itsParamNameStrU_ = CA2T(itsDrawParam->Param().GetParamName());
    itsProducerStrU_ = CA2T(itsDrawParam->Param().GetProducer()->GetName());
	NFmiFileString fileString(itsDrawParam->InitFileName());
	itsDrawParamFileNameStrU_ = CString(_TEXT("\"")) + CA2T(fileString.FileName()) + _TEXT("\"");

	InitDialogTexts();
	ParamAbbreviation();
	IsHidden();
	OneSymbolRelativePositionOffset();
	OneSymbolRelativeSize();
	IsoLineGab();
//	ModifyingUnit();
	ModifyingStep();
	AbsoluteMinAndMaxValue();
	TimeSeriesScaleMinAndMax();
	TimeSerialModifyingLimit();
	InitRestOfVersion2Data();
	InitColors();
	MakeViewMacroAdjustments();
    InitFixedDrawParamSelector();
}

static std::string GetFileName(const std::string &theFilePath)
{
    NFmiFileString fileString(theFilePath);
    return std::string(fileString.Header());
}

void CFmiModifyDrawParamDlg::AddFixedDrawParamFolderToSelector(int theTreeIndex, const NFmiFixedDrawParamFolder &theFolder, CWzComboBox &theFixedDrawParamSelector)
{
    // Laitetaan ensin dropdown-puulistaan tämän hakemiston alihakemistot (rekursiivisesti)
    const std::vector<NFmiFixedDrawParamFolder> &subFolders = theFolder.SubFolders();
    for(auto subFolder : subFolders)
    {
        int subTreeIndex = theFixedDrawParamSelector.AddCTString(theTreeIndex, itsIconTypeFolderId, CA2T(subFolder.SubFolderName().c_str()));
        AddFixedDrawParamFolderToSelector(subTreeIndex, subFolder, theFixedDrawParamSelector);
    }

    // Laitetaan lopuksi dropdown-puulistaan tämän hakemiston drawParamit
    const std::vector<std::shared_ptr<NFmiDrawParam>> &drawParams = theFolder.DrawParams();
    for(auto drawParam : drawParams)
    {
        std::string listText = ::GetFileName(drawParam->InitFileName());
        theFixedDrawParamSelector.AddCTString(theTreeIndex, itsIconTypeFileId, CA2T(listText.c_str()));
    }
}

// HUOM! Nämä ikoni id:t on määritelty oikeasti SmartMet projektin recource.h:ssa. 
// Halusin irroittaa ToolBoxDep kirjaston riippuvuuden sinne, ja yritin lisätä ikonit tähän kirjastoon
// ja ladata ikoneja täältä käsin. Tämä ei onnistunut, joten minun pitää ladata ikonit SmartMet ohjelmasta
// sen projektin oikeilla id:llä.
#define IDI_ICON_FOLDER_OPEN_in_smartmet_project            2024
#define IDI_ICON_FILE_CHART_in_smartmet_project             2025

void CFmiModifyDrawParamDlg::InitFixedDrawParamSelector()
{
    if(fFixedDrawParamSelectorInitialized)
        return;
    fFixedDrawParamSelectorInitialized = true;

    const NFmiFixedDrawParamFolder &rootFolder = itsSmartMetDocumentInterface->FixedDrawParamSystem().RootFolder();
    if(rootFolder.Empty())
        return;

    HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON_FOLDER_OPEN_in_smartmet_project);
    itsIconTypeFolderId = itsFixedDrawParamSelector.AddIcon(hIcon);
    ::DestroyIcon(hIcon);

    hIcon = AfxGetApp()->LoadIcon(IDI_ICON_FILE_CHART_in_smartmet_project);
    itsIconTypeFileId = itsFixedDrawParamSelector.AddIcon(hIcon);
    ::DestroyIcon(hIcon);

    //itsFixedDrawParamSelector.SetBkGndColor(RGB(255, 255, 255));
    //itsFixedDrawParamSelector.SetHiLightBkGndColor(RGB(255, 255, 255));
    //itsFixedDrawParamSelector.SetTextColor(RGB(0, 0, 0));
    //itsFixedDrawParamSelector.SetHiLightTextColor(RGB(0, 0, 0));
    //itsFixedDrawParamSelector.SetHiLightFrameColor(RGB(0, 0, 0));

    AddFixedDrawParamFolderToSelector(nRootIndex, rootFolder, itsFixedDrawParamSelector);

    //WORD wParentIdx1 = itsFixedDrawParamSelector.AddCTString(nRootIndex, iconTypeFolder, _T("Root 1"));
    //WORD wParentIdx2 = itsFixedDrawParamSelector.AddCTString(nRootIndex, iconTypeFolder, _T("Root 2"));

    //WORD wParentlevel1 = itsFixedDrawParamSelector.AddCTString(wParentIdx1, iconTypeFile, _T("Root 1 1st level child"));
    //WORD wParentlevel2 = itsFixedDrawParamSelector.AddCTString(wParentIdx1, iconTypeFile, _T("Root 1 2nd level child"));

    itsFixedDrawParamSelector.SetCurSel(0);
}

void CFmiModifyDrawParamDlg::InitColors(void)
{
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->FillColor(), itsSymbolFillColorRef, &itsSymbolFillBitmap, itsSymbolFillColorRect, itsSymbolFillColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->FrameColor(), itsSymbolColorRef, &itsSymbolBitmap, itsSymbolColorRect, itsSymbolColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->IsolineTextColor(), itsIsoLineLabelColorRef, &itsIsoLineLabelBitmap, itsIsoLineLabelColorRect, itsIsoLineLabelColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->IsolineColor(), itsIsoLineColorRef, &itsIsoLineBitmap, itsIsoLineColorRect, itsIsoLineColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->IsolineLabelBoxFillColor(), itsIsoLineLabelBoxFillColorRef, &itsIsoLineLabelBoxFillBitmap, itsIsoLineLabelBoxFillColorRect, itsIsolineLabelBoxFillColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->StationSymbolColorShadeMidValueColor(), itsSymbolMidColorRef, &itsSymbolMidBitmap, itsSymbolMidColorRect, itsSymbolMidColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->StationSymbolColorShadeLowValueColor(), itsSymbolLowColorRef, &itsSymbolLowBitmap, itsSymbolLowColorRect, itsSymbolLowColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->StationSymbolColorShadeHighValueColor(), itsSymbolHighColorRef, &itsSymbolHighBitmap, itsSymbolHighColorRect, itsSymbolHighColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->SimpleIsoLineColorShadeMidValueColor(), itsSimpleIsoLineMidColorRef, &itsSimpleIsoLineMidBitmap, itsSimpleIsoLineMidColorRect, itsSimpleIsoLineMidColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->SimpleIsoLineColorShadeLowValueColor(), itsSimpleIsoLineLowColorRef, &itsSimpleIsoLineLowBitmap, itsSimpleIsoLineLowColorRect, itsSimpleIsoLineLowColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->SimpleIsoLineColorShadeHighValueColor(), itsSimpleIsoLineHighColorRef, &itsSimpleIsoLineHighBitmap, itsSimpleIsoLineHighColorRect, itsSimpleIsoLineHighColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->ColorContouringColorShadeMidValueColor(), itsSimpleColorContourMidColorRef, &itsSimpleColorContourMidBitmap, itsSimpleColorContourMidColorRect, itsSimpleColorContourMidColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->ColorContouringColorShadeLowValueColor(), itsSimpleColorContourLowColorRef, &itsSimpleColorContourLowBitmap, itsSimpleColorContourLowColorRect, itsSimpleColorContourLowColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->ColorContouringColorShadeHighValueColor(), itsSimpleColorContourHighColorRef, &itsSimpleColorContourHighBitmap, itsSimpleColorContourHighColorRect, itsSimpleColorContourHighColor);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->ColorContouringColorShadeHigh2ValueColor(), itsSimpleColorContourHigh2ColorRef, &itsSimpleColorContourHigh2Bitmap, itsSimpleColorContourHigh2ColorRect, itsSimpleColorContourHigh2Color);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->IsoLineHatchColor1(), itsHatch1ColorRef, &itsHatch1Bitmap, itsHatch1ColorRect, itsHatch1Color);
    CtrlView::InitialButtonColorUpdate(this, itsDrawParam->IsoLineHatchColor2(), itsHatch2ColorRef, &itsHatch2Bitmap, itsHatch2ColorRect, itsHatch2Color);
}

// itsStationDataViewSelector -combobox on täytetty legacy syistä seuraavasti:
// 1. ComboBoxin indeksit alkavat 0:sta
// 2. ComboBoxin indeksit 0-6 ovat NFmiMetEditorTypes::View:in arvot 1-7, tästä johtuen niihin joko lisätään tai vähennetään 1 kun muunnoksia tehdään (riippuen muunnos suunasta).
// 3. ComboBoxin indeksit 7-n ovat NFmiMetEditorTypes::View:in arvot 11-m, tästä johtuen niihin joko lisätään tai vähennetään 4 kun muunnoksia tehdään.
const int g_ViewTypeOffset1 = 1;
const int g_ViewTypeOffset2 = 4;
static int GetStationDataViewSelectorIndex(NFmiMetEditorTypes::View theViewType)
{
    if(theViewType >= NFmiMetEditorTypes::kFmiTextView && theViewType <= NFmiMetEditorTypes::kFmiIndexedTextView)
        return theViewType - g_ViewTypeOffset1;
    else if(theViewType <= NFmiMetEditorTypes::kFmiSmartSymbolView)
        return theViewType - g_ViewTypeOffset2;
    else if(theViewType <= NFmiMetEditorTypes::kFmiCustomSymbolView)
        return theViewType - g_ViewTypeOffset2;
    else
        return -1;
}

static NFmiMetEditorTypes::View GetSelectedStationDataViewType(CComboBox &theStationDataViewSelector)
{
    auto currentSelection = theStationDataViewSelector.GetCurSel();
    if(currentSelection >= (NFmiMetEditorTypes::kFmiTextView - g_ViewTypeOffset1) && currentSelection <= (NFmiMetEditorTypes::kFmiIndexedTextView - g_ViewTypeOffset1))
        return static_cast<NFmiMetEditorTypes::View>(theStationDataViewSelector.GetCurSel() + g_ViewTypeOffset1);
    else
        return static_cast<NFmiMetEditorTypes::View>(theStationDataViewSelector.GetCurSel() + g_ViewTypeOffset2);
}

void CFmiModifyDrawParamDlg::FillStationDataViewSelector(void)
{
	itsStationDataViewSelector.ResetContent();
	itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE").c_str())); // näiden 7 ensimmäisen arvot menevät 1-7, joten niille tehdään -1 ja +1 operaatiot
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE2").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE3").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE4").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE5").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE6").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE7").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("Precipitation Form").c_str())); // Huom! tämän arvo on oikeasti 11, joten pitää tehdä virityksiä
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("Synop weather symbol").c_str())); // Huom! tämän arvo on oikeasti 12, joten pitää tehdä virityksiä
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("Raw Mirri font symbol").c_str())); // Huom! tämän arvo on oikeasti 13, joten pitää tehdä virityksiä
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("Better weather symbol").c_str())); // Huom! tämän arvo on oikeasti 14, joten pitää tehdä virityksiä
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("SmartSymbol").c_str())); // Huom! tämän arvo on oikeasti 15, joten pitää tehdä virityksiä
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("CustomSymbol").c_str())); // Huom! tämän arvo on oikeasti 16, joten pitää tehdä virityksiä
    itsStationDataViewSelector.SetCurSel(::GetStationDataViewSelectorIndex(itsDrawParam->StationDataViewType()));
}

void CFmiModifyDrawParamDlg::InitRestOfVersion2Data(void)
{
	fUSeChangingColorsWithSymbols = itsDrawParam->ShowColoredNumbers();
	fUSeColorScaleWithSimpleIsoLines = !itsDrawParam->UseSingleColorsWithSimpleIsoLines();
	fUseIsoLineFeathering = itsDrawParam->UseIsoLineFeathering();
	fUseHatch1 = itsDrawParam->UseWithIsoLineHatch1();
	fDrawLabelBox = itsDrawParam->ShowSimpleIsoLineLabelBox();
	fDrawOnlyOverMask = itsDrawParam->DrawOnlyOverMask();
	fUseSpecialClasses = !itsDrawParam->UseSimpleIsoLineDefinitions();
	fUSeSeparatingLinesBetweenColorContourClasses = itsDrawParam->UseSeparatorLinesBetweenColorContourClasses();
	itsHatch1Style = itsDrawParam->IsoLineHatchType1();
	itsHatch1EndValue = itsDrawParam->IsoLineHatchHighValue1();
	itsHatch1StartValue = itsDrawParam->IsoLineHatchLowValue1();
	itsIsoLineGap = itsDrawParam->IsoLineGab();
	itsAlpha = itsDrawParam->Alpha();
	itsContourGap = itsDrawParam->ContourGab();
	itsIsoLineDecimals = itsDrawParam->IsoLineLabelDigitCount();
	itsIsoLineLabelHeight = itsDrawParam->SimpleIsoLineLabelHeight();
	itsIsoLineSmoothFactor = itsDrawParam->IsoLineSplineSmoothingFactor();
	itsIsoLineStyle = itsDrawParam->SimpleIsoLineLineStyle();
	itsIsoLineWidth = itsDrawParam->SimpleIsoLineWidth();
	itsIsoLineZeroValue_NEW = itsDrawParam->SimpleIsoLineZeroValue();
	itsGridDataDrawStyle = itsDrawParam->GridDataPresentationStyle()-1;

	itsSimpleClassCount = itsDrawParam->SimpleIsoLineColorShadeClassCount();
	itsSimpleClassEndValue = itsDrawParam->SimpleIsoLineColorShadeHighValue();
	itsSimpleClassMiddleValue_NEW = itsDrawParam->SimpleIsoLineColorShadeMidValue();
	itsSimpleClassStartValue = itsDrawParam->SimpleIsoLineColorShadeLowValue();
	itsSymbolsWithColorsClassCount = itsDrawParam->StationSymbolColorShadeClassCount();
	itsSymbolsWithColorsEndValue = itsDrawParam->StationSymbolColorShadeHighValue();
	itsSymbolsWithColorsMiddleValue = itsDrawParam->StationSymbolColorShadeMidValue();
	itsSymbolsWithColorsStartValue = itsDrawParam->StationSymbolColorShadeLowValue();

	itsSimpleColorContourClassEnd2Value = itsDrawParam->ColorContouringColorShadeHigh2Value();
	itsSimpleColorContourClassEndValue = itsDrawParam->ColorContouringColorShadeHighValue();
	itsSimpleColorContourClassMiddleValue = itsDrawParam->ColorContouringColorShadeMidValue();
	itsSimpleColorContourClassStartValue = itsDrawParam->ColorContouringColorShadeLowValue();
	fUseHatch2 = itsDrawParam->UseWithIsoLineHatch2() == TRUE;
	itsHatch2Style = itsDrawParam->IsoLineHatchType2();
	itsHatch2EndValue = itsDrawParam->IsoLineHatchHighValue2();
	itsHatch2StartValue = itsDrawParam->IsoLineHatchLowValue2();
	fUseIsoLineGabWithCustomContours = itsDrawParam->UseIsoLineGabWithCustomContours();
    fUseTransparentLabelBoxFillColor = itsDrawParam->UseTransparentFillColor();
    fDoSparseDataSymbolVisualization = itsDrawParam->DoSparseSymbolVisualization();

    FillStationDataViewSelector();
	InitSpecialClassesData();
}

void CFmiModifyDrawParamDlg::InitSpecialClassesData(void)
{
	checkedVector<float> specialIsoLineValues = itsDrawParam->SpecialIsoLineValues();
	size_t ssize = specialIsoLineValues.size();
	itsSpecialClassCount = static_cast<int>(specialIsoLineValues.size());
	NFmiString str;
	if(ssize > 0)
	{
		for(size_t i = 0; i < ssize; i++)
		{
			str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(specialIsoLineValues[i], 4);
			if(i < ssize - 1)
				str += ", ";
		}
	}
    itsSpecialClassValuesStrU_ = CA2T(str);

	checkedVector<int> specialClassColorIndices = itsDrawParam->SpecialIsoLineColorIndexies();
	ssize = specialClassColorIndices.size();
	str = "";
	if(ssize > 0)
	{
		for(size_t i = 0; i < ssize; i++)
		{
			NFmiValueString valStr(specialClassColorIndices[i], "%d");
			str += valStr;
			if(i < ssize - 1)
				str += ", ";
		}
	}
    itsSpecialClassColorIndicesStrU_ = CA2T(str);


	checkedVector<int> specialClassLineStyles = itsDrawParam->SpecialIsoLineStyle();
	ssize = specialClassLineStyles.size();
	str = "";
	if(ssize > 0)
	{
		for(size_t i = 0; i < ssize; i++)
		{
			NFmiValueString valStr(specialClassLineStyles[i], "%d");
			str += valStr;
			if(i < ssize - 1)
				str += ", ";
		}
	}
    itsSpecialClassLineStylesStrU_ = CA2T(str);


	checkedVector<float> specialClassLineWidth = itsDrawParam->SpecialIsoLineWidth();
	ssize = specialClassLineWidth.size();
	str = "";
	if(ssize > 0)
	{
		for(size_t i = 0; i < ssize; i++)
		{
			str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(specialClassLineWidth[i], 4);
			if(i < ssize - 1)
				str += ", ";
		}
	}
    itsSpecialClassLineWidthStrU_ = CA2T(str);

	// itsSpecialClassLabelColorIndicesStr:iä käytetään väliaikaisesti label korkeuden kanssa!!!!!!!!!!!!!!!!!!!!!!!!!!
	checkedVector<float> specialIsoLineLabelHeight = itsDrawParam->SpecialIsoLineLabelHeight();
	ssize = specialIsoLineLabelHeight.size();
	str = "";
	if(ssize > 0)
	{
		for(size_t i = 0; i < ssize; i++)
		{
			str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(specialIsoLineLabelHeight[i], 4);
			if(i < ssize - 1)
				str += ", ";
		}
	}
    itsSpecialClassLabelColorIndicesStrU_ = CA2T(str);

}



void CFmiModifyDrawParamDlg::ReadRestOfVersion2Data(void)
{
	itsDrawParam->ShowColoredNumbers(fUSeChangingColorsWithSymbols != 0);
	itsDrawParam->UseSingleColorsWithSimpleIsoLines(!fUSeColorScaleWithSimpleIsoLines);
	itsDrawParam->UseIsoLineFeathering(fUseIsoLineFeathering == TRUE);
	itsDrawParam->UseWithIsoLineHatch1(fUseHatch1 == TRUE);
	itsDrawParam->ShowSimpleIsoLineLabelBox(fDrawLabelBox == TRUE);
	itsDrawParam->DrawOnlyOverMask(fDrawOnlyOverMask == TRUE);
	itsDrawParam->UseSimpleIsoLineDefinitions(!fUseSpecialClasses);
	itsDrawParam->UseSeparatorLinesBetweenColorContourClasses(fUSeSeparatingLinesBetweenColorContourClasses == TRUE);
	itsDrawParam->IsoLineHatchType1(itsHatch1Style);
	itsDrawParam->IsoLineHatchHighValue1(static_cast<float>(itsHatch1EndValue));
	itsDrawParam->IsoLineHatchLowValue1(static_cast<float>(itsHatch1StartValue));
	itsDrawParam->IsoLineGab(itsIsoLineGap);
	itsDrawParam->Alpha(itsAlpha);
	if(fSkipreadingSpecialClassColorIndices == false) // nämä on siis jo päivitetty TMColorIndex-disalogissa, ei saa päivittää niitä arvoja päälle
	{
		itsDrawParam->ContourGab(itsContourGap);
	}
	itsDrawParam->IsoLineLabelDigitCount(itsIsoLineDecimals);
	itsDrawParam->SimpleIsoLineLabelHeight(static_cast<float>(itsIsoLineLabelHeight));
	itsDrawParam->IsoLineSplineSmoothingFactor(itsIsoLineSmoothFactor);
	itsDrawParam->SimpleIsoLineLineStyle(itsIsoLineStyle);
	itsDrawParam->SimpleIsoLineWidth(itsIsoLineWidth);
	itsDrawParam->SimpleIsoLineZeroValue(static_cast<float>(itsIsoLineZeroValue_NEW));
	itsDrawParam->GridDataPresentationStyle(itsGridDataDrawStyle+1);

	itsDrawParam->SimpleIsoLineColorShadeClassCount(itsSimpleClassCount);
	itsDrawParam->SimpleIsoLineColorShadeHighValue(itsSimpleClassEndValue);
	itsDrawParam->SimpleIsoLineColorShadeMidValue(static_cast<float>(itsSimpleClassMiddleValue_NEW));
	itsDrawParam->SimpleIsoLineColorShadeLowValue(itsSimpleClassStartValue);
	itsDrawParam->StationSymbolColorShadeClassCount(itsSymbolsWithColorsClassCount);
	itsDrawParam->StationSymbolColorShadeHighValue(itsSymbolsWithColorsEndValue);
	itsDrawParam->StationSymbolColorShadeMidValue(itsSymbolsWithColorsMiddleValue);
	itsDrawParam->StationSymbolColorShadeLowValue(itsSymbolsWithColorsStartValue);

	itsDrawParam->ColorContouringColorShadeHigh2Value(itsSimpleColorContourClassEnd2Value);
	itsDrawParam->ColorContouringColorShadeHighValue(itsSimpleColorContourClassEndValue);
	itsDrawParam->ColorContouringColorShadeMidValue(itsSimpleColorContourClassMiddleValue);
	itsDrawParam->ColorContouringColorShadeLowValue(itsSimpleColorContourClassStartValue);
	itsDrawParam->UseWithIsoLineHatch2(fUseHatch2 == TRUE);
	itsDrawParam->IsoLineHatchType2(static_cast<int>(itsHatch2Style));
	itsDrawParam->IsoLineHatchHighValue2(static_cast<float>(itsHatch2EndValue));
	itsDrawParam->IsoLineHatchLowValue2(static_cast<float>(itsHatch2StartValue));
	if(fSkipreadingSpecialClassColorIndices == false) // nämä on siis jo päivitetty TMColorIndex-disalogissa, ei saa päivittää niitä arvoja päälle
	{
		itsDrawParam->UseIsoLineGabWithCustomContours(fUseIsoLineGabWithCustomContours == TRUE);
	}
	itsDrawParam->StationDataViewType(::GetSelectedStationDataViewType(itsStationDataViewSelector));
    itsDrawParam->UseTransparentFillColor(fUseTransparentLabelBoxFillColor == TRUE);
    itsDrawParam->DoSparseSymbolVisualization(fDoSparseDataSymbolVisualization == TRUE);

	ReadSpecialClassesData();
}

void CFmiModifyDrawParamDlg::ReadSpecialClassesData(void)
{
	std::string problemVariableStr;
	std::string currentValueStr;
	try
	{
		if(fSkipreadingSpecialClassColorIndices == false) // nämä on siis jo päivitetty TMColorIndex-disalogissa, ei saa päivittää niitä arvoja päälle
		{
			problemVariableStr = "Problem with SpecialClassValues string\n";
			currentValueStr = CT2A(itsSpecialClassValuesStrU_);
			itsDrawParam->SetSpecialIsoLineValues(NFmiStringTools::Split<checkedVector<float> >(currentValueStr, ","));

			problemVariableStr = "Problem with SpecialClassColorIndices string\n";
            currentValueStr = CT2A(itsSpecialClassColorIndicesStrU_);
			itsDrawParam->SetSpecialIsoLineColorIndexies(NFmiStringTools::Split<checkedVector<int> >(currentValueStr, ","));
		}

		problemVariableStr = "Problem with SpecialClassLineStyles string\n";
        currentValueStr = CT2A(itsSpecialClassLineStylesStrU_);
		itsDrawParam->SetSpecialIsoLineStyle(NFmiStringTools::Split<checkedVector<int> >(currentValueStr, ","));

		problemVariableStr = "Problem with SpecialClassLineWidth string\n";
        currentValueStr = CT2A(itsSpecialClassLineWidthStrU_);
		itsDrawParam->SetSpecialIsoLineWidth(NFmiStringTools::Split<checkedVector<float> >(currentValueStr, ","));

		problemVariableStr = "Problem with SpecialIsoLineLabelHeight string\n";
        currentValueStr = CT2A(itsSpecialClassLabelColorIndicesStrU_);
		itsDrawParam->SetSpecialIsoLineLabelHeight(NFmiStringTools::Split<checkedVector<float> >(currentValueStr, ","));
	}
	catch(std::exception &e)
	{
		::MessageBox(this->GetSafeHwnd(), CA2T(std::string(problemVariableStr + currentValueStr + "\nWith error:\n" + e.what()).c_str()), _TEXT("Error with special contour settings"), MB_OK);
	}
	catch(...)
	{
        ::MessageBox(this->GetSafeHwnd(), CA2T(std::string(problemVariableStr + currentValueStr + "\nUnknown problem with given string.").c_str()), _TEXT("Error with special contour settings"), MB_OK);
	}
}


void CFmiModifyDrawParamDlg::AbsoluteMinAndMaxValue(void)
{
	itsParamAbsolutValueMin = itsDrawParam->AbsoluteMinValue();
	itsParamAbsolutValueMax = itsDrawParam->AbsoluteMaxValue();
	return;
}

void CFmiModifyDrawParamDlg::TimeSeriesScaleMinAndMax(void)
{
	itsTimeSeriesScaleMin = itsDrawParam->TimeSeriesScaleMin();
	itsTimeSeriesScaleMax = itsDrawParam->TimeSeriesScaleMax();
	return;
}

void CFmiModifyDrawParamDlg::TimeSerialModifyingLimit(void)
{
	itsTimeSeriesModifyLimit = itsDrawParam->TimeSerialModifyingLimit();
	return;
}

void CFmiModifyDrawParamDlg::ModifyingStep(void)
{
	itsModifyingStep = itsDrawParam->ModifyingStep();
	return;
}

void CFmiModifyDrawParamDlg::IsoLineGab(void)
{
	itsIsoLineGap = itsDrawParam->IsoLineGab();
	return;
}

//---------------------------------------------------------------------------
//	OneSymbolRelativeSize
//---------------------------------------------------------------------------
void CFmiModifyDrawParamDlg::OneSymbolRelativeSize(void)
{
	itsOneSymbolWidth = itsDrawParam->OnlyOneSymbolRelativeSize().X();
	itsOneSymbolHeight = itsDrawParam->OnlyOneSymbolRelativeSize().Y();
}

//---------------------------------------------------------------------------
//	OneSymbolRelativePositionOffset
//---------------------------------------------------------------------------
void CFmiModifyDrawParamDlg::OneSymbolRelativePositionOffset(void)
{
	itsOneSymbolHorizontalOffset_NEW = itsDrawParam->OnlyOneSymbolRelativePositionOffset().X();
	itsOneSymbolVerticalOffset_NEW = itsDrawParam->OnlyOneSymbolRelativePositionOffset().Y();
}

void CFmiModifyDrawParamDlg::IsHidden(void)
{
	fIsHidden = itsDrawParam->IsParamHidden();
	return;
}

void CFmiModifyDrawParamDlg::ParamAbbreviation(void)
{
    itsParamAbbreviationStrU_ = CA2T(itsDrawParam->ParameterAbbreviation().c_str());
    if(!NFmiDrawParam::IsMacroParamCase(itsDrawParam->DataType()) && itsParamAbbreviationStrU_.GetLength() > 15) // jos ei macroParam tyyppiä (niiden nimiä ei voi editoida), leikataan nimen lyhennetta niin että se on maksimissaan 15 merkkiä
		itsParamAbbreviationStrU_ = itsParamAbbreviationStrU_.Left(15);
	return;
}

void CFmiModifyDrawParamDlg::OnButtonHatch1Color()
{
    CtrlView::ColorButtonPressed(this, itsHatch1ColorRef, &itsHatch1Bitmap, itsHatch1ColorRect, itsHatch1Color);
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleIsolineHigh()
{
    CtrlView::ColorButtonPressed(this, itsSimpleIsoLineHighColorRef, &itsSimpleIsoLineHighBitmap, itsSimpleIsoLineHighColorRect, itsSimpleIsoLineHighColor);
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleIsolineLow()
{
    CtrlView::ColorButtonPressed(this, itsSimpleIsoLineLowColorRef, &itsSimpleIsoLineLowBitmap, itsSimpleIsoLineLowColorRect, itsSimpleIsoLineLowColor);
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleIsolineMid()
{
    CtrlView::ColorButtonPressed(this, itsSimpleIsoLineMidColorRef, &itsSimpleIsoLineMidBitmap, itsSimpleIsoLineMidColorRect, itsSimpleIsoLineMidColor);
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSymbHigh()
{
    CtrlView::ColorButtonPressed(this, itsSymbolHighColorRef, &itsSymbolHighBitmap, itsSymbolHighColorRect, itsSymbolHighColor);
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSymbLow()
{
    CtrlView::ColorButtonPressed(this, itsSymbolLowColorRef, &itsSymbolLowBitmap, itsSymbolLowColorRect, itsSymbolLowColor);
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSymbMid()
{
    CtrlView::ColorButtonPressed(this, itsSymbolMidColorRef, &itsSymbolMidBitmap, itsSymbolMidColorRect, itsSymbolMidColor);
}

void CFmiModifyDrawParamDlg::OnButtonIsolineColor()
{
    CtrlView::ColorButtonPressed(this, itsIsoLineColorRef, &itsIsoLineBitmap, itsIsoLineColorRect, itsIsoLineColor);
}

void CFmiModifyDrawParamDlg::OnBnClickedButtonIsolineLabelBoxFillColor()
{
    CtrlView::ColorButtonPressed(this, itsIsoLineLabelBoxFillColorRef, &itsIsoLineLabelBoxFillBitmap, itsIsoLineLabelBoxFillColorRect, itsIsolineLabelBoxFillColor);
}

void CFmiModifyDrawParamDlg::OnButtonIsolineLabelColor()
{
    CtrlView::ColorButtonPressed(this, itsIsoLineLabelColorRef, &itsIsoLineLabelBitmap, itsIsoLineLabelColorRect, itsIsoLineLabelColor);
}

void CFmiModifyDrawParamDlg::OnButtonSymbolColor()
{
    CtrlView::ColorButtonPressed(this, itsSymbolColorRef, &itsSymbolBitmap, itsSymbolColorRect, itsSymbolColor);
}

void CFmiModifyDrawParamDlg::OnButtonSymbolFillColor()
{
    CtrlView::ColorButtonPressed(this, itsSymbolFillColorRef, &itsSymbolFillBitmap, itsSimpleIsoLineLowColorRect, itsSymbolFillColor);
}

void CFmiModifyDrawParamDlg::OnOK()
{
	UpdateData(TRUE);
	GetSelectedDrawParamSetUp();
	TakeDrawParamModificationInUse(); // vain onok:ssa initialisoidaan takaisin originaali drawparamiin. cancel ei siirrä muutoksia

	CDialog::OnOK();
}

void CFmiModifyDrawParamDlg::GetSelectedDrawParamSetUp(void)
{
	ReadParameterAbbreviation();
	ReadHidden();
	ReadOneSymbolRelativeSize();
	ReadOneSymbolRelativePositionOffset();
	ReadIsoLineGab();
	ReadModifyingStep();
	ReadAbsoluteMinAndMaxValue();
	ReadTimeSeriesScaleMinAndMax();
	ReadTimeSerialModifyingLimit();
	ReadAllButtonColors();
	ReadRestOfVersion2Data();
}

void CFmiModifyDrawParamDlg::DoOnCancel()
{
    itsOrigDrawParam->Init(itsBackupDrawParam); // tässä palautetaan originaali asetukset takaisin.
    // Tämä on tarpeen jos on käytetty refreshiä ja sitten perutaan cancelista muutokset.

    ForceStationViewUpdate();
}

void CFmiModifyDrawParamDlg::ForceStationViewUpdate()
{
    if(itsDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
    {
        // Liataan myös maphandler, jotta käytössä karttanäytössä ollut stationView vaihtuu tarvittaessa jos esim. isoline piirto vaihtuu teksti esitykseen.
        itsSmartMetDocumentInterface->MapViewDescTop(itsDescTopIndex)->MapHandler()->SetUpdateMapViewDrawingLayers(true);
    }
}

void CFmiModifyDrawParamDlg::OnCancel()
{
    DoOnCancel();

	CDialog::OnCancel();
}

void CFmiModifyDrawParamDlg::OnClose()
{
    DoOnCancel();
    
    CDialog::OnClose();
}

void CFmiModifyDrawParamDlg::OnSaveAsButton()
{
    CFileDialog dlg(FALSE, _TEXT("dpa"), CA2T(itsDrawParam->InitFileName().c_str()));
	if(dlg.DoModal() == IDOK)
	{
		UpdateData(TRUE);
		GetSelectedDrawParamSetUp();
		std::string tmp = CT2A(dlg.GetPathName());
		itsDrawParam->StoreData(tmp);
	}
}

void CFmiModifyDrawParamDlg::OnBnClickedDrawParamLoadFrom()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, _TEXT("DrawParam-files (*.dpa)|*.dpa|"));
    dlg.m_ofn.lpstrInitialDir = CA2T(itsDrawParamPath.c_str());
	if(dlg.DoModal() == IDOK)
	{
		NFmiDrawParam dParam;
		std::string tmp = CT2A(dlg.GetPathName());
		dParam.Init(tmp);
		itsDrawParam->Init(&dParam, true);
		InitDialogFromDrawParam();
		UpdateData(FALSE);
	}
}

void CFmiModifyDrawParamDlg::OnSaveButton()
{
	UpdateData(TRUE);
	OnBnClickedModifyDrwParamRefresh(); // otetaan muutokset myös heti käyttöön, koska ne tulevat käyttöön joka tapauksessa
	TakeDrawParamModificationInUse(); // vain onok:ssa (ja kun asetukset otetaan kaikkialle käyttöön ja save-nappia painettaessa)
									  // initialisoidaan takaisin originaali drawparamiin. cancel ei siirrä muutoksia
	if(itsDrawParam->StoreData(itsDrawParam->InitFileName()) == false)
	{
		std::string errMsgTitle("Cannot save drawParam");
		std::string errMsg;
		if(itsDrawParam->InitFileName().empty())
		{
			errMsg += "SmartMet drawParam configurations are invalid.\n";
			errMsg += "You have to change drawParam saving location from\n";
			errMsg += "Edit - Macro path Settings... - DrawParam path\n";
			errMsg += "and given path must be able to be created.\n";
			errMsg += "After changing the path you must restart the program.";
		}
		else
		{
			errMsg += "Cannot save drawParam in file:";
			errMsg += itsDrawParam->InitFileName();
			errMsg += "\nCheck that the folder exists and you have write permissions there.";
		}
        ::MessageBox(this->GetSafeHwnd(), CA2T(errMsg.c_str()), CA2T(errMsgTitle.c_str()), MB_ICONINFORMATION | MB_OK);
	}
}

void CFmiModifyDrawParamDlg::ReadParameterAbbreviation(void)
{
	std::string tmp = CT2A(itsParamAbbreviationStrU_);
	itsDrawParam->ParameterAbbreviation(tmp);
	return;
}

void CFmiModifyDrawParamDlg::ReadHidden(void)
{
	itsDrawParam->HideParam(fIsHidden != 0);
}

void CFmiModifyDrawParamDlg::ReadOneSymbolRelativeSize()
{
	itsDrawParam->OnlyOneSymbolRelativeSize(NFmiPoint(itsOneSymbolWidth, itsOneSymbolHeight));
}

void CFmiModifyDrawParamDlg::ReadOneSymbolRelativePositionOffset(void)
{
	itsDrawParam->OnlyOneSymbolRelativePositionOffset(NFmiPoint(itsOneSymbolHorizontalOffset_NEW, itsOneSymbolVerticalOffset_NEW));
}

void CFmiModifyDrawParamDlg::ReadIsoLineGab(void)
{
	itsDrawParam->IsoLineGab(itsIsoLineGap);
}

void CFmiModifyDrawParamDlg::ReadModifyingStep(void)
{
	itsDrawParam->ModifyingStep(itsModifyingStep);
}

void CFmiModifyDrawParamDlg::ReadAbsoluteMinAndMaxValue(void)
{
	if(itsParamAbsolutValueMin > itsParamAbsolutValueMax)
	{
		double value = itsParamAbsolutValueMin;
		itsParamAbsolutValueMin = itsParamAbsolutValueMax;
		itsParamAbsolutValueMax = value;
	}
	itsDrawParam->AbsoluteMinValue(itsParamAbsolutValueMin);
	itsDrawParam->AbsoluteMaxValue(itsParamAbsolutValueMax);
}

void CFmiModifyDrawParamDlg::ReadTimeSeriesScaleMinAndMax(void)
{
	if (itsTimeSeriesScaleMin > itsTimeSeriesScaleMax)
	{
		double value = itsTimeSeriesScaleMin;
		itsTimeSeriesScaleMin = itsTimeSeriesScaleMax;
		itsTimeSeriesScaleMax = value;
	}

	itsDrawParam->TimeSeriesScaleMin(itsTimeSeriesScaleMin);
	itsDrawParam->TimeSeriesScaleMax(itsTimeSeriesScaleMax);
}

void CFmiModifyDrawParamDlg::ReadTimeSerialModifyingLimit(void)
{
	itsDrawParam->TimeSerialModifyingLimit(itsTimeSeriesModifyLimit);
}

void CFmiModifyDrawParamDlg::ReadAllButtonColors(void)
{
	NFmiColor tColor;

	tColor.SetRGB((float(GetRValue(itsSymbolColorRef))/float(255.0)),
					(float(GetGValue(itsSymbolColorRef))/float(255.0)),
						(float(GetBValue(itsSymbolColorRef))/float(255.0)));
	itsDrawParam->FrameColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSymbolFillColorRef))/float(255.0)),
					(float(GetGValue(itsSymbolFillColorRef))/float(255.0)),
						(float(GetBValue(itsSymbolFillColorRef))/float(255.0)));
	itsDrawParam->FillColor(tColor);

	tColor.SetRGB((float(GetRValue(itsIsoLineColorRef))/float(255.0)),
					(float(GetGValue(itsIsoLineColorRef))/float(255.0)),
						(float(GetBValue(itsIsoLineColorRef))/float(255.0)));
	itsDrawParam->IsolineColor(tColor);

	tColor.SetRGB((float(GetRValue(itsIsoLineLabelBoxFillColorRef))/float(255.0)),
					(float(GetGValue(itsIsoLineLabelBoxFillColorRef))/float(255.0)),
						(float(GetBValue(itsIsoLineLabelBoxFillColorRef))/float(255.0)));
	itsDrawParam->IsolineLabelBoxFillColor(tColor);

	tColor.SetRGB((float(GetRValue(itsIsoLineLabelColorRef))/float(255.0)),
					(float(GetGValue(itsIsoLineLabelColorRef))/float(255.0)),
						(float(GetBValue(itsIsoLineLabelColorRef))/float(255.0)));
	itsDrawParam->IsolineTextColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSimpleIsoLineHighColorRef))/float(255.0)),
					(float(GetGValue(itsSimpleIsoLineHighColorRef))/float(255.0)),
						(float(GetBValue(itsSimpleIsoLineHighColorRef))/float(255.0)));
	itsDrawParam->SimpleIsoLineColorShadeHighValueColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSimpleIsoLineLowColorRef))/float(255.0)),
					(float(GetGValue(itsSimpleIsoLineLowColorRef))/float(255.0)),
						(float(GetBValue(itsSimpleIsoLineLowColorRef))/float(255.0)));
	itsDrawParam->SimpleIsoLineColorShadeLowValueColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSimpleIsoLineMidColorRef))/float(255.0)),
					(float(GetGValue(itsSimpleIsoLineMidColorRef))/float(255.0)),
						(float(GetBValue(itsSimpleIsoLineMidColorRef))/float(255.0)));
	itsDrawParam->SimpleIsoLineColorShadeMidValueColor(tColor);

	tColor.SetRGB((float(GetRValue(itsHatch1ColorRef))/float(255.0)),
					(float(GetGValue(itsHatch1ColorRef))/float(255.0)),
						(float(GetBValue(itsHatch1ColorRef))/float(255.0)));
	itsDrawParam->IsoLineHatchColor1(tColor);

	tColor.SetRGB((float(GetRValue(itsHatch2ColorRef))/float(255.0)),
					(float(GetGValue(itsHatch2ColorRef))/float(255.0)),
						(float(GetBValue(itsHatch2ColorRef))/float(255.0)));
	itsDrawParam->IsoLineHatchColor2(tColor);

	tColor.SetRGB((float(GetRValue(itsSymbolHighColorRef))/float(255.0)),
					(float(GetGValue(itsSymbolHighColorRef))/float(255.0)),
						(float(GetBValue(itsSymbolHighColorRef))/float(255.0)));
	itsDrawParam->StationSymbolColorShadeHighValueColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSymbolLowColorRef))/float(255.0)),
					(float(GetGValue(itsSymbolLowColorRef))/float(255.0)),
						(float(GetBValue(itsSymbolLowColorRef))/float(255.0)));
	itsDrawParam->StationSymbolColorShadeLowValueColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSymbolMidColorRef))/float(255.0)),
					(float(GetGValue(itsSymbolMidColorRef))/float(255.0)),
						(float(GetBValue(itsSymbolMidColorRef))/float(255.0)));
	itsDrawParam->StationSymbolColorShadeMidValueColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSimpleColorContourHighColorRef))/float(255.0)),
					(float(GetGValue(itsSimpleColorContourHighColorRef))/float(255.0)),
						(float(GetBValue(itsSimpleColorContourHighColorRef))/float(255.0)));
	itsDrawParam->ColorContouringColorShadeHighValueColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSimpleColorContourHigh2ColorRef))/float(255.0)),
					(float(GetGValue(itsSimpleColorContourHigh2ColorRef))/float(255.0)),
						(float(GetBValue(itsSimpleColorContourHigh2ColorRef))/float(255.0)));
	itsDrawParam->ColorContouringColorShadeHigh2ValueColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSimpleColorContourLowColorRef))/float(255.0)),
					(float(GetGValue(itsSimpleColorContourLowColorRef))/float(255.0)),
						(float(GetBValue(itsSimpleColorContourLowColorRef))/float(255.0)));
	itsDrawParam->ColorContouringColorShadeLowValueColor(tColor);

	tColor.SetRGB((float(GetRValue(itsSimpleColorContourMidColorRef))/float(255.0)),
					(float(GetGValue(itsSimpleColorContourMidColorRef))/float(255.0)),
						(float(GetBValue(itsSimpleColorContourMidColorRef))/float(255.0)));
	itsDrawParam->ColorContouringColorShadeMidValueColor(tColor);

	return;
}



void CFmiModifyDrawParamDlg::OnShowColorIndexDlg()
{
	UpdateData(TRUE);
	GetSelectedDrawParamSetUp();
    
	CFmiTMColorIndexDlg dlg(this, ::GetDictionaryString("ModifyDrawParamColorIndexTitle"), ::GetDictionaryString("ModifyDrawParamColorIndexHelpStr"),
        ToolMasterColorCube::UsedColorsCube(), itsDrawParam, this);
	if(dlg.DoModal() == IDOK)
	{
		InitSpecialClassesData(); // valitut special värit pitää alustaa uudestaan
		itsContourGap = itsDrawParam->ContourGab();
		fUseIsoLineGabWithCustomContours = itsDrawParam->UseIsoLineGabWithCustomContours();
		UpdateData(FALSE);
	}
}

void CFmiModifyDrawParamDlg::OnButtonHatch2Color()
{
    CtrlView::ColorButtonPressed(this, itsHatch2ColorRef, &itsHatch2Bitmap, itsHatch2ColorRect, itsHatch2Color);
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleColorcontourHigh()
{
    CtrlView::ColorButtonPressed(this, itsSimpleColorContourHighColorRef, &itsSimpleColorContourHighBitmap, itsSimpleColorContourHighColorRect, itsSimpleColorContourHighColor);
}

void CFmiModifyDrawParamDlg::OnBnClickedButtonColorShowSimpleColorcontourHigh2()
{
    CtrlView::ColorButtonPressed(this, itsSimpleColorContourHigh2ColorRef, &itsSimpleColorContourHigh2Bitmap, itsSimpleColorContourHigh2ColorRect, itsSimpleColorContourHigh2Color);
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleColorcontourLow()
{
    CtrlView::ColorButtonPressed(this, itsSimpleColorContourLowColorRef, &itsSimpleColorContourLowBitmap, itsSimpleColorContourLowColorRect, itsSimpleColorContourLowColor);
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleColorcontourMid()
{
    CtrlView::ColorButtonPressed(this, itsSimpleColorContourMidColorRef, &itsSimpleColorContourMidBitmap, itsSimpleColorContourMidColorRect, itsSimpleColorContourMidColor);
}

void CFmiModifyDrawParamDlg::OnButtonResetDrawParam()
{
	NFmiDrawParam defaultDrawParam;
	itsDrawParam->Init(&defaultDrawParam, true);
	InitDialogFromDrawParam();
	UpdateData(FALSE);
}

void CFmiModifyDrawParamDlg::MakeViewMacroAdjustments(void)
{
	if(itsDrawParam->ViewMacroDrawParam())
	{ // jos viewmacro drawParam, estetään talletus mahdollisuus ja laitetaan varoitus näkyviin punaisella
		CWnd* win = GetDlgItem(IDC_STATIC_VIEW_MACRO_DRAW_PARAM_WARNING);
		if(win)
			win->ShowWindow(SW_SHOW);
		win = GetDlgItem(IDC_SAVE_BUTTON);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_SAVE_AS_BUTTON);
		if(win)
			win->EnableWindow(FALSE);
	}
	else
	{
		CWnd* win = GetDlgItem(IDC_STATIC_VIEW_MACRO_DRAW_PARAM_WARNING);
		if(win)
			win->ShowWindow(SW_HIDE);
		win = GetDlgItem(IDC_SAVE_BUTTON);
		if(win)
			win->EnableWindow(TRUE);
		win = GetDlgItem(IDC_SAVE_AS_BUTTON);
		if(win)
			win->EnableWindow(TRUE);
	}
}

HBRUSH CFmiModifyDrawParamDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_VIEW_MACRO_DRAW_PARAM_WARNING) // värjätään viewmacro-drawparam varoitus teksti punaisella
		pDC->SetTextColor(RGB(255, 0, 0));

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_DRAW_PARAM_SPECIAL_CLASSES_STR)
	{
		if(this->fSpecialClassesHaveInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // virhetilanteissa erikois luokka edit boxin labeli väritetään punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0));
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiModifyDrawParamDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("ModifyDrawParamDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_PARAM_ST, "IDC_PARAM_ST");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_PRIORITY_ST, "IDC_PRIORITY_ST");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_PARAM_ST2, "IDC_PARAM_ST2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_F_HIDDEN, "IDC_F_HIDDEN");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_SAVE_BUTTON, "IDC_SAVE_BUTTON");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_SAVE_AS_BUTTON, "IDC_SAVE_AS_BUTTON");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RESET_DRAW_PARAM, "IDC_BUTTON_RESET_DRAW_PARAM");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VIEW_MACRO_DRAW_PARAM_WARNING, "IDC_STATIC_VIEW_MACRO_DRAW_PARAM_WARNING");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SYMBOL_SETTINGS_STR, "IDC_STATIC_DRAW_PARAM_SYMBOL_SETTINGS_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_ONE_SYMBOL_SIZE_STR, "IDC_STATIC_DRAW_PARAM_ONE_SYMBOL_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_ONE_SYMBOL_OFFSET_STR, "IDC_STATIC_DRAW_PARAM_ONE_SYMBOL_OFFSET_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_WIDTH_STR2, "IDC_STATIC_DRAW_PARAM_WIDTH_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HEIGHT_STR2, "IDC_STATIC_DRAW_PARAM_HEIGHT_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HORIZ_STR2, "IDC_STATIC_DRAW_PARAM_HORIZ_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_VERTICAL_STR2, "IDC_STATIC_DRAW_PARAM_VERTICAL_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_SYMBOL_CHANGING_COLORS, "IDC_CHECK_SHOW_SYMBOL_CHANGING_COLORS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_COLOR_CLASS_COUNT_STR, "IDC_STATIC_DRAW_PARAM_COLOR_CLASS_COUNT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SYMBOL_COLOR_STR, "IDC_STATIC_DRAW_PARAM_SYMBOL_COLOR_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SYMBOL_FILL_COLOR_STR, "IDC_STATIC_DRAW_PARAM_SYMBOL_FILL_COLOR_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_GRID_DATA_SETTINGS_STR, "IDC_STATIC_DRAW_PARAM_GRID_DATA_SETTINGS_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_DATA_DRAW_STYLE, "IDC_RADIO_GRID_DATA_DRAW_STYLE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_DATA_DRAW_STYLE2, "IDC_RADIO_GRID_DATA_DRAW_STYLE2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_DATA_DRAW_STYLE3, "IDC_RADIO_GRID_DATA_DRAW_STYLE3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_DATA_DRAW_STYLE4, "IDC_RADIO_GRID_DATA_DRAW_STYLE4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_DATA_DRAW_STYLE5, "IDC_RADIO_GRID_DATA_DRAW_STYLE5");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_DATA_DRAW_STYLE6, "IDC_RADIO_GRID_DATA_DRAW_STYLE6");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_DATA_DRAW_STYLE7, "IDC_RADIO_GRID_DATA_DRAW_STYLE7");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_FEATHERING, "IDC_CHECK_USE_FEATHERING");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_REGIONS, "IDC_CHECK_USE_REGIONS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SMOOTH_STR, "IDC_STATIC_DRAW_PARAM_SMOOTH_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_DECIMAL_COUNT_STR, "IDC_STATIC_DRAW_PARAM_DECIMAL_COUNT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_STEP_STR, "IDC_STATIC_DRAW_PARAM_STEP_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_ZERO_POINT_STR, "IDC_STATIC_DRAW_PARAM_ZERO_POINT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_LINE_TYPE_STR, "IDC_STATIC_DRAW_PARAM_LINE_TYPE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_ISOLINE_COLOR_STR, "IDC_STATIC_DRAW_PARAM_ISOLINE_COLOR_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_ISOLINE_LABEL_COLOR_STR, "IDC_STATIC_DRAW_PARAM_ISOLINE_LABEL_COLOR_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_LABEL_BOX_COLOR_STR, "IDC_STATIC_DRAW_PARAM_LABEL_BOX_COLOR_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_LINE_WIDTH_STR, "IDC_STATIC_DRAW_PARAM_LINE_WIDTH_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_LABEL_FONT_SIZE_STR, "IDC_STATIC_DRAW_PARAM_LABEL_FONT_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_LABEL_BOX, "IDC_CHECK_USE_LABEL_BOX");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_ISOLINE_COLORS_STR, "IDC_STATIC_DRAW_PARAM_ISOLINE_COLORS_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_COLOR_CONTOUR_COLORS_STR, "IDC_STATIC_DRAW_PARAM_COLOR_CONTOUR_COLORS_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_ISOLINE_CLASS_COUNT_STR, "IDC_STATIC_DRAW_PARAM_ISOLINE_CLASS_COUNT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_COLOR_SCALE_WITH_SIMPLE_ISOLINES, "IDC_CHECK_USE_COLOR_SCALE_WITH_SIMPLE_ISOLINES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_SEPARATOR_LINES_BETWEEN_COLORCONTOUR_CLASSES, "IDC_CHECK_USE_SEPARATOR_LINES_BETWEEN_COLORCONTOUR_CLASSES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_SPECIAL_CLASSES, "IDC_CHECK_USE_SPECIAL_CLASSES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_SHOW_COLOR_INDEX_DLG, "IDC_SHOW_COLOR_INDEX_DLG");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_CLASSES_STR, "IDC_STATIC_DRAW_PARAM_SPECIAL_CLASSES_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_WIDTHS_STR, "IDC_STATIC_DRAW_PARAM_SPECIAL_WIDTHS_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_TYPES_STR, "IDC_STATIC_DRAW_PARAM_SPECIAL_TYPES_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_COLOR_INDEXIES_STR, "IDC_STATIC_DRAW_PARAM_SPECIAL_COLOR_INDEXIES_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_LABEL_SIZES_STR, "IDC_STATIC_DRAW_PARAM_SPECIAL_LABEL_SIZES_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_CLASSES_COUNT_STR, "IDC_STATIC_DRAW_PARAM_SPECIAL_CLASSES_COUNT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HATCH_SETTINGS_STR, "IDC_STATIC_DRAW_PARAM_HATCH_SETTINGS_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_HACTH1, "IDC_CHECK_USE_HACTH1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_HACTH2, "IDC_CHECK_USE_HACTH2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HATCH_LOW_STR1, "IDC_STATIC_DRAW_PARAM_HATCH_LOW_STR1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HATCH_LOW_STR2, "IDC_STATIC_DRAW_PARAM_HATCH_LOW_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HATCH_HIGH_STR1, "IDC_STATIC_DRAW_PARAM_HATCH_HIGH_STR1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HATCH_HIGH_STR2, "IDC_STATIC_DRAW_PARAM_HATCH_HIGH_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HATCH_PATTERN_STR1, "IDC_STATIC_DRAW_PARAM_HATCH_PATTERN_STR1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HATCH_PATTERN_STR2, "IDC_STATIC_DRAW_PARAM_HATCH_PATTERN_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HATCH_COLOR_STR1, "IDC_STATIC_DRAW_PARAM_HATCH_COLOR_STR1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_HATCH_COLOR_STR2, "IDC_STATIC_DRAW_PARAM_HATCH_COLOR_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_LIMITS_STR, "IDC_STATIC_DRAW_PARAM_LIMITS_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_TIME_SERIAL_STR, "IDC_STATIC_DRAW_PARAM_TIME_SERIAL_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_VALUE_STR, "IDC_STATIC_DRAW_PARAM_VALUE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_TIME_SERIAL_LOWLIMIT_STR, "IDC_STATIC_DRAW_PARAM_TIME_SERIAL_LOWLIMIT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_TIME_SERIAL_HIGHLIMIT_STR, "IDC_STATIC_DRAW_PARAM_TIME_SERIAL_HIGHLIMIT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_TIME_SERIAL_MOD_LIMIT_STR, "IDC_STATIC_DRAW_PARAM_TIME_SERIAL_MOD_LIMIT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_TIME_SERIAL_MOD_LIMIT_CHANGE_STR, "IDC_STATIC_DRAW_PARAM_TIME_SERIAL_MOD_LIMIT_CHANGE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_VALUE_LOW_STR, "IDC_STATIC_DRAW_PARAM_VALUE_LOW_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_VALUE_HIGH_STR, "IDC_STATIC_DRAW_PARAM_VALUE_HIGH_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_MODIFY_DRW_PARAM_REFRESH, "IDC_BUTTON_REFRESH");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_DRAW_PARAM_LOAD_FROM, "IDC_DRAW_PARAM_LOAD_FROM");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_MODIFY_DRW_PARAM_USE_WITH_ALL, "IDC_MODIFY_DRW_PARAM_USE_WITH_ALL");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_FIXED_DRAW_PARAM_LABEL, "Fixed DrawParams");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_CONTOUR_ALPHA_STR, "Alpha 5-100");
    
//    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_APPLY_FIXED_DRAW_PARAMS_RIGHT_AWAY, "Apply Fixed Settings At Once");
}

void CFmiModifyDrawParamDlg::OnBnClickedModifyDrwParamRefresh()
{
	UpdateData(TRUE);
	GetSelectedDrawParamSetUp();
	itsOrigDrawParam->Init(itsDrawParam); // otetaan muuutokset ainakin hetkellisesti käyttöön, cancel peruu muutokset!!!
	fRefreshPressed = true;
    ForceStationViewUpdate();

    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, TRUE, TRUE, itsDescTopIndex);
}

void CFmiModifyDrawParamDlg::TakeDrawParamModificationInUse(void)
{
	itsOrigDrawParam->Init(itsDrawParam);
	itsBackupDrawParam->Init(itsDrawParam);
	fRefreshPressed = false; // tämä voidaan nollata, koska nyt muutokset on otettu käyttöön
}

void CFmiModifyDrawParamDlg::OnBnClickedModifyDrwParamUseWithAll()
{
	UpdateData(TRUE);
	OnBnClickedModifyDrwParamRefresh(); // otetaan muutokset myös heti käyttöön, koska ne tulevat käyttöön joka tapauksessa
	TakeDrawParamModificationInUse(); // vain onok:ssa (ja kun asetukset otetaan kaikkialle käyttöön)
									  // initialisoidaan takaisin originaali drawparamiin. cancel ei siirrä muutoksia
    itsSmartMetDocumentInterface->TakeDrawParamInUseEveryWhere(itsDrawParam, fModifyMapViewParam, fModifyMapViewParam, fModifyCrossSectionViewParam, fModifyMapViewParam);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("ModifyDrawParamDlg: Using these setting for all similar parameters button pressed", TRUE, TRUE);
}

void CFmiModifyDrawParamDlg::OnEnChangeSpecialClassesValues()
{
	UpdateData(TRUE);

	fSpecialClassesHaveInvalidValues = false;
	try
	{
		std::string tmp = CT2A(itsSpecialClassValuesStrU_);
		checkedVector<float> classValues = NFmiStringTools::Split<checkedVector<float> >(tmp, ",");
		if(CtrlViewUtils::AreVectorValuesInRisingOrder(classValues) == false)
			throw std::runtime_error("xxx");
	}
	catch(...)
	{
		fSpecialClassesHaveInvalidValues = true; // vain asetetaan tämä lippu päälle jos tulee mitään ongelmia
	}
	CWnd *win = GetDlgItem(IDC_STATIC_DRAW_PARAM_SPECIAL_CLASSES_STR);
	if(win)
		win->Invalidate(FALSE);
}

std::string CFmiModifyDrawParamDlg::GetSelectedFixedDrawParamPath()
{
    UpdateData(TRUE);

    int currentIndex = itsFixedDrawParamSelector.GetCurSel();
    LPITEMDATA currentData = (LPITEMDATA)itsFixedDrawParamSelector.GetItemData(currentIndex);
    if(currentData->cType != itsIconTypeFolderId) // Jos cType ei ole kansio, voidaan tehdä piirtoasetus säätöjä
    {
        CString drawParamFileName;
        itsFixedDrawParamSelector.GetLBText(currentIndex, drawParamFileName);
        CString folderName;
        while(currentData->cLevel > 0)
        {
            CString tmpStr;
            itsFixedDrawParamSelector.GetLBText(currentData->wParantOriginIdx, tmpStr);
            if(folderName.GetLength())
                folderName = tmpStr + _TEXT("\\") + folderName;
            else
                folderName = tmpStr;
            currentData = (LPITEMDATA)itsFixedDrawParamSelector.GetItemData(currentData->wParantOriginIdx);
        }
        CString fixedDrawParamPath = drawParamFileName;
        if(folderName.GetLength())
            fixedDrawParamPath = folderName + _TEXT("\\") + drawParamFileName;
        return std::string(CT2A(fixedDrawParamPath));
    }
    else
        return "";
}

std::shared_ptr<NFmiDrawParam> CFmiModifyDrawParamDlg::GetSelectedFixedDrawParam()
{
    std::string fixedDrawParamPath = GetSelectedFixedDrawParamPath();
    return itsSmartMetDocumentInterface->FixedDrawParamSystem().GetRelativePathDrawParam(fixedDrawParamPath);
}

void CFmiModifyDrawParamDlg::OnCbnSelchangeComboFixedDrawParamSelector()
{
    UpdateData(TRUE);

    std::shared_ptr<NFmiDrawParam> selectedDrawParam = GetSelectedFixedDrawParam();
    if(selectedDrawParam)
    {
        itsDrawParam->Init(selectedDrawParam.get(), true);
        InitDialogFromDrawParam(); // alustetaan dialogin arvot 
        itsOrigDrawParam->Init(itsDrawParam); // otetaan muuutokset ainakin hetkellisesti käyttöön, cancel peruu muutokset!!!
        UpdateData(FALSE);
//        if(fApplyFixedDrawParamSettingsAtOnce)
        {
            fRefreshPressed = true;
            ForceStationViewUpdate();
            itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("ModifyDrawParamDlg: Fixed draw param selected", TRUE, TRUE, itsDescTopIndex); // päivitetään näyttöjä
        }
    }
}

void CFmiModifyDrawParamDlg::OnBnClickedButtonReloadOriginal()
{
    DoOnCancel(); // otetaan originaali asetukset käyttöön, liataan näyttöjä
    itsDrawParam->Init(itsOrigDrawParam); // DoCancel laitta originaalit itsOrigDrawParam:iin, josta ne laitetaan tässä itsDrawParam
    InitDialogFromDrawParam(); // alustetaan dialogin arvot itsDrawParam:in arvoilla
    UpdateData(FALSE);

    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("ModifyDrawParamDlg: Reload original draw param settings", TRUE, TRUE, itsDescTopIndex); // päivitetään näyttöjä
}
