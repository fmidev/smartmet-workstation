//© Ilmatieteenlaitos/Marko.
//Original 28.12.2004
//
// Luokka pitää tietoa onko MetEditori ns. MTA luotaus moodissa, vai noormaali moodissa.
// Jos ollaan MTA moodissa, pitää myös listaa eri luotauksista, mitä näytetään (tiedot
// näyttöön lisätyistä luotauksista pitää sisällään ajan,paikan ja tuottajan).
//---------------------------------------------------------- NFmiMTATempSystem.h

#pragma once

#include "NFmiRect.h"
#include "NFmiMetTime.h"
#include "NFmiProducer.h"
#include "NFmiDataMatrix.h"
#include "NFmiColor.h"
#include "GraphicalInfo.h"
#include "NFmiTempLineInfo.h"
#include "NFmiTempLabelInfo.h"
#include "SoundingDataServerConfigurations.h"
#include "SoundingViewSettingsFromWindowsRegisty.h"

class NFmiProducerSystem;

//_________________________________________________________ NFmiCrossSectionSystem
class NFmiMTATempSystem
{
 public:
	static double itsLatestVersionNumber;
	mutable double itsCurrentVersionNumber;

	class HodografViewData
	{
		// Tähän piirretään hodograafi (relatiivinen laatikko)
		NFmiRect itsRect; 
		// Tämän avulla säädetään hodografin arvoalueen suuruutta
		double itsScaleMaxValue = 50.;
		// Tämän avulla säädetään hodografin ikkunan suhteellista kokoa
		double itsRelativiHeightFactor = 0.35;
	public:

		const NFmiRect& Rect() const { return itsRect; }
		void Rect(const NFmiRect& hodografRect) { itsRect = hodografRect; }
		double ScaleMaxValue() const { return itsScaleMaxValue; }
		void ScaleMaxValue(double newValue) { itsScaleMaxValue = newValue; }
		double RelativiHeightFactor() const { return itsRelativiHeightFactor; }
		void RelativiHeightFactor(double newValue) { itsRelativiHeightFactor = newValue; }

		void AdjustScaleMaxValue(short theDelta);
		void AdjustRelativiHeightFactor(short theDelta);
		void SetCenter(const NFmiPoint& thePlace);

		std::string GenerateSettingsString() const;
		void InitializeFromSettingsString(const std::string &settingsString);
	};

    // ServerProducer luokalla on tieto, käyttääkö se data lähteenään lokaali queryDataa vai serveriä.
    class ServerProducer : public NFmiProducer
    {
        bool useServer_ = false;
    public:
        ServerProducer();
        ServerProducer(const NFmiProducer &producer, bool useServer);
        ServerProducer(const ServerProducer &) = default;
        ServerProducer& operator=(const ServerProducer &) = default;

        bool useServer() const { return useServer_; }
        void useServer(bool useServer) { useServer_ = useServer; }

        // Tehdään vertailu operaattorit, jotka vertaavat prod-id, name ja ServerProducer tapauksissa myös useServer -tilaa.
        // Originaali NFmiProducer vertailut vertaavat vain prod-id:tä.
        bool operator==(const ServerProducer &other) const;
        bool operator==(const NFmiProducer &other) const;
        bool operator!=(const ServerProducer &other) const;
        bool operator!=(const NFmiProducer &other) const;
        static bool ProducersAreEqual(const NFmiProducer &first, const NFmiProducer &second);
        bool operator<(const ServerProducer &other) const;
    };

	class TempInfo
	{
	public:
		TempInfo(void)
		:itsLatlon()
		,itsTime()
		,itsProducer()
		{}

		TempInfo(const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiProducer &theProducer)
		:itsLatlon(theLatlon)
		,itsTime(theTime)
		,itsProducer(theProducer)
		{}

