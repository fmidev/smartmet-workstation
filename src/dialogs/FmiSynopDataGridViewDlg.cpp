// FmiSynopDataGridViewDlg.cpp : implementation file
//
#ifdef _MSC_VER
#pragma warning(disable : 4996) // poistaa varoituksen "warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead"
#endif

#include "stdafx.h"
#include "FmiSynopDataGridViewDlg.h"
#include "NFmiStringTools.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiProducerName.h"
#include "FmiCountryFilterDlg.h"
#include "NFmiDataModifierMinMax.h"
#include "NFmiDataModifierSum.h"
#include "NFmiDictionaryFunction.h"
#include "FmiNMeteditLibraryDefinitions.h"
#include "FmiWin32TemplateHelpers.h"
#include "NFmiProducerSystem.h"
#include "NFmiArea.h"
#include "NFmiSoundingFunctions.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "NFmiMapViewDescTop.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiFastInfoUtils.h"
#include "SynopCodeUtils.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "persist2.h"
#include "UnicodeStringConversions.h"
#include "MapDrawFunctions.h"

using namespace std;

namespace
{
    LOGFONT gSynopFont; // haluan ww parametrin tapauksessa k‰ytt‰‰ synop-fonttia ja t‰h‰n talletetaan sen rakenne
    bool gSynopFontInitialized = false;

    void InitSynopFont()
    {
        if(!gSynopFontInitialized)
        {
            CString fontName("synop");
            gSynopFont.lfWeight = FW_BOLD;
            gSynopFont.lfHeight = 23;
            _tcsncpy_s(gSynopFont.lfFaceName, LF_FACESIZE, fontName, fontName.GetLength());

            gSynopFontInitialized = true;
        }
    }

    const COLORREF gLastGoodSynopBkColor = RGB(252, 154, 143);
    const COLORREF gForecastBkColor = RGB(240, 249, 255);
    const COLORREF gNormalSynopBkColor = RGB(255, 255, 255);
    const COLORREF gFixedBkColor = RGB(239, 235, 222);

    unsigned long gPressureChangePreviousTimeIndex; // t‰m‰ on ik‰v‰‰ koosia, mutta en jaksa nyt tehd‰ hienompaa (optimointia)
                                                            // t‰h‰n laitetaan aina se aika indeksi, josta saadaan 3h aiemmat paine arvot
                                                            // kun lasketaan havainnoista paineen muutosta
}

std::string WmoIdFilterManager::GetFilterString(void) const
{
	std::string str;
	if(fAllSelected)
		str += ::GetDictionaryString("StationDataTableViewAllCountries");
	else
	{
		str += ::GetDictionaryString("StationDataTableViewCountries");
		str += ": ";
		int ssize = static_cast<int>(itsCountryFilters.size());
		int foundCount = 0;
		for(int i=0; i<ssize; i++)
		{
			if(itsCountryFilters[i].fSelected)
			{
				if(foundCount > 0)
					str += ",";
				str += itsCountryFilters[i].itsShortName;
				foundCount++;
			}
		}
		if(foundCount == 0)
			str += "-";
	}
	return str;
}

// tekee stringin, miss‰ on yhdistettyn‰ kaikki valitut maat lyhentein‰ pilkuilla erotettuna
// esim. FI,NO,UK
std::string WmoIdFilterManager::GetSelectedCountryAbbrStr(void) const
{
	std::string str;
	int foundCount = 0;
	int ssize = static_cast<int>(itsCountryFilters.size());
	for(int i=0; i<ssize; i++)
	{
		if(itsCountryFilters[i].fSelected)
		{
			if(foundCount > 0)
				str += ",";
			str += itsCountryFilters[i].itsShortName;
			foundCount++;
		}
	}
	return str;
}

void WmoIdFilterManager::SetSelectedCountryAbbrStr(const std::string &theStr)
{
	std::set<string> countryMap = NFmiStringTools::Split<std::set<string> >(theStr, ",");
	int ssize = static_cast<int>(itsCountryFilters.size());
	for(int i=0; i<ssize; i++)
	{
		std::set<string>::iterator it = countryMap.find(itsCountryFilters[i].itsShortName);
		if(it != countryMap.end())
			itsCountryFilters[i].fSelected = true;
		else
			itsCountryFilters[i].fSelected = false;
	}
}


SynopDataGridViewUsedFileNames::SynopDataGridViewUsedFileNames(const std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &obsInfos, const boost::shared_ptr<NFmiFastQueryInfo> &usedInfo, const NFmiMetTime &validTime)
:itsUsedDataFileName()
,itsObsDataFileNames()
,itsValidTime(validTime)
{
    UpdateNames(obsInfos, usedInfo);
}

bool SynopDataGridViewUsedFileNames::IsUpdateNeeded(const SynopDataGridViewUsedFileNames &other, bool modelDataCase, bool minMaxModeUsed) const
{
    // Tarkastellaan ensin onko aika muuttunut
    if(IsUpdateNeededDueTimeChange(other, minMaxModeUsed))
        return true;

    if(modelDataCase)
    {
        // Jos itsUsedDataFileName on erilainen TAI itsObsDataFileNames.size() ovat erikokoisia
        if(itsUsedDataFileName != other.itsUsedDataFileName)
            return true;
        if(itsObsDataFileNames.size() != other.itsObsDataFileNames.size())
            return true;
    }
    else
    {
        // Synop datat tulee aina tietyss‰ prioriteetti j‰rjestyksess‰, joten voimme suoraan verrata vectro:eita, 
        // ilman ett‰ tarvitsee etsi‰ tiettyj‰ tiedoston nimi‰ toisesta vektorista erikseen...
        if(itsObsDataFileNames != other.itsObsDataFileNames)
            return true;
    }

    return false;
}

bool SynopDataGridViewUsedFileNames::IsUpdateNeededDueTimeChange(const SynopDataGridViewUsedFileNames &other, bool minMaxModeUsed) const
{
    // Tarkastelu vain jos ei olla min/max moodissa, siin‰ tulee aina pakotettu update, kun aikakontrolleja muutetaan.
    if(!minMaxModeUsed)
    {
        if(itsValidTime != other.itsValidTime)
            return true;
    }

    return false;
}

std::string SynopDataGridViewUsedFileNames::GetChangedFileNames(const SynopDataGridViewUsedFileNames &other, bool modelDataCase) const
{
    if(modelDataCase)
    {
        // Jos itsUsedDataFileName on erilainen TAI itsObsDataFileNames.size() ovat erikokoisia
        if(itsUsedDataFileName != other.itsUsedDataFileName)
            return other.itsUsedDataFileName;
        if(itsObsDataFileNames.size() != other.itsObsDataFileNames.size())
            return GetChangedFileNames(other.itsObsDataFileNames);
    }
    else
    {
        if(itsObsDataFileNames != other.itsObsDataFileNames)
            return GetChangedFileNames(other.itsObsDataFileNames);
    }

    return "";
}

std::string SynopDataGridViewUsedFileNames::GetChangedFileNames(const std::vector<std::string> &otherObsDataFileNames) const
{
    std::string changedFileNames;
    for(const auto &fileName : otherObsDataFileNames)
    {
        auto iter = std::find(itsObsDataFileNames.begin(), itsObsDataFileNames.end(), fileName);
        if(iter == itsObsDataFileNames.end())
        {
            if(!changedFileNames.empty())
                changedFileNames += ", ";
            changedFileNames += fileName;
        }
    }

    return changedFileNames;
}

void SynopDataGridViewUsedFileNames::Clear()
{
    ClearNames();
    itsValidTime = NFmiMetTime::gMissingTime;
}

void SynopDataGridViewUsedFileNames::ClearNames()
{
    itsUsedDataFileName.clear();
    itsObsDataFileNames.clear();
}

bool SynopDataGridViewUsedFileNames::Empty() const
{
    if(itsUsedDataFileName.empty() && itsObsDataFileNames.empty())
        return true;
    else
        return false;
}

void SynopDataGridViewUsedFileNames::UpdateNames(const std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &obsInfos, const boost::shared_ptr<NFmiFastQueryInfo> &usedInfo)
{
    ClearNames();
    if(usedInfo)
        itsUsedDataFileName = usedInfo->DataFileName();
    for(const auto &info : obsInfos)
        itsObsDataFileNames.push_back(info->DataFileName());
}



static string GetMinMaxDateString(const NFmiMetTime &theTime)
{
	string str(theTime.ToStr("DD.MM.YYYY HH:mm"));
	return str;
}

static NFmiMetTime GetTimeFromMinMaxDateString(const string &theTimeStr)
{
	NFmiMetTime aTime(NFmiMetTime::gMissingTime);
	const std::vector<string> list1 = NFmiStringTools::Split<std::vector<string> >(theTimeStr, " ");
	if(list1.size() == 2)
	{
		const std::vector<string> list2 = NFmiStringTools::Split<std::vector<string> >(list1[0], ".");
		const std::vector<string> list3 = NFmiStringTools::Split<std::vector<string> >(list1[1], ":");
		if(list2.size() == 3 && list3.size() == 2)
		{
			short YYYY = NFmiStringTools::Convert<short>(list2[2]);
			short MM = NFmiStringTools::Convert<short>(list2[1]);
			short DD = NFmiStringTools::Convert<short>(list2[0]);
			short HH = NFmiStringTools::Convert<short>(list3[0]);
			short mm = NFmiStringTools::Convert<short>(list3[1]);
			aTime = NFmiMetTime(YYYY, MM, DD, HH, mm);
		}
	}

	return aTime;
}

NFmiMetTime GetMinMaxEndTime(const NFmiMetTime &theStartTime, double theDayRangeValue)
{
	NFmiMetTime endTime(theStartTime);
	if(theDayRangeValue < 1.)
		endTime.ChangeByHours(static_cast<long>(theDayRangeValue)*24);
	else
		endTime.ChangeByDays(static_cast<long>(theDayRangeValue));
	return endTime;
}

static int FindWantedHeaderColumn(std::vector<HeaderParInfo> *theHeaders, HeaderParInfo::RangeFunction theFunction)
{
	for(int i=0; i< static_cast<int>(theHeaders->size()); i++)
		if((*theHeaders)[i].itsRangeCalculationFunction == theFunction)
			return i;
	return -1;
}

static HeaderParInfo::RangeFunction GetWantedHeaderColumnFunction(std::vector<HeaderParInfo> *theHeaders,  const CCellID &focusCell)
{
	if(focusCell.col >= 0 && focusCell.col < static_cast<int>(theHeaders->size()))
		return (*theHeaders)[focusCell.col].itsRangeCalculationFunction;
	return HeaderParInfo::kWmoId;
}

static string GetTimeAndStationString(FmiLanguage language, const NFmiMetTime &theTime, int theStationCount, const WmoIdFilterManager &theWmoIdFilterManager, const std::string &theProducerString)
{
	string str;
	str = theProducerString;
	str += ": ";
	str += static_cast<char*>(theTime.ToStr(::GetDictionaryString("StationDataTableViewTimeFormat"), language));
	str += "  ";
	str += ::GetDictionaryString("StationDataTableViewStation");
	str += " ";
	str += NFmiStringTools::Convert<int>(theStationCount).c_str();
	str += " ";
	str += ::GetDictionaryString("StationDataTableViewCount");
	str += " (";
	str += theWmoIdFilterManager.GetFilterString();
	str += ")";
	return str;
}

static string GetMinMaxTimeRangeAndStationString(FmiLanguage language, const NFmiMetTime &theStartTime, double theDayRange, int theStationCount, const WmoIdFilterManager &theWmoIdFilterManager, const std::string &theProducerString)
{
	NFmiMetTime endTime(GetMinMaxEndTime(theStartTime, theDayRange));
	string str;
	str = theProducerString;
	str += ": ";
	str += static_cast<char*>(theStartTime.ToStr(::GetDictionaryString("StationDataTableViewTimeFormat"), language));
	str += " - ";
	str += static_cast<char*>(endTime.ToStr(::GetDictionaryString("StationDataTableViewTimeFormat"), language));
	str += "  ";
	str += ::GetDictionaryString("StationDataTableViewStation");
	str += " ";
	str += NFmiStringTools::Convert<int>(theStationCount).c_str();
	str += " ";
	str += ::GetDictionaryString("StationDataTableViewCount");
	str += " (";
	str += theWmoIdFilterManager.GetFilterString();
	str += ")";
	return str;
}

BEGIN_MESSAGE_MAP(NFmiGridCtrl, CGridCtrl)
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
ON_WM_KEYDOWN()
ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

void NFmiGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CGridCtrl::OnLButtonDblClk(nFlags, point);

	// Jos ollaan min/max moodissa ja tuplaklikataan aika saraketta
	// jolloin siirret‰‰n kartan aika t‰ss‰ solussa olevaan aikaan
	if(fMinMaxModeOn)
	{
		CCellID focusCell(GetCellFromPt(point));
		if(GetWantedHeaderColumnFunction(itsUsedHeaders, focusCell) == HeaderParInfo::kDateAndTime)
		{
            CString aTimeStrU_(GetCell(focusCell.row, focusCell.col)->GetText());
            std::string tmp = CT2A(aTimeStrU_);
			NFmiMetTime aTime(GetTimeFromMinMaxDateString(tmp));
			if(aTime != NFmiMetTime::gMissingTime)
			{
				itsSmartMetDocumentInterface->CurrentTime(itsMapViewDescTopIndex, aTime);
                itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("SynopDataGridViewDlg: Left mouse button double click");
			}
		}
	}
}

void NFmiGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
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
	// nollataan highlight juttu kartta n‰ytˆst‰
	NFmiPoint latlon;
    itsSmartMetDocumentInterface->SetHighlightedSynopStation(latlon, -1, false); // -1 on wmoid, mill‰ ei kait olekaan k‰yttˆ‰
    itsSmartMetDocumentInterface->LastSelectedSynopWmoId(-1);
	SetSelectedRange(-1,-1,-1,-1, TRUE, TRUE);
    itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(0, true, false); // p‰ivitet‰‰n vain p‰‰n‰ytˆn jutut (0 indeksi)

	if(focusCell.row < m_nFixedRows)
	{ // jos oltiin klikattu otsikko rivi‰, l‰hetet‰‰n viesti emolle, ett‰ se osaa p‰ivitt‰‰ taulukon uudestaan
		fUpdateParent = true;
		SendMessageToParent(focusCell.row, focusCell.col, GVN_COLUMNCLICK);
	}
}

void NFmiGridCtrl::DoMapHighLightThings(const CCellID &idCurrentCell)
{
	if(itsUsedHeaders == 0)
		return ;
	if(idCurrentCell.row >= this->GetFixedRowCount() && idCurrentCell.row < this->GetRowCount())
	{
		int wmoColumn = FindWantedHeaderColumn(itsUsedHeaders, HeaderParInfo::kWmoId);
		if(wmoColumn < 0)
			return ;
		int wmoId = _tstoi(GetCell(idCurrentCell.row, wmoColumn)->GetText());
        itsSmartMetDocumentInterface->LastSelectedSynopWmoId(wmoId);
		int lonColumn = FindWantedHeaderColumn(itsUsedHeaders, HeaderParInfo::kLon);
		int latColumn = FindWantedHeaderColumn(itsUsedHeaders, HeaderParInfo::kLat);
		if(lonColumn < 0 || latColumn < 0 )
			return ;
		string lonStr = CT2A(GetCell(idCurrentCell.row, lonColumn)->GetText());
		string latStr = CT2A(GetCell(idCurrentCell.row, latColumn)->GetText());
		string missingLon("-");
		double lon = (lonStr == missingLon) ? kFloatMissing : atof(lonStr.c_str());
		double lat = (latStr == missingLon) ? kFloatMissing : atof(latStr.c_str());
		NFmiPoint latlon(lon, lat);
		if(latlon != itsSmartMetDocumentInterface->GetSynopHighlightLatlon())
		{
            itsSmartMetDocumentInterface->SetHighlightedSynopStation(latlon, -1, true); // -1 on wmoid, mill‰ ei kait olekaan k‰yttˆ‰
            itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(0, true, false); // p‰ivitet‰‰n vain p‰‰n‰ytˆn jutut (0 indeksi)
		}
	}
	else
	{ // muuten nollataan highlight juttu
		NFmiPoint latlon;
        itsSmartMetDocumentInterface->SetHighlightedSynopStation(latlon, -1, false); // -1 on wmoid, mill‰ ei kait olekaan k‰yttˆ‰
        itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(0, true, false); // p‰ivitet‰‰n vain p‰‰n‰ytˆn jutut (0 indeksi)
        itsSmartMetDocumentInterface->LastSelectedSynopWmoId(-1);
	}
}

void NFmiGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CGridCtrl::OnLButtonUp(nFlags, point);
	CCellID idCurrentCell = GetCellFromPt(point);
	DoMapHighLightThings(idCurrentCell);
}

void NFmiGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CGridCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

    CCellRange selectedRange(GetSelectedCellRange());
	CCellID idCurrentCell = CCellID(selectedRange.GetMaxRow(), selectedRange.GetMinCol());
	DoMapHighLightThings(idCurrentCell);
}

void NFmiGridCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CGridCtrl::OnMouseMove(nFlags, point);

	if(nFlags & MK_CONTROL) // jos kontrolli pohjassa ja liikutetaan hiiren kursoria taulukon p‰‰ll‰
	{
		// t‰m‰ ottaa rivin talteen ja sen rivin synop aseman latlon pisteen
		CCellID idCurrentCell = GetCellFromPt(point);
		DoMapHighLightThings(idCurrentCell);
	}
}

// Huom. numerot ovat teksti muodossa ja missing value on '-'-merkki.
// Haluan ett‰ puuttuvat ovat aina h‰nnill‰, joten pit‰‰ kikkailla.
int CALLBACK NFmiGridCtrl::pfnCellTextCaseInSensitiveCompare(LPARAM lParam1,
																LPARAM lParam2,
																LPARAM lParamSort)
{
	UNUSED_ALWAYS(lParamSort);

	CGridCellBase* pCell1 = (CGridCellBase*) lParam1;
	CGridCellBase* pCell2 = (CGridCellBase*) lParam2;
	if (!pCell1 || !pCell2) return 0;

	return _tcsnicmp(pCell1->GetText(), pCell2->GetText(), 50); // 50 on hatusta, koska en tied‰ miten saan LPCTSTR-otukselta kysytty‰ stringin pituutta
}

