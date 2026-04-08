//� Ilmatieteenlaitos/Marko.
//Original 7.4.2004
//
// Luokka pit�� huolta poikkileikkauksiin liittyvist� p��- ja alipisteist�.
// p��pisteet Laitetaan Metkun editorin kartalle ja niiden avulla lasketaan
// alipisteet, joista tulee poikkileikkaus-hilan sarakkeisto.
//---------------------------------------------------------- NFmiCrossSectionSystem.h

#pragma once

#include "NFmiPoint.h"
#include "NFmiInfoData.h"
#include "NFmiTimeBag.h"
#include "NFmiLocation.h"
#include "GraphicalInfo.h"
#include "NFmiColor.h"
#include "TrueMapViewSizeInfo.h"

#include <memory>

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
		kObsAndFor = 4 // l�hinmm�n luotausaseman ja ennusteen yhdistelm� aika moodissa
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

    // T�ss� structissa tuodaan alustus arvoja Windows rekisterist� ja vied��n ne sinne takaisin.
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

	void Init(const NFmiCrossSectionSystem &theData, bool disableWindowManipulations);
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

	void CalcMinorPoints(const std::shared_ptr<NFmiArea> &theArea); // laskee  v�lipisteet p��pisteiden avulla k�ytt�en hyv�kseen annettua areaa (josta laskuissa k�ytet��n arean xy-maailmaa)
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
	std::shared_ptr<NFmiDrawParam> CrossSectionDrawParam(void) const {return itsCrossSectionDrawParam;}
	void CrossSectionDrawParam(std::shared_ptr<NFmiDrawParam> &newValue);

	int MaxViewRowSize(void) const {return itsMaxViewRowSize;} // ei tehd� asetus metodia, koska se vaatii jonkin verran logiikkaa
    int StartRowIndex(void) const { return itsStartRowIndex; }
    void StartRowIndex(int newValue) { itsStartRowIndex = newValue; }
	int RowCount(void) const {return itsRowCount;}
	int LowestVisibleRowIndex(void);
	Mode CrossSectionMode(void) const {return itsCrossSectionMode;}
	void CrossSectionMode(Mode newMode) {itsCrossSectionMode = newMode;}
	PressureMode StandardPressureLevelMode(void) const {return itsStandardPressureLevelMode;}
	void StandardPressureLevelMode(PressureMode newMode) {itsStandardPressureLevelMode = newMode;}
	bool ChangeStartRowIndex(bool fMoveUp); // paluu arvo kertoo tarvitaanko n�yt�n p�ivityst�
	bool RowCount(int newValue); // paluu arvo kertoo tarvitaanko n�yt�n p�ivityst�
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
	ExtraRowInfo& GetRowInfo(int theRowIndex); // t�m� on muokkausta varten
    CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(){return itsGraphicalInfo;}
	TrueMapViewSizeInfo& GetTrueMapViewSizeInfo() { return itsTrueMapViewSizeInfo; }

	const NFmiColor& StartPointFillColor(void) const {return itsStartPointFillColor;}
	void StartPointFillColor(const NFmiColor &newValue) {itsStartPointFillColor = newValue;}
	const NFmiColor& EndPointFillColor(void) const {return itsEndPointFillColor;}
	void EndPointFillColor(const NFmiColor &newValue) {itsEndPointFillColor = newValue;}
	const NFmiColor& MiddlePointFillColor(void) const {return itsMiddlePointFillColor;}
	void MiddlePointFillColor(const NFmiColor &newValue) {itsMiddlePointFillColor = newValue;}
	void GetStartAndEndTimes(NFmiMetTime &theStartTimeOut, NFmiMetTime &theEndTimeOut, const NFmiMetTime &theCurrentTime, bool fShowTrajectories) const;
	void ParamWindowViewPositionChange(bool forward);
	FmiDirection ParamWindowViewPosition() const { return itsParamWindowViewPosition; }
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

     // Seuraavat dataosiot talletetaan ja luetaan Windows rekistereist�, niiden per�ss� on WinReg -lis�osa sen merkiksi.
	NFmiPoint itsStartPointWinReg; // alkupiste (latlon)
	NFmiPoint itsMiddlePointWinReg; // keskipiste (latlon), k�yt�ss� kun ollaan 3-piste moodissa
	NFmiPoint itsEndPointWinReg; // loppupiste (latlon)
	int itsWantedMinorPointCountWinReg; // t�h�n kokonais lukum��r��n pyrit��n, kun lasketaan p��pisteiden avulla v�lipisteit�
	int itsVerticalPointCountWinReg; // kuinka monta data pistett� lasketaan vertikaali suunnassa
	ExtraRowInfo itsAxisValuesDefaultWinReg; // SmartMetin poikkileikkauksen [CTRL] + [SHIFT] + D s��d�t t�st�/t�h�n
	ExtraRowInfo itsAxisValuesSpecialWinReg; // SmartMetin poikkileikkauksen [CTRL] + [SHIFT] + S s��d�t t�st�/t�h�n

    // loppuja ei ole talletettu Windows rekistereihin
	std::vector<NFmiPoint> itsMainXYPoints; // p�� pisteiden xy pisteet, joiden avulla piirret��n v�ri-pallot CrossSectionManagerView:in footeriin (viimeksi piirretty CrossSectionView p�ivitt�� pisteet)
	std::vector<NFmiPoint> itsMinorPoints; // p��pisteiden avulla lasketut v�lipisteet
	std::vector<NFmiMetTime> itsRouteTimes; // reitin alku ja loppu ajan ja paikkojen avulla lasketut v�lipiste ajat (minuutin tarkkuudella)
	int itsActivatedMinorPointIndex; // aktiivisen v�lipisteen indeksi tai -1, jos mik��n ei ole aktiivinen
	bool fCrossSectionSystemActive; // ollaanko poikkileikkaus moodissa vai ei?
	bool fCrossSectionViewNeedsUpdate; // jos ladataan uutta crosssection dataa, pit�� n�ytt� ja sen data p�ivitt��
	bool fUseTimeCrossSection; // k�ytet��nk� poikkileikkauksessa yhden pisteen leikkausta ajan suhteen vai kahden/useamman paikan v�list� alue leikkausta
	bool fUseRouteCrossSection; // k�ytet��nk� poikkileikkauksessa ns. reitti poikkileikkausta, jolloin alku/loppu ajat ja alku ja loppu paikat. T�m� on voimakkaampi, jos aikapoikkileikkaus on my�s p��ll�
	bool fUseObsAndForCrossSection; // k�ytet��nk� poikkileikkauksessa ns. havainto+ennuste aika moodia
	bool fTimeCrossSectionDirty; // jos poikkileikkaus 'timebagin' ajat muuttuneet, pit�� luoda uusi aika-kontrolli
	bool fCrossSectionViewOn; // onko poikkileikkausn�ytt� p��ll� vai ei?
	bool fShowHybridLevels; // n�yt� hybridi levelit poikkileikkauksessa jos mahdollista (samat ehdot kuin n�yt� maanpinta)
	NFmiTimeBag itsCrossSectionTimeControlTimeBag; // aikakontrolliikkuna tehd��n t�m�n bagin mukaan
	std::shared_ptr<NFmiDrawParam> itsCrossSectionDrawParam; // ei omista, ei tuhoa!!

	int itsMaxViewRowSize; // t�m�n enemp�� ei voi olla poikkileikkaus n�yt�ss� rivej�
	int itsStartRowIndex; // mist� indeksist� poikkileikkausn�yt�n rivien indeksit alkavat (1-5)
	int itsRowCount; // kuinka monta rivi� on kerallaan n�yt�ss� nyt (maksimi tietenkin itsMaxViewRowSize)
	Mode itsCrossSectionMode; // 0 = 2-pisteinen moodi, 1 = 3-pisteinen moodi
	PressureMode itsStandardPressureLevelMode; // miten peruspaine pinnat piirret��n n�ytt��n
									   // 0= i ollenkaan, 1= muun datan alle ja 2= kaiken p��lle
    CtrlViewDocumentInterface *itsCtrlViewDocumentInterface; // tarvitaan kun on muutettu poikkileikkaus n�yt�n hilan kokoa ja pit�� p�ivitt�� mm. infoorganizerin poikkileikkausmacroparamdata

	NFmiPoint itsLastMousePosition; // NFmiStationViewHandlerin mousemove:ssa tarvitaan t�t� laskemaan hiiren vetoa
	NFmiLocation itsObsForModeLocation;
	bool fDragWholeCrossSection; // jos tehty tietynlainen mouse drag kartalla, liikutetaan koko poikkileikkausjanaa kerrallaan
	std::vector<ExtraRowInfo> itsExtraRowInfos; // t�h� talletetaan ylim��r�iset poikkileikkausrivi kohtaiset tiedot
	CtrlViewUtils::GraphicalInfo itsGraphicalInfo;
	TrueMapViewSizeInfo itsTrueMapViewSizeInfo;
	NFmiColor itsStartPointFillColor;
	NFmiColor itsEndPointFillColor;
	NFmiColor itsMiddlePointFillColor;
	FmiDirection itsParamWindowViewPosition;
	bool fShowTooltipOnCrossSectionView; // Erottelin poikkileikkaus tooltip on/off asetuksen pois karttan�ytt�jen asetuksesta, 
                                         // t�t� ei talletetan mihink��n (ei n�ytt� makroon eik� rekistereihin eik� konffeihin), 
                                         // oletuksena arvo on true.
};

inline std::ostream& operator<<(std::ostream& os, const NFmiCrossSectionSystem& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiCrossSectionSystem& item){item.Read(is); return is;}

