// FmiSeaIcingWarningsDlg.cpp : implementation file
//

#ifdef _MSC_VER
#pragma warning(disable : 4996) // poistaa varoituksen "warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead"
#endif

#include "stdafx.h"
#include "FmiSeaIcingWarningsDlg.h"
#include "FmiWin32TemplateHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiSeaIcingWarningSystem.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiDictionaryFunction.h"
#include "CloneBitmap.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "persist2.h"
#include "ApplicationInterface.h"

static const COLORREF gFixedBkColor = RGB(239, 235, 222);

// **************************************************
// NFmiSeaIcingWarningsGridCtrl dialog
// **************************************************

BEGIN_MESSAGE_MAP(NFmiSeaIcingWarningsGridCtrl, CGridCtrl)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void NFmiSeaIcingWarningsGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
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
}

void NFmiSeaIcingWarningsGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnLButtonUp(nFlags, point);
	CCellID idCurrentCell = GetCellFromPt(point);
//	DoMapHighLightThings(idCurrentCell);
}

// Huom. numerot ovat teksti muodossa ja missing value on '-'-merkki.
// Haluan ett‰ puuttuvat ovat aina h‰nnill‰, joten pit‰‰ kikkailla.
int CALLBACK NFmiSeaIcingWarningsGridCtrl::pfnCellTextCaseInSensitiveCompare(LPARAM lParam1,
																LPARAM lParam2,
																LPARAM lParamSort)
{
	UNUSED_ALWAYS(lParamSort);

	CGridCellBase* pCell1 = (CGridCellBase*) lParam1;
	CGridCellBase* pCell2 = (CGridCellBase*) lParam2;
	if (!pCell1 || !pCell2) return 0;

    return _tcsnicmp(pCell1->GetText(), pCell2->GetText(), 50); // 50 on hatusta, koska en tied‰ miten saan LPCTSTR-otukselta kysytty‰ stringin pituutta
}

// NFmiGridCtrl luokka macro + alustukset
IMPLEMENT_DYNCREATE(NFmiSeaIcingWarningsGridCtrl, CGridCtrl)


// **************************************************
// CFmiSeaIcingWarningsDlg dialog
// **************************************************

const NFmiViewPosRegistryInfo CFmiSeaIcingWarningsDlg::s_ViewPosRegistryInfo(CRect(200, 100, 700, 800), "\\SeaIcingWarningView");

IMPLEMENT_DYNAMIC(CFmiSeaIcingWarningsDlg, CDialog)
CFmiSeaIcingWarningsDlg::CFmiSeaIcingWarningsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiSeaIcingWarningsDlg::IDD, pParent)
,itsGridCtrl()
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,fGridCtrlInitialized(false)
, itsTimeStrU_()
,itsTimeAndStationTextFont()
,itsMapViewDescTopIndex(0)
,itsHeaders()
{
}

CFmiSeaIcingWarningsDlg::~CFmiSeaIcingWarningsDlg()
{
}

void CFmiSeaIcingWarningsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_GridControl(pDX, IDC_CUSTOM_GRID_CTRL, itsGridCtrl);
    DDX_Text(pDX, IDC_STATIC_TIME_STR, itsTimeStrU_);
}


BEGIN_MESSAGE_MAP(CFmiSeaIcingWarningsDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_PRINT, OnBnClickedButtonPrint)
	ON_BN_CLICKED(IDC_BUTTON_PREVIOUS_TIME, OnBnClickedButtonPreviousTime)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_TIME, OnBnClickedButtonNextTime)
	ON_BN_CLICKED(IDC_BUTTON_WARNING_CENTER_OPTIONS, OnBnClickedButtonWarningCenterOptions)
	ON_BN_CLICKED(IDC_BUTTON_WARNING_CENTER_REFRESH_MESSAGES, OnBnClickedButtonWarningCenterRefreshMessages)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ALL_MESSAGES, OnBnClickedCheckShowAllMessages)
END_MESSAGE_MAP()


// CFmiSeaIcingWarningsDlg message handlers

BOOL CFmiSeaIcingWarningsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

    std::string titleStr = ::GetDictionaryString("SeaIcingWarningsDlgTitle").c_str();
    SetWindowText(CA2T(titleStr.c_str()));

	std::string errorBaseStr("Error in CFmiSeaIcingWarningsDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
	itsGridCtrl.SetDocument(itsSmartMetDocumentInterface);
	itsGridCtrl.MoveWindow(CalcClientArea(), FALSE);
	fGridCtrlInitialized = true;
	InitHeaders();
	Update();

	// disabloidaan optio-nappi toistaiseksi
	CWnd *aButton = GetDlgItem(IDC_BUTTON_WARNING_CENTER_OPTIONS);
	if(aButton)
		aButton->EnableWindow(FALSE);
	// disabloidaan N‰yt‰ kaikki -nappi, koska ei ole erilaisia varoituksia, mit‰ filtterˆid‰
	aButton = GetDlgItem(IDC_CHECK_SHOW_ALL_MESSAGES);
	if(aButton)
		aButton->EnableWindow(FALSE);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiSeaIcingWarningsDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

void CFmiSeaIcingWarningsDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiSeaIcingWarningsDlg::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose();
}

