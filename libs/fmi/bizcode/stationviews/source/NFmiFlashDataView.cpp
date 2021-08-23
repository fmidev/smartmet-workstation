#include "NFmiFlashDataView.h"
#include "NFmiToolBox.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiPolyline.h"
#include "NFmiValueString.h"
#include "NFmiArea.h"
#include "CtrlViewDocumentInterface.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "ToolBoxStateRestorer.h"

//_________________________________________________________ NFmiFlashDataView

NFmiFlashDataView::NFmiFlashDataView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
									,NFmiToolBox * theToolBox
									,NFmiDrawingEnvironment * theDrawingEnvi
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
,itsNegativeFlashPolyLine(0)
,itsPositiveFlashPolyLine(0)
,itsCloudFlashPolyLine(0)
{
}

NFmiFlashDataView::~NFmiFlashDataView(void)
{
	delete itsNegativeFlashPolyLine;
	delete itsPositiveFlashPolyLine;
	delete itsCloudFlashPolyLine;
}

void NFmiFlashDataView::Draw(NFmiToolBox * theGTB)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    if(!IsParamDrawn())
		return;

	itsDrawingEnvironment->EnableFill();
	itsDrawingEnvironment->EnableFrame();

	NFmiColor negativeColor(0.f, 0.f, 0.9f); // v�ri, jolla kehystet��n negatiiviset salamat
	NFmiColor positiveColor(0.9f, 0.f, 0.f); // v�ri jolla kehystet��n positiiviset salamat ruudussa
	NFmiColor cloudColor(0.f, 0.7f, 0.f); // v�ri jolla kehystet��n pilvi salamat ruudussa
	CreateFlashPolyLines();
	itsPositiveFlashPolyLine->GetEnvironment()->SetFrameColor(positiveColor);
	itsNegativeFlashPolyLine->GetEnvironment()->SetFrameColor(negativeColor);
	itsCloudFlashPolyLine->GetEnvironment()->SetFrameColor(cloudColor);
    MakeDrawedInfoVector();
	// Asetetaan clippaus yhden kerran salama piirrossa p��lle.
	ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsRect);
	itsToolBox->SetUpClipping();
	// Clippaus pit�� sitten laittaa "pois p��lt�" (TurnClippingOffHelper:in avulla), muuten tulee p��llekk�isi� 
	// clippaus asetuksia eri salamatyyppien polyline piirron sis�ll�, ja ohjelma kaatuu...
	// T�m� pit�� tehd� erillisen scopen sis�ll�, jotta TurnClippingOffHelper destruktori kutsutaan ennen itsToolBox->EndClipping kutsua.
	{
		TurnClippingOffHelper turnClippingOffHelper(itsToolBox);

		for(size_t j = 0; j < itsInfoVector.size(); j++)
		{
			boost::shared_ptr<NFmiFastQueryInfo>& info = itsInfoVector[j];
			DrawFlashes(*info);
		}
	}
	// Lopuksi otetaan clippaus pois
	itsToolBox->EndClipping();
}

