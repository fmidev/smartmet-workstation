//© Ilmatieteenlaitos/Persa.
//12.9.1995   Persa
//
//Ver. 12.9.1995
//Ver. 24.04.1997/Persa 
//Ver. 14.05.1997/Persa itsMargins modified
//Ver. 14.05.1997/Persa CollectOwnDrawingItems modified
//Ver. 18.05.1997/Persa Constructor  and destructor fixed
//Ver. 23.09.1999/Viljo Added the parameter bool isUpsideDown to the constructor
//--------------------------------------------------------------------------- ntimscav.cpp

#include <iostream>


#include "NFmiTimeScaleView.h"
#include "NFmiLine.h"
#include "NFmiRectangle.h"
#include "NFmiText.h"
#include "NFmiToolBox.h"
#include "NFmiStepTimeScale.h"
#include "NFmiValueString.h"
#include "NFmiTimeScale.h"



//___________________________________________________________ NFmiTimeScaleView
NFmiTimeScaleView::NFmiTimeScaleView(const NFmiRect &theRect
                                    ,NFmiToolBox *theToolBox
                                    ,NFmiStepTimeScale *theBaseScale
                                    ,NFmiStepTimeScale *theSelectedScale
                                    ,NFmiView *theEnclosure
                                    ,NFmiDrawingEnvironment *theEnvironment
                                    ,unsigned long theIdent
									,bool isUpsideDown)
         : NFmiMetaFileView(theRect
                           ,theToolBox
                           ,theEnclosure
                           ,theEnvironment
                           ,theIdent)
         , itsBaseScale(new NFmiStepTimeScale(*theBaseScale))
         , itsSelectedScale(theSelectedScale ? new NFmiStepTimeScale(*theSelectedScale) : new NFmiStepTimeScale(*theBaseScale))
{
  // NFmiRect theFrame = GetDrawingArea();

  itsVerticalMargin = 0; //Persa..GetBorderWidth() * 3.; //(theFrame.Bottom() - theFrame.Top()) / 4.;
  itsHorisontalMargin = 0; //Persa..GetBorderWidth() * 3.; //itsVerticalMargin * 1. / GetToolBox()->WidthPerHeight() * 1.5;
  itsTickLenght = GetBorderWidth() * 3.;//Persa..itsVerticalMargin;
  fUpsideDown = isUpsideDown;
  
}
//___________________________________________________________ ~NFmiTimeScaleView
NFmiTimeScaleView::~NFmiTimeScaleView(void)
{
  delete itsBaseScale;
  delete itsSelectedScale;
}
//_________________________________________________________________________ CollectOwnDrawingItems

NFmiVoidPtrList &NFmiTimeScaleView::CollectOwnDrawingItems(NFmiVoidPtrList &theDrawingList)
{
//Persa poisti 14.5.97  NFmiMetaFileView::CollectOwnDrawingItems(theDrawingList);

  if(!itsBaseScale)
    return theDrawingList;

  NFmiDrawingEnvironment theEnvironment;

  NFmiRect subScaleRect(CalcSelectedArea());

  theEnvironment.SetFillColor(NFmiColor(0.95f,0.85f,0.75f));
  theEnvironment.EnableFill();
  theEnvironment.DisableFrame();
  NFmiRectangle tmp(subScaleRect.TopLeft(),
		    subScaleRect.BottomRight(),
		    0,
		    GetEnvironment() ? GetEnvironment() : &theEnvironment);
  GetToolBox()->ConvertShape(&tmp);

/***
  NFmiRectangle *aRect = new NFmiRectangle(subScaleRect.TopLeft()
                                         ,subScaleRect.BottomRight()
                                         ,0  //this
                                         ,&theEnvironment);
  theDrawingList.Add((void *)aRect);
***/  
  return DrawScale(theDrawingList
                  ,itsBaseScale
                  ,CalcScaleArea());
                  
                  
}
//_________________________________________________________________________ CalcScaleArea

