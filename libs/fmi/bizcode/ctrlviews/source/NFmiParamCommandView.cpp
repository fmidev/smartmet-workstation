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
NFmiParamCommandView::NFmiParamCommandView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, NFmiDrawingEnvironment * theDrawingEnvi, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex)
:NFmiCtrlView(theMapViewDescTopIndex, theRect, theToolBox, theDrawingEnvi, theDrawParam, theRowIndex, theColumnIndex)
,fShowView(true)
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

// indeksit alkavat 1:stä eli 1. rivi on indeksillä 1 jne.
NFmiRect NFmiParamCommandView::CheckBoxRect(int lineIndex, bool drawedRect)
{
	NFmiPoint p = GetFrame().TopLeft();
	p += itsFirstLinePlace;
	p.Y(p.Y() + (lineIndex-1) * itsLineHeight);
	NFmiRect rect;
	rect.Place(p);
	if(drawedRect) // piirrettävää boxia pitää siirtää hieman vertikaali suunnassa alas. Ikävää koodia, mutta voi voi
		rect.Top(rect.Top()+itsCheckBoxSize.Y()/3.);
	rect.Size(itsCheckBoxSize);
	return rect;
}
// indeksit alkavat 1:stä eli 1. rivi on indeksillä 1 jne.
NFmiPoint NFmiParamCommandView::LineTextPlace(int lineIndex, bool checkBoxMove)
{
	// tekstin alku paikka lasketaan checkboxin avulla
	NFmiRect checkBoxRect(CheckBoxRect(lineIndex, false));
	NFmiPoint p = checkBoxRect.TopLeft();
	if(checkBoxMove)
		p.X(checkBoxRect.Right() + itsPixelSize.X() * 2); // + kaksi pikseliä eli laitetaan vähän väliä tekstin ja checkboxin väliin
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
		// Rastin tai ruksin piirto ei onnistunut, tuli rumaa jälkeä. Joten
		// piirrän merkin checkboxin sisään fillatulla laatikolla.
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

int NFmiParamCommandView::CalcIndex(const NFmiPoint& thePlace)
{
	for(int counter = 1; counter < 100 ; counter++) // < 100 hatusta (= sata riviä tekstiä!!!)
	{
		NFmiPoint place = LineTextPlace(counter+1, false); // joku feelu tuli refactorointi laskuissa, mutta tässä pitää lisätä yhdellä counteria
		if(place.Y() >= thePlace.Y())
			return counter;
	}
	return 0; // 
}

bool NFmiParamCommandView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
/*
	if(IsIn(thePlace))
	{
		if(theDelta < 0)
			return itsDoc->ScrollViewRow(itsMapViewDescTopIndex, 1);
		else
			return itsDoc->ScrollViewRow(itsMapViewDescTopIndex, -1);
	}
*/
	return false;
}
