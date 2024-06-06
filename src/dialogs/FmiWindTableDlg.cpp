// FmiWindTableDlg.cpp : implementation file
//

#ifdef _MSC_VER
#pragma warning(disable : 4996) // poistaa VC++ k‰‰nt‰j‰n varoituksen "This function or variable may be unsafe. Consider using...."
#endif

#include "stdafx.h"
#include "FmiWindTableDlg.h"
#include "FmiWin32TemplateHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiWindTableSystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiSmartInfo.h"
#include "NFmiMetEditorTypes.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiFileSystem.h"
#include "QueryDataIntegrator.h"
#include "CalculatorFactory.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiCtrlView.h"
#include "NFmiApplicationWinRegistry.h"
#include "TextGenPosixTime.h"
#include "CtrlViewFastInfoFunctions.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "EditedInfoMaskHandler.h"
#include "persist2.h"
#include "ApplicationInterface.h"

#include <fstream>

#include <boost/math/special_functions/round.hpp>

static const COLORREF gFixedBkColor = RGB(239, 235, 222);
static const int gHowManyDifferentValueTimesPerLine = 4;
static const int gDataBlockCount = 2;


// **************************************************
// NFmiWindTableGridCtrl dialog
// **************************************************

BEGIN_MESSAGE_MAP(NFmiWindTableGridCtrl, CGridCtrl)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

void NFmiWindTableGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
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

	// deselectoidaan kaikki
	SetSelectedRange(-1,-1,-1,-1, TRUE, TRUE);
}

bool NFmiWindTableGridCtrl::IsFixedRow(const CCellID &theCell)
{
	if(theCell.row < m_nFixedRows)
		return true;
	else
		return false;
}

void NFmiWindTableGridCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CCellID focusCell = GetCellFromPt(point);
	if(IsFixedRow(focusCell))
		return ; // ei saa menn‰ sorttaus p‰‰lle, joten kun on klikattu otsikko riviin lopetetaan

	CGridCtrl::OnLButtonDown(nFlags, point);
}

void NFmiWindTableGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnLButtonUp(nFlags, point);
	CCellID idCurrentCell = GetCellFromPt(point);
}

// Huom. numerot ovat teksti muodossa ja missing value on '-'-merkki.
// Haluan ett‰ puuttuvat ovat aina h‰nnill‰, joten pit‰‰ kikkailla.
int CALLBACK NFmiWindTableGridCtrl::pfnCellTextCaseInSensitiveCompare(LPARAM lParam1,
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
IMPLEMENT_DYNCREATE(NFmiWindTableGridCtrl, CGridCtrl)


// CFmiWindTableDlg dialog

const NFmiViewPosRegistryInfo CFmiWindTableDlg::s_ViewPosRegistryInfo(CRect(100, 150, 600, 600), "\\WindTableView");

IMPLEMENT_DYNAMIC(CFmiWindTableDlg, CDialog)

CFmiWindTableDlg::CFmiWindTableDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
:CDialog(CFmiWindTableDlg::IDD, pParent)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsMapViewDescTopIndex(0) // 0 eli sama kuin p‰‰karttan‰ytˆll‰
,fGridCtrlInitialized(false)
,itsGridCtrl()
,itsHeaders()
,itsProducerList()
,itsLastStartTime()
,fUseMaptime(FALSE)
{

}

CFmiWindTableDlg::~CFmiWindTableDlg()
{
}

void CFmiWindTableDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_GridControl(pDX, IDC_CUSTOM_GRID_CTRL, itsGridCtrl);
	DDX_Control(pDX, IDC_COMBO_WIND_PRODUCER_SELECTOR, itsProducerSelector);
	DDX_Check(pDX, IDC_CHECK_WIND_TABLE_MAP_TIME, fUseMaptime);
	DDX_Control(pDX, IDC_COMBO_WIND_AREA_FILTER_SELECTOR, itsAreaFilterSelector);
}


