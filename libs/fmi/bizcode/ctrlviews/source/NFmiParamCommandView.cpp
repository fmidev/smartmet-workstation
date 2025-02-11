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

double NFmiParamCommandView::itsParameterRowVerticalMarginInMM = 0.6;
double NFmiParamCommandView::itsParameterRowHorizontalMarginInMM = 0.5;

//--------------------------------------------------------
// ParamCommandView 
//--------------------------------------------------------
NFmiParamCommandView::NFmiParamCommandView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex, bool hasMapLayer)
:NFmiCtrlView(theMapViewDescTopIndex, theRect, theToolBox, theDrawParam, theRowIndex, theColumnIndex)
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
		itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
		itsDrawingEnvironment.DisableFill();
		NFmiRect frame = GetFrame();
		NFmiRectangle tmp(frame.TopLeft(),
				  frame.BottomRight(),
				  0,
				  &itsDrawingEnvironment);
		itsToolBox->Convert(&tmp);
	}
}

double NFmiParamCommandView::ConvertMilliMeterToRelative(double lengthInMilliMeter, bool isDirectionX) const
{
	const auto & graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
	if(isDirectionX)
	{
		double pixels = graphicalInfo.itsPixelsPerMM_x * lengthInMilliMeter;
		return itsToolBox->SXs(pixels);
	}
	else
	{
		double pixels = graphicalInfo.itsPixelsPerMM_y * lengthInMilliMeter;
		return itsToolBox->SYs(pixels);
	}
}

NFmiRect NFmiParamCommandView::CheckBoxRect(const NFmiRect& parameterRowRect)
{
	NFmiRect checkBoxRect(parameterRowRect);
	// Siirret‰‰n laatikkoa pikkuisen oikealle ja alas
	auto leftMargin = ConvertMilliMeterToRelative(itsParameterRowHorizontalMarginInMM + 0.1, true);
	checkBoxRect.Place(NFmiPoint(parameterRowRect.Left() + leftMargin, parameterRowRect.Top() + (itsCheckBoxSize.Y() / 15.)));
	checkBoxRect.Size(itsCheckBoxSize);
	checkBoxRect.Center(NFmiPoint(checkBoxRect.Center().X(), parameterRowRect.Center().Y()));
	return checkBoxRect;
}

NFmiPoint NFmiParamCommandView::LineTextPlace(int zeroBasedRowIndex, const NFmiRect& parameterRowRect, bool checkBoxMove)
{
	NFmiPoint textPlace = parameterRowRect.TopLeft();
	// Siirret‰‰n tekstin paikka v‰h‰n oikealle, jotta se ei olisi kiinni parametri laatikon left reunassa
	auto leftMargin = ConvertMilliMeterToRelative(itsParameterRowHorizontalMarginInMM, true);
	textPlace.X(textPlace.X() + leftMargin);
	auto rowHasCheckBox = (checkBoxMove || !(fHasMapLayer && zeroBasedRowIndex == 0));
	if(rowHasCheckBox)
	{
		// Jos rivill‰ on checkbox, laitetaan teksti sen oikeaan laitaan + margin
		auto checkboxRect = CheckBoxRect(parameterRowRect);
		textPlace.X(checkboxRect.Right() + leftMargin);
	}
	return textPlace;
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

void NFmiParamCommandView::UpdateTextData(const NFmiPoint& theFontSize, double theLineHeight, const NFmiPoint &theCheckBoxSize, const NFmiPoint &thePixelSize)
{
   itsFontSize = theFontSize;
   itsLineHeight = theLineHeight;
   itsCheckBoxSize = theCheckBoxSize;
   itsPixelSize = thePixelSize;
}

void NFmiParamCommandView::CalcTextData(void)
{
	CalcFontSize();
	itsLineHeight = this->itsToolBox->SY(static_cast<long>(itsFontSize.Y())) * .80;
	auto rowMarginY = ConvertMilliMeterToRelative(itsParameterRowVerticalMarginInMM, false);
	itsLineHeight += rowMarginY; // venytet‰‰n hieman rivin korkeutta
	double xSize = this->itsToolBox->SX(static_cast<long>(itsFontSize.Y())) * .80;
	double x = itsLineHeight/10.;
	double factor = 0.8;
	itsCheckBoxSize.Set(xSize * factor, itsLineHeight * factor);
	itsPixelSize.X(itsToolBox->SX(1));
	itsPixelSize.Y(itsToolBox->SY(1));
}

// Parametri rivit alkavat 1:st‰, paitsi jos fHasMapLayer = true, eli kyse on NFmiViewParamsView luokan oliosta
// jolloin indeksit alkavat 0:sta (0 on aina tuo map layer, jota ei voi manipuloida mitenk‰‰n).
int NFmiParamCommandView::CalcParameterRowIndex(const NFmiPoint& pointedPlace, double* indexRealValueOut) const
{
	auto startMargin = ConvertMilliMeterToRelative(itsParameterRowVerticalMarginInMM, false);
	auto cursorHeight = pointedPlace.Y() - (GetFrame().Top() + startMargin);
	auto lineIndexRealValue = cursorHeight / itsLineHeight;

	if(!fHasMapLayer)
		lineIndexRealValue += 1.0;

	if(indexRealValueOut)
		*indexRealValueOut = lineIndexRealValue;
	return static_cast<int>(lineIndexRealValue);
}

bool NFmiParamCommandView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	return false;
}

NFmiRect NFmiParamCommandView::CalcParameterRowRect(int zeroBasedRowIndex) const
{
	NFmiRect rowRect = GetFrame();
	auto startMargin = ConvertMilliMeterToRelative(itsParameterRowVerticalMarginInMM, false);
	rowRect.Top(rowRect.Top() + startMargin + (zeroBasedRowIndex * itsLineHeight));
	rowRect.Height(itsLineHeight);
	return rowRect;
}

void NFmiParamCommandView::DrawCheckBox(const NFmiRect& parameterRowRect, bool isChecked)
{
	static NFmiDrawingEnvironment envi;
	static bool enviInitialized = false;
	if(enviInitialized == false)
	{
		enviInitialized = true;
		envi.SetFrameColor(NFmiColor(0, 0, 0));
		envi.SetFillColor(NFmiColor(1, 1, 1));
		envi.EnableFill();
	}
	NFmiRect rect(CheckBoxRect(parameterRowRect));
	DrawCheckBox(rect, envi, isChecked);
}
