#include "stdafx.h"
#include "FmiCaseStudyDlg.h"
#include "FmiWin32TemplateHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "CloneBitmap.h"
#include "SmartMetToolboxDep_resource.h"
#include "GridCellCheck.h"
#include "NFmiCaseStudySystem.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "NFmiFileString.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiApplicationDataBase.h"
#include "NFmiFileSystem.h"
#include "FmiWin32Helpers.h"
#include "persist2.h"
#include "HakeMessage/Main.h"

#include <boost/math/special_functions/round.hpp>
#include "execute-command-in-separate-process.h"

static const COLORREF gFixedBkColor = RGB(239, 235, 222);
const double gMegaByteSize = 1024*1024;

static const std::string gEditEnableDataCheckControlOffStr = "Edit Enable Data";
static const std::string gEditEnableDataCheckControlOnStr = "Edit Enable Data (MUST restart SmartMet to take changes in use)";

namespace
{
	const NFmiApplicationDataBase::AppSpyData gCaseStudyMakerData("CaseStudyExe.exe", false); // false = ei olla kiinnostuneita ohjelman versiosta
}

IMPLEMENT_DYNCREATE(NFmiCaseStudyGridCtrl, CGridCtrl)

BEGIN_MESSAGE_MAP(NFmiCaseStudyGridCtrl, CGridCtrl)
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

void NFmiCaseStudyGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnRButtonUp(nFlags, point);

	// Tarkoitus on deselectoida kaikki
	SetSelectedRange(-1,-1,-1,-1, TRUE, TRUE);
}


static double ConvertToMB(double theFileSizeInBytes)
{
	return theFileSizeInBytes/gMegaByteSize;
}

static void SetHeaders(CGridCtrl &theGridCtrl, const std::vector<CaseStudyHeaderParInfo> &theHeaders, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount)
{
	int columnCount = static_cast<int>(theHeaders.size());
	theGridCtrl.SetRowCount(rowCount);
	theGridCtrl.SetColumnCount(columnCount);
	theGridCtrl.SetGridLines(GVL_BOTH);
	theGridCtrl.SetFixedRowCount(theFixedRowCount);
	theGridCtrl.SetFixedColumnCount(theFixedColumnCount);
	theGridCtrl.SetListMode(TRUE);
	theGridCtrl.SetHeaderSort(FALSE);
	theGridCtrl.SetFixedBkColor(gFixedBkColor);

	int currentRow = 0;
	// 1. on otsikko rivi on parametrien nimi� varten
	for(int i=0; i<columnCount; i++)
	{
		theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
		theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
		if(fFirstTime) // s��det��n sarakkeiden leveydet vain 1. kerran
			theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
	}
	fFirstTime = false;
}

void CFmiCaseStudyDlg::InitHeaders()
{
	int basicColumnWidthUnit = 18;
	itsHeaders.clear();
	itsHeaders.push_back(CaseStudyHeaderParInfo("Row", CaseStudyHeaderParInfo::kRowNumber, boost::math::iround(basicColumnWidthUnit*2.5)));
    itsHeaders.push_back(CaseStudyHeaderParInfo("Name", CaseStudyHeaderParInfo::kModelName, basicColumnWidthUnit * 18));
    itsHeaders.push_back(CaseStudyHeaderParInfo("ProdId", CaseStudyHeaderParInfo::kModelName, basicColumnWidthUnit * 4));
	itsHeaders.push_back(CaseStudyHeaderParInfo("Store", CaseStudyHeaderParInfo::kStoreData, basicColumnWidthUnit*3));
    itsHeaders.push_back(CaseStudyHeaderParInfo("CS CNT", CaseStudyHeaderParInfo::kCaseStudyDataCount, boost::math::iround(basicColumnWidthUnit * 4.)));
    itsHeaders.push_back(CaseStudyHeaderParInfo("LC CNT", CaseStudyHeaderParInfo::kLocalCacheDataCount, boost::math::iround(basicColumnWidthUnit * 4.)));
    itsHeaders.push_back(CaseStudyHeaderParInfo("Enable Data", CaseStudyHeaderParInfo::kEnableData, basicColumnWidthUnit*5)); // sijoita t�m� indeksille CaseStudyHeaderParInfo::kEnableData
    itsHeaders.push_back(CaseStudyHeaderParInfo("File(s) size [MB]", CaseStudyHeaderParInfo::kDataSize, boost::math::iround(basicColumnWidthUnit * 10.)));
}

const NFmiViewPosRegistryInfo CFmiCaseStudyDlg::s_ViewPosRegistryInfo(CRect(150, 250, 670, 800), "\\CaseStudyDialog");

// CFmiCaseStudyDlg dialog

IMPLEMENT_DYNAMIC(CFmiCaseStudyDlg, CDialog)

CFmiCaseStudyDlg::CFmiCaseStudyDlg(SmartMetDocumentInterface *smartMetDocumentInterface, const std::string theTitleStr, CWnd* pParent)
	: CDialog(CFmiCaseStudyDlg::IDD, pParent)
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	,itsTitleStr(theTitleStr)
	,fGridControlInitialized(false)
	,fDialogInitialized(false)
	,itsGridCtrl()
	,itsHeaders()
	,itsOffsetEditTimer(0)
	,itsDisableStoreButtonTimer(0)
	,itsOffsetEditedCell()
	,itsBoldFont()
	, itsCaseStudySettingsWinRegistry(smartMetDocumentInterface->ApplicationWinRegistry().CaseStudySettingsWinRegistry())
    , itsNameStrU_(_T(""))
    , itsInfoStrU_(_T(""))
    , itsPathStrU_(_T(""))
    , fEditEnableData(FALSE)
    , fZipData(FALSE)
	, fStoreWarningMessages(FALSE)
	, fCropDataToZoomedMapArea(FALSE)
{

}

