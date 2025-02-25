// CFmiVisualizationSettings.cpp : implementation file
//

#include "stdafx.h"
#include "CFmiVisualizationSettings.h"
#include "FmiWin32Helpers.h"
#include "NFmiDictionaryFunction.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiVisualizationSpaceoutSettings.h"
#include "catlog/catlog.h"
#include "CombinedMapHandlerInterface.h"
#include "persist2.h"
#include "FmiWin32TemplateHelpers.h"
#include "NFmiValueString.h"
#include "boost/math/special_functions/round.hpp"
#include <numeric>


// CFmiVisualizationSettings dialog

const NFmiViewPosRegistryInfo CFmiVisualizationSettings::s_ViewPosRegistryInfo(CRect(320, 270, 970, 680), "\\VisualizationSettingsDlg");

IMPLEMENT_DYNAMIC(CFmiVisualizationSettings, CDialogEx)

CFmiVisualizationSettings::CFmiVisualizationSettings(CWnd* pParent)
	: CDialogEx(IDD, pParent)
	, itsPixelToGridPointRatio_valueFromSlider(0)
	, itsUsePixelToGridPointRatioSafetyFeature(FALSE)
	, itsUseGlobalVisualizationSpaceoutFactorOptimization(FALSE)
	, itsPixelToGridPointRatioWarningStr(_T(""))
	, itsPixelToGridPointRatioValueStr(_T(""))
	, itsGlobalVisualizationSpaceoutGridSizeResultsStr(_T(""))
	, fUseSpaceoutOptimizationsWithBetaProducts(FALSE)
{
	itsApplicationWinRegistry = &SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->ApplicationWinRegistry();
}

CFmiVisualizationSettings::~CFmiVisualizationSettings()
{
}

void CFmiVisualizationSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_VISUALIZATIONS_USE_PIXEL_TO_GRID_POINT_RATIO_SAFETY_FEATURE, itsUsePixelToGridPointRatioSafetyFeature);
	DDX_Check(pDX, IDC_CHECK_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR, itsUseGlobalVisualizationSpaceoutFactorOptimization);
	DDX_Control(pDX, IDC_COMBO_VISUALIZATIONS_SPACEOUT_DATA_GATHERING_METHOD, itsSpaceoutDataGatheringMethodComboBox);
	DDX_Text(pDX, IDC_STATIC_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO_WARNING_STR, itsPixelToGridPointRatioWarningStr);
	DDX_Control(pDX, IDC_SLIDER_PIXEL_TO_GRID_POINT_RATIO, itsPixelToGridPointRatioSlider);
	DDX_Text(pDX, IDC_STATIC_PIXEL_TO_GRID_POINT_RATIO_VALUE_STR, itsPixelToGridPointRatioValueStr);
	DDX_Control(pDX, IDC_SLIDER_GLOBAL_VISUALIZATION_SPACEOUT_FACTOR, itsGlobalVisualizationSpaceoutGridSizeSlider);
	DDX_Text(pDX, IDC_STATIC_GLOBAL_VISUALIZATION_SPACEOUT_FACTOR_RESULTS_STR, itsGlobalVisualizationSpaceoutGridSizeResultsStr);
	DDX_Check(pDX, IDC_CHECK_USE_SPACE_OUT_OPTIMIZATIONS_WITH_BETA_PRODUCTS, fUseSpaceoutOptimizationsWithBetaProducts);
}


BEGIN_MESSAGE_MAP(CFmiVisualizationSettings, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CFmiVisualizationSettings::OnBnClickedButtonUpdate)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_VISUALIZATIONS_USE_PIXEL_TO_GRID_POINT_RATIO_SAFETY_FEATURE, &CFmiVisualizationSettings::OnBnClickedCheckVisualizationsUsePixelToGridPointRatioSafetyFeature)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CFmiVisualizationSettings message handlers

