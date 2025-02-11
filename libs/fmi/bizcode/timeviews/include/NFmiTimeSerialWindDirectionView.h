//**********************************************************
// C++ Class Name : NFmiTimeSerialWindDirectionView 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiTimeSerialWindDirectionView.h 
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
//  Creation Date  : Tues - Apr 6, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// 
//**********************************************************

#pragma once

#include "NFmiTimeSerialView.h"


class NFmiTimeSerialWindDirectionView : public NFmiTimeSerialView
{

 public:
    NFmiTimeSerialWindDirectionView		(int theMapViewDescTopIndex, const NFmiRect & theRect
										,NFmiToolBox * theToolBox
										,boost::shared_ptr<NFmiDrawParam> &theDrawParam
										,int theIndex);
   virtual  ~NFmiTimeSerialWindDirectionView (void);
 
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   void DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines) override;
   void CreateValueScale (void) override;
   void DrawValueAxis (void) override;
   using NFmiTimeSerialView::DrawDataLine; 
   virtual void DrawDataLine (const NFmiMetTime& theTime1,  const NFmiMetTime& theTime2, double value1, double value2, NFmiDrawingEnvironment & envi);
   virtual void EvaluateValue (double& theValue);

 protected:
   void DrawData (void) override;
   void DrawGrids (NFmiDrawingEnvironment & envi) override;
   void DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos);
   void DrawModifyFactorPointGrids (void);
   void DrawSelectedStationData (void);
   void DrawBackground (void);
   void CreateValueScaleView (void) override;
   bool AutoAdjustValueScale(void){return false;};

};

