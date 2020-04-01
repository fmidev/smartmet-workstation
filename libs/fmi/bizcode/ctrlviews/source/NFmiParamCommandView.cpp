//**********************************************************
// C++ Class Name : NFmiParamCommandView 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiParamCommandView.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 2 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : Markon ehdotus 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thur - Jan 28, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// 
//**********************************************************
#include "NFmiParamCommandView.h"
#include "NFmiToolBox.h"
#include "NFmiRectangle.h"
#include "CtrlViewDocumentInterface.h"
#include "GraphicalInfo.h"

//--------------------------------------------------------
// ParamCommandView 
//--------------------------------------------------------
NFmiParamCommandView::NFmiParamCommandView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, NFmiDrawingEnvironment * theDrawingEnvi, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex, bool hasMapLayer)
:NFmiCtrlView(theMapViewDescTopIndex, theRect, theToolBox, theDrawingEnvi, theDrawParam, theRowIndex, theColumnIndex)
,fShowView(true)
,fHasMapLayer(hasMapLayer)
{
}
//--------------------------------------------------------
// ParamCommandView 
//--------------------------------------------------------
NFmiRect NFmiParamCommandView::CalcSize(void)
{
	NFmiRect returnRect;
	return returnRect;
}
//--------------------------------------------------------
// CalcFontSize 
//--------------------------------------------------------
void NFmiParamCommandView::CalcFontSize(void)
{
	double fontSizeInMM = 2.3;
	int pixelSize = static_cast<int>(fontSizeInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x * 1.88);
	itsFontSize = NFmiPoint(pixelSize, pixelSize);
}
//--------------------------------------------------------
// Draw 
//--------------------------------------------------------
void NFmiParamCommandView::Draw(NFmiToolBox* theGTB)
{
	if(fShowView)
	{
		DrawBackground();
		DrawData();
	}
}
//--------------------------------------------------------
// DrawBackground 
//--------------------------------------------------------
void NFmiParamCommandView::DrawBackground(void)
{
	if(fShowView)
	{
		itsDrawingEnvironment->SetFrameColor(NFmiColor(0,0,0));
		itsDrawingEnvironment->DisableFill();
		NFmiRect frame = GetFrame();
		NFmiRectangle tmp(frame.TopLeft(),
				  frame.BottomRight(),
				  0,
				  itsDrawingEnvironment);
		itsToolBox->Convert(&tmp);
	}
}

// Piirrett‰viin parametreihin liittyv‰ lineIndex alkaa 1:st‰ eli 1. parametririvi on indeksill‰ 1 jne.
// Mutta koska rivill‰ 0 on manipuloimaton map-layer, pit‰‰ se ottaa laskettaessa rivin laatikkoa.
NFmiRect NFmiParamCommandView::CheckBoxRect(int lineIndex, bool drawedRect)
{
	if(!fHasMapLayer)
		lineIndex--;
	NFmiPoint p = GetFrame().TopLeft();
	p += itsFirstLinePlace;
	p.Y(p.Y() + lineIndex * itsLineHeight);
	NFmiRect rect;
	rect.Place(p);
	if(drawedRect) // piirrett‰v‰‰ boxia pit‰‰ siirt‰‰ hieman vertikaali suunnassa alas. Ik‰v‰‰ koodia, mutta voi voi
		rect.Top(rect.Top()+itsCheckBoxSize.Y()/3.);
	rect.Size(itsCheckBoxSize);
	return rect;
}

NFmiPoint NFmiParamCommandView::LineTextPlace(int lineIndex, bool checkBoxMove)
{
	// tekstin alku paikka lasketaan checkboxin avulla
	NFmiRect checkBoxRect(CheckBoxRect(lineIndex, false));
	NFmiPoint p = checkBoxRect.TopLeft();
	if(checkBoxMove)
		p.X(checkBoxRect.Right() + itsPixelSize.X() * 2); // + kaksi pikseli‰ eli laitetaan v‰h‰n v‰li‰ tekstin ja checkboxin v‰liin
	return p;
}

void NFmiParamCommandView::DrawCheckBox(const NFmiRect &theRect, NFmiDrawingEnvironment &theEnvi, bool fDrawCheck)
{
	theEnvi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	theEnvi.SetPenSize(NFmiPoint(1,1));
	NFmiRectangle rec(theRect, 0, &theEnvi);
	itsToolBox->Convert(&rec);
	if(fDrawCheck)
	{
		// Rastin tai ruksin piirto ei onnistunut, tuli rumaa j‰lke‰. Joten
		// piirr‰n merkin checkboxin sis‰‰n fillatulla laatikolla.
		theEnvi.SetFrameColor(NFmiColor(0.5f,0.5f,0.5f));
		theEnvi.SetPenSize(NFmiPoint(3,3));
		NFmiRect rect2(theRect);
		rect2.Left(rect2.Left()+itsPixelSize.X()*2);
		rect2.Right(rect2.Right()-itsPixelSize.X()*2);
		rect2.Top(rect2.Top()+itsPixelSize.Y()*2);
		rect2.Bottom(rect2.Bottom()-itsPixelSize.Y()*2);
		NFmiRectangle rec2(rect2, 0, &theEnvi);
		itsToolBox->Convert(&rec2);
	}
}

void NFmiParamCommandView::UpdateTextData(const NFmiPoint& theFontSize, const NFmiPoint& theFirstLinePlace, double theLineHeight, const NFmiPoint &theCheckBoxSize, const NFmiPoint &thePixelSize)
{
   itsFontSize = theFontSize;
   itsFirstLinePlace = theFirstLinePlace;
   itsLineHeight = theLineHeight;
   itsCheckBoxSize = theCheckBoxSize;
   itsPixelSize = thePixelSize;
}

void NFmiParamCommandView::CalcTextData(void)
{
	CalcFontSize();
	itsLineHeight = this->itsToolBox->SY(static_cast<long>(itsFontSize.Y())) * .80;
	double xSize = this->itsToolBox->SX(static_cast<long>(itsFontSize.Y())) * .80;
	double x = itsLineHeight/10.;
	itsFirstLinePlace.Set(2*x,x);
	double factor = 0.8;
	itsCheckBoxSize.Set(xSize * factor, itsLineHeight * factor);
	itsPixelSize.X(itsToolBox->SX(1));
	itsPixelSize.Y(itsToolBox->SY(1));
}

// Oikeat parametri rivit alkavat 1:st‰. 
// Rivi 0 on map-layer rivi, jota ei voi manipuloida mitenk‰‰n.
int NFmiParamCommandView::CalcIndex(const NFmiPoint& thePlace, double* indexRealValueOut)
{
	auto cursorHeight = thePlace.Y() - GetFrame().Top();
	auto zeroBasedLineIndexRealValue = cursorHeight / itsLineHeight;
	if(!fHasMapLayer)
		zeroBasedLineIndexRealValue += 1;

	// T‰m‰n pirun parametri boxin laskukoodit pit‰isi tehd‰ uudestaan, nyt minun kuitenkin pit‰‰ vain v‰hent‰‰ 
	// lasketusta indeksist‰ joku vakio desimaaliosio, jotta indeksin ja klikattu paikka ovat mahd. oikean tuntuisia.
	zeroBasedLineIndexRealValue -= 0.25;

	if(indexRealValueOut)
		*indexRealValueOut = zeroBasedLineIndexRealValue;
	return static_cast<int>(zeroBasedLineIndexRealValue);
}

bool NFmiParamCommandView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	return false;
}
