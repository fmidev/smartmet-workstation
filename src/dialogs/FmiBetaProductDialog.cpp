#include "stdafx.h"
#include "FmiBetaProductDialog.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "afxdialogex.h"

#include "NFmiDictionaryFunction.h"
#include "CloneBitmap.h"
#include "FmiWin32Helpers.h"
#include "NFmiValueString.h"
#include "XFolderDialog.h"
#include "FmiGdiPlusHelpers.h"
#include "NFmiFileSystem.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiBetaProductSystem.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiViewSettingMacro.h"
#include "NFmiSatelliteImageCacheSystem.h"
#include "NFmiSatelView.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "catlog/catlog.h"
#include "SmartMetDocumentInterface.h"
#include "CtrlViewFastInfoFunctions.h"
#include "SpecialDesctopIndex.h"
#include "NFmiLocationSelectionTool.h"

#include <boost/algorithm/string.hpp>
#include <fstream>
#include "boost/math/special_functions/round.hpp"
#include "execute-command-in-separate-process.h"

const static int gMissingRowIndex = -1;

// CFmiBetaProductDialog dialog

IMPLEMENT_DYNAMIC(CFmiBetaProductDialog, CTabPageSSL) //CDialogEx)

CFmiBetaProductDialog::CFmiBetaProductDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
: CTabPageSSL(CFmiBetaProductDialog::IDD, pParent)
, itsSmartMetDocumentInterface(smartMetDocumentInterface)
, itsBetaProductionSystem(&smartMetDocumentInterface->BetaProductionSystem())
, itsImageStoragePathU_(_T(""))
, itsFileNameTemplateU_(_T(""))
, fFileNameTemplateContainsValidTime(false)
, itsTimeRangeInfoTextU_(_T(""))
, itsTimeLengthInHoursStringU_(_T("15"))
, itsTimeStepInMinutesStringU_(_T("60"))
, itsRowIndexListStringU_(_T(""))
, itsRowSubdirectoryTemplateStringU_(_T(""))
, itsRowIndexListInfoTextU_(_T(""))
, itsSynopStationIdListTextU_(_T(""))
, itsBetaProduct(std::make_shared<NFmiBetaProduct>())
, itsImageProductionTextU_(_T(""))
, itsSelectedViewIndex(0)
, itsViewMacroErrorTextU_(_T(""))
, itsViewMacroPathU_(_T(""))
, itsWebSiteTitleStringU_(_T(""))
, itsWebSiteDescriptionStringU_(_T(""))
, itsFileNameTemplateStampsStringU_(_T(""))
, itsCommandLineStringU_(_T(""))
, itsDueAutomations(nullptr)
, fUseUtcTimesInTimeBox(FALSE)
, fUseAutoFileNames(FALSE)
, fDisplayRuntimeInfo(FALSE)
, itsTotalImagesGenerated(0)
, itsBetaProductFullFilePath()
, itsBetaProductNameU_(_T(""))
{

}

CFmiBetaProductDialog::~CFmiBetaProductDialog()
{
}

void CFmiBetaProductDialog::DoDataExchange(CDataExchange* pDX)
{
    CTabPageSSL::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_IMAGE_DIRECTORY, itsImageStoragePathU_);
    DDX_Text(pDX, IDC_EDIT_FILE_NAME_TEMPLATE, itsFileNameTemplateU_);
    DDX_Text(pDX, IDC_STATIC_DEDUCED_INFO_OF_IMAGE_PRODUCTION, itsTimeRangeInfoTextU_);
    DDX_Text(pDX, IDC_EDIT_TIME_LENGTH_IN_HOURS, itsTimeLengthInHoursStringU_);
    DDX_Text(pDX, IDC_EDIT_TIME_STEP_IN_MINUTES, itsTimeStepInMinutesStringU_);
    DDX_Text(pDX, IDC_EDIT_ROW_INDEX_LIST, itsRowIndexListStringU_);
    DDX_Text(pDX, IDC_EDIT_SYNOP_STATION_ID_STRING, itsSynopStationIdListTextU_);
    DDX_Text(pDX, IDC_EDIT_ROW_SUBDIRECTORY_TEMPLATE_STRING, itsRowSubdirectoryTemplateStringU_);
    DDX_Text(pDX, IDC_STATIC_DEDUCED_INFO_FROM_USED_ROWS, itsRowIndexListInfoTextU_);
    DDX_Control(pDX, IDC_PROGRESS_IMAGE_GENERATION, itsImageGenerationProgressControl);
    DDX_Control(pDX, IDC_BUTTON_GENERATE_IMAGES, itsGenerateImagesButton);
    DDX_Text(pDX, IDC_STATIC_IMAGE_PRODUCTION_TEXT, itsImageProductionTextU_);
    DDX_Radio(pDX, IDC_RADIO_MAIN_MAP_VIEW, itsSelectedViewIndex);
    DDX_Text(pDX, IDC_STATIC_VIEW_MACRO_ERROR_TEXT, itsViewMacroErrorTextU_);
    DDX_Text(pDX, IDC_EDIT_VIEW_MACRO_PATH, itsViewMacroPathU_);
    DDX_Text(pDX, IDC_EDIT_WEB_TITLE_STRING, itsWebSiteTitleStringU_);
    DDX_Text(pDX, IDC_EDIT_WEB_DESCRIPTION_STRING, itsWebSiteDescriptionStringU_);
    DDX_Text(pDX, IDC_STATIC_IMAGE_FILE_NAME_TEMPLATE_STAMPS_TEXT, itsFileNameTemplateStampsStringU_);
    DDX_Control(pDX, IDC_BUTTON_SAVE_BETA_PRODUCT, itsSaveButton);
    DDX_Control(pDX, IDC_BUTTON_SAVE_AS_BETA_PRODUCT, itsSaveAsButton);
    DDX_Text(pDX, IDC_EDIT_COMMAND_LINE_STRING, itsCommandLineStringU_);
    DDX_Control(pDX, IDC_COMBO_TIME_BOX_LOCATION_SELECTOR, itsTimeBoxLocationSelector);
    DDX_Check(pDX, IDC_CHECK_TIME_BOX_USE_UTC_TIME, fUseUtcTimesInTimeBox);
    DDX_Check(pDX, IDC_CHECK_USE_AUTO_FILE_NAMES, fUseAutoFileNames);
    DDX_Control(pDX, IDC_COMBO_PARAM_BOX_LOCATION_SELECTOR, itsParamBoxLocationSelector);
    DDX_Check(pDX, IDC_CHECK_DISPLAY_RUNTIME_INFO, fDisplayRuntimeInfo);
    DDX_Text(pDX, IDC_STATIC_BETA_PRODUCT_NAME, itsBetaProductNameU_);
}


BEGIN_MESSAGE_MAP(CFmiBetaProductDialog, CTabPageSSL) //CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_IMAGE_DIRECTORY_BROWSE, &CFmiBetaProductDialog::OnBnClickedButtonImageDirectoryBrowse)
    ON_BN_CLICKED(IDC_BUTTON_GENERATE_IMAGES, &CFmiBetaProductDialog::OnBnClickedButtonGenerateImages)
    ON_EN_CHANGE(IDC_EDIT_TIME_STEP_IN_MINUTES, &CFmiBetaProductDialog::OnEnChangeEditTimeStepInMinutes)
    ON_EN_CHANGE(IDC_EDIT_TIME_LENGTH_IN_HOURS, &CFmiBetaProductDialog::OnEnChangeEditTimeLengthInHours)
    ON_EN_CHANGE(IDC_EDIT_ROW_INDEX_LIST, &CFmiBetaProductDialog::OnEnChangeEditRowIndexList)
    ON_EN_CHANGE(IDC_EDIT_ROW_SUBDIRECTORY_TEMPLATE_STRING, &CFmiBetaProductDialog::OnEnChangeEditRowSubdirectoryTemplateString)
    ON_EN_CHANGE(IDC_EDIT_FILE_NAME_TEMPLATE, &CFmiBetaProductDialog::OnEnChangeEditFileNameTemplate)
    ON_BN_CLICKED(IDC_RADIO_MAIN_MAP_VIEW, &CFmiBetaProductDialog::OnBnClickedRadioMainMapView)
    ON_BN_CLICKED(IDC_RADIO__MAP_VIEW_2, &CFmiBetaProductDialog::OnBnClickedRadioMapView2)
    ON_BN_CLICKED(IDC_RADIO__MAP_VIEW_3, &CFmiBetaProductDialog::OnBnClickedRadioMapView3)
    ON_BN_CLICKED(IDC_BUTTON_VIEW_MACRO_PATH_BROWSE, &CFmiBetaProductDialog::OnBnClickedButtonViewMacroPathBrowse)
    ON_EN_CHANGE(IDC_EDIT_VIEW_MACRO_PATH, &CFmiBetaProductDialog::OnEnChangeEditViewMacroPath)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_SAVE_BETA_PRODUCT, &CFmiBetaProductDialog::OnBnClickedButtonSaveBetaProduct)
    ON_BN_CLICKED(IDC_BUTTON_LOAD_BETA_PRODUCT, &CFmiBetaProductDialog::OnBnClickedButtonLoadBetaProduct)
    ON_EN_CHANGE(IDC_EDIT_IMAGE_DIRECTORY, &CFmiBetaProductDialog::OnEnChangeEditImageDirectory)
    ON_EN_CHANGE(IDC_EDIT_WEB_TITLE_STRING, &CFmiBetaProductDialog::OnEnChangeEditWebTitleString)
    ON_EN_CHANGE(IDC_EDIT_WEB_DESCRIPTION_STRING, &CFmiBetaProductDialog::OnEnChangeEditWebDescriptionString)
    ON_WM_DESTROY()
    ON_EN_CHANGE(IDC_EDIT_COMMAND_LINE_STRING, &CFmiBetaProductDialog::OnEnChangeEditCommandLineString)
    ON_CBN_SELCHANGE(IDC_COMBO_TIME_BOX_LOCATION_SELECTOR, &CFmiBetaProductDialog::OnCbnSelchangeComboTimeBoxLocationSelector)
    ON_BN_CLICKED(IDC_CHECK_TIME_BOX_USE_UTC_TIME, &CFmiBetaProductDialog::OnBnClickedCheckTimeBoxUseUtcTime)
    ON_BN_CLICKED(IDC_CHECK_USE_AUTO_FILE_NAMES, &CFmiBetaProductDialog::OnBnClickedCheckUseAutoFileNames)
    ON_CBN_SELCHANGE(IDC_COMBO_PARAM_BOX_LOCATION_SELECTOR, &CFmiBetaProductDialog::OnCbnSelchangeComboParamBoxLocationSelector)
    ON_BN_CLICKED(IDC_CHECK_DISPLAY_RUNTIME_INFO, &CFmiBetaProductDialog::OnBnClickedCheckDisplayRuntimeInfo)
    ON_BN_CLICKED(IDC_RADIO_TIME_SERIAL_VIEW, &CFmiBetaProductDialog::OnBnClickedRadioTimeSerialView)
    ON_BN_CLICKED(IDC_RADIO_SOUNDING_VIEW, &CFmiBetaProductDialog::OnBnClickedRadioSoundingView)
    ON_BN_CLICKED(IDC_RADIO_CROSS_SECTION_VIEW, &CFmiBetaProductDialog::OnBnClickedRadioCrossSectionView)
    ON_EN_CHANGE(IDC_EDIT_SYNOP_STATION_ID_STRING, &CFmiBetaProductDialog::OnEnChangeEditSynopStationIdString)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_AS_BETA_PRODUCT, &CFmiBetaProductDialog::OnBnClickedButtonSaveAsBetaProduct)