void CFmiVisualizationSettings::SetDefaultValues(void)
{
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	// gets current window position
	BOOL bRet = GetWindowPlacement(&wndpl);

	CRect oldRect(wndpl.rcNormalPosition);
	const CRect& defaultRect = CFmiVisualizationSettings::ViewPosRegistryInfo().DefaultWindowRect();
	// HUOM! dialogin kokoa ei saa muuttaa!!
	MoveWindow(defaultRect.left, defaultRect.top, oldRect.Width(), oldRect.Height());
	Persist2::WriteWindowRectToWinRegistry(*itsApplicationWinRegistry, MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiVisualizationSettings::Update()
{
	// T�m� metodi pit�� toteuttaa, vaikka tyhj�n�, jotta luokka menisi SmartMetissa k�ytettyjen n�yt�n talletus frameworkin l�pi
}

BOOL CFmiVisualizationSettings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	InitPixelToGridPointRatioSlider();
	InitGlobalVisualizationSpaceoutGridSizeSlider();

	itsSpaceoutDataGatheringMethodComboBox.AddString(CA2T(::GetDictionaryString("Linear-interpolation").c_str()));
	itsSpaceoutDataGatheringMethodComboBox.AddString(CA2T(::GetDictionaryString("Median-filter").c_str()));
	itsSpaceoutDataGatheringMethodComboBox.SetCurSel(0); // GetVisualizationSettings().spaceoutDataGatheringMethod());

	fUseSpaceoutOptimizationsWithBetaProducts = GetVisualizationSettings().useSpaceoutOptimizationsForBetaProducts();

	// Tee paikan asetus vasta tooltipin alustuksen j�lkeen, niin se toimii ilman OnSize-kutsua.
	std::string errorBaseStr("Error in CFmiVisualizationSettings::OnInitDialog while reading dialog size and position values");
	CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(*itsApplicationWinRegistry, this, true, errorBaseStr, 0);

	// P�ivitet��n muuttujien arvot dialogin kontrolleihin
	UpdateData(FALSE); 
	// Tehd��n alustuksen j�lkeen testit varoituksien suhteen
	CheckInputAndDoWarningTexts();
	// return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
	return TRUE;  
}

void CFmiVisualizationSettings::InitPixelToGridPointRatioSlider()
{
	auto& visualizationSettings = GetVisualizationSettings();
	auto pixelToGridPointRatio = visualizationSettings.pixelToGridPointRatio();
	auto minPixelToGridPointRatio = visualizationSettings.minPixelToGridPointRatioValue();
	auto maxPixelToGridPointRatio = visualizationSettings.maxPixelToGridPointRatioValue();
	// T�m� vastaa minPixelToGridPointRatio arvoa
	int startIndex = 0; 
	// T�m� vastaa maxPixelToGridPointRatio arvoa
	int endIndex = boost::math::iround((maxPixelToGridPointRatio - minPixelToGridPointRatio) / itsPixelToGridPointRatioSliderTickStep);
	itsPixelToGridPointRatioSlider.SetRange(startIndex, endIndex);
	itsPixelToGridPointRatioSlider.SetPos(CalcPixelToGridPointRatioSliderPosition(pixelToGridPointRatio));
	itsUsePixelToGridPointRatioSafetyFeature = visualizationSettings.usePixelToGridPointRatioSafetyFeature();
	UpdateData(FALSE);
	UpdatePixelToGridPointRatioFromSliderToStringCtrl();
}

void CFmiVisualizationSettings::InitGlobalVisualizationSpaceoutGridSizeSlider()
{
	auto& visualizationSettings = GetVisualizationSettings();
	auto baseSpaceoutGridSizeValue = visualizationSettings.baseSpaceoutGridSize();
	auto minBaseSpaceoutGridSize = visualizationSettings.minBaseSpaceoutGridSize();
	auto maxBaseSpaceoutGridSize = visualizationSettings.maxBaseSpaceoutGridSize();
	itsGlobalVisualizationSpaceoutGridSizeSlider.SetRange(minBaseSpaceoutGridSize, maxBaseSpaceoutGridSize);
	itsGlobalVisualizationSpaceoutGridSizeSlider.SetPos(baseSpaceoutGridSizeValue);
	itsUseGlobalVisualizationSpaceoutFactorOptimization = visualizationSettings.useGlobalVisualizationSpaceoutFactorOptimization();
	UpdateData(FALSE);
	UpdateGlobalVisualizationSpaceoutGridSizeFromSliderToStringCtrl();
}

int CFmiVisualizationSettings::CalcPixelToGridPointRatioSliderPosition(double value)
{
	auto& visualizationSettings = GetVisualizationSettings();
	auto minPixelToGridPointRatio = visualizationSettings.minPixelToGridPointRatioValue();
	auto maxPixelToGridPointRatio = visualizationSettings.maxPixelToGridPointRatioValue();
	if(value <= minPixelToGridPointRatio)
		return 0;
	if(value >= maxPixelToGridPointRatio)
		return itsPixelToGridPointRatioSlider.GetRangeMax();

	return boost::math::iround((value - minPixelToGridPointRatio) / itsPixelToGridPointRatioSliderTickStep);
}

static double GetActualValueFromSlider(CSliderCtrl &slider, double minValue, double tickStepValue)
{
	auto sliderPos = slider.GetPos();
	auto value = (sliderPos * tickStepValue) + minValue;
	if(std::abs(value - std::round(value)) <= std::numeric_limits<double>::epsilon() * 3)
	{
		value = std::round(value);
	}
	return value;
}


double CFmiVisualizationSettings::GetPixelToGridPointRatioFromSlider()
{
	return GetActualValueFromSlider(itsPixelToGridPointRatioSlider, GetVisualizationSettings().minPixelToGridPointRatioValue(), itsPixelToGridPointRatioSliderTickStep);
}

// T�m� funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell�.
// T�m� on ik�v� kyll� teht�v� erikseen dialogin muokkaus ty�kalusta, eli
// tekij�n pit�� lis�t� erikseen t�nne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiVisualizationSettings::InitDialogTexts()
{
    SetWindowText(CA2T(::GetDictionaryString("Visualization settings").c_str()));
    CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
    CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");

	auto& visualizationSettings = GetVisualizationSettings();

	std::string pixelToGridPointRatioStr = "Pixel-To-Grid-Point-Ratio is safety feature for isoline visualizations, used to\nprevent crashes. It's min value is ";
	pixelToGridPointRatioStr += std::to_string(boost::math::iround(visualizationSettings.minPixelToGridPointRatioValue()));
	pixelToGridPointRatioStr += " and max value is ";
	pixelToGridPointRatioStr += std::to_string(boost::math::iround(visualizationSettings.maxPixelToGridPointRatioValue()));
	pixelToGridPointRatioStr += " (too high-res data will\nbe 'thinned' to lower resolution grid).";
	pixelToGridPointRatioStr += " 'Critical' limit for this is ";
	pixelToGridPointRatioStr += std::to_string(boost::math::iround(visualizationSettings.criticalPixelToGridPointRatioLimit()));
	pixelToGridPointRatioStr += ", if value is set\nlower, warning messages are issued in Log viewer and Map-view tooltips.";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO_STR, pixelToGridPointRatioStr.c_str());

	std::string usePixelToGridPointRatioSafetyFeatureStr = "This safety feature is on by default when application starts,\nso you can disable it for now, but not permanently!";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_VISUALIZATIONS_USE_PIXEL_TO_GRID_POINT_RATIO_SAFETY_FEATURE, usePixelToGridPointRatioSafetyFeatureStr.c_str());

	std::string globalVisualizationSpaceoutFactorStr = "Global-Visualization-Spaceout-Grid-Size variable is used to speed up map-view\nvisualizations. Setting base grid size between ";
	globalVisualizationSpaceoutFactorStr += std::to_string(visualizationSettings.minBaseSpaceoutGridSize());
	globalVisualizationSpaceoutFactorStr += " - ";
	globalVisualizationSpaceoutFactorStr += std::to_string(visualizationSettings.maxBaseSpaceoutGridSize());
	globalVisualizationSpaceoutFactorStr += " yields visualized data grid sizes\n~ ";
	auto maxGridSizeStr = std::to_string(visualizationSettings.minBaseSpaceoutGridSize());
	globalVisualizationSpaceoutFactorStr += maxGridSizeStr;
	globalVisualizationSpaceoutFactorStr += "x";
	globalVisualizationSpaceoutFactorStr += maxGridSizeStr;
	globalVisualizationSpaceoutFactorStr += " - ";
	auto minGridSizeStr = std::to_string(visualizationSettings.maxBaseSpaceoutGridSize());
	globalVisualizationSpaceoutFactorStr += minGridSizeStr;
	globalVisualizationSpaceoutFactorStr += "x";
	globalVisualizationSpaceoutFactorStr += minGridSizeStr;
	globalVisualizationSpaceoutFactorStr += ". Low-res data is faster to draw but it loses information.";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR_STR, globalVisualizationSpaceoutFactorStr.c_str());

	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR, "Use spaceout optimizations");

	std::string spaceoutDataGatheringMethodStr = "Spaceout-Data-Gathering-Method setting, Median-filter is not implemented yet.\nUsed grid data gathering methods: 1) Linear-interpolation, 2) Median-filter.";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VISUALIZATIONS_SPACEOUT_DATA_GATHERING_METHOD_STR, spaceoutDataGatheringMethodStr.c_str());

	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PIXEL_TO_GRID_POINT_RATIO_GROUP, "Pixel-to-grid-point ratio");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SPACE_OUT_FACTOR_GROUP, "Spaceout factor");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SPACE_OUT_DATA_GATHERING_GROUP, "Data gathering method");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BETA_PRODUCT_OPTIONS_GROUP, "Beta product options");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_SPACE_OUT_OPTIMIZATIONS_WITH_BETA_PRODUCTS, "Use Spaceout optimizations with Beta products");
}