		const NFmiPoint& Latlon(void) const {return itsLatlon;}
		void Latlon(const NFmiPoint &newValue) {itsLatlon = newValue;}
		const NFmiMetTime& Time(void) const {return itsTime;}
		void Time(const NFmiMetTime &newValue) {itsTime = newValue;}
		const NFmiProducer& Producer(void) const {return itsProducer;}
		void Producer(const NFmiProducer &newValue) {itsProducer = newValue;}
        bool operator<(const TempInfo &other) const;

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		NFmiPoint itsLatlon;
		NFmiMetTime itsTime;
		NFmiProducer itsProducer;
	};

    class SoundingDataCacheMapKey
    {
        TempInfo tempInfo_;
        ServerProducer serverProducer_;
        int modelRunIndex_ = 0;
    public:
        SoundingDataCacheMapKey() = default;
        SoundingDataCacheMapKey(const SoundingDataCacheMapKey &) = default;
        SoundingDataCacheMapKey(const TempInfo &tempInfo, const ServerProducer &serverProducer, int modelRunIndex);

        bool operator<(const SoundingDataCacheMapKey &other) const;
    };

    // voidaan helposti siirtyä käyttämään tavallista vector-luokkaa jos haluaa halutaan
	using Container = std::vector<TempInfo>; 
    // SoundingProducer:in bool osa tarkoittaa sitä että haetaanko itse luotausdata lokaali queryDatasta (false) vai serveriltä (true)
    using SoundingProducer = ServerProducer;
    using SelectedProducerContainer = std::vector<SoundingProducer>;
    using SelectedProducerLegacyContainer = std::vector<NFmiProducer>;

	NFmiMTATempSystem(void);
	virtual ~NFmiMTATempSystem(void);

	// Heittää poikkeuksia virhetilanteissa
    void Init(NFmiProducerSystem &theProducerSystem, const std::vector<NFmiProducer>& theExtraSoundingProducers, const SoundingViewSettingsFromWindowsRegisty & soundingViewSettingsFromWindowsRegisty);
	void InitFromViewMacro(const NFmiMTATempSystem &theOther);
	void AddTemp(const TempInfo &theTempInfo);
	void ClearTemps(void);
	const Container& GetTemps(void) const {return itsTempInfos;}
    void SetAllTempTimes(const NFmiMetTime &theTime);
    SelectedProducerContainer& PossibleProducerList() {return itsPossibleProducerList;}
	const SelectedProducerContainer& SoundingComparisonProducers() const { return itsSoundingComparisonProducers; }
    void SoundingComparisonProducers(const SelectedProducerLegacyContainer &selectedProducersLegacyContainer);

	int MaxTempsShowed(void) const {return itsMaxTempsShowed;}
	void MaxTempsShowed(int newValue) {itsMaxTempsShowed = newValue;}
	const NFmiProducer& CurrentProducer(void) const;
	void CurrentProducer(const NFmiProducer &newValue);
	bool TempViewOn(void) const {return fTempViewOn;}
	void TempViewOn(bool newState) {fTempViewOn = newState;}
	const NFmiColor& SoundingColor(int theIndex);
	void SoundingColor(int theIndex, const NFmiColor &theColor);
	double SkewTDegree(void) const {return itsSkewTDegree;}
	void SkewTDegree(double newValue) {itsSkewTDegree = newValue;}
	bool ShowMapMarkers(void) {return fShowMapMarkers;}
	void ShowMapMarkers(bool newValue) {fShowMapMarkers = newValue;}

