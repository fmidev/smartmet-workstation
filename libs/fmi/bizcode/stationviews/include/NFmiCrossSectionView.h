//**********************************************************
// C++ Class Name : NFmiCrossSectionView
// ---------------------------------------------------------
// Filetype: (HEADER)
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : metedit nfmiviews
//  - GD View Type      : Class Diagram
//  - GD View Name      : Class Diagram
// ---------------------------------------------------
//  Author         : pietarin
//
//  Description:
//   Piirt‰‰ halutun poikkileikkauksen n‰ytˆlle halutulle
//   datalle halutuilla piirto-ominaisuuksilla.
//   Periytyy isolineview:sta, koska se osaa piirt‰‰ hilan isoviivoina.
//   HUOM! DrawParamin sijasta pit‰‰ hallussaan DrawParamList:i‰, jossa
//   kaikki kyseisen ruudun piirrett‰v‰t parametrit.
//
//  Change Log:
//
//**********************************************************

#pragma once

#include "NFmiIsoLineView.h"
#include "NFmiTimeBag.h"

class NFmiAxisView;
class NFmiAxis;
class NFmiTimeControlView;
class NFmiDrawParamList;
class NFmiTrajectory;
class NFmiSingleTrajector;
class NFmiParamHandlerView;

class CrossSectionTooltipData
{
public:
    CrossSectionTooltipData()
        :values(1, 1)
        ,pressures(1, kFloatMissing)
        ,latlons(1, NFmiPoint::gMissingLatlon)
        ,times(1, NFmiMetTime::gMissingTime)
    {}

    NFmiDataMatrix<float> values;
    checkedVector<float> pressures;
    checkedVector<NFmiPoint> latlons;
    checkedVector<NFmiMetTime> times;
};

class NFmiCrossSectionView : public NFmiIsoLineView
{
 public:
	 // En yrit‰ nyt v‰hent‰‰ turhia parametreja
	 // joita emo isolineview vaatii, joten
	 // niit‰ voi mietti‰ v‰hent‰‰ myˆhemmin.
	NFmiCrossSectionView(NFmiToolBox * theToolBox
						,NFmiDrawingEnvironment * theDrawingEnvi
                        , int viewGridRowNumber
                        , int viewGridColumnNumber);
    virtual  ~NFmiCrossSectionView(void);

