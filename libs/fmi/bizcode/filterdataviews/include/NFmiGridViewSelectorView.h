// NFmiGridViewSelectorView.h: interface for the NFmiGridViewSelectorView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiZoomView.h"

class NFmiGridViewSelectorView : public NFmiZoomView
{

public:
   const NFmiPoint& SelectedGridViewSize(void){return itsSelectedGridViewSize;};
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   void Draw(NFmiToolBox * theGTB) override;
   NFmiGridViewSelectorView(int theMapViewDescTopIndex, NFmiToolBox * theToolBox
							 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							 ,const NFmiRect& theRect
							 ,int rows
							 ,int columns);
   ~NFmiGridViewSelectorView(){};
   void SetViewGridSize(int rows, int columns);

protected:
   void Draw(void);
   NFmiPoint Relative2GridPoint(const NFmiPoint& theRelativePlace);

   NFmiPoint itsSelectedGridViewSize;
   int itsGridViewRows; 
   int itsGridViewColumns; 
};