END_MESSAGE_MAP()


// CFmiBetaProductDialog message handlers


BOOL CFmiBetaProductDialog::OnInitDialog()
{
    CTabPageSSL::OnInitDialog();

    SetIcon(CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164)), FALSE);
    itsImageGenerationProgressControl.ShowWindow(SW_HIDE);
    itsImageGenerationProgressControl.SetStep(1);
    itsImageGenerationProgressControl.SetShowPercent(TRUE);
    InitDialogTexts();
    itsGenerateImagesButton.EnableWindow(FALSE);
    NFmiBetaProduct::Language(itsSmartMetDocumentInterface->Language());
    NFmiBetaProduct::RootViewMacroPath(itsSmartMetDocumentInterface->RootViewMacroPath());
    NFmiBetaProductionSystem::SetGenerateBetaProductsCallback(std::bind(&CFmiBetaProductDialog::DoBetaProductGenerations, this, std::placeholders::_1, std::placeholders::_2));

    InitControlsFromDocument();

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiBetaProductDialog::InitControlsFromDocument()
{
    itsImageStoragePathU_ = CA2T(itsBetaProductionSystem->BetaProductStoragePath().c_str());
    itsFileNameTemplateU_ = CA2T(itsBetaProductionSystem->BetaProductFileNameTemplate().c_str());
    fUseAutoFileNames = itsBetaProductionSystem->BetaProductUseAutoFileNames();
    itsRowIndexListStringU_ = CA2T(itsBetaProductionSystem->BetaProductRowIndexListString().c_str());
    itsSynopStationIdListTextU_ = CA2T(itsBetaProductionSystem->BetaProductSynopStationIdListString().c_str());
    itsRowSubdirectoryTemplateStringU_ = CA2T(itsBetaProductionSystem->BetaProductRowSubDirectoryTemplate().c_str());
    // Pit�� asettaa sek� dialogin SelectedViewIndex -kontrolli ett� itsBetaProduct:in SelectedViewIndex
    itsSelectedViewIndex = itsBetaProductionSystem->BetaProductSelectedViewIndex();
    itsBetaProduct->SelectedViewIndex(itsSelectedViewIndex);
    itsViewMacroPathU_ = CA2T(itsBetaProductionSystem->BetaProductViewMacroPath().c_str());
    itsWebSiteTitleStringU_ = CA2T(itsBetaProductionSystem->BetaProductWebSiteTitle().c_str());
    itsWebSiteDescriptionStringU_ = CA2T(itsBetaProductionSystem->BetaProductWebSiteDescription().c_str());
    itsCommandLineStringU_ = CA2T(itsBetaProductionSystem->BetaProductCommandLine().c_str());
    fUseUtcTimesInTimeBox = itsBetaProductionSystem->BetaProductUseUtcTimesInTimeBox();
    SetBoxLocationSelector(itsTimeBoxLocationSelector, itsBetaProductionSystem->BetaProductTimeBoxLocation());
    SetBoxLocationSelector(itsParamBoxLocationSelector, itsBetaProductionSystem->BetaProductParamBoxLocation());
    fDisplayRuntimeInfo = itsBetaProductionSystem->BetaProductDisplayRuntime();
    try
    {
        itsTimeLengthInHoursStringU_ = CA2T(boost::lexical_cast<std::string>(itsBetaProductionSystem->BetaProductTimeLengthInHours()).c_str());
        itsTimeStepInMinutesStringU_ = CA2T(boost::lexical_cast<std::string>(itsBetaProductionSystem->BetaProductTimeStepInMinutes()).c_str());
    }
    catch(...)
    {
    }

    UpdateData(FALSE);

    DoAllBetaProductUpdates();
}

void CFmiBetaProductDialog::DoAllBetaProductUpdates()
{
    Update();
    UpdateRowInfo();
    UpdateViewMacroInfo();
    UpdateWebSiteInfo();
    UpdateFileNameTemplateInfo();
    UpdateCommandLine();
    UpdateSynopStationIdInfo();
}

void CFmiBetaProductDialog::InitControlsFromLoadedBetaProduct()
{
    itsImageStoragePathU_ = CA2T(itsBetaProduct->ImageStoragePath().c_str());
    itsFileNameTemplateU_ = CA2T(itsBetaProduct->FileNameTemplate().c_str());
    fUseAutoFileNames = itsBetaProduct->UseAutoFileNames();
    itsRowIndexListStringU_ = CA2T(itsBetaProduct->RowIndexListString().c_str());
    itsSynopStationIdListTextU_ = CA2T(itsBetaProduct->SynopStationIdListString().c_str());
    itsRowSubdirectoryTemplateStringU_ = CA2T(itsBetaProduct->RowSubdirectoryTemplate().c_str());
    itsSelectedViewIndex = itsBetaProduct->SelectedViewIndex();
    itsViewMacroPathU_ = CA2T(itsBetaProduct->OriginalViewMacroPath().c_str());
    itsWebSiteTitleStringU_ = CA2T(itsBetaProduct->WebSiteTitleString().c_str());
    itsWebSiteDescriptionStringU_ = CA2T(itsBetaProduct->WebSiteDescriptionString().c_str());
    itsCommandLineStringU_ = CA2T(itsBetaProduct->CommandLineString().c_str());
    itsTimeLengthInHoursStringU_ = CA2T(itsBetaProduct->TimeLengthInHoursString().c_str());
    itsTimeStepInMinutesStringU_ = CA2T(itsBetaProduct->TimeStepInMinutesString().c_str());
    fUseUtcTimesInTimeBox = itsBetaProduct->UseUtcTimesInTimeBox();
    SetBoxLocationSelector(itsTimeBoxLocationSelector, itsBetaProduct->TimeBoxLocation());
    SetBoxLocationSelector(itsParamBoxLocationSelector, itsBetaProduct->ParamBoxLocation());
    fDisplayRuntimeInfo = itsBetaProduct->DisplayRunTimeInfo();

    itsTimeRangeInfoTextU_ = CA2T(itsBetaProduct->TimeRangeInfoText().c_str());
    itsRowIndexListInfoTextU_ = CA2T(itsBetaProduct->RowIndexListInfoText().c_str());
    itsViewMacroErrorTextU_ = CA2T(itsBetaProduct->ViewMacroInfoText().c_str());

    UpdateData(FALSE);

    DoAllBetaProductUpdates();
}

void CFmiBetaProductDialog::StoreControlValuesToDocument()
{
    UpdateData(TRUE);

    itsBetaProductionSystem->BetaProductStoragePath(CFmiWin32Helpers::CT2std(itsImageStoragePathU_));
    itsBetaProductionSystem->BetaProductFileNameTemplate(CFmiWin32Helpers::CT2std(itsFileNameTemplateU_));
    itsBetaProductionSystem->BetaProductUseAutoFileNames(fUseAutoFileNames == TRUE);
    itsBetaProductionSystem->BetaProductRowIndexListString(CFmiWin32Helpers::CT2std(itsRowIndexListStringU_));
    itsBetaProductionSystem->BetaProductSynopStationIdListString(CFmiWin32Helpers::CT2std(itsSynopStationIdListTextU_));
    itsBetaProductionSystem->BetaProductRowSubDirectoryTemplate(CFmiWin32Helpers::CT2std(itsRowSubdirectoryTemplateStringU_));
    itsBetaProductionSystem->BetaProductSelectedViewIndex(itsSelectedViewIndex);
    itsBetaProductionSystem->BetaProductViewMacroPath(CFmiWin32Helpers::CT2std(itsViewMacroPathU_));
    itsBetaProductionSystem->BetaProductWebSiteTitle(CFmiWin32Helpers::CT2std(itsWebSiteTitleStringU_));
    itsBetaProductionSystem->BetaProductWebSiteDescription(CFmiWin32Helpers::CT2std(itsWebSiteDescriptionStringU_));
    itsBetaProductionSystem->BetaProductCommandLine(CFmiWin32Helpers::CT2std(itsCommandLineStringU_));
    itsBetaProductionSystem->BetaProductUseUtcTimesInTimeBox(fUseUtcTimesInTimeBox == TRUE);
    itsBetaProductionSystem->BetaProductTimeBoxLocation(GetSelectedBoxLocation(true));
    itsBetaProductionSystem->BetaProductParamBoxLocation(GetSelectedBoxLocation(false));
    itsBetaProductionSystem->BetaProductDisplayRuntime(fDisplayRuntimeInfo == TRUE);

    try
    {
        itsBetaProductionSystem->BetaProductTimeLengthInHours(itsBetaProduct->TimeLengthInHours());
        itsBetaProductionSystem->BetaProductTimeStepInMinutes(itsBetaProduct->TimeStepInMinutes());
    }
    catch(...)
    {
    }
}

BOOL CFmiBetaProductDialog::Create(CWnd* pParentWnd)
{
    return CTabPageSSL::Create(CFmiBetaProductDialog::IDD, pParentWnd);
}

// Tab dialogia varten
BOOL CFmiBetaProductDialog::Create(UINT nIDTemplate, CWnd* pParentWnd)
{
    return CTabPageSSL::Create(nIDTemplate, pParentWnd);
}

