//© Ilmatieteenlaitos/Persa.
//08.12.1994   Persa
//
//Ver. 08.12.1994
//Ver. 11.05.1997/Persa itsBackgroundColor added
//--------------------------------------------------------------------------- nview.cp




#include "NFmiView.h"
#include "NFmiToolBox.h"
#include "NFmiVoidPtrList.h"
#include "NFmiRectangle.h"

//___________________________________________________________ NFmiView
NFmiView::NFmiView(const NFmiRect &theRect
                  ,NFmiToolBox *theToolBox
                  ,const NFmiView *theEnclosure
                  ,NFmiDrawingEnvironment *theEnvironment
                  ,unsigned long theIdent)
         : NFmiDrawingItem(theEnvironment, theIdent)
		 , itsBackgroundColor(new NFmiColor(0.8f, 0.8f, 1.f))
		 , itsToolBox(0)
//       , itsToolBox(theToolBox ? theToolBox : theEnclosure ? theEnclosure->GetToolBox() : 0)
         , itsRelativeRect(new NFmiRect(theRect))
         , itsAbsoluteRect(0)
         , itsEnclosure(theEnclosure)
         , itsBorderWidth(3.f)
         , itsFrame(false)
         , itsSubViews(0)
//         , itsDirtyFlag(true)
//         , itsRect(0)
{
  if(theToolBox)
	itsToolBox = theToolBox;
  else if(theEnclosure)
	itsToolBox = theEnclosure->GetToolBox();
  SetAbsoluteRect();                                          
}
/*
//___________________________________________________________ NFmiView
NFmiView::NFmiView(const NFmiView &theView)
{
}
*/
//___________________________________________________________ ~NFmiView
NFmiView::~NFmiView(void)
{
  delete itsAbsoluteRect;
  delete itsRelativeRect;
  delete itsBackgroundColor;
}
/*
//___________________________________________________________ SetRect
void NFmiView::SetRect(const NFmiRect &theRect)
{
  if(*itsRect != theRect)
  {
    SetDirty();
    delete itsRect;
    itsRect = new NFmiRect(theRect);
  }
}
//___________________________________________________________ SetRect
void NFmiView::SetRect(const NFmiRect &theRect)
{
  if(itsRect != 0 && *itsRect == theRect)
    return;

  
  if(itsRect == 0)
    itsRect = new NFmiRect(theRect);
  else
    *itsRect = theRect;

  SetDirty();
//  CalcRect();
  
  if(itsSubViews)
  {
    NFmiVoidPtrIterator theIterator(itsSubViews);
    void *theItem;
    while(theIterator.Next(theItem))
      ((NFmiView *)theItem)->SetRect(theRect);
  }
}
//___________________________________________________________ CalcRect
void NFmiView::CalcRect(void)
{
  if(itsEnclosure)
  {
    if(itsRect)
      delete itsRect;
    itsRect = new NFmiRect(itsMap->Transform(itsEnclosure->GetFrame()));
  }
}
*/

// asetetaan relatiivinen rect uudestaan, pitää sitten myös päivittää absoluuttinen samalla
void  NFmiView::SetRelativeRect(const NFmiRect &theRect)
{
	if(itsRelativeRect)
		*itsRelativeRect = theRect;
	else
		itsRelativeRect = new NFmiRect(theRect);
	SetAbsoluteRect();
}

//___________________________________________________________ SetAbsoluteRect
void NFmiView::SetAbsoluteRect(void)
{
  if(itsEnclosure)
  {
    if(itsAbsoluteRect)
      delete itsAbsoluteRect;
    itsAbsoluteRect = new NFmiRect
                          (itsEnclosure->GetAbsoluteRect().Right() + itsRelativeRect->Right() * itsEnclosure->GetAbsoluteRect().Width()
                          ,itsEnclosure->GetAbsoluteRect().Top() + itsRelativeRect->Top() * itsEnclosure->GetAbsoluteRect().Height()
                          ,itsEnclosure->GetAbsoluteRect().Left() + itsRelativeRect->Left() * itsEnclosure->GetAbsoluteRect().Width()
                          ,itsEnclosure->GetAbsoluteRect().Bottom() + itsRelativeRect->Bottom() * itsEnclosure->GetAbsoluteRect().Height()
                          );
  }
  else
  {
    if(itsAbsoluteRect)
      delete itsAbsoluteRect;
    itsAbsoluteRect = new NFmiRect(*itsRelativeRect);
  }
}
//___________________________________________________________ AddSubView
void NFmiView::AddSubView(NFmiView *theSubView)
{
  if(theSubView)
  {
    if(!itsSubViews)
      itsSubViews = new NFmiVoidPtrList;
    
    itsSubViews->Add(static_cast<void *>(theSubView));
  }
}
//_________________________________________________________________________ BuildFrame
NFmiRect NFmiView::BuildFrame(const NFmiColor &theFrameColor
                             ,const NFmiColor &theFillColor
                             ,NFmiVoidPtrList &theDrawingList)
{

  return itsToolBox->BuildFrame(this, theFrameColor, theFillColor, theDrawingList);
  
}
//_________________________________________________________________________ BuildFrame
NFmiRect NFmiView::BuildBackground(const NFmiColor &theFillColor
                                  ,NFmiVoidPtrList &theDrawingList)
{

  NFmiRect theFrame = GetFrame();
  
  NFmiDrawingEnvironment theEnvironment;
  theEnvironment.DisableFrame();
  theEnvironment.SetFillColor(theFillColor);
  theEnvironment.EnableFill();
  NFmiRectangle *aRect = new NFmiRectangle(NFmiPoint(theFrame.Left(), theFrame.Top())
                                          ,NFmiPoint(theFrame.Right(), theFrame.Bottom())
                                          ,0  //this
                                          ,&theEnvironment);
  theDrawingList.Add(static_cast<void *>(aRect));

  return theFrame;
}
//___________________________________________________________ SetAbsoluteRect
NFmiPoint NFmiView::GetRelativeBorderWidth(void) const

{
  return itsToolBox->ToViewPoint(long(itsBorderWidth), long(itsBorderWidth));
}

NFmiView& NFmiView::operator=(const NFmiView &theView)
{
	if(itsAbsoluteRect)
		delete itsAbsoluteRect;
	if(itsRelativeRect)
		delete itsRelativeRect;
	if(itsBackgroundColor)
		delete itsBackgroundColor;

	NFmiDrawingItem::operator=(theView);
	itsBackgroundColor = new NFmiColor(*theView.itsBackgroundColor);
	itsToolBox = theView.itsToolBox;
	itsRelativeRect = new NFmiRect(*theView.itsRelativeRect);
	itsAbsoluteRect = new NFmiRect(*theView.itsAbsoluteRect);
	itsEnclosure = theView.itsEnclosure;
	itsBorderWidth = theView.itsBorderWidth;
	itsFrame = theView.itsFrame;
	itsSubViews = theView.itsSubViews;

	return *this;
}

void  NFmiView::SetBackColor(const NFmiColor &newColor) 
{
	delete itsBackgroundColor;
	itsBackgroundColor = new NFmiColor(newColor);
}

