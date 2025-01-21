// NFmiTimeSerialSymbolView.cpp: implementation of the NFmiTimeSerialSymbolView class.
//
//////////////////////////////////////////////////////////////////////

#include "NFmiTimeSerialSymbolView.h"
#include "NFmiWeatherAndCloudiness.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiSimpleWeatherSymbol.h"
#include "NFmiToolBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NFmiTimeSerialSymbolView::NFmiTimeSerialSymbolView(int theMapViewDescTopIndex, const NFmiRect & theRect
												  ,NFmiToolBox * theToolBox
												  ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
												  ,int theIndex)
:NFmiTimeSerialView(theMapViewDescTopIndex, theRect
				  ,theToolBox
				  ,theDrawParam
				  ,theIndex)
{

}

NFmiTimeSerialSymbolView::~NFmiTimeSerialSymbolView()
{

}

NFmiRect NFmiTimeSerialSymbolView::CalcSymbolRect(NFmiMetTime& time)
{
	float size = float(GetFrame().Height()/5.);	// symbolin koko reagoi korkeuteen
	NFmiPoint place((0 + size/2),(0.5 + size)); 
	NFmiPoint topLeft(place.X() - itsToolBox->SX(itsToolBox->HY(size)), place.Y() - size);
	NFmiPoint bottomRight(place.X() + itsToolBox->SX(itsToolBox->HY(size)), place.Y() + size);
	NFmiRect rect(topLeft,bottomRight);
	NFmiPoint centerPoint(Time2Value(time), GetFrame().Top() + GetFrame().Height()/2.);
	rect.Center(centerPoint);
	return rect;
}

void NFmiTimeSerialSymbolView::DrawSymbol(const NFmiPoint &theLatLonPoint, NFmiMetTime& time, NFmiRect& rect, NFmiTimePerioid& step)
{
	
	itsDrawingEnvironment.SetFrameColor(NFmiColor(1,0,0));
	itsDrawingEnvironment.DisableFill();
	
	//dataparamilta pit‰isi saada weatherandCloudiness!!
	NFmiWeatherAndCloudiness weather((unsigned long)itsInfo->InterpolatedValue(theLatLonPoint),kFmiHessaaWeather, kFloatMissing, itsInfo->InfoVersion());
	NFmiMetTime time1 = time;
	time1.ChangeByMinutes(-EditedDataTimeBag().Resolution());
	NFmiMetTime time2 = time;
	time2.ChangeByMinutes(EditedDataTimeBag().Resolution());
	itsInfo->Time(time1);
	NFmiWeatherAndCloudiness weather1((unsigned long)itsInfo->InterpolatedValue(theLatLonPoint),kFmiHessaaWeather, kFloatMissing, itsInfo->InfoVersion());
	NFmiWeatherAndCloudiness weather2(itsInfo->InfoVersion()); //viimeiselle ajalle ei en‰‰ ole arvoa
	if(itsInfo->Time(time2))
		weather2 = NFmiWeatherAndCloudiness((unsigned long)itsInfo->InterpolatedValue(theLatLonPoint),kFmiHessaaWeather, kFloatMissing, itsInfo->InfoVersion());
	float factor = 0.3f;
	long symbolValue = weather.TimeIntegratedHessaa(weather1,factor,weather,factor,weather2,factor);
	itsInfo->Time(time); //asetetaan takaisin oikeaan aikaan

	NFmiSimpleWeatherSymbol((short)symbolValue
                          ,rect
                          ,itsToolBox
						  ,0
						  ,&itsDrawingEnvironment).Build();
}

void NFmiTimeSerialSymbolView::DrawSimpleSymbol(const NFmiPoint &theLatLonPoint, NFmiMetTime& time, NFmiRect& rect)
{
	itsDrawingEnvironment.SetFrameColor(NFmiColor(1,0,0));
	itsDrawingEnvironment.DisableFill();
	long symbolValue = (long)itsInfo->InterpolatedValue(theLatLonPoint);
	NFmiSimpleWeatherSymbol((short)symbolValue
                          ,rect
                          ,itsToolBox
						  ,0
						  ,&itsDrawingEnvironment).Build();
}

void NFmiTimeSerialSymbolView::DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines)
{
	
	NFmiMetTime time;

	itsInfo->ResetTime();
	itsInfo->NextTime(); //menn‰‰n heti ekaan aikaan
	time = itsInfo->Time();

	NFmiRect rect;
	
	int i=1;
	int step = 3; //t‰h‰n symbolejen aikav‰li (step*timeStep)
	if(EditedDataTimeBag().Resolution()/60 > step)
	{
		for(itsInfo->ResetTime(); itsInfo->NextTime();)
		{
			time = itsInfo->Time();
			rect = CalcSymbolRect(time); //lasketaan symbolin koko jo t‰‰ll‰ valmiiksi
			DrawSimpleSymbol(theLatLonPoint, time, rect);
		}
	}
	else
	{
		int timeCount = itsInfo->SizeTimes();
		for(itsInfo->NextTime(); i < timeCount-1 && itsInfo->NextTime(); )
		{
			int j=1;
			time = itsInfo->Time();
			rect = CalcSymbolRect(time); //lasketaan symbolin koko jo t‰‰ll‰ valmiiksi
			NFmiTimePerioid period(step*60);
			DrawSymbol(theLatLonPoint, time, rect, period);
			i += step;
			for(j=1; j<step && itsInfo->NextTime();j++);
		}
	} 
}

void NFmiTimeSerialSymbolView::DrawValueAxis(void)
{
	return;
}

void NFmiTimeSerialSymbolView::DrawModifyingUnit(void)
{
	return;
}
void NFmiTimeSerialSymbolView::DrawModifyFactorAxis(void)
{
	return;
}
void NFmiTimeSerialSymbolView::CreateValueScale(void)
{
	return;
}
