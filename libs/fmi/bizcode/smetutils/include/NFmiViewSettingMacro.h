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
#include "NFmiMTATempSystem.h"
#include "NFmiTrajectorySystem.h"
#include "NFmiCrossSectionSystem.h"
#include "NFmiSynopPlotSettings.h"
#include "NFmiObsComparisonInfo.h"
#ifndef UNIX
#include "HakeMessage/HakeSystemConfigurations.h"
#endif
#include "NFmiMapViewDescTop.h"

class NFmiArea;
class NFmiDrawParamList;
class NFmiAreaMaskList;
class NFmiMacroParamSystem;
class NFmiMapViewWinRegistry;

// T�h�n luetaan vain viewmacron nimi (tiedosto nimest�) ja description.
// Alustus on nopeaa ja n�iden avulla p�ivitet��n viewMacro-dialogin listaa.
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
    bool fViewMacroOk; // Jos erityisesti havaitaan ett� n�ytt�makro tiedostossa on jotain vikaa, laitetaan t�m� false tilaan.
    NFmiTime itsFileModificationTime;

    void Read(std::istream& is);
};

class MfcViewStatus
{
    unsigned int itsShowCommand = 0; // minimized/maximized, ei muita tietoja
    bool fShowWindow = false; // onko ikkuna auki vai ei
public:
    unsigned int ShowCommand() const { return itsShowCommand; }
    void ShowCommand(unsigned int newValue) { itsShowCommand = newValue; }
    bool ShowWindow() const { return fShowWindow; }
    void ShowWindow(bool newValue) { fShowWindow = newValue; }
};

class NFmiViewSettingMacro
{
public:
	static double itsLatestVersionNumber;
	mutable double itsCurrentVersionNumber;

	class Param
	{
	public:
		Param(void);
		Param(const std::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiLevel &theLevel, NFmiInfoData::Type theDataType, int theModelOrigTimeOffsetInHours);
		Param(const NFmiDataIdent &theDataIdent, const NFmiLevel &theLevel, NFmiInfoData::Type theDataType, int theModelOrigTimeOffsetInHours);
		~Param(void);

		const std::shared_ptr<NFmiDrawParam>& DrawParam(void) const {return itsDrawParam;}
		void DrawParam(const std::shared_ptr<NFmiDrawParam> &newValue);
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
		bool ShowDifferenceToOriginalData(void) const {return fShowDifferenceToOriginalData;}
		void ShowDifferenceToOriginalData(bool newValue) {fShowDifferenceToOriginalData = newValue;}
		void SetMacroParamInitFileNames(const std::string &theRootPath);

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		std::shared_ptr<NFmiDrawParam> itsDrawParam; // sis. mm. data identin // talletetaan koko drawparam, ett� voisi tehd� esim. eri v�risi�/erilaisia virityksi� samalle parametrille / eri tuottajalle
		NFmiDataIdent itsDataIdent; // t�m� pit�� olla erikseen, koska drawparam ei talleta sen omaa dataidentti��n.
		NFmiLevel itsLevel;
		NFmiInfoData::Type itsDataType;
		int itsModelOrigTimeOffsetInHours; // t�m� kertoo mahdollisen ed. hirlam vai sit� edellinen esim. -6h tai -12h

		// ***********************************************************************************************************
		// N�m� ovat DrawParamin ominaisuuksia, mutta koska niit� ei talleteta tiedostoon, talletan ne t�h�n
		// koska en halua juuri nyt muuttaa drawparamin talletus versiota.
		bool fHidden;	// n�ytt� voidaan piiloittaa t�m�n mukaisesti
		bool fActive;	// onko kyseinen parametri n�yt�n aktiivinen parametri (jokaisella n�ytt�rivill� aina yksi aktivoitunut parametri)
		bool fShowDifferenceToOriginalData; // ero originaaliin dataan karttan�yt�ss�
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

	// sis�lt�� maski listan ja yleiset asetukset maskeille
	class MaskSettings
	{
	public:
		MaskSettings(void);
		MaskSettings(const std::vector<Mask> &theMasks, bool theShowMasksOnMapView, bool theUseMasksInTimeSerialViews, bool theUseMasksWithFilterTool, bool theUseMaskWithBrush);
		~MaskSettings(void);

		void SetAllMasks(NFmiAreaMaskList &theMasks);
		const std::vector<Mask>& Masks(void) const {return itsMasks;}
		void Masks(const std::vector<Mask>& newValue) {itsMasks = newValue;}
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

