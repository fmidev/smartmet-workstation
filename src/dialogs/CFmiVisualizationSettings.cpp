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


// CFmiVisualizationSettings dialog

const NFmiViewPosRegistryInfo CFmiVisualizationSettings::s_ViewPosRegistryInfo(CRect(320, 270, 970, 680), "\\VisualizationSettingsDlg");

IMPLEMENT_DYNAMIC(CFmiVisualizationSettings, CDialogEx)

CFmiVisualizationSettings::CFmiVisualizationSettings(CWnd* pParent)
	: CDialogEx(IDD, pParent)
	, itsPixelToGridPointRatio(0)
	, itsGlobalVisualizationSpaceoutFactor(0)
	, itsUsePixelToGridPointRatioSafetyFeature(FALSE)
	, itsUseGlobalVisualizationSpaceoutFactorOptimization(FALSE)
	, itsPixelToGridPointRatioWarningStr(_T(""))
{
}

CFmiVisualizationSettings::~CFmiVisualizationSettings()
{
}

void CFmiVisualizationSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO, itsPixelToGridPointRatio);
	DDX_Text(pDX, IDC_EDIT_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR, itsGlobalVisualizationSpaceoutFactor);
	DDX_Check(pDX, IDC_CHECK_VISUALIZATIONS_USE_PIXEL_TO_GRID_POINT_RATIO_SAFETY_FEATURE, itsUsePixelToGridPointRatioSafetyFeature);
	DDX_Check(pDX, IDC_CHECK_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR, itsUseGlobalVisualizationSpaceoutFactorOptimization);
	DDX_Control(pDX, IDC_COMBO_VISUALIZATIONS_SPACEOUT_DATA_GATHERING_METHOD, itsSpaceoutDataGatheringMethodComboBox);
	DDX_Text(pDX, IDC_STATIC_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO_WARNING_STR, itsPixelToGridPointRatioWarningStr);
}


BEGIN_MESSAGE_MAP(CFmiVisualizationSettings, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CFmiVisualizationSettings::OnBnClickedButtonUpdate)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_VISUALIZATIONS_USE_PIXEL_TO_GRID_POINT_RATIO_SAFETY_FEATURE, &CFmiVisualizationSettings::OnBnClickedCheckVisualizationsUsePixelToGridPointRatioSafetyFeature)
	ON_EN_CHANGE(IDC_EDIT_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO, &CFmiVisualizationSettings::OnEnChangeEditVisualizationsPixelToGridPointRatio)
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
	itsPixelToGridPointRatio = visualizationSpaceoutSettings.pixelToGridPointRatio();
	itsUsePixelToGridPointRatioSafetyFeature = visualizationSpaceoutSettings.usePixelToGridPointRatioSafetyFeature();
	itsGlobalVisualizationSpaceoutFactor = visualizationSpaceoutSettings.globalVisualizationSpaceoutFactor();
	itsUseGlobalVisualizationSpaceoutFactorOptimization = visualizationSpaceoutSettings.useGlobalVisualizationSpaceoutFactorOptimization();

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
	auto needsToUpdateMapViews = itsApplicationWinRegistry->VisualizationSpaceoutSettings().updateFromDialog(itsPixelToGridPointRatio, itsUsePixelToGridPointRatioSafetyFeature, itsGlobalVisualizationSpaceoutFactor, itsUseGlobalVisualizationSpaceoutFactorOptimization, selectedSpaceoutDataGatheringMethod);
	// Tee päätelmä oikeasti muuttuneista asetuksista että mitä likauksia ja näytön päivityksiä pitää tehdä
	if(needsToUpdateMapViews)
	{
		std::string updateMessage = "Visualization-Settings changed, updating all map-views";
		CatLog::logMessage(updateMessage, CatLog::Severity::Info, CatLog::Category::Visualization, true);
		auto* smartMetDocumentInterface = SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation();
		if(smartMetDocumentInterface)
		{
			// Laitetaan kaikki kartat likaiseksi ja nollataan kaikki macroParam cachet
			smartMetDocumentInterface->GetCombinedMapHandlerInterface().mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, true, true, true);
			// Asetetaan vain karttanäytöt päivitettäviksi
			smartMetDocumentInterface->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
			// Lopullinen päivityskäsky
			smartMetDocumentInterface->RefreshApplicationViewsAndDialogs(updateMessage);
		}
	}
}


void CFmiVisualizationSettings::OnBnClickedButtonUpdate()
{
	UpdateValuesBackToDocument();
}

void CFmiVisualizationSettings::CheckInputAndDoWarningTexts()
{
	UpdateData(TRUE);
	itsPixelToGridPointRatioWarningStr = "";
	auto criticalLimit = itsApplicationWinRegistry->VisualizationSpaceoutSettings().criticalPixelToGridPointRatioLimit();
	if(!itsUsePixelToGridPointRatioSafetyFeature)
	{
		std::string warningMessage = "Warning: Use-Pixel-To-Grid-Point-Ratio-Safety-Feature is set off, may cause isoline visualization crashes...";
		itsPixelToGridPointRatioWarningStr = CA2T(warningMessage.c_str());
	}
	else if(itsPixelToGridPointRatio < criticalLimit)
	{
		std::string warningMessage = "Warning: Pixel-To-Grid-Point-Ratio is under critical value ";
		warningMessage += NFmiValueString::GetStringWithMaxDecimalsSmartWay(criticalLimit, 1);
		warningMessage += ", may cause isoline visualization crashes or other errors...";
		itsPixelToGridPointRatioWarningStr = CA2T(warningMessage.c_str());
	}
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

void CFmiVisualizationSettings::OnEnChangeEditVisualizationsPixelToGridPointRatio()
{
	CheckInputAndDoWarningTexts();
}
