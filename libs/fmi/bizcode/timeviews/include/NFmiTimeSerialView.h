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
#include "NFmiDrawingEnvironment.h"
#include "NFmiInfoData.h"
#include "NFmiParameterName.h"
#include "NFmiDataModifierMinMax.h"

class NFmiFastQueryInfo;
class NFmiDataModifier;
class NFmiProducer;
class NFmiParam;
class NFmiEditorControlPointManager;

namespace NFmiFastInfoUtils
{
    class MetaWindParamUsage;
}

namespace ModelClimatology
{
    using ParamIds = std::vector<FmiParameterName>;
    using ParamMapItem = std::pair<std::string, ParamIds>;
    using ParamMap = std::map<FmiParameterName, ParamMapItem>;
}

class CpDrawingOptions
{
public:
    CpDrawingOptions(const NFmiDrawingEnvironment &currentLine, const NFmiDrawingEnvironment &changeLine, const NFmiRect &viewFrame, const NFmiToolBox *toolbox);

    NFmiDrawingEnvironment currentDataEnvi;
    NFmiDrawingEnvironment changeDataEnvi;
    NFmiPoint normalLine{ 2, 2 };
    NFmiPoint thickLine{ 3, 3 };
    NFmiPoint normalChangeLine{ 2, 2 };
    NFmiPoint thickChangeLine{ 3, 3 };
    NFmiPoint textPoint;
    double heightInc = 0;
    double endPointX1 = 0;
    double endPointX2 = 0;
    int currentLineIndex = 0;
};

class NFmiTimeSerialView : public NFmiTimeView
{