void NFmiFlashDataView::DrawFlashes(NFmiFastQueryInfo &theInfo)
{
	NFmiColor startColor(0.9f, 0.9f, 0.9f); // v�ri, jolla v�rj�t��n 'vanhat' salamat ruudussa
	NFmiColor endColor(itsDrawParam->FillColor()); // v�ri jolla v�rj�t��n 'tuoreimmat' salamat ruudussa
	CalculateGeneralStationRect();
	NFmiRect rect(GeneralStationRect());
	float lat = 0;
	float lon = 0;
    float power = 0;
    float multiplicity = 0;
	theInfo.Param(kFmiLatitude);
	unsigned long latIndex = theInfo.ParamIndex();
	theInfo.Param(kFmiLongitude);
	unsigned long lonIndex = theInfo.ParamIndex();
	theInfo.Param(kFmiFlashStrength);
	unsigned long powerIndex = theInfo.ParamIndex();
    theInfo.Param(kFmiFlashMultiplicity);
    unsigned long multiplicityIndex = theInfo.ParamIndex();
    theInfo.First();
	// Piirt�� t�st� kartan hetkesta aika-aseleen verran taaksep�in salamn iskuja

	unsigned long timeIndex1 = 0;
	unsigned long timeIndex2 = 0;
	if(GetTimeSpanIndexies(theInfo, timeIndex1, timeIndex2))
	{
		int editorTimeStep = CalcUsedTimeStepInMinutes();
		NFmiMetTime time1 = CalcFirstTimeOfSpan();
		NFmiPoint scale(itsDrawParam->OnlyOneSymbolRelativeSize());
		// k�y l�pi halutut salamat (olettaen ett� muut asetukset ovat kohdallaan)
		for(unsigned long i=timeIndex1; i<=timeIndex2; i++)
		{
			theInfo.TimeIndex(i);
			theInfo.ParamIndex(latIndex);
			lat = theInfo.FloatValue();
			theInfo.ParamIndex(lonIndex);
			lon = theInfo.FloatValue();
			theInfo.ParamIndex(powerIndex);
			power = theInfo.FloatValue();
            theInfo.ParamIndex(multiplicityIndex);
            multiplicity = theInfo.FloatValue();
            NFmiPoint latlon(lon, lat);

			if (itsArea->IsInside(latlon))
			{
				// lasketaan luku (0-1), jonka avulla v�rj�t��n salama
				float flashColorFactor = CalcFlashColorFactor(time1, theInfo.Time(), editorTimeStep);
				NFmiColor fillColor(CalcFlashFillColor(startColor, endColor, flashColorFactor));
				NFmiPoint viewPoint(LatLonToViewPoint(NFmiPoint(lon, lat))); // t�m� on offset
                if(multiplicity == kFloatMissing)
                {
                    itsCloudFlashPolyLine->GetEnvironment()->SetFillColor(fillColor);
                    itsToolBox->DrawPolyline(itsCloudFlashPolyLine, viewPoint, scale);
                }
                else if(power > 0)
				{
					itsPositiveFlashPolyLine->GetEnvironment()->SetFillColor(fillColor);
					itsToolBox->DrawPolyline(itsPositiveFlashPolyLine, viewPoint, scale);
				}
				else if(power < 0)
				{
					itsNegativeFlashPolyLine->GetEnvironment()->SetFillColor(fillColor);
					itsToolBox->DrawPolyline(itsNegativeFlashPolyLine, viewPoint, scale);
				}
			}
		}
	}
}

void NFmiFlashDataView::CreateFlashPolyLines(void)
{
	if(itsNegativeFlashPolyLine && itsPositiveFlashPolyLine && itsCloudFlashPolyLine)
		return ;
	// dafaultti koko on 18 pikseli� eli 20 yksikk�� (symbolit
	// menev�t -10:st� 10:een) -> 18 pikseli�.
	float xScale = static_cast<float>(itsToolBox->SX(1));
	float yScale = static_cast<float>(itsToolBox->SY(1));

	// N�m� plus ja miinus merkkien pisteet pit�� laskea n�in erikseen, muuten release versio v��rist�� merkkej� (debug versio toimii ok joka tapauksessa?!?!?!?)
	double x1 = -10*xScale;
	double y1 = -4*yScale;
	double x2 = 10*xScale;
	double y2 = 4*xScale;
	double x3 = -4*xScale;
	double x4 = 4*xScale;
	double y3 = -10*yScale;
	double y4 = 10*yScale;
	NFmiPoint p1(x1, y1);
	NFmiPoint p2(x2, y1);
	NFmiPoint p3(x2, y2);
	NFmiPoint p4(x1, y2);
	NFmiPoint p5(x3, y1);
	NFmiPoint p6(x3, y3);
	NFmiPoint p7(x4, y3);
	NFmiPoint p8(x4, y1);
	NFmiPoint p9(x4, y2);
	NFmiPoint p10(x4, y4);
	NFmiPoint p11(x3, y4);
	NFmiPoint p12(x3, y2);

	if(itsNegativeFlashPolyLine == 0)
	{
		itsNegativeFlashPolyLine = new NFmiPolyline(itsRect, 0, itsDrawingEnvironment);
		itsNegativeFlashPolyLine->AddPoint(p1);
		itsNegativeFlashPolyLine->AddPoint(p2);
		itsNegativeFlashPolyLine->AddPoint(p3);
		itsNegativeFlashPolyLine->AddPoint(p4);
		itsNegativeFlashPolyLine->AddPoint(p1);
	}
	if(itsPositiveFlashPolyLine == 0)
	{
		itsPositiveFlashPolyLine = new NFmiPolyline(itsRect, 0, itsDrawingEnvironment);
		itsPositiveFlashPolyLine->AddPoint(p1);
		itsPositiveFlashPolyLine->AddPoint(p5);
		itsPositiveFlashPolyLine->AddPoint(p6);
		itsPositiveFlashPolyLine->AddPoint(p7);
		itsPositiveFlashPolyLine->AddPoint(p8);
		itsPositiveFlashPolyLine->AddPoint(p2);
		itsPositiveFlashPolyLine->AddPoint(p3);
		itsPositiveFlashPolyLine->AddPoint(p9);
		itsPositiveFlashPolyLine->AddPoint(p10);
		itsPositiveFlashPolyLine->AddPoint(p11);
		itsPositiveFlashPolyLine->AddPoint(p12);
		itsPositiveFlashPolyLine->AddPoint(p4);
		itsPositiveFlashPolyLine->AddPoint(p1);
	}
	if(itsCloudFlashPolyLine == 0)
	{ // pilvi salama on pieni laatikko
		const double sizeFactor = 1.7;
		itsCloudFlashPolyLine = new NFmiPolyline(itsRect, 0, itsDrawingEnvironment);
		itsCloudFlashPolyLine->AddPoint(p5*sizeFactor);
		itsCloudFlashPolyLine->AddPoint(p8*sizeFactor);
		itsCloudFlashPolyLine->AddPoint(p9*sizeFactor);
		itsCloudFlashPolyLine->AddPoint(p12*sizeFactor);
		itsCloudFlashPolyLine->AddPoint(p5*sizeFactor);
	}
}

