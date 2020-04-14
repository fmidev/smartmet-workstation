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
	// 1. on otsikko rivi on parametrien nimiä varten
	for(int i=0; i<columnCount; i++)
	{
		theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
		theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
		if(fFirstTime) // säädetään sarakkeiden leveydet vain 1. kerran
			theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
	}
	fFirstTime = false;
}

void CFmiCaseStudyDlg::InitHeaders(void)
{
	int basicColumnWidthUnit = 18;
	itsHeaders.clear();
	itsHeaders.push_back(CaseStudyHeaderParInfo("Row", CaseStudyHeaderParInfo::kRowNumber, boost::math::iround(basicColumnWidthUnit*2.5)));
    itsHeaders.push_back(CaseStudyHeaderParInfo("Name", CaseStudyHeaderParInfo::kModelName, basicColumnWidthUnit * 18));
    itsHeaders.push_back(CaseStudyHeaderParInfo("ProdId", CaseStudyHeaderParInfo::kModelName, basicColumnWidthUnit * 4));
	itsHeaders.push_back(CaseStudyHeaderParInfo("Store", CaseStudyHeaderParInfo::kStoreData, basicColumnWidthUnit*3));
    itsHeaders.push_back(CaseStudyHeaderParInfo("Start [h]", CaseStudyHeaderParInfo::kStartTimeOffset, boost::math::iround(basicColumnWidthUnit * 4.)));
    itsHeaders.push_back(CaseStudyHeaderParInfo("End [h]", CaseStudyHeaderParInfo::kEndTimeOffset, boost::math::iround(basicColumnWidthUnit * 4.)));
    itsHeaders.push_back(CaseStudyHeaderParInfo("Enable Data", CaseStudyHeaderParInfo::kEnableData, basicColumnWidthUnit*5)); // sijoita tämä indeksille CaseStudyHeaderParInfo::kEnableData
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
    , itsNameStrU_(_T(""))
    , itsInfoStrU_(_T(""))
    , itsPathStrU_(_T(""))
    , fEditEnableData(FALSE)
    , fZipData(FALSE)
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

    // Tähän oma tarkastelu polulle, jonka pitää olla absoluuttinen
    if(itsPathStrU_.IsEmpty() == false)
    {
        // CString -> NFmiFileString piti tehdä hankalasti kolmessa vaiheessa
        std::string tmpStr = CT2A(itsPathStrU_);
        NFmiString tmpStr2(tmpStr);
        NFmiFileString fileStr(tmpStr2);
        if(fileStr.IsAbsolutePath() == false)
        {
            std::string errStr(::GetDictionaryString("Given path"));
            errStr += "\n";
            errStr += tmpStr;
            errStr += "\n";
            errStr += "was not absolute, you must provide absolute path for Case Study data.\nE.g. C:\\data or D:\\data";
            std::string captionStr(::GetDictionaryString("Case-Study data path was not absolute"));
            ::MessageBox(GetSafeHwnd(), CA2T(errStr.c_str()), CA2T(captionStr.c_str()), MB_OK | MB_ICONWARNING);
        }
    }
    DDX_Check(pDX, IDC_CHECK_EDIT_ENABLE_DATA, fEditEnableData);
    DDX_Check(pDX, IDC_CHECK_ZIP_DATA, fZipData);
}

#define WM_CASE_STUDY_OFFSET_EDITED = WM_USER + 222

BEGIN_MESSAGE_MAP(CFmiCaseStudyDlg, CDialog)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
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

void CFmiCaseStudyDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiCaseStudyDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

