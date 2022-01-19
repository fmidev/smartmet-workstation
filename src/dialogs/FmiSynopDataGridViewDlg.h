#pragma once

#include "SmartMetDialogs_resource.h"
#include <vector>
#include "GridCtrl.h"
#include "NFmiMetTime.h"
#include "NFmiProducerSystem.h"
#include "NFmiViewSettingMacro.h"
#include "afxwin.h"
#include "afxdtctl.h"
#include "NFmiViewPosRegistryInfo.h"


class NFmiFastQueryInfo;
class SmartMetDocumentInterface;

struct IdRange
{
	IdRange(void)
	:low(-1)
	,high(-1)
	{}

	IdRange(int theLow, int theHigh)
	:low(theLow)
	,high(theHigh)
	{}

	bool IsInside(int theId)
	{
		return ((low <= theId) && (theId <= high));
	}

	int low;
	int high;
};

struct CountryFilter
{
	CountryFilter(void)
	:itsShortName()
	,itsLongName()
	,itsWmoIdRanges()
	,fSelected(false)
	,fSharesIdRange(false)
	,itsRow(-1)
	,itsColumn(-1)
	{}

	bool ShowStation(int theWmoId)
	{
		if(fSelected)
		{
			int ssize = static_cast<int>(itsWmoIdRanges.size());
			for(int i=0; i<ssize; i++)
			{
				if(itsWmoIdRanges[i].IsInside(theWmoId))
					return true;
			}
		}
		return false;
	}
	bool IsInRange(int theWmoId) // t‰t‰ funktiota on tarkoitettu k‰ytett‰v‰ksi vain optimoinnissa
	{
		if(fSharesIdRange) // jos jaettu id-v‰li, ei voida optimoida WmoIdFilterManager::ShowStation-metodia
			return false;
		int ssize = static_cast<int>(itsWmoIdRanges.size());
		for(int i=0; i<ssize; i++)
		{
			if(itsWmoIdRanges[i].IsInside(theWmoId))
				return true;
		}
		return false;
	}

	std::string itsShortName;  // FI
	std::string itsLongName;  // FINLAND
	std::vector<IdRange> itsWmoIdRanges; // piti laittaa wmo id ranget vektoriin koska esim. suomella on kaksi numero id v‰li‰, milla Suomen asemat ovat
	bool fSelected;
	bool fSharesIdRange; // joillain alueilla voi olla yhten‰inen ID v‰li, joka pit‰‰ ottaa huomioon optimoinnissa, esim. IsInRange
	int itsRow; // t‰m‰ on t‰m‰n maan sijainti filtteri taulukossa
	int itsColumn; // t‰m‰ on t‰m‰n maan sijainti filtteri taulukossa
};

class WmoIdFilterManager
{
public:
	WmoIdFilterManager(SmartMetDocumentInterface *smartMetDocumentInterface)
	:fAllSelected(true)
	,itsCountryFilters()
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	{}

	void AddCountryFilter(const CountryFilter &theCountry)
	{ itsCountryFilters.push_back(theCountry); }
	int GetSize(void)
	{return static_cast<int>(itsCountryFilters.size());}
	bool AllSelected(void)const{return fAllSelected;}
	void AllSelected(bool newState){fAllSelected = newState;}

	bool ShowStation(int theWmoId)
	{
		if(fAllSelected)
			return true;
		else
		{
			int ssize = static_cast<int>(itsCountryFilters.size());
			for(int i=0; i<ssize; i++)
			{
				if(itsCountryFilters[i].ShowStation(theWmoId))
					return true;
				if(itsCountryFilters[i].IsInRange(theWmoId)) // jos maa ei ollut valittu n‰ytett‰v‰ksi, voidaan optimoida false polku, jos wmoid oli t‰m‰n maan haarukassa
					return false;
			}
			return false;
		}
	}