// Huom. numerot ovat teksti muodossa ja missing value on '-'-merkki.
// Haluan ett‰ puuttuvat ovat aina h‰nnill‰, joten pit‰‰ kikkailla.
int CALLBACK NFmiGridCtrl::pfnCellNumericWithMissingValuesCompare(LPARAM lParam1,
																LPARAM lParam2,
																LPARAM lParamSort) // t‰nne tulee tieto onko sorttaus nouseva (1) vai laskeva (0)
{
	CGridCellBase* pCell1 = (CGridCellBase*) lParam1;
	CGridCellBase* pCell2 = (CGridCellBase*) lParam2;
	if (!pCell1 || !pCell2) return 0;

	LPCTSTR str1(pCell1->GetText());
	LPCTSTR str2(pCell2->GetText());
    CString strMissU_(_TEXT("-"));
    if(str1 == strMissU_ && str2 == strMissU_)
		return 0;
    if(str1 == strMissU_) // jos toinen luku on miss, pit‰‰ kikkailla laskevan ja nousevan sorttauksen kanssa, ett‰ puuttuvat arvot menev‰t aina viimeiseksi
		return lParamSort ? 1 : -1;
    if(str2 == strMissU_) // jos toinen luku on miss, pit‰‰ kikkailla laskevan ja nousevan sorttauksen kanssa, ett‰ puuttuvat arvot menev‰t aina viimeiseksi
		return lParamSort ? -1 : 1;

	double nValue1 = _tstof(str1);
	double nValue2 = _tstof(str2);

	if (nValue1 < nValue2)
		return -1;
	else if (nValue1 == nValue2)
		return 0;
	else
		return 1;
}

// NFmiGridCtrl luokka macro + alustukset
IMPLEMENT_DYNCREATE(NFmiGridCtrl, CGridCtrl)

// piti laittaa columnin headerista sorttaus uusiksi
void NFmiGridCtrl::OnFixedRowClick(CCellID& cell)
{
	if(cell.col == 2) // pieni hardcode sorttausta varten, jos nimi columni, tehd‰‰n teksti sorttaus
		SetCompareFunction(NFmiGridCtrl::pfnCellTextCaseInSensitiveCompare);
	else // muuten numero sorttaus
		SetCompareFunction(NFmiGridCtrl::pfnCellNumericWithMissingValuesCompare);

// **** TƒSTƒ loppuun kopioitu suoraan emosta koodia PAITSI kun SortItems-metodille annetaan kolmas parametri
    if (!IsValid(cell))
        return;

    if (GetHeaderSort())
    {
		WaitCursorHelper waitCursorHelper(itsSmartMetDocumentInterface->ShowWaitCursorWhileDrawingView());
        if (cell.col == GetSortColumn())
            SortItems(cell.col, !GetSortAscending(), !GetSortAscending()); // *** KOLMAS parametri annettu t‰ss‰ koodissa ****
        else
            SortItems(cell.col, TRUE, TRUE); // *** KOLMAS parametri annettu t‰ss‰ koodissa ********
        // sorttaus info talletus tehd‰‰n vasta SortItems kutsun j‰lkeen!
        StoreLastSortInformation(cell);
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

void NFmiGridCtrl::StoreLastSortInformation(CCellID& cell)
{
    itsLastSortedCell = cell;
    fLastSortedAscending = GetSortAscending();
    fLastSortedExist = true;
}

void NFmiGridCtrl::DoLastSort(void)
{
	if(this->fLastSortedExist)
	{
		this->SetSortAscending(this->fLastSortedAscending);
		this->SetSortColumn(itsLastSortedCell.col);
        SortItems(GetSortColumn(), GetSortAscending(), GetSortAscending());
	}
}

//print footer with a line and date, and page number
void NFmiGridCtrl::PrintFooter(CDC *pDC, CPrintInfo *pInfo)
{
    // page numbering on left
    CString strLeftU_;
    strLeftU_.Format(_T("s. %d/%d"), pInfo->m_nCurPage, pInfo->GetMaxPage());

    // date and time on the right
    CString strRightU_;
	NFmiTime atime;
    strRightU_ = CA2T(atime.ToStr("YYYY.MM.DD HH:mm"));

    CRect rc(pInfo->m_rectDraw);

    // draw ruled line on bottom
    pDC->SelectStockObject(BLACK_PEN);
    pDC->MoveTo(rc.left, rc.top);
    pDC->LineTo(rc.right, rc.top);

    CFont BoldFont;
    LOGFONT lf;

    //create bold font for header and footer
    m_PrinterFont.GetLogFont(&lf);
    lf.lfWeight = FW_BOLD;
    BoldFont.CreateFontIndirect(&lf);

    CFont *pNormalFont = pDC->SelectObject(&BoldFont);
    int nPrevBkMode = pDC->SetBkMode(TRANSPARENT);

    // EFW - Bug fix - Force text color to black.  It doesn't always
    // get set to a printable color when it gets here.
    pDC->SetTextColor(RGB(0, 0, 0));

    if(!strLeftU_.IsEmpty())
        pDC->DrawText(strLeftU_, &rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);
    if(!strRightU_.IsEmpty())
        pDC->DrawText(strRightU_, &rc, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    pDC->SetBkMode(nPrevBkMode);
    pDC->SelectObject(pNormalFont);
    BoldFont.DeleteObject();
}

void NFmiGridCtrl::PrintHeader(CDC *pDC, CPrintInfo *pInfo)
{
    // print App title on top right margin
    CString strRightU_;
    strRightU_.LoadString(AFX_IDS_APP_TITLE);

    auto language = itsSmartMetDocumentInterface->Language();
    // print parent window title in the centre (Gert Rijs)
    CString strCenterU_(fMinMaxModeOn ?
		GetMinMaxTimeRangeAndStationString(language, itsLastDataTime, itsDayRangeValue,itsLastStationCount, *itsWmoIdFilterManager, itsProducerString).c_str() :
		GetTimeAndStationString(language, itsLastDataTime, itsLastStationCount, *itsWmoIdFilterManager, itsProducerString).c_str());

	CFont   BoldFont;
    LOGFONT lf;

    //create bold font for header and footer
    VERIFY(m_PrinterFont.GetLogFont(&lf));
    lf.lfWeight = FW_BOLD;
	lf.lfHeight = 16;
    VERIFY(BoldFont.CreateFontIndirect(&lf));

    CFont *pNormalFont = pDC->SelectObject(&BoldFont);
    int nPrevBkMode = pDC->SetBkMode(TRANSPARENT);

    CRect   rc(pInfo->m_rectDraw);
    if(!strCenterU_.IsEmpty())
        pDC->DrawText(strCenterU_, &rc, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);
    if(!strRightU_.IsEmpty())
        pDC->DrawText(strRightU_, &rc, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    pDC->SetBkMode(nPrevBkMode);
    pDC->SelectObject(pNormalFont);
    BoldFont.DeleteObject();

    // draw ruled-line across top
    pDC->SelectStockObject(BLACK_PEN);
    pDC->MoveTo(rc.left, rc.bottom);
    pDC->LineTo(rc.right, rc.bottom);
}

// CFmiSynopDataGridViewDlg dialog

const NFmiViewPosRegistryInfo CFmiSynopDataGridViewDlg::s_ViewPosRegistryInfo(CRect(450, 150, 950, 750), "\\SynopGridView");

IMPLEMENT_DYNAMIC(CFmiSynopDataGridViewDlg, CDialog)
CFmiSynopDataGridViewDlg::CFmiSynopDataGridViewDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
:CDialog(CFmiSynopDataGridViewDlg::IDD, pParent)
,itsGridCtrl(0) // sidotaan toistaiseksi p‰‰karttan‰yttˆˆn
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,fGridCtrlInitialized(false)
, itsTimeStrU_(_T(""))
,itsTimeAndStationTextFont()
,itsMapViewDescTopIndex(0) // sidotaan toistaiseksi p‰‰karttan‰yttˆˆn
,itsSynopHeaders()
,itsForecastSynopHeaders()
,itsMinMaxDataHeaders()
,itsForecastMinMaxDataHeaders()
,itsWmoIdFilterManager(0)
,itsProducerList()
,fMinMaxModeOn(FALSE)
,itsDayRangeValue(1)
,itsUsedHeaders(0)
,fUseSadeData(false)
,fUpdateHeadersAfterViewMacroLoad(false)
{
    ::InitSynopFont();
}

CFmiSynopDataGridViewDlg::~CFmiSynopDataGridViewDlg()
{
}

void CFmiSynopDataGridViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_GridControl(pDX, IDC_CUSTOM_GRID_CTRL, itsGridCtrl);
    DDX_Text(pDX, IDC_STATIC_TIME_STR, itsTimeStrU_);
	DDX_Control(pDX, IDC_COMBO_PRODUCER_SELECTION, itsProducerSelector);
	DDX_Text(pDX, IDC_EDIT_DAY_COUNT, itsDayRangeValue);
	DDV_MinMaxDouble(pDX, itsDayRangeValue, 0.1, 100);
	DDX_Check(pDX, IDC_CHECK_MIN_MAX_MODE, fMinMaxModeOn);
	DDX_Control(pDX, IDC_DATETIMEPICKER_MINMAX_DATE, itsMinMaxDatePicker);
	DDX_Control(pDX, IDC_DATETIMEPICKER_MINMAX_TIME, itsMinMaxTimePicker);
}


BEGIN_MESSAGE_MAP(CFmiSynopDataGridViewDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_PRINT, OnBnClickedButtonPrint)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_COUNTRY_FILTER_DLG, OnBnClickedButtonCountryFilterDlg)
	ON_BN_CLICKED(IDC_BUTTON_PREVIOUS_TIME2, OnBnClickedButtonPreviousTime2)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_TIME, OnBnClickedButtonNextTime)
	ON_CBN_SELCHANGE(IDC_COMBO_PRODUCER_SELECTION, OnCbnSelchangeComboProducerSelection)
	ON_BN_CLICKED(IDC_CHECK_MIN_MAX_MODE, OnBnClickedCheckMinMaxMode)
	ON_EN_CHANGE(IDC_EDIT_DAY_COUNT, OnEnChangeEditDayCount)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_MINMAX_DATE, OnDtnDatetimechangeDatetimepickerMinmaxDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_MINMAX_TIME, OnDtnDatetimechangeDatetimepickerMinmaxTime)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_SIZE_COLUMNS, OnBnClickedButtonAutoSizeColumns)
END_MESSAGE_MAP()


// CFmiSynopDataGridViewDlg message handlers

BOOL CFmiSynopDataGridViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	InitWmoIdFilterManager();
	InitSynopHeaders();
	InitForecastSynopHeaders();
	InitMinMaxDataHeaders();
	InitForecastMinMaxDataHeaders();

	// Tee paikan asetus vasta tooltipin alustuksen j‰lkeen, niin se toimii ilman OnSize-kutsua.

	std::string errorBaseStr("Error in CFmiSynopDataGridViewDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

	InitProducerSelector();
	itsGridCtrl.SetDocument(itsSmartMetDocumentInterface);
	itsGridCtrl.SetWmoIdFilterManager(&itsWmoIdFilterManager);
	itsGridCtrl.MoveWindow(CalcClientArea(), FALSE);
	fGridCtrlInitialized = true;
	CTime aTime;
	itsMinMaxTimePicker.GetTime(aTime);
	CTime aTime2(aTime.GetYear(), aTime.GetMonth(), aTime.GetDay(), aTime.GetHour(), 0, 0);
	itsMinMaxTimePicker.SetTime(&aTime2);

	if(itsSmartMetDocumentInterface->Language() == kFinnish)
		itsMinMaxDatePicker.SetFormat(_TEXT("dd.MM.yyyy"));
	else
		itsMinMaxDatePicker.SetFormat(_TEXT("dd/MM/yyyy"));

	// laitetaan isompi ja bold fontti aika ja asema lkm teksti static kontrolliin
    LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));
    lf.lfWeight = FW_BOLD;
	lf.lfHeight = 16;
	itsTimeAndStationTextFont.CreateFontIndirect(&lf);
	CWnd *win = GetDlgItem(IDC_STATIC_TIME_STR);
	if(win)
		win->SetFont(&itsTimeAndStationTextFont);
	EnableDisableControls();
    ForcedUpdate();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiSynopDataGridViewDlg::EnableDisableControls(void)
{
	if(fMinMaxModeOn)
	{ // pit‰‰ n‰ytt‰‰ tietyt kontrollit, jotka on muuten piilotettu
		CWnd *win = GetDlgItem(IDC_EDIT_DAY_COUNT);
		if(win)
			win->ShowWindow(SW_SHOW);
		win = GetDlgItem(IDC_DATETIMEPICKER_MINMAX_DATE);
		if(win)
			win->ShowWindow(SW_SHOW);
		win = GetDlgItem(IDC_DATETIMEPICKER_MINMAX_TIME);
		if(win)
			win->ShowWindow(SW_SHOW);
		win = GetDlgItem(IDC_STATIC_D_STR);
		if(win)
			win->ShowWindow(SW_SHOW);
	}
	else
	{ // jos ei olla minmax-moodissa, piilotetaan tietyt kontrollit
		CWnd *win = GetDlgItem(IDC_EDIT_DAY_COUNT);
		if(win)
			win->ShowWindow(SW_HIDE);
		win = GetDlgItem(IDC_DATETIMEPICKER_MINMAX_DATE);
		if(win)
			win->ShowWindow(SW_HIDE);
		win = GetDlgItem(IDC_DATETIMEPICKER_MINMAX_TIME);
		if(win)
			win->ShowWindow(SW_HIDE);
		win = GetDlgItem(IDC_STATIC_D_STR);
		if(win)
			win->ShowWindow(SW_HIDE);
	}
}

void CFmiSynopDataGridViewDlg::InitProducerSelector(void)
{
	itsProducerList.clear();

	// editoitu ja synop tuottajat laitetaan listaan ensin
	itsProducerList.push_back(NFmiProducerHelperInfo(::GetDictionaryString("StationDataTableViewEditedProducerName"), kFmiMETEOR, NFmiInfoData::kEditable));
	itsProducerList.push_back(NFmiProducerHelperInfo("Synop", kFmiSYNOP, NFmiInfoData::kObservations));

	// loput tuottajat otetaan ProducerSystemist‰
    auto &producerSystem = itsSmartMetDocumentInterface->ProducerSystem();
	int modelCount = static_cast<int>(producerSystem.Producers().size());
	int i=0;
	for(i=0; i<modelCount; i++)
	{
		NFmiProducerHelperInfo helperInfo = NFmiProducerHelperInfo::MakeProducerHelperInfo(producerSystem.Producer(i+1), NFmiInfoData::kViewable);  // +1 johtuu producersystemin 1-pohjaisesta indeksi systeemist‰
		if(helperInfo.itsProducerId == 999) // t‰m‰ on tosi rumaa hardcodea, mutta virallinen editoitu data (hyv‰ksytty automaatioon) muutetaan luettaessa tuottajaksi 999 ja sen data-tyyppi on kepadata
			helperInfo.itsDataType = NFmiInfoData::kKepaData; 

		itsProducerList.push_back(helperInfo);
	}

	itsProducerSelector.ResetContent();
	int ssize = static_cast<int>(itsProducerList.size());
	for(int i=0; i<ssize; i++)
        itsProducerSelector.AddString(CA2T(itsProducerList[i].itsName.c_str()));
	itsProducerSelector.SetCurSel(1); // laitetaan synop oletus tuottajaksi, koska se kiinnostaa eniten ja on nopein
	UpdateData(FALSE);
}

void CFmiSynopDataGridViewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(fGridCtrlInitialized)
		itsGridCtrl.MoveWindow(CalcClientArea(), FALSE);
	Invalidate(FALSE);
}


CRect CFmiSynopDataGridViewDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CWnd *win = GetDlgItem(IDC_STATIC_TIME_STR);
	if(win)
	{
		CRect rect2;
		win->GetWindowRect(rect2);
		CPoint pt(rect2.BottomRight());
		this->ScreenToClient(&pt);
		rect.top = rect.top + pt.y; // rect2.Height();
	}
	else
		rect.top = rect.top + 42;
	return rect;
}

BOOL CFmiSynopDataGridViewDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
//	return CDialog::OnEraseBkgnd(pDC);
}

void CFmiSynopDataGridViewDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

 // t‰m‰ on pika viritys, kun muuten Print (ja muiden ) -nappulan kohdalta j‰‰ kaista maalaamatta kun laitoin ikkunaan v‰lkkym‰ttˆm‰n p‰ivityksen
	CBrush brush(RGB(239, 235, 222));
	CRect area(CalcOtherArea());
	dc.FillRect(&area, &brush);
}

CRect CFmiSynopDataGridViewDlg::CalcOtherArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CRect gridRect(CalcClientArea());
	rect.bottom = gridRect.top + 0;
	return rect;
}

void CFmiSynopDataGridViewDlg::OnBnClickedButtonPrint()
{
//	itsGridCtrl.EnableWysiwygPrinting(TRUE); // wysiwyg tekee liian isoa j‰lke‰, ja jakaa mm. sarakkeet kahdelle sivulle
	itsGridCtrl.Print(); // t‰h‰n tulee defaulttina footeria ja headeria ja "ei wysiwygi‰", mik‰ on ehk‰ parempi
}

