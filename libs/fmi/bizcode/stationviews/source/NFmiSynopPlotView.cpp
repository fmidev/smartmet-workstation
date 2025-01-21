//© Ilmatieteenlaitos/software by Marko
//  Original 31.08.2004
//
//
//-------------------------------------------------------------------- NFmiSynopPlotView.cpp

#include "NFmiSynopPlotView.h"
#include "NFmiRectangle.h"
#include "NFmiToolBox.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiWindBarb.h"
#include "NFmiText.h"
#include "NFmiStringTools.h"
#include "NFmiArea.h"
#include "NFmiValueString.h"
#include "NFmiSynopPlotSettings.h"
#include "NFmiInfoOrganizer.h"
#include "FmiNMeteditLibraryDefinitions.h"
#include "NFmiIgnoreStationsData.h"
#include "CtrlViewDocumentInterface.h"
#include "GraphicalInfo.h"
#include "NFmiSynopStationPrioritySystem.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "ToolBoxStateRestorer.h"

#include <limits>

// palauttaa annetun laatikon sis‰lt‰ pisteen, johon relatiivisessa 0,0 - 1,1 maailmassa
// oleva piste osoittaa. T‰m‰ relatiivinen maailma on positiivinen oikealle ja ylˆs.
static NFmiPoint GetRelativeLocationFromRect(const NFmiRect &theRect, const NFmiPoint &thePoint)
{
	double x = theRect.Left() + thePoint.X() * theRect.Width();
	double y = theRect.Bottom() - thePoint.Y() * theRect.Height();
	return NFmiPoint(x, y);
}

NFmiSynopPlotView::NFmiSynopPlotView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
									,NFmiToolBox * theToolBox
									,boost::shared_ptr<NFmiDrawParam> &theDrawParam
									,FmiParameterName theParamId
									,int theRowIndex
                                    ,int theColumnIndex)
:NFmiStationView(theMapViewDescTopIndex, theArea, theToolBox, theDrawParam, theParamId,
				 NFmiPoint(), NFmiPoint(), theRowIndex, theColumnIndex)
,itsFontSizeX(10)
,itsFontSizeY(10)
,fSoundingPlotDraw(false)
,fMinMaxPlotDraw(false)
,fMetarPlotDraw(false)
,itsQ2WantedParamVector()
{
	itsQ2WantedParamVector.push_back(kFmiTemperature);
	itsQ2WantedParamVector.push_back(kFmiMaximumTemperature);
	itsQ2WantedParamVector.push_back(kFmiMinimumTemperature);
	itsQ2WantedParamVector.push_back(kFmiSnowDepth);
	itsQ2WantedParamVector.push_back(kFmiPrecipitationAmount);
	itsQ2WantedParamVector.push_back(kFmiGroundTemperature);
	itsQ2WantedParamVector.push_back(kFmiWindDirection);
	itsQ2WantedParamVector.push_back(kFmiWindSpeedMS);
	itsQ2WantedParamVector.push_back(kFmiWindVectorMS);
	itsQ2WantedParamVector.push_back(kFmiPressure);
	itsQ2WantedParamVector.push_back(kFmiVisibility);
	itsQ2WantedParamVector.push_back(kFmiCloudHeight);
	itsQ2WantedParamVector.push_back(kFmiLowCloudCover);
	itsQ2WantedParamVector.push_back(kFmiPressureChange);
	itsQ2WantedParamVector.push_back(kFmiPrecipitationAmount);
	itsQ2WantedParamVector.push_back(kFmiDewPoint);
	itsQ2WantedParamVector.push_back(kFmiPresentWeather);
	itsQ2WantedParamVector.push_back(kFmiLowCloudType);
	itsQ2WantedParamVector.push_back(kFmiMiddleCloudType);
	itsQ2WantedParamVector.push_back(kFmiHighCloudType);
	itsQ2WantedParamVector.push_back(kFmiPressureTendency);
	itsQ2WantedParamVector.push_back(kFmiPastWeather1);
	itsQ2WantedParamVector.push_back(kFmiPastWeather2);
	itsQ2WantedParamVector.push_back(kFmiTotalCloudCover);
	itsQ2WantedParamVector.push_back(kFmiPrecipitationPeriod);
}

NFmiSynopPlotView::~NFmiSynopPlotView(void)
{
}

// T‰m‰ lyhyt versio riitt‰‰ kun tiedet‰‰n ett‰ laatikot ovat saman kokoisia.
static bool AreRectsOverLapping(const NFmiRect &first, const NFmiRect &second, double thePlotSpacing)
{
	if(thePlotSpacing == 0) // nollalla sallitaan kaikki asemat plottiin
		return false;
	else if(thePlotSpacing == 1)
		return first.IsInside(second.BottomLeft())  || first.IsInside(second.TopRight())
			|| first.IsInside(second.BottomRight()) || first.IsInside(second.TopLeft());
	else
	{ // lasketaan kuinka l‰hell‰ laatikoiden keskipisteet saavat olla toisiinsa n‰hden
		NFmiPoint diffP = first.Center() - second.Center();
		double xDiffRatio = diffP.X() / first.Width();
		double yDiffRatio = diffP.Y() / first.Height();
		if(::fabs(xDiffRatio) > thePlotSpacing || ::fabs(yDiffRatio) > thePlotSpacing)
			return false;
		return true;
	}
}

static bool IsRectOverLapping(const NFmiRect &theRect, std::vector<NFmiRect> &theExistingRects, double thePlotSpacing)
{
	int count = static_cast<int>(theExistingRects.size());
	for(int i=0; i < count; i++)
	{
		if(AreRectsOverLapping(theExistingRects[i], theRect, thePlotSpacing))
			return true;
	}
	return false;
}

static const float gPressureLevelEpsilon = std::numeric_limits<float>::epsilon() * 3; // t‰m‰ pit‰‰ olla pieni arvo (~epsilon) koska muuten pienet rajat eiv‰t toimi, mutta pelkk‰ epsilon on liian pieni

// synop-dataa ei tarvitsee asettaa mihink‰‰n, mutta
// luotaus plotin yhteydess‰ pit‰‰ etsi‰ oikea korkeus/level
static void SetLevel(boost::shared_ptr<NFmiDrawParam> &drawParam, NFmiFastQueryInfo &info)
{
	if(drawParam->Param().GetParamIdent() == NFmiInfoData::kFmiSpSoundingPlot)
	{ // sounding plotin yhteydess‰ pit‰‰ asettaa level haluttuun korkeuteen
		float levelValue = static_cast<float>(drawParam->Level().LevelValue());
		if(info.Param(kFmiPressure))
		{
			for(info.ResetLevel(); info.NextLevel(); )
				if(::fabs(info.FloatValue() - levelValue) < gPressureLevelEpsilon) // pieni virhe sallitaan kun etsit‰‰n haluttua painepintaa
					break;
		}
	}
}

NFmiRect NFmiSynopPlotView::CalcBaseEmptySoundingMarker(void)
{
	double circleSizeInMM = 1.5f;
    auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
	int pixelSizeX = boost::math::iround(circleSizeInMM * graphicalInfo.itsPixelsPerMM_x);
	int pixelSizeY = boost::math::iround(circleSizeInMM * graphicalInfo.itsPixelsPerMM_y);
	double xWidth = itsToolBox->SX(pixelSizeX);
	double yWidth = itsToolBox->SY(pixelSizeY);
	return NFmiRect(0, 0, xWidth, yWidth);
}

void NFmiSynopPlotView::SetMapViewSettings(boost::shared_ptr<NFmiFastQueryInfo> &theUsedInfo)
{
    NFmiStationView::SetMapViewSettings(theUsedInfo);
    SetupPossibleWindMetaParamData();
}

// HUOM!! Piirto koodia voisi optimoida kun k‰ytt‰‰ enemm‰n param-index (ja subParam) tekniikkaa. Nyt
// optimointia k‰ytet‰‰n vain T, Td ja P parametrien kanssa!!!
void NFmiSynopPlotView::Draw(NFmiToolBox * theGTB)
{
    if(!IsParamDrawn())
    {
        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging("NFmiSynopPlotView doesn't draw anything, param was hidden", this);
        return;
    }

    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    itsToolBox = theGTB;
	fGetSynopDataFromQ2 = false;

	fSoundingPlotDraw = itsDrawParam->Param().GetParamIdent() == NFmiInfoData::kFmiSpSoundingPlot;
	fMinMaxPlotDraw = itsDrawParam->Param().GetParamIdent() == NFmiInfoData::kFmiSpMinMaxPlot;
	fMetarPlotDraw = itsDrawParam->Param().GetParamIdent() == NFmiInfoData::kFmiSpMetarPlot;

	NFmiSynopPlotSettings & synopSettings = *itsCtrlViewDocumentInterface->SynopPlotSettings();
	auto usedFontSize = fMetarPlotDraw ? synopSettings.MetarPlotSettings().FontSize() : synopSettings.FontSize();
	double fontSizeInMM_x = usedFontSize;
	double fontSizeInMM_y = usedFontSize;
    auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
    itsFontSizeX = boost::math::iround(fontSizeInMM_x * graphicalInfo.itsPixelsPerMM_x * 1.88);
	itsFontSizeY = boost::math::iround(fontSizeInMM_y * graphicalInfo.itsPixelsPerMM_y * 1.88);
	double relativeFontSizeX = itsToolBox->SX(itsFontSizeX);
	double relativeFontSizeY = itsToolBox->SY(itsFontSizeY);
	double synopBoxRelWidth = relativeFontSizeX * 4.8 * 0.976;
	double synopBoxRelHeight = relativeFontSizeY * 3.85 * 0.976;
	itsObjectSize = NFmiPoint(synopBoxRelWidth, synopBoxRelHeight);
	NFmiRect emptySoundingMarkerRect(CalcBaseEmptySoundingMarker());

	NFmiRect rect(0, 0, synopBoxRelWidth, synopBoxRelHeight);
	itsDrawingEnvironment.DisableFill();
	// optimointia, nyt filli on disabloitu, mutta myˆhemmin tarvitaan valkoista filli‰
	itsDrawingEnvironment.SetFillColor(NFmiColor(1.f, 1.f, 1.f));
	itsDrawingEnvironment.EnableFrame();
	itsDrawingEnvironment.SetFrameColor(itsDrawParam->FrameColor());

    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsArea->XYArea());
	std::vector<NFmiRect> synopRects;
	MakeDrawedInfoVector();
	if(itsInfoVector.empty())
		return ;

	NFmiIgnoreStationsData &ignorestationdata = itsCtrlViewDocumentInterface->IgnoreStationsData();
	bool useSynopDataForPlot = (fSoundingPlotDraw == false) && (fMetarPlotDraw == false);

	if(useSynopDataForPlot && UseQ2ForSynopData(itsDrawParam))
	{
		GetQ2SynopData(0, itsQ2WantedParamVector);
		if(fGetSynopDataFromQ2 == false)
			return ;
	}

    bool drawStationMarker = IsAccessoryStationDataDrawn();
	NFmiDrawingEnvironment stationPointEnvi;
	SetStationPointDrawingEnvi(stationPointEnvi);
	for(auto& fastInfo : itsInfoVector)
	{
		// Varmistetaan ett‰ osoitetaan johon validiin asemaan/pisteeseen, muuten tulee ongelmia nan -pohjaisten point-olioiden kanssa
		fastInfo->FirstLocation();
		SetMapViewSettings(fastInfo);
		if(itsInfo == 0)
			continue ;
		if(!itsInfo->Time(itsTime) && fGetSynopDataFromQ2 == false)
			continue ;
		CalculateGeneralStationRect();
		auto usedPlotSpacing = fMetarPlotDraw ? synopSettings.MetarPlotSettings().PlotSpacing() : synopSettings.PlotSpacing();
		if(usedPlotSpacing > 0)
		{
			// Priorisointi jutusta on hyˆty‰ vain jos harvennus on p‰‰ll‰, PlotSpacing on suurempi kuin 0.
			// priorisoidut asemat k‰yd‰‰n ensin l‰pi
			NFmiSynopStationPrioritySystem &prioritySystem = *itsCtrlViewDocumentInterface->SynopStationPrioritySystem();
			int maxPriorityCount = prioritySystem.MaxPriorityLevel();
			for(int i=1; i<=maxPriorityCount; i++)
			{
				const SynopStationPriorityList &priorityList = prioritySystem.PriorityList(i);
				int ssize = static_cast<int>(priorityList.size());
				for(int j=0; j<ssize; j++)
				{
					if(itsInfo->Location(priorityList[j].itsStationId))
					{
						if(ignorestationdata.IsStationBlocked(*(itsInfo->Location()), true) == false)
						{
							DrawSynopPlot(usedPlotSpacing, rect, stationPointEnvi, synopRects, emptySoundingMarkerRect, drawStationMarker);
						}
					}
				}
			}
		}

		for(itsInfo->ResetLocation(); itsInfo->NextLocation(); )
		{
			if(ignorestationdata.IsStationBlocked(*(itsInfo->Location()), true) == false)
			{
				DrawSynopPlot(usedPlotSpacing, rect, stationPointEnvi, synopRects, emptySoundingMarkerRect, drawStationMarker);
			}
		}
	}
}

