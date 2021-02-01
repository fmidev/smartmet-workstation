#pragma once

#include "NFmiCtrlView.h"
#include "NFmiParameterName.h"
#include "NFmiMTATempSystem.h"
#include "NFmiSoundingDataOpt1.h"
#include "NFmiTempViewDataRects.h"
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
	virtual ~NFmiTempView(void);
	void Draw(NFmiToolBox *theToolBox);

	bool LeftButtonUp(const NFmiPoint &thePlace, unsigned long theKey);
	bool RightButtonUp(const NFmiPoint &thePlace, unsigned long theKey);
	bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);
	void ResetScales(void);
	void DrawOverBitmapThings(NFmiToolBox *theGTB, const NFmiPoint &thePlace);
	bool MustResetFirstSoundingData(void) const {return fMustResetFirstSoundingData;}
	void MustResetFirstSoundingData(bool newState) {fMustResetFirstSoundingData = newState;}
	const std::string& SoundingIndexStr(void) const {return itsSoundingIndexStr;}

	bool LeftButtonDown(const NFmiPoint &thePlace, unsigned long theKey);
	bool RightButtonDown(const NFmiPoint &thePlace, unsigned long theKey);
	bool MouseMove(const NFmiPoint &thePlace, unsigned long theKey);
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint);

 private:
	void DrawOneSounding(const NFmiMTATempSystem::SoundingProducer &theProducer, const NFmiMTATempSystem::TempInfo &theTempInfo, int theIndex, double theBrightningFactor, int theModelRunIndex);
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
	void DrawHodograf(NFmiSoundingDataOpt1 &theData, int theIndex);
	void DrawHodografTextWithMarker(const std::string &theText, float u, float v, const NFmiColor &theTextColor, const NFmiColor &theMarkerColor, const NFmiColor &theMarkerFillColor, int theMarkerSizeInPixel, int theFontSize, FmiDirection theTextAlignment, MarkerShape theMarkerShape);
	void DrawHodografUpAndDownWinds(NFmiSoundingDataOpt1 & theData, int theIndex);
	void DrawHodografWindVectorMarkers(NFmiSoundingDataOpt1 & theData, int theIndex);
	void DrawHodografBase(int theIndex);
	void DrawHodografCurve(NFmiSoundingDataOpt1 &theData, int theIndex);
	void DrawHodografHeightMarkers(NFmiSoundingDataOpt1 &theData, int theIndex);
	NFmiPoint GetRelativePointFromHodograf(double u, double v);
	void DrawSoundingInTextFormat(NFmiSoundingDataOpt1 &theData);
	bool FillSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiSoundingDataOpt1 &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation, boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo, const NFmiMTATempSystem::SoundingProducer &theProducer);
	void DrawSounding(NFmiSoundingDataOpt1 &theData, int theIndex, const NFmiColor &theUsedSoundingColor, bool fMainCurve, bool onSouthernHemiSphere);
	void DrawSoundingsInMTAMode(void);
	void DrawBackground(void);
	void DrawStabilityIndexData(void);
	void DrawTextualSoundingData(void);
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
						   const checkedVector<double> &theValues, double startP, double endP, double deltaStartLevelP,
						   NFmiDrawingEnvironment * theEnvi);
	void DrawTemperatures(NFmiSoundingDataOpt1 &theData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo);
    void DrawOneLevelStringData(NFmiText &text, NFmiPoint &p, int levelIndex, std::deque<float> &pVec, std::deque<float> &tVec, std::deque<float> &tdVec, std::deque<float> &zVec, std::deque<float> &wsVec, std::deque<float> &wdVec);
	double GetPAxisChangeValue(double change);
	void DrawLine(const NFmiPoint &p1, const NFmiPoint &p2, bool drawSpecialLines, int theTrueLineWidth, bool startWithXShift, int theHelpDotPixelSize, NFmiDrawingEnvironment * theEnvi);
	void DrawHelpLineLabel(const NFmiPoint &p1, const NFmiPoint &theMoveLabelRelatively, double theValue, const NFmiTempLabelInfo &theLabelInfo, NFmiDrawingEnvironment * theEnvi, const NFmiString &thePostStr = NFmiString(""));
	void DrawWind(NFmiSoundingDataOpt1 &theData, int theIndex, bool onSouthernHemiSphere);
	void DrawStationInfo(NFmiSoundingDataOpt1 &theData, int theIndex);
	void DrawHeightValues(NFmiSoundingDataOpt1 &theData, int theIndex);
	void MoveToNextLine(NFmiPoint &theTextPoint, double factor);
	void DrawNextLineToIndexView(NFmiText &theText, const NFmiString &theStr, NFmiPoint &theTextPoint, double factor, bool moveFirst = true, bool addToString = true);
	void DrawLCL(NFmiSoundingDataOpt1 &theData, int theIndex, FmiLCLCalcType theLCLCalcType);
	void DrawTrMw(NFmiSoundingDataOpt1 &theData, int theIndex);
	NFmiString GetIndexText(double theValue, const NFmiString &theText, int theDecimalCount);
	void InitializeHodografRect(void);
	void DrawAnimationControls(void);
	bool ModifySoundingWinds(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);
    void DrawSecondaryDataRect();
    void DrawSecondaryDataHorizontalAxel(NFmiTempLabelInfo &theLabelInfo, NFmiTempLineInfo &theLineInfo, FmiDirection theLabelTextAlignment, double theYPosition, const NFmiPoint &theLabelOffset);
    void DrawSecondaryData(NFmiSoundingDataOpt1 & theData, int theIndex, const NFmiColor &theUsedSoundingColor);
    void DrawSecondaryData(NFmiSoundingDataOpt1 &theData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo);
    double SecondaryDataFrameXoffset(double theValue);
    void DrawSecondaryVerticalHelpLine(double theBottom, double theTop, double theValue);
    bool FillSoundingDataFromServer(const NFmiMTATempSystem::SoundingProducer &theProducer, NFmiSoundingDataOpt1 &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation);
    Gdiplus::SmoothingMode GetUsedCurveDrawSmoothingMode() const;
    bool IsRectangularTemperatureHelperLines() const;
	bool IsAnyTextualSideViewVisible() const;
	void DrawTextualSideViewRelatedStuff();
	void CalculateAllDataViewRelatedRects();

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

	// first-dataa k�ytet��n my�s luotausten muokkauksessa!!!!
	NFmiSoundingDataOpt1 itsFirstSoundingData; // t�h�n talletetaan 1. piirrett�v�n luotauksen datat, ett� niit� k�ytet��n indeksi osio laskuissa
	bool fMustResetFirstSoundingData; // joskus k�ytt�j� haluaa resetoida muokattua luotausta
	bool fHodografInitialized; // kun ikkuna piirret��n 1. kerran, pit�� laskea alkuarvaus, muutoin ei
	double itsFirstSoundinWindBarbXPos;
	std::string itsSoundingIndexStr; // t�h�n talletetaan ain piirretty stabiilisuus indeksi teksti rimpsu / luotaus numeroina, riippuen mik� moodi on valittuna

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
};


