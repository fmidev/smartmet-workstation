// FmiWarningCenterDlg.cpp : implementation file
//
#ifdef _MSC_VER
#pragma warning(disable : 4996) // poistaa varoituksen "warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead"
#endif

#include "stdafx.h"
#include "FmiWarningCenterDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWarningMessageOptionsDlg.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "FmiWin32TemplateHelpers.h"
#include "NFmiMapViewDescTop.h"
#include "CloneBitmap.h"
#include "FmiWin32Helpers.h"
#include "NFmiApplicationWinRegistry.h"
#include "HakeMessage/Main.h"
#include "HakeMessage/HakeSystemConfigurations.h"
#include "HakeMessage/HakeMsg.h"
#include "persist2.h"
#include "ApplicationInterface.h"

static const COLORREF gFixedBkColor = RGB(239, 235, 222);

BEGIN_MESSAGE_MAP(NFmiWarningCenterGridCtrl, CGridCtrl)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void NFmiWarningCenterGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnRButtonUp(nFlags, point);

	// Tarkoitus on deselectoida kaikki
	CCellID focusCell;
    focusCell = GetCellFromPt(point);
	if(focusCell == itsLastSortedCell)
	{
		fLastSortedExist = false;
		itsLastSortedCell = CCellID();
	}

	if(focusCell.row < m_nFixedRows)
	{ // jos oltiin klikattu otsikko rivi‰, l‰hetet‰‰n viesti emolle, ett‰ se osaa p‰ivitt‰‰ taulukon uudestaan
		fUpdateParent = true;
		SendMessageToParent(focusCell.row, focusCell.col, GVN_COLUMNCLICK);
	}
}

void NFmiWarningCenterGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnLButtonUp(nFlags, point);
	CCellID idCurrentCell = GetCellFromPt(point);
}

// Huom. numerot ovat teksti muodossa ja missing value on '-'-merkki.
// Haluan ett‰ puuttuvat ovat aina h‰nnill‰, joten pit‰‰ kikkailla.
int CALLBACK NFmiWarningCenterGridCtrl::pfnCellTextCaseInSensitiveCompare(LPARAM lParam1,
																LPARAM lParam2,
																LPARAM lParamSort)
{
	UNUSED_ALWAYS(lParamSort);

	CGridCellBase* pCell1 = (CGridCellBase*) lParam1;
	CGridCellBase* pCell2 = (CGridCellBase*) lParam2;
	if (!pCell1 || !pCell2) return 0;

    return _tcsnicmp(pCell1->GetText(), pCell2->GetText(), 50); // 50 on hatusta, koska en tied‰ miten saan LPCTSTR-otukselta kysytty‰ stringin pituutta
}

// piti laittaa columnin headerista sorttaus uusiksi
void NFmiWarningCenterGridCtrl::OnFixedRowClick(CCellID& cell)
{
// **** TƒSTƒ loppuun kopioitu suoraan emosta koodia PAITSI kun SortItems-metodille annetaan kolmas parametri
    if (!IsValid(cell))
        return;

    if (GetHeaderSort())
    {
		itsLastSortedCell = cell;
		fLastSortedAscending = GetSortAscending();
		fLastSortedExist = true;

        WaitCursorHelper waitCursorHelper(itsSmartMetDocumentInterface->ShowWaitCursorWhileDrawingView());
        if (cell.col == GetSortColumn())
            SortItems(cell.col, !GetSortAscending(), !GetSortAscending()); // *** KOLMAS parametri annettu t‰ss‰ koodissa ****
        else
            SortItems(cell.col, TRUE, TRUE); // *** KOLMAS parametri annettu t‰ss‰ koodissa ********
        Invalidate();
    }

	// Did the user click on a fixed column cell (so the cell was within the overlap of
	// fixed row and column cells) - (fix by David Pritchard)
    if (fEnableFixedColumnSelection && GetFixedColumnSelection())
    {
        if (cell.col < GetFixedColumnCount())
        {
            m_MouseMode = MOUSE_SELECT_ALL;
            OnSelecting(cell);
        }
        else
        {
            m_MouseMode = MOUSE_SELECT_COL;
            OnSelecting(cell);
        }
    }
}