	double TAxisStart(void);
	double TAxisEnd(void);
	void TAxisStart(double newValue);
	void TAxisEnd(double newValue);
	double TemperatureHelpLineStart(void) const {return itsTemperatureHelpLineStart;}
	void TemperatureHelpLineStart(double newValue) {itsTemperatureHelpLineStart = newValue;}
	double TemperatureHelpLineEnd(void) const {return itsTemperatureHelpLineEnd;}
	void TemperatureHelpLineEnd(double newValue) {itsTemperatureHelpLineEnd = newValue;}
	double TemperatureHelpLineStep(void) const {return itsTemperatureHelpLineStep;}
	void TemperatureHelpLineStep(double newValue) {itsTemperatureHelpLineStep = newValue;}
    NFmiTempLineInfo& TemperatureHelpLineInfo(void) {return itsTemperatureHelpLineInfo;}
	void TemperatureHelpLineInfo(const NFmiTempLineInfo &theLineInfo) {itsTemperatureHelpLineInfo = theLineInfo;}
    NFmiTempLabelInfo& TemperatureHelpLabelInfo(void) {return itsTemperatureHelpLabelInfo;}
	void TemperatureHelpLabelInfo(const NFmiTempLabelInfo& theLabelInfo) {itsTemperatureHelpLabelInfo = theLabelInfo;}

	double PAxisStart(void) const {return itsPAxisStart;}
	void PAxisStart(double newValue);
	double PAxisEnd(void) const {return itsPAxisEnd;}
	void PAxisEnd(double newValue);
	std::vector<double>& PressureValues(void) {return itsPressureValues;}
    NFmiTempLineInfo& PressureLineInfo(void) {return itsPressureLineInfo;}
	void PressureLineInfo(const NFmiTempLineInfo &newValue) {itsPressureLineInfo = newValue;}
    NFmiTempLabelInfo& PressureLabelInfo(void) {return itsPressureLabelInfo;}
	void PressureLabelInfo(const NFmiTempLabelInfo &newValue) {itsPressureLabelInfo = newValue;}
	std::vector<double>& MixingRatioValues(void) {return itsMixingRatioValues;}
    NFmiTempLineInfo& MixingRatioLineInfo(void) {return itsMixingRatioLineInfo;}
	void MixingRatioLineInfo(const NFmiTempLineInfo &newValue) {itsMixingRatioLineInfo = newValue;}
    NFmiTempLabelInfo& MixingRatioLabelInfo(void) {return itsMixingRatioLabelInfo;}
	void MixingRatioLabelInfo(const NFmiTempLabelInfo &newValue) {itsMixingRatioLabelInfo = newValue;}
	std::vector<double>& MoistAdiabaticValues(void) {return itsMoistAdiabaticValues;}
    NFmiTempLineInfo& MoistAdiabaticLineInfo(void) {return itsMoistAdiabaticLineInfo;}
	void MoistAdiabaticLineInfo(const NFmiTempLineInfo &newValue) {itsMoistAdiabaticLineInfo = newValue;}
    NFmiTempLabelInfo& MoistAdiabaticLabelInfo(void) {return itsMoistAdiabaticLabelInfo;}
	void MoistAdiabaticLabelInfo(const NFmiTempLabelInfo &newValue) {itsMoistAdiabaticLabelInfo = newValue;}
	std::vector<double>& DryAdiabaticValues(void) {return itsDryAdiabaticValues;}
    NFmiTempLineInfo& DryAdiabaticLineInfo(void) {return itsDryAdiabaticLineInfo;}
	void DryAdiabaticLineInfo(const NFmiTempLineInfo &newValue) {itsDryAdiabaticLineInfo = newValue;}
    NFmiTempLabelInfo& DryAdiabaticLabelInfo(void) {return itsDryAdiabaticLabelInfo;}
	void DryAdiabaticLabelInfo(const NFmiTempLabelInfo &newValue) {itsDryAdiabaticLabelInfo = newValue;}
	const NFmiPoint& WindvectorSizeInPixels(void) const {return itsWindvectorSizeInPixels;}
	void WindvectorSizeInPixels(const NFmiPoint &newValue) {itsWindvectorSizeInPixels = newValue;}
	const NFmiPoint& LegendTextSize(void) const {return itsLegendTextSize;}
	void LegendTextSize(const NFmiPoint &newValue) {itsLegendTextSize = newValue;}
    NFmiTempLineInfo& TemperatureLineInfo(void) {return itsTemperatureLineInfo;}
	void TemperatureLineInfo(const NFmiTempLineInfo &newValue) {itsTemperatureLineInfo = newValue;}
    NFmiTempLineInfo& DewPointLineInfo(void) {return itsDewPointLineInfo;}
	void DewPointLineInfo(const NFmiTempLineInfo &newValue) {itsDewPointLineInfo = newValue;}
	bool DrawWinds(void) const {return fDrawWinds;}
	void DrawWinds(bool newValue) {fDrawWinds = newValue;}
	bool DrawLegendText(void) const {return fDrawLegendText;}
	void DrawLegendText(bool newValue) {fDrawLegendText = newValue;}
    NFmiTempLabelInfo& HeightValueLabelInfo(void) {return itsHeightValueLabelInfo;}
	void HeightValueLabelInfo(const NFmiTempLabelInfo &newValue) {itsHeightValueLabelInfo = newValue;}
	bool DrawOnlyHeightValuesOfFirstDrawedSounding(void) const {return fDrawOnlyHeightValuesOfFirstDrawedSounding;}
	void DrawOnlyHeightValuesOfFirstDrawedSounding(bool newValue) {fDrawOnlyHeightValuesOfFirstDrawedSounding = newValue;}
	int SoundingColorCount(void) const {return static_cast<int>(itsSoundingColors.size());}
	void StoreSettings(void); // initialisoinnin (InitializeSoundingColors-metodin) vastakohta eli tallettaa asetukset NFmiSettings-systeemiin.