BEGIN_MESSAGE_MAP(CFmiWindTableDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_PRINT, &CFmiWindTableDlg::OnBnClickedButtonPrint)
	ON_CBN_SELCHANGE(IDC_COMBO_WIND_PRODUCER_SELECTOR, &CFmiWindTableDlg::OnCbnSelchangeComboWindProducerSelector)
	ON_BN_CLICKED(IDC_CHECK_WIND_TABLE_MAP_TIME, &CFmiWindTableDlg::OnBnClickedCheckWindTableMapTime)
	ON_CBN_SELCHANGE(IDC_COMBO_WIND_AREA_FILTER_SELECTOR, &CFmiWindTableDlg::OnCbnSelchangeComboWindAreaFilterSelector)
END_MESSAGE_MAP()


// CFmiWindTableDlg message handlers

void CFmiWindTableDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiWindTableDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

static void SetHeaders(NFmiWindTableGridCtrl &theGridCtrl, const std::vector<WindTableHeaderParInfo> &theHeaders, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount)
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

void CFmiWindTableDlg::Update(void)
{
    static bool fFirstTime = true; // sarakkeiden s‰‰tˆ tehd‰‰n vain 1. kerran

    if(IsWindowVisible() && !IsIconic()) // N‰yttˆ‰ p‰ivitet‰‰n vain jos se on n‰kyviss‰ ja se ei ole minimized tilassa
    {
        CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, __FUNCTION__);
        UpdateProducerList();

        // optimointi kohta, ei p‰ivitet‰ n‰yttˆ‰, jos tuottaja tai alkuaika ei ole vaihtunut
        auto &windTableSystem = itsSmartMetDocumentInterface->WindTableSystem();
        NFmiMetTime newStartTime = GetWantedStartTime();
        if(windTableSystem.DoInitializeAreaMasks() || itsLastStartTime != newStartTime || windTableSystem.MustaUpdateTable())
        {
			WaitCursorHelper waitCursorHelper(itsSmartMetDocumentInterface->ShowWaitCursorWhileDrawingView());

            windTableSystem.MustaUpdateTable(false);
            int fixedRowCount = 1;
            int fixedColumnCount = 1;
            // pistet‰‰n aika ja asema lkm tiedot talteen, jos printataan t‰m‰ n‰yttˆ
            itsGridCtrl.SetUsedHeaders(&itsHeaders);

            FillGridWithValues(fFirstTime, fixedRowCount, fixedColumnCount);
            itsLastStartTime = newStartTime;
        }
        UpdateData(FALSE);
    }
}

class WindTableData
{
public:
	NFmiMetTime itsTime;
	NFmiPoint itsLatlon;
	std::string itsAreaName;
	float itsWS;
	float itsWD;
	float itsWMax;
	int itsRowNumber;
};

static const char * Value2String(double value, int wantedDecimalCount)
{
	static char buffer[128]="";
	static char buffer2[128]="";
	// HUOM! n‰m‰ on syyt‰ tehd‰ _snprintf-funktioilla, koska tehokasta varsinkin MSVC k‰‰nt‰jill‰
	// NFmiStringTools-Convert on liian hidas, olen kokeillut. T‰t‰ funktiota saatetaan kutsua yhden 
	// ruudun p‰ivityksen aikana kymmeni‰tuhansia kertoja
	if(value == kFloatMissing)
	{
		buffer2[0] = '-';
		buffer2[1] = 0;
	}
	else
	{
		if(wantedDecimalCount == 0)
			value = ::round(value);
		else if(wantedDecimalCount == -1) // -1 on kikka vitonen eli tarkoitus on saada 10 jaollinen luku ulos
		{
			value = ::round(value/10.)*10;
			wantedDecimalCount = 0; // desimaali luku pit‰‰ muuttaa 0:ksi, ett‰ _snprintf tuottaa oikean laisen stringin
		}
		::_snprintf(buffer, sizeof(buffer)-1, "%%0.%df", wantedDecimalCount);
		buffer[sizeof(buffer)-1] = 0; // pit‰‰ varmistaa ett‰ p‰‰ttyy 0-merkkiin!!!!
		::_snprintf(buffer2, sizeof(buffer2)-1, buffer, value);
		buffer2[sizeof(buffer2)-1] = 0; // pit‰‰ varmistaa ett‰ p‰‰ttyy 0-merkkiin!!!!
	}
	return buffer2;
}

static void SetGridCell(NFmiWindTableGridCtrl &theGridCtrl, int row, int column, const std::string &str)
{ // t‰h‰n tulee j‰rjestys numero fixed columniin
    theGridCtrl.SetItemText(row, column, CA2T(str.c_str()));
	theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
}

