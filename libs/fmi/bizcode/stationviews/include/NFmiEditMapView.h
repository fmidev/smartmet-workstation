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
	int itsUsedLabelCount; // t�m� on luultavasti times + 1 koska "Play"-lappu pit�� olla my�s
	NFmiRect itsControlRect;
	NFmiRect itsControlRectOnMouseMove; // kun karttan�yt�ll� liikutetaan hiirt�, pit�� mouse move (= animaatio) ottaa isommalta alueelta
};

class NFmiEditMapView : public NFmiCtrlView
{

public:
   void Update(void) override;
   bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MiddleButtonDown(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonDown(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MouseMove(const NFmiPoint& thePlace, unsigned long theKey) override;
   bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
   bool LeftDoubleClick(const NFmiPoint &, unsigned long) override; // Marko lis�si 14.12.2001
   bool RightDoubleClick(const NFmiPoint &, unsigned long) override;
   void Draw(NFmiToolBox * theGTB) override;
   void DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4); // t�ll� piirret��n tavara, joka tulee my�s bitmapin p��lle
   NFmiEditMapView(int theMapViewDescTopIndex
				   ,NFmiToolBox * theToolBox
				   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   ~NFmiEditMapView ();
   const NFmiRect& MapRect(void){return itsMapRect;};
   std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
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
   void SetViewTimes(void); // k�y asettamassa kaikkiin mahdollisiin n�ytt�ihin oikeat ajat
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
   bool fMouseMovedInCapture; // t�m�n avulla voidaan tarkastaa mousebutton up -metodeissa mik� toiminto tehd��n
   NFmiVerticalAnimationInfo itsVerticalAnimationInfo;
   NFmiRect itsLastSendTimeTextRect; // T�h�n laatikkoon piirret��n mahdollinen viimeinen datan l�hetys aika teksti. Jos hiiri on t�m�n laatikon sis�ll�, laitetaan tooltip siit�
};
