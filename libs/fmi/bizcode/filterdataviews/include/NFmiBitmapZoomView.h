// NFmiBitmapZoomView.h: interface for the NFmiBitmapZoomView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiZoomView.h"

class NFmiBitmapZoomView : public NFmiZoomView
{

public:
   void Draw (NFmiToolBox * theGTB) override;
   NFmiBitmapZoomView (NFmiToolBox * theToolBox
						 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   ~NFmiBitmapZoomView();

};