void NFmiSynopPlotView::DrawSynopPlot(double plotSpacing, NFmiRect &theSynopRect, NFmiDrawingEnvironment &theStationPointEnvi, std::vector<NFmiRect> &theSynopRects, NFmiRect &theEmptySoundingMarkerRect, bool drawStationMarker)
{
	if(itsArea->IsInside(CurrentLatLon()))
	{
		if(drawStationMarker)
            DrawStation(theStationPointEnvi); // piirret‰‰n asema piste kaikille asemille jos niin on asetuksissa m‰‰r‰tty
		theSynopRect.Center(this->LatLonToViewPoint(CurrentLatLon()));
		if(plotSpacing == 0 || !IsRectOverLapping(theSynopRect, theSynopRects, plotSpacing))
		{
			bool fWindDrawed = false;
			bool anythingDrawn = false;
			::SetLevel(itsDrawParam, *itsInfo);
			if(fMinMaxPlotDraw) // min/max-plot
				anythingDrawn |= DrawMinMaxPlot(itsInfo, theSynopRect);
			else if(fMetarPlotDraw) // metar-plot
				anythingDrawn |= DrawMetarPlot(itsInfo, theSynopRect);
			else
			{
				anythingDrawn |= DrawWindVector(itsInfo, theSynopRect, fWindDrawed, false);
				anythingDrawn |= DrawNormalFontValues(itsInfo, theSynopRect);
				anythingDrawn |= DrawSynopFontValues(itsInfo, theSynopRect, fWindDrawed);
			}

			if(anythingDrawn && plotSpacing > 0)
				theSynopRects.push_back(theSynopRect);
			if((fSoundingPlotDraw && anythingDrawn == false) || (fMinMaxPlotDraw && anythingDrawn == true)) // min-max plottiin tulee aina markeri
			{
				theEmptySoundingMarkerRect.Center(theSynopRect.Center());
				DrawEmptySoundingMarker(theEmptySoundingMarkerRect);
			}
		}
	}
}

void NFmiSynopPlotView::DrawEmptySoundingMarker(const NFmiRect &theEmptySoundingMarkerRect)
{
	NFmiDrawingEnvironment envi;
	envi.SetFillColor(NFmiColor(0,0,0));
	envi.EnableFill();
	itsToolBox->DrawEllipse(theEmptySoundingMarkerRect, &envi);
}

// uusi tooltip systeemi CPPTooltip k‰ytt‰‰ t‰t‰ piirtoa (t‰t‰ ei k‰ytet‰ muualta)
void NFmiSynopPlotView::DrawSynopPlot(NFmiToolBox * theGTB, const NFmiLocation &theLocation, const NFmiRect &theRect, const NFmiMetTime &theTime, double theFontSize, bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot)
{
	NFmiSynopPlotSettings & synopSettings = *itsCtrlViewDocumentInterface->SynopPlotSettings();
	double oldFontSize = synopSettings.FontSize();
	synopSettings.FontSize(theFontSize);
	try
	{
		itsToolBox = theGTB;
		boost::shared_ptr<NFmiFastQueryInfo> info;
		this->fMinMaxPlotDraw = fDrawMinMaxPlot;
		if(fDrawSoundingPlot)
		{
			itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false); // t‰m‰kin pit‰‰ viel‰ jostain syyst‰ asettaa
			if(itsInfo == 0)
				return;
			itsInfo->Location(theLocation.GetLocation());
			itsInfo->Time(itsTime);
			// time ja location pit‰‰ asettaa ensin ennekuin voidaan etsi‰ leveli‰!!!
			::SetLevel(itsDrawParam, *itsInfo);
			info = itsInfo;
		}
		else if(fDrawMetarPlot)
		{
			itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false); // t‰m‰kin pit‰‰ viel‰ jostain syyst‰ asettaa
			if(itsInfo == 0)
				return;
			itsInfo->Location(theLocation.GetLocation());
			itsInfo->Time(itsTime);
			info = itsInfo;
		}
		else
			info = itsCtrlViewDocumentInterface->GetNearestSynopStationInfo(theLocation, theTime, false, 0);
		if(info == 0)
			return;

		// T‰m‰ asetus funktio pit‰‰ kutsua, ett‰ ei j‰‰ tiettyj‰ asetuksia normaali piirrosta p‰‰lle (esim. fDoMovingStationDataLocations -lippu 
		// v‰‰r‰‰n tilaan, joka aiheutti ep‰m‰‰r‰isen tooltipiss‰ olleen tuuliviiri piirron v‰‰nnˆn)
        SetMapViewSettings(info); 

        auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
        itsFontSizeX = boost::math::iround(theFontSize * graphicalInfo.itsPixelsPerMM_x * 1.88);
		itsFontSizeY = boost::math::iround(theFontSize * graphicalInfo.itsPixelsPerMM_y * 1.88);

		itsDrawingEnvironment.DisableFill();
		// optimointia, nyt filli on disabloitu, mutta myˆhemmin tarvitaan valkoista filli‰
		itsDrawingEnvironment.SetFillColor(NFmiColor(1.f, 1.f, 1.f));
		itsDrawingEnvironment.EnableFrame();

		bool fWindDrawed = false;
		if(fMinMaxPlotDraw) // min/max-plot
		{
			DrawMinMaxPlot(info, theRect);
			NFmiRect markerRect(0,0,itsToolBox->SX(5), itsToolBox->SY(5));
			markerRect.Center(theRect.Center());
			itsDrawingEnvironment.EnableFill();
			itsDrawingEnvironment.SetFillColor(NFmiColor(0.f, 0.f, 0.f));
			itsToolBox->DrawEllipse(markerRect, &itsDrawingEnvironment);
		}
		else if(fDrawMetarPlot) // min/max-plot
		{
			DrawMetarPlot(info, theRect);
		}
		else
		{
			DrawWindVector(info, theRect, fWindDrawed, false);
			DrawNormalFontValues(info, theRect);
			DrawSynopFontValues(info, theRect, fWindDrawed);
		}
	}
	catch(...)
	{
		// ei tehd‰ mit‰‰n, nyt poikkeuksen yhteydess‰ voidaan kuitenkin asettaa originaali fontti koko takaisin
	}
	synopSettings.FontSize(oldFontSize);
}

void NFmiSynopPlotView::GetWindValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, float &theWindSpeed, float &theWindDir)
{
	float windVector = GetSynopPlotValue(theInfo, kFmiWindVectorMS);
	if(windVector != kFloatMissing)
	{
		theWindSpeed = windVector / 100;
		theWindDir = static_cast<float>( ((int)windVector % 100) * 10);
		return ;
	}
	
	theWindSpeed = GetSynopPlotValue(theInfo, kFmiWindSpeedMS);
	theWindDir = GetSynopPlotValue(theInfo, kFmiWindDirection);
}

NFmiString NFmiSynopPlotView::GetLowCloudCoverStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiLowCloudCover);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		int code = static_cast<int>(value);
		std::string str(NFmiStringTools::Convert(code));
		return NFmiString(str);
	}
}

NFmiString NFmiSynopPlotView::GetCloudHeightStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiCloudHeight);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		int code = 0;
		if(value < 10)
			code = static_cast<int>(value); // jos arvo alle 10, k‰ytet‰‰n sit‰, t‰m‰ on luultavasti oikea synop koodi
		else
		{ // muuten muutetaan korkeus koodiksi
			if(value < 50)
				code = 0;
			else if(value < 100)
				code = 1;
			else if(value < 200)
				code = 2;
			else if(value < 300)
				code = 3;
			else if(value < 600)
				code = 4;
			else if(value < 1000)
				code = 5;
			else if(value < 1500)
				code = 6;
			else if(value < 2000)
				code = 7;
			else if(value < 2500)
				code = 8;
			else
				code = 9;
		}
		std::string str(NFmiStringTools::Convert(code));
		return NFmiString(str);
	}
}

