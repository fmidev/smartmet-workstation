//**********************************************************
// C++ Class Name : NFmiStationViewHandler
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiStationViewHandler.h
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 3
//  - GD View Type      : Class Diagram
//  - GD View Name      : ruudukkon‰yttˆ
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Fri - Sep 10, 1999
//
//
//  Description:
//   T‰m‰ luokka pit‰‰ sis‰ll‰‰n kartan piirron
//   ja stationviewlistin joita se piirt‰‰.
//   T‰t‰ luokkaa on tarkoitus k‰ytt‰‰ editorin
//   'ruudukko'-n‰ytˆn palasena. N‰ist‰ ruudunpalasista
//   kootaan 'rivit' NFmiTimeStationViwRow-luokkaan ja
//   ruudukon muodostaa NFmiTimeStationViwRowList.
//   T‰t‰ viimeista pit‰‰
//   taas editorin n‰yttˆluokka omanaan.
//
//  Change Log:
//
//**********************************************************

#pragma once

#include "stdafx.h"
#include "NFmiCtrlView.h"
#include "NFmiWindTableSystem.h"
#include "HakeMessage\Main.h"
#include <gdiplus.h>

class NFmiArea;
class NFmiCtrlViewList;
class NFmiStationView;
class NFmiDataIdent;
class NFmiFastQueryInfo;
class NFmiTrajectory;
class NFmiSingleTrajector;
class NFmiParamHandlerView;
class NFmiSilamStationList;
class NFmiACLocationInfo;
class NFmiColor;
class CDC;
class NFmiBetaProduct;
class NFmiColorContourLegendValues;
class NFmiColorRectSettings;

namespace Gdiplus
{
	class Bitmap;
	class PointF;
    class Font;
    class StringFormat;
    class Rect;
}

class LegendDrawingMeasures
{
public:
    NFmiPoint backgroundRectSizeInPixels = NFmiPoint(0, 0);
    NFmiPoint colorRectSizeInPixels = NFmiPoint(0, 0);
    double maxValueStringLengthInPixels = 0;
    double paddingLengthInPixels = 0;
    double usedFontSizeInPixels = 0;
    double usedFontSizeInMM = 0;
    Gdiplus::Rect backgroundRectInPixels;
};

class NFmiStationViewHandler : public NFmiCtrlView
{

 public:
	NFmiStationViewHandler(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
						   ,NFmiToolBox * theToolBox
						   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
						   ,int theRowIndex
						   ,int theColumnIndex);
	~NFmiStationViewHandler();
	void Draw(NFmiToolBox* theGTB) override;
    void Update(void) override;
	bool LeftButtonDown(const NFmiPoint& thePlace, unsigned long theKey) override;
	bool LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey) override;
	bool MiddleButtonDown(const NFmiPoint & thePlace, unsigned long theKey) override;
	bool MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey) override;
	bool RightButtonDown (const NFmiPoint &thePlace, unsigned long theKey) override;
	bool MouseMove (const NFmiPoint &thePlace, unsigned long theKey) override;
	bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
	bool LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) override;
	bool RightDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) override;
    bool IsMouseDraggingOn(void) override;
    void Time(const NFmiMetTime& theTime) override;
	void DrawSelectedLocations(void);
	void StoreToolTipDataInDoc(const NFmiPoint& theRelativePlace);
	void DrawTimeText(void);

	void DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4);// t‰ll‰ piirret‰‰n tavara, joka tulee myˆs bitmapin p‰‰lle
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
	NFmiCtrlView* GetView(const NFmiDataIdent &theDataIdent, bool fUseParamIdOnly);
	void DrawParamView(NFmiToolBox * theGTB);
	boost::shared_ptr<NFmiArea> GetArea() const override;
	void SetArea(const boost::shared_ptr<NFmiArea>& theArea) override;
	NFmiRect CalcParamHandlerViewRect();
	void SetParamHandlerViewRect(const NFmiRect &newRect);

 private:
	void DoTotalLocationSelection(const NFmiPoint & thePlace, const NFmiPoint &theLatlon, unsigned long theKey, bool fDrawSelectionOnThisView);
	NFmiPoint GetViewSizeInPixels(void);
	NFmiPoint CalcRelativeWarningIconSize(Gdiplus::Bitmap *theImage);
	NFmiRect CalcSymbolRelativeRect(const NFmiPoint &theLatlon, Gdiplus::Bitmap *theImage, double theSizeFactor = 1.0);
	void DrawWarningIcon(const NFmiPoint &theLatlon, Gdiplus::Bitmap *theImage, float theAlpha, double theSizeFactor = 1.0);
