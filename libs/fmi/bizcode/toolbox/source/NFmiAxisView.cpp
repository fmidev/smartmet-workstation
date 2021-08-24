#include "NFmiAxisView.h"
#include "NFmiToolBox.h"
#include "NFmiText.h"
#include "NFmiLine.h"
#include "NFmiAxis.h"

//___________________________________________________________ NFmiGramDataView
NFmiAxisView::NFmiAxisView(const NFmiRect &theRect
                          ,NFmiToolBox *theToolBox
                          ,NFmiAxis *theAxis
                          ,FmiDirection theOrientation	// alas: Or=Ri,Ro=Ri
                          ,FmiDirection theRotation		// vasen:Or=Do,Ro=Ri
                          ,bool isAxis			// ylös: Or=Ri,Ro=Le
                          ,bool isPrimaryText	  //eikö akselilla jo ole tietoa
                          ,bool isSecondaryText //     -"-
                          ,float theTickLength
						  ,FmiFontType theFontType	  //130397
                          ,NFmiView *theEnclosure
                          ,NFmiDrawingEnvironment *theEnvironment
                          ,unsigned long theIdent)
         : NFmiMetaFileView(theRect
                           ,theToolBox
                           ,theEnclosure
                           ,theEnvironment
                           ,theIdent)
          ,itsAxis(theAxis)
	  ,itsFontType(theFontType)
          ,itsTickLenght(theTickLength)  
          ,itsOrientation(theOrientation)  
          ,itsRotation(theRotation)
          ,fItsAxis(isAxis)
          ,fItsPrimaryText(isPrimaryText)  
          ,fItsSecondaryText(isSecondaryText)
{
}
//___________________________________________________________ ~NFmiAxisView
NFmiAxisView::~NFmiAxisView(void)
{
//  delete itsAxis;
}
//_________________________________________________________________________ DrawAll

void NFmiAxisView::DrawAll(void)
{
  NFmiDrawingEnvironment theEnvironment;
  theEnvironment.EnableFrame();
  theEnvironment.SetFrameColor(NFmiColor(0.2f, 0.45f, 0.8f));
  theEnvironment.SetFillColor(NFmiColor(0.9f, 0.95f, 0.35f));
  theEnvironment.EnableFill();
//  GetToolBox()->Convert(&NFmiDrawingItem(&theEnvironment));

/***Persan lisays 9.1.1995 alkaa ****/

  NFmiPoint oldFontSize(float(theEnvironment.GetFontWidth()), float(theEnvironment.GetFontHeight()));
  FmiFontType oldFontType = theEnvironment.GetFontType();

  double theHeight;          // tässä ilmeisesti virhe LW ja PK
  double relHeight = 0.0;
  if (NumberOfTextLines() > 0)	// 240297
  {
	 int nChar = itsAxis->MaxTextLength();
	 int nStep = itsAxis->Scale()->StepNumber();

     theEnvironment.SetFontType(itsFontType);//130297 kTimesNewRoman);	//040397
//	 NFmiRect apu = CalcTickTextArea(); //*****
	 float fac = 60.f; //090497 6 mm koko fontti
     if(itsOrientation == kDown || itsOrientation == kUp)
     {
		 // voitaisiin käyttää nChar:ia ja nStep:iä mutta silloin gramissa eri ryhmissä
		 //   olisi erikokoista fonttia (lisäoption paikka axisView:lle)
       double height1 = GetToolBox()->HX(CalcTickTextArea().Width()/3.f); // 140297 oli 4; 3*1.8 = 5.4 merkkiä?
       double height2 = GetToolBox()->HY(CalcTickTextArea().Height()/3.f); // 170297 
       theHeight = FmiMin(height1, height2);
	   if(theHeight > fac)
		 theHeight = fac + (theHeight-fac) * .4f;	//260297
	   relHeight = GetToolBox()->SY(static_cast<long>(theHeight));
     }
     else
     {
       double height1 = GetToolBox()->HY(CalcTickTextArea().Height() / NumberOfTextLines()) * 1.3f; //170297 kerroin lisätty
//       double height2 = GetToolBox()->HX(CalcTickTextArea().Width() / 3.f); //240297 
 	   double height2 = GetToolBox()->HX(CalcTickTextArea().Width() / (nChar * nStep));
	   theHeight = FmiMin(height1, height2); //pitääkö olla täällä sisällä
	   if(theHeight > fac)
		 theHeight = fac + (theHeight-fac) * .4f;	//260297
	   relHeight = GetToolBox()->SY(static_cast<long>(theHeight));
     }
     NFmiPoint newFontSize(theHeight, theHeight);
     theEnvironment.SetFontSize(newFontSize);
  }

/****Persan lisays 9.1.1995 loppuu***/

  NFmiDrawingItem tmpdummy(&theEnvironment);
  GetToolBox()->Convert(&tmpdummy);

  if(fItsAxis)  
    DrawAxisBase();
  if(itsTickLenght > 0)
  {
      if(fItsAxis)   
         DrawTicks();
      if (NumberOfTextLines() > 0)	  //240297
	     DrawTickTexts(relHeight);	  //240297
  }
//  DrawHeader();

/***Persan lisays 9.1.1995 alkaa ****/
  theEnvironment.SetFontSize(oldFontSize);
  theEnvironment.SetFontType(oldFontType); //010497
  NFmiDrawingItem tmpdummy2(&theEnvironment);
  GetToolBox()->Convert(&tmpdummy2);
/****Persan lisays 9.1.1995 loppuu***/

}
//_________________________________________________________________________ DrawItem

