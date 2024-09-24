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
#include <thread>

const NFmiViewPosRegistryInfo CFmiMacroParamDataGeneratorDlg::s_ViewPosRegistryInfo(CRect(300, 200, 793, 739), "\\MacroParamDataGenerator");

// CFmiMacroParamDataGeneratorDlg dialog

IMPLEMENT_DYNAMIC(CFmiMacroParamDataGeneratorDlg, CDialogEx)

CFmiMacroParamDataGeneratorDlg::CFmiMacroParamDataGeneratorDlg(SmartMetDocumentInterface* smartMetDocumentInterface, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MACRO_PARAM_DATA_GENERATOR, pParent),
	itsSmartMetDocumentInterface(smartMetDocumentInterface),
	itsMacroParamDataGenerator(smartMetDocumentInterface ? &(smartMetDocumentInterface->GetMacroParamDataGenerator()) : nullptr)
	, itsBaseDataParamProducerString(_T(""))
	, itsProducerIdNamePairString(_T(""))
	, itsUsedDataGenerationSmarttoolPath(_T(""))
	, itsUsedParameterListString(_T(""))
	, itsGeneratedDataStorageFileFilter(_T(""))
	, mLoadedMacroParamDataInfoName(_T(""))
	, itsDataTriggerList(_T(""))
{
}

CFmiMacroParamDataGeneratorDlg::~CFmiMacroParamDataGeneratorDlg()
{
}

void CFmiMacroParamDataGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_BASE_DATA_PAR_PROD_LEV, itsBaseDataParamProducerString);
	DDX_Text(pDX, IDC_EDIT_PRODUCER_ID_NAME_PAIR, itsProducerIdNamePairString);
	DDX_Text(pDX, IDC_EDIT_USED_DATA_GENERATION_SMARTTOOL_PATH, itsUsedDataGenerationSmarttoolPath);
	DDX_Text(pDX, IDC_EDIT_USED_PARAMETER_LIST, itsUsedParameterListString);
	DDX_Text(pDX, IDC_EDIT_GENERATED_DATA_STORAGE_FILE_FILTER, itsGeneratedDataStorageFileFilter);
	DDX_Control(pDX, IDC_PROGRESS_OF_OPERATION_BAR, mProgressControl);
	DDX_Text(pDX, IDC_STATIC_LOADED_MACRO_PARAM_DATA_INFO_NAME, mLoadedMacroParamDataInfoName);
	DDX_Text(pDX, IDC_EDIT_USED_DATA_TRIGGER_LIST, itsDataTriggerList);
}


BEGIN_MESSAGE_MAP(CFmiMacroParamDataGeneratorDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_MACRO_PARAM_DATA, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonGenerateMacroParamData)
	ON_EN_CHANGE(IDC_EDIT_BASE_DATA_PARAM_PRODUCER, &CFmiMacroParamDataGeneratorDlg::OnChangeEditBaseDataParamProducer)
	ON_WM_CTLCOLOR()
	ON_EN_CHANGE(IDC_EDIT_PRODUCER_ID_NAME_PAIR, &CFmiMacroParamDataGeneratorDlg::OnChangeEditProducerIdNamePair)
	ON_EN_CHANGE(IDC_EDIT_USED_PARAMETER_LIST, &CFmiMacroParamDataGeneratorDlg::OnChangeEditUsedParameterList)
	ON_EN_CHANGE(IDC_EDIT_USED_DATA_GENERATION_SMARTTOOL_PATH, &CFmiMacroParamDataGeneratorDlg::OnChangeEditUsedDataGenerationSmarttoolPath)
	ON_EN_CHANGE(IDC_EDIT_GENERATED_DATA_STORAGE_FILE_FILTER, &CFmiMacroParamDataGeneratorDlg::OnChangeEditGeneratedDataStorageFileFilter)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_MACRO_PARAM_DATA, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonSaveMacroParamData)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_MACRO_PARAM_DATA, &CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonLoadMacroParamData)
	ON_EN_CHANGE(IDC_EDIT_USED_DATA_TRIGGER_LIST, &CFmiMacroParamDataGeneratorDlg::OnEnChangeEditUsedDataTriggerList)
END_MESSAGE_MAP()


// CFmiMacroParamDataGeneratorDlg message handlers


BOOL CFmiMacroParamDataGeneratorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	std::string errorBaseStr("Error in CFmiMacroParamDataGeneratorDlg::OnInitDialog while reading dialog size and position values");
	CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, true, errorBaseStr, 0);
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	InitControlsFromDocument();
	DoFullInputChecks();

	mProgressControl.SetRange(0, 100);
	mProgressControl.SetStep(1);
	mProgressControl.SetShowPercent(TRUE);
	mThreadCallBacksPtr = std::make_unique<NFmiThreadCallBacks>(&mStopper, this);
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiMacroParamDataGeneratorDlg::InitDialogTexts()
{
	SetWindowText(CA2T(::GetDictionaryString("MacroParam data generator").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_BASE_DATA_PARAM_PRODUCER_STR, "Base data's parameter+producer\nE.g. 'T_Ec' or 'par4_prod240' (no level!)");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PRODUCER_ID_NAME_PAIR_STR, "Generated data's producer id,name -pair\nE.g. 3001,ProducerName");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DATA_GENERATION_SMARTTOOL_PATH_STR, "Used data generation smarttool macro file path");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_BROWSE_USED_SMARTTOOL_PATH, "Browse smarttool");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_USED_PARAMETER_LIST_STR, "Used generated data parameter list (id,name -pairs) \nE.g. \"6201, Param1, 6202, Param2, ...\"");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GENERATED_DATA_STORAGE_FILE_FILTER_STR, "Generated data's storage file filter (* is replaced with time-stamp)\nE.g. C:\\data\\*_mydata.sqd");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_BROWSE_STORED_DATA_FILE_FILTER, "Browse stored path");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_GENERATE_MACRO_PARAM_DATA, "Generate MacroParam data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GROUP_MACRO_PARAM_DATA_INFO, "MacroParam data info section");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SAVE_MACRO_PARAM_DATA, "Save MacroPar data info");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOAD_MACRO_PARAM_DATA, "Load MacroPar data info");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GROUP_DATA_GENERATION_LIST, "Data generation list");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_USED_DATA_TRIGGER_LIST, "Data trigger list, e.g. T_ec[,T_gfs_500, ...]\nTrigger delayed: T_ec[0.5h] ->delays half an hour");
}

void CFmiMacroParamDataGeneratorDlg::InitControlsFromDocument()
{
	InitControlsFromLoadedMacroParamDataInfo(itsMacroParamDataGenerator->MakeDataInfo());
}

