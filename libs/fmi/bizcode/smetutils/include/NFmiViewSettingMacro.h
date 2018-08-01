// ======================================================================
/*!
 * \file NFmiViewSettingMacro.h
 * \brief Interface of class NFmiViewSettingMacro
 */
// ======================================================================

#pragma once

#include "NFmiDrawParam.h"
#include "NFmiLevel.h"
#include "NFmiRect.h"
#include "NFmiCalculationCondition.h"
#include "NFmiProjectionCurvatureInfo.h"
#include "NFmiPtrList.h"
#include "NFmiDataMatrix.h"
#include "NFmiMTATempSystem.h"
#include "NFmiTrajectorySystem.h"
#include "NFmiCrossSectionSystem.h"
#include "NFmiSynopPlotSettings.h"
#include "NFmiObsComparisonInfo.h"
#include "HakeMessage/HakeSystemConfigurations.h"
#include "NFmiMapViewDescTop.h"

class NFmiArea;
class NFmiDrawParamList;
class NFmiAreaMaskList;
class NFmiMacroParamSystem;
class NFmiMapViewWinRegistry;

// Tähän luetaan vain viewmacron nimi (tiedosto nimestä) ja description.
// Alustus on nopeaa ja näiden avulla päivitetään viewMacro-dialogin listaa.
class NFmiLightWeightViewSettingMacro
{
public:
    NFmiLightWeightViewSettingMacro()
        :fIsViewMacroDirectory(false)
        ,fViewMacroOk(true)
    {}

    bool ViewMacroOk() const;
    bool IsEmpty() const;

    std::string itsName;
    std::string itsDescription;
    std::string itsInitFilePath;
    bool fIsViewMacroDirectory;
    bool fViewMacroOk; // Jos erityisesti havaitaan että näyttömakro tiedostossa on jotain vikaa, laitetaan tämä false tilaan.
    NFmiTime itsFileModificationTime;

    void Read(std::istream& is);
};

class NFmiViewSettingMacro
{
public:
	static double itsLatestVersionNumber;
	mutable double itsCurrentVersionNumber;

    class MfcViewStatus
    {
        unsigned int itsShowCommand = 0; // minimized/maximized, ei muita tietoja
        bool fShowWindow = false; // onko ikkuna auki vai ei
    public:
        unsigned int ShowCommand() const { return itsShowCommand; }
        void ShowCommand(unsigned int newValue) {itsShowCommand = newValue;}
        bool ShowWindow() const { return fShowWindow; }
        void ShowWindow(bool newValue) { fShowWindow = newValue; }
    };

	class Param
	{
	public:
		Param(void);
		Param(const boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiLevel &theLevel, NFmiInfoData::Type theDataType, int theModelOrigTimeOffsetInHours);
		Param(const NFmiDataIdent &theDataIdent, const NFmiLevel &theLevel, NFmiInfoData::Type theDataType, int theModelOrigTimeOffsetInHours);
		~Param(void);

		const boost::shared_ptr<NFmiDrawParam>& DrawParam(void) const {return itsDrawParam;}
		void DrawParam(const boost::shared_ptr<NFmiDrawParam> &newValue);
		const NFmiDataIdent& DataIdent(void) const {return itsDataIdent;}
		void DataIdent(const NFmiDataIdent& newValue) {itsDataIdent = newValue;}
		const NFmiLevel& Level(void) const {return itsLevel;}
		void Level(const NFmiLevel& newValue) {itsLevel = newValue;}
		NFmiInfoData::Type DataType(void) const {return itsDataType;}
		void DataType(NFmiInfoData::Type newValue) {itsDataType = newValue;}
		int ModelOrigTimeOffsetInHours(void) const {return itsModelOrigTimeOffsetInHours;}
		void ModelOrigTimeOffsetInHours(int newValue) {itsModelOrigTimeOffsetInHours = newValue;}