NFmiString NFmiSynopPlotView::Get12or24HourRainAmountStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter)
{
	// t‰ss‰ pit‰‰ tarkistaa myˆs mik‰ on sade jakso. Jos joku muu kuin 12/24, palautetaan tyhj‰‰
	float period = GetSynopPlotValue(theInfo, kFmiPrecipitationPeriod);
	if(!(period == 12 || period == 24))
		return NFmiString("");

	float value = GetSynopPlotValue(theInfo, theWantedParameter);
	if(value == kFloatMissing)
		return NFmiString("");
	else if(value == 0)
		return NFmiString("-");
	else
	{
		NFmiString valStr;
		if(value > 0 && value < 1) // alle yhden lukema annetaan yhdell‰ desimaalilla
			valStr = NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, 1);
		else // yli yhden luvut annetaan ilman desimaalia
			valStr = NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, 0);

		if(period == 24)
			valStr += "/4";

		return valStr;
	}
}

NFmiString NFmiSynopPlotView::GetSnowDepthStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter)
{
	float value = GetSynopPlotValue(theInfo, theWantedParameter);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
        int intValue = boost::math::iround(value);
		return NFmiValueString(intValue, "%03d");
	}
}

NFmiString NFmiSynopPlotView::GetVisibilityStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiVisibility);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		int code = 0;
		if(value <= 5000)
			code = static_cast<int>(value) / 100;
		else if(value <= 30000)
			code = 50 + (static_cast<int>(value) / 1000);
		else if(value <= 70000)
			code = 80 + (static_cast<int>(value-30000) / 5000);
		else
			code = 89;
		std::string str(NFmiStringTools::Convert(code));
		return NFmiString(str);
	}
}

NFmiString NFmiSynopPlotView::GetAviVisStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiAviationVisibility);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		int intValue = static_cast<int>(value);
		if(intValue > 9999)
			intValue = 9999;
		std::string str(NFmiStringTools::Convert(intValue));
		return NFmiString(str);
	}
}

NFmiString NFmiSynopPlotView::GetPresentWeatherStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiPresentWeather);
	value = ::ConvertPossible_WaWa_2_WW(value);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(value < 4 || value > 99)
			return NFmiString("");
		else
		{
			int code = 157 + static_cast<int>(value);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

NFmiString NFmiSynopPlotView::GetWindGustStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiHourlyMaximumGust);
	if(value == kFloatMissing)
	{
		// Datan puuska voi olla eri parametrien alla
		value = GetSynopPlotValue(theInfo, kFmiWindGust);
	}

	if(value == kFloatMissing)
	{
		return NFmiString("");
	}
	else
	{
        int intValue = boost::math::iround(value * 2); // muutetaan solmuiksi kertomalla kahdella
		std::string str("G"); // lis‰t‰‰n etu liite G eli Gust
		str += NFmiStringTools::Convert(intValue);
		return NFmiString(str);
	}
}

namespace
{
	std::map<float, std::string> gAviWeatherSymbols;
};

using namespace std;

static void InitAviWeatherSymbolMap(std::map<float, std::string> &theAviWeatherSymbols)
{
	theAviWeatherSymbols.clear();

	// N‰m‰ METAR ww konversiot -> synop code on saatu Viljo Kangasniemen dokumentista:
	// "METAReiden purkamisen kuvaus lyhyt versio.doc"
	theAviWeatherSymbols.insert(make_pair(11.f, string("BCFG")));
	theAviWeatherSymbols.insert(make_pair(41.f, string("PRFG")));
	theAviWeatherSymbols.insert(make_pair(40.f, string("VCFG")));
	theAviWeatherSymbols.insert(make_pair(49.f, string("FG")));
	theAviWeatherSymbols.insert(make_pair(68.f, string("-RASN")));
	theAviWeatherSymbols.insert(make_pair(82.f, string("+SHRA")));
	theAviWeatherSymbols.insert(make_pair(87.f, string("SHGS")));
	theAviWeatherSymbols.insert(make_pair(97.f, string("+TSRA")));
	theAviWeatherSymbols.insert(make_pair(4.f, string("FU")));
	theAviWeatherSymbols.insert(make_pair(5.f, string("HZ")));
	theAviWeatherSymbols.insert(make_pair(6.f, string("DU")));
	theAviWeatherSymbols.insert(make_pair(7.f, string("BLSA")));
	theAviWeatherSymbols.insert(make_pair(8.f, string("PO")));
	theAviWeatherSymbols.insert(make_pair(9.f, string("VCDS")));
	theAviWeatherSymbols.insert(make_pair(10.f, string("BR")));
	theAviWeatherSymbols.insert(make_pair(12.f, string("MIFG")));
	theAviWeatherSymbols.insert(make_pair(13.f, string("VCTS")));
	theAviWeatherSymbols.insert(make_pair(16.f, string("VCSH"))); // 16 = Precipitation within sight, reaching the ground or the surface of the sea, near to, but not at the station
	theAviWeatherSymbols.insert(make_pair(17.f, string("TS")));
	theAviWeatherSymbols.insert(make_pair(18.f, string("SQ")));
	theAviWeatherSymbols.insert(make_pair(19.f, string("FC")));
	theAviWeatherSymbols.insert(make_pair(31.f, string("DS")));
	theAviWeatherSymbols.insert(make_pair(36.f, string("DRSN")));
	theAviWeatherSymbols.insert(make_pair(37.f, string("+DRSN")));
	theAviWeatherSymbols.insert(make_pair(38.f, string("BLSN")));
	theAviWeatherSymbols.insert(make_pair(39.f, string("+BLSN")));
	theAviWeatherSymbols.insert(make_pair(51.f, string("-DZ")));
	theAviWeatherSymbols.insert(make_pair(53.f, string("DZ")));
	theAviWeatherSymbols.insert(make_pair(55.f, string("+DZ")));
	theAviWeatherSymbols.insert(make_pair(56.f, string("-FZDZ")));
	theAviWeatherSymbols.insert(make_pair(57.f, string("FZDZ")));
	theAviWeatherSymbols.insert(make_pair(58.f, string("-RADZ")));
	theAviWeatherSymbols.insert(make_pair(59.f, string("RADZ")));
	theAviWeatherSymbols.insert(make_pair(61.f, string("-RA")));
	theAviWeatherSymbols.insert(make_pair(63.f, string("RA")));
	theAviWeatherSymbols.insert(make_pair(66.f, string("-FZRA")));
	theAviWeatherSymbols.insert(make_pair(67.f, string("FZRA")));
	theAviWeatherSymbols.insert(make_pair(69.f, string("RASN")));
	theAviWeatherSymbols.insert(make_pair(71.f, string("-SN")));
	theAviWeatherSymbols.insert(make_pair(73.f, string("SN")));
	theAviWeatherSymbols.insert(make_pair(75.f, string("+SN")));
	theAviWeatherSymbols.insert(make_pair(76.f, string("IC")));
	theAviWeatherSymbols.insert(make_pair(77.f, string("SG")));
	theAviWeatherSymbols.insert(make_pair(79.f, string("PL")));
	theAviWeatherSymbols.insert(make_pair(80.f, string("-SHRA")));
	theAviWeatherSymbols.insert(make_pair(81.f, string("SHRA")));
	theAviWeatherSymbols.insert(make_pair(83.f, string("-SHRASN")));
	theAviWeatherSymbols.insert(make_pair(84.f, string("SHRASN")));
	theAviWeatherSymbols.insert(make_pair(85.f, string("-SHSN")));
	theAviWeatherSymbols.insert(make_pair(86.f, string("SHSN")));
	theAviWeatherSymbols.insert(make_pair(88.f, string("SHGS")));
	theAviWeatherSymbols.insert(make_pair(89.f, string("-SHGR")));
	theAviWeatherSymbols.insert(make_pair(90.f, string("SHGR")));
	theAviWeatherSymbols.insert(make_pair(95.f, string("TSRA")));
	theAviWeatherSymbols.insert(make_pair(96.f, string("TSGR")));
	theAviWeatherSymbols.insert(make_pair(99.f, string("+TSGR")));
}


static std::string ConvertAviWeatherCode2String(float theCode)
{
	if(theCode == kFloatMissing)
		return "";
	else
	{
		if(gAviWeatherSymbols.empty())
			::InitAviWeatherSymbolMap(gAviWeatherSymbols);

		std::map<float, std::string>::iterator it = gAviWeatherSymbols.find(theCode);
		if(it != gAviWeatherSymbols.end())
			return it->second;
		return "";
	}
}

static float DoRounding(float theValue)
{
	float roundedValue = kFloatMissing;
	if(theValue < 9.5f)
		roundedValue = static_cast<float>(::round(theValue));
	else if(theValue < 95)
		roundedValue = ::round(theValue/10.f)*10.f;
	else
		roundedValue = ::round(theValue/100.f)*100.f;
	return roundedValue;
}

static float Meters2FeetsWithRounding(float theValueInMeters)
{
	if(theValueInMeters == kFloatMissing)
		return kFloatMissing;
	else
	{
		const float meter2feetFactor = 1.f/0.3048f;
		float feetValue = meter2feetFactor * theValueInMeters;
		float roundedFeetValue = ::DoRounding(feetValue);
		return roundedFeetValue;
	}
}

static float GetCloudBaseInFeets(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float cloudBaseInFeets = kFloatMissing;

	// 1. katsotaan onko vervis-arvoa ja k‰ytet‰‰n sit‰
	theInfo->Param(kFmiVerticalVisibility);
	float vervisInMeters = theInfo->FloatValue();
	if(vervisInMeters != kFloatMissing)
	{
		cloudBaseInFeets = ::Meters2FeetsWithRounding(vervisInMeters);
		return cloudBaseInFeets;

	}

	// 2. katsotaan onko ensimm‰isen pilvikerroksen alarajaa ja k‰ytet‰‰n sit‰
	theInfo->Param(kFmi1CloudBase);
	float cloudBase1InMeters = theInfo->FloatValue();
	if(cloudBase1InMeters != kFloatMissing)
	{
		cloudBaseInFeets = ::Meters2FeetsWithRounding(cloudBase1InMeters);
		return cloudBaseInFeets;

	}
	return cloudBaseInFeets;
}

static float GetAviVisibility(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	theInfo->Param(kFmiAviationVisibility);
	return theInfo->FloatValue();
}

static bool ParameterHasNonMissingValue(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, FmiParameterName parameter)
{
	if(theInfo->Param(parameter))
	{
		auto value = theInfo->FloatValue();
		return value != kFloatMissing;
	}
	else
		return false;
}