 public:
	NFmiTimeSerialView (const NFmiRect & theRect
					   ,NFmiToolBox * theToolBox
					   ,NFmiDrawingEnvironment * theDrawingEnvi
					   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					   ,int theRowIndex);
	~NFmiTimeSerialView (void);
	void Draw (NFmiToolBox * theToolBox) override;
	virtual void ChangeTimeSeriesValues(void);
	void MaxStationShowed (unsigned int newCount);
	bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool MouseMove (const NFmiPoint &thePlace, unsigned long theKey) override;
	bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
	virtual void DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines);
	virtual void CreateValueScale (void);
	virtual void DrawValueAxis (void);
	virtual void DrawDataLine (const NFmiMetTime& theTime1,  const NFmiMetTime& theTime2, double value1, double value2, NFmiDrawingEnvironment & envi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis);
	virtual void DrawDataLineOpt(double xpos1,  double xpos2, double value1, double value2, NFmiDrawingEnvironment & envi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis);
	virtual void EvaluateChangedValue(double& theValue);
	virtual void ResetModifyFactorValues (void);
	bool IsActivated(void) const; // onko t�m� aikasarja aktiivinen n�ytt�
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
	virtual bool AutoAdjustValueScale(void);
    bool EditingMouseMotionsAllowed(void) const {return fEditingMouseMotionsAllowed;}
    void EditingMouseMotionsAllowed(bool newValue) {fEditingMouseMotionsAllowed = newValue;}
    bool IsTimeSerialView(void) override { return true; };
	void UpdateCachedParameterName() override;

 protected:
	// ***** uusia selkeit� piirtofunktioita *********
	void DrawModelRunsPlume(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment &theCurrentDataLineStyle, boost::shared_ptr<NFmiDrawParam> &theDrawParam);
	void DrawEditedDataLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines);
	void PlotTimeSerialData(const std::vector<double> &theValues, const std::vector<NFmiMetTime> &theTimes, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis, bool drawConnectingLines = false);
	void PlotTimeSerialDataOpt(const std::vector<double> &theValues, double xPosStart, double xStep, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePointSize, const NFmiPoint& theSinglePointSize, bool fUseValueAxis, bool drawConnectingLines = false);
	void FillTimeSerialDataFromInfo(boost::shared_ptr<NFmiFastQueryInfo> &theSourceInfo, const NFmiPoint &theLatLonPoint, std::vector<double> &theValues, const NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage, unsigned long wantedParamId);
	void FillTimeSerialDataFromInfo(boost::shared_ptr<NFmiFastQueryInfo> &theSourceInfo, const NFmiPoint &theLatLonPoint, const NFmiTimeBag &theLimitTimes, std::vector<double> &theValues, const NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage, unsigned long wantedParamId);
	void FillTimeSerialMaskValues(const std::vector<NFmiMetTime> &theTimes, const NFmiPoint &theLatLonPoint, std::vector<double> &theMaskValues);
	void FillTimeSerialChangedValues(const std::vector<double> &theValues, const std::vector<double> &theMaskValues, std::vector<double> &theChangedValues);
	void FillTimeSerialTimesFromInfo(NFmiFastQueryInfo &theSourceInfo, std::vector<NFmiMetTime> &theTimes);
	void FillTimeSerialTimesFromInfo(NFmiFastQueryInfo &theSourceInfo, const NFmiTimeBag &theLimitTimes, std::vector<NFmiMetTime> &theTimes);
	void CalcOptimizedDrawingValues(const NFmiTimeBag &theTimesIn, double &theXStartPosOut, double &theXStepOut);
	bool OpenOverViewPopUp(const NFmiPoint &thePlace, unsigned long theKey);

    // Splitting DrawEditedDataLocationInTime -method in smaller parts
    bool DrawEditedDataLocationInTime_PreliminaryActions(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle);
    void DrawEditedDataLocationInTime_TimebagOptimized(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, const std::vector<NFmiMetTime> &theTimes, bool drawModificationLines);
    void DrawEditedDataLocationInTime_Timelist(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, const std::vector<NFmiMetTime> &theTimes, bool drawModificationLines);
    void DrawEditedDataLocationInTime_ModificationLineOptimized(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theModifiedDataLineStyle, const std::vector<NFmiMetTime> &theTimes, const std::vector<double> &values);
    void DrawEditedDataLocationInTime_ModificationLine(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theModifiedDataLineStyle, const std::vector<NFmiMetTime> &theTimes, const std::vector<double> &values);
    // ***** uusia selkeit� piirtofunktioita *********

    boost::shared_ptr<NFmiFastQueryInfo> GetObservationInfo(const NFmiParam &theParam, const NFmiPoint &theLatlon);
    boost::shared_ptr<NFmiFastQueryInfo> GetNonSynopObservation(const NFmiParam &theParam);
	bool ChangeDataLevel(boost::shared_ptr<NFmiDrawParam> &theDrawParam, short theDelta);
	std::string GetHelpDataToolTipText(const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
	bool GetDrawedTimes(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiTimeBag &theTimesOut);
	bool SetObsDataToNearestLocationWhereIsData(boost::shared_ptr<NFmiFastQueryInfo> &theObsInfo, const NFmiPoint &theLatlon, std::pair<int, double> &theLocationWithDataOut);
	std::string GetObservationToolTipText(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, const NFmiColor &theColor);
	void DrawModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &theFractileData, long theStartParamIndex, const NFmiPoint &theLatlon, long theParamIndexIncrement = 1);
	void DrawSelectedStationData(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo, const NFmiPoint &theLatlon, int &theDrawedLocationCounter);
    void DrawHelperObservationData(const NFmiPoint &theLatlon);
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
    std::vector<NFmiPoint> GetViewedLatlonPoints(void);
	NFmiTimeBag GetViewLimitingTimes(void);
    void DrawHelperData2LocationInTime(const NFmiPoint &theLatlon);
    void DrawHelperData3LocationInTime(const NFmiPoint &theLatlon);
    void DrawHelperData4LocationInTime(const NFmiPoint &theLatlon);
    void DrawAnnualModelFractileDataLocationInTime(FmiParameterName mainParameter, boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon);
    void DrawAnnualModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, NFmiDrawingEnvironment* overrideEnvi);
    void DrawAnnualModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, const NFmiMetTime &startTime, const NFmiMetTime &endTime, int climateDataYearDifference, NFmiDrawingEnvironment* overrideEnvi);
    void DrawAnnualModelFractileDataLocationInTime(boost::shared_ptr<NFmiFastQueryInfo> &climateInfo, const NFmiPoint &theLatlon, const ModelClimatology::ParamIds &paramIds, const NFmiTimeBag &theDrawedTimes, int theTimeWhenDrawedInMinutes, NFmiDrawingEnvironment *overrideEnvi);
    void DrawTemperatureMinAndMaxFromHelperData(FmiParameterName mainParameter, boost::shared_ptr<NFmiFastQueryInfo> &helperDataInfo, const NFmiPoint &theLatlon);
    void DrawModelDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon);
    bool DrawModelDataLocationInTime(NFmiDrawingEnvironment &envi, const NFmiPoint &theLatlon, const NFmiProducer &theProducer);
	void DrawSimpleDataVectorInTimeSerial(std::vector<std::pair<double, NFmiMetTime> > &theDataVector, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& thePenSize, const NFmiPoint& theSinglePointSize);
	void DrawAnalyzeToolDataLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment &envi, boost::shared_ptr<NFmiFastQueryInfo> &analyzeDataInfo);
	void DrawAnalyzeToolChangeLine(const NFmiPoint &theLatLonPoint);
    void DrawObsBlenderChangeLine(const NFmiPoint &theLatLonPoint);
    void DrawAnalyzeToolEndTimeLine(void);
    void DrawAnalyzeToolRelatedMessages(const std::vector<std::string> &messages, NFmiDrawingEnvironment &envi);
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
    void DrawCPReferenceLines();
    void DrawCPReferenceLines_DrawAllCps(bool drawModificationLines);
    void DrawCPReferenceLines_ForCurrentCp(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager, boost::shared_ptr<NFmiFastQueryInfo> &info, CpDrawingOptions &cpDrawingOptions, bool drawModificationLines);
    void DrawCPReferenceLines_SetLineOptions(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager, CpDrawingOptions &cpDrawingOptions);
    bool DrawCPReferenceLines_IsCpDrawn(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager);
    void DrawCPReferenceLines_AdvanceDrawingOptions(CpDrawingOptions &cpDrawingOptions);
    void DrawCPReferenceLines_DrawLegend(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager, CpDrawingOptions &cpDrawingOptions);
    void DrawCPReferenceLines_DrawCpLocation(boost::shared_ptr<NFmiEditorControlPointManager> &cpManager, CpDrawingOptions &cpDrawingOptions, bool drawModificationLines);
    bool IsThisFirstEditedParamRow();
    NFmiDrawingEnvironment MakeNormalCpLineDrawOptions() const;
    NFmiDrawingEnvironment MakeChangeCpLineDrawOptions() const;
	void DrawLocationDataIncrementally(void);
	void DrawIncrementalDataLegend(void);
	void DrawSelectedStationDataIncrementally(void);
	virtual boost::shared_ptr<NFmiFastQueryInfo> Info(void) const;
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
	bool ModifyFactorPointsManual (double theValue, int theIndex);
	virtual void FixModifyFactorValue (double & theValue);
	void CalcSuitableLowAndHighLimits (double & low, double & high, double step);
	double LowerScaleValueRound (double value, double step);
	double HigherScaleValueRound (double value, double step);
	NFmiDrawingEnvironment NormalStationDataCurveEnvironment (void);
	NFmiDrawingEnvironment ChangeStationDataCurveEnvironment (double theRelativeFillFactor = 0.6, double theSublinesPerUnitFactor = 50) const;
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
    bool IsEditedData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo) const;
    bool IsMosTemperatureMinAndMaxDisplayed(boost::shared_ptr<NFmiFastQueryInfo> &theViewedInfo);
    void DrawExistingDataLegend(const NFmiProducer &producer, NFmiInfoData::Type dataType, boost::shared_ptr<NFmiDrawParam> &drawParam, const NFmiColor &color, double heightIncrement, double endPointX, NFmiPoint &legendPlaceInOut, NFmiDrawingEnvironment &drawingEnvironmentInOut);
    bool IsAnalyzeRelatedToolUsed() const;
    bool IsModifyFactorViewClicked(const NFmiPoint &thePlace) const;
    void DrawSinglePointData(double value, const NFmiMetTime &time, NFmiDrawingEnvironment &theEnvi, const NFmiPoint& theSinglePointSize);
    void DrawObservationBlenderDataInCpMode();
    bool IsModificationLineDrawn() const;
    boost::shared_ptr<NFmiFastQueryInfo> GetAnalyzeToolData();
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> GetObsBlenderDataVector();
    virtual bool DrawHelperData() const { return true; }
    void DrawAnalyzeToolRelatedChangeLineFinal(bool useObservationData, float usedAnalyzeValue, const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment &envi, boost::shared_ptr<NFmiFastQueryInfo> &editedInfo, boost::shared_ptr<NFmiFastQueryInfo> &usedToolInfo, const NFmiMetTime &startTime, std::vector<std::string> &messages);

	NFmiDrawingEnvironment itsNormalCurveEnvi; // miten normaali k�yr� piirret��n
	NFmiDrawingEnvironment itsChangeCurveEnvi; // miten muutos k�yr� piirret��n (punainen katkoviiva)
	NFmiDrawingEnvironment itsIncrementalCurveEnvi; // miten mahd. incrementaali k�yr� piirret��n
	NFmiDrawingEnvironment itsChanheIncrementalCurveEnvi; // miten mahd. incrementaali k�yr�n mahd. muutos k�yr� piirret��n

	NFmiAxisView * itsValueView;
	NFmiAxisView * itsModifyFactorView;
	std::vector<double> itsModificationFactorCurvePoints;
	NFmiAxis * itsValueAxis;
	NFmiAxis * itsModifyFactorAxis;

	NFmiRect itsModifyingUnitTextRect;
	NFmiRect itsDataRect; // t�lle alueelle piirret��n aikasarja ikkunassa k�yr�t

 protected:
	bool ScanDataForMinAndMaxValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theLatlon, const NFmiTimeBag &theLimitingTimes, NFmiDataModifierMinMax &theAutoAdjustMinMaxValuesOut, const NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage, unsigned long wantedParamId);

	bool fJustScanningForMinMaxValues; // auto-adjust s��d�t lasketaan l�ydettyjen min ja max arvojen avulla. Jos t�m� optio on true, 
										// ei piirret� mit��n, etsit��n vain min ja max arvoja. Tarkoitus on ett� optio laitetaan p��lle
										// AutoAdjustValueScale-metodissa ja sitten kutsutaan DrawSelectedStationData() -metodia, joka k�y
										// l�pi kaikki 'piirrett�v�t' datat etsien min ja max arvoja.
	NFmiDataModifierMinMax itsAutoAdjustMinMaxValues; // T�h�n talletetaan skannauksen yhteydes�� l�ydetyt minimi ja maksimi arvot
	NFmiTimeBag itsAutoAdjustScanTimes; // kaikki datat k�yd��n l�pi alkaen timabagin alku ajasta loppu aikaan etsitt�ess� min ja max arvoja
	std::vector<NFmiPoint> itsScannedLatlonPoints; // katseltavalle datalle t�ss� voi olla useita pisteit�, mutta muille apudatoille k�ytet��n vain 1. pistett�

private:

	unsigned long itsMaxStationShowed;
	double itsSinAmplitude;
	int itsPhase;
	std::vector<NFmiColor> itsProducerModelDataColors; // t�h�n on laitettu eri mallien piirtovarit (producersystemin j�rjestys)
    bool fEditingMouseMotionsAllowed; // T�ll� kontrolloidaan hiirell� tehtyj� editointiin liittyvi� muutoksia. Tietyt muutokset sallitaan vain jos t�m� on true.
                        // Tarkoitus on est�� vanhinkojen teko, esim. hiirell� manipuloidaan aikakontrollia, mutta se p��stet��n vasta n�yt�n dataosion 
                        // p��ll� irti ja buttonUp:issa tehd��n muutos editointi pisteisiin. Sama p�tee asteikkojen s��d�st� alkanutta hiiren k�sittely�.
};

