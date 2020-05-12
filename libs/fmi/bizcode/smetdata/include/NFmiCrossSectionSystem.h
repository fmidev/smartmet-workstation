//© Ilmatieteenlaitos/Marko.
//Original 7.4.2004
//
// Luokka pitää huolta poikkileikkauksiin liittyvistä pää- ja alipisteistä.
// pääpisteet Laitetaan Metkun editorin kartalle ja niiden avulla lasketaan
// alipisteet, joista tulee poikkileikkaus-hilan sarakkeisto.
//---------------------------------------------------------- NFmiCrossSectionSystem.h

#pragma once

#include "NFmiPoint.h"
#include "NFmiDataMatrix.h" // täältä tulee std::vector-luokka
#include "NFmiInfoData.h"
#include "NFmiTimeBag.h"
#include "NFmiLocation.h"
#include "GraphicalInfo.h"
#include "NFmiColor.h"
#include "TrueMapViewSizeInfo.h"

class NFmiArea;
class NFmiQueryData;
class NFmiDrawParam;
class CtrlViewDocumentInterface;

//_________________________________________________________ NFmiCrossSectionSystem
class NFmiCrossSectionSystem
{
 public:
	static double itsLatestVersionNumber;
	mutable double itsCurrentVersionNumber;

	enum Mode
	{
		k2Point=0,
		k3Point=1
	};
	enum PressureMode
	{
		kDontShow=0,
		kShowUnder=1,
		kShowOver=2
	};
	enum CrossMode
	{
		kNormal = 1,
		kTime = 2,
		kRoute = 3,
		kObsAndFor = 4 // lähinmmän luotausaseman ja ennusteen yhdistelmä aika moodissa
	};

	class ExtraRowInfo
	{
	public:
		ExtraRowInfo(void)
		:itsUpperEndOfPressureAxis(kFloatMissing)
		,itsLowerEndOfPressureAxis(kFloatMissing)
		{}
		ExtraRowInfo(double theUpperEndOfPressureAxis, double theLowerEndOfPressureAxis)
		:itsUpperEndOfPressureAxis(theUpperEndOfPressureAxis)
		,itsLowerEndOfPressureAxis(theLowerEndOfPressureAxis)
		{}

		double itsUpperEndOfPressureAxis;
        double itsLowerEndOfPressureAxis;
	};

    // Tässä structissa tuodaan alustus arvoja Windows rekisteristä ja viedään ne sinne takaisin.
    class CrossSectionInitValuesWinReg
    {
    public:
        std::string itsStartPointStr;
        std::string itsMiddlePointStr;
        std::string itsEndPointStr;
        ExtraRowInfo itsAxisValuesDefault;
        ExtraRowInfo itsAxisValuesSpecial;
        int itsVerticalPointCount;
        int itsWantedMinorPointCount;
    };

	NFmiCrossSectionSystem(int theMaxViewRowSize);

	virtual ~NFmiCrossSectionSystem(void);

	void Init(const NFmiCrossSectionSystem &theData);
	void InitializeFromSettings(const CrossSectionInitValuesWinReg &initValuesFromWinReg);
	void StoreSettings(CrossSectionInitValuesWinReg &storeValuesToWinReg);
	void SetDocumentInterface(CtrlViewDocumentInterface *theCtrlViewDocumentInterface){itsCtrlViewDocumentInterface = theCtrlViewDocumentInterface;}
	const NFmiPoint& StartPoint(void) const;
	void StartPoint(const NFmiPoint &thePoint);
	const NFmiPoint& MiddlePoint(void) const;
	void MiddlePoint(const NFmiPoint &thePoint);
	const NFmiPoint& EndPoint(void) const;
	void EndPoint(const NFmiPoint &thePoint);

	const NFmiPoint& StartXYPoint(void) const;
	void StartXYPoint(const NFmiPoint &thePoint);
	const NFmiPoint& MiddleXYPoint(void) const;
	void MiddleXYPoint(const NFmiPoint &thePoint);
	const NFmiPoint& EndXYPoint(void) const;
	void EndXYPoint(const NFmiPoint &thePoint);

	void CalcMinorPoints(const boost::shared_ptr<NFmiArea> &theArea); // laskee  välipisteet pääpisteiden avulla käyttäen hyväkseen annettua areaa (josta laskuissa käytetään arean xy-maailmaa)
	void CalcRouteTimes(void);

