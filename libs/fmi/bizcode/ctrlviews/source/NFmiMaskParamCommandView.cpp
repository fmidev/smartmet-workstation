//**********************************************************
// C++ Class Name : NFmiMaskParamCommandView 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiMaskParamCommandView.cpp 
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
#include "NFmiMaskParamCommandView.h"
#include "NFmiString.h"
#include "NFmiToolBox.h"
#include "NFmiText.h"
#include "CtrlViewDocumentInterface.h"
#include "ToolBoxStateRestorer.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiMaskParamCommandView::NFmiMaskParamCommandView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex)
:NFmiParamCommandView(theMapViewDescTopIndex, theRect, theToolBox, theDrawParam, theRowIndex, theColumnIndex, false)
{
}
//--------------------------------------------------------
// Draw 
//--------------------------------------------------------
void NFmiMaskParamCommandView::DrawData(void)
{
	NFmiString str("M.");
	itsDrawingEnvironment.SetFontSize(itsFontSize);
	itsDrawingEnvironment.SetFontType(kArial);
	int zeroBasedRowIndex = 0;
	auto parameterRowRect = CalcParameterRowRect(zeroBasedRowIndex);
	NFmiText text(LineTextPlace(zeroBasedRowIndex, parameterRowRect, false), str, false, 0, &itsDrawingEnvironment);
	ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, kCenter, true);
	itsToolBox->Convert(&text);
}

//--------------------------------------------------------
// RightButtonDown
//--------------------------------------------------------
bool NFmiMaskParamCommandView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
		return itsCtrlViewDocumentInterface->CreateMaskSelectionPopup(itsViewGridRowNumber) == true;
	return false;
}

//--------------------------------------------------------
// CalcSize
//--------------------------------------------------------
NFmiRect NFmiMaskParamCommandView::CalcSize(void)
{
	NFmiRect returnRect(GetFrame());
	int lineCount = 1; // = "Maski"-rivi
// ruudun korkeus on rivien m‰‰r‰*rivinkorkeus + viidesosa rivin korkeudesta (v‰h‰n tilaa pohjalle)
	double heigth = lineCount * itsLineHeight + 0.5 * itsLineHeight;
	returnRect.Bottom(returnRect.Top() + heigth);
	return returnRect;
}