CFmiCaseStudyDlg::~CFmiCaseStudyDlg()
{
}

BOOL CFmiCaseStudyDlg::Create(CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::Create(CFmiCaseStudyDlg::IDD, pParentWnd);
}

void CFmiCaseStudyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_GridControl(pDX, IDC_CUSTOM_GRID_CASE_STUDY, itsGridCtrl);
	DDX_Text(pDX, IDC_EDIT_NAME_STR, itsNameStrU_);
	DDX_Text(pDX, IDC_EDIT_INFO_STR, itsInfoStrU_);
	DDX_Text(pDX, IDC_EDIT_PATH_STR, itsPathStrU_);
	DDX_Check(pDX, IDC_CHECK_EDIT_ENABLE_DATA, fEditEnableData);
	DDX_Check(pDX, IDC_CHECK_ZIP_DATA, fZipData);
	DDX_Check(pDX, IDC_CHECK_STORE_WARNING_MESSAGES, fStoreWarningMessages);
	DDX_Check(pDX, IDC_CHECK_CROP_DATA_TO_ZOOMED_MAP_AREA, fCropDataToZoomedMapArea);
}

#define WM_CASE_STUDY_OFFSET_EDITED = WM_USER + 222

BEGIN_MESSAGE_MAP(CFmiCaseStudyDlg, CDialog)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_CLOSE()
//	ON_WM_PAINT()
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_CUSTOM_GRID_CASE_STUDY, OnGridEndEdit)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_STORE_DATA, &CFmiCaseStudyDlg::OnBnClickedButtonStoreData)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_DATA, &CFmiCaseStudyDlg::OnBnClickedButtonLoadData)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CFmiCaseStudyDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE_MODE, &CFmiCaseStudyDlg::OnBnClickedButtonCloseMode)
    ON_BN_CLICKED(IDC_CHECK_EDIT_ENABLE_DATA, &CFmiCaseStudyDlg::OnBnClickedCheckEditEnableData)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_REFRESH_GRID, &CFmiCaseStudyDlg::OnBnClickedButtonRefreshGrid)
END_MESSAGE_MAP()


// CFmiCaseStudyDlg message handlers

void CFmiCaseStudyDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CRect windowRect;
	GetWindowRect(&windowRect);
	CRect clientRect;
	GetClientRect(&clientRect);
	int widthOverHead = windowRect.Width() - clientRect.Width();
	int heightOverHead = windowRect.Height() - clientRect.Height();
	lpMMI->ptMinTrackSize.x = 400 + widthOverHead;
	lpMMI->ptMinTrackSize.y = 400 + heightOverHead;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

void CFmiCaseStudyDlg::SetDefaultValues()
{
    MoveWindow(CFmiCaseStudyDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

BOOL CFmiCaseStudyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	DoResizerHooking(); // T�t� pit�� kutsua ennen kuin dialogin talletettu koko otetaan Windows rekisterist�
	fDialogInitialized = true;

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	// Call InitHeaders before CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry !!
    InitHeaders();
    // Tee paikan asetus vasta tooltipin alustuksen j�lkeen, niin se toimii ilman OnSize-kutsua.
	std::string errorBaseStr("Error in CFmiCaseStudyDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
	itsNameStrU_ = CA2T(caseStudySystem.Name().c_str());
    itsInfoStrU_ = CA2T(caseStudySystem.Info().c_str());
    itsPathStrU_ = CA2T(caseStudySystem.CaseStudyPath().c_str());
	fZipData = caseStudySystem.ZipFiles();
	fStoreWarningMessages = caseStudySystem.StoreWarningMessages();

	UpdateButtonStates();
    UpdateEditEnableDataText();
    EnableColorCodedControls();
	InitializeGridControlRelatedData();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiCaseStudyDlg::InitDialogTexts()
{
	SetWindowText(CA2T(itsTitleStr.c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_STORE_DATA, "Store data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_LOAD_DATA, "Load data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CLOSE_MODE, "Close mode");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REFRESH_GRID, "Refresh");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_NAME2_STR, "Name:");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_INFO_STR, "Info:");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PATH_STR, "Absolute path:\n(e.g.c:\\data)");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_STORE_WARNING_MESSAGES, "Store warnings");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_ZIP_DATA, "Zip data");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_CROP_DATA_TO_ZOOMED_MAP_AREA, "Crop data to zoomed main-map area");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_BROWSE, "Browse");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_EDIT_ENABLE_DATA, gEditEnableDataCheckControlOffStr.c_str());
}

