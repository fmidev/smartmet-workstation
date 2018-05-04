//© Ilmatieteenlaitos/Lasse.
//7.11.1994   Lasse
//
//Ver. 28.2.1996/Lasse  conversion from NFmiCloud
//muutettu 15.3.96/LW täysin uusi ulkonäkö
//muutettu 18.3.96/LW ulkonäköä muutettu
//muutettu 13.2.97/LW ulkonäköä muutettu, peruttu, vähän kuitenkin muutettu
//--------------------------------------------------------------------------- narrow.cpp

#include "NFmiArrow.h"
#include "NFmiDrawingEnvironment.h"
               
NFmiArrow::NFmiArrow (float theValue
					 ,const NFmiRect &theRect
                     ,NFmiToolBox *theToolBox
                     ,NFmiView *theEnclosure
                     ,NFmiDrawingEnvironment *theEnvironment
                     ,ShapeIdentifier theIdent) 
         : NFmiSymbol(theRect
                     ,theToolBox
                     ,theEnclosure
                     ,theEnvironment
                     ,theIdent)
		 , itsValue(theValue)
{
}
//-------------------------------------------------------------------------------
void NFmiArrow::Build (void) 
{  
   GetEnvironment()->EnableFill();
   GetEnvironment()->EnableFrame();
   float factor;
   switch(Class())
   {
      case kCalm:
	  default:
	  {
		  factor = 0.f;
		  break;
	  }
	  case kWeak:
	  {
		 factor = 0.f;
         break;
      }
	  case kModerate:
	  {
		 factor = 0.f;
         break;
	  }
	  case kNearGale:
	  {
		 factor = 1.f;
         break;
	  }
	  case kGale:
	  {
		 factor = 1.f;
         break;
	  }
	  case kStorm:
	  case kSevereStorm:
	  {
		 factor = 1.f; 
	     break;
	  }
   }
/*   const int kNumberOfPoints = 39;  ei oiken hyvä vaikka kapeampi
   float x[kNumberOfPoints] = 
        {6.f,5.7f,5.45f,5.4f,5.5f,5.7f,5.3f,4.9f,4.7f,4.5f,4.5f,4.15f,3.75f,3.15f,2.7f,2.5f,2.6f
		   ,2.f,1.65f,2.5f,2.6f,1.f,.6f,0.f,.4f,1.f,2.f,3.f,3.5f,3.95f,3.75f,3.25f,2.7f,2.f,2.6f,3.3f,4.1f,5.f,6.f};
   float y[kNumberOfPoints] = 
            {.1f,.5f,1.1f,1.9f,2.6f,3.4f,3.1f,2.6f,2.2f,1.7f,1.25f,1.5f,2.f,3.f,4.f,5.3f,6.f
		     ,5.55f,5.f,4.5f,4.1f,4.3f,4.5f,5.2f,4.f,3.2f,2.3f,1.45f,1.15f,1.f,.9f,.8f,.8f,.9f,.45f,.2f,.1f,.0f,.15f};
   */
   const int kNumberOfPoints = 34;
   float x[kNumberOfPoints] = 
        {6.f,5.7f,5.45f,5.4f,5.5f,5.7f,5.3f,4.9f,4.7f,4.5f,3.9f,3.35f,2.8f,2.5f,2.6f
		   ,2.f,1.65f,1.6f,1.15f,.6f,0.f,.4f,1.f,1.75f,2.6f,3.75f,3.25f,2.7f,1.2f,2.6f,3.3f,4.1f,5.f,6.f};
   float y[kNumberOfPoints] = 
            {.1f,.5f,1.1f,1.9f,2.6f,3.4f,3.1f,2.6f,2.2f,1.7f,2.3f,3.05f,4.05f,5.3f,6.f
		     ,5.55f,5.f,4.85f,4.8f,4.85f,5.1f,4.f,3.15f,2.35f,1.6f,.9f,.8f,.8f,1.f,.2f,.1f,.05f,.0f,.0f};
   float xNew[kNumberOfPoints];
   float yNew[kNumberOfPoints];
   if (factor > 0.f)
   {
      for(int ind=0; ind < kNumberOfPoints; ind++)
	  {
		  xNew[ind] = /*1.f - */((3.f-x[ind])*factor + 3.f) / 6.f;
		  yNew[ind] = ((3.f-y[ind])*1.f + 3.f) / 6.f;
	  }
      BuildPolyLine(xNew, yNew, kNumberOfPoints, GetEnvironment());
   }
}
//-------------------------------------------------------------------------------
FmiWindClass NFmiArrow::Class(void)
{
	if(itsValue < 1.)
		return kCalm;
	else if(itsValue < 4.1)
		return kWeak;
	else if(itsValue < 7.1)
		return kModerate;
	else if(itsValue < 14.1)
		return kNearGale;
	else if(itsValue < 20.9)
		return kGale;
	else if(itsValue < 31.9)
		return kStorm;
	else 
		return kSevereStorm;
}
               
