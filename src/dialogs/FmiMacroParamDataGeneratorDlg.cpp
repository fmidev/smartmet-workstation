// CFmiMacroParamDataGeneratorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "FmiMacroParamDataGeneratorDlg.h"
#include "SmartMetDialogs_resource.h"
#include "SmartMetDocumentInterface.h"
#include "FmiWin32Helpers.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiMacroParamDataGenerator.h"
#include "FmiWin32TemplateHelpers.h"
#include "persist2.h"
#include "CFmiVisualizationSettings.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiSmartToolModifier.h"
#include "NFmiValueString.h"
#include "GridCellCheck.h"
#include <boost/algorithm/string.hpp>
#include <thread>

// Jos painaa "Add smarttool" nappia ja avatusta CFileDialog dialogista tekee
// smarttool tiedoston valinnan tuplaklikkaamalla ja tuplaklikkaus tapahtuu
// Automaatiolista kontrollin p��ll�, se j�lkimm�inen klikkauksista j�� 'muistiin'
// ja se toteutetaan ja klikatun rivin automaatio tulee valituksi editoitavaksi.
// Siksi tehd��n seuraavanlainen operaatio:
// 1) Kun ollaan CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonAddUsedSmarttoolPath metodissa, laitetaan timer p��lle
// 2) Kun tullaan NFmiMacroParDataAutomationGridCtrl::OnLButtonUp metodiin, tarkistetaan jos timerin aika on liian lyhyt (n. < 0.2 s), ei tehd� mit��n
NFmiNanoSecondTimer gAddSmarttoolPathDoubleClickBugPreventerTimer;

// ***********************************************
// NFmiMacroParDataAutomationGridCtrl grid-control
// ***********************************************

IMPLEMENT_DYNCREATE(NFmiMacroParDataAutomationGridCtrl, CGridCtrl)

BEGIN_MESSAGE_MAP(NFmiMacroParDataAutomationGridCtrl, CGridCtrl)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void NFmiMacroParDataAutomationGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnRButtonUp(nFlags, point);

	// Tarkoitus on deselectoida kaikki
	SetSelectedRange(-1, -1, -1, -1, TRUE, TRUE);

	if(itsRightClickUpCallback)
		itsRightClickUpCallback(GetCellFromPt(point));
}

void NFmiMacroParDataAutomationGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnLButtonUp(nFlags, point);

	if(gAddSmarttoolPathDoubleClickBugPreventerTimer.elapsedTimeInSeconds() < 0.4)
		return;

	if(itsLeftClickUpCallback)
		itsLeftClickUpCallback(GetCellFromPt(point));
}


// ***********************************************
// CFmiMacroParamDataGeneratorDlg dialog
// ***********************************************

const NFmiViewPosRegistryInfo CFmiMacroParamDataGeneratorDlg::s_ViewPosRegistryInfo(CRect(300, 200, 793, 739), "\\MacroParamDataGenerator");

IMPLEMENT_DYNAMIC(CFmiMacroParamDataGeneratorDlg, CDialogEx)

CFmiMacroParamDataGeneratorDlg::CFmiMacroParamDataGeneratorDlg(SmartMetDocumentInterface* smartMetDocumentInterface, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MACRO_PARAM_DATA_GENERATOR, pParent)
	,itsGridCtrl()
	,itsHeaders()
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	,itsMacroParamDataGenerator(smartMetDocumentInterface ? &(smartMetDocumentInterface->GetMacroParamDataGenerator()) : nullptr)
	, itsBaseDataParamProducerString(_T(""))
	, itsProducerIdNamePairString(_T(""))
	, itsUsedDataGenerationSmarttoolPathList(_T(""))
	, itsUsedParameterListString(_T(""))
	, itsGeneratedDataStorageFileFilter(_T(""))
	, mLoadedMacroParamDataInfoName(_T(""))
	, itsDataTriggerList(_T(""))
	, itsMaxGeneratedFilesKept(_T(""))
	, mGeneratedDataInfoStr(_T(""))
	, fAutomationModeOn(FALSE)
	, itsEditedMacroParamDataInfo(std::make_shared<NFmiMacroParamDataInfo>())
{
}

CFmiMacroParamDataGeneratorDlg::~CFmiMacroParamDataGeneratorDlg()
{
}

void CFmiMacroParamDataGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_GridControl(pDX, IDC_CUSTOM_GRID_MACRO_PARAM_DATA_AUTOMATION_LIST, itsGridCtrl);
	DDX_Text(pDX, IDC_EDIT_BASE_DATA_PAR_PROD_LEV, itsBaseDataParamProducerString);
	DDX_Text(pDX, IDC_EDIT_PRODUCER_ID_NAME_PAIR, itsProducerIdNamePairString);
	DDX_Text(pDX, IDC_EDIT_USED_DATA_GENERATION_SMARTTOOL_PATH_LIST, itsUsedDataGenerationSmarttoolPathList);
	DDX_Text(pDX, IDC_EDIT_USED_PARAMETER_LIST, itsUsedParameterListString);
	DDX_Text(pDX, IDC_EDIT_GENERATED_DATA_STORAGE_FILE_FILTER, itsGeneratedDataStorageFileFilter);
	DDX_Control(pDX, IDC_PROGRESS_OF_OPERATION_BAR, mProgressControl);
	DDX_Text(pDX, IDC_STATIC_LOADED_MACRO_PARAM_DATA_INFO_NAME, mLoadedMacroParamDataInfoName);
	DDX_Text(pDX, IDC_EDIT_USED_DATA_TRIGGER_LIST, itsDataTriggerList);
	DDX_Text(pDX, IDC_EDIT_MAX_GENERATED_FILES_KEPT, itsMaxGeneratedFilesKept);
	DDX_Control(pDX, IDC_BUTTON_GENERATE_MACRO_PARAM_DATA, itsGenerateMacroParamDataButton);
	DDX_Control(pDX, IDC_BUTTON_SAVE_MACRO_PARAM_DATA, itsSaveMacroParamDataInfoButton);
	DDX_Control(pDX, IDC_BUTTON_SAVE_MACRO_PARAM_DATA_AUTOMATION_LIST, itsSaveMacroParamDataAutomationListButton);
	DDX_Control(pDX, IDC_BUTTON_RUN_SELECTED_MACRO_PARAM_DATA_AUTOMATION, itsRunSelectedMacroParamDataAutomationButton);
	DDX_Control(pDX, IDC_BUTTON_RUN_ENABLED_MACRO_PARAM_DATA_AUTOMATIONS, itsRunEnabledMacroParamDataAutomationButton);
	DDX_Control(pDX, IDC_BUTTON_RUN_ALL_MACRO_PARAM_DATA_AUTOMATIONS, itsRunAllMacroParamDataAutomationButton);
	DDX_Control(pDX, IDC_BUTTON_ADD_EDITED_MACRO_PARAM_DATA_AUTOMATION_TO_LIST, itsAddEditedMacroParamDataAutomationToListButton);
	DDX_Control(pDX, IDC_BUTTON_ADD_MACRO_PARAM_DATA_AUTOMATION_TO_LIST, itsAddFromFileMacroParamDataAutomationToListButton);
	DDX_Control(pDX, IDC_BUTTON_REMOVE_MACRO_PARAM_DATA_AUTOMATION_FROM_LIST, itsRemoveMacroParamDataAutomationFromListButton);
	DDX_Control(pDX, IDC_BUTTON_LOAD_MACRO_PARAM_DATA_AUTOMATION_LIST, itsLoadMacroParamDataAutomationListButton);
	DDX_Control(pDX, IDC_BUTTON_LOAD_MACRO_PARAM_DATA, itsLoadMacroParamDataInfoButton);
	DDX_Control(pDX, IDC_BUTTON_CANCEL_DATA_GENERATION, itsCancelDataGenerationButton);
	DDX_Check(pDX, IDC_CHECK_MACRO_PARAM_DATA_AUTOMATION_MODE_ON, fAutomationModeOn);
	DDX_Text(pDX, IDC_STATIC_MACRO_PARAM_DATA_AUTOMATION_LIST_NAME_VALUE, mLoadedMacroParamDataAutomationListName);
	DDX_Text(pDX, IDC_STATIC_GENERATED_DATA_INFO_STR, mGeneratedDataInfoStr);
	DDX_Text(pDX, IDC_EDIT_BASE_DATA_GRID_SCALE, mBaseDataGridScaleString);
	DDX_Text(pDX, IDC_EDIT_CPU_USAGE_PERCENTAGE, itsCpuUsagePercentageString);
}

