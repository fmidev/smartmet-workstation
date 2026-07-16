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
#include "HakeMessage/HakeSystemConfigurations.h"
#include "NFmiMapViewDescTop.h"

class NFmiArea;
class NFmiDrawParamList;
class NFmiAreaMaskList;
class NFmiMacroParamSystem;
class NFmiMapViewWinRegistry;

// Only the view macro's name (from the file name) and description are read here.
// Initialization is fast, and these are used to update the viewMacro dialog's list.
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
    // If something is specifically detected to be wrong in the view macro file, this is set to false.
    bool fViewMacroOk;
    NFmiTime itsFileModificationTime;

    void Read(std::istream& is);
};

class MfcViewStatus
{
    // Minimized/maximized, no other information
    unsigned int itsShowCommand = 0;
    // Whether the window is open or not
    bool fShowWindow = false;
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
		bool ShowDifferenceToOriginalData(void) const {return fShowDifferenceToOriginalData;}
		void ShowDifferenceToOriginalData(bool newValue) {fShowDifferenceToOriginalData = newValue;}
		void SetMacroParamInitFileNames(const std::string &theRootPath);

		void Write(std::ostream& os) const;
		void Read(std::istream& is);
	private:
		// Contains e.g. the data ident // the whole drawparam is stored so that one could make e.g. different colored/different tunings for the same parameter / different producer
		boost::shared_ptr<NFmiDrawParam> itsDrawParam;
		// This must be separate, because drawparam does not store its own dataident.
		NFmiDataIdent itsDataIdent;
		NFmiLevel itsLevel;
		NFmiInfoData::Type itsDataType;
		// This tells the possible previous hirlam or the one before that, e.g. -6h or -12h
		int itsModelOrigTimeOffsetInHours;

		// ***********************************************************************************************************
		// These are properties of DrawParam, but since they are not stored in the file, I store them here
		// because I don't want to change the drawparam storage version right now.
		// The view can be hidden according to this
		bool fHidden;
		// Whether the given parameter is the active parameter of the view (each view row always has one activated parameter)
		bool fActive;
		// Difference to the original data on the map view
		bool fShowDifferenceToOriginalData;
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

	// Contains the mask list and the general settings for masks
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

		// Settings for the lat-lon helper grid drawn on top of the map
		NFmiProjectionCurvatureInfo itsProjectionCurvatureInfo;
		// Control points stored (latlon points)
		std::vector<NFmiPoint> itsCPLocationVector;
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

		// Settings of all map view rows
		std::vector<TimeViewRow> itsRows;
		// View size and position in pixels
		NFmiRect itsAbsolutRect;
        MfcViewStatus itsViewStatus;
        // Whether model+observation+climatological data is shown in operational use in the time series alongside the edited data
        bool fShowHelpData;
        // Whether short fractile model data is shown in the time series view
        bool fShowHelpData2;
        // Whether climatology data computed from model data is shown in the time series view
        bool fShowHelpData3;
        // Whether T-min and T-max temperature curves from some helper data are shown as an aid in the time series view
        bool fShowHelpData4;
		// If the time window has been zoomed in time, this holds the offset to the start time of the edited data
		int itsStartTimeOffset;
		// If the time window has been zoomed in time, this holds the offset to the end time of the edited data
		int itsEndTimeOffset;
		// This is the timebag used in the time series (stored as an offset from the clock time)
		NFmiTimeBag itsTimeBag;
		// If the timebag has been read, this is set to true
		bool fTimeBagUpdated;
		// The point selected from the map to the time series stored here
		NFmiPoint itsPreciseTimeSerialLatlonPoint = NFmiPoint::gMissingLatlon;
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
		// View size and position in pixels
		NFmiRect itsAbsolutRect;
        MfcViewStatus itsViewStatus;

		// Whether the hirlam view checkbox is on or not
		bool fShowHirlam;
		// Whether the ecmwf view checkbox is on or not
		bool fShowEcmwf;
		// In the future: whether the observation view checkbox is on or not
		bool fShowRealSounding;

		NFmiMTATempSystem itsMTATempSystem;
		// This is not stored in the TempView data section of the file; instead it is provided here for use by NFmiViewSettingMacro
		double itsCurrentVersionNumber;
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
		// View size and position in pixels
		NFmiRect itsAbsolutRect;
        MfcViewStatus itsViewStatus;

