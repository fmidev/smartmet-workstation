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
#include <numeric>


// CFmiVisualizationSettings dialog

const NFmiViewPosRegistryInfo CFmiVisualizationSettings::s_ViewPosRegistryInfo(CRect(320, 270, 970, 680), "\\VisualizationSettingsDlg");

IMPLEMENT_DYNAMIC(CFmiVisualizationSettings, CDialogEx)

CFmiVisualizationSettings::CFmiVisualizationSettings(CWnd* pParent)
	: CDialogEx(IDD, pParent)
	, itsPixelToGridPointRatio_valueFromSlider(0)
	, itsGlobalVisualizationSpaceoutFactor_valueFromSlider(0)
	, itsUsePixelToGridPointRatioSafetyFeature(FALSE)
	, itsUseGlobalVisualizationSpaceoutFactorOptimization(FALSE)
	, itsPixelToGridPointRatioWarningStr(_T(""))
	, itsPixelToGridPointRatioValueStr(_T(""))
	, itsGlobalVisualizationSpaceoutFactorResultsStr(_T(""))
{
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
	DDX_Control(pDX, IDC_SLIDER_GLOBAL_VISUALIZATION_SPACEOUT_FACTOR, itsGlobalVisualizationSpaceoutFactorSlider);
	DDX_Text(pDX, IDC_STATIC_GLOBAL_VISUALIZATION_SPACEOUT_FACTOR_RESULTS_STR, itsGlobalVisualizationSpaceoutFactorResultsStr);
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
	// Tämä metodi pitää toteuttaa, vaikka tyhjänä, jotta luokka menisi SmartMetissa käytettyjen näytön talletus frameworkin läpi
}

BOOL CFmiVisualizationSettings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	itsApplicationWinRegistry = &SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->ApplicationWinRegistry();
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	auto& visualizationSpaceoutSettings = itsApplicationWinRegistry->VisualizationSpaceoutSettings();
	InitPixelToGridPointRatioSlider();
	InitGlobalVisualizationSpaceoutFactorSlider();

	itsSpaceoutDataGatheringMethodComboBox.AddString(CA2T(::GetDictionaryString("Linear-interpolation").c_str()));
	itsSpaceoutDataGatheringMethodComboBox.AddString(CA2T(::GetDictionaryString("Median-filter").c_str()));
	itsSpaceoutDataGatheringMethodComboBox.SetCurSel(0); // itsVisualizationSpaceoutSettings->spaceoutDataGatheringMethod());

	// Tee paikan asetus vasta tooltipin alustuksen jälkeen, niin se toimii ilman OnSize-kutsua.
	std::string errorBaseStr("Error in CFmiVisualizationSettings::OnInitDialog while reading dialog size and position values");
	CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(*itsApplicationWinRegistry, this, true, errorBaseStr, 0);

	// Päivitetään muuttujien arvot dialogin kontrolleihin
	UpdateData(FALSE); 
	// Tehdään alustuksen jälkeen testit varoituksien suhteen
	CheckInputAndDoWarningTexts();
	// return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
	return TRUE;  
}

void CFmiVisualizationSettings::InitPixelToGridPointRatioSlider()
{
	auto& visualizationSpaceoutSettings = itsApplicationWinRegistry->VisualizationSpaceoutSettings();
	auto pixelToGridPointRatio = visualizationSpaceoutSettings.pixelToGridPointRatio();
	auto minPixelToGridPointRatio = visualizationSpaceoutSettings.minPixelToGridPointRatioValue();
	auto maxPixelToGridPointRatio = visualizationSpaceoutSettings.maxPixelToGridPointRatioValue();
	// Tämä vastaa minPixelToGridPointRatio arvoa
	int startIndex = 0; 
	// Tämä vastaa maxPixelToGridPointRatio arvoa
	int endIndex = boost::math::iround((maxPixelToGridPointRatio - minPixelToGridPointRatio) * 10);
	itsPixelToGridPointRatioSlider.SetRange(startIndex, endIndex);
	itsPixelToGridPointRatioSlider.SetPos(CalcPixelToGridPointRatioSliderPosition(pixelToGridPointRatio));
	itsUsePixelToGridPointRatioSafetyFeature = visualizationSpaceoutSettings.usePixelToGridPointRatioSafetyFeature();
	UpdateData(FALSE);
	UpdatePixelToGridPointRatioFromSliderToStringCtrl();
}