static int GetWantedDecimalCount(FmiParameterName parId)
{
	switch(parId)
	{
	case kFmiWindDirection:
	case kFmiPresentWeather:
	case kFmiTotalCloudCover:
	case kFmiVisibility:
	case kFmiPressureTendency:
	case kFmiSnowDepth:
	case kFmiWeatherSymbol1:
	case kFmiCloudHeight:
	case kFmiLowCloudCover:
		return 0;
	default:
		return 1;
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

static const char * GetWmoIdString(const NFmiLocation &theLocation)
{
	static NFmiString wmoIdStr;
	wmoIdStr = "";
	if(theLocation.GetIdent() < 10000)
		wmoIdStr += "0"; // etu nolla laitetaan tarvittaessa
	wmoIdStr += ::Value2String(static_cast<float>(theLocation.GetIdent()), 0);
	return wmoIdStr;
}

static bool isWwColumnParamId(FmiParameterName parId)
{
    return parId == kFmiPresentWeather || parId == kFmiWeatherSymbol1;
}

static string GetStringValue(float value, int wantedDecimalCount, FmiParameterName parId)
{
    if(::isWwColumnParamId(parId))
        return CtrlViewUtils::GetSynopCodeAsSynopFontText(value);
    else
    {
        if(value == kFloatMissing)
            return string("-");
        else
        {
            return string(Value2String(value, wantedDecimalCount));
        }
    }
}

// Oletus: ei lˆytynyt kastepistett‰ datasta, katsotaan voidaanko se laskea l‰mpˆtilan ja kosteuden avulla
static float DoDewPointChecksFor(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime)
{
	if(theInfo->Param(kFmiTemperature))
	{
		float T = theInfo->InterpolatedValue(theLatlon, theTime);
		if(theInfo->Param(kFmiHumidity))
		{
			float RH = theInfo->InterpolatedValue(theLatlon, theTime);
			return static_cast<float>(NFmiSoundingFunctions::CalcDP(T, RH));
		}
	}
	return false;
}

// Oletus: ei lˆytynyt kastepistett‰ datasta, katsotaan voidaanko se laskea l‰mpˆtilan ja kosteuden avulla
static float DoDewPointChecksObs(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	if(theInfo->Param(kFmiTemperature))
	{
		float T = theInfo->FloatValue();
		if(theInfo->Param(kFmiHumidity))
		{
			float RH = theInfo->FloatValue();
			return static_cast<float>(NFmiSoundingFunctions::CalcDP(T, RH));
		}
	}
	return false;
}

static float GetFinalModelDataValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName parId, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, NFmiFastInfoUtils::MetaWindParamUsage *metaWindParamUsage)
{
    if(metaWindParamUsage && metaWindParamUsage->ParamNeedsMetaCalculations(parId))
    {
        // Kun tullaan t‰h‰n kohtaan, oletetaan ett‰ halutaan WS/WD parametreja, mutta datasta lˆytyy vain tuulen u ja v komponentit
        NFmiFastInfoUtils::QueryInfoParamStateRestorer restorer(*theInfo);
        theInfo->Param(kFmiWindUMS);
        float u = theInfo->InterpolatedValue(theLatlon, theTime);
        theInfo->Param(kFmiWindVMS);
        float v = theInfo->InterpolatedValue(theLatlon, theTime);
        if(parId == kFmiWindSpeedMS)
            return NFmiFastInfoUtils::CalcWS(u, v);
        else if(parId == kFmiWindDirection)
            return NFmiFastInfoUtils::CalcWD(u, v);
        else
            return kFloatMissing;
    }
    else
        return theInfo->InterpolatedValue(theLatlon, theTime);
}

static float GetParamValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName parId, const NFmiLocation *theLocation, const NFmiMetTime &theTime, bool useForecast, NFmiFastInfoUtils::MetaWindParamUsage *metaWindParamUsage)
{
	float value = kFloatMissing;
	if(useForecast)
	{
		switch(parId)
		{
		case kFmiPressureChange: // t‰m‰ lasketaan ennusteista paineen 3h erotuksella
			{
				theInfo->Param(kFmiPressure); // pit‰‰ laittaa paine p‰‰lle, ett‰ voidaan laskea paineen muutos
				NFmiMetTime previousTime(theTime);
				previousTime.ChangeByHours(-3);
				float previousValue = theInfo->InterpolatedValue(theLocation->GetLocation(), previousTime);
				float currentValue = theInfo->InterpolatedValue(theLocation->GetLocation(), theTime);
				if(previousValue != kFloatMissing && currentValue != kFloatMissing)
					value = currentValue - previousValue;
				break;
			}
		case kFmiMinimumTemperature:
		case kFmiMaximumTemperature:
			{
				if(theTime.GetHour() == 6 || theTime.GetHour() == 18)
				{
					theInfo->Param(kFmiTemperature); // pit‰‰ laittaa l‰mpˆtila p‰‰lle, ett‰ voidaan laskea minimi/maksimi l‰mpˆtiloja
					NFmiMetTime aTime(theTime);
					aTime.ChangeByHours(-12);
					NFmiDataModifierMinMax calculator;
					for(int i = 0; i <= 12; i++)
					{
						calculator.Calculate(theInfo->InterpolatedValue(theLocation->GetLocation(), aTime));
						aTime.ChangeByHours(1);
					}
					if(parId == kFmiMinimumTemperature)
						value = calculator.MinValue();
					else
						value = calculator.MaxValue();
				}
				break;
			}
		case kFmiPrecipitation1h:
			{  // lasketaan oikeasti 12 sade summa
				if(theTime.GetHour() == 6 || theTime.GetHour() == 18)
				{
					NFmiMetTime aTime(theTime);
					aTime.ChangeByHours(-12);
					NFmiDataModifierSum calculator;
					for(int i = 0; i <= 12; i++)
					{
						calculator.Calculate(theInfo->InterpolatedValue(theLocation->GetLocation(), aTime));
						aTime.ChangeByHours(1);
					}
					value = calculator.CalculationResult();
				}
				break;
			}
		default:
			value = ::GetFinalModelDataValue(theInfo, parId, theLocation->GetLocation(), theTime, metaWindParamUsage);
			// Jos kyseess‰ oli dew-point prametri id=10, ja oli puuttuvaa, koetetaan laskea arvo l‰mpˆtilan id=4 ja kosteuden id=13 avulla
			if(parId == kFmiDewPoint && value == kFloatMissing)
				value = ::DoDewPointChecksFor(theInfo, theLocation->GetLocation(), theTime);
			break;
		}
	}
	else
	{
		if(parId == kFmiPressureChange) // t‰m‰ lasketaan havainnoista paineen 3h erotuksella
		{
			theInfo->Param(kFmiPressure); // pit‰‰ laittaa paine p‰‰lle, ett‰ voidaan laskea paineen muutos
			int timeOffset = static_cast<int>(gPressureChangePreviousTimeIndex) - static_cast<int>(theInfo->TimeIndex());
			float previousValue = theInfo->PeekTimeValue(timeOffset);
			float currentValue = theInfo->FloatValue();
			if(previousValue != kFloatMissing && currentValue != kFloatMissing)
				value = currentValue - previousValue;
		}
		else
			value = theInfo->FloatValue();
		// Jos kyseess‰ oli dew-point prametri id=10, ja oli puuttuvaa, koetetaan laskea arvo l‰mpˆtilan id=4 ja kosteuden id=13 avulla
		if(parId == kFmiDewPoint && value == kFloatMissing)
			value = ::DoDewPointChecksObs(theInfo);
	}
	return value;
}

static bool IsLatestGoodValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, float value)
{
	if(value == kFloatMissing)
		return false;
	else
	{
		unsigned long oldtimeIndex = theInfo->TimeIndex();
		for( ; theInfo->NextTime() ; ) // tutkitaan lˆytyykˆ myˆhemmin kunnon dataa t‰lle asemalle t‰lt‰ parametrilta
		{
			if(theInfo->FloatValue() != kFloatMissing)
			{
				theInfo->TimeIndex(oldtimeIndex);
				return false;
			}
		}
		theInfo->TimeIndex(oldtimeIndex);
		return true;
	}
}

static bool HaveToSetSynopFont(const LOGFONT *gridCellFont)
{
    if(gridCellFont)
    {
        if(_tcsncmp(gridCellFont->lfFaceName, gSynopFont.lfFaceName, LF_FACESIZE - 1) == 0)
            return false;
    }
    return true;
}

static void DoSynopFontSetup(NFmiGridCtrl &theGridCtrl, int row, int column, float value)
{
    if(::HaveToSetSynopFont(theGridCtrl.GetItemFont(row, column)))
        theGridCtrl.SetItemFont(row, column, &gSynopFont);
    auto synopCodeColor = CtrlViewUtils::GetSynopCodeSymbolColor(value);
    theGridCtrl.SetItemFgColour(row, column, CtrlView::Color2ColorRef(synopCodeColor));
}

static void DoSynopWeatherCellFixes(NFmiGridCtrl &theGridCtrl, FmiParameterName parId, int row, int column, float *valueInOut)
{
    if(::isWwColumnParamId(parId))
    {
        *valueInOut = ::ConvertPossible_WaWa_2_WW(*valueInOut);
        ::DoSynopFontSetup(theGridCtrl, row, column, *valueInOut);
    }
}

static void SetGridSellValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiGridCtrl &theGridCtrl, FmiParameterName parId, int row, int column, const NFmiLocation *theLocation, const NFmiMetTime &theTime, bool useForecast, NFmiFastInfoUtils::MetaWindParamUsage *metaWindParamUsage)
{
	float value = GetParamValue(theInfo, parId, theLocation, theTime, useForecast, metaWindParamUsage);
    ::DoSynopWeatherCellFixes(theGridCtrl, parId, row, column, &value);
	bool isLatestGoodValue = false;
	if(!useForecast)
		isLatestGoodValue = IsLatestGoodValue(theInfo, value);
	string str(GetStringValue(value, GetWantedDecimalCount(parId), parId));
    theGridCtrl.SetItemText(row, column, CA2T(str.c_str()));
	theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
	if(isLatestGoodValue)
		theGridCtrl.SetItemBkColour(row, column, gLastGoodSynopBkColor);
	else if(useForecast)
		theGridCtrl.SetItemBkColour(row, column, gForecastBkColor);
	else
		theGridCtrl.SetItemBkColour(row, column, gNormalSynopBkColor);
}

static void SetParamIndexGridSellValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiGridCtrl &theGridCtrl, unsigned long parIndex, int row, int column, FmiParameterName parId, const NFmiLocation *theLocation, const NFmiMetTime &theTime, bool useForecast)
{
    // Jos parametri asetetaan pelk‰st‰‰n indeksin avulla (nopeaa), ei tarvitse tehd‰ tuulen metaparametri tarkasteluja
	theInfo->ParamIndex(parIndex);
	SetGridSellValues(theInfo, theGridCtrl, parId, row, column, theLocation, theTime, useForecast, nullptr);
}
static void SetParamIdGridSellValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiGridCtrl &theGridCtrl, FmiParameterName parId, int row, int column, const NFmiLocation *theLocation, const NFmiMetTime &theTime, bool useForecast, NFmiFastInfoUtils::MetaWindParamUsage *metaWindParamUsage)
{
	theInfo->Param(parId);
	SetGridSellValues(theInfo, theGridCtrl, parId, row, column, theLocation, theTime, useForecast, metaWindParamUsage);
}

class ValueSearcher
{
public:
	virtual bool SearchValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, const NFmiLocation *theLocation, bool useForecast) = 0;
	virtual void Clear(void) = 0;
	virtual float Value(void) = 0;
	virtual unsigned long TimeIndex(void) = 0;
};

class SumValueSearcher : public ValueSearcher
{
public:
	SumValueSearcher(void)
	:ValueSearcher()
	,itsSumValue(kFloatMissing)
	,fUse1hRRCalculationFactor(false)
	{}
	void Use1hRRCalculationFactor(bool newState){fUse1hRRCalculationFactor=newState;}

	bool SearchValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, const NFmiLocation *theLocation, bool useForecast)
	{
		theInfo->TimeIndex(theStartTimeIndex);
		do
		{
			float value = useForecast ? theInfo->InterpolatedValue(theLocation->GetLocation()) : theInfo->FloatValue();
			if(value != kFloatMissing)
			{
				// summaa k‰ytet‰‰n vain sade summan laskuun ja se pit‰‰ kertoa aika-askeleen m‰‰r‰ll‰, koska rr on nyt aina tunti sadetta
				float precipitationFactor = fUse1hRRCalculationFactor ? theInfo->TimeResolution()/60.f : 1.f;
				if(itsSumValue == kFloatMissing)
					itsSumValue = value * precipitationFactor;
				else
					itsSumValue += value * precipitationFactor;
			}
			theInfo->NextTime();
		} while(theInfo->TimeIndex() < theEndTimeIndex);
		return itsSumValue != kFloatMissing;
	}

	void Clear(void)
	{
		itsSumValue = kFloatMissing;
	}
	float Value(void) {return itsSumValue;}
	unsigned long TimeIndex(void) {return static_cast<unsigned long >(-1);}

private:
	float itsSumValue;
	bool fUse1hRRCalculationFactor; // speciaali tapaus ennusteissa olevien sateiden summat, pit‰‰ kertoa aika askeleen tunti m‰‰r‰ll‰
};

class MinValueSearcher : public ValueSearcher
{
public:
	const static float MinStartValue;
	MinValueSearcher(void)
	:ValueSearcher()
	,itsMinValue(MinStartValue)
	,itsMinValueTimeIndex(static_cast<unsigned long >(-1))
	{
	}

	bool SearchValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, const NFmiLocation *theLocation, bool useForecast)
	{
		theInfo->TimeIndex(theStartTimeIndex);
		do
		{
			float value = useForecast ? theInfo->InterpolatedValue(theLocation->GetLocation()) : theInfo->FloatValue();
			if(value != kFloatMissing)
			{
				if(itsMinValue > value)
				{
					itsMinValue = value;
					itsMinValueTimeIndex = theInfo->TimeIndex();
				}
			}
			theInfo->NextTime();
		} while(theInfo->TimeIndex() < theEndTimeIndex);
		return itsMinValue != MinStartValue;
	}

	void Clear(void)
	{
		itsMinValue = MinStartValue;
		itsMinValueTimeIndex = static_cast<unsigned long >(-1);
	}
	float Value(void) {return itsMinValue;}
	unsigned long TimeIndex(void) {return itsMinValueTimeIndex;}

private:
	float itsMinValue;
	unsigned long itsMinValueTimeIndex;
};

const float MinValueSearcher::MinStartValue = 3.4E+38f;

class MaxValueSearcher : public ValueSearcher
{
public:
	const static float MaxStartValue;
	MaxValueSearcher(void)
	:ValueSearcher()
	,itsMaxValue(MaxStartValue)
	,itsMaxValueTimeIndex(static_cast<unsigned long >(-1))
	{
	}

	bool SearchValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, const NFmiLocation *theLocation, bool useForecast)
	{
		theInfo->TimeIndex(theStartTimeIndex);
		do
		{
			float value = useForecast ? theInfo->InterpolatedValue(theLocation->GetLocation()) : theInfo->FloatValue();
			if(value != kFloatMissing)
			{
				if(itsMaxValue < value)
				{
					itsMaxValue = value;
					itsMaxValueTimeIndex = theInfo->TimeIndex();
				}
			}
			theInfo->NextTime();
		} while(theInfo->TimeIndex() < theEndTimeIndex);
		return itsMaxValue != MaxStartValue;
	}

	void Clear(void)
	{
		itsMaxValue = MaxStartValue;
		itsMaxValueTimeIndex = static_cast<unsigned long >(-1);
	}
	float Value(void) {return itsMaxValue;}
	unsigned long TimeIndex(void) {return itsMaxValueTimeIndex;}

private:
	float itsMaxValue;
	unsigned long itsMaxValueTimeIndex;
};
const float MaxValueSearcher::MaxStartValue = -3.4E+38f;

static float GetMinMaxParamValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiLocation *theLocation, bool useForecast, unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, const HeaderParInfo &theHeaderParInfo, map<unsigned long, string> &theTimeDateMap, string &theFoundDateString)
{
	static MinValueSearcher minValueSearcher;
	static MaxValueSearcher maxValueSearcher;
	static SumValueSearcher sumValueSearcher;

	ValueSearcher *valueSearcher = &minValueSearcher;
	float value = kFloatMissing;
	theFoundDateString = "-";
	switch(theHeaderParInfo.itsRangeCalculationFunction)
	{
	case HeaderParInfo::kMin:
		valueSearcher = &minValueSearcher;
		break;
	case HeaderParInfo::kMax:
		valueSearcher = &maxValueSearcher;
		break;
	case HeaderParInfo::kSum:
		if(useForecast && theHeaderParInfo.itsParId == kFmiPrecipitation1h)
			sumValueSearcher.Use1hRRCalculationFactor(true);
		else
			sumValueSearcher.Use1hRRCalculationFactor(false);
		valueSearcher = &sumValueSearcher;
		break;
	}

	valueSearcher->Clear();
	if(valueSearcher->SearchValue(theInfo, theStartTimeIndex, theEndTimeIndex, theLocation, useForecast))
	{
		value = valueSearcher->Value();
		if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kMin || theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kMax)
		{
			map<unsigned long, string>::iterator it = theTimeDateMap.find(valueSearcher->TimeIndex());
			if(it != theTimeDateMap.end())
				theFoundDateString = (*it).second;
			else // pit‰‰ laskea kyseisen ajan aika stringi
			{
				theInfo->TimeIndex(valueSearcher->TimeIndex());
				theFoundDateString = GetMinMaxDateString(theInfo->Time());
				theTimeDateMap.insert(make_pair(valueSearcher->TimeIndex(), theFoundDateString));
			}
		}
	}
	else
		value = kFloatMissing;

	return value;
}

static void SetMinMaxGridSellValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiGridCtrl &theGridCtrl, int row, int column, const NFmiLocation *theLocation, bool useForecast, unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, const HeaderParInfo &theHeaderParInfo, int theDateColumn, map<unsigned long, string> &theTimeDateMap)
{
	string foundDateString;
	float value = GetMinMaxParamValue(theInfo, theLocation, useForecast, theStartTimeIndex, theEndTimeIndex, theHeaderParInfo, theTimeDateMap, foundDateString);
    ::DoSynopWeatherCellFixes(theGridCtrl, theHeaderParInfo.itsParId, row, column, &value);
    string str(GetStringValue(value, GetWantedDecimalCount(theHeaderParInfo.itsParId), theHeaderParInfo.itsParId));
    theGridCtrl.SetItemText(row, column, CA2T(str.c_str()));
	theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
	if(useForecast)
		theGridCtrl.SetItemBkColour(row, column, gForecastBkColor);
	else
		theGridCtrl.SetItemBkColour(row, column, gNormalSynopBkColor);

	// jos tietyn tyyppinen parametri kentt‰, siihen liittyv‰ aika leima pit‰‰ laittaa
	// osoitettuun aikaleima columniin
	if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kMin || theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kMax)
	{
        theGridCtrl.SetItemText(row, theDateColumn, CA2T(foundDateString.c_str()));
		theGridCtrl.SetItemState(row, theDateColumn, theGridCtrl.GetItemState(row, theDateColumn) | GVIS_READONLY);
		if(useForecast)
			theGridCtrl.SetItemBkColour(row, theDateColumn, gForecastBkColor);
		else
			theGridCtrl.SetItemBkColour(row, theDateColumn, gNormalSynopBkColor);
	}
}

static void SetMinMaxParamData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiGridCtrl &theGridCtrl, const HeaderParInfo &theHeaderParInfo, int row, int column, const NFmiLocation *theLocation, bool useForecast, unsigned long theStartTimeIndex, unsigned long theEndTimeIndex, map<unsigned long, string> &theTimeDateMap)
{
	if(row < 0 || row >= theGridCtrl.GetRowCount())
		return ; // t‰m‰ voisi laittaa heitt‰m‰‰n poikkeuksen tai tekem‰‰n yhden kerran lokiin error-viestin

	if(theHeaderParInfo.itsParId == kFmiBadParameter)
	{
		if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kRowNumber)
		{ // t‰h‰n tulee j‰rjestys numero fixed columniin
            theGridCtrl.SetItemText(row, column, CA2T(Value2String(row, 0)));
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		else if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kWmoId)
		{
            theGridCtrl.SetItemText(row, column, CA2T(::GetWmoIdString(*theLocation)));
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		else if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kStationName)
		{
            theGridCtrl.SetItemText(row, column, ::convertPossibleUtf8StringToWideString(std::string(theLocation->GetName())).c_str());
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		else if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kLon)
		{
            theGridCtrl.SetItemText(row, column, CA2T(GetStringValue(static_cast<float>(theLocation->GetLongitude()), 3, kFmiBadParameter).c_str()));
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		else if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kLat)
		{
            theGridCtrl.SetItemText(row, column, CA2T(GetStringValue(static_cast<float>(theLocation->GetLatitude()), 3, kFmiBadParameter).c_str()));
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		// lopuksi s‰‰det‰‰n erikois solujen tausta v‰ri
		if(theHeaderParInfo.itsRangeCalculationFunction != HeaderParInfo::kRowNumber) // row number column on s‰‰detty fixediksi, ja sille m‰‰r‰t‰‰n v‰ritys toisaalla
		{
			if(useForecast)
				theGridCtrl.SetItemBkColour(row, column, gForecastBkColor);
			else
				theGridCtrl.SetItemBkColour(row, column, gNormalSynopBkColor);
		}
	}
	else
	{
		if(theHeaderParInfo.fUseParIndex)
			theInfo->ParamIndex(theHeaderParInfo.itsParIndex);
		else
			theInfo->Param(theHeaderParInfo.itsParId);

		SetMinMaxGridSellValues(theInfo, theGridCtrl, row, column, theLocation, useForecast, theStartTimeIndex, theEndTimeIndex, theHeaderParInfo, column+1, theTimeDateMap);
	}
}