void CFmiCaseStudyDlg::DoResizerHooking(void)
{
	BOOL bOk = m_resizer.Hook(this);
	ASSERT(bOk == TRUE);

	bOk = m_resizer.SetAnchor(IDC_BUTTON_STORE_DATA, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_LOAD_DATA, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_CLOSE_MODE, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_REFRESH_GRID, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_NAME2_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_INFO_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_STATIC_PATH_STR, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_CHECK_STORE_WARNING_MESSAGES, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_CHECK_ZIP_DATA, ANCHOR_TOP | ANCHOR_LEFT);
	ASSERT(bOk == TRUE);

	bOk = m_resizer.SetAnchor(IDC_CHECK_CROP_DATA_TO_ZOOMED_MAP_AREA, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_BUTTON_BROWSE, ANCHOR_TOP | ANCHOR_RIGHT);
	ASSERT(bOk == TRUE);

	bOk = m_resizer.SetAnchor(IDC_EDIT_NAME_STR, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_INFO_STR, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_EDIT_PATH_STR, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
	bOk = m_resizer.SetAnchor(IDC_CHECK_EDIT_ENABLE_DATA, ANCHOR_TOP | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);

	bOk = m_resizer.SetAnchor(IDC_CUSTOM_GRID_CASE_STUDY, ANCHOR_VERTICALLY | ANCHOR_HORIZONTALLY);
	ASSERT(bOk == TRUE);
}

void CFmiCaseStudyDlg::EnableColorCodedControls()
{
    CFmiWin32Helpers::EnableColorCodedControl(this, IDC_CHECK_EDIT_ENABLE_DATA);
}

void CFmiCaseStudyDlg::UpdateEditEnableDataText()
{
    if(fEditEnableData)
        CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_EDIT_ENABLE_DATA, gEditEnableDataCheckControlOnStr.c_str());
    else
        CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_EDIT_ENABLE_DATA, gEditEnableDataCheckControlOffStr.c_str());
}

void CFmiCaseStudyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

//	AdjustDialogControls();
}

void CFmiCaseStudyDlg::InitializeGridControlRelatedData()
{
	if(fGridControlInitialized == false)
	{
		fGridControlInitialized = true;
		itsProducerSystemsHolder = std::make_unique<NFmiProducerSystemsHolder>();
		itsProducerSystemsHolder->itsModelProducerSystem = &(itsSmartMetDocumentInterface->ProducerSystem());
		itsProducerSystemsHolder->itsObsProducerSystem = &(itsSmartMetDocumentInterface->ObsProducerSystem());
		itsProducerSystemsHolder->itsSatelImageProducerSystem = &(itsSmartMetDocumentInterface->SatelImageProducerSystem());
		itsGridCtrl.GetFont()->GetLogFont(&itsBoldFont); // otetaan defaultti fontti tiedot talteen
		itsBoldFont.lfWeight = FW_BOLD; // asetetaan 'paino' bold-arvoon 
		// CaseStudy datoihin pit�� tehd� tuottaja kohtaisia p�ivityksi� ennen kuin ne n�ytet��n 1. kerran
		OnBnClickedButtonRefreshGrid();
		// Kun dialogia alustetaan, pit�� t�t� kutsua kerran, koska t�m� laittaa piiloon "Enable data" sarakkeen, 
		// joka aukeaa n�kyville vasta kun sen toglaava checkbox kontrollia on klikattu.
		ShowEnableColumn();
	}
}

void CFmiCaseStudyDlg::DoWhenClosing()
{
//	itsDoc->DataQualityChecker().ViewOn(false); // jos jossain olisi tieto ett� onko caseStudy-dialogi auki/ei, t�ss� olisi muokattava sen statusta
	GetBasicInfoFromDialog();
	itsSmartMetDocumentInterface->StoreCaseStudyMemory(); // Kun dialogi suljetaan, talletetaan aina casestudy-metatieto talteen
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan�ytt� eli mainframe
}

void CFmiCaseStudyDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiCaseStudyDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

void CFmiCaseStudyDlg::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose();
}

static std::string GetMegaByteSizeString(double theSizeInMB)
{
	double rounder = 1;
	if(theSizeInMB < 1)
		rounder = 0.01;
	else if(theSizeInMB < 10)
		rounder = 0.1;

	theSizeInMB = ::round(theSizeInMB/rounder)*rounder;
	return NFmiStringTools::Convert(theSizeInMB);
}

static bool IsHeaderRow(const NFmiCaseStudyDataFile &theCaseData)
{
	if(theCaseData.CategoryHeader() == true || theCaseData.ProducerHeader() == true)
		return true;
	else
		return false;
}

// Sizi column string on seuraavanlainen:
// 1. Category/producer -headerit:
// Tot=xx.x Max=yy.y
// 2. normaali data:
// Tot=xx.x 1=yy.y
// 3. normaali data jos store=false:
// (Tot=xx.x 1=yy.y)
// 4. normaali data, jossa vain viim. data talletetaan:
// 1=yy.y
// 5. normaali data, jossa vain viim. data talletetaan, mutta store=false:
// (1=yy.y)
static std::string GetSizeColumnString(const NFmiCaseStudyDataFile &theCaseData)
{
	size_t tabLineCount = 8; // yritin laittaa tabulaattorin stringiin joka annetaan hila ruutuun, mutta se ignoorataan jotenkin
						  // joten yrit�n itse laskea mist� kohtaa alkaa sulku lause, niin ett� ne alkaisivat tasaisesti taulukossa
	std::string str;
	if(::IsHeaderRow(theCaseData))
	{
		str += "Tot=";
		str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.TotalFileSize()));
		str += "   Max=";
		str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.MaxFileSize()));
	}
	else
	{
		bool dontStoreData = (theCaseData.DataFileWinRegValues().Store() == false); // huom! sulut tulee vain jos ollaan tavallisen datan kohdassa, ei headerien
		if(dontStoreData)
			str += "(";
		if(theCaseData.StoreLastDataOnly())
		{
			str += "1=";
			str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.SingleFileSize()));
		}
		else
		{
			str += "Tot=";
			str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.TotalFileSize()));
			str += "   1=";
			str += ::GetMegaByteSizeString(::ConvertToMB(theCaseData.SingleFileSize()));
		}

		if(dontStoreData)
			str += ")";
	}
	return str;
}

