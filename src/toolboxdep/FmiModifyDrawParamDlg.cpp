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
#include "SpecialDesctopIndex.h"
#include "CtrlViewFunctions.h"
#include "ToolMasterColorCube.h"
#include "PERSIST2.H"
#include "CtrlViewGdiPlusFunctions.h"
#include "ApplicationInterface.h"
#include <boost/algorithm/string.hpp>
#include "NFmiSmartToolIntepreter.h"

#include <direct.h> // working directory juttuja varten

const std::string gSpecialClassesSettingsStr = "Special classes settings";
const std::string gUseSpecialClassesStr = "Use special classes";

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
/////////////////////////////////////////////////////////////////////////////
// CFmiModifyDrawParamDlg dialog

CFmiModifyDrawParamDlg::CFmiModifyDrawParamDlg(SmartMetDocumentInterface *smartMetDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const std::string &theDrawParamPath, bool modifyMapViewParam, bool modifyCrossSectionViewParam, unsigned int theDescTopIndex, unsigned int theRealRowNumber, CWnd* pParent)
:CDialog(CFmiModifyDrawParamDlg::IDD, pParent)
,itsDrawParam(new NFmiDrawParam())
,itsBackupDrawParam(new NFmiDrawParam())
,itsOrigDrawParam(theDrawParam)
,fModifyMapViewParam(modifyMapViewParam)
,fModifyCrossSectionViewParam(modifyCrossSectionViewParam)
,itsDrawParamPath(theDrawParamPath)
,itsSymbolFillBitmap(nullptr)
,itsSymbolBitmap(nullptr)
,itsIsoLineLabelBitmap(nullptr)
,itsIsoLineBitmap(nullptr)
,itsIsoLineLabelBoxFillBitmap(nullptr)
,itsSymbolMidBitmap(nullptr)
,itsSymbolLowBitmap(nullptr)
,itsSymbolHighBitmap(nullptr)
,itsSimpleIsoLineLowBitmap(nullptr)
,itsSimpleIsoLineMidBitmap(nullptr)
,itsSimpleIsoLineHighBitmap(nullptr)
,itsSimpleIsoLineHigh2Bitmap(nullptr)
,itsSimpleIsoLineHigh3Bitmap(nullptr)
,itsSimpleColorContourLowBitmap(nullptr)
,itsSimpleColorContourMidBitmap(nullptr)
,itsSimpleColorContourHighBitmap(nullptr)
,itsSimpleColorContourHigh2Bitmap(nullptr)
,itsSimpleColorContourHigh3Bitmap(nullptr)
,itsHatch1Bitmap(nullptr)
,itsHatch2Bitmap(nullptr)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsDescTopIndex(theDescTopIndex)
,itsRealRowNumber(theRealRowNumber)
, itsDrawParamFileNameStrU_(_T(""))
, fUseColorBlendingWithCustomContours(FALSE)
, itsContourGap(0)
,fRefreshPressed(false)
,fSkipReadingSpecialClassColorIndices(false)
, itsAlpha(100)
,fSpecialIsolineClassesHaveInvalidValues(false)
, fSpecialContourClassesHaveInvalidValues(false)
,fFixedDrawParamSelectorInitialized(false)
, itsOneSymbolHorizontalOffset_NEW(0)
, itsOneSymbolVerticalOffset_NEW(0)
, itsIsoLineZeroValue_NEW(0)
, fUseTransparentLabelBoxFillColor(TRUE)
, fDoSparseDataSymbolVisualization(FALSE)
, fUseLegend(FALSE)
, fSimpleContourTransparency1(FALSE)
, fSimpleContourTransparency2(FALSE)
, fSimpleContourTransparency3(FALSE)
, fSimpleContourTransparency4(FALSE)
, fSimpleContourTransparency5(FALSE)
, itsSimpleColorContourLimit1StringU_(_T(""))
, itsSimpleColorContourLimit2StringU_(_T(""))
, itsSimpleColorContourLimit3StringU_(_T(""))
, itsSimpleColorContourLimit4StringU_(_T(""))
, fTreatWmsLayerAsObservation(FALSE)
, itsFixedTextSymbolDrawLength(DefaultFixedTextSymbolDrawLength)
, itsSymbolDrawDensityStr(_T(""))
, itsPossibleColorParameterStr(_T(""))
, fFlipArrowSymbol(FALSE)
, itsSpecialContourClassValuesStrU_(_T(""))
, itsSpecialContourColorIndicesStrU_(_T(""))
, itsSpecialContourClassCount(0)
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
	fUSeMultiColorWithSimpleIsoLines = FALSE;
	fUSeColorBlendWithSimpleIsoLines = FALSE;
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
	itsSimpleIsoLineColorLowValue = 0.0f;
	itsSimpleIsoLineColorLowValueStringU_ = _T("");
	itsSimpleIsoLineColorMidValue = 0.f;
	itsSimpleIsoLineColorMidValueStringU_ = _T("");
	itsSimpleIsoLineColorHighValue = 0.0f;
	itsSimpleIsoLineColorHighValueStringU_ = _T("");
	itsSimpleIsoLineColorHigh2Value = 0.0f;
	itsSimpleIsoLineColorHigh2ValueStringU_ = _T("");
	itsSymbolsWithColorsClassCount = 0;
	itsSymbolsWithColorsEndValue = 0.0f;
	itsSymbolsWithColorsMiddleValue = 0.0f;
	itsSymbolsWithColorsStartValue = 0.0f;
	itsSpecialIsolineClassCount = 0;
    itsSpecialIsolineColorIndicesStrU_ = _T("");
    itsSpecialClassLabelHeightsStrU_ = _T("");
    itsSpecialClassLineStylesStrU_ = _T("");
    itsSpecialClassLineWidthsStrU_ = _T("");
    itsSpecialIsolineClassValuesStrU_ = _T("");
	itsTimeSeriesModifyLimit = 0.0;
	itsTimeSeriesScaleMax = 0.0;
	itsTimeSeriesScaleMin = 0.0;
    itsParamUnitStrU_ = _T("");
	fIsHidden = FALSE;
	fUseHatch2 = FALSE;
	itsHatch2Style = 0.0;
	itsHatch2EndValue = 0.0;
	itsHatch2StartValue = 0.0;
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
	CtrlView::DestroyBitmap(&itsSimpleIsoLineHigh2Bitmap);
	CtrlView::DestroyBitmap(&itsSimpleIsoLineHigh3Bitmap);
    CtrlView::DestroyBitmap(&itsHatch1Bitmap);
    CtrlView::DestroyBitmap(&itsHatch2Bitmap);
    CtrlView::DestroyBitmap(&itsSimpleColorContourMidBitmap);
    CtrlView::DestroyBitmap(&itsSimpleColorContourLowBitmap);
    CtrlView::DestroyBitmap(&itsSimpleColorContourHighBitmap);
	CtrlView::DestroyBitmap(&itsSimpleColorContourHigh2Bitmap);
	CtrlView::DestroyBitmap(&itsSimpleColorContourHigh3Bitmap);
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
	DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_HIGH2, itsSimpleIsoLineHigh2Color);
	DDX_Control(pDX, IDC_BUTTON__HATCH1_COLOR, itsHatch1Color);
	DDX_Check(pDX, IDC_CHECK_SHOW_SYMBOL_CHANGING_COLORS, fUSeChangingColorsWithSymbols);
	DDX_Check(pDX, IDC_CHECK_USE_COLOR_SCALE_WITH_SIMPLE_ISOLINES, fUSeMultiColorWithSimpleIsoLines);
	DDX_Check(pDX, IDC_CHECK_USE_COLOR_BLEND_WITH_SIMPLE_ISOLINES, fUSeColorBlendWithSimpleIsoLines);
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
	DDX_Text(pDX, IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_START_VALUE, itsSimpleIsoLineColorLowValue);
	DDX_Text(pDX, IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_MIDDLE_VALUE_NEW, itsSimpleIsoLineColorMidValue);
	DDX_Text(pDX, IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_HIGH_VALUE, itsSimpleIsoLineColorHighValue);
	DDX_Text(pDX, IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_HIGH2_VALUE, itsSimpleIsoLineColorHigh2Value);
	DDX_Text(pDX, IDC_SHOW_SYMBOL_WITH_COLORS_CLASS_COUNT, itsSymbolsWithColorsClassCount);
	DDX_Text(pDX, IDC_SHOW_SYMBOL_WITH_COLORS_END_VALUE, itsSymbolsWithColorsEndValue);
	DDX_Text(pDX, IDC_SHOW_SYMBOL_WITH_COLORS_MIDDLE_VALUE, itsSymbolsWithColorsMiddleValue);
	DDX_Text(pDX, IDC_SHOW_SYMBOL_WITH_COLORS_START_VALUE, itsSymbolsWithColorsStartValue);
	DDX_Text(pDX, IDC_SPECIAL_ISOLINE_CLASSES_COUNT, itsSpecialIsolineClassCount);
	DDX_Text(pDX, IDC_SPECIAL_CLASSES_ISOLINE_COLOR_INDICES, itsSpecialIsolineColorIndicesStrU_);
	DDX_Text(pDX, IDC_SPECIAL_CLASSES_LINE_LABEL_HEIGHTS, itsSpecialClassLabelHeightsStrU_);
	DDX_Text(pDX, IDC_SPECIAL_CLASSES_LINE_STYLES, itsSpecialClassLineStylesStrU_);
	DDX_Text(pDX, IDC_SPECIAL_CLASSES_LINE_WIDTHS, itsSpecialClassLineWidthsStrU_);
	DDX_Text(pDX, IDC_SPECIAL_ISOLINE_CLASSES_VALUES, itsSpecialIsolineClassValuesStrU_);
	DDX_Text(pDX, IDC_TIME_SERIES_MOD_LIMIT, itsTimeSeriesModifyLimit);
	DDX_Text(pDX, IDC_TIME_SERIES_SCALE_MAX, itsTimeSeriesScaleMax);
	DDX_Text(pDX, IDC_TIME_SERIES_SCALE_MIN, itsTimeSeriesScaleMin);
	DDX_Text(pDX, IDC_UNIT, itsParamUnitStrU_);
	DDX_Check(pDX, IDC_F_HIDDEN, fIsHidden);
	DDX_Check(pDX, IDC_CHECK_USE_HACTH2, fUseHatch2);
	DDX_Text(pDX, IDC_HATCH2_HATCH_STYLE, itsHatch2Style);
	DDX_Text(pDX, IDC_HATCH2_VALUE_END, itsHatch2EndValue);
	DDX_Text(pDX, IDC_HATCH2_VALUE_START, itsHatch2StartValue);
	DDX_Check(pDX, IDC_CHECK_USE_SEPARATOR_LINES_BETWEEN_COLORCONTOUR_CLASSES, fUSeSeparatingLinesBetweenColorContourClasses);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_STATIC_DRAW_PARAM_FILE_NAME, itsDrawParamFileNameStrU_);
	DDX_Control(pDX, IDC_COMBO_DRAW_PARAM_STATION_DATA_VIEW_SELECTOR, itsStationDataViewSelector);
	DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_HIGH2, itsSimpleColorContourHigh2Color);
	DDX_Check(pDX, IDC_CHECK_USE_COLOR_BLENDING_WITH_CUSTOM_CONTOURS, fUseColorBlendingWithCustomContours);
	DDX_Text(pDX, IDC_CONTOUR_GAP, itsContourGap);
	DDX_Text(pDX, IDC_CONTOUR_ALPHA, itsAlpha);
	DDV_MinMaxFloat(pDX, itsAlpha, NFmiDrawParam::itsMinAlpha, 100);
	DDX_Control(pDX, IDC_COMBO_FIXED_DRAW_PARAM_SELECTOR, itsFixedDrawParamSelector);
	DDX_Text(pDX, IDC_ONE_HORIZONTAL_RELATIVE_POS_OFFSET_NEW, itsOneSymbolHorizontalOffset_NEW);
	DDX_Text(pDX, IDC_ONE_VERTICAL_RELATIVE_POS_OFFSET_NEW, itsOneSymbolVerticalOffset_NEW);
	DDX_Text(pDX, IDC_ISOLINE_ZERO_VALUE_POINT_NEW, itsIsoLineZeroValue_NEW);
	DDX_Check(pDX, IDC_CHECK_DRAW_PARAM_USE_TRANSPARENT_LABEL_FILL_COLOR, fUseTransparentLabelBoxFillColor);
	DDX_Check(pDX, IDC_CHECK_DO_SPARSE_SYMBOL_VISUALIZATION, fDoSparseDataSymbolVisualization);
	DDX_Check(pDX, IDC_CHECK_DRAW_PARAM_USE_LEGEND, fUseLegend);
	DDX_Check(pDX, IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY1, fSimpleContourTransparency1);
	DDX_Check(pDX, IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY2, fSimpleContourTransparency2);
	DDX_Check(pDX, IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY3, fSimpleContourTransparency3);
	DDX_Check(pDX, IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY4, fSimpleContourTransparency4);
	DDX_Check(pDX, IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY5, fSimpleContourTransparency5);
	DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_HIGH3, itsSimpleColorContourHigh3Color);
	DDX_Text(pDX, IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_START_VALUE, itsSimpleColorContourLimit1StringU_);
	DDX_Text(pDX, IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_MIDDLE_VALUE, itsSimpleColorContourLimit2StringU_);
	DDX_Text(pDX, IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_END_VALUE, itsSimpleColorContourLimit3StringU_);
	DDX_Text(pDX, IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_END2_VALUE, itsSimpleColorContourLimit4StringU_);
	DDX_Check(pDX, IDC_TREAT_WMS_LAYER_AS_OBSERVATION, fTreatWmsLayerAsObservation);
	DDX_Control(pDX, IDC_SLIDER_SYMBOL_DRAW_DENSITY_X, itsSymbolDrawDensityXSlider);
	DDX_Control(pDX, IDC_SLIDER_SYMBOL_DRAW_DENSITY_Y, itsSymbolDrawDensityYSlider);
	DDX_Text(pDX, IDC_FIXED_TEXT_SYMBOL_DRAW_LENGTH, itsFixedTextSymbolDrawLength);
	DDX_Text(pDX, IDC_STATIC_SYMBOL_DRAW_DENSITY_STR, itsSymbolDrawDensityStr);
	DDX_Text(pDX, IDC_EDIT_DRAW_PARAM_COLOR_PARAM_STR, itsPossibleColorParameterStr);
	DDX_Control(pDX, IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_HIGH3, itsSimpleIsoLineHigh3Color);
	DDX_Check(pDX, IDC_CHECK_FLIP_ARROW_SYMBOL, fFlipArrowSymbol);
	DDX_Text(pDX, IDC_SPECIAL_CONTOUR_CLASSES_VALUES, itsSpecialContourClassValuesStrU_);
	DDX_Text(pDX, IDC_SPECIAL_CLASSES_CONTOUR_COLOR_INDICES, itsSpecialContourColorIndicesStrU_);
	DDX_Text(pDX, IDC_SPECIAL_CONTOUR_CLASSES_COUNT, itsSpecialContourClassCount);
}