BOOL CFmiCaseStudyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    fDialogInitialized = true;

    SetWindowText(CA2T(itsTitleStr.c_str()));
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
    // Call InitHeaders before CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry !!
    InitHeaders();
    // Tee paikan asetus vasta tooltipin alustuksen jälkeen, niin se toimii ilman OnSize-kutsua.
	std::string errorBaseStr("Error in CFmiCaseStudyDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
	itsNameStrU_ = CA2T(caseStudySystem.Name().c_str());
    itsInfoStrU_ = CA2T(caseStudySystem.Info().c_str());
    itsPathStrU_ = CA2T(caseStudySystem.CaseStudyPath().c_str());
    fZipData = caseStudySystem.ZipFiles();

	UpdateButtonStates();
    UpdateEditEnableDataText();
    EnableColorCodedControls();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiCaseStudyDlg::EnableColorCodedControls()
{
    CFmiWin32Helpers::EnableColorCodedControl(this, IDC_CHECK_EDIT_ENABLE_DATA);
}

void CFmiCaseStudyDlg::UpdateEditEnableDataText(void)
{
    if(fEditEnableData)
        CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_EDIT_ENABLE_DATA, gEditEnableDataCheckControlOnStr.c_str());
    else
        CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_EDIT_ENABLE_DATA, gEditEnableDataCheckControlOffStr.c_str());
}

void CFmiCaseStudyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	AdjustDialogControls();
}

CRect CFmiCaseStudyDlg::CalcGridArea(void)
{
	CRect clientRect;
	GetClientRect(clientRect);
	CWnd *win = GetDlgItem(IDC_CHECK_DELETE_TMP_DATA);
	if(win)
	{
		CRect rect2;
		win->GetWindowRect(rect2);
		CPoint pt(rect2.BottomRight());
		ScreenToClient(&pt);
		clientRect.top = clientRect.top + pt.y; // rect2.Height();
	}
	return clientRect;
}

CRect CFmiCaseStudyDlg::CalcBrowseButtomRect(void)
{
	CRect buttomRect;
	CWnd *win = GetDlgItem(IDC_BUTTON_BROWSE);
	if(win)
	{
		CRect clientRect;
		GetClientRect(clientRect);

		win->GetWindowRect(buttomRect);
		ScreenToClient(&buttomRect);
		buttomRect.MoveToX(clientRect.right - buttomRect.Width() - 10);
	}
	return buttomRect;
}

void CFmiCaseStudyDlg::AdjustDialogControls(void)
{
	if(fDialogInitialized)
	{
		AdjustGridControl();
		AdjustControl(IDC_EDIT_NAME_STR, 10);
		AdjustControl(IDC_EDIT_INFO_STR, 10);
		CRect browseRect = CalcBrowseButtomRect();
		AdjustBrowseButton(browseRect);
		AdjustEditPathControl(browseRect);

        static bool firstTime = true;
        if(firstTime)
        {
            firstTime = false;
            ShowEnableColumn();
        }
	}
}

void CFmiCaseStudyDlg::AdjustBrowseButton(const CRect &theButtonRect)
{
	CWnd *win = GetDlgItem(IDC_BUTTON_BROWSE);
	if(win)
	{
		win->MoveWindow(theButtonRect);
	}
}

void CFmiCaseStudyDlg::AdjustEditPathControl(const CRect &theButtonRect)
{
	CWnd *win = GetDlgItem(IDC_EDIT_PATH_STR);
	if(win)
	{
		CRect editBoxRect;
		win->GetWindowRect(editBoxRect);
		ScreenToClient(&editBoxRect);
		editBoxRect.right = theButtonRect.left - 5;

		win->MoveWindow(editBoxRect);
	}
}

void CFmiCaseStudyDlg::AdjustGridControl(void)
{
	CWnd *win = GetDlgItem(IDC_CUSTOM_GRID_CASE_STUDY);
	if(win)
	{
		win->MoveWindow(CalcGridArea());

		if(fGridControlInitialized == false)
		{
			fGridControlInitialized = true;
            auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
            caseStudySystem.Init(*(itsSmartMetDocumentInterface->HelpDataInfoSystem()), *(itsSmartMetDocumentInterface->InfoOrganizer()));
            itsProducerSystemsHolder = std::make_unique<NFmiProducerSystemsHolder>();
			itsProducerSystemsHolder->itsModelProducerSystem = &(itsSmartMetDocumentInterface->ProducerSystem());
            itsProducerSystemsHolder->itsObsProducerSystem = &(itsSmartMetDocumentInterface->ObsProducerSystem());
            itsProducerSystemsHolder->itsSatelImageProducerSystem = &(itsSmartMetDocumentInterface->SatelImageProducerSystem());
            caseStudySystem.FillCaseStudyDialogData(*itsProducerSystemsHolder);
			itsGridCtrl.GetFont()->GetLogFont(&itsBoldFont); // otetaan defaultti fontti tiedot talteen
			itsBoldFont.lfWeight = FW_BOLD; // asetetaan 'paino' bold-arvoon 
			InitGridControlValues();
		}
	}
}

