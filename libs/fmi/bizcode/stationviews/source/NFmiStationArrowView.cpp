//� Ilmatieteenlaitos/Marko
//  Original 27.06.2006
//
//
//Ver. xx.xx.xxxx/Marko
//
//-------------------------------------------------------------------- NFmiStationArrowView.cpp

#include "NFmiStationArrowView.h"
#include "NFmiDrawingEnvironment.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiArea.h"
#include "NFmiPolyline.h"
#include "NFmiToolBox.h"
#include "CtrlViewDocumentInterface.h"
#include "GraphicalInfo.h"

#include "boost/math/special_functions/round.hpp"

NFmiStationArrowView::NFmiStationArrowView 
								 (int theMapViewDescTopIndex, std::shared_ptr<NFmiArea> &theArea
								 ,NFmiToolBox * theToolBox
								 ,std::shared_ptr<NFmiDrawParam> &theDrawParam
								 ,FmiParameterName theParamId
								 ,NFmiPoint theOffSet
								 ,NFmiPoint theSize
								 ,int theRowIndex
                                 , int theColumnIndex)
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

NFmiStationArrowView::~NFmiStationArrowView(void)
{
}

double gSizeFactor = 0.6; // Muunnan kokoja ja offsetteja t�ll� kertoimella jotta symbolin koko olisi paremmin yhteensopiva muiden symbolien s��t�jen kanssa
NFmiPoint gSizeFactorPoint(gSizeFactor, gSizeFactor);

float NFmiStationArrowView::ViewFloatValue(bool doTooltipValue)
{
	float angle = NFmiStationView::ViewFloatValue(doTooltipValue);
	if(angle != kFloatMissing)
	{
		if(!doTooltipValue)
		{
			// Tehd��n pohjoissuunta korjaus tuuliviirin piirtoon, paitsi jos kyse tooltip arvosta
			NFmiAngle ang(itsArea->TrueNorthAzimuth(CurrentLatLon()));
			angle += static_cast<float>(ang.Value());
			FmiParameterName parId = static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent());
			// Tietyt parametrit pit�� k��nt�� 180 astetta ja tietyt ei:
			// K��nnett�viin kuuluu tuulen suunta ja erilaiset aallon suunnat.
			bool flipArrowAngle = (parId == kFmiWindDirection || parId == kFmiWaveDirection || parId == kFmiWaveDirectionBandB || parId == kFmiWaveDirectionBandC || parId == kFmiWaveDirectionSwell0 || parId == kFmiWaveDirectionSwell1 || parId == kFmiWaveDirectionSwell2);
			if(itsDrawParam->FlipArrowSymbol())
			{
				flipArrowAngle = !flipArrowAngle;
			}

			if(flipArrowAngle)
			{
				angle += 180;
			}
		}
	}
	return angle;
}

// Huono nimi virtuaali metodilla GetSpaceOutFontFactor,
// tuuli vektori ei ole fontti pohjainen symboli ja metodin pit�isi olla joku SymbolSizeFactor
NFmiPoint NFmiStationArrowView::GetSpaceOutFontFactor(void)
{
	return NFmiPoint(1., 1.);
}

// t�m�kin on huono viritys, mutta harvennuskoodi ottaa t�ss� vaiheessa fontti koon huomioon kun
// se laskee miten harvennetaan hila dataa. Nyt pit�� siis laskea fontti koko t��ll�kin, vaikka
// tuuli vektori ei olekaan fontti pohjainen symboli.
void NFmiStationArrowView::ModifyTextEnvironment(void)
{
    itsDrawingEnvironment.SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 45), itsCtrlViewDocumentInterface->Printing()));
}

int NFmiStationArrowView::GetApproxmationOfDataTextLength(std::vector<float>* )
{
	return 1;
}

NFmiPoint NFmiStationArrowView::SbdCalcDrawObjectOffset() const
{
	NFmiRect rect(CurrentStationRect());
	NFmiPoint offset(rect.Center());
	NFmiPoint symbolOffset(itsDrawParam->OnlyOneSymbolRelativePositionOffset());
	symbolOffset *= gSizeFactorPoint;
	symbolOffset *= rect.Size();
	offset += symbolOffset;
	offset -= CurrentStationPosition();
	return offset;
}

NFmiPoint NFmiStationArrowView::SbdCalcFixedSymbolSize() const
{
	NFmiRect rect(CurrentStationRect());
	NFmiPoint symbolSize(itsDrawParam->OnlyOneSymbolRelativeSize());
	symbolSize *= gSizeFactorPoint;
	NFmiPoint scale(rect.Size());
	scale *= symbolSize;
	// Originaali koodissa gSizeFactorPoint:illa skaalailtu kahdssa kohtaa koodia, pakko j�tt�� toistaiseksi
	scale *= gSizeFactorPoint;
	// T�m� korjaa erilaisten karttaruudukko asetelmien aiheuttamia v��ristymi�, pahimpina
	// ovat suuret erot x/y dimensioissa kuten 3x1 ja 2x4 tyyppiset ruudukot (3x1 on 3 saraketta ja 1 rivi)
	scale = SbdCalcOldSchoolSymbolScaleFix(scale);
	// Fiksattu symbol size eli nuolen originaali piirrossa k�ytetty scale:n y komponentti.
	return scale;
}

int NFmiStationArrowView::SbdCalcFixedPenSize() const
{
	// otetaan k�ytetyn kyn�n paksuus isoviiva leveydest�!!!
	double wantedPenWidthInMM = itsDrawParam->SimpleIsoLineWidth();
	double pixelsPerMM = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y;
	int penSize = FmiMax(1, boost::math::iround(wantedPenWidthInMM * pixelsPerMM));
	return penSize;
}

NFmiSymbolBulkDrawType NFmiStationArrowView::SbdGetDrawType() const
{
	return NFmiSymbolBulkDrawType::Arrow;
}

NFmiSymbolColorChangingType NFmiStationArrowView::SbdGetSymbolColorChangingType() const
{
	return NFmiSymbolColorChangingType::OnlyWithOtherParameterValues;
}