	const std::vector<NFmiPoint>& MinorPoints(void) const {return itsMinorPoints;};

    int WantedMinorPointCount(void) const;
	void WantedMinorPointCount(int newValue);
	bool CrossSectionSystemActive(void) const {return fCrossSectionSystemActive;}
	void CrossSectionSystemActive(bool newState) {fCrossSectionSystemActive = newState;}
	bool CrossSectionViewNeedsUpdate(void){return fCrossSectionViewNeedsUpdate;}
	void CrossSectionViewNeedsUpdate(bool newState){fCrossSectionViewNeedsUpdate = newState;}
	void CheckIfCrossSectionViewNeedsUpdate(NFmiQueryData *theData, NFmiInfoData::Type theType);
	int VerticalPointCount(void) const {return itsVerticalPointCountWinReg;}
	void VerticalPointCount(int newValue);
	void ActivateNearestMinorPoint(const NFmiPoint &thePlace);
	const NFmiPoint& ActivatedMinorPoint(void);
	bool IsMinorPointActivated(void){return (itsActivatedMinorPointIndex >= 0) && (itsActivatedMinorPointIndex < static_cast<int>(itsMinorPoints.size()));};
	int ActivatedMinorPointIndex(void) const {return itsActivatedMinorPointIndex;}
	bool UseTimeCrossSection(void)const {return fUseTimeCrossSection;}
	void UseTimeCrossSection(bool newState){fUseTimeCrossSection = newState;}
	boost::shared_ptr<NFmiDrawParam> CrossSectionDrawParam(void) const {return itsCrossSectionDrawParam;}
	void CrossSectionDrawParam(boost::shared_ptr<NFmiDrawParam> &newValue);