		bool Hidden(void) const {return fHidden;}
		void Hidden(bool newValue) {fHidden = newValue;}
		bool Active(void) const {return fActive;}
		void Active(bool newValue) {fActive = newValue;}
		bool ShowTimeDifference(void) const {return fShowTimeDifference;}
		void ShowTimeDifference(bool newValue) {fShowTimeDifference = newValue;}
		bool ShowDifferenceToOriginalData(void) const {return fShowDifferenceToOriginalData;}
		void ShowDifferenceToOriginalData(bool newValue) {fShowDifferenceToOriginalData = newValue;}
		void SetMacroParamInitFileNames(const std::string &theRootPath);

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		boost::shared_ptr<NFmiDrawParam> itsDrawParam; // sis. mm. data identin // talletetaan koko drawparam, että voisi tehdä esim. eri värisiä/erilaisia virityksiä samalle parametrille / eri tuottajalle
		NFmiDataIdent itsDataIdent; // tämä pitää olla erikseen, koska drawparam ei talleta sen omaa dataidenttiään.
		NFmiLevel itsLevel;
		NFmiInfoData::Type itsDataType;
		int itsModelOrigTimeOffsetInHours; // tämä kertoo mahdollisen ed. hirlam vai sitä edellinen esim. -6h tai -12h

		// ***********************************************************************************************************
		// Nämä ovat DrawParamin ominaisuuksia, mutta koska niitä ei talleteta tiedostoon, talletan ne tähän
		// koska en halua juuri nyt muuttaa drawparamin talletus versiota.
		bool fHidden;	// näyttö voidaan piiloittaa tämän mukaisesti
		bool fActive;	// onko kyseinen parametri näytön aktiivinen parametri (jokaisella näyttörivillä aina yksi aktivoitunut parametri)
		bool fShowTimeDifference;	// näytetäänkö kartalla parametrin arvo, vai erotus edelliseen aikaan (ei ole vielä talletettu tiedostoon)
		bool fShowDifferenceToOriginalData; // ero originaaliin dataan karttanäytössä
		// ***********************************************************************************************************

	};

	class Mask
	{
	public:
		Mask(void);
		Mask(const Param &theParamSettings, const NFmiCalculationCondition &theMaskSettings, bool theMaskEnabled);
		~Mask(void);

		const Param& ParamSettings(void) const {return itsParamSettings;}
		void ParamSettings(const Param& newValue) {itsParamSettings = newValue;}
		const NFmiCalculationCondition& MaskSettings(void) const {return itsMaskSettings;}
		void  MaskSettings(const NFmiCalculationCondition& newValue) {itsMaskSettings = newValue;}
		bool MaskEnabled(void) const {return fMaskEnabled;}
		void MaskEnabled(bool newValue) {fMaskEnabled = newValue;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		Param itsParamSettings;
		NFmiCalculationCondition itsMaskSettings;
		bool fMaskEnabled;
	};

	// sisältää maski listan ja yleiset asetukset maskeille
	class MaskSettings
	{
	public:
		MaskSettings(void);
		MaskSettings(const checkedVector<Mask> &theMasks, bool theShowMasksOnMapView, bool theUseMasksInTimeSerialViews, bool theUseMasksWithFilterTool, bool theUseMaskWithBrush);
		~MaskSettings(void);

		void SetAllMasks(NFmiAreaMaskList &theMasks);
		const checkedVector<Mask>& Masks(void) const {return itsMasks;}
		void Masks(const checkedVector<Mask>& newValue) {itsMasks = newValue;}
		bool ShowMasksOnMapView(void) const {return fShowMasksOnMapView;}
		void ShowMasksOnMapView(bool newValue) {fShowMasksOnMapView = newValue;}
		bool UseMasksInTimeSerialViews(void) const {return fUseMasksInTimeSerialViews;}
		void UseMasksInTimeSerialViews(bool newValue) {fUseMasksInTimeSerialViews = newValue;}
		bool UseMasksWithFilterTool(void) const {return fUseMasksWithFilterTool;}
		void UseMasksWithFilterTool(bool newValue) {fUseMasksWithFilterTool = newValue;}
		bool UseMaskWithBrush(void) const {return fUseMaskWithBrush;}
		void UseMaskWithBrush(bool newValue) {fUseMaskWithBrush = newValue;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		void Clear(void);
		void Add(const Mask &theMask);

		checkedVector<Mask> itsMasks;
		bool fShowMasksOnMapView;
		bool fUseMasksInTimeSerialViews;
		bool fUseMasksWithFilterTool;
		bool fUseMaskWithBrush;
	};

