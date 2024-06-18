#pragma once

#include "NFmiCtrlView.h"
#include "NFmiParameterName.h"
#include "NFmiMTATempSystem.h"
#include "NFmiSoundingData.h"
#include "NFmiTempViewDataRects.h"
#include "NFmiTempViewScrollingData.h"
#include <gdiplus.h>

class NFmiFastQueryInfo;
class NFmiProducer;
class NFmiText;
class NFmiTempLineInfo;
class NFmiTempLabelInfo;
class NFmiDataModifierMinMax;

enum class SoundingViewOperationMode
{
	NormalDrawMode,
	FitScalesScanMode
};

class TotalSoundingData
{
public:
	NFmiSoundingData itsSoundingData;
	double itsIntegrationRangeInKm = 0;
	double itsIntegrationTimeOffset1InHours = 0;
	double itsIntegrationTimeOffset2InHours = 0;
	int itsIntegrationPointsCount = 0;
	int itsIntegrationTimesCount = 0;

	TotalSoundingData();
	TotalSoundingData(NFmiMTATempSystem& mtaTempSystem);
	bool IsSameSounding(TotalSoundingData& other);
	bool HasAvgIntegrationData() const;
};

class NFmiTempView : public NFmiCtrlView
{
 public:
	typedef enum
	{
		kNone = 0, // jos ei haluta mitään merkkiä, vaan pelkkä teksti markerin piirto funktiossa
		kCircle = 1,
		kTriangle = 2,
		kCross = 3
	} MarkerShape;

	struct SoundingDataEqual
	{
		double itsAvgRangeInKm = 0;
		double itsAvgTimeRange1Inhours = 0;
		double itsAvgTimeRange2Inhours = 0;

		bool operator==(const SoundingDataEqual& other) const;
	};

	struct LegendDrawingSetup
	{
		FmiDirection oldTextAlignment = kNoDirection;
		double maxNameLengthRelative = 0;
		double textLineHeightRelative = 0;
	};

	struct LegendDrawingLineData
	{
		std::string itsText;
		NFmiColor itsTextColor;
		NFmiColor itsBackgroundColor;
		bool fDoHorizontalLineSeparator = false;
	};

    using SoundingDataCacheMap = std::map<NFmiMTATempSystem::SoundingDataCacheMapKey, TotalSoundingData>;

	NFmiTempView( const NFmiRect& theRect
				 ,NFmiToolBox* theToolBox);
	~NFmiTempView(void);
	void Draw(NFmiToolBox *theToolBox) override;

	bool LeftButtonUp(const NFmiPoint &thePlace, unsigned long theKey) override;
	bool RightButtonUp(const NFmiPoint &thePlace, unsigned long theKey) override;
	bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
	void ResetScales(void);
	void DrawOverBitmapThings(NFmiToolBox *theGTB, const NFmiPoint &thePlace);
	bool MustResetFirstSoundingData(void) const {return fMustResetFirstSoundingData;}
	void MustResetFirstSoundingData(bool newState) {fMustResetFirstSoundingData = newState;}
	const std::string& SoundingIndexStr(void) const {return itsSoundingIndexStr;}

	bool LeftButtonDown(const NFmiPoint &thePlace, unsigned long theKey) override;
	bool RightButtonDown(const NFmiPoint &thePlace, unsigned long theKey) override;
	bool MouseMove(const NFmiPoint &thePlace, unsigned long theKey) override;
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
	void AutoAdjustSoundingScales();