BEGIN_MESSAGE_MAP(CFmiModifyDrawParamDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiModifyDrawParamDlg)
	ON_BN_CLICKED(IDC_BUTTON__HATCH1_COLOR, OnButtonHatch1Color)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_LOW, OnButtonColorShowSimpleIsolineLow)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_MID, OnButtonColorShowSimpleIsolineMid)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_HIGH, OnButtonColorShowSimpleIsolineHigh)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_HIGH2, OnButtonColorShowSimpleIsolineHigh2)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_ISOLINE_HIGH3, OnButtonColorShowSimpleIsolineHigh3)
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
	ON_BN_CLICKED(IDC_SHOW_ISOLINE_COLOR_INDEX_DLG, OnBnClickedShowIsolineColorIndexDlg)
	ON_BN_CLICKED(IDC_BUTTON__HATCH2_COLOR, OnButtonHatch2Color)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_LOW, OnButtonColorShowSimpleColorcontourLow)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_MID, OnButtonColorShowSimpleColorcontourMid)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_HIGH, OnButtonColorShowSimpleColorcontourHigh)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_HIGH2, OnButtonColorShowSimpleColorcontourHigh2)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_SHOW_SIMPLE_COLORCONTOUR_HIGH3, OnButtonColorShowSimpleColorcontourHigh3)
	ON_BN_CLICKED(IDC_BUTTON_RESET_DRAW_PARAM, OnButtonResetDrawParam)
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_MODIFY_DRW_PARAM_REFRESH, OnBnClickedModifyDrwParamRefresh)
	ON_BN_CLICKED(IDC_DRAW_PARAM_LOAD_FROM, OnBnClickedDrawParamLoadFrom)
	ON_BN_CLICKED(IDC_MODIFY_DRW_PARAM_USE_WITH_ALL, OnBnClickedModifyDrwParamUseWithAll)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_SPECIAL_ISOLINE_CLASSES_VALUES, &CFmiModifyDrawParamDlg::OnEnChangeSpecialIsolineClassesValues)
    ON_CBN_SELCHANGE(IDC_COMBO_FIXED_DRAW_PARAM_SELECTOR, &CFmiModifyDrawParamDlg::OnCbnSelchangeComboFixedDrawParamSelector)
    ON_BN_CLICKED(IDC_BUTTON_RELOAD_ORIGINAL, &CFmiModifyDrawParamDlg::OnBnClickedButtonReloadOriginal)
	ON_BN_CLICKED(IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY1, &CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency1)
	ON_BN_CLICKED(IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY2, &CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency2)
	ON_BN_CLICKED(IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY3, &CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency3)
	ON_BN_CLICKED(IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY4, &CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency4)
	ON_BN_CLICKED(IDC_CHECK_SIMPLE_CONTOUR_TRANSPARENCY5, &CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency5)
	ON_EN_CHANGE(IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_START_VALUE, &CFmiModifyDrawParamDlg::OnEnChangeShowSimpleColorcontourWithColorsStartValue)
	ON_EN_CHANGE(IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_MIDDLE_VALUE, &CFmiModifyDrawParamDlg::OnEnChangeShowSimpleColorcontourWithColorsMiddleValue)
	ON_EN_CHANGE(IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_END_VALUE, &CFmiModifyDrawParamDlg::OnEnChangeShowSimpleColorcontourWithColorsEndValue)
	ON_EN_CHANGE(IDC_SHOW_SIMPLE_COLORCONTOUR_WITH_COLORS_END2_VALUE, &CFmiModifyDrawParamDlg::OnEnChangeShowSimpleColorcontourWithColorsEnd2Value)
	ON_EN_CHANGE(IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_START_VALUE, &CFmiModifyDrawParamDlg::OnEnChangeShowSimpleIsolineWithColorsStartValue)
	ON_EN_CHANGE(IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_MIDDLE_VALUE_NEW, &CFmiModifyDrawParamDlg::OnEnChangeShowSimpleIsolineWithColorsMiddleValue)
	ON_EN_CHANGE(IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_HIGH_VALUE, &CFmiModifyDrawParamDlg::OnEnChangeShowSimpleIsolineWithColorsEndValue)
	ON_EN_CHANGE(IDC_SHOW_SIMPLE_ISOLINE_WITH_COLORS_HIGH2_VALUE, &CFmiModifyDrawParamDlg::OnEnChangeShowSimpleIsolineWithColorsEnd2Value)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT_DRAW_PARAM_COLOR_PARAM_STR, &CFmiModifyDrawParamDlg::OnEnChangeEditDrawParamColorParamStr)
	ON_EN_CHANGE(IDC_SPECIAL_CONTOUR_CLASSES_VALUES, &CFmiModifyDrawParamDlg::OnEnChangeSpecialContourClassesValues)
	ON_BN_CLICKED(IDC_SHOW_CONTOUR_COLOR_INDEX_DLG, &CFmiModifyDrawParamDlg::OnBnClickedShowContourColorIndexDlg)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiModifyDrawParamDlg message handlers

BOOL CFmiModifyDrawParamDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogFromDrawParam();

    if(IsMacroParamCase()) // jos macroParam kyseess‰, ei lyhennett‰ voi antaa editoida, koska se on tunniste Metkun editorille
	{
		CWnd *win = GetDlgItem(IDC_PARAM_ABBREVIATION);
		if(win)
			win->EnableWindow(FALSE);
	}

    AdjustStartingPosition();
	UpdateSymbolDrawDensityStr();
	InitTooltipControl();

	UpdateData(FALSE);
	DoPostInitializationChecks();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

bool CFmiModifyDrawParamDlg::IsMacroParamCase()
{
	return NFmiDrawParam::IsMacroParamCase(itsDrawParam->DataType());
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
    // Laitetaan ensin dropdown-puulistaan t‰m‰n hakemiston alihakemistot (rekursiivisesti)
    const std::vector<NFmiFixedDrawParamFolder> &subFolders = theFolder.SubFolders();
    for(auto subFolder : subFolders)
    {
        int subTreeIndex = theFixedDrawParamSelector.AddCTString(theTreeIndex, itsIconTypeFolderId, CA2T(subFolder.SubFolderName().c_str()));
        AddFixedDrawParamFolderToSelector(subTreeIndex, subFolder, theFixedDrawParamSelector);
    }

    // Laitetaan lopuksi dropdown-puulistaan t‰m‰n hakemiston drawParamit
    const std::vector<std::shared_ptr<NFmiDrawParam>> &drawParams = theFolder.DrawParams();
    for(auto drawParam : drawParams)
    {
        std::string listText = ::GetFileName(drawParam->InitFileName());
        theFixedDrawParamSelector.AddCTString(theTreeIndex, itsIconTypeFileId, CA2T(listText.c_str()));
    }
}

// HUOM! N‰m‰ ikoni id:t on m‰‰ritelty oikeasti SmartMet projektin recource.h:ssa. 
// Halusin irroittaa ToolBoxDep kirjaston riippuvuuden sinne, ja yritin lis‰t‰ ikonit t‰h‰n kirjastoon
// ja ladata ikoneja t‰‰lt‰ k‰sin. T‰m‰ ei onnistunut, joten minun pit‰‰ ladata ikonit SmartMet ohjelmasta
// sen projektin oikeilla id:ll‰.
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
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->FillColor(), itsSymbolFillColorRef, &itsSymbolFillBitmap, itsSymbolFillColorRect, itsSymbolFillColor));
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->FrameColor(), itsSymbolColorRef, &itsSymbolBitmap, itsSymbolColorRect, itsSymbolColor));
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->IsolineTextColor(), itsIsoLineLabelColorRef, &itsIsoLineLabelBitmap, itsIsoLineLabelColorRect, itsIsoLineLabelColor));
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->IsolineColor(), itsIsoLineColorRef, &itsIsoLineBitmap, itsIsoLineColorRect, itsIsoLineColor));
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->IsolineLabelBoxFillColor(), itsIsoLineLabelBoxFillColorRef, &itsIsoLineLabelBoxFillBitmap, itsIsoLineLabelBoxFillColorRect, itsIsolineLabelBoxFillColor));
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->StationSymbolColorShadeMidValueColor(), itsSymbolMidColorRef, &itsSymbolMidBitmap, itsSymbolMidColorRect, itsSymbolMidColor));
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->StationSymbolColorShadeLowValueColor(), itsSymbolLowColorRef, &itsSymbolLowBitmap, itsSymbolLowColorRect, itsSymbolLowColor));
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->StationSymbolColorShadeHighValueColor(), itsSymbolHighColorRef, &itsSymbolHighBitmap, itsSymbolHighColorRect, itsSymbolHighColor));
	CtrlView::InitialButtonColorUpdate(GetSimpleIsolineColorButtonData(1, true));
	CtrlView::InitialButtonColorUpdate(GetSimpleIsolineColorButtonData(2, true));
	CtrlView::InitialButtonColorUpdate(GetSimpleIsolineColorButtonData(3, true));
	CtrlView::InitialButtonColorUpdate(GetSimpleIsolineColorButtonData(4, true));
	CtrlView::InitialButtonColorUpdate(GetSimpleIsolineColorButtonData(5, true));
	CtrlView::InitialButtonColorUpdate(GetSimpleContourColorButtonData(1, true));
	CtrlView::InitialButtonColorUpdate(GetSimpleContourColorButtonData(2, true));
	CtrlView::InitialButtonColorUpdate(GetSimpleContourColorButtonData(3, true));
	CtrlView::InitialButtonColorUpdate(GetSimpleContourColorButtonData(4, true));
	CtrlView::InitialButtonColorUpdate(GetSimpleContourColorButtonData(5, true));
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->IsoLineHatchColor1(), itsHatch1ColorRef, &itsHatch1Bitmap, itsHatch1ColorRect, itsHatch1Color));
    CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsDrawParam->IsoLineHatchColor2(), itsHatch2ColorRef, &itsHatch2Bitmap, itsHatch2ColorRect, itsHatch2Color));
}