	class MapRow
	{
	public:
		MapRow(void);
		MapRow(const checkedVector<Param>& theParams);
		~MapRow(void);

		const checkedVector<Param>& RowParams(void) const {return itsRowParams;}
		void RowParams(const checkedVector<Param>& newValue) {itsRowParams = newValue;}
		void Clear(void);
		void Add(const Param &theParam);
		void SetMacroParamInitFileNames(const std::string &theRootPath);

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		checkedVector<Param> itsRowParams;
	};

	class TimeViewRow
	{
	public:
		TimeViewRow(void);
		TimeViewRow(const Param& theParam);
		~TimeViewRow(void);

		const NFmiViewSettingMacro::Param& Param(void) const {return itsParam;}
		void Param(const NFmiViewSettingMacro::Param& newValue) {itsParam = newValue;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		NFmiViewSettingMacro::Param itsParam;
	};

	class GeneralDoc
	{
	public:
		GeneralDoc(void);
		~GeneralDoc(void);

		const NFmiProjectionCurvatureInfo& ProjectionCurvatureInfo(void) const {return itsProjectionCurvatureInfo;}
		void ProjectionCurvatureInfo(const NFmiProjectionCurvatureInfo& newValue) {itsProjectionCurvatureInfo = newValue;}
		const checkedVector<NFmiPoint>& CPLocationVector(void) const {return itsCPLocationVector;}
		void CPLocationVector(const checkedVector<NFmiPoint>& newValue) {itsCPLocationVector = newValue;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:

		NFmiProjectionCurvatureInfo itsProjectionCurvatureInfo; // kartan päälle piirrettävät lat-lon apuviivasto asetukset
		checkedVector<NFmiPoint> itsCPLocationVector; // controllipisteet talteen (latlon pisteet)
	};

	class TimeView
	{
	public:
		TimeView(void);
		~TimeView(void);

		void SetAllParams(NFmiDrawParamList *theDrawParamList);

		const checkedVector<TimeViewRow>& Rows(void) const {return itsRows;}
		void Rows(const checkedVector<TimeViewRow>& newValue) {itsRows = newValue;}
		const NFmiRect& AbsolutRect(void) const {return itsAbsolutRect;}
		void AbsolutRect(const NFmiRect& newValue) {itsAbsolutRect = newValue;}
        const MfcViewStatus& ViewStatus() const { return itsViewStatus; }
        MfcViewStatus& ViewStatus() { return itsViewStatus; }
		bool ShowHelpData(void) const {return fShowHelpData;}
		void ShowHelpData(bool newValue) {fShowHelpData = newValue;}
        bool ShowHelpData2(void) const { return fShowHelpData2; }
        void ShowHelpData2(bool newValue) { fShowHelpData2 = newValue; }
        bool ShowHelpData3(void) const { return fShowHelpData3; }
        void ShowHelpData3(bool newValue) { fShowHelpData3 = newValue; }
        bool ShowHelpData4(void) const { return fShowHelpData4; }
        void ShowHelpData4(bool newValue) { fShowHelpData4 = newValue; }
        int StartTimeOffset(void) const {return itsStartTimeOffset;}
		void StartTimeOffset(int newValue) {itsStartTimeOffset = newValue;}
		int EndTimeOffset(void) const {return itsEndTimeOffset;}
		void EndTimeOffset(int newValue) {itsEndTimeOffset = newValue;}
		const NFmiTimeBag& TimeBag(void) const {return itsTimeBag;}
		void TimeBag(const NFmiTimeBag &newTimes) {itsTimeBag = newTimes;}
		bool TimeBagUpdated(void) const {return fTimeBagUpdated;}
		void TimeBagUpdated(bool newValue) {fTimeBagUpdated = newValue;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		void Clear(void);
		void Add(const TimeViewRow &theTimeViewRow);

		checkedVector<TimeViewRow> itsRows; //  kaikkien karttanäyttörivien asetukset
		NFmiRect itsAbsolutRect; // näytön koko ja sijainti pixeleissä
        MfcViewStatus itsViewStatus;
        bool fShowHelpData; // näytetäänkö malli+havainto+klimatologiset data operatiivisessä käytössä aikasarjassa editoidun datan ohella
        bool fShowHelpData2; // näytetäänkö lyhyt fraktiili mallidata aikasarjanäytössä
        bool fShowHelpData3; // näytetäänkö mallidatasta lasketun klimatologia data aikasarjanäytössä
        bool fShowHelpData4; // näytetäänkö jostain apudatasta T-min ja T-max lämpötilakäyriä apuna aikasarjanäytössä
		int itsStartTimeOffset; // jos aikaikkunaa on zoomattu ajallisesti, tässä offsetti editoitavan datan alkuaikaan
		int itsEndTimeOffset; // jos aikaikkunaa on zoomattu ajallisesti, tässä offsetti editoitavan datan loppuaikaan
		NFmiTimeBag itsTimeBag; // tässä on aikasarjassa käytetty timebag (talletetaan siirtymänä kellonajasta)
		bool fTimeBagUpdated; // jos timebagi on luettu, asetetaan tähän true
	};

