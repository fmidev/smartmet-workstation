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
    std::vector<float> pressures;
    std::vector<NFmiPoint> latlons;
    std::vector<NFmiMetTime> times;
	std::string macroParamErrorMessage;
};

class NFmiCrossSectionView : public NFmiIsoLineView
{
 public:
	 // En yrit‰ nyt v‰hent‰‰ turhia parametreja
	 // joita emo isolineview vaatii, joten
	 // niit‰ voi mietti‰ v‰hent‰‰ myˆhemmin.
	NFmiCrossSectionView(NFmiToolBox * theToolBox
                        , int viewGridRowNumber
                        , int viewGridColumnNumber);
    ~NFmiCrossSectionView(void);

	void Draw (NFmiToolBox *theGTB) override;
    bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey) override;
    bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
    bool MouseMove(const NFmiPoint& thePlace, unsigned long theKey) override;
    bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
	bool LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) override;
    bool IsMouseDraggingOn(void) override;
    void Update(void) override;
	void DrawOverBitmapThings(NFmiToolBox *theGTB, const NFmiPoint &thePlace);
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;

	void SetDrawSizeFactors(double theFactorX, double theFactorY)
	{
		itsDrawSizeFactorX = theFactorX;
		itsDrawSizeFactorY = theFactorY;
	}
	double CalcDataRectPressureScaleRatio(void);
	NFmiRect CalcPressureScaleRect(void);
	NFmiRect CalcParamHandlerViewRect(void);
	void SetParamHandlerViewRect(const NFmiRect& newRect);
	const NFmiRect& GetFrameForParamBox(void) const override { return itsDataViewFrame; };
	void UpdateCachedParameterName() override;

 protected:
   bool IsToolMasterAvailable(void) override;
   bool DeleteTransparencyBitmap() override;
   bool IsMapViewCase() override;
   NFmiTimeBag GetUsedTimeBagForDataCalculations(void);
   float GetLevelValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, float P, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, bool doMetaParamCheck = true);
   float GetLevelValueForMetaParam(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, float P, const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
   void DrawLegends();
   std::string ComposeTrajectoryToolTipText();
private:
	const NFmiMetTime& CurrentTime(void);
	void UpdateParamHandlerView(void);
	bool ShowParamHandlerView(void);
	void DrawParamView(NFmiToolBox *theGTB);
	void InitParamHandlerView(void);
	NFmiTimeBag CalcHelperLineTimeBag(void);
	boost::shared_ptr<NFmiFastQueryInfo> GetFirstHybridInfo(void);
    void StorePressureScaleLimits(void);
	void GetStartAndEndTimes(NFmiMetTime &theStartTimeOut, NFmiMetTime &theEndTimeOut);
	void DrawHelperTimeLines(void);
	std::vector<NFmiPoint> MakeLatlonVector(void);
    std::vector<NFmiMetTime> MakeTimeVector(void);
    std::vector<NFmiMetTime> MakeMacroParamTimeModeTimeVector(void);
    void PreCalculateTrajectoryLatlonPoints(void);
	void DrawTrajectory(const NFmiTrajectory &theTrajectory, const NFmiColor &theColor);
	void DrawSingleTrajector(const NFmiSingleTrajector &theSingleTrajector, NFmiDrawingEnvironment &theEnvi, int theTimeStepInMinutes, int theTimeMarkerPixelSize, int theTimeMarkerPixelPenSize, FmiDirection theDirection);
	const std::vector<NFmiPoint>& GetMinorPoints(void);
	NFmiMetTime GetCrossSectionTime(const NFmiPoint &theRelativePlace);
	void FillMainPointXYInfo(NFmiDataMatrix<NFmiPoint> &theCoordinates);
	NFmiPoint GetCrossSectionLatlonPoint(const NFmiPoint &theRelativePlace);
	std::string GetSelectedParamInfoString(NFmiFastQueryInfo *theInfo);
	bool ChangePressureScale(FmiDirection theDir, bool fChangeUpperAxis, bool ctrlKeyDown);
	void DrawActivatedMinorPointLine(void);
	boost::shared_ptr<NFmiArea> GetZoomedArea(void);
	void DrawCrossSection(void);
	void DrawCrosssectionWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, Imagine::NFmiDataHints &theHelper, NFmiDataMatrix<NFmiPoint> &theCoordinates);
	void DrawCrosssectionWithToolMaster(NFmiIsoLineData& theIsoLineData);
	void FillCrossSectionMacroParamData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures, CrossSectionTooltipData *possibleTooltipData = nullptr, NFmiExtraMacroParamData* possibleExtraMacroParamData = nullptr);
	void FillTrajectoryCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures);
	void FillObsAndForCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures);
	int FillObsPartOfTimeCrossSectionData(NFmiDataMatrix<float>& theValues, NFmiIsoLineData& theIsoLineData, std::vector<float>& thePressures);
	void FillRouteCrossSectionData(NFmiDataMatrix<float>& theValues, NFmiIsoLineData& theIsoLineData, std::vector<float>& thePressures);
	void FillRouteCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures,
		const std::vector<NFmiPoint>& theLatlonPoints,
		const std::vector<NFmiMetTime>& thePointTimes);
    bool FillRouteCrossSectionDataForMetaWindParam(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures, unsigned long wantedParamId, bool doUserDrawData,
		const std::vector<NFmiPoint>& theLatlonPoints,
		const std::vector<NFmiMetTime>& thePointTimes);
    void FillTimeCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures);
    bool FillTimeCrossSectionDataForMetaWindParam(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures, unsigned int theStartTimeIndex, unsigned long wantedParamId, bool doUserDrawData);
	void FillCrossSectionData(NFmiDataMatrix<float>& theValues, NFmiIsoLineData& theIsoLineData, std::vector<float>& thePressures);
	void FillCrossSectionUserDrawData(NFmiIsoLineData& theIsoLineData);
	void FillTimeCrossSectionUserDrawData(NFmiIsoLineData &theIsoLineData);
	void FillRouteCrossSectionUserDrawData(NFmiIsoLineData& theIsoLineData,
		const std::vector<NFmiPoint>& theLatlonPoints,
		const std::vector<NFmiMetTime>& thePointTimes);
	NFmiDataMatrix<float> MakeCrossSectionUserDrawValueData(NFmiIsoLineData& theIsoLineData);
	NFmiDataMatrix<float> MakeTimeCrossSectionUserDrawValueData(NFmiIsoLineData& theIsoLineData);
	NFmiDataMatrix<float> MakeRouteCrossSectionUserDrawValueData(NFmiIsoLineData& theIsoLineData,
		const std::vector<NFmiPoint>& theLatlonPoints,
		const std::vector<NFmiMetTime>& thePointTimes);
	bool IsUserDrawDataNeeded(NFmiFastQueryInfo & usedInfo);
	NFmiDataMatrix<NFmiPoint> CalcRelativeCoordinatesFromPressureMatrix(const NFmiDataMatrix<float>& pressureValues) const;
	bool FillCrossSectionDataForMetaWindParam(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures, unsigned long wantedParamId, bool doUserDrawData);
	void FillXYMatrix(NFmiIsoLineData &theIsoLineData, NFmiDataMatrix<NFmiPoint> &theCoordinates, std::vector<float> &thePressures);
	void CalculateViewRects(void);
	NFmiRect CalcDataViewRect(void);
	void DrawPressureScale(void);
	std::vector<float> MakePressureVector(int usedCount, int normalCount);
	int GetNearestCrossSectionColumn(const NFmiPoint &thePlace);
	void DrawGridPoints(NFmiDataMatrix<NFmiPoint> &theCoordinates);
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
	std::vector<float> CalcCurrentLevelPressures(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	int CalcHorizontalPointCount(void);
	double Column2x(int theColumn);

	double p2y(double p) const;
	double p2y(const NFmiRect &usedDataRect, double p) const;
	double y2p(double y) const;

	double itsLowerEndOfPressureAxis;
	double itsUpperEndOfPressureAxis;
	NFmiRect itsDataViewFrame; // t‰lle alueelle piirret‰‰n itse data
	NFmiRect itsPressureScaleFrame; // t‰lle alueelle piirret‰‰n log(p) asteikko

	// tooltippia varten laitetaan talteen n‰m‰ viimeksi piirretyty parametri ja paine matriisit
	std::vector<float> itsPressures;

	std::vector<float> itsModelGroundPressures; // maanpinnan korkeus pit‰‰ laskea jokaiseen poikileikkaus pisteeseen
	std::vector<float> itsGroundHeights; // maanpinnan korkeus metreiss‰ pit‰‰ laskea jokaiseen poikileikkaus pisteeseen
	std::vector<float> itsRoutePointsDistToEndPoint; // reitti tapauksessa lasketaan tah‰n jokaisen poikkileikkauspisteen
													   // kumulatiivinen (ei linnuntiet‰, jos mutkainen poikkileikkaus)
													   // et‰isyys alkupisteest‰ metrein‰.
													   // Viimeisen pisteen arvo on siis koko reitin pituus.

	int itsCrossSectionIsoLineDrawIndex; // iso viivojen piirto poikkileikkausikkunaan laittaa labelit p‰‰llekk‰in.
										// t‰m‰n avulla ohjataan labeleita hieman syrjemm‰ksi toisistaan

	NFmiPoint itsPressureScaleFontSize;

	std::vector<NFmiPoint> itsTrajectoryLatlonPoints; // t‰h‰n talletetaan trajektori pisteiden latlon pisteet
	NFmiRect itsTrajectoryDataRect; // trajektori datan toolmaster piirrossa t‰m‰ pit‰‰ laskea ja k‰ytt‰‰
	double itsViewWidthInMM;
	double itsViewHeightInMM;
	int itsFirstForecastTimeIndex;
	std::vector<NFmiMetTime> itsObsForModeFoundObsTimes; // n‰m‰ ajat merkit‰‰n poikkileikaus n‰yttˆˆn, ett‰ n‰hd‰‰n miss‰ kohtaa on ollut havaittuja luotauksia

	double itsDrawSizeFactorX; // ruutu piirrossa 1, lasketaan printatessa t‰lle erillinen arvo
	double itsDrawSizeFactorY;

	std::unique_ptr<NFmiParamHandlerView> itsParamHandlerView;
	NFmiRect itsParamHandlerViewRect;
	bool fDoCrossSectionDifferenceData;
};