static bool HasAnySignificantCloudTypes(boost::shared_ptr<NFmiFastQueryInfo>& theInfo)
{
	if(::ParameterHasNonMissingValue(theInfo, kFmi1CloudType))
		return true;
	if(::ParameterHasNonMissingValue(theInfo, kFmi2CloudType))
		return true;
	if(::ParameterHasNonMissingValue(theInfo, kFmi3CloudType))
		return true;
	if(::ParameterHasNonMissingValue(theInfo, kFmi4CloudType))
		return true;

	return false;
}

static bool HasAnySignificantWeather(boost::shared_ptr<NFmiFastQueryInfo>& theInfo)
{
	if(::ParameterHasNonMissingValue(theInfo, kFmiAviationWeather1))
		return true;
	if(::ParameterHasNonMissingValue(theInfo, kFmiAviationWeather2))
		return true;
	if(::ParameterHasNonMissingValue(theInfo, kFmiAviationWeather3))
		return true;

	return false;
}

static bool IsCavok(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	// 1. Jos on n‰kyvyys on 9999 tai yli
	float visibilityInMeters = ::GetAviVisibility(theInfo);
	if(visibilityInMeters == kFloatMissing || visibilityInMeters >= 9999)
	{
		// 2.  Ja jos alin pilvi on yli 5000 jalkaa eik‰ ole vertical-visibility arvoa
		float cloudBaseInFeets = ::GetCloudBaseInFeets(theInfo); // t‰m‰ kattaa sek‰ ver-vis arvon ja bloud-basen
		if(cloudBaseInFeets == kFloatMissing || cloudBaseInFeets >= 5000)
		{
			// Jos kaikki merkitt‰v‰t parametrit olivat pelkk‰‰ puuttuvaa, ei voi sanoa ett‰ on CAVOK keli
			if(visibilityInMeters == kFloatMissing && cloudBaseInFeets == kFloatMissing)
				return false;

			// 3. Eik‰ saa olla pilvityyppein‰ Cb/Tcu pilvi‰ miss‰‰n kerroksessa
			if(!::HasAnySignificantCloudTypes(theInfo))
			{
				// 4. Eik‰ saa kent‰ll‰ tail‰heisyydess‰ mit‰‰n merkitt‰v‰‰ s‰‰t‰ (eli tarkistetaan ett‰ ei ole yht‰‰n mit‰‰n s‰‰t‰)
				if(!::HasAnySignificantWeather(theInfo))
				{
					return true;
				}
			}
		}
	}
	return false;
}

static bool IsSkc(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	theInfo->Param(kFmi1CloudType);
	float cloudtype1 = theInfo->FloatValue();
	if(cloudtype1 == 4) // cloudBase1:een on talletettu arvo 4 jos sky clear tilanne
		return true;
	return false;
}

static bool HasParamValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theParam)
{
	theInfo->Param(theParam);
	float value = theInfo->FloatValue();
	if(value != kFloatMissing)
		return true;
	else
		return false;
}

static std::string MetarCloudCoverCode2String(float theCoverValue)
{
	if(theCoverValue == kFloatMissing)
		return "";
	else
	{
		if(theCoverValue < 0)
			return "NEG"; // t‰m‰ on virhe koodi!!!
		else if(theCoverValue == 0)
			return "SKC"; // t‰m‰kin on virhe koodi!!!
		else if(theCoverValue < 3)
			return "FEW";
		else if(theCoverValue < 5)
			return "SCT";
		else if(theCoverValue < 8)
			return "BKN";
		else if(theCoverValue == 8)
			return "OVC";
		else
			return "OVCXX"; // t‰m‰kin on virhe koodi!!!
	}
}

static std::string GetAviationCloudStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theBaseHeightPar, FmiParameterName theCloudCoverPar, FmiParameterName theCloudTypePar)
{
	std::string str;
	theInfo->Param(theBaseHeightPar);
	float heightInFeets = ::Meters2FeetsWithRounding(theInfo->FloatValue());
	theInfo->Param(theCloudCoverPar);
	float cloudCover = theInfo->FloatValue();
	if(heightInFeets != kFloatMissing && cloudCover != kFloatMissing)
	{
        int heightInHectoFeets = boost::math::iround(heightInFeets / 100.f);
		str += ::MetarCloudCoverCode2String(cloudCover);
		if(heightInHectoFeets < 10)
			str += "0";
		if(heightInHectoFeets < 100)
			str += "0";
		str += NFmiStringTools::Convert(heightInHectoFeets);
		theInfo->Param(theCloudTypePar);
		float cloudType = theInfo->FloatValue();
		if(cloudType == 2)
			str += "TCU";
		if(cloudType == 3)
			str += "CB";
	}
	return str;
}


static NFmiString GetAviationCloudsStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	std::string str;

	// 1. jos cavok keli, laita se tekstiksi
	if(::IsCavok(theInfo))
		str = "CAVOK";
	// 2. jos skc keli, laita se tekstiksi
	else if(::IsSkc(theInfo))
		str = "SKC";
	// 3. jos vervis ilmoitettu, laita se
	else if(::HasParamValue(theInfo, kFmiVerticalVisibility))
	{
        int vervisInHectoFeets = boost::math::iround(::Meters2FeetsWithRounding(theInfo->FloatValue()) / 100.f);
		str = "VV";
		if(vervisInHectoFeets < 10)
			str += "0";
		if(vervisInHectoFeets < 100)
			str += "0";
		str += NFmiStringTools::Convert(vervisInHectoFeets);
	}
	// 4. muuten laita eri pilvi ryhm‰t omille riveilleen niin ett‰ 4. ryhm‰ ensin, siteen 3., 2. ja lopuksi 1. ja jokainen omille riveilleen
	else
	{
		std::string cloud4Str = ::GetAviationCloudStr(theInfo, kFmi4CloudBase, kFmi4CloudCover, kFmi4CloudType);
		std::string cloud3Str = ::GetAviationCloudStr(theInfo, kFmi3CloudBase, kFmi3CloudCover, kFmi3CloudType);
		std::string cloud2Str = ::GetAviationCloudStr(theInfo, kFmi2CloudBase, kFmi2CloudCover, kFmi2CloudType);
		std::string cloud1Str = ::GetAviationCloudStr(theInfo, kFmi1CloudBase, kFmi1CloudCover, kFmi1CloudType);
		if(cloud4Str.empty() == false)
		{
			if(str.empty() == false)
				str += "\r\n";
			str += cloud4Str;
		}
		if(cloud3Str.empty() == false)
		{
			if(str.empty() == false)
				str += "\r\n";
			str += cloud3Str;
		}
		if(cloud2Str.empty() == false)
		{
			if(str.empty() == false)
				str += "\r\n";
			str += cloud2Str;
		}
		if(cloud1Str.empty() == false)
		{
			if(str.empty() == false)
				str += "\r\n";
			str += cloud1Str;
		}
	}


	return NFmiString(str);
}

static NFmiString GetAviationStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{ // tulostetaan mahd. kaikki kolme s‰‰ kentt‰‰ metarista teksti muodossa
	theInfo->Param(kFmiAviationWeather1);
	float value1 = theInfo->FloatValue();
	theInfo->Param(kFmiAviationWeather2);
	float value2 = theInfo->FloatValue();
	theInfo->Param(kFmiAviationWeather3);
	float value3 = theInfo->FloatValue();

	if(value1 == kFloatMissing)
		return NFmiString("");
	else
	{
		std::string str = ::ConvertAviWeatherCode2String(value1);
		std::string value2Str = ::ConvertAviWeatherCode2String(value2);
		if(value2Str.empty() == false)
			str += " " + value2Str;
		std::string value3Str = ::ConvertAviWeatherCode2String(value3);
		if(value3Str.empty() == false)
			str += " " + value3Str;

		return NFmiString(str);
	}
}

NFmiString NFmiSynopPlotView::GetPressureTendencyStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiPressureTendency);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(value < 0 || value > 9)
			return NFmiString("");
		else
		{
			int code = 86 + static_cast<int>(value);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

// olettaa ett‰ pilvisyys tulee oktina
NFmiString NFmiSynopPlotView::GetTotalCloudinessStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiTotalCloudCover);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(value < 0 || value > 9)
			return NFmiString("");
		else
		{
			int code = 96 + static_cast<int>(value);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

NFmiString NFmiSynopPlotView::GetClStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiLowCloudType);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(value < 1 || value > 9)
			return NFmiString("");
		else
		{
			int code = 56 + static_cast<int>(value);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

NFmiString NFmiSynopPlotView::GetCmStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiMiddleCloudType);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(value < 1 || value > 9)
			return NFmiString("");
		else
		{
			int code = 66 + static_cast<int>(value);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

NFmiString NFmiSynopPlotView::GetStateOfGroundStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter)
{
	float value = GetSynopPlotValue(theInfo, theWantedParameter);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(value < 0 || value > 9)
			return NFmiString("");
		else
		{
			// 116 on offset E¥ parametrille kun maassa on lunta
			// 107 on offset E parametrille kun maassa ei ole lunta, 
			// huom! nyt 0 symbolia ei ole ja piirto alkaa symbolista 1
			// Mist‰ tiet‰‰ kumpi koodi k‰ytˆss‰?!?!?
			int code = 116 + static_cast<int>(value);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

NFmiString NFmiSynopPlotView::GetChStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiHighCloudType);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(value < 1 || value > 9)
			return NFmiString("");
		else
		{
			int code = 76 + static_cast<int>(value);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

NFmiString NFmiSynopPlotView::GetPastWeatherStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter)
{
	float value = GetSynopPlotValue(theInfo, theWantedParameter);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(value < 3 || value > 9)
			return NFmiString("");
		else
		{
			int code = 46 + static_cast<int>(value);
			std::string str;
			str += static_cast<unsigned char>(code);
			return NFmiString(str);
		}
	}
}

NFmiString NFmiSynopPlotView::GetPressureStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter, bool fDoMetarPlotString)
{
	float value = GetSynopPlotValue(theInfo, theWantedParameter);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		std::string str;
		if(fDoMetarPlotString)
		{
            int intValue = boost::math::iround(value);
			str = NFmiStringTools::Convert(intValue);
		}
		else
		{
			if(theWantedParameter == kFmiPressureChange)
				if(value < 0)
					value = -value; // luvut halutaan ilman miinus merkki‰ ja ne printataan punaisina, jos negatiivinen arvo
			value *= 10;
			int intValue = static_cast<int>(value) % 1000;
			str = NFmiStringTools::Convert(intValue);
			if(theWantedParameter == kFmiPressure)
			{ // lis‰t‰‰n paine tapauksessa etu nollia, jos paine esim. 1001.3 -> 013 ja jos 1000.3 -> 003
				if(str.length() == 1)
				{
					std::string tmp("00");
					tmp += str;
					str.swap(tmp);
				}
				else if(str.length() == 2)
				{
					std::string tmp("0");
					tmp += str;
					str.swap(tmp);
				}
			}
		}
		return NFmiString(str);
	}
}