void CFmiVisualizationSettings::OnOK()
{
	UpdateValuesBackToDocument();
	CDialogEx::OnOK();
}

void CFmiVisualizationSettings::UpdateValuesBackToDocument()
{
	UpdateData(TRUE);

	// Tarkista mitk� asetukset ovat oikeasti muuttuneet
	int selectedSpaceoutDataGatheringMethod = 0; // itsSpaceoutDataGatheringMethodComboBox.GetCurSel();
	auto needsToUpdateMapViews = GetVisualizationSettings().updateFromDialog(GetPixelToGridPointRatioFromSlider(), itsUsePixelToGridPointRatioSafetyFeature, itsGlobalVisualizationSpaceoutGridSizeSlider.GetPos(), itsUseGlobalVisualizationSpaceoutFactorOptimization, selectedSpaceoutDataGatheringMethod, fUseSpaceoutOptimizationsWithBetaProducts);
	// Tee p��telm� oikeasti muuttuneista asetuksista ett� mit� likauksia ja n�yt�n p�ivityksi� pit�� tehd�
	if(needsToUpdateMapViews)
	{
		std::string updateMessage = "Visualization-Settings changed, updating all map-views";
		CatLog::logMessage(updateMessage, CatLog::Severity::Info, CatLog::Category::Visualization, true);
		auto* smartMetDocumentInterface = SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation();
		if(smartMetDocumentInterface)
		{
			smartMetDocumentInterface->UpdateMacroParamDataGridSizeAfterVisualizationOptimizationsChanged();
			// Laitetaan kaikki kartat likaiseksi ja nollataan kaikki macroParam cachet
			smartMetDocumentInterface->GetCombinedMapHandlerInterface().mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, true, true, true);
			// Asetetaan vain karttan�yt�t p�ivitett�viksi
			smartMetDocumentInterface->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
			// Lopullinen p�ivitysk�sky
			smartMetDocumentInterface->RefreshApplicationViewsAndDialogs(updateMessage);
		}
	}
	CheckInputAndDoWarningTexts();
	UpdateGlobalVisualizationSpaceoutGridSizeFromSliderToStringCtrl();
	UpdatePixelToGridPointRatioFromSliderToStringCtrl();
}