static void SetParamData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiGridCtrl &theGridCtrl, const HeaderParInfo &theHeaderParInfo, int row, int column, const NFmiLocation *theLocation, const NFmiMetTime &theTime, bool useForecast, NFmiFastInfoUtils::MetaWindParamUsage *metaWindParamUsage)
{
	if(row < 0 || row >= theGridCtrl.GetRowCount())
		return ; // t‰m‰ voisi laittaa heitt‰m‰‰n poikkeuksen tai tekem‰‰n yhden kerran lokiin error-viestin

	if(theHeaderParInfo.itsParId == kFmiBadParameter)
	{ // PAHAA HARDCODEA muutamalle erikois tapaukselle eli asema id ja nimi tapauksille ja lon/lat
		if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kRowNumber)
		{ // t‰h‰n tulee j‰rjestys numero fixed columniin
            theGridCtrl.SetItemText(row, column, CA2T(Value2String(row, 0)));
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		else if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kWmoId)
		{
            theGridCtrl.SetItemText(row, column, CA2T(::GetWmoIdString(*theLocation)));
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		else if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kStationName)
		{
            auto unicodeLocationName = ::convertPossibleUtf8StringToWideString(std::string(theLocation->GetName()));
            theGridCtrl.SetItemText(row, column, unicodeLocationName.c_str());
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		else if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kLon)
		{
            theGridCtrl.SetItemText(row, column, CA2T(GetStringValue(static_cast<float>(theLocation->GetLongitude()), 3, kFmiBadParameter).c_str()));
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		else if(theHeaderParInfo.itsRangeCalculationFunction == HeaderParInfo::kLat)
		{
            theGridCtrl.SetItemText(row, column, CA2T(GetStringValue(static_cast<float>(theLocation->GetLatitude()), 3, kFmiBadParameter).c_str()));
			theGridCtrl.SetItemState(row, column, theGridCtrl.GetItemState(row, column) | GVIS_READONLY);
		}
		// lopuksi s‰‰det‰‰n erikois solujen tausta v‰ri
		if(column != 0) // column 0 on s‰‰detty fixediksi, ja sille m‰‰r‰t‰‰n v‰ritys toisaalla
		{
			if(useForecast)
				theGridCtrl.SetItemBkColour(row, column, gForecastBkColor);
			else
				theGridCtrl.SetItemBkColour(row, column, gNormalSynopBkColor);
		}
	}
	else if(theHeaderParInfo.fUseParIndex)
		SetParamIndexGridSellValue(theInfo, theGridCtrl, theHeaderParInfo.itsParIndex, row, column, theHeaderParInfo.itsParId, theLocation, theTime, useForecast);
	else
		SetParamIdGridSellValue(theInfo, theGridCtrl, theHeaderParInfo.itsParId, row, column, theLocation, theTime, useForecast, metaWindParamUsage);
}

static void SetHeaders(NFmiGridCtrl &theGridCtrl, const std::vector<HeaderParInfo> &theHeaders, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount)
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

std::vector<boost::shared_ptr<NFmiFastQueryInfo> > CFmiSynopDataGridViewDlg::GetWantedSynopInfos(void)
{
	UpdateData(TRUE);

	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector = itsSmartMetDocumentInterface->GetSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed, kFmiSHIP, kFmiBUOY);
	if(infoVector.size() == 0)
	{ // jos ei lˆytynyt havainnoista synop dataa, tarkastetaan editoitava data, onko se asema dataa
		boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->EditedSmartInfo();
		if(info)
			if(!info->IsGrid())
				infoVector.push_back(info);
	}
	return infoVector;
}

static int CalcSuitableParameters(boost::shared_ptr<NFmiFastQueryInfo> &info, const std::vector<FmiParameterName> &wantedParameters)
{
    NFmiFastInfoUtils::QueryInfoParamStateRestorer restorer(*info);
    int suitableParametercounter = 0;
    for(auto paramId : wantedParameters)
    {
        if(info->Param(paramId))
            suitableParametercounter++;
    }
    return suitableParametercounter;
}

static boost::shared_ptr<NFmiFastQueryInfo> GetBestSuitableData(std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &infos)
{
    if(infos.size() == 0)
        return boost::shared_ptr<NFmiFastQueryInfo>();
    else if(infos.size() == 1)
        return infos[0]; // palauta ainoa info listalta
    else
    {
        // palauta sopivin info listalta, miss‰ niist‰ on eniten haluttuja parametreja
        std::vector<FmiParameterName> wantedParameters{ kFmiTemperature, kFmiDewPoint, kFmiWindDirection, kFmiWindSpeedMS, kFmiMaximumWind, kFmiPresentWeather, kFmiPrecipitationAmount, kFmiTotalCloudCover, kFmiCloudHeight, kFmiLowCloudCover, kFmiVisibility, kFmiSnowDepth, kFmiPressure, kFmiPressureTendency, kFmiPressureChange, kFmiMinimumTemperature, kFmiMaximumTemperature, kFmiGroundTemperature };

        boost::shared_ptr<NFmiFastQueryInfo> bestSuitableInfo;
        int mostSuitableParameters = 0;
        for(auto &info : infos)
        {
            auto suitableParameters = ::CalcSuitableParameters(info, wantedParameters);
            if(mostSuitableParameters < suitableParameters)
            {
                bestSuitableInfo = info;
                mostSuitableParameters = suitableParameters;
            }
        }
        return bestSuitableInfo;
    }
}

// t‰m‰ on ik‰v‰ kopio CZeditmap2View-luokasta, t‰m‰ pit‰isi laittaa
// dokumenttiin, ett‰ saataisiin yhtenev‰ funktio molemmissa paikoissa
boost::shared_ptr<NFmiFastQueryInfo> CFmiSynopDataGridViewDlg::GetWantedInfo(bool fGetObsStationData)
{
	UpdateData(TRUE);
    auto *infoOrganizer = itsSmartMetDocumentInterface->InfoOrganizer();
	if(fGetObsStationData)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer->FindInfo(NFmiInfoData::kObservations, NFmiProducer(kFmiSYNOP), true);
		if(info == 0)
		{ // jos ei lˆytynyt havainnoista synop dataa, tarkastetaan editoitava data, onko se asema dataa
			info = itsSmartMetDocumentInterface->EditedSmartInfo();
			if(info)
				if(info->IsGrid())
					info = boost::shared_ptr<NFmiFastQueryInfo>(); // ei haluta hila dataa, laitetaan 0-pointteri t‰h‰n
		}
		return info;
	}
	else
	{
		int selProd = itsProducerSelector.GetCurSel();
		std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVec = infoOrganizer->GetInfos(
																						itsProducerList[selProd].itsDataType,
																						itsProducerList[selProd].fGroundData,
																						itsProducerList[selProd].itsProducerId);
        return ::GetBestSuitableData(infoVec);
	}
}

int g_BasicColumnWidthUnit = 18;
int g_RowNumberColumnWidth = static_cast<int>(g_BasicColumnWidthUnit * 2.5);
int g_WmoIdColumnWidth = static_cast<int>(g_BasicColumnWidthUnit * 3.0);
int g_StationNameColumnWidth = static_cast<int>(g_BasicColumnWidthUnit * 8.0);
int g_LatlonColumnWidth = static_cast<int>(g_BasicColumnWidthUnit*2.9);

void CFmiSynopDataGridViewDlg::InitSynopHeaders(void)
{
	itsSynopHeaders.clear();
	std::string numberStr = ::GetDictionaryString("StationDataTableViewNumberAbreviation");
	itsSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, numberStr, false, g_RowNumberColumnWidth, HeaderParInfo::kRowNumber));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, "WmoID", false, g_WmoIdColumnWidth, HeaderParInfo::kWmoId));
	std::string stationStr = ::GetDictionaryString("NormalWordCapitalStation");
	itsSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, stationStr, false, g_StationNameColumnWidth, HeaderParInfo::kStationName));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiTemperature, "T", true, static_cast<int>(g_BasicColumnWidthUnit*2.3)));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiDewPoint, "Td", true, static_cast<int>(g_BasicColumnWidthUnit*2.3)));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiWindDirection, "dd", false, g_BasicColumnWidthUnit *2));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiWindSpeedMS, "ff(ms)", false, static_cast<int>(g_BasicColumnWidthUnit*2.4)));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiMaximumWind, "fx(ms)", false, static_cast<int>(g_BasicColumnWidthUnit*2.4)));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiPrecipitationAmount, "rr", true, g_BasicColumnWidthUnit *2));
    // HUOM! s‰‰symboli parametrin (kFmiPresentWeather tai kFmiWeatherSymbol1) pit‰‰ olla aina samassa sarakkeessa (fontin s‰‰tˆ juttu), nyt 10. sarake
    itsSynopHeaders.push_back(HeaderParInfo(kFmiPresentWeather, "ww", true, g_BasicColumnWidthUnit * 2));
    itsSynopHeaders.push_back(HeaderParInfo(kFmiTotalCloudCover, "N", true, g_BasicColumnWidthUnit *2));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiCloudHeight, "h", true, g_BasicColumnWidthUnit *1));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiLowCloudCover, "Nh", true, static_cast<int>(g_BasicColumnWidthUnit*1.8)));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiVisibility, "V", true, g_BasicColumnWidthUnit *3));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiSnowDepth, "SD", true, g_BasicColumnWidthUnit *2));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiPressure, "PPPP", true, g_BasicColumnWidthUnit *3));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiPressureTendency, "a", true, g_BasicColumnWidthUnit *1));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiPressureChange, "ppp", true, g_BasicColumnWidthUnit *2));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiMinimumTemperature, "Tmin", true, static_cast<int>(g_BasicColumnWidthUnit*2.3)));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiMaximumTemperature, "Tmax", true, static_cast<int>(g_BasicColumnWidthUnit*2.3)));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiGroundTemperature, "Tg", true, g_BasicColumnWidthUnit *2));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, "Lon", false, g_LatlonColumnWidth, HeaderParInfo::kLon));
	itsSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, "Lat", false, g_LatlonColumnWidth, HeaderParInfo::kLat));
}

void CFmiSynopDataGridViewDlg::InitForecastSynopHeaders(void)
{
	itsForecastSynopHeaders.clear();
	std::string numberStr = ::GetDictionaryString("StationDataTableViewNumberAbreviation");
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, numberStr, false, g_RowNumberColumnWidth, HeaderParInfo::kRowNumber));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, "WmoID", false, g_WmoIdColumnWidth, HeaderParInfo::kWmoId));
	std::string stationStr = ::GetDictionaryString("NormalWordCapitalStation");
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, stationStr, false, g_StationNameColumnWidth, HeaderParInfo::kStationName));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiTemperature, "T", true, static_cast<int>(g_BasicColumnWidthUnit*2.3)));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiDewPoint, "Td", true, static_cast<int>(g_BasicColumnWidthUnit*2.3)));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiWindDirection, "dd", false, g_BasicColumnWidthUnit *2));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiWindSpeedMS, "ff(ms)", false, static_cast<int>(g_BasicColumnWidthUnit*2.4)));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiMaximumWind, "fx(ms)", false, static_cast<int>(g_BasicColumnWidthUnit*2.4)));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiPrecipitation1h, "rr", false, g_BasicColumnWidthUnit *2));
    // HUOM! s‰‰symboli parametrin (kFmiPresentWeather tai kFmiWeatherSymbol1) pit‰‰ olla aina samassa sarakkeessa (fontin s‰‰tˆ juttu), nyt 10. sarake
    itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiWeatherSymbol1, "ww", false, g_BasicColumnWidthUnit * 2)); // t‰ss‰ haetaan hsade1 ww tilalle
    itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiTotalCloudCover, "N", false, g_BasicColumnWidthUnit *2));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiCloudHeight, "h", true, g_BasicColumnWidthUnit *2));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiLowCloudCover, "Nh", false, static_cast<int>(g_BasicColumnWidthUnit*1.8)));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiVisibility, "V", true, g_BasicColumnWidthUnit *3));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiSnowDepth, "SD", true, g_BasicColumnWidthUnit *2));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiPressure, "PPPP", true, g_BasicColumnWidthUnit *3));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiPressureTendency, "a", true, g_BasicColumnWidthUnit *1));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiPressureChange, "ppp", true, g_BasicColumnWidthUnit *2));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiMinimumTemperature, "Tmin", true, static_cast<int>(g_BasicColumnWidthUnit*2.3)));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiMaximumTemperature, "Tmax", true, static_cast<int>(g_BasicColumnWidthUnit*2.3)));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiGroundTemperature, "Tg", true, g_BasicColumnWidthUnit *2));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, "Lon", false, g_LatlonColumnWidth, HeaderParInfo::kLon));
	itsForecastSynopHeaders.push_back(HeaderParInfo(kFmiBadParameter, "Lat", false, g_LatlonColumnWidth, HeaderParInfo::kLat));
}

void CFmiSynopDataGridViewDlg::InitMinMaxDataHeaders(void)
{
	itsMinMaxDataHeaders.clear();
	std::string numberStr = ::GetDictionaryString("StationDataTableViewNumberAbreviation");
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, numberStr, false, g_RowNumberColumnWidth, HeaderParInfo::kRowNumber)); // MUISTA SetParamData-special kohta
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, "WmoID", false, g_WmoIdColumnWidth, HeaderParInfo::kWmoId)); // MUISTA SetParamData-special kohta
	std::string stationStr = ::GetDictionaryString("NormalWordCapitalStation");
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, stationStr, false, g_StationNameColumnWidth, HeaderParInfo::kStationName)); // MUISTA SetParamData-special kohta
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiTemperature, "Tmin", true, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kMin));
	std::string timeStr = ::GetDictionaryString("NormalWordCapitalTime");
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiTemperature, "Tmax", true, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kMax));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiWindSpeedMS, "fx", false, static_cast<int>(g_BasicColumnWidthUnit*2.4), HeaderParInfo::kMax));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
    // HUOM! s‰‰symboli parametrin (kFmiPresentWeather tai kFmiWeatherSymbol1) pit‰‰ olla aina samassa sarakkeessa (fontin s‰‰tˆ juttu), nyt 10. sarake
    itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiPresentWeather, "wwmax", false, g_BasicColumnWidthUnit *2, HeaderParInfo::kMax)); // t‰ss‰ haetaan hsade1 ww tilalle
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiPrecipitationAmount, "rrsum", false, g_BasicColumnWidthUnit *2, HeaderParInfo::kSum));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiVisibility, "Vmin", true, g_BasicColumnWidthUnit *3, HeaderParInfo::kMin));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiSnowDepth, "SDmax", true, g_BasicColumnWidthUnit *2, HeaderParInfo::kMax));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, "Lon", false, g_LatlonColumnWidth, HeaderParInfo::kLon)); // MUISTA SetParamData-special kohta
	itsMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, "Lat", false, g_LatlonColumnWidth, HeaderParInfo::kLat)); // MUISTA SetParamData-special kohta
}

void CFmiSynopDataGridViewDlg::InitForecastMinMaxDataHeaders(void)
{
	itsForecastMinMaxDataHeaders.clear();
	std::string numberStr = ::GetDictionaryString("StationDataTableViewNumberAbreviation");
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, numberStr, false, g_RowNumberColumnWidth, HeaderParInfo::kRowNumber)); // MUISTA SetParamData-special kohta
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, "WmoID", false, g_WmoIdColumnWidth, HeaderParInfo::kWmoId)); // MUISTA SetParamData-special kohta
	std::string stationStr = ::GetDictionaryString("NormalWordCapitalStation");
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, stationStr, false, g_StationNameColumnWidth, HeaderParInfo::kStationName)); // MUISTA SetParamData-special kohta
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiTemperature, "Tmin", true, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kMin));
	std::string timeStr = ::GetDictionaryString("NormalWordCapitalTime");
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiTemperature, "Tmax", true, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kMax));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiWindSpeedMS, "fx", false, static_cast<int>(g_BasicColumnWidthUnit*2.4), HeaderParInfo::kMax));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
    // HUOM! s‰‰symboli parametrin (kFmiPresentWeather tai kFmiWeatherSymbol1) pit‰‰ olla aina samassa sarakkeessa (fontin s‰‰tˆ juttu), nyt 10. sarake
    itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiWeatherSymbol1, "wwmax", false, g_BasicColumnWidthUnit *2, HeaderParInfo::kMax)); // t‰ss‰ haetaan hsade1 ww tilalle
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiPrecipitation1h, "rrsum", false, g_BasicColumnWidthUnit *2, HeaderParInfo::kSum));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiVisibility, "Vmin", true, g_BasicColumnWidthUnit *3, HeaderParInfo::kMin));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiSnowDepth, "SDmax", true, g_BasicColumnWidthUnit *2, HeaderParInfo::kMax));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, timeStr, false, static_cast<int>(g_BasicColumnWidthUnit*2.3), HeaderParInfo::kDateAndTime));
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, "Lon", false, g_LatlonColumnWidth, HeaderParInfo::kLon)); // MUISTA SetParamData-special kohta
	itsForecastMinMaxDataHeaders.push_back(HeaderParInfo(kFmiBadParameter, "Lat", false, g_LatlonColumnWidth, HeaderParInfo::kLat)); // MUISTA SetParamData-special kohta
}

void CFmiSynopDataGridViewDlg::TakeWantedHeadersInUse(int theProducerId)
{
	static std::vector<HeaderParInfo> dummyHeaders;
	itsUsedHeaders = &dummyHeaders;
	if(fMinMaxModeOn)
	{
		if(theProducerId == kFmiSYNOP)
			itsUsedHeaders = &itsMinMaxDataHeaders;
		else if(theProducerId == kFmiHIRLAM)
			itsUsedHeaders = &itsForecastMinMaxDataHeaders;
	}
	else
	{
		if(theProducerId == kFmiSYNOP)
			itsUsedHeaders = &itsSynopHeaders;
		else if(theProducerId == kFmiHIRLAM)
			itsUsedHeaders = &itsForecastSynopHeaders;
	}
	itsGridCtrl.SetUsedHeaders(itsUsedHeaders);
}

const std::string& CFmiSynopDataGridViewDlg::GetProducerString(void) const
{
	int selProd = itsProducerSelector.GetCurSel();
	return itsProducerList[selProd].itsName;
}

int CFmiSynopDataGridViewDlg::GetMaxStationCount(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfos)
{
	int count = 0;
	for(unsigned int i=0; i<theInfos.size(); i++)
		count += theInfos[i]->SizeLocations();

	return count;
}