void CFmiCaseStudyDlg::AdjustControl(int theControlId, int rightOffset)
{
	CWnd *win = GetDlgItem(theControlId);
	if(win)
	{
		CRect clientRect;
		GetClientRect(clientRect);

		CRect rect2;
		win->GetWindowRect(rect2);
		CPoint tl(rect2.TopLeft());
		ScreenToClient(&tl);
		CPoint br(rect2.BottomRight());
		ScreenToClient(&br);
		br.x = clientRect.right - rightOffset;

		CRect nameRect(tl, br);
		win->MoveWindow(nameRect);
	}
}

void CFmiCaseStudyDlg::DoWhenClosing(void)
{
//	itsDoc->DataQualityChecker().ViewOn(false); // jos jossain olisi tieto että onko caseStudy-dialogi auki/ei, tässä olisi muokattava sen statusta
	GetBasicInfoFromDialog();
	itsSmartMetDocumentInterface->StoreCaseStudyMemory(); // Kun dialogi suljetaan, talletetaan aina casestudy-metatieto talteen
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
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

static std::string GetOffsetStr(const NFmiCaseStudyDataFile &theCaseData, int theColumn)
{
	int offsetValueInMinutes = (theColumn == CaseStudyHeaderParInfo::kStartTimeOffset) ? theCaseData.StartOffsetInMinutes() : theCaseData.EndOffsetInMinutes();
	if(theCaseData.StoreLastDataOnly())
		return "";
	else
		return NFmiStringTools::Convert(::round((offsetValueInMinutes/60.)*100)/100.);
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
						  // joten yritän itse laskea mistä kohtaa alkaa sulku lause, niin että ne alkaisivat tasaisesti taulukossa
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
		bool dontStoreData = (theCaseData.Store() == false); // huom! sulut tulee vain jos ollaan tavallisen datan kohdassa, ei headerien
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
    case CaseStudyHeaderParInfo::kStartTimeOffset:
	case CaseStudyHeaderParInfo::kEndTimeOffset:
		return ::GetOffsetStr(theCaseData, theColumn);
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
	if(column == CaseStudyHeaderParInfo::kStartTimeOffset || column == CaseStudyHeaderParInfo::kEndTimeOffset)
	{
		if(theCaseData.StoreLastDataOnly() == false)
			return false;
	}

	return true;
}

static const COLORREF gNormalBkColor = RGB(255, 255, 255);
static const COLORREF gCategoryBkColor = RGB(250, 220, 220);
static const COLORREF gProducerBkColor = RGB(220, 250, 220);
static const COLORREF gOneDataBkColor = RGB(240, 240, 255);
static const COLORREF gDisabledDataBkColor = RGB(170, 170, 170); // disabloitujen datojen rivi väriksi harmaa

static void SetCellFont(CGridCtrl &theGridCtrl, int row, int col, const NFmiCaseStudyDataFile &theCaseData, LOGFONT *theBoldFont)
{
	bool useBoldFont = theCaseData.Store();
	if(::IsHeaderRow(theCaseData))
	{
		if(theCaseData.TotalFileSize() <= 0)
			useBoldFont = false; // jos kyse on header-rivistä ja sen total koko on 0, ei laiteta riviä boldilla riippumatta mikä on Store-asetus
		else
			useBoldFont = true; // jos kyse on header-rivistä ja total size suupempi kuin 0, laitetaan boldiksi
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
				    itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) | GVIS_READONLY); // Laita read-only -bitti päälle
                else
				    itsGridCtrl.SetItemState(row, column, itsGridCtrl.GetItemState(row, column) & ~GVIS_READONLY); // Laita read-only -bitti pois päältä
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
				pTempCell->SetCheck(theCaseData.Store()); 
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

void CFmiCaseStudyDlg::InitGridControlValues(void)
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
	if(treePatternArray.size()) // pitää testata 0 koko vastaan, muuten voi kaatua
		itsTreeColumn.TreeSetup(&itsGridCtrl, 1, static_cast<int>(treePatternArray.size()), 1, &treePatternArray[0], TRUE, FALSE);
    fFirstTime = false;
}