// itsStationDataViewSelector -combobox on t‰ytetty legacy syist‰ seuraavasti:
// 1. ComboBoxin indeksit alkavat 0:sta
// 2. ComboBoxin indeksit 0-6 ovat NFmiMetEditorTypes::View:in arvot 1-7, t‰st‰ johtuen niihin joko lis‰t‰‰n tai v‰hennet‰‰n 1 kun muunnoksia tehd‰‰n (riippuen muunnos suunasta).
// 3. ComboBoxin indeksit 7-n ovat NFmiMetEditorTypes::View:in arvot 11-m, t‰st‰ johtuen niihin joko lis‰t‰‰n tai v‰hennet‰‰n 4 kun muunnoksia tehd‰‰n.
const int g_ViewTypeOffset1 = 1;
const int g_ViewTypeOffset2 = 4;
static int GetStationDataViewSelectorIndex(NFmiMetEditorTypes::View theViewType)
{
	if(theViewType >= NFmiMetEditorTypes::View::kFmiTextView && (theViewType < NFmiMetEditorTypes::View::kFmiFontTextView || theViewType > NFmiMetEditorTypes::View::kFmiParamsDefaultView))
	{
		if(theViewType <= NFmiMetEditorTypes::View::kFmiWindVectorView)
			return static_cast<int>(theViewType) - g_ViewTypeOffset1;
		else if(theViewType <= NFmiMetEditorTypes::View::kFmiCustomSymbolView)
			return static_cast<int>(theViewType) - g_ViewTypeOffset2;
	}

	return -1;
}

static NFmiMetEditorTypes::View GetSelectedStationDataViewType(CComboBox &theStationDataViewSelector)
{
    auto currentSelection = theStationDataViewSelector.GetCurSel();
    if(currentSelection >= (static_cast<int>(NFmiMetEditorTypes::View::kFmiTextView) - g_ViewTypeOffset1) && currentSelection <= (static_cast<int>(NFmiMetEditorTypes::View::kFmiWindVectorView) - g_ViewTypeOffset1))
        return static_cast<NFmiMetEditorTypes::View>(theStationDataViewSelector.GetCurSel() + g_ViewTypeOffset1);
    else
        return static_cast<NFmiMetEditorTypes::View>(theStationDataViewSelector.GetCurSel() + g_ViewTypeOffset2);
}

void CFmiModifyDrawParamDlg::FillStationDataViewSelector(void)
{
	itsStationDataViewSelector.ResetContent();
	itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE").c_str())); // n‰iden 7 ensimm‰isen arvot menev‰t 1-7, joten niille tehd‰‰n -1 ja +1 operaatiot
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE2").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE3").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE4").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE5").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE6").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("IDC_RADIO_GRID_DATA_DRAW_STYLE7").c_str()));
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("Precipitation Form").c_str())); // Huom! t‰m‰n arvo on oikeasti 11, joten pit‰‰ tehd‰ virityksi‰
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("Synop weather symbol").c_str())); // Huom! t‰m‰n arvo on oikeasti 12, joten pit‰‰ tehd‰ virityksi‰
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("Raw Mirri font symbol").c_str())); // Huom! t‰m‰n arvo on oikeasti 13, joten pit‰‰ tehd‰ virityksi‰
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("Better weather symbol").c_str())); // Huom! t‰m‰n arvo on oikeasti 14, joten pit‰‰ tehd‰ virityksi‰
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("SmartSymbol").c_str())); // Huom! t‰m‰n arvo on oikeasti 15, joten pit‰‰ tehd‰ virityksi‰
    itsStationDataViewSelector.AddString(CA2T(::GetDictionaryString("CustomSymbol").c_str())); // Huom! t‰m‰n arvo on oikeasti 16, joten pit‰‰ tehd‰ virityksi‰
    itsStationDataViewSelector.SetCurSel(::GetStationDataViewSelectorIndex(itsDrawParam->StationDataViewType()));

	for(int viewType = 0; viewType < 18; viewType++)
	{
		auto selectorIndex = ::GetStationDataViewSelectorIndex(static_cast<NFmiMetEditorTypes::View>(viewType));
		std::string resultStr = "ViewType " + std::to_string(viewType) + " produced selectorIndex " + std::to_string(selectorIndex);
		CatLog::logMessage(resultStr, CatLog::Severity::Info, CatLog::Category::Operational);
	}
}

void CFmiModifyDrawParamDlg::InitRestOfVersion2Data(void)
{
	fUSeChangingColorsWithSymbols = itsDrawParam->ShowColoredNumbers();
	fUSeMultiColorWithSimpleIsoLines = !itsDrawParam->UseSingleColorsWithSimpleIsoLines();
	fUSeColorBlendWithSimpleIsoLines = itsDrawParam->DoIsoLineColorBlend();
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
	itsGridDataDrawStyle = static_cast<int>(itsDrawParam->GridDataPresentationStyle()) - 1;

	SetSimpleColorContourLimit(itsDrawParam->SimpleIsoLineColorShadeLowValue(), &itsSimpleIsoLineColorLowValue, &itsSimpleIsoLineColorLowValueStringU_);
	SetSimpleColorContourLimit(itsDrawParam->SimpleIsoLineColorShadeMidValue(), &itsSimpleIsoLineColorMidValue, &itsSimpleIsoLineColorMidValueStringU_);
	SetSimpleColorContourLimit(itsDrawParam->SimpleIsoLineColorShadeHighValue(), &itsSimpleIsoLineColorHighValue, &itsSimpleIsoLineColorHighValueStringU_);
	SetSimpleColorContourLimit(itsDrawParam->SimpleIsoLineColorShadeHigh2Value(), &itsSimpleIsoLineColorHigh2Value, &itsSimpleIsoLineColorHigh2ValueStringU_);
	itsSymbolsWithColorsClassCount = itsDrawParam->StationSymbolColorShadeClassCount();
	itsSymbolsWithColorsEndValue = itsDrawParam->StationSymbolColorShadeHighValue();
	itsSymbolsWithColorsMiddleValue = itsDrawParam->StationSymbolColorShadeMidValue();
	itsSymbolsWithColorsStartValue = itsDrawParam->StationSymbolColorShadeLowValue();
	SetSimpleColorContourLimit(itsDrawParam->ColorContouringColorShadeLowValue(), &itsSimpleColorContourLimit1Value, &itsSimpleColorContourLimit1StringU_);
	SetSimpleColorContourLimit(itsDrawParam->ColorContouringColorShadeMidValue(), &itsSimpleColorContourLimit2Value, &itsSimpleColorContourLimit2StringU_);
	SetSimpleColorContourLimit(itsDrawParam->ColorContouringColorShadeHighValue(), &itsSimpleColorContourLimit3Value, &itsSimpleColorContourLimit3StringU_);
	SetSimpleColorContourLimit(itsDrawParam->ColorContouringColorShadeHigh2Value(), &itsSimpleColorContourLimit4Value, &itsSimpleColorContourLimit4StringU_);
	fSimpleContourTransparency1 = itsDrawParam->SimpleColorContourTransparentColor1();
	fSimpleContourTransparency2 = itsDrawParam->SimpleColorContourTransparentColor2();
	fSimpleContourTransparency3 = itsDrawParam->SimpleColorContourTransparentColor3();
	fSimpleContourTransparency4 = itsDrawParam->SimpleColorContourTransparentColor4();
	fSimpleContourTransparency5 = itsDrawParam->SimpleColorContourTransparentColor5();
	fUseHatch2 = itsDrawParam->UseWithIsoLineHatch2() == TRUE;
	itsHatch2Style = itsDrawParam->IsoLineHatchType2();
	itsHatch2EndValue = itsDrawParam->IsoLineHatchHighValue2();
	itsHatch2StartValue = itsDrawParam->IsoLineHatchLowValue2();
	// Luokan dataosan fUseColorBlendingWithCustomContours nimi on oikea 
	// k‰yttˆtarkoitus kyseiselle asetukselle (EI DrawParamin UseIsoLineGabWithCustomContours)
	fUseColorBlendingWithCustomContours = itsDrawParam->UseIsoLineGabWithCustomContours();
    fUseTransparentLabelBoxFillColor = itsDrawParam->UseTransparentFillColor();
    fDoSparseDataSymbolVisualization = itsDrawParam->DoSparseSymbolVisualization();
	fUseLegend = itsDrawParam->ShowColorLegend();
	fTreatWmsLayerAsObservation = itsDrawParam->TreatWmsLayerAsObservation();

    FillStationDataViewSelector();
	InitSpecialClassesData();

	itsFixedTextSymbolDrawLength = itsDrawParam->FixedTextSymbolDrawLength();
	InitSymbolDrawDensitySliders();
	itsPossibleColorParameterStr = CA2T(itsDrawParam->PossibleColorValueParameter().c_str());
	fFlipArrowSymbol = itsDrawParam->FlipArrowSymbol();
}

static int SymbolDrawDensityToSliderPos(double symbolDrawDensity)
{
	int sliderPos = boost::math::iround((symbolDrawDensity - DrawParamMinSymbolDrawDensity) * 10.);
	return sliderPos;
}

static double SliderPosToSymbolDrawDensity(int sliderPos)
{
	double symbolDrawDensity = (sliderPos / 10.) + DrawParamMinSymbolDrawDensity;
	return symbolDrawDensity;
}

void CFmiModifyDrawParamDlg::InitSymbolDrawDensitySliders()
{
	// Arvot 0.4 - 2.0 yhden desimaalin tarkkuudella
	int startIndex = 0;
	int endIndex = boost::math::iround((DrawParamMaxSymbolDrawDensity - DrawParamMinSymbolDrawDensity) * 10.);
	itsSymbolDrawDensityXSlider.SetRange(startIndex, endIndex);
	itsSymbolDrawDensityXSlider.SetPos(::SymbolDrawDensityToSliderPos(itsDrawParam->SymbolDrawDensityX()));
	itsSymbolDrawDensityYSlider.SetRange(startIndex, endIndex);
	itsSymbolDrawDensityYSlider.SetPos(::SymbolDrawDensityToSliderPos(itsDrawParam->SymbolDrawDensityY()));
}

static std::string GetPrettyLimitValue(float limitValue)
{
	// to_string laittaa vimmatusti desimaaleja '0':ia stringin per‰‰n
	auto limitString = std::to_string(limitValue);
	// Poistetaan lopusta kaikki turhat 0:t pois
	boost::trim_right_if(limitString, [](auto character) {return character == '0'; });
	// Poistetaan viel‰ mahdollinen lopun turha desimaalipiste
	if(limitString.back() == '.')
		limitString.pop_back();
	return limitString;
}

void CFmiModifyDrawParamDlg::SetSimpleColorContourLimit(float limitValue, float *limitValueDlg, CString *limitStringDlgU_)
{
	*limitValueDlg = limitValue;
	if(limitValue == kFloatMissing)
		*limitStringDlgU_ = _TEXT("#");
	else
	{
		auto limitValueStdString = ::GetPrettyLimitValue(limitValue);
		//int usedDecimals = 2;
		//if(limitValue == int(limitValue))
		//	usedDecimals = 0;
		//std::string limitValueStdString = NFmiValueString::GetStringWithMaxDecimalsSmartWay(limitValue, usedDecimals);
		*limitStringDlgU_ = CA2T(limitValueStdString.c_str());
	}
}

template<typename Container>
static std::string MakeCommaSeparatedValuesString(const Container& values, int usedDecimals)
{
	std::string str;
	for(auto value : values)
	{
		if(!str.empty())
		{
			str += ", ";
		}
		if(usedDecimals > 0)
			str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, 4);
		else
			str += std::to_string(value);
	}
	return str;
}

void CFmiModifyDrawParamDlg::InitSpecialClassesData(void)
{
	// Isoviivojen asetuksiin liittyv‰t jutut
	const auto &specialIsoLineValues = itsDrawParam->SpecialIsoLineValues();
	itsSpecialIsolineClassCount = static_cast<int>(specialIsoLineValues.size());
	auto str = ::MakeCommaSeparatedValuesString(specialIsoLineValues, 4);
    itsSpecialIsolineClassValuesStrU_ = CA2T(str.c_str());

	str = ::MakeCommaSeparatedValuesString(itsDrawParam->SpecialIsoLineColorIndexies(), 0);
	itsSpecialIsolineColorIndicesStrU_ = CA2T(str.c_str());

	str = ::MakeCommaSeparatedValuesString(itsDrawParam->SpecialIsoLineStyle(), 0);
    itsSpecialClassLineStylesStrU_ = CA2T(str.c_str());

	str = ::MakeCommaSeparatedValuesString(itsDrawParam->SpecialIsoLineWidth(), 4);
    itsSpecialClassLineWidthsStrU_ = CA2T(str.c_str());

	str = ::MakeCommaSeparatedValuesString(itsDrawParam->SpecialIsoLineLabelHeight(), 4);
    itsSpecialClassLabelHeightsStrU_ = CA2T(str.c_str());

	// Contour asetuksiin liittyv‰t jutut
	std::vector<float> specialContourValues = itsDrawParam->SpecialContourValues();
	itsSpecialContourClassCount = static_cast<int>(specialContourValues.size());
	str = ::MakeCommaSeparatedValuesString(specialContourValues, 4);
	itsSpecialContourClassValuesStrU_ = CA2T(str.c_str());

	str = ::MakeCommaSeparatedValuesString(itsDrawParam->SpecialContourColorIndexies(), 0);
	itsSpecialContourColorIndicesStrU_ = CA2T(str.c_str());
}