BEGIN_MESSAGE_MAP(CFmiMacroParamDataGeneratorDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_MACRO_PARAM_DATA, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonGenerateMacroParamData)
	ON_EN_CHANGE(IDC_EDIT_BASE_DATA_PARAM_PRODUCER, &CFmiMacroParamDataGeneratorDlg::OnChangeEditBaseDataParamProducer)
	ON_WM_CTLCOLOR()
	ON_EN_CHANGE(IDC_EDIT_PRODUCER_ID_NAME_PAIR, &CFmiMacroParamDataGeneratorDlg::OnChangeEditProducerIdNamePair)
	ON_EN_CHANGE(IDC_EDIT_USED_PARAMETER_LIST, &CFmiMacroParamDataGeneratorDlg::OnChangeEditUsedParameterList)
	ON_EN_CHANGE(IDC_EDIT_USED_DATA_GENERATION_SMARTTOOL_PATH_LIST, &CFmiMacroParamDataGeneratorDlg::OnChangeEditUsedDataGenerationSmarttoolPathList)
	ON_EN_CHANGE(IDC_EDIT_GENERATED_DATA_STORAGE_FILE_FILTER, &CFmiMacroParamDataGeneratorDlg::OnChangeEditGeneratedDataStorageFileFilter)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_MACRO_PARAM_DATA, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonSaveMacroParamData)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_MACRO_PARAM_DATA, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonLoadMacroParamData)
	ON_EN_CHANGE(IDC_EDIT_USED_DATA_TRIGGER_LIST, &CFmiMacroParamDataGeneratorDlg::OnEnChangeEditUsedDataTriggerList)
	ON_EN_CHANGE(IDC_EDIT_MAX_GENERATED_FILES_KEPT, &CFmiMacroParamDataGeneratorDlg::OnEnChangeEditMaxGeneratedFilesKept)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_BUTTON_ADD_EDITED_MACRO_PARAM_DATA_AUTOMATION_TO_LIST, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonAddEditedMacroParamDataAutomationToList)
	ON_BN_CLICKED(IDC_BUTTON_ADD_MACRO_PARAM_DATA_AUTOMATION_TO_LIST, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonAddMacroParamDataAutomationToList)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_MACRO_PARAM_DATA_AUTOMATION_FROM_LIST, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonRemoveMacroParamDataAutomationFromList)
	ON_BN_CLICKED(IDC_CHECK_MACRO_PARAM_DATA_AUTOMATION_MODE_ON, &CFmiMacroParamDataGeneratorDlg::OnBnClickedCheckMacroParamDataAutomationModeOn)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_MACRO_PARAM_DATA_AUTOMATION_LIST, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonSaveMacroParamDataAutomationList)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_MACRO_PARAM_DATA_AUTOMATION_LIST, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonLoadMacroParamDataAutomationList)
	ON_BN_CLICKED(IDC_BUTTON_RUN_SELECTED_MACRO_PARAM_DATA_AUTOMATION, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonRunSelectedMacroParamDataAutomation)
	ON_BN_CLICKED(IDC_BUTTON_RUN_ENABLED_MACRO_PARAM_DATA_AUTOMATIONS, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonRunEnabledMacroParamDataAutomations)
	ON_BN_CLICKED(IDC_BUTTON_RUN_ALL_MACRO_PARAM_DATA_AUTOMATIONS, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonRunAllMacroParamDataAutomations)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL_DATA_GENERATION, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonCancelDataGeneration)
	ON_BN_CLICKED(IDC_BUTTON_ADD_USED_SMARTTOOL_PATH, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonAddUsedSmarttoolPath)
	ON_EN_CHANGE(IDC_EDIT_BASE_DATA_GRID_SCALE, &CFmiMacroParamDataGeneratorDlg::OnEnChangeEditBaseDataGridScale)
	ON_EN_CHANGE(IDC_EDIT_CPU_USAGE_PERCENTAGE, &CFmiMacroParamDataGeneratorDlg::OnEnChangeEditCpuUsagePercentage)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_STORED_DATA_FILE_FILTER, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonBrowseStoredDataFileFilter)
END_MESSAGE_MAP()

// CFmiMacroParamDataGeneratorDlg message handlers

static void SetHeaders(CGridCtrl& theGridCtrl, const std::vector<MacroParDataAutomationHeaderParInfo>& theHeaders)
{
	static const COLORREF gFixedBkColor = RGB(239, 235, 222);

	int columnCount = static_cast<int>(theHeaders.size());
	theGridCtrl.SetColumnCount(columnCount);
	theGridCtrl.SetFixedRowCount(1);
	theGridCtrl.SetFixedColumnCount(1);
	theGridCtrl.SetGridLines(GVL_BOTH);
	theGridCtrl.SetListMode(TRUE);
	theGridCtrl.SetHeaderSort(FALSE);
	theGridCtrl.SetFixedBkColor(gFixedBkColor);
	theGridCtrl.SetSingleColSelection(TRUE);
	theGridCtrl.SetSingleRowSelection(TRUE);

	int currentRow = 0;
	// 1. on otsikko rivi on parametrien nimi� varten
	for(int i = 0; i < columnCount; i++)
	{
		theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
		theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
		theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
	}
}


BOOL CFmiMacroParamDataGeneratorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// T�t� DoResizerHooking:ia pit�� kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisterist�
	DoResizerHooking(); 

	std::string errorBaseStr("Error in CFmiMacroParamDataGeneratorDlg::OnInitDialog while reading dialog size and position values");
	CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	itsGridCtrl.SetLeftClickUpCallback(std::bind(&CFmiMacroParamDataGeneratorDlg::SelectedGridCell, this, std::placeholders::_1));
	itsGridCtrl.SetRightClickUpCallback(std::bind(&CFmiMacroParamDataGeneratorDlg::DeselectGridCell, this, std::placeholders::_1));
	InitHeaders();
	SetHeaders(itsGridCtrl, itsHeaders);

	InitDialogTexts();
	InitControlsFromDocument();
	DoFullInputChecks();
	EnableButtons();
	UpdateAutomationList();

	mProgressControl.SetRange(0, 100);
	mProgressControl.SetStep(1);
	mProgressControl.SetShowPercent(TRUE);
	mThreadCallBacksPtr = std::make_unique<NFmiThreadCallBacks>(&mStopper, this);
	ShowCancelButton(false);
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiMacroParamDataGeneratorDlg::SelectedGridCell(const CCellID& theSelectedCell)
{
	if(theSelectedCell.IsValid())
	{
		int zeroBasedRowIndex = theSelectedCell.row - itsGridCtrl.GetFixedRowCount();
		auto& selectedListItem = itsMacroParamDataGenerator->UsedMacroParamDataAutomationList().Get(zeroBasedRowIndex);
		if(!selectedListItem.IsEmpty())
		{
			itsEditedMacroParamDataInfo = selectedListItem.itsMacroParamDataAutomation;
			UpdateMacroParamDataInfoName(selectedListItem.itsMacroParamDataAutomationAbsolutePath);
			InitControlsFromLoadedMacroParamDataInfo();
			DoFullInputChecks();
			EnableButtons();
		}
	}
}

void CFmiMacroParamDataGeneratorDlg::DeselectGridCell(const CCellID& theSelectedCell)
{
	// Grid-ctrl on deselectoinut itsens�, nyt on tarkoitus laittaa eri kontrollit oikeisiin tiloihin
	EnableButtons();
}

void CFmiMacroParamDataGeneratorDlg::InitHeaders()
{
	int basicColumnWidthUnit = 18;
	itsHeaders.clear();

	itsHeaders.push_back(MacroParDataAutomationHeaderParInfo(::GetDictionaryString("Row"), MacroParDataAutomationHeaderParInfo::kRowNumber, boost::math::iround(basicColumnWidthUnit * 2.3)));
	itsHeaders.push_back(MacroParDataAutomationHeaderParInfo(::GetDictionaryString("Name"), MacroParDataAutomationHeaderParInfo::kAutomationName, basicColumnWidthUnit * 9));
	itsHeaders.push_back(MacroParDataAutomationHeaderParInfo(::GetDictionaryString("Enable"), MacroParDataAutomationHeaderParInfo::kEnable, basicColumnWidthUnit * 3));
	itsHeaders.push_back(MacroParDataAutomationHeaderParInfo(::GetDictionaryString("Next"), MacroParDataAutomationHeaderParInfo::kNextRuntime, boost::math::iround(basicColumnWidthUnit * 3.2)));
	itsHeaders.push_back(MacroParDataAutomationHeaderParInfo(::GetDictionaryString("Last"), MacroParDataAutomationHeaderParInfo::kLastRuntime, boost::math::iround(basicColumnWidthUnit * 3.2)));
	itsHeaders.push_back(MacroParDataAutomationHeaderParInfo(::GetDictionaryString("Status"), MacroParDataAutomationHeaderParInfo::kAutomationStatus, boost::math::iround(basicColumnWidthUnit * 8.)));
	itsHeaders.push_back(MacroParDataAutomationHeaderParInfo(::GetDictionaryString("Path"), MacroParDataAutomationHeaderParInfo::kAutomationPath, boost::math::iround(basicColumnWidthUnit * 30.)));
}