	class TempView
	{
	public:
		TempView(void);
		~TempView(void);

		const NFmiRect& AbsolutRect(void) const {return itsAbsolutRect;}
		void AbsolutRect(const NFmiRect& newValue) {itsAbsolutRect = newValue;}
        const MfcViewStatus& ViewStatus() const { return itsViewStatus; }
        MfcViewStatus& ViewStatus() { return itsViewStatus; }
        bool ShowHirlam(void) const {return fShowHirlam;}
		void ShowHirlam(bool newValue) {fShowHirlam = newValue;}
		bool ShowEcmwf(void) const {return fShowEcmwf;}
		void ShowEcmwf(bool newValue) {fShowEcmwf = newValue;}
		bool ShowRealSounding(void) const {return fShowRealSounding;}
		void ShowRealSounding(bool newValue) {fShowRealSounding = newValue;}
		const NFmiMTATempSystem& MTATempSystem(void) const {return itsMTATempSystem;}
		void MTATempSystem(const NFmiMTATempSystem &theData);
		double CurrentVersionNumber(void) {return itsCurrentVersionNumber;}
		void CurrentVersionNumber(double newValue) {itsCurrentVersionNumber = newValue;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		NFmiRect itsAbsolutRect; // näytön koko ja sijainti pixeleissä
        MfcViewStatus itsViewStatus;

		bool fShowHirlam; // onko hirlam näyttö ruksi päällä vai ei
		bool fShowEcmwf; // onko ecmwf näyttö ruksi päällä vai ei
		bool fShowRealSounding; // Tulevaisuudessa: onko havainto näyttö ruksi päällä vai ei

		NFmiMTATempSystem itsMTATempSystem;
		double itsCurrentVersionNumber; // tämä ei ole talletettuna TempView dataosiossa tiedostossa vaan tämä annetaan NFmiViewSettingMacro:lta tänne käyttöön
	};

	class TrajectoryView
	{
	public:
		TrajectoryView(void);
		~TrajectoryView(void);

		const NFmiRect& AbsolutRect(void) const {return itsAbsolutRect;}
		void AbsolutRect(const NFmiRect& newValue) {itsAbsolutRect = newValue;}
        const MfcViewStatus& ViewStatus() const { return itsViewStatus; }
        MfcViewStatus& ViewStatus() { return itsViewStatus; }
        const NFmiTrajectorySystem& TrajectorySystem(void) const {return itsTrajectorySystem;}
		void TrajectorySystem(const NFmiTrajectorySystem &theData);
		double CurrentVersionNumber(void) {return itsCurrentVersionNumber;}
		void CurrentVersionNumber(double newValue) {itsCurrentVersionNumber = newValue;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		NFmiRect itsAbsolutRect; // näytön koko ja sijainti pixeleissä
        MfcViewStatus itsViewStatus;

		NFmiTrajectorySystem itsTrajectorySystem;
		double itsCurrentVersionNumber; // tämä ei ole talletettuna TempView dataosiossa tiedostossa vaan tämä annetaan NFmiViewSettingMacro:lta tänne käyttöön
	};