	void Draw (NFmiToolBox *theGTB);
    bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
    bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
	bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
    bool MouseMove(const NFmiPoint& thePlace, unsigned long theKey);
    bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);
	bool LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey);
    bool IsMouseDraggingOn(void);
    void Update(void);
	void DrawOverBitmapThings(NFmiToolBox *theGTB, const NFmiPoint &thePlace);
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint);

	void SetDrawSizeFactors(double theFactorX, double theFactorY)
	{
		itsDrawSizeFactorX = theFactorX;
		itsDrawSizeFactorY = theFactorY;
	}
	double CalcDataRectPressureScaleRatio(void);
	NFmiRect CalcPressureScaleRect(void);

 protected:
   bool IsToolMasterAvailable(void) override;
   bool DeleteTransparencyBitmap() override;
   NFmiTimeBag GetUsedTimeBagForDataCalculations(void);
   void SetupUsedDrawParam(void);
   float GetLevelValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, float P, const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
 private:
	const NFmiMetTime& CurrentTime(void);
	void UpdateParamHandler(void);
	bool ShowParamHandlerView(void);
	void DrawParamView(NFmiToolBox *theGTB);
	NFmiRect CalcParamHandlerViewRect(void);
	void InitParamHandlerView(void);
	NFmiTimeBag CalcHelperLineTimeBag(void);
	NFmiRect CalcHeaderRect(void);
	boost::shared_ptr<NFmiFastQueryInfo> GetFirstHybridInfo(void);
    void StorePressureScaleLimits(void);
	void GetStartAndEndTimes(NFmiMetTime &theStartTimeOut, NFmiMetTime &theEndTimeOut);
	void DrawHelperTimeLines(void);
	checkedVector<NFmiPoint> MakeLatlonVector(void);
    checkedVector<NFmiMetTime> MakeTimeVector(void);
    checkedVector<NFmiMetTime> MakeMacroParamTimeModeTimeVector(void);
    void PreCalculateTrajectoryLatlonPoints(void);
	void DrawTrajectory(const NFmiTrajectory &theTrajectory, const NFmiColor &theColor);
	void DrawSingleTrajector(const NFmiSingleTrajector &theSingleTrajector, NFmiDrawingEnvironment *theEnvi, int theTimeStepInMinutes, int theTimeMarkerPixelSize, int theTimeMarkerPixelPenSize, FmiDirection theDirection);
	const checkedVector<NFmiPoint>& GetMinorPoints(void);
	NFmiMetTime GetCrossSectionTime(const NFmiPoint &theRelativePlace);
	void FillMainPointXYInfo(NFmiDataMatrix<NFmiPoint> &theCoordinates);
	NFmiPoint GetCrossSectionLatlonPoint(const NFmiPoint &theRelativePlace);
	std::string GetSelectedParamInfoString(NFmiFastQueryInfo *theInfo);
	bool FillIsoLineDataForToolMaster(const NFmiDataMatrix<float> &theValues, NFmiIsoLineData& theIsoLineData);
	bool ChangePressureScale(FmiDirection theDir, bool fChangeUpperAxis);
	void DrawActivatedMinorPointLine(void);
	void DrawHeader(void);
	boost::shared_ptr<NFmiArea> GetZoomedArea(void);
	void DrawCrossSection(void);
	void DrawCrosssectionWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, Imagine::NFmiDataHints &theHelper, NFmiDataMatrix<NFmiPoint> &theCoordinates);
	void DrawCrosssectionWithToolMaster(NFmiIsoLineData& theIsoLineData);
	void FillCrossSectionMacroParamData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, checkedVector<float> &thePressures, CrossSectionTooltipData *possibleTooltipData = nullptr);
	void FillTrajectoryCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, checkedVector<float> &thePressures);
	void FillObsAndForCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, checkedVector<float> &thePressures);
	void FillRouteCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, checkedVector<float> &thePressures);
    void FillRouteCrossSectionDataForMetaWindVector(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, checkedVector<float> &thePressures);
    void FillTimeCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, checkedVector<float> &thePressures);
    void FillTimeCrossSectionDataForMetaWindVector(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, checkedVector<float> &thePressures, unsigned int theStartTimeIndex = 0);
    void FillCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, checkedVector<float> &thePressures);
    void FillCrossSectionDataForMetaWindVector(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, checkedVector<float> &thePressures);
	void FillXYMatrix(NFmiIsoLineData &theIsoLineData, NFmiDataMatrix<NFmiPoint> &theCoordinates, checkedVector<float> &thePressures);
	void CalculateViewRects(void);
	NFmiRect CalcDataViewRect(void);
	void DrawPressureScale(void);
	checkedVector<float> MakePressureVector(int usedCount, int normalCount);
	int GetNearestCrossSectionColumn(const NFmiPoint &thePlace);
	void MakeMultiLineToolTip(const NFmiString& theStr, double theFontSize, NFmiDrawingEnvironment *theEnvi, double theTooltipTextXOffset, const NFmiPoint &thePoint);
	NFmiDrawingEnvironment GetToolTipEnvironment(void);
	NFmiRect CalcToolTipRect(checkedVector<std::basic_string<char> > &theStrVector, int theFontSize, double& theLineHeigth, const NFmiPoint &thePoint);
	void DrawGridPoints(NFmiDataMatrix<NFmiPoint> &theCoordinates);
	void AddFooterTextForCurrentData(bool fLastOne);
	void DrawCrosssectionWindVectors(NFmiIsoLineData& theIsoLineData, const NFmiDataMatrix<NFmiPoint> &theXYCoordinates);
	void DrawWindVector(float theValue, const NFmiRect &theSymbolRect);
	void DrawFlightLevelScale(void);
	void DrawHeightScale(void);
	void DrawObsForModeTimeLine(void);
	void DrawGround(void);
	void DrawGroundLevel(NFmiDrawingEnvironment &theEnvi);
	void DrawSeaLevel(void);
	bool DrawModelGroundLevel(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDrawingEnvironment &theEnvi, bool fUseHybridCalculations);
	void CalcModelGroundPressures(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	void CalcModelPressuresAtStation(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	void CalcGroundHeights(void);
	void CalcRouteDistances(void);
	void DrawHybridLevels(void);
	checkedVector<float> CalcCurrentLevelPressures(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	int CalcHorizontalPointCount(void);
	double Column2x(int theColumn);

	double p2y(double p);
	double y2p(double y);

	double itsLowerEndOfPressureAxis;
	double itsUpperEndOfPressureAxis;
	NFmiRect itsDataViewFrame; // t‰lle alueelle piirret‰‰n itse data
	NFmiRect itsPressureScaleFrame; // t‰lle alueelle piirret‰‰n log(p) asteikko

	// tooltippia varten laitetaan talteen n‰m‰ viimeksi piirretyty parametri ja paine matriisit
	checkedVector<float> itsPressures;

	checkedVector<float> itsModelGroundPressures; // maanpinnan korkeus pit‰‰ laskea jokaiseen poikileikkaus pisteeseen
	checkedVector<float> itsGroundHeights; // maanpinnan korkeus metreiss‰ pit‰‰ laskea jokaiseen poikileikkaus pisteeseen
	checkedVector<float> itsRoutePointsDistToEndPoint; // reitti tapauksessa lasketaan tah‰n jokaisen poikkileikkauspisteen
													   // kumulatiivinen (ei linnuntiet‰, jos mutkainen poikkileikkaus)
													   // et‰isyys alkupisteest‰ metrein‰.
													   // Viimeisen pisteen arvo on siis koko reitin pituus.

	NFmiString itsHeaderParamString; // t‰h‰n ker‰t‰‰n parametreihin liittyv‰ tieto yhteen pˆtkˆˆn, jotka lopuksi tulostetaan otsikko riville
	int itsCrossSectionIsoLineDrawIndex; // iso viivojen piirto poikkileikkausikkunaan laittaa labelit p‰‰llekk‰in.
										// t‰m‰n avulla ohjataan labeleita hieman syrjemm‰ksi toisistaan

	NFmiPoint itsPressureScaleFontSize;

	checkedVector<NFmiPoint> itsTrajectoryLatlonPoints; // t‰h‰n talletetaan trajektori pisteiden latlon pisteet
	NFmiRect itsTrajectoryDataRect; // trajektori datan toolmaster piirrossa t‰m‰ pit‰‰ laskea ja k‰ytt‰‰
	double itsViewWidthInMM;
	double itsViewHeightInMM;
	int itsFirstForecastTimeIndex;
	checkedVector<NFmiMetTime> itsObsForModeFoundObsTimes; // n‰m‰ ajat merkit‰‰n poikkileikaus n‰yttˆˆn, ett‰ n‰hd‰‰n miss‰ kohtaa on ollut havaittuja luotauksia

	double itsDrawSizeFactorX; // ruutu piirrossa 1, lasketaan printatessa t‰lle erillinen arvo
	double itsDrawSizeFactorY;

	NFmiParamHandlerView* itsParamHandlerView;
	NFmiRect itsParamHandlerViewRect;
	bool fDoCrossSectionDifferenceData;
};