float NFmiSynopPlotView::GetPressureChangeValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float valuePPP = GetSynopPlotValue(theInfo, kFmiPressureChange); // kokeillaan ensin lˆytyykˆ paineen muutos suoraan
	if(valuePPP != kFloatMissing)
		return valuePPP;

	theInfo->Param(kFmiPressure); // muuten lasketaan paineen muutos jos mahdollista, paineen muutos lasketaan paineesta 3h erotus
	float value1 = theInfo->FloatValue();
	NFmiMetTime previousTime(theInfo->Time());
	previousTime.ChangeByHours(-3); // lasketaan 3h paine erotus
	float value2 = theInfo->InterpolatedValue(previousTime);
	float value = (value1 != kFloatMissing && value2 != kFloatMissing) ? value1 - value2 : kFloatMissing;
	return value;
}

NFmiString NFmiSynopPlotView::GetPressureChangeStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter)
{
	float value = GetPressureChangeValue(theInfo);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(theWantedParameter == kFmiPressureChange)
			if(value < 0)
				value = -value; // luvut halutaan ilman miinus merkki‰ ja ne printataan punaisina, jos negatiivinen arvo
		value *= 10;
		value = static_cast<float>(round(value));
		int intValue = static_cast<int>(value) % 1000;
		std::string str(NFmiStringTools::Convert(intValue));
		if(theWantedParameter == kFmiPressure)
		{ // lis‰t‰‰n paine tapauksessa etu nollia, jos paine esim. 1001.3 -> 013 ja jos 1000.3 -> 003
			if(str.length() == 1)
			{
				std::string tmp("00");
				tmp += str;
				str.swap(tmp);
			}
			else if(str.length() == 2)
			{
				std::string tmp("0");
				tmp += str;
				str.swap(tmp);
			}
		}
		return NFmiString(str);
	}
}

// palauttaa halutun parametrin arvon siten ett‰ luku
// on leikattu maksimissaan kahteen lukuun ja siit‰ on poistettu desimaalit
NFmiString NFmiSynopPlotView::GetTemperatureStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter, bool fDoMetarPlotString)
{
	float value = GetSynopPlotValue(theInfo, theWantedParameter);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		std::string str;
		int intValue = static_cast<int>(round(value)) % 100;
		if(fDoMetarPlotString)
		{ // metar-plotissa negatiivisille arvoille laitetaan M-merkki eteen
			if(value < 0)
			{
				str += "M";
				intValue = ::abs(intValue); // ei laiteta miinus merkki‰, koska laitetaan M-merkki
			}
			if(intValue < 10)
				str += "0"; // laitetaan etu nolla, jos alle 10
			str += (NFmiStringTools::Convert(intValue));
		}
		else
		{
			str = NFmiStringTools::Convert(intValue);
		}
		return NFmiString(str);
	}
}

NFmiString NFmiSynopPlotView::GetHeightStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiGeomHeight);
	if(value == kFloatMissing)
	{ // kokeillaan viel‰ geomheight paramia
		theInfo->Param(kFmiGeopHeight);
		value = theInfo->FloatValue();
	}

	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		// korkeudesta 1440 leikataan pois 1 ja 0 eli vain keski luvut j‰tet‰‰n
		// t‰m‰ toimii ainakin 850 mbar korkeus arvoille, muita korkeuksia en osaa sanoa
		int intValue = FmiRound(value) % 1000; // FmiRound pit‰‰ olla t‰ss‰, koska std::round palauttaa float:in ja sen kanssa ei % -operaaattori k‰y VC++ 2013 k‰‰nt‰j‰ss‰
		intValue /= 10;
		std::string str(NFmiStringTools::Convert(intValue));
		return NFmiString(str);
	}
}

NFmiString NFmiSynopPlotView::GetMinMaxTStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theParam)
{
	float value = GetSynopPlotValue(theInfo, theParam);
	if(value == kFloatMissing)
		return NFmiString("");
	else
		return NFmiValueString(value, "%0.1f");
}

static NFmiString GetTminusTdSoundingStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	theInfo->Param(kFmiTemperature);
	float T = theInfo->FloatValue();
	theInfo->Param(kFmiDewPoint);
	float Td = theInfo->FloatValue();
	if(Td == kFloatMissing)
	{
		theInfo->Param(kFmiDewPoint2M); // luotaus-datassa kastepiste voi olla t‰m‰ parametri
		Td = theInfo->FloatValue();
	}
	if(T == kFloatMissing || Td == kFloatMissing)
		return NFmiString("");
	else
	{
		float diff = T - Td;
        int intValue = boost::math::iround(diff);
		std::string str(NFmiStringTools::Convert(intValue));
		return NFmiString(str);
	}
}

NFmiString NFmiSynopPlotView::GetPrecipitationAmountStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetSynopPlotValue(theInfo, kFmiPrecipitationAmount);
	if(value == kFloatMissing)
		return NFmiString("");
	else
	{
		if(value <= 0) // nollalle ei tehd‰ stringi‰
			return NFmiString("");
		else if(value > 0 && value < 1) // alle yhden lukema annetaan yhdell‰ desimaalilla
			return NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, 1);
		else // yli yhden luvut annetaan ilman desimaalia
			return NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, 0);
	}
}

void NFmiSynopPlotView::SetPressureChangeColor(NFmiDrawingEnvironment & theDrawingEnvi, boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	float value = GetPressureChangeValue(theInfo);
	if(value != kFloatMissing)
	{
		if(value >= 0)
			theDrawingEnvi.SetFrameColor(NFmiColor(0,0,0));
		else
			theDrawingEnvi.SetFrameColor(NFmiColor(0.78f, 0.08f, 0.06f));
	}
}

bool NFmiSynopPlotView::GetDataFromLocalInfo() const
{
    if(fGetSynopDataFromQ2)
        return false;
    else
        return true;
}

float NFmiSynopPlotView::GetSynopPlotValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theParam)
{
	if(fGetSynopDataFromQ2)
	{
		ParamIdSeekContainer::iterator itPar = itsSynopPlotParamIndexies.find(theParam);
		if(itPar != itsSynopPlotParamIndexies.end())
		{
			StationIdSeekContainer::iterator itLoc = itsSynopDataFromQ2StationIndexies.find(theInfo->Location()->GetIdent());
			if(itLoc != itsSynopDataFromQ2StationIndexies.end())
				return itsSynopDataValuesFromQ2[(*itPar).second][(*itLoc).second];
		}
		return kFloatMissing;
	}
	else
	{
        if(theParam == kFmiWindVectorMS && metaWindParamUsage.MakeMetaWindVectorParam())
        {
            return NFmiFastInfoUtils::GetMetaWindValue(theInfo, metaWindParamUsage, theParam);
        }
        else
        {
            theInfo->Param(theParam);
            return theInfo->FloatValue();
        }
	}
}

bool NFmiSynopPlotView::PrintParameterValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theSynopRect, const NFmiPoint &theRelLocation, FmiParameterName theWantedParameter, bool fDoMetarPlotString)
{
	NFmiPoint viewLocation(GetRelativeLocationFromRect(theSynopRect, theRelLocation));
	NFmiString str;
	switch(theWantedParameter)
	{
	case kFmiSnowDepth:
		str = GetSnowDepthStr(theInfo, theWantedParameter);
		break;
	case kFmiPressure:
		str = GetPressureStr(theInfo, theWantedParameter, fDoMetarPlotString);
		break;
	case kFmiStateOfGround:
		str = GetStateOfGroundStr(theInfo, theWantedParameter);
		break;
	case kFmiPressureChange:
		str = GetPressureChangeStr(theInfo, theWantedParameter);
		break;
	case kFmiVisibility:
		str = GetVisibilityStr(theInfo);
		break;
	case kFmiAviationVisibility:
		str = GetAviVisStr(theInfo);
		break;
	case kFmiCloudHeight:
		str = GetCloudHeightStr(theInfo);
		break;
	case kFmiPresentWeather:
		str = GetPresentWeatherStr(theInfo);
		break;
	case kFmiPastWeather1:
	case kFmiPastWeather2:
		str = GetPastWeatherStr(theInfo, theWantedParameter);
		break;
	case kFmiLowCloudType:
		str = GetClStr(theInfo);
		break;
	case kFmiMiddleCloudType:
		str = GetCmStr(theInfo);
		break;
	case kFmiHighCloudType:
		str = GetChStr(theInfo);
		break;
	case kFmiPressureTendency:
		str = GetPressureTendencyStr(theInfo);
		break;
	case kFmiHourlyMaximumGust:
		str = GetWindGustStr(theInfo);
		break;
	case kFmiAviationWeather1:
		str = GetAviationStr(theInfo);
		break;
	case kFmi1CloudBase:
		str = GetAviationCloudsStr(theInfo);
		break;
	case kFmiTotalCloudCover:
		str = GetTotalCloudinessStr(theInfo);
		break;
	case kFmiPrecipitationAmount:
		{
			if(fMinMaxPlotDraw)
				str = Get12or24HourRainAmountStr(theInfo, kFmiPrecipitationAmount);
			else
				str = GetPrecipitationAmountStr(theInfo);
			break;
		}
	case kFmiLowCloudCover:
		str = GetLowCloudCoverStr(theInfo);
		break;
	case kFmiDewPoint:
		{
			if(fSoundingPlotDraw == false)
			{
				str = GetTemperatureStr(theInfo, theWantedParameter, fDoMetarPlotString);
				if(str.GetLen() == 0)
					str = GetTemperatureStr(theInfo, kFmiDewPoint2M, fDoMetarPlotString); // luotaus datassa voi Td parametri olla t‰m‰
			}
			else
				str = GetTminusTdSoundingStr(theInfo);
			break;
		}
	case kFmiGeopHeight:
		str = GetHeightStr(theInfo);
		break;

	case kFmiMaximumTemperature:
	case kFmiMinimumTemperature:
		str = GetMinMaxTStr(theInfo, theWantedParameter);
		break;

	default: // t‰h‰n menee mm. T jne.
		str = GetTemperatureStr(theInfo, theWantedParameter, fDoMetarPlotString);
		break;
	}
	if(str.GetLen() > 0)
	{
		NFmiText text(viewLocation, str, false, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&text);
		return true;
	}
	else
		return false;
}