void NFmiAxisView::DrawHeader(void)
{
  NFmiDrawingEnvironment theEnvironment;
  theEnvironment.EnableFrame();
  theEnvironment.SetFrameColor(NFmiColor(0.2f, 0.45f, 0.3f));
  
  NFmiText tmp(HeaderPlace(),itsAxis->Text(), false);
  GetToolBox()->Convert(&tmp);
}
//_________________________________________________________________________ HeaderPlace

NFmiPoint NFmiAxisView::HeaderPlace(void)
{
  NFmiRect theRect(GetFrame());

  switch(itsOrientation)
  {
  case kRight:
    if(itsRotation == kRight)
      return theRect.BottomLeft();
    else
      return theRect.TopLeft();
  case kDown:
    if(itsRotation == kRight)
      return theRect.TopLeft();
    else
      return theRect.TopLeft();
  default:
    return theRect.TopLeft(); //tassa pitaisi olla jotain jarkevaa
  }
}
//_________________________________________________________________________ CalcAxisArea

NFmiRect NFmiAxisView::CalcAxisArea(void)
{
  NFmiRect theRect(GetFrame());
  NFmiPoint topLeft;
  NFmiPoint bottomRight;
  double theTick = CalcTickLength(theRect);
  
  switch(itsOrientation)
  {
  case kRight:

    if(itsRotation == kRight)
    {
      topLeft     = theRect.TopLeft();
      bottomRight = theRect.TopRight() + NFmiPoint(0., theTick);
    }
    else
    {
      topLeft     = theRect.TopLeft() + NFmiPoint(0., theTick);
      bottomRight = theRect.BottomRight();
    }
    break;
  case kDown:

    if(itsRotation == kRight)
    {
      topLeft     = theRect.TopRight() + NFmiPoint(-theTick, 0.);
      bottomRight = theRect.BottomRight();
    }
    else
    {
      topLeft     = theRect.TopLeft();
      bottomRight = theRect.BottomLeft() + NFmiPoint(theTick, 0.);
    }
    break;
  default:
    break;
  }  
  return NFmiRect(topLeft, bottomRight);
}
//_________________________________________________________________________ CalcTickTextArea

NFmiRect NFmiAxisView::CalcTickTextArea(void)
{
  NFmiRect theRect(GetFrame());
  NFmiPoint topLeft;
  NFmiPoint bottomRight;
  double theTick;
  theTick = CalcTickLength(theRect);

  switch(itsOrientation)
  {
  case kRight:

    if(itsRotation == kRight)
    {
      topLeft     = theRect.TopLeft() + NFmiPoint(0., theTick);
      bottomRight = theRect.BottomRight();
    }
    else
    {
      topLeft     = theRect.TopLeft();
      bottomRight = theRect.BottomRight() + NFmiPoint(0., -theTick);
    }
    break;
  case kDown:

    if(itsRotation == kRight)
    {
      topLeft     = theRect.TopLeft();
      bottomRight = theRect.BottomRight() + NFmiPoint(-theTick, 0.);
    }
    else
    {
      topLeft     = theRect.TopLeft() + NFmiPoint(theTick, 0.);
      bottomRight = theRect.BottomRight();
    }
    break;
  default:
    break;
  }
    
  return NFmiRect(topLeft, bottomRight);
}
//_________________________________________________________________________ TickStart

