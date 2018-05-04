//© Ilmatieteenlaitos/Lasse.
//25.9.1996   Lasse
//27.9.96/LW
//031096/LW itsPixels: NFmiGrid->unsigned char
//081096/LW itsBitMap=NFmiGrid
//10.12.97/EL added ShortPixMap()
//---------------------------------------------------------------

#include "NFmiBitmap.h"
#include "NFmiColorPool.h"
#include <algorithm>

NFmiBitmap::NFmiBitmap (const NFmiPoint& theTopLeftCorner
                             ,const NFmiPoint& theBottomRightCorner
                             ,NFmiColorPool *theColors
							 ,const NFmiView *theEnclosure
                             ,NFmiDrawingEnvironment *theEnvironment
							 ) 
             : NFmiShape(NFmiRect(theTopLeftCorner, theBottomRightCorner)
                        ,theEnclosure
                        ,theEnvironment
                        ,kBitmap) //27.9.96/LW
			 ,itsColorGrid(theColors)
{
//itsBitMap=theBitMap;
}
               
short* NFmiBitmap::ShortPixMap(int theColorRes)
{
	 int xres =  itsColorGrid->XNumber();
	 int yres =  itsColorGrid->YNumber();
	 short* pixMap = new short[xres*yres];

	 long ind = 0;
	 itsColorGrid->First();

	 while(itsColorGrid->Next())
		{
		if (theColorRes == 16 || theColorRes == 24 || theColorRes == 0)	//240197/LW + 15.7.1997/Marko
		  pixMap[ind] = static_cast<short>(
			  std::max(0,
					   static_cast<int>((itsColorGrid->BValue())/255.*31.) +
					   static_cast<int>((itsColorGrid->GValue())/255.*31.) * 32 +
					   static_cast<int>((itsColorGrid->RValue())/255.*31.) * 1024));
		// 5R+6G+5B vaikka manuaali v‰itt‰‰ muuta
		else // theColorRes == 8
		  pixMap[ind] = static_cast<short>(
			std::max(0,
					 static_cast<int>((itsColorGrid->BValue())/255.*3.) +
					 static_cast<int>((itsColorGrid->GValue())/255.*7.) * 4 +
					 static_cast<int>((itsColorGrid->RValue())/255.*3.) * 32));

				   // olisiko 2R+3G+2B 
		ind++;
		}

	 return pixMap;
}
