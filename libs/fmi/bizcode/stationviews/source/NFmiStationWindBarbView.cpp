#include "NFmiStationWindBarbView.h"
#include "NFmiDrawingEnvironment.h"
#include "NFmiWindBarb.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiDrawParam.h"
#include "NFmiArea.h"
#include "CtrlViewDocumentInterface.h"
#include "GraphicalInfo.h"
#include "boost\math\special_functions\round.hpp"

NFmiStationWindBarbView::NFmiStationWindBarbView
								 (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
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
{
}

NFmiStationWindBarbView::~NFmiStationWindBarbView(void)
{
}

void NFmiStationWindBarbView::Draw(NFmiToolBox * theGTB)
{
	NFmiStationView::Draw(theGTB);
}

bool NFmiStationWindBarbView::PrepareForStationDraw(void)
{
	bool status = NFmiStationView::PrepareForStationDraw();
	itsDrawingEnvironment->SetFrameColor(itsDrawParam->FrameColor());
	itsDrawingEnvironment->EnableFill();
	itsDrawingEnvironment->SetFillColor(itsDrawParam->FillColor());
	if(NFmiDrawParam::IsMacroParamCase(itsInfo->DataType()))
	{
		fUseMacroParamSpecialCalculations = true;
		CalcViewFloatValueMatrix(itsMacroParamSpecialCalculationsValues, 0, 0, 0, 0); // datahila pit‰‰ laskea jo t‰ss‰
		status = true; // macroParam tapauksessa t‰m‰ menee false:ksi (NFmiStationView::PrepareForStationDraw),
						// koska itsInfo:n aika-ayateemi rakennetaan vasta CalcViewFloatValueMatrix-metodin kutsun yhteydess‰
	}
	return status;
}

void NFmiStationWindBarbView::DrawData(void)
{
	DrawSymbol();
}

void AdjustWindBarbSizeAndPlace(NFmiRect &rect)
{
    // Increase width
    double avgSize = (rect.Width() + rect.Height()) / 2.0f;
    double change = (avgSize - rect.Width()) / 2;
    rect.Size(NFmiPoint(avgSize, rect.Height()));
    // Move rect back to it's correct position.
    rect.Place(NFmiPoint(rect.Place().X() - change, rect.Place().Y()));
}

void NFmiStationWindBarbView::DrawSymbol(void)
{
	NFmiRect rect(CurrentStationRect());
    AdjustWindBarbSizeAndPlace(rect);

	float windVector = ViewFloatValue();
	if(windVector != kFloatMissing)
	{
		float windSpeed = static_cast<float>(static_cast<int>(windVector/100));
		float windDir =float( ((int)windVector % 100) * 10);
		// tehd‰‰n pohjois korjaus tuuliviirin piirtoon
        NFmiPoint latlon = CurrentLatLon();
		NFmiAngle ang(itsArea->TrueNorthAzimuth(latlon));
		windDir += static_cast<float>(ang.Value());

		NFmiPoint symbolSize(itsDrawParam->OnlyOneSymbolRelativeSize());
        long pixelWidth = boost::math::iround(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x * itsDrawParam->SimpleIsoLineWidth());
		itsDrawingEnvironment->SetPenSize(NFmiPoint(pixelWidth, pixelWidth));
		NFmiWindBarb(windSpeed
					,windDir
					,rect
					,itsToolBox
                    ,latlon.Y() < 0
					,symbolSize.X() * 0.7
					,symbolSize.Y() * 0.5
					,0
					,itsDrawingEnvironment).Build();
		itsDrawingEnvironment->SetPenSize(NFmiPoint(1,1));
	}
}

// Huono nimi virtuaali metodilla GetSpaceOutFontFactor,
// tuuli vektori ei ole fontti pohjainen symboli ja metodin pit‰isi olla joku SymbolSizeFactor
NFmiPoint NFmiStationWindBarbView::GetSpaceOutFontFactor(void)
{
	return NFmiPoint(2, 1);
}

// t‰m‰kin on huono viritys, mutta harvennuskoodi ottaa t‰ss‰ vaiheessa fontti koon huomioon kun
// se laskee miten harvennetaan hila dataa. Nyt pit‰‰ siis laskea fontti koko t‰‰ll‰kin, vaikka
// tuuli vektori ei olekaan fontti pohjainen symboli.
void NFmiStationWindBarbView::ModifyTextEnvironment(void)
{
    itsDrawingEnvironment->SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 48), itsCtrlViewDocumentInterface->Printing()));
}

int NFmiStationWindBarbView::GetApproxmationOfDataTextLength(void)
{
	return 1;
}

float NFmiStationWindBarbView::InterpolatedToolTipValue(const NFmiMetTime &theUsedTime, const NFmiPoint& theLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
    if(metaWindParamUsage.MakeMetaWindVectorParam())
    {
        return NFmiFastInfoUtils::GetMetaWindValue(theInfo, theUsedTime, theLatlon, metaWindParamUsage, kFmiWindVectorMS);
    }
    else
        return NFmiStationView::InterpolatedToolTipValue(theUsedTime, theLatlon, theInfo);
}