NFmiPoint NFmiAxisView::TickStart(const NFmiRect &theRect
                                 ,double theRelativeLocation)
{
  double x = 0.0;
  double y = 0.0;
  double theTick = CalcTickLength(theRect);
  
  switch(itsOrientation)
  {
  case kRight:
    x = theRect.Left() + theRelativeLocation * theRect.Width();
    if(itsRotation == kRight)
      y = theRect.Top();
    else
      y =  theRect.Bottom() - theTick;
    break;
  case kDown:
    y = theRect.Top() + theRelativeLocation * theRect.Height();
    if(itsRotation == kRight)
      x = theRect.Right();
    else
      x = theRect.Left() + theTick;
    break;
  default:
    break;
  
  }
    
  return NFmiPoint(x,y);

}
//_________________________________________________________________________ TickEnd

NFmiPoint NFmiAxisView::TickEnd(const NFmiRect &theRect
                               ,double theRelativeLocation)
{
  double x, y;
  double theTick = CalcTickLength(theRect);
  
  switch(itsOrientation)
  {
  case kRight:
    x = theRect.Left() + theRelativeLocation * theRect.Width();
    if(itsRotation == kRight)
      y = theRect.Top() + theTick;
    else
      y =  theRect.Bottom();
    break;
  case kDown:
    y = theRect.Top() + theRelativeLocation * theRect.Height();
    if(itsRotation == kRight)
      x = theRect.Right() - theTick;
    else
      x = theRect.Left();
    break;
  default:
    x = 0.;  //tassa pitaisi olla jotain jarkevaa
    y = 0.;
  }
    
  return NFmiPoint(x,y);

}
//_________________________________________________________________________ TickTextStart

NFmiPoint NFmiAxisView::TickTextStart(const NFmiRect &theRect
                                     ,double theRelativeLocation
                                     ,const NFmiString &theText)
{
  double x, y;
  
  switch(itsOrientation)
  {
  case kRight:
    if(itsRotation == kRight)
	{
		x = theRect.Left() + theRelativeLocation * theRect.Width() - GetToolBox()->MeasureText(theText) / 2.;
		y = theRect.Bottom() - (itsCurrentTextLine - 1) * theRect.Height() / NumberOfTextLines();
	}
	else
	{
		x = theRect.Left() + theRelativeLocation * theRect.Width() - GetToolBox()->MeasureText(theText) / 2.;
		y = theRect.Top() + (itsCurrentTextLine - 1) * theRect.Height() / NumberOfTextLines();
	}
    break;
  case kDown:
    y = theRect.Top() + theRelativeLocation * theRect.Height();//+fontSize.Y();
		                               //    + GetEnvironment()->GetFontHeight();//LW kaatuu
    if(itsRotation == kRight)
      x = theRect.Right() - GetToolBox()->MeasureText(theText);
    else
      x = theRect.Left();
    break;
  default:
    x = 0.;	//tassa pitaisi olla jotain jarkevaa
    y = 0.;
  
  }
    
  return NFmiPoint(x,y);
}
//_________________________________________________________________________ CalcTickLength

double NFmiAxisView::CalcTickLength(const NFmiRect &theRect)
{
  if(fItsAxis)
  {
    return itsOrientation == kRight || itsOrientation == kLeft ? itsTickLenght * theRect.Height() :
           itsOrientation == kUp || itsOrientation == kDown ? itsTickLenght * theRect.Width() : 1;
  }
  else
    return 0;
}
//_________________________________________________________________________ DrawAxisBase

void NFmiAxisView::DrawAxisBase(void)
{
  NFmiRect theRect(CalcAxisArea());
  NFmiPoint startPoint;
  NFmiPoint endPoint;

  switch(itsOrientation)
  {
  case kRight:
    if(itsRotation == kRight)
    {
      startPoint = theRect.TopLeft();
      endPoint   = theRect.TopRight();
    }
    else
    {
      startPoint = theRect.BottomLeft();
      endPoint   = theRect.BottomRight();
    }
    break;
  case kDown:
    if(itsRotation == kRight)
    {
      startPoint = theRect.TopRight();
      endPoint   = theRect.BottomRight();
    }
    else
    {
      startPoint = theRect.TopLeft();
      endPoint   = theRect.BottomLeft();
    }
    break;
  default:
    break;
    }
 
 
  NFmiLine tmp(startPoint,endPoint);
  GetToolBox()->Convert(&tmp);
}
//_________________________________________________________________________ DrawTicks

void NFmiAxisView::DrawTicks(void)
{
//  NFmiPoint fontSize(GetToolBox()->ToViewPoint(theEnvironment.GetFontWidth(),theEnvironment.GetFontHeight()));

  NFmiRect theRect(GetFrame()); //

  itsAxis->Reset();
  while(itsAxis->Next())
  {
    NFmiLine tmp(TickStart(theRect, RelativeTickPlace()),
		 TickEnd(theRect, RelativeTickPlace()));
    GetToolBox()->ConvertShape(&tmp);
  } 
}
//_________________________________________________________________________ DrawTickTexts

