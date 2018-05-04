//© Ilmatieteenlaitos/Lasse.
//13.5.1997   Lasse
//
//210597/LW pyˆristys korjattu
//----------------------------------------------------------------------------nsimweat.cpp

#include "NFmiWindBarb.h"
#include "NFmiDrawingEnvironment.h"
#include "NFmiToolBox.h"

#include <cmath>
               
//____________________________________________________________________ 
NFmiWindBarb::NFmiWindBarb(float theSpeed
									  ,float theDirection
                                                ,const NFmiRect &theRect
                                                ,NFmiToolBox *theToolBox
                                                ,bool onSouthernHemiSphere
												,double theBarbLength
												,double theFlagLength
                                                ,NFmiView *theEnclosure
                                                ,NFmiDrawingEnvironment *theEnvironment
                                                ,ShapeIdentifier theIdent) 
                        : NFmiSymbol(theRect
                                    ,theToolBox
                                    ,theEnclosure
                                    ,theEnvironment
                                    ,theIdent)
                         , itsSpeed(theSpeed)
                         , itsDirection(theDirection)
						 , itsBarbLength(theBarbLength)
						 , itsFlagLength(theFlagLength)
                         , fOnSouthernHemiSphere(onSouthernHemiSphere)
{
}

NFmiPoint NFmiWindBarb::ConvertFromThisToToolBoxWorld(const NFmiPoint &thePoint)
{
	const NFmiRect &rec = GetFrame();
	NFmiPoint p(thePoint.X() * rec.Width() + rec.Left(),
				thePoint.Y() * rec.Height() + rec.Top());
	return p;
}

//____________________________________________________________________ Build
void NFmiWindBarb::Build (void)  
{
   // voisi viel‰ tehd‰ suunnan puuttuvalla nopeudella

   float x[4];
   float y[4];

   double xCenter = .5f;
   double yCenter = .5f;
//   GetEnvironment()->DisableFill();
   GetEnvironment()->EnableFrame();


	double halfSide = .015; 
	// vaihdoin asema pisteen piirron laatikosta fillattuun ympyr‰‰n
   double width = GetToolBox()->SX(3);
   double height = GetToolBox()->SY(3);
   GetEnvironment()->EnableFill();
   NFmiRect rec1(0, 0, width, height);
   rec1.Center(ConvertFromThisToToolBoxWorld(NFmiPoint(xCenter, yCenter)));
	GetToolBox()->DrawEllipse(rec1, GetEnvironment());
   GetEnvironment()->DisableFill();

	if(itsSpeed == kFloatMissing || itsDirection == kFloatMissing)
		return;


//	itsSpeed += 85.f;  //****
	if(itsSpeed <= 0.) //210597 oli 2.5
	{
 		halfSide = .06;
		x[0] = float(xCenter - halfSide);
		y[0] = float(yCenter - halfSide);
		x[1] = float(xCenter + halfSide);
		y[1] = float(yCenter - halfSide);
		x[2] = float(xCenter + halfSide);
		y[2] = float(yCenter + halfSide);
		x[3] = float(xCenter - halfSide);
		y[3] = float(yCenter + halfSide);
        BuildPolyLine(x, y, 4, GetEnvironment());
		return;
	}
	
	double barbLength = itsBarbLength;
	double flagLength = itsFlagLength;
    double flagDistance = .05f + (barbLength * 0.085);	// Mill‰ et‰isyydell‰ v‰k‰set ovat toisistaan
	double triangleBase = .15f;
	float roundedSpeed = itsSpeed + 1.25f; //210597
	double fac = .017453;//(2.f * 3.1416f) / 360;
	double radDir = itsDirection * fac;
	double radDirFlag =	fOnSouthernHemiSphere ? ((itsDirection - 70.f) * fac) : ((itsDirection + 70.f) * fac);

	short number25 = static_cast<short>(FmiMin(5.1,roundedSpeed/25.)); //cut too many
	short number5 = static_cast<short>((roundedSpeed-static_cast<float>(number25*25))/5.f);
	short number2 = static_cast<short>((roundedSpeed-static_cast<float>(number25*25)-static_cast<float>(number5*5))/2.5f);


// base barb
	x[0] = static_cast<float>(xCenter);
	y[0] = static_cast<float>(yCenter);

	x[1] = static_cast<float>(barbLength * sin(radDir)+xCenter);
	y[1] = static_cast<float>(-barbLength * cos(radDir)+yCenter);
    BuildPolyLine(x, y, 2, GetEnvironment());

	float xLast = static_cast<float>((barbLength-flagDistance) * sin(radDir)+xCenter);
	float yLast = static_cast<float>(-(barbLength-flagDistance) * cos(radDir)+yCenter);

// short flag
	if(number2 > 0)
	{
	    x[0] = xLast;
	    y[0] = yLast;
	    x[1] = static_cast<float>(.37f*flagLength * sin(radDirFlag)+x[0]);
	    y[1] = static_cast<float>(-.37f*flagLength * cos(radDirFlag)+y[0]);
        BuildPolyLine(x, y, 2, GetEnvironment());
	}
 
// long flags
// HUOM Polyline yhdist‰‰ alku- ja loppupisteen, pit‰‰ vet‰‰ kaksi erillist‰ viivaa  
	int i;
	for(i=1; i<=number5; i++)
	{
	   x[0] = xLast;
	   y[0] = yLast;
	   x[1] = static_cast<float>(flagDistance * sin(radDir)+xLast);
	   y[1] = static_cast<float>(-flagDistance * cos(radDir)+yLast);
	   if(i>1)
		   BuildPolyLine(x, y, 2, GetEnvironment());
	   xLast = x[1];
	   yLast = y[1];
	   x[0] = xLast;
	   y[0] = yLast;
	   x[1] = static_cast<float>(flagLength * sin(radDirFlag)+xLast);
	   y[1] = static_cast<float>(-flagLength * cos(radDirFlag)+yLast);
	   BuildPolyLine(x, y, 2, GetEnvironment());
	}

// triangles

 	GetEnvironment()->EnableFill();
    GetEnvironment()->EnableFrame();

	for(i=1; i<=number25; i++)
	{
	   if(i==1)
	   {
	      x[0] = xLast;
	      y[0] = yLast;
	      x[1] = static_cast<float>(flagDistance * sin(radDir)+xLast);
	      y[1] = static_cast<float>(-flagDistance * cos(radDir)+yLast);
          BuildPolyLine(x, y, 2, GetEnvironment());
	      xLast = x[1];
	      yLast = y[1];
	   }
	   x[0] = xLast;
	   y[0] = yLast;
	   x[1] = static_cast<float>(flagLength * sin(radDirFlag)+xLast);
	   y[1] = static_cast<float>(-flagLength * cos(radDirFlag)+yLast);
	   x[2] = static_cast<float>(triangleBase * sin(radDir)+xLast);
	   y[2] = static_cast<float>(-triangleBase * cos(radDir)+yLast);
       BuildPolyLine(x, y, 3, GetEnvironment());
	   xLast = x[2];
	   yLast = y[2];
	}
}