static std::string GetProducerIdString(const NFmiCaseStudyDataFile &theCaseData)
{
    if(theCaseData.CategoryHeader())
        return ""; // Categories don't have specific producer id.
    else
        return NFmiStringTools::Convert(theCaseData.Producer().GetIdent());
}

static std::string GetDataCountStr(int theDataCount)
{
	return std::to_string(theDataCount);
}

static std::string GetColumnText(int theRow, int theColumn, const NFmiCaseStudyDataFile &theCaseData)
{
	switch(theColumn)
	{
	case CaseStudyHeaderParInfo::kRowNumber:
		return NFmiStringTools::Convert(theRow);
	case CaseStudyHeaderParInfo::kModelName:
		return theCaseData.Name();
    case CaseStudyHeaderParInfo::kProducerId:
        return ::GetProducerIdString(theCaseData);
    case CaseStudyHeaderParInfo::kCaseStudyDataCount:
		return ::GetDataCountStr(theCaseData.DataFileWinRegValues().CaseStudyDataCount());
	case CaseStudyHeaderParInfo::kLocalCacheDataCount:
		return ::GetDataCountStr(theCaseData.DataFileWinRegValues().LocalCacheDataCount());
	case CaseStudyHeaderParInfo::kDataSize:
		return ::GetSizeColumnString(theCaseData);
	default:
		return "";
	}
}

static bool IsReadOnlyCell(int column, const NFmiCaseStudyDataFile &theCaseData)
{
    if(column == CaseStudyHeaderParInfo::kStoreData || column == CaseStudyHeaderParInfo::kEnableData)
		return false;
	if(column == CaseStudyHeaderParInfo::kCaseStudyDataCount)
	{
		if(theCaseData.StoreLastDataOnly())
			return true;
		else
			return false;
	}
	if(column == CaseStudyHeaderParInfo::kLocalCacheDataCount)
	{
		if(theCaseData.StoreLastDataOnly())
			return true;
		if(theCaseData.DataType() == NFmiInfoData::kSatelData)
			return true;
		else
			return false;
	}

	return true;
}

static const COLORREF gNormalBkColor = RGB(255, 255, 255);
static const COLORREF gCategoryBkColor = RGB(250, 220, 220);
static const COLORREF gProducerBkColor = RGB(220, 250, 220);
static const COLORREF gOneDataBkColor = RGB(240, 240, 255);
static const COLORREF gDisabledDataBkColor = RGB(170, 170, 170); // disabloitujen datojen rivi v�riksi harmaa

static void SetCellFont(CGridCtrl &theGridCtrl, int row, int col, const NFmiCaseStudyDataFile &theCaseData, LOGFONT *theBoldFont)
{
	bool useBoldFont = theCaseData.DataFileWinRegValues().Store();
	if(::IsHeaderRow(theCaseData))
	{
		if(theCaseData.TotalFileSize() <= 0)
			useBoldFont = false; // jos kyse on header-rivist� ja sen total koko on 0, ei laiteta rivi� boldilla riippumatta mik� on Store-asetus
		else
			useBoldFont = true; // jos kyse on header-rivist� ja total size suupempi kuin 0, laitetaan boldiksi
	}

	if(useBoldFont)
		theGridCtrl.SetItemFont(row, col, theBoldFont);
	else
		theGridCtrl.SetItemFont(row, col, NULL);
}