#ifndef DISABLE_CPPRESTSDK
    void DrawWantedWarningIcon(const HakeMessage::HakeMsg &theWarningMessage, bool isHakeMessage);
    void DrawHakeMessageIcon(const HakeMessage::HakeMsg &theWarningMessage, const NFmiPoint &latlon);
    void DrawKaHaMessageIcon(const HakeMessage::HakeMsg &theWarningMessage, const NFmiPoint &latlon);
#endif // DISABLE_CPPRESTSDK

	void InitializeWarningSymbolFiles(void);
	void InitializeWarningSymbols(void);
	bool CheckBoundingBox(NFmiRect &theBoundBox, std::vector<NFmiRect> &theAutoCompletionRects, double relativeX, double relativeY, double relativeW, double radius, double angle, FmiDirection &theMarkerConnectingPlace);
	NFmiPoint CalcNewCenterPoint(double relativeX, double relativeY, double relativeW, double radius, double angle);
	NFmiRect SearchNameBoxLocation(const NFmiPoint &theRelativePoint, const NFmiRect &theRelativeBoundingBox, std::vector<NFmiRect> &theAutoCompletionRects, double theOneLineBoxHeight, FmiDirection &theMarkerConnectingPlace);
	NFmiRect CalcBaseMarkerRect(double theMarkerSizeInMM);
	void DrawMarkerPoint(const NFmiPoint &theRelativePlace, const NFmiRect &theTextRect, std::vector<NFmiRect> &theAutoCompletionRects, FmiDirection theMarkerConnectingPlace, NFmiRect &theMarkerCircleBase);
	void DrawAutocompleteLocations(void);
	void DrawAutocompleteLocation(Gdiplus::Graphics *theGdiPlusGraphics, const NFmiACLocationInfo &theLocInfo, NFmiRect &theMarkerCircleBase);
	void DrawAreaMask(Gdiplus::Graphics &theGdiPlusGraphics, NFmiWindTableSystem::AreaMaskData &theAreaMaskData);
	void DrawWindTableAreas(void);
	void DrawSilamStationMarkers(NFmiSilamStationList &theStationList, NFmiDrawingEnvironment &theEnvi, const NFmiString &theSynopStr, double symbolXShift, double symbolYShift, NFmiRect &thePlaceRect);
	bool ShowParamHandlerView(void);
	void InitParamHandlerView(void);
	void UpdateParamHandlerView(void);
	bool DrawTimeTextInThisMapViewTile(void);
	bool MouseMoveControlPointAction(const NFmiPoint &thePlace);
	bool MouseMoveBrushAction(const NFmiPoint &thePlace);
	bool MouseDragZooming(const NFmiPoint &thePlace);
	bool ShowWarningMessages(void);
	std::string ComposeWarningMessageToolTipText(void);
	std::string ComposeSilamLocationsToolTipText(void);
	std::string ComposeMapLayerToolTipText(bool beforeDataIsDrawnCase);
	void DrawMouseCursorHelperCrossHair(void);
	void DrawSelectedSynopFromGridView(void);
	bool ChangeHybridDataLevel(NFmiStationView* theView, short theDelta);
	bool ChangeSatelDataChannel(NFmiStationView* theView, short theDelta);
	void SelectLocations(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint& theLatLon, int theSelectionCombineFunction, unsigned long theMask, bool fMakeMTAModeAdd, bool fDoOnlyMTAModeAdd = false);
	void DrawCrossSectionPoints(void);
	NFmiPoint LatLonToViewPoint(const NFmiPoint& theLatLon) const override;
	NFmiPoint ViewPointToLatLon(const NFmiPoint& theViewPoint) const override;
	void DrawCurrentFrame(NFmiToolBox* theGTB);
	void DrawSoundingPlaces(void);
	void DrawSoundingSymbols(boost::shared_ptr<NFmiFastQueryInfo> &theSoundingInfo, int theUsedSymbol, double theSymbolSizeInMM);
    bool IsSoundingMarkersDrawnOnThisMap(bool fDrawThisOnEveryRow);
	void DrawMovingSoundingSymbols(boost::shared_ptr<NFmiFastQueryInfo> &theSoundingInfo, int theUsedSymbol, double theSymbolSizeInMM);
	void DrawSelectedMTAModeSoundingPlaces(void);
	void DrawHALYMessageMarkers(void);
	void DrawSilamStationMarkers(void);
	void DrawTrajectories(void);
	void DrawTrajectory(const NFmiTrajectory &theTrajectory, const NFmiColor &theColor);
	void DrawSingleTrajector(const NFmiSingleTrajector &theSingleTrajector, NFmiDrawingEnvironment &theEnvi, int theTimeStepInMinutes, int theTimeMarkerPixelSize, int theTimeMarkerPixelPenSize, FmiDirection theDirection);
	NFmiPoint MakeSoundingMarkerScale(NFmiToolBox *theToolBox, double theSymbolSizeInMM);
	void DrawCPCropArea(void);
	NFmiRect CalcCPCropAreasRelativeRect(const boost::shared_ptr<NFmiArea> &theArea);
    bool UseDrawingCache();

	NFmiPoint CalcFontSize(double theWantedSizeInMM);
	double MM2PixelSize(double theWantedMM);
	void DrawProjetionLines(NFmiToolBox * theGTB);
	void DrawControlPointData(void);
	void DrawControlPoints(void);
	void DoBrushingUndoRituals(boost::shared_ptr<NFmiDrawParam> &theDrawParam);
	void SetViewListArea(void);
	NFmiStationView * CreateStationView(boost::shared_ptr<NFmiDrawParam> &theDrawParam);
	void SetMapAreaAndRect(const boost::shared_ptr<NFmiArea> &theArea, const NFmiRect& theRect);
	void DrawMap(NFmiToolBox* theGTB, const NFmiRect& theRect);
	void DrawOverMap(NFmiToolBox* theGTB, const NFmiRect& theRect);
	void DrawData(NFmiToolBox* theGTB); // Piirt‰‰ datan ruutuun
    void DrawLegends(NFmiToolBox* theGTB);
    void DrawWmsLegends(NFmiToolBox* theGTB);
	void DrawMapInMouseMove(NFmiToolBox * theGTB, const NFmiRect& theRect);
	void DrawMasksOnMap(NFmiToolBox* theGTB);
    void DoBasicDrawing(NFmiToolBox * theGTB, const NFmiRect &theMapFrame);
    void DoCacheDrawing(NFmiToolBox * theGTB, const NFmiRect &theMapFrame, CDC *destinationDc, int cacheRowIndex);
    int CalcUsedCacheRowIndex() const;
    void GetShownMessages();