// T�m� funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell�.
// T�m� on ik�v� kyll� teht�v� erikseen dialogin muokkaus ty�kalusta, eli
// tekij�n pit�� lis�t� erikseen t�nne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiBetaProductDialog::InitDialogTexts()
{
    SetWindowText(CA2T(::GetDictionaryString("Beta Product Generation").c_str())); // T�m� otsikko ei tule n�kyviin koska t�m� on oikeasti tabi-sivu
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TIME_STEP_IN_MINUTES_TEXT, NFmiBetaProduct::TimeStepLabel().c_str());
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TIME_LENGTH_IN_HOURS_TEXT, NFmiBetaProduct::TimeLengthLabel().c_str());
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_IMAGE_DIRECTORY_TEXT, "Image storage path");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_IMAGE_FILE_NAME_TEMPLATE_TEXT, "Image file name template");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_AUTO_FILE_NAMES, "Auto file names");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_IMAGE_DIRECTORY_BROWSE, "Browse");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_GENERATE_IMAGES, "Generate images");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_ROW_INDEX_LIST_TEXT, "Used row index list (samples):\n<empty>  OR  r1,r2,r3-r4,...");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_ROW_SUBDIRECTORY_TEMPLATE_TEXT, "Possible row subdirectory format:\nRow#  =>  Row1, Row2,...");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SELECTED_VIEW_GROUP_TEXT, "View selection");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_MAIN_MAP_VIEW, "Main map");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO__MAP_VIEW_2, "Map 2");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO__MAP_VIEW_3, "Map 3");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TIME_SERIAL_VIEW, "Time serial");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_SOUNDING_VIEW, "Sounding");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_CROSS_SECTION_VIEW, "Cross section");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_IMAGE_GENERATION_GROUP, "Image generation");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_ROW_RELATED_GROUP, "View row information");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_FILE_RELATED_GROUP, "Stored image information");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TIME_RELATED_GROUP, "Image times information");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VIEW_MACRO_GROUP, "ViewMacro information");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VIEW_MACRO_PATH_TEXT, "ViewMacro path");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_VIEW_MACRO_PATH_BROWSE, "Browse VM");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_WEB_SITE_INFO_GROUP, "Web site info");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_WEB_TITLE_TEXT, "Web site title text");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_WEB_TITLE_STAMPS_TEXT, "(Available 'stamps':  <paramName>, <producerName>)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_WEB_DESCRIPTION_TEXT, "Web site description text");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_WEB_DESCRIPTION_STAMPS_TEXT, "(Available 'stamps':  <paramName>, <producerName>)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SAVE_BETA_PRODUCT, "Save");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SAVE_AS_BETA_PRODUCT, "Save as");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOAD_BETA_PRODUCT, "Load");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_IMAGE_FILE_NAME_TEMPLATE_STAMPS_TEXT, GetFileNameTemplateStampsString().c_str());
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_COMMAND_LINE_GROUP_TEXT, "Used post image generation command line");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DISPLAY_RUNTIME_INFO, "Display runtime info");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SYNOP_STATION_ID_VIEW_GROUP_TEXT, "Synop station ids: <empty> OR id1,id2,id3-id4,...");

    InitLocationSelector(itsTimeBoxLocationSelector);
    InitLocationSelector(itsParamBoxLocationSelector);
}

void CFmiBetaProductDialog::CheckForGenerateButtonActivation()
{
    if(!itsSmartMetDocumentInterface->BetaProductGenerationRunning() && itsBetaProduct->InputWasGood() && fFileNameTemplateContainsValidTime)
    {
        // Jos kaikki inputit olivat kunnossa, sallitaan Generate- ja Save -buttonien k�ytt�
        itsGenerateImagesButton.EnableWindow(TRUE); 
        itsSaveAsButton.EnableWindow(TRUE);
        if(itsBetaProductFullFilePath.empty())
            itsSaveButton.EnableWindow(FALSE);
        else
            itsSaveButton.EnableWindow(TRUE);
    }
    else
    {
        // Jos ei, niin estet��n Generate- ja Save -buttonien k�ytt�
        itsGenerateImagesButton.EnableWindow(FALSE);
        itsSaveAsButton.EnableWindow(FALSE);
        itsSaveButton.EnableWindow(FALSE);
    }
}

void CFmiBetaProductDialog::EnableFileNameTemplateEdit()
{
    CWnd *editControl = GetDlgItem(IDC_EDIT_FILE_NAME_TEMPLATE);
    if(editControl)
    {
        if(fUseAutoFileNames)
            editControl->EnableWindow(FALSE);
        else
            editControl->EnableWindow(TRUE);
    }
}

// Update:a kutsutaan kun halutaan p�ivitt�� aikoja Beta-product dialogissa, esim. kun karttan�yt�n aikaa muutetaan.
void CFmiBetaProductDialog::Update()
{
    UpdateData(TRUE);

    // Katsotaan p��karttan�yt�n aika ja s��det��n aikatekstin sen mukaisesti
    itsBetaProduct->CheckTimeRelatedInputs(GetCurrentViewTime(*itsBetaProduct), ::CFmiWin32Helpers::CT2std(itsTimeLengthInHoursStringU_), CFmiWin32Helpers::CT2std(itsTimeStepInMinutesStringU_), fUseUtcTimesInTimeBox == TRUE, GetSelectedBoxLocation(true));
    itsTimeRangeInfoTextU_ = CA2T(itsBetaProduct->TimeRangeInfoText().c_str());
    itsBetaProduct->ImageStoragePath(CFmiWin32Helpers::CT2std(itsImageStoragePathU_));
    itsBetaProduct->DisplayRunTimeInfo(fDisplayRuntimeInfo == TRUE);
    CheckForGenerateButtonActivation();

    UpdateData(FALSE);
}

NFmiMetTime CFmiBetaProductDialog::GetCurrentViewTime(const NFmiBetaProduct &theBetaProduct)
{
    return itsSmartMetDocumentInterface->CurrentTime(theBetaProduct.SelectedViewIndex());
}

void CFmiBetaProductDialog::OnBnClickedButtonImageDirectoryBrowse()
{
    CXFolderDialog dlg(itsImageStoragePathU_);
    dlg.SetTitle(CA2T(::GetDictionaryString("Select folder").c_str())); 
    if(dlg.DoModal() == IDOK)
    {
        itsImageStoragePathU_ = dlg.GetPath();
        UpdateData(FALSE);
        Update();
    }
}

void CFmiBetaProductDialog::DoImageProducingProcess(std::function<void(const NFmiMetTime&)> &theGenerationFunction, const NFmiMetTime &theMakeTime, bool justLogMessages)
{
    // Ota nykyhetkest� viewMacro talteen
    NFmiViewSettingMacro currentStateViewMacro;
    itsSmartMetDocumentInterface->FillViewMacroInfo(currentStateViewMacro, "Current state", "Current state view macro description");

    itsGenerateImagesButton.EnableWindow(FALSE); // Estet��n napin k�ytt� kuvien generoinnin ajan
    StoreControlValuesToDocument(); // Ennen tuotantoa talletetaan varmuuden vuoksi s��d�t my�s dokumenttiin

    itsImageGenerationProgressControl.ShowWindow(SW_SHOW);
    std::string errorDialogTitleStr = ::GetDictionaryString("Error with Beta product generation");
    itsSmartMetDocumentInterface->BetaProductGenerationRunning(true);

    try
    {
        // Generate images
        theGenerationFunction(theMakeTime);
    }
    catch(std::exception &e)
    {
        std::string errorStr = ::GetDictionaryString("Following error occured");
        errorStr += ":\n";
        errorStr += e.what();
        itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorDialogTitleStr, CatLog::Severity::Error, CatLog::Category::Operational, justLogMessages);
    }
    catch(...)
    {
        std::string errorStr = ::GetDictionaryString("Unknown error occured");
        itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorDialogTitleStr, CatLog::Severity::Error, CatLog::Category::Operational, justLogMessages);
    }

    itsSmartMetDocumentInterface->BetaProductGenerationRunning(false);
    itsImageGenerationProgressControl.ShowWindow(SW_HIDE);
    itsGenerateImagesButton.EnableWindow(TRUE); // Sallitaan taas napin k�ytt�

    // Palautetaan ollut tila viewMacro takaisin
    itsSmartMetDocumentInterface->LoadViewMacroInfo(currentStateViewMacro, false);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("BetaProductDialog: Restoring old state after Beta product image generation");
}

void CFmiBetaProductDialog::OnBnClickedButtonGenerateImages()
{
    NFmiMetTime makeTime(1);
    std::function<void(const NFmiMetTime&)> generationFunction = std::bind(&CFmiBetaProductDialog::MakeVisualizationImagesManually, this, std::placeholders::_1);
    DoImageProducingProcess(generationFunction, makeTime, false);
}

void CFmiBetaProductDialog::OnEnChangeEditTimeStepInMinutes()
{
    Update();
}


void CFmiBetaProductDialog::OnEnChangeEditTimeLengthInHours()
{
    Update();
}


void CFmiBetaProductDialog::DoWhenClosing(void)
{
    StoreControlValuesToDocument();
}

#ifdef _MSC_VER
#pragma warning( disable : 4996 ) // poista _stprintf funktion aiheuttama 'turvallisuus' varoitus
#endif

static void DeleteAllFiles(TCHAR *folderPath)
{
    TCHAR fileFound[MAX_PATH + 1];
    WIN32_FIND_DATA info;
    HANDLE hp;
    _stprintf(fileFound, _TEXT("%s\\*.*"), folderPath);
    hp = FindFirstFile(fileFound, &info);
    do
    {
        _stprintf(fileFound, _TEXT("%s\\%s"), folderPath, info.cFileName);
        DeleteFile(fileFound);

    } while(FindNextFile(hp, &info));
    FindClose(hp);
}

void CFmiBetaProductDialog::StoreWebInfoFile(const std::string &theDestinationDirectory, const std::string &theFileName, const std::string &theFileText)
{
    static const std::string errorDialogTitle = ::GetDictionaryString("Beta product web site info file error");
    std::string finalFileText = theFileText;
    NFmiStringTools::Trim(finalFileText); // trimmataan varmuuden vuoksi white spacet alusta ja lopusta
    if(!finalFileText.empty())
    {
        std::string fileName = theDestinationDirectory + theFileName;
        std::ofstream out(fileName.c_str(), std::ios::binary);
        if(out)
            out << finalFileText;
        else
        {
            std::string errorStr = ::GetDictionaryString("Unable to create web site info file");
            errorStr += ":\n";
            errorStr += fileName;
            itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorDialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, true);
        }
    }
}

static std::string MakeFinalCmdLineString(const std::string &theCommandLine)
{
    std::string trimmedCmdLine = theCommandLine;
    // Korvataan rivinvaihdot && -merkeill�
    NFmiStringTools::ReplaceAll(trimmedCmdLine, "\r\n", " && ");
    NFmiStringTools::TrimAll(trimmedCmdLine); // trimmataan varmuuden vuoksi white spacet alusta ja lopusta ja v�list�
    NFmiStringTools::ReplaceAll(trimmedCmdLine, "&& &&", "&&"); // Jos k�ytt�j� oli laittanut my�s && -merkit ja rivin vaihdon, t�ll�in poistetaan turhia &&-merkist�j�
    if(trimmedCmdLine.empty())
        return "";

    // Lopulliseen komentorivi komentoketjuun halutaan seuraavaa:
    // 1. Aluksi pit�� olla cmd /k -optiolla, joka pit�� komentorivin k�ynniss� usealle komennolle, jotka on eroteltu && -merkeill�
    std::string finalCmdLine = "cmd /k ";
    // 2. Laita k�ytt�j�n antamant komennot per��n.
    finalCmdLine += trimmedCmdLine;
    // 3. Loppuun pit�� laittaa exit, muuten komentorivi j�� eloon
    finalCmdLine += " && exit";

    return finalCmdLine;
}