void CFmiMacroParamDataGeneratorDlg::InitDialogTexts()
{
	SetWindowText(CA2T(::GetDictionaryString("MacroParam data generator").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BASE_DATA_PARAM_PRODUCER_STR, "Base data's parameter+producer\nE.g. 'T_Ec' or 'par4_prod240' (no level!)");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PRODUCER_ID_NAME_PAIR_STR, "Generated data's producer id,name -pair\nE.g. 3001,ProducerName");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DATA_GENERATION_SMARTTOOL_PATH_LIST_STR, "List of used data generation smarttool paths, comma separated");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_ADD_USED_SMARTTOOL_PATH, "Add smarttool");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_USED_PARAMETER_LIST_STR, "Used generated data parameter list (id,name -pairs) \nE.g. \"6201, Param X, 6202, Param Y, ...\"    (Good id range 5000-9000)");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GENERATED_DATA_STORAGE_FILE_FILTER_STR, "Generated data's storage file filter (* is replaced with time-stamp)\nE.g. C:\\data\\*_mydata.sqd");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_BROWSE_STORED_DATA_FILE_FILTER, "Browse stored path");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_GENERATE_MACRO_PARAM_DATA, "Generate MacroParam data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GROUP_MACRO_PARAM_DATA_INFO, "MacroParam data info section");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SAVE_MACRO_PARAM_DATA, "Save MacroPar data info");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOAD_MACRO_PARAM_DATA, "Load MacroPar data info");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GROUP_DATA_GENERATION_LIST, "Data generation list");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_USED_DATA_TRIGGER_LIST, "Data trigger list, e.g. T_ec[,T_gfs_500, ...]\nTrigger delayed: T_ec[0.5h] ->delays half an hour");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GROUP_DATA_GENERATION_LIST, "Data generation list");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PARAM_DATA_AUTOMATION_LIST_NAME_TEXT, "List name:");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_ADD_EDITED_MACRO_PARAM_DATA_AUTOMATION_TO_LIST, "Add edited");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_ADD_MACRO_PARAM_DATA_AUTOMATION_TO_LIST, "Add from file");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REMOVE_MACRO_PARAM_DATA_AUTOMATION_FROM_LIST, "Remove");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SAVE_MACRO_PARAM_DATA_AUTOMATION_LIST, "Save list");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOAD_MACRO_PARAM_DATA_AUTOMATION_LIST, "Load list");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_MACRO_PARAM_DATA_AUTOMATION_MODE_ON, "Automation mode on");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RUN_SELECTED_MACRO_PARAM_DATA_AUTOMATION, "Run selected");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RUN_ENABLED_MACRO_PARAM_DATA_AUTOMATIONS, "Run enabled");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RUN_ALL_MACRO_PARAM_DATA_AUTOMATIONS, "Run all");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MACRO_PARAM_INFO_NAME_STR, "Macro name");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CANCEL_DATA_GENERATION, "Cancel data generation");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MAX_GENERATED_FILES_KEPT_STR, "Max generated files kept\n(1 - 10) on target directory");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BASE_DATA_GRID_SCALE_STR, "Generated data grid scale\n(1-10) or (1-10),(1-10) x/y");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_CPU_USAGE_PERCENTAGE_STR, "CPU usage [%] (10-100)");
}

void CFmiMacroParamDataGeneratorDlg::InitControlsFromDocument()
{
	UpdateMacroParamDataInfoName(itsMacroParamDataGenerator->AutomationPath());
	UpdateMacroParamDataAutomationListName(itsMacroParamDataGenerator->AutomationListPath());
	InitEditedMacroParamDataInfo(itsMacroParamDataGenerator->MakeDataInfo());
	fAutomationModeOn = itsMacroParamDataGenerator->AutomationModeOn() ? TRUE : FALSE;
	InitCpuUsagePercentage(itsMacroParamDataGenerator->DialogCpuUsagePercentage());
	InitControlsFromLoadedMacroParamDataInfo();
}

void CFmiMacroParamDataGeneratorDlg::InitEditedMacroParamDataInfo(const NFmiMacroParamDataInfo& macroParamInfoFromDocument)
{
	// 1. Jos mLoadedMacroParamDataInfoFullPath:ista saa ladattua jotain, k�ytet��n sit�
	if(!BetaProduct::LoadObjectFromKnownFileInJsonFormat(*itsEditedMacroParamDataInfo, mLoadedMacroParamDataInfoFullPath, "EditedMacroParamDataInfo", true))
	{
		// 2. Muuten otetaan parametrina annettu k�ytt��n
		*itsEditedMacroParamDataInfo = macroParamInfoFromDocument;
	}
}

