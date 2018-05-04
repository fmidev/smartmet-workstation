//© Ilmatieteenlaitos/Lasse.
//14.3.1997   
//
//		conversion from TFmiSimpleWeatherSymbol
//----------------------------------------------------------------------------nsimweat.cpp


#include "NFmiSynopWeatherSymbol.h"
//____________________________________________________________________ 
NFmiSynopWeatherSymbol::NFmiSynopWeatherSymbol(short theValue
												,short theCloudValue
                                                ,const NFmiRect &theRect
                                                ,NFmiToolBox *theToolBox
                                                ,NFmiView *theEnclosure
                                                ,NFmiDrawingEnvironment *theEnvironment
                                                ,ShapeIdentifier theIdent) 
                        : NFmiSimpleWeatherSymbol(theValue
                                                       ,theRect
                                                       ,theToolBox
                                                       ,theEnclosure
                                                       ,theEnvironment
									                   ,theIdent)
                         , itsCloudValue(theCloudValue)
{
}
//____________________________________________________________________ RankCloudiness
/*inline*/ CloudinessType NFmiSynopWeatherSymbol::RankCloudiness (void) 
{
  switch (itsCloudValue)
  {
    case 0:
    case 1:
    case 2:
      return kNoClouds;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
       return kHalfCloudy;
    case 8:
      return kOvercast;
    default:
      return kUnknownCloudiness;
  }
}
//____________________________________________________________________ RankCloudiness
/*inline*/ PrecipitationType NFmiSynopWeatherSymbol::RankPrecipitation (void) 
{
  
  switch (itsValue)
  {
    case 50:
    case 51:
    case 52:
    case 53:
    case 56:
    case 58:
    case 60:
    case 61:
    case 66:
    case 80:
       return kWeakRain;
    case 54:
    case 55:
    case 57:
    case 59:
    case 62:
    case 63:
    case 67:
    case 81:
       return kModerateRain;
    case 64:
    case 65:
    case 82:
      return kHeavyRain;
    case 68:
    case 70:
    case 71:
    case 76:
    case 78:
    case 83:
    case 85:
    case 87:
      return kWeakSnow;
    case 72:
    case 73:
    case 77:
    case 79:
    case 84:
    case 86:
    case 88:
    case 89:
       return kModerateSnow;
    case 74:
    case 75:
    case 90:
      return kHeavySnow;
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
     return kLightThunder;
    case 97:
    case 98:
    case 99:
     return kHeavyThunder;
    default:
      return kNoPrecipitation;//kUnknownPrecipitation;
  }
}