void CFmiBetaProductDialog::MakeCommandLineExecution(const std::string &theDestinationDirectory, const std::string &theCommandLine, bool justLogMessages)
{
    std::string finalCmdLine = ::MakeFinalCmdLineString(theCommandLine);
    if(!finalCmdLine.empty())
    {
        bool showErrorMessageBox = !justLogMessages;
        bool waitExecutionToStop = !justLogMessages;
        // Komento rivi ajetaan piilossa, eik� sen loppumista odoteta
        CFmiProcessHelpers::ExecuteCommandInSeparateProcess(finalCmdLine, nullptr, showErrorMessageBox, SW_SHOW, waitExecutionToStop, NORMAL_PRIORITY_CLASS, &theDestinationDirectory);
    }
}

void CFmiBetaProductDialog::GenerateWebInfoFiles(const std::string &theDestinationDirectory, const NFmiBetaProduct &theBetaProduct)
{
    std::string finalDestinationDirectory = theDestinationDirectory;
    PathUtils::addDirectorySeparatorAtEnd(finalDestinationDirectory);
    StoreWebInfoFile(finalDestinationDirectory, "title.txt", theBetaProduct.WebSiteTitleString());
    StoreWebInfoFile(finalDestinationDirectory, "description.txt", theBetaProduct.WebSiteDescriptionString());
}

bool CFmiBetaProductDialog::CheckDestinationDirectory(const std::string &theDestinationDirectory, bool fAllowDestinationDelete, const NFmiBetaProduct &theBetaProduct)
{
    static const std::string errorDialogTitle = ::GetDictionaryString("Beta product output directory error");
    if(theDestinationDirectory.empty())
    {
        std::string errorStr = ::GetDictionaryString("Given Beta product directory was empty, no images will be produced...");
        itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorDialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, true);
        return false;
    }
    else 
    {
        if(!NFmiFileSystem::DirectoryExists(theDestinationDirectory))
        {
            if(!NFmiFileSystem::CreateDirectory(theDestinationDirectory))
            {
                std::string errorStr = ::GetDictionaryString("Unable to create given Beta product directory, no images will be produced...");
                itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorDialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, true);
                return false;
            }
        }
        if(fAllowDestinationDelete)
            ::DeleteAllFiles(CA2T(theDestinationDirectory.c_str()));

        GenerateWebInfoFiles(theDestinationDirectory, theBetaProduct);
        return true;
    }
}

// Palauttaa n�yt�n oikean (absoluuttisen) rivinumeron (ei suhteellista rivinumeroa)
int CFmiBetaProductDialog::GetViewRowIndex(int selectedViewRadioButtonIndex)
{
    int rowIndex = gMissingRowIndex;
    if(selectedViewRadioButtonIndex <= BetaProductViewIndex::kMapView3)
    {
        NFmiMapViewDescTop *mapViewDescTop = itsSmartMetDocumentInterface->MapViewDescTop(selectedViewRadioButtonIndex);
        if(mapViewDescTop)
            rowIndex = mapViewDescTop->MapRowStartingIndex();
    }
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::kCrossSectionView)
        rowIndex = itsSmartMetDocumentInterface->CrossSectionSystem()->StartRowIndex();

    return rowIndex;
}

// Asettaa n�yt�n oikean (absoluuttisen) rivinumeron (ei suhteellista rivinumeroa)
void CFmiBetaProductDialog::SetViewRowIndex(int selectedViewRadioButtonIndex, int rowIndex)
{
    if(selectedViewRadioButtonIndex <= BetaProductViewIndex::kMapView3)
    {
        NFmiMapViewDescTop *mapViewDescTop = itsSmartMetDocumentInterface->MapViewDescTop(selectedViewRadioButtonIndex);
        if(mapViewDescTop)
            mapViewDescTop->MapRowStartingIndex(rowIndex);
    }
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::kCrossSectionView)
        itsSmartMetDocumentInterface->CrossSectionSystem()->StartRowIndex(rowIndex);
}

static boost::shared_ptr<NFmiFastQueryInfo> GetFirstModelsInfo(SmartMetDocumentInterface *smartMetDocumentInterface, unsigned int theDescTopIndex, int theRealRowIndex)
{
    // 1. Haetaan n�ytt��n ja rivinumeroon perustuen drawParam-lista
    NFmiDrawParamList *drawParamList = smartMetDocumentInterface->DrawParamListWithRealRowNumber(theDescTopIndex, theRealRowIndex);
    if(drawParamList)
    {
        // 2. Tutkitaan param listaa kunnes l�ytyy 1. malli data, ja otetaan siit� originTime talteen
        for(drawParamList->Reset(); drawParamList->Next();)
        {
            boost::shared_ptr<NFmiDrawParam> drawParam = drawParamList->Current();
            if(drawParam->IsModelRunDataType())
            {
                return smartMetDocumentInterface->InfoOrganizer()->Info(drawParam, false, false);
            }
        }
    }
    return boost::shared_ptr<NFmiFastQueryInfo>();
}

NFmiMetTime CFmiBetaProductDialog::GetFirstModelOrigTime(unsigned int theDescTopIndex, int theRealRowIndex)
{
    boost::shared_ptr<NFmiFastQueryInfo> info = ::GetFirstModelsInfo(itsSmartMetDocumentInterface, theDescTopIndex, theRealRowIndex);
    if(info)
        return info->OriginTime();
    else
        return NFmiMetTime::gMissingTime; // Jos jokin meni pieleen tai dataa ei l�ydy, palautetaan puuttuva aika
}

NFmiTimeDescriptor CFmiBetaProductDialog::GetFirstModelsValidTimeDescriptor(unsigned int theDescTopIndex, int theRealRowIndex)
{
    boost::shared_ptr<NFmiFastQueryInfo> info = ::GetFirstModelsInfo(itsSmartMetDocumentInterface, theDescTopIndex, theRealRowIndex);
    if(info)
        return info->TimeDescriptor();
    else
        return NFmiTimeDescriptor(); // Jos jokin meni pieleen tai dataa ei l�ydy, palautetaan tyhj��
}

// Lis�t��n tiedosto nimen polkuun mahdollinen listatuista riveist� ja row-subdirectory templaatista tuleva osio.
void CFmiBetaProductDialog::AddImageRowPath(std::string &theImageFileNameBaseInOut, int theRowIndex)
{
    if(itsBetaProduct->GetOriginalRowIndexies().size() && !itsBetaProduct->RowSubdirectoryTemplate().empty())
    {
        PathUtils::addDirectorySeparatorAtEnd(theImageFileNameBaseInOut);
        std::string rowIndexStr = boost::lexical_cast<std::string>(theRowIndex);
        std::string usedSubDirectory = itsBetaProduct->RowSubdirectoryTemplate();
        boost::algorithm::ireplace_first(usedSubDirectory, "#", rowIndexStr);
        theImageFileNameBaseInOut += usedSubDirectory;
        PathUtils::addDirectorySeparatorAtEnd(theImageFileNameBaseInOut);
    }
}

static std::string MakeImagesGeneratedText()
{
    std::string text;
    text += " ";
    text += ::GetDictionaryString("images generated");
    return text;
}

static NFmiTime GetCurrentUtcTimeWithSeconds()
{
    NFmiTime atime;
    NFmiTime utcTime = atime.UTCTime();
    // UTCTime() -metodi hukkaa sekunnit, joten ne otetaan originaali ajasta
    NFmiTime usedTime(utcTime.GetYear(), utcTime.GetMonth(), utcTime.GetDay(), utcTime.GetHour(), utcTime.GetMin(), atime.GetSec());
    return usedTime;
}

void CFmiBetaProductDialog::UpdateGeneratedImagesText(int theIndex, int theTotalImageCount)
{
    std::string text;
    if(theIndex < theTotalImageCount)
    {
        text = boost::lexical_cast<std::string>(theIndex);
        text += "/";
        text += boost::lexical_cast<std::string>(theTotalImageCount);
        text += MakeImagesGeneratedText();
    }
    else
    {
        text = boost::lexical_cast<std::string>(theIndex);
        text += MakeImagesGeneratedText();
        text += "  (";
        text += ::GetDictionaryString("at");
        text += " ";
        NFmiTime utcTimeWithSeconds = ::GetCurrentUtcTimeWithSeconds();
        text += utcTimeWithSeconds.ToStr("YYYY.MM.DD HH:mm:SS", itsSmartMetDocumentInterface->Language());
        text += ")";
    }
    itsImageProductionTextU_ = CA2T(text.c_str());
    UpdateData(FALSE);
}

bool CFmiBetaProductDialog::RunViewMacro(const NFmiBetaProduct &theBetaProduct, bool justLogMessages)
{
    bool extensionAdded = false;
    std::string viewMacroPath = theBetaProduct.ViewMacroPath();
    if(!viewMacroPath.empty())
    {
        std::string errorString;
        if(!itsSmartMetDocumentInterface->LoadViewMacroFromBetaProduct(viewMacroPath, errorString, justLogMessages))
        {
            if(!justLogMessages)
                ::MessageBox(this->GetSafeHwnd(), CA2T(errorString.c_str()), CA2T(::GetDictionaryString("Beta product ViewMacro problem").c_str()), MB_OK | MB_ICONERROR);
            return false;
        }
    }
    return true;
}

#ifdef min
#undef min
#endif

static NFmiMetTime CalcWallClockOffsetTime(const NFmiBetaProduct &theBetaProduct, const NFmiBetaProductAutomation::NFmiTimeModeInfo &theTimeMode, const NFmiMetTime &theMakeTime)
{
    NFmiMetTime aTime = theMakeTime;
    long usedOffsetInMinutes = boost::math::iround(theTimeMode.itsWallClockOffsetInHours * 60.);
    int absoluteOffsetMinutes = std::abs(usedOffsetInMinutes % 60);
    long usedTimeStepInMinutes = 60;
    // Yleens� halutaan py�rist�� aloitus/lopetus ajat l�himp��n tuntiin, mutta jos annetut wall-time offsetit eiv�t ole tasatuntisia, 
    // laitetaan time-stepiksi pienempi 'stepeist�', jotta p��st��n k�siksi haluttuihin minuutti lukemiin...
    if((absoluteOffsetMinutes != 0 && absoluteOffsetMinutes <= 30) || theBetaProduct.TimeStepInMinutes() <= 30)
    {
        if(absoluteOffsetMinutes == 0)
            usedTimeStepInMinutes = theBetaProduct.TimeStepInMinutes();
        else
            usedTimeStepInMinutes = std::min(absoluteOffsetMinutes, theBetaProduct.TimeStepInMinutes());
    }
    // Varmistetaan ett� timestep ei ole mahdoton
    if(usedTimeStepInMinutes < 1)
        usedTimeStepInMinutes = 1;
    aTime.SetTimeStep(usedTimeStepInMinutes);
    aTime.ChangeByMinutes(usedOffsetInMinutes);
    aTime.SetTimeStep(theBetaProduct.TimeStepInMinutes()); // Laitetaan lopuksi aloitusaika haluttuun aikasteppiin, jotta sille voidaan tehd� aika-loopissa NextMetTime
    return aTime;
}