		NFmiTrajectorySystem itsTrajectorySystem;
		// This is not stored in the TempView data section of the file; instead it is provided here for use by NFmiViewSettingMacro
		double itsCurrentVersionNumber;
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

		// Settings of all map view rows
		std::vector<MapRow> itsMapRowSettings;
		NFmiCrossSectionSystem itsCrossSectionSystem;
		// View size and position in pixels
		NFmiRect itsAbsolutRect;
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

		// Settings of all map view rows
		std::vector<MapRow> itsMapRowSettings;
		NFmiMapViewDescTop itsMapViewDescTop;
		// View size and position in pixels
		NFmiRect itsAbsolutRect;
        MfcViewStatus itsViewStatus;

		// The dipmaphandler section must be given special handling here
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
		// View size and position in pixels
		NFmiRect itsAbsolutRect;
        MfcViewStatus itsViewStatus;
        std::vector<int> itsHeaderColumnWidthsInPixels;
        // Checkbox setting for HAKE messages in the dialog
        bool fShowHakeMessages;
        // Checkbox setting for KaHa messages in the dialog
        bool fShowKaHaMessages;
        // See the explanation of the corresponding member data in NFmiApplicationWinRegistry.h.
        int itsMinimumTimeRangeForWarningsOnMapViewsInMinutes;
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
		// View size and position in pixels
		NFmiRect itsAbsolutRect;
        MfcViewStatus itsViewStatus;

		NFmiProducer itsSelectedProducer;
		// This is adjusted using the time and calendar controls
		NFmiMetTime itsMinMaxRangeStartTime;
		bool fMinMaxModeOn;
		double itsDayRangeValue;

		// Country filter section
		bool fAllCountriesSelected;
		// List of selected countries by country abbreviation strings, separated by commas
		std::string itsSelectedCountryAbbrStr;
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

	// Values are set through this
	GeneralDoc& GetGeneralDoc(void) {return itsGeneralDoc;}
	// Values are set through this
	TimeView& GetTimeView(void) {return itsTimeView;}
	// Values are set through this
	TempView& GetTempView(void) {return itsTempView;}
	// Values are set through this
	TrajectoryView& GetTrajectoryView(void) {return itsTrajectoryView;}
	CrossSectionView& GetCrossSectionView(void) {return itsCrossSectionView;}
	SynopPlotSettings& GetSynopPlotSettings(void) {return itsSynopPlotSettings;}
	ObsComparisonInfo& GetObsComparisonInfo(void) {return itsObsComparisonInfo;}
	// Values are set through this
	MaskSettings& GetMaskSettings(void) {return itsMaskSettings;}
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

	// NOTE!! This includes comments!
	void Write(std::ostream& os) const;
	// NOTE!! before this class is read in from a file, remove the comments
	// with the NFmiCommentStripper class, because comments are inserted
	// when writing.
	void Read(std::istream& is);
private:

	// I store these as NFmiStrings so that reading and writing work ok even if there are white spaces
	// Macro's name -> file name when path and type are added
	mutable std::string itsName;
	// Macro's longer description
	mutable std::string itsDescription;
	// When the macro is loaded, the file name with its path is stored here, so that the macro can be reloaded if needed
	std::string itsInitFileName;

	GeneralDoc itsGeneralDoc;
	TimeView itsTimeView;
	TempView itsTempView;
	// Settings of all masks
	MaskSettings itsMaskSettings;
	bool fIsPrinterPortrait;

	// If this is true, the selection dialog behaves differently
	// additionally such a macro is not stored to a file
	// This is meant to be used to add a folder system to the view macros
	bool fViewMacroDirectory;

	// If reading the file failed, it must be marked as corrupted
	bool fViewMacroWasCorrupted;
	// Which tools are on
	bool fUseBrushTool;
	bool fUseAnalyzeTool;
	bool fUseControlPoinTool;
	bool fUseAnimationTool;
    bool fKeepMapAspectRatio;
	// Animation settings
	// From which frame of the edited data the animation starts
	int itsAnimationStartPosition;
	// At which frame of the edited data the animation ends
	int itsAnimationEndPosition;
	// Delay in the animation in milliseconds
	int itsAnimationDelayInMS;

	// When the viewMacro was loaded, this was its original version
	double itsOriginalLoadVersionNumber;
	// Version 2 parts
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