	CountryFilter& GetCountryFilter(int theIndex)
	{
		static CountryFilter dummy;
		int ssize = static_cast<int>(itsCountryFilters.size());
		if(theIndex >= 0 && theIndex < ssize)
			return itsCountryFilters[theIndex];
		return dummy;
	}
	CountryFilter& GetCountryFilter(int row, int column)
	{
		static CountryFilter dummy;
		int ssize = static_cast<int>(itsCountryFilters.size());
		for(int i=0; i<ssize; i++)
		{
			if(itsCountryFilters[i].itsRow == row && itsCountryFilters[i].itsColumn == column)
				return itsCountryFilters[i];
		}
		return dummy;
	}
	std::string GetFilterString(void) const;
	std::string GetSelectedCountryAbbrStr(void) const;
	void SetSelectedCountryAbbrStr(const std::string &theStr);
private:
	bool fAllSelected; // t‰m‰ overridaa muut valinnat, jos p‰‰ll‰
	std::vector<CountryFilter> itsCountryFilters;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista/tuhoa
};

// T‰nne talletetaan k‰ytˆss‰ olleet datatiedostojen nimet.
// Niiden avulla voidaan optimoida, ett‰ tarvitaanko Update metodissa todella p‰ivitt‰‰
// Kaikkea taulukkoon liittyv‰‰.
class SynopDataGridViewUsedFileNames
{
    // Valitun tuottajan ja k‰ytetyn datan tiedostonimi (esim. synop data tai ecmwf/Hirlam/gfs normi-pintadata)
    std::string itsUsedDataFileName;
    // Kaikkien synop-havaintodatojen tiedosto nimet, niist‰ otetaan asematiedot kun k‰ytet‰‰n mallidataa
    std::vector<std::string> itsObsDataFileNames;
    // Milt‰ ajanhetkelt‰ data on piirretty
    NFmiMetTime itsValidTime = NFmiMetTime::gMissingTime;
public:
    SynopDataGridViewUsedFileNames() = default;
    SynopDataGridViewUsedFileNames(const std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &obsInfos, const boost::shared_ptr<NFmiFastQueryInfo> &usedInfo, const NFmiMetTime &validTime);
    SynopDataGridViewUsedFileNames(const SynopDataGridViewUsedFileNames &other) = default;

    bool IsUpdateNeeded(const SynopDataGridViewUsedFileNames &other, bool modelDataCase, bool minMaxModeUsed) const;
    bool IsUpdateNeededDueTimeChange(const SynopDataGridViewUsedFileNames &other, bool minMaxModeUsed) const;
    void Clear();
    bool Empty() const;
    std::string GetChangedFileNames(const SynopDataGridViewUsedFileNames &other, bool modelDataCase) const;
private:
    void ClearNames();
    void UpdateNames(const std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &obsInfos, const boost::shared_ptr<NFmiFastQueryInfo> &usedInfo);
    std::string GetChangedFileNames(const std::vector<std::string> &otherObsDataFileNames) const;
};

struct HeaderParInfo
{
	enum RangeFunction
	{
		kValue=1,
		kMin=2,
		kMax=3,
		kSum=4,
		kAvg=5,
		kDateAndTime=6,
		kRowNumber=7,
		kWmoId=8,
		kStationName=9,
		kLon=10,
		kLat=11
	};

	HeaderParInfo(void)
	:itsParId(kFmiBadParameter)
	,itsHeader("")
	,itsParIndex(static_cast<unsigned long>(-1))
	,itsColumnWidth(3)
	,itsRangeCalculationFunction(kValue)
	,fUseParIndex(false)
	{}

	HeaderParInfo(FmiParameterName parId, const std::string &theHeader, bool useParIndex, int theColumnWidth, RangeFunction func = kValue)
	:itsParId(parId)
	,itsHeader(theHeader)
	,itsParIndex(static_cast<unsigned long>(-1))
	,itsColumnWidth(theColumnWidth)
	,itsRangeCalculationFunction(func)
	,fUseParIndex(useParIndex)
	{}

	FmiParameterName itsParId;
	std::string itsHeader;
	unsigned long itsParIndex;
	int itsColumnWidth;
	RangeFunction itsRangeCalculationFunction;
	bool fUseParIndex;
};

// pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiGridCtrl)