void CFmiVisualizationSettings::OnBnClickedButtonUpdate()
{
	UpdateValuesBackToDocument();
}

void CFmiVisualizationSettings::CheckInputAndDoWarningTexts()
{
	UpdateData(TRUE);
	itsPixelToGridPointRatioWarningStr = "";
	auto& visualizationSettings = GetVisualizationSettings();
	auto usePixelToGridPointRatioSafetyFeature_inUseNow = visualizationSettings.usePixelToGridPointRatioSafetyFeature();
	auto pixelToGridPointRatio_inUseNow = visualizationSettings.pixelToGridPointRatio();
	auto criticalLimit = visualizationSettings.criticalPixelToGridPointRatioLimit();
	auto pixelToGridPointValueFromSlider = GetPixelToGridPointRatioFromSlider();
	std::string warningMessage;
	if(!usePixelToGridPointRatioSafetyFeature_inUseNow)
	{
		warningMessage = "Warning: Use-Pixel-To-Grid-Point-Ratio-Safety-Feature is set off, may cause isoline visualization crashes...";
		if(itsUsePixelToGridPointRatioSafetyFeature)
		{
			// Dialogissa on ok asetukset, mutta niit� ei ole otettu k�ytt��n
			warningMessage += " (In dialog setting ok, but not applied yet)";
		}
	}
	else if(!itsUsePixelToGridPointRatioSafetyFeature)
	{
		warningMessage = "Warning: Use-Pixel-To-Grid-Point-Ratio-Safety-Feature is set off in dialog, but not applied yet...";
	}
	else if(pixelToGridPointRatio_inUseNow < criticalLimit)
	{
		warningMessage = "Warning: Pixel-To-Grid-Point-Ratio is under critical value ";
		warningMessage += NFmiValueString::GetStringWithMaxDecimalsSmartWay(criticalLimit, 1);
		warningMessage += ", which may cause isoline visualization crashes or other problems...";
		if(pixelToGridPointValueFromSlider >= criticalLimit)
		{
			// Dialogissa on ok asetukset, mutta niit� ei ole otettu k�ytt��n
			warningMessage += " (In dialog setting ok, but not applied yet)";
		}
	}
	else if(pixelToGridPointValueFromSlider < criticalLimit)
	{
		warningMessage = "Warning: Pixel-To-Grid-Point-Ratio in dialog is under critical value ";
		warningMessage += NFmiValueString::GetStringWithMaxDecimalsSmartWay(criticalLimit, 1);
		warningMessage += ", but not applied yet";
	}
	itsPixelToGridPointRatioWarningStr = CA2T(warningMessage.c_str());
	UpdateData(FALSE);
}