// Jos palautetun pair:in second arvo on false, ei piirret‰ statusta ollenkaan
static std::pair<NFmiColor, bool> GetMetarPlotStatusColor(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	const float bigCloudBaseValue = 5000; // t‰m‰ on helpottamassa puuttuvien arvojen tarkastelua
	const float bigAviVisValue = 10000; // t‰m‰ on helpottamassa puuttuvien arvojen tarkastelua
	NFmiColor statusColor(0, 0, 1); // sininen on normaali status

	float visibilityInMeters = ::GetAviVisibility(theInfo);
	float cloudBaseInFeets = ::GetCloudBaseInFeets(theInfo);
	if(visibilityInMeters == kFloatMissing && cloudBaseInFeets == kFloatMissing)
	{
		return std::make_pair(statusColor, false);
	}
		
	if(visibilityInMeters == kFloatMissing)
		visibilityInMeters = bigAviVisValue;
	if(cloudBaseInFeets == kFloatMissing)
		cloudBaseInFeets = bigCloudBaseValue;

	if(visibilityInMeters < 800 || cloudBaseInFeets < 200)
		statusColor = NFmiColor(1, 0, 0); // punainen huonoimmille s‰ille
	else if(visibilityInMeters < 1600 || cloudBaseInFeets < 300)
		statusColor = NFmiColor(1, 0.5f, 0); // oranssi seuraaville s‰ille
	else if(visibilityInMeters < 3700 || cloudBaseInFeets < 700)
		statusColor = NFmiColor(1, 1, 0); // keltainen seuraaville s‰ille
	else if(visibilityInMeters < 5000 || cloudBaseInFeets < 1500)
		statusColor = NFmiColor(0, 0.5f, 0); // vihre‰ seuraaville s‰ille
	else if(visibilityInMeters < 8000 || cloudBaseInFeets < 2500)
		statusColor = NFmiColor(1, 1, 1); // valkoinen seuraaville s‰ille

	return std::make_pair(statusColor, true);;
}

static bool HasAnyMetarPlotData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	if(::HasParamValue(theInfo, kFmiWindSpeedMS))
		return true;
	if(::HasParamValue(theInfo, kFmiTemperature))
		return true;
	if(::HasParamValue(theInfo, kFmiPressure))
		return true;
	if(::HasParamValue(theInfo, kFmi1CloudBase))
		return true;
	if(::HasParamValue(theInfo, kFmiAviationWeather1))
		return true;
	return false;
}

static bool DrawAviVis(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	// jos on CAVOK tai SKC tilanne, ei piirret‰ AviVis:ia
	if(::IsCavok(theInfo) || ::IsSkc(theInfo))
		return false;
	return true;
}

bool NFmiSynopPlotView::DrawMetarPlot(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theDrawRect)
{
	if(::HasAnyMetarPlotData(theInfo) == false)
		return false;

	const double topYpos = 0.90;
	const double middleYpos = 0.675;
	const double bottomYpos = 0.45;
	const double leftXpos = 0.35;
	const double rightXpos = 0.6;

	bool anythingDrawed = false;
	FmiFontType oldFont = itsDrawingEnvironment.GetFontType();
//	itsDrawingEnvironment.BoldFont(true);
	itsDrawingEnvironment.SetFontType(kNewber);
	itsDrawingEnvironment.SetFontSize(NFmiPoint(itsFontSizeX, itsFontSizeY));
	itsDrawingEnvironment.EnableFill();
	itsToolBox->SetTextAlignment(kRight); // tehd‰‰n ensin tekstit keskiosan vasemmalle puolelle right-alignmentilla

	NFmiMetarPlotSettings & metarSettings = itsCtrlViewDocumentInterface->SynopPlotSettings()->MetarPlotSettings();

	// 1. piirr‰ n‰kyvyyden ja cloudbae/vervis:in m‰‰r‰‰m‰ v‰ri boksi (keskelle)
	if(metarSettings.Show_Status())
	{
		NFmiPoint relativeStatusLocation(0.5, 0.5); // status-laatikon sijainti 0,0 1,1 maailmassa synop boxissa
		auto statusColorData = ::GetMetarPlotStatusColor(theInfo);
		if(statusColorData.second)
		{
			itsDrawingEnvironment.SetFillColor(statusColorData.first);
			double statusRectSizeFactor = 0.7;
			NFmiRect statusRect(0, 0, itsToolBox->SX(itsFontSizeX) * statusRectSizeFactor, itsToolBox->SY(itsFontSizeY) * statusRectSizeFactor);
			statusRect.Center(GetRelativeLocationFromRect(theDrawRect, relativeStatusLocation));
			NFmiRectangle statusRectangle(statusRect, 0, &itsDrawingEnvironment);
			itsToolBox->Convert(&statusRectangle);
			anythingDrawed |= true;
		}
	}

	// *******  defaulttina seuraavat piirret‰‰n mustana tai halutulla v‰rill‰ ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	itsDrawingEnvironment.SetFillColor(NFmiColor(0,0,0));
	if(metarSettings.UseSingleColor())
	{
		itsDrawingEnvironment.SetFrameColor(metarSettings.SingleColor());
		itsDrawingEnvironment.SetFillColor(metarSettings.SingleColor());
	}

	// 2. piirr‰ tuuli viiri (keskelle), sen piirto optiot testataan DrawWindVector funktiossa
	bool windDrawedDummy = false; // t‰ll‰ tiedolla ei tehd‰ mit‰‰n, se pit‰‰ kuitenkin antaa DrawWindVector-metodille
	anythingDrawed |= DrawWindVector(theInfo, theDrawRect, windDrawedDummy, true);

	// 3. piirr‰ l‰mpˆtila (vasen yl‰) ja kastepiste (vasen ala): arvot aina kahdella luvulla ja M-notaatio negatiivisille arvoille
	if(metarSettings.Show_TT())
	{
		NFmiPoint relativeTemperatureLocation(leftXpos, topYpos); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeTemperatureLocation, kFmiTemperature, true);
	}
	if(metarSettings.Show_TdTd())
	{
		NFmiPoint relativeDewPointLocation(leftXpos, bottomYpos); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeDewPointLocation, kFmiDewPoint, true);
	}

	// 4. piirr‰ Vis metrein‰ 1-4 numeroa (vasen keskelle) Huom. ei piirret‰, jos on CAVOK tai SKC tilanne
	if(metarSettings.Show_VVVV())
	{
		if(::DrawAviVis(theInfo))
		{
			NFmiPoint relativeDewPointLocation(leftXpos, middleYpos); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeDewPointLocation, kFmiAviationVisibility);
		}
	}

	itsToolBox->SetTextAlignment(kLeft); // tehd‰‰n seuraavat tekstit keskiosan oikealle puolelle left-alignmentilla
	// 5. piirr‰ QNH hPa:ina 3-4 numeroa (oikea yl‰)
	if(metarSettings.Show_PhPhPhPh())
	{
		NFmiPoint relativePressureLocation(rightXpos, topYpos); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativePressureLocation, kFmiPressure, true);
	}

	// 6. piirr‰ Gust solmuina vaikka G15 eli G ja 2-3 numeroa (oikea keskell‰)
	if(metarSettings.Show_Gff())
	{
		NFmiPoint relativeWindGustPointLocation(rightXpos, middleYpos); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeWindGustPointLocation, kFmiHourlyMaximumGust);
	}

	// 7. piirr‰ Wx s‰‰ teksti muodossa kaikki kent‰t per‰kk‰in "SHRA FG" (oikea alhaalla)
	if(metarSettings.Show_ww())
	{
		NFmiPoint relativeMetarWeatherPointLocation(rightXpos, bottomYpos); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeMetarWeatherPointLocation, kFmiAviationWeather1);
	}

	itsToolBox->SetTextAlignment(kCenter); // tehd‰‰n seuraavat tekstit keskiosan ala puolelle center-alignmentilla
	// 8. piirr‰ pilvi‰ kuvaavat kent‰t joko VV002, CAVOK, SKC tai 1-4 pilvikentt‰‰ alekkain OVC009 (alas keskell‰)
	if(metarSettings.Show_SkyInfo())
	{
		NFmiPoint relativeMetarCloudsPointLocation(0.5, bottomYpos - 0.2); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeMetarCloudsPointLocation, kFmi1CloudBase);
	}

	itsDrawingEnvironment.SetFontType(oldFont);
//	itsDrawingEnvironment.BoldFont(false);

	return anythingDrawed;
}

bool NFmiSynopPlotView::DrawMinMaxPlot(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theDrawRect)
{
	bool anythingDrawed = false;
	FmiFontType oldFont = itsDrawingEnvironment.GetFontType();
	itsDrawingEnvironment.BoldFont(true);
	itsDrawingEnvironment.SetFontType(kNewber);
	itsDrawingEnvironment.SetFontSize(NFmiPoint(itsFontSizeX, itsFontSizeY));
	itsToolBox->SetTextAlignment(kLeft);

	NFmiSynopPlotSettings & synopSettings = *itsCtrlViewDocumentInterface->SynopPlotSettings();

	// *******  defaulttina seuraavat piirret‰‰n punaisina tai halutulla v‰rill‰ ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(1,0,0));
	if(synopSettings.UseSingleColor())
		itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());


	// piirret‰‰n maksimi l‰mpˆtila
	NFmiPoint relativeLocation(0.56, 0.77); // max T sijainti 0,0 1,1 maailmassa synop boxissa
	anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeLocation, kFmiMaximumTemperature);

	// piirret‰‰n minimi l‰mpˆtila
	relativeLocation = NFmiPoint(0.56, 0.53); // min T sijainti 0,0 1,1 maailmassa synop boxissa
	anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeLocation, kFmiMinimumTemperature);

	// *******  defaulttina seuraavat piirret‰‰n vihert‰v‰ll‰ tai halutulla v‰rill‰ ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.16f, 0.64f, 0.30f));
	if(synopSettings.UseSingleColor())
		itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

	// piirret‰‰n lumensyvyys
	relativeLocation = NFmiPoint(0.6, 0.30); // Tg min sijainti 0,0 1,1 maailmassa synop boxissa
	anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeLocation, kFmiSnowDepth);

	// piirret‰‰n 12/24h sadekertym‰
	relativeLocation = NFmiPoint(0.52, 0.95); // Tg min sijainti 0,0 1,1 maailmassa synop boxissa
	anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeLocation, kFmiPrecipitationAmount);

	// *******  defaulttina seuraavat piirret‰‰n mustina tai halutulla v‰rill‰ ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	if(synopSettings.UseSingleColor())
		itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

	itsToolBox->SetTextAlignment(kRight);
	// piirret‰‰n maanpinta minimi l‰mpˆtila
	relativeLocation = NFmiPoint(0.44, 0.41); // Tg min sijainti 0,0 1,1 maailmassa synop boxissa
	anythingDrawed |= PrintParameterValue(theInfo, theDrawRect, relativeLocation, kFmiGroundTemperature);