void NFmiWarningCenterGridCtrl::DoLastSort(void)
{
	if(this->fLastSortedExist)
	{
		this->SetSortAscending(this->fLastSortedAscending);
		this->SetSortColumn(itsLastSortedCell.col);
		fEnableFixedColumnSelection = false; // estet‰‰n hetkeksi FixedColumnSelection toiminto, n‰in ei mene valitut ruudut sekaisen
		OnFixedRowClick(this->itsLastSortedCell);
		fEnableFixedColumnSelection = true; // laitetaan se taas p‰‰lle
	}
}

// NFmiGridCtrl luokka macro + alustukset
IMPLEMENT_DYNCREATE(NFmiWarningCenterGridCtrl, CGridCtrl)


// CFmiWarningCenterDlg dialog

const NFmiViewPosRegistryInfo CFmiWarningCenterDlg::s_ViewPosRegistryInfo(CRect(300, 200, 800, 900), "\\WarningCenterView");

IMPLEMENT_DYNAMIC(CFmiWarningCenterDlg, CDialog)
CFmiWarningCenterDlg::CFmiWarningCenterDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
:CDialog(CFmiWarningCenterDlg::IDD, pParent)
,itsGridCtrl()
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,fGridCtrlInitialized(false)
, itsTimeStrU_()
,itsTimeAndStationTextFont()
,itsMapViewDescTopIndex(0)
,itsHeaders()
,fShowAllMessages(FALSE)
, fShowHakeMessages(FALSE)
, fShowKaHaMessages(FALSE)
, itsMinimumTimeRangeForWarningsOnMapViewsInMinutes(0)
{
}

CFmiWarningCenterDlg::~CFmiWarningCenterDlg()
{
}

void CFmiWarningCenterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_GridControl(pDX, IDC_CUSTOM_GRID_CTRL, itsGridCtrl);
    DDX_Text(pDX, IDC_STATIC_TIME_STR, itsTimeStrU_);
	DDX_Check(pDX, IDC_CHECK_SHOW_ALL_MESSAGES, fShowAllMessages);
    DDX_Check(pDX, IDC_CHECK_SHOW_HAKE_MESSAGES, fShowHakeMessages);
    DDX_Check(pDX, IDC_CHECK_SHOW_KAHA_MESSAGES, fShowKaHaMessages);
    DDX_Text(pDX, IDC_EDIT_MINIMUM_TIME_STEP_IN_MINUTES, itsMinimumTimeRangeForWarningsOnMapViewsInMinutes);
}


BEGIN_MESSAGE_MAP(CFmiWarningCenterDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_PRINT, OnBnClickedButtonPrint)
	ON_BN_CLICKED(IDC_BUTTON_PREVIOUS_TIME, OnBnClickedButtonPreviousTime)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_TIME, OnBnClickedButtonNextTime)
	ON_BN_CLICKED(IDC_BUTTON_WARNING_CENTER_OPTIONS, OnBnClickedButtonWarningCenterOptions)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ALL_MESSAGES, OnBnClickedCheckShowAllMessages)
	ON_BN_CLICKED(IDC_BUTTON_WARNING_CENTER_REFRESH_MESSAGES, OnBnClickedButtonWarningCenterRefreshMessages)
    ON_BN_CLICKED(IDC_CHECK_SHOW_HAKE_MESSAGES, &CFmiWarningCenterDlg::OnBnClickedCheckShowHakeMessages)
    ON_BN_CLICKED(IDC_CHECK_SHOW_KAHA_MESSAGES, &CFmiWarningCenterDlg::OnBnClickedCheckShowKahaMessages)
    ON_EN_CHANGE(IDC_EDIT_MINIMUM_TIME_STEP_IN_MINUTES, &CFmiWarningCenterDlg::OnEnChangeEditMinimumTimeStepInMinutes)
END_MESSAGE_MAP()


// CFmiWarningCenterDlg message handlers

