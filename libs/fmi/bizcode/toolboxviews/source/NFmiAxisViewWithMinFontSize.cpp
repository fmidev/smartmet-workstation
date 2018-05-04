//**********************************************************
// C++ Class Name : NFmiAxisViewWithMinFontSize 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: NFmiAxisViewWithMinFontSize.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 3 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : 2. uusi aikasarja viritys 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Fri - Dec 10, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// 
//**********************************************************

#include "NFmiAxisViewWithMinFontSize.h"
#include "NFmiToolBox.h"
#include "NFmiAxis.h"

NFmiAxisViewWithMinFontSize::NFmiAxisViewWithMinFontSize(const NFmiRect &theRect
                               ,NFmiToolBox *theToolBox
                               ,NFmiAxis *theAxis
                               ,FmiDirection theOrientation
                               ,FmiDirection theRotation
                               ,bool isAxis
                               ,bool isPrimaryText
                               ,bool isSecondaryText
                               ,float theTickLenght
							   ,FmiFontType theFontType
                               ,NFmiView *theEnclosure
                               ,NFmiDrawingEnvironment *theEnvironment
                               ,unsigned long theIdent
							   ,NFmiPoint theMinFontSize
							   ,NFmiPoint theMaxFontSize)
:NFmiAxisView(theRect, theToolBox, theAxis, theOrientation, theRotation
             ,isAxis, isPrimaryText, isSecondaryText, theTickLenght, theFontType
             ,theEnclosure, theEnvironment, theIdent)
,itsMinFontSize(theMinFontSize)
,itsMaxFontSize(theMaxFontSize)
{
}

NFmiAxisViewWithMinFontSize::~NFmiAxisViewWithMinFontSize(void)
{
}

void NFmiAxisViewWithMinFontSize::DrawAll(void)
{
	NFmiDrawingEnvironment theEnvironment;
	theEnvironment.EnableFrame();
	theEnvironment.SetFrameColor(NFmiColor(0.2f, 0.45f, 0.8f));
	theEnvironment.SetFillColor(NFmiColor(0.9f, 0.95f, 0.35f));
	theEnvironment.EnableFill();
	NFmiPoint oldFontSize(float(theEnvironment.GetFontWidth()), float(theEnvironment.GetFontHeight()));
	FmiFontType oldFontType = theEnvironment.GetFontType();

	double theHeight;          // tässä ilmeisesti virhe LW ja PK
	double relHeight = 0.0;
	if (NumberOfTextLines() > 0)	// 240297
	{
		int nChar = itsAxis->MaxTextLength();
		int nStep = itsAxis->Scale()->StepNumber();

		theEnvironment.SetFontType(itsFontType);//130297 kTimesNewRoman);	//040397
		float fac = 60.f; //090497 6 mm koko fontti
		if(itsOrientation == kDown || itsOrientation == kUp)
		{
			// voitaisiin käyttää nChar:ia ja nStep:iä mutta silloin gramissa eri ryhmissä
			//   olisi erikokoista fonttia (lisäoption paikka axisView:lle)
			double height1 = GetToolBox()->HX(CalcTickTextArea().Width()/3.4f); // 140297 oli 4; 3*1.8 = 5.4 merkkiä?
			double height2 = GetToolBox()->HY(CalcTickTextArea().Height()/3.4f); // 170297 
			theHeight = FmiMin(height1, height2);
			theHeight = FmiMax(theHeight, itsMinFontSize.X());
			theHeight = FmiMin(theHeight, itsMaxFontSize.X());
			if(theHeight > fac)
				theHeight = fac + (theHeight-fac) * .4f;	//260297
			relHeight = GetToolBox()->SY((long)theHeight);	//240297
		}
		else
		{
			double height1 = GetToolBox()->HY(CalcTickTextArea().Height() / NumberOfTextLines()) * 1.3f; //170297 kerroin lisätty
			double height2 = GetToolBox()->HX(CalcTickTextArea().Width() / (nChar * nStep));
			theHeight = FmiMin(height1, height2); //pitääkö olla täällä sisällä
			theHeight = FmiMax(theHeight, itsMinFontSize.X());
			theHeight = FmiMin(theHeight, itsMaxFontSize.X());
			if(theHeight > fac)
				theHeight = fac + (theHeight-fac) * .4f;	//260297
			relHeight = GetToolBox()->SY((long)theHeight);	 //090497
		}
		NFmiPoint newFontSize(theHeight, theHeight);
		theEnvironment.SetFontSize(newFontSize);
	}

	NFmiDrawingItem tmpitem(&theEnvironment);
	GetToolBox()->Convert(&tmpitem);

	if(fItsAxis)  
	DrawAxisBase();
	if(itsTickLenght > 0)
	{
		if(fItsAxis)   
		DrawTicks();
		if (NumberOfTextLines() > 0)	  //240297
			DrawTickTexts(relHeight);	  //240297
	}

	theEnvironment.SetFontSize(oldFontSize);
	theEnvironment.SetFontType(oldFontType); //010497
	NFmiDrawingItem tmpitem2(&theEnvironment);
	GetToolBox()->Convert(&tmpitem2);
}