/*  // Toistaiseksi ei ole maanpinnan latu parametria k‰ytˆss‰, joten piirto koodi on kommentissa, koska
	// en ole varma toimiiko se kun parametri sitten joskus ilmestyy. Kokeile poistamalla kommentit.

	// *******  seuraavat piirret‰‰n mustina tai halutulla v‰rill‰ synop-fontilla ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	if(synopSettings.UseSingleColor())
		itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

	itsDrawingEnvironment.SetFontType(kSynop);
	itsToolBox->SetTextAlignment(kRight);
	// piirret‰‰n maanpinta minimi l‰mpˆtila
	relativeLocation = NFmiPoint(0.47, 0.64); // maanpinnan laadun sijainti 0,0 1,1 maailmassa synop boxissa
	anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeLocation, kFmiStateOfGround);
*/
	itsDrawingEnvironment.SetFontType(oldFont);
	itsDrawingEnvironment.BoldFont(false);

	return anythingDrawed;
}

bool NFmiSynopPlotView::DrawNormalFontValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theRect)
{
	bool anythingDrawed = false;
	FmiFontType oldFont = itsDrawingEnvironment.GetFontType();
	itsDrawingEnvironment.SetFontType(kNewber);
	itsDrawingEnvironment.SetFontSize(NFmiPoint(itsFontSizeX, itsFontSizeY));
	itsToolBox->SetTextAlignment(kCenter);

	NFmiSynopPlotSettings & synopSettings = *itsCtrlViewDocumentInterface->SynopPlotSettings();

	// *******  defaulttina seuraavat piirret‰‰n mustina tai halutulla v‰rill‰ ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	if(synopSettings.UseSingleColor())
		itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

	if(fSoundingPlotDraw == false)
	{ // luotaus datasta ei lˆydy kaikkia parametreja tai niit‰ ei ole tarkoitus printata
		if(synopSettings.ShowPPPP())
		{
			NFmiPoint relativePressureLocation(0.74, 0.87); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theRect, relativePressureLocation, kFmiPressure);
		}

		if(synopSettings.ShowV())
		{
			NFmiPoint relativeVisibilityLocation(0.13, 0.62); // n‰kyvyyden sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeVisibilityLocation, kFmiVisibility);
		}

		if(synopSettings.ShowH())
		{
			NFmiPoint relativeCloudHeightLocation(0.5, 0.25); // pilven korkeuden sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeCloudHeightLocation, kFmiCloudHeight);
		}

		if(synopSettings.ShowNh())
		{
			NFmiPoint relativeClCoverLocation(0.62, 0.4); // alapilven m‰‰r‰n sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeClCoverLocation, kFmiLowCloudCover);
		}

		if(synopSettings.ShowPpp())
		{
			// *******  piirret‰‰n joko punaisena (laskeva) tai mustana (tasainen/nouseva)
			SetPressureChangeColor(itsDrawingEnvironment, theInfo);
			if(synopSettings.UseSingleColor())
				itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

			NFmiPoint relativePreChangeLocation(0.67, 0.62); // paineen muutoksen sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theRect, relativePreChangeLocation, kFmiPressureChange);
		}

		if(synopSettings.ShowRr())
		{
			// *******  defaulttina seuraavat piirret‰‰n vihre‰n‰ tai halutulla v‰rill‰ ***************
			itsDrawingEnvironment.SetFrameColor(NFmiColor(0.137f, 0.627f, 0.3f));
			if(synopSettings.UseSingleColor())
				itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

			NFmiPoint relativePrecipitationAmountLocation(0.74, 1.05); // sadem‰‰r‰n sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theRect, relativePrecipitationAmountLocation, kFmiPrecipitationAmount);
		}
	} // piirret‰‰n vain maanpinta synop-plotissa

	// *******  defaulttina seuraavat piirret‰‰n punaisina tai halutulla v‰rill‰ ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.78f, 0.08f, 0.06f));
	if(synopSettings.UseSingleColor())
		itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

	if(synopSettings.ShowT())
	{
		NFmiPoint relativeTemperatureLocation(0.265, 0.8); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeTemperatureLocation, kFmiTemperature);
	}

	if(fSoundingPlotDraw == false)
	{
		if(synopSettings.ShowTd())
		{
			NFmiPoint relativeDewPointLocation(0.265, 0.45); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeDewPointLocation, kFmiDewPoint);
		}
	}
	else
	{ // luotauksissa piirret‰‰n T - Td erotus vihre‰ll‰
		if(synopSettings.ShowTd()) // vaikka kyse on T-Td arvosta, k‰ytet‰‰n t‰t‰ ehtoa
		{
			// *******  defaulttina seuraavat piirret‰‰n vihre‰n‰ tai halutulla v‰rill‰ ***************
			itsDrawingEnvironment.SetFrameColor(NFmiColor(0.137f, 0.627f, 0.3f));
			if(synopSettings.UseSingleColor())
				itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

			NFmiPoint relativeDewPointLocation(0.265, 0.45); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeDewPointLocation, kFmiDewPoint); // haetaan dewpoint paramilla, osaa tehd‰ kikan kun kyse luotaus-piirrosta

			// piirret‰‰n viel‰ samaa kyyti‰ korkeus arvo mustalla
			// *******  defaulttina seuraavat piirret‰‰n mustana tai halutulla v‰rill‰ ***************
			itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f, 0.f, 0.f));
			if(synopSettings.UseSingleColor())
				itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

			relativeDewPointLocation = NFmiPoint(0.72, 0.62); // paineen sijainti 0,0 1,1 maailmassa synop boxissa
			anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeDewPointLocation, kFmiGeopHeight);

		}
	}

	itsDrawingEnvironment.SetFontType(oldFont);

	return anythingDrawed;
}


bool NFmiSynopPlotView::DrawSynopFontValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theRect, bool fWindDrawed)
{
	bool anythingDrawed = false;
	if(fSoundingPlotDraw)
		return anythingDrawed; // n‰it‰ juttuja ei lˆydy luotaus datasta

	itsDrawingEnvironment.BoldFont(true);
	FmiFontType oldFont = itsDrawingEnvironment.GetFontType();
	itsDrawingEnvironment.SetFontType(kSynop);
	itsDrawingEnvironment.SetFontSize(NFmiPoint(itsFontSizeX, itsFontSizeY));
	itsToolBox->SetTextAlignment(kCenter);

	NFmiSynopPlotSettings & synopSettings = *itsCtrlViewDocumentInterface->SynopPlotSettings();

	// *******  defaulttina seuraavat piirret‰‰n mustina tai halutulla v‰rill‰ ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	if(synopSettings.UseSingleColor())
		itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

	if(synopSettings.ShowWw())
	{
		NFmiPoint relativePresentWeatherLocation(0.33, 0.62); // nyky s‰‰n sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theRect, relativePresentWeatherLocation, kFmiPresentWeather);
	}

	if(synopSettings.ShowCl())
	{
		NFmiPoint relativeClLocation(0.5, 0.43); // Cl-pilvi tyypin sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeClLocation, kFmiLowCloudType);
	}

	if(synopSettings.ShowCm())
	{
		NFmiPoint relativeCmLocation(0.5, 0.83); // Cm-pilvi tyypin sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeCmLocation, kFmiMiddleCloudType);
	}

	if(synopSettings.ShowCh())
	{
		NFmiPoint relativeChLocation(0.5, 0.99); // Ch-pilvi tyypin sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeChLocation, kFmiHighCloudType);
	}

	if(synopSettings.ShowA())
	{
		// *******  piirret‰‰n joko punaisena (laskeva) tai mustana (tasainen/nouseva) tai halutulla v‰rill‰ ***************
		SetPressureChangeColor(itsDrawingEnvironment, theInfo);
		if(synopSettings.UseSingleColor())
			itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

		NFmiPoint relativePressureTendencyLocation(0.85, 0.62); // paine tendessin sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theRect, relativePressureTendencyLocation, kFmiPressureTendency);
	}

	// *******  defaulttina seuraavat piirret‰‰n punaisina tai halutulla v‰rill‰ ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.78f, 0.08f, 0.06f));
	if(synopSettings.UseSingleColor())
		itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

	if(synopSettings.ShowW1())
	{
		NFmiPoint relativeW1Location(0.74, 0.4); // menneen s‰‰ 1 sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeW1Location, kFmiPastWeather1);
	}

	if(synopSettings.ShowW2())
	{
		NFmiPoint relativeW2Location(0.85, 0.4); // menneen s‰‰ 2 sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theRect, relativeW2Location, kFmiPastWeather2);
	}

	// *******  defaulttina seuraavat piirret‰‰n mustina tai halutulla v‰rill‰ ***************
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	if(synopSettings.UseSingleColor())
		itsDrawingEnvironment.SetFrameColor(synopSettings.SingleColor());

	if(synopSettings.ShowN())
	{
		// pilvisyys symboli pit‰‰ pohjustaa valkoisell‰ ympyr‰ll‰, koska tuuli viiri muuten sotkee kuvaa
		if(fWindDrawed) // ei kuitenkaan piirret‰ pohjustus ympyr‰‰, jos tuuliviiri‰ ei piirretty
		{
			NFmiString str = GetTotalCloudinessStr(theInfo);
			if(str != NFmiString("")) // ei piirret‰ pohjustusta, jos ei piirret‰ pilvisyytt‰k‰‰n
			{
				NFmiPoint relativeWindLocation(0.505, 0.5); // tuulivektorin sijainti 0,0 1,1 maailmassa synop boxissa
				itsDrawingEnvironment.EnableFill();
				itsDrawingEnvironment.DisableFrame();
				double circleSizeFactor = 0.67;
				NFmiRect circleRect(0, 0, itsToolBox->SX(itsFontSizeX) * circleSizeFactor, itsToolBox->SY(itsFontSizeY)* circleSizeFactor);
				circleRect.Center(GetRelativeLocationFromRect(theRect, relativeWindLocation));
				itsToolBox->DrawEllipse(circleRect, &itsDrawingEnvironment);
				itsDrawingEnvironment.DisableFill();
				itsDrawingEnvironment.EnableFrame();
				anythingDrawed = true;
			}
		}
	}
	// kokonais pilvisyyden symbolia pit‰‰ isontaa, ett‰ homma n‰ytt‰‰ oikealta
	if(synopSettings.ShowN())
	{
		double cloudSymbolFactor = 1.5;
		itsDrawingEnvironment.SetFontSize(NFmiPoint(itsFontSizeX * cloudSymbolFactor, itsFontSizeY * cloudSymbolFactor));
		NFmiPoint relativetotalCloudinessLocation(0.5, 0.70); // kokonais pilvisyyden sijainti 0,0 1,1 maailmassa synop boxissa
		anythingDrawed |= PrintParameterValue(theInfo, theRect, relativetotalCloudinessLocation, kFmiTotalCloudCover);
	}

	itsDrawingEnvironment.BoldFont(false);
	itsDrawingEnvironment.SetFontType(oldFont);

	return anythingDrawed;
}

