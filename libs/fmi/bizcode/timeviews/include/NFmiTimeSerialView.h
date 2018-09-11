//**********************************************************
// C++ Class Name : NFmiTimeSerialView
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiTimeSerialView.h
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 3
//  - GD View Type      : Class Diagram
//  - GD View Name      : 2. uusi aikasarja viritys
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Tues - Apr 6, 1999
//
//
//  Description:
//
//  Change Log:
//
//**********************************************************

#pragma once

#include "NFmiTimeView.h"
#include "NFmiTimeEditMode.h"
#include "NFmiDrawingEnvironment.h"
#include "NFmiDataMatrix.h"
#include "NFmiInfoData.h"
#include "NFmiParameterName.h"
#include "NFmiDataModifierMinMax.h"

class NFmiFastQueryInfo;
class NFmiDataModifier;
class NFmiProducer;
class NFmiParam;

namespace ModelClimatology
{
    using ParamIds = std::vector<FmiParameterName>;
    using ParamMapItem = std::pair<std::string, ParamIds>;
    using ParamMap = std::map<FmiParameterName, ParamMapItem>;
}

class NFmiTimeSerialView : public NFmiTimeView
{

 public:
	NFmiTimeSerialView (const NFmiRect & theRect
					   ,NFmiToolBox * theToolBox
					   ,NFmiDrawingEnvironment * theDrawingEnvi
					   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					   ,int theRowIndex
					   ,double theManualModifierLength);
	virtual  ~NFmiTimeSerialView (void);
	void Draw (NFmiToolBox * theToolBox);
	virtual void ChangeTimeSeriesValues(void);
	virtual void EditingMode (int newMode);
	void MaxStationShowed (unsigned int newCount);
	void ManualModifierLength(double newValue);
	bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
	bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey);
	bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
	bool RightButtonDown (const NFmiPoint & thePlace, unsigned long theKey);
	bool MouseMove (const NFmiPoint &thePlace, unsigned long theKey);
	bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);
	virtual void DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle);
	virtual void CreateValueScale (void);
	virtual void DrawValueAxis (void);
	virtual void DrawDataLine (const NFmiMetTime& theTime1,  const NFmiMetTime& theTime2, double value1, double value2, NFmiDrawingEnvironment & envi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis);
	virtual void DrawDataLineOpt(double xpos1,  double xpos2, double value1, double value2, NFmiDrawingEnvironment & envi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis);
	virtual void EvaluateChangedValue(double& theValue);
	virtual void ResetModifyFactorValues (void);
	bool IsActivated(void) const; // onko t‰m‰ aikasarja aktiivinen n‰yttˆ
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint);
	virtual bool AutoAdjustValueScale(void);
    bool EditingMouseMotionsAllowed(void) const {return fEditingMouseMotionsAllowed;}
    void EditingMouseMotionsAllowed(bool newValue) {fEditingMouseMotionsAllowed = newValue;}

 protected:
	// ***** uusia selkeit‰ piirtofunktioita *********
	void DrawModelRunsPlume(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment &theCurrentDataLineStyle, boost::shared_ptr<NFmiDrawParam> &theDrawParam);
	void DrawEditedDataLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle);
	void PlotTimeSerialData(const checkedVector<double> &theValues, const checkedVector<NFmiMetTime> &theTimes, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis, bool drawConnectingLines = false);
	void PlotTimeSerialDataOpt(const checkedVector<double> &theValues, double xPosStart, double xStep, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis, bool drawConnectingLines = false);
	void FillTimeSerialDataFromInfo(NFmiFastQueryInfo &theSourceInfo, const NFmiPoint &theLatLonPoint, checkedVector<double> &theValues);
	void FillTimeSerialDataFromInfo(NFmiFastQueryInfo &theSourceInfo, const NFmiPoint &theLatLonPoint, const NFmiTimeBag &theLimitTimes, checkedVector<double> &theValues);
	void FillTimeSerialMaskValues(const checkedVector<NFmiMetTime> &theTimes, const NFmiPoint &theLatLonPoint, checkedVector<double> &theMaskValues);
	void FillTimeSerialChangedValues(const checkedVector<double> &theValues, const checkedVector<double> &theMaskValues, checkedVector<double> &theChangedValues);
	void FillTimeSerialTimesFromInfo(NFmiFastQueryInfo &theSourceInfo, checkedVector<NFmiMetTime> &theTimes);
	void FillTimeSerialTimesFromInfo(NFmiFastQueryInfo &theSourceInfo, const NFmiTimeBag &theLimitTimes, checkedVector<NFmiMetTime> &theTimes);
	void CalcOptimizedDrawingValues(const NFmiTimeBag &theTimesIn, double &theXStartPosOut, double &theXStepOut);
	bool OpenOverViewPopUp(const NFmiPoint &thePlace, unsigned long theKey);
	// ***** uusia selkeit‰ piirtofunktioita *********

    boost::shared_ptr<NFmiFastQueryInfo> GetObservationInfo(const NFmiParam &theParam, const NFmiPoint &theLatlon);
    boost::shared_ptr<NFmiFastQueryInfo> GetNonSynopObservation(const NFmiParam &theParam);
	bool ChangeDataLevel(boost::shared_ptr<NFmiDrawParam> &theDrawParam, short theDelta);
	std::string GetHelpDataToolTipText(const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
	bool GetDrawedTimes(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiTimeBag &theTimesOut);
	bool SetObsDataToNearestLocationWhereIsData(boost::shared_ptr<NFmiFastQueryInfo> &theObsInfo, const NFmiPoint &theLatlon, std::pair<int, double> &theLocationWithDataOut);
	std::string GetObservationToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor);
	void DrawModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &theFractileData, long theStartParamIndex, const NFmiPoint &theLatlon, long theParamIndexIncrement = 1);
	void DrawSelectedStationData(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, int &theDrawedLocationCounter);
    void DrawHelperObservationData(const NFmiPoint &theLatlon, int &theDrawedLocationCounter);
    bool IsParamWeatherSymbol3();
    bool IsSynopticObservationData();
    std::string GetModelDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
    std::string GetEcFraktileDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor);
    std::string GetSeaLevelPlumeDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor);
    std::string GetSeaLevelProbDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, boost::shared_ptr<NFmiFastQueryInfo> &theSeaLevelFractileData, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor);
    boost::shared_ptr<NFmiFastQueryInfo> GetSeaLevelPlumeData();
    bool IsSeaLevelPlumeParam();
    bool IsSeaLevelProbLimitParam();
    std::string GetEcFraktileParamToolTipText(long theStartParamIndex, const std::string &theParName, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor, long theParamIndexIncrement = 1);
    std::string GetModelClimatologyDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor);
    std::string GetModelClimatologyParamToolTipText(const ModelClimatology::ParamMapItem &paramItem, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor);
    std::string GetObsFraktileDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor);
    std::string GetMosTemperatureMinAndMaxDataToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor);
    checkedVector<NFmiPoint> GetViewedLatlonPoints(void);
	NFmiTimeBag GetViewLimitingTimes(void);
	const NFmiPoint& GetUsedLatlon(void);
    void DrawHelperData2LocationInTime(const NFmiPoint &theLatlon);
    void DrawHelperData3LocationInTime(const NFmiPoint &theLatlon);
    void DrawHelperData4LocationInTime(const NFmiPoint &theLatlon);
    void DrawAnnualModelFractileDataLocationInTime(FmiParameterName mainParameter, boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon);
    void DrawAnnualModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds);
    void DrawAnnualModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, const NFmiMetTime &startTime, const NFmiMetTime &endTime, int climateDataYearDifference);
    void DrawAnnualModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, const NFmiTimeBag &theDrawedTimes, int theTimeWhenDrawedInMinutes);
    void DrawTemperatureMinAndMaxFromHelperData(FmiParameterName mainParameter, boost::shared_ptr<NFmiFastQueryInfo> &helperDataInfo, const NFmiPoint &theLatlon);
    void DrawModelDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon);
    bool DrawModelDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon, const NFmiProducer &theProducer);
	void DrawSimpleDataVectorInTimeSerial(checkedVector<std::pair<double, NFmiMetTime> > &theDataVector, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePenSize, const NFmiPoint& theSinglePointSize);
	void DrawAnalyzeToolDataLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment &envi);
	void DrawAnalyzeToolChangeLine(const NFmiPoint &theLatLonPoint);
	void DrawAnalyzeToolEndTimeLine(void);
	void DrawStationNameLegend(const NFmiLocation* theLocation, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& theFontSize, const NFmiPoint& theTextPos, const NFmiString& thePreLocationString, FmiDirection theTextAligment, double theDistanceInMeters = kFloatMissing);
	void DrawModelDataLegend(const std::vector<NFmiColor> &theUsedColors, const std::vector<std::string> &theFoundProducerNames);
	NFmiPoint CalcParamTextPosition(void);
	void DrawHelperDataLocationInTime(const NFmiPoint &theLatlon);
	void DrawKepaDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon);
	void DrawObservationDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon);
	void DrawFraktiiliDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon);
	void DrawModelFractileDataLocationInTime(const NFmiPoint &theLatlon);
    void DrawPossibleSeaLevelPlumeDataLocationInTime(const NFmiPoint &theLatlon);
    void DrawPossibleSeaLevelForecastProbLimitDataPlume(const NFmiPoint &theLatlon);
    void DrawSeaLevelProbLines(const NFmiPoint &theLatlon);
    void DrawSeaLevelProbLine(NFmiDrawingEnvironment &theEnvi, const NFmiColor &theLineColor, float theProbValue);
    void DrawParamInTime(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDrawingEnvironment &theEnvi, const NFmiPoint &theLatlon, FmiParameterName theParam, const NFmiColor &theColor, const NFmiPoint &theEmptyPointSize);
	void DrawSimpleDataInTimeSerial(const NFmiTimeBag &theDrawedTimes, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDrawingEnvironment &theEnvi, const NFmiPoint &theLatLonPoint, int theTimeWhenDrawedInMinutes, const NFmiPoint& theSinglePointSize, bool drawConnectingLines = false, int theWantedLocationIndex = -1);
	void DrawSimpleDataInTimeSerial(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDrawingEnvironment &theEnvi, const NFmiPoint &theLatlon, int theTimeWhenDrawedInMinutes, const NFmiPoint& theSinglePointSize, bool fUseValueAxis = true);
	void DrawNightShades(void);
	void DrawShade(NFmiDrawingEnvironment &theEnvi, const NFmiMetTime &theTime, short theChangeByMinutes, double theYShift);
	void DrawCPReferenceLines(void);
	void DrawLocationDataIncrementally(void);
	void DrawIncrementalDataLegend(void);
	void DrawSelectedStationDataIncrementally(void);
	virtual boost::shared_ptr<NFmiFastQueryInfo> Info(void);
	void DrawPointInDataRect(NFmiDrawingEnvironment& envi, const NFmiPoint& theRelativePoint, const NFmiPoint& thePointSize);
	void DrawLineInDataRect(NFmiPoint& relativeStartPoint, NFmiPoint& relativeEndPoint, NFmiDrawingEnvironment & envi);
	virtual double CalcLinePerUnitValue(double *theLowTimeGridPos);
	double CalcValueLinePerUnitValue(void);
	bool IsModifyFactorValuesNonZero(void);
	double CalcMouseClickProximityFactor(void);
    using NFmiTimeView::DrawTimeLine;
	virtual void DrawTimeLine(void);
	virtual void DrawData (void);
	virtual void DrawGrids (NFmiDrawingEnvironment & envi);
	void DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos);
	void DrawModifyFactorPointGrids (void);
	void DrawPoint (NFmiDrawingEnvironment & envi, const NFmiPoint & thePoint, const NFmiPoint & thePointSize);
	virtual void DrawModifyFactorPoints (void);
	void DrawSelectedStationData (void);
	void DrawSelectedStationDataForNonEditedData(void);
	void DrawStationDataStationNameLegend(boost::shared_ptr<NFmiFastQueryInfo> &info, const NFmiPoint &theLatlon, int counter, NFmiDrawingEnvironment &envi);
	virtual void DrawParamName (void);
	virtual void DrawModifyingUnit(void);
	void DrawBackground (void);
	virtual void DrawModifyFactorAxis (void);
	void DrawModifyFactorPointsSin (void);
	void DrawModifyFactorPointsLinear (void);
	void DrawModifyFactorPointsManual (void);
	bool IsModifiedValueLineDrawn (long theEndPointIndex);
	bool ValueInsideValueAxis (double theValue);
	bool ValueInsideModifyFactorAxis (double theValue);
	NFmiRect CalcValueAxisRect (void);
	NFmiRect CalcModifyFactorAxisRect (void);
	NFmiRect CalcMaxValueAxisRect (void);
	NFmiPoint CalcRelativeValuePosition (const NFmiMetTime & theTime, double theValue);
	NFmiPoint CalcRelativeModifyFactorPosition (const NFmiMetTime & theTime, double theValue);
	double Value2AxisPosition (float theValue);
	double ModifyFactor2AxisPosition (float theValue);
	double Position2Value (const NFmiPoint & thePos);
	double Position2ModifyFactor (const NFmiPoint & thePos);
	NFmiPoint CalcFontSize (void);
	virtual void CreateValueScaleView (void);
	virtual void CreateModifyFactorScaleView(bool fSetScalesDirectlyWithLimits = false, double theValue = kFloatMissing);
	double CalcModifiedValue (double theRealValue, long theIndexconst, double theMaskFactor);
	NFmiPoint CalcModifiedValuePoint (double theRealValue, long theIndex, double theMaskFactor);
	bool ModifyFactorPointsSin (double theValue, int theIndex, const NFmiPoint & thePlace);
	bool ModifyFactorPointsLinear (double theValue, int theIndex);
	bool ModifyFactorPointsManual (double theValue, int theIndex);
	virtual void FixModifyFactorValue (double & theValue);
	void CalcLinearModifyFactorPoints (void);
	void CalcSinModifyFactorPoints (void);
	void CalcModifyFactorPoints (void);
	void CalcSuitableLowAndHighLimits (double & low, double & high, double step);
	double LowerScaleValueRound (double value, double step);
	double HigherScaleValueRound (double value, double step);
	NFmiDrawingEnvironment NormalStationDataCurveEnvironment (void);
	NFmiDrawingEnvironment ChangeStationDataCurveEnvironment (double theRelativeFillFactor = 0.6, double theSublinesPerUnitFactor = 50);
	NFmiDrawingEnvironment IncrementalStationDataCurveEnvironment(void);
	NFmiDrawingEnvironment ChangeIncrementalStationDataCurveEnvironment(void);
	bool ChangeModifyFactorView (double theChangeDirectionFactor);
	bool ChangeValueView (double theChangeDirectionFactor, bool fMoveTop);
	bool MoveValueView (double theChangeDirectionFactor);
	void CutLinePoint2YPlane(const NFmiPoint& theOtherPoint, NFmiPoint& theChangedPoint, double theYPlane);
	bool CheckIsPointsDrawable(NFmiPoint& point1, bool fPoint1In, NFmiPoint& point2, bool fPoint2In, bool fUseValueAxis);
	NFmiRect CalcModifyingUnitRect(void);
	const NFmiRect& ModifyingUnitRect(void){return itsModifyingUnitTextRect;};
	void DrawWantedDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon, boost::shared_ptr<NFmiDrawParam> &theViewedDrawParam, NFmiInfoData::Type theDataType, const NFmiProducer &theProducer, bool fGroundData);
	void DrawHelpEditorDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon);
	NFmiRect CalculateDataRect(void);
    std::string MultiModelRunToolTip(boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiMetTime &theTime, const NFmiPoint &theLatlon);
    NFmiPoint GetTooltipLatlon() const;
    NFmiPoint GetFirstSelectedLatlonFromEditedData() const;
    bool DoControlPointModeDrawing() const;
    bool IsEditedData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
    void SetEditedDatasCorrectMaskOn(boost::shared_ptr<NFmiFastQueryInfo> &theEditedInfo) const;
    bool IsMosTemperatureMinAndMaxDisplayed(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo);
    void DrawExistingDataLegend(const NFmiProducer &producer, NFmiInfoData::Type dataType, boost::shared_ptr<NFmiDrawParam> &drawParam, const NFmiColor &color, double heightIncrement, double endPointX, NFmiPoint &legendPlaceInOut, NFmiDrawingEnvironment &drawingEnvironmentInOut);

	NFmiDrawingEnvironment itsNormalCurveEnvi; // miten normaali k‰yr‰ piirret‰‰n
	NFmiDrawingEnvironment itsChangeCurveEnvi; // miten muutos k‰yr‰ piirret‰‰n (punainen katkoviiva)
	NFmiDrawingEnvironment itsIncrementalCurveEnvi; // miten mahd. incrementaali k‰yr‰ piirret‰‰n
	NFmiDrawingEnvironment itsChanheIncrementalCurveEnvi; // miten mahd. incrementaali k‰yr‰n mahd. muutos k‰yr‰ piirret‰‰n

	NFmiAxisView * itsValueView;
	NFmiAxisView * itsModifyFactorView;
	FmiTimeEditMode itsEditingMode;
	checkedVector<double> itsModificationFactorCurvePoints;
	NFmiAxis * itsValueAxis;
	NFmiAxis * itsModifyFactorAxis;

	NFmiRect itsModifyingUnitTextRect;
	NFmiRect itsDataRect; // t‰lle alueelle piirret‰‰n aikasarja ikkunassa k‰yr‰t

 protected:
	bool ScanDataForMinAndMaxValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theLatlon, const NFmiTimeBag &theLimitingTimes, NFmiDataModifierMinMax &theAutoAdjustMinMaxValuesOut);

	bool fJustScanningForMinMaxValues; // auto-adjust s‰‰dˆt lasketaan lˆydettyjen min ja max arvojen avulla. Jos t‰m‰ optio on true, 
										// ei piirret‰ mit‰‰n, etsit‰‰n vain min ja max arvoja. Tarkoitus on ett‰ optio laitetaan p‰‰lle
										// AutoAdjustValueScale-metodissa ja sitten kutsutaan DrawSelectedStationData() -metodia, joka k‰y
										// l‰pi kaikki 'piirrett‰v‰t' datat etsien min ja max arvoja.
	NFmiDataModifierMinMax itsAutoAdjustMinMaxValues; // T‰h‰n talletetaan skannauksen yhteydes‰‰ lˆydetyt minimi ja maksimi arvot
	NFmiTimeBag itsAutoAdjustScanTimes; // kaikki datat k‰yd‰‰n l‰pi alkaen timabagin alku ajasta loppu aikaan etsitt‰ess‰ min ja max arvoja
	checkedVector<NFmiPoint> itsScannedLatlonPoints; // katseltavalle datalle t‰ss‰ voi olla useita pisteit‰, mutta muille apudatoille k‰ytet‰‰n vain 1. pistett‰

private:

	double itsManualModifierLength;
	unsigned long itsMaxStationShowed;
	double itsSinAmplitude;
	int itsPhase;
	std::vector<NFmiColor> itsProducerModelDataColors; // t‰h‰n on laitettu eri mallien piirtovarit (producersystemin j‰rjestys)
    bool fEditingMouseMotionsAllowed; // T‰ll‰ kontrolloidaan hiirell‰ tehtyj‰ editointiin liittyvi‰ muutoksia. Tietyt muutokset sallitaan vain jos t‰m‰ on true.
                        // Tarkoitus on est‰‰ vanhinkojen teko, esim. hiirell‰ manipuloidaan aikakontrollia, mutta se p‰‰stet‰‰n vasta n‰ytˆn dataosion 
                        // p‰‰ll‰ irti ja buttonUp:issa tehd‰‰n muutos editointi pisteisiin. Sama p‰tee asteikkojen s‰‰dˆst‰ alkanutta hiiren k‰sittely‰.
};