void CFmiModifyDrawParamDlg::ReadRestOfVersion2Data(void)
{
	itsDrawParam->ShowColoredNumbers(fUSeChangingColorsWithSymbols != 0);
	itsDrawParam->UseSingleColorsWithSimpleIsoLines(!fUSeMultiColorWithSimpleIsoLines);
	itsDrawParam->DoIsoLineColorBlend(fUSeColorBlendWithSimpleIsoLines);
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
	if(fSkipReadingSpecialClassColorIndices == false) // n‰m‰ on siis jo p‰ivitetty TMColorIndex-disalogissa, ei saa p‰ivitt‰‰ niit‰ arvoja p‰‰lle
	{
		itsDrawParam->ContourGab(itsContourGap);
	}
	itsDrawParam->IsoLineLabelDigitCount(itsIsoLineDecimals);
	itsDrawParam->SimpleIsoLineLabelHeight(static_cast<float>(itsIsoLineLabelHeight));
	itsDrawParam->IsoLineSplineSmoothingFactor(itsIsoLineSmoothFactor);
	itsDrawParam->SimpleIsoLineLineStyle(itsIsoLineStyle);
	itsDrawParam->SimpleIsoLineWidth(itsIsoLineWidth);
	itsDrawParam->SimpleIsoLineZeroValue(static_cast<float>(itsIsoLineZeroValue_NEW));
	itsDrawParam->GridDataPresentationStyle(static_cast<NFmiMetEditorTypes::View>(itsGridDataDrawStyle+1));

	itsDrawParam->SimpleIsoLineColorShadeLowValue(itsSimpleIsoLineColorLowValue);
	itsDrawParam->SimpleIsoLineColorShadeMidValue(itsSimpleIsoLineColorMidValue);
	itsDrawParam->SimpleIsoLineColorShadeHighValue(itsSimpleIsoLineColorHighValue);
	itsDrawParam->SimpleIsoLineColorShadeHigh2Value(itsSimpleIsoLineColorHigh2Value);
	itsDrawParam->StationSymbolColorShadeClassCount(itsSymbolsWithColorsClassCount);
	itsDrawParam->StationSymbolColorShadeHighValue(itsSymbolsWithColorsEndValue);
	itsDrawParam->StationSymbolColorShadeMidValue(itsSymbolsWithColorsMiddleValue);
	itsDrawParam->StationSymbolColorShadeLowValue(itsSymbolsWithColorsStartValue);

	itsDrawParam->ColorContouringColorShadeLowValue(itsSimpleColorContourLimit1Value);
	itsDrawParam->ColorContouringColorShadeMidValue(itsSimpleColorContourLimit2Value);
	itsDrawParam->ColorContouringColorShadeHighValue(itsSimpleColorContourLimit3Value);
	itsDrawParam->ColorContouringColorShadeHigh2Value(itsSimpleColorContourLimit4Value);
	itsDrawParam->SimpleColorContourTransparentColor1(fSimpleContourTransparency1);
	itsDrawParam->SimpleColorContourTransparentColor2(fSimpleContourTransparency2);
	itsDrawParam->SimpleColorContourTransparentColor3(fSimpleContourTransparency3);
	itsDrawParam->SimpleColorContourTransparentColor4(fSimpleContourTransparency4);
	itsDrawParam->SimpleColorContourTransparentColor5(fSimpleContourTransparency5);
	itsDrawParam->UseWithIsoLineHatch2(fUseHatch2 == TRUE);
	itsDrawParam->IsoLineHatchType2(static_cast<int>(itsHatch2Style));
	itsDrawParam->IsoLineHatchHighValue2(static_cast<float>(itsHatch2EndValue));
	itsDrawParam->IsoLineHatchLowValue2(static_cast<float>(itsHatch2StartValue));
	if(fSkipReadingSpecialClassColorIndices == false) // n‰m‰ on siis jo p‰ivitetty TMColorIndex-dialogissa, ei saa p‰ivitt‰‰ niit‰ arvoja p‰‰lle
	{
		// Luokan dataosan fUseColorBlendingWithCustomContours nimi on oikea 
		// k‰yttˆtarkoitus kyseiselle asetukselle (EI DrawParamin UseIsoLineGabWithCustomContours)
		itsDrawParam->UseIsoLineGabWithCustomContours(fUseColorBlendingWithCustomContours == TRUE);
	}
	itsDrawParam->StationDataViewType(::GetSelectedStationDataViewType(itsStationDataViewSelector));
    itsDrawParam->UseTransparentFillColor(fUseTransparentLabelBoxFillColor == TRUE);
    itsDrawParam->DoSparseSymbolVisualization(fDoSparseDataSymbolVisualization == TRUE);
	itsDrawParam->ShowColorLegend(fUseLegend == TRUE);
	itsDrawParam->TreatWmsLayerAsObservation(fTreatWmsLayerAsObservation == TRUE);
	itsDrawParam->FixedTextSymbolDrawLength(itsFixedTextSymbolDrawLength);
	auto densityValues = GetSymbolDrawDensityValuesFromSlider();
	itsDrawParam->SymbolDrawDensityX(densityValues.first);
	itsDrawParam->SymbolDrawDensityY(densityValues.second);
	itsDrawParam->PossibleColorValueParameter(std::string(CT2A(itsPossibleColorParameterStr)));
	itsDrawParam->FlipArrowSymbol(fFlipArrowSymbol == TRUE);

	ReadSpecialClassesData();
}

void CFmiModifyDrawParamDlg::ReadSpecialClassesData(void)
{
	std::string problemVariableStr;
	std::string currentValueStr;
	try
	{
		if(fSkipReadingSpecialClassColorIndices == false) // n‰m‰ on siis jo p‰ivitetty TMColorIndex-disalogissa, ei saa p‰ivitt‰‰ niit‰ arvoja p‰‰lle
		{
			problemVariableStr = "Problem with SpecialIsolineClassValues string\n";
			currentValueStr = CT2A(itsSpecialIsolineClassValuesStrU_);
			itsDrawParam->SetSpecialIsoLineValues(NFmiStringTools::Split<std::vector<float> >(currentValueStr, ","));

			problemVariableStr = "Problem with SpecialIsolineClassColorIndices string\n";
            currentValueStr = CT2A(itsSpecialIsolineColorIndicesStrU_);
			itsDrawParam->SetSpecialIsoLineColorIndexies(NFmiStringTools::Split<std::vector<int> >(currentValueStr, ","));

			problemVariableStr = "Problem with SpecialContourClassValues string\n";
			currentValueStr = CT2A(itsSpecialContourClassValuesStrU_);
			itsDrawParam->SetSpecialContourValues(NFmiStringTools::Split<std::vector<float> >(currentValueStr, ","));

			problemVariableStr = "Problem with SpecialContourClassColorIndices string\n";
			currentValueStr = CT2A(itsSpecialContourColorIndicesStrU_);
			itsDrawParam->SetSpecialContourColorIndexies(NFmiStringTools::Split<std::vector<int> >(currentValueStr, ","));
		}

		problemVariableStr = "Problem with SpecialClassLineStyles string\n";
        currentValueStr = CT2A(itsSpecialClassLineStylesStrU_);
		itsDrawParam->SetSpecialIsoLineStyle(NFmiStringTools::Split<std::vector<int> >(currentValueStr, ","));

		problemVariableStr = "Problem with SpecialClassLineWidth string\n";
        currentValueStr = CT2A(itsSpecialClassLineWidthsStrU_);
		itsDrawParam->SetSpecialIsoLineWidth(NFmiStringTools::Split<std::vector<float> >(currentValueStr, ","));

		problemVariableStr = "Problem with SpecialIsoLineLabelHeights string\n";
        currentValueStr = CT2A(itsSpecialClassLabelHeightsStrU_);
		itsDrawParam->SetSpecialIsoLineLabelHeight(NFmiStringTools::Split<std::vector<float> >(currentValueStr, ","));
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
	return;
}

void CFmiModifyDrawParamDlg::OnButtonHatch1Color()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsHatch1ColorRef, &itsHatch1Bitmap, itsHatch1ColorRect, itsHatch1Color));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleIsolineLow()
{
	UpdateData(TRUE);
	CtrlView::ColorButtonPressed(GetSimpleIsolineColorButtonData(1, false));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleIsolineMid()
{
	UpdateData(TRUE);
	CtrlView::ColorButtonPressed(GetSimpleIsolineColorButtonData(2, false));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleIsolineHigh()
{
	UpdateData(TRUE);
	CtrlView::ColorButtonPressed(GetSimpleIsolineColorButtonData(3, false));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleIsolineHigh2()
{
	UpdateData(TRUE);
	CtrlView::ColorButtonPressed(GetSimpleIsolineColorButtonData(4, false));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleIsolineHigh3()
{
	UpdateData(TRUE);
	CtrlView::ColorButtonPressed(GetSimpleIsolineColorButtonData(5, false));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSymbHigh()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsSymbolHighColorRef, &itsSymbolHighBitmap, itsSymbolHighColorRect, itsSymbolHighColor));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSymbLow()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsSymbolLowColorRef, &itsSymbolLowBitmap, itsSymbolLowColorRect, itsSymbolLowColor));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSymbMid()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsSymbolMidColorRef, &itsSymbolMidBitmap, itsSymbolMidColorRect, itsSymbolMidColor));
}

void CFmiModifyDrawParamDlg::OnButtonIsolineColor()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsIsoLineColorRef, &itsIsoLineBitmap, itsIsoLineColorRect, itsIsoLineColor));
}

void CFmiModifyDrawParamDlg::OnBnClickedButtonIsolineLabelBoxFillColor()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsIsoLineLabelBoxFillColorRef, &itsIsoLineLabelBoxFillBitmap, itsIsoLineLabelBoxFillColorRect, itsIsolineLabelBoxFillColor));
}

void CFmiModifyDrawParamDlg::OnButtonIsolineLabelColor()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsIsoLineLabelColorRef, &itsIsoLineLabelBitmap, itsIsoLineLabelColorRect, itsIsoLineLabelColor));
}

void CFmiModifyDrawParamDlg::OnButtonSymbolColor()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsSymbolColorRef, &itsSymbolBitmap, itsSymbolColorRect, itsSymbolColor));
}

void CFmiModifyDrawParamDlg::OnButtonSymbolFillColor()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsSymbolFillColorRef, &itsSymbolFillBitmap, itsSimpleIsoLineLowColorRect, itsSymbolFillColor));
}

void CFmiModifyDrawParamDlg::OnOK()
{
	UpdateData(TRUE);
	GetSelectedDrawParamSetUp();
	TakeDrawParamModificationInUse(); // vain onok:ssa initialisoidaan takaisin originaali drawparamiin. cancel ei siirr‰ muutoksia

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
    itsOrigDrawParam->Init(itsBackupDrawParam); // t‰ss‰ palautetaan originaali asetukset takaisin.
    // T‰m‰ on tarpeen jos on k‰ytetty refreshi‰ ja sitten perutaan cancelista muutokset.

    ForceStationViewUpdate();
}

bool CFmiModifyDrawParamDlg::IsMacroParamSymbolDrawCase()
{
	if(IsMacroParamCase())
	{
		// Jos macroParam tapaus, silloin myˆs hiladatasta kyse
		auto viewType = itsDrawParam->GetViewType(false);
		if(!NFmiDrawParam::IsColorContourType(viewType) && !NFmiDrawParam::IsIsolineType(viewType))
			return true;
	}
	return false;
}