	class CrossSectionView
	{
	public:

		CrossSectionView(void);
		~CrossSectionView(void);
		CrossSectionView(const CrossSectionView &theOther);
		CrossSectionView& operator=(const CrossSectionView &theOther);

		void SetAllRowParams(NFmiPtrList<NFmiDrawParamList> *theDrawParamListVector, NFmiMacroParamSystem& theMacroParamSystem);
		const checkedVector<MapRow>& MapRowSettings(void) const {return itsMapRowSettings;}
		void MapRowSettings(const checkedVector<MapRow>& newValue) {itsMapRowSettings = newValue;}
		const NFmiRect& AbsolutRect(void) const {return itsAbsolutRect;}
		void AbsolutRect(const NFmiRect& newValue) {itsAbsolutRect = newValue;}
        const MfcViewStatus& ViewStatus() const { return itsViewStatus; }
        MfcViewStatus& ViewStatus() { return itsViewStatus; }
        void SetMacroParamInitFileNames(const std::string &theRootPath);
		const NFmiCrossSectionSystem& CrossSectionSystem(void) const {return itsCrossSectionSystem;}
		void CrossSectionSystem(const NFmiCrossSectionSystem &theData) {itsCrossSectionSystem = theData;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);

	private:
		void Clear(void);
		void Add(const MapRow &theMapRow);

		checkedVector<MapRow> itsMapRowSettings; //  kaikkien karttanäyttörivien asetukset
		NFmiCrossSectionSystem itsCrossSectionSystem;
		NFmiRect itsAbsolutRect; // näytön koko ja sijainti pixeleissä
        MfcViewStatus itsViewStatus;
    };

	class MapViewDescTop
	{
	public:

		MapViewDescTop(void);
		~MapViewDescTop(void);

		void SetAllRowParams(NFmiPtrList<NFmiDrawParamList> *theDrawParamListVector, NFmiMacroParamSystem& theMacroParamSystem);
		const checkedVector<MapRow>& MapRowSettings(void) const {return itsMapRowSettings;}
		void MapRowSettings(const checkedVector<MapRow>& newValue) {itsMapRowSettings = newValue;}
		const NFmiRect& AbsolutRect(void) const {return itsAbsolutRect;}
		void AbsolutRect(const NFmiRect& newValue) {itsAbsolutRect = newValue;}
        const MfcViewStatus& ViewStatus() const { return itsViewStatus; }
        MfcViewStatus& ViewStatus() { return itsViewStatus; }
        void SetMacroParamInitFileNames(const std::string &theRootPath);
		const NFmiMapViewDescTop& GetMapViewDescTop(void) const {return itsMapViewDescTop;}
		void SetMapViewDescTop(const NFmiMapViewDescTop &theData, NFmiMapViewWinRegistry &theMapViewWinRegistry);

		const std::vector<NFmiMapViewDescTop::ViewMacroDipMapHelper>& DipMapHelperList(void) const {return itsDipMapHelperList;}
		void DipMapHelperList(const std::vector<NFmiMapViewDescTop::ViewMacroDipMapHelper> &theList) {itsDipMapHelperList = theList;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);

	private:
		void Clear(checkedVector<MapRow> &theMapRowSettings);
		void Add(checkedVector<MapRow> &theMapRowSettings, const MapRow &theMapRow);

		checkedVector<MapRow> itsMapRowSettings; //  kaikkien karttanäyttörivien asetukset
		NFmiMapViewDescTop itsMapViewDescTop;
		NFmiRect itsAbsolutRect; // näytön koko ja sijainti pixeleissä
        MfcViewStatus itsViewStatus;

		// dipmaphandler osio pitää ottaa tähän erikois käsittelyyn
		std::vector<NFmiMapViewDescTop::ViewMacroDipMapHelper> itsDipMapHelperList;
	};

	class WarningCenterView
	{
	public:

		WarningCenterView(void);
		~WarningCenterView(void);

