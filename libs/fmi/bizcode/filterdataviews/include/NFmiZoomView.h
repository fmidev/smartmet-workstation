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
   bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey);
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
   bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
   bool MouseMove (const NFmiPoint & thePlace, unsigned long theKey);
   void Draw (NFmiToolBox * theGTB);
    NFmiZoomView (int theMapViewDescTopIndex, NFmiToolBox * theToolBox
				 ,NFmiDrawingEnvironment * theDrawingEnvi
				 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam);
	NFmiZoomView (int theMapViewDescTopIndex, NFmiToolBox *theToolBox
				  ,NFmiDrawingEnvironment* theDrawingEnvi
				  ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
				  ,const NFmiRect& theRect);
   virtual  ~NFmiZoomView ();
   boost::shared_ptr<NFmiArea> ZoomedArea (void);
   void Update(void);


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