NFmiMetTime CFmiBetaProductDialog::CalcStartingTime(const NFmiBetaProduct &theBetaProduct, const NFmiBetaProductAutomation::NFmiTimeModeInfo &theTimeMode, const NFmiMetTime &theMakeTime)
{
    if(theTimeMode.itsTimeMode == NFmiBetaProductAutomation::kFmiBetaProductTime || theTimeMode.itsTimeMode == NFmiBetaProductAutomation::kFmiFirstModelDataTime)
    {
        // Otetaan BetaProduct aika my�s kFmiFirstModelDataTime -tyypille, sill� ne pit�� laskea erikseen kuvien generointi loopissa 
        // ja jos rivilt� ei l�ydy yht��n mallituottajaa, k�ytet��n beta-product timea
        NFmiMetTime startingTime = GetCurrentViewTime(theBetaProduct);
        startingTime.SetTimeStep(theBetaProduct.TimeStepInMinutes()); // Laitetaan lopuksi aloitusaika haluttuun aikasteppiin, jotta sille voidaan tehd� aika-loopissa NextMetTime
        return startingTime;
    }
    else if(theTimeMode.itsTimeMode == NFmiBetaProductAutomation::kFmiWallClockOffsetTime)
        return ::CalcWallClockOffsetTime(theBetaProduct, theTimeMode, theMakeTime);
    else
        return NFmiMetTime::gMissingTime;
}

NFmiMetTime CFmiBetaProductDialog::CalcEndingTime(const NFmiBetaProduct &theBetaProduct, const NFmiBetaProductAutomation::NFmiTimeModeInfo &theTimeMode, const NFmiMetTime &theStartTime, const NFmiMetTime &theMakeTime)
{
    if(theTimeMode.itsTimeMode == NFmiBetaProductAutomation::kFmiBetaProductTime || theTimeMode.itsTimeMode == NFmiBetaProductAutomation::kFmiFirstModelDataTime)
    {
        // Otetaan BetaProduct aika my�s kFmiFirstModelDataTime -tyypille, sill� ne pit�� laskea erikseen kuvien generointi loopissa 
        // ja jos rivilt� ei l�ydy yht��n mallituottajaa, k�ytet��n beta-product timea
        NFmiMetTime endingTime(theStartTime);
        long timeLenghtInMinutes = boost::math::iround(theBetaProduct.TimeLengthInHours() * 60);
        endingTime.ChangeByMinutes(timeLenghtInMinutes);
        return endingTime;
    }
    else if(theTimeMode.itsTimeMode == NFmiBetaProductAutomation::kFmiWallClockOffsetTime)
        return ::CalcWallClockOffsetTime(theBetaProduct, theTimeMode, theMakeTime);
    else
        return NFmiMetTime::gMissingTime;
}

bool CFmiBetaProductDialog::CheckGenerationTimes(const NFmiMetTime &theStartingTime, const NFmiMetTime &theEndingTime, bool justLogMessages)
{
    if(theStartingTime != NFmiMetTime::gMissingTime && theEndingTime != NFmiMetTime::gMissingTime)
        return true;
    else
    {
        std::string errorDialogTitle = "Unknown Beta-product errror";
        std::string errorStr = "Unknown SmartMet application error when generating Beta-products. start-time and/or end-time were missing.";
        itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorDialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, justLogMessages);
        return false;
    }
}

NFmiMetTime CFmiBetaProductDialog::GetUsedModelTime(bool fGetStartTime, unsigned int theDescTopIndex, int theRealRowIndex, const NFmiMetTime &thePreCalculatedTime)
{
    NFmiTimeDescriptor timeDesc = GetFirstModelsValidTimeDescriptor(theDescTopIndex, theRealRowIndex);
    if(timeDesc.IsEmpty())
        return thePreCalculatedTime;
    NFmiMetTime atime;
    if(fGetStartTime)
        atime = timeDesc.FirstTime();
    else
        atime = timeDesc.LastTime();
    atime.SetTimeStep(thePreCalculatedTime.GetTimeStep());
    return atime;
}

static NFmiRect GetWantedImageRelativeRect(NFmiMapViewDescTop &mapViewDescTop, int selectedViewRadioButtonIndex)
{
    if(selectedViewRadioButtonIndex <= CtrlViewUtils::SpecialDesctopIndex::kFmiMaxMapDescTopIndex)
        return mapViewDescTop.RelativeMapRect();
    else
    {
        // Muille kuin karttan�yt�ille pit�� palauttaa totaali alue
        static const NFmiRect totalRelativeRect(0., 0., 1., 1.);
        return totalRelativeRect;
    }
}

// Jos on annettu synop asemien id:t� beta tuotteessa
// JA jos on kyse aikasarjaikkuna piirrosta
// TAI on kyse poikkileikkausn�yt�st� ja k�yt�ss� on aikasarja TAI obs-model moodi.
bool CFmiBetaProductDialog::IsSynopLocationsUsed(int selectedViewRadioButtonIndex, const std::vector<int> &synopLocationIds)
{
    if(synopLocationIds.empty())
        return false;
    if(selectedViewRadioButtonIndex == BetaProductViewIndex::kTimeSerialView || selectedViewRadioButtonIndex == BetaProductViewIndex::kSoundingView)
        return true;
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::kCrossSectionView)
    {
        auto crossMode = itsSmartMetDocumentInterface->CrossSectionSystem()->GetCrossMode();
        if(crossMode == NFmiCrossSectionSystem::CrossMode::kObsAndFor || crossMode == NFmiCrossSectionSystem::CrossMode::kTime)
            return true;
    }
    return false;
}

static int CalcTotalImageCount(const NFmiBetaProduct &theBetaProduct, const std::vector<int> &usedRowIndexies, const std::vector<int> &synopLocationIds, bool useSynopLocations)
{
    int totalImageCount = static_cast<int>(usedRowIndexies.size() * theBetaProduct.TimeStepCount());
    if(useSynopLocations)
        totalImageCount = static_cast<int>(totalImageCount * synopLocationIds.size());
    return totalImageCount;
}

static const NFmiLocation* GetSynopLocation(SmartMetDocumentInterface *smartMetDocumentInterface, int synopLocationId)
{
    auto infoVector = smartMetDocumentInterface->GetSortedSynopInfoVector(kFmiSYNOP);
    if(!infoVector.empty())
    {
        for(auto &info : infoVector)
        {
            if(info->Location(static_cast<unsigned long>(synopLocationId)))
                return info->Location();
        }
    }
    return nullptr;
}

bool CFmiBetaProductDialog::SetStationIdLocation(const NFmiBetaProduct &theBetaProduct, int synopLocationId)
{
    auto synopLocation = ::GetSynopLocation(itsSmartMetDocumentInterface, synopLocationId);
    if(synopLocation)
    {
        int usedViewRadioButtonIndex = theBetaProduct.SelectedViewIndex();
        if(usedViewRadioButtonIndex == BetaProductViewIndex::kTimeSerialView || usedViewRadioButtonIndex == BetaProductViewIndex::kSoundingView)
        {
            unsigned int mainMapViewIndex = 0;
            auto editedInfo = itsSmartMetDocumentInterface->EditedSmartInfo();
            auto mapViewDesktop = itsSmartMetDocumentInterface->MapViewDescTop(mainMapViewIndex);
            bool dummyParam = false;
            itsSmartMetDocumentInterface->SelectLocations(mainMapViewIndex, editedInfo, mapViewDesktop->MapHandler()->Area(), synopLocation->GetLocation(), itsSmartMetDocumentInterface->CurrentTime(mainMapViewIndex), kFmiSelectionCombineClearFirst, NFmiMetEditorTypes::kFmiSelectionMask, dummyParam, true, false);
        }
        else if(usedViewRadioButtonIndex == BetaProductViewIndex::kCrossSectionView)
        {
            itsSmartMetDocumentInterface->CrossSectionSystem()->StartPoint(synopLocation->GetLocation());
        }
    }
    return synopLocation != nullptr;
}

static std::vector<int> GetFoundSynopLocationIds(SmartMetDocumentInterface *smartMetDocumentInterface, const std::vector<int> &synopLocationIds)
{
    std::vector<int> actuallyFoundLocationIds;
    for(auto synopLocationId : synopLocationIds)
    {
        if(::GetSynopLocation(smartMetDocumentInterface, synopLocationId))
            actuallyFoundLocationIds.push_back(synopLocationId);
    }
    return actuallyFoundLocationIds;
}