float NFmiFlashDataView::CalcFlashColorFactor(const NFmiMetTime &theStarTime, const NFmiMetTime &theFlashTime, int theEditorTimeStepInMinutes)
{
	return theFlashTime.DifferenceInMinutes(theStarTime)/static_cast<float>(theEditorTimeStepInMinutes);
}

// laskee v�rin miksaamalla siten, ett� jos mixFactor on 0, palauttaa startColorin, jos
// mixFactor on 1, palauttaa endColorin ja sill� v�lill� sekoittaa v�rej� kertoimen mukaan.
NFmiColor NFmiFlashDataView::CalcFlashFillColor(const NFmiColor &theStartColor, const NFmiColor &theEndColor, float theMixRatio)
{
	if(theMixRatio >= 1.)
		return theEndColor;
	else if(theMixRatio <= 0.)
		return theStartColor;
	else
	{
		NFmiColor returnColor(theEndColor);
		returnColor.Mix(theStartColor, (1.f - theMixRatio));
		return returnColor;
	}
}

static std::string Value2Str(float theValue, int decimalCount)
{
    if(theValue == kFloatMissing)
        return "-";
    else
        return std::string(NFmiValueString::GetStringWithMaxDecimalsSmartWay(theValue, decimalCount));
}

std::string NFmiFlashDataView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	float lat,lon,power,acc,multi;
	NFmiMetTime atime;
	std::string returnStr;
	if(FindNearestFlashInMapTimeSection(theRelativePoint, lat, lon, power, acc, multi, atime))
	{
		bool cloudFlash = false;
		if(multi == kFloatMissing)
			cloudFlash = true;

		if(cloudFlash)
			returnStr += "cloud-flash ";
		else
			returnStr += "ground-flash ";

		returnStr += atime.ToStr("HH:mm:SS ");
        returnStr += "(";
        returnStr += ::Value2Str(lon, 3);
        returnStr += (lon >= 0 && lon < 180) ? "E" : "W";
		returnStr += ",";
		returnStr += ::Value2Str(lat, 3);
        returnStr += (lat >= 0) ? "N" : "S";
        returnStr += ")";
//        if(cloudFlash == false)
		{
			returnStr += " p:";
			returnStr += ::Value2Str(power, 0);
			returnStr += " a:";
			returnStr += ::Value2Str(acc, 0);
			returnStr += " m:";
			returnStr += ::Value2Str(multi, 0);
		}
	}
	else
		returnStr += "No close flashes.";

	int groundFlashCount = 0; 
	int cloudFlashCount = 0; 
	CalcFlashCount(groundFlashCount, cloudFlashCount);
	returnStr += "\nTotal ground flash count: ";
	returnStr += NFmiStringTools::Convert(groundFlashCount);
	returnStr += "\nTotal cloud flash count: ";
	returnStr += NFmiStringTools::Convert(cloudFlashCount);
    AddLatestObsInfoToString(returnStr);
    return returnStr;
}

