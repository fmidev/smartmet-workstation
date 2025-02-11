//**********************************************************
// C++ Class Name : NFmiTimeSerialWindDirectionView 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiTimeSerialWindDirectionView.cpp 
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
#include "NFmiTimeSerialWindDirectionView.h"
//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiTimeSerialWindDirectionView::NFmiTimeSerialWindDirectionView (int theMapViewDescTopIndex, const NFmiRect & theRect
															     ,NFmiToolBox * theToolBox
															     ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
																 ,int theIndex)
:NFmiTimeSerialView(theMapViewDescTopIndex, theRect
				  ,theToolBox
				  ,theDrawParam
				  ,theIndex)
{
}
NFmiTimeSerialWindDirectionView::~NFmiTimeSerialWindDirectionView (void)
{
}
//--------------------------------------------------------
// LeftButtonUp 
//--------------------------------------------------------
bool NFmiTimeSerialWindDirectionView::LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey)
{
   bool returnVal = false;
   return returnVal;
}
//--------------------------------------------------------
// RightButtonUp 
//--------------------------------------------------------
bool NFmiTimeSerialWindDirectionView::RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey)
{
   bool returnVal = false;
   return returnVal;
}
//--------------------------------------------------------
// DrawData 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::DrawData (void)
{
   return;
}
//--------------------------------------------------------
// DrawGrids 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::DrawGrids (NFmiDrawingEnvironment & envi)
{
   return;
}
//--------------------------------------------------------
// DrawValueGrids 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::DrawValueGrids (NFmiDrawingEnvironment & envi, double minPos, double maxPos)
{
   return;
}
//--------------------------------------------------------
// DrawModifyFactorPointGrids 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::DrawModifyFactorPointGrids (void)
{
   return;
}
//--------------------------------------------------------
// DrawSelectedStationData 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::DrawSelectedStationData (void)
{
   return;
}
//--------------------------------------------------------
// DrawBackground 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::DrawBackground (void)
{
   return;
}
//--------------------------------------------------------
// DrawValueAxis 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::DrawValueAxis (void)
{
   return;
}
//--------------------------------------------------------
// CreateValueScaleView 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::CreateValueScaleView (void)
{
   return;
}
//--------------------------------------------------------
// DrawLocationInTime 
//--------------------------------------------------------
//void NFmiTimeSerialWindDirectionView::DrawLocationInTime (void)
void NFmiTimeSerialWindDirectionView::DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines)
{
   return;
}
//--------------------------------------------------------
// CreateValueScale 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::CreateValueScale (void)
{
   return;
}
//--------------------------------------------------------
// DrawDataLine 
//--------------------------------------------------------
void NFmiTimeSerialWindDirectionView::DrawDataLine (const NFmiMetTime& theTime1,  const NFmiMetTime& theTime2, double value1, double value2, NFmiDrawingEnvironment & envi)
{
   return;
}
//--------------------------------------------------------
// EvaluateValue 
//--------------------------------------------------------

//   Asettaa uuden arvon datalle muutoskäyrää 
//   piirrettäessä. Esim. kok.pilv. 90 + 10 -> 
//   100 ja WD 350 + 20 -> 10, jne.
void NFmiTimeSerialWindDirectionView::EvaluateValue (double& theValue)
{
   return;
}
