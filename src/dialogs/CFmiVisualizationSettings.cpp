// CFmiVisualizationSettings.cpp : implementation file
//

#include "stdafx.h"
#include "CFmiVisualizationSettings.h"
#include "FmiWin32Helpers.h"
#include "NFmiDictionaryFunction.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiVisualizationSpaceoutSettings.h"


// CFmiVisualizationSettings dialog

IMPLEMENT_DYNAMIC(CFmiVisualizationSettings, CDialog)

CFmiVisualizationSettings::CFmiVisualizationSettings(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD, pParent)
	, itsPixelToGridPointRatio(0)
	, itsGlobalVisualizationSpaceoutFactor(0)
	, itsUsePixelToGridPointRatioSafetyFeature(FALSE)
	, itsUseGlobalVisualizationSpaceoutFactorOptimization(FALSE)
{
}

CFmiVisualizationSettings::~CFmiVisualizationSettings()
{
}

void CFmiVisualizationSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO, itsPixelToGridPointRatio);
	DDX_Text(pDX, IDC_EDIT_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR, itsGlobalVisualizationSpaceoutFactor);
	DDX_Check(pDX, IDC_CHECK_VISUALIZATIONS_USE_PIXEL_TO_GRID_POINT_RATIO_SAFETY_FEATURE, itsUsePixelToGridPointRatioSafetyFeature);
	DDX_Check(pDX, IDC_CHECK_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR, itsUseGlobalVisualizationSpaceoutFactorOptimization);
	DDX_Control(pDX, IDC_COMBO_VISUALIZATIONS_SPACEOUT_DATA_GATHERING_METHOD, itsSpaceoutDataGatheringMethodComboBox);
}


BEGIN_MESSAGE_MAP(CFmiVisualizationSettings, CDialog)
END_MESSAGE_MAP()


// CFmiVisualizationSettings message handlers


BOOL CFmiVisualizationSettings::OnInitDialog()
{
	CDialog::OnInitDialog();

	itsVisualizationSpaceoutSettings = &SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->ApplicationWinRegistry().VisualizationSpaceoutSettings();
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	itsPixelToGridPointRatio = itsVisualizationSpaceoutSettings->pixelToGridPointRatio();
	itsUsePixelToGridPointRatioSafetyFeature = itsVisualizationSpaceoutSettings->usePixelToGridPointRatioSafetyFeature();
	itsGlobalVisualizationSpaceoutFactor = itsVisualizationSpaceoutSettings->globalVisualizationSpaceoutFactor();
	itsUseGlobalVisualizationSpaceoutFactorOptimization = itsVisualizationSpaceoutSettings->useGlobalVisualizationSpaceoutFactorOptimization();

	itsSpaceoutDataGatheringMethodComboBox.AddString(CA2T(::GetDictionaryString("Linear-interpolation").c_str()));
	itsSpaceoutDataGatheringMethodComboBox.SetCurSel(itsVisualizationSpaceoutSettings->spaceoutDataGatheringMethod());

	// Päivitetään muuttujien arvot dialogin kontrolleihin
	UpdateData(FALSE); 
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

	std::string pixelToGridPointRatioStr = "Pixel-To-Grid-Point-Ratio variable is safety feature for isoline visualizations used to prevent crashes\nIt's min value is ";
	pixelToGridPointRatioStr += std::to_string(itsVisualizationSpaceoutSettings->minPixelToGridPointRatioValue());
	pixelToGridPointRatioStr += " and max is ";
	pixelToGridPointRatioStr += std::to_string(itsVisualizationSpaceoutSettings->maxPixelToGridPointRatioValue());
	pixelToGridPointRatioStr += " (too dense data will be 'thinned')";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VISUALIZATIONS_PIXEL_TO_GRID_POINT_RATIO_STR, pixelToGridPointRatioStr.c_str());

	std::string usePixelToGridPointRatioSafetyFeatureStr = "This safety feature is on by default on application restart, so you can't disable it permanently";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_VISUALIZATIONS_USE_PIXEL_TO_GRID_POINT_RATIO_SAFETY_FEATURE, usePixelToGridPointRatioSafetyFeatureStr.c_str());

	std::string globalVisualizationSpaceoutFactorStr = "Global-Visualization-Spaceout-Factor variable is used to speed up map-view visualizations\nSetting factor between ";
	globalVisualizationSpaceoutFactorStr += std::to_string(itsVisualizationSpaceoutSettings->minVisualizationSpaceoutFactor());
	globalVisualizationSpaceoutFactorStr += " - ";
	globalVisualizationSpaceoutFactorStr += std::to_string(itsVisualizationSpaceoutSettings->maxVisualizationSpaceoutFactor());
	globalVisualizationSpaceoutFactorStr += " results visualized data grid size around 400x400 - 40x40";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR_STR, globalVisualizationSpaceoutFactorStr.c_str());

	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_GLOBAL_VISUALIZATIONS_SPACEOUT_FACTOR, "Use spaceout optimizations");

	std::string spaceoutDataGatheringMethodStr = "Spaceout-Data-Gathering-Method setting (selection not implemented yet)\nUsed optimized grid data gathering methods: 1) linear interpolation, 2) median filter";
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VISUALIZATIONS_SPACEOUT_DATA_GATHERING_METHOD_STR, spaceoutDataGatheringMethodStr.c_str());
}


void CFmiVisualizationSettings::OnOK()
{
	UpdateData(TRUE);
	itsVisualizationSpaceoutSettings->pixelToGridPointRatio(itsPixelToGridPointRatio);
	itsVisualizationSpaceoutSettings->usePixelToGridPointRatioSafetyFeature(itsUsePixelToGridPointRatioSafetyFeature);
	itsVisualizationSpaceoutSettings->globalVisualizationSpaceoutFactor(itsGlobalVisualizationSpaceoutFactor);
	itsVisualizationSpaceoutSettings->useGlobalVisualizationSpaceoutFactorOptimization(itsUseGlobalVisualizationSpaceoutFactorOptimization);
	itsVisualizationSpaceoutSettings->spaceoutDataGatheringMethod(itsSpaceoutDataGatheringMethodComboBox.GetCurSel());

	CDialog::OnOK();
}