void CFmiModifyDrawParamDlg::ForceStationViewUpdate()
{
    if(itsDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
    {
        // Liataan myˆs maphandler, jotta k‰ytˆss‰ karttan‰ytˆss‰ ollut stationView vaihtuu tarvittaessa jos esim. isoline piirto vaihtuu teksti esitykseen.
        itsSmartMetDocumentInterface->MapViewDescTop(itsDescTopIndex)->MapHandler()->SetUpdateMapViewDrawingLayers(true);
		if(IsMacroParamSymbolDrawCase())
		{
			// Pit‰‰ tyhjent‰‰ macroParam tapauksissa ja niiden symbolipiirto tapauksissa tarkastetulta rivilt‰ macroParamCache
			itsSmartMetDocumentInterface->GetCombinedMapHandlerInterface().clearMacroParamCache(itsDescTopIndex, itsRealRowNumber, itsDrawParam);
		}
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
		DoPostInitializationChecks();
	}
}

void CFmiModifyDrawParamDlg::OnSaveButton()
{
	UpdateData(TRUE);
	OnBnClickedModifyDrwParamRefresh(); // otetaan muutokset myˆs heti k‰yttˆˆn, koska ne tulevat k‰yttˆˆn joka tapauksessa
	TakeDrawParamModificationInUse(); // vain onok:ssa (ja kun asetukset otetaan kaikkialle k‰yttˆˆn ja save-nappia painettaessa)
									  // initialisoidaan takaisin originaali drawparamiin. cancel ei siirr‰ muutoksia
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

static void SetColorrefToColor(COLORREF colorref, NFmiColor& color)
{
	color.SetRGB((float(GetRValue(colorref)) / float(255.0)),
		(float(GetGValue(colorref)) / float(255.0)),
		(float(GetBValue(colorref)) / float(255.0)));
}

void CFmiModifyDrawParamDlg::ReadAllButtonColors(void)
{
	NFmiColor tmpColor;

	SetColorrefToColor(itsSymbolColorRef, tmpColor);
	itsDrawParam->FrameColor(tmpColor);

	SetColorrefToColor(itsSymbolFillColorRef, tmpColor);
	itsDrawParam->FillColor(tmpColor);

	SetColorrefToColor(itsIsoLineColorRef, tmpColor);
	itsDrawParam->IsolineColor(tmpColor);

	SetColorrefToColor(itsIsoLineLabelBoxFillColorRef, tmpColor);
	itsDrawParam->IsolineLabelBoxFillColor(tmpColor);

	SetColorrefToColor(itsIsoLineLabelColorRef, tmpColor);
	itsDrawParam->IsolineTextColor(tmpColor);

	// Vaihtuvat isoviivav‰rit (5 kpl)
	SetColorrefToColor(itsSimpleIsoLineLowColorRef, tmpColor);
	itsDrawParam->SimpleIsoLineColorShadeLowValueColor(tmpColor);

	SetColorrefToColor(itsSimpleIsoLineMidColorRef, tmpColor);
	itsDrawParam->SimpleIsoLineColorShadeMidValueColor(tmpColor);

	SetColorrefToColor(itsSimpleIsoLineHighColorRef, tmpColor);
	itsDrawParam->SimpleIsoLineColorShadeHighValueColor(tmpColor);

	SetColorrefToColor(itsSimpleIsoLineHigh2ColorRef, tmpColor);
	itsDrawParam->SimpleIsoLineColorShadeHigh2ValueColor(tmpColor);

	SetColorrefToColor(itsSimpleIsoLineHigh3ColorRef, tmpColor);
	itsDrawParam->SimpleIsoLineColorShadeHigh3ValueColor(tmpColor);

	// Hatch v‰rit (2 kpl)
	SetColorrefToColor(itsHatch1ColorRef, tmpColor);
	itsDrawParam->IsoLineHatchColor1(tmpColor);

	SetColorrefToColor(itsHatch2ColorRef, tmpColor);
	itsDrawParam->IsoLineHatchColor2(tmpColor);

	// Vaihtuvat symboli v‰rit (3 kpl)
	SetColorrefToColor(itsSymbolLowColorRef, tmpColor);
	itsDrawParam->StationSymbolColorShadeLowValueColor(tmpColor);

	SetColorrefToColor(itsSymbolMidColorRef, tmpColor);
	itsDrawParam->StationSymbolColorShadeMidValueColor(tmpColor);

	SetColorrefToColor(itsSymbolHighColorRef, tmpColor);
	itsDrawParam->StationSymbolColorShadeHighValueColor(tmpColor);

	// Simple color contour v‰rit (5 kpl)
	SetColorrefToColor(itsSimpleColorContourLowColorRef, tmpColor);
	itsDrawParam->ColorContouringColorShadeLowValueColor(tmpColor);

	SetColorrefToColor(itsSimpleColorContourMidColorRef, tmpColor);
	itsDrawParam->ColorContouringColorShadeMidValueColor(tmpColor);

	SetColorrefToColor(itsSimpleColorContourHighColorRef, tmpColor);
	itsDrawParam->ColorContouringColorShadeHighValueColor(tmpColor);

	SetColorrefToColor(itsSimpleColorContourHigh2ColorRef, tmpColor);
	itsDrawParam->ColorContouringColorShadeHigh2ValueColor(tmpColor);

	SetColorrefToColor(itsSimpleColorContourHigh3ColorRef, tmpColor);
	itsDrawParam->ColorContouringColorShadeHigh3ValueColor(tmpColor);

	return;
}



void CFmiModifyDrawParamDlg::OnShowColorIndexDlg(bool doIsolineCase)
{
	UpdateData(TRUE);
	GetSelectedDrawParamSetUp();
    
	CFmiTMColorIndexDlg dlg(this, ::GetDictionaryString("ModifyDrawParamColorIndexTitle"), ::GetDictionaryString("ModifyDrawParamColorIndexHelpStr"),
        ToolMasterColorCube::UsedColorsCube(), itsDrawParam, doIsolineCase, this);
	if(dlg.DoModal() == IDOK)
	{
		InitSpecialClassesData(); // valitut special v‰rit pit‰‰ alustaa uudestaan
		itsContourGap = itsDrawParam->ContourGab();
		// Luokan dataosan fUseColorBlendingWithCustomContours nimi on oikea 
		// k‰yttˆtarkoitus kyseiselle asetukselle (EI DrawParamin UseIsoLineGabWithCustomContours)
		fUseColorBlendingWithCustomContours = itsDrawParam->UseIsoLineGabWithCustomContours();
		UpdateData(FALSE);
	}
}

void CFmiModifyDrawParamDlg::OnBnClickedShowIsolineColorIndexDlg()
{
	OnShowColorIndexDlg(true);
}

void CFmiModifyDrawParamDlg::OnBnClickedShowContourColorIndexDlg()
{
	OnShowColorIndexDlg(false);
}

void CFmiModifyDrawParamDlg::OnButtonHatch2Color()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsHatch2ColorRef, &itsHatch2Bitmap, itsHatch2ColorRect, itsHatch2Color));
}

// T‰m‰n funktion tarkoitus on yritt‰‰ yksinkertaistaa koodia, kun k‰sitell‰‰n simple-contour
// v‰rinappuloiden kasittely‰.
// colorIndex alkaa 1:st‰, joka myˆs k‰sitell‰‰n default arvona, jos indeksi on muuten pieless‰.
// Huom! fSimpleContourTransparency# ja itsSimpleColorContourLimit#Value dataosilla on eri indeksit,
// koska transparencyja on yksi enemm‰n.
std::pair<bool, bool> CFmiModifyDrawParamDlg::GetSimpleContourTransparencyAndDisabledOptions(int colorIndex) const
{
	auto colorIsTransparent = false;
	auto colorIsDisabled = false;
	switch(colorIndex)
	{
	case 2:
	{
		colorIsTransparent = fSimpleContourTransparency2 == TRUE;
		colorIsDisabled = itsSimpleColorContourLimit1Value == kFloatMissing;
		break;
	}
	case 3:
	{
		colorIsTransparent = fSimpleContourTransparency3 == TRUE;
		colorIsDisabled = itsSimpleColorContourLimit2Value == kFloatMissing;
		break;
	}
	case 4:
	{
		colorIsTransparent = fSimpleContourTransparency4 == TRUE;
		colorIsDisabled = itsSimpleColorContourLimit3Value == kFloatMissing;
		break;
	}
	case 5:
	{
		colorIsTransparent = fSimpleContourTransparency5 == TRUE;
		colorIsDisabled = itsSimpleColorContourLimit4Value == kFloatMissing;
		break;
	}
	default:
	{
		colorIsTransparent = fSimpleContourTransparency1 == TRUE;
		colorIsDisabled = false; // HUOM! 1. v‰ri‰ ei voi disabloida (myˆs oletus indeksi k‰sittely)
		break;
	}
	}

	return std::make_pair(colorIsTransparent, colorIsDisabled);
}

// T‰m‰n funktion tarkoitus on yritt‰‰ yksinkertaistaa koodia, kun k‰sitell‰‰n simple-isoline
// v‰rinappuloiden kasittely‰.
// colorIndex alkaa 1:st‰, joka myˆs k‰sitell‰‰n default arvona, jos indeksi on muuten pieless‰.
// Huom! Ainakaan toistaiseksi isoline jutuilla ei ole transparencya, koska siin‰ ei ole mit‰‰n j‰rke‰.
std::pair<bool, bool> CFmiModifyDrawParamDlg::GetSimpleIsolineTransparencyAndDisabledOptions(int colorIndex) const
{
	auto colorIsTransparent = false; // kaikissa tapauksissa
	auto colorIsDisabled = false;
	switch(colorIndex)
	{
	case 2:
	{
		colorIsDisabled = itsSimpleIsoLineColorMidValue == kFloatMissing;
		break;
	}
	case 3:
	{
		colorIsDisabled = itsSimpleIsoLineColorHighValue == kFloatMissing;
		break;
	}
	case 4:
	{
		colorIsDisabled = itsSimpleIsoLineColorHigh2Value == kFloatMissing;
		break;
	}
	default:
	{
		colorIsDisabled = itsSimpleIsoLineColorLowValue == kFloatMissing;
		break;
	}
	}

	return std::make_pair(colorIsTransparent, colorIsDisabled);
}

NFmiColorButtonDrawingData MakeColorButtonDrawingData(CWnd* view, COLORREF& color, CBitmap** bitmap, CRect& rect, CButton& button, const std::pair<bool, bool>& colorOptions, bool initColor, const NFmiColor& nfmiColor)
{
	auto buttonDrawingData = NFmiColorButtonDrawingData(view, color, bitmap, rect, button, colorOptions);
	if(initColor)
		buttonDrawingData.SetNfmiColor(nfmiColor);
	return buttonDrawingData;
}

NFmiColorButtonDrawingData CFmiModifyDrawParamDlg::GetSimpleContourColorButtonData(int colorIndex, bool initColor)
{
	auto colorOptions = GetSimpleContourTransparencyAndDisabledOptions(colorIndex);
	switch(colorIndex)
	{
	case 2:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleColorContourMidColorRef, &itsSimpleColorContourMidBitmap, itsSimpleColorContourMidColorRect, itsSimpleColorContourMidColor, colorOptions, initColor, itsDrawParam->ColorContouringColorShadeMidValueColor());
	}
	case 3:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleColorContourHighColorRef, &itsSimpleColorContourHighBitmap, itsSimpleColorContourHighColorRect, itsSimpleColorContourHighColor, colorOptions, initColor, itsDrawParam->ColorContouringColorShadeHighValueColor());
	}
	case 4:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleColorContourHigh2ColorRef, &itsSimpleColorContourHigh2Bitmap, itsSimpleColorContourHigh2ColorRect, itsSimpleColorContourHigh2Color, colorOptions, initColor, itsDrawParam->ColorContouringColorShadeHigh2ValueColor());
	}
	case 5:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleColorContourHigh3ColorRef, &itsSimpleColorContourHigh3Bitmap, itsSimpleColorContourHigh3ColorRect, itsSimpleColorContourHigh3Color, colorOptions, initColor, itsDrawParam->ColorContouringColorShadeHigh3ValueColor());
	}
	default:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleColorContourLowColorRef, &itsSimpleColorContourLowBitmap, itsSimpleColorContourLowColorRect, itsSimpleColorContourLowColor, colorOptions, initColor, itsDrawParam->ColorContouringColorShadeLowValueColor());
	}
	}
}