void CFmiVisualizationSettings::InitGlobalVisualizationSpaceoutFactorSlider()
{
	auto& visualizationSpaceoutSettings = itsApplicationWinRegistry->VisualizationSpaceoutSettings();
	auto globalVisualizationSpaceoutFactor = visualizationSpaceoutSettings.globalVisualizationSpaceoutFactor();
	auto minVisualizationSpaceoutFactor = visualizationSpaceoutSettings.minVisualizationSpaceoutFactor();
	auto maxVisualizationSpaceoutFactor = visualizationSpaceoutSettings.maxVisualizationSpaceoutFactor();
	// Tämä vastaa minVisualizationSpaceoutFactor arvoa
	int startIndex = 0;
	// Tämä vastaa maxVisualizationSpaceoutFactor arvoa
	int endIndex = boost::math::iround((maxVisualizationSpaceoutFactor - minVisualizationSpaceoutFactor) * 10);
	itsGlobalVisualizationSpaceoutFactorSlider.SetRange(startIndex, endIndex);
	itsGlobalVisualizationSpaceoutFactorSlider.SetPos(CalcGlobalVisualizationSpaceoutFactorSliderPosition(globalVisualizationSpaceoutFactor));
	itsUseGlobalVisualizationSpaceoutFactorOptimization = visualizationSpaceoutSettings.useGlobalVisualizationSpaceoutFactorOptimization();
	UpdateData(FALSE);
	UpdateGlobalVisualizationSpaceoutFactorFromSliderToStringCtrl();
}

int CFmiVisualizationSettings::CalcPixelToGridPointRatioSliderPosition(double value)
{
	auto& visualizationSpaceoutSettings = itsApplicationWinRegistry->VisualizationSpaceoutSettings();
	auto minPixelToGridPointRatio = visualizationSpaceoutSettings.minPixelToGridPointRatioValue();
	auto maxPixelToGridPointRatio = visualizationSpaceoutSettings.maxPixelToGridPointRatioValue();
	if(value <= minPixelToGridPointRatio)
		return 0;
	if(value >= maxPixelToGridPointRatio)
		return itsPixelToGridPointRatioSlider.GetRangeMax();

	return boost::math::iround((value - minPixelToGridPointRatio) * 10);
}

int CFmiVisualizationSettings::CalcGlobalVisualizationSpaceoutFactorSliderPosition(double value)
{
	auto& visualizationSpaceoutSettings = itsApplicationWinRegistry->VisualizationSpaceoutSettings();
	auto minVisualizationSpaceoutFactor = visualizationSpaceoutSettings.minVisualizationSpaceoutFactor();
	auto maxVisualizationSpaceoutFactor = visualizationSpaceoutSettings.maxVisualizationSpaceoutFactor();
	if(value <= minVisualizationSpaceoutFactor)
		return 0;
	if(value >= maxVisualizationSpaceoutFactor)
		return itsGlobalVisualizationSpaceoutFactorSlider.GetRangeMax();

	return boost::math::iround((value - minVisualizationSpaceoutFactor) * 10);
}

static double GetActualValueFromSlider(CSliderCtrl &slider, double minValue)
{
	auto sliderPos = slider.GetPos();
	auto value = (sliderPos / 10.) + minValue;
	if(std::abs(value - std::round(value)) <= std::numeric_limits<double>::epsilon() * 3)
	{
		value = std::round(value);
	}
	return value;
}


double CFmiVisualizationSettings::GetPixelToGridPointRatioFromSlider()
{
	return GetActualValueFromSlider(itsPixelToGridPointRatioSlider, itsApplicationWinRegistry->VisualizationSpaceoutSettings().minPixelToGridPointRatioValue());
}

