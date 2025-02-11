//**********************************************************
// C++ Class Name : NFmiMaskParamsView 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiMaskParamsView.h 
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

#pragma once

#include "NFmiParamCommandView.h"

class NFmiAreaMask;

class NFmiMaskParamsView : public NFmiParamCommandView
{

 public:
   NFmiMaskParamsView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex);
   bool RightButtonUp(const NFmiPoint &thePlace, unsigned long theKey) override;
   bool LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey) override;
   NFmiRect CalcSize(void); // koko saattaa muuttua, ja uutta kokoa pit‰‰ voida kysy‰ oliolta

 protected:
   void DrawData(void) override;

};