static std::string GetGridCellTimeString(const NFmiMetTime &theTime)
{
	return std::string(theTime.ToStr("DDHH"));
}


static std::string GetGridCellString(float theValue, WindTableHeaderParInfo::ColumnFunction theFunction)
{
	switch(theFunction)
	{
	case WindTableHeaderParInfo::kRowNumber:
		return ::Value2String(theValue, 0);
	case WindTableHeaderParInfo::kWindSpeed:
		return ::Value2String(theValue, 0);
	case WindTableHeaderParInfo::kMaxWind:
		return ::Value2String(theValue, 0);
	case WindTableHeaderParInfo::kWindDirection:
		{
			theValue = static_cast<float>(::round(theValue/10.)*10);
			if(theValue == 0)
				theValue = 360;
			return ::Value2String(theValue, 0);
		}
	case WindTableHeaderParInfo::kLon:
		return ::Value2String(theValue, 3);
	case WindTableHeaderParInfo::kLat:
		return ::Value2String(theValue, 3);
	default:
		return std::string();
	}
}

static NFmiPoint GetSelectedLatlon(SmartMetDocumentInterface *smartMetDocumentInterface)
{
	NFmiPoint latlon(kFloatMissing, kFloatMissing);
	if(smartMetDocumentInterface)
	{
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = smartMetDocumentInterface->EditedSmartInfo();
		if(editedInfo)
		{
            EditedInfoMaskHandler editedInfoMaskHandler(editedInfo, CtrlViewFastInfoFunctions::GetProperMaskTypeFromEditeInfo(editedInfo, smartMetDocumentInterface->AllowRightClickDisplaySelection()));

			editedInfo->ResetLocation();
			if(editedInfo->NextLocation())
				latlon = editedInfo->LatLon();
		}
	}
	return latlon;
}

std::vector<FmiParameterName> gWantedSurfaceParams{kFmiWindDirection, kFmiWindSpeedMS};

static boost::shared_ptr<NFmiFastQueryInfo> GetSelectedData(SmartMetDocumentInterface *smartMetDocumentInterface, unsigned long producerId, NFmiInfoData::Type dataType, bool groundData)
{
    // Eri malleilla voi olla lukuisia pintadatoja. Etsit‰‰n niist‰ se, miss‰ on eniten kiinnostavia parametreja
    auto infos = smartMetDocumentInterface->InfoOrganizer()->GetInfos(dataType, groundData, producerId);
    return NFmiInfoOrganizer::GetInfoWithMostWantedParams(infos, gWantedSurfaceParams);
}

static boost::shared_ptr<NFmiFastQueryInfo> GetSelectedData(SmartMetDocumentInterface *smartMetDocumentInterface, const NFmiProducerHelperInfo *prodInfo)
{
    if(prodInfo)
        return ::GetSelectedData(smartMetDocumentInterface, prodInfo->itsProducerId, prodInfo->itsDataType, prodInfo->fGroundData);
	else
		return boost::shared_ptr<NFmiFastQueryInfo>();
}

static std::string GetSelectedProducerName(const CComboBox &theProducerSelector)
{
	int index = theProducerSelector.GetCurSel();
    CString tmpNameU_;
	if(index >= 0)
        theProducerSelector.GetLBText(index, tmpNameU_);
    std::string selectedNameStr = CT2A(tmpNameU_);
	return selectedNameStr;
}

static const NFmiProducerHelperInfo* GetProducerInfoByName(const std::vector<NFmiProducerHelperInfo> &theProducerList, const std::string &theProdName)
{
	for(size_t i = 0; i < theProducerList.size(); i++)
	{
		if(theProducerList[i].itsName == theProdName)
			return &theProducerList[i];
	}
	return 0;
}

static void SetRowNumberToGrid(NFmiWindTableGridCtrl &theGridCtrl, int theRowCounter)
{
	std::string rowNumberStr = ::GetGridCellString(static_cast<float>(theRowCounter), WindTableHeaderParInfo::kRowNumber);
	::SetGridCell(theGridCtrl, theRowCounter, 0, rowNumberStr);
}

