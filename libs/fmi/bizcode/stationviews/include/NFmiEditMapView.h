// NFmiEditMapView.h: interface for the NFmiEditMapView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiCtrlView.h"
#include "NFmiDataIdent.h"
#include "NFmiTimeBag.h"

class NFmiTimeControlView;
class NFmiArea;
class NFmiTimeStationViewRowList;
class NFmiSynopPlotView;
class NFmiStationViewHandler;
class NFmiMilliSecondTimer;

namespace Gdiplus
{
	class GraphicsPath;
}

class NFmiVerticalAnimationInfo
{
public:

	NFmiVerticalAnimationInfo(void)
	:itsLabelWidthInMM(0)
	,itsLabelHeightInMM(0)
	,itsLabelGabInMM(0)
	,itsMapAreaHeightInMM(0)
	,itsRelLabelHeight(0)
	,itsRelLabelGab(0)
	,itsRelMapAreaHeight()
	,itsUsedTimes()
	,itsTimeStampFormat()
	,itsUsedLabelCount(0)
	,itsControlRect()
	,itsControlRectOnMouseMove()
	{
	}
	~NFmiVerticalAnimationInfo(void)
	{
	}

	double itsLabelWidthInMM;
	double itsLabelHeightInMM;
	double itsLabelGabInMM;
	double itsMapAreaHeightInMM;
	double itsRelLabelHeight;
	double itsRelLabelGab;
	double itsRelMapAreaHeight;
	NFmiTimeBag itsUsedTimes;
	NFmiString itsTimeStampFormat;
	int itsUsedLabelCount; // tämä on luultavasti times + 1 koska "Play"-lappu pitää olla myös
	NFmiRect itsControlRect;
	NFmiRect itsControlRectOnMouseMove; // kun karttanäytöllä liikutetaan hiirtä, pitää mouse move (= animaatio) ottaa isommalta alueelta
};

class NFmiEditMapView : public NFmiCtrlView
{

public:
   void Update(void);
   bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
   bool MiddleButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
   bool MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
   bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
   bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
   bool RightButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
   bool MouseMove(const NFmiPoint& thePlace, unsigned long theKey);
   bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);
   bool LeftDoubleClick(const NFmiPoint &, unsigned long); // Marko lisäsi 14.12.2001
   bool RightDoubleClick(const NFmiPoint &, unsigned long);
   void Draw(NFmiToolBox * theGTB);
   void DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4); // tällä piirretään tavara, joka tulee myös bitmapin päälle
   NFmiEditMapView(int theMapViewDescTopIndex
				   ,NFmiToolBox * theToolBox
				   ,NFmiDrawingEnvironment * theDrawingEnvi
				   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   virtual  ~NFmiEditMapView ();
   const NFmiRect& MapRect(void){return itsMapRect;};
   std::string ComposeToolTipText(const NFmiPoint& theRelativePoint);
   void DrawSynopPlotOnToolTip(NFmiToolBox * theToolBox, const NFmiRect &theRect, bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot);
   NFmiStationViewHandler* GetMapViewHandler(int theRowIndex, int theIndex);

private:
   void ActivateMapView(const NFmiPoint & thePlace);
   void MakeLabelPath(Gdiplus::GraphicsPath &theLabelPathOut, NFmiRect &theBaseRectInOut);
   void CalcVerticalAnimationControlRectAndStuff(NFmiVerticalAnimationInfo &theVerticalAnimationInfo);
   void DrawVerticalAnimationControl(void);
   void DrawVerticalAnimationControlInit(void);
   NFmiSynopPlotView* GetSynopPlotViewFromToolTipPos(bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot);
   NFmiString Value2String(float theValue, int theDigitCount, FmiInterpolationMethod theInterpolationMethod, FmiParamType theParamType);
   bool CreateTimeControlView(void);
   void SetViewTimes(void); // käy asettamassa kaikkiin mahdollisiin näyttöihin oikeat ajat
   NFmiString GetToolTipText(void);
   void	PrintNoDataMessageOnTimeControlView(void);
   void UpdateTimeControlView(void);
   void SetMapAreaAndRect (const boost::shared_ptr<NFmiArea> &theArea, const NFmiRect & theRect);
   void UpdateMap (void);
   void LogWarningForTooLongMapViewDrawTime(NFmiMilliSecondTimer &theTimer);
   void DrawLastEditedDataSendTime();
   std::string GetLastSendTimeTooltipText();

   NFmiRect itsTimeControlViewRect;
   NFmiTimeControlView * itsTimeControlView;
   boost::shared_ptr<NFmiArea> itsMapArea;
   NFmiRect itsMapRect;
   boost::shared_ptr<NFmiDrawParam> itsMapDrawParam;
   NFmiTimeStationViewRowList* itsViewGrid;
   bool fMouseMovedInCapture; // tämän avulla voidaan tarkastaa mousebutton up -metodeissa mikä toiminto tehdään
   NFmiVerticalAnimationInfo itsVerticalAnimationInfo;
   NFmiRect itsLastSendTimeTextRect; // Tähän laatikkoon piirretään mahdollinen viimeinen datan lähetys aika teksti. Jos hiiri on tämän laatikon sisällä, laitetaan tooltip siitä
};