	int IndexiesFontSize(void) const {return itsIndexiesFontSize;}
	void IndexiesFontSize(int newValue) {itsIndexiesFontSize = newValue;}
	int SoundingTextFontSize(void) const {return itsSoundingTextFontSize;}
	void SoundingTextFontSize(int newValue) {itsSoundingTextFontSize = newValue;}
	bool ShowHodograf(void) const {return fShowHodograf;}
	void ShowHodograf(bool newValue) {fShowHodograf = newValue;}
	bool ShowCondensationTrailProbabilityLines(void) const {return fShowCondensationTrailProbabilityLines;}
	void ShowCondensationTrailProbabilityLines(bool newValue) {fShowCondensationTrailProbabilityLines = newValue;}
	bool ShowKilometerScale(void) const {return fShowKilometerScale;}
	void ShowKilometerScale(bool newValue) {fShowKilometerScale = newValue;}
	bool ShowFlightLevelScale(void) const {return fShowFlightLevelScale;}
	void ShowFlightLevelScale(bool newValue) {fShowFlightLevelScale = newValue;}
	bool ShowOnlyFirstSoundingInHodograf(void) const {return fShowOnlyFirstSoundingInHodograf;}
	void ShowOnlyFirstSoundingInHodograf(bool newValue) {fShowOnlyFirstSoundingInHodograf = newValue;}

	double ResetScalesStartP(void) const {return itsResetScalesStartP;}
	void ResetScalesStartP(double newValue) {itsResetScalesStartP = newValue;}
	double ResetScalesEndP(void) const {return itsResetScalesEndP;}
	void ResetScalesEndP(double newValue) {itsResetScalesEndP = newValue;}
	double ResetScalesStartT(void) const {return itsResetScalesStartT;}
	void ResetScalesStartT(double newValue) {itsResetScalesStartT = newValue;}
	double ResetScalesEndT(void) const {return itsResetScalesEndT;}
	void ResetScalesEndT(double newValue) {itsResetScalesEndT = newValue;}
	double ResetScalesSkewTStartT(void) const {return itsResetScalesSkewTStartT;}
	void ResetScalesSkewTStartT(double newValue) {itsResetScalesSkewTStartT = newValue;}
	double ResetScalesSkewTEndT(void) const {return itsResetScalesSkewTEndT;}
	void ResetScalesSkewTEndT(double newValue) {itsResetScalesSkewTEndT = newValue;}

