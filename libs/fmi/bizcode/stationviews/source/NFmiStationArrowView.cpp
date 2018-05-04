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

#include "boost\math\special_functions\round.hpp"

NFmiStationArrowView::NFmiStationArrowView 
								 (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
								 ,NFmiToolBox * theToolBox
								 ,NFmiDrawingEnvironment * theDrawingEnvi
								 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								 ,FmiParameterName theParamId
								 ,NFmiPoint theOffSet
								 ,NFmiPoint theSize
								 ,int theRowIndex
                                 , int theColumnIndex)
:NFmiStationView(theMapViewDescTopIndex, theArea
				,theToolBox
				,theDrawingEnvi
				,theDrawParam
				,theParamId
				,theOffSet
				,theSize
				,theRowIndex
                ,theColumnIndex)
,itsArrowSymbol(0)
{
}

NFmiStationArrowView::~NFmiStationArrowView(void)
{
	delete itsArrowSymbol;
}

void NFmiStationArrowView::Draw(NFmiToolBox * theGTB)
{
	if(itsArrowSymbol)
		delete itsArrowSymbol;
	itsDrawingEnvironment->SetFrameColor(itsDrawParam->FrameColor());
	itsDrawingEnvironment->DisableFill();

	// otetaan k�ytetyn kyn�n paksuus isoviiva leveydest�!!!
	double wantedPenWidthInMM = itsDrawParam->SimpleIsoLineWidth();
	double pixelsPerMM = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y;

    int penSize = FmiMax(1, boost::math::iround(wantedPenWidthInMM*pixelsPerMM));
	NFmiPoint oldPenSize(itsDrawingEnvironment->GetPenSize());
	itsDrawingEnvironment->SetPenSize(NFmiPoint(penSize, penSize));

	// Luodaan perus nuoli symboli, jota sitten k�ytet��n koko piirto funktion ajan (sit� rotatoidaan toolboxissa)
	itsArrowSymbol = new NFmiPolyline(itsRect, 0, itsDrawingEnvironment);
	// HUOM! t�m� suhteellinen maailma on yl�salaisin eli -1 on ylh��ll� ja +1 on alhaalla
	itsArrowSymbol->AddPoint(NFmiPoint(0, 1));
	itsArrowSymbol->AddPoint(NFmiPoint(0, -1));
	itsArrowSymbol->AddPoint(NFmiPoint(-0.5, -0.25));
	itsArrowSymbol->AddPoint(NFmiPoint(0, -1));
	itsArrowSymbol->AddPoint(NFmiPoint(0.5, -0.25));

	NFmiStationView::Draw(theGTB);

	itsDrawingEnvironment->SetPenSize(oldPenSize);
}

void NFmiStationArrowView::DrawData(void)
{
	DrawSymbol();
}

double gSizeFactor = 0.6; // Muunnan kokoja ja offsetteja t�ll� kertoimella jotta symbolin koko olisi paremmin yhteensopiva muiden symbolien s��t�jen kanssa
NFmiPoint gSizeFactorPoint(gSizeFactor, gSizeFactor);

void NFmiStationArrowView::DrawSymbol(void)
{
	NFmiRect rect(CurrentStationRect());
	if(!itsRect.IsInside(rect.Center()))
		return ;

	float angle = ViewFloatValue();
	if(angle != kFloatMissing)
	{
		// tehd��n pohjois korjaus tuuliviirin piirtoon
		NFmiAngle ang(itsArea->TrueNorthAzimuth(CurrentLatLon()));
		angle += static_cast<float>(ang.Value());
		FmiParameterName parId = static_cast<FmiParameterName>(itsDrawParam->Param().GetParamIdent());
		if(parId == kFmiWindDirection || parId == kFmiWaveDirection || parId == kFmiWaveDirectionBandB || parId == kFmiWaveDirectionBandC || parId == kFmiWaveDirectionSwell0 || parId == kFmiWaveDirectionSwell1 || parId == kFmiWaveDirectionSwell2)
			angle += 180; // suunta kuvaa siis aina mist� tullaan, eik� mihin menn��n..., aaltojen suunto halutaan my�s tuulen suuntaiseksi



		NFmiPoint offset(rect.Center());
		NFmiPoint symbolOffset(itsDrawParam->OnlyOneSymbolRelativePositionOffset());
        symbolOffset *= gSizeFactorPoint;
        symbolOffset *= rect.Size();
		offset += symbolOffset;
		NFmiPoint symbolSize(itsDrawParam->OnlyOneSymbolRelativeSize());
        symbolSize *= gSizeFactorPoint;
        NFmiPoint scale(rect.Size());
        scale *= symbolSize;
        scale *= gSizeFactorPoint;
		itsToolBox->DrawPolyline(itsArrowSymbol, offset, scale, angle);
	}
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
//	itsDrawingEnvironment->SetFontSize(CalcFontSize(24, 24));
    itsDrawingEnvironment->SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 45), itsCtrlViewDocumentInterface->Printing()));
}

int NFmiStationArrowView::GetApproxmationOfDataTextLength(void)
{
	return 1;
}