boost::shared_ptr<NFmiFastQueryInfo> CFmiWindTableDlg::GetSelectedInfo(void)
{
	std::string selectedProdName = ::GetSelectedProducerName(itsProducerSelector);
	const NFmiProducerHelperInfo *prodInfo = ::GetProducerInfoByName(itsProducerList, selectedProdName);
	return ::GetSelectedData(itsSmartMetDocumentInterface, prodInfo);
}

NFmiMetTime CFmiWindTableDlg::GetWantedStartTime(void)
{
	int timeStepInMinutes = 360;
	NFmiMetTime wantedStartTime(itsSmartMetDocumentInterface->CurrentTime(itsMapViewDescTopIndex));
	if(itsSmartMetDocumentInterface->WindTableSystem().UseMapTime() == false)
		wantedStartTime = NFmiMetTime(); // jos ei k‰ytet‰ MapTime-asetusta, otetaan sein‰kelloon l‰hin timeSteppiin sopiva aika
	wantedStartTime.SetTimeStep(timeStepInMinutes);
	return wantedStartTime;
}

static void FillEmptyLine(NFmiWindTableGridCtrl &theGridCtrl, int theRowNumber, int theColumnStart, int theColumnEnd)
{
	for(int i=theColumnStart; i < theColumnEnd; i++)
		::SetGridCell(theGridCtrl, theRowNumber, i, "");
}