BOOL CFmiWarningCenterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
    std::string titleStr = ::GetDictionaryString("WarningCenterDlgTitle").c_str();
    SetWindowText(CA2T(titleStr.c_str()));

    auto &applicationWinRegistry = itsSmartMetDocumentInterface->ApplicationWinRegistry();
    std::string errorBaseStr("Error in CFmiWarningCenterDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(applicationWinRegistry, this, false, errorBaseStr, 0);
#ifndef DISABLE_CPPRESTSDK
    fShowAllMessages = itsSmartMetDocumentInterface->WarningCenterSystem().getLegacyData().ShowAllMessages();
#endif // DISABLE_CPPRESTSDK
    fShowHakeMessages = applicationWinRegistry.ShowHakeMessages();
    fShowKaHaMessages = applicationWinRegistry.ShowKaHaMessages();
    itsMinimumTimeRangeForWarningsOnMapViewsInMinutes = applicationWinRegistry.MinimumTimeRangeForWarningsOnMapViewsInMinutes();
    itsGridCtrl.SetDocument(itsSmartMetDocumentInterface);
	itsGridCtrl.MoveWindow(CalcClientArea(), FALSE);
	fGridCtrlInitialized = true;
	InitHeaders();
	Update();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiWarningCenterDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

void CFmiWarningCenterDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiWarningCenterDlg::OnClose()
{
	DoWhenClosing();
	CDialog::OnClose();
}

void CFmiWarningCenterDlg::DoWhenClosing(void)
{
#ifndef DISABLE_CPPRESTSDK
    itsSmartMetDocumentInterface->WarningCenterSystem().getLegacyData().WarningCenterViewOn(false);
#endif // DISABLE_CPPRESTSDK
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, true, true);
}

BOOL CFmiWarningCenterDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
//	return CDialog::OnEraseBkgnd(pDC);
}

void CFmiWarningCenterDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

 // t‰m‰ on pika viritys, kun muuten Print (ja muiden ) -nappulan kohdalta j‰‰ kaista maalaamatta kun laitoin ikkunaan v‰lkkym‰ttˆm‰n p‰ivityksen
	CBrush brush(RGB(239, 235, 222));
	CRect area(CalcOtherArea());
	dc.FillRect(&area, &brush);
}

CRect CFmiWarningCenterDlg::CalcOtherArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CRect gridRect(CalcClientArea());
	rect.bottom = gridRect.top + 0;
	return rect;
}

CRect CFmiWarningCenterDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
    // Kuinka paljon grid kontrollin yl‰osan pit‰‰ siirt‰‰ alasp‰in
    int gridControlTopPositionOffset = 40;
    CWnd *staticControl = GetDlgItem(IDC_STATIC_TIME_STR);
    if(staticControl)
    {
        CRect staticRect;
        staticControl->GetWindowRect(staticRect);
        ScreenToClient(staticRect);
        gridControlTopPositionOffset = staticRect.bottom;
    }
	rect.top = rect.top + gridControlTopPositionOffset + 1;
	return rect;
}

void CFmiWarningCenterDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(fGridCtrlInitialized)
		itsGridCtrl.MoveWindow(CalcClientArea(), FALSE);
	Invalidate(FALSE);
}

void CFmiWarningCenterDlg::OnBnClickedButtonPrint()
{
	itsGridCtrl.Print(); // t‰h‰n tulee defaulttina footeria ja headeria ja "ei wysiwygi‰", mik‰ on ehk‰ parempi
}

void CFmiWarningCenterDlg::OnBnClickedButtonPreviousTime()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->SetDataToPreviousTime(itsMapViewDescTopIndex);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__);
}

void CFmiWarningCenterDlg::OnBnClickedButtonNextTime()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->SetDataToNextTime(itsMapViewDescTopIndex);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__);
}

void CFmiWarningCenterDlg::OnBnClickedButtonWarningCenterOptions()
{
#ifndef DISABLE_CPPRESTSDK
    auto &warningCenterSystem = itsSmartMetDocumentInterface->WarningCenterSystem();
    CFmiWarningMessageOptionsDlg dlg(&warningCenterSystem, itsSmartMetDocumentInterface->ApplicationWinRegistry(), this);
	if(dlg.DoModal() == IDOK)
	{
        warningCenterSystem.getLegacyData().StoreSettings();
        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, true, true);
	}
#endif // DISABLE_CPPRESTSDK
}