void CFmiBetaProductDialog::MakeVisualizationImages(const NFmiBetaProduct &theBetaProduct, const NFmiMetTime &theStartingTime, bool useModelStartTime, const NFmiMetTime &theEndingTime, bool useModelEndTime, const NFmiMetTime &theMakeTime, bool justLogMessages)
{
    try
    {
        itsSmartMetDocumentInterface->SetCurrentGeneratedBetaProduct(&theBetaProduct); // asetetaan aluksi generoitavan beta-productin pointteri dokumenttiin, jotta piirrossa voidaan tarvittaessa k�ytt�� sit�

        if(!CheckGenerationTimes(theStartingTime, theEndingTime, justLogMessages))
            return;

        if(CheckDestinationDirectory(theBetaProduct.ImageStoragePath(), true, theBetaProduct))
        {
            itsTotalImagesGenerated = 0;
            std::string makeTimeStampString = theMakeTime.ToStr(kYYYYMMDDHHMM);

            int usedViewRadioButtonIndex = theBetaProduct.SelectedViewIndex();
            int origRowIndex = GetViewRowIndex(usedViewRadioButtonIndex);
            std::vector<int> usedRowIndexies = theBetaProduct.GetUsedRowIndexies(origRowIndex);
            std::vector<int> synopLocationIds = theBetaProduct.SynopStationIdList();
            synopLocationIds = ::GetFoundSynopLocationIds(itsSmartMetDocumentInterface, synopLocationIds);
            bool useSynopLocations = IsSynopLocationsUsed(usedViewRadioButtonIndex, synopLocationIds);
            int totalImageCount = ::CalcTotalImageCount(theBetaProduct, usedRowIndexies, synopLocationIds, useSynopLocations);
            itsImageGenerationProgressControl.SetRange(0, totalImageCount);
            itsImageGenerationProgressControl.SetPos(0);
            auto descTop = itsSmartMetDocumentInterface->MapViewDescTop(usedViewRadioButtonIndex);
            float oldTimeStepInHours = descTop->TimeControlTimeStep();
            descTop->TimeControlTimeStep(theBetaProduct.TimeStepInMinutes() / 60.f);
            NFmiMetTime originalViewTime = itsSmartMetDocumentInterface->CurrentTime(usedViewRadioButtonIndex); // otetaan nykyinen n�yt�n aika talteen

            if(useSynopLocations)
            {
                bool deleteDestinationDirectory = true;
                for(auto synopLocationId : synopLocationIds)
                {
                    if(SetStationIdLocation(theBetaProduct, synopLocationId))
                    {
                        MakeVisualizationImagesRowLoop(theBetaProduct, theStartingTime, useModelStartTime, theEndingTime, useModelEndTime, theMakeTime, justLogMessages, usedRowIndexies, totalImageCount, synopLocationId, deleteDestinationDirectory);
                    }
                    deleteDestinationDirectory = false;
                }
            }
            else
            {
                MakeVisualizationImagesRowLoop(theBetaProduct, theStartingTime, useModelStartTime, theEndingTime, useModelEndTime, theMakeTime, justLogMessages, usedRowIndexies, totalImageCount, 0, true);
            }
            UpdateGeneratedImagesText(itsTotalImagesGenerated, totalImageCount);

            descTop->TimeControlTimeStep(oldTimeStepInHours);
            SetViewRowIndex(usedViewRadioButtonIndex, origRowIndex);

            itsSmartMetDocumentInterface->CurrentTime(usedViewRadioButtonIndex, originalViewTime); // asetus takaisin alkuaikaan
        }
    }
    catch(...)
    {
        // Poikkeuksen yhteydess� nollataan beta-product pointteri ja heitet��n poikkeus uudestaan
        itsSmartMetDocumentInterface->SetCurrentGeneratedBetaProduct(nullptr);
        throw;
    }
    itsSmartMetDocumentInterface->SetCurrentGeneratedBetaProduct(nullptr); // nollataan beta-product pointteri lopuksi
}

static std::string MakeImageFileNameBase(const NFmiBetaProduct &theBetaProduct)
{
    std::string imageFileNameBase = theBetaProduct.ImageStoragePath();
    PathUtils::addDirectorySeparatorAtEnd(imageFileNameBase);
    return imageFileNameBase;
}

static bool IsObservationQdDataType(NFmiInfoData::Type theDataType)
{
    if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(theDataType))
        return true;
    else if(theDataType == NFmiInfoData::kAnalyzeData) // Lis�t��n hyv�ksyttyihin obs-tyyppisiin datoihin my�s analyysit
        return true;
    else
        return false;
}

static bool SatelImageExist(SmartMetDocumentInterface *smartMetDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiMetTime &theValidTime)
{
    const NFmiDataIdent &dataIdent = theDrawParam->Param();
    NFmiImageHolder imageHolder = smartMetDocumentInterface->SatelliteImageCacheSystem().FindImage(dataIdent, theValidTime, NFmiSatelView::ImagesOffsetInMinutes(dataIdent));
    if(imageHolder)
        return true;
    else
        return false;
}

static checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> MakeDrawedInfoVector(SmartMetDocumentInterface *smartMetDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theDrawParam, unsigned int theUsedMapViewDesktopIndex)
{
    boost::shared_ptr<NFmiArea> mapArea = smartMetDocumentInterface->MapViewDescTop(theUsedMapViewDesktopIndex)->MapHandler()->Area();
    checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> infoVector;
    smartMetDocumentInterface->MakeDrawedInfoVectorForMapView(infoVector, theDrawParam, mapArea);
    return infoVector;
}