	int MaxViewRowSize(void) const {return itsMaxViewRowSize;} // ei tehdä asetus metodia, koska se vaatii jonkin verran logiikkaa
    int StartRowIndex(void) const { return itsStartRowIndex; }
    void StartRowIndex(int newValue) { itsStartRowIndex = newValue; }
	int RowCount(void) const {return itsRowCount;}
	int LowestVisibleRowIndex(void);
	Mode CrossSectionMode(void) const {return itsCrossSectionMode;}
	void CrossSectionMode(Mode newMode) {itsCrossSectionMode = newMode;}
	PressureMode StandardPressureLevelMode(void) const {return itsStandardPressureLevelMode;}
	void StandardPressureLevelMode(PressureMode newMode) {itsStandardPressureLevelMode = newMode;}
	bool ChangeStartRowIndex(bool fMoveUp); // paluu arvo kertoo tarvitaanko näytön päivitystä
	bool RowCount(int newValue); // paluu arvo kertoo tarvitaanko näytön päivitystä
	void NextCrossSectionMode(void);
	void NextStandardPressureLevelMode(void);
	bool IsViewVisible(int theIndex);
	bool TimeCrossSectionDirty(void) const {return fTimeCrossSectionDirty;}
	void TimeCrossSectionDirty(bool newState) {fTimeCrossSectionDirty = newState;}
	const NFmiTimeBag& CrossSectionTimeControlTimeBag(void) const;
	void CrossSectionTimeControlTimeBag(const NFmiTimeBag &newTimeBag);
	bool CrossSectionViewOn(void) const {return fCrossSectionViewOn;}
	void CrossSectionViewOn(bool newValue) {fCrossSectionViewOn = newValue;}
	bool UseRouteCrossSection(void) const {return fUseRouteCrossSection;}
	void UseRouteCrossSection(bool newValue) {fUseRouteCrossSection = newValue;}
	bool UseObsAndForCrossSection(void) const {return fUseObsAndForCrossSection;}
	void UseObsAndForCrossSection(bool newValue)
	{
		if(fUseObsAndForCrossSection != newValue)
			TimeCrossSectionDirty(true);
		fUseObsAndForCrossSection = newValue;
	}
	const std::vector<NFmiMetTime>& RouteTimes(void) const {return itsRouteTimes;}
	CrossMode GetCrossMode(void) const;
	bool ShowHybridLevels(void) const {return fShowHybridLevels;}
	void ShowHybridLevels(bool newState) {fShowHybridLevels = newState;}
	bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);
	const NFmiPoint& LastMousePosition(void) const {return itsLastMousePosition;}
	void LastMousePosition(const NFmiPoint &newValue) {itsLastMousePosition = newValue;}
	bool DragWholeCrossSection(void) const {return fDragWholeCrossSection;}
	void DragWholeCrossSection(bool newValue) {fDragWholeCrossSection = newValue;}
	const NFmiLocation& ObsForModeLocation(void) const {return itsObsForModeLocation;}
	void ObsForModeLocation(const NFmiLocation &newLocation) {itsObsForModeLocation = newLocation;}
	const std::vector<ExtraRowInfo>& ExtraRowInfos(void) const {return itsExtraRowInfos;}
	ExtraRowInfo& GetRowInfo(int theRowIndex); // tämä on muokkausta varten
    CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(){return itsGraphicalInfo;}
	TrueMapViewSizeInfo& GetTrueMapViewSizeInfo() { return itsTrueMapViewSizeInfo; }

	const NFmiColor& StartPointFillColor(void) const {return itsStartPointFillColor;}
	void StartPointFillColor(const NFmiColor &newValue) {itsStartPointFillColor = newValue;}
	const NFmiColor& EndPointFillColor(void) const {return itsEndPointFillColor;}
	void EndPointFillColor(const NFmiColor &newValue) {itsEndPointFillColor = newValue;}
	const NFmiColor& MiddlePointFillColor(void) const {return itsMiddlePointFillColor;}
	void MiddlePointFillColor(const NFmiColor &newValue) {itsMiddlePointFillColor = newValue;}
	void GetStartAndEndTimes(NFmiMetTime &theStartTimeOut, NFmiMetTime &theEndTimeOut, const NFmiMetTime &theCurrentTime, bool fShowTrajectories) const;
	bool ShowParamWindowView(void){return fShowParamWindowView;};
	void ShowParamWindowView(bool newState){fShowParamWindowView = newState;};
    bool ShowTooltipOnCrossSectionView(void) const {return fShowTooltipOnCrossSectionView;}
    void ShowTooltipOnCrossSectionView(bool newValue) {fShowTooltipOnCrossSectionView = newValue;}
	void SetCaseStudyTimes(const NFmiMetTime &theCaseStudyTime);
	void SetCrossSectionDefaultAxisToFirstVisibleView(void);
	void SetCrossSectionDefaultAxisAll(void);
	void SetCrossSectionSpecialAxisToFirstVisibleView(void);
	void SetCrossSectionSpecialAxisAll(void);
	void SaveCrossSectionDefaultAxisValues(void);
	void SaveCrossSectionSpecialAxisValues(void);
	ExtraRowInfo& AxisValuesDefault(void) {return itsAxisValuesDefaultWinReg;}
	ExtraRowInfo& AxisValuesSpecial(void) {return itsAxisValuesSpecialWinReg;}

	void Write(std::ostream& os) const;
	void Read(std::istream& is);
 private:
	std::vector<NFmiPoint> MakeMainPointsVector(void) const;
	void SetMainPointsFromVector(const std::vector<NFmiPoint> &theMainPointVector);
    static int itsNumberOfMainPoints;

     // Seuraavat dataosiot talletetaan ja luetaan Windows rekistereistä, niiden perässä on WinReg -lisäosa sen merkiksi.
	NFmiPoint itsStartPointWinReg; // alkupiste (latlon)
	NFmiPoint itsMiddlePointWinReg; // keskipiste (latlon), käytössä kun ollaan 3-piste moodissa
	NFmiPoint itsEndPointWinReg; // loppupiste (latlon)
	int itsWantedMinorPointCountWinReg; // tähän kokonais lukumäärään pyritään, kun lasketaan pääpisteiden avulla välipisteitä
	int itsVerticalPointCountWinReg; // kuinka monta data pistettä lasketaan vertikaali suunnassa
	ExtraRowInfo itsAxisValuesDefaultWinReg; // SmartMetin poikkileikkauksen [CTRL] + [SHIFT] + D säädöt tästä/tähän
	ExtraRowInfo itsAxisValuesSpecialWinReg; // SmartMetin poikkileikkauksen [CTRL] + [SHIFT] + S säädöt tästä/tähän

    // loppuja ei ole talletettu Windows rekistereihin
	std::vector<NFmiPoint> itsMainXYPoints; // pää pisteiden xy pisteet, joiden avulla piirretään väri-pallot CrossSectionManagerView:in footeriin (viimeksi piirretty CrossSectionView päivittää pisteet)
	std::vector<NFmiPoint> itsMinorPoints; // pääpisteiden avulla lasketut välipisteet
	std::vector<NFmiMetTime> itsRouteTimes; // reitin alku ja loppu ajan ja paikkojen avulla lasketut välipiste ajat (minuutin tarkkuudella)
	int itsActivatedMinorPointIndex; // aktiivisen välipisteen indeksi tai -1, jos mikään ei ole aktiivinen
	bool fCrossSectionSystemActive; // ollaanko poikkileikkaus moodissa vai ei?
	bool fCrossSectionViewNeedsUpdate; // jos ladataan uutta crosssection dataa, pitää näyttö ja sen data päivittää
	bool fUseTimeCrossSection; // käytetäänkö poikkileikkauksessa yhden pisteen leikkausta ajan suhteen vai kahden/useamman paikan välistä alue leikkausta
	bool fUseRouteCrossSection; // käytetäänkö poikkileikkauksessa ns. reitti poikkileikkausta, jolloin alku/loppu ajat ja alku ja loppu paikat. Tämä on voimakkaampi, jos aikapoikkileikkaus on myös päällä
	bool fUseObsAndForCrossSection; // käytetäänkö poikkileikkauksessa ns. havainto+ennuste aika moodia
	bool fTimeCrossSectionDirty; // jos poikkileikkaus 'timebagin' ajat muuttuneet, pitää luoda uusi aika-kontrolli
	bool fCrossSectionViewOn; // onko poikkileikkausnäyttö päällä vai ei?
	bool fShowHybridLevels; // näytä hybridi levelit poikkileikkauksessa jos mahdollista (samat ehdot kuin näytä maanpinta)
	NFmiTimeBag itsCrossSectionTimeControlTimeBag; // aikakontrolliikkuna tehdään tämän bagin mukaan
	boost::shared_ptr<NFmiDrawParam> itsCrossSectionDrawParam; // ei omista, ei tuhoa!!

	int itsMaxViewRowSize; // tämän enempää ei voi olla poikkileikkaus näytössä rivejä
	int itsStartRowIndex; // mistä indeksistä poikkileikkausnäytön rivien indeksit alkavat (1-5)
	int itsRowCount; // kuinka monta riviä on kerallaan näytössä nyt (maksimi tietenkin itsMaxViewRowSize)
	Mode itsCrossSectionMode; // 0 = 2-pisteinen moodi, 1 = 3-pisteinen moodi
	PressureMode itsStandardPressureLevelMode; // miten peruspaine pinnat piirretään näyttöön
									   // 0= i ollenkaan, 1= muun datan alle ja 2= kaiken päälle
    CtrlViewDocumentInterface *itsCtrlViewDocumentInterface; // tarvitaan kun on muutettu poikkileikkaus näytön hilan kokoa ja pitää päivittää mm. infoorganizerin poikkileikkausmacroparamdata

	NFmiPoint itsLastMousePosition; // NFmiStationViewHandlerin mousemove:ssa tarvitaan tätä laskemaan hiiren vetoa
	NFmiLocation itsObsForModeLocation;
	bool fDragWholeCrossSection; // jos tehty tietynlainen mouse drag kartalla, liikutetaan koko poikkileikkausjanaa kerrallaan
	std::vector<ExtraRowInfo> itsExtraRowInfos; // tähä talletetaan ylimääräiset poikkileikkausrivi kohtaiset tiedot
	CtrlViewUtils::GraphicalInfo itsGraphicalInfo;
	TrueMapViewSizeInfo itsTrueMapViewSizeInfo;
	NFmiColor itsStartPointFillColor;
	NFmiColor itsEndPointFillColor;
	NFmiColor itsMiddlePointFillColor;
	bool fShowParamWindowView;
	bool fShowTooltipOnCrossSectionView; // Erottelin poikkileikkaus tooltip on/off asetuksen pois karttanäyttöjen asetuksesta, 
                                         // tätä ei talletetan mihinkään (ei näyttö makroon eikä rekistereihin eikä konffeihin), 
                                         // oletuksena arvo on true.
};

inline std::ostream& operator<<(std::ostream& os, const NFmiCrossSectionSystem& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiCrossSectionSystem& item){item.Read(is); return is;}