    NFmiTempLineInfo& AirParcel1LineInfo(void) {return itsAirParcel1LineInfo;}
	void AirParcel1LineInfo(const NFmiTempLineInfo &newValue) {itsAirParcel1LineInfo = newValue;}
    NFmiTempLabelInfo& AirParcel1LabelInfo(void) {return itsAirParcel1LabelInfo;}
	void AirParcel1LabelInfo(const NFmiTempLabelInfo &newValue) {itsAirParcel1LabelInfo = newValue;}
    NFmiTempLineInfo& AirParcel2LineInfo(void) {return itsAirParcel2LineInfo;}
	void AirParcel2LineInfo(const NFmiTempLineInfo &newValue) {itsAirParcel2LineInfo = newValue;}
    NFmiTempLabelInfo& AirParcel2LabelInfo(void) {return itsAirParcel2LabelInfo;}
	void AirParcel2LabelInfo(const NFmiTempLabelInfo &newValue) {itsAirParcel2LabelInfo = newValue;}
    NFmiTempLineInfo& AirParcel3LineInfo(void) {return itsAirParcel3LineInfo;}
	void AirParcel3LineInfo(const NFmiTempLineInfo &newValue) {itsAirParcel3LineInfo = newValue;}
    NFmiTempLabelInfo& AirParcel3LabelInfo(void) {return itsAirParcel3LabelInfo;}
	void AirParcel3LabelInfo(const NFmiTempLabelInfo &newValue) {itsAirParcel3LabelInfo = newValue;}
    NFmiTempLineInfo& WindModificationAreaLineInfo(void) {return itsWindModificationAreaLineInfo;}
	void WindModificationAreaLineInfo(NFmiTempLineInfo &newValue) {itsWindModificationAreaLineInfo = newValue;}

	void ChangeSoundingsInTime(FmiDirection theDirection);
	void NextAnimationStep(void);
	void PreviousAnimationStep(void);
	int AnimationTimeStepInMinutes(void) const {return itsAnimationTimeStepInMinutes;}
	bool LeftMouseDown(void) const {return fLeftMouseDown;}
	void LeftMouseDown(bool newValue) {fLeftMouseDown = newValue;}
	bool RightMouseDown(void) const {return fRightMouseDown;}
	void RightMouseDown(bool newValue) {fRightMouseDown = newValue;}
	bool UpdateFromViewMacro(void) const {return fUpdateFromViewMacro;}
	void UpdateFromViewMacro(bool newValue) {fUpdateFromViewMacro = newValue;}
	int WindBarbSpaceOutFactor(void) const {return itsWindBarbSpaceOutFactor;}
	void WindBarbSpaceOutFactor(int newValue);
	void ChangeWindBarbSpaceOutFactor(void);
	CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(void) {return itsGraphicalInfo;}
	int ModelRunCount(void) const {return itsModelRunCount;}
	void ModelRunCount(int newValue) {itsModelRunCount = newValue;}
	void SetCaseStudyTimes(const NFmiMetTime &theCaseStudyTime);
    bool DrawSecondaryData() const { return fDrawSecondaryData; }
    void DrawSecondaryData(bool newValue) { fDrawSecondaryData = newValue; }
    double SecondaryDataFrameWidthFactor() const { return itsSecondaryDataFrameWidthFactor; }
    void SecondaryDataFrameWidthFactor(double newValue) { itsSecondaryDataFrameWidthFactor = newValue; }
    double UsedSecondaryDataFrameWidthFactor();
    NFmiTempLineInfo& WSLineInfo() { return itsWSLineInfo; }
    void WSLineInfo(const NFmiTempLineInfo &newValue) { itsWSLineInfo = newValue; }
    NFmiTempLineInfo& NLineInfo() { return itsNLineInfo; }
    void NLineInfo(const NFmiTempLineInfo &newValue) { itsNLineInfo = newValue; }
    NFmiTempLineInfo& RHLineInfo() { return itsRHLineInfo; }
    void RHLineInfo(const NFmiTempLineInfo &newValue) { itsRHLineInfo = newValue; }
    SoundingDataServerConfigurations& GetSoundingDataServerConfigurations() { return itsSoundingDataServerConfigurations; }
	HodografViewData& GetHodografViewData() { return itsHodografViewData; }
	SoundingViewSettingsFromWindowsRegisty& GetSoundingViewSettingsFromWindowsRegisty() { return itsSoundingViewSettingsFromWindowsRegisty; }
	int GetSelectedProducerIndex(bool getLimitCheckedIndex) const;
	void SetSelectedProducerIndex(int newValue, bool ignoreHighLimit);
	void ToggleSelectedProducerIndex(FmiDirection direction);