// Onko kyseisen n�yt�n kyseisen n�ytt�rivin mik��n parametri havainto ja onko siin� kyseisell� ajalla dataa?
static bool HasObservations(SmartMetDocumentInterface *smartMetDocumentInterface, unsigned int theDescTopIndex, int theRealRowIndex, const NFmiMetTime &theValidTime)
{
    NFmiDrawParamList *drawParamList = smartMetDocumentInterface->DrawParamListWithRealRowNumber(theDescTopIndex, theRealRowIndex);
    if(drawParamList && drawParamList->NumberOfItems() > 0)
    {
        for(drawParamList->Reset(); drawParamList->Next();)
        {
            boost::shared_ptr<NFmiDrawParam> drawParam = drawParamList->Current();
            //            if(!drawParam->IsParamHidden()) // Ei v�lit� onko parametri piilossa, koska t�t� voidaan k�ytt�� apuna, eli esim. n�yt�ll� on macroParamissa kyseisi� piilossa olevia havaintoja
            {
                if(::IsObservationQdDataType(drawParam->DataType()))
                {
                    checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> infoVector = ::MakeDrawedInfoVector(smartMetDocumentInterface, drawParam, theDescTopIndex);
                    for(const auto & info : infoVector)
                        if(info->Time(theValidTime))
                            return true;
                }
                else if(drawParam->DataType() == NFmiInfoData::kSatelData)
                {
                    if(!drawParam->IsParamHidden()) // satelkuvilla ei voi kikkailla ja niit� ei voi laittaa mitenk��n macroParam laskuihin, joten hidden pit�� tarkistaa erikseen
                    {
                        // Satelliitti datat ovat kuva pohjaisia datoja, ja niit� pit�� tarkastella eri lailla kuin queryDatoja
                        if(::SatelImageExist(smartMetDocumentInterface, drawParam, theValidTime))
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

static void AutoFileNameChanges(SmartMetDocumentInterface *smartMetDocumentInterface, const NFmiBetaProduct &theBetaProduct, unsigned int theDescTopIndex, int theRealRowIndex, const NFmiMetTime &theValidTime, const NFmiMetTime &theWallClockTime, std::string &theImageFileNameInOut)
{
    if(theBetaProduct.UseAutoFileNames())
    {
        if(::HasObservations(smartMetDocumentInterface, theDescTopIndex, theRealRowIndex, theValidTime) && theValidTime <= theWallClockTime)
            boost::algorithm::ireplace_first(theImageFileNameInOut, NFmiBetaProductionSystem::BetaAutomationAutoFileNameTypeStamp(), "obs");
        else
            boost::algorithm::ireplace_first(theImageFileNameInOut, NFmiBetaProductionSystem::BetaAutomationAutoFileNameTypeStamp(), "for");
    }
}

const std::string g_OrigTimeStampString = "origTime";
const std::string g_MakeTimeStampString = "makeTime";

static std::string MakeFinalImageFileName(SmartMetDocumentInterface *smartMetDocumentInterface, const NFmiBetaProduct &theBetaProduct, unsigned int selectedViewRadioButtonIndex, const std::string &rowImageFileNameBase, const NFmiMetTime &currentTime, const NFmiMetTime &theMakeTime, const NFmiMetTime &modelOrigTime, int rowIndex, int synopStationId)
{
    std::string imageFileName = rowImageFileNameBase;
    std::string validTimeStampString = currentTime.ToStr(kYYYYMMDDHHMM);
    std::string makeTimeStampString = theMakeTime.ToStr(kYYYYMMDDHHMM);
    std::string origTimeStampString = modelOrigTime.ToStr(kYYYYMMDDHHMM);

    boost::algorithm::ireplace_first(imageFileName, NFmiBetaProductionSystem::FileNameTemplateValidTimeStamp(), validTimeStampString);
    boost::algorithm::ireplace_first(imageFileName, g_MakeTimeStampString, makeTimeStampString);
    boost::algorithm::ireplace_first(imageFileName, g_OrigTimeStampString, origTimeStampString);
    // Jos synop asema id on 0:sta poikkeava liitet��n se mahdollisesti tiedostonimeen
    if(synopStationId)
    {
        std::string synopStationIdString = std::to_string(synopStationId);
        boost::algorithm::ireplace_first(imageFileName, NFmiBetaProductionSystem::FileNameTemplateStationIdStamp(), synopStationIdString);
    }
    ::AutoFileNameChanges(smartMetDocumentInterface, theBetaProduct, selectedViewRadioButtonIndex, rowIndex, currentTime, theMakeTime, imageFileName); // theMakeTime:a k�ytet��n t��ll� tarkasteluissa my�s sein�kelloaikana
    return imageFileName;
}

void CFmiBetaProductDialog::MakeVisualizationImagesRowLoop(const NFmiBetaProduct &theBetaProduct, const NFmiMetTime &theStartingTime, bool useModelStartTime, const NFmiMetTime &theEndingTime, bool useModelEndTime, const NFmiMetTime &theMakeTime, bool justLogMessages, const std::vector<int> &usedRowIndexies, int totalImageCount, int synopStationId, bool deleteDestinationDirectory)
{
    std::string imageFileNameBase = ::MakeImageFileNameBase(theBetaProduct);
    CBitmap mapScreenBitmap;
    unsigned int usedMapViewDesktopIndex = theBetaProduct.SelectedViewIndex();
    auto descTop = itsSmartMetDocumentInterface->MapViewDescTop(usedMapViewDesktopIndex);
    // row-for-loop
    for(size_t i = 0; i < usedRowIndexies.size(); i++)
    {
        int rowIndex = usedRowIndexies[i];
        SetViewRowIndex(usedMapViewDesktopIndex, rowIndex);
        itsSmartMetDocumentInterface->UpdateViewForOffScreenDraw(usedMapViewDesktopIndex); // T�m� p�ivitt�� oikeat n�ytt�rivit kohdalleen ja niihin oikeat parametrit
        std::string rowImageBaseDirectory = imageFileNameBase;
        AddImageRowPath(rowImageBaseDirectory, rowIndex);
        std::string rowImageFileNameBase = rowImageBaseDirectory;

        // pit�� varmistaa my�s ett� rivikohtaiset alihakemistot luodaan tarvittaessa, paitsi jos on k�ytetty 
        // rowIndex -leimaa tiedoston nimess�, t�ll�in ei saa siivota yht� ja samaa hakemistosa uudestaan ja uudestaan.
        CheckDestinationDirectory(rowImageBaseDirectory, deleteDestinationDirectory, theBetaProduct);
        rowImageFileNameBase += theBetaProduct.GetUsedFileNameTemplate(synopStationId != 0);

        std::string rowIndexString = "Row";
        rowIndexString += NFmiStringTools::Convert(rowIndex);
        NFmiMetTime modelOrigTime = GetFirstModelOrigTime(usedMapViewDesktopIndex, rowIndex);
        NFmiMetTime usedStartTime = useModelStartTime ? GetUsedModelTime(true, usedMapViewDesktopIndex, rowIndex, theStartingTime) : theStartingTime;
        NFmiMetTime usedEndTime = useModelStartTime ? GetUsedModelTime(false, usedMapViewDesktopIndex, rowIndex, theEndingTime) : theEndingTime;
        for(NFmiMetTime currentTime(usedStartTime); currentTime <= usedEndTime; currentTime.NextMetTime())
        {
            std::string imageFileName = ::MakeFinalImageFileName(itsSmartMetDocumentInterface, theBetaProduct, usedMapViewDesktopIndex, rowImageFileNameBase, currentTime, theMakeTime, modelOrigTime, rowIndex, synopStationId);
            itsSmartMetDocumentInterface->CurrentTime(usedMapViewDesktopIndex, currentTime);
            itsSmartMetDocumentInterface->DoOffScreenDraw(usedMapViewDesktopIndex, mapScreenBitmap);

            bool throwErrors = justLogMessages; // Jos ollaan automaatio moodissa, halutaan ett� CFmiGdiPlusHelpers::SafelySaveMfcBitmapToFile -funktio heittaa poikkeuksen (joka lokitetaan), eik� avaa MessageBoxia (t�m� tehd��n kun generoidaan tuotteita manuaalisesti)
            auto relativeViewImageArea = ::GetWantedImageRelativeRect(*descTop, usedMapViewDesktopIndex);
            CFmiGdiPlusHelpers::SafelySaveMfcBitmapToFile(__FUNCTION__, &mapScreenBitmap, imageFileName, &relativeViewImageArea, throwErrors);
            UpdateGeneratedImagesText(++itsTotalImagesGenerated, totalImageCount);
            itsImageGenerationProgressControl.StepIt();
        }
        MakeCommandLineExecution(rowImageBaseDirectory, theBetaProduct.CommandLineString(), justLogMessages);
    }
    mapScreenBitmap.DeleteObject();
}

// Oletus, jos t�nne p��see (= Generate nappi on enabloitu), ei tarvitse tehd� mit��n tarkasteluja.
void CFmiBetaProductDialog::MakeVisualizationImagesManually(const NFmiMetTime &theMakeTime)
{
    if(!RunViewMacro(*itsBetaProduct, false))
        return;

    itsBetaProductionSystem->BetaProductRuntime(theMakeTime);

    // Lasketaan start- ja end -time:t suoraan Beta-productin avulla
    NFmiBetaProductAutomation::NFmiTimeModeInfo startTimeMode(NFmiBetaProductAutomation::kFmiBetaProductTime);
    NFmiBetaProductAutomation::NFmiTimeModeInfo endTimeMode(NFmiBetaProductAutomation::kFmiBetaProductTime);

    NFmiMetTime startingTime = CalcStartingTime(*itsBetaProduct, startTimeMode, theMakeTime);
    NFmiMetTime endingTime = CalcEndingTime(*itsBetaProduct, endTimeMode, startingTime, theMakeTime);

    MakeVisualizationImages(*itsBetaProduct, startingTime, false, endingTime, false, theMakeTime, false);
}

void CFmiBetaProductDialog::DoBetaProductGenerations(std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> &theDueAutomations, const NFmiMetTime &theMakeTime)
{
    itsDueAutomations = &theDueAutomations;

    std::function<void(const NFmiMetTime&)> generationFunction = std::bind(&CFmiBetaProductDialog::DoBetaProductGenerationsFinal, this, std::placeholders::_1);
    DoImageProducingProcess(generationFunction, theMakeTime, true);

    itsDueAutomations = nullptr;
}

static void UpdateAutomationAfterProduction(std::shared_ptr<NFmiBetaProductAutomationListItem> &theAutomationListItem, const NFmiMetTime &theMakeTime)
{
    if(theAutomationListItem)
    {
        theAutomationListItem->itsLastRunTime = theMakeTime;
        theAutomationListItem->fProductsHaveBeenGenerated = true;
        theAutomationListItem->itsNextRunTime = theAutomationListItem->itsBetaProductAutomation->TriggerModeInfo().CalcNextDueTime(theMakeTime);
    }
}

void CFmiBetaProductDialog::DoBetaProductGenerationsFinal(const NFmiMetTime &theMakeTime)
{
    if(!itsDueAutomations)
    {
        std::string errorDialogTitle = "Unknown automated Beta-product errror";
        std::string errorStr = "Unknown error when generating Beta-products. itsDueAutomations work-list was null-pointer.";
        itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorDialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, true);
    }
    else
    {
        itsBetaProductionSystem->BetaProductRuntime(theMakeTime);
        for(auto & automationListItem : *itsDueAutomations)
        {
            std::shared_ptr<NFmiBetaProduct> betaProduct = automationListItem->itsBetaProductAutomation->GetBetaProduct(true); // true = pakotetaan lukemaan Beta-product tiedostosta, jos vaikka joku on k�ynyt sit� muuttamassa
            if(!betaProduct)
            {
                std::string errorDialogTitle = "Unknown automated Beta-product errror";
                std::string errorStr = "Unknown error when generating Beta-products. Unable to load proper Beta-product from file:\n";
                errorStr += automationListItem->itsBetaProductAutomation->BetaProductPath();
                itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorDialogTitle, CatLog::Severity::Error, CatLog::Category::Operational, true);
            }
            else
            {
                if(!RunViewMacro(*betaProduct, true))
                {
                    ::UpdateAutomationAfterProduction(automationListItem, theMakeTime); // T�m� pit�� p�ivitt�� vaikka menikin pieleen
                    continue;
                }

                NFmiMetTime startingTime = CalcStartingTime(*betaProduct, automationListItem->itsBetaProductAutomation->StartTimeModeInfo(), theMakeTime);
                NFmiMetTime endingTime = CalcEndingTime(*betaProduct, automationListItem->itsBetaProductAutomation->EndTimeModeInfo(), startingTime, theMakeTime);

                bool useModelStartingTime = automationListItem->itsBetaProductAutomation->StartTimeModeInfo().itsTimeMode == NFmiBetaProductAutomation::kFmiFirstModelDataTime;
                bool useModelEndingTime = automationListItem->itsBetaProductAutomation->EndTimeModeInfo().itsTimeMode == NFmiBetaProductAutomation::kFmiFirstModelDataTime;
                MakeVisualizationImages(*betaProduct, startingTime, useModelStartingTime, endingTime, useModelEndingTime, theMakeTime, true);
                ::UpdateAutomationAfterProduction(automationListItem, theMakeTime);
            }
        }
    }
}


void CFmiBetaProductDialog::UpdateRowInfo(bool fUpdateTimeSectionInTheEnd)
{
    UpdateData(TRUE);

    itsBetaProduct->CheckRowRelatedInputs(CFmiWin32Helpers::CT2std(itsRowIndexListStringU_), CFmiWin32Helpers::CT2std(itsRowSubdirectoryTemplateStringU_), CFmiWin32Helpers::CT2std(itsFileNameTemplateU_), fUseAutoFileNames == TRUE, GetSelectedBoxLocation(false));
    itsRowIndexListInfoTextU_ = CA2T(itsBetaProduct->RowIndexListInfoText().c_str());

    CheckForGenerateButtonActivation();
    UpdateData(FALSE);

    if(fUpdateTimeSectionInTheEnd) // Kun rivien lukum��r�� muutetaan, pit�� my�s aikaan liittyvi� tekstej� p�ivitt��
        Update();
}

void CFmiBetaProductDialog::UpdateSynopStationIdInfo()
{
    UpdateData(TRUE);
    itsBetaProduct->CheckSynopStationIdListRelatedInputs(CFmiWin32Helpers::CT2std(itsSynopStationIdListTextU_));
    CheckForGenerateButtonActivation();
    UpdateData(FALSE);

    UpdateSynopStationEditColors();
}

void CFmiBetaProductDialog::UpdateSynopStationEditColors()
{
    // Pakotetaan viel� edit boxin p�ivitys, jotta mahdollinen virhetilanne v�rj�� tekstin punaiseksi (tai korjattu virhe tekee tekstin mustaksi takaisin)
    auto synopEditCtrl = GetDlgItem(IDC_EDIT_SYNOP_STATION_ID_STRING);
    if(synopEditCtrl)
        synopEditCtrl->Invalidate(TRUE);

}

void CFmiBetaProductDialog::OnEnChangeEditSynopStationIdString()
{
    UpdateSynopStationIdInfo();
}

bool CFmiBetaProductDialog::IsFileNameTemplateStampsOk() const
{
    return fFileNameTemplateContainsValidTime || fUseAutoFileNames;
}

std::string CFmiBetaProductDialog::GetFileNameTemplateStampsString()
{
    if(IsFileNameTemplateStampsOk())
    {
        std::string stampHelperString("('Stamps': ");
        stampHelperString += NFmiBetaProductionSystem::FileNameTemplateValidTimeStamp();
        stampHelperString += ", ";
        stampHelperString += g_OrigTimeStampString;
        stampHelperString += ", ";
        stampHelperString += g_MakeTimeStampString;
        stampHelperString += ", ";
        stampHelperString += NFmiBetaProductionSystem::FileNameTemplateStationIdStamp();
        stampHelperString += ")";
        return stampHelperString;
    }
    else
        return std::string("File name must contain '") + NFmiBetaProductionSystem::FileNameTemplateValidTimeStamp() + "'";
}

void CFmiBetaProductDialog::UpdateFileNameTemplateInfo()
{
    UpdateData(TRUE);
    itsBetaProduct->UseAutoFileNames(fUseAutoFileNames == TRUE);
    fFileNameTemplateContainsValidTime = NFmiBetaProduct::ContainsStringCaseInsensitive(CFmiWin32Helpers::CT2std(itsFileNameTemplateU_), NFmiBetaProductionSystem::FileNameTemplateValidTimeStamp());
    itsFileNameTemplateStampsStringU_ = CA2T(::GetDictionaryString(GetFileNameTemplateStampsString().c_str()).c_str());

    CheckForGenerateButtonActivation();
    EnableFileNameTemplateEdit();
    UpdateData(FALSE);
}

void CFmiBetaProductDialog::OnEnChangeEditRowIndexList()
{
    UpdateRowInfo(true);
}


void CFmiBetaProductDialog::OnEnChangeEditRowSubdirectoryTemplateString()
{
    UpdateRowInfo(true);
}


void CFmiBetaProductDialog::OnEnChangeEditFileNameTemplate()
{
    UpdateRowInfo(true);
    UpdateFileNameTemplateInfo();
}

void CFmiBetaProductDialog::OnBnClickedRadioMainMapView()
{
    UpdateViewSelection();
}

void CFmiBetaProductDialog::OnBnClickedRadioMapView2()
{
    UpdateViewSelection();
}

void CFmiBetaProductDialog::OnBnClickedRadioMapView3()
{
    UpdateViewSelection();
}

void CFmiBetaProductDialog::OnBnClickedRadioTimeSerialView()
{
    UpdateViewSelection();
}


void CFmiBetaProductDialog::OnBnClickedRadioSoundingView()
{
    UpdateViewSelection();
}


void CFmiBetaProductDialog::OnBnClickedRadioCrossSectionView()
{
    UpdateViewSelection();
}

void CFmiBetaProductDialog::UpdateViewSelection()
{
    UpdateData(TRUE);
    itsBetaProduct->SelectedViewIndex(itsSelectedViewIndex);
    Update(); // Aika tekstit pit�� my�s p�ivitt�� kun n�ytt� vaihtuu
    UpdateSynopStationEditColors();
}

void CFmiBetaProductDialog::OnEnChangeEditViewMacroPath()
{
    UpdateViewMacroInfo();
}

void CFmiBetaProductDialog::UpdateViewMacroInfo()
{
    UpdateData(TRUE);
    std::string infoText = itsBetaProduct->MakeViewMacroInfoText(CFmiWin32Helpers::CT2std(itsViewMacroPathU_));
    itsViewMacroErrorTextU_ = CA2T(infoText.c_str());
    CheckForGenerateButtonActivation();
    UpdateData(FALSE);
}

void CFmiBetaProductDialog::UpdateWebSiteInfo()
{
    UpdateData(TRUE);
    itsBetaProduct->WebSiteTitleString(CFmiWin32Helpers::CT2std(itsWebSiteTitleStringU_));
    itsBetaProduct->WebSiteDescriptionString(CFmiWin32Helpers::CT2std(itsWebSiteDescriptionStringU_));
}

HBRUSH CFmiBetaProductDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CTabPageSSL::OnCtlColor(pDC, pWnd, nCtlColor);

    if(pWnd->GetDlgCtrlID() == IDC_STATIC_VIEW_MACRO_ERROR_TEXT)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsBetaProduct->GivenViewMacroOk());
    else if(pWnd->GetDlgCtrlID() == IDC_STATIC_DEDUCED_INFO_OF_IMAGE_PRODUCTION)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsBetaProduct->TimeInputOk());
    else if(pWnd->GetDlgCtrlID() == IDC_STATIC_DEDUCED_INFO_FROM_USED_ROWS)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsBetaProduct->RowIndexInputOk());
    else if(pWnd->GetDlgCtrlID() == IDC_STATIC_IMAGE_FILE_NAME_TEMPLATE_STAMPS_TEXT)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, IsFileNameTemplateStampsOk());
    else if(pWnd->GetDlgCtrlID() == IDC_EDIT_SYNOP_STATION_ID_STRING)
    {
        // T��ll� kontollilla voi olla 3 v�ri�: virhe=punainen, ei k�yt�ss� = harmaa ja normaali = musta
        bool isSynopControlUsed = (itsBetaProduct->SelectedViewIndex() > BetaProductViewIndex::kMapView3);
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, itsBetaProduct->SynopStationIdListInputOk(), isSynopControlUsed);
    }
    return hbr;
}