void CFmiSeaIcingWarningsDlg::DoWhenClosing(void)
{
	itsSmartMetDocumentInterface->SeaIcingWarningSystem().ViewVisible(false);
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
	itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, true, true);
}

void CFmiSeaIcingWarningsDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(fGridCtrlInitialized)
		itsGridCtrl.MoveWindow(CalcClientArea(), FALSE);
	Invalidate(FALSE);
}

BOOL CFmiSeaIcingWarningsDlg::OnEraseBkgnd(CDC * /* pDC */ )
{
	return FALSE;
//	return CDialog::OnEraseBkgnd(pDC);
}

void CFmiSeaIcingWarningsDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

 // t‰m‰ on pika viritys, kun muuten Print (ja muiden ) -nappulan kohdalta j‰‰ kaista maalaamatta kun laitoin ikkunaan v‰lkkym‰ttˆm‰n p‰ivityksen
	CBrush brush(RGB(239, 235, 222));
	CRect area(CalcOtherArea());
	dc.FillRect(&area, &brush);
}

CRect CFmiSeaIcingWarningsDlg::CalcOtherArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CRect gridRect(CalcClientArea());
	rect.bottom = gridRect.top + 0;
	return rect;
}

CRect CFmiSeaIcingWarningsDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
	rect.top = rect.top + 40;
	return rect;
}

void CFmiSeaIcingWarningsDlg::OnBnClickedButtonPrint()
{
	itsGridCtrl.Print(); // t‰h‰n tulee defaulttina footeria ja headeria ja "ei wysiwygi‰", mik‰ on ehk‰ parempi
}

void CFmiSeaIcingWarningsDlg::OnBnClickedButtonPreviousTime()
{
	UpdateData(TRUE);
	itsSmartMetDocumentInterface->SetDataToPreviousTime(itsMapViewDescTopIndex);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__);
}

void CFmiSeaIcingWarningsDlg::OnBnClickedButtonNextTime()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->SetDataToNextTime(itsMapViewDescTopIndex);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__);
}

void CFmiSeaIcingWarningsDlg::OnBnClickedButtonWarningCenterOptions()
{
	// ei ainakaan viel‰ s‰‰tˆdialogia t‰lle. S‰‰dˆt teht‰v‰ suoraan konffeihin!!!
	// nappi on disabloitu OnInitDialog:issa

	// HUOM!! t‰h‰n pit‰isi saada optio, ett‰ vain default-symbolia (symboleista) p‰‰see muuttamaan
}

void CFmiSeaIcingWarningsDlg::OnBnClickedButtonWarningCenterRefreshMessages()
{
    itsSmartMetDocumentInterface->SeaIcingWarningSystem().Clear(false);
    itsSmartMetDocumentInterface->SeaIcingWarningSystem().CheckForNewMessages(0);
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, true, true);
}

void CFmiSeaIcingWarningsDlg::OnBnClickedCheckShowAllMessages()
{
	UpdateData(TRUE);
	// T‰m‰ onkin ik‰v‰ juttu ett‰ t‰m‰ avain pit‰‰ olla useissa paikoissa annettuna (esim. genDocissa)
    itsSmartMetDocumentInterface->SeaIcingWarningSystem().StoreSettings("MetEditor::SeaIcingWarningSystem::");
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, true, true);
}

void CFmiSeaIcingWarningsDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiSeaIcingWarningsDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiSeaIcingWarningsDlg::InitHeaders(void)
{
	int basicColumnWidthUnit = 18;
	itsHeaders.clear();
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Nr", SeaIcingWarningsHeaderParInfo::kRowNumber, basicColumnWidthUnit*3));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Time (UTC)", SeaIcingWarningsHeaderParInfo::kReportTime, basicColumnWidthUnit*7));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("ShipID", SeaIcingWarningsHeaderParInfo::kShipId, basicColumnWidthUnit*3));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Spray", SeaIcingWarningsHeaderParInfo::kSpray, static_cast<int>(round(basicColumnWidthUnit*2.5))));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Int1", SeaIcingWarningsHeaderParInfo::kIntensity1, basicColumnWidthUnit*5));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Int2", SeaIcingWarningsHeaderParInfo::kIntensity2, basicColumnWidthUnit*5));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Vis KM", SeaIcingWarningsHeaderParInfo::kVisibility, basicColumnWidthUnit*3));
    itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("WS", SeaIcingWarningsHeaderParInfo::kWindSpeed, static_cast<int>(round(basicColumnWidthUnit*2.5))));
    itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Tair", SeaIcingWarningsHeaderParInfo::kTempAir, static_cast<int>(round(basicColumnWidthUnit*2.5))));
    itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Tw", SeaIcingWarningsHeaderParInfo::kTempWater, static_cast<int>(round(basicColumnWidthUnit*2.5))));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Waves", SeaIcingWarningsHeaderParInfo::kWaves, basicColumnWidthUnit*3));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Ice sp", SeaIcingWarningsHeaderParInfo::kIceSpeed, basicColumnWidthUnit*3));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Message", SeaIcingWarningsHeaderParInfo::kMessage, basicColumnWidthUnit*10));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Lon", SeaIcingWarningsHeaderParInfo::kLon, basicColumnWidthUnit*3));
	itsHeaders.push_back(SeaIcingWarningsHeaderParInfo("Lat", SeaIcingWarningsHeaderParInfo::kLat, basicColumnWidthUnit*3));
}