	void Write(std::ostream& os) const;
	void Read(std::istream& is);
private:
    void InitPossibleProducerList(NFmiProducerSystem &theProducerSystem, const std::vector<NFmiProducer>& theExtraSoundingProducers);
	void InitializeSoundingColors();
    void InitializeSoundingDataServerConfigurations();
    std::string MakeSecondaryDataLineInfoString() const;
    void ReadSecondaryDataLineInfoFromString(const std::string &theStr);
    void AddSpecialDataToPossibleProducerList(SelectedProducerContainer &possibleProducerList);
    void AddVerticalModelDataToPossibleProducerList(SelectedProducerContainer &possibleProducerList, NFmiProducerSystem &theProducerSystem);
    void AddExtraSoundingDataToPossibleProducerList(SelectedProducerContainer &possibleProducerList, const std::vector<NFmiProducer>& theExtraSoundingProducers);
    void AddSoundingDataFromServerToPossibleProducerList(SelectedProducerContainer &possibleProducerList);
	bool ShowIndexiesViewMacroLegacy() const;
	bool ShowSideViewViewMacroLegacy() const;
	void SetupSideViewsFromLegacyViewMacroValues(bool showIndexiesLegacyValue, bool showSideViewLegacyValue);
	std::string MakeSelectedProducerStringForViewMacro() const;
	void SetSelectedProducerFromViewMacroString(const std::string &str);

	Container itsTempInfos;
	// MTA-moodissa tämän enempää ei oteta listaan näytettäviä temppejä. Jos joku lisää tempin ja 
	// listassa on jo näin monta, tyhjennetään ensin lista ja lisätään sitten tämä uusi luotaus tieto
	int itsMaxTempsShowed; 
	// Lista kaikista mahdollisista luotaus tuottajista
    SelectedProducerContainer itsPossibleProducerList; 
	// Näiden tuottajien luotauksia näytetään comp-moodissa luotaus näytössä
    SelectedProducerContainer itsSoundingComparisonProducers; 
	// Indeksi edelliseen listaan, -1 jos ei ole valittu mitään
	// HUOM! en muista miksi tämä on oikeasti tehty, mutta en uskalla muuttaa sitä siihen 
	// käyttöön mihin uusi itsSelectedProducerIndex dataosa otetaan käyttöön.
	// Siksi tämä itsSelectedProducer jää sellaisenaan olemaan tässä.
	int itsSelectedProducer; 
	bool fTempViewOn; // onko luotaus ikkuna auki vai kiinni
	double itsSkewTDegree; // tuetaan ainakin 0 ja 45 astetta

	std::vector<NFmiColor> itsSoundingColors; // eri luotaukset piirretään eri väreillä, tähän on ne värit talletettu