const NFmiColor g_LightWarningTextColor(0.87f, 0.44f, 0);
const NFmiColor g_NormalTextColor(0, 0, 0);

HBRUSH CFmiVisualizationSettings::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO_WARNING_STR)
	{
		CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsPixelToGridPointRatioWarningStr.IsEmpty());
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_PIXEL_TO_GRID_POINT_RATIO_VALUE_STR)
	{
		if(PixelToGridPointRatioValueIsInUse())
			CFmiWin32Helpers::SetColorForTextControl(pDC, g_NormalTextColor);
		else
			CFmiWin32Helpers::SetColorForTextControl(pDC, g_LightWarningTextColor);
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_GLOBAL_VISUALIZATION_SPACEOUT_FACTOR_RESULTS_STR)
	{
		if(GlobalVisualizationSpaceoutGridSizeValueIsInUse())
			CFmiWin32Helpers::SetColorForTextControl(pDC, g_NormalTextColor);
		else
			CFmiWin32Helpers::SetColorForTextControl(pDC, g_LightWarningTextColor);
	}
	return hbr;
}

void CFmiVisualizationSettings::OnBnClickedCheckVisualizationsUsePixelToGridPointRatioSafetyFeature()
{
	CheckInputAndDoWarningTexts();
}

void CFmiVisualizationSettings::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdatePixelToGridPointRatioFromSliderToStringCtrl();
	UpdateGlobalVisualizationSpaceoutGridSizeFromSliderToStringCtrl();
	CheckInputAndDoWarningTexts();

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

