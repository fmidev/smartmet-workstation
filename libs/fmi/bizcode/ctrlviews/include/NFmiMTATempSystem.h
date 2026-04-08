//� Ilmatieteenlaitos/Marko.
//Original 28.12.2004
//
// Luokka pit�� tietoa onko MetEditori ns. MTA luotaus moodissa, vai noormaali moodissa.
// Jos ollaan MTA moodissa, pit�� my�s listaa eri luotauksista, mit� n�ytet��n (tiedot
// n�ytt��n lis�tyist� luotauksista pit�� sis�ll��n ajan,paikan ja tuottajan).
//---------------------------------------------------------- NFmiMTATempSystem.h

#pragma once

#include "NFmiPoint.h"
#include "NFmiRect.h"
#include "NFmiMetTime.h"
#include "NFmiProducer.h"
#include "NFmiColor.h"
#include "GraphicalInfo.h"
#include "NFmiTempLineInfo.h"
#include "NFmiTempLabelInfo.h"
#include "SoundingDataServerConfigurations.h"
#ifndef UNIX
#include "SoundingViewSettingsFromWindowsRegisty.h"
#else
// Stub: SoundingViewSettingsFromWindowsRegisty not available on Linux
class SoundingViewSettingsFromWindowsRegisty {
public:
    bool SoundingTextUpward() const { return false; }
    void SoundingTextUpward(bool) {}
    bool SoundingTimeLockWithMapView() const { return false; }
    void SoundingTimeLockWithMapView(bool) {}
    bool ShowStabilityIndexSideView() const { return false; }
    void ShowStabilityIndexSideView(bool) {}
    bool ShowTextualSoundingDataSideView() const { return false; }
    void ShowTextualSoundingDataSideView(bool) {}
};
#endif // UNIX

class NFmiProducerSystem;

//_________________________________________________________ NFmiCrossSectionSystem
class NFmiMTATempSystem
{
 public:
	static double itsLatestVersionNumber;
	mutable double itsCurrentVersionNumber;

	class HodografViewData
	{
		// T�h�n piirret��n hodograafi (relatiivinen laatikko)
		NFmiRect itsRect; 
		// T�m�n avulla s��det��n hodografin arvoalueen suuruutta
		double itsScaleMaxValue = 50.;
		// T�m�n avulla s��det��n hodografin ikkunan suhteellista kokoa
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

    // ServerProducer luokalla on tieto, k�ytt��k� se data l�hteen��n lokaali queryDataa vai serveri�.
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

        // Tehd��n vertailu operaattorit, jotka vertaavat prod-id, name ja ServerProducer tapauksissa my�s useServer -tilaa.
        // Originaali NFmiProducer vertailut vertaavat vain prod-id:t�.
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

    // voidaan helposti siirty� k�ytt�m��n tavallista vector-luokkaa jos haluaa halutaan
	using Container = std::vector<TempInfo>; 
    // SoundingProducer:in bool osa tarkoittaa sit� ett� haetaanko itse luotausdata lokaali queryDatasta (false) vai serverilt� (true)
    using SoundingProducer = ServerProducer;
    using SelectedProducerContainer = std::vector<SoundingProducer>;
    using SelectedProducerLegacyContainer = std::vector<NFmiProducer>;

	NFmiMTATempSystem(void);
	virtual ~NFmiMTATempSystem(void);

	// Heitt�� poikkeuksia virhetilanteissa
    void Init(NFmiProducerSystem &theProducerSystem, const std::vector<NFmiProducer>& theExtraSoundingProducers, const SoundingViewSettingsFromWindowsRegisty & soundingViewSettingsFromWindowsRegisty);
	void InitFromViewMacro(const NFmiMTATempSystem &theOther, bool disableWindowManipulations);
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
	double IntegrationRangeInKm() const { return itsIntegrationRangeInKm; }
	void IntegrationRangeInKm(double newValue);
	double IntegrationTimeOffset1InHours() const { return itsIntegrationTimeOffset1InHours; }
	void IntegrationTimeOffset1InHours(double newValue);
	double IntegrationTimeOffset2InHours() const { return itsIntegrationTimeOffset2InHours; }
	void IntegrationTimeOffset2InHours(double newValue);

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
	// MTA-moodissa t�m�n enemp�� ei oteta listaan n�ytett�vi� temppej�. Jos joku lis�� tempin ja 
	// listassa on jo n�in monta, tyhjennet��n ensin lista ja lis�t��n sitten t�m� uusi luotaus tieto
	int itsMaxTempsShowed; 
	// Lista kaikista mahdollisista luotaus tuottajista
    SelectedProducerContainer itsPossibleProducerList; 
	// N�iden tuottajien luotauksia n�ytet��n comp-moodissa luotaus n�yt�ss�
    SelectedProducerContainer itsSoundingComparisonProducers; 
	// Indeksi edelliseen listaan, -1 jos ei ole valittu mit��n
	// HUOM! en muista miksi t�m� on oikeasti tehty, mutta en uskalla muuttaa sit� siihen 
	// k�ytt��n mihin uusi itsSelectedProducerIndex dataosa otetaan k�ytt��n.
	// Siksi t�m� itsSelectedProducer j�� sellaisenaan olemaan t�ss�.
	int itsSelectedProducer; 
	bool fTempViewOn; // onko luotaus ikkuna auki vai kiinni
	double itsSkewTDegree; // tuetaan ainakin 0 ja 45 astetta