void CFmiCaseStudyDlg::UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly)
{
	std::vector<NFmiCaseStudyDataFile*> &dataVector = itsSmartMetDocumentInterface->CaseStudySystem().CaseStudyDialogData();
	int currentRowCount = fixedRowCount;
	for(size_t i = 0; i < dataVector.size(); i++)
	{
        SetGridRow(currentRowCount++, *(dataVector[i]), fixedColumnCount, updateOnly);
	}
}

void CFmiCaseStudyDlg::UpdateGridControlValues(bool updateOnly)
{
    UpdateRows(1, 1, updateOnly);
	UpdateData(FALSE);
	Invalidate(FALSE);
}

BOOL CFmiCaseStudyDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;

//	return CDialog::OnEraseBkgnd(pDC);
}

void CFmiCaseStudyDlg::OnPaint()
{
	if(fGridControlInitialized)
	{
		CPaintDC dc(this); // device context for painting

	 // tämä on pika viritys, kun muuten Print (ja muiden ) -nappulan kohdalta jää kaista maalaamatta kun laitoin ikkunaan välkkymättömän päivityksen
		CBrush brush(RGB(240, 240, 240));
		CRect gridCtrlArea(CalcGridArea());
		CRect clientRect;
		GetClientRect(clientRect);
		clientRect.bottom = gridCtrlArea.top;
		dc.FillRect(&clientRect, &brush);
	}
}