double CFmiVisualizationSettings::GetGlobalVisualizationSpaceoutFactorFromSlider()
{
	return GetActualValueFromSlider(itsGlobalVisualizationSpaceoutFactorSlider, itsApplicationWinRegistry->VisualizationSpaceoutSettings().minVisualizationSpaceoutFactor());
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiVisualizationSettings::InitDialogTexts()
{
    SetWindowText(CA2T(::GetDictionaryString("Visualization settings").c_str()));
    CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
    CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");

	auto& visualizationSpaceoutSettings = itsApplicationWinRegistry->VisualizationSpaceoutSettings();

	std::string pixelToGridPointRatioStr = "Pixel-To-Grid-Point-Ratio is safety feature for isoline visualizations, used to\nprevent crashes. It's min value is ";
	pixelToGridPointRatioStr += std::to_string(boost::math::iround(visualizationSpaceoutSettings.minPixelToGridPointRatioValue()));
	pixelToGridPointRatioStr += " and max value is ";
	pixelToGridPointRatioStr += std::to_string(boost::math::iround(visualizationSpaceoutSettings.maxPixelToGridPointRatioValue()));
	pixelToGridPointRatioStr += " (too high-res data will\nbe 'thinned' to lower resolution grid).";
	pixelToGridPointRatioStr += " 'Critical' limit for this is ";
	pixelToGridPointRatioStr += std::to_string(boost::math::iround(visualizationSpaceoutSettings.criticalPixelToGridPointRatioLimit()));
	pixelToGridPointRatioStr += ", if value is set\nlower, warning messages are issued in Log viewer and Map-view tooltips.";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO_STR, pixelToGridPointRatioStr.c_str());

	std::string usePixelToGridPointRatioSafetyFeatureStr = "This safety feature is on by default when application starts,\nso you can disable it for now, but not permanently!";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_VISUALIZATIONS_USE_PIXEL_TO_GRID_POINT_RATIO_SAFETY_FEATURE, usePixelToGridPointRatioSafetyFeatureStr.c_str());

	std::string globalVisualizationSpaceoutFactorStr = "Global-Visualization-Spaceout-Factor variable is used to speed up map-view\nvisualizations. Setting factor between ";
	globalVisualizationSpaceoutFactorStr += std::to_string(boost::math::iround(visualizationSpaceoutSettings.minVisualizationSpaceoutFactor()));
	globalVisualizationSpaceoutFactorStr += " - ";
	globalVisualizationSpaceoutFactorStr += std::to_string(boost::math::iround(visualizationSpaceoutSettings.maxVisualizationSpaceoutFactor()));
	globalVisualizationSpaceoutFactorStr += " yields visualized data grid sizes\n~ ";
	auto maxGridSizeStr = std::to_string(boost::math::iround(visualizationSpaceoutSettings.maxVisualizationSpaceoutGridSize()));
	globalVisualizationSpaceoutFactorStr += maxGridSizeStr;
	globalVisualizationSpaceoutFactorStr += "x";
	globalVisualizationSpaceoutFactorStr += maxGridSizeStr;
	globalVisualizationSpaceoutFactorStr += " - ";
	auto minGridSizeStr = std::to_string(boost::math::iround(visualizationSpaceoutSettings.minVisualizationSpaceoutGridSize()));
	globalVisualizationSpaceoutFactorStr += minGridSizeStr;
	globalVisualizationSpaceoutFactorStr += "x";
	globalVisualizationSpaceoutFactorStr += minGridSizeStr;
	globalVisualizationSpaceoutFactorStr += ". Low-res data is faster to draw but it loses information.";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR_STR, globalVisualizationSpaceoutFactorStr.c_str());

	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR, "Use spaceout optimizations");

	std::string spaceoutDataGatheringMethodStr = "Spaceout-Data-Gathering-Method setting, Median-filter is not implemented yet.\nUsed grid data gathering methods: 1) Linear-interpolation, 2) Median-filter.";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VISUALIZATIONS_SPACEOUT_DATA_GATHERING_METHOD_STR, spaceoutDataGatheringMethodStr.c_str());
}

void CFmiVisualizationSettings::OnOK()
{
	UpdateValuesBackToDocument();
	CDialogEx::OnOK();
}

void CFmiVisualizationSettings::UpdateValuesBackToDocument()
{
	UpdateData(TRUE);

	// Tarkista mitkä asetukset ovat oikeasti muuttuneet
	int selectedSpaceoutDataGatheringMethod = 0; // itsSpaceoutDataGatheringMethodComboBox.GetCurSel();
	auto needsToUpdateMapViews = itsApplicationWinRegistry->VisualizationSpaceoutSettings().updateFromDialog(GetPixelToGridPointRatioFromSlider(), itsUsePixelToGridPointRatioSafetyFeature, GetGlobalVisualizationSpaceoutFactorFromSlider(), itsUseGlobalVisualizationSpaceoutFactorOptimization, selectedSpaceoutDataGatheringMethod);
	// Tee päätelmä oikeasti muuttuneista asetuksista että mitä likauksia ja näytön päivityksiä pitää tehdä
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
			// Asetetaan vain karttanäytöt päivitettäviksi
			smartMetDocumentInterface->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
			// Lopullinen päivityskäsky
			smartMetDocumentInterface->RefreshApplicationViewsAndDialogs(updateMessage);
		}
	}
	CheckInputAndDoWarningTexts();
	UpdateGlobalVisualizationSpaceoutFactorFromSliderToStringCtrl();
}


