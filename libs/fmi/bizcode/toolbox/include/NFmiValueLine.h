//**********************************************************
// C++ Class Name : NFmiValueLine 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiValueLine.h 
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

#pragma once

#include "NFmiPoint.h"

class NFmiValueLine 
{

 public:
   NFmiValueLine(void);
   NFmiValueLine(const NFmiPoint& theStart, const NFmiPoint& theEnd, float theValue);
   NFmiValueLine(double x1, double y1, double x2, double y2, float theValue);
   ~NFmiValueLine(void);
   const NFmiPoint& Start(void) const{return itsStartPoint;};
   const NFmiPoint& End(void) const{return itsEndPoint;};
   float Value(void) const{return itsValue;};
   void Start(const NFmiPoint& thePoint){itsStartPoint=thePoint;};
   void End(const NFmiPoint& thePoint){itsEndPoint=thePoint;};
   void Value(float newValue){itsValue=newValue;};

 private:
   NFmiPoint itsStartPoint;
   NFmiPoint itsEndPoint;
   float itsValue;

};