#ifndef DISABLE_CPPRESTSDK
    void ShowWarningMessages(const std::vector<HakeMessage::HakeMsg> &messages, bool isHakeMessage);
    void GetMinimumDistanceMessage(const std::vector<HakeMessage::HakeMsg> &messages, const NFmiLocation &wantedLocation, HakeMessage::HakeMsg &minimumDistanceMessage, double &minimumDistance);
#endif // DISABLE_CPPRESTSDK
    void TraceLogValidTimeAndAbsoluteRow();
    void MakeParamLevelChangeDirtyOperations(bool changesHappened);
    bool IsMouseCursorOverParameterBox(const NFmiPoint & theMouseCursorPlace);
    void UpdateOnlyThisMapViewAtNextGeneralViewUpdate();
    bool IsBrushToolUsed();
    void LeftButtonUpBrushToolActions();
    bool IsControlPointModeOn();
    void LeftButtonUpControlPointModeActions(const NFmiPoint& thePlace, unsigned long theKey);
    bool AllowCrossSectionPointManipulations();
    void LeftButtonDownCrossSectionActions(const NFmiPoint& thePlace, unsigned long theKey);
    bool LeftButtonUpCrossSectionActions(const NFmiPoint& thePlace, unsigned long theKey);
    void MouseMoveCrossSectionActions(const NFmiPoint& thePlace, unsigned long theKey);
    bool DrawContourLegendOnThisMapRow();
	bool HasSeparateCountryBorderLayer() const;
	void SetThisAsActiveViewRow() const;
	bool IsThisActiveViewRow() const;
	void DrawMapViewRangeMeterData();
	std::vector<NFmiPoint> ConvertLatlonToRelativePoints(const std::vector<NFmiPoint>& latlonPoints);
	bool IsCrossSectionViewStuffShownOnThisMapView() const;
	bool IsCrossSectionSystemDisableingNormalMiddleMouseButtonUse() const;
	bool IsRangeMeterModeOn(bool checkAlsoIfMouseDragIsOn) const;
	void SetRangeMeterDragStart(const NFmiPoint& thePlace);
	void SetRangeMeterDragEnd(const NFmiPoint& thePlace, bool mouseDragging);
	void MoveRangeMeterStart(const NFmiPoint& thePlace);
	float GetUsedTimeResolutionInHours();
	bool HandleTimeBoxMouseWheel(const NFmiPoint& thePlace, unsigned long theKey, short theDelta);

    template<typename T>
    bool MakeParamHandlerViewActions(T action)
    {
        // Ensin pit‰‰ suorittaa toiminto, ja ottaa status talteen
        auto status = action();
        // Vasta sitten laitetaan optimointi likauslippu p‰‰lle. Ainakin yhdess‰ action funktiossa (double-click) tehd‰‰n 
        // ruudun p‰ivitys v‰liss‰ jonka j‰lkeen pit‰‰ tehd‰ viel‰ lopuksi toinen ruudun p‰ivitys, kun t‰‰lt‰ palataan.
        UpdateOnlyThisMapViewAtNextGeneralViewUpdate();
        // Palautetaan status lopuksi
        return status;
    }

    boost::shared_ptr<NFmiArea> itsMapArea;
	NFmiRect itsMapRect;
	NFmiCtrlViewList * itsViewList;
	boost::shared_ptr<NFmiDrawParam> itsMapDrawParam; // t‰m‰ on feikki, mik‰ annetaan ctrllist:alle
	NFmiPoint itsZoomDragDownPoint; // kun tehd‰‰n middlemouse dragia eli vedet‰‰n zoomi laatikkoa, t‰ss‰ on sen alku ja loppu pisteet
	NFmiPoint itsZoomDragUpPoint;
	NFmiRect itsOldZoomRect; // t‰ll‰ piirret‰‰n vanha laatikko pois (k‰‰nteis v‰ri kikka)
	NFmiPoint itsLastMouseDownRelPlace; //yrit‰n t‰ll‰ selvitt‰‰ miksi mouse klick tekee joskus kaksikin turhaa mousemovea

	std::unique_ptr<NFmiParamHandlerView> itsParamHandlerView;
	NFmiRect itsParamHandlerViewRect;
	bool fWholeCrossSectionReallyMoved; 
    // T‰h‰n talletet‰‰n autocompletionissa piirrettyjen nimi laatikkojen ja piste makkerien laatikot. 
    // Kun etsit‰‰n paikkaa seuraavalle laatikoille, ne eiv‰t saa menn‰ p‰‰llekk‰in.
    std::vector<NFmiRect> itsAutoCompletionRects;
	NFmiRect itsTimeBoxRelativeRect;


#ifndef DISABLE_CPPRESTSDK
    std::vector<HakeMessage::HakeMsg> itsShownHakeMessages;
    std::vector<HakeMessage::KahaMsg> itsShownKaHaMessages;
#endif // DISABLE_CPPRESTSDK
};