template <typename T>
static std::string to_string_with_fixed_precision(const T a_value, const int precision)
{
	std::ostringstream out;
	out.precision(precision);
	out << std::fixed << a_value;
	return out.str();
}

void CFmiVisualizationSettings::UpdatePixelToGridPointRatioFromSliderToStringCtrl()
{
	UpdateData(TRUE);
	std::string pixelToGridPointRatioValueStr = "Value: ";
	pixelToGridPointRatioValueStr += ::to_string_with_fixed_precision(GetPixelToGridPointRatioFromSlider(), 1);
	if(!PixelToGridPointRatioValueIsInUse())
	{
		pixelToGridPointRatioValueStr += " (in use ";
		pixelToGridPointRatioValueStr += ::to_string_with_fixed_precision(GetVisualizationSettings().pixelToGridPointRatio(), 1);
		pixelToGridPointRatioValueStr += ")";
	}
	itsPixelToGridPointRatioValueStr = CA2T(pixelToGridPointRatioValueStr.c_str());
	UpdateData(FALSE);
}

bool CFmiVisualizationSettings::PixelToGridPointRatioValueIsInUse()
{
	return GetPixelToGridPointRatioFromSlider() == GetVisualizationSettings().pixelToGridPointRatio();
}

bool CFmiVisualizationSettings::GlobalVisualizationSpaceoutGridSizeValueIsInUse()
{
	return itsGlobalVisualizationSpaceoutGridSizeSlider.GetPos() == GetVisualizationSettings().baseSpaceoutGridSize();
}

void CFmiVisualizationSettings::UpdateGlobalVisualizationSpaceoutGridSizeFromSliderToStringCtrl()
{
	UpdateData(TRUE);
	std::string globalVisualizationSpaceoutFactorResultsStr = "Value: ";
	auto globalVisualizationSpaceoutGridSize_fromSlider = itsGlobalVisualizationSpaceoutGridSizeSlider.GetPos();
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(globalVisualizationSpaceoutGridSize_fromSlider);
	globalVisualizationSpaceoutFactorResultsStr += "   which would result used data-grid size of ~ ";
	auto& visualizationSettings = GetVisualizationSettings();
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(globalVisualizationSpaceoutGridSize_fromSlider);
	globalVisualizationSpaceoutFactorResultsStr += "x";
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(globalVisualizationSpaceoutGridSize_fromSlider);
	globalVisualizationSpaceoutFactorResultsStr += "\nActual value in use is ";
	auto usedBaseGridSize = visualizationSettings.baseSpaceoutGridSize();
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(usedBaseGridSize);
	globalVisualizationSpaceoutFactorResultsStr += ", that results grid-size ~ ";
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(usedBaseGridSize);
	globalVisualizationSpaceoutFactorResultsStr += "x";
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(usedBaseGridSize);
	globalVisualizationSpaceoutFactorResultsStr += ")";
	itsGlobalVisualizationSpaceoutGridSizeResultsStr = CA2T(globalVisualizationSpaceoutFactorResultsStr.c_str());
	UpdateData(FALSE);
}

NFmiVisualizationSpaceoutSettings& CFmiVisualizationSettings::GetVisualizationSettings()
{
	return itsApplicationWinRegistry->VisualizationSpaceoutSettings();
}
