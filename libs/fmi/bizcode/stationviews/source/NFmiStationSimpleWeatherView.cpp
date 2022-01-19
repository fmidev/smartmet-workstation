//© Ilmatieteenlaitos/Marko
//  Original 24.09.1998
//
//
//Ver. xx.xx.xxxx/Marko
//
//-------------------------------------------------------------------- NFmiStationSimpleWeatherView.cpp

#include "NFmiStationSimpleWeatherView.h"
#include "NFmiSimpleWeatherSymbol.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiDrawParam.h"
#include "NFmiArea.h"
#include "NFmiDrawingEnvironment.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiWeatherAndCloudiness.h"

#include<boost\math\special_functions\round.hpp>

NFmiStationSimpleWeatherView::NFmiStationSimpleWeatherView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
														  ,NFmiToolBox *theToolBox
														  ,NFmiDrawingEnvironment* theDrawingEnvi
														  ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
														  ,FmiParameterName theParamId
														  ,NFmiPoint theOffSet
														  ,NFmiPoint theSize
														  ,int theRowIndex
                                                          ,int theColumnIndex)
:NFmiStationView(theMapViewDescTopIndex, theArea
				,theToolBox
				,theDrawingEnvi
				,theDrawParam
				,theParamId
				,theOffSet
				,theSize
				,theRowIndex
                ,theColumnIndex)
{
}

NFmiStationSimpleWeatherView::~NFmiStationSimpleWeatherView(void)
{
}

void NFmiStationSimpleWeatherView::Draw(NFmiToolBox * theGTB)
{
	NFmiStationView::Draw(theGTB);
	if(fDoTimeInterpolation && itsInfo)
		itsInfo->Param(itsParamId);
}

bool NFmiStationSimpleWeatherView::PrepareForStationDraw(void)
{
	bool status = NFmiStationView::PrepareForStationDraw();
	if(fDoTimeInterpolation)
	{ 
		// jos tarvitaan aikainterpolaatiota, pitää tehdä jippo tässä ja asettaa totalwind päälle
		if(!itsInfo->Param(kFmiWeatherAndCloudiness)) // yritetään laittaa WeatherAndCloudiness parametri päälle jos pitää tehdä aikainterpolaatiota
			itsInfo->Param(itsParamId); // jos datassa ei ole totalwindiä, palauta originaali parametri takaisin
	}
	return status;
}

float NFmiStationSimpleWeatherView::ViewFloatValue(bool )
{
	float value = kFloatMissing;
	if(itsInfo)
	{
		if(fDoTimeInterpolation)
		{ // tehdään itse aikainterpolaatio totalwindin avulla
            if(itsInfo->Param().GetParamIdent() == kFmiWeatherAndCloudiness)
            {
			    NFmiWeatherAndCloudiness weather(itsInfo->InterpolatedValue(itsTime, 360), kFmiPackedWeather, kFloatMissing, itsInfo->InfoVersion());
			    return static_cast<float>(weather.SubValue(itsParamId));
            }
            else
                value = itsInfo->InterpolatedValue(itsTime);
		}
		else
			value = itsInfo->FloatValue();
	}
	return value;
}

// Huono nimi virtuaali metodilla GetSpaceOutFontFactor,
// tuuli vektori ei ole fontti pohjainen symboli ja metodin pitäisi olla joku SymbolSizeFactor
NFmiPoint NFmiStationSimpleWeatherView::GetSpaceOutFontFactor(void)
{
	return NFmiPoint(0.8, 1.2);
}

void NFmiStationSimpleWeatherView::ModifyTextEnvironment(void)
{
    itsDrawingEnvironment->SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 48), itsCtrlViewDocumentInterface->Printing()));
}

NFmiPoint NFmiStationSimpleWeatherView::SbdCalcFixedSymbolSize() const
{
	return SbdBasicSymbolSizeCalculation(12, 48);
}

NFmiSymbolBulkDrawType NFmiStationSimpleWeatherView::SbdGetDrawType() const
{
	return NFmiSymbolBulkDrawType::HessaaSymbol;
}

NFmiSymbolColorChangingType NFmiStationSimpleWeatherView::SbdGetSymbolColorChangingType() const
{
	return NFmiSymbolColorChangingType::Never;
}

NFmiPoint NFmiStationSimpleWeatherView::SbdCalcDrawObjectOffset() const
{
	NFmiPoint offset = CurrentDataRect().Center();
	// CurrentDataRect:issa on mukana symbolipiirtoon liittyvät offsetit.
	offset -= CurrentStationPosition();
	return offset;
}
