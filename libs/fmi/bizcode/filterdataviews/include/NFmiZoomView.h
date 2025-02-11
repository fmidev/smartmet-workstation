// NFmiZoomView.h: interface for the NFmiZoomView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiCtrlView.h"

class NFmiDibMapHandler;
class NFmiArea;

class NFmiZoomView : public NFmiCtrlView
{

public:
   bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MouseMove (const NFmiPoint & thePlace, unsigned long theKey) override;
   void Draw (NFmiToolBox * theGTB) override;
    NFmiZoomView (int theMapViewDescTopIndex, NFmiToolBox * theToolBox
				 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam);
	NFmiZoomView (int theMapViewDescTopIndex, NFmiToolBox *theToolBox
				  ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
				  ,const NFmiRect& theRect);
   ~NFmiZoomView ();
   boost::shared_ptr<NFmiArea> ZoomedArea (void);
   void Update(void) override;


protected:
   void DrawConstAreaRects (void);
   void DrawZoomedAreaRect (void);

   FmiDirection itsDragTarget;

private:
   void ScaleZoomedAreaRect(double theScaleFactor);
   void AdjustZoomedAreaRect(void);
   NFmiRect CalcHandlersZoomedAreaRect(void);
   void DrawZoomedAreaRect (const NFmiRect & newZoomedRect);
   void DrawInvertRect (const NFmiRect & theRect, const NFmiPoint & thePenSize);
   void CalcZoomedArea (void);
   NFmiRect CalcZoomedAreaRect (void);
   void DrawZoomedAreaRect (const NFmiPoint & thePlace);
   void DrawZoomedAreaRect (const NFmiPoint & thePlace, FmiDirection theDragTarget);
   NFmiRect itsZoomedAreaRect;
   boost::shared_ptr<NFmiArea> itsZoomedArea;
};