void CFmiWindTableDlg::FillGridWithValues(bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount)
{
    auto &windTableSystem = itsSmartMetDocumentInterface->WindTableSystem();
    auto& areaMaskDataList = windTableSystem.AreaMaskDataList();
	// T‰m‰ row count pit‰‰ laskea jotenkin paremmin, nyt simppeli kaava t‰ss‰
	int dataRowCount = static_cast<int>(gDataBlockCount * (areaMaskDataList.size() + 1 + 1)); // + 1 on time legenda +1 on tyhj‰ rivi
	int maxRowCount = static_cast<int>(theFixedRowCount + dataRowCount);
	SetHeaders(itsGridCtrl, itsHeaders, maxRowCount, fFirstTime, theFixedRowCount, theFixedColumnCount);

	int columnCount = static_cast<int>(itsHeaders.size());
	NFmiPoint tmpLatLon = ::GetSelectedLatlon(itsSmartMetDocumentInterface);
	NFmiMetTime wantedStartTime = GetWantedStartTime();
	itsGridCtrl.LastDataTime(wantedStartTime);

	boost::shared_ptr<NFmiFastQueryInfo> info = GetSelectedInfo();
	int currentRowCount = theFixedRowCount;
	if(info && info->Grid())
	{
		info->First();
		if(windTableSystem.DoInitializeAreaMasks())
		{
            windTableSystem.InitializeAllAreaMasks(info);
		}
		NFmiMetTime tmpTime = wantedStartTime;
		NFmiMetTime lastUsedTime(tmpTime);
		for(int jj=0; jj < gDataBlockCount; jj++) // tehd‰‰n 'kaksi' kertaa haluttu data blokki allekkain taulukkoon
		{
			tmpTime = lastUsedTime;
			::SetRowNumberToGrid(itsGridCtrl, currentRowCount);
			::FillEmptyLine(itsGridCtrl, currentRowCount, theFixedColumnCount, columnCount);
			currentRowCount++; // tyhj‰ rivi laitetaan t‰ss‰
			if(areaMaskDataList.size() > 0)
			{ // pit‰‰ laittaa data sarakkeiden aika legendat omalle riville ensin
				::SetRowNumberToGrid(itsGridCtrl, currentRowCount);
				::FillEmptyLine(itsGridCtrl, currentRowCount, theFixedColumnCount, columnCount);
				NFmiMetTime legendTime(tmpTime);
				int columnIndex = 2;
				for(int i=0; i<gHowManyDifferentValueTimesPerLine; i++)
				{
					std::string timeLegendStr = legendTime.ToStr("DDHH");
					::SetGridCell(itsGridCtrl, currentRowCount, columnIndex, timeLegendStr);
					columnIndex += 3; // oletus ett‰ 3 tuuli parametria aina jonossa!!!!
					legendTime.NextMetTime();
				}
				currentRowCount++;
			}

			for(size_t k = 0; k < areaMaskDataList.size(); k++)
			{
				{
					NFmiMetTime usedTime(tmpTime); // jokaiselle alueelle pit‰‰ k‰yd‰ samat ajat l‰pi
					// t‰m‰ pit‰isi saada jotenkin dymaaniseksi, mutta teen t‰h‰n hard code.
					// ELI column 0 on rivinumero
					// column 1 on alue nimi
					// siit‰ eteenp‰in tulee datan arvoja esim. ws,wd,wmax ja niit‰ tulee esim 4 kertaa yhdelle riville
					::SetRowNumberToGrid(itsGridCtrl, currentRowCount);
					int columnCounter = 1;
					::SetGridCell(itsGridCtrl, currentRowCount, columnCounter++, areaMaskDataList[k].Name());

					for(int i=0; i<gHowManyDifferentValueTimesPerLine; i++)
					{
						NFmiTime startTime(usedTime);
						startTime.ChangeByHours(windTableSystem.StartHourOffset());
						NFmiTime endTime(usedTime);
						endTime.ChangeByHours(windTableSystem.EndHourOffset());

                        info->Param(kFmiWindDirection);
                        boost::shared_ptr<TextGen::Calculator> wdSpaceCalculator(TextGen::CalculatorFactory::create(TextGen::Mean, 360));
                        boost::shared_ptr<TextGen::Calculator> wdTimeCalculator(TextGen::CalculatorFactory::create(TextGen::Mean, 360));
                        float WD = TextGen::QueryDataIntegrator::Integrate(*info,
                            areaMaskDataList[k].GetMask(),
                            *wdSpaceCalculator.get(),
                            TextGenPosixTime(startTime),
                            TextGenPosixTime(endTime),
                            *wdTimeCalculator.get());
                        std::string valueStr = ::GetGridCellString(WD, WindTableHeaderParInfo::kWindDirection);
                        ::SetGridCell(itsGridCtrl, currentRowCount, columnCounter++, valueStr);

                        info->Param(kFmiWindSpeedMS);
                        boost::shared_ptr<TextGen::Calculator> wsSpaceCalculator(TextGen::CalculatorFactory::create(TextGen::Mean));
                        boost::shared_ptr<TextGen::Calculator> wsTimeCalculator(TextGen::CalculatorFactory::create(TextGen::Mean));
                        float WS = TextGen::QueryDataIntegrator::Integrate(*info,
                            areaMaskDataList[k].GetMask(),
                            *wsSpaceCalculator.get(),
                            TextGenPosixTime(startTime),
                            TextGenPosixTime(endTime),
                            *wsTimeCalculator.get());

                        valueStr = ::GetGridCellString(WS, WindTableHeaderParInfo::kWindSpeed);
                        ::SetGridCell(itsGridCtrl, currentRowCount, columnCounter++, valueStr);

                        if(info->Param(kFmiHourlyMaximumWindSpeed) == false)
                            info->Param(kFmiWindSpeedMS); // jos ei lˆydy maximum WS parametria, otetaan sitten k‰yttˆˆn normaali WS
                        boost::shared_ptr<TextGen::Calculator> wsMaxSpaceCalculator(TextGen::CalculatorFactory::create(TextGen::Maximum));
                        boost::shared_ptr<TextGen::Calculator> wsMaxTimeCalculator(TextGen::CalculatorFactory::create(TextGen::Maximum));
                        float WMax = TextGen::QueryDataIntegrator::Integrate(*info,
                            areaMaskDataList[k].GetMask(),
                            *wsMaxSpaceCalculator.get(),
                            TextGenPosixTime(startTime),
                            TextGenPosixTime(endTime),
                            *wsMaxTimeCalculator.get());

						valueStr = ::GetGridCellString(WMax, WindTableHeaderParInfo::kMaxWind);
						::SetGridCell(itsGridCtrl, currentRowCount, columnCounter++, valueStr);

						usedTime.NextMetTime();
					}
					currentRowCount++;
					if(currentRowCount >= maxRowCount)
						break; // t‰m‰ on h‰t‰ paska koodia, jos ollaan menossa tasulukon yli, lopetetaan, mutta koodissa on vikaa!!!!
					lastUsedTime = usedTime;
				}
			}
		}
	}
	else // tyhj‰ taulukko muuten
		SetHeaders(itsGridCtrl, itsHeaders, theFixedRowCount, fFirstTime, theFixedRowCount, theFixedColumnCount);
}