// Laske kahden pisteen avulla lasketun suoran avulla annetun y:n arvo x-akselilla.
// Arvon min ja max arvot voidaan rajata, jos ei, ne ovat missing arvoja
static double CalcLineValue(double y, double x1, double y1, double x2, double y2, double minValue = kFloatMissing, double maxValue = kFloatMissing)
{
	// line function is y = ax + b
	// we calculate here new x value: x = (y-b)/a
	// if two points make line, then a and b are the following
	// b = (y2*x1-y1*x2)/(x1-x2)
	// a = (y1-b)/x1
	double b = (y2 * x1 - y1 * x2) / (x1 - x2);
	double a = (y1 - b) / x1;
	double newX = (y-b)/a;
	if(minValue != kFloatMissing)
		newX = FmiMax(newX, minValue);
	if(maxValue != kFloatMissing)
		newX = FmiMin(newX, maxValue);
	return newX;
}

// paluu arvo kertoo onko mit‰‰n piirretty
bool NFmiSynopPlotView::DrawWindVector(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theRect, bool &fWindDrawed, bool metarCase)
{
	fWindDrawed = false;
	NFmiSynopPlotSettings& synopSettings = *itsCtrlViewDocumentInterface->SynopPlotSettings();
	NFmiMetarPlotSettings & metarSettings = synopSettings.MetarPlotSettings();
	bool drawWind = metarCase ? metarSettings.Show_dddff() : synopSettings.ShowDdff();
	if(drawWind)
	{
		float windSpeed = kFloatMissing;
		float windDir = kFloatMissing;
		GetWindValues(theInfo, windSpeed, windDir);
		if(windSpeed != kFloatMissing && windDir != kFloatMissing)
		{
			// *******  defaulttina seuraavat piirret‰‰n mustina tai halutulla v‰rill‰ *************
			itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
			NFmiColor oldFillcolor(itsDrawingEnvironment.GetFillColor());
			bool useSingleColor = metarCase ? metarSettings.UseSingleColor() : synopSettings.UseSingleColor();
			if(useSingleColor)
			{
				const auto &singleColor = metarCase ? metarSettings.SingleColor() : synopSettings.SingleColor();
				itsDrawingEnvironment.SetFrameColor(singleColor);
			}
			itsDrawingEnvironment.SetFillColor(itsDrawingEnvironment.GetFrameColor());
			itsDrawingEnvironment.EnableFill();

			// tehd‰‰n pohjois korjaus tuuliviirin piirtoon
            NFmiPoint latlon = CurrentLatLon(theInfo);
            if(latlon != NFmiPoint::gMissingLatlon)
            { // jos jostain syyst‰ olisi puuttuva latlon-piste, ei tehd‰ korjausta
			    NFmiAngle ang(itsArea->TrueNorthAzimuth(latlon));
			    windDir += static_cast<float>(ang.Value());
            }

			fWindDrawed = true;
			NFmiPoint relativeWindLocation(0.5, 0.5); // tuulivektorin sijainti 0,0 1,1 maailmassa synop boxissa
			NFmiPoint viewWindLocation(GetRelativeLocationFromRect(theRect, relativeWindLocation));
			NFmiRect rect(0, 0, theRect.Height(), theRect.Height()); // huom! tarkoituksella molemmat height:eja, koska haluan neliˆn pohjaksi
			rect.Center(viewWindLocation);

			if(fSoundingPlotDraw)
			{ // sounding plotissa laitetaan pieni ympyr‰ aseman kohdalle
				double circleSizeFactor = 0.35;
				NFmiRect circleRect(0, 0, itsToolBox->SX(itsFontSizeX) * circleSizeFactor, itsToolBox->SY(itsFontSizeY)* circleSizeFactor);
				circleRect.Center(GetRelativeLocationFromRect(theRect, relativeWindLocation));
				itsToolBox->DrawEllipse(circleRect, &itsDrawingEnvironment);
			}

			auto fontSizeInMM = metarCase ? metarSettings.FontSize() : synopSettings.FontSize();
			double windBarbLineWidthFactor = ::CalcLineValue(fontSizeInMM, 8., 2., 12., 7., 7., 13.);
			int lineWidthInPixels = boost::math::iround(itsFontSizeX / windBarbLineWidthFactor);
			NFmiPoint oldSize = itsDrawingEnvironment.GetPenSize();
			itsDrawingEnvironment.SetPenSize(NFmiPoint(lineWidthInPixels, lineWidthInPixels));

			NFmiPoint symbolSize(1, 1);
			NFmiWindBarb(windSpeed
						,windDir
						,rect
						,itsToolBox
                        ,latlon.Y() < 0
						,symbolSize.X() * 0.5
						,symbolSize.Y() * 0.35
						,0
						,&itsDrawingEnvironment).Build();
			itsDrawingEnvironment.SetPenSize(oldSize);
			itsDrawingEnvironment.SetFillColor(oldFillcolor);
			itsDrawingEnvironment.DisableFill();
			if(windSpeed < 1)
			{ // jos tyynt‰, piirret‰‰n ympyr‰ kuvaamaan tyynt‰
				double circleSizeFactor = 1.2;
				NFmiRect circleRect(0, 0, itsToolBox->SX(itsFontSizeX) * circleSizeFactor, itsToolBox->SY(itsFontSizeY)* circleSizeFactor);
				circleRect.Center(GetRelativeLocationFromRect(theRect, relativeWindLocation));
				itsToolBox->DrawEllipse(circleRect, &itsDrawingEnvironment);
			}
		}
		else
			fWindDrawed = false;
	}
	return fWindDrawed;
}

static NFmiString Value2String(float theValue, int theDigitCount)
{
	NFmiString str;
	if(theDigitCount == 0)
        str = NFmiValueString(boost::math::iround(theValue), "%d");
	else
	{
		NFmiString format("%0.");
		format += NFmiValueString(theDigitCount, "%d");
		format += "f";
		str = NFmiValueString(theValue, format);
	}
	return str;
}

std::string NFmiSynopPlotView::ComposeToolTipText(const NFmiPoint & /* theRelativePoint */ )
{
	std::string str;
	if(itsDrawParam)
	{
		std::string paramNameStr;
		bool isSoundingPlot = itsDrawParam->Param().GetParamIdent() == NFmiInfoData::kFmiSpSoundingPlot;
		bool isMinMaxPlot = itsDrawParam->Param().GetParamIdent() == NFmiInfoData::kFmiSpMinMaxPlot;
		bool isMetarPlot = itsDrawParam->Param().GetParamIdent() == NFmiInfoData::kFmiSpMetarPlot;
		if(isSoundingPlot)
		{
			paramNameStr += "Temp ";
			paramNameStr += NFmiStringTools::Convert<float>(itsDrawParam->Level().LevelValue());
			paramNameStr += " mbar";
		}
		else if(isMinMaxPlot)
			paramNameStr += "MinMax";
		else if(isMetarPlot)
			paramNameStr += "Metar";
		else
		{
			paramNameStr += "Synop";
		}

		auto fontColor = CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData());
		paramNameStr = AddColorTagsToString(paramNameStr, fontColor, true);
		str += paramNameStr;
		str += " ";
		NFmiLocation loc(itsCtrlViewDocumentInterface->ToolTipLatLonPoint());
		boost::shared_ptr<NFmiFastQueryInfo> info;
		if(isSoundingPlot)
		{
			itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false); // t‰m‰kin pit‰‰ viel‰ jostain syyst‰ asettaa
			if(itsInfo == 0)
				return str;
			itsInfo->Location(loc.GetLocation());
			itsInfo->Time(itsCtrlViewDocumentInterface->ToolTipTime());
			// time ja location pit‰‰ asettaa ensin ennekuin voidaan etsi‰ leveli‰!!!
			::SetLevel(itsDrawParam, *itsInfo);
			info = itsInfo;
		}
		else if(isMetarPlot)
		{
			itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false); // t‰m‰kin pit‰‰ viel‰ jostain syyst‰ asettaa
			if(itsInfo == 0)
				return str;
			itsInfo->Location(loc.GetLocation());
			itsInfo->Time(itsCtrlViewDocumentInterface->ToolTipTime());
			info = itsInfo;
		}
		else
			info = itsCtrlViewDocumentInterface->GetNearestSynopStationInfo(loc, itsCtrlViewDocumentInterface->ToolTipTime(), false, 0);

		if(info)
		{
			if(info->IsLocation())
			{
				str += " (";
				NFmiValueString statIdStr(static_cast<int>(info->Location()->GetIdent()), "%d");
				str += statIdStr.CharPtr();
				str += " ";
				str += info->Location()->GetName();
				str += ")";
			}
			// Laitetaan t‰h‰n tagi kuvalle joka luodaan toisaalla
			str += "\n";
			if(isSoundingPlot)
				str += "#sounding-plot-image#"; // t‰m‰ on place holderi bitmapille joka tulee  sitten tooltippiin
			else if(isMinMaxPlot)
				str += "#minmax-plot-image#"; // t‰m‰ on place holderi bitmapille joka tulee  sitten tooltippiin
			else if(isMetarPlot)
				str += "#metar-plot-image#"; // t‰m‰ on place holderi bitmapille joka tulee  sitten tooltippiin
			else
				str += "#synop-plot-image#"; // t‰m‰ on place holderi bitmapille joka tulee  sitten tooltippiin
		}
	}
	return str;
}