		std::vector<Mask> itsMasks;
		bool fShowMasksOnMapView;
		bool fUseMasksInTimeSerialViews;
		bool fUseMasksWithFilterTool;
		bool fUseMaskWithBrush;
	};

	class MapRow
	{
	public:
		MapRow(void);
		MapRow(const std::vector<NFmiViewSettingMacro::Param>& theParams);
		~MapRow(void);

		const std::vector<NFmiViewSettingMacro::Param>& RowParams(void) const {return itsRowParams;}
		void RowParams(const std::vector<NFmiViewSettingMacro::Param>& newValue) {itsRowParams = newValue;}
		void Clear(void);
		void Add(const Param &theParam);
		void SetMacroParamInitFileNames(const std::string &theRootPath);

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		std::vector<NFmiViewSettingMacro::Param> itsRowParams;
	};

	class TimeViewRow
	{
	public:
		TimeViewRow(void);
		TimeViewRow(const Param& theParam);
		~TimeViewRow(void);

		const NFmiViewSettingMacro::Param& Param(void) const {return itsParam;}
		void Param(const NFmiViewSettingMacro::Param& newValue) {itsParam = newValue;}
		const std::vector<NFmiViewSettingMacro::Param>& SideParameters(void) const { return itsSideParameters; }
		void SideParameters(const std::vector<NFmiViewSettingMacro::Param>& newValue) { itsSideParameters = newValue; }
		void SetMacroParamInitFileNames(const std::string& theRootPath);

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		NFmiViewSettingMacro::Param itsParam;
		std::vector<NFmiViewSettingMacro::Param> itsSideParameters;
	};

	class GeneralDoc
	{
	public:
		GeneralDoc(void);
		~GeneralDoc(void);

		const NFmiProjectionCurvatureInfo& ProjectionCurvatureInfo(void) const {return itsProjectionCurvatureInfo;}
		void ProjectionCurvatureInfo(const NFmiProjectionCurvatureInfo& newValue) {itsProjectionCurvatureInfo = newValue;}
		const std::vector<NFmiPoint>& CPLocationVector(void) const {return itsCPLocationVector;}
		void CPLocationVector(const std::vector<NFmiPoint>& newValue) {itsCPLocationVector = newValue;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:

		NFmiProjectionCurvatureInfo itsProjectionCurvatureInfo; // kartan p��lle piirrett�v�t lat-lon apuviivasto asetukset
		std::vector<NFmiPoint> itsCPLocationVector; // controllipisteet talteen (latlon pisteet)
	};

	class TimeView
	{
	public:
		TimeView(void);
		~TimeView(void);

		void SetAllParams(NFmiDrawParamList* theDrawParamList);
		void SetAllSideParameters(CombinedMapHandlerInterface::SideParametersContainer &theSideParameterList);

		const std::vector<TimeViewRow>& Rows(void) const {return itsRows;}
		void Rows(const std::vector<TimeViewRow>& newValue) {itsRows = newValue;}
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
		const NFmiPoint& PreciseTimeSerialLatlonPoint() const { return itsPreciseTimeSerialLatlonPoint; }
		void PreciseTimeSerialLatlonPoint(const NFmiPoint& newValue) { itsPreciseTimeSerialLatlonPoint = newValue; }
		void SetMacroParamInitFileNames(const std::string& theRootPath);

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		void Clear(void);
		void Add(const TimeViewRow &theTimeViewRow);

