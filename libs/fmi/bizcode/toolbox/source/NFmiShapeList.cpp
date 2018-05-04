//© Ilmatieteenlaitos/Persa.
//13.12.1994   Persa
//
//Ver. 13.12.1994

//Ver. 

//--------------------------------------------------------------------------- nshaplis.cp


#include "NFmiShapeList.h"
#include "NFmiVoidPtrList.h"

//____________________________________________________________________ TFmiShapeList
NFmiShapeList::NFmiShapeList (const NFmiRect &theRect
                             ,NFmiView *theEnclosure
                             ,NFmiDrawingEnvironment *theEnvironment
                             ,ShapeIdentifier theIdent) 
             : NFmiShape(theRect, theEnclosure, theEnvironment, theIdent)
{
   itsShapes = new NFmiVoidPtrList ;
}
//____________________________________________________________________ ~NFmiShapeList
NFmiShapeList::~NFmiShapeList (void) 
{
  NFmiVoidPtrIterator shapeIterator(itsShapes);
  void *shape;
  while(shapeIterator.Next(shape))
    delete (static_cast<NFmiShape *>(shape));
    
  delete itsShapes;
}
//____________________________________________________________________ AddShape
void NFmiShapeList::AddShape (NFmiShape* newShape) 
{
   itsShapes->Add(static_cast<void *>(newShape));
}
//____________________________________________________________________ SwapX
void NFmiShapeList::SwapX (void) 
{
/**************************
   if(itsShapes->Ensimmainen())
   {
     while(itsShapes->OnJaljella())
     {
       ((TFmiShape*)(itsShapes->Alkio()))->SwapX();
       itsShapes->Seuraava();
     }
   }
**************************/
}
//____________________________________________________________________ SwapY
void NFmiShapeList::SwapY (void) 
{
/**************************
   if(itsShapes->Ensimmainen())
   {
     while(itsShapes->OnJaljella())
     {
       ((TFmiShape*)(itsShapes->Alkio()))->SwapY();
       itsShapes->Seuraava();
     }
   }
**************************/
}
               
               