NFmiRect NFmiTimeScaleView::CalcScaleArea(void) const
{    
  NFmiRect theFrame = GetDrawingArea();

  if(fUpsideDown)
  {
   return NFmiRect(theFrame.TopLeft() + GetToolBox()->ToViewPoint(static_cast<long>(itsHorisontalMargin), static_cast<long>(itsVerticalMargin))
                  ,theFrame.TopRight() + GetToolBox()->ToViewPoint(-static_cast<long>(itsHorisontalMargin), static_cast<long>(itsVerticalMargin) + static_cast<long>(itsTickLenght)));
  }
  else
  {
   return NFmiRect (theFrame.BottomLeft() + GetToolBox()->ToViewPoint(static_cast<long>(itsHorisontalMargin), -static_cast<long>(itsVerticalMargin))
                   ,theFrame.BottomRight() + GetToolBox()->ToViewPoint(-static_cast<long>(itsHorisontalMargin), -(static_cast<long>(itsVerticalMargin) + static_cast<long>(itsTickLenght))));
  }
										

//  NFmiRect baseScaleRect(theFrame.TopLeft() + GetToolBox()->ToViewPoint(itsHorisontalMargin, itsVerticalMargin)
//                        ,theFrame.TopRight() + GetToolBox()->ToViewPoint(-itsHorisontalMargin, itsVerticalMargin + itsTickLenght));

//  return baseScaleRect;
}
//_________________________________________________________________________ CalcSelectedArea

