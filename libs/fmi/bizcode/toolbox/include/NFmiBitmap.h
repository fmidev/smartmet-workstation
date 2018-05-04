//© Ilmatieteenlaitos/Lasse.
//Original 26.9.1996
//
//10.12.97/EL added ShortPixMap()

#pragma once

#include "NFmiShape.h"

class NFmiColorPool;
//_________________________________________________________ NFmiBitMap
class NFmiBitmap : public NFmiShape
{
 public:

     NFmiBitmap(const NFmiPoint& theStartingPoint
             ,const NFmiPoint& theEndingPoint
			 ,NFmiColorPool *theBitmap
             ,const NFmiView *theEnclosure = 0
             ,NFmiDrawingEnvironment *theEnvironment = 0); 

 public:
	 NFmiColorPool *ColorGrid(void) const {return itsColorGrid;};
	 short* ShortPixMap(int theColorRes = 24);

 private:
	 NFmiColorPool *itsColorGrid;
};

typedef NFmiBitmap* PNFmiBitmap;