NFmiColorButtonDrawingData CFmiModifyDrawParamDlg::GetSimpleIsolineColorButtonData(int colorIndex, bool initColor)
{
	auto colorOptions = GetSimpleIsolineTransparencyAndDisabledOptions(colorIndex);
	switch(colorIndex)
	{
	case 2:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleIsoLineMidColorRef, &itsSimpleIsoLineMidBitmap, itsSimpleIsoLineMidColorRect, itsSimpleIsoLineMidColor, colorOptions, initColor, itsDrawParam->SimpleIsoLineColorShadeMidValueColor());
	}
	case 3:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleIsoLineHighColorRef, &itsSimpleIsoLineHighBitmap, itsSimpleIsoLineHighColorRect, itsSimpleIsoLineHighColor, colorOptions, initColor, itsDrawParam->SimpleIsoLineColorShadeHighValueColor());
	}
	case 4:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleIsoLineHigh2ColorRef, &itsSimpleIsoLineHigh2Bitmap, itsSimpleIsoLineHigh2ColorRect, itsSimpleIsoLineHigh2Color, colorOptions, initColor, itsDrawParam->SimpleIsoLineColorShadeHigh2ValueColor());
	}
	case 5:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleIsoLineHigh3ColorRef, &itsSimpleIsoLineHigh3Bitmap, itsSimpleIsoLineHigh3ColorRect, itsSimpleIsoLineHigh3Color, colorOptions, initColor, itsDrawParam->SimpleIsoLineColorShadeHigh3ValueColor());
	}
	default:
	{
		return ::MakeColorButtonDrawingData(this, itsSimpleIsoLineLowColorRef, &itsSimpleIsoLineLowBitmap, itsSimpleIsoLineLowColorRect, itsSimpleIsoLineLowColor, colorOptions, initColor, itsDrawParam->SimpleIsoLineColorShadeLowValueColor());
	}
	}
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleColorcontourLow()
{
	UpdateData(TRUE);
    CtrlView::ColorButtonPressed(GetSimpleContourColorButtonData(1, false));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleColorcontourMid()
{
	UpdateData(TRUE);
	CtrlView::ColorButtonPressed(GetSimpleContourColorButtonData(2, false));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleColorcontourHigh()
{
	UpdateData(TRUE);
	CtrlView::ColorButtonPressed(GetSimpleContourColorButtonData(3, false));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleColorcontourHigh2()
{
	UpdateData(TRUE);
	CtrlView::ColorButtonPressed(GetSimpleContourColorButtonData(4, false));
}

void CFmiModifyDrawParamDlg::OnButtonColorShowSimpleColorcontourHigh3()
{
	UpdateData(TRUE);
	CtrlView::ColorButtonPressed(GetSimpleContourColorButtonData(5, false));
}

void CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency1()
{
	UpdateData(TRUE);
	// Simple-contour v‰rin transparency on muuttunut, pit‰‰ piirt‰‰ siihen liittyv‰ v‰rinappula uudestaan
	CtrlView::ColorButtonDraw(GetSimpleContourColorButtonData(1, false));
}

void CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency2()
{
	UpdateData(TRUE);
	// Simple-contour v‰rin transparency on muuttunut, pit‰‰ piirt‰‰ siihen liittyv‰ v‰rinappula uudestaan
	CtrlView::ColorButtonDraw(GetSimpleContourColorButtonData(2, false));
}

void CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency3()
{
	UpdateData(TRUE);
	// Simple-contour v‰rin transparency on muuttunut, pit‰‰ piirt‰‰ siihen liittyv‰ v‰rinappula uudestaan
	CtrlView::ColorButtonDraw(GetSimpleContourColorButtonData(3, false));
}

void CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency4()
{
	UpdateData(TRUE);
	// Simple-contour v‰rin transparency on muuttunut, pit‰‰ piirt‰‰ siihen liittyv‰ v‰rinappula uudestaan
	CtrlView::ColorButtonDraw(GetSimpleContourColorButtonData(4, false));
}

void CFmiModifyDrawParamDlg::OnBnClickedCheckSimpleContourTransparency5()
{
	UpdateData(TRUE);
	// Simple-contour v‰rin transparency on muuttunut, pit‰‰ piirt‰‰ siihen liittyv‰ v‰rinappula uudestaan
	CtrlView::ColorButtonDraw(GetSimpleContourColorButtonData(5, false));
}

void CFmiModifyDrawParamDlg::OnEnChangeShowSimpleColorcontourLimitValue(ColorType colorType, int colorIndex, CString& limitStringU_, float& limitValue)
{
	// Pit‰‰ tutkia muuttuuko v‰rinappulan disable tilanne, kun rajaa muutetaan
	bool doContourCase = (colorType == ColorType::SimpleContour);
	auto originalColorOptions = doContourCase ? GetSimpleContourTransparencyAndDisabledOptions(colorIndex) : GetSimpleIsolineTransparencyAndDisabledOptions(colorIndex);
	UpdateData(TRUE);
	// Annetaan originaali tekstin olla sellaisenaan, ja otetaan stringi erilliseen muuttujaan
	CString editText = limitStringU_;
	// Siivotaan tekstist‰ pois alku ja loppu whitespacet
	editText.Trim();
	std::string valueInStdString = CT2A(editText);
	limitValue = kFloatMissing;
	try
	{
		size_t firstCharacterPositionAfterFloatValue = 0;
		auto tmpLimitValue = std::stof(valueInStdString, &firstCharacterPositionAfterFloatValue);
		if(firstCharacterPositionAfterFloatValue == valueInStdString.size())
		{
			// std::stof (funktioperhe) hyv‰ksyy kaikki stringit, jos alussa on hyv‰ksytt‰v‰ luku,
			// mutta sit‰ seuraa teksti‰ tai esim. toinen luku, t‰llˆin tulkinta lopetetaan ja 
			// std::stof funktion 2. parametriin talletetaan 1. ei lukuun liittyv‰n merkin sijainti.
			// Nyt siis hyv‰ksyt‰‰n vain sellaiset luvu-stringit, mink‰ per‰ss‰ ei ole en‰‰ mit‰‰n muuta.
			limitValue = tmpLimitValue;
		}
	}
	catch(...)
	{
	}
	auto newColorOptions = doContourCase ? GetSimpleContourTransparencyAndDisabledOptions(colorIndex) : GetSimpleIsolineTransparencyAndDisabledOptions(colorIndex);
	if(originalColorOptions.second != newColorOptions.second)
	{
		CtrlView::ColorButtonDraw(GetSimpleContourColorButtonData(colorIndex, false));
	}
}

void CFmiModifyDrawParamDlg::OnEnChangeShowSimpleColorcontourWithColorsStartValue()
{
	OnEnChangeShowSimpleColorcontourLimitValue(ColorType::SimpleContour, 2, itsSimpleColorContourLimit1StringU_, itsSimpleColorContourLimit1Value);
}

void CFmiModifyDrawParamDlg::OnEnChangeShowSimpleColorcontourWithColorsMiddleValue()
{
	OnEnChangeShowSimpleColorcontourLimitValue(ColorType::SimpleContour, 3, itsSimpleColorContourLimit2StringU_, itsSimpleColorContourLimit2Value);
}

void CFmiModifyDrawParamDlg::OnEnChangeShowSimpleColorcontourWithColorsEndValue()
{
	OnEnChangeShowSimpleColorcontourLimitValue(ColorType::SimpleContour, 4, itsSimpleColorContourLimit3StringU_, itsSimpleColorContourLimit3Value);
}

void CFmiModifyDrawParamDlg::OnEnChangeShowSimpleColorcontourWithColorsEnd2Value()
{
	OnEnChangeShowSimpleColorcontourLimitValue(ColorType::SimpleContour, 5, itsSimpleColorContourLimit4StringU_, itsSimpleColorContourLimit4Value);
}

void CFmiModifyDrawParamDlg::OnEnChangeShowSimpleIsolineWithColorsStartValue()
{
	OnEnChangeShowSimpleColorcontourLimitValue(ColorType::SimpleIsoline, 1, itsSimpleIsoLineColorLowValueStringU_, itsSimpleIsoLineColorLowValue);
}

void CFmiModifyDrawParamDlg::OnEnChangeShowSimpleIsolineWithColorsMiddleValue()
{
	OnEnChangeShowSimpleColorcontourLimitValue(ColorType::SimpleIsoline, 2, itsSimpleIsoLineColorLowValueStringU_, itsSimpleIsoLineColorLowValue);
}

void CFmiModifyDrawParamDlg::OnEnChangeShowSimpleIsolineWithColorsEndValue()
{
	OnEnChangeShowSimpleColorcontourLimitValue(ColorType::SimpleIsoline, 3, itsSimpleIsoLineColorHighValueStringU_, itsSimpleIsoLineColorHighValue);
}

void CFmiModifyDrawParamDlg::OnEnChangeShowSimpleIsolineWithColorsEnd2Value()
{
	OnEnChangeShowSimpleColorcontourLimitValue(ColorType::SimpleIsoline, 4, itsSimpleIsoLineColorHigh2ValueStringU_, itsSimpleIsoLineColorHigh2Value);
}

void CFmiModifyDrawParamDlg::OnButtonResetDrawParam()
{
	NFmiDrawParam defaultDrawParam;
	itsDrawParam->Init(&defaultDrawParam, true);
	InitDialogFromDrawParam();
	UpdateData(FALSE);
	DoPostInitializationChecks();
}

void CFmiModifyDrawParamDlg::MakeViewMacroAdjustments(void)
{
	if(itsDrawParam->ViewMacroDrawParam())
	{ // jos viewmacro drawParam, estet‰‰n talletus mahdollisuus ja laitetaan varoitus n‰kyviin punaisella
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

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_VIEW_MACRO_DRAW_PARAM_WARNING) // v‰rj‰t‰‰n viewmacro-drawparam varoitus teksti punaisella
		pDC->SetTextColor(RGB(255, 0, 0));

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_SPECIAL_ISOLINE_CLASSES_STR)
	{
		if(this->fSpecialIsolineClassesHaveInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // virhetilanteissa erikois luokka edit boxin labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0));
	}

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_SPECIAL_CONTOUR_CLASSES_STR)
	{
		if(this->fSpecialContourClassesHaveInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // virhetilanteissa erikois luokka edit boxin labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0));
	}

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_DRAW_PARAM_COLOR_PARAM_TEXT)
	{
		if(!fPossibleColorParameterOk)
			pDC->SetTextColor(RGB(255, 0, 0)); // virhetilanteissa labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0));
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

const std::string gMultiColorIsolinesText = "Multi-color isoline";
const std::string gContourColorsText = "Contour colors";
const std::string gIsolineColorsText = "Isoline colors";
const std::string gIsolineClassesText = "Isoline classes";
const std::string gIsolineColorOpenDlgText = "Line colors";
const std::string gContourColorOpenDlgText = "Cont. colors";
const std::string gContourClassesText = "Contour classes";

// T‰m‰ funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell‰.
// T‰m‰ on ik‰v‰ kyll‰ teht‰v‰ erikseen dialogin muokkaus tyˆkalusta, eli
// tekij‰n pit‰‰ lis‰t‰ erikseen t‰nne kaikki dialogin osat, joihin
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
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VIEW_MACRO_DRAW_PARAM_WARNING, "This is loaded from viewmacro, save it through viewmacro dialog!");
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
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_ISOLINE_COLORS_STR, gIsolineColorsText.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_COLOR_CONTOUR_COLORS_STR, gContourColorsText.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_COLOR_SCALE_WITH_SIMPLE_ISOLINES, gMultiColorIsolinesText.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_COLOR_BLEND_WITH_SIMPLE_ISOLINES, "Color-blend isol");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_SEPARATOR_LINES_BETWEEN_COLORCONTOUR_CLASSES, "Use separating line with contours");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_SPECIAL_CLASSES, gUseSpecialClassesStr.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_SHOW_ISOLINE_COLOR_INDEX_DLG, gIsolineColorOpenDlgText.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SPECIAL_ISOLINE_CLASSES_STR, gIsolineClassesText.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_WIDTHS_STR, "Widths\r\n[mm]");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_TYPES_STR, "IDC_STATIC_DRAW_PARAM_SPECIAL_TYPES_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SPECIAL_ISOLINE_COLOR_INDEXIES_STR, gIsolineColorsText.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_LABEL_SIZES_STR, "Label hgt\r\n[mm]");
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
	CFmiWin32Helpers::SetDialogItemText(this, IDC_MODIFY_DRW_PARAM_REFRESH, "Refresh");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_DRAW_PARAM_LOAD_FROM, "IDC_DRAW_PARAM_LOAD_FROM");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_MODIFY_DRW_PARAM_USE_WITH_ALL, "IDC_MODIFY_DRW_PARAM_USE_WITH_ALL");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_FIXED_DRAW_PARAM_LABEL, "Fixed DrawParams");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_CONTOUR_ALPHA_STR, "Alpha 5-100");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DRAW_PARAM_USE_LEGEND, "Use legend");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_COLOR_BLENDING_WITH_CUSTOM_CONTOURS, "Use color blending with contours");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_TREAT_WMS_LAYER_AS_OBSERVATION, "Treat Wms layer as observation (in animations)");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_FIXED_TEXT_SYMBOL_DRAW_LENGTH, "Fixed text length");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SYMBOL_DRAW_DENSITY_STR, "Symbol draw density");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_FLIP_ARROW_SYMBOL, "Flip arrow");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SPECIAL_CLASSES_SETTINGS_STR, gSpecialClassesSettingsStr.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_SHOW_CONTOUR_COLOR_INDEX_DLG, gContourColorOpenDlgText.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SPECIAL_CONTOUR_COLOR_INDEXIES_STR, gContourColorsText.c_str());
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SPECIAL_CONTOUR_CLASSES_STR, gContourClassesText.c_str());
	