NFmiRect NFmiTimeScaleView::CalcSelectedArea(void)
{    
  NFmiRect baseScaleRect(CalcScaleArea());

  double startX = (static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->RelTimeLocation((static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->StartTime()) * baseScaleRect.Width();
  double endX = (static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->RelTimeLocation((static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->EndTime()) * baseScaleRect.Width();

  NFmiRect subScaleRect(baseScaleRect.TopLeft() + NFmiPoint(startX, 0)
                       ,baseScaleRect.BottomLeft() + NFmiPoint(endX, 0));
  return subScaleRect;
}
//_________________________________________________________________________ MoveScale

void NFmiTimeScaleView::MoveScale(const NFmiPoint &upTo)
{
  NFmiPoint where(AdjustToScaleArea(upTo));

  (static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->Move((static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->GetTime(static_cast<float>(where.X())));

  if((static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->StartTime() < (static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->StartTime())
    (static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->MoveLeft((static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->StartTime()); 

  if((static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->EndTime() > (static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->EndTime())
    (static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->MoveRight((static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->EndTime());

  SetDirty(); 
}
//_________________________________________________________________________ ResizeScale

void NFmiTimeScaleView::ResizeScale(const NFmiPoint &upTo)
{    
  NFmiPoint where(AdjustToScaleArea(upTo));

  (static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->Resize((static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->GetTime(static_cast<float>(where.X())));

  if((static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->StartTime() < (static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->StartTime())
    (static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->MoveLeft((static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->StartTime()); 

  if((static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->EndTime() > (static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->EndTime())
    (static_cast<NFmiTimeScale *>(itsSelectedScale->Scale()))->MoveRight((static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->EndTime());

  SetDirty(); 
}
//_________________________________________________________________________ AdjustToScaleArea

NFmiPoint NFmiTimeScaleView::AdjustToScaleArea(const NFmiPoint &klickedPoint)
{    
  NFmiRect scaleArea(CalcScaleArea());
  NFmiRect theFrame(GetFrame());
  double xScale = theFrame.Width() / scaleArea.Width();
  double yScale = theFrame.Height() / scaleArea.Height();
  NFmiPoint adjustedPoint(klickedPoint.X() * xScale - scaleArea.Left() * xScale
                         ,klickedPoint.Y() * yScale - scaleArea.Top() * yScale);

  return adjustedPoint;
}
//_________________________________________________________________________ CollectOwnDrawingItems

NFmiVoidPtrList &NFmiTimeScaleView::DrawScale(NFmiVoidPtrList &theDrawingList
                                             ,NFmiStepTimeScale *theScale
                                             ,const NFmiRect &theRect)
{  
  NFmiDrawingEnvironment theEnvironment;
  theEnvironment.SetFrameColor(NFmiColor(0.4f,0.0f,0.0f));
  theEnvironment.EnableFrame();
 
  double y1, y2, y3, y4; //y4 laura 27041999

  NFmiPoint fontSize(GetToolBox()->ToViewPoint(8, 12));  //Tämä pitäisi olla paremmin

  if(fUpsideDown)
  {
    y1 = theRect.Top();
    y2 = theRect.Bottom();
    y3 = y2 + fontSize.Y();
    y4 = y3 + fontSize.Y();
  }
  else
  {
    y1 = theRect.Bottom();
    y2 = theRect.Top();
    y3 = y2;
	y4 = y2-fontSize.Y();
  }

  NFmiLine tmp(NFmiPoint(theRect.Left(), y1),
	       NFmiPoint(theRect.Right(), y1),
	       0,
	       &theEnvironment);
  GetToolBox()->ConvertShape(&tmp);

//  NFmiTimeScaleIterator itsTime(theScale);
  theScale->Reset();
  float cursor;
  while(theScale->Next(cursor))
  {
    double x = theRect.Left() + theScale->Scale()->RelLocation(cursor) * theRect.Width();

    TFmiTime aTime(theScale->CurrentTime());

	int odd = aTime.GetDay()%2;	//persa (laura) 27041999 (joka toinen p‰iv‰ eri v‰rill‰)
	theEnvironment.SetFrameColor(NFmiColor(odd*1.0f,0.0f,(1-odd)*1.0f)); //persa (laura) 
	NFmiLine tmpline(NFmiPoint(x,y1), NFmiPoint(x,y2), 0, &theEnvironment);
	GetToolBox()->ConvertShape(&tmpline);

//    itsTime.CurrentTime(aTime);

	if(aTime.GetHour()%3 == 0) //Laura 02061999 //piirret‰‰n vain joka kolmas tunti
	  {
	    NFmiValueString tmpstr;
	    if(NFmiValueString(aTime.GetHour()).GetLen() <= 1)
	      tmpstr += NFmiValueString("0");
	    tmpstr += NFmiValueString(aTime.GetHour());

	    NFmiText tmptext(NFmiPoint(x - fontSize.X()/2., y3), tmpstr, 0, &theEnvironment);
	    GetToolBox()->ConvertShape(&tmptext);
	}

	if(aTime.GetHour() == 12) //laura 27041999 (p‰iv‰m‰‰r‰t)
	{
	  NFmiValueString tmpstr;
	  if(NFmiValueString(aTime.GetDay()).GetLen() <= 1)
	    tmpstr += NFmiValueString("0");
	  tmpstr += NFmiValueString(aTime.GetDay());
	  NFmiText tmptext(NFmiPoint(x - fontSize.X()/2., y4), tmpstr, 0, &theEnvironment);
	  GetToolBox()->ConvertShape(&tmptext);
	}
  }

  return theDrawingList;
}

NFmiMetTime NFmiTimeScaleView::GetTime(const NFmiPoint &clickedPoint, bool fExact)
{
    NFmiTime aTime = (static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->GetTime(static_cast<float>(RelativePoint(clickedPoint).X()));
    if(fExact)
        return NFmiMetTime(aTime ,1);
    else
        return NFmiMetTime(aTime); // default 60 minuutin resoluutio
}

NFmiMetTime NFmiTimeScaleView::GetAccurateTime(const NFmiPoint &clickedPoint, int theResolutionInMinutes)
{
	if(theResolutionInMinutes <= 0)
		theResolutionInMinutes = 1;
	return NFmiMetTime((static_cast<NFmiTimeScale *>(itsBaseScale->Scale()))->GetTime(static_cast<float>(RelativePoint(clickedPoint).X())), theResolutionInMinutes);
}

void NFmiTimeScaleView::SetSelectedScale(NFmiStepTimeScale *newScale)
{
	if(itsSelectedScale)
		delete itsSelectedScale;
	itsSelectedScale = new NFmiStepTimeScale(*newScale);
}

NFmiPoint NFmiTimeScaleView::RelativePoint(const NFmiPoint &theAbsolutePoint)
{    
	NFmiPoint adjustedPoint(AdjustToScaleArea(theAbsolutePoint));
	adjustedPoint.X(adjustedPoint.X()/GetFrame().Width());
	adjustedPoint.Y(adjustedPoint.Y()/GetFrame().Height());

	return adjustedPoint;
}