void CFmiCaseStudyDlg::SetGridRow(int row, const NFmiCaseStudyDataFile &theCaseData, int theFixedColumnCount, bool updateOnly)
{
	for(int column = 0; column < static_cast<int>(itsHeaders.size()); column++)
	{
        if(!updateOnly)
    		::SetCellFont(itsGridCtrl, row, column, theCaseData, &itsBoldFont);
		bool dataDisabled = theCaseData.DataEnabled() == false;
		bool readOnlyCell = ::IsReadOnlyCell(column, theCaseData);
        itsGridCtrl.SetItemText(row, column, CA2T(::GetColumnText(row, column, theCaseData).c_str()));
		if(column >= theFixedColumnCount)
		{
			if(column != CaseStudyHeaderParInfo::kEnableData)
            {
			    if(readOnlyCell || dataDisabled)
				    itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY); // Laita read-only -bitti p��lle
                else
				    itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) & ~GVIS_READONLY); // Laita read-only -bitti pois p��lt�
            }

			if(dataDisabled)
				itsGridCtrl.SetItemBkColour(row, column, gDisabledDataBkColor);
			else if(theCaseData.CategoryHeader())
				itsGridCtrl.SetItemBkColour(row, column, gCategoryBkColor);
			else if(theCaseData.ProducerHeader())
				itsGridCtrl.SetItemBkColour(row, column, gProducerBkColor);
			else if(theCaseData.OnlyOneData())
				itsGridCtrl.SetItemBkColour(row, column, gOneDataBkColor);
			else
				itsGridCtrl.SetItemBkColour(row, column, gNormalBkColor);

            if(column == CaseStudyHeaderParInfo::kStoreData)
			{
                if(!updateOnly)
				    itsGridCtrl.SetCellType(row, column, RUNTIME_CLASS(CGridCellCheck));
				CGridCellCheck *pTempCell = (CGridCellCheck*) itsGridCtrl.GetCell(row, column);
				pTempCell->SetCheck(theCaseData.DataFileWinRegValues().Store());
                if(!updateOnly)
    				pTempCell->SetCheckBoxClickedCallback(boost::bind(&CFmiCaseStudyDlg::HandleCheckBoxClick, this, _1, _2));
			}
            else if(column == CaseStudyHeaderParInfo::kEnableData)
			{
                if(!updateOnly)
    				itsGridCtrl.SetCellType(row, column, RUNTIME_CLASS(CGridCellCheck));
				CGridCellCheck *pTempCell = (CGridCellCheck*) itsGridCtrl.GetCell(row, column);
                pTempCell->SetCheck(theCaseData.DataEnabled()); 
                if(!updateOnly)
    				pTempCell->SetCheckBoxClickedCallback(boost::bind(&CFmiCaseStudyDlg::HandleEnableDataCheckBoxClick, this, _1, _2));
			}
		}
	}
}

void CFmiCaseStudyDlg::InitGridControlValues()
{
	static bool fFirstTime = true;
	int fixedRowCount = 1;
	int fixedColumnCount = 1;

    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
    int dataRowCount = static_cast<int>(caseStudySystem.CaseStudyDialogData().size());
	int maxRowCount = fixedRowCount + dataRowCount;
	SetHeaders(itsGridCtrl, itsHeaders, maxRowCount, fFirstTime, fixedRowCount, fixedColumnCount);
    UpdateRows(fixedRowCount, fixedColumnCount, false);

	std::vector<unsigned char> &treePatternArray = caseStudySystem.TreePatternArray();
	if(treePatternArray.size()) // pit�� testata 0 koko vastaan, muuten voi kaatua
		itsTreeColumn.TreeSetup(&itsGridCtrl, 1, static_cast<int>(treePatternArray.size()), 1, &treePatternArray[0], TRUE, FALSE);
    fFirstTime = false;
}

void CFmiCaseStudyDlg::UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly)
{
	auto &dataVector = itsSmartMetDocumentInterface->CaseStudySystem().CaseStudyDialogData();
	int currentRowCount = fixedRowCount;
	for(auto data : dataVector)
	{
        SetGridRow(currentRowCount++, *data, fixedColumnCount, updateOnly);
	}
}

void CFmiCaseStudyDlg::UpdateGridControlValues(bool updateOnly)
{
    UpdateRows(1, 1, updateOnly);
	UpdateData(FALSE);
	Invalidate(FALSE);
}

void CFmiCaseStudyDlg::HandleCheckBoxClick(int col, int row)
{
	UpdateData(TRUE);
	CGridCellCheck *pCell = dynamic_cast<CGridCellCheck *>(itsGridCtrl.GetCell(row, col));
	if(pCell)
	{
		bool newState = pCell->GetCheck() == TRUE;
		int dataIndex = row - 1; // rivit alkavat 1:st�, mutta datat on vektorissa 0:sta alkaen
        auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
        std::vector<NFmiCaseStudyDataFile*> &dataVector = caseStudySystem.CaseStudyDialogData();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{
			NFmiCaseStudyDataFile &dataFile = *dataVector[dataIndex];
			if(dataFile.CategoryHeader())
			{
                caseStudySystem.CategoryStore(dataFile.Category(), newState); // t�m� tekee my�s tarvittavat updatet data rakenteille
			}
			else if(dataFile.ProducerHeader())
			{
                caseStudySystem.ProducerStore(dataFile.Category(), dataFile.Producer().GetIdent(), newState); // t�m� tekee my�s tarvittavat updatet data rakenteille
			}
			else
			{
				dataFile.DataFileWinRegValues().Store(newState);
                caseStudySystem.Update(dataFile.Category(), dataFile.Producer().GetIdent());
			}
			UpdateGridControlValues(true);
		}
	}
}

void CFmiCaseStudyDlg::HandleEnableDataCheckBoxClick(int col, int row)
{
	UpdateData(TRUE);
	CGridCellCheck *pCell = dynamic_cast<CGridCellCheck *>(itsGridCtrl.GetCell(row, col));
	if(pCell)
	{
		bool newState = pCell->GetCheck() == TRUE;
		int dataIndex = row - 1; // rivit alkavat 1:st�, mutta datat on vektorissa 0:sta alkaen
        auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
		std::vector<NFmiCaseStudyDataFile*> &dataVector = caseStudySystem.CaseStudyDialogData();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{
			NFmiCaseStudyDataFile &dataFile = *dataVector[dataIndex];
			if(dataFile.CategoryHeader())
			{
                caseStudySystem.CategoryEnable(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), dataFile.Category(), newState); // t�m� tekee my�s tarvittavat updatet data rakenteille
			}
			else if(dataFile.ProducerHeader())
			{
                caseStudySystem.ProducerEnable(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), dataFile.Category(), dataFile.Producer().GetIdent(), newState); // t�m� tekee my�s tarvittavat updatet data rakenteille
			}
			else
			{
                dataFile.DataEnabled(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), newState);
                caseStudySystem.Update(dataFile.Category(), dataFile.Producer().GetIdent());
			}
			UpdateGridControlValues(true);
            itsSmartMetDocumentInterface->UpdateEnableDataChangesToWinReg();
		}
	}
}