void CFmiBetaProductDialog::OnBnClickedButtonViewMacroPathBrowse()
{
    static TCHAR BASED_CODE szFilter[] = _TEXT("ViewMacro Files (*.vmr)|*.vmr|All Files (*.*)|*.*||");

    UpdateData(TRUE);

    std::string viewMacroPath = itsBetaProduct->ViewMacroPath();

    std::string initialDirectory = PathUtils::getPathSectionFromTotalFilePath(viewMacroPath);

    CFileDialog dlg(TRUE, NULL, CA2T(viewMacroPath.c_str()), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
    dlg.m_ofn.lpstrInitialDir = CA2T(initialDirectory.c_str());
    if(dlg.DoModal() == IDOK)
    {
        itsViewMacroPathU_ = dlg.GetPathName();
        std::string relativeFilePath = PathUtils::getRelativePathIfPossible(CFmiWin32Helpers::CT2std(itsViewMacroPathU_), itsSmartMetDocumentInterface->RootViewMacroPath());
        itsViewMacroPathU_ = CA2T(relativeFilePath.c_str());
        UpdateData(FALSE);
        UpdateViewMacroInfo();
    }
}

void CFmiBetaProductDialog::OnBnClickedButtonSaveBetaProduct()
{
    StoreControlValuesToDocument(); // Ennen tallennusta talletetaan varmuuden vuoksi s��d�t my�s dokumenttiin

    BetaProduct::SaveObjectToKnownFileInJsonFormat(*itsBetaProduct, itsBetaProductFullFilePath, "Beta-product", false);
}

void CFmiBetaProductDialog::OnBnClickedButtonSaveAsBetaProduct()
{
    StoreControlValuesToDocument(); // Ennen tallennusta talletetaan varmuuden vuoksi s��d�t my�s dokumenttiin

    BetaProduct::SaveObjectInJsonFormat(*itsBetaProduct, BetaProduct::InitialSavePath(), NFmiBetaProductionSystem::BetaProductFileFilter(), NFmiBetaProductionSystem::BetaProductFileExtension(), itsBetaProductionSystem->GetBetaProductionBaseDirectory(true), "Beta-product", false, &itsBetaProductFullFilePath);
    UpdateBetaProductName();
}

void CFmiBetaProductDialog::OnBnClickedButtonLoadBetaProduct()
{
    if(BetaProduct::LoadObjectInJsonFormat(*itsBetaProduct, BetaProduct::InitialSavePath(), NFmiBetaProductionSystem::BetaProductFileFilter(), NFmiBetaProductionSystem::BetaProductFileExtension(), itsBetaProductionSystem->GetBetaProductionBaseDirectory(true), "Beta-product", false, &itsBetaProductFullFilePath))
    {
        itsBetaProduct->InitFromJsonRead(GetCurrentViewTime(*itsBetaProduct));
        UpdateBetaProductName();
        InitControlsFromLoadedBetaProduct();
    }
}

void CFmiBetaProductDialog::UpdateBetaProductName()
{
    itsBetaProductNameU_ = CA2T(PathUtils::getRelativeStrippedFileName(itsBetaProductFullFilePath, itsBetaProductionSystem->GetBetaProductionBaseDirectory(true), NFmiBetaProductionSystem::BetaProductFileExtension()).c_str());
    UpdateData(FALSE);
}

void CFmiBetaProductDialog::OnEnChangeEditImageDirectory()
{
    Update();
}


void CFmiBetaProductDialog::OnEnChangeEditWebTitleString()
{
    UpdateWebSiteInfo();
}


void CFmiBetaProductDialog::OnEnChangeEditWebDescriptionString()
{
    UpdateWebSiteInfo();
}


void CFmiBetaProductDialog::OnDestroy()
{
    CTabPageSSL::OnDestroy();

    StoreControlValuesToDocument();
}

void CFmiBetaProductDialog::UpdateCommandLine()
{
    UpdateData(TRUE);
    itsBetaProduct->CommandLineString(CFmiWin32Helpers::CT2std(itsCommandLineStringU_));
}

void CFmiBetaProductDialog::OnEnChangeEditCommandLineString()
{
    UpdateCommandLine();
}

void CFmiBetaProductDialog::InitLocationSelector(CComboBox &theLocationSelector)
{
    theLocationSelector.ResetContent();

    // HUOM! T��ll� tehty j�rjestys m��r�� GetSelectedBoxLocation- ja SetBoxLocationSelector -metodien koodit.
    theLocationSelector.AddString(CA2T(::GetDictionaryString("Bottom-left").c_str()));
    theLocationSelector.AddString(CA2T(::GetDictionaryString("Top-left").c_str()));
    theLocationSelector.AddString(CA2T(::GetDictionaryString("Bottom-right").c_str()));
    theLocationSelector.AddString(CA2T(::GetDictionaryString("Top-right").c_str()));
    theLocationSelector.AddString(CA2T(::GetDictionaryString("Bottom-center").c_str()));
    theLocationSelector.AddString(CA2T(::GetDictionaryString("Top-center").c_str()));
    theLocationSelector.AddString(CA2T(::GetDictionaryString("Disable").c_str()));
}

FmiDirection CFmiBetaProductDialog::GetSelectedBoxLocation(bool fTimeBoxCase)
{
    // Ks. j�rjestys CFmiBetaProductDialog::InitLocationSelector -metodista.
    int currentSelectionIndex = fTimeBoxCase ? itsTimeBoxLocationSelector.GetCurSel() : itsParamBoxLocationSelector.GetCurSel();
    switch(currentSelectionIndex)
    {
    case 0:
        return kBottomLeft;
    case 1:
        return kTopLeft;
    case 2:
        return kBottomRight;
    case 3:
        return kTopRight;
    case 4:
        return kBottomCenter;
    case 5:
        return kTopCenter;
    case 6:
        return kNoDirection;
    default:
        return fTimeBoxCase ? kBottomLeft : kNoDirection; // En tied� mit� virhetilanteissa pit�isi tehd�, joten palautetaan silloin oletusarvoa
    }
}

void CFmiBetaProductDialog::SetBoxLocationSelector(CComboBox &theBoxLocationSelector, FmiDirection newValue)
{
    // Ks. j�rjestys CFmiBetaProductDialog::InitLocationSelector -metodista.
    if(newValue == kBottomLeft)
        theBoxLocationSelector.SetCurSel(0);
    else if(newValue == kTopLeft)
        theBoxLocationSelector.SetCurSel(1);
    else if(newValue == kBottomRight)
        theBoxLocationSelector.SetCurSel(2);
    else if(newValue == kTopRight)
        theBoxLocationSelector.SetCurSel(3);
    else if(newValue == kBottomCenter)
        theBoxLocationSelector.SetCurSel(4);
    else if(newValue == kTopCenter)
        theBoxLocationSelector.SetCurSel(5);
    else if(newValue == kNoDirection)
        theBoxLocationSelector.SetCurSel(6);
    else
        theBoxLocationSelector.SetCurSel(0); // En tied� mit� virhetilanteissa pit�isi tehd�, joten asetetaan silloin oletusarvo p��lle

}

void CFmiBetaProductDialog::OnCbnSelchangeComboTimeBoxLocationSelector()
{
    Update();
}


void CFmiBetaProductDialog::OnBnClickedCheckTimeBoxUseUtcTime()
{
    Update();
}


void CFmiBetaProductDialog::OnBnClickedCheckUseAutoFileNames()
{
    UpdateFileNameTemplateInfo();
}


void CFmiBetaProductDialog::OnCbnSelchangeComboParamBoxLocationSelector()
{
    UpdateRowInfo(false);
}


void CFmiBetaProductDialog::OnBnClickedCheckDisplayRuntimeInfo()
{
    UpdateData(TRUE);
    itsBetaProduct->DisplayRunTimeInfo(fDisplayRuntimeInfo == TRUE);
}
