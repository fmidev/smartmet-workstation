//**********************************************************
// C++ Class Name : NFmiViewParamCommandView
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiViewParamCommandView.cpp
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
#include "NFmiViewParamCommandView.h"
#include "NFmiString.h"
#include "NFmiToolBox.h"
#include "NFmiValueString.h"
#include "NFmiText.h"
#include "CtrlViewDocumentInterface.h"
#include "ToolBoxStateRestorer.h"

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiViewParamCommandView::NFmiViewParamCommandView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex)
:NFmiParamCommandView(theMapViewDescTopIndex, theRect, theToolBox, theDrawParam, theRowIndex, theColumnIndex, false)
{
}
//--------------------------------------------------------
// Draw
//--------------------------------------------------------
void NFmiViewParamCommandView::DrawData(void)
{
	NFmiValueString valuStr(CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber), "%d");
	NFmiString str(valuStr);
	str += ".";
	itsDrawingEnvironment.SetFontSize(itsFontSize);
	itsDrawingEnvironment.SetFontType(kArial);
	int zeroBasedRowIndex = 0;
	auto parameterRowRect = CalcParameterRowRect(zeroBasedRowIndex);
	NFmiText text(LineTextPlace(zeroBasedRowIndex, parameterRowRect, false), str, true, 0, &itsDrawingEnvironment);
	ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, kCenter, false);
	itsToolBox->Convert(&text);
}

//--------------------------------------------------------
// RightButtonDown
//--------------------------------------------------------
bool NFmiViewParamCommandView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
		return itsCtrlViewDocumentInterface->CreateParamSelectionPopup(itsMapViewDescTopIndex, GetUsedParamRowIndex()) == true;
	return false;
}

bool NFmiViewParamCommandView::LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) // Marko lis‰si 14.12.2001
{
	if(IsIn(thePlace))
	{
		itsCtrlViewDocumentInterface->ActivateParamSelectionDlgAfterLeftDoubleClick(true);
		return true;
	}
	return false;
}

//--------------------------------------------------------
// CalcSize
//--------------------------------------------------------
NFmiRect NFmiViewParamCommandView::CalcSize(void)
{
	NFmiRect returnRect(GetFrame());
	int lineCount = 1; // = "N‰yt‰"-rivi
// ruudun korkeus on rivien m‰‰r‰*rivinkorkeus + viidesosa rivin korkeudesta (v‰h‰n tilaa pohjalle)
	double heigth = lineCount * itsLineHeight + 0.5 * itsLineHeight;
	returnRect.Bottom(returnRect.Top() + heigth);
	return returnRect;
}

bool NFmiViewParamCommandView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(IsIn(thePlace))
	{
		if(theDelta < 0)
			return itsCtrlViewDocumentInterface->ScrollViewRow(itsMapViewDescTopIndex, 1);
		else
			return itsCtrlViewDocumentInterface->ScrollViewRow(itsMapViewDescTopIndex, -1);
	}
	return false;
}
