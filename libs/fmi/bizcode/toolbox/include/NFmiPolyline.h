//© Ilmatieteenlaitos/Persa.        fpolylin.h     ver. 4.11.1994
//13.9.1994   Persa
//
//Ver. 26.10.1994

//Muutos 4.11.1994     Lisatty SwapX jaSwapY metodit
//Ver. 25.10.1999 /Marko Lis‰sin konstruktorin.
//Ver. 19.01.2000 /Marko Lis‰sin muutaman piirto option t‰nne, koska en keksinyt miten ne saisi parhaiten drawenviin.

//--------------------------------------------------------------------------- NFmiPolyline.h

#pragma once

#include "NFmiShape.h"

//_________________________________________________________ NFmiPolyline
class NFmiPolyline : public NFmiShape
{
 public:

	NFmiPolyline(const NFmiPoint& theTopLeftCorner
			   ,const NFmiPoint& theBottomRightCorner
			   ,const NFmiView *theEnclosure = 0
			   ,NFmiDrawingEnvironment *theEnvironment = 0
			   ,int theBackGroundMode = 1 // opaque
			   ,int theHatchMode = -1);  // -1 tarkoittaa, ettei k‰ytet‰ kuvioita
	NFmiPolyline(const NFmiRect& theRect
			   ,const NFmiView *theEnclosure = 0
			   ,NFmiDrawingEnvironment *theEnvironment = 0
			   ,int theBackGroundMode = 1 // opaque
			   ,int theHatchMode = -1);  // -1 tarkoittaa, ettei k‰ytet‰ kuvioita


						   virtual ~NFmiPolyline(void);
    NFmiVoidPtrList *GetPoints (void) const {return itsPoints;};
                    void AddPoint(const NFmiPoint &newPoint);
//            virtual void SwapX(void); 
//            virtual void SwapY(void); 

   int BackGroundMode(void) const {return itsBackGroundMode;};
   void BackGroundMode(int newMode){itsBackGroundMode = newMode;};
   int HatchMode(void){return itsHatchMode;};
   void HatchMode(int newMode){itsHatchMode = newMode;};
   bool UseHatch(void){return fUseHatch;};
   void UseHatch(bool newStatus){fUseHatch = newStatus;};

 private:
   NFmiVoidPtrList *itsPoints;
   int itsBackGroundMode; // opaque (oletus) tai transparent
   int itsHatchMode; // millainen kuvio pensseliin, jos k‰ytet‰‰n
   bool fUseHatch; // oletus ei k‰yt‰ kuvioita
};

typedef NFmiPolyline* PNFmiPolyline;