bool NFmiFlashDataView::FindNearestFlash(NFmiFastQueryInfo &theInfo, const NFmiMetTime &theTime, int theUsedTimeStep, const NFmiLocation &theCursorLocation, double &theCurrentMinDist, unsigned long &theMinDistTimeIndex)
{
	float lat = 0;
	float lon = 0;
	theInfo.Param(kFmiLatitude);
	unsigned long latIndex = theInfo.ParamIndex();
	theInfo.Param(kFmiLongitude);
	unsigned long lonIndex = theInfo.ParamIndex();
	theInfo.First();

	bool status = false;
	unsigned long timeIndex1 = 0;
	unsigned long timeIndex2 = 0;
	if(GetTimeSpanIndexies(theInfo, timeIndex1, timeIndex2))
	{
		for(unsigned long i=timeIndex1; i<=timeIndex2; i++)
		{
			theInfo.TimeIndex(i);
			theInfo.ParamIndex(latIndex);
			lat = theInfo.FloatValue();
			theInfo.ParamIndex(lonIndex);
			lon = theInfo.FloatValue();
			NFmiLocation flashLoc(lon, lat);
			double dist = flashLoc.Distance(theCursorLocation);
			if(dist < theCurrentMinDist)
			{
				theCurrentMinDist = dist;
				theMinDistTimeIndex = i;
				status = true;
			}
		}
	}
	return status;
}

bool NFmiFlashDataView::FindNearestFlashInMapTimeSection(const NFmiPoint &theRelativePoint, float &theLat, float &theLon, float &thePow, float &theAcc, float &theMulti, NFmiMetTime &theTime)
{
	bool status = false;
	if(itsDrawParam)
	{
		NFmiLocation cursorLoc(ViewPointToLatLon(theRelativePoint));
		double minDist = 9999999999.; // et. metreiss�
		double maxAllowedDist = 100 * 1000; // max 100 km et.
		int editorTimeStep = static_cast<int>(::round(itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex)*60));
		unsigned long minDistTimeIndex = static_cast<unsigned long>(-1);

		for(size_t j = 0; j < itsInfoVector.size(); j++)
		{
			boost::shared_ptr<NFmiFastQueryInfo> &info = itsInfoVector[j];
			if(FindNearestFlash(*info, itsTime, editorTimeStep, cursorLoc, minDist, minDistTimeIndex))
			{
				if(minDist <= maxAllowedDist)
				{
					info->TimeIndex(minDistTimeIndex);
					info->Param(kFmiLatitude);
					theLat = info->FloatValue();
					info->Param(kFmiLongitude);
					theLon = info->FloatValue();
					info->Param(kFmiFlashStrength);
					thePow = info->FloatValue();
					info->Param(kFmiFlashAccuracy);
					theAcc = info->FloatValue();
					info->Param(kFmiFlashMultiplicity);
					theMulti = info->FloatValue();
					theTime = info->Time();
					status = true;
				}
			}
		}
	}
	return status;
}

bool NFmiFlashDataView::PrepareForStationDraw(void)
{
	return true;
}

bool NFmiFlashDataView::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	return NFmiStationView::LeftButtonUp(thePlace, theKey);
}
bool NFmiFlashDataView::RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	return NFmiStationView::RightButtonUp(thePlace, theKey);
}
bool NFmiFlashDataView::MouseMove(const NFmiPoint& /* thePlace */ , unsigned long /* theKey */ )
{
	return false;
}