void CFmiWindTableDlg::DoWhenClosing(void)
{
    itsSmartMetDocumentInterface->WindTableSystem().ViewVisible(false);
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, true, true);
}

void CFmiWindTableDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

void CFmiWindTableDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiWindTableDlg::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose();
}

BOOL CFmiWindTableDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	std::string titleStr = ::GetDictionaryString("Wind table dialog");
    SetWindowText(CA2T(titleStr.c_str()));

	std::string errorBaseStr("Error in CFmiWindTableDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

	itsGridCtrl.SetDocument(itsSmartMetDocumentInterface);
	itsGridCtrl.MoveWindow(CalcClientArea(), FALSE);
	fGridCtrlInitialized = true;
    auto &windTableSystem = itsSmartMetDocumentInterface->WindTableSystem();
    windTableSystem.InitAreaMaskDatas();
	fUseMaptime = windTableSystem.UseMapTime();
	InitHeaders();
    windTableSystem.DoInitializeAreaMasks(true);

	itsAreaFilterSelector.ResetContent();
    itsAreaFilterSelector.AddString(CA2T(::GetDictionaryString("Normal areas").c_str()));
    itsAreaFilterSelector.AddString(CA2T(::GetDictionaryString("Smaller areas").c_str()));
	itsAreaFilterSelector.SetCurSel(windTableSystem.SelectedAreaMaskList());

	Update();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiWindTableDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(fGridCtrlInitialized)
		itsGridCtrl.MoveWindow(CalcClientArea(), FALSE);
	Invalidate(FALSE);
}

CRect CFmiWindTableDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
	rect.top = rect.top + 40;
	return rect;
}

void CFmiWindTableDlg::InitHeaders(void)
{
	int basicColumnWidthUnit = 16;
	itsHeaders.clear();
	itsHeaders.push_back(WindTableHeaderParInfo(::GetDictionaryString("Row"), WindTableHeaderParInfo::kRowNumber, basicColumnWidthUnit*2));
	itsHeaders.push_back(WindTableHeaderParInfo(::GetDictionaryString("Area"), WindTableHeaderParInfo::kAreaName, basicColumnWidthUnit*3));
	for(int i=0; i<gHowManyDifferentValueTimesPerLine; i++)
	{
        itsHeaders.push_back(WindTableHeaderParInfo("WD", WindTableHeaderParInfo::kWindDirection, boost::math::iround(basicColumnWidthUnit*2.5)));
        itsHeaders.push_back(WindTableHeaderParInfo("WS", WindTableHeaderParInfo::kWindSpeed, boost::math::iround(basicColumnWidthUnit*2.5)));
        itsHeaders.push_back(WindTableHeaderParInfo("Wmax", WindTableHeaderParInfo::kMaxWind, boost::math::iround(basicColumnWidthUnit*2.5)));
	}
}

void CFmiWindTableDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

 // t‰m‰ on pika viritys, kun muuten Print (ja muiden ) -nappulan kohdalta j‰‰ kaista maalaamatta kun laitoin ikkunaan v‰lkkym‰ttˆm‰n p‰ivityksen
	CBrush brush(RGB(239, 235, 222));
	CRect area(CalcOtherArea());
	dc.FillRect(&area, &brush);
}

CRect CFmiWindTableDlg::CalcOtherArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CRect gridRect(CalcClientArea());
	rect.bottom = gridRect.top + 0;
	return rect;
}

void CFmiWindTableDlg::OnBnClickedButtonPrint()
{
	itsGridCtrl.Print(); // t‰h‰n tulee defaulttina footeria ja headeria ja "ei wysiwygi‰", mik‰ on ehk‰ parempi
}

void CFmiWindTableDlg::OnCbnSelchangeComboWindProducerSelector()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->WindTableSystem().DoInitializeAreaMasks(true);
	Update();
}