void CFmiWarningCenterDlg::InitHeaders(void)
{
	int basicColumnWidthUnit = 18;
	itsHeaders.clear();
	itsHeaders.push_back(WarningCenterHeaderParInfo("Nr", WarningCenterHeaderParInfo::kRowNumber, static_cast<int>(basicColumnWidthUnit*2.5)));
	itsHeaders.push_back(WarningCenterHeaderParInfo("Time (UTC)", WarningCenterHeaderParInfo::kStartTime, basicColumnWidthUnit*8));
	itsHeaders.push_back(WarningCenterHeaderParInfo("Center", WarningCenterHeaderParInfo::kCenterId, basicColumnWidthUnit*3));
	itsHeaders.push_back(WarningCenterHeaderParInfo("MsgNum", WarningCenterHeaderParInfo::kMessageNumber, basicColumnWidthUnit*3));
	itsHeaders.push_back(WarningCenterHeaderParInfo("Type", WarningCenterHeaderParInfo::kMessageType, basicColumnWidthUnit*3));
	itsHeaders.push_back(WarningCenterHeaderParInfo("County", WarningCenterHeaderParInfo::kCountyName, basicColumnWidthUnit*5));
	itsHeaders.push_back(WarningCenterHeaderParInfo("Address", WarningCenterHeaderParInfo::kStreetName, basicColumnWidthUnit*5));
	itsHeaders.push_back(WarningCenterHeaderParInfo("Message", WarningCenterHeaderParInfo::kMessage, basicColumnWidthUnit*8));
	itsHeaders.push_back(WarningCenterHeaderParInfo("Reason", WarningCenterHeaderParInfo::kReason, basicColumnWidthUnit*8));
	itsHeaders.push_back(WarningCenterHeaderParInfo("Level", WarningCenterHeaderParInfo::kMessageLevel, basicColumnWidthUnit*2));
	itsHeaders.push_back(WarningCenterHeaderParInfo("Lon", WarningCenterHeaderParInfo::kLon, basicColumnWidthUnit*3));
	itsHeaders.push_back(WarningCenterHeaderParInfo("Lat", WarningCenterHeaderParInfo::kLat, basicColumnWidthUnit*3));
}

static std::string GetTimeAndStationString(SmartMetDocumentInterface *smartMetDocumentInterface, const NFmiMetTime &theTime, int theMessageCount, int theMapViewDescTopIndex)
{
	std::string str;
	str += "Warning messages was between ";
	str += " (-";
    double timeStepForMessagesInHours = smartMetDocumentInterface->GetTimeRangeForWarningMessagesOnMapViewInMinutes() / 60.;
    str += NFmiStringTools::Convert(timeStepForMessagesInHours);
	str += " hrs) - ";
	str += static_cast<char*>(theTime.ToStr(::GetDictionaryString("StationDataTableViewTimeFormat"), smartMetDocumentInterface->Language()));
	str += " ";
	str += NFmiStringTools::Convert<int>(theMessageCount).c_str();
	str += " ";
	str += ::GetDictionaryString("StationDataTableViewCount");
	return str;
}

static void SetHeaders(NFmiWarningCenterGridCtrl &theGridCtrl, const std::vector<WarningCenterHeaderParInfo> &theHeaders, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount)
{
	int columnCount = static_cast<int>(theHeaders.size());
	theGridCtrl.SetRowCount(rowCount);
	theGridCtrl.SetColumnCount(columnCount);
	theGridCtrl.SetGridLines(GVL_BOTH);
	theGridCtrl.SetFixedRowCount(theFixedRowCount);
	theGridCtrl.SetFixedColumnCount(theFixedColumnCount);
	theGridCtrl.SetListMode(TRUE);
	theGridCtrl.SetHeaderSort(TRUE);
	theGridCtrl.SetFixedBkColor(gFixedBkColor);

	int currentRow = 0;
	// 1. on otsikko rivi on parametrien nimi‰ varten
	for(int i=0; i<columnCount; i++)
	{
		theGridCtrl.SetItemText(currentRow, i, CA2T(theHeaders[i].itsHeader.c_str()));
		theGridCtrl.SetItemState(currentRow, i, theGridCtrl.GetItemState(currentRow, i) | GVIS_READONLY);
		if(fFirstTime) // s‰‰det‰‰n sarakkeiden leveydet vain 1. kerran
			theGridCtrl.SetColumnWidth(i, theHeaders[i].itsColumnWidth);
	}
	fFirstTime = false;
}

int CFmiWarningCenterDlg::CountShownMessages(void)
{
	return static_cast<int>(itsShownHakeMessages.size() + itsShownKaHaMessages.size());
}

