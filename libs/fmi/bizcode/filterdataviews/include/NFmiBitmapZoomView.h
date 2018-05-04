// NFmiBitmapZoomView.h: interface for the NFmiBitmapZoomView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiZoomView.h"

class NFmiBitmapZoomView : public NFmiZoomView
{

public:
   virtual void Draw (NFmiToolBox * theGTB);
    NFmiBitmapZoomView (NFmiToolBox * theToolBox
						 ,NFmiDrawingEnvironment * theDrawingEnvi
						 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   virtual  ~NFmiBitmapZoomView();

};