void CFmiMacroParamDataGeneratorDlg::InitControlsFromLoadedMacroParamDataInfo()
{
	itsBaseDataParamProducerString = CA2T(itsEditedMacroParamDataInfo->BaseDataParamProducerString().c_str());
	itsProducerIdNamePairString = CA2T(itsEditedMacroParamDataInfo->UsedProducerString().c_str());
	itsUsedDataGenerationSmarttoolPathList = CA2T(itsEditedMacroParamDataInfo->DataGeneratingSmarttoolPathListString().c_str());
	itsUsedParameterListString = CA2T(itsEditedMacroParamDataInfo->UsedParameterListString().c_str());
	itsGeneratedDataStorageFileFilter = CA2T(itsEditedMacroParamDataInfo->DataStorageFileFilter().c_str());
	itsDataTriggerList = CA2T(itsEditedMacroParamDataInfo->DataTriggerList().c_str());
	InitMaxGeneratedFilesKept(itsEditedMacroParamDataInfo->MaxGeneratedFilesKept());
	mBaseDataGridScaleString = CA2T(itsEditedMacroParamDataInfo->BaseDataGridScaleString().c_str());
	UpdateData(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::InitMaxGeneratedFilesKept(int maxGeneratedFilesKept)
{
	try
	{
		itsMaxGeneratedFilesKept = CA2T(std::to_string(maxGeneratedFilesKept).c_str());
	}
	catch(...)
	{
		itsMaxGeneratedFilesKept = _T("2");
	}
}

int CFmiMacroParamDataGeneratorDlg::GetMaxGeneratedFilesKept()
{
	try
	{
		std::string tmpStr = CT2A(itsMaxGeneratedFilesKept);
		return NFmiMacroParamDataInfo::FixMaxGeneratedFilesKeptValue(std::stoi(tmpStr));
	}
	catch(...)
	{
		return NFmiMacroParamDataInfo::FixMaxGeneratedFilesKeptValue(2);
	}
}

void CFmiMacroParamDataGeneratorDlg::InitCpuUsagePercentage(double cpuUsagePercentage)
{
	try
	{
		itsCpuUsagePercentageString = CA2T(NFmiValueString::GetStringWithMaxDecimalsSmartWay(cpuUsagePercentage, 1));
	}
	catch(...)
	{
		itsCpuUsagePercentageString = CA2T(NFmiValueString::GetStringWithMaxDecimalsSmartWay(NFmiSmartToolModifier::DefaultUsedCpuCapacityPercentageInCalculations, 1));
	}
}

std::pair<bool, double> CFmiMacroParamDataGeneratorDlg::GetCpuUsagePercentage()
{
	try
	{
		std::string tmpStr = CT2A(itsCpuUsagePercentageString);
		return std::make_pair(true, std::stod(tmpStr));
	}
	catch(...)
	{
		return std::make_pair(false, NFmiSmartToolModifier::DefaultUsedCpuCapacityPercentageInCalculations);
	}
}

void CFmiMacroParamDataGeneratorDlg::StoreControlValuesToDocument()
{
	UpdateData(TRUE);

	itsMacroParamDataGenerator->DialogBaseDataParamProducerString(CFmiWin32Helpers::CT2std(itsBaseDataParamProducerString));
	itsMacroParamDataGenerator->DialogUsedProducerString(CFmiWin32Helpers::CT2std(itsProducerIdNamePairString));
	itsMacroParamDataGenerator->DialogDataGeneratingSmarttoolPathListString(CFmiWin32Helpers::CT2std(itsUsedDataGenerationSmarttoolPathList));
	itsMacroParamDataGenerator->DialogUsedParameterListString(CFmiWin32Helpers::CT2std(itsUsedParameterListString));
	itsMacroParamDataGenerator->DialogDataStorageFileFilter(CFmiWin32Helpers::CT2std(itsGeneratedDataStorageFileFilter));
	itsMacroParamDataGenerator->DialogDataTriggerList(CFmiWin32Helpers::CT2std(itsDataTriggerList));
	itsMacroParamDataGenerator->DialogMaxGeneratedFilesKept(GetMaxGeneratedFilesKept());
	itsMacroParamDataGenerator->DialogBaseDataGridScaleString(CFmiWin32Helpers::CT2std(mBaseDataGridScaleString));
	// Vain oikein t�ytetty double arvo voitaan tallettaa dialogimuistiin, Sen oikeellisuus 
	// varmistetaan ja sen arvo mahdollisesti korjataan viel� DialogCpuUsagePercentage metodissa.
	auto cpuUsageCheckResult = GetCpuUsagePercentage();
	if(cpuUsageCheckResult.first)
	{
		itsMacroParamDataGenerator->DialogCpuUsagePercentage(cpuUsageCheckResult.second);
	}
	// Tehd��n samalla t�m� dialogin macroParamDataInfo olion t�ytt�
	StoreControlValuesToEditedMacroParamDataInfo();
}

void CFmiMacroParamDataGeneratorDlg::StoreControlValuesToEditedMacroParamDataInfo()
{
	UpdateData(TRUE);

	itsEditedMacroParamDataInfo->BaseDataParamProducerString(CFmiWin32Helpers::CT2std(itsBaseDataParamProducerString));
	itsEditedMacroParamDataInfo->UsedProducerString(CFmiWin32Helpers::CT2std(itsProducerIdNamePairString));
	itsEditedMacroParamDataInfo->DataGeneratingSmarttoolPathListString(CFmiWin32Helpers::CT2std(itsUsedDataGenerationSmarttoolPathList));
	itsEditedMacroParamDataInfo->UsedParameterListString(CFmiWin32Helpers::CT2std(itsUsedParameterListString));
	itsEditedMacroParamDataInfo->DataStorageFileFilter(CFmiWin32Helpers::CT2std(itsGeneratedDataStorageFileFilter));
	itsEditedMacroParamDataInfo->DataTriggerList(CFmiWin32Helpers::CT2std(itsDataTriggerList));
	itsEditedMacroParamDataInfo->MaxGeneratedFilesKept(GetMaxGeneratedFilesKept());
	itsEditedMacroParamDataInfo->BaseDataGridScaleString(CFmiWin32Helpers::CT2std(mBaseDataGridScaleString));
}

// Tarkista kaikki sy�tteet ja jos niiss� on vikaa:
// 1. Lokita ongelmasta
// 2. Maalaa kontrolliin liittyv� static teksti kontrolli punaiseksi
// 3. Disabloi Generate nappi
void CFmiMacroParamDataGeneratorDlg::DoFullInputChecks()
{
	OnChangeEditBaseDataParamProducer();
	OnChangeEditProducerIdNamePair();
	OnChangeEditUsedParameterList();
	OnChangeEditUsedDataGenerationSmarttoolPathList();
	OnChangeEditGeneratedDataStorageFileFilter();
	OnEnChangeEditUsedDataTriggerList();
	OnEnChangeEditBaseDataGridScale();
}

void CFmiMacroParamDataGeneratorDlg::SetDefaultValues(void)
{
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	// gets current window position
	BOOL bRet = GetWindowPlacement(&wndpl);

	CRect oldRect(wndpl.rcNormalPosition);
	const CRect& defaultRect = CFmiVisualizationSettings::ViewPosRegistryInfo().DefaultWindowRect();
	// HUOM! dialogin kokoa ei saa muuttaa!!
	MoveWindow(defaultRect.left, defaultRect.top, oldRect.Width(), oldRect.Height());
	Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiMacroParamDataGeneratorDlg::Update()
{
	// T�t� dialogia ei tarvitse p�ivitt�� oikeasti, Update metodia tarvitaan vain koska tietyt template apufunktiot sit� vaativat.
}


void CFmiMacroParamDataGeneratorDlg::OnOK()
{
	DoWhenClosing();

	CDialogEx::OnOK();
}


void CFmiMacroParamDataGeneratorDlg::OnCancel()
{
	DoWhenClosing();

	CDialogEx::OnCancel();
}


void CFmiMacroParamDataGeneratorDlg::OnClose()
{
	DoWhenClosing();

	CDialogEx::OnClose();
}

void CFmiMacroParamDataGeneratorDlg::DoWhenClosing()
{
	StoreControlValuesToDocument();
	// aktivoidaan karttan�ytt� eli mainframe
	AfxGetMainWnd()->SetActiveWindow();
}

void CFmiMacroParamDataGeneratorDlg::StartDataGenerationControlEnablations()
{
	itsMacroParamDataGenerator->DataGenerationIsOn(true);
	EnableButtons();
}

void CFmiMacroParamDataGeneratorDlg::DoControlColoring(CDC* pDC, bool status)
{
	if(status)
		pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli v�ritet��n punaiseksi
	else
		pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se v�rj�t��n mustaksi
}

HBRUSH CFmiMacroParamDataGeneratorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_BASE_DATA_PARAM_PRODUCER_STR)
	{
		DoControlColoring(pDC, fBaseDataParamProducerStringHasInvalidValues);
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_PRODUCER_ID_NAME_PAIR_STR)
	{
		DoControlColoring(pDC, fProducerIdNamePairStringHasInvalidValues);
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_USED_PARAMETER_LIST_STR)
	{
		DoControlColoring(pDC, fUsedParameterListStringHasInvalidValues);
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_DATA_GENERATION_SMARTTOOL_PATH_LIST_STR)
	{
		DoControlColoring(pDC, fUsedDataGenerationSmarttoolPathListHasInvalidValues);
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_GENERATED_DATA_STORAGE_FILE_FILTER_STR)
	{
		DoControlColoring(pDC, fGeneratedDataStorageFileFilterHasInvalidValues);
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_USED_DATA_TRIGGER_LIST)
	{
		DoControlColoring(pDC, fDataTriggerListHasInvalidValues);
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_BASE_DATA_GRID_SCALE_STR)
	{
		DoControlColoring(pDC, fBaseDataGridScaleHasInvalidValues);
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_GENERATED_DATA_INFO_STR)
	{
		DoControlColoring(pDC, fDataGenerationStoppedOrFailed);
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_CPU_USAGE_PERCENTAGE_STR)
	{
		DoControlColoring(pDC, fCpuUsagePercentageHasInvalidValues);
	}
	// CButton kontrollin v�ri s��d�t ei toimi
	//else if(pWnd->GetDlgCtrlID() == IDC_BUTTON_CANCEL_DATA_GENERATION)
	//{
	//	// Vain v�liaikaisesti n�kyv�n Cancel nappulan tekstit v�ritet��n aina punaiseksi
	//	pDC->SetTextColor(RGB(255, 0, 0));

	//	pDC->SetBkColor(RGB(255, 0, 0)); // Blue background

	//	// If brush is not created, create a new one
	//	if(!itsCancelButtonBackgroundBrush.GetSafeHandle())
	//	{
	//		itsCancelButtonBackgroundBrush.CreateSolidBrush(RGB(0, 100, 255)); // Blue background brush
	//	}

	//	return itsCancelButtonBackgroundBrush; // Return the brush
	//}

	return hbr;
}

void CFmiMacroParamDataGeneratorDlg::DoControlColoringUpdates(int controlId)
{
	CWnd* win = GetDlgItem(controlId);
	if(win)
		win->Invalidate(FALSE);
	EnableButtons();
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditBaseDataParamProducer()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsBaseDataParamProducerString);
	auto checkResult = NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(tmp, true);
	fBaseDataParamProducerStringHasInvalidValues = !checkResult.first.empty();
	if(fBaseDataParamProducerStringHasInvalidValues)
	{
		DoErrorLogging("BaseData", checkResult.first, CatLog::Severity::Error);
	}

	// Edit kentt��n liittyv� otsikkokontrolli v�rj�t��n punaiseksi, jos inputissa on vikaa
	DoControlColoringUpdates(IDC_STATIC_BASE_DATA_PARAM_PRODUCER_STR);
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditProducerIdNamePair()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsProducerIdNamePairString);
	auto checkResult = NFmiMacroParamDataInfo::CheckUsedProducerString(tmp);
	fProducerIdNamePairStringHasInvalidValues = !checkResult.first.empty();
	if(fProducerIdNamePairStringHasInvalidValues)
	{
		DoErrorLogging("Producer", checkResult.first, CatLog::Severity::Error);
	}

	// Edit kentt��n liittyv� otsikkokontrolli v�rj�t��n punaiseksi, jos inputissa on vikaa
	DoControlColoringUpdates(IDC_STATIC_PRODUCER_ID_NAME_PAIR_STR);
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditUsedParameterList()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsUsedParameterListString);
	NFmiProducer dummyProducer;
	auto checkResult = NFmiMacroParamDataInfo::CheckUsedParameterListString(tmp, dummyProducer);
	fUsedParameterListStringHasInvalidValues = !checkResult.first.empty();
	if(fUsedParameterListStringHasInvalidValues)
	{
		DoErrorLogging("ParameterList", checkResult.first, CatLog::Severity::Error);
	}

	// Edit kentt��n liittyv� otsikkokontrolli v�rj�t��n punaiseksi, jos inputissa on vikaa
	DoControlColoringUpdates(IDC_STATIC_USED_PARAMETER_LIST_STR);
}