int CFmiSynopDataGridViewDlg::GetMaxStationCount(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	int count = 0;
	if(theInfo)
		count = theInfo->SizeLocations();
	return count;
}



static bool IsForecastData(const struct NFmiProducerHelperInfo &theProdInfo, boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	if(theInfo)
	{
		if(theProdInfo.itsDataType == NFmiInfoData::kViewable || theProdInfo.itsDataType == NFmiInfoData::kKepaData)
			return true;
		if(theProdInfo.itsDataType == NFmiInfoData::kObservations)
			return false;
		if(theProdInfo.itsDataType == NFmiInfoData::kEditable)
		{
			if(theInfo->Grid() == 0)
				return false; // jos ei ollut hila dataa, ei ollut ennuste dataa (ennuste datan pit‰‰ t‰ss‰ olla hilassa)
			else
				return true;
		}
	}
	return false;
}

static bool HasAnyMatchingTimes(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfos, const NFmiMetTime &theWantedTime)
{
	for(unsigned int i = 0; i < theInfos.size(); i++)
	{
		if(theInfos[i]->Time(theWantedTime))
			return true;
	}
	return false;
}

static std::vector<int> GetWidthVector(NFmiGridCtrl &theGridCtrl)
{
	vector<int> headerColumnWidths;

	int ssize = theGridCtrl.GetColumnCount();
	for(int i=0; i<ssize;i++)
		headerColumnWidths.push_back(theGridCtrl.GetColumnWidth(i));

	return headerColumnWidths;
}

static void SetFromWidthVector(const std::vector<int> theWidthVector, std::vector<HeaderParInfo> *theUsedHeaders)
{
	if(theUsedHeaders)
	{
		std::vector<HeaderParInfo> &usedHeaders = *theUsedHeaders;
		size_t ssize1 = usedHeaders.size();
		size_t ssize2 = theWidthVector.size();
		size_t usedSize = FmiMin(ssize1, ssize2);
		for(std::vector<HeaderParInfo>::size_type i=0; i<usedSize; i++)
			usedHeaders[i].itsColumnWidth = theWidthVector[i];
	}
}

void CFmiSynopDataGridViewDlg::FillSynopDataGridViewMacro(NFmiViewSettingMacro::SynopDataGridView &theViewMacro)
{
	UpdateData(TRUE);

	theViewMacro.AllCountriesSelected(this->itsWmoIdFilterManager.AllSelected());
	theViewMacro.DayRangeValue(this->itsDayRangeValue);
	theViewMacro.MinMaxModeOn(this->fMinMaxModeOn == TRUE);
	theViewMacro.MinMaxRangeStartTime(this->itsMinMaxRangeStartTime);
	UpdateMinMaxTimePickers();


	theViewMacro.SelectedCountryAbbrStr(itsWmoIdFilterManager.GetSelectedCountryAbbrStr());

	int selProd = itsProducerSelector.GetCurSel();
	NFmiProducer prod;
	if(selProd >= 0 && selProd < static_cast<int>(itsProducerList.size()))
		prod = NFmiProducer(itsProducerList[selProd].itsProducerId);
	theViewMacro.SelectedProducer(prod);
	theViewMacro.HeaderColumnWidthsInPixels(::GetWidthVector(itsGridCtrl));
}

void CFmiSynopDataGridViewDlg::ApplySynopDataGridViewMacro(NFmiViewSettingMacro::SynopDataGridView &theViewMacro)
{
    itsSmartMetDocumentInterface->SynopDataGridViewOn(theViewMacro.ViewStatus().ShowWindow()); // t‰m‰kin on asetettava, arvo saatiin CZeditmap2Doc-luokassa tutkimalla ikkunan tilaa

	this->itsWmoIdFilterManager.AllSelected(theViewMacro.AllCountriesSelected());
	this->itsDayRangeValue = theViewMacro.DayRangeValue();
	this->fMinMaxModeOn = theViewMacro.MinMaxModeOn();
	this->itsMinMaxRangeStartTime = theViewMacro.MinMaxRangeStartTime();
	UpdateMinMaxTimePickers();

	itsWmoIdFilterManager.SetSelectedCountryAbbrStr(theViewMacro.SelectedCountryAbbrStr());

	SelectProducer(theViewMacro.SelectedProducer());
	EnableDisableControls();

	TakeWantedHeadersInUse(theViewMacro.SelectedProducer().GetIdent()); // pit‰‰ ensin ottaa haluttu headeri k‰yttˆˆn ett‰ s‰‰det‰‰n sen sarake leveyksi‰
	::SetFromWidthVector(theViewMacro.HeaderColumnWidthsInPixels(), itsUsedHeaders);
	fUpdateHeadersAfterViewMacroLoad = true;

	UpdateData(FALSE);
}

void CFmiSynopDataGridViewDlg::SelectProducer(const NFmiProducer &theProducer)
{
	int ssize = static_cast<int>(itsProducerList.size());
	for(int i=0; i<ssize; i++)
	{
		if(itsProducerList[i].itsProducerId == theProducer.GetIdent())
		{
			itsProducerSelector.SetCurSel(i);
			break;
		}
	}
}

static bool IsAnydataInWantedRange(const NFmiTimeDescriptor &theDataTimes, const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime)
{
	if(theDataTimes.IsInside(theStartTime))
		return true;
	if(theDataTimes.IsInside(theEndTime))
		return true;
	// jos alku jsa loppu aika ovat datan aikojen ulkona eri puolilla, on dataa lˆydett‰v‰n‰
	if(theDataTimes.FirstTime() > theStartTime && theDataTimes.LastTime() < theEndTime)
		return true;
	return false; // haluttu aikajakso on data aikojen ulkona kokonaan toisella puolella (takana tai edess‰)
}

static bool IsDataInMinMaxRange(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime, unsigned long &theStartTimeIndex, unsigned long &theEndTimeIndex, bool fDoSadeDataFix = false)
{
	theStartTimeIndex = static_cast<unsigned long>(-1);
	theEndTimeIndex = static_cast<unsigned long>(-1);
	bool dataInRange = IsAnydataInWantedRange(theInfo->TimeDescriptor(), theStartTime, theEndTime);
	if(theInfo->FindNearestTime(theStartTime, kForward))
		theStartTimeIndex = theInfo->TimeIndex();
	if(fDoSadeDataFix)
	{
		NFmiMetTime currentTime;
		if(theStartTimeIndex == theInfo->SizeTimes() - 1 && theInfo->TimeDescriptor().LastTime() > currentTime)
			theStartTimeIndex = static_cast<unsigned long>(-1); // sade-datassa viimeisin aika menee huomiselle asti, johtuu luultavasti ett‰ par-haussa on step 24h, t‰st‰ johtuen smartmet voi luulla ett‰ sadedataa on viimeisell‰ aika-stepill‰, vaikka sit‰ ei ole
	}

	if(theInfo->FindNearestTime(theEndTime, kBackward))
		theEndTimeIndex = theInfo->TimeIndex();
	return dataInRange;
}


static int CalcUsedStationCount(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfos, const boost::shared_ptr<NFmiArea> &theZoomedArea, WmoIdFilterManager &theWmoIdFilterManager, const NFmiMetTime &theWantedTime, const NFmiMetTime &theEndTime, bool fDoForecast, bool fDoMinMax)
{
	int stationsFound = 0;
	for(unsigned int x = 0; x < theInfos.size(); x++)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = theInfos[x];

		if(fDoForecast || fDoMinMax || info->Time(theWantedTime))
		{
			bool doShipData = NFmiFastInfoUtils::IsInfoShipTypeData(*info);
			if(doShipData && (fDoForecast || fDoMinMax))
				continue;

			for(info->ResetLocation(); info->NextLocation(); )
			{
				if(doShipData)
				{
					NFmiPoint latlon = info->GetLatlonFromData();
					if(theZoomedArea->IsInside(latlon) == false)
						continue;
				}
				else
				{
					if(theZoomedArea->IsInside(info->LatLon()) == false)
						continue;
					const NFmiLocation *location = info->Location();
					if(theWmoIdFilterManager.AllSelected() == false)
						if(theWmoIdFilterManager.ShowStation(location->GetIdent()) == false)
							continue;
				}
				stationsFound++;
			}
		}
	}
	return stationsFound;
}

// Synop data on ainoa havainto data tuottajalistassa, joten jos valittu 
// tuottaja ei ole synop, on se mallidataa.
bool CFmiSynopDataGridViewDlg::IsSelectedProducerModelData() const
{
    return itsProducerList[itsProducerSelector.GetCurSel()].itsProducerId != kFmiSYNOP;
}

bool CFmiSynopDataGridViewDlg::GridControlNeedsUpdate(const std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &obsInfos, const boost::shared_ptr<NFmiFastQueryInfo> &usedInfo)
{
    const std::string baseFunctionNameForLogging = "Station-data-grid-view";

    if(!MapDraw::mapIsNotDirty(itsSmartMetDocumentInterface, 0))
    {
        if(CatLog::doTraceLevelLogging())
        {
            std::string message = baseFunctionNameForLogging;
            message += ": 'forced' update to grid-control due main-map-view's area changed";
            CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization);
        }
        return true;
    }

    const auto &wantedTime = GetMainMapViewTime();
    SynopDataGridViewUsedFileNames usedFileNames(obsInfos, usedInfo, wantedTime);
    bool modelDataCase = IsSelectedProducerModelData();
    bool minMaxModeUsed = fMinMaxModeOn == TRUE;
    if(itsUsedFileNames.IsUpdateNeeded(usedFileNames, modelDataCase, minMaxModeUsed))
    {
        if(CatLog::doTraceLevelLogging())
        {
            std::string message = baseFunctionNameForLogging;
            if(itsUsedFileNames.Empty())
            {
                message += ": 'forced' update to grid-control view's content";
                CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization);
            }
            else if(itsUsedFileNames.IsUpdateNeededDueTimeChange(usedFileNames, minMaxModeUsed))
            {
                message += ": time was changed to ";
                message += wantedTime.ToStr("YYYY.MM.DD HH:mm", kEnglish);
                CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization);
            }
            else
            {
                message += ": update needed due following file(s) changed: ";
                message += itsUsedFileNames.GetChangedFileNames(usedFileNames, modelDataCase);
                CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization);
            }
        }

        // Tiedostojen mukaan tarvitaan taulukon p‰ivitys, otetaan uusin tiedosto nimilista talteen ja palautetaan true
        itsUsedFileNames = usedFileNames;
        return true;
    }

    if(CatLog::doTraceLevelLogging())
    {
        std::string message = baseFunctionNameForLogging;
        message += ": none of used data have changed, no need to update grid-control view";
        CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization);
    }
    return false;
}

void CFmiSynopDataGridViewDlg::MakeNextUpdateForced()
{
    itsUsedFileNames.Clear();
}

void CFmiSynopDataGridViewDlg::ForcedUpdate()
{
    MakeNextUpdateForced();
    Update();
}

const NFmiMetTime& CFmiSynopDataGridViewDlg::GetMainMapViewTime() const
{
    return itsSmartMetDocumentInterface->CurrentTime(itsMapViewDescTopIndex);
}

void CFmiSynopDataGridViewDlg::Update(void)
{
    static bool fFirstTime = true; // sarakkeiden s‰‰tˆ tehd‰‰n vain 1. kerran

    if(IsWindowVisible() && !IsIconic()) // N‰yttˆ‰ p‰ivitet‰‰n vain jos se on n‰kyviss‰ ja se ei ole minimized tilassa
    {
        CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, __FUNCTION__);
        CCellID focusCell = itsGridCtrl.GetFocusCell();

        if(fUpdateHeadersAfterViewMacroLoad)
        {
            fUpdateHeadersAfterViewMacroLoad = false;
            fFirstTime = true;
        }
        if(itsSmartMetDocumentInterface->SynopDataGridViewOn() == false)
            return;
        const auto &wantedTime = GetMainMapViewTime();
        int fixedRowCount = 1;
        int fixedColumnCount = 1;

        std::vector<boost::shared_ptr<NFmiFastQueryInfo> > obsInfos = GetWantedSynopInfos();
        boost::shared_ptr<NFmiFastQueryInfo> sadeInfo = itsSmartMetDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kObservations, NFmiProducer(10002), true);
        fUseSadeData = sadeInfo && IsSadeDataUsed(); // jos katsellaan synop-tuottajaa, silloin laitetaan myˆs sadedataa mukaan
        boost::shared_ptr<NFmiFastQueryInfo> usedInfo = GetWantedInfo(false);
        if(!GridControlNeedsUpdate(obsInfos, usedInfo))
            return;
        // kuinka monta asemaa datassa on aktivoitu nykyiseen zoomattuun alueeseen
        int maxStationCount = GetMaxStationCount(obsInfos);
        int stationCount = 0;
        int sadeStationCount = 0;
        if(fUseSadeData)
        {
            if(obsInfos.size() > 0 && !::HasAnyMatchingTimes(obsInfos, wantedTime))
            {
                maxStationCount = 0;
            }
            if(sadeInfo && sadeInfo->Time(wantedTime))
            {
                sadeStationCount = GetMaxStationCount(sadeInfo);
                maxStationCount += sadeStationCount;
            }
        }
        int maxRowCount = maxStationCount + fixedRowCount;  // +fixedRowCount on tilaa headerille

        itsGridCtrl.MinMaxModeOn(fMinMaxModeOn);
        itsGridCtrl.DayRangeValue(itsDayRangeValue);

        int selProd = itsProducerSelector.GetCurSel();
        bool forecastDataExist = ::IsForecastData(itsProducerList[selProd], usedInfo);

        int realStationCount = 0;
        if(maxStationCount != 0)
        {
            NFmiMetTime startTime = fMinMaxModeOn ? itsMinMaxRangeStartTime : wantedTime;
            NFmiMetTime endTime(GetMinMaxEndTime(startTime, itsDayRangeValue));

            realStationCount = CalcUsedStationCount(obsInfos, itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapHandler()->Area(), itsWmoIdFilterManager, startTime, endTime, forecastDataExist, fMinMaxModeOn == TRUE);
            if(sadeStationCount == 0 && fMinMaxModeOn)
            {
                if(fUseSadeData && sadeInfo && ::IsAnydataInWantedRange(sadeInfo->TimeDescriptor(), startTime, endTime))
                    sadeStationCount = GetMaxStationCount(sadeInfo);
            }
            realStationCount += sadeStationCount;
        }

        itsGridCtrl.SetProducerString(GetProducerString()); // pit‰‰ laittaa printtausta varten tuottaja talteen
        if(obsInfos.size() > 0 && usedInfo && fMinMaxModeOn)
        {
            FillGridWithMinMaxData(obsInfos, usedInfo, maxRowCount, fFirstTime, fixedRowCount, fixedColumnCount, sadeInfo, realStationCount);
            stationCount = realStationCount;

            if(stationCount == 0)
            { // jos lopputuloksena oli tyhj‰ eli dataa ei lˆytynyt tms, pit‰‰ taulukko nollata
                SetHeaders(itsGridCtrl, *itsUsedHeaders, fixedRowCount, fFirstTime, fixedRowCount, fixedColumnCount); // t‰h‰n pit‰‰ antaa myˆs rowcounttiin fixedCount, koska muuten asmia on, mutta ei niihin laitettavaa dataa
            }
        }
        else
        {
            bool timeFound = usedInfo ? usedInfo->Time(wantedTime) : false;
            if(timeFound == false)
            {
                for(size_t i = 0; i < obsInfos.size(); i++)
                    if(obsInfos[i]->Time(wantedTime))
                        timeFound = true;
            }

            // pistet‰‰n aika ja asema lkm tiedot talteen, jos printataan t‰m‰ n‰yttˆ
            itsGridCtrl.LastDataTime(wantedTime);

            if(usedInfo == 0 || obsInfos.size() == 0 || maxRowCount <= fixedRowCount || ((fUseSadeData ? maxStationCount == 0 : timeFound == false) && forecastDataExist == false)) // jos ei ollut asemia, ei lˆytynyt aikaa => tyhj‰ gridi
            {
                stationCount = 0;
                TakeWantedHeadersInUse(kFmiSYNOP);
                SetHeaders(itsGridCtrl, *itsUsedHeaders, fixedRowCount, fFirstTime, fixedRowCount, fixedColumnCount); // t‰h‰n pit‰‰ antaa myˆs rowcounttiin fixedCount, koska muuten asmia on, mutta ei niihin laitettavaa dataa
            }
            else if(usedInfo && obsInfos.size() > 0 && forecastDataExist == true)
            {
                FillGridWithForecastData(obsInfos, usedInfo, maxRowCount, fFirstTime, fixedRowCount, fixedColumnCount, wantedTime, realStationCount);
                stationCount = realStationCount;
            }
            else // t‰ytet‰‰n taulukko havainto datalla
            {
                FillGridWithSynopData(obsInfos, maxRowCount, fFirstTime, fixedRowCount, fixedColumnCount, wantedTime, sadeInfo, realStationCount);
                stationCount = realStationCount;
            }
            itsTimeStrU_ = CA2T(GetTimeAndStationString(itsSmartMetDocumentInterface->Language(), wantedTime, stationCount, itsWmoIdFilterManager, GetProducerString()).c_str());
        }

        itsGridCtrl.LastStationCount(stationCount);
        itsGridCtrl.DoLastSort(); // jos ollaan tehty aiemmin jokin sorttaus, se halutaan tehd‰ nyt t‰ss‰ uudestaan
        EnsureVisibleStationRow(); // skrollataan viel‰ viimeisin valittu asema n‰kyviin
        itsGridCtrl.SetFocusCell(focusCell); // laitetaan fokus takaisin p‰‰lle, koska se h‰visi tuossa ruudun p‰ivityksess‰
        UpdateData(FALSE);
    }
}

bool CFmiSynopDataGridViewDlg::IsSadeDataUsed(void)
{
	int selProd = itsProducerSelector.GetCurSel();
	if(itsProducerList[selProd].itsProducerId == kFmiSYNOP)
		return true;
	return false;
}

