//© Ilmatieteenlaitos/Lasse.
//14.3.1997  
//
//		conversion from TFmiSimpleWeatherSymbol

#pragma once

#include "NFmiSimpleWeatherSymbol.h"

//_________________________________________________________ NFmiSynopWeatherSymbol
class NFmiSynopWeatherSymbol : public NFmiSimpleWeatherSymbol
{
 public:

   NFmiSynopWeatherSymbol(short theValue
	                      ,short theCloudValue
                          ,const NFmiRect &theRect
                          ,NFmiToolBox *theToolBox
                          ,NFmiView *theEnclosure = 0
                          ,NFmiDrawingEnvironment *theEnvironment = 0
                          ,ShapeIdentifier theIdent = kSymbol); 

 //   virtual void Build(void);	 // 170397 pois
     
 protected:
 
         CloudinessType RankCloudiness (void); 
      PrecipitationType RankPrecipitation (void); 
  //       unsigned short GetWeatherSpecifier (void); 
 //                  void BuildCloudiness (void);
 //                  void BuildPrecipitation (void); 
 private:
   short itsCloudValue;
};
typedef NFmiSynopWeatherSymbol* PNFmiSynopWeatherSymbol;


