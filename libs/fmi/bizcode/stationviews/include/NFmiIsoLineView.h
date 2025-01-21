//**********************************************************
// C++ Class Name : NFmiIsoLineView
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/ver104/ProjApp/zeditmap2/nfmiviews/NFmiIsoLineView.h
//
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
//  Creation Date  : Mon - Nov 16, 1998
//
//
//  Description:
//   Draws the given grid-data in isoline form.
//   Only lines are drawn, not iso values.
//
//
//  Change Log:
//
//**********************************************************

#pragma once

#include "NFmiStationView.h"
#include "stdafx.h"
#include "matrix3d.h"
#include "NFmiPath.h"
#include "ColorContouringData.h"
#include <list>
#include <mutex>


typedef unsigned long       DWORD;
typedef DWORD   COLORREF;

class NFmiGridBase;
class NFmiValueLineList;
class NFmiIsoLineData;
class NFmiHatchingSettings;
class CBitmap;
class CDC;
class NFmiPolyline;

namespace SmartMetDataUtilities
{
    class DataUtilitiesSingleton;
    template<typename A = DataUtilitiesSingleton>
    class DataUtilitiesAdapter;
}

namespace Imagine
{
	class NFmiDataHints;
	class NFmiImage;
}

bool ColorTableInitialized(void); // laitoin esittelyn globaalista funktiosta tähän

struct LabelingInfo
{
	LabelingInfo(void)
	:itsStrategy(2)
	,itsFontSize(5.f)
	,itsFontColor(NFmiColor(0, 0, 0))
	,itsFontOpacity(1)
	,itsBoxFillColor(NFmiColor(1, 1, 0))
	,itsBoxFillOpacity(1)
	,itsBoxStrokeColor(NFmiColor(0, 0, 0))
	,itsBoxStrokeOpacity(1)
	,itsBoxStrokeWidth(1)
	,itsIsoLineLabelHeight(5.f) //EL
	{
	}

	int itsStrategy; // 0 = ei labeleita, 1 = simppeli, 2 = simppeli, muutta ei päällekkäisiä labeleita (jos ei löydy paikkaa, ei labelia)
	float itsFontSize;
	NFmiColor itsFontColor;
	float itsFontOpacity; // arvo 0-1, 0 ei näy ollenkaan ja 1 täysin läpinäkymätön
	NFmiColor itsBoxFillColor;
	float itsBoxFillOpacity; // arvo 0-1, 0 ei näy ollenkaan ja 1 täysin läpinäkymätön
	NFmiColor itsBoxStrokeColor;
	float itsBoxStrokeOpacity; // arvo 0-1, 0 ei näy ollenkaan ja 1 täysin läpinäkymätön
	float itsBoxStrokeWidth;
	float itsIsoLineLabelHeight;  //EL
};

class LabelBox
{

public:

		LabelBox(void);
		LabelBox(float theFontHeight, float theIsoLineValue /*int theLabelLetterCount*/, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiToolBox *theToolBox, NFmiDrawingEnvironment &theEnviroment);

		void Init(void);
		void Init(float theFontHeight, float theIsoLineValue, /*int theLabelLetterCount*/ boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiToolBox *theToolBox, NFmiDrawingEnvironment &theEnviroment);

		const NFmiRect& LabelBoxRect(void) const;

		void Center(const NFmiPoint &theLabelCenterPoint);
		NFmiPoint Center(void);

		void IsoLineValue(float theIsoLineValue) {itsIsoLineValue = theIsoLineValue;};
		float IsoLineValue(void) {return itsIsoLineValue;};

		const std::string& LabelString(void);
		double CalcWidthInPixels(void);
		double CalcHeightInPixels(void);

		const NFmiColor& BoxFillColor(void) {return itsBoxFillColor; };
		const NFmiColor& BoxStrokeColor(void) {return itsBoxStrokeColor;};
		const NFmiColor& FontColor(void) {return itsFontColor;};
		int Strategy(void) {return itsStrategy;};
		static void InitPixelPerMMRatio(NFmiToolBox * theGTB);


protected:

		double CalcFontWidthInPixels(void);
		double CalcFontHeightInPixels(void);
		double CalcRelativeFontWidth(NFmiToolBox *theToolBox);
		double CalcRelativeFontHeight(NFmiToolBox *theToolBox);
		void CalcLabelBoxRect(/*int theLabelLetterCount,*/ NFmiToolBox *theToolBox);
		void CalcLabelString(void);