void CFmiSynopDataGridViewDlg::FillGridWithMinMaxData(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theObsInfos, boost::shared_ptr<NFmiFastQueryInfo> &theUsedInfo, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, boost::shared_ptr<NFmiFastQueryInfo> &theSadeInfo, int &theRealStationCountInOut)
{
	bool useObsData = theUsedInfo->Grid() == 0; // t‰m‰ vaikuttaa siihen mit‰ smartinfoa k‰ytet‰‰n ja intepoloidaanko data havainto pisteeseen vai otetaanko arvo suoraan asema datan tapauksessa
	TakeWantedHeadersInUse(useObsData ? kFmiSYNOP : kFmiHIRLAM);
	int columnCount = static_cast<int>(itsUsedHeaders->size());

	NFmiMetTime startTime(itsMinMaxRangeStartTime);
	NFmiMetTime endTime(GetMinMaxEndTime(startTime, itsDayRangeValue));
	itsGridCtrl.LastDataTime(startTime);

	boost::shared_ptr<NFmiFastQueryInfo> reallyUsedDataInfo = useObsData ? theObsInfos[0] : theUsedInfo;

	unsigned long startTimeIndex;
	unsigned long endTimeIndex;
	bool anyDataInRange = ::IsDataInMinMaxRange(reallyUsedDataInfo, startTime, endTime, startTimeIndex, endTimeIndex);
	bool continueWithAnyData = anyDataInRange && startTimeIndex != static_cast<unsigned long>(-1) && endTimeIndex != static_cast<unsigned long>(-1);

	unsigned long sadeStartTimeIndex = static_cast<unsigned long>(-1);
	unsigned long sadeEndTimeIndex = static_cast<unsigned long>(-1);
	bool continueWithSadeData = false;
	if(fUseSadeData)
	{
		bool sadeDataInRange = ::IsDataInMinMaxRange(theSadeInfo, startTime, endTime, sadeStartTimeIndex, sadeEndTimeIndex, true);
		continueWithSadeData = sadeDataInRange && sadeStartTimeIndex != static_cast<unsigned long>(-1) && sadeEndTimeIndex != static_cast<unsigned long>(-1);
	}

	bool includeShipTypeData = false; //useObsData; // vain jos havainto data on valittu pohjaksi, laitetaan mukaan liikkuvat laiva tyyppiset datat

	SetHeaders(itsGridCtrl, *itsUsedHeaders, theRealStationCountInOut + theFixedRowCount, fFirstTime, theFixedRowCount, theFixedColumnCount);

	boost::shared_ptr<NFmiArea> zoomedArea = itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapHandler()->Area();

	map<unsigned long, string> timeDateMap; // t‰m‰ on optimointia, t‰h‰n talletetaan eri aikoja stringi muodossa aika indekseineen
	int stationsFound = 0;
	// obsInfo ja usedInfo on jo testattu, ne eiv‰t ole 0-pointtereita
	if(useObsData)
	{
		for(unsigned int x = 0; x < theObsInfos.size(); x++)
		{
			reallyUsedDataInfo = theObsInfos[x];

			bool doShipData = NFmiFastInfoUtils::IsInfoShipTypeData(*(reallyUsedDataInfo));
			if(doShipData && includeShipTypeData == false)
				continue; // liikkuville datoille on liian vaikeaa laittaa mukaan ennusteita

			::IsDataInMinMaxRange(reallyUsedDataInfo, startTime, endTime, startTimeIndex, endTimeIndex);

			if(startTimeIndex == static_cast<unsigned long>(-1) || endTimeIndex == static_cast<unsigned long>(-1))
				continue;

			if(continueWithSadeData == false && continueWithAnyData == false)
			{	// jos halutut ajat eiv‰t osu datan aikojen kanssa yhteen, tulee tyhj‰ taulukko
				SetHeaders(itsGridCtrl, *itsUsedHeaders, theFixedRowCount, fFirstTime, theFixedRowCount, theFixedColumnCount); // t‰h‰n pit‰‰ antaa myˆs rowcounttiin fixedCount, koska muuten asmia on, mutta ei niihin laitettavaa dataa
			}
			else
			{
				for(int i=0; i<columnCount; i++)
				{
					reallyUsedDataInfo->Param((*itsUsedHeaders)[i].itsParId);
					(*itsUsedHeaders)[i].itsParIndex = reallyUsedDataInfo->ParamIndex();
				}

				if(continueWithAnyData)
				{
					for(reallyUsedDataInfo->ResetLocation(); reallyUsedDataInfo->NextLocation(); )
					{
						if(zoomedArea->IsInside(reallyUsedDataInfo->LatLon()) == false)
							continue;

						const NFmiLocation *location = reallyUsedDataInfo->Location();
						if(itsWmoIdFilterManager.AllSelected() == false)
							if(itsWmoIdFilterManager.ShowStation(location->GetIdent()) == false)
								continue;

						for(int i=0; i<columnCount; i++)
							SetMinMaxParamData(reallyUsedDataInfo, itsGridCtrl, (*itsUsedHeaders)[i], stationsFound + theFixedRowCount, i, location, !useObsData, startTimeIndex, endTimeIndex, timeDateMap);
						stationsFound++;
					}
				}
			}
		}
	}
	else // not useObsData
	{
		reallyUsedDataInfo = theUsedInfo;
		::IsDataInMinMaxRange(reallyUsedDataInfo, startTime, endTime, startTimeIndex, endTimeIndex);

		if(continueWithSadeData == false && continueWithAnyData == false)
		{	// jos halutut ajat eiv‰t osu datan aikojen kanssa yhteen, tulee tyhj‰ taulukko
			SetHeaders(itsGridCtrl, *itsUsedHeaders, theFixedRowCount, fFirstTime, theFixedRowCount, theFixedColumnCount); // t‰h‰n pit‰‰ antaa myˆs rowcounttiin fixedCount, koska muuten asmia on, mutta ei niihin laitettavaa dataa
		}
		else
		{
			for(int i=0; i<columnCount; i++)
			{
				reallyUsedDataInfo->Param((*itsUsedHeaders)[i].itsParId);
				(*itsUsedHeaders)[i].itsParIndex = reallyUsedDataInfo->ParamIndex();
			}

			if(continueWithAnyData)
			{
				for(unsigned int k=0; k<theObsInfos.size(); k++)
				{
					boost::shared_ptr<NFmiFastQueryInfo> obsInfo = theObsInfos[k];
					bool doShipData = NFmiFastInfoUtils::IsInfoShipTypeData(*obsInfo);
					if(doShipData)
						continue; // liikkuville datoille on liian vaikeaa laittaa mukaan ennusteita

					for(obsInfo->ResetLocation(); obsInfo->NextLocation(); )
					{
						if(zoomedArea->IsInside(obsInfo->LatLon()) == false)
							continue;
						const NFmiLocation *location = obsInfo->Location();
						if(itsWmoIdFilterManager.AllSelected() == false)
							if(itsWmoIdFilterManager.ShowStation(location->GetIdent()) == false)
								continue;

						for(int i=0; i<columnCount; i++)
							SetMinMaxParamData(reallyUsedDataInfo, itsGridCtrl, (*itsUsedHeaders)[i], stationsFound + theFixedRowCount, i, location, !useObsData, startTimeIndex, endTimeIndex, timeDateMap);
						stationsFound++;
					}
				}
			}
		}
	}

	if(continueWithSadeData && useObsData) // HUOM! pit‰‰ olla myˆs obs-data moodissa ennen kuin sade-datan kanssa jatketaan
	{
		for(int i=0; i<columnCount; i++)
		{
			theSadeInfo->Param((*itsUsedHeaders)[i].itsParId);
			(*itsUsedHeaders)[i].itsParIndex = theSadeInfo->ParamIndex();
		}

		
		for(theSadeInfo->ResetLocation(); theSadeInfo->NextLocation(); )
		{
			if(zoomedArea->IsInside(theSadeInfo->LatLon()) == false)
				continue;

			const NFmiLocation *location = theSadeInfo->Location();
			if(itsWmoIdFilterManager.AllSelected() == false)
				if(itsWmoIdFilterManager.ShowStation(location->GetIdent()) == false)
					continue;

			for(int i=0; i<columnCount; i++)
				SetMinMaxParamData(theSadeInfo, itsGridCtrl, (*itsUsedHeaders)[i], stationsFound + theFixedRowCount, i, location, !useObsData, sadeStartTimeIndex, sadeEndTimeIndex, timeDateMap);
			stationsFound++;
		}
	}
	theRealStationCountInOut = stationsFound;
	itsGridCtrl.SetRowCount(stationsFound + theFixedRowCount); // asetetaan todellinen rivi m‰‰r‰ lopuksi
    itsTimeStrU_ = CA2T(GetMinMaxTimeRangeAndStationString(itsSmartMetDocumentInterface->Language(), startTime, itsDayRangeValue, theRealStationCountInOut, itsWmoIdFilterManager, GetProducerString()).c_str());
}

void CFmiSynopDataGridViewDlg::FillGridWithSynopData(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theObsInfos, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, const NFmiMetTime &theTime, boost::shared_ptr<NFmiFastQueryInfo> &theSadeInfo, int &theRealStationCountInOut)
{ // t‰ytet‰‰n taulukko havainto datalla
	static NFmiLocation shipDataLocation;
	TakeWantedHeadersInUse(kFmiSYNOP);
	SetHeaders(itsGridCtrl, (*itsUsedHeaders), theRealStationCountInOut + theFixedRowCount, fFirstTime, theFixedRowCount, theFixedColumnCount);
	int stationsFound = 0;
	int columnCount = static_cast<int>(itsUsedHeaders->size());
	boost::shared_ptr<NFmiArea> zoomedArea = itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapHandler()->Area();
	for(unsigned int x = 0; x < theObsInfos.size(); x++)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = theObsInfos[x];
		for(int i=0; i<columnCount; i++)
		{
			info->Param((*itsUsedHeaders)[i].itsParId);
			(*itsUsedHeaders)[i].itsParIndex = info->ParamIndex();
		}

		if(info->Time(theTime))
		{
            NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(info);
            unsigned long oldTimeIndex = info->TimeIndex();
			NFmiMetTime prevTime(info->Time());
			prevTime.ChangeByHours(-3);
			info->Time(prevTime);
			gPressureChangePreviousTimeIndex = info->TimeIndex();
			info->TimeIndex(oldTimeIndex);
			bool doShipData = NFmiFastInfoUtils::IsInfoShipTypeData(*info);

			for(info->ResetLocation(); info->NextLocation(); )
			{
				const NFmiLocation *location = info->Location();
				if(doShipData)
				{
					shipDataLocation = *location;
					NFmiPoint latlon = info->GetLatlonFromData();
					if(zoomedArea->IsInside(latlon) == false)
						continue;
					shipDataLocation.SetLongitude(latlon.X());
					shipDataLocation.SetLatitude(latlon.Y());
					location = &shipDataLocation;
				}
				else if(zoomedArea->IsInside(info->LatLon()) == false)
					continue;

				if(itsWmoIdFilterManager.AllSelected() == false)
					if(itsWmoIdFilterManager.ShowStation(location->GetIdent()) == false)
						continue;

				for(int i=0; i<columnCount; i++)
					SetParamData(info, itsGridCtrl, (*itsUsedHeaders)[i], stationsFound + theFixedRowCount, i, location, theTime, false, &metaWindParamUsage);
				stationsFound++;
			}
		}
	}

	if(theSadeInfo && theSadeInfo->Time(theTime))
	{ // t‰‰ll‰ lis‰t‰‰n synop tauluun sadeasemat sen mukaan kuin niit‰ voi laittaa, eli sadem‰‰r‰ ja lumen syvyys
	  // en tehnyt omaa funktiota, koska en jaksanut mietti‰ miten sen fiksuiten tekisi.
	  // Eli siis yll‰ synopdatalle ei tehd‰ samoja temppuja kuin sadedatalle, joten en miettinyt miten voisin
	  // erist‰‰ toiminnot yhten‰iseen funktioon, joille annettaisiin parametrina aina n‰m‰ eri datat.
        NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(theSadeInfo);
        for(int i=0; i<columnCount; i++)
		{
			theSadeInfo->Param((*itsUsedHeaders)[i].itsParId);
			(*itsUsedHeaders)[i].itsParIndex = theSadeInfo->ParamIndex();
		}

		for(theSadeInfo->ResetLocation(); theSadeInfo->NextLocation(); )
		{
			if(zoomedArea->IsInside(theSadeInfo->LatLon()) == false)
				continue;
			if(itsWmoIdFilterManager.AllSelected() == false)
				if(itsWmoIdFilterManager.ShowStation(theSadeInfo->Location()->GetIdent()) == false)
					continue;

			for(int i=0; i<columnCount; i++)
				SetParamData(theSadeInfo, itsGridCtrl, (*itsUsedHeaders)[i], stationsFound + theFixedRowCount, i, theSadeInfo->Location(), theTime, false, &metaWindParamUsage);
			stationsFound++;
		}
	}
	theRealStationCountInOut = stationsFound;
	itsGridCtrl.SetRowCount(stationsFound + theFixedRowCount); // asetetaan todellinen rivi m‰‰r‰ lopuksi
}

void CFmiSynopDataGridViewDlg::FillGridWithForecastData(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theObsInfos, boost::shared_ptr<NFmiFastQueryInfo> &theForInfo, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, const NFmiMetTime &theTime, int &theRealStationCountInOut)
{
	TakeWantedHeadersInUse(kFmiHIRLAM); // hilam-id on fixattu ennuste datoja varten yleens‰
	int columnCount = static_cast<int>(itsUsedHeaders->size());
	// t‰ytet‰‰n taulukko ennuste datalla
	for(int i=0; i<columnCount; i++)
	{
		theForInfo->Param((*itsUsedHeaders)[i].itsParId);
		(*itsUsedHeaders)[i].itsParIndex = theForInfo->ParamIndex();
	}

	if(theForInfo->TimeDescriptor().IsInside(theTime) == false)
	{
		SetHeaders(itsGridCtrl, *itsUsedHeaders, theFixedRowCount, fFirstTime, theFixedRowCount, theFixedColumnCount);
		return ;
	}

	SetHeaders(itsGridCtrl, *itsUsedHeaders, theRealStationCountInOut + theFixedRowCount, fFirstTime, theFixedRowCount, theFixedColumnCount);

    NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(theForInfo);
    boost::shared_ptr<NFmiArea> zoomedArea = itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapHandler()->Area();
	// K‰yd‰‰n l‰pi havainto datan asemat, mutta lasketaan niille ennuste datasta arvot
	int stationsFound = 0;
	for(unsigned int x = 0; x < theObsInfos.size(); x++)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = theObsInfos[x];

		bool doShipData = NFmiFastInfoUtils::IsInfoShipTypeData(*info);
		if(doShipData)
			continue; // liikkuville datoille on liian vaikeaa laittaa mukaan ennusteita

        for(info->ResetLocation(); info->NextLocation(); )
		{
			if(zoomedArea->IsInside(info->LatLon()) == false)
				continue;
			const NFmiLocation *location = info->Location();
			if(itsWmoIdFilterManager.AllSelected() == false)
				if(itsWmoIdFilterManager.ShowStation(location->GetIdent()) == false)
					continue;

			for(int i=0; i<columnCount; i++)
				SetParamData(theForInfo, itsGridCtrl, (*itsUsedHeaders)[i], stationsFound + theFixedRowCount, i, location, theTime, true, &metaWindParamUsage);
			stationsFound++;
		}
	}
	theRealStationCountInOut = stationsFound;
	itsGridCtrl.SetRowCount(stationsFound + theFixedRowCount); // asetetaan todellinen rivi m‰‰r‰ lopuksi
}

void CFmiSynopDataGridViewDlg::OnClose()
{
	DoWhenClosing();
	CDialog::OnClose();
}

void CFmiSynopDataGridViewDlg::DoWhenClosing(void)
{
    itsSmartMetDocumentInterface->SynopDataGridViewOn(false);
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
    itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(0, true, false); // p‰ivitet‰‰n vain p‰‰n‰ytˆn jutut (0 indeksi)
}

void CFmiSynopDataGridViewDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiSynopDataGridViewDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiSynopDataGridViewDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiSynopDataGridViewDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

int CFmiSynopDataGridViewDlg::FindStationRow(int theWmoId)
{
	if(theWmoId >= 0)
	{
        int wmoColumn = FindWantedHeaderColumn(itsUsedHeaders, HeaderParInfo::kWmoId);
        if(wmoColumn < 0)
            return -1;

        for(int i = itsGridCtrl.GetFixedRowCount(); i < itsGridCtrl.GetRowCount(); i++)
		{
			if(itsUsedHeaders == 0)
				return -1;
			int wmoId = _tstoi(itsGridCtrl.GetCell(i, wmoColumn)->GetText());
			if(theWmoId == wmoId)
				return i;
		}
	}
	return -1;
}

void CFmiSynopDataGridViewDlg::EnsureVisibleStationRow(void)
{
	CCellID cell(FindStationRow(itsSmartMetDocumentInterface->LastSelectedSynopWmoId()), 0);
	if(cell.row >= 0)
	{
		if(itsUsedHeaders == 0)
			return ;
		int lonColumn = FindWantedHeaderColumn(itsUsedHeaders, HeaderParInfo::kLon);
		int latColumn = FindWantedHeaderColumn(itsUsedHeaders, HeaderParInfo::kLat);
		if(lonColumn < 0 || latColumn < 0 )
			return ;
		itsGridCtrl.SetSelectedRange(cell.row, 0, cell.row, latColumn, FALSE);
		itsGridCtrl.EnsureVisible(cell);

		double lon = _tstof(itsGridCtrl.GetCell(cell.row, lonColumn)->GetText());
		double lat = _tstof(itsGridCtrl.GetCell(cell.row, latColumn)->GetText());
		NFmiPoint latlon(lon, lat);
		if(latlon != itsSmartMetDocumentInterface->GetSynopHighlightLatlon())
		{
            itsSmartMetDocumentInterface->SetHighlightedSynopStation(latlon, -1, true); // -1 on wmoid, mill‰ ei kait olekaan k‰yttˆ‰
		}
	}
	else
	{ // muuten nollataan highlight juttu
		NFmiPoint latlon;
        itsSmartMetDocumentInterface->SetHighlightedSynopStation(latlon, -1, false); // -1 on wmoid, mill‰ ei kait olekaan k‰yttˆ‰
	}
}

void CFmiSynopDataGridViewDlg::OnBnClickedButtonPreviousTime2()
{
	UpdateData(TRUE);
	if(fMinMaxModeOn)
	{
		itsMinMaxRangeStartTime.ChangeByHours(static_cast<long>(-itsDayRangeValue*24));
		UpdateMinMaxTimePickers();
	}
	else
        itsSmartMetDocumentInterface->SetDataToPreviousTime(itsMapViewDescTopIndex);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("SynopDataGridViewDlg: Previous time button pressed");
}

void CFmiSynopDataGridViewDlg::OnBnClickedButtonNextTime()
{
	UpdateData(TRUE);
	if(fMinMaxModeOn)
	{
		itsMinMaxRangeStartTime.ChangeByHours(static_cast<long>(itsDayRangeValue*24));
		UpdateMinMaxTimePickers();
	}
	else
        itsSmartMetDocumentInterface->SetDataToNextTime(itsMapViewDescTopIndex);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("SynopDataGridViewDlg: Next time button pressed");
}

void CFmiSynopDataGridViewDlg::WhenProducerRadioButtonClikked(void)
{
    ForcedUpdate();
}