void CFmiCaseStudyDlg::OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	UpdateData(TRUE);
    NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
    BOOL bAllowEdit = AcceptChange(pItem->iColumn, pItem->iRow);
    *pResult = (bAllowEdit)? 0 : -1;
}

// AcceptChange is a routine that returns TRUE
// if you want to accept the new value for the cell.
BOOL CFmiCaseStudyDlg::AcceptChange(int col, int row)
{
	if(itsGridCtrl.IsValid(row, col))
	{
		std::string str = CT2A(itsGridCtrl.GetCell(row, col)->GetText());
		try
		{
			double offsetInHours = NFmiStringTools::Convert<double>(str);
			itsOffsetEditedCell = CCellID(row, col);
			itsOffsetEditTimer = static_cast<UINT>(SetTimer(kFmiDataCountEditTimer, 50, NULL)); // k�ynnistet��n timer, ett� saadaan pikkuisen viiveen j�lkeen k�ynnistetty� offsetin muutoksesta aiheutuva p�ivitys
			return TRUE;
		}
		catch(...)
		{
			return FALSE;
		}
	}
	return FALSE;
}

void CFmiCaseStudyDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case kFmiDataCountEditTimer:
		{
			KillTimer(itsOffsetEditTimer);
			OnDataCountEdited();
			return ;
		}
	case kFmiDisableStoreButtonTimer:
		{
			if(IsThereCaseStudyMakerRunning() == false)
			{ // voidaan lopettaa timer ja laittaa Store-nappi taas enabloiduksi
				KillTimer(itsDisableStoreButtonTimer);
				itsDisableStoreButtonTimer = 0;
				UpdateButtonStates();
			}
			return ;
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CFmiCaseStudyDlg::OnDataCountEdited()
{
	if(itsGridCtrl.IsValid(itsOffsetEditedCell))
	{

		int dataIndex = itsOffsetEditedCell.row - 1; // rivit alkavat 1:st�, mutta datat on vektorissa 0:sta alkaen
        auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
        std::vector<NFmiCaseStudyDataFile*> &dataVector = caseStudySystem.CaseStudyDialogData();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{

			std::string str = CT2A(itsGridCtrl.GetCell(itsOffsetEditedCell.row, itsOffsetEditedCell.col)->GetText());
			int dataCount = 0;
			try
			{
				dataCount = boost::math::iround(NFmiStringTools::Convert<double>(str));
				bool caseStudyDataCountCase = (itsOffsetEditedCell.col == CaseStudyHeaderParInfo::kCaseStudyDataCount);

				NFmiCaseStudyDataFile &dataFile = *dataVector[dataIndex];
				if(dataFile.CategoryHeader())
				{
                    caseStudySystem.CategoryDataCount(dataFile.Category(), dataCount, caseStudyDataCountCase); // t�m� tekee my�s tarvittavat updatet data rakenteille
				}
				else if(dataFile.ProducerHeader())
				{
                    caseStudySystem.ProducerDataCount(dataFile.Category(), dataFile.Producer().GetIdent(), dataCount, caseStudyDataCountCase); // t�m� tekee my�s tarvittavat updatet data rakenteille
				}
				else
				{ 
					// normaali fileData-tapaus
					if(caseStudyDataCountCase)
						dataFile.DataFileWinRegValues().CaseStudyDataCount(dataCount);
					else
						dataFile.DataFileWinRegValues().LocalCacheDataCount(dataCount);

                    caseStudySystem.Update(dataFile.Category(), dataFile.Producer().GetIdent());
				}
				UpdateGridControlValues(true);
			}
			catch(...)
			{
			}
		}

	}
	itsOffsetEditedCell = CCellID();
}

bool CFmiCaseStudyDlg::IsThereCaseStudyMakerRunning()
{
	std::string dummyStr;
	return (NFmiApplicationDataBase::CountProcessCount(gCaseStudyMakerData, dummyStr) > 0);
}

void CFmiCaseStudyDlg::GetBasicInfoFromDialog()
{
	UpdateData(TRUE);
    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
    caseStudySystem.Name(std::string(CT2A(itsNameStrU_)));
    caseStudySystem.Info(std::string(CT2A(itsInfoStrU_)));

	// polun per�ss� ei saa olla mink��nsuuntaisia kenoviivoja. Datan kopioinnissa k�ytetty 
	// CShellFileOp -luokka on traumaattisen herkk� mink��nlaisille tai asteisille ep�m��r�isyyksille.
	// nyt jos polussa on kenoviiva per�ss�, tulee siihen kaksi kenoviivaa ja tiedostojen kopioinnit ep�onnistuvat.
	std::string strippedPathString = CT2A(itsPathStrU_);
	NFmiStringTools::TrimR(strippedPathString, '\\');
	NFmiStringTools::TrimR(strippedPathString, '/');
	NFmiStringTools::ReplaceChars(strippedPathString, '/', '\\'); // varmistetaan viel� ett� kaikki kenot ovat winkkarin omaan suuntaan.
    caseStudySystem.CaseStudyPath(strippedPathString);

	caseStudySystem.ZipFiles(fZipData == TRUE);
	caseStudySystem.StoreWarningMessages(fStoreWarningMessages == TRUE);
}

void CFmiCaseStudyDlg::OnBnClickedButtonStoreData()
{
    //	return; // Store-napin toiminta on v�liaikaisesti disabloitu, ett� saan tehty� 5.8.3.1-jakeluversion, joka korjaa kaatumistaudin 

    EnableButton(IDC_BUTTON_STORE_DATA, FALSE);
    UpdateData(TRUE);
    GetBasicInfoFromDialog();

    itsSmartMetDocumentInterface->StoreCaseStudyMemory(); // laitetaan CaseStudy-datan tilanne varmuudeksi talteen erilliseen 'muisti'-tiedostoon
    std::string dummyStr;
    if(IsThereCaseStudyMakerRunning())
    { // vain yksi CaseStudyMaker voi olla k�ynnisss� kerrallaan...
        std::string noticeStr(::GetDictionaryString("Only one Case Study data collector can be run at once."));
        noticeStr += "\n";
        noticeStr += (::GetDictionaryString("Stopping this data storing action."));
        noticeStr += "\n";
        noticeStr += (::GetDictionaryString("Try again later..."));
        std::string titleStr(::GetDictionaryString("Case Study data won't be stored"));
        ::MessageBox(this->GetSafeHwnd(), CA2T(noticeStr.c_str()), CA2T(titleStr.c_str()), MB_OK | MB_ICONSTOP);
        UpdateButtonStates();
        return;
    }

    std::string metaDataTotalFileName;
    if(itsSmartMetDocumentInterface->CaseStudySystem().StoreMetaData(this, metaDataTotalFileName, false))
    {
        std::string commandStr;
        commandStr += "\""; // laitetaan lainausmerkit komento polun ymp�rille, jos siin� sattuisi olemaan spaceja
        commandStr += itsSmartMetDocumentInterface->ApplicationDataBase().GetDecodedApplicationDirectory();
        commandStr += "\\";
        commandStr += gCaseStudyMakerData.first;
        commandStr += "\" \""; // laitetaan lainausmerkit metadatatiedoston polun ymp�rille, jos siin� sattuisi olemaan spaceja
        commandStr += metaDataTotalFileName;
        commandStr += "\""; // laitetaan lainausmerkit metadatatiedoston polun ymp�rille, jos siin� sattuisi olemaan spaceja
		commandStr += AddPossibleZippingOptions();
		commandStr += AddPossibleHakeMessageOptions();

        CFmiProcessHelpers::ExecuteCommandInSeparateProcess(commandStr, true, true, SW_MINIMIZE);
    }
    UpdateButtonStates();
}

std::string CFmiCaseStudyDlg::AddPossibleZippingOptions() const
{
	if(fZipData)
	{
		// T�m� kysymysmerkki on erotin, jolla erotellaan metatiedosto ja zip-ohjelman polku+nimi
		// En keksinyt t�h�n h�t��n muuta keinoa antaa winExelle erilaisia argumentteja yhdell� kertaa, joutuisin tekem��n erillesen komentorivi 
		// parserin, joka osaisi k�sitell� erilaisia optioita (kuten SmartMetissa tehd��n).
		// '?' on erotin, koska sellaista kirjainta ei voi olla polussa kuin wild-card -merkkin�, ei varsinaisessa polussa tai tiedoston nimess�.
		std::string zipOptions = "?";
		zipOptions += CFmiProcessHelpers::Make7zipExePath(itsSmartMetDocumentInterface->WorkingDirectory());
		return zipOptions;
	}
	else
		return "";
}

std::string CFmiCaseStudyDlg::AddPossibleHakeMessageOptions() const
{
	if(fStoreWarningMessages && itsSmartMetDocumentInterface->WarningCenterSystem().isThereAnyWorkToDo())
	{
		// T�ll�isi� store-messages optioita voi olla monia, jos tulevaisuudessa tulee uusia talletettavia sanomia.
		// ?store-messages="absolute-path-to-messages-file-filter"
		// And here is actual sample from FMI HAKE messages file-filter:
		// ?store-messages="D:\SmartMet\Dropbox (FMI)\data_FMI\HAKE\*.json"
		std::string storeMessagesOptions = "?";
		storeMessagesOptions += CFmiProcessHelpers::GetStoreMessagesString();
		storeMessagesOptions += "\""; // laitetaan lainausmerkit komento polun ymp�rille, jos siin� sattuisi olemaan spaceja
		storeMessagesOptions += itsSmartMetDocumentInterface->WarningCenterSystem().getHakeMessageAbsoluteFileFilter();
		storeMessagesOptions += "\""; // laitetaan lainausmerkit komento polun ymp�rille, jos siin� sattuisi olemaan spaceja
		return storeMessagesOptions;
	}
	else
		return "";
}

void CFmiCaseStudyDlg::OnBnClickedButtonLoadData()
{
	CWaitCursor waitCursor;
	UpdateData(TRUE);
	std::string fullPathAndName = CT2A(itsPathStrU_);
	if(itsSmartMetDocumentInterface->LoadCaseStudyData(fullPathAndName))
	{
        itsNameStrU_ = CA2T(itsSmartMetDocumentInterface->LoadedCaseStudySystem().Name().c_str());
        itsInfoStrU_ = CA2T(itsSmartMetDocumentInterface->LoadedCaseStudySystem().Info().c_str());
		UpdateData(FALSE);
	}
	UpdateButtonStates();
	itsSmartMetDocumentInterface->SetAllViewIconsDynamically();
}

void CFmiCaseStudyDlg::OnBnClickedButtonBrowse()
{
	static TCHAR BASED_CODE szFilter[] = _TEXT("CaseStudy Files (*.csmeta)|*.csmeta|All Files (*.*)|*.*||");

	UpdateData(TRUE);

	std::string initialDir = CT2A(itsPathStrU_);
	if(NFmiFileSystem::DirectoryExists(initialDir) == false)
	{
		NFmiFileString currentFilePath(initialDir);
		initialDir = currentFilePath.Device();
		initialDir += currentFilePath.Path();
	}

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
    dlg.m_ofn.lpstrInitialDir = CA2T(initialDir.c_str());
	if(dlg.DoModal() == IDOK)
	{
		itsPathStrU_ = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void CFmiCaseStudyDlg::OnBnClickedButtonCloseMode()
{
	CWaitCursor waitCursor;
	itsSmartMetDocumentInterface->CaseStudyToNormalMode();
	UpdateButtonStates();
	itsSmartMetDocumentInterface->SetAllViewIconsDynamically();
}

// T�t� kutsutaan aina tietyiss� tilanteissa (initialisointi, Store- ja Close Mode- nappien handlauksen lopuksi).
// Tarkoitus on enabloida ja disabloida Store- ja Close Mode- nappeja.
// Store-nappi on disabloitu, jos SmartMet on CaseStudy-moodissa.
// Kun Store-nappia on painettu, menn��n erikois tilaan, eli disabloidaan Store-nappi ja k�ynnistyy timer, 
// jonka avulla katsotaan onko CaseStudyExe-tallentamassa dataa, kun ei en�� ole t�t� ohjelmaa k�ynniss�, voidaan Store-nappi enabloida (jos ei olla CaseStudy-moodissa tietenkin).
// Close Mode-nappi on disabloitu, jos ei olla CaseStudy-moodissa.
void CFmiCaseStudyDlg::UpdateButtonStates()
{
	if(itsSmartMetDocumentInterface->CaseStudyModeOn())
	{
		EnableButton(IDC_BUTTON_STORE_DATA, FALSE);
		EnableButton(IDC_BUTTON_CLOSE_MODE, TRUE);
	}
	else
	{
		if(IsThereCaseStudyMakerRunning() && itsDisableStoreButtonTimer == 0)
		{
			EnableButton(IDC_BUTTON_STORE_DATA, FALSE);
			itsDisableStoreButtonTimer = static_cast<UINT>(SetTimer(kFmiDisableStoreButtonTimer, 2000, NULL)); // k�ynnistet��n timer, joka kahden sekunnin v�lein tarkistaa, voidaanko Store-nappi taas enabloida
		}
		else if(itsDisableStoreButtonTimer == 0)
			EnableButton(IDC_BUTTON_STORE_DATA, TRUE);
		EnableButton(IDC_BUTTON_CLOSE_MODE, FALSE);
	}
}

void CFmiCaseStudyDlg::EnableButton(UINT theButtonId, BOOL state)
{
	CWnd *button = GetDlgItem(theButtonId);
	if(button)
		button->EnableWindow(state);
}


void CFmiCaseStudyDlg::OnBnClickedCheckEditEnableData()
{
    ShowEnableColumn();
}

void CFmiCaseStudyDlg::ShowEnableColumn()
{
    static int lastTrueEnableColumnWidth = 0;

	UpdateData(TRUE);

    for(int i = 0; i < itsGridCtrl.GetRowCount(); i++)
    {
        if(fEditEnableData)
            itsGridCtrl.SetColumnWidth(CaseStudyHeaderParInfo::kEnableData, lastTrueEnableColumnWidth); // otetaan talessa ollut viimeinen leveys k�ytt��n kun sarake n�ytet��n taas
        else
        {
            if(i == 0)
                lastTrueEnableColumnWidth = itsGridCtrl.GetColumnWidth(CaseStudyHeaderParInfo::kEnableData); // otetaan sarakkeen leveys talteen ennen piilotusta
            itsGridCtrl.SetColumnWidth(CaseStudyHeaderParInfo::kEnableData, 0);
        }
    }

    UpdateRows(1, 1, true);
    itsGridCtrl.EnableHiddenColUnhide(FALSE);
    UpdateEditEnableDataText();
	UpdateData(FALSE);
	Invalidate(FALSE);
}


HBRUSH CFmiCaseStudyDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if(pWnd && pWnd->GetDlgCtrlID() == IDC_CHECK_EDIT_ENABLE_DATA)
    {
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, !fEditEnableData);
    }

    return hbr;
}


void CFmiCaseStudyDlg::OnBnClickedButtonRefreshGrid()
{
    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
    caseStudySystem.UpdateNoProducerData(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), *itsSmartMetDocumentInterface->InfoOrganizer());
	caseStudySystem.PutNoneProducerDataToEndFix();
	caseStudySystem.FillCaseStudyDialogData(*itsProducerSystemsHolder);
    InitGridControlValues();
}
