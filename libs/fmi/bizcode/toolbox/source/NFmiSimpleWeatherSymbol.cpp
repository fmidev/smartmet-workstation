//© Ilmatieteenlaitos/Persa.
//13.10.1994   Persa
//
//Ver. 30.11.1995/Persa		conversion from TFmiSimpleWeather
// 061196/LW
// 091296/LW lumelle oma symboli eikä auringon käyttö
// 111296/LW salamat mukaan
//----------------------------------------------------------------------------nsimweat.cpp

#include "NFmiSimpleWeatherSymbol.h"
#include "NFmiCloud.h"
#include "NFmiSnow.h"
#include "NFmiSun.h"
#include "NFmiFlash.h"
#include "NFmiDrop.h"
#include "NFmiDrawingEnvironment.h"

const float kDropSize = 0.18f; //0.1
const float kSnowSize = 0.22f; //091296/LW
const float kFlashSize = 0.45f; //0.25
const float kSunSize = 0.50f;
               
//____________________________________________________________________ 
NFmiSimpleWeatherSymbol::NFmiSimpleWeatherSymbol(short theValue
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
//____________________________________________________________________ Build
void NFmiSimpleWeatherSymbol::Build (void) 
{
  BuildCloudiness();
  BuildPrecipitation();
}
//____________________________________________________________________ BuildCloudiness
void NFmiSimpleWeatherSymbol::BuildCloudiness (void) 
{
/*
  GetEnvironment()->EnableFill();
  GetEnvironment()->EnableFrame();

  GetEnvironment()->SetFillColor(NFmiColor(0.0f, 0.4f, 0.5f));

  GetEnvironment()->DisableFrame();
 
  NFmiRect baseRect(GetFrame());
  GetToolBox()->Convert(&NFmiRectangle (baseRect.TopLeft()
                ,baseRect.BottomRight()
                ,0  //this
                ,GetEnvironment()));
*/
 GetEnvironment()->EnableFill();
 GetEnvironment()->SetFrameColor(NFmiColor(0.f, 0.f, 0.f));
 GetEnvironment()->SetFillColor(NFmiColor(1.f, 1.f, 0.f));
 GetEnvironment()->SetPenSize(NFmiPoint(.5,.5)); //061196/LW oli 1.,1.
 GetEnvironment()->EnableFrame();
 switch(RankCloudiness())
  {
    case kNoClouds:
    {
 //     GetEnvironment()->SetFrameColor(NFmiColor(0.f, 0.f, 0.f));//0.1f, 0.1f, 0.1f));//LW
 //     GetEnvironment()->SetFillColor(NFmiColor(1.f, 1.f, 0.f));//0.9f, 0.95f, 0.f));

      GetEnvironment()->SetFrameColor(NFmiColor(1.f, 0.f, 0.f));
      NFmiRect sunRect(GetFrame());
      sunRect.Inflate(-kSunSize/3.f * sunRect.Width(), -kSunSize/3.f * sunRect.Height());
      sunRect += NFmiPoint(0.f, -kSunSize/3.f * sunRect.Height());
      NFmiSun (sunRect
                     ,GetToolBox()
                     ,0  //this
                     ,GetEnvironment()).Build();

      break;
    }
    case kHalfCloudy:
    {
      GetEnvironment()->SetFrameColor(NFmiColor(1.f, 0.f, 0.f));
      NFmiRect sunRect(GetFrame());
      sunRect.Inflate(-kSunSize/3.f * sunRect.Width(), -kSunSize/3.f * sunRect.Height());
      sunRect += NFmiPoint(kSunSize/3.f * sunRect.Width(),  -kSunSize/3.f * sunRect.Height());
 //     GetEnvironment()->SetFrameColor(NFmiColor(0.1f, 0.1f, 0.1f));
 //     GetEnvironment()->SetFillColor(NFmiColor(0.9f, 0.95f, 0.f));
      GetEnvironment()->EnableFrame();

      NFmiSun (sunRect ,GetToolBox() , 0, GetEnvironment()).Build();

      NFmiRect cloudRect(sunRect);
      cloudRect += NFmiPoint(-2.f * kSunSize/3.f * sunRect.Width(), kSunSize/5.f * sunRect.Height());
      GetEnvironment()->SetFillColor(NFmiColor(0.8f, 0.8f, 0.8f));
      GetEnvironment()->SetFrameColor(NFmiColor(0.1f, 0.1f, 0.1f));
      GetEnvironment()->EnableFrame();

      NFmiCloud (cloudRect ,GetToolBox() , 0, GetEnvironment()).Build();

//** 061196/LW tuleeko rajat näkyviin
 //     GetEnvironment()->EnableFrame();
      GetEnvironment()->DisableFill();
      GetEnvironment()->SetFrameColor(NFmiColor(0.f, 0.f, 0.f));
      GetEnvironment()->SetFillColor(NFmiColor(0.f, 0.f, 0.f));
      NFmiCloud (cloudRect ,GetToolBox() , 0, GetEnvironment()).Build();
//** 061196/LW

      break;
    }
    case kOvercast:
    {
	  GetEnvironment()->SetFrameColor(NFmiColor(0.1f, 0.1f, 0.1f)); // oli SetFill.../LW
      GetEnvironment()->SetFillColor(NFmiColor(0.6f, 0.6f, 0.6f));
      GetEnvironment()->EnableFrame();
      NFmiRect cloudRect(GetFrame());
      cloudRect.Inflate(-kSunSize/3.f * cloudRect.Width(), -kSunSize/3.f * cloudRect.Height());
      cloudRect += NFmiPoint(kSunSize/3.f * cloudRect.Width(),  -kSunSize/3.f * cloudRect.Height());

      NFmiCloud (cloudRect ,GetToolBox() , 0, GetEnvironment()).Build();

      cloudRect += NFmiPoint(-2.5f * kSunSize/3.f * cloudRect.Width(), kSunSize/5.f * cloudRect.Height());

      GetEnvironment()->SetFillColor(NFmiColor(0.4f, 0.4f, 0.4f));
      NFmiCloud (cloudRect ,GetToolBox() , 0, GetEnvironment()).Build();

      break;
    }
    default:
      break;
   }

}
//____________________________________________________________________ BuildPresipitation
void NFmiSimpleWeatherSymbol::BuildPrecipitation (void) 
{
/*****

  NFmiDrawingEnvironment *theEnvironment = new NFmiDrawingEnvironment;  //GetEnvironment();
  theEnvironment->SetFillColor(NFmiColor(0.0, 0.8, 0.5));
  theEnvironment->SetFrameColor(NFmiColor(0.0, 0.8, 0.6));
  theEnvironment->EnableFill();
  
  NFmiPoint thePlace;
  NFmiPoint theDropSize;
  NFmiPoint theFlashSize;
  GlobalSize(kDropSize, kDropSize, theDropSize);
  GlobalSize(kFlashSize, kFlashSize, theFlashSize);
****/
  GetEnvironment()->EnableFrame();
  GetEnvironment()->EnableFill();
  GetEnvironment()->SetFrameColor(NFmiColor(0.f,0.f,0.f));
  NFmiDrawingEnvironment dropEnvi(*GetEnvironment());
  NFmiDrawingEnvironment snowEnvi(*GetEnvironment());

  NFmiRect theRect(GetFrame());
  NFmiRect dropRect(theRect.Left()
                   ,theRect.Top()
                   ,theRect.Left() + kDropSize * theRect.Width()
                   ,theRect.Top() + kDropSize * theRect.Height());
  dropRect += NFmiPoint(kSunSize/3.f * theRect.Width(),  kSunSize * 1.3f * theRect.Height());

  NFmiRect snowRect(theRect.Left()
                   ,theRect.Top()
                   ,theRect.Left() + kSnowSize * theRect.Width() 
                   ,theRect.Top() + kSnowSize * theRect.Height());
 
  NFmiRect flashRect(theRect.Left()
                   ,theRect.Top()
                   ,theRect.Left() + kFlashSize * theRect.Width() *.8f
                   ,theRect.Top() + kFlashSize * theRect.Height());

   switch(RankPrecipitation())
  {
    case kWeakRain:
    case kModerateRain:
    case kHeavyRain:
    {
      dropEnvi.SetFillColor(NFmiColor(0.0f, 0.7f, 0.4f));
        break;
    }	
    case kWeakSnow:
    case kModerateSnow:
    case kHeavySnow:
    {
      snowRect += NFmiPoint(kSunSize/3.f * theRect.Width(),  kSunSize * 1.3f * theRect.Height());

      snowEnvi.DisableFill();
      snowEnvi.SetPenSize(NFmiPoint(2.f,2.f)); //100397 oli (1.5;1.5)
//      GetEnvironment()->SetFrameColor(NFmiColor(0.0f, 0.6f, 0.5f));
      snowEnvi.SetFrameColor(NFmiColor(0.75f, 0.25f, 0.25f));
      break;
    }
	case kWeakSleet:
	case kModerateSleet:
	case kHeavySleet:
    {
      dropEnvi.SetFillColor(NFmiColor(0.0f, 0.7f, 0.4f));

      snowRect += NFmiPoint(kSunSize/3.f * theRect.Width(),  kSunSize * 1.3f * theRect.Height());

      snowEnvi.DisableFill();
      snowEnvi.SetPenSize(NFmiPoint(2.f,2.f)); //100397 oli (1.5;1.5)
      snowEnvi.SetFrameColor(NFmiColor(0.75f, 0.25f, 0.25f));
        break;
    }	

     case kLightThunder:
     case kHeavyThunder:
    {
      flashRect += NFmiPoint(kSunSize/4.f * theRect.Width(),  kSunSize * .95f * theRect.Height());

      GetEnvironment()->SetFrameColor(NFmiColor(1.f,0.f,0.f));
      GetEnvironment()->SetFillColor(NFmiColor(1.f,1.f,0.f));
      break;
    }
     default:
      break;
 }

// 
  switch(RankPrecipitation())
  {
    case kWeakRain:
    {
      NFmiDrop (dropRect
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      break;
    }
    case kModerateRain:
    {
      NFmiDrop (dropRect
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      NFmiDrop (dropRect += NFmiPoint(dropRect.Width(), 0.f)
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();

      break;
    }
    case kHeavyRain:
    {
      NFmiDrop (dropRect
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      NFmiDrop (dropRect += NFmiPoint(dropRect.Width(), 0.f)
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      NFmiDrop (dropRect += NFmiPoint(dropRect.Width(), 0.f)
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      break;
    }

	
    case kWeakSnow:
    {
      NFmiSnow (snowRect
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();
      break;
    }
    case kModerateSnow:
    {
      NFmiSnow (snowRect
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();
      NFmiSnow (snowRect += NFmiPoint(snowRect.Width(), 0.f)
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();

      break;
    }
    case kHeavySnow:
    {
       NFmiSnow (snowRect
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();
      NFmiSnow (snowRect += NFmiPoint(snowRect.Width(), 0.f)
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();
      NFmiSnow (snowRect += NFmiPoint(snowRect.Width(), 0.f)
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();
      break;
    }
    case kWeakSleet:
    {
      NFmiDrop (dropRect -= NFmiPoint(dropRect.Width(), 0.f)
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      NFmiSnow (snowRect
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();
      break;
    }
    case kModerateSleet:
    {
      NFmiDrop (dropRect -= NFmiPoint(dropRect.Width(), 0.f)
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      NFmiDrop (dropRect += NFmiPoint(dropRect.Width() * 2.2, 0.f)
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      NFmiSnow (snowRect
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();
      break;
    }
    case kHeavySleet:
    {
      NFmiDrop (dropRect -= NFmiPoint(dropRect.Width(), 0.f)
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      NFmiDrop (dropRect += NFmiPoint(dropRect.Width() * 2.2, 0.f)
               ,GetToolBox()
               ,0
               ,&dropEnvi).Build();
      NFmiSnow (snowRect
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();
      NFmiSnow (snowRect += NFmiPoint(snowRect.Width() * 2, 0.f)
               ,GetToolBox()
               ,0
               ,&snowEnvi).Build();
      break;
    }

/*****
    case kWeakSnow:
    {
      BuildSnowFlake (kCenterPOINT + kCloudHeight / 4
                     ,kFirstDropXPlace - kDropSize/2.
                     ,kCenter + kCloudHeight / 4 + kDropSize
                     ,kFirstDropXPlace + kDropSize/2.
                     ,theEnvironment);
      break;
    }
    case kModerateSnow:
    {
      BuildSnowFlake (kCenter + kCloudHeight / 4
                     ,kFirstDropXPlace  - kDropSize - kDropSize/2.
                     ,kCenter + kCloudHeight / 4 + kDropSize
                     ,kFirstDropXPlace  - kDropSize  +  kDropSize/2.
                     ,theEnvironment);
      BuildSnowFlake (kCenter + kCloudHeight / 4
                     ,kFirstDropXPlace  +  kDropSize - kDropSize/2.
                     ,kCenter + kCloudHeight / 4 + kDropSize
                     ,kFirstDropXPlace  +  kDropSize + kDropSize/2.
                     ,theEnvironment);
      break;
//      BuildSnowFlake (0.60, 0.45, 0.70, 0.55, theEnvironment);
//      break;
    }
    case kHeavySnow:
    {
      BuildSnowFlake (kCenter + kCloudHeight / 4
                     ,kFirstDropXPlace  -  2 * kDropSize - kDropSize/2.
                     ,kCenter + kCloudHeight / 4 + kDropSize
                     ,kFirstDropXPlace  -  2 * kDropSize + kDropSize/2.
                     ,theEnvironment);
      BuildSnowFlake (kCenter + kCloudHeight / 4 + kDropSize
                     ,kFirstDropXPlace   - kDropSize/2.
                     ,kCenter + kCloudHeight / 4 + 2 * kDropSize
                     ,kFirstDropXPlace   + kDropSize/2.
                     ,theEnvironment);
      BuildSnowFlake (kCenter + kCloudHeight / 4
                     ,kFirstDropXPlace  +  2 * kDropSize - kDropSize/2.
                     ,kCenter + kCloudHeight / 4 + kDropSize
                     ,kFirstDropXPlace  +  2 * kDropSize + kDropSize/2.
                     ,theEnvironment);
                     
//      BuildSnowFlake (0.45, 0.30, 0.55, 0.40, theEnvironment);
//      BuildSnowFlake (0.45, 0.60, 0.55, 0.70, theEnvironment);
//      BuildSnowFlake (0.45, 0.60, 0.55, 0.70, theEnvironment);
      break;

    } 
*****/	   //LW otti käyttöön 111296
    case kLightThunder:
    {
        NFmiFlash (flashRect
               ,GetToolBox()
               ,0
               ,GetEnvironment()).Build();
//     BuildFlash (GlobalPlace(kFirstDropXPlace -  kDropSize, kCenter + kCloudHeight / 4, thePlace),
//                  theFlashSize);
      break;
    }
    case kHeavyThunder:
    {
      NFmiFlash (flashRect
               ,GetToolBox()
               ,0
               ,GetEnvironment()).Build();
      NFmiFlash (flashRect += NFmiPoint(flashRect.Width()*.6f, 0.f)
               ,GetToolBox()
               ,0
               ,GetEnvironment()).Build();
       break;
    }
//*****/
    default:
      break;
   }
}
/***
//__________________________________________________________________________ BuildCloud
void NFmiSimpleWeatherSymbol::BuildCloud  (const NFmiPoint &thePlace,
                                           const NFmiPoint &theSize,
                                           NFmiDrawingEnvironment *theEnvironment) 
{
   NFmiCloud *theCloud = new NFmiCloud(thePlace,
                                           NFmiPoint(thePlace.GetX() + theSize.GetX(),
                                                     thePlace.GetY() + theSize.GetY()
                                                    ),
                                           theEnvironment);
   AddShape(theCloud);

******   NFmiColor shadeColor(theEnvironment->GetFillColor());

   for(short shade = 5; shade > 0; shade--)
   {
     short x = theSize.GetX() / 5 * shade;
     short y = theSize.GetY() / 5 * shade;
     shadeColor.Mix(NFmiColor(0., 0., 0.), 0.3);
     theEnvironment->SetFillColor(shadeColor.GetRGB());
     NFmiCloud *theCloud = new NFmiCloud(thePlace,
                                           NFmiPoint(thePlace.GetX() + x,
                                                     thePlace.GetY() + y
                                                    ),
                                           theEnvironment);
   AddShape(theCloud);
   }
*******
  // return theCloud;
}
*****/
/******
//__________________________________________________________________________ BuildSun
void NFmiSimpleWeatherSymbol::BuildSun  (const NFmiPoint &thePlace,
                                         const NFmiPoint &theSize) 
{
  NFmiSun kaniini(NFmiRect(NFmiPoint(itsItemRect.Left(), itsItemRect.Top())
                          ,NFmiPoint(itsItemRect.Right(), itsItemRect.Bottom()))
                 ,0  //this
                 ,GetEnvironment());
  kaniini.Build();
  GetToolBox()->Convert(&kaniini);
}
*****/
/******
//__________________________________________________________________________ BuildCloud
void NFmiSimpleWeatherSymbol::BuildFlash  (const NFmiPoint &thePlace,
                                           const NFmiPoint &theSize,
                                           NFmiDrawingEnvironment *theEnvironment) 
{
  NFmiDrawingEnvironment *myEnvironment;
  
  if(theEnvironment)
    myEnvironment = theEnvironment;
  else
  {
    myEnvironment = new NFmiDrawingEnvironment;
    myEnvironment->EnableFill();
    myEnvironment->SetFillColor(NFmiColor(1.0, 0.9, 0.0));
    myEnvironment->SetFrameColor(NFmiColor(0.8, 0.6, 0.0));
  }
   NFmiFlash *theFlash = new NFmiFlash(thePlace,
                                       NFmiPoint(thePlace.GetX() + theSize.GetX(),
                                                 thePlace.GetY() + theSize.GetY()
                                                ),
                                       myEnvironment);
   AddShape(theFlash);

   AddShape(theFlash);
  // return theFlash;
}
*****/
