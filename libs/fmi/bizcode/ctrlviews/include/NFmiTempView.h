#pragma once

#include "NFmiCtrlView.h"
#include "NFmiParameterName.h"
#include "NFmiMTATempSystem.h"
#include "NFmiSoundingDataOpt1.h"
#include "NFmiTempViewDataRects.h"
#include "NFmiTempViewScrollingData.h"
#include <gdiplus.h>

class NFmiFastQueryInfo;
class NFmiProducer;
class NFmiText;
class NFmiTempLineInfo;
class NFmiTempLabelInfo;

class NFmiTempView : public NFmiCtrlView
{
 public:
	typedef enum
	{
		kNone = 0, // jos ei haluta mit��n merkki�, vaan pelkk� teksti markerin piirto funktiossa
		kCircle = 1,
		kTriangle = 2,
		kCross = 3
	} MarkerShape;

    using SoundingDataCacheMap = std::map<NFmiMTATempSystem::SoundingDataCacheMapKey, NFmiSoundingDataOpt1>;

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

 private:
	void DrawOneSounding(const NFmiMTATempSystem::SoundingProducer &theProducer, const NFmiMTATempSystem::TempInfo &theTempInfo, int theProducerIndex, double theBrightningFactor, int theModelRunIndex);
	double ExtraPrintLineThicknesFactor(bool fMainCurve);
	NFmiPoint ScaleOffsetPoint(const NFmiPoint &thePoint);
	double CalcDataRectPressureScaleRatio(void);
    void CalcDrawSizeFactors(void);
	void DrawWindModificationArea(void);
	bool QuickModifySounding(NFmiSoundingDataOpt1 &theSoundingData, FmiParameterName theParam, double P, double T, double Td);
	NFmiPoint CalcStabilityIndexStartPoint(void);
	void DrawAllLiftedAirParcels(NFmiSoundingDataOpt1 &theData);
	void DrawLiftedAirParcel(NFmiSoundingDataOpt1 &theData, FmiLCLCalcType theLCLCalcType);
	bool ModifySounding(NFmiSoundingDataOpt1 &theSoundingData, const NFmiPoint &thePlace, unsigned long theKey, FmiParameterName theParam, int theDistToleranceInPixels);
	void DrawHodograf(NFmiSoundingDataOpt1 &theData, int theProducerIndex);
	void DrawHodografTextWithMarker(const std::string &theText, float u, float v, const NFmiColor &theTextColor, const NFmiColor &theMarkerColor, const NFmiColor &theMarkerFillColor, int theMarkerSizeInPixel, int theFontSize, FmiDirection theTextAlignment, MarkerShape theMarkerShape);
	void DrawHodografUpAndDownWinds(NFmiSoundingDataOpt1 & theData, int theProducerIndex);
	void DrawHodografWindVectorMarkers(NFmiSoundingDataOpt1 & theData, int theProducerIndex);
	void DrawHodografBase(int theProducerIndex);
	void DrawHodografCurve(NFmiSoundingDataOpt1 &theData, int theProducerIndex);
	void DrawHodografHeightMarkers(NFmiSoundingDataOpt1 &theData, int theProducerIndex);
	NFmiPoint GetRelativePointFromHodograf(double u, double v);
	void DrawSoundingInTextFormat(NFmiSoundingDataOpt1 &theData);
	bool FillSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiSoundingDataOpt1 &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation, boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo, const NFmiMTATempSystem::SoundingProducer &theProducer);
	void DrawSounding(NFmiSoundingDataOpt1 & theUsedDataInOut, int theProducerIndex, const NFmiColor &theUsedSoundingColor, bool fMainCurve, bool onSouthernHemiSphere);
	void DrawSoundingsInMTAMode(void);
	void DrawBackground(void);
	void DrawStabilityIndexData(NFmiSoundingDataOpt1& usedData);
	void DrawTextualSoundingData(NFmiSoundingDataOpt1& usedData);
	void DrawStabilityIndexBackground(const NFmiRect &sideViewRect);
	void DrawDryAdiapaticks(void);
	void DrawMoistAdiapaticks(void);
	void DrawYAxel(void);
	void DrawXAxel(void);
	void DrawHeightScale(void);
	void DrawFlightLevelScale(void);
	void DrawMixingRatio(void);
	void DrawCondensationTrailProbabilityLines(void);
	void DrawCondensationTrailRHValues(NFmiSoundingDataOpt1 &theData, double startP, double endP, double theMixRatio);
	void DrawMixingRatio(const NFmiTempLabelInfo &theLabelInfo, const NFmiTempLineInfo &theLineInfo,
						   const std::vector<double> &theValues, double startP, double endP, double deltaStartLevelP,
						   NFmiDrawingEnvironment * theEnvi);
	void DrawTemperatures(NFmiSoundingDataOpt1 &theData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo);
	std::string MakeTextualSoundingLevelString(int levelIndex, std::deque<float>& pVec, std::deque<float>& tVec, std::deque<float>& tdVec, std::deque<float>& zVec, std::deque<float>& wsVec, std::deque<float>& wdVec);
	std::vector<std::string> MakeSoundingDataLevelStrings(NFmiSoundingDataOpt1& theData);
	void DrawWantedTextualSoundingDataLevels(NFmiText& text, NFmiPoint& p, const std::vector<std::string>& levelStrings, double relativeLineHeight);
	double CalcRelativeTextLineHeight(int fontSizeInPixels, double heightFactor);
	double CalcSideViewTextRowCount(const NFmiRect& viewRect, const NFmiPoint &currentRowCursor, double relativeTextRowHeight, bool advanceBeforeDraw);
	void DrawSimpleLineWithGdiplus(const NFmiTempLineInfo& lineInfo, const NFmiPoint& relativeP1, const NFmiPoint& relativeP2, bool fixEndPixelX, bool fixEndPixelY);
	double GetPAxisChangeValue(double change);
	void DrawLine(const NFmiPoint &p1, const NFmiPoint &p2, bool drawSpecialLines, int theTrueLineWidth, bool startWithXShift, int theHelpDotPixelSize, NFmiDrawingEnvironment * theEnvi);
	void DrawHelpLineLabel(const NFmiPoint &p1, const NFmiPoint &theMoveLabelRelatively, double theValue, const NFmiTempLabelInfo &theLabelInfo, NFmiDrawingEnvironment * theEnvi, const NFmiString &thePostStr = NFmiString(""));
	void DrawWind(NFmiSoundingDataOpt1 &theData, int theProducerIndex, bool onSouthernHemiSphere);
	void DrawStationInfo(NFmiSoundingDataOpt1 &theData, int theProducerIndex);
	void DrawHeightValues(NFmiSoundingDataOpt1 &theData, int theProducerIndex);
	void MoveToNextLine(double relativeLineHeight, NFmiPoint &theTextPoint);
	void DrawNextLineToIndexView(double relativeLineHeight, NFmiText& theText, const NFmiString& theStr, NFmiPoint& theTextPoint, bool moveFirst = true, bool addToString = true);
	void DrawLCL(NFmiSoundingDataOpt1 &theData, int theProducerIndex, FmiLCLCalcType theLCLCalcType);
	void DrawTrMw(NFmiSoundingDataOpt1 &theData, int theProducerIndex);
	NFmiString GetIndexText(double theValue, const NFmiString &theText, int theDecimalCount);
	void InitializeHodografRect(void);
	void DrawAnimationControls(void);
	bool ModifySoundingWinds(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);
    void DrawSecondaryDataRect();
    void DrawSecondaryDataHorizontalAxel(NFmiTempLabelInfo &theLabelInfo, NFmiTempLineInfo &theLineInfo, FmiDirection theLabelTextAlignment, double theYPosition, const NFmiPoint &theLabelOffset);
    void DrawSecondaryData(NFmiSoundingDataOpt1 &theUsedData, const NFmiColor &theUsedSoundingColor);
    void DrawSecondaryData(NFmiSoundingDataOpt1 &theUsedData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo);
    double SecondaryDataFrameXoffset(double theValue);
    void DrawSecondaryVerticalHelpLine(double theBottom, double theTop, double theValue);
    bool FillSoundingDataFromServer(const NFmiMTATempSystem::SoundingProducer &theProducer, NFmiSoundingDataOpt1 &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation);
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
	void FillInPossibleMissingPressureData(NFmiSoundingDataOpt1& theSoundingData, const NFmiProducer& dataProducer, const NFmiMetTime& theTime, const NFmiLocation& theLocation);
	bool IsSelectedProducerIndex(int theProducerIndex) const;
	void SetupUsedSoundingData(NFmiSoundingDataOpt1& theUsedDataInOut, int theProducerIndex, bool fMainCurve);
	const NFmiColor& GetSelectedProducersColor() const;
	NFmiPoint CalcStringRelativeSize(const std::string& str, double fontSize, const std::string& fontName);

	double Tpot2x(double tpot, double p);
	double pt2x(double p, double t);
	double p2y(double p);
	double h2y(double h);
	double y2p(double y);
	double xy2t(double x, double y);

	// T�m� piirtoasetus olio luodaan konstruktorissa ja se annetaan emoluokalle ja tuhotaan destruktorissa
	NFmiDrawingEnvironment* itsTempDrawingEnvi; 

	double tmax;
	double tmin;
	double dt;

	double pmin;
	double pmax;

	double tdegree;

	double dtperpix; // kuinka paljon l�mp�tila muuttuu x-pixelin suuntaan
	double dlogpperpix; // kuinka paljon log(p) muuttuu y-pixelin suuntaan
	int xpix; // kuinka monta pikseli� data alueella x-suunnassa
	int ypix; // kuinka monta pikseli� data alueella y-suunnassa
	NFmiPoint _1PixelInRel; // kuinka paljon on yksi pikseli relatiivisessa maailmassa

	// T�h�n talletetaan valitun tuottajan piirrett�v�n luotauksen datat, ett� niit� k�ytet��n indeksi osio laskuissa
	// Valitun luotaustuottajan dataa k�ytet��n my�s luotausten muokkauksessa!!!!
	NFmiSoundingDataOpt1 itsSelectedProducerSoundingData;
	bool fMustResetFirstSoundingData; // joskus k�ytt�j� haluaa resetoida muokattua luotausta
	bool fHodografInitialized; // kun ikkuna piirret��n 1. kerran, pit�� laskea alkuarvaus, muutoin ei
	double itsFirstSoundinWindBarbXPos;
	std::string itsSoundingIndexStr; // t�h�n talletetaan aina piirretty stabiilisuus indeksi teksti rimpsu / luotaus numeroina, riippuen mik� moodi on valittuna, tai molemmat

	NFmiPoint itsGdiplusScale; // GDI+ piirrossa kaikki koordinaatit pit�� muuttaa pikseli maailmaan (toolbaxin relatiivisesta 0,0 - 1,1 maailmasta)

	// Teen seuraavanlaisen systeemin ett� printtauksessa tulee sopivan kokoisia fontteja ja viiva paksuuksia:
	// Otan talteen viimeisen ruudulla piirrettyjen pikselien koot mm.
	// Kun ollaan printtaamassa kuvaa, lasketaan kerroin, mill� ruutu piirrossa olevat pikseli m��r�t on kerrottava
	// ett� saadaan saman kokoisia piirto-olioita my�s paperille.
	NFmiPoint itsDrawSizeFactor; // ruutu piirrossa 1, lasketaan printatessa t�lle erillinen arvo
	NFmiPoint itsLastScreenDrawPixelSizeInMM;
    // t�m�n avulla yritet��n viel� korjata koko laskuja, koska n�yt�nohjaimet eiv�t anna aina 
    // oikeita millimetri kokoja n�yt�ille. Jos 0, ei ole tietoa suhteesta, eik� korjausta voi tehd�.
	double itsLastScreenDataRectPressureScaleRatio; 
    // Jotta luotausk�yrien tooltipit saadaan varmasti laskettua kaikissa tilanteissa, laitetaan kaikki piirretyt luotausdatat erilliseen cacheen talteen.
    SoundingDataCacheMap itsSoundingDataCacheForTooltips;
    Gdiplus::SmoothingMode itsCurveDrawSmoothingMode = Gdiplus::SmoothingMode::SmoothingModeAntiAlias;
	NFmiTempViewDataRects itsTempViewDataRects;
	double itsStabilityIndexNextLineFontHeightFactor = 1.;
	double itsTextualSoundingDataNextLineFontHeightFactor = 0.8;
	double itsStabilityIndexRelativeLineHeight;
	double itsTextualSoundingDataRelativeLineHeight;
	NFmiTempViewScrollingData itsTempViewScrollingData;
};