void NFmiAxisView::DrawTickTexts(double theFontHeight)
{
  if(!fItsPrimaryText)
    return;

  NFmiRect theRect(CalcTickTextArea());

/* ei toimi korjauksista huolimatta, poistettu toistaiseksi 240297/LW
  if(itsOrientation == kLeft || itsOrientation == kRight)
  {
    int n = itsAxis->MaxTextLength();
    if(n > 0)
    {
      double kissa = theRect.Width() / GetToolBox()->SX(GetToolBox()->MeasureText(NFmiValueString(short(8*pow(10., (double)(n)))))); //240297 GetToolBox()->SX( ; n-1
      itsAxis->StepNumber((short)kissa);
    }
  }
*/
  GetToolBox()->SetTextAlignment(kTopCenter);			   // alas
  if (itsOrientation == kRight && itsRotation == kLeft) // ylös, tämä oli kaikilla
     GetToolBox()->SetTextAlignment(kBottomCenter);
  if (itsOrientation == kDown && itsRotation == kRight)  // vasen
     GetToolBox()->SetTextAlignment(kBaseRight);
  if (itsOrientation == kDown && itsRotation == kLeft)   // oikea
     GetToolBox()->SetTextAlignment(kBaseLeft);
  itsAxis->Reset();
//  NFmiPoint point = NFmiPoint((theRect.Left() + theRect.Right()) / 2.f, theRect.Bottom()); // ota suoraan missä se nyt sitten on
  NFmiString secondText;
  while(itsAxis->Next())
  {
    itsCurrentTextLine = 2;
	NFmiPoint startPoint;
	startPoint = TickEnd(theRect, RelativeTickPlace());
	if(itsOrientation == kDown) startPoint += NFmiPoint(0.f, .3f * theFontHeight);// 170297 BaseLine ei oikein hyvä
	NFmiText tmp(startPoint,itsAxis->Text(), false);
    GetToolBox()->ConvertShape(&tmp);
//    GetToolBox()->ConvertShape(&NFmiText(point, itsAxis->Text()));
    if(!fItsSecondaryText)
		continue;
	secondText = itsAxis->SecondaryText(); 
	if(secondText.GetLen() > 0)  //120397
	{
//	   startPoint = TickTextStart(theRect, RelativeTickPlace() 	 //120397 ei osu x-suunnassa tarkkaan
//		                                , secondText);
	   if(itsOrientation == kRight || itsOrientation == kLeft)	 //120397
		     startPoint +=  NFmiPoint(0.f, .45f * GetFrame().Height());
       if(itsOrientation == kDown) startPoint +=  NFmiPoint(0.f, .31f * theFontHeight);// 170297 BaseLine ei oikein hyvä
       itsCurrentTextLine = 1;
       NFmiText tmp(startPoint, secondText, false);
       GetToolBox()->ConvertShape(&tmp);
	}

  } 
}
//_________________________________________________________________________ SetUpEnvironment
void NFmiAxisView::SetUpEnvironment(void)
{
  NFmiMetaFileView::SetUpEnvironment();
//  GetEnvironment()->SetFrameColor(itsFrameColor->GetRGB());
//  GetEnvironment()->SetFillColor(itsFillColor->GetRGB());
}
//_________________________________________________________________________ RelativeTickPlace

double NFmiAxisView::RelativeTickPlace(void)
{
  return itsOrientation == kDown || itsOrientation == kLeft ? 1. - itsAxis->Location() 
                                                             : itsAxis->Location();
}
//_________________________________________________________________________ NumberOfTextLines
double NFmiAxisView::NumberOfTextLines(void)
{
  if(fItsPrimaryText && fItsSecondaryText)
    return 2;
  else if (fItsPrimaryText || fItsSecondaryText)
    return 1;
  else
    return 0;
}
//_________________________________________________________________________ RelativeTickPlace
double NFmiAxisView::Value(const NFmiPoint& theClickPoint)
{
	double relLocation = 0.0;
	NFmiRect theRect(GetFrame());
	if(itsOrientation == kUp)
		relLocation = (theClickPoint.Y() - theRect.Top()) / theRect.Height();
	else if(itsOrientation == kRight)
		relLocation = (theClickPoint.X() - theRect.Left()) / theRect.Width();
	else if(itsOrientation == kDown)
		relLocation = (theRect.Bottom() - theClickPoint.Y()) / theRect.Height();
	else if(itsOrientation == kLeft)
		relLocation = (theRect.Right() - theClickPoint.X())  / theRect.Width();

	return static_cast<double>(itsAxis->Scale()->Scale()->Location(static_cast<float>(relLocation)));
}
