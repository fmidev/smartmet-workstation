//**********************************************************
// C++ Class Name : NFmiAxisViewWithMinFontSize 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: NFmiAxisViewWithMinFontSize.h 
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

#pragma once

#include "NFmiAxisView.h"

class NFmiAxisViewWithMinFontSize : public NFmiAxisView
{

 public:
	NFmiAxisViewWithMinFontSize(const NFmiRect &theRect
                               ,NFmiToolBox *theToolBox
                               ,NFmiAxis *theAxis
                               ,FmiDirection theOrientation
                               ,FmiDirection theRotation = kRight
                               ,bool isAxis = true
                               ,bool isPrimaryText = true
                               ,bool isSecondaryText = false
                               ,float theTickLenght = 0.2
							   ,FmiFontType = kTimesNewRoman
                               ,NFmiView *theEnclosure = 0
                               ,NFmiDrawingEnvironment *theEnvironment = 0
                               ,unsigned long theIdent = 0
							   ,NFmiPoint theMinFontSize = NFmiPoint(14,14)
							   ,NFmiPoint theMaxFontSize = NFmiPoint(24,24));
	virtual ~NFmiAxisViewWithMinFontSize(void);
	virtual void DrawAll(void);

 private:
	NFmiPoint itsMinFontSize;
	NFmiPoint itsMaxFontSize;
};