		const NFmiRect& AbsolutRect(void) const {return itsAbsolutRect;}
		void AbsolutRect(const NFmiRect& newValue) {itsAbsolutRect = newValue;}
        const MfcViewStatus& ViewStatus() const { return itsViewStatus; }
        MfcViewStatus& ViewStatus() { return itsViewStatus; }
        const HakeLegacySupport::HakeSystemConfigurations& WarningCenterSystem(void) const {return itsWarningCenterSystem;}
		void WarningCenterSystem(const HakeLegacySupport::HakeSystemConfigurations &theData) {itsWarningCenterSystem.Init(theData);}
		const std::vector<int>& HeaderColumnWidthsInPixels(void) const {return itsHeaderColumnWidthsInPixels;}
		void HeaderColumnWidthsInPixels(const std::vector<int> &theValues) {itsHeaderColumnWidthsInPixels = theValues;}
        bool ShowHakeMessages() const { return fShowHakeMessages; }
        void ShowHakeMessages(bool newValue) { fShowHakeMessages = newValue; }
        bool ShowKaHaMessages() const { return fShowKaHaMessages; }
        void ShowKaHaMessages(bool newValue) { fShowKaHaMessages = newValue; }
        int MinimumTimeRangeForWarningsOnMapViewsInMinutes() const { return itsMinimumTimeRangeForWarningsOnMapViewsInMinutes; }
        void MinimumTimeRangeForWarningsOnMapViewsInMinutes(int newValue) { itsMinimumTimeRangeForWarningsOnMapViewsInMinutes = newValue; }

		void Write(std::ostream& os) const;
		void Read(std::istream& is);

	private:
        HakeLegacySupport::HakeSystemConfigurations itsWarningCenterSystem;
		NFmiRect itsAbsolutRect; // näytön koko ja sijainti pixeleissä
        MfcViewStatus itsViewStatus;
        std::vector<int> itsHeaderColumnWidthsInPixels;
        bool fShowHakeMessages; // HAKE sanomien checkbox asetus dialogissa
        bool fShowKaHaMessages; // KaHa sanomien checkbox asetus dialogissa
        int itsMinimumTimeRangeForWarningsOnMapViewsInMinutes; // Ks. NFmiApplicationWinRegistry.h:sta vastaavan member-datan selitys.
	};

	class SynopDataGridView
	{
	public:

		SynopDataGridView(void);
		~SynopDataGridView(void);

		const NFmiRect& AbsolutRect(void) const {return itsAbsolutRect;}
		void AbsolutRect(const NFmiRect& newValue) {itsAbsolutRect = newValue;}
        const MfcViewStatus& ViewStatus() const { return itsViewStatus; }
        MfcViewStatus& ViewStatus() { return itsViewStatus; }

		const NFmiProducer& SelectedProducer(void) const {return itsSelectedProducer;}
		void SelectedProducer(const NFmiProducer &theValue) {itsSelectedProducer = theValue;}
		const NFmiMetTime& MinMaxRangeStartTime(void) const {return itsMinMaxRangeStartTime;}
		void MinMaxRangeStartTime(const NFmiMetTime &theValue) {itsMinMaxRangeStartTime = theValue;}
		bool MinMaxModeOn(void) const {return fMinMaxModeOn;}
		void MinMaxModeOn(bool theValue) {fMinMaxModeOn = theValue;}
		double DayRangeValue(void) const {return itsDayRangeValue;}
		void DayRangeValue(double newValue) {itsDayRangeValue = newValue;}
		bool AllCountriesSelected(void) const {return fAllCountriesSelected;}
		void AllCountriesSelected(bool newValue) {fAllCountriesSelected = newValue;}
		const std::string& SelectedCountryAbbrStr(void) const {return itsSelectedCountryAbbrStr;}
		void SelectedCountryAbbrStr(const std::string &theValue) {itsSelectedCountryAbbrStr = theValue;}
		const std::vector<int>& HeaderColumnWidthsInPixels(void) const {return itsHeaderColumnWidthsInPixels;}
		void HeaderColumnWidthsInPixels(const std::vector<int> &theValues) {itsHeaderColumnWidthsInPixels = theValues;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);