	std::vector<NFmiColor> itsSoundingColors; // eri luotaukset piirret��n eri v�reill�, t�h�n on ne v�rit talletettu

	// t�h�n tulee kaikenlaisia s��t�j� luotaus n�yt�st�.
	// mm. akselien s��d�t, eri viivojen v�rit, viivatyypit ja paksuudet on/off tila ja
	// labelointi on/off, fontti koko, v�ri jne
	// lis�ksi on piirrett�vien apuviivojen arvolistat. Kuten mitk� paineet, mixing ratiot jne piirret��n
	double itsTAxisStart0Degree; // pysty l�mp�tila-asteikolle omat arvot
	double itsTAxisEnd0Degree; // pysty l�mp�tila-asteikolle omat arvot
	double itsTAxisStart45Degree; // n�it� k�ytet��n l�mp�tila asteikolle kaikilla positiivisilla asteilla
	double itsTAxisEnd45Degree; // n�it� k�ytet��n l�mp�tila asteikolle kaikilla positiivisilla asteilla
	double itsTAxisStartNegDegree; // n�it� k�ytet��n l�mp�tila asteikolle kaikilla negatiivisilla asteilla
	double itsTAxisEndNegDegree; // n�it� k�ytet��n l�mp�tila asteikolle kaikilla negatiivisilla asteilla
	double itsTemperatureHelpLineStart; // jos vino T diagrammi, pit�� l�mp�tila apuviivoja piirrell� enemm�n, t�st� l�htien aletaan piirt�m��n niit�
	double itsTemperatureHelpLineEnd; // jos vino T diagrammi, pit�� l�mp�tila apuviivoja piirrell� enemm�n, t�h�n asti niit� piirret��n
	double itsTemperatureHelpLineStep; // mill� jakov�lill� l�mpp�rin apuviivoja piirret��n
    NFmiTempLineInfo itsTemperatureHelpLineInfo; // l�mpp�ri apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsTemperatureHelpLabelInfo; // l�mpp�ri apuviivojen label tekstin piirto-ominaisuudet


	double itsPAxisStart; // paineasteikon alku (maanpinta eli iso arvo)
	double itsPAxisEnd; // paineasteikon loppu
	std::vector<double> itsPressureValues; // n�m� paine pinnat piirret��n ja niihin laitetaan label
    NFmiTempLineInfo itsPressureLineInfo; // paine apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsPressureLabelInfo; // paine apuviivojen label tekstin piirto-ominaisuudet
	std::vector<double> itsMixingRatioValues; // n�m� mixing ratio arvot piirret��n ja niihin laitetaan label
    NFmiTempLineInfo itsMixingRatioLineInfo; // MixingRatio apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsMixingRatioLabelInfo; // MixingRatio apuviivojen label tekstin piirto-ominaisuudet
	std::vector<double> itsMoistAdiabaticValues; // n�m� kostea adiabaatti arvot piirret��n ja niihin laitetaan label
    NFmiTempLineInfo itsMoistAdiabaticLineInfo; // MoistAdiabatic apuviivojen piirto-ominaisuudet
    NFmiTempLabelInfo itsMoistAdiabaticLabelInfo; // MoistAdiabatic apuviivojen label tekstin piirto-ominaisuudet
	std::vector<double> itsDryAdiabaticValues; // n�m� kuiva adiabaatti arvot piirret��n ja niihin laitetaan label
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

    NFmiTempLineInfo itsTemperatureLineInfo; // itse l�mp�tila luotaus viivojen asetukset
    NFmiTempLineInfo itsDewPointLineInfo; // itse kastepiste luotaus viivojen asetukset

	// korkeus teksti piirret��n paine akselin viereen.
    NFmiTempLabelInfo itsHeightValueLabelInfo;
	bool fDrawOnlyHeightValuesOfFirstDrawedSounding;