		int itsStrategy; // 0 = ei labeleita, 1 = simppeli, 2 = simppeli, muutta ei päällekkäisiä labeleita (jos ei löydy paikkaa, ei labelia)
		float itsFontSize;
		NFmiColor itsFontColor;
		float itsFontOpacity; // arvo 0-1, 0 ei näy ollenkaan ja 1 täysin läpinäkymätön
		NFmiColor itsBoxFillColor;
		float itsBoxFillOpacity; // arvo 0-1, 0 ei näy ollenkaan ja 1 täysin läpinäkymätön
		NFmiColor itsBoxStrokeColor;
		float itsBoxStrokeOpacity; // arvo 0-1, 0 ei näy ollenkaan ja 1 täysin läpinäkymätön
		float itsBoxStrokeWidth;
		float itsIsoLineLabelHeight;  //EL

		NFmiRect itsLabelBoxRect;
		float itsIsoLineValue;
		std::string itsLabelString;

		static double itsPixelsPerMM_x;
		static double itsPixelsPerMM_y;

};


class NFmiIsoLineView : public NFmiStationView
{

 public:
   NFmiIsoLineView (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
				   ,NFmiToolBox * theToolBox
				   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
				   ,FmiParameterName theParamId
				   ,NFmiPoint theOffSet
				   ,NFmiPoint theSize
                   ,int viewGridRowNumber
                   ,int viewGridColumnNumber);

   ~NFmiIsoLineView (void);

   void Draw (NFmiToolBox *theGTB) override;
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;

   static NFmiPolyline* CreateEmptyPolyLine(const NFmiRect &theRect, NFmiDrawingEnvironment *theEnvi);
   static void ConvertPath2PolyLineList(Imagine::NFmiPath& thePath, std::list<NFmiPolyline*> &thePolyLineList, bool relative_moves, bool removeghostlines, const NFmiRect &theRect, NFmiDrawingEnvironment *theEnvi);
   static bool DifferentWorldViews(const NFmiArea *area1, const NFmiArea * area2);

 protected:
    virtual bool IsToolMasterAvailable(void);
	virtual bool DeleteTransparencyBitmap();
	virtual bool IsMapViewCase();
	bool FillIsoLineVisualizationInfo(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData, bool fToolMasterUsed, bool fStationData);
	void FillIsoLineInfoSimple(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData, bool fToolMasterUsed, bool fStationData);
	void FillIsoLineInfoSimple_new(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData, bool fStationData);
	void FillSimpleColorContourInfo(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData, bool fStationData, bool fToolMasterUsed);
	void FillSimpleColorContourInfo_new(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData);
    void FillCustomColorContourInfo(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData, bool fStationData, bool fToolMasterUsed);
	void FillCustomColorContourInfo_new(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData);
	void FillBaseColorContourInfo_new(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData, bool fStationData, bool fToolMasterUsed);
	void FillIsoLineInfoCustom(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData, bool fToolMasterUsed, bool fStationData);
    void FillHatchInfo(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData);
	void PrepareForTransparentDraw(void);
	void EndTransparentDraw(void);
    bool FillGridRelatedData(NFmiIsoLineData &isoLineData, NFmiRect &zoomedAreaRect);
	bool FillGridRelatedData_IsDataVisible();
	bool FillGridRelatedData_VisualizationOptimizationChecks(NFmiIsoLineData& isoLineData, NFmiRect& zoomedAreaRect, bool& fillGridDataStatus);
	bool FillGridRelatedData_BetterVisualizationChecks(NFmiIsoLineData& isoLineData, NFmiRect& zoomedAreaRect, bool& fillGridDataStatus);
	bool FillGridRelatedData_ZoomingChecks(NFmiIsoLineData& isoLineData, NFmiRect& zoomedAreaRect, bool& fillGridDataStatus);
	void FillGridRelatedData_NormalDataCase(NFmiIsoLineData& isoLineData, NFmiRect& zoomedAreaRect, bool &fillGridDataStatus);
    void DoGridRelatedVisualization(NFmiIsoLineData &isoLineData, NFmiRect &zoomedAreaRect);
	ContouringJobData MakeContouringJobData(boost::shared_ptr<NFmiDrawParam>& theDrawParam);
	bool IsIsoLinesDrawnWithImagine(void);
	bool FillIsoLineDataWithGridData(NFmiIsoLineData& theIsoLineData, int x1, int y1, int x2, int y2, NFmiGrid *optimizedDataGrid = nullptr);