public:
    NFmiGridCtrl(int theMapViewDescTopIndex = 0, int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
	:CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
	,itsSmartMetDocumentInterface(nullptr)
	,itsWmoIdFilterManager(nullptr)
	,itsLastSortedCell()
	,fLastSortedAscending(FALSE)
	,fLastSortedExist(false)
	,itsLastDataTime()
	,itsLastStationCount(0)
	,fEnableFixedColumnSelection(true)
	,itsProducerString()
	,itsUsedHeaders(nullptr)
	,fMinMaxModeOn(FALSE)
	,itsDayRangeValue(1)
	,itsMapViewDescTopIndex(theMapViewDescTopIndex)
	,fUpdateParent(false)
	{}

	void SetDocument(SmartMetDocumentInterface *smartMetDocumentInterface){itsSmartMetDocumentInterface = smartMetDocumentInterface;};
	void SetWmoIdFilterManager(WmoIdFilterManager *theWmoIdFilterManager){itsWmoIdFilterManager = theWmoIdFilterManager;};
	void SetProducerString(const std::string &theStr) {itsProducerString = theStr;}
	void SetUsedHeaders(std::vector<HeaderParInfo> *theUsedHeaders){itsUsedHeaders = theUsedHeaders;};

	void DoLastSort(void);
	void LastDataTime(const NFmiMetTime &theTime)
	{ itsLastDataTime = theTime;}
	void LastStationCount(int newValue)
	{ itsLastStationCount = newValue;}
	void MinMaxModeOn(BOOL newState) {fMinMaxModeOn = newState;}
	void DayRangeValue(double newValue) {itsDayRangeValue = newValue;}
	bool UpdateParent(void) {return fUpdateParent;}
	void UpdateParent(bool newValue) {fUpdateParent = newValue;}

	static int CALLBACK pfnCellTextCaseInSensitiveCompare(LPARAM lParam1,
																LPARAM lParam2,
																LPARAM lParamSort);

	static int CALLBACK pfnCellNumericWithMissingValuesCompare(LPARAM lParam1,
															LPARAM lParam2,
															LPARAM lParamSort);

protected:
    void OnFixedRowClick(CCellID& cell);
    void PrintFooter(CDC *pDC, CPrintInfo *pInfo);
    void PrintHeader(CDC *pDC, CPrintInfo *pInfo);
private:
	void DoMapHighLightThings(const CCellID &idCurrentCell);
    void StoreLastSortInformation(CCellID& cell);

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista/tuhoa
	WmoIdFilterManager *itsWmoIdFilterManager; // ei omista/tuhoa
	// v‰lill‰ pit‰‰ tehd‰ viimeksi tehty sorttaus uudestaan, t‰ss‰ on tiedot
	// viimeksi tehdyst‰ sortista
	CCellID itsLastSortedCell; // mille solulle se tehtiin
	BOOL fLastSortedAscending; // mik‰ oli sottauksen suunta
	bool fLastSortedExist; // onko edes tehty mit‰‰n sorttausta

	NFmiMetTime itsLastDataTime; // k‰ytet‰‰n printtauksessa otsikossa
	int itsLastStationCount; // k‰ytet‰‰n printtauksessa otsikossa
	bool fEnableFixedColumnSelection; // t‰m‰ on pikku kikka mill‰ estet‰‰n valittujen ruutujen sekoittuminen kun tehd‰‰n DoLastSort
	std::string itsProducerString;
	std::vector<HeaderParInfo> *itsUsedHeaders; // ei omista/tuhoa
	BOOL fMinMaxModeOn;
	double itsDayRangeValue;
	int itsMapViewDescTopIndex;
	bool fUpdateParent;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};

// CFmiSynopDataGridViewDlg dialog

class CFmiSynopDataGridViewDlg : public CDialog
{
private:
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiSynopDataGridViewDlg)

public:
	CFmiSynopDataGridViewDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiSynopDataGridViewDlg();
    void Update(void);
    void MakeNextUpdateForced();
	void SetDefaultValues(void);
	void EnsureVisibleStationRow(void);
	void FillSynopDataGridViewMacro(NFmiViewSettingMacro::SynopDataGridView &theViewMacro);
	void ApplySynopDataGridViewMacro(NFmiViewSettingMacro::SynopDataGridView &theViewMacro);
	const NFmiMetTime& MinMaxRangeStartTime(void) const {return itsMinMaxRangeStartTime;}
	void MinMaxRangeStartTime(const NFmiMetTime &theTime) {itsMinMaxRangeStartTime = theTime;}
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}