bool NFmiFlashDataView::CalcFlashCount(int &theGroundFlashCountOut, int &theCloudFlashCountOut)
{
	bool status = false;
    MakeDrawedInfoVector();
	for(size_t j = 0; j < itsInfoVector.size(); j++)
	{
		boost::shared_ptr<NFmiFastQueryInfo> &info = itsInfoVector[j];

		float lat = 0;
		float lon = 0;
		info->Param(kFmiLatitude);
		unsigned long latIndex = info->ParamIndex();
		info->Param(kFmiLongitude);
		unsigned long lonIndex = info->ParamIndex();
        info->Param(kFmiFlashMultiplicity);
        unsigned long multiplicityIndex = info->ParamIndex();
        info->First();

		unsigned long timeIndex1 = 0;
		unsigned long timeIndex2 = 0;
		if(GetTimeSpanIndexies(*info, timeIndex1, timeIndex2))
		{
			int editorTimeStep = static_cast<int>(::round(itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex)*60));
			NFmiMetTime time2(itsTime);
			NFmiMetTime time1(time2);
			time1.SetTimeStep(1);
			time1.ChangeByMinutes(-editorTimeStep);

			// etsi ensin alku aikaindeksi
			bool found1 = info->TimeToNearestStep(time1, kBackward);
			int index1 = info->TimeIndex();

			// etsi sitten loppu aikaindeksi
			bool found2 = info->TimeToNearestStep(time2, kBackward);
			int index2 = info->TimeIndex();
			if(index1 == index2) // pit�� testata erikois tapaus, koska TimeToNearestStep-palauttaa aina jotain, jos on dataa
			{
				info->TimeIndex(index1);
				NFmiMetTime foundTime(info->Time());
				if(foundTime > time2 || foundTime < time1) // jos l�ydetty aika on alku ja loppu ajan ulkopuolella ei piirret� salamaa
					continue;
			}

			int counter = 0;
			for(int i=index1; i<=index2; i++)
			{
				info->TimeIndex(i);
				info->ParamIndex(latIndex);
				lat = info->FloatValue();
				info->ParamIndex(lonIndex);
				lon = info->FloatValue();
				NFmiPoint latlon(lon, lat);

				if (itsArea->IsInside(latlon))
				{
					info->ParamIndex(multiplicityIndex);
					float multiplicity = info->FloatValue();
					if(multiplicity == kFloatMissing)
						theCloudFlashCountOut++;
					else
						theGroundFlashCountOut++;
					status = true;
				}
			}
		}
	}
	return status;
}

int NFmiFlashDataView::CalcUsedTimeStepInMinutes(void)
{
	return static_cast<int>(::round(itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex) * 60));
}

NFmiMetTime NFmiFlashDataView::CalcFirstTimeOfSpan(void)
{
	int editorTimeStep = CalcUsedTimeStepInMinutes();
	NFmiMetTime time2(itsTime);
	NFmiMetTime time1(time2);
	time1.SetTimeStep(1);
	time1.ChangeByMinutes(-editorTimeStep);
	return time1;
}

// Etsii karttan�yt�lle haluttuun salamadataan alku ja loppu ajan indeksit. Jos ei l�ydy halutulle
// aikav�lille yht��n salamaa, palauttaa false:n, muuten true.
bool NFmiFlashDataView::GetTimeSpanIndexies(NFmiFastQueryInfo &theInfo, unsigned long &theStartIndexOut, unsigned long &theEndIndexOut)
{
	NFmiMetTime time2(itsTime);
	NFmiMetTime time1 = CalcFirstTimeOfSpan();
    int usedTimeRangeInMinutes = CalcUsedTimeStepInMinutes();

	// etsi ensin alku aikaindeksi
	bool found1 = theInfo.TimeToNearestStep(time1, kForward, usedTimeRangeInMinutes);
	int index1 = theInfo.TimeIndex();

	// etsi sitten loppu aikaindeksi
	bool found2 = theInfo.TimeToNearestStep(time2, kBackward, usedTimeRangeInMinutes);
	int index2 = theInfo.TimeIndex();

    // Jos kumpaakaa aikaa ei l�ytynyt, ei tarvitse jatkaa
    if(found1 == false && found2 == false)
        return false;

    // Jos toinen ajoista l�ytyy mutta toista ei, annetaan l�ydetty aikaindeksi molemmille ja katsotaan tuottaako se tulosta
    if(found1 == true && found2 == false)
        index2 = index1;
    else if(found1 == false && found2 == true)
        index1 = index2;

	if(index1 == index2) // pit�� testata erikois tapaus, koska TimeToNearestStep-palauttaa aina jotain, jos on dataa
	{
		theInfo.TimeIndex(index1);
		NFmiMetTime foundTime(theInfo.Time());
		if(foundTime > time2 || foundTime < time1) // jos l�ydetty aika on alku ja loppu ajan ulkopuolella ei piirret� salamaa
			return false;
	}
	theStartIndexOut = index1;
	theEndIndexOut = index2;

	return true;
}