static bool FillProducerInfo(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiProducerHelperInfo &theProdInfo, const std::string &theProdName)
{
	if(theInfo)
	{
		theProdInfo.fGroundData = true;
		theProdInfo.itsName = theProdName;
		theProdInfo.itsDataType = theInfo->DataType();
		theProdInfo.itsProducerId = static_cast<int>(theInfo->Producer()->GetIdent());
		return true;
	}
	return false;
}

void CFmiWindTableDlg::UpdateProducerList(void)
{
	int lastSelectedProducer = itsProducerSelector.GetCurSel();
    CString lastProdNameU_;
	bool wasProducerSelected = true;
	if(lastSelectedProducer >= 0)
        itsProducerSelector.GetLBText(lastSelectedProducer, lastProdNameU_);
	else
		wasProducerSelected = false;

	// T‰ytet‰‰n ensin potentiaalisten tuottajien lista
	itsProducerList.clear();
	// editoitu tuottaja laitetaan listaan ensin, jos on editoitua dataa
	NFmiProducerHelperInfo tmpProdInfo;
    auto *infoOrganizer = itsSmartMetDocumentInterface->InfoOrganizer();
	if(::FillProducerInfo(infoOrganizer->FindInfo(NFmiInfoData::kEditable), tmpProdInfo, ::GetDictionaryString("StationDataTableViewEditedProducerName")))
		itsProducerList.push_back(tmpProdInfo);
	if(::FillProducerInfo(infoOrganizer->FindInfo(NFmiInfoData::kKepaData), tmpProdInfo, "Operative data"))
		itsProducerList.push_back(tmpProdInfo);
	if(::FillProducerInfo(infoOrganizer->FindInfo(NFmiInfoData::kEditingHelpData), tmpProdInfo, "Help edit data"))
		itsProducerList.push_back(tmpProdInfo);

	// loput tuottajat otetaan ProducerSystemist‰
    auto &producerSystem = itsSmartMetDocumentInterface->ProducerSystem();
	int modelCount = static_cast<int>(producerSystem.Producers().size());
	int i=0;
	for(i=0; i<modelCount; i++)
		itsProducerList.push_back(NFmiProducerHelperInfo::MakeProducerHelperInfo(producerSystem.Producer(i+1), NFmiInfoData::kViewable)); // +1 johtuu producersystemin 1-pohjaisesta indeksi systeemist‰

	// T‰ytet‰‰n sitten lista tuottajista, jotka todellakin lˆytyiv‰t
	itsProducerSelector.ResetContent();
	int ssize = static_cast<int>(itsProducerList.size());
	for(int i=0; i<ssize; i++)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = ::GetSelectedData(itsSmartMetDocumentInterface, itsProducerList[i].itsProducerId, itsProducerList[i].itsDataType, itsProducerList[i].fGroundData);
        // Datalla pit‰‰ olla haluttuja parametreja ennen kuin se valitaan listaan
		if(NFmiInfoOrganizer::CalcWantedParameterCount(info, gWantedSurfaceParams))
            itsProducerSelector.AddString(CA2T(itsProducerList[i].itsName.c_str()));
	}

	if(wasProducerSelected == false)
	{
		if(itsProducerSelector.GetCount() > 0)
			itsProducerSelector.SetCurSel(0); // valitaan 1. tuottaja jos mahdollista, jos ei ollut mit‰‰n tuottajaa valittuna
	}
	else
	{ // Laita viimeksi valitut taas p‰‰lle listasta
		itsProducerSelector.SetCurSel(-1);
		for(int j=0; j<itsProducerSelector.GetCount(); j++)
		{
            CString tmpNameStrU_;
            itsProducerSelector.GetLBText(j, tmpNameStrU_);
            if(lastProdNameU_ == tmpNameStrU_)
			{
				itsProducerSelector.SetCurSel(j);
				break;
			}
		}
	}
}


void CFmiWindTableDlg::OnBnClickedCheckWindTableMapTime()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->WindTableSystem().UseMapTime(fUseMaptime == TRUE);
	Update();
}

void CFmiWindTableDlg::OnCbnSelchangeComboWindAreaFilterSelector()
{
	UpdateData(TRUE);
	
    itsSmartMetDocumentInterface->WindTableSystem().SelectedAreaMaskList(itsAreaFilterSelector.GetCurSel());
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, true, true, itsMapViewDescTopIndex);
}