	private:
		NFmiRect itsAbsolutRect; // näytön koko ja sijainti pixeleissä
        MfcViewStatus itsViewStatus;

		NFmiProducer itsSelectedProducer;
		NFmiMetTime itsMinMaxRangeStartTime; // tätä  säädetään aika ja kalenteri kontrollien avulla
		bool fMinMaxModeOn;
		double itsDayRangeValue;

		// maafiltteri osio
		bool fAllCountriesSelected;
		std::string itsSelectedCountryAbbrStr; // lista valituista maista maan lyhenne stringien mukaan pilkuilla erotettuna
		std::vector<int> itsHeaderColumnWidthsInPixels;

	};

	class SynopPlotSettings
	{
	public:
		SynopPlotSettings(void);
		~SynopPlotSettings(void);

		const NFmiSynopPlotSettings& GetSynopPlotSettings(void) const {return itsSynopPlotSettings;}
		void SetSynopPlotSettings(const NFmiSynopPlotSettings &theData) {itsSynopPlotSettings = theData;};

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:

		NFmiSynopPlotSettings itsSynopPlotSettings;
	};

	class ObsComparisonInfo
	{
	public:
		ObsComparisonInfo(void);
		~ObsComparisonInfo(void);

		const NFmiObsComparisonInfo& GetObsComparisonInfo(void) const {return itsObsComparisonInfo;}
		void SetObsComparisonInfo(const NFmiObsComparisonInfo &theData) {itsObsComparisonInfo = theData;};

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:

		NFmiObsComparisonInfo itsObsComparisonInfo;
	};


	NFmiViewSettingMacro(void);
	~NFmiViewSettingMacro(void);

	const std::string& Name(void) const {return itsName;}
	void Name(const std::string& newValue) {itsName = newValue;}
	const std::string& Description(void) const {return itsDescription;}
	void Description(const std::string& newValue) {itsDescription = newValue;}
	const std::string& InitFileName(void) const {return itsInitFileName;}
	void InitFileName(const std::string &theFileName) {itsInitFileName = theFileName;}

	GeneralDoc& GetGeneralDoc(void) {return itsGeneralDoc;} // arvojen asetus tätä kautta
	TimeView& GetTimeView(void) {return itsTimeView;} // arvojen asetus tätä kautta
	TempView& GetTempView(void) {return itsTempView;} // arvojen asetus tätä kautta
	TrajectoryView& GetTrajectoryView(void) {return itsTrajectoryView;} // arvojen asetus tätä kautta
	CrossSectionView& GetCrossSectionView(void) {return itsCrossSectionView;}
	SynopPlotSettings& GetSynopPlotSettings(void) {return itsSynopPlotSettings;}
	ObsComparisonInfo& GetObsComparisonInfo(void) {return itsObsComparisonInfo;}
	MaskSettings& GetMaskSettings(void) {return itsMaskSettings;} // arvojen asetus tätä kautta
	WarningCenterView& GetWarningCenterView(void) {return itsWarningCenterView;}
	SynopDataGridView& GetSynopDataGridView(void) {return itsSynopDataGridView;}
	std::vector<MapViewDescTop>& ExtraMapViewDescTops(void) {return itsExtraMapViewDescTops;}