//    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_APPLY_FIXED_DRAW_PARAMS_RIGHT_AWAY, "Apply Fixed Settings At Once");
}

void CFmiModifyDrawParamDlg::OnBnClickedModifyDrwParamRefresh()
{
	UpdateData(TRUE);
	GetSelectedDrawParamSetUp();
	itsOrigDrawParam->Init(itsDrawParam); // otetaan muuutokset ainakin hetkellisesti k‰yttˆˆn, cancel peruu muutokset!!!
	fRefreshPressed = true;
    ForceStationViewUpdate();

    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(::GetWantedMapViewIdFlag(itsDescTopIndex));
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, TRUE, TRUE, itsDescTopIndex);
}

void CFmiModifyDrawParamDlg::TakeDrawParamModificationInUse(void)
{
	itsOrigDrawParam->Init(itsDrawParam);
	itsBackupDrawParam->Init(itsDrawParam);
	fRefreshPressed = false; // t‰m‰ voidaan nollata, koska nyt muutokset on otettu k‰yttˆˆn
}

void CFmiModifyDrawParamDlg::OnBnClickedModifyDrwParamUseWithAll()
{
	UpdateData(TRUE);
	OnBnClickedModifyDrwParamRefresh(); // otetaan muutokset myˆs heti k‰yttˆˆn, koska ne tulevat k‰yttˆˆn joka tapauksessa
	TakeDrawParamModificationInUse(); // vain onok:ssa (ja kun asetukset otetaan kaikkialle k‰yttˆˆn)
									  // initialisoidaan takaisin originaali drawparamiin. cancel ei siirr‰ muutoksia
    itsSmartMetDocumentInterface->TakeDrawParamInUseEveryWhere(itsDrawParam, fModifyMapViewParam, fModifyMapViewParam, fModifyCrossSectionViewParam, fModifyMapViewParam);
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("ModifyDrawParamDlg: Using these setting for all similar parameters button pressed", TRUE, TRUE);
}

void CFmiModifyDrawParamDlg::OnEnChangeSpecialIsolineClassesValues()
{
	UpdateData(TRUE);

	fSpecialIsolineClassesHaveInvalidValues = false;
	try
	{
		std::string tmp = CT2A(itsSpecialIsolineClassValuesStrU_);
		std::vector<float> classValues = NFmiStringTools::Split<std::vector<float> >(tmp, ",");
		if(CtrlViewUtils::AreVectorValuesInRisingOrder(classValues) == false)
			throw std::runtime_error("xxx");
		itsSpecialIsolineClassCount = (int)classValues.size();
		UpdateData(FALSE);
	}
	catch(...)
	{
		fSpecialIsolineClassesHaveInvalidValues = true; // vain asetetaan t‰m‰ lippu p‰‰lle jos tulee mit‰‰n ongelmia
	}

	// IDC_STATIC_SPECIAL_ISOLINE_CLASSES_STR kontrolli v‰rj‰t‰‰n punaiseksi, jos inputissa on vikaa
	CWnd *win = GetDlgItem(IDC_STATIC_SPECIAL_ISOLINE_CLASSES_STR);
	if(win)
		win->Invalidate(FALSE);
}

void CFmiModifyDrawParamDlg::OnEnChangeSpecialContourClassesValues()
{
	UpdateData(TRUE);

	fSpecialContourClassesHaveInvalidValues = false;
	try
	{
		std::string tmp = CT2A(itsSpecialContourClassValuesStrU_);
		std::vector<float> classValues = NFmiStringTools::Split<std::vector<float> >(tmp, ",");
		if(CtrlViewUtils::AreVectorValuesInRisingOrder(classValues) == false)
			throw std::runtime_error("xxx");
		itsSpecialContourClassCount = (int)classValues.size();
		UpdateData(FALSE);
	}
	catch(...)
	{
		fSpecialContourClassesHaveInvalidValues = true; // vain asetetaan t‰m‰ lippu p‰‰lle jos tulee mit‰‰n ongelmia
	}
	CWnd* win = GetDlgItem(IDC_STATIC_SPECIAL_CONTOUR_CLASSES_STR);
	if(win)
		win->Invalidate(FALSE);
}

std::string CFmiModifyDrawParamDlg::GetSelectedFixedDrawParamPath()
{
    UpdateData(TRUE);

    int currentIndex = itsFixedDrawParamSelector.GetCurSel();
    LPITEMDATA currentData = (LPITEMDATA)itsFixedDrawParamSelector.GetItemData(currentIndex);
    if(currentData->cType != itsIconTypeFolderId) // Jos cType ei ole kansio, voidaan tehd‰ piirtoasetus s‰‰tˆj‰
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
        itsOrigDrawParam->Init(itsDrawParam); // otetaan muuutokset ainakin hetkellisesti k‰yttˆˆn, cancel peruu muutokset!!!
        UpdateData(FALSE);
		DoPostInitializationChecks();
//        if(fApplyFixedDrawParamSettingsAtOnce)
        {
            fRefreshPressed = true;
            ForceStationViewUpdate();
            ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(::GetWantedMapViewIdFlag(itsDescTopIndex));
            itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("ModifyDrawParamDlg: Fixed draw param selected", TRUE, TRUE, itsDescTopIndex); // p‰ivitet‰‰n n‰yttˆj‰
        }
    }
}

void CFmiModifyDrawParamDlg::OnBnClickedButtonReloadOriginal()
{
    DoOnCancel(); // otetaan originaali asetukset k‰yttˆˆn, liataan n‰yttˆj‰
    itsDrawParam->Init(itsOrigDrawParam); // DoCancel laitta originaalit itsOrigDrawParam:iin, josta ne laitetaan t‰ss‰ itsDrawParam
    InitDialogFromDrawParam(); // alustetaan dialogin arvot itsDrawParam:in arvoilla
    UpdateData(FALSE);
	DoPostInitializationChecks();

    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(::GetWantedMapViewIdFlag(itsDescTopIndex));
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("ModifyDrawParamDlg: Reload original draw param settings", TRUE, TRUE, itsDescTopIndex); // p‰ivitet‰‰n n‰yttˆj‰
}

std::pair<double, double> CFmiModifyDrawParamDlg::GetSymbolDrawDensityValuesFromSlider()
{
	double symbolDrawDensityX = ::SliderPosToSymbolDrawDensity(itsSymbolDrawDensityXSlider.GetPos());
	double symbolDrawDensityY = ::SliderPosToSymbolDrawDensity(itsSymbolDrawDensityYSlider.GetPos());
	return std::make_pair(symbolDrawDensityX, symbolDrawDensityY);
}

void CFmiModifyDrawParamDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateData(TRUE);
	UpdateSymbolDrawDensityStr();
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFmiModifyDrawParamDlg::UpdateSymbolDrawDensityStr()
{
	auto densityValues = GetSymbolDrawDensityValuesFromSlider();
	std::string densityValuesStr = "Symbol draw density X: ";
	densityValuesStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(densityValues.first, 1);
	densityValuesStr += ", Y: ";
	densityValuesStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(densityValues.second, 1);
	itsSymbolDrawDensityStr = CA2T(densityValuesStr.c_str());
	UpdateData(FALSE);
}


void CFmiModifyDrawParamDlg::OnEnChangeEditDrawParamColorParamStr()
{
	UpdateData(TRUE);
	auto originalStatus = fPossibleColorParameterOk;
	fPossibleColorParameterOk = IsPossibleColorParameterStrOk(itsPossibleColorParameterStr);
	if(originalStatus != fPossibleColorParameterOk)
	{
		CWnd* win = GetDlgItem(IDC_STATIC_DRAW_PARAM_COLOR_PARAM_TEXT);
		if(win)
			win->Invalidate(FALSE);
	}
}

bool CFmiModifyDrawParamDlg::IsPossibleColorParameterStrOk(const CString& colorParameterStr)
{
	std::string originalVariableStr = CT2A(colorParameterStr);
	NFmiStringTools::Trim(originalVariableStr);
	if(!originalVariableStr.empty())
	{
		try
		{
			auto variableData = NFmiSmartToolIntepreter::CheckForVariableDataType(originalVariableStr);
			// Meit‰ kiinnostaa t‰ss‰ vaiheessa vain editoitu data, eli pelkk‰ parametri
			auto editedDataOnly = variableData.second.IsEditedData();
			// Lis‰ksi editoitu data ei voi sis‰lt‰‰ level-tietoa, koska emme halua vaihtaa sellaista viel‰
			auto noLevelInfo = variableData.second.UsedLevel() == nullptr;
			return variableData.first && editedDataOnly && noLevelInfo;
		}
		catch(std::exception &)
		{ }
		return false;
	}

	// Esim. tyhj‰ parametri on ok, koska silloin ei haluta k‰ytt‰‰ erillist‰ v‰ritysparametria
	return true;
}

// Kun dialogi on alustettu tavalla tai toisella drawParam oliosta, t‰t‰ 
// tarkastelufunktiota voidaan kutsua, sen j‰lkeen kun UpdateData(FALSE) on kutsuttu.
void CFmiModifyDrawParamDlg::DoPostInitializationChecks()
{
	fPossibleColorParameterOk = IsPossibleColorParameterStrOk(itsPossibleColorParameterStr);
}

void CFmiModifyDrawParamDlg::SetDialogControlTooltip(int controlId, const std::string& tooltipRawText)
{
	CWnd* controlWnd = GetDlgItem(controlId);
	if(controlWnd)
	{
		auto finalText = ::GetDictionaryString(tooltipRawText.c_str());
		m_tooltip.AddTool(controlWnd, CA2T(finalText.c_str()));
	}
}

static std::string MakeDecoratedTooltipString(const std::string& baseText, const std::string& colorString, bool useBold)
{
	std::string str;
	if(useBold)
	{
		str += "<b>";
	}
	auto useColor = !colorString.empty();
	if(useColor)
	{
		str += "<font color = ";
		str += colorString;
		str += ">";
	}

	str += baseText;

	if(useColor)
	{
		str += "</font>";
	}
	if(useBold)
	{
		str += "</b>";
	}

	return str;
}

static std::string MakeColorParamCtrlTooltip()
{
	std::string str = ::MakeDecoratedTooltipString("Color parameter", "darkgreen", true);
	str += "<br>Write e.g. ";
	str += ::MakeDecoratedTooltipString("T", "green", true);
	str += " or ";
	str += ::MakeDecoratedTooltipString("par4", "green", true);
	str += " if you want to color viewed parameter's symbols with values of Temperature from the same producer.";
	str += "<br>Also you must check the \"Show symbol in colors\" checkbox in order to utilize colors.";
	str += "<br>You can't change producer or level for this color parameter.";
	str += "<br>If \"";
	str += ::MakeDecoratedTooltipString("Color Parameter", "", true);
	str += "\" text turns into ";
	str += ::MakeDecoratedTooltipString("red", "red", true);
	str += ", then input was incorrectand and this setting is ignored.";
	return str;
}

static std::string MakeParamNameOnDataCtrlTooltip()
{
	std::string str = "Name of the parameter in current data file";
	return str;
}

static std::string MakeProducerNameOnDataCtrlTooltip()
{
	std::string str = "Name of the producer in current data file";
	return str;
}

static std::string MakeIsolineColorsTooltip()
{
	std::string str = "Setup 'simple' multi colored isolines<br>with 4 limits and their max 5 colors.<br>Remember to check the \"";
	str += ::MakeDecoratedTooltipString(gMultiColorIsolinesText, "", true);
	str += "\" checkbox also.";
	return str;
}

static std::string MakeUseMultiColorIsolineTooltip()
{
	std::string str = "Check this if you want to use multi colored isolines<br>defined by \"";
	str += ::MakeDecoratedTooltipString(gIsolineColorsText, "", true);
	str += "\" limits and their colors.<br>If not checked, single color isolines are used.<br>If no color blend in use can utilize max 4 limits with 5 colors.";
	return str;
}

static std::string MakeContourColorsTooltip()
{
	std::string str = "Setup 'simple' contour colors<br>with 4 limits and their max 5 colors.";
	str += "<br>Note that used number of colors depend if limits and contour step matches.";
	str += "<br>So if used step is 5 and given limits are also 5, then max 4 limits and 5 colors are used.";
	str += "<br>If given step is smaller than gaps between limits, then max 4 limits and colors are used.";
	return str;
}

static std::string MakeUseSpecialClassesTooltip(const std::string &titleStr)
{
	std::string str = ::MakeDecoratedTooltipString(titleStr, "blue", true);
	str += "<br>You can use special classes to color isolines, contours and symbols.";
	str += "<br>Give wanted value limits, color indices, line widths,";
	str += "<br>line styles, isoline label sizes in comma separated lists (e.g. 1,2,3,...).";
	str += "<br>There are separate classes and color indice values for contours.";
	str += "<br>Symbol class limits and coloring are done with related isoline values.";
	return str;
}