static std::string GetTimeAndStationString(SmartMetDocumentInterface *smartMetDocumentInterface, const NFmiMetTime &theTime, int theMessageCount, int theMapViewDescTopIndex)
{
	std::string str;
	str += "Warning messages was between ";
	str += " (-";
	str += NFmiStringTools::Convert(smartMetDocumentInterface->MapViewDescTop(theMapViewDescTopIndex)->TimeControlTimeStep());
	str += " hrs) - ";
	str += static_cast<char*>(theTime.ToStr(::GetDictionaryString("StationDataTableViewTimeFormat"), smartMetDocumentInterface->Language()));
	str += " ";
	str += NFmiStringTools::Convert<int>(theMessageCount).c_str();
	str += " ";
	str += ::GetDictionaryString("StationDataTableViewCount");
	return str;
}

static void SetHeaders(NFmiSeaIcingWarningsGridCtrl &theGridCtrl, const checkedVector<SeaIcingWarningsHeaderParInfo> &theHeaders, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount)
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

int CFmiSeaIcingWarningsDlg::CountShownMessages(void)
{
	boost::shared_ptr<NFmiArea> zoomedArea = itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapHandler()->Area();
	int editorTimeStep = static_cast<int>(::round(itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->TimeControlTimeStep()*60));
	std::vector<NFmiSeaIcingWarningMessage*> warningMessages = itsSmartMetDocumentInterface->SeaIcingWarningSystem().GetWantedWarningMessages(itsSmartMetDocumentInterface->CurrentTime(itsMapViewDescTopIndex), editorTimeStep, zoomedArea);
    return static_cast<int>(warningMessages.size());
}