static void SetCountryFilterValues(CountryFilter &theCountryFilter, char* shortName, char* longName, bool sharesIdRange, int low1, int high1, int low2 = -1, int high2 = -1, int low3 = -1, int high3 = -1, int low4 = -1, int high4 = -1, int low5 = -1, int high5 = -1, int low6 = -1, int high6 = -1, int low7 = -1, int high7 = -1, int low8 = -1, int high8 = -1, int low9 = -1, int high9 = -1, int low10 = -1, int high10 = -1)
{
	theCountryFilter = CountryFilter(); // nollataan country-filter
	theCountryFilter.itsShortName = shortName;
	theCountryFilter.itsLongName = longName;
	theCountryFilter.fSharesIdRange = sharesIdRange;
	theCountryFilter.itsWmoIdRanges.push_back(IdRange(low1, high1));
	if(low2 != -1 &&  high2 != -1)
		theCountryFilter.itsWmoIdRanges.push_back(IdRange(low2, high2));
	if(low3 != -1 &&  high3 != -1)
		theCountryFilter.itsWmoIdRanges.push_back(IdRange(low3, high3));
	if(low4 != -1 &&  high4 != -1)
		theCountryFilter.itsWmoIdRanges.push_back(IdRange(low4, high4));
	if(low5 != -1 &&  high5 != -1)
		theCountryFilter.itsWmoIdRanges.push_back(IdRange(low5, high5));
	if(low6 != -1 &&  high6 != -1)
		theCountryFilter.itsWmoIdRanges.push_back(IdRange(low6, high6));
	if(low7 != -1 &&  high7 != -1)
		theCountryFilter.itsWmoIdRanges.push_back(IdRange(low7, high7));
	if(low8 != -1 &&  high8 != -1)
		theCountryFilter.itsWmoIdRanges.push_back(IdRange(low8, high8));
	if(low9 != -1 &&  high9 != -1)
		theCountryFilter.itsWmoIdRanges.push_back(IdRange(low9, high9));
	if(low10 != -1 &&  high10 != -1)
		theCountryFilter.itsWmoIdRanges.push_back(IdRange(low10, high10));
}

void CFmiSynopDataGridViewDlg::OnBnClickedButtonCountryFilterDlg()
{
	CFmiCountryFilterDlg dlg(&itsWmoIdFilterManager, this);
	if(dlg.DoModal() == IDOK)
	{
        ForcedUpdate();
	}
}

void CFmiSynopDataGridViewDlg::InitWmoIdFilterManager(void)
{
	itsWmoIdFilterManager = WmoIdFilterManager(itsSmartMetDocumentInterface);
	CountryFilter cFilter;

	// Euroopan maita
	SetCountryFilterValues(cFilter, "FI", "Finland", false, 2700, 2999, 5000, 5999); // wmo sivuilla v‰itet‰‰n 2800-2900, mutta suomalaisilla asemailla on alempiakin numeroita
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SA", "Sadeasemat", false, 110000, 119999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NO", "Norway", false, 1000, 1499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SN", "Sweden", false, 2000, 2699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "UK", "United Kingdom", false, 3000, 3949);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "IE", "Ireland", false, 3950, 3999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "IL", "Iceland", false, 4000, 4099);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GL","GREENLAND", false, 4100, 4399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "FA","FAEROE ISLANDS", false, 6000, 6019);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "DN","DENMARK", false, 6020, 6199);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NL","NETHERLANDS", false, 6200, 6399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BX","BELGIUM", false, 6400, 6499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BX","LUXEMBOURG", false, 6500, 6599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SW","SWITZERLAND", false, 6600, 6799);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LI","LIECHTENSTEIN", false, 6900, 6999); // NOAA:lla oli LT, mutta se kuulu Liettualle, Lichtensteinin tunnus on oikeasti LI
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "FR","FRANCE", false, 7000, 7999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SP","SPAIN", false, 8000, 8490);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GI","GIBRALTAR", false, 8495, 8495);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "AZ","AZORES", false, 8500, 8519);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MD","MADEIRA ISLANDS", false, 8520, 8529);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "PO","PORTUGAL", false, 8530, 8579);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CV","CAPE VERDE", false, 8580, 8599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "DL","GERMANY", false, 10000, 10999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "OS","AUSTRIA", false, 11000, 11399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CZ","CZECH REPUBLIC", false, 11400, 11799);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LO","SLOVAKIA", false, 11800, 11999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "PL","POLAND", false, 12100, 12699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "HU","HUNGARY", false, 12700, 12999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "YG","YUGOSLAVIA", true, 13000, 13599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LJ","SLOVENIA", true, 13000, 13599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "RH","CROATIA", true, 13000, 13599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "AB","ALBANIA", false, 13600, 13699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "RO","ROMANIA", false, 15000, 15499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BU","BULGARIA", false, 15500, 15999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "IY","ITALY", false, 16000, 16589);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ML","MALTA", false, 16590, 16599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GR","GREECE", false, 16600, 16999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TU","TURKEY", false, 17000, 17599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CY","CYPRUS", false, 17600, 17699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	// Ven‰j‰n ja entisen Nuvostoliiton maita.
	// Ven‰j‰ pit‰isi jakaa viel‰ erikseen Russia, Kazakhstan, Azerbaijan, Georgia, Armenia, Uzbekistan, Kyrgyzstan, Turkmenistan, Tajikistan,
	// mutta se jako kannattaa tehd‰ mieleuummin NFmiWmoStationLookUpSystem -luokan avulla, mutta se vaatii lis‰‰ logiikkaa ja tyˆt‰.
	SetCountryFilterValues(cFilter, "RU","RUSSIA", false, 20000, 25956, 26059, 26099, 26157, 26196, 26258, 26298, 26976, 32618, 34000, 34399, 34720, 38999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	SetCountryFilterValues(cFilter, "EE","ESTONIA", false, 26029, 26058, 26115, 26145, 26214, 26226, 26231, 26233, 26242, 26249);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	SetCountryFilterValues(cFilter, "LV","LATVIA", false, 26229, 26230, 26238, 26239, 26313, 26348, 26403, 26446, 26544, 26545, 26551, 26552);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	SetCountryFilterValues(cFilter, "LT","LITHUANIA", false, 26502, 26531, 26547, 26548, 26600, 26634, 26713, 26737);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	SetCountryFilterValues(cFilter, "BY","BELARUS", false, 26554, 26555, 26643, 26668, 26745, 26774, 26825, 26878, 28887, 26888, 26923, 26966, 33008, 33041, 33105, 33106);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	SetCountryFilterValues(cFilter, "UR","UKRAIN", false, 33046, 33999, 34400, 34719);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	// Aasian maita
	SetCountryFilterValues(cFilter, "SY", "SYRIAN ARAB REPUBLIC", false, 40000, 40099);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LB", "LEBANON", false, 40100, 40149);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "IS", "ISRAEL", false, 40150, 40199);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "JD", "JORDAN", false, 40250, 40349);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SD", "SAUDI ARABIA", false, 40350, 40499, 41000, 41149);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "KW", "KUWAIT", false, 40500, 40599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "IQ", "IRAQ", false, 40600, 40699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "IR", "IRAN", false, 40700, 40899);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "AH", "AFGHANISTAN", false, 40900, 40999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "QT", "QATAR", false, 41160, 41179);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ER", "UNITED ARAB EMIRATES", false, 41180, 41239);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "OM", "OMAN", false, 41240, 41319);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "YE", "YEMEN", false, 41320, 41409);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "DY", "DEMOCRATIC YEMEN", false, 41410, 41499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "PK", "PAKISTAN", false, 41500, 41799);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BW", "BANGLADESH", false, 41800, 41999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "IN", "INDIA", false, 42000, 43999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SB", "SRI LANKA", false, 43400, 43499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MV", "MALDIVES", false, 43500, 43599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MO", "MONGOLIA", false, 44000, 44399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NP", "NEPAL", false, 44400, 44499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "HK", "HONG KONG", false, 45000, 45009, 45030, 45039);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MU", "MACAO", false, 45010, 45019);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TW", "TAIWAN", false, 46000, 46999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "KR", "DEMOCRATIC PEOPLE*S REPUBLIC OF KOREA (NORTH)", false, 47000, 47079);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "KO", "KOREA,REPUBLIC OF (SOUTH)", false, 47080, 47199);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "JP", "JAPAN", false, 47400, 47999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BM", "BURMA/MYANMAR", false, 48000, 48199);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TH", "THAILAND", false, 48300, 48599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MS", "MALAYSIA", false, 48600, 48689);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SR", "SINGAPORE", false, 48690, 46999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "VN", "VIETNAM", false, 48800, 48920);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LA", "LAO PEOPLE*S DEMOCRATIC REPUBLIC", false, 48930, 48959);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "KP", "DEMOCRATIC KAMPUCHEA", false, 48960, 48999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CI", "CHINA", false, 50000, 59999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BN", "BAHRAIN", false, 41150, 41150);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	// Afrikan maat
	SetCountryFilterValues(cFilter, "CR", "CANARY ISLAND (SPAIN)", false, 60000, 60049);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MC", "MOROCCO", false, 60050, 60319, 60340, 60349);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SP", "SPAIN", false, 60320, 60339);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "AL", "ALGERIA", false, 60350, 60699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TS", "TUNISIA", false, 60700, 60799);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NR", "NIGER", false, 61000, 61099);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MI", "MALI", false, 61100, 61299);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MT", "MAURITANIA", false, 61400, 61499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SG", "SENEGAL", false, 61600, 61699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GB", "GAMBIA", false, 61700, 61749);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GW", "GUINEA-BISSAU", false, 61750, 61799);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GN", "GUINEA", false, 61800, 61849);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SL", "SIERRA LEONE", false, 61850, 61899);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "AI", "ASCENSION ISLAND", false, 61902, 61903);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TP", "SAO TOME AND PRINCIPE", false, 61930, 61939);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "RE", "REUNION AND ASSOCIATED ISLANDS", false, 61968, 61972);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LY", "LIBYAN ARAB JAMAHIRIYA", false, 62000, 62299);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "EG", "EGYPT", false, 62300, 62499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SU", "SUDAN", false, 62600, 62999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ET", "ETHIOPIA", false, 63000, 63099, 63300, 63499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "DJ", "DJIBOUTI", false, 63100, 63149);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SI", "SOMALIA", false, 63150, 63299);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "UG", "UGANDA", true, 63600, 63979);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "KN", "KENYA", true, 63600, 63979);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TN", "TANZANIA,UNITED REPUBLIC OF", true, 63600, 63979);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SC", "SEYCHELLES", false, 63980, 63999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ZR", "ZAIRE", false, 64000, 64360);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "RW", "RWANDA", false, 64380, 64389);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BI", "BURUNDI", false, 64390, 64399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CG", "CONGO", false, 64000, 64499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GO", "GABON", false, 64500, 64599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CE", "CENTRAL AFRICAN REPUBLIC", false, 64600, 64699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CD", "CHAD", false, 64700, 64799);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GQ", "EQUATORIAL GUINEA", false, 64800, 64849);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CM", "CAMEROON,UNITED REPUBLIC OF", false, 64850, 64999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NI", "NIGERIA", false, 65000, 65299);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BJ", "BENIN", false, 65300, 65349);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TG", "TOGO", false, 65350, 65399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GH", "GHANA", false, 65400, 65499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "HV", "BURKINA FASO", false, 65500, 65524);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "IV", "IVORY COAST", false, 65525, 65599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LI", "LIBERIA", false, 65600, 65699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "AN", "ANGOLA", false, 66000, 66999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "IC", "COMOROS", false, 67000, 67005);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MG", "MADAGASCAR", false, 67009, 67199);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MZ", "MOZAMBIQUE", false, 67200, 67399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ZB", "ZAMBIA", true, 67400, 67753);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MW", "MALAWI", true, 67400, 67753, 67790, 67799);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ZW", "ZIMBABWE", false, 67755, 67789, 67800, 67999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NM", "NAMIBIA", true, 68000, 68349);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BC", "BOTSWANA", true, 68000, 68349);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ZA", "SOUTH AFRICA", true, 68000, 68989);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LS", "LESOTHO", true, 68450, 68459);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BV", "BOUVET ISLAND", false, 68992, 68992);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SV", "SWAZILAND", true, 68396, 68396);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "HE", "ST. HELENA", false, 61901, 61901);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BT", "BRITISH INDIAN OCEAN TERRITORY", false, 61967, 61967);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MA", "MAURITIUS", false, 61974, 61974);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	// Pohjois Amerikan maat
	SetCountryFilterValues(cFilter, "US","UNITED STATES OF AMERICA", false, 69000, 69999, 70000, 70999, 72000, 72999, 74000, 74999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "US", "UNITED STATES OF AMERICA (HAWAII)", false, 91000, 91199, 91260, 91299);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CN", "CANADA", false, 71000, 71999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MX", "MEXICO", false, 76000, 76999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BE", "BERMUDA", false, 78016, 78016);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BA", "BAHAMAS", false, 78050, 78199);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TI", "TURKS ISLANDS", false, 78118, 78118);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CU", "CUBA", false, 78200, 78379);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GC", "CAYMAN ISLAND", false, 78380, 78384);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "JM", "JAMAICA", false, 78385, 78399); // t‰m‰ Jamaikan lopetus koodi on muutettu konfliktin takia (oli 78999)
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "HA", "HAITI", false, 78400, 78449);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "DR", "DOMINICAN REPUBLIC", false, 78450, 78499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "HO", "HONDURAS (ISLANDS)", false, 78500, 78519);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "PU", "PUERTO RICO", false, 78520, 78534);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "VI", "VIRGIN ISLANDS", false, 78535, 78550);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BH", "BELIZE", false, 78580, 78599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GU", "GUATEMALA", false, 78600, 78649);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ES", "EL SALVADOR", false, 78650, 78699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "HO", "HONDURAS", false, 78700, 78729);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NK", "NICARAGUA", false, 78730, 78759);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CS", "COSTA RICA", false, 78760, 78789);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "PM", "PANAMA", false, 78790, 78819);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "AT", "ANTIGUA,ST.KITTS AND BRITISH ISLANDS VCNTY", false, 78840, 78864);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MN", "ST. MAARTEN,ST. EUSTATIUS AND SABA", false, 78865, 78889);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MF", "ST. MARTIN,ST. BARTHELEMY,GUADELOUPE,AND VCNT", false, 78890, 78899);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "DO", "DOMINICA", false, 78900, 78919);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MR", "MARTINIQUE", false, 78925, 78925);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LC", "ST. LUCIA AND BRITISH ISLANDS TO THE SOUTH", false, 78940, 78951);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BR", "BARBADOS", false, 78954, 78954);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GD", "GRENADA", false, 78958, 78958);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TD", "TRINIDAD AND TOBAGO", false, 78960, 78979);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NU", "NETHERLANDS ANTILLES (ARUBA,BONAIRE,CURACAO)", false, 78980, 78999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	// Etel‰ Amerikka
	SetCountryFilterValues(cFilter, "CO", "COLOMBIA", false, 80000, 80399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "VN", "VENEZUELA", false, 80400, 80499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "GY", "GUYANA", false, 81000, 81199);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SM", "SURINAME", false, 81200, 81399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "FG", "FRENCH GUIANA", false, 81400, 81599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BZ", "BRAZIL", false, 82000, 83999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "EQ", "ECUADOR", false, 84000, 84299);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "PR", "PERU", false, 84300, 84999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BO", "BOLIVIA", false, 85000, 85399);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CH", "CHILE", false, 85400, 85999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "PY", "PARAGUAY", false, 86000, 86299);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "UY", "URUGUAY", false, 86300, 86599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "AG", "ARGENTINA", false, 87000, 87999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "FK", "FALKLAND ISLANDS (MALVINAS)", false, 88800, 88899);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MM", "MISCELLANEOUS MILITARY (SOUTH AMERICA)", false, 89800, 89999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);

	// Tyynen meren maat