		std::vector<TimeViewRow> itsRows; //  kaikkien karttan�ytt�rivien asetukset
		NFmiRect itsAbsolutRect; // n�yt�n koko ja sijainti pixeleiss�
        MfcViewStatus itsViewStatus;
        bool fShowHelpData; // n�ytet��nk� malli+havainto+klimatologiset data operatiivisess� k�yt�ss� aikasarjassa editoidun datan ohella
        bool fShowHelpData2; // n�ytet��nk� lyhyt fraktiili mallidata aikasarjan�yt�ss�
        bool fShowHelpData3; // n�ytet��nk� mallidatasta lasketun klimatologia data aikasarjan�yt�ss�
        bool fShowHelpData4; // n�ytet��nk� jostain apudatasta T-min ja T-max l�mp�tilak�yri� apuna aikasarjan�yt�ss�
		int itsStartTimeOffset; // jos aikaikkunaa on zoomattu ajallisesti, t�ss� offsetti editoitavan datan alkuaikaan
		int itsEndTimeOffset; // jos aikaikkunaa on zoomattu ajallisesti, t�ss� offsetti editoitavan datan loppuaikaan
		NFmiTimeBag itsTimeBag; // t�ss� on aikasarjassa k�ytetty timebag (talletetaan siirtym�n� kellonajasta)
		bool fTimeBagUpdated; // jos timebagi on luettu, asetetaan t�h�n true
		NFmiPoint itsPreciseTimeSerialLatlonPoint = NFmiPoint::gMissingLatlon; // Kartalta aikakasarjaan valittu piste talteen
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
		void MTATempSystem(const NFmiMTATempSystem &theData, bool disableWindowManipulations);
		double CurrentVersionNumber(void) {return itsCurrentVersionNumber;}
		void CurrentVersionNumber(double newValue) {itsCurrentVersionNumber = newValue;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		NFmiRect itsAbsolutRect; // n�yt�n koko ja sijainti pixeleiss�
        MfcViewStatus itsViewStatus;

		bool fShowHirlam; // onko hirlam n�ytt� ruksi p��ll� vai ei
		bool fShowEcmwf; // onko ecmwf n�ytt� ruksi p��ll� vai ei
		bool fShowRealSounding; // Tulevaisuudessa: onko havainto n�ytt� ruksi p��ll� vai ei

		NFmiMTATempSystem itsMTATempSystem;
		double itsCurrentVersionNumber; // t�m� ei ole talletettuna TempView dataosiossa tiedostossa vaan t�m� annetaan NFmiViewSettingMacro:lta t�nne k�ytt��n
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
		NFmiRect itsAbsolutRect; // n�yt�n koko ja sijainti pixeleiss�
        MfcViewStatus itsViewStatus;

		NFmiTrajectorySystem itsTrajectorySystem;
		double itsCurrentVersionNumber; // t�m� ei ole talletettuna TempView dataosiossa tiedostossa vaan t�m� annetaan NFmiViewSettingMacro:lta t�nne k�ytt��n
	};

	class CrossSectionView
	{
	public:

		CrossSectionView(void);
		~CrossSectionView(void);

		void SetAllRowParams(NFmiPtrList<NFmiDrawParamList> *theDrawParamListVector, NFmiMacroParamSystem& theMacroParamSystem);
		const std::vector<MapRow>& MapRowSettings(void) const {return itsMapRowSettings;}
		void MapRowSettings(const std::vector<MapRow>& newValue) {itsMapRowSettings = newValue;}
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

		std::vector<MapRow> itsMapRowSettings; //  kaikkien karttan�ytt�rivien asetukset
		NFmiCrossSectionSystem itsCrossSectionSystem;
		NFmiRect itsAbsolutRect; // n�yt�n koko ja sijainti pixeleiss�
        MfcViewStatus itsViewStatus;
    };

	class MapViewDescTop
	{
	public:

		MapViewDescTop(void);
		~MapViewDescTop(void);

		void SetAllRowParams(NFmiPtrList<NFmiDrawParamList> *theDrawParamListVector, NFmiMacroParamSystem& theMacroParamSystem);
		const std::vector<MapRow>& MapRowSettings(void) const {return itsMapRowSettings;}
		void MapRowSettings(const std::vector<MapRow>& newValue) {itsMapRowSettings = newValue;}
		const NFmiRect& AbsolutRect(void) const {return itsAbsolutRect;}
		void AbsolutRect(const NFmiRect& newValue) {itsAbsolutRect = newValue;}
        const MfcViewStatus& ViewStatus() const { return itsViewStatus; }
        MfcViewStatus& ViewStatus() { return itsViewStatus; }
        void SetMacroParamInitFileNames(const std::string &theRootPath);
		const NFmiMapViewDescTop& GetMapViewDescTop(void) const {return itsMapViewDescTop;}
		void SetMapViewDescTop(const NFmiMapViewDescTop &theData, NFmiMapViewWinRegistry &theMapViewWinRegistry, bool disableWindowManipulations);

		const std::vector<NFmiMapViewDescTop::ViewMacroDipMapHelper>& DipMapHelperList(void) const {return itsDipMapHelperList;}
		void DipMapHelperList(const std::vector<NFmiMapViewDescTop::ViewMacroDipMapHelper> &theList) {itsDipMapHelperList = theList;}

		void Write(std::ostream& os) const;
		void Read(std::istream& is);