    bool initializeIsoLineData(NFmiIsoLineData &theIsoLineData);

    void DrawIsoLinesWithImagine(void);
	void DrawSimpleIsoLinesWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet);
	void DrawCustomIsoLinesWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet);
	void DrawSimpleColorContourWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet);
	void DrawCustomColorContourWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet);
	void DrawHatchesWithImagine(NFmiIsoLineData& theIsoLineData, const NFmiHatchingSettings& theHatchSettings, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet);
	void SetUpDifferenceDrawing(boost::shared_ptr<NFmiDrawParam> &theUsedDrawParam); // näitä on kutsuttava pareittain RestoreUpDifferenceDrawing-metodin kanssa
	void RestoreUpDifferenceDrawing(boost::shared_ptr<NFmiDrawParam> &theUsedDrawParam); // näitä on kutsuttava pareittain SetUpDifferenceDrawing-metodin kanssa
    CRect GetTrueClientRect();
	void DoPossibleIsolineSafetyFeatureDownSizing(NFmiIsoLineData* theIsoLineDataInOut, const NFmiRect& zoomedAreaRect);
private:
	bool IsZoomingPossible(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiArea> &theMapArea, NFmiRect & theCroppedXyRectOut, int &theLeftIndex, int &theRightIndex, int &theTopIndex, int &theBottomIndex);
	void DrawIsoLinesWithToolMaster(void);
	void DrawIsoLines(void);

protected:
	bool fDataOk; // false, jos pitäisi piirtää tasainen kenttä (min = max)
	NFmiDataMatrix<float> itsIsolineValues; // tähän kerätään isoviivoja varten dataa, tämän avulla tehdään myös hatch-laskut ToolMaster-piirrossa

	CBitmap *itsTransparencyDrawBitmap;
	CBitmap *itsTransparencyDrawOldBitmap;
	CDC *itsTransparencyDrawDC;
	CDC *itsTransparencyDrawBackupDC; // tähän talletetaan väliaikaisesti oikea DC, kun piirto suoritetaan väliaikaisesti tranparentti DC:lle
	CSize itsLastBitmapSize;
	bool fDrawUsingTransparency;
    std::unique_ptr<SmartMetDataUtilities::DataUtilitiesAdapter<>> dataUtilitiesAdapter;
    // Preventing to calling drawing related ToolMaster functions simultaneously from different places
    static std::mutex sToolMasterOperationMutex;

	// label funktiot
	bool AreRectsOverLapping(const NFmiRect &first, const NFmiRect &second);
	void DrawAllLabels(const NFmiPoint &theOffSet);
	bool MoveLabelBoxToBestPossiblePlace(const Imagine::NFmiPathData &thePath, const NFmiRect &theRelativeRect, LabelBox &theLabelBox /*, const std::string &theLabelStr*/);
	bool LabelDontOvelapWithOthers(LabelBox &theLabelBoxObj); // EL
	void AddLabelsToDrawList(Imagine::NFmiPath &thePath, float theIsoLineValue, const NFmiRect &theRelativeRect, LabelBox &theLabelBox, const NFmiPoint &theOffSet);
	void AddLabelsToDrawList(std::list<Imagine::NFmiPathData*> &thePathDataList, float theIsoLineValue, const NFmiRect &theRelativeRect, LabelBox &theLabelBox, const NFmiPoint &theOffSet);
	void AddLabelToSingleIsoLine(const Imagine::NFmiPathData &thePath, float theIsoLineValue, const NFmiRect &theRelativeRect, LabelBox &theLabelBox /*const LabelingInfo &theLabelInfo*/);
	void StoreLabel(LabelBox &theLabelBox /*, const std::string &theLabelString*/); //EL


	// label dataa
/*/EL*/	std::vector<LabelBox> itsExistingLabels; // laitetaan kaikki käytetyt label paikat talteen tarkistuksia varten (taas globaali pikaviritys)

};