void CFmiCaseStudyDlg::HandleCheckBoxClick(int col, int row)
{
	UpdateData(TRUE);
	CGridCellCheck *pCell = dynamic_cast<CGridCellCheck *>(itsGridCtrl.GetCell(row, col));
	if(pCell)
	{
		bool newState = pCell->GetCheck() == TRUE;
		int dataIndex = row - 1; // rivit alkavat 1:stä, mutta datat on vektorissa 0:sta alkaen
        auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
        std::vector<NFmiCaseStudyDataFile*> &dataVector = caseStudySystem.CaseStudyDialogData();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{
			NFmiCaseStudyDataFile &dataFile = *dataVector[dataIndex];
			if(dataFile.CategoryHeader())
			{
                caseStudySystem.CategoryStore(dataFile.Category(), newState); // tämä tekee myös tarvittavat updatet data rakenteille
			}
			else if(dataFile.ProducerHeader())
			{
                caseStudySystem.ProducerStore(dataFile.Category(), dataFile.Producer().GetIdent(), newState); // tämä tekee myös tarvittavat updatet data rakenteille
			}
			else
			{
				dataFile.Store(newState);
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
		int dataIndex = row - 1; // rivit alkavat 1:stä, mutta datat on vektorissa 0:sta alkaen
        auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
		std::vector<NFmiCaseStudyDataFile*> &dataVector = caseStudySystem.CaseStudyDialogData();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{
			NFmiCaseStudyDataFile &dataFile = *dataVector[dataIndex];
			if(dataFile.CategoryHeader())
			{
                caseStudySystem.CategoryEnable(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), dataFile.Category(), newState); // tämä tekee myös tarvittavat updatet data rakenteille
			}
			else if(dataFile.ProducerHeader())
			{
                caseStudySystem.ProducerEnable(*itsSmartMetDocumentInterface->HelpDataInfoSystem(), dataFile.Category(), dataFile.Producer().GetIdent(), newState); // tämä tekee myös tarvittavat updatet data rakenteille
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
			itsOffsetEditTimer = static_cast<UINT>(SetTimer(kFmiOffsetEditTimer, 50, NULL)); // käynnistetään timer, että saadaan pikkuisen viiveen jälkeen käynnistettyä offsetin muutoksesta aiheutuva päivitys
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
	case kFmiOffsetEditTimer:
		{
			KillTimer(itsOffsetEditTimer);
			OnOffsetEdited();
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

void CFmiCaseStudyDlg::OnOffsetEdited(void)
{
	if(itsGridCtrl.IsValid(itsOffsetEditedCell))
	{

		int dataIndex = itsOffsetEditedCell.row - 1; // rivit alkavat 1:stä, mutta datat on vektorissa 0:sta alkaen
        auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
        std::vector<NFmiCaseStudyDataFile*> &dataVector = caseStudySystem.CaseStudyDialogData();
		if(dataIndex >= 0 && dataIndex < static_cast<int>(dataVector.size()))
		{

			std::string str = CT2A(itsGridCtrl.GetCell(itsOffsetEditedCell.row, itsOffsetEditedCell.col)->GetText());
			int offsetInMinutes = 0;
			try
			{
				offsetInMinutes = boost::math::iround(NFmiStringTools::Convert<double>(str) * 60);
				bool startOffset = (itsOffsetEditedCell.col == CaseStudyHeaderParInfo::kStartTimeOffset);

				NFmiCaseStudyDataFile &dataFile = *dataVector[dataIndex];
				if(dataFile.CategoryHeader())
				{
                    caseStudySystem.CategoryOffset(dataFile.Category(), startOffset, offsetInMinutes); // tämä tekee myös tarvittavat updatet data rakenteille
				}
				else if(dataFile.ProducerHeader())
				{
                    caseStudySystem.ProducerOffset(dataFile.Category(), dataFile.Producer().GetIdent(), startOffset, offsetInMinutes); // tämä tekee myös tarvittavat updatet data rakenteille
				}
				else
				{ // normaali fileData-tapaus
					if(startOffset)
						dataFile.StartOffsetInMinutes(offsetInMinutes);
					else
						dataFile.EndOffsetInMinutes(offsetInMinutes);

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

bool CFmiCaseStudyDlg::IsThereCaseStudyMakerRunning(void)
{
	std::string dummyStr;
	return (NFmiApplicationDataBase::CountProcessCount(gCaseStudyMakerData, dummyStr) > 0);
}

void CFmiCaseStudyDlg::GetBasicInfoFromDialog(void)
{
	UpdateData(TRUE);
    auto &caseStudySystem = itsSmartMetDocumentInterface->CaseStudySystem();
    caseStudySystem.Name(std::string(CT2A(itsNameStrU_)));
    caseStudySystem.Info(std::string(CT2A(itsInfoStrU_)));

	// polun perässä ei saa olla minkäänsuuntaisia kenoviivoja. Datan kopioinnissa käytetty 
	// CShellFileOp -luokka on traumaattisen herkkä minkäänlaisille tai asteisille epämääräisyyksille.
	// nyt jos polussa on kenoviiva perässä, tulee siihen kaksi kenoviivaa ja tiedostojen kopioinnit epäonnistuvat.
	std::string strippedPathString = CT2A(itsPathStrU_);
	NFmiStringTools::TrimR(strippedPathString, '\\');
	NFmiStringTools::TrimR(strippedPathString, '/');
	NFmiStringTools::ReplaceChars(strippedPathString, '/', '\\'); // varmistetaan vielä että kaikki kenot ovat winkkarin omaan suuntaan.
    caseStudySystem.CaseStudyPath(strippedPathString);

    caseStudySystem.ZipFiles(fZipData == TRUE);
}

void CFmiCaseStudyDlg::OnBnClickedButtonStoreData()
{
    //	return; // Store-napin toiminta on väliaikaisesti disabloitu, että saan tehtyä 5.8.3.1-jakeluversion, joka korjaa kaatumistaudin 

    EnableButton(IDC_BUTTON_STORE_DATA, FALSE);
    UpdateData(TRUE);
    GetBasicInfoFromDialog();

    itsSmartMetDocumentInterface->StoreCaseStudyMemory(); // laitetaan CaseStudy-datan tilanne varmuudeksi talteen erilliseen 'muisti'-tiedostoon
    std::string dummyStr;
    if(IsThereCaseStudyMakerRunning())
    { // vain yksi CaseStudyMaker voi olla käynnisssä kerrallaan...
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
        commandStr += "\""; // laitetaan lainausmerkit komento polun ympärille, jos siinä sattuisi olemaan spaceja
        commandStr += itsSmartMetDocumentInterface->ApplicationDataBase().apppath;
        commandStr = NFmiStringTools::UrlDecode(commandStr); // valitettavasti tämä stringi on url-encodattu ja se pitää purkaa...
        commandStr += "\\";
        commandStr += gCaseStudyMakerData.first;
        commandStr += "\" \""; // laitetaan lainausmerkit metadatatiedoston polun ympärille, jos siinä sattuisi olemaan spaceja
        commandStr += metaDataTotalFileName;
        commandStr += "\""; // laitetaan lainausmerkit metadatatiedoston polun ympärille, jos siinä sattuisi olemaan spaceja
        if(fZipData)
        {
            commandStr += "?"; // tämä kysymysmerkki on erotin, jolla erotellaan metatiedosto ja zip-ohjelman polku+nimi
                            // En keksinyt tähän hätään muuta keinoa antaa winExelle erilaisia argumentteja yhdellä kertaa, joutuisin tekemään erillesen komentorivi 
                            // parserin, joka osaisi käsitellä erilaisia optioita (kuten SmartMetissa tehdään).
                            // '?' on erotin, koska sellaista kirjainta ei voi olla polussa kuin wild-card -merkkinä, ei varsinaisessa polussa tai tiedoston nimessä.
            std::string zipCommandStr = CFmiProcessHelpers::Make7zipExePath(itsSmartMetDocumentInterface->WorkingDirectory());
            commandStr += zipCommandStr;
        }

        CFmiProcessHelpers::ExecuteCommandInSeparateProcess(commandStr, true, true, SW_MINIMIZE);
    }
    UpdateButtonStates();
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

// Tätä kutsutaan aina tietyissä tilanteissa (initialisointi, Store- ja Close Mode- nappien handlauksen lopuksi).
// Tarkoitus on enabloida ja disabloida Store- ja Close Mode- nappeja.
// Store-nappi on disabloitu, jos SmartMet on CaseStudy-moodissa.
// Kun Store-nappia on painettu, mennään erikois tilaan, eli disabloidaan Store-nappi ja käynnistyy timer, 
// jonka avulla katsotaan onko CaseStudyExe-tallentamassa dataa, kun ei enää ole tätä ohjelmaa käynnissä, voidaan Store-nappi enabloida (jos ei olla CaseStudy-moodissa tietenkin).
// Close Mode-nappi on disabloitu, jos ei olla CaseStudy-moodissa.
void CFmiCaseStudyDlg::UpdateButtonStates(void)
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
			itsDisableStoreButtonTimer = static_cast<UINT>(SetTimer(kFmiDisableStoreButtonTimer, 2000, NULL)); // käynnistetään timer, joka kahden sekunnin välein tarkistaa, voidaanko Store-nappi taas enabloida
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

void CFmiCaseStudyDlg::ShowEnableColumn(void)
{
    static int lastTrueEnableColumnWidth = 0;

	UpdateData(TRUE);

    for(int i = 0; i < itsGridCtrl.GetRowCount(); i++)
    {
        if(fEditEnableData)
            itsGridCtrl.SetColumnWidth(CaseStudyHeaderParInfo::kEnableData, lastTrueEnableColumnWidth); // otetaan talessa ollut viimeinen leveys käyttöön kun sarake näytetään taas
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
    caseStudySystem.FillCaseStudyDialogData(*itsProducerSystemsHolder);
    InitGridControlValues();
}