	int itsIndexiesFontSize; // indeksi taulukon fontin koko pikselein�
	int itsSoundingTextFontSize; // luotaus tekstin� fontin koko pikselein�

	bool fShowHodograf;
	bool fShowCondensationTrailProbabilityLines;
	bool fShowKilometerScale;
	bool fShowFlightLevelScale;
	bool fShowMapMarkers;

	bool fShowOnlyFirstSoundingInHodograf;

	bool fLeftMouseDown;
	bool fRightMouseDown;
	bool fInitializationOk; // alustuksen onnistuessa t�m� asetetaan true:ksi, jos ep�onnistunut, ei talleteta takaisin konffiin, koska voi olla roskaa

	// N�m� ovat luotaus n�yt�n reset-skales buttonia varten, joka asettelee asteikot
	// johonkin sopiviin default arvoihin, jos joku on s��dellyt asteikot ihan poskelleen.
	double itsResetScalesStartP;
	double itsResetScalesEndP;
	double itsResetScalesStartT;
	double itsResetScalesEndT;
	double itsResetScalesSkewTStartT;
	double itsResetScalesSkewTEndT;

	int itsAnimationTimeStepInMinutes;
	int itsWindBarbSpaceOutFactor; // 0 = ei harvennusta, 1 v�h�n ja 2 enemm�n

	bool fUpdateFromViewMacro; // t�t� k�ytet��n vain kertomaan ett� luotaus-dialogia p�ivitet��n viewMacrosta ja t�ytyy toimia hieman erilailla kuin normaalisti
    CtrlViewUtils::GraphicalInfo itsGraphicalInfo;
	int itsModelRunCount; // Kuinka monta viimeisint� malliajoa n�ytet��n luotausn�yt�ss�, jos katsotaan jotain malli luotausta (0 = vain viimeisin data, 1 on viimeisin + edellinen malliajo, 2 = viimeisin ja kaksi edellista jne.)
    bool fDrawSecondaryData; // piirret��nk� apudatat (0-100 asteikkoon WS, N, RH, muita?) vai ei (SmartMetin luotausn�yt�ss� CTRL + F s��t�� on/off tilaa)
    double itsSecondaryDataFrameWidthFactor; // Kuinka leve� apudatalaatikko on suhteessa koko luotausn�ytt��n. T�ll� on aina arvo, vaikka apudataa ei piirett�isi.
    NFmiTempLineInfo itsWSLineInfo; // Apudatan�yt�n WS (tuulen nopeus) viivan piirto-ominaisuudet
    NFmiTempLineInfo itsNLineInfo; // Apudatan�yt�n N (kokonaispilvisyys) viivan piirto-ominaisuudet
    NFmiTempLineInfo itsRHLineInfo; // Apudatan�yt�n RH (suhteellinen kosteus prosentti) viivan piirto-ominaisuudet

    // Windows registry:ss� oikeasti talletetut muuttujat, jotka otetaan my�s t�nne talteen, ett� ne saadaan mukaan n�ytt�makroihin
	SoundingViewSettingsFromWindowsRegisty itsSoundingViewSettingsFromWindowsRegisty;
	SoundingDataServerConfigurations itsSoundingDataServerConfigurations;
	HodografViewData itsHodografViewData;
	// Luotausn�yt�ss� voidaan valita multi-select listasta tuottajat, jotka n�ytet��n eriv�risill� viivoilla.
	// T�m� menness� 1. niist� on ollut ns. p��luotaus, josta on piirretty mm. teksti ja indeksi tietoja luotaun�yt�lle.
	// Nyt halutun luotauksen voi v�lit� p��luotaukseksi luotausn�yt�ss� CTRL + SHIFT + rullalla.
	// Indeksit alkavat 0:sta ja jos indeksi on isompi kuin valittujen luotausten m��r�, valituksi tulee viimeisin tuottaja listalta.
	int itsSelectedProducerIndex = 0;
	// Luotauksien kokoomalaskentoihin liittyv�t asetukset
	// Jos itsIntegrationRangeInKm arvo on 0, ei tehd� kokoomalaskuja alueen suhteen ollenkaan
	double itsIntegrationRangeInKm = 0;
	// Jos aikakokooma asetukset ovat 0:aa, ei tehd� ajansuhteen mit��n kokoomalaskuja
	double itsIntegrationTimeOffset1InHours = 0;
	double itsIntegrationTimeOffset2InHours = 0;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiMTATempSystem::TempInfo& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMTATempSystem::TempInfo& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiMTATempSystem& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiMTATempSystem& item){item.Read(is); return is;}