void CFmiWarningCenterDlg::GetShownMessages()
{
#ifndef DISABLE_CPPRESTSDK
    itsShownHakeMessages.clear();
    itsShownKaHaMessages.clear();

    boost::shared_ptr<NFmiArea> zoomedArea = itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapHandler()->Area();
    int timeStepForMessagesInMinutes = itsSmartMetDocumentInterface->GetTimeRangeForWarningMessagesOnMapViewInMinutes();
    NFmiMetTime mapTime = itsSmartMetDocumentInterface->CurrentTime(itsMapViewDescTopIndex);
    NFmiMetTime startTime = HakeLegacySupport::HakeSystemConfigurations::MakeStartTimeForHakeMessages(mapTime, timeStepForMessagesInMinutes);
    auto &warningCenterSystem = itsSmartMetDocumentInterface->WarningCenterSystem();
    if(fShowHakeMessages)
        itsShownHakeMessages = warningCenterSystem.getHakeMessages(startTime, mapTime, *zoomedArea);
    if(fShowKaHaMessages)
        itsShownKaHaMessages = warningCenterSystem.getKahaMessages(startTime, mapTime, *zoomedArea);
#endif // DISABLE_CPPRESTSDK
}

void CFmiWarningCenterDlg::Update(void)
{
    static bool fFirstTime = true; // sarakkeiden s‰‰tˆ tehd‰‰n vain 1. kerran

    if(IsWindowVisible() && !IsIconic()) // N‰yttˆ‰ p‰ivitet‰‰n vain jos se on n‰kyviss‰ ja se ei ole minimized tilassa
    {
        NFmiMetTime wantedTime(itsSmartMetDocumentInterface->CurrentTime(itsMapViewDescTopIndex));
        int fixedRowCount = 1;
        int fixedColumnCount = 1;

        GetShownMessages();
        // LASKE kuinka monta HALY messagea on nykyisess‰ zoomatussa alueessa
        int messageCount = CountShownMessages();
        itsGridCtrl.LastMessageCount(messageCount);

        itsTimeStrU_ = CA2T(::GetTimeAndStationString(itsSmartMetDocumentInterface, wantedTime, messageCount, itsMapViewDescTopIndex).c_str());
        // pistet‰‰n aika ja asema lkm tiedot talteen, jos printataan t‰m‰ n‰yttˆ
        itsGridCtrl.LastDataTime(wantedTime);

        itsGridCtrl.SetUsedHeaders(&itsHeaders);
        if(messageCount)
            FillGridWithWarningMessages(fFirstTime, fixedRowCount, fixedColumnCount, wantedTime, messageCount);
        else
            SetHeaders(itsGridCtrl, itsHeaders, fixedRowCount, fFirstTime, fixedRowCount, fixedColumnCount);
        

        itsGridCtrl.DoLastSort(); // jos ollaan tehty aiemmin jokin sorttaus, se halutaan tehd‰ nyt t‰ss‰ uudestaan
        UpdateData(FALSE);
    }
}

static const char * Value2String(double value, int wantedDecimalCount)
{
	static char buffer[128]="";
	static char buffer2[128]="";
	// HUOM! n‰m‰ on syyt‰ tehd‰ _snprintf-funktioilla, koska tehokasta varsinkin MSVC k‰‰nt‰jill‰
	// NFmiStringTools-Convert on liian hidas, olen kokeillut. T‰t‰ funktiota saatetaan kutsua yhden 
	// ruudun p‰ivityksen aikana kymmeni‰tuhansia kertoja
	::_snprintf(buffer, sizeof(buffer)-1, "%%0.%df", wantedDecimalCount);
	buffer[sizeof(buffer)-1] = 0; // pit‰‰ varmistaa ett‰ p‰‰ttyy 0-merkkiin!!!!
	::_snprintf(buffer2, sizeof(buffer2)-1, buffer, value);
	buffer2[sizeof(buffer2)-1] = 0; // pit‰‰ varmistaa ett‰ p‰‰ttyy 0-merkkiin!!!!
	return buffer2;
}

static void SetGridCell(NFmiWarningCenterGridCtrl &theGridCtrl, int row, int column, const char *str)
{ 
    // t‰h‰n tulee j‰rjestys numero fixed columniin
    auto wideStr = ::convertPossibleUtf8StringToWideString(str);
	theGridCtrl.SetItemText(row, column, wideStr.c_str());
	theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
}