	bool IsPrinterPortrait(void) const {return fIsPrinterPortrait;}
	void IsPrinterPortrait(bool newValue) {fIsPrinterPortrait = newValue;}
	bool IsViewMacroDirectory(void) const {return fViewMacroDirectory;}
	void IsViewMacroDirectory(bool newState) {fViewMacroDirectory = newState;}
	bool UseBrushTool(void) const {return fUseBrushTool;}
	void UseBrushTool(bool newValue) {fUseBrushTool = newValue;}
	bool UseAnalyzeTool(void) const {return fUseAnalyzeTool;}
	void UseAnalyzeTool(bool newValue) {fUseAnalyzeTool = newValue;}
	bool UseControlPoinTool(void) const {return fUseControlPoinTool;}
	void UseControlPoinTool(bool newValue) {fUseControlPoinTool = newValue;}
	bool UseAnimationTool(void) const {return fUseAnimationTool;}
	void UseAnimationTool(bool newValue) {fUseAnimationTool = newValue;}
	int AnimationStartPosition(void) const {return itsAnimationStartPosition;}
	void AnimationStartPosition(int newValue) {itsAnimationStartPosition = newValue;}
	int AnimationEndPosition(void) const {return itsAnimationEndPosition;}
	void AnimationEndPosition(int newValue) {itsAnimationEndPosition = newValue;}
	int AnimationDelayInMS(void) const {return itsAnimationDelayInMS;}
	void AnimationDelayInMS(int newValue) {itsAnimationDelayInMS = newValue;}
	void SetMacroParamInitFileNames(const std::string &theRootPath);
	double OriginalLoadVersionNumber(void) const {return itsOriginalLoadVersionNumber;}
	bool ViewMacroWasCorrupted(void) const {return fViewMacroWasCorrupted;}
	void ViewMacroWasCorrupted(bool newValue) {fViewMacroWasCorrupted = newValue;}

	// HUOM!! Tämä laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin tämä luokka luetaan sisään tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan.
	void Read(std::istream& is);
private:

	// talletan nämä NFmiString:einä, että luku ja kirjoitus menevät ok vaikka olisi white spaceja
	mutable std::string itsName; // macron nimi -> tiedoston nimi kun lisätään polku ja tyyppi
	mutable std::string itsDescription; // macron pidempi kuvaus
	std::string itsInitFileName; // kun makro ladataan, laitetaan tähän tiedoston nimi polun kanssa, että makro voidaan tarvittaessa ladata uudestaan

	GeneralDoc itsGeneralDoc;
	TimeView itsTimeView;
	TempView itsTempView;
	MaskSettings itsMaskSettings; //  kaikkien maskien asetukset
	bool fIsPrinterPortrait;

	bool fViewMacroDirectory; // jos tämä on true, valinta dialogissa toimitaan erilailla
							  // lisäksi tälläistä makroa ei talleteta tiedostoon
							  // Tämän avulla on tarkoitus laittaa kansio systeemi näyttömakroihin

	bool fViewMacroWasCorrupted; // jos tiedoston luku epäonnistui, on se merkittävä korruptoituneeksi
	// mitkä työkalut ovat päällä
	bool fUseBrushTool;
	bool fUseAnalyzeTool;
	bool fUseControlPoinTool;
	bool fUseAnimationTool;
	// animaatio asetukset
	int itsAnimationStartPosition; // kuinka monennestako framesta editoitavaa dataa aloitetaan
	int itsAnimationEndPosition; // kuinka monennestako framesta editoitavaa dataa lopetetaan
	int itsAnimationDelayInMS; // viiva animaatiossa millisekunneissa

	double itsOriginalLoadVersionNumber; // kun viewMacro ladattiin, tämä oli sen alkuperäinen versio
	// versio 2 osasia
	TrajectoryView itsTrajectoryView;
	CrossSectionView itsCrossSectionView;
	SynopPlotSettings itsSynopPlotSettings;
	ObsComparisonInfo itsObsComparisonInfo;
	WarningCenterView itsWarningCenterView;
	SynopDataGridView itsSynopDataGridView;
	std::vector<MapViewDescTop> itsExtraMapViewDescTops;
};

inline std::istream& operator>>(std::istream& is, NFmiLightWeightViewSettingMacro& item){ item.Read(is); return is; }

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::Param& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::Param& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::Mask& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::Mask& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::MaskSettings& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::MaskSettings& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::MapRow& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::MapRow& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::TimeViewRow& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::TimeViewRow& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::GeneralDoc& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::GeneralDoc& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::CrossSectionView& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::CrossSectionView& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::TimeView& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::TimeView& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::TempView& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::TempView& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::TrajectoryView& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::TrajectoryView& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::SynopPlotSettings& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::SynopPlotSettings& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::ObsComparisonInfo& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::ObsComparisonInfo& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::WarningCenterView& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::WarningCenterView& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::SynopDataGridView& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::SynopDataGridView& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiViewSettingMacro::MapViewDescTop& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiViewSettingMacro::MapViewDescTop& item){item.Read(is); return is;}