 private:
	void DrawOneSounding(const NFmiMTATempSystem::SoundingProducer &theProducer, const NFmiMTATempSystem::TempInfo &theTempInfo, int theProducerIndex, double theBrightningFactor, int theModelRunIndex);
	double ExtraPrintLineThicknesFactor(bool fMainCurve);
	NFmiPoint ScaleOffsetPoint(const NFmiPoint &thePoint);
	double CalcDataRectPressureScaleRatio(void);
    void CalcDrawSizeFactors(void);
	void DrawWindModificationArea(void);
	bool QuickModifySounding(NFmiSoundingData &theSoundingData, FmiParameterName theParam, double P, double T, double Td);
	NFmiPoint CalcStabilityIndexStartPoint(void);
	void DrawAllLiftedAirParcels(NFmiSoundingData &theData);
	void DrawLiftedAirParcel(NFmiSoundingData &theData, FmiLCLCalcType theLCLCalcType);
	bool ModifySounding(NFmiSoundingData &theSoundingData, const NFmiPoint &thePlace, unsigned long theKey, FmiParameterName theParam, int theDistToleranceInPixels);
	void DrawHodograf(NFmiSoundingData &theData, int theProducerIndex);
	void DrawHodografTextWithMarker(const std::string &theText, float u, float v, const NFmiColor &theTextColor, const NFmiColor &theMarkerColor, const NFmiColor &theMarkerFillColor, int theMarkerSizeInPixel, int theFontSize, FmiDirection theTextAlignment, MarkerShape theMarkerShape);
	void DrawHodografUpAndDownWinds(NFmiSoundingData & theData, int theProducerIndex);
	void DrawHodografWindVectorMarkers(NFmiSoundingData & theData, int theProducerIndex);
	void DrawHodografBase(int theProducerIndex);
	void DrawHodografCurve(NFmiSoundingData &theData, int theProducerIndex);
	void DrawHodografHeightMarkers(NFmiSoundingData &theData, int theProducerIndex);
	NFmiPoint GetRelativePointFromHodograf(double u, double v);
	void DrawSoundingInTextFormat(TotalSoundingData & usedTotalData);
	bool FillSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, TotalSoundingData &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation, boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo, const NFmiMTATempSystem::SoundingProducer &theProducer);
	void DrawSounding(TotalSoundingData& theUsedDataInOut, int theProducerIndex, const NFmiColor &theUsedSoundingColor, bool fMainCurve, bool onSouthernHemiSphere, bool isNewData);
	void DrawSoundingsInMTAMode(void);
	void DrawBackground(void);
	void DrawStabilityIndexData(NFmiSoundingData& usedData);
	void DrawTextualSoundingData(TotalSoundingData& usedTotalData);
	void DrawStabilityIndexBackground(const NFmiRect &sideViewRect);
	void DrawDryAdiapaticks(void);
	void DrawMoistAdiapaticks(void);
	void DrawYAxel(void);
	void DrawXAxel(void);
	void DrawHeightScale(void);
	void DrawFlightLevelScale(void);
	void DrawMixingRatio(void);
	void DrawCondensationTrailProbabilityLines(void);
	void DrawCondensationTrailRHValues(NFmiSoundingData &theData, double startP, double endP, double theMixRatio);
	void DrawMixingRatio(const NFmiTempLabelInfo &theLabelInfo, const NFmiTempLineInfo &theLineInfo,
						   const std::vector<double> &theValues, double startP, double endP, double deltaStartLevelP,
						   NFmiDrawingEnvironment * theEnvi);
	void DrawTemperatures(NFmiSoundingData &theData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo, const NFmiGroundLevelValue& groundLevelValue);
	std::pair<float, std::string> MakeTextualSoundingLevelString(int levelIndex, std::deque<float>& pVec, std::deque<float>& tVec, std::deque<float>& tdVec, std::deque<float>& zVec, std::deque<float>& wsVec, std::deque<float>& wdVec);
	std::vector<std::pair<float, std::string>> MakeSoundingDataLevelStrings(NFmiSoundingData& theData);
	void DrawWantedTextualSoundingDataLevels(NFmiText& text, NFmiPoint& p, const std::vector<std::pair<float, std::string>>& levelStrings, double relativeLineHeight, const NFmiGroundLevelValue& groundLevelValue);
	double CalcRelativeTextLineHeight(int fontSizeInPixels, double heightFactor);
	double CalcSideViewTextRowCount(const NFmiRect& viewRect, const NFmiPoint &currentRowCursor, double relativeTextRowHeight, bool advanceBeforeDraw);
	void DrawSimpleLineWithGdiplus(const NFmiTempLineInfo& lineInfo, const NFmiPoint& relativeP1, const NFmiPoint& relativeP2, bool fixEndPixelX, bool fixEndPixelY);
	double GetPAxisChangeValue(double change);
	void DrawLine(const NFmiPoint &p1, const NFmiPoint &p2, bool drawSpecialLines, int theTrueLineWidth, bool startWithXShift, int theHelpDotPixelSize, NFmiDrawingEnvironment * theEnvi);
	void DrawHelpLineLabel(const NFmiPoint &p1, const NFmiPoint &theMoveLabelRelatively, double theValue, const NFmiTempLabelInfo &theLabelInfo, NFmiDrawingEnvironment * theEnvi, const std::string&thePostStr = std::string());
	void DrawWind(NFmiSoundingData &theData, int theProducerIndex, bool onSouthernHemiSphere, const NFmiGroundLevelValue& groundLevelValue);
	void DrawStationInfo(TotalSoundingData &theData, int theProducerIndex, bool isNewData);
	void DrawHeightValues(NFmiSoundingData &theData, int theProducerIndex, const NFmiGroundLevelValue& groundLevelValue);
	void MoveToNextLine(double relativeLineHeight, NFmiPoint &theTextPoint);
	void DrawNextLineToIndexView(double relativeLineHeight, NFmiText& theText, const std::string& theStr, NFmiPoint& theTextPoint, bool moveFirst = true, bool addToString = true);
	void DrawLCL(NFmiSoundingData &theData, int theProducerIndex, FmiLCLCalcType theLCLCalcType);
	void DrawTrMw(NFmiSoundingData &theData, int theProducerIndex);
	std::string GetIndexText(double theValue, const std::string &theText, int theDecimalCount);
	void InitializeHodografRect(void);
	void DrawAnimationControls(void);
	bool ModifySoundingWinds(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);
    void DrawSecondaryDataRect();
    void DrawSecondaryDataHorizontalAxel(NFmiTempLabelInfo &theLabelInfo, NFmiTempLineInfo &theLineInfo, FmiDirection theLabelTextAlignment, double theYPosition, const NFmiPoint &theLabelOffset);
    void DrawSecondaryData(NFmiSoundingData &theUsedData, const NFmiColor &theUsedSoundingColor, const NFmiGroundLevelValue& groundLevelValue);
    void DrawSecondaryData(NFmiSoundingData &theUsedData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo, const NFmiGroundLevelValue& groundLevelValue);
    double SecondaryDataFrameXoffset(double theValue);
    void DrawSecondaryVerticalHelpLine(double theBottom, double theTop, double theValue);
    bool FillSoundingDataFromServer(const NFmiMTATempSystem::SoundingProducer &theProducer, NFmiSoundingData &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation);
	bool FillServerGribSoundingData(NFmiSoundingData& theSoundingData, const NFmiMetTime& theTime, const NFmiLocation& theLocation, const ModelDataServerConfiguration& modelDataServerConf, const std::string& gribSoundingDataResponseFromServer);
	Gdiplus::SmoothingMode GetUsedCurveDrawSmoothingMode() const;
    bool IsRectangularTemperatureHelperLines() const;
	bool IsAnyTextualSideViewVisible() const;
	void DrawTextualSideViewRelatedStuff();
	void CalculateAllDataViewRelatedRects();
	double ConvertFixedPixelSizeToRelativeWidth(long fixedPixelSize);
	double ConvertFixedPixelSizeToRelativeHeight(long fixedPixelSize);
	void ResetTextualScrollingIfSoundingDataChanged(const NFmiMTATempSystem::SoundingProducer& theProducer, const NFmiMTATempSystem::TempInfo& theTempInfo, boost::shared_ptr<NFmiFastQueryInfo>& theInfo, int theProducerIndex);
	void DrawTextualSideViewScrollingVisuals(NFmiPoint& p, double relativeLineHeight, int totalSoundingRows, int fullVisibleRows, int startingRowIndex, bool drawUpwardSounding);
	bool DoTextualSideViewSetup(bool showSideView, const NFmiRect& sideViewRect, int fontSize, double fontHeightFactor, double& relativeLineHeightOut);
	void FillInPossibleMissingPressureData(NFmiSoundingData& theSoundingData, const NFmiProducer& dataProducer, const NFmiMetTime& theTime, const NFmiLocation& theLocation);
	bool IsSelectedProducerIndex(int theProducerIndex) const;
	void SetupUsedSoundingData(TotalSoundingData& theUsedDataInOut, int theProducerIndex, bool fMainCurve);
	const NFmiColor& GetSelectedProducersColor() const;
	NFmiPoint CalcStringRelativeSize(const std::string& str, double fontSize, const std::string& fontName);
	bool DoIntegrationSounding(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, TotalSoundingData& theSoundingData);
	bool FillIntegrationSounding(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, TotalSoundingData& theSoundingData, const NFmiMetTime& theTime, const NFmiLocation& theLocation, boost::shared_ptr<NFmiFastQueryInfo>& theGroundDataInfo);
	std::vector<unsigned long> CalcAreaIntegrationLocationIndexes(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const NFmiLocation& theLocation, double theRangeInMeters);
	bool FillIntegrationSounding(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, TotalSoundingData& theSoundingData, const NFmiMetTime& theTime, const NFmiLocation& theLocation, boost::shared_ptr<NFmiFastQueryInfo>& theGroundDataInfo, unsigned long timeIndex1, unsigned long timeIndex2, const std::vector<unsigned long> &locationIndexes);
	bool CheckIsSoundingDataChanged(TotalSoundingData& theUsedData);
	SoundingDataEqual MakeSoundingDataEqual();
	void SetupLegendDrawingEnvironment();
	NFmiPoint CalcLegendTextStartPoint();
	void DrawSelectedProducerIndexText(const NFmiPoint &textPoint);
	void AddLegendLineData(const std::string &text, const NFmiColor &textColor, const NFmiColor &backgroundColor, bool doHorizontalLineSeparator);
	void DrawLegendLineData();
	void DrawLegendLineDataSeparator(const NFmiPoint& textPoint);
	void AddPossibleAvgIntegrationInfo(TotalSoundingData& theData, const NFmiColor& textColor, const NFmiColor& backgroundColor);
	void ResetSelectedDataInEmptyCase(int theProducerIndex, int theModelRunIndex);
	void DrawMainDataLegendInEmptyCase(bool mainCurve, const NFmiMTATempSystem::TempInfo& usedTempInfo, const NFmiMTATempSystem::SoundingProducer& theProducer, int theModelRunIndex, int theProducerIndex);
	TotalSoundingData GetTotalsoundingData(boost::shared_ptr<NFmiFastQueryInfo>& info, NFmiMTATempSystem::TempInfo& usedTempInfo, const NFmiMTATempSystem::SoundingProducer& theProducer, int theProducerIndex);
	bool IsInScanMode() const;
	void ScanVisualizedData();
	void ScanSingleDataVaisala(TotalSoundingData& totalSoundingData, NFmiDataModifierMinMax& theAutoAdjustTMinMaxValuesOut);
	void ScanSingleDataSkewT(TotalSoundingData& totalSoundingData, int &potenciallyVisibleValuesInOut, int& actuallyVisibleValuesInOut);
	std::pair<double, double> GetVisibleTemperatureRangeForVaisalaDiagram(const std::pair<double, double>& originalRange);
	std::pair<double, double> GetVisibleTemperatureRangeForSkewTDiagram(const std::pair<double, double> &originalRange);
	void CheckIsTVisible(float T, float P, double yPos, int& potenciallyVisibleValuesInOut, int& actuallyVisibleValuesInOut);
	void SetupTAxisValues(double startT, double endT);
	bool ScanRangeForAllDataSkewT(double startT, double endT, int& potenciallyVisibleValuesInOut, int& actuallyVisibleValuesInOut);
	NFmiGroundLevelValue GetPossibleGroundLevelValue(boost::shared_ptr<NFmiFastQueryInfo> &soundingInfo, const NFmiPoint& latlon, const NFmiMetTime& atime);

	double Tpot2x(double tpot, double p);
	double pt2x(double p, double t);
	double p2y(double p);
	double h2y(double h);
	double y2p(double y);
	double xy2t(double x, double y);

	// Tämä piirtoasetus olio luodaan konstruktorissa ja se annetaan emoluokalle ja tuhotaan destruktorissa
	NFmiDrawingEnvironment* itsTempDrawingEnvi; 

	double tmax;
	double tmin;
	double dt;

	double pmin;
	double pmax;

	double tdegree;

	double dtperpix; // kuinka paljon lämpötila muuttuu x-pixelin suuntaan
	double dlogpperpix; // kuinka paljon log(p) muuttuu y-pixelin suuntaan
	int xpix; // kuinka monta pikseliä data alueella x-suunnassa
	int ypix; // kuinka monta pikseliä data alueella y-suunnassa
	NFmiPoint _1PixelInRel; // kuinka paljon on yksi pikseli relatiivisessa maailmassa

	// Tähän talletetaan valitun tuottajan piirrettävän luotauksen datat, että niitä käytetään indeksi osio laskuissa
	// Valitun luotaustuottajan dataa käytetään myös luotausten muokkauksessa!!!!
	TotalSoundingData itsSelectedProducerSoundingData;
	bool fMustResetFirstSoundingData; // joskus käyttäjä haluaa resetoida muokattua luotausta
	bool fHodografInitialized; // kun ikkuna piirretään 1. kerran, pitää laskea alkuarvaus, muutoin ei
	double itsFirstSoundinWindBarbXPos;
	std::string itsSoundingIndexStr; // tähän talletetaan aina piirretty stabiilisuus indeksi teksti rimpsu / luotaus numeroina, riippuen mikä moodi on valittuna, tai molemmat

	NFmiPoint itsGdiplusScale; // GDI+ piirrossa kaikki koordinaatit pitää muuttaa pikseli maailmaan (toolbaxin relatiivisesta 0,0 - 1,1 maailmasta)

	// Teen seuraavanlaisen systeemin että printtauksessa tulee sopivan kokoisia fontteja ja viiva paksuuksia:
	// Otan talteen viimeisen ruudulla piirrettyjen pikselien koot mm.
	// Kun ollaan printtaamassa kuvaa, lasketaan kerroin, millä ruutu piirrossa olevat pikseli määrät on kerrottava
	// että saadaan saman kokoisia piirto-olioita myös paperille.
	NFmiPoint itsDrawSizeFactor; // ruutu piirrossa 1, lasketaan printatessa tälle erillinen arvo
	NFmiPoint itsLastScreenDrawPixelSizeInMM;
    // tämän avulla yritetään vielä korjata koko laskuja, koska näytönohjaimet eivät anna aina 
    // oikeita millimetri kokoja näytöille. Jos 0, ei ole tietoa suhteesta, eikä korjausta voi tehdä.
	double itsLastScreenDataRectPressureScaleRatio; 
    // Jotta luotauskäyrien tooltipit saadaan varmasti laskettua kaikissa tilanteissa, laitetaan kaikki piirretyt luotausdatat erilliseen cacheen talteen.
    SoundingDataCacheMap itsSoundingDataCacheForTooltips;
    Gdiplus::SmoothingMode itsCurveDrawSmoothingMode = Gdiplus::SmoothingMode::SmoothingModeAntiAlias;
	NFmiTempViewDataRects itsTempViewDataRects;
	double itsStabilityIndexNextLineFontHeightFactor = 1.;
	double itsTextualSoundingDataNextLineFontHeightFactor = 0.8;
	double itsStabilityIndexRelativeLineHeight;
	double itsTextualSoundingDataRelativeLineHeight;
	NFmiTempViewScrollingData itsTempViewScrollingData;
	LegendDrawingSetup itsLegendDrawingSetup;
	std::vector<LegendDrawingLineData> itsLegendDrawingLineData;
	SoundingViewOperationMode itsOperationalMode = SoundingViewOperationMode::NormalDrawMode;
	std::vector<TotalSoundingData> itsScanData;
};


