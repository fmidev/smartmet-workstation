//**********************************************************
// C++ Class Name : NFmiValueLine 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiValueLine.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 3 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : multipolyline 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Wed - Sep 8, 1999 
// 
// 
//  Description: 
//   Luokka viivalle jolla on jokin arvo. K‰ytet‰‰n 
//   esim. j‰rjestett‰ess‰ isoviivastoja.
//   
// 
//  Change Log: 
// 
//**********************************************************
#include "NFmiValueLine.h"
//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiValueLine::NFmiValueLine(void)
:itsStartPoint()
,itsEndPoint()
,itsValue(0)
{
}
NFmiValueLine::NFmiValueLine(const NFmiPoint& theStart, const NFmiPoint& theEnd, float theValue)
:itsStartPoint(theStart)
,itsEndPoint(theEnd)
,itsValue(theValue)
{
}

NFmiValueLine::NFmiValueLine(double x1, double y1, double x2, double y2, float theValue)
:itsStartPoint(x1, y1)
,itsEndPoint(x2, y2)
,itsValue(theValue)
{
}

NFmiValueLine::~NFmiValueLine(void)
{
}
