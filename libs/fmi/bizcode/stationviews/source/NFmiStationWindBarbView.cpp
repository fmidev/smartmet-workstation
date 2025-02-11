#include "NFmiStationWindBarbView.h"
#include "NFmiDrawingEnvironment.h"
#include "NFmiWindBarb.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiDrawParam.h"
#include "NFmiArea.h"
#include "CtrlViewDocumentInterface.h"
#include "GraphicalInfo.h"
#include "NFmiToolBox.h"
#include "boost\math\special_functions\round.hpp"

NFmiStationWindBarbView::NFmiStationWindBarbView
								 (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
								 ,NFmiToolBox * theToolBox
								 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								 ,FmiParameterName theParamId
								 ,NFmiPoint theOffSet
								 ,NFmiPoint theSize
								 ,int theRowIndex
                                 ,int theColumnIndex)
:NFmiStationView(theMapViewDescTopIndex, theArea
				,theToolBox
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
	itsDrawingEnvironment.SetFrameColor(itsDrawParam->FrameColor());
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(itsDrawParam->FillColor());
	if(NFmiDrawParam::IsMacroParamCase(itsInfo->DataType()))
	{
		fUseMacroParamSpecialCalculations = true;
		bool dummyBoolNotUsed = false;
		CalcViewFloatValueMatrix(itsMacroParamSpecialCalculationsValues, 0, 0, 0, 0, dummyBoolNotUsed); // datahila pitää laskea jo tässä
		status = true; // macroParam tapauksessa tämä menee false:ksi (NFmiStationView::PrepareForStationDraw),
						// koska itsInfo:n aika-ayateemi rakennetaan vasta CalcViewFloatValueMatrix-metodin kutsun yhteydessä
	}
	return status;
}

float NFmiStationWindBarbView::ViewFloatValue(bool doTooltipValue)
{
	float windVector = NFmiStationView::ViewFloatValue(doTooltipValue);
	if(windVector == kFloatMissing)
		return kFloatMissing;
	else
	{
		float ws, wd;
		std::tie(ws, wd) = NFmiToolBox::GetWsAndWdFromWindVector(windVector);
		if(!doTooltipValue)
		{
			// Tehdään pohjoissuunta korjaus tuuliviirin piirtoon, paitsi ei tooltip arvoa laskettaessa
			NFmiPoint latlon = CurrentLatLon();
			NFmiAngle ang(itsArea->TrueNorthAzimuth(latlon));
			wd += static_cast<float>(ang.Value());
		}
		return NFmiFastInfoUtils::CalcWindVectorFromSpeedAndDirection(ws, wd);
	}
}

// Huono nimi virtuaali metodilla GetSpaceOutFontFactor,
// tuuli vektori ei ole fontti pohjainen symboli ja metodin pitäisi olla joku SymbolSizeFactor
NFmiPoint NFmiStationWindBarbView::GetSpaceOutFontFactor(void)
{
	return NFmiPoint(2, 1);
}

// tämäkin on huono viritys, mutta harvennuskoodi ottaa tässä vaiheessa fontti koon huomioon kun
// se laskee miten harvennetaan hila dataa. Nyt pitää siis laskea fontti koko täälläkin, vaikka
// tuuli vektori ei olekaan fontti pohjainen symboli.
void NFmiStationWindBarbView::ModifyTextEnvironment(void)
{
    itsDrawingEnvironment.SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 48), itsCtrlViewDocumentInterface->Printing()));
}

NFmiPoint NFmiStationWindBarbView::SbdCalcFixedSymbolSize() const
{
	NFmiPoint symbolSize(itsDrawParam->OnlyOneSymbolRelativeSize());
	symbolSize.X(symbolSize.X() * 0.7);
	symbolSize.Y(symbolSize.Y() * 0.5);
	return symbolSize;
}

int NFmiStationWindBarbView::SbdCalcFixedPenSize() const
{
	long pixelWidth = boost::math::iround(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x * itsDrawParam->SimpleIsoLineWidth());
	return pixelWidth;
}

NFmiSymbolBulkDrawType NFmiStationWindBarbView::SbdGetDrawType() const
{
	return NFmiSymbolBulkDrawType::WindBarb;
}

NFmiPoint NFmiStationWindBarbView::SbdCalcFixedRelativeDrawObjectSize() const
{
	NFmiRect rect(CurrentStationRect());
	auto scale = SbdCalcOldSchoolSymbolScaleFix(rect.Size());
	return scale;
}

NFmiPoint NFmiStationWindBarbView::SbdCalcDrawObjectOffset() const
{
	NFmiRect rect(CurrentStationRect());
	NFmiPoint offset(rect.Center());
	offset -= CurrentStationPosition();
	return offset;
}

int NFmiStationWindBarbView::GetApproxmationOfDataTextLength(std::vector<float>* )
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

NFmiSymbolColorChangingType NFmiStationWindBarbView::SbdGetSymbolColorChangingType() const
{
	return NFmiSymbolColorChangingType::OnlyWithOtherParameterValues;
}