// Dialog Data
	enum { IDD = IDD_DIALOG_SYNOP_DATA_GRID_VIEW };

	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedButtonPrint();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonCountryFilterDlg();
	afx_msg void OnBnClickedButtonPreviousTime2();
	afx_msg void OnBnClickedButtonNextTime();
	afx_msg void OnCbnSelchangeComboProducerSelection();
    afx_msg void OnBnClickedCheckMinMaxMode();
    afx_msg void OnEnChangeEditDayCount();
    afx_msg void OnDtnDatetimechangeDatetimepickerMinmaxDate(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDtnDatetimechangeDatetimepickerMinmaxTime(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedButtonAutoSizeColumns();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel();
	virtual void OnOK();
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
private:
	int GetMaxStationCount(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfos);
	int GetMaxStationCount(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	int FindStationRow(int theWmoId);
	CRect CalcClientArea(void);
	CRect CalcOtherArea(void);
	void DoWhenClosing(void);
	boost::shared_ptr<NFmiFastQueryInfo> GetWantedInfo(bool fGetObsStationData);
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > GetWantedSynopInfos(void);
	void InitSynopHeaders(void);
	void InitForecastSynopHeaders(void);
	void InitMinMaxDataHeaders(void);
	void InitForecastMinMaxDataHeaders(void);
	void TakeWantedHeadersInUse(int theProducerId);
	void InitWmoIdFilterManager(void);
	void WhenProducerRadioButtonClikked(void);
	const std::string& GetProducerString(void) const;
	void FillGridWithSynopData(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theObsInfos, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, const NFmiMetTime &theTime, boost::shared_ptr<NFmiFastQueryInfo> &theSadeInfo, int &theRealStationCount);
	void FillGridWithForecastData(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theObsInfos, boost::shared_ptr<NFmiFastQueryInfo> &theForInfo, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, const NFmiMetTime &theTime, int &theRealStationCount);
	void FillGridWithMinMaxData(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theObsInfos, boost::shared_ptr<NFmiFastQueryInfo> &theForInfo, int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, boost::shared_ptr<NFmiFastQueryInfo> &theSadeInfo, int &theRealStationCount);
	void InitProducerSelector(void);
	void EnableDisableControls(void);
	void UpdateMinMaxRangeStartTime(void);
	void UpdateMinMaxTimePickers(void);
	bool IsSadeDataUsed(void);
	void InitDialogTexts(void);
	void SelectProducer(const NFmiProducer &theProducer);
    bool GridControlNeedsUpdate(const std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &obsInfos, const boost::shared_ptr<NFmiFastQueryInfo> &usedInfo);
    bool IsSelectedProducerModelData() const;
    void ForcedUpdate();
    const NFmiMetTime& GetMainMapViewTime() const;

	NFmiGridCtrl itsGridCtrl;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	bool fGridCtrlInitialized;

	// T‰h‰n laitetaan valittu aika ja p‰iv‰m‰‰r‰
    CString itsTimeStrU_;
	CFont itsTimeAndStationTextFont;
	int itsMapViewDescTopIndex;

	std::vector<HeaderParInfo> itsSynopHeaders;
	std::vector<HeaderParInfo> itsForecastSynopHeaders;
	std::vector<HeaderParInfo> itsMinMaxDataHeaders;
	std::vector<HeaderParInfo> itsForecastMinMaxDataHeaders;

	std::vector<HeaderParInfo> *itsUsedHeaders;

	WmoIdFilterManager itsWmoIdFilterManager;
	// Tuottajan valinta, Synop, Hirlam, Ec jne.
	CComboBox itsProducerSelector;
	std::vector<NFmiProducerHelperInfo> itsProducerList;
	NFmiMetTime itsMinMaxRangeStartTime; // t‰t‰  s‰‰det‰‰n aika ja kalenteri kontrollien avulla
	bool fUseSadeData; // jos katsellaan synop-tuottajaa, silloin laitetaan myˆs sadedataa mukaan
	bool fUpdateHeadersAfterViewMacroLoad;
    BOOL fMinMaxModeOn;
    // Min/max moodissa lasketaan arvoja n‰in monen p‰iv‰n yli, myˆs desimaalit mukana eli 0.25 on 6 tuntia
    double itsDayRangeValue;
    CDateTimeCtrl itsMinMaxDatePicker;
    CDateTimeCtrl itsMinMaxTimePicker;
    // K‰ytet‰‰n optimoimaan Update metodia, eli aina kun Update:ia kutsutaan, ei ole tarvis p‰ivitt‰‰ taulukkoa, koska uutta dataa ei ole k‰ytˆss‰.
    SynopDataGridViewUsedFileNames itsUsedFileNames;
};