static std::string MakeSpecialIsolineClassesLimitTooltip(const std::string& titleStr, bool isolineCase)
{
	std::string str = ::MakeDecoratedTooltipString(titleStr, "blue", true);
	str += "<br>Give a list of value limits in rising order.";
	str += "<br>Sample \"1, 2, 5, 10, 20, 50\".";
	if(isolineCase)
	{
		str += "<br>This sample list of 6 limits needs 6 colors in order to get all the lines their own colors.";
		str += "<br>If less colors, then the last one will be repeated, if more then the rest are ignored.";
	}
	else
	{
		str += "<br>This sample list of 6 limits needs 6 colors if color blending is used.";
		str += "<br>It needs 7 colors if colors are not blended in order to get all possible colors in use.";
	}
	str += "<br>If any of the value limit numbers are not in rising order, nothing will be drawn.";
	str += "<br>This value limit list is the only list that has to have all the values, other<br>related lists keep using the last value for rest of the value limits.";
	return str;
}

const std::string gLastValueIsRepeatedStr = "<br>Last value is repeated automatically for the rest of value limits.";

static std::string MakeSpecialClassesLineWidthTooltip()
{
	std::string str = "Give a list of isoline widths in mm.";
	str += "<br>You need only give one value, if all the isolines are the same width.";
	str += gLastValueIsRepeatedStr;
	return str;
}

static std::string MakeSpecialClassesLineStyleTooltip()
{
	std::string str = "Give a list of isoline drawing style with numbers 0-30.";
	str += "<br>0 is solid line style and rest are different kind of dot-dash style lines.";
	str += "<br>Experiment with different values until you find suitable ones.";
	str += "<br>You need only give one style, if all the isolines are drawn with the same style.";
	str += gLastValueIsRepeatedStr;
	return str;
}

static std::string MakeSpecialClassesLabelHeightTooltip()
{
	std::string str = "Give a list of isoline label height in mm.";
	str += "<br>If you don't want a certain isoline to have labels at all, give value 0.";
	str += "<br>You need only give one value, if all the isolines have the same size labels.";
	str += gLastValueIsRepeatedStr;
	return str;
}

static std::string MakeSpecialClassesColorIndexTooltip(const std::string& titleStr, const std::string& buttonControlStr)
{
	std::string str = ::MakeDecoratedTooltipString(titleStr, "blue", true);
	str += "<br>Give a list of color indices, their values are 2-514.";
	str += "<br>2 is for transparent color used with non visible contours.";
	str += "<br>Values 3-514 form a 8x8x8 (RGB) color cube with plenty of colors.";
	str += "<br>Some basic colors: black=3, white=514, blue=10, red=451, cyan=66, yellow=507, violet=458";
	str += "<br>Open up the Color indexies dialog from ";
	str += ::MakeDecoratedTooltipString(buttonControlStr, "", true);
	str += " button.";
	str += gLastValueIsRepeatedStr;
	return str;
}

static std::string MakeOpenColorDialogTooltip(const std::string& titleStr, const std::string &relatedControlStr)
{
	std::string str = ::MakeDecoratedTooltipString(titleStr, "blue", true);
	str += "<br>Open up dialog that will help you to choose colors for<br>you special classes visualization scheme easier.";
	str += "<br>Selected color's indexies will be added to the ";
	str += ::MakeDecoratedTooltipString(relatedControlStr, "", true);
	str += " list.";
	return str;
}

// HUOM! Outouksia miten tooltipit eri kontrolleissa toimivat t‰ss‰ dialogissa:
// Syy outouksiin on tuntematon, mutta varmaan liittyy siihen ett‰ dialogi on tungettu t‰yteen erilaisia kontrolleja.
// Joskus static-teksti-kontrollit voivat saada tooltipin toimimaan, mutta varsinkin viime aikoina lis‰tyt kontrollit eiv‰t.
// Lˆysin keinon mill‰ toiminta voidaan 'pakottaa' static-teksti-kontrolliin: Laita Notify optio true:ksi, niin rupee pelitt‰m‰‰n.
// Ei ole tietoa onko t‰st‰ asetuksesta haittaa, koska silloin kontolli rupeaa l‰hett‰m‰‰n notifikaatioita, kun sit‰ klikataan jne.
// Pikaisten testien valossa t‰st‰ viestien l‰hettelyst‰ ei ole haittaa.
// N‰ytt‰‰ silt‰ ett‰ toimivat/aktiiviset kontrollit voivat aina saada tooltipin.
void CFmiModifyDrawParamDlg::InitTooltipControl()
{
	m_tooltip.Create(this);
	m_tooltip.SetDelayTime(PPTOOLTIP_TIME_AUTOPOP, 30000); // kuinka kauan tooltippi viipyy, jos kursoria ei liikuteta [ms]
	m_tooltip.SetDelayTime(PPTOOLTIP_TIME_INITIAL, 1200); // kuinka nopeasti tooltip ilmestyy n‰kyviin, jos kursoria ei liikuteta [ms]

	// T‰ss‰ erikseen jokainen kontrolli, jolle halutaan joku tooltip teksti
	SetDialogControlTooltip(IDC_STATIC_DRAW_PARAM_COLOR_PARAM_TEXT, ::MakeColorParamCtrlTooltip());
//	SetDialogControlTooltip(IDC_EDIT_DRAW_PARAM_COLOR_PARAM_STR, ::MakeColorParamCtrlTooltip());
	SetDialogControlTooltip(IDC_BUTTON_RELOAD_ORIGINAL, "Reload original drawing-property settings from the file<br>to revert your changes you have made so far here.");
	SetDialogControlTooltip(IDC_DRAW_PARAM_LOAD_FROM, "Load drawing-property settings from an other parameter's file.");
	SetDialogControlTooltip(IDC_SAVE_BUTTON, "Store these settings to this parameter's drawing-property<br>file permanently to be used in this PC and if file<br>sharing is in use, for all other PC's in your group as well.");
	SetDialogControlTooltip(IDC_MODIFY_DRW_PARAM_REFRESH, "Check how your changes here affect on this parameter's<br>visualizations on map-view or cross-section-view.");
	SetDialogControlTooltip(IDC_BUTTON_RESET_DRAW_PARAM, "Reset all drawing-property settings of this parameter to the<br>hard coded general default settings. These default settings doesn't<br>mean the settings that are stored in the file of this parameter.");
	SetDialogControlTooltip(IDC_MODIFY_DRW_PARAM_USE_WITH_ALL, "Take these setting from this parameter (+level) and use them with all the<br>cases with the same parameter. Affects all the e.g. Temperatures on all<br>map-views with Temperatures from different producers. This doesn't<br>affect on different levels or different views (map-view vs cross-section-view).");
	SetDialogControlTooltip(IDC_STATIC_FIXED_TEXT_SYMBOL_DRAW_LENGTH_STR, "You can define fixed character length for text based<br>symbol visualization when Smartmet calculates needed<br>space for each drawn symbol on map-view.<br>Default value 0 let's SmartMet to determine needed character length,<br>but this self determination can be tricky in many cases.<br>So giving here positive integer value fixes the used space for symbols.<br>The text will be drawn as whole even if therre are<br>more characters then the fixed length suggests.");
	SetDialogControlTooltip(IDC_STATIC_SYMBOL_DRAW_DENSITY_STR, "You can adjust symbol drawing density in x- and y-directions (map-view).<br>Values smaller than 1 will make symbols more dense and values<br>greater than 1 will make symbols more sparse.<br>Use sliders to adjust x- and y-values.<br>With default value 1 SmartMet will use normal<br>symbol density when visualizing symbols.");
	SetDialogControlTooltip(IDC_CHECK_DO_SPARSE_SYMBOL_VISUALIZATION, "In case drawing grid data with symbols where data happens to be<br>sparse (= a lot of values are missing), visualization can be poorly done.<br>Normally SmartMet visualizes nth data row and mth data column and<br>if on those grid points parameter values are missing,<br>nothing or few values will be drawn.<br>Sparse visualization draws a value to those nth row and mth column,<br>but if at that point is missing value, SmartMet searches near by<br>the closest non-missing value and draws that on the spot.");
	SetDialogControlTooltip(IDC_STATIC_FIXED_DRAW_PARAM_LABEL, "Fixed DrawParam system is set of sample visualization settings.<br>They are divided in groups of different characteristiks (isoline,color-contour,etc.).<br>Just select one from the dropdown list and it's drawing properties<br>are set into dialog for you to use or further modify.");
	SetDialogControlTooltip(IDC_PARAM_ST, MakeParamNameOnDataCtrlTooltip());
	SetDialogControlTooltip(IDC_PARAM, MakeParamNameOnDataCtrlTooltip());
	SetDialogControlTooltip(IDC_PRIORITY_ST, MakeProducerNameOnDataCtrlTooltip());
	SetDialogControlTooltip(IDC_PRODUCER, MakeProducerNameOnDataCtrlTooltip());
	SetDialogControlTooltip(IDC_PARAM_ST2, "User given abbreviation for this parameter's name.<br>This is used in Smartmet over the actual parameter name on data.");
	SetDialogControlTooltip(IDC_CHECK_USE_COLOR_SCALE_WITH_SIMPLE_ISOLINES, ::MakeUseMultiColorIsolineTooltip());
	SetDialogControlTooltip(IDC_CHECK_USE_COLOR_BLEND_WITH_SIMPLE_ISOLINES, "Check this if you want to blend isoline colors<br>by the isoline steps and limiting colors.<br>Always uses the same amount limits and colors, max 4.");
	SetDialogControlTooltip(IDC_STATIC_DRAW_PARAM_ISOLINE_COLORS_STR, ::MakeIsolineColorsTooltip());
	SetDialogControlTooltip(IDC_STATIC_DRAW_PARAM_COLOR_CONTOUR_COLORS_STR, ::MakeContourColorsTooltip());
	SetDialogControlTooltip(IDC_CHECK_USE_SPECIAL_CLASSES, ::MakeUseSpecialClassesTooltip(gUseSpecialClassesStr));
	// Ei pysty antamaan tooltip juttua t‰ll‰iselle group-box tekstille
	//SetDialogControlTooltip(IDC_STATIC_SPECIAL_CLASSES_SETTINGS_STR, ::MakeUseSpecialClassesTooltip(gSpecialClassesSettingsStr));
	SetDialogControlTooltip(IDC_STATIC_SPECIAL_ISOLINE_CLASSES_STR, ::MakeSpecialIsolineClassesLimitTooltip(gIsolineClassesText, true));
	SetDialogControlTooltip(IDC_STATIC_DRAW_PARAM_SPECIAL_WIDTHS_STR, ::MakeSpecialClassesLineWidthTooltip());
	SetDialogControlTooltip(IDC_STATIC_DRAW_PARAM_SPECIAL_TYPES_STR, ::MakeSpecialClassesLineStyleTooltip());
	SetDialogControlTooltip(IDC_STATIC_DRAW_PARAM_SPECIAL_LABEL_SIZES_STR, ::MakeSpecialClassesLabelHeightTooltip());
	SetDialogControlTooltip(IDC_STATIC_SPECIAL_ISOLINE_COLOR_INDEXIES_STR, ::MakeSpecialClassesColorIndexTooltip(gIsolineColorsText, gIsolineColorOpenDlgText));
	SetDialogControlTooltip(IDC_CHECK_USE_COLOR_BLENDING_WITH_CUSTOM_CONTOURS, "Usable only with contour coloring schemes.<br>If this checked and contours used and contour step smaller<br>than value limits, then the smaller step colors are constructed by color blending.");
	SetDialogControlTooltip(IDC_SHOW_ISOLINE_COLOR_INDEX_DLG, ::MakeOpenColorDialogTooltip("Open isoline colors dialogue", gIsolineColorsText));
	SetDialogControlTooltip(IDC_SHOW_CONTOUR_COLOR_INDEX_DLG, ::MakeOpenColorDialogTooltip("Open contour colors dialogue", gContourColorsText));
	SetDialogControlTooltip(IDC_STATIC_SPECIAL_CONTOUR_CLASSES_STR, ::MakeSpecialIsolineClassesLimitTooltip(gContourClassesText, false));
	SetDialogControlTooltip(IDC_STATIC_SPECIAL_CONTOUR_COLOR_INDEXIES_STR, ::MakeSpecialClassesColorIndexTooltip(gContourColorsText, gContourColorOpenDlgText));
}


BOOL CFmiModifyDrawParamDlg::PreTranslateMessage(MSG* pMsg)
{
	m_tooltip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