	// tähän tulee kaikenlaisia säätöjä luotaus näytöstä.
	// mm. akselien säädöt, eri viivojen värit, viivatyypit ja paksuudet on/off tila ja
	// labelointi on/off, fontti koko, väri jne
	// lisäksi on piirrettävien apuviivojen arvolistat. Kuten mitkä paineet, mixing ratiot jne piirretään
	double itsTAxisStart0Degree; // pysty lämpötila-asteikolle omat arvot
	double itsTAxisEnd0Degree; // pysty lämpötila-asteikolle omat arvot
	double itsTAxisStart45Degree; // näitä käytetään lämpötila asteikolle kaikilla positiivisilla asteilla
	double itsTAxisEnd45Degree; // näitä käytetään lämpötila asteikolle kaikilla positiivisilla asteilla
	double itsTAxisStartNegDegree; // näitä käytetään lämpötila asteikolle kaikilla negatiivisilla asteilla
	double itsTAxisEndNegDegree; // näitä käytetään lämpötila asteikolle kaikilla negatiivisilla asteilla
	double itsTemperatureHelpLineStart; // jos vino T diagrammi, pitää lämpötila apuviivoja piirrellä enemmän, tästä lähtien aletaan piirtämään niitä
	double itsTemperatureHelpLineEnd; // jos vino T diagrammi, pitää lämpötila apuviivoja piirrellä enemmän, tähän asti niitä piirretään
	double itsTemperatureHelpLineStep; // millä jakovälillä lämppärin apuviivoja piirretään
    NFmiTempLineInfo itsTemperatureHelpLineInfo; // lämppäri apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsTemperatureHelpLabelInfo; // lämppäri apuviivojen label tekstin piirto-ominaisuudet


	double itsPAxisStart; // paineasteikon alku (maanpinta eli iso arvo)
	double itsPAxisEnd; // paineasteikon loppu
	std::vector<double> itsPressureValues; // nämä paine pinnat piirretään ja niihin laitetaan label
    NFmiTempLineInfo itsPressureLineInfo; // paine apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsPressureLabelInfo; // paine apuviivojen label tekstin piirto-ominaisuudet
	std::vector<double> itsMixingRatioValues; // nämä mixing ratio arvot piirretään ja niihin laitetaan label
    NFmiTempLineInfo itsMixingRatioLineInfo; // MixingRatio apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsMixingRatioLabelInfo; // MixingRatio apuviivojen label tekstin piirto-ominaisuudet
	std::vector<double> itsMoistAdiabaticValues; // nämä kostea adiabaatti arvot piirretään ja niihin laitetaan label
    NFmiTempLineInfo itsMoistAdiabaticLineInfo; // MoistAdiabatic apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsMoistAdiabaticLabelInfo; // MoistAdiabatic apuviivojen label tekstin piirto-ominaisuudet
	std::vector<double> itsDryAdiabaticValues; // nämä kuiva adiabaatti arvot piirretään ja niihin laitetaan label
    NFmiTempLineInfo itsDryAdiabaticLineInfo; // DryAdiabatic apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsDryAdiabaticLabelInfo; // DryAdiabatic apuviivojen label tekstin piirto-ominaisuudet

    NFmiTempLineInfo itsAirParcel1LineInfo; // Ilmapaketti nostettuna pinta arvojen mukaan apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsAirParcel1LabelInfo; // Ilmapaketti nostettuna pinta arvojen mukaan label tekstin piirto-ominaisuudet
    NFmiTempLineInfo itsAirParcel2LineInfo; // Ilmapaketti nostettuna 500 m mix arvojen mukaan apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsAirParcel2LabelInfo; // Ilmapaketti nostettuna 500 m mix arvojen mukaan label tekstin piirto-ominaisuudet
    NFmiTempLineInfo itsAirParcel3LineInfo; // Ilmapaketti most unstable jutun mukaan
    NFmiTempLabelInfo itsAirParcel3LabelInfo; // Ilmapaketti most unstable jutun mukaan
    NFmiTempLineInfo itsWindModificationAreaLineInfo; // Tuulimuokkaus alueen rajojen piirto

	NFmiPoint itsWindvectorSizeInPixels;
	bool fDrawWinds;
	NFmiPoint itsLegendTextSize;
	bool fDrawLegendText;

    NFmiTempLineInfo itsTemperatureLineInfo; // itse lämpötila luotaus viivojen asetukset
    NFmiTempLineInfo itsDewPointLineInfo; // itse kastepiste luotaus viivojen asetukset

	// korkeus teksti piirretään paine akselin viereen.
    NFmiTempLabelInfo itsHeightValueLabelInfo;
	bool fDrawOnlyHeightValuesOfFirstDrawedSounding;