void CFmiMacroParamDataGeneratorDlg::InitControlsFromLoadedMacroParamDataInfo(const NFmiMacroParamDataInfo& macroParamsDataInfo)
{
	itsBaseDataParamProducerString = CA2T(macroParamsDataInfo.BaseDataParamProducerString().c_str());
	itsProducerIdNamePairString = CA2T(macroParamsDataInfo.UsedProducerString().c_str());
	itsUsedDataGenerationSmarttoolPath = CA2T(macroParamsDataInfo.DataGeneratingSmarttoolPathString().c_str());
	itsUsedParameterListString = CA2T(macroParamsDataInfo.UsedParameterListString().c_str());
	itsGeneratedDataStorageFileFilter = CA2T(macroParamsDataInfo.DataStorageFileFilter().c_str());
	itsDataTriggerList = CA2T(macroParamsDataInfo.DataTriggerList().c_str());
	UpdateData(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::StoreControlValuesToDocument()
{
	UpdateData(TRUE);

	itsMacroParamDataGenerator->DialogBaseDataParamProducerString(CFmiWin32Helpers::CT2std(itsBaseDataParamProducerString));
	itsMacroParamDataGenerator->DialogUsedProducerString(CFmiWin32Helpers::CT2std(itsProducerIdNamePairString));
	itsMacroParamDataGenerator->DialogDataGeneratingSmarttoolPathString(CFmiWin32Helpers::CT2std(itsUsedDataGenerationSmarttoolPath));
	itsMacroParamDataGenerator->DialogUsedParameterListString(CFmiWin32Helpers::CT2std(itsUsedParameterListString));
	itsMacroParamDataGenerator->DialogDataStorageFileFilter(CFmiWin32Helpers::CT2std(itsGeneratedDataStorageFileFilter));
	itsMacroParamDataGenerator->DialogDataTriggerList(CFmiWin32Helpers::CT2std(itsDataTriggerList));
}

// Tarkista kaikki syˆtteet ja jos niiss‰ on vikaa:
// 1. Lokita ongelmasta
// 2. Maalaa kontrolliin liittyv‰ static teksti kontrolli punaiseksi
// 3. Disabloi Generate nappi
void CFmiMacroParamDataGeneratorDlg::DoFullInputChecks()
{
	OnChangeEditBaseDataParamProducer();
	OnChangeEditProducerIdNamePair();
	OnChangeEditUsedParameterList();
	OnChangeEditUsedDataGenerationSmarttoolPath();
	OnChangeEditGeneratedDataStorageFileFilter();
	OnEnChangeEditUsedDataTriggerList();
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
	// T‰t‰ dialogia ei tarvitse p‰ivitt‰‰ oikeasti, Update metodia tarvitaan vain koska tietyt template apufunktiot sit‰ vaativat.
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
	// aktivoidaan karttan‰yttˆ eli mainframe
	AfxGetMainWnd()->SetActiveWindow();
}


void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonGenerateMacroParamData()
{
	EnableDialogueControl(IDC_BUTTON_GENERATE_MACRO_PARAM_DATA, false);
	StoreControlValuesToDocument();
	std::thread t(&CFmiMacroParamDataGeneratorDlg::LaunchMacroParamDataGeneration, this);
	t.detach();
}

void CFmiMacroParamDataGeneratorDlg::LaunchMacroParamDataGeneration()
{
	itsMacroParamDataGenerator->GenerateMacroParamData(mThreadCallBacksPtr.get());
}

void CFmiMacroParamDataGeneratorDlg::EnableDialogueControl(int controlId, bool enable)
{
	auto* ctrl = GetDlgItem(controlId);
	if(ctrl)
	{
		ctrl->EnableWindow(enable ? TRUE : FALSE);
	}
}

HBRUSH CFmiMacroParamDataGeneratorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_BASE_DATA_PARAM_PRODUCER_STR)
	{
		if(fBaseDataParamProducerStringHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se v‰rj‰t‰‰n mustaksi
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_PRODUCER_ID_NAME_PAIR_STR)
	{
		if(fProducerIdNamePairStringHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se v‰rj‰t‰‰n mustaksi
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_USED_PARAMETER_LIST_STR)
	{
		if(fUsedParameterListStringHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se v‰rj‰t‰‰n mustaksi
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_DATA_GENERATION_SMARTTOOL_PATH_STR)
	{
		if(fUsedDataGenerationSmarttoolPathHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se v‰rj‰t‰‰n mustaksi
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_GENERATED_DATA_STORAGE_FILE_FILTER_STR)
	{
		if(fGeneratedDataStorageFileFilterHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se v‰rj‰t‰‰n mustaksi
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_USED_DATA_TRIGGER_LIST)
	{
		if(fDataTriggerListHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se v‰rj‰t‰‰n mustaksi
	}

	return hbr;
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditBaseDataParamProducer()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsBaseDataParamProducerString);
	auto checkResult = NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(tmp, false);
	fBaseDataParamProducerStringHasInvalidValues = !checkResult.first.empty();

	// Edit kentt‰‰n liittyv‰ otsikkokontrolli v‰rj‰t‰‰n punaiseksi, jos inputissa on vikaa
	CWnd* win = GetDlgItem(IDC_STATIC_BASE_DATA_PARAM_PRODUCER_STR);
	if(win)
		win->Invalidate(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditProducerIdNamePair()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsProducerIdNamePairString);
	auto checkResult = NFmiMacroParamDataInfo::CheckUsedProducerString(tmp);
	fProducerIdNamePairStringHasInvalidValues = !checkResult.first.empty();

	// Edit kentt‰‰n liittyv‰ otsikkokontrolli v‰rj‰t‰‰n punaiseksi, jos inputissa on vikaa
	CWnd* win = GetDlgItem(IDC_STATIC_PRODUCER_ID_NAME_PAIR_STR);
	if(win)
		win->Invalidate(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditUsedParameterList()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsUsedParameterListString);
	NFmiProducer dummyProducer;
	auto checkResult = NFmiMacroParamDataInfo::CheckUsedParameterListString(tmp, dummyProducer);
	fUsedParameterListStringHasInvalidValues = !checkResult.first.empty();

	// Edit kentt‰‰n liittyv‰ otsikkokontrolli v‰rj‰t‰‰n punaiseksi, jos inputissa on vikaa
	CWnd* win = GetDlgItem(IDC_STATIC_USED_PARAMETER_LIST_STR);
	if(win)
		win->Invalidate(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditGeneratedDataStorageFileFilter()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsGeneratedDataStorageFileFilter);
	auto checkResult = NFmiMacroParamDataInfo::CheckDataStorageFileFilter(tmp);
	fGeneratedDataStorageFileFilterHasInvalidValues = !checkResult.empty();

	// Edit kentt‰‰n liittyv‰ otsikkokontrolli v‰rj‰t‰‰n punaiseksi, jos inputissa on vikaa
	CWnd* win = GetDlgItem(IDC_STATIC_GENERATED_DATA_STORAGE_FILE_FILTER_STR);
	if(win)
		win->Invalidate(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditUsedDataGenerationSmarttoolPath()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsUsedDataGenerationSmarttoolPath);
	auto checkResult = NFmiMacroParamDataInfo::CheckDataGeneratingSmarttoolPathString(tmp);
	fUsedDataGenerationSmarttoolPathHasInvalidValues = !checkResult.empty();

	// Edit kentt‰‰n liittyv‰ otsikkokontrolli v‰rj‰t‰‰n punaiseksi, jos inputissa on vikaa
	CWnd* win = GetDlgItem(IDC_STATIC_DATA_GENERATION_SMARTTOOL_PATH_STR);
	if(win)
		win->Invalidate(FALSE);
}

void CFmiMacroParamDataGeneratorDlg::OnEnChangeEditUsedDataTriggerList()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsDataTriggerList);
	auto checkResult = NFmiMacroParamDataInfo::CheckDataTriggerListString(tmp);
	fDataTriggerListHasInvalidValues = !checkResult.empty();

	// Edit kentt‰‰n liittyv‰ otsikkokontrolli v‰rj‰t‰‰n punaiseksi, jos inputissa on vikaa
	CWnd* win = GetDlgItem(IDC_STATIC_USED_DATA_TRIGGER_LIST);
	if(win)
		win->Invalidate(FALSE);
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
	// Progress dialogi k‰ynnistet‰‰n p‰‰-threadista, mutta sen kanssa operoidaan
	// tyˆ-threadista k‰sin. T‰m‰n funktion tarkoitus on varmistaa ett‰ tietyt
	// ikkunat ja kontrollit on alustettu, ennen kuin niit‰ oikeasti k‰ytet‰‰n.
	// T‰m‰ varmistus tehd‰‰n loopissa jossa testataan onko GetSafeHwnd -metodin palauttama arvo ok. 
	// Jos ei, odota pikkuisen ja yrit‰ uudestaan...
	// Jos ikkuna on lopuksi alustettu palauttaa true, mutta jos tarpeeksi monen yrityksen j‰lkeen homma 
	// ei ole valmis, palautetaan false, jotta ei j‰‰d‰ iki-looppiin.
	if(checkedWindow)
	{
		for(int i = 0; checkedWindow->GetSafeHwnd() == NULL; i++)
		{
			if(i > 200)
				return false; // t‰ll‰ estet‰‰n iki-looppi
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
	// Pit‰‰ varmistaa ett‰ dialogi on alustettu
	if(::WaitForWindowToInitialize(this))
	{
		// Lis‰ksi pit‰‰ varmistaa ett‰ itsProgressCtrl on myˆs alustettu!
		if(::WaitForWindowToInitialize(&mProgressControl))
		{
			return true;
		}
	}

	return false;
}


BOOL CFmiMacroParamDataGeneratorDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if(message == ID_MACRO_PARAM_DATA_GENERATION_FINISHED)
	{
		EnableDialogueControl(IDC_BUTTON_GENERATE_MACRO_PARAM_DATA, true);
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonSaveMacroParamData()
{
	// Ennen tallennusta talletetaan varmuuden vuoksi s‰‰dˆt myˆs dokumenttiin
	StoreControlValuesToDocument(); 

	auto macroParamsDataInfo = itsMacroParamDataGenerator->MakeDataInfo();
	auto initialSavePath = itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath();
	std::string fullPath;
	std::string rootMacroParDataInfoPath;
	if(BetaProduct::SaveObjectInJsonFormat(macroParamsDataInfo, initialSavePath, NFmiMacroParamDataGenerator::MacroParamDataInfoFileFilter(), NFmiMacroParamDataGenerator::MacroParamDataInfoFileExtension(), rootMacroParDataInfoPath, "MacroParam data info", "MacroParDataInfo1", false, &fullPath, this))
	{
		itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath(initialSavePath);
		UpdateMacroParamDataInfoName(fullPath);
	}
}

void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonLoadMacroParamData()
{
	NFmiMacroParamDataInfo macroParamsDataInfo;
	auto initialSavePath = itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath();
	std::string fullPath;
	std::string rootMacroParDataInfoPath;
	if(BetaProduct::LoadObjectInJsonFormat(macroParamsDataInfo, initialSavePath, NFmiMacroParamDataGenerator::MacroParamDataInfoFileFilter(), NFmiMacroParamDataGenerator::MacroParamDataInfoFileExtension(), rootMacroParDataInfoPath, "MacroParam data info", false, &fullPath, this))
	{
		itsMacroParamDataGenerator->MacroParamDataInfoSaveInitialPath(initialSavePath);
		UpdateMacroParamDataInfoName(fullPath);
		InitControlsFromLoadedMacroParamDataInfo(macroParamsDataInfo);
	}
}

void CFmiMacroParamDataGeneratorDlg::UpdateMacroParamDataInfoName(const std::string& fullPath)
{
	mLoadedMacroParamDataInfoName = CA2T(fullPath.c_str());
	UpdateData(FALSE);
}
