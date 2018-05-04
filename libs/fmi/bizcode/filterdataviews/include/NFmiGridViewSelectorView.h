// NFmiGridViewSelectorView.h: interface for the NFmiGridViewSelectorView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiZoomView.h"

class NFmiGridViewSelectorView : public NFmiZoomView
{

public:
   const NFmiPoint& SelectedGridViewSize(void){return itsSelectedGridViewSize;};
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
   void Draw(NFmiToolBox * theGTB);
   NFmiGridViewSelectorView(int theMapViewDescTopIndex, NFmiToolBox * theToolBox
							 ,NFmiDrawingEnvironment * theDrawingEnvi
							 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							 ,const NFmiRect& theRect
							 ,int rows
							 ,int columns);
   virtual  ~NFmiGridViewSelectorView(){};
   void SetViewGridSize(int rows, int columns);

protected:
   void Draw(void);
   NFmiPoint Relative2GridPoint(const NFmiPoint& theRelativePlace);

   NFmiPoint itsSelectedGridViewSize;
   int itsGridViewRows; 
   int itsGridViewColumns; 
};