//	SetCountryFilterValues(cFilter, "US", "UNITED STATES OF AMERICA (HAWAII)", false, 91000, 91199, 91260, 91299);
//	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MY", "MARIANA ISLANDS", false, 91200, 91222);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "WK", "WAKE ISLAND", false, 91245, 91245);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "MH", "MARSHALL ISLANDS", false, 91250, 91259, 91360, 91379);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "KA", "CAROLINE ISLANDS", false, 91300, 91359, 91400, 91449);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "LN", "SOUTHERN LINE ISLANDS", false, 91385, 91385);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SO", "SOLOMON ISLANDS", false, 91500, 91549);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NW", "NAURU", false, 91530, 91530);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NH", "VANUATA", false, 91550, 91569);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NC", "NEW CALEDONIA AND LOYALTY ISLANDS", false, 91570, 91599);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "KB", "KIRIBATI", false, 91600, 91629);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TV", "TUVALU", false, 91630, 91649);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "FJ", "FIJI", false, 91650, 91699);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "CT", "CANTON ISLAND", false, 91700, 91709);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TK", "TOKELAU ISLAND", false, 91720, 91729);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "FW", "WALLIS AND FUTUNA ISLAND", false, 91750, 91759);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ZM", "WESTERN SAMOA", false, 91760, 91769);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "TO", "TONGA", false, 91770, 91799);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "KU", "COOK ISLANDS", false, 91800, 91849);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "PF", "FRENCH POLYNESIA", false, 91900, 91959);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NZ", "NEW ZEALAND", false, 91450, 91499, 93000, 93999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "NG", "PAPUA NEW GUINEA", false, 94000, 94099);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "AU", "AUSTRALIA", false, 94100, 95999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ID", "INDONESIA", false, 96000, 96299);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BF", "BRUNEI", false, 96315, 96315);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "SK", "SARAWAK", false, 96400, 96499);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "ID", "INDONESIA", false, 96000, 97999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "PH", "PHILIPPINES", false, 98100, 98999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
	SetCountryFilterValues(cFilter, "BM", "BUOYS AND MISCELLANEOUS MILITARY", false, 99000, 99999);
	itsWmoIdFilterManager.AddCountryFilter(cFilter);
}
/*
NO  NORWAY                                             EN..     01000-01499
SN  SWEDEN                                             ES..     02000-02699
FI  FINLAND                                            EF..     02800-02999
UK  UNITED KINGDOM AND NORTHERN IRELAND                EG..     03000-03949
IE  IRELAND                                            EI..     03950-03999
IL  ICELAND                                            BI..     04000-04099
GL  GREENLAND                                          BG..     04100-04399
FA  FAEROE ISLANDS                                     EK..     06000-06019
DN  DENMARK                                            EK..     06020-06199
NL  NETHERLANDS                                        EH..     06200-06399
BX  BELGIUM                                            EB..     06400-06499
BX  LUXEMBOURG                                         EL..     06500-06599
SW  SWITZERLAND                                        LS..     06600-06799
LT  LIECHTENSTEIN                                      ----     06900-06999
FR  FRANCE                                             LF..     07000-07999
SP  SPAIN                                              LE..     08000-08490
GI  GIBRALTAR                                          LX..     08495
AZ  AZORES                                             LP..     08500-08519
MD  MADEIRA ISLANDS                                    LP..     08520-08529
PO  PORTUGAL                                           LP..     08530-08579
CV  CAPE VERDE                                         GV..     08580-08599
DL  GERMANY                                            E[DT]..  10000-10999
OS  AUSTRIA                                            LO..     11000-11399
CZ  CZECH REPUBLIC                                     LK..     11400-11799
LO  SLOVAKIA                                           LK..     11800-11999
PL  POLAND                                             EP..     12100-12699
HU  HUNGARY                                            LH..     12700-12999
YG  YUGOSLAVIA                                         LY..     13000-13599+
LJ  SLOVENIA                                           LJ..     13000-13599+
RH  CROATIA                                            LD..     13000-13599+
AB  ALBANIA                                            ----     13600-13699
RO  ROMANIA                                            LR..     15000-15499
BU  BULGARIA                                           LB..     15500-15999
IY  ITALY                                              LI..     16000-16589
ML  MALTA                                              LM..     16590-16599
GR  GREECE                                             LG..     16600-16999
TU  TURKEY                                             LT..     17000-17599
CY  CYPRUS                                             LC..     17600-17699

  * East Germany's numbers revised to 10000-10999
  + Yugoslavia, Slovenia, Croatia IDs are intermixed

********************************************************************************
    RUSSIA                                                      20000-39999
--------------------------------------------------------------------------------
RA  RUSSIA   (ASIA)                                    ----     20000-21999
RS  RUSSIA   (EUROPE)                                  U...     22000-22999
RA  RUSSIA   (ASIA)                                    U...     23000-25999
BY  BYELORUSSIA                                        U...     26000-26049
RS  RUSSIA   (EUROPE)                                  U...     26050-26099
BY  BYELORUSSIA                                        U...     26100-26149
RS  RUSSIA   (EUROPE)                                  U...     26150-26199
BY  BYELORUSSIA                                        U...     26200-26249
RS  RUSSIA   (EUROPE)                                  U...     26250-26299
BY  BYELORUSSIA                                        U...     26300-26349
RS  RUSSIA   (EUROPE)                                  U...     26350-26399
BY  BYELORUSSIA                                        U...     26400-26449
RS  RUSSIA   (EUROPE)                                  U...     26450-26499
BY  BYELORUSSIA                                        U...     26500-26549
RS  RUSSIA   (EUROPE)                                  U...     26550-26599
BY  BYELORUSSIA                                        U...     26600-26649
RS  RUSSIA   (EUROPE)                                  U...     26650-26699
BY  BYELORUSSIA                                        U...     26700-26749
RS  RUSSIA   (EUROPE)                                  U...     26750-26799
BY  BYELORUSSIA                                        U...     26800-26869
RS  RUSSIA   (EUROPE)                                  U...     26870-26899
BY  BYELORUSSIA                                        U...     26900-26969
RS  RUSSIA   (EUROPE)                                  U...     26970-26999
RS  RUSSIA   (EUROPE)                                  U...     27000-27999
RA  RUSSIA   (ASIA)                                    U...     28000-32999
BY  BYELORUSSIA                                        U...     33000-33045
UR  UKRAIN                                             U...     33046-33999
RS  RUSSIA   (EUROPE)                                  U...     34000-34399
UR  UKRAIN  AND RUSSIA   (EUROPE)                      U...     34400-34719
RS  RUSSIA   (EUROPE)                                  U...     34720-34999
RA  RUSSIA   (ASIA)                                    U...     35000-36999
RS  RUSSIA   (EUROPE)                                  U...     37000-37999
RA  RUSSIA   (ASIA)                                    U...     38000-38999

********************************************************************************
    ASIA                                                        40000-59999
--------------------------------------------------------------------------------
SY  SYRIAN ARAB REPUBLIC                               OS..     40000-40099
LB  LEBANON                                            OL..     40100-40149
IS  ISRAEL                                             LL..     40150-40199
JD  JORDAN                                             OJ..     40250-40349
SD  SAUDI ARABIA                                       OE..     40350-40499
KW  KUWAIT                                             OK..     40500-40599
IQ  IRAQ                                               OR..     40600-40699
IR  IRAN                                               OI..     40700-40899
AH  AFGHANISTAN                                        OA..     40900-40999
SD  SAUDI ARABIA                                       OE..     41000-41149
BN  BAHRAIN                                            OB..     41150
QT  QATAR                                              OT..     41160-41179
ER  UNITED ARAB EMIRATES                               OM..     41180-41239
OM  OMAN                                               OO..     41240-41319
YE  YEMEN                                              OY..     41320-41409
DY  DEMOCRATIC YEMEN                                   OD..     41410-41499
PK  PAKISTAN                                           OP..     41500-41799
BW  BANGLADESH                                         VG..     41800-41999
IN  INDIA                                             V[IAEO].. 42000-43999
SB  SRI LANKA                                          VC..     43400-43499
MV  MALDIVES                                           VR..     43500-43599
MO  MONGOLIA                                           ----     44000-44399
NP  NEPAL                                              VN..     44400-44499
HK  HONG KONG                                          VH..     45000-45009
MU  MACAO                                              VM..     45010-45019
HK  HONG KONG                                          VH..     45030-45039
TW  TAIWAN                                             RC..     46000-46999
KR  DEMOCRATIC PEOPLE*S REPUBLIC OF KOREA (NORTH)      ZK..     47000-47079
KO  KOREA,REPUBLIC OF (SOUTH)                          RK..     47080-47199
JP  JAPAN                                              R[JO]..  47400-47999
BM  BURMA/MYANMAR                                      VB..     48000-48199
TH  THAILAND                                           VT..     48300-48599
MS  MALAYSIA                                           WM..     48600-48689
SR  SINGAPORE                                          WS..     48690-46999
VS  VIET NAM                                           VV..     48800-48920
LA  LAO PEOPLE*S DEMOCRATIC REPUBLIC                   VL..     48930-48959
KP  DEMOCRATIC KAMPUCHEA                               VD..     48960-48999
CI  CHINA                                        Z[YWLBSUPHG].. 50000-59999

********************************************************************************
    AFRICA                                                      60000-68999
--------------------------------------------------------------------------------
CR  CANARY ISLAND (SPAIN)                              GC..     60000-60049
MC  MOROCCO                                            GM..     60050-60319
SP  SPAIN                                              GE..     60320-60339
MC  MOROCCO                                            GM..     60340-60349
AL  ALGERIA                                            DA..     60350-60699
TS  TUNISIA                                            DT..     60700-60799
NR  NIGER                                              DR..     61000-61099
MI  MALI                                               GA..     61100-61299
MT  MAURITANIA                                         GQ..     61400-61499
SG  SENEGAL                                            GO..     61600-61699
GB  GAMBIA                                             GB..     61700-61749
GW  GUINEA-BISSAU                                      G[GP]..  61750-61799
GN  GUINEA                                             GU..     61800-61849
SL  SIERRA LEONE                                       GF..     61850-61899
HE  ST. HELENA                                         ----     61901
AI  ASCENSION ISLAND                                   FH..     61902-61903
TP  SAO TOME AND PRINCIPE                              FP..     61930-61939
BT  BRITISH INDIAN OCEAN TERRITORY                     FJ..     61967
RE  REUNION AND ASSOCIATED ISLANDS                     ----     61968-61972
MA  MAURITIUS                                          ----     61974
LY  LIBYAN ARAB JAMAHIRIYA                             HL..     62000-62299
EG  EGYPT                                              HE..     62300-62499
SU  SUDAN                                              HS..     62600-62999
ET  ETHIOPIA                                           HA..     63000-63099
DJ  DJIBOUTI                                           HF..     63100-63149
SI  SOMALIA                                            HC..     63150-63299
ET  ETHIOPIA                                           HA..     63300-63499
UG  UGANDA                                             HU..     63600-63979*
KN  KENYA                                              HK..     63600-63979*
TN  TANZANIA,UNITED REPUBLIC OF                        HT..     63600-63979*
SC  SEYCHELLES                                         FS..     63980-63999
ZR  ZAIRE                                              FZ..     64000-64360
RW  RWANDA                                             HR..     64380-64389
BI  BURUNDI                                            HB..     64390-64399
CG  CONGO                                              FC..     64000-64499
GO  GABON                                              FO..     64500-64599
CE  CENTRAL AFRICAN REPUBLIC                           FE..     64600-64699
CD  CHAD                                               FT..     64700-64799
GQ  EQUATORIAL GUINEA                                  FG..     64800-64849
CM  CAMEROON,UNITED REPUBLIC OF                        FK..     64850-64999
NI  NIGERIA                                            DN..     65000-65299
BJ  BENIN                                              DB..     65300-65349
TG  TOGO                                               DX..     65350-65399
GH  GHANA                                              DG..     65400-65499
HV  BURKINA FASO                                       D[FH]..  65500-65524
IV  IVORY COAST                                        DI..     65525-65599
LI  LIBERIA                                            GL..     65600-65699
AN  ANGOLA                                             ----     66000-66999
IC  COMOROS                                            FMC.     67000-67005
MG  MADAGASCAR                                         FM..     67009-67199
MZ  MOZAMBIQUE                                         FQ..     67200-67399
ZB  ZAMBIA                                             FL..     67400-67753*
MW  MALAWI                                             FW..     67400-67753*
ZW  ZIMBABWE                                           FV..     67755-67789
MW  MALAWI                                             FW..     67790-67799
ZW  ZIMBABWE                                           FV..     67800-67999
NM  NAMIBIA                                            FA..     68000-68349*
BC  BOTSWANA                                           FB..     68000-68349*
ZA  SOUTH AFRICA                                       FA..     68000-68349*
ZA  SOUTH AFRICA                                       FA..     68350-68989*
SV  SWAZILAND                                          FD..     68396      *
LS  LESOTHO                                            FX..     68450-68459*
BV  BOUVET ISLAND                                               68992

  * Intermixed numbers between countries

********************************************************************************
    NORTH AMERICA                                               69000-79999
--------------------------------------------------------------------------------
US  UNITED STATES OF AMERICA (MILITARY)                KQ..     69000-69999
US  UNITED STATES OF AMERICA (ALASKA)                  PA..     70000-70999
CN  CANADA                                             C[WYZ].. 71000-71999
US  UNITED STATES OF AMERICA                           K...     72000-72999
US  UNITED STATES OF AMERICA                           K...     74000-74999
MX  MEXICO                                             MM..     76000-76999
BE  BERMUDA                                            TX..     78016
BA  BAHAMAS                                            MY..     78050-78199
TI  TURKS ISLANDS                                      MB..     78118
CU  CUBA                                               MU..     78200-78379
GC  CAYMAN ISLAND                                      MW..     78380-78384
JM  JAMAICA                                            MK..     78385-78999
HA  HAITI                                              MT..     78400-78449
DR  DOMINICAN REPUBLIC                                 MD..     78450-78499
HO  HONDURAS (ISLANDS)                                 MH..     78500-78519
PU  PUERTO RICO                                        TJ..     78520-78534
VI  VIRGIN ISLANDS                                     TI..     78535-78550
BH  BELIZE                                             MZ..     78580-78599
GU  GUATEMALA                                          MG..     78600-78649
ES  EL SALVADOR                                        MS..     78650-78699
HO  HONDURAS                                           MH..     78700-78729
NK  NICARAGUA                                          MN..     78730-78759
CS  COSTA RICA                                         MR..     78760-78789
PM  PANAMA                                             MP..     78790-78819
AT  ANTIGUA,ST.KITTS AND BRITISH ISLANDS VCNTY         TK..     78840-78864
MN  ST. MAARTEN,ST. EUSTATIUS AND SABA                 TN..     78865-78889
MF  ST. MARTIN,ST. BARTHELEMY,GUADELOUPE,AND VCNTY     TF..     78890-78899
DO  DOMINICA                                           TD..     78900-78919
MR  MARTINIQUE                                         TF..     78925
LC  ST. LUCIA AND BRITISH ISLANDS TO THE SOUTH         T[LV]..  78940-78951
BR  BARBADOS                                           TB..     78954
GD  GRENADA                                            TG..     78958
TD  TRINIDAD AND TOBAGO                                TT..     78960-78979
NU  NETHERLANDS ANTILLES (ARUBA,BONAIRE,CURACAO)       TN..     78980-78999
CA  CARIBBEAN AREA AND CENTRAL AMERICA                 ----     78xxx

********************************************************************************
    SOUTH AMERICA                                               80000-89999
--------------------------------------------------------------------------------
CO  COLOMBIA                                           SK..     80000-80399
VN  VENEZUELA                                          SV..     80400-80499
GY  GUYANA                                             SY..     81000-81199
SM  SURINAME                                           SM..     81200-81399
FG  FRENCH GUIANA                                      SO..     81400-81599
BZ  BRAZIL                                             SB..     82000-83999
EQ  ECUADOR                                            SE..     84000-84299
PR  PERU                                               SP..     84300-84999
BO  BOLIVIA                                            SL..     85000-85399
CH  CHILE                                              SC..     85400-85999
PY  PARAGUAY                                           SG..     86000-86299
UY  URUGUAY                                            SU..     86300-86599
AG  ARGENTINA                                          SA..     87000-87999
FK  FALKLAND ISLANDS (MALVINAS)                        ----     88800-88899
--  MISCELLANEOUS MILITARY                             ----     89800-89999

********************************************************************************
    PACIFIC AREA                                                90000-98999
--------------------------------------------------------------------------------
US  UNITED STATES OF AMERICA (HAWAII)                  PH..     91000-91199
MY  MARIANA ISLANDS                                    PG..     91200-91222
WK  WAKE ISLAND                                        PW..     91245
MH  MARSHALL ISLANDS                                   PK..     91250-91259
US  UNITED STATES OF AMERICA (HAWAII)                  PH..     91260-91299
KA  CAROLINE ISLANDS                                   PT..     91300-91359
MH  MARSHALL ISLANDS                                   PK..     91360-91379
LN  SOUTHERN LINE ISLANDS                              PL..     91385
KA  CAROLINE ISLANDS                                   PT..     91400-91449
NZ  NEW ZEALAND (ISLANDS)                              PL..     91450-91499
SO  SOLOMON ISLANDS                                    AG..     91500-91549
NW  NAURU                                              AN..     91530
NH  VANUATA                                            NV..     91550-91569
NC  NEW CALEDONIA AND LOYALTY ISLANDS                  NW..     91570-91599
KB  KIRIBATI                                           NG..     91600-91629
TV  TUVALU                                             NG..     91630-91649
FJ  FIJI                                               NF..     91650-91699
CT  CANTON ISLAND                                      ----     91700-91709
TK  TOKELAU ISLAND                                     ----     91720-91729
FW  WALLIS AND FUTUNA ISLAND                           NL..     91750-91759
ZM  WESTERN SAMOA                                      NS..     91760-91769
TO  TONGA                                              NF..     91770-91799
KU  COOK ISLANDS                                       NC..     91800-91849
PF  FRENCH POLYNESIA                                   NT..     91900-91959
PN  NORTH PACIFIC ISLANDS                              ----     91xxx
PS  SOUTH PACIFIC ISLANDS                              ----     91xxx
NZ  NEW ZEALAND                                        NZ..     93000-93999
NG  PAPUA NEW GUINEA                                   AY..     94000-94099
AU  AUSTRALIA                                       A[DBPMS]..  94100-95999
ID  INDONESIA                                          WI..     96000-96299
BF  BRUNEI                                             WB..     96315
SK  SARAWAK                                            WB..     96400-96499
ID  INDONESIA                                          W[AIR].. 96000-97999
PH  PHILIPPINES                                        RP..     98100-98999
--  BUOYS AND MISCELLANEOUS MILITARY                   ----     99000-99999

*/



void CFmiSynopDataGridViewDlg::OnCbnSelchangeComboProducerSelection()
{
	WhenProducerRadioButtonClikked();
}

void CFmiSynopDataGridViewDlg::OnBnClickedCheckMinMaxMode()
{
	UpdateData(TRUE);
	EnableDisableControls();
    ForcedUpdate();
}

void CFmiSynopDataGridViewDlg::OnEnChangeEditDayCount()
{
	UpdateData(TRUE);
    ForcedUpdate();
}

void CFmiSynopDataGridViewDlg::OnDtnDatetimechangeDatetimepickerMinmaxDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	UpdateData(TRUE);
	UpdateMinMaxRangeStartTime();
    ForcedUpdate();
	*pResult = 0;
}

void CFmiSynopDataGridViewDlg::OnDtnDatetimechangeDatetimepickerMinmaxTime(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	UpdateData(TRUE);
	UpdateMinMaxRangeStartTime();
    ForcedUpdate();
	*pResult = 0;
}

void CFmiSynopDataGridViewDlg::UpdateMinMaxRangeStartTime(void)
{
	CTime aDate;
	CTime aTime;
	itsMinMaxDatePicker.GetTime(aDate);
	itsMinMaxTimePicker.GetTime(aTime);
	itsMinMaxRangeStartTime = NFmiMetTime(
		aDate.GetYear(), aDate.GetMonth(), aDate.GetDay(),
		aTime.GetHour()); // j‰tet‰‰n tarkkuus tahallaan tunti tasolle
}

// t‰t‰ on tarkoitus kutsua kun minmax aloitus aikaa on jotenkin muutettu esim. seuraava/edellinen aika-napeista
void CFmiSynopDataGridViewDlg::UpdateMinMaxTimePickers(void)
{
	CTime aTime(itsMinMaxRangeStartTime.GetYear(), itsMinMaxRangeStartTime.GetMonth(), itsMinMaxRangeStartTime.GetDay(), itsMinMaxRangeStartTime.GetHour(), 0, 0);
	itsMinMaxDatePicker.SetTime(&aTime);
	itsMinMaxTimePicker.SetTime(&aTime);
	UpdateData(FALSE);
}

void CFmiSynopDataGridViewDlg::OnBnClickedButtonAutoSizeColumns()
{
	itsGridCtrl.AutoSizeColumns();
	Invalidate(FALSE);
}

// T‰m‰ funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell‰.
// T‰m‰ on ik‰v‰ kyll‰ teht‰v‰ erikseen dialogin muokkaus tyˆkalusta, eli
// tekij‰n pit‰‰ lis‰t‰ erikseen t‰nne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiSynopDataGridViewDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("StationDataTableViewTitle").c_str()));

	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_PRINT, "IDC_BUTTON_PRINT");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_COUNTRY_FILTER_DLG, "IDC_BUTTON_COUNTRY_FILTER_DLG");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_PREVIOUS_TIME2, "IDC_BUTTON_PREVIOUS_TIME2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_NEXT_TIME, "IDC_BUTTON_NEXT_TIME");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_AUTO_SIZE_COLUMNS, "IDC_BUTTON_AUTO_SIZE_COLUMNS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_MIN_MAX_MODE, "IDC_CHECK_MIN_MAX_MODE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_D_STR, "IDC_STATIC_D_STR");
}

BOOL CFmiSynopDataGridViewDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// HUOM!!! Yritin l‰hett‰‰ viestiss‰ komentoa, mutta jostain syyst‰ pResult on on nollautunut jotenkin matkalla
	// Jotain h‰ikk‰ viestin v‰lityksess‰ GridControllista emolle. Sen takia tein UpdateParent-virityksen

	// jos oltiin klikattu otsikko rivi‰, l‰hetet‰‰n viesti emolle, ett‰ se osaa p‰ivitt‰‰ taulukon uudestaan
	if(itsGridCtrl.UpdateParent())
        ForcedUpdate();
	itsGridCtrl.UpdateParent(false);

	return CDialog::OnNotify(wParam, lParam, pResult);
}