	int itsIndexiesFontSize; // indeksi taulukon fontin koko pikseleinä
	int itsSoundingTextFontSize; // luotaus tekstinä fontin koko pikseleinä

	bool fShowHodograf;
	bool fShowCondensationTrailProbabilityLines;
	bool fShowKilometerScale;
	bool fShowFlightLevelScale;
	bool fShowMapMarkers;

	bool fShowOnlyFirstSoundingInHodograf;

	bool fLeftMouseDown;
	bool fRightMouseDown;
	bool fInitializationOk; // alustuksen onnistuessa tämä asetetaan true:ksi, jos epäonnistunut, ei talleteta takaisin konffiin, koska voi olla roskaa

	// Nämä ovat luotaus näytön reset-skales buttonia varten, joka asettelee asteikot
	// johonkin sopiviin default arvoihin, jos joku on säädellyt asteikot ihan poskelleen.
	double itsResetScalesStartP;
	double itsResetScalesEndP;
	double itsResetScalesStartT;
	double itsResetScalesEndT;
	double itsResetScalesSkewTStartT;
	double itsResetScalesSkewTEndT;

	int itsAnimationTimeStepInMinutes;
	int itsWindBarbSpaceOutFactor; // 0 = ei harvennusta, 1 vähän ja 2 enemmän

	bool fUpdateFromViewMacro; // tätä käytetään vain kertomaan että luotaus-dialogia päivitetään viewMacrosta ja täytyy toimia hieman erilailla kuin normaalisti
    CtrlViewUtils::GraphicalInfo itsGraphicalInfo;
	int itsModelRunCount; // Kuinka monta viimeisintä malliajoa näytetään luotausnäytössä, jos katsotaan jotain malli luotausta (0 = vain viimeisin data, 1 on viimeisin + edellinen malliajo, 2 = viimeisin ja kaksi edellista jne.)
    bool fDrawSecondaryData; // piirretäänkö apudatat (0-100 asteikkoon WS, N, RH, muita?) vai ei (SmartMetin luotausnäytössä CTRL + F säätää on/off tilaa)
    double itsSecondaryDataFrameWidthFactor; // Kuinka leveä apudatalaatikko on suhteessa koko luotausnäyttöön. Tällä on aina arvo, vaikka apudataa ei piirettäisi.
    NFmiTempLineInfo itsWSLineInfo; // Apudatanäytön WS (tuulen nopeus) viivan piirto-ominaisuudet
    NFmiTempLineInfo itsNLineInfo; // Apudatanäytön N (kokonaispilvisyys) viivan piirto-ominaisuudet
    NFmiTempLineInfo itsRHLineInfo; // Apudatanäytön RH (suhteellinen kosteus prosentti) viivan piirto-ominaisuudet

    // Windows registry:ssä oikeasti talletetut muuttujat, jotka otetaan myös tänne talteen, että ne saadaan mukaan näyttömakroihin
	SoundingViewSettingsFromWindowsRegisty itsSoundingViewSettingsFromWindowsRegisty;
	SoundingDataServerConfigurations itsSoundingDataServerConfigurations;
	HodografViewData itsHodografViewData;
	// Luotausnäytössä voidaan valita multi-select listasta tuottajat, jotka näytetään erivärisillä viivoilla.
	// Tämä mennessä 1. niistä on ollut ns. pääluotaus, josta on piirretty mm. teksti ja indeksi tietoja luotaunäytölle.
	// Nyt halutun luotauksen voi välitä pääluotaukseksi luotausnäytössä CTRL + SHIFT + rullalla.
	// Indeksit alkavat 0:sta ja jos indeksi on isompi kuin valittujen luotausten määrä, valituksi tulee viimeisin tuottaja listalta.
	int itsSelectedProducerIndex = 0;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiMTATempSystem::TempInfo& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMTATempSystem::TempInfo& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiMTATempSystem& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMTATempSystem& item){item.Read(is); return is;}