void CFmiVisualizationSettings::OnBnClickedButtonUpdate()
{
	UpdateValuesBackToDocument();
}

void CFmiVisualizationSettings::CheckInputAndDoWarningTexts()
{
	UpdateData(TRUE);
	itsPixelToGridPointRatioWarningStr = "";
	auto& visSettings = itsApplicationWinRegistry->VisualizationSpaceoutSettings();
	auto usePixelToGridPointRatioSafetyFeature_inUseNow = visSettings.usePixelToGridPointRatioSafetyFeature();
	auto pixelToGridPointRatio_inUseNow = visSettings.pixelToGridPointRatio();
	auto criticalLimit = visSettings.criticalPixelToGridPointRatioLimit();
	auto pixelToGridPointValueFromSlider = GetPixelToGridPointRatioFromSlider();
	std::string warningMessage;
	if(!usePixelToGridPointRatioSafetyFeature_inUseNow)
	{
		warningMessage = "Warning: Use-Pixel-To-Grid-Point-Ratio-Safety-Feature is set off, may cause isoline visualization crashes...";
		if(itsUsePixelToGridPointRatioSafetyFeature)
		{
			// Dialogissa on ok asetukset, mutta niitä ei ole otettu käyttöön
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
			// Dialogissa on ok asetukset, mutta niitä ei ole otettu käyttöön
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

HBRUSH CFmiVisualizationSettings::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO_WARNING_STR)
	{
		CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsPixelToGridPointRatioWarningStr.IsEmpty());
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
	UpdateGlobalVisualizationSpaceoutFactorFromSliderToStringCtrl();
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
	itsPixelToGridPointRatioValueStr = CA2T(pixelToGridPointRatioValueStr.c_str());
	UpdateData(FALSE);
}

void CFmiVisualizationSettings::UpdateGlobalVisualizationSpaceoutFactorFromSliderToStringCtrl()
{
	UpdateData(TRUE);
	std::string globalVisualizationSpaceoutFactorResultsStr = "Value: ";
	auto globalVisualizationSpaceoutFactor_fromSlider = GetGlobalVisualizationSpaceoutFactorFromSlider();
	globalVisualizationSpaceoutFactorResultsStr += ::to_string_with_fixed_precision(globalVisualizationSpaceoutFactor_fromSlider, 1);
	globalVisualizationSpaceoutFactorResultsStr += "   which would result used data-grid size of ~ ";
	auto& visSettings = itsApplicationWinRegistry->VisualizationSpaceoutSettings();
	auto baseGridSize = boost::math::iround(visSettings.calcBaseOptimizedGridSize(globalVisualizationSpaceoutFactor_fromSlider));
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(baseGridSize);
	globalVisualizationSpaceoutFactorResultsStr += "x";
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(baseGridSize);
	globalVisualizationSpaceoutFactorResultsStr += "\nActual value in use is ";
	auto usedFactor = visSettings.globalVisualizationSpaceoutFactor();
	globalVisualizationSpaceoutFactorResultsStr += ::to_string_with_fixed_precision(usedFactor, 1);
	auto usedBaseGridSize = boost::math::iround(visSettings.calcBaseOptimizedGridSize(usedFactor));
	globalVisualizationSpaceoutFactorResultsStr += ", that results grid-size ~ ";
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(usedBaseGridSize);
	globalVisualizationSpaceoutFactorResultsStr += "x";
	globalVisualizationSpaceoutFactorResultsStr += std::to_string(usedBaseGridSize);
	globalVisualizationSpaceoutFactorResultsStr += ")";
	itsGlobalVisualizationSpaceoutFactorResultsStr = CA2T(globalVisualizationSpaceoutFactorResultsStr.c_str());
	UpdateData(FALSE);
}