static const COLORREF gHakeMessageBkColor = RGB(255, 255, 255);
static const COLORREF gKaHaMessageBkColor = RGB(245, 245, 255);

static void SetGridRow(FmiLanguage language, const HakeMessage::HakeMsg &theMessage, NFmiWarningCenterGridCtrl &theGridCtrl, const WarningCenterHeaderParInfo &theHeaderParInfo, int row, int column, const NFmiPoint &theLoc, const NFmiMetTime & /* theTime */, bool isHakeMessage)
{
    switch(theHeaderParInfo.itsColumnFunction)
    {
    case WarningCenterHeaderParInfo::kRowNumber:
        ::SetGridCell(theGridCtrl, row, column, ::Value2String(row, 0));
        break;
    case WarningCenterHeaderParInfo::kStartTime:
        ::SetGridCell(theGridCtrl, row, column, static_cast<char*>(theMessage.StartTime().ToStr(::GetDictionaryString("StationDataTableViewTimeFormat"), language)));
        break;
    case WarningCenterHeaderParInfo::kReportTime:
        ::SetGridCell(theGridCtrl, row, column, static_cast<char*>(theMessage.SendingTime().ToStr(::GetDictionaryString("StationDataTableViewTimeFormat"), language)));
        break;
    case WarningCenterHeaderParInfo::kCenterId:
        ::SetGridCell(theGridCtrl, row, column, theMessage.MessageCenterId().c_str());
        break;
    case WarningCenterHeaderParInfo::kMessageType:
        ::SetGridCell(theGridCtrl, row, column, ::Value2String(theMessage.Category(), 0));
        break;
    case WarningCenterHeaderParInfo::kMessageLevel:
        ::SetGridCell(theGridCtrl, row, column, ::Value2String(theMessage.MessageLevel(), 0));
        break;
    case WarningCenterHeaderParInfo::kMessage:
        ::SetGridCell(theGridCtrl, row, column, theMessage.MessageStr().c_str());
        break;
    case WarningCenterHeaderParInfo::kReason:
        ::SetGridCell(theGridCtrl, row, column, theMessage.ReasonStr().c_str());
        break;
    case WarningCenterHeaderParInfo::kCountyName:
        ::SetGridCell(theGridCtrl, row, column, theMessage.CountyStr().c_str());
        break;
    case WarningCenterHeaderParInfo::kStreetName:
        ::SetGridCell(theGridCtrl, row, column, theMessage.Address().c_str());
        break;
    case WarningCenterHeaderParInfo::kMessageNumber:
        ::SetGridCell(theGridCtrl, row, column, theMessage.Number().c_str());
        break;
    case WarningCenterHeaderParInfo::kAdditionalInfo:
        ::SetGridCell(theGridCtrl, row, column, theMessage.MessageStr().c_str());
        break;
    case WarningCenterHeaderParInfo::kLon:
        ::SetGridCell(theGridCtrl, row, column, ::Value2String(theMessage.LatlonPoint().X(), 3));
        break;
    case WarningCenterHeaderParInfo::kLat:
        ::SetGridCell(theGridCtrl, row, column, ::Value2String(theMessage.LatlonPoint().Y(), 3));
        break;
    }

    // Set backgroung colors following:
    // if isHakeMessage == true  => bkcolor white
    // else  => bkcolor light blue
    if(column != 0) // column 0 on s‰‰detty fixediksi, ja sille m‰‰r‰t‰‰n v‰ritys toisaalla
    {
        if(isHakeMessage)
            theGridCtrl.SetItemBkColour(row, column, gHakeMessageBkColor);
        else
            theGridCtrl.SetItemBkColour(row, column, gKaHaMessageBkColor);
    }
}

void CFmiWarningCenterDlg::FillGridWithWarningMessages(const std::vector<HakeMessage::HakeMsg> &messages, bool isHakeMessage, bool &fFirstTime, const NFmiMetTime &theTime, int &currentRowCounter)
{
    int columnCount = static_cast<int>(itsHeaders.size());
    for(const auto &message : messages)
    {
        for(int i = 0; i < columnCount; i++)
        {
            SetGridRow(itsSmartMetDocumentInterface->Language(), message, itsGridCtrl, itsHeaders[i], currentRowCounter, i, message.LatlonPoint(), theTime, isHakeMessage);
        }
        currentRowCounter++;
    }
}