const std::string g_loggerName = "MacroParamDataGeneratorDlg";
void CFmiMacroParamDataGeneratorDlg::DoErrorLogging(std::string errorItem, std::string errorMessage, CatLog::Severity logLevel)
{
	CatLog::logMessage(g_loggerName + " " + errorItem + ": " + errorMessage, logLevel, CatLog::Category::Operational);
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditGeneratedDataStorageFileFilter()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsGeneratedDataStorageFileFilter);
	auto checkResult = NFmiMacroParamDataInfo::CheckDataStorageFileFilter(tmp);
	fGeneratedDataStorageFileFilterHasInvalidValues = !checkResult.empty();
	if(fGeneratedDataStorageFileFilterHasInvalidValues)
	{
		DoErrorLogging("Data file filter", checkResult, CatLog::Severity::Error);
	}

	// Edit kentt��n liittyv� otsikkokontrolli v�rj�t��n punaiseksi, jos inputissa on vikaa
	DoControlColoringUpdates(IDC_STATIC_GENERATED_DATA_STORAGE_FILE_FILTER_STR);
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonBrowseStoredDataFileFilter()
{
	auto initialSavePath = itsMacroParamDataGenerator->GeneratedDataStorageInitialPath();
	std::string selectedAbsoluteFilePath;
	if(BetaProduct::GetFilePathFromUser(NFmiMacroParamDataGenerator::GeneratedDataFileFilter(), initialSavePath, selectedAbsoluteFilePath, true, "", this))
	{
		initialSavePath = PathUtils::getPathSectionFromTotalFilePath(selectedAbsoluteFilePath);
		itsMacroParamDataGenerator->GeneratedDataStorageInitialPath(initialSavePath);
		itsGeneratedDataStorageFileFilter = CA2T(selectedAbsoluteFilePath.c_str());
		UpdateData(FALSE);
		OnChangeEditGeneratedDataStorageFileFilter();
	}
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditUsedDataGenerationSmarttoolPathList()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsUsedDataGenerationSmarttoolPathList);
	auto checkResult = NFmiMacroParamDataInfo::CheckDataGeneratingSmarttoolPathListString(tmp);
	fUsedDataGenerationSmarttoolPathListHasInvalidValues = !checkResult.empty();
	if(fUsedDataGenerationSmarttoolPathListHasInvalidValues)
	{
		DoErrorLogging("Smarttool paths", checkResult, CatLog::Severity::Error);
	}

	// Edit kentt��n liittyv� otsikkokontrolli v�rj�t��n punaiseksi, jos inputissa on vikaa
	DoControlColoringUpdates(IDC_STATIC_DATA_GENERATION_SMARTTOOL_PATH_LIST_STR);
}

void CFmiMacroParamDataGeneratorDlg::OnEnChangeEditUsedDataTriggerList()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsDataTriggerList);
	auto checkResult = NFmiMacroParamDataInfo::CheckDataTriggerListString(tmp);
	fDataTriggerListHasInvalidValues = !checkResult.first.empty();
	if(fDataTriggerListHasInvalidValues)
	{
		DoErrorLogging("Data triggers", checkResult.first, CatLog::Severity::Error);
	}

	// Edit kentt��n liittyv� otsikkokontrolli v�rj�t��n punaiseksi, jos inputissa on vikaa
	DoControlColoringUpdates(IDC_STATIC_USED_DATA_TRIGGER_LIST);
}

void CFmiMacroParamDataGeneratorDlg::OnEnChangeEditMaxGeneratedFilesKept()
{
	UpdateData(TRUE);
	InitMaxGeneratedFilesKept(GetMaxGeneratedFilesKept());
	UpdateData(FALSE);
}


void CFmiMacroParamDataGeneratorDlg::OnEnChangeEditCpuUsagePercentage()
{
	UpdateData(TRUE);
	auto cpuUsageCheckResult = GetCpuUsagePercentage();
	auto checkedCpuUsageValue = NFmiSmartToolModifier::FixCpuCapacityPercentageInCalculations(cpuUsageCheckResult.second);
	fCpuUsagePercentageHasInvalidValues = !(cpuUsageCheckResult.first && checkedCpuUsageValue == cpuUsageCheckResult.second);
	if(fCpuUsagePercentageHasInvalidValues)
	{
		DoErrorLogging("CPU usage", "usage percentage has invalid values", CatLog::Severity::Error);
	}

	// Edit kentt��n liittyv� otsikkokontrolli v�rj�t��n punaiseksi, jos inputissa on vikaa
	DoControlColoringUpdates(IDC_STATIC_CPU_USAGE_PERCENTAGE_STR);
}

void CFmiMacroParamDataGeneratorDlg::OnEnChangeEditBaseDataGridScale()
{
	UpdateData(TRUE);
	std::string tmp = CT2A(mBaseDataGridScaleString);
	auto checkResult = NFmiMacroParamDataInfo::CheckBaseDataGridScaleString(tmp);
	fBaseDataGridScaleHasInvalidValues = !checkResult.first.empty();

	// Edit kentt��n liittyv� otsikkokontrolli v�rj�t��n punaiseksi, jos inputissa on vikaa
	DoControlColoringUpdates(IDC_STATIC_BASE_DATA_GRID_SCALE_STR);
}

void CFmiMacroParamDataGeneratorDlg::ResetProgressControl()
{
	mProgressControl.SetPos(0);
}

void CFmiMacroParamDataGeneratorDlg::StepIt(void)
{
	mProgressControl.StepIt();
}

void CFmiMacroParamDataGeneratorDlg::SetRange(int low, int high, int stepCount)
{
	mProgressControl.SetRange(low, high);
	mProgressControl.SetStep(stepCount);
}

void CFmiMacroParamDataGeneratorDlg::AddRange(int value)
{
	int low = 0;
	int high = 0;
	mProgressControl.GetRange(low, high);
	high += value;
	mProgressControl.SetRange(low, high);
}

static bool WaitForWindowToInitialize(CWnd* checkedWindow)
{
	// Progress dialogi k�ynnistet��n p��-threadista, mutta sen kanssa operoidaan
	// ty�-threadista k�sin. T�m�n funktion tarkoitus on varmistaa ett� tietyt
	// ikkunat ja kontrollit on alustettu, ennen kuin niit� oikeasti k�ytet��n.
	// T�m� varmistus tehd��n loopissa jossa testataan onko GetSafeHwnd -metodin palauttama arvo ok. 
	// Jos ei, odota pikkuisen ja yrit� uudestaan...
	// Jos ikkuna on lopuksi alustettu palauttaa true, mutta jos tarpeeksi monen yrityksen j�lkeen homma 
	// ei ole valmis, palautetaan false, jotta ei j��d� iki-looppiin.
	if(checkedWindow)
	{
		for(int i = 0; checkedWindow->GetSafeHwnd() == NULL; i++)
		{
			if(i > 200)
				return false; // t�ll� estet��n iki-looppi
			Sleep(10);
		}
		return true;
	}
	return false;
}

bool CFmiMacroParamDataGeneratorDlg::DoPostMessage(unsigned int message, unsigned int wParam, long lParam)
{
	if(::WaitForWindowToInitialize(this))
	{
		return PostMessage(static_cast<UINT>(message), static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam)) == TRUE;
	}

	return false;
}

bool CFmiMacroParamDataGeneratorDlg::WaitUntilInitialized(void)
{
	// Pit�� varmistaa ett� dialogi on alustettu
	if(::WaitForWindowToInitialize(this))
	{
		// Lis�ksi pit�� varmistaa ett� itsProgressCtrl on my�s alustettu!
		if(::WaitForWindowToInitialize(&mProgressControl))
		{
			return true;
		}
	}

	return false;
}

void CFmiMacroParamDataGeneratorDlg::DoOnStopDataGeneration(const std::string& stopMethodName, bool workFinished, bool operationStatus)
{
	UpdateGeneratedDataInfoStr(stopMethodName, workFinished, operationStatus);
	EnableButtons();
	ShowCancelButton(false);
	mStopper.Stop(false);
	ResetProgressControl();
//	UpdateData(FALSE);
}

BOOL CFmiMacroParamDataGeneratorDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if(message == ID_MACRO_PARAM_DATA_GENERATION_FINISHED)
	{
		DoOnStopDataGeneration("Finished", true, true);
	}
	else if(message == ID_MACRO_PARAM_DATA_GENERATION_CANCELED)
	{
		DoOnStopDataGeneration("Canceled", false, false);
	}
	else if(message == ID_MACRO_PARAM_DATA_GENERATION_FAILED)
	{
		DoOnStopDataGeneration("Failed", false, false);
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

static std::string GetUsedSaveFileName(const std::string &loadedMacroParamDataInfoFullPath, bool listCase)
{
	auto fileName = PathUtils::getFilename(loadedMacroParamDataInfoFullPath, true);
	if(fileName.empty())
	{
		if(listCase)
			fileName = "MacroParDataList1." + NFmiMacroParamDataGenerator::MacroParamDataListFileExtension();
		else
			fileName = "MacroParDataInfo1." + NFmiMacroParamDataGenerator::MacroParamDataInfoFileExtension();
	}
	return fileName;
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonSaveMacroParamData()
{
	// Ennen tallennusta talletetaan varmuuden vuoksi s��d�t my�s dokumenttiin
	StoreControlValuesToDocument(); 

	auto macroParamsDataInfo = itsMacroParamDataGenerator->MakeDataInfo();
	auto initialSavePath = itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath();
	std::string fullPath;
	auto initialFileName = ::GetUsedSaveFileName(mLoadedMacroParamDataInfoFullPath, false);
	std::string rootPath = itsMacroParamDataGenerator->RootMacroParamDataDirectory();
	if(BetaProduct::SaveObjectInJsonFormat(macroParamsDataInfo, initialSavePath, NFmiMacroParamDataGenerator::MacroParamDataInfoFileFilter(), NFmiMacroParamDataGenerator::MacroParamDataInfoFileExtension(), rootPath, "MacroParam data info", initialFileName, false, &fullPath, this))
	{
		itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath(initialSavePath);
		UpdateMacroParamDataInfoName(fullPath);
		EnableButtons();
		UpdateAutomationList();
	}
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonLoadMacroParamData()
{
	auto initialSavePath = itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath();
	std::string fullPath;
	std::string rootMacroParDataInfoPath;
	if(BetaProduct::LoadObjectInJsonFormat(*itsEditedMacroParamDataInfo, initialSavePath, NFmiMacroParamDataGenerator::MacroParamDataInfoFileFilter(), NFmiMacroParamDataGenerator::MacroParamDataInfoFileExtension(), rootMacroParDataInfoPath, "MacroParam data info", false, &fullPath, this))
	{
		itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath(initialSavePath);
		UpdateMacroParamDataInfoName(fullPath);
		InitControlsFromLoadedMacroParamDataInfo();
		DoFullInputChecks();
	}
}

void CFmiMacroParamDataGeneratorDlg::UpdateMacroParamDataInfoName(const std::string& fullPath)
{
	mLoadedMacroParamDataInfoFullPath = fullPath;
	itsMacroParamDataGenerator->AutomationPath(fullPath);
	// Laitetaan suhteellinen polku n�kyviin jos mahdollista
	auto tmpName = PathUtils::getRelativePathIfPossible(fullPath, itsMacroParamDataGenerator->RootMacroParamDataDirectory());
	mLoadedMacroParamDataInfoName = CA2T(tmpName.c_str());
	UpdateData(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::UpdateMacroParamDataAutomationListName(const std::string& fullPath)
{
	mLoadedMacroParamDataAutomationListFullPath = fullPath;
	itsMacroParamDataGenerator->AutomationListPath(fullPath);
	// Laitetaan suhteellinen polku n�kyviin jos mahdollista
	auto tmpName = PathUtils::getRelativePathIfPossible(fullPath, itsMacroParamDataGenerator->RootMacroParamDataDirectory());
	mLoadedMacroParamDataAutomationListName = CA2T(tmpName.c_str());
	UpdateData(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::EnableButtons()
{
	if(itsMacroParamDataGenerator->DataGenerationIsOn() || itsMacroParamDataGenerator->AutomationModeOn())
	{
		itsGenerateMacroParamDataButton.EnableWindow(false);
		itsSaveMacroParamDataInfoButton.EnableWindow(false);
		itsSaveMacroParamDataAutomationListButton.EnableWindow(false);
		itsRunSelectedMacroParamDataAutomationButton.EnableWindow(false);
		itsRunEnabledMacroParamDataAutomationButton.EnableWindow(false);
		itsRunAllMacroParamDataAutomationButton.EnableWindow(false);

		itsAddEditedMacroParamDataAutomationToListButton.EnableWindow(false);
		itsAddFromFileMacroParamDataAutomationToListButton.EnableWindow(false);
		itsRemoveMacroParamDataAutomationFromListButton.EnableWindow(false);
		itsLoadMacroParamDataAutomationListButton.EnableWindow(false);
		itsLoadMacroParamDataInfoButton.EnableWindow(false);

		return;
	}

	auto macroParamDataInfoOk = !fBaseDataParamProducerStringHasInvalidValues && !fProducerIdNamePairStringHasInvalidValues && !fUsedDataGenerationSmarttoolPathListHasInvalidValues && !fUsedParameterListStringHasInvalidValues && !fGeneratedDataStorageFileFilterHasInvalidValues && !fDataTriggerListHasInvalidValues && !fBaseDataGridScaleHasInvalidValues;
	itsGenerateMacroParamDataButton.EnableWindow(macroParamDataInfoOk);
	itsSaveMacroParamDataInfoButton.EnableWindow(macroParamDataInfoOk);

	auto macroParamDataAutomationListOk = itsMacroParamDataGenerator->UsedMacroParamDataAutomationList().DoFullChecks(itsMacroParamDataGenerator->AutomationModeOn());
	if(macroParamDataAutomationListOk != MacroParamDataStatus::kFmiListItemOk)
	{
		itsSaveMacroParamDataAutomationListButton.EnableWindow(false);
		itsRunSelectedMacroParamDataAutomationButton.EnableWindow(false);
		itsRunEnabledMacroParamDataAutomationButton.EnableWindow(false);
		itsRunAllMacroParamDataAutomationButton.EnableWindow(false);
	}
	else
	{
		itsSaveMacroParamDataAutomationListButton.EnableWindow(true);
		itsRunSelectedMacroParamDataAutomationButton.EnableWindow(true);
		itsRunEnabledMacroParamDataAutomationButton.EnableWindow(true);
		itsRunAllMacroParamDataAutomationButton.EnableWindow(true);
	}

	itsAddEditedMacroParamDataAutomationToListButton.EnableWindow(true);
	itsAddFromFileMacroParamDataAutomationToListButton.EnableWindow(true);
	itsRemoveMacroParamDataAutomationFromListButton.EnableWindow(true);
	itsLoadMacroParamDataAutomationListButton.EnableWindow(true);
	itsLoadMacroParamDataInfoButton.EnableWindow(true);
}

void CFmiMacroParamDataGeneratorDlg::DoResizerHooking(void)
{
	BOOL bOk = m_resizer.Hook(this);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_GROUP_MACRO_PARAM_DATA_INFO, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_PARAM_INFO_NAME_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_BASE_DATA_PARAM_PRODUCER_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_BASE_DATA_PARAM_PRODUCER, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_PRODUCER_ID_NAME_PAIR_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_PRODUCER_ID_NAME_PAIR, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_DATA_GENERATION_SMARTTOOL_PATH_LIST_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_USED_DATA_GENERATION_SMARTTOOL_PATH_LIST, ANCHOR_HORIZONTALLY | ANCHOR_TOP);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_ADD_USED_SMARTTOOL_PATH, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_USED_PARAMETER_LIST_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_USED_PARAMETER_LIST, ANCHOR_HORIZONTALLY | ANCHOR_TOP);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_USED_DATA_TRIGGER_LIST, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_USED_DATA_TRIGGER_LIST, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_GENERATED_DATA_STORAGE_FILE_FILTER_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_GENERATED_DATA_STORAGE_FILE_FILTER, ANCHOR_HORIZONTALLY | ANCHOR_TOP);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_BROWSE_STORED_DATA_FILE_FILTER, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_MAX_GENERATED_FILES_KEPT_STR, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_MAX_GENERATED_FILES_KEPT, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_GENERATE_MACRO_PARAM_DATA, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_PROGRESS_OF_OPERATION_BAR, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_SAVE_MACRO_PARAM_DATA, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_LOAD_MACRO_PARAM_DATA, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_LOADED_MACRO_PARAM_DATA_INFO_NAME, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_GROUP_DATA_GENERATION_LIST, ANCHOR_VERTICALLY | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_PARAM_DATA_AUTOMATION_LIST_NAME_TEXT, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_MACRO_PARAM_DATA_AUTOMATION_LIST_NAME_VALUE, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_CUSTOM_GRID_MACRO_PARAM_DATA_AUTOMATION_LIST, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_ADD_EDITED_MACRO_PARAM_DATA_AUTOMATION_TO_LIST, ANCHOR_BOTTOM | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_ADD_MACRO_PARAM_DATA_AUTOMATION_TO_LIST, ANCHOR_BOTTOM | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_REMOVE_MACRO_PARAM_DATA_AUTOMATION_FROM_LIST, ANCHOR_BOTTOM | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_SAVE_MACRO_PARAM_DATA_AUTOMATION_LIST, ANCHOR_BOTTOM | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_LOAD_MACRO_PARAM_DATA_AUTOMATION_LIST, ANCHOR_BOTTOM | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_CHECK_MACRO_PARAM_DATA_AUTOMATION_MODE_ON, ANCHOR_BOTTOM | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_RUN_SELECTED_MACRO_PARAM_DATA_AUTOMATION, ANCHOR_BOTTOM | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_RUN_ENABLED_MACRO_PARAM_DATA_AUTOMATIONS, ANCHOR_BOTTOM | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_RUN_ALL_MACRO_PARAM_DATA_AUTOMATIONS, ANCHOR_BOTTOM | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_GENERATED_DATA_INFO_STR, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_CANCEL_DATA_GENERATION, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_BASE_DATA_GRID_SCALE, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_BASE_DATA_GRID_SCALE_STR, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_CPU_USAGE_PERCENTAGE_STR, ANCHOR_BOTTOM | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_CPU_USAGE_PERCENTAGE, ANCHOR_BOTTOM | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
}


void CFmiMacroParamDataGeneratorDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// set the minimum tracking width and height of the window
	lpMMI->ptMinTrackSize.x = 773;
	lpMMI->ptMinTrackSize.y = 699;

	__super::OnGetMinMaxInfo(lpMMI);
}


void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonAddEditedMacroParamDataAutomationToList()
{
	AddAutomationToList(mLoadedMacroParamDataInfoFullPath);
}

void CFmiMacroParamDataGeneratorDlg::AddAutomationToList(const std::string& theFullFilePath)
{
	if(itsMacroParamDataGenerator->UsedMacroParamDataAutomationList().Add(theFullFilePath))
	{
		UpdateAutomationList();
		EnableButtons();
	}
}

void CFmiMacroParamDataGeneratorDlg::UpdateAutomationList()
{
	auto& dataVector = itsMacroParamDataGenerator->UsedMacroParamDataAutomationList().AutomationVector();
	itsGridCtrl.SetRowCount(static_cast<int>(dataVector.size() + itsGridCtrl.GetFixedRowCount()));
	int currentRowCount = itsGridCtrl.GetFixedRowCount();
	for(size_t i = 0; i < dataVector.size(); i++)
	{
		SetGridRow(currentRowCount++, *dataVector[i]);
	}
	ForceGridCtrlUpdate();
}

void CFmiMacroParamDataGeneratorDlg::ForceGridCtrlUpdate()
{
	itsGridCtrl.UpdateData(FALSE);
	itsGridCtrl.Invalidate(FALSE);
}

// Halutaan palauttaa HH:mm eli hours:minutes teksti annetulle ajalle.
// Jos aika oli puuttuvaa, palautetaan --:--.
// Viimeiselle Beta-automaation ajoaika halutaan kuitenkin merkit� S-kirjaimella (= started),
// jos tuotetta ei ole ohjelman ajon aikan tehty viel�.
static std::string GetTimeTextHHmm(const NFmiMetTime& theTime, bool fJustStarted = false)
{
	if(theTime == NFmiMetTime::gMissingTime)
		return "--:--";

	NFmiMetTime aTime(1);
	bool nextDay = aTime.GetDay() < theTime.GetDay();

	std::string str;
	if(fJustStarted)
		str = "S ";
	else if(nextDay)
		str = "N ";
	str += theTime.ToStr("HH:mm", kEnglish);
	return str;
}

static std::string GetColumnText(int theRow, int theColumn, const NFmiMacroParamDataAutomationListItem& theListItem)
{
	switch(theColumn)
	{
	case MacroParDataAutomationHeaderParInfo::kRowNumber:
		return NFmiStringTools::Convert(theRow);
	case MacroParDataAutomationHeaderParInfo::kAutomationName:
		return theListItem.AutomationName();
	case MacroParDataAutomationHeaderParInfo::kNextRuntime:
		return ::GetTimeTextHHmm(NFmiMetTime::gMissingTime);
	case MacroParDataAutomationHeaderParInfo::kLastRuntime:
		return ::GetTimeTextHHmm(theListItem.itsLastRunTime, !theListItem.fProductsHaveBeenGenerated);
	case MacroParDataAutomationHeaderParInfo::kAutomationStatus:
		return theListItem.ShortStatusText();
	case MacroParDataAutomationHeaderParInfo::kAutomationPath:
		return theListItem.FullAutomationPath();
	default:
		return "";
	}
}

void CFmiMacroParamDataGeneratorDlg::SetGridRow(int row, const NFmiMacroParamDataAutomationListItem& theListItem)
{
	static const COLORREF gEnabledBkColor = RGB(255, 255, 255);
	static const COLORREF gDisabledBkColor = RGB(222, 222, 222);
	static const COLORREF gErrorBkColor = RGB(239, 135, 122);

	auto listItemErrorStatus = theListItem.GetErrorStatus();
	bool itemEnabled = theListItem.fEnable;
	for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
	{
		itsGridCtrl.SetItemText(row, column, CA2T(::GetColumnText(row, column, theListItem).c_str()));
		if(column >= itsGridCtrl.GetFixedColumnCount())
		{
			if(column != MacroParDataAutomationHeaderParInfo::kEnable) // kaikki muut ovat read-only paitsi enable -checkbox
				itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY); // Laita read-only -bitti p��lle

			if(itemEnabled)
				itsGridCtrl.SetItemBkColour(row, column, gEnabledBkColor);
			else
				itsGridCtrl.SetItemBkColour(row, column, gDisabledBkColor);

			if(listItemErrorStatus != MacroParamDataStatus::kFmiListItemOk)
				itsGridCtrl.SetItemBkColour(row, column, gErrorBkColor);

			if(column == MacroParDataAutomationHeaderParInfo::kEnable)
			{
				itsGridCtrl.SetCellType(row, column, RUNTIME_CLASS(CGridCellCheck));
				CGridCellCheck* pTempCell = (CGridCellCheck*)itsGridCtrl.GetCell(row, column);
				pTempCell->SetCheck(itemEnabled);
				pTempCell->SetCheckBoxClickedCallback(std::bind(&CFmiMacroParamDataGeneratorDlg::HandleEnableAutomationCheckBoxClick, this, std::placeholders::_1, std::placeholders::_2));
			}
		}
	}
}

void CFmiMacroParamDataGeneratorDlg::HandleEnableAutomationCheckBoxClick(int col, int row)
{
	UpdateData(TRUE);
	CGridCellCheck* pCell = dynamic_cast<CGridCellCheck*>(itsGridCtrl.GetCell(row, col));
	if(pCell)
	{
		bool newState = pCell->GetCheck() == TRUE;
		int dataIndex = row - 1; // rivit alkavat 1:st�, mutta datat on vektorissa 0:sta alkaen
		auto& dataVector = itsMacroParamDataGenerator->UsedMacroParamDataAutomationList().AutomationVector();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{
			auto& listItem = *dataVector[dataIndex];
			listItem.fEnable = newState;
			SetGridRow(row, listItem);
			itsGridCtrl.UpdateData(FALSE);
		}
	}
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonAddMacroParamDataAutomationToList()
{
	auto initialSavePath = itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath();
	std::string selectedAbsoluteFilePath;
	if(BetaProduct::GetFilePathFromUser(NFmiMacroParamDataGenerator::MacroParamDataInfoFileFilter(), initialSavePath, selectedAbsoluteFilePath, true, "", this))
	{
		itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath(initialSavePath);
		AddAutomationToList(selectedAbsoluteFilePath);
	}
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonRemoveMacroParamDataAutomationFromList()
{
	CCellRange selectedRange(itsGridCtrl.GetSelectedCellRange());
	for(int row = selectedRange.GetMinRow(); row <= selectedRange.GetMaxRow(); row++)
	{
		if(row >= itsGridCtrl.GetFixedRowCount() && row < itsGridCtrl.GetRowCount())
		{
			itsMacroParamDataGenerator->UsedMacroParamDataAutomationList().Remove(row - itsGridCtrl.GetFixedRowCount());
			itsMacroParamDataGenerator->UsedMacroParamDataAutomationList().DoFullChecks(itsMacroParamDataGenerator->AutomationModeOn());
			UpdateAutomationList();
			EnableButtons();
		}
	}
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedCheckMacroParamDataAutomationModeOn()
{
	UpdateData(TRUE);
	itsMacroParamDataGenerator->AutomationModeOn(fAutomationModeOn == TRUE);
	itsMacroParamDataGenerator->UsedMacroParamDataAutomationList().DoFullChecks(itsMacroParamDataGenerator->AutomationModeOn());
	UpdateAutomationList();
	itsSmartMetDocumentInterface->SetAllViewIconsDynamically();
	EnableButtons();
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonSaveMacroParamDataAutomationList()
{
	// Ennen tallennusta talletetaan varmuuden vuoksi s��d�t my�s dokumenttiin
	StoreControlValuesToDocument();

	auto initialSavePath = itsMacroParamDataGenerator->MacroParamDataAutomationListSaveInitialPath();
	std::string usedAbsoluteFilePath;
	auto initialFileName = ::GetUsedSaveFileName(mLoadedMacroParamDataAutomationListFullPath, true);
	std::string rootPath = itsMacroParamDataGenerator->RootMacroParamDataDirectory();
	if(BetaProduct::SaveObjectInJsonFormat(itsMacroParamDataGenerator->UsedMacroParamDataAutomationList(), initialSavePath, NFmiMacroParamDataGenerator::MacroParamDataListFileFilter(), NFmiMacroParamDataGenerator::MacroParamDataListFileExtension(), rootPath, "MacroPar data automation-list", initialFileName, false, &usedAbsoluteFilePath, this))
	{
		itsMacroParamDataGenerator->MacroParamDataAutomationListSaveInitialPath(initialSavePath);
		UpdateMacroParamDataAutomationListName(usedAbsoluteFilePath);
		EnableButtons();
		UpdateData(FALSE);
	}
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonLoadMacroParamDataAutomationList()
{
	auto initialSavePath = itsMacroParamDataGenerator->MacroParamDataAutomationListSaveInitialPath();
	std::string usedAbsoluteFilePath;
	std::string rootPath = itsMacroParamDataGenerator->RootMacroParamDataDirectory();
	if(BetaProduct::LoadObjectInJsonFormat(itsMacroParamDataGenerator->UsedMacroParamDataAutomationList(), initialSavePath, NFmiMacroParamDataGenerator::MacroParamDataListFileFilter(), NFmiMacroParamDataGenerator::MacroParamDataListFileExtension(), rootPath, "MacroPar data automation-list", false, &usedAbsoluteFilePath, this))
	{
		itsMacroParamDataGenerator->MacroParamDataAutomationListSaveInitialPath(initialSavePath);
		// Tehd��n t�ydet tarkastelut viel� kun tiedet��n miss� moodissa ollaan
		itsMacroParamDataGenerator->UsedMacroParamDataAutomationList().DoFullChecks(itsMacroParamDataGenerator->AutomationModeOn());
		UpdateMacroParamDataAutomationListName(usedAbsoluteFilePath);
		UpdateAutomationList();
		EnableButtons();
		UpdateData(FALSE);
	}
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonGenerateMacroParamData()
{
	DoUserStartedDataGenerationPreparations("Started edited");
	std::thread t(&CFmiMacroParamDataGeneratorDlg::LaunchMacroParamDataGeneration, this);
	t.detach();
}

void CFmiMacroParamDataGeneratorDlg::UpdateGeneratedDataInfoStr(const std::string& status, bool workFinished, bool operationStatus)
{
	NFmiTime currentTime;
	auto infoStr = status;
	if(workFinished)
	{
		infoStr += " (" + itsMacroParamDataGenerator->LastGeneratedDataMakeTime() + ")";
	}
	infoStr += " at ";
	infoStr += currentTime.ToStr("HH:mm:SS");
	mGeneratedDataInfoStr = CA2T(infoStr.c_str());
	fDataGenerationStoppedOrFailed = !operationStatus;
	// T�m� tekstikentt� v�rj�t��n punaiseksi, jos datan generointi keskeytynyt
	DoControlColoringUpdates(IDC_STATIC_GENERATED_DATA_INFO_STR);
	UpdateData(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::LaunchMacroParamDataGeneration()
{
	itsMacroParamDataGenerator->GenerateMacroParamData(mThreadCallBacksPtr.get());
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonRunSelectedMacroParamDataAutomation()
{
	UpdateData(TRUE);
	auto selectedCellRange = itsGridCtrl.GetSelectedCellRange();
	if(selectedCellRange.IsValid())
	{
		DoUserStartedDataGenerationPreparations("Started selected");
		std::thread t(&CFmiMacroParamDataGeneratorDlg::LaunchOnDemandMacroParamDataAutomation, this, selectedCellRange.GetMinRow(), true);
		t.detach();
	}
	else
		CatLog::logMessage("Nothing was selected for on-demand Beta-automation run", CatLog::Severity::Warning, CatLog::Category::Operational, true);
	UpdateAutomationList();
}

void CFmiMacroParamDataGeneratorDlg::LaunchOnDemandMacroParamDataAutomation(int selectedAutomationIndex, bool doOnlyEnabled)
{
	itsMacroParamDataGenerator->DoOnDemandBetaAutomations(selectedAutomationIndex, doOnlyEnabled, mThreadCallBacksPtr.get());
}


void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonRunEnabledMacroParamDataAutomations()
{
	DoUserStartedDataGenerationPreparations("Started enabled");
	std::thread t(&CFmiMacroParamDataGeneratorDlg::LaunchOnDemandMacroParamDataAutomation, this, -1, true);
	t.detach();
	UpdateAutomationList();
}


void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonRunAllMacroParamDataAutomations()
{
	DoUserStartedDataGenerationPreparations("Started all");
	std::thread t(&CFmiMacroParamDataGeneratorDlg::LaunchOnDemandMacroParamDataAutomation, this, -1, false);
	t.detach();
	UpdateAutomationList();
}


void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonCancelDataGeneration()
{
	mStopper.Stop(true);
}

// Ainakun Cancel button n�ytet��n, silloin pit�� Generate button piilottaa ja 
// toisin p�in, koska ne ovat p��llekk�in dialogissa.
void CFmiMacroParamDataGeneratorDlg::ShowCancelButton(bool show)
{
	itsCancelDataGenerationButton.ShowWindow(show ? SW_SHOW : SW_HIDE);
	itsGenerateMacroParamDataButton.ShowWindow(show ? SW_HIDE : SW_SHOW);
}

void CFmiMacroParamDataGeneratorDlg::DoUserStartedDataGenerationPreparations(const std::string &infoStr)
{
	UpdateData(TRUE);
	StartDataGenerationControlEnablations();
	StoreControlValuesToDocument();
	UpdateGeneratedDataInfoStr(infoStr, false, true);
	ShowCancelButton(true);
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonAddUsedSmarttoolPath()
{
	UpdateData(TRUE);
	std::string fileFilter = "Smarttool Files (*.st)|*.st|All Files (*.*)|*.*||";
	std::string initialDirectory = itsMacroParamDataGenerator->MacroParamDataAutomationAddSmarttoolInitialPath();
	std::string usedFilePath;
	std::string initialfileName;
	if(BetaProduct::GetFilePathFromUser(fileFilter, initialDirectory, usedFilePath, true, initialfileName, this))
	{
		gAddSmarttoolPathDoubleClickBugPreventerTimer.restart();
		itsMacroParamDataGenerator->MacroParamDataAutomationAddSmarttoolInitialPath(PathUtils::getPathSectionFromTotalFilePath(usedFilePath));
		AddSmarttoolToList(usedFilePath);
	}
}

void CFmiMacroParamDataGeneratorDlg::AddSmarttoolToList(const std::string& newFilePath)
{
	auto usedNewFilePath = PathUtils::getRelativePathIfPossible(newFilePath, itsMacroParamDataGenerator->RootSmarttoolDirectory());
	std::string tmpPathList = CT2A(itsUsedDataGenerationSmarttoolPathList);
	boost::trim(tmpPathList);
	if(tmpPathList.empty())
	{
		tmpPathList = usedNewFilePath;
	}
	else
	{
		tmpPathList += ", " + usedNewFilePath;
	}
	itsEditedMacroParamDataInfo->DataGeneratingSmarttoolPathListString(tmpPathList);
	itsEditedMacroParamDataInfo->CheckData();
	itsUsedDataGenerationSmarttoolPathList = CA2T(tmpPathList.c_str());
	UpdateData(FALSE);
	OnChangeEditUsedDataGenerationSmarttoolPathList();
	UpdateAutomationList();
}