void CFmiSeaIcingWarningsDlg::Update(void)
{
    static bool fFirstTime = true; // sarakkeiden s‰‰tˆ tehd‰‰n vain 1. kerran

    if(IsWindowVisible() && !IsIconic()) // N‰yttˆ‰ p‰ivitet‰‰n vain jos se on n‰kyviss‰ ja se ei ole minimized tilassa
    {
        NFmiMetTime wantedTime(itsSmartMetDocumentInterface->CurrentTime(itsMapViewDescTopIndex));
        int fixedRowCount = 1;
        int fixedColumnCount = 1;

        // LASKE kuinka monta HALY messagea on nykyisess‰ zoomatussa alueessa
        int messageCount = CountShownMessages();

        int rowCount = messageCount + fixedRowCount;  // +fixedRowCount on tilaa headerille
        itsGridCtrl.LastMessageCount(messageCount);

        itsTimeStrU_ = CA2T(::GetTimeAndStationString(itsSmartMetDocumentInterface, wantedTime, messageCount, itsMapViewDescTopIndex).c_str());

        // pistet‰‰n aika ja asema lkm tiedot talteen, jos printataan t‰m‰ n‰yttˆ
        itsGridCtrl.LastDataTime(wantedTime);

        itsGridCtrl.SetUsedHeaders(&itsHeaders);
        if(rowCount <= fixedRowCount) // jos ei ollut sanomia => tyhj‰ gridi
            SetHeaders(itsGridCtrl, itsHeaders, fixedRowCount, fFirstTime, fixedRowCount, fixedColumnCount); // t‰h‰n pit‰‰ antaa myˆs rowcounttiin fixedCount, koska muuten asmia on, mutta ei niihin laitettavaa dataa
        else // t‰ytet‰‰n taulukko HALY sanomilla
            FillGridWithWarningMessages(rowCount, fFirstTime, fixedRowCount, fixedColumnCount, wantedTime, messageCount);

        //	itsGridCtrl.DoLastSort(); // jos ollaan tehty aiemmin jokin sorttaus, se halutaan tehd‰ nyt t‰ss‰ uudestaan
        //	EnsureVisibleStationRow(); // skrollataan viel‰ viimeisin valittu asema n‰kyviin
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

static void SetGridCell(NFmiSeaIcingWarningsGridCtrl &theGridCtrl, int row, int column, const char *str)
{ // t‰h‰n tulee j‰rjestys numero fixed columniin
    theGridCtrl.SetItemText(row, column, CA2T(str));
	theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
}

static void SetGridRow(FmiLanguage language, const NFmiSeaIcingWarningMessage &theMessage, NFmiSeaIcingWarningsGridCtrl &theGridCtrl, const SeaIcingWarningsHeaderParInfo &theHeaderParInfo, int row, int column, const NFmiPoint & /* theLoc */ , const NFmiMetTime & /* theTime */ )
{
	switch(theHeaderParInfo.itsColumnFunction)
	{
	case SeaIcingWarningsHeaderParInfo::kRowNumber:
		::SetGridCell(theGridCtrl, row, column, ::Value2String(row, 0));
		break;
	case SeaIcingWarningsHeaderParInfo::kReportTime:
		::SetGridCell(theGridCtrl, row, column, static_cast<char*>(theMessage.Time().ToStr(::GetDictionaryString("StationDataTableViewTimeFormat"), language)));
		break;
	case SeaIcingWarningsHeaderParInfo::kShipId:
		::SetGridCell(theGridCtrl, row, column, theMessage.IdStr().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kSpray:
		::SetGridCell(theGridCtrl, row, column, theMessage.SprayStr().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kIntensity1:
		::SetGridCell(theGridCtrl, row, column, theMessage.Intensity1Str().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kIntensity2:
		::SetGridCell(theGridCtrl, row, column, theMessage.Intensity2Str().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kVisibility:
		::SetGridCell(theGridCtrl, row, column, theMessage.VisKMStr().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kWindSpeed:
		::SetGridCell(theGridCtrl, row, column, theMessage.WSStr().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kTempAir:
		::SetGridCell(theGridCtrl, row, column, theMessage.TemperatureStr().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kTempWater:
		::SetGridCell(theGridCtrl, row, column, theMessage.TwStr().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kWaves:
		::SetGridCell(theGridCtrl, row, column, theMessage.WaveStr().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kIceSpeed:
		::SetGridCell(theGridCtrl, row, column, theMessage.IceSpeedStr().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kMessage:
		::SetGridCell(theGridCtrl, row, column, theMessage.MessageStr().c_str());
		break;
	case SeaIcingWarningsHeaderParInfo::kLon:
		::SetGridCell(theGridCtrl, row, column, ::Value2String(theMessage.LatlonPoint().X(), 3));
		break;
	case SeaIcingWarningsHeaderParInfo::kLat:
		::SetGridCell(theGridCtrl, row, column, ::Value2String(theMessage.LatlonPoint().Y(), 3));
		break;
	}
// lopuksi s‰‰det‰‰n erikois solujen tausta v‰ri
//		if(column != 0) // column 0 on s‰‰detty fixediksi, ja sille m‰‰r‰t‰‰n v‰ritys toisaalla
//		{
//			if(useForecast)
//				theGridCtrl.SetItemBkColour(row, column, gForecastBkColor);
//			else
//				theGridCtrl.SetItemBkColour(row, column, gNormalSynopBkColor);
//		}
}


void CFmiSeaIcingWarningsDlg::FillGridWithWarningMessages(int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, const NFmiMetTime &theTime, int theMessageCount)
{
	SetHeaders(itsGridCtrl, itsHeaders, rowCount, fFirstTime, theFixedRowCount, theFixedColumnCount);

	int messagesFound = 0;
    int columnCount = static_cast<int>(itsHeaders.size());
	boost::shared_ptr<NFmiArea> zoomedArea = itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapHandler()->Area();
	int editorTimeStep = static_cast<int>(::round(itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->TimeControlTimeStep()*60));
	std::vector<NFmiSeaIcingWarningMessage*> warningMessages = itsSmartMetDocumentInterface->SeaIcingWarningSystem().GetWantedWarningMessages(theTime, editorTimeStep, zoomedArea);

    auto language = itsSmartMetDocumentInterface->Language();
	std::vector<NFmiSeaIcingWarningMessage*>::iterator it = warningMessages.begin();
	std::vector<NFmiSeaIcingWarningMessage*>::iterator endIt = warningMessages.end();
	for( ; it != endIt; ++it)
	{
		for(int i=0; i<columnCount; i++)
			SetGridRow(language, *(*it), itsGridCtrl, itsHeaders[i], messagesFound + theFixedRowCount, i, (*it)->LatlonPoint(), theTime);
		messagesFound++;
		if(messagesFound >= theMessageCount)
			break; // optimointia
	}
}