void CFmiWarningCenterDlg::FillGridWithWarningMessages(bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, const NFmiMetTime &theTime, int totalMessageCount)
{
	SetHeaders(itsGridCtrl, itsHeaders, totalMessageCount + theFixedRowCount, fFirstTime, theFixedRowCount, theFixedColumnCount);

    int currentRowCounter = theFixedRowCount;
    FillGridWithWarningMessages(itsShownHakeMessages, true, fFirstTime, theTime, currentRowCounter);
    FillGridWithWarningMessages(itsShownKaHaMessages, false, fFirstTime, theTime, currentRowCounter);
}

void CFmiWarningCenterDlg::OnBnClickedCheckShowAllMessages()
{
#ifndef DISABLE_CPPRESTSDK
    UpdateData(TRUE);
    itsSmartMetDocumentInterface->WarningCenterSystem().getLegacyData().ShowAllMessages(fShowAllMessages == TRUE);
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, true, true);
#endif // DISABLE_CPPRESTSDK
}

void CFmiWarningCenterDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiWarningCenterDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiWarningCenterDlg::OnBnClickedButtonWarningCenterRefreshMessages()
{
/*
	itsSmartMetDocumentInterface->WarningCenterSystem().Clear();
	itsSmartMetDocumentInterface->WarningCenterSystem().CheckForNewMessages();
	itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(true, true);
*/
}

BOOL CFmiWarningCenterDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// HUOM!!! Yritin l‰hett‰‰ viestiss‰ komentoa, mutta jostain syyst‰ pResult on on nollautunut jotenkin matkalla
	// Jotain h‰ikk‰ viestin v‰lityksess‰ GridControllista emolle. Sen takia tein UpdateParent-virityksen

	// jos oltiin klikattu otsikko rivi‰, l‰hetet‰‰n viesti emolle, ett‰ se osaa p‰ivitt‰‰ taulukon uudestaan
	if(itsGridCtrl.UpdateParent())
		Update();
	itsGridCtrl.UpdateParent(false);

	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CFmiWarningCenterDlg::OnBnClickedCheckShowHakeMessages()
{
    UpdateData(TRUE);
    itsSmartMetDocumentInterface->ApplicationWinRegistry().ShowHakeMessages(fShowHakeMessages == TRUE);
    Update();
    ForceMainMapViewUpdate(__FUNCTION__);
}


void CFmiWarningCenterDlg::OnBnClickedCheckShowKahaMessages()
{
    UpdateData(TRUE);
    itsSmartMetDocumentInterface->ApplicationWinRegistry().ShowKaHaMessages(fShowKaHaMessages == TRUE);
    Update();
    ForceMainMapViewUpdate(__FUNCTION__);
}

void CFmiWarningCenterDlg::ForceMainMapViewUpdate(const std::string &reasonForUpdate)
{
    // Make main map view dirty and force update on it
    itsSmartMetDocumentInterface->MapViewDirty(0, false, true, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(reasonForUpdate);
}

void CFmiWarningCenterDlg::LoadViewMacroSettingsFromDocument()
{
    auto &applicationWinRegistry = itsSmartMetDocumentInterface->ApplicationWinRegistry();
    fShowHakeMessages = applicationWinRegistry.ShowHakeMessages();
    fShowKaHaMessages = applicationWinRegistry.ShowKaHaMessages();
    itsMinimumTimeRangeForWarningsOnMapViewsInMinutes = applicationWinRegistry.MinimumTimeRangeForWarningsOnMapViewsInMinutes();

    UpdateData(FALSE);
}


void CFmiWarningCenterDlg::OnEnChangeEditMinimumTimeStepInMinutes()
{
    UpdateData(TRUE);
    auto &applicationWinRegistry = itsSmartMetDocumentInterface->ApplicationWinRegistry();
    applicationWinRegistry.MinimumTimeRangeForWarningsOnMapViewsInMinutes(itsMinimumTimeRangeForWarningsOnMapViewsInMinutes);
    Update();
    ForceMainMapViewUpdate(__FUNCTION__);
}
