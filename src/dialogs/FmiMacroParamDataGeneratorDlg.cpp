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
}

void CFmiMacroParamDataGeneratorDlg::InitControlsFromDocument()
{
	if(!itsMacroParamDataGenerator)
		return;

	itsBaseDataParamProducerString = CA2T(itsMacroParamDataGenerator->DialogBaseDataParamProducerString().c_str());
	itsProducerIdNamePairString = CA2T(itsMacroParamDataGenerator->DialogUsedProducerString().c_str());
	itsUsedDataGenerationSmarttoolPath = CA2T(itsMacroParamDataGenerator->DialogDataGeneratingSmarttoolPathString().c_str());
	itsUsedParameterListString = CA2T(itsMacroParamDataGenerator->DialogUsedParameterListString().c_str());
	itsGeneratedDataStorageFileFilter = CA2T(itsMacroParamDataGenerator->DialogDataStorageFileFilter().c_str());
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
}

// Tarkista kaikki syötteet ja jos niissä on vikaa:
// 1. Lokita ongelmasta
// 2. Maalaa kontrolliin liittyvä static teksti kontrolli punaiseksi
// 3. Disabloi Generate nappi
void CFmiMacroParamDataGeneratorDlg::DoFullInputChecks()
{
	OnChangeEditBaseDataParamProducer();
	OnChangeEditProducerIdNamePair();
	OnChangeEditUsedParameterList();
	OnChangeEditUsedDataGenerationSmarttoolPath();
	OnChangeEditGeneratedDataStorageFileFilter();
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
	// Tätä dialogia ei tarvitse päivittää oikeasti, Update metodia tarvitaan vain koska tietyt template apufunktiot sitä vaativat.
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
	// aktivoidaan karttanäyttö eli mainframe
	AfxGetMainWnd()->SetActiveWindow();
}


void CFmiMacroParamDataGeneratorDlg::OnBnClickedButtonGenerateMacroParamData()
{
	StoreControlValuesToDocument();
	itsMacroParamDataGenerator->GenerateMacroParamData();
}

HBRUSH CFmiMacroParamDataGeneratorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_BASE_DATA_PARAM_PRODUCER_STR)
	{
		if(fBaseDataParamProducerStringHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli väritetään punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se värjätään mustaksi
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_PRODUCER_ID_NAME_PAIR_STR)
	{
		if(fProducerIdNamePairStringHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli väritetään punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se värjätään mustaksi
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_USED_PARAMETER_LIST_STR)
	{
		if(fUsedParameterListStringHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli väritetään punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se värjätään mustaksi
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_DATA_GENERATION_SMARTTOOL_PATH_STR)
	{
		if(fUsedDataGenerationSmarttoolPathHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli väritetään punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se värjätään mustaksi
	}
	else if(pWnd->GetDlgCtrlID() == IDC_STATIC_GENERATED_DATA_STORAGE_FILE_FILTER_STR)
	{
		if(fGeneratedDataStorageFileFilterHasInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // Virhetilanteissa edit boxin labeli väritetään punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0)); // Muuten se värjätään mustaksi
	}

	return hbr;
}

void CFmiMacroParamDataGeneratorDlg::OnChangeEditBaseDataParamProducer()
{
	UpdateData(TRUE);

	std::string tmp = CT2A(itsBaseDataParamProducerString);
	auto checkResult = NFmiMacroParamDataInfo::CheckBaseDataParamProducerString(tmp);
	fBaseDataParamProducerStringHasInvalidValues = !checkResult.first.empty();

	// Edit kenttään liittyvä otsikkokontrolli värjätään punaiseksi, jos inputissa on vikaa
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

	// Edit kenttään liittyvä otsikkokontrolli värjätään punaiseksi, jos inputissa on vikaa
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

	// Edit kenttään liittyvä otsikkokontrolli värjätään punaiseksi, jos inputissa on vikaa
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

	// Edit kenttään liittyvä otsikkokontrolli värjätään punaiseksi, jos inputissa on vikaa
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

	// Edit kenttään liittyvä otsikkokontrolli värjätään punaiseksi, jos inputissa on vikaa
	CWnd* win = GetDlgItem(IDC_STATIC_DATA_GENERATION_SMARTTOOL_PATH_STR);
	if(win)
		win->Invalidate(FALSE);
}
