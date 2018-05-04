//140397/LW itsValue protected-osastoon

#pragma once

#include "NFmiSymbol.h"

#ifdef _MSC_VER
#pragma warning(disable : 4512) // poistaa VC++ k‰‰nt‰j‰n C4512 "assignment operator could not be generated" varoituksen
#endif

typedef enum
        {kNoPrecipitation,
         kWeakRain,
         kModerateRain,
         kHeavyRain,
         kWeakSnow,
         kModerateSnow,
         kHeavySnow,
         kWeakSleet,
         kModerateSleet,
         kHeavySleet,
         kLightThunder,
         kHeavyThunder,
         kUnknownPrecipitation} PrecipitationType;
typedef enum
        {kNoClouds,
         kHalfCloudy,
         kOvercast,
         kUnknownCloudiness} CloudinessType;

//_________________________________________________________ NFmiSimpleWeatherSymbol
class NFmiSimpleWeatherSymbol : public NFmiSymbol
{
 public:

   NFmiSimpleWeatherSymbol(short theValue
                          ,const NFmiRect &theRect
                          ,NFmiToolBox *theToolBox
                          ,NFmiView *theEnclosure = 0
                          ,NFmiDrawingEnvironment *theEnvironment = 0
                          ,ShapeIdentifier theIdent = kSymbol); 

    virtual void Build(void);
     
 protected:
 
       virtual CloudinessType RankCloudiness (void); 
    virtual PrecipitationType RankPrecipitation (void); 
               unsigned short GetWeatherSpecifier (void); 
                         void BuildCloudiness (void);
                         void BuildPrecipitation (void); 
/**
                   void BuildSun (const NFmiPoint &thePlace,
                                  const NFmiPoint &theSize,
                                  NFmiDrawingEnvironment *theEnvironment = 0);
                   void BuildCloud (const NFmiPoint &thePlace,
                                    const NFmiPoint &theSize,
                                    NFmiDrawingEnvironment *theEnvironment = 0); 
                   void BuildFlash (const NFmiPoint &thePlace,
                                    const NFmiPoint &theSize,
                                    NFmiDrawingEnvironment *theEnvironment = 0); 
**/
   short itsValue; //140397 oli privaatti
 private:

};
typedef NFmiSimpleWeatherSymbol* PNFmiSimpleWeatherSymbol;




//____________________________________________________________________ RankCloudiness
inline CloudinessType NFmiSimpleWeatherSymbol::RankCloudiness (void) 
{
  switch (itsValue)
  {
    case 1:
      return kNoClouds;
    case 2:
    case 21:
    case 22:
    case 23:
    case 41:
    case 42:
    case 43:
    case 61:
    case 62:
    case 71:
    case 72:
    case 73:
      return kHalfCloudy;
    case 3:
    case 31:
    case 32:
    case 33:
    case 51:
    case 52:
    case 53:
    case 63:
    case 64:
    case 81:
    case 82:
    case 83:
      return kOvercast;
    default:
      return kUnknownCloudiness;
  }
}
//____________________________________________________________________ RankCloudiness
inline PrecipitationType NFmiSimpleWeatherSymbol::RankPrecipitation (void) 
{
  
  switch (itsValue)
  {
    case 1:
    case 2:
    case 3:
      return kNoPrecipitation;
    case 21:
    case 31:
      return kWeakRain;
    case 22:
    case 32:
      return kModerateRain;
    case 23:
    case 33:
      return kHeavyRain;
    case 41:
    case 51:
      return kWeakSnow;
    case 42:
    case 52:
      return kModerateSnow;
    case 43:
    case 53:
      return kHeavySnow;
    case 61:
    case 63:
      return kLightThunder;
    case 62:
    case 64:
      return kHeavyThunder;
    case 71:
    case 81:
		return kWeakSleet;
    case 72:
    case 82:
		return kModerateSleet;
    case 73:
    case 83:
		return kHeavySleet;
    default:
      return kUnknownPrecipitation;
  }
}