	private:
		void Clear(std::vector<MapRow> &theMapRowSettings);
		void Add(std::vector<MapRow> &theMapRowSettings, const MapRow &theMapRow);

		std::vector<MapRow> itsMapRowSettings; //  kaikkien karttan�ytt�rivien asetukset
		NFmiMapViewDescTop itsMapViewDescTop;
		NFmiRect itsAbsolutRect; // n�yt�n koko ja sijainti pixeleiss�
        MfcViewStatus itsViewStatus;

		// dipmaphandler osio pit�� ottaa t�h�n erikois k�sittelyyn
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
#ifndef UNIX
        const HakeLegacySupport::HakeSystemConfigurations& WarningCenterSystem(void) const {return itsWarningCenterSystem;}
		void WarningCenterSystem(const HakeLegacySupport::HakeSystemConfigurations &theData) {itsWarningCenterSystem.Init(theData);}
#endif
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
#ifndef UNIX
        HakeLegacySupport::HakeSystemConfigurations itsWarningCenterSystem;
#endif
		NFmiRect itsAbsolutRect; // n�yt�n koko ja sijainti pixeleiss�
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
		NFmiRect itsAbsolutRect; // n�yt�n koko ja sijainti pixeleiss�
        MfcViewStatus itsViewStatus;

		NFmiProducer itsSelectedProducer;
		NFmiMetTime itsMinMaxRangeStartTime; // t�t�  s��det��n aika ja kalenteri kontrollien avulla
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

	GeneralDoc& GetGeneralDoc(void) {return itsGeneralDoc;} // arvojen asetus t�t� kautta
	TimeView& GetTimeView(void) {return itsTimeView;} // arvojen asetus t�t� kautta
	TempView& GetTempView(void) {return itsTempView;} // arvojen asetus t�t� kautta
	TrajectoryView& GetTrajectoryView(void) {return itsTrajectoryView;} // arvojen asetus t�t� kautta
	CrossSectionView& GetCrossSectionView(void) {return itsCrossSectionView;}
	SynopPlotSettings& GetSynopPlotSettings(void) {return itsSynopPlotSettings;}
	ObsComparisonInfo& GetObsComparisonInfo(void) {return itsObsComparisonInfo;}
	MaskSettings& GetMaskSettings(void) {return itsMaskSettings;} // arvojen asetus t�t� kautta
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
    bool KeepMapAspectRatio() const { return fKeepMapAspectRatio; }
    void KeepMapAspectRatio(bool newValue) { fKeepMapAspectRatio = newValue; }

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

	// HUOM!! T�m� laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin t�m� luokka luetaan sis��n tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan.
	void Read(std::istream& is);
private:

	// talletan n�m� NFmiString:ein�, ett� luku ja kirjoitus menev�t ok vaikka olisi white spaceja
	mutable std::string itsName; // macron nimi -> tiedoston nimi kun lis�t��n polku ja tyyppi
	mutable std::string itsDescription; // macron pidempi kuvaus
	std::string itsInitFileName; // kun makro ladataan, laitetaan t�h�n tiedoston nimi polun kanssa, ett� makro voidaan tarvittaessa ladata uudestaan

	GeneralDoc itsGeneralDoc;
	TimeView itsTimeView;
	TempView itsTempView;
	MaskSettings itsMaskSettings; //  kaikkien maskien asetukset
	bool fIsPrinterPortrait;

	bool fViewMacroDirectory; // jos t�m� on true, valinta dialogissa toimitaan erilailla
							  // lis�ksi t�ll�ist� makroa ei talleteta tiedostoon
							  // T�m�n avulla on tarkoitus laittaa kansio systeemi n�ytt�makroihin

	bool fViewMacroWasCorrupted; // jos tiedoston luku ep�onnistui, on se merkitt�v� korruptoituneeksi
	// mitk� ty�kalut ovat p��ll�
	bool fUseBrushTool;
	bool fUseAnalyzeTool;
	bool fUseControlPoinTool;
	bool fUseAnimationTool;
    bool fKeepMapAspectRatio;
	// animaatio asetukset
	int itsAnimationStartPosition; // kuinka monennestako framesta editoitavaa dataa aloitetaan
	int itsAnimationEndPosition; // kuinka monennestako framesta editoitavaa dataa lopetetaan
	int itsAnimationDelayInMS; // viiva animaatiossa millisekunneissa

	double itsOriginalLoadVersionNumber; // kun viewMacro ladattiin, t�m� oli sen alkuper�inen versio
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

