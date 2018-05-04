//© Ilmatieteenlaitos/Persa.
//13.11.1995   Persa
//
//Ver. 00.00.1995
//--------------------------------------------------------------------------- nsymbol.cpp



#include "NFmiSymbol.h"
#include "NFmiLine.h"
#include "NFmiPolyline.h"
#include "NFmiToolBox.h"
#include "NFmiShapeList.h"


//__________________________________________________________________________ NFmiSymbol
NFmiSymbol::NFmiSymbol (const NFmiRect &theRect
                       ,NFmiToolBox *theToolBox
                       ,NFmiView *theEnclosure
                       ,NFmiDrawingEnvironment *theEnvironment
                       ,ShapeIdentifier theIdent) 
         : NFmiMetaFileView(theRect
                        ,theToolBox
                        ,theEnclosure
                        ,theEnvironment
                        ,theIdent)
{
}
//__________________________________________________________________________ BuildMultiLine
void NFmiSymbol::BuildMultiLine (const float *xPoints,
                                 const float *yPoints,
                                 const int numberOfPoints,
                                 NFmiDrawingEnvironment *theEnvironment) 
{
  NFmiShapeList theShape (GetFrame()
                         ,0
                         ,theEnvironment);
                      
   for(int point = 0; point < (numberOfPoints - 1); point += 2)
   {
     NFmiPoint startingPoint(xPoints[point] * GetFrame().Width(),
                             yPoints[point] * GetFrame().Height());
                             
     NFmiPoint endingPoint(xPoints[point+1] * GetFrame().Width(),
                           yPoints[point+1] * GetFrame().Height());

     NFmiLine *aLine = new NFmiLine(startingPoint, endingPoint, 0, theEnvironment);
     theShape.AddShape(aLine);

   }
   GetToolBox()->Convert(&theShape);
}
//__________________________________________________________________________ BuildPolyLine
void NFmiSymbol::BuildPolyLine (const float *xPoints,
                                      const float *yPoints,
                                      const int numberOfPoints,
                                      NFmiDrawingEnvironment *theEnvironment) 
{
   NFmiPolyline aPolyline(GetFrame().TopLeft()
                         ,GetFrame().BottomRight()
                         ,0
                         ,theEnvironment);
   
   for(int point = 0; point < (numberOfPoints); point++)
   {
     NFmiPoint thePoint(xPoints[point] * GetFrame().Width() + GetFrame().Left(),
                        yPoints[point] * GetFrame().Height() + GetFrame().Top());
                             
     aPolyline.AddPoint(thePoint);

   }
   if(numberOfPoints > 2)
   {
     NFmiPoint thePoint(xPoints[0] * GetFrame().Width() + GetFrame().Left(),
                        yPoints[0] * GetFrame().Height() + GetFrame().Top());
     aPolyline.AddPoint(thePoint);
   }
   GetToolBox()->Convert(&aPolyline);
}
/*******
//__________________________________________________________________________ BuildRectangle
NFmiShape *NFmiSymbol::BuildRectangle (const float top,
                                 const float left,
                                 const float bottom,
                                 const float right,
                                 NFmiDrawingEnvironment *theEnvironment) 
{
   NFmiRectangle *theRectangle = new NFmiRectangle(NFmiPoint(left * GetSize().x + GetX(),
                                                              top * GetSize().y + GetY()
                                                            ),
                                                   NFmiPoint(right * GetSize().x + GetX(),
                                                              bottom * GetSize().y + GetY()
                                                            ),
                                                   theEnvironment);
   AddShape(theRectangle);
   return theRectangle;
}
//__________________________________________________________________________ BuildRainDrop
NFmiShape *NFmiSymbol::BuildRainDrop  (const NFmiPoint &thePlace,
                                       const NFmiPoint &theSize,
                                       NFmiDrawingEnvironment *theEnvironment) 
{
   NFmiDrop *theDrop = new NFmiDrop(thePlace,
                                    NFmiPoint(thePlace.GetX() + theSize.GetX(),
                                              thePlace.GetY() + theSize.GetY()
                                             ),
                                    theEnvironment);
   AddShape(theDrop);
   return theDrop;
}
//__________________________________________________________________________ BuildTriangle
NFmiShape *NFmiSymbol::BuildTriangle  (const float top,
                                 const float left,
                                 const float bottom,
                                 const float right,
                                 NFmiDrawingEnvironment *theEnvironment) 
{
   NFmiTriangle *theTriangle = new NFmiTriangle (NFmiPoint(left * GetSize().x + GetX(),
                                                           top * GetSize().y + GetY()
                                                          ),
                                                 NFmiPoint(right * GetSize().x + GetX(),
                                                           bottom * GetSize().y + GetY()
                                                          ),
                                                 theEnvironment);
   AddShape(theTriangle);
   return theTriangle;
}

//__________________________________________________________________________ BuildSnowFlake
NFmiShape *NFmiSymbol::BuildSnowFlake (const float top,
                                 const float left,
                                 const float bottom,
                                 const float right,
                                 NFmiDrawingEnvironment *theEnvironment) 
{
   NFmiSnowFlake *theSnowFlake = new NFmiSnowFlake (NFmiPoint(left * GetSize().x + GetX(),
                                                              top * GetSize().y + GetY())
                                                   ,NFmiPoint(right * GetSize().x + GetX(),
                                                              bottom * GetSize().y + GetY())
                                                   ,theEnvironment);
   AddShape(theSnowFlake);
   return theSnowFlake;
}

*****/
