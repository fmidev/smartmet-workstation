//© Ilmatieteenlaitos/Persa.
//30.3.1995   Persa
//muutettu 6.3.1996/LW: tekstille FrameColor-v‰ri
//muutettu 1.4.1996/LW: TopLeft-> BottomLeft
//muutettu 2.4.1996/LW: y-mheight -> y teksteiss‰
// 25.9.96/LW Uusi shape=BitMap lis‰tty
// 291096/LW
// 061196/LW itsFillColor korjattu itsFramColoriksi BuildPolyline():ssa
// 071196/LW BuildLine():iin v‰ri ja paksuus
// 201196/LW StretchBuild():iin moodi
// 251196/LW fGifOutput-valmius
// 020197/LW GIF-valmiudet poistettu toolboxilta
// 240197/LW BuildBitMap() 256:n v‰rin Windowsille 65xxx:n v‰rin lis‰ksi
// 290197/LW uusi konstruktori: NFmiToolBox(const CRect&)


// Version 03.05. 1997/Persa FmiCrtlKeys added
//					         ConvertCrtlKeys method added
// Version 15.07. 1997/Marko Changed BuildBitmap() to accept also 24-bit colors
//
//
// 9.12.97/EL revised BuildBitmap() a bit to re-create a new bitmap
// only when color pool itself has changed. This is done with new Bitmap*(*) methods.
// 13.3.98/Marko Changed behaviour with bitmaps so that does't need to set any bitmat arrays.
// 12.6.1998/Marko	Added functionality to draw NFmiPixMap.
// 29.9.1998/Marko&Persa // Changed BuildText() to use DEFAULT_PITCH instead of FIXED_PITCH
//		in CFont creation parameter, so that Fmi defined font-set could be used.
// 06.10.1998/Marko Added BuildInvertRectangle() that draws rectangle with inverted colors.
// 15.06.1999/Marko Bold fonts can be used now.
// 04.11.1999/Marko Added another DrawDC()-method and DrawEllipse.

// tehd‰‰n dummy unix-toolbox headeriin
#ifdef _MSC_VER


#include "NFmiToolBox.h"
#include "NFmiMetaFileView.h"
#include "NFmiShapeList.h"
#include "NFmiColorPool.h"
#include "NFmiValueLineList.h"
#include "NFmiText.h"
#include "NFmiRectangle.h"
#include "NFmiBitmap.h"
#include "NFmiLine.h"
#include "NFmiPolyLine.h"
#include "NFmiValueLine.h"
#include "UnicodeStringConversions.h"
#include "NFmiSymbolBulkDrawData.h"
#include "NFmiWindBarb.h"
#include "NFmiSimpleWeatherSymbol.h"
#include "catlog/LoggingProfilerHelper.h"
#include <set>
#include <map>

/* RR aikaisemman prototyypin kanssa
extern "C"	ei tarvittu en‰‰
{
  int gdImageColorAllocate(gdImagePtr, int, int, int);
}
*/

bool gUseTransparencyWithText = true; // TƒMƒ ON MARKON PIKAVIRITYS!!!!!!

TurnClippingOffHelper::TurnClippingOffHelper(NFmiToolBox* toolbox)
	:toolbox_(toolbox)
	, oldClippingMode_(toolbox->UseClipping())
{
	toolbox_->UseClipping(false);
}

TurnClippingOffHelper::~TurnClippingOffHelper()
{
	toolbox_->UseClipping(oldClippingMode_);
}


//--------------------------------------------------------TFmiToolBox
  NFmiToolBox::NFmiToolBox (CView *theEnclosure,
					        bool isScrollView)//, bool GifOutput)
              : itsEnclosure(theEnclosure)
              , itsBaseEnvironment(new NFmiDrawingEnvironment),
//			  fGifOutput(GifOutput),
			  pItsPen(new CPen),
			  pItsFillPattern(new CBrush),
			  pItsPrintInfo(0),
			  itsXDirection(1),
			  itsYDirection(1),
			  fIsScrollView(isScrollView),//Persa 060397
			  itsRelativeClipRect(),
			  fUseClipping(false),
			  itsHatchPattern(-1)
{
 //**//251196/LW
 /* fGifOutput = GifOutput;
  if (fGifOutput)
  {
     itsGdImage = gdImageCreate(600, 800);
     itsGdWhite = gdImageColorAllocate(itsGdImage, 255, 255, 255);
     itsGdRed = gdImageColorAllocate(itsGdImage, 255, 0, 0);
     itsGdGreen = gdImageColorAllocate(itsGdImage, 0, 255, 0);
     itsGdBlue = gdImageColorAllocate(itsGdImage, 0, 0, 255);
  }
*/
//**//251196/LW
	itsPenSize.cx = itsXDirection * 1;
	itsPenSize.cy = itsYDirection * 1;
}
//-290197/LW--------------------------------------------------------TFmiToolBox
  NFmiToolBox::NFmiToolBox (const CRect& theClientRect)
              : itsEnclosure(0)
			  , mClientRect(theClientRect)
              , itsBaseEnvironment(new NFmiDrawingEnvironment),
			  pItsPen(new CPen),
//			  pItsFillPattern(0/*new CBrush*/),
			  pItsFillPattern(new CBrush),
			  pItsPrintInfo(0),
			  itsXDirection(1),
			  itsYDirection(1),
			  fIsScrollView(false),//Persa 060397
			  itsRelativeClipRect(),
			  fUseClipping(false),
			  itsHatchPattern(-1),
			  itsPenStyle(0) // 0=solid
{
  itsPenSize.cx = itsXDirection * 1;
  itsPenSize.cy = itsYDirection * 1;
/* // Marko: Mik‰ v***u t‰m‰ kissa oli??????  Homma meni nurin kun pItsFillPattern:ille yritettiin luoda uutta brushia myˆhemmin.
  CBrush *kissa = new CBrush;
  if(kissa->CreateSolidBrush(RGB(255,255,255)))
	  pItsFillPattern = kissa;
*/
  itsFillColor = RGB(0,0,0);
}
//--------------------------------------------------------TFmiToolBox
NFmiToolBox::~NFmiToolBox()
{
//** 261196/LW
 /*  if(fGifOutput)
   {
	  FILE *out;
      out = fopen("dg.gif","wb");
	  gdImageGif(itsGdImage,out);
	  fclose(out);
	  gdImageDestroy(itsGdImage);
   }
*/
//**
	itsMemBitmap.DeleteObject(); // resurssien siivous pit‰‰ tehd‰ erikseen!!!
	itsClipRegion.DeleteObject();

	if(pItsPen)
		pItsPen->DeleteObject();
	delete pItsPen;

	if(pItsFillPattern)
		pItsFillPattern->DeleteObject();
	delete pItsFillPattern;
	delete itsBaseEnvironment;
}

//--------------------------------------------------------SetXDirection
bool NFmiToolBox::SetXDirection(FmiDirection newDirection)
{
	  if(newDirection == kLeft)
	  {
		  itsXDirection = -1;
		  return true;
	  }
	  else if(newDirection == kRight)
	  {
		  itsXDirection = 1;
		  return true;
	  }
	  else
	  {
		  itsXDirection = 1;
		  return false;
	  }
}
//--------------------------------------------------------SetYDirection
bool NFmiToolBox::SetYDirection(FmiDirection newDirection)
{
	  if(newDirection == kUp)
	  {
		  itsYDirection = -1;
		  return true;
	  }
	  else if(newDirection == kDown)
	  {
		  itsYDirection = 1;
		  return true;
	  }
	  else
	  {
		  itsYDirection = 1;
		  return false;
	  }
}
//--------------------------------------------------------Draw
void NFmiToolBox::Draw(NFmiMetaFileView *theView)
{
  NFmiVoidPtrList shapeList(theView->GetDrawingItems());

  NFmiVoidPtrIterator shapeIterator(shapeList);
  void *aShape;
  while(shapeIterator.Next(aShape))
    ConvertShape((NFmiShape *)aShape);

}
//--------------------------------------------------------Convert
bool NFmiToolBox::Convert (const NFmiDrawingItem *fmiItem)
{
  bool status = false;
  if(fmiItem)
  {
	SetUpClipping();

//  	GetDravingRect(); // 23.2.2001/Marko T‰m‰ lasketaan kun asetetaan CD:t‰ toolboxille

    switch (fmiItem->GetIdentifier())
    {
      case kLineShape:
		  status = BuildLine(static_cast<const NFmiLine *>(fmiItem));
		break;

	  case kPolylineShape:
		  status = BuildPolyline(static_cast<const NFmiPolyline *>(fmiItem));
		break;

	  case kRectangleShape:
		  status = BuildRectangle(static_cast<const NFmiRectangle *>(fmiItem));
		break;

	  case kShapeList:
		BuildShapeList(static_cast<const NFmiShapeList *>(fmiItem)->GetShapes());
		status = false;
		break;

	  case kTextShape:
		  status = BuildText(static_cast<const NFmiText *>(fmiItem));
		break;

      case kDrawingItem:
		  status = BuildDrawingItem(fmiItem);
		break;

 	  case kBitmap:
			status = BuildBitmap(static_cast<const NFmiBitmap *>(fmiItem));
		break;

//	  case kPixmap: // 12.6.1998/Marko
//			status = BuildPixmap((NFmiPixmap *)fmiItem); //24.11.2000/Marko poisti t‰m‰n roskan
//		break;

	  default:
		  status = false;
		break;
	 }
	EndClipping();
  }
  return status;
}
//--------------------------------------------------------ConvertShape
bool NFmiToolBox::ConvertShape (const NFmiShape *fmiShape)
{

  if(fmiShape)
  {
//  	GetDravingRect();



    switch (fmiShape->GetIdentifier())
    {
      case kLineShape:
		  return BuildLine(static_cast<const NFmiLine *>(fmiShape));

		case kPolylineShape:
		  return BuildPolyline(static_cast<const NFmiPolyline *>(fmiShape));

		case kRectangleShape:
		  return BuildRectangle(static_cast<const NFmiRectangle *>(fmiShape));

 		case kShapeList:
		BuildShapeList((static_cast<const NFmiShapeList *>(fmiShape)->GetShapes()));
		return false;

		case kTextShape:
		  return BuildText(static_cast<const NFmiText *>(fmiShape));

		case kBitmap:
			return BuildBitmap(static_cast<const NFmiBitmap *>(fmiShape));

		default:
		  return false;
	 }
  }
  return false;
}
/* //24.11.2000/Marko poisti t‰m‰n roskan
//--------------------------------------------------------BuildPixmap
bool NFmiToolBox::BuildPixmap (NFmiPixmap *fmiShape)
{
	if (!itsEnclosure)
		return false;

	SelectEnvironment(fmiShape);
	BOOL kissabool = itsMemBitmap.CreateCompatibleBitmap(pDC, fmiShape->XNumber(), fmiShape->YNumber());
	ASSERT(itsMemBitmap.m_hObject != NULL);

	DWORD dword = itsMemBitmap.SetBitmapBits((DWORD)fmiShape->ColorPoolByteSize(), fmiShape->ColorPool()->Data());
    ASSERT(itsMemBitmap.m_hObject != NULL);

	CClientDC dc(itsEnclosure);
	itsdcMem.CreateCompatibleDC(&dc);

	CBitmap* oldCBitmap = itsdcMem.SelectObject(&itsMemBitmap);
	CRect theMFCRect;
//	ConvertRect(fmiShape->GetFrame(), theMFCRect);
	ConvertRect(fmiShape->TemporaryPlace(), theMFCRect);

	int test = pDC->SetStretchBltMode(COLORONCOLOR);

	pDC->StretchBlt(theMFCRect.left
				   ,theMFCRect.top
				   ,theMFCRect.Width()
				   ,theMFCRect.Height()
				   ,&itsdcMem
				   ,0
				   ,0
				   ,fmiShape->XNumber()
				   , fmiShape->YNumber()
				   , SRCCOPY);

	DeSelectEnvironment(fmiShape);
	itsdcMem.SelectObject(oldCBitmap);
	itsdcMem.DeleteDC();
	itsMemBitmap.DeleteObject();

	return true;
}
*/
// 5.08.1999/Marko
void NFmiToolBox::DrawBitmap(CBitmap* theBitmap, const NFmiRect& theDrawedBitmapPortion, const NFmiRect& theRelativeSize)
{
	itsEnclosure->GetClientRect(mClientRect);
	CClientDC dc(itsEnclosure);
	itsdcMem.CreateCompatibleDC(&dc);

	CBitmap* oldCBitmap = itsdcMem.SelectObject(theBitmap);
	CRect theMFCRect;
	ConvertRect(theRelativeSize, theMFCRect);

	int test = pDC->SetStretchBltMode(COLORONCOLOR);

	int destLeft = (int)theMFCRect.left;
	int destTop = (int)theMFCRect.top;
	int destWidth = (int)theMFCRect.Width();
	int destHeight = (int)theMFCRect.Height();
	int sourLeft = (int)theDrawedBitmapPortion.Left();
	int sourTop = (int)theDrawedBitmapPortion.Top();
	int sourWidth = (int)theDrawedBitmapPortion.Width();
	int sourHeight = (int)theDrawedBitmapPortion.Height();
	pDC->StretchBlt(destLeft
				   ,destTop
				   ,destWidth
				   ,destHeight
				   ,&itsdcMem
				   ,sourLeft
				   ,sourTop
				   ,sourWidth
				   ,sourHeight
				   ,SRCCOPY);
/*	pDC->StretchBlt(theMFCRect.left
				   ,theMFCRect.top
				   ,theMFCRect.Width()
				   ,theMFCRect.Height()
				   ,&itsdcMem
				   ,(int)theDrawedBitmapPortion.Left()
				   ,(int)theDrawedBitmapPortion.Top()
				   ,(int)theDrawedBitmapPortion.Width()
				   ,(int)theDrawedBitmapPortion.Height()
				   ,SRCCOPY);*/

	itsdcMem.SelectObject(oldCBitmap);
	itsdcMem.DeleteDC();
}

// 07.09.1998/Marko Not sure if this works anymore (could't test after cleaning this class).
// Use NFmiPixmap instead!!!!
//--------------------------------------------------------BuildBitMap
bool NFmiToolBox::BuildBitmap (const NFmiBitmap *fmiShape)
{
	if (!itsEnclosure) // 290197/LW ei en‰‰ voi toimia koska: CClientDC dc(itsEnclosure);
		return false;

	SelectEnvironment(fmiShape);

	int colorRes = pDC->GetDeviceCaps(COLORRES);

	int bitmapXRes = fmiShape->ColorGrid()->XNumber();
	int bitmapYRes = fmiShape->ColorGrid()->YNumber();

	itsMemBitmap.DeleteObject();
	BOOL kissabool = itsMemBitmap.CreateCompatibleBitmap(pDC, bitmapXRes, bitmapYRes);
	ASSERT(itsMemBitmap.m_hObject != NULL);

	short* shortPixMap;

	shortPixMap = const_cast<NFmiBitmap *>(fmiShape)->ShortPixMap(colorRes);

	DWORD dword = itsMemBitmap.SetBitmapBits(2*bitmapXRes*bitmapYRes, shortPixMap);
	ASSERT(itsMemBitmap.m_hObject != NULL);

	delete []shortPixMap;

	CClientDC dc(itsEnclosure);
	itsdcMem.CreateCompatibleDC(&dc);

	CBitmap* oldCBitmap = itsdcMem.SelectObject(&itsMemBitmap);

	CRect theMFCRect;
	ConvertRect(fmiShape->GetFrame(), theMFCRect);


	int test = pDC->SetStretchBltMode(COLORONCOLOR);//STRETCH_DELETESCANS); //201196/LW h‰vi‰‰kˆ mustat h‰iriˆt

	pDC->StretchBlt(theMFCRect.left // 111096 k‰‰ntyi, oli left  //291096/LW takaisin
				   ,theMFCRect.top
				   ,theMFCRect.Width() // 111096 k‰‰ntyi, oli +  //291096/LW takaisin
				   ,theMFCRect.Height()
				   ,&itsdcMem
				   ,0
				   ,0
				   ,bitmapXRes
				   ,bitmapYRes
				   , SRCCOPY);

	DeSelectEnvironment(fmiShape);
	itsdcMem.SelectObject(oldCBitmap);
	itsdcMem.DeleteDC();
	itsMemBitmap.DeleteObject();

	return true;
}

//--------------------------------------------------------BuildLine
bool NFmiToolBox::BuildLine (const NFmiLine *fmiShape)
{
	SelectEnvironment(fmiShape);

   CPoint theMFCPoint;
   CPen* pOldPen ;//= pDC->SelectObject(pItsPen); 	//071196/LW

//	if(fFramed)	//071196/LW +t‰m‰ lohko
//    {

	FmiPattern pattern = FMI_SOLID;
	if(fmiShape->GetEnvironment())
		pattern = fmiShape->GetEnvironment()->GetFillPattern();
	int usedMFCPattern = pattern;
	if(usedMFCPattern == FMI_RELATIVE_FILL)
		usedMFCPattern = FMI_SOLID;

	   if(!pItsPen->CreatePen(usedMFCPattern, itsPenSize.cx, itsFrameColor)) //onko frameColor oikein
	   {
			DeSelectEnvironment(fmiShape);
			return false;
	   }
//	}

 	pOldPen = pDC->SelectObject(pItsPen);  //071196/LW

	CPoint gdStartingPoint;
	switch (pattern)
	{
	case FMI_SOLID:
	case FMI_DASH: // HUOM! tulee yhten‰ist‰ viivaa jos viivan paksuus on yli 1
	case FMI_DOT: // HUOM! tulee yhten‰ist‰ viivaa jos viivan paksuus on yli 1
	case FMI_DASHDOT: // HUOM! tulee yhten‰ist‰ viivaa jos viivan paksuus on yli 1
	case FMI_DASHDOTDOT: // HUOM! tulee yhten‰ist‰ viivaa jos viivan paksuus on yli 1
		{
  		ConvertPoint(fmiShape->GetStartingPoint(), theMFCPoint);
  		pDC->MoveTo(theMFCPoint.x, theMFCPoint.y);
		gdStartingPoint = theMFCPoint; //261196/LW
  		ConvertPoint(fmiShape->GetEndingPoint(), theMFCPoint);
  		pDC->LineTo(theMFCPoint.x, theMFCPoint.y);
		break;
		}
	case FMI_RELATIVE_FILL:
		{
		double distance = fmiShape->GetStartingPoint().Distance(fmiShape->GetEndingPoint());
		double subLineCount = distance * fmiShape->GetEnvironment()->GetSubLinePerUnit();
		int subLineCountInt = boost::math::iround(subLineCount);
		double deltaX = (fmiShape->GetEndingPoint().X() - fmiShape->GetStartingPoint().X())/subLineCount;
		double deltaY = (fmiShape->GetEndingPoint().Y() - fmiShape->GetStartingPoint().Y())/subLineCount;
		double relativeFill = fmiShape->GetEnvironment()->GetRelativeFill();
		NFmiPoint currentPoint(fmiShape->GetStartingPoint());
  		ConvertPoint(currentPoint, theMFCPoint);
  		pDC->MoveTo(theMFCPoint.x, theMFCPoint.y);
		for(int i=0; i < subLineCountInt; i++)
		{
			currentPoint += NFmiPoint(deltaX*relativeFill, deltaY*relativeFill);
	  		ConvertPoint(currentPoint, theMFCPoint);
	  		pDC->LineTo(theMFCPoint.x, theMFCPoint.y);
			currentPoint += NFmiPoint(deltaX*(1.-relativeFill), deltaY*(1.-relativeFill));
	  		ConvertPoint(currentPoint, theMFCPoint);
	  		pDC->MoveTo(theMFCPoint.x, theMFCPoint.y);
		}
		double lastDist = fmiShape->GetEndingPoint().Distance(currentPoint);
		if(lastDist > 0.000001)
		{  // checking if the last 'drop' of line is worth to draw (prevents that drawing doesn't allway end with little dot)
//			currentPoint += NFmiPoint(deltaX*relativeFill, deltaY*relativeFill);
			currentPoint = fmiShape->GetEndingPoint();
  			ConvertPoint(currentPoint, theMFCPoint);
  			pDC->LineTo(theMFCPoint.x, theMFCPoint.y);
		}
		break;
		}
	}

 	pDC->SelectObject(pOldPen);

//** 261196/LW
/*	if(fGifOutput)
	   gdImageLine(itsGdImage,gdStartingPoint.x
		                     ,gdStartingPoint.y
						     ,theMFCPoint.x
						     ,theMFCPoint.y
						     ,itsGdGreen);
*/
//**

	pItsPen->DeleteObject(); // 071196/LW HUOM pit‰‰ deletoida

  DeSelectEnvironment(fmiShape);

  return true;
}
//--------------------------------------------------------BuildRectangle
bool NFmiToolBox::BuildRectangle (const NFmiRectangle *fmiShape)
{
	SelectEnvironment(fmiShape);
	if(!fInvert) // 1998.10.06/Marko
	{
		CRect theMFCRect;
		ConvertRect(fmiShape->GetFrame(), theMFCRect);
		CPen* pOldPen;
		if(fFilled)
		{
			if(!pItsFillPattern->CreateSolidBrush(itsFillColor))
			{
				DeSelectEnvironment(fmiShape);
				return false;
			}
			pDC->FillRect(&theMFCRect, pItsFillPattern);

			pItsFillPattern->DeleteObject();
		}
		if(fFramed)
		{
			pOldPen = pDC->SelectObject(pItsPen);
			for(int i = 0; i < itsPenSize.cx; i++)
			{
				pDC->FrameRect(&theMFCRect, &CBrush(itsFrameColor));
				theMFCRect.InflateRect(-1,-1,-1,-1);
			}

			pDC->SelectObject(pOldPen);
		}
	}
	else
		BuildInvertRectangle(fmiShape); // 1998.10.06/Marko

	DeSelectEnvironment(fmiShape);
	return true;
}

// Piti tehd‰ optimointia varten viritys, jossa voidaan piirt‰‰ sama polyline mahdollisesti siirrettyn‰ ja skaalattuna.
// Jos theOffSet on 0,0 ei tehd‰ mit‰‰n siirtoa ja jos theScale on 0,0, ei skaalausta tehd‰
bool NFmiToolBox::DrawPolyline(NFmiPolyline * polyline, const NFmiPoint &theOffSet, const NFmiPoint &theScale, double rotationAlfa)
{
	SetUpClipping();
	SelectEnvironment(polyline);

	std::vector<CPoint> pMFCPoints;

	CPoint mfcOffSet;
	ConvertPoint(theOffSet, mfcOffSet);
//	double xScale = itsXDirection * theScale.X() * mClientRect.Width();
	double xScale = theScale.X();
//	double yScale = itsYDirection * theScale.Y() * mClientRect.Height();
	double yScale = theScale.Y();
	ConvertPointList(polyline->GetPoints(), pMFCPoints, mfcOffSet, xScale, yScale, rotationAlfa);
	
	if(pMFCPoints.size() > 1)
    {
        if(fFilled)
        {
            bool status = false;
            if(itsHatchPattern == -1)
                status = pItsFillPattern->CreateSolidBrush(itsFillColor) != 0; // pDC->GetNearestColor
            else
                status = pItsFillPattern->CreateHatchBrush(itsHatchPattern, itsFillColor) != 0; // pDC->GetNearestColor
            if(!status)
            {
                DeSelectEnvironment(polyline);
                EndClipping();
                return false;
            }
            pDC->SetBkMode(polyline->BackGroundMode());
            CBrush* pOldBrush = pDC->SelectObject(pItsFillPattern);

            pDC->Polygon(pMFCPoints.data(), static_cast<int>(pMFCPoints.size()));

            pDC->SelectObject(pOldBrush);
            pItsFillPattern->DeleteObject();
        }
        if(fFramed)
        {
            if(!pItsPen->CreatePen(itsPenStyle, itsPenSize.cx, itsFrameColor)) //061196/LW oli itsFillColor
            {
                DeSelectEnvironment(polyline);
                EndClipping();
                return false;
            }

            CPen* pOldPen = pDC->SelectObject(pItsPen);

            pDC->Polyline(pMFCPoints.data(), static_cast<int>(pMFCPoints.size()));

            pDC->SelectObject(pOldPen);
            pItsPen->DeleteObject();
        }

        pDC->MoveTo(pMFCPoints[pMFCPoints.size() - 1].x, pMFCPoints[pMFCPoints.size() - 1].y);
    }

	DeSelectEnvironment(polyline);
	EndClipping();
	return true;
}

//--------------------------------------------------------BuildPolyline
bool NFmiToolBox::BuildPolyline (const NFmiPolyline *fmiShape)
{
	SelectEnvironment(fmiShape);

	std::vector<CPoint> pMFCPoints;

	ConvertPointList(fmiShape->GetPoints(), pMFCPoints);
	if(pMFCPoints.size() > 1)
	{
		if(fFilled)
		{
			CPen *oldPen = 0;
			int penStyle = PS_NULL;
			int penWidth = 0;
			COLORREF penColor1 = 0x00000000;
			CPen myPen(penStyle, penWidth, penColor1);
			oldPen = pDC->SelectObject(&myPen); // ei piirret‰ polygonille reunoja
			bool status = false;
			if(itsHatchPattern == -1)
				status = pItsFillPattern->CreateSolidBrush(itsFillColor) != 0; // pDC->GetNearestColor
			else
				status = pItsFillPattern->CreateHatchBrush(itsHatchPattern, itsFillColor) != 0; // pDC->GetNearestColor
			if(!status)
			{
				DeSelectEnvironment(fmiShape);
				return false;
			}
			pDC->SetBkMode(fmiShape->BackGroundMode());
			CBrush* pOldBrush = pDC->SelectObject(pItsFillPattern);

			pDC->Polygon(&pMFCPoints[0], static_cast<int>(pMFCPoints.size()));

			pDC->SelectObject(pOldBrush);
			pDC->SelectObject(oldPen);
			pItsFillPattern->DeleteObject();
		}

		if(fFramed)
		{
			if(!pItsPen->CreatePen(itsPenStyle, itsPenSize.cx, itsFrameColor)) //061196/LW oli itsFillColor
			{
				DeSelectEnvironment(fmiShape);
				return false;
			}

			CPen* pOldPen = pDC->SelectObject(pItsPen);

			pDC->Polyline(&pMFCPoints[0], static_cast<int>(pMFCPoints.size()));

			pDC->SelectObject(pOldPen);
			pItsPen->DeleteObject();
		}

		// MIKƒ hiivatti on t‰m‰ viimeinen moveto?!?!?!
		pDC->MoveTo(pMFCPoints[pMFCPoints.size()-1].x, pMFCPoints[pMFCPoints.size()-1].y);
	}

	DeSelectEnvironment(fmiShape);
	return true;
}

//--------------------------------------------------------BuildText
bool NFmiToolBox::BuildText (const NFmiText *fmiShape)
{
  SelectEnvironment(fmiShape);

 // LW
///  CFont *oldFont = (CFont *)(pDC->SelectStockObject(ANSI_VAR_FONT));
  CFont myFont;
  NFmiDrawingEnvironment *fmiEnvironment = fmiShape->GetEnvironment();
  double fontHeight;
  FmiFontType fontType;
  int fontWeight; // 15.6.1999/Marko
  if(fmiEnvironment)
  {
	  fontHeight = fmiEnvironment->GetFontHeight();
 	  fontType = fmiEnvironment->GetFontType();		//040397/LW
	  if(!fmiEnvironment->BoldFont()) // 15.6.1999/Marko
		  fontWeight = FW_NORMAL;
	  else
		  fontWeight = FW_BOLD;
  }
  else
  {
	  fontHeight = itsBaseEnvironment->GetFontHeight();
  	  fontType = itsBaseEnvironment->GetFontType();		//040397/LW
	  if(!itsBaseEnvironment->BoldFont()) // 15.6.1999/Marko
		  fontWeight = FW_NORMAL;
	  else
		  fontWeight = FW_BOLD;
  }

  myFont.CreateFont((int)fontHeight
				   ,0 /* width */
				   ,0 // escapement (angle in 0.1 degrees)
                   ,1 // orientation (angle in 0.1 degrees)
				   ,fontWeight // weight (FW_NORMAL, FW_BOLD jne.)
				   ,0 // italic
				   ,0 // underline
                   ,0 // strikeout
				   ,1 /* char set */
				   ,OUT_TT_PRECIS//040397 //291096/LW oli 1
                   ,1
				   ,PROOF_QUALITY
				   ,DEFAULT_PITCH /* FF_DONTCARE*/ /* 1 == FIXED_PITCH */ // 29.9.1998/Marko&Persa
				   ,ConvertFont(fontType)); //040397/LW oli NULL

//  myFont.CreatePointFont((int)fontHeight, ConvertFont(fontType), pDC);  //040397/LW
  CFont *oldFont = pDC->SelectObject(&myFont);

  CPoint theMFCPoint;				// BottomLeft() ei auta
  ConvertPoint(fmiShape->GetFrame().TopLeft(), theMFCPoint);
  TEXTMETRIC mfontdata;
  pDC->GetTextMetrics(&mfontdata);

  pDC->MoveTo(theMFCPoint.x, theMFCPoint.y);
  int mheight = mfontdata.tmAscent + mfontdata.tmExternalLeading ;
  int mprevbkmode = pDC->SetBkMode(TRANSPARENT);
  int oldBkColor = pDC->GetBkColor();

	// NƒMƒ ON MARKON PIKAVIRITYKSIƒ!!!!!!!!!!!!
  if(!gUseTransparencyWithText)
  {
	  pDC->SetBkMode(OPAQUE);
	  pDC->SetBkColor(itsFillColor);
  }
	// NƒMƒ ON MARKON PIKAVIRITYKSIƒ!!!!!!!!!!!!

   pDC->SetTextColor(itsFrameColor); //Should be itsForegroundColor

//   pDC->SetTextAlign(TA_RIGHT);
                                    // oli .y-mheight; toimiiko k‰‰nnettyn‰
  std::string drawedString = fmiShape->GetText();
  bool doMultiLineDrawing = drawedString.find("\r\n") != std::string::npos;
  if(doMultiLineDrawing)
  {
	  CRect drawrect(theMFCPoint.x - 1, theMFCPoint.y, theMFCPoint.x + 1, theMFCPoint.y - 1);
	  pDC->DrawText(::convertPossibleUtf8StringToWideString(drawedString).c_str(), drawrect, DT_WORDBREAK | DT_CALCRECT);
      pDC->DrawText(::convertPossibleUtf8StringToWideString(drawedString).c_str(), drawrect, DT_WORDBREAK | DT_NOCLIP);
  }
  else
      pDC->TextOut(theMFCPoint.x, theMFCPoint.y, ::convertPossibleUtf8StringToWideString(drawedString).c_str());


  pDC->SelectObject(oldFont);
  myFont.DeleteObject();

  pDC->SetBkColor(oldBkColor);
  pDC->SetBkMode(mprevbkmode);
  DeSelectEnvironment(fmiShape);

  return true;
}
//--------------------------------------------------------SelectEnvironment
void NFmiToolBox::SelectEnvironment (const NFmiDrawingItem *fromFmiDrawingItem)
{
  const NFmiDrawingEnvironment *fmiEnvironment = fromFmiDrawingItem->GetEnvironment();
  if(fmiEnvironment)
    ConvertEnvironment (fmiEnvironment);

//    itsUserEnvironment = fmiEnvironment;
//  else
//    itsUserEnvironment = itsBaseEnvironment;
}
//--------------------------------------------------------DeSelectEnvironment
void NFmiToolBox::DeSelectEnvironment (const NFmiDrawingItem *fromFmiDrawingItem)
{
  const NFmiDrawingEnvironment *fmiEnvironment = fromFmiDrawingItem->GetEnvironment();
  if(fmiEnvironment)
    ConvertEnvironment (itsBaseEnvironment);

//  itsUserEnvironment = itsBaseEnvironment;									 vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
}
//--------------------------------------------------------ConvertEnvironment
void NFmiToolBox::ConvertEnvironment (const NFmiDrawingEnvironment *fmiEnvironment)
{
//  NFmiDrawingEnvironment *fmiEnvironment = fromFmiShape->GetEnvironment();

	// pit‰‰kˆ t‰m‰ tehd‰ GetNearestColor:lla alempi resoluutioisella putkella
	// ks siin‰ tapauksessa simpleweatherview
//  if(pDC->GetDeviceCaps(BITSPIXEL) == 16 ||          //060297/LW
//	 pDC->GetDeviceCaps(TECHNOLOGY) == DT_PLOTTER)  //070297/LW
  {
	  itsFrameColor = ConvertColor(fmiEnvironment->GetFrameColor());
      itsFillColor = ConvertColor(fmiEnvironment->GetFillColor());
  }
 /* else
  {
     itsFrameColor = pDC->GetNearestColor(ConvertColor(fmiEnvironment->GetFrameColor()));//040297LW
     itsFillColor  = pDC->GetNearestColor(ConvertColor(fmiEnvironment->GetFillColor()));//040297LW
  }	*/
 //  COLORREF* testColor = pDC->GetNearestColor(itsFillColor); virhe
//  CPalette* testPalette = pDC->GetPalette();				   virhe
//  int testIndex = testPalette->GetNearestPaletteIndex(itsFillColor);

/////  if (pItsFillPattern) delete pItsFillPattern;
/////  CBrush *kissa = new CBrush;
/////  if(kissa->CreateSolidBrush(itsFillColor))
/////	  pItsFillPattern = kissa;
  fFramed = fmiEnvironment->IsFramed();
  fFilled = fmiEnvironment->IsFilled();
  fInvert = fmiEnvironment->IsInverted();
  itsPenSize.cx = itsXDirection * (long)fmiEnvironment->GetPenSize().X();
  itsPenSize.cy = itsYDirection * (long)fmiEnvironment->GetPenSize().Y();
  itsHatchPattern = fmiEnvironment->GetHatchPattern();
  itsPenStyle = fmiEnvironment->GetLineStyle();
///////  if(pItsPen)	delete pItsPen;
//   pItsPen = new CPen(PS_SOLID, itsPenSize.cx, itsFrameColor);
//////     pItsPen = new CPen;

   SetFont (fmiEnvironment);
 //  TextSize(fmiEnvironment->GetFontHeight());
 // TextMode(srcOr);
  //RGBForeColor(&itsFrameColor);
}
//--------------------------------------------------------
void NFmiToolBox::SetFont (const NFmiDrawingEnvironment *theEnvironment)
{
/****
   pDC->SelectStockObject(ANSI_VAR_FONT);
   CFont myFont;
   myFont.CreateFont(-theEnvironment->GetFontHeight(), 0, 0
                     ,1,0,0,0
                     ,0,1,1
                     ,1,1,1,NULL);
    pDC->SelectObject(&myFont);
*****/
}
//--------------------------------------------------------
bool NFmiToolBox::BuildDrawingItem (const NFmiDrawingItem *fromFmiDrawingItem)
{
  delete itsBaseEnvironment;
  itsBaseEnvironment = new NFmiDrawingEnvironment(*(fromFmiDrawingItem->GetEnvironment()));

  ConvertEnvironment (itsBaseEnvironment);

  return true;
}
//--------------------------------------------------------ConvertColor

/*
NFmiPoint NFmiToolBox::ViewFontSize (const NFmiDrawingEnvironment &fmiEnvironment)
{

  TEXTMETRIC currentFont;
  pDC->GetTextMetrics(&currentFont);
  TextSize(fmiEnvironment.GetFontHeight());
  GetFontInfo(&fontToTest);
  TextSize(currentFont.ascent + currentFont.descent);
  return ToViewPoint(fontToTest.widMax, fontToTest.ascent + fontToTest.descent);

}
*/
//--------------------------------------------------------ConvertColor
COLORREF NFmiToolBox::ConvertColor (const FmiRGBColor &fromFmiColor)
{
  return
  RGB(fromFmiColor.red*255,fromFmiColor.green*255,fromFmiColor.blue*255 );
}
//--------------------------------------------------------ConvertPointList
void NFmiToolBox::ConvertPointList(const std::vector<NFmiPoint>& fmiPoints, std::vector<CPoint> &theMFCPoints)
{
	theMFCPoints.clear();
	if(fmiPoints.empty())
		return ;

	theMFCPoints.resize(fmiPoints.size());
	for(size_t index = 0; index < fmiPoints.size(); index++)
	{
		ConvertPoint(fmiPoints[index], theMFCPoints[index]);
	}
}

// Jos skaala arvo on 0 tai 1, ei tehd‰ skaalausta
static bool UseScale(double scaleValue)
{
	return (scaleValue != 0 && scaleValue != 1);
}

// ConvertPoint/Rect/PointList ja vastaavien funktioiden double -> int konveriot 
// tehd‰‰n kaikki yhdell‰ ja samalla toiminnalla, joka m‰‰r‰t‰‰n 
// CONVERT_POINT_TO_INT_FUNCTION makrolla.
// 
// Originaali konversio oli cast:aus int:iksi, eli leikkuri.
#define CONVERT_POINT_TO_INT_FUNCTION int

// Uusi versio k‰ytt‰‰ boost:in interger round funktiota, joten loppu tuloksen
// pit‰isi olla parempi, mutta tarvittaessa voidaan palata vanhaan.
//#define CONVERT_POINT_TO_INT_FUNCTION boost::math::iround

//---------------------------------------------------------------------ConvertPoint
void NFmiToolBox::ConvertPoint(const NFmiPoint& relativePoint, CPoint& absolutePoint)
{
	absolutePoint.y = itsYDirection * CONVERT_POINT_TO_INT_FUNCTION(relativePoint.Y() * mClientRect.Height());
	absolutePoint.x = itsXDirection * CONVERT_POINT_TO_INT_FUNCTION(relativePoint.X() * mClientRect.Width());
}
//---------------------------------------------------------------------ConvertPoint
void NFmiToolBox::ConvertPoint(const NFmiRect& theFrame, const NFmiPoint& relativePoint, CPoint& absolutePoint)
{
	absolutePoint.y = itsYDirection * CONVERT_POINT_TO_INT_FUNCTION(relativePoint.Y() * theFrame.Height());
	absolutePoint.x = itsXDirection * CONVERT_POINT_TO_INT_FUNCTION(relativePoint.X() * theFrame.Width());
}
//---------------------------------------------------------------------ConvertRect
void NFmiToolBox::ConvertRect(const NFmiRect& relativeRect, CRect& absoluteRect)
{
	absoluteRect.top = itsYDirection * CONVERT_POINT_TO_INT_FUNCTION(relativeRect.Top() * mClientRect.Height());
	absoluteRect.left = itsXDirection * CONVERT_POINT_TO_INT_FUNCTION(relativeRect.Left() * mClientRect.Width());
	absoluteRect.bottom = itsYDirection * CONVERT_POINT_TO_INT_FUNCTION(relativeRect.Bottom() * mClientRect.Height());
	absoluteRect.right = itsXDirection * CONVERT_POINT_TO_INT_FUNCTION(relativeRect.Right() * mClientRect.Width());
}

//---------------------------------------------------------------------ToViewPoint
// muutos pikseli-maailmasta toolboxin suhteelliseen maailmaan
NFmiPoint NFmiToolBox::ToViewPoint(long xPix, long yPix)
{
	return NFmiPoint(static_cast<float>(xPix) / mClientRect.Width(), static_cast<float>(yPix) / mClientRect.Height());
}

// muutos toolboxin suhteellisesta maailmasta n‰ytˆn pikseli-maailmaan
NFmiPoint NFmiToolBox::ToScreenPoint(double x, double y)
{
	return NFmiPoint(CONVERT_POINT_TO_INT_FUNCTION(x * mClientRect.Width()), CONVERT_POINT_TO_INT_FUNCTION(y * mClientRect.Height()));
}

void NFmiToolBox::ConvertPointList(const std::vector<NFmiPoint>& fmiPoints, std::vector<CPoint>& theMFCPoints,
	const CPoint& MFCOffSet, double xScale, double yScale, double rotationAlfa)
{
	theMFCPoints.clear();
	if(fmiPoints.empty())
		return;

	theMFCPoints.resize(fmiPoints.size());

	bool useOffSet = MFCOffSet != CPoint(0, 0);
	bool useScale = ::UseScale(xScale) || ::UseScale(yScale);

	if(useOffSet && useScale)
	{
		if(rotationAlfa == 0)
		{
			// jos ei rotaatiota
			for(size_t index = 0; index < fmiPoints.size(); index++)
			{
				ConvertPoint(fmiPoints[index], theMFCPoints[index]);
				theMFCPoints[index].x = CONVERT_POINT_TO_INT_FUNCTION(theMFCPoints[index].x * xScale); // en ole testannut t‰t‰ skaalausta ollenkaan, en tied‰ mit‰ se oikeasti tekee (Marko)
				theMFCPoints[index].y = CONVERT_POINT_TO_INT_FUNCTION(theMFCPoints[index].y * yScale); // en ole testannut t‰t‰ skaalausta ollenkaan, en tied‰ mit‰ se oikeasti tekee (Marko)
				theMFCPoints[index].Offset(MFCOffSet);
			}
		}
		else
		{ 
			// muuten pyˆr‰ytet‰‰n kappaletta samalla
			for(size_t index = 0; index < fmiPoints.size(); index++)
			{
				ConvertPoint(::RotatePoint(fmiPoints[index], rotationAlfa), theMFCPoints[index]); // rotaatio pit‰‰ tehd‰ ennen muita muunnoksia, eli kappaletta pyˆritet‰‰n itsens‰ suhteen ymp‰ri
				theMFCPoints[index].x = CONVERT_POINT_TO_INT_FUNCTION(theMFCPoints[index].x * xScale); // en ole testannut t‰t‰ skaalausta ollenkaan, en tied‰ mit‰ se oikeasti tekee (Marko)
				theMFCPoints[index].y = CONVERT_POINT_TO_INT_FUNCTION(theMFCPoints[index].y * yScale); // en ole testannut t‰t‰ skaalausta ollenkaan, en tied‰ mit‰ se oikeasti tekee (Marko)
				theMFCPoints[index].Offset(MFCOffSet);
			}
		}
	}
	else if(useOffSet)
	{
		for(size_t index = 0; index < fmiPoints.size(); index++)
		{
			ConvertPoint(fmiPoints[index], theMFCPoints[index]);
			theMFCPoints[index].Offset(MFCOffSet);
		}
	}
	else if(useScale)
	{
		for(size_t index = 0; index < fmiPoints.size(); index++)
		{
			ConvertPoint(fmiPoints[index], theMFCPoints[index]);
			theMFCPoints[index].x = CONVERT_POINT_TO_INT_FUNCTION(theMFCPoints[index].x * xScale); // en ole testannut t‰t‰ skaalausta ollenkaan, en tied‰ mit‰ se oikeasti tekee (Marko)
			theMFCPoints[index].y = CONVERT_POINT_TO_INT_FUNCTION(theMFCPoints[index].y * yScale); // en ole testannut t‰t‰ skaalausta ollenkaan, en tied‰ mit‰ se oikeasti tekee (Marko)
		}
	}
	else
	{
		for(size_t index = 0; index < fmiPoints.size(); index++)
		{
			ConvertPoint(fmiPoints[index], theMFCPoints[index]);
		}
	}
}

//--------------------------------------------------------ConvertShapeList
void NFmiToolBox::BuildShapeList (const NFmiVoidPtrList *fmiShapeList)
{
  if(!fmiShapeList)
    return;

  NFmiVoidPtrIterator fmiShapes(const_cast<NFmiVoidPtrList *>(fmiShapeList));
  void *VPtr;

  while(fmiShapes.Next(VPtr))
  {
      Convert((NFmiDrawingItem *)VPtr);
  }
}
////////////////////////////////////////////////////////////////////////////
/*
TFmiRect NFmiToolBox::ToFmiRect(const Rect &MacRect )
{
  return TFmiRect(MacRect.Top(), MacRect.Left(), MacRect.Bottom(), MacRect.Right());
}
*/
//_________________________________________________________________________ BuildFrame
NFmiRect NFmiToolBox::BuildFrame(const NFmiView *viewToFrame
                                ,const NFmiColor &theFrameColor
                                ,const NFmiColor &theFillColor
                                ,NFmiVoidPtrList &theDrawingList)
{
//  NFmiRect theFrame = GetFrame();	//GetDrawingArea();
  NFmiRect theFrame = viewToFrame->GetFrame();

  if(false)
  {
  NFmiDrawingEnvironment theEnvironment;
  theEnvironment.DisableFrame();
  theEnvironment.SetFillColor(theFrameColor);
  theEnvironment.EnableFill();
  ConvertShape(&NFmiRectangle(NFmiPoint(theFrame.Left(), theFrame.Top())
                                          ,NFmiPoint(theFrame.Right(), theFrame.Bottom())
                                          ,0 //viewToFrame
                                          ,&theEnvironment));
/***
  NFmiRectangle *aRect = new NFmiRectangle(NFmiPoint(theFrame.Left(), theFrame.Top())
                                          ,NFmiPoint(theFrame.Right(), theFrame.Bottom())
                                          ,0 //viewToFrame
                                          ,&theEnvironment);
  theDrawingList.Add((void *)aRect);
***/
  NFmiColor black(0.f,0.f,0.f);
  NFmiColor white(1.f,1.f,1.f);
  NFmiColor *theColor = new NFmiColor(theFrameColor);
  theColor->Mix(black, 0.15f);
  theEnvironment.SetFillColor(*theColor);

  NFmiPoint penSize(viewToFrame->GetRelativeBorderWidth());
  theFrame.Inflate(-penSize.X(), -penSize.Y());

  NFmiPolyline *aPoly = new NFmiPolyline(NFmiPoint(theFrame.Left(), theFrame.Top())
                                        ,NFmiPoint(theFrame.Right(), theFrame.Bottom())
                                        ,0 //viewToFrame
                                        ,&theEnvironment);

  double cornerYSize = (theFrame.Bottom() - theFrame.Top()) / 2.;
  double cornerXSize = cornerYSize * 1. / WidthPerHeight();
  aPoly->AddPoint(NFmiPoint(theFrame.Left(), theFrame.Top()));
  aPoly->AddPoint(NFmiPoint(theFrame.Right(), theFrame.Top()));
  aPoly->AddPoint(NFmiPoint(theFrame.Right() - cornerXSize, theFrame.Top() + cornerYSize));
  aPoly->AddPoint(NFmiPoint(theFrame.Left() + cornerXSize,  theFrame.Top() + cornerYSize));
  aPoly->AddPoint(NFmiPoint(theFrame.Left(), theFrame.Bottom()));
  aPoly->AddPoint(NFmiPoint(theFrame.Left(), theFrame.Top()));

  ConvertShape(aPoly);
  delete aPoly;
/***
  theDrawingList.Add((void *)aPoly);
***/

  delete theColor;
//  theColor = new NFmiColor(theFrameColor);

  NFmiColor anOtherColor(theFrameColor);
  anOtherColor.Mix(white, 0.6f);	//white
  theEnvironment.SetFillColor(anOtherColor);

  aPoly = new NFmiPolyline(NFmiPoint(theFrame.Left(), theFrame.Top())
                          ,NFmiPoint(theFrame.Right(), theFrame.Bottom())
                          ,0 //viewToFrame
                          ,&theEnvironment);
  aPoly->AddPoint(NFmiPoint(theFrame.Right(), theFrame.Bottom()));
  aPoly->AddPoint(NFmiPoint(theFrame.Left(), theFrame.Bottom()));
  aPoly->AddPoint(NFmiPoint(theFrame.Left() + cornerXSize,  theFrame.Top() + cornerYSize));
  aPoly->AddPoint(NFmiPoint(theFrame.Right() - cornerXSize,  theFrame.Top() + cornerYSize));
  aPoly->AddPoint(NFmiPoint(theFrame.Right(), theFrame.Top()));
  aPoly->AddPoint(NFmiPoint(theFrame.Right(), theFrame.Bottom()));

  ConvertShape(aPoly);
  delete aPoly;
/***
  theDrawingList.Add((void *)aPoly);
***/

  theFrame.Inflate(-penSize.X()*2., -penSize.Y()*2.);

  theEnvironment.SetFillColor(theFillColor);
  } //Katso alusta if(false)

  NFmiDrawingEnvironment theEnvironment;
  theEnvironment.SetFrameColor(theFrameColor);
  theEnvironment.EnableFrame();
  ConvertShape(&NFmiRectangle(NFmiPoint(theFrame.Left(), theFrame.Top())
                           ,NFmiPoint(theFrame.Right(), theFrame.Bottom())
                           ,0 //viewToFrame
                           ,&theEnvironment));
/***
  aRect = new NFmiRectangle(NFmiPoint(theFrame.Left(), theFrame.Top())
                           ,NFmiPoint(theFrame.Right(), theFrame.Bottom())
                           ,0 //viewToFrame
                           ,&theEnvironment);
  theDrawingList.Add((void *)aRect);
***/
  return theFrame;
}
//----------------------------------------------
CRect NFmiToolBox::GetDravingRect()
{
	if (itsEnclosure)// 290197/LW
	{
		if(fIsScrollView)//Persa 060397
			mClientRect = CRect(CPoint(0,0),((CScrollView*)itsEnclosure)->GetTotalSize());
		else
			itsEnclosure->GetClientRect(mClientRect);
	}
	return 	mClientRect;
/*
	if(pDC->IsPrinting())
	{
		CRect apu = pItsPrintInfo->m_rectDraw;
		float mWidthDiv =((float)apu.Width())/mClientRect.Width();
		float mHeigthDiv =((float) apu.Height())/mClientRect.Height();
		double maddHeight = 360./72. * mClientRect.Height();
		double maddWidth = 360./72. * mClientRect.Width();
	//	if(mWidthDiv < mHeigthDiv)
	//		{
	////		maddHeight= mHeigthDiv*mClientRect.Height();
	////		maddWidth = mWidthDiv*mClientRect.Width();
  	//		}
	//	else
	//		{
	//	   	maddHeight= mHeigthDiv*mClientRect.Height();
	//	   	maddWidth = mHeigthDiv*mClientRect.Width();
	//		}
		mClientRect= CRect(0,0,(int)maddWidth, (int)maddHeight);
	}

//	return 	CRect(0,0,18000,21000);;
	return 	mClientRect;
	*/
}
//------------------MeasureText(const NFmiString &theText)
double NFmiToolBox::MeasureText(const NFmiString &theText)
{
  TEXTMETRIC mfontdata;
  pDC->GetTextMetrics(&mfontdata);

 return (float)theText.GetLen()*mfontdata.tmAveCharWidth/mClientRect.Width();

}
 //---------------SetTextAlignment(FmiDirection theAlignment)
void NFmiToolBox::SetTextAlignment(FmiDirection theAlignment)
{
	// NƒMƒ ON MARKON PIKAVIRITYKSIƒ!!!!!!!!!!!!
	// Tein t‰h‰n funktioon virityksen, jolla voidaan s‰‰t‰‰ onko piirrett‰v‰ll‰
	// tekstill‰ l‰pin‰kyv‰ tausta vai ei (jos ei, tulee backround colorista tekstille tausta).
	if(theAlignment / 1000)
		gUseTransparencyWithText = false;
	else
		gUseTransparencyWithText = true;

	theAlignment = FmiDirection(int(theAlignment)%1000);
	// NƒMƒ ON MARKON PIKAVIRITYKSIƒ!!!!!!!!!!!!

	switch(theAlignment)
	{
	case kLeft:
		pDC->SetTextAlign(TA_LEFT);
		break;
	case kTop:
		pDC->SetTextAlign(TA_TOP);
		break;
	case kBottom:
		pDC->SetTextAlign(TA_BOTTOM);
		break;
	case kCenter:
		pDC->SetTextAlign(TA_CENTER);
		break;
	case kRight:
		pDC->SetTextAlign(TA_RIGHT);
		break;
	case kBase:
		pDC->SetTextAlign(TA_BASELINE);
		break;
	case kBottomRight:
		pDC->SetTextAlign(TA_BOTTOM | TA_RIGHT);
		break;
	case kBottomLeft:
		pDC->SetTextAlign(TA_LEFT | TA_BOTTOM);
		break;
	case kTopRight:
		pDC->SetTextAlign(TA_RIGHT | TA_TOP);
		break;
	case kTopLeft:
		pDC->SetTextAlign(TA_LEFT | TA_TOP);
		break;
	case kTopCenter:
		pDC->SetTextAlign(TA_TOP | TA_CENTER);
		break;
	case kBottomCenter:
		pDC->SetTextAlign(TA_BOTTOM | TA_CENTER);
		break;
	case kBaseLeft:
		pDC->SetTextAlign(TA_BASELINE | TA_LEFT);
		break;
	case kBaseCenter:
		pDC->SetTextAlign(TA_BASELINE | TA_CENTER);
		break;
	case kBaseRight:
		pDC->SetTextAlign(TA_BASELINE | TA_RIGHT);
		break;
	case kMiddleRight:
		pDC->SetTextAlign(TA_CENTER | TA_RIGHT);
		break;
	default:
		break;
	}
}

FmiDirection NFmiToolBox::GetTextAlignment(void)
{
	int align = pDC->GetTextAlign();

	if(align == TA_LEFT)
		return kLeft;
	else if(align == TA_TOP)
		return kTop;
	else if(align == TA_BOTTOM)
		return kBottom;
	else if(align == TA_CENTER)
		return kCenter;
	else if(align == TA_RIGHT)
		return kRight;
	else if(align == TA_BASELINE)
		return kBase;
	else if(align & TA_BOTTOM && align & TA_RIGHT)
		return kBottomRight;
	else if(align & TA_BOTTOM && align & TA_LEFT)
		return kBottomLeft;
	else if(align & TA_TOP && align & TA_LEFT)
		return kTopLeft;
	else if(align & TA_TOP && align & TA_RIGHT)
		return kTopRight;
	else if(align & TA_TOP && align & TA_CENTER)
		return kTopCenter;
	else if(align & TA_BOTTOM && align & TA_CENTER)
		return kBottomCenter;
	else if(align & TA_BASELINE && align & TA_LEFT)
		return kBaseLeft;
	else if(align & TA_BASELINE && align & TA_RIGHT)
		return kBaseRight;
	else if(align & TA_BASELINE && align & TA_CENTER)
		return kBaseCenter;
	else
		return kCenter;
}

LPCTSTR NFmiToolBox::ConvertFont(FmiFontType theFont)
{
	switch(theFont)
	{
	case kArcl:
		return _TEXT("Arcl");
		break;
	case kArial:
        return _TEXT("Arial");
		break;
	case kBodub:
        return _TEXT("Bodoni MT Ultra Bold");
		break ;
	case kCourier:
        return _TEXT("Courier");
		break ;
	case kCourierNew:
        return _TEXT("Courier New");
		break ;
	case kGillcdb:
        return _TEXT("Gill Sans Condensed");
		break;
	case kKlang:
        return _TEXT("Klang MT");
		break;
	case kLucidaConsole:
        return _TEXT("Lucida Console");
		break;
	case kModern:				//200397 +
        return _TEXT("Modern");
		break;
	case kSansSerif:
        return _TEXT("MS Sans Serif");
		break;
	case kSerif:
        return _TEXT("MS Serif");
		break;
	case kNewber:
        return _TEXT("New Berolina MT");
		break;
	case kOld:
        return _TEXT("Old English Text MT");
		break;
	case kRoman:
        return _TEXT("Roman");
		break;
	case kScrib:
        return _TEXT("Scrib");
		break;
	case kScript:
        return _TEXT("Script");
		break;
	case kSymbols:
        return _TEXT("Symbol");
		break;
	case kTimesNewRoman:
        return _TEXT("Times New Roman");
		break;
	case kWingDings:
        return _TEXT("WingDings");
		break;
	case kSynop:
        return _TEXT("Synop");
		break;
    case kMirri:
        return _TEXT("Mirri");
        break;
    default:
        return _TEXT("Courier New");
		break;
	}
}
unsigned long NFmiToolBox::ConvertCtrlKey (UINT theHardCtrl) const
{
	unsigned long fmiCrtls = 0L;
	if(theHardCtrl & MK_CONTROL)
		fmiCrtls = fmiCrtls | kCtrlKey;
	if(theHardCtrl & MK_SHIFT)
		fmiCrtls = fmiCrtls | kShiftKey;
	return fmiCrtls;
}

//--------------------------------------------------------BuildRectangle
bool NFmiToolBox::BuildInvertRectangle(const NFmiRectangle *fmiShape)
{
	CRect theMFCRect;
	ConvertRect(fmiShape->GetFrame(), theMFCRect);
	if(fFilled)
	{
		pDC->InvertRect(&theMFCRect);
	}
	else // only framed if not filled!?!?
	{// draws four inverted rects as four lines of the big rect (much faster
	 // in case of big rectangle
		CRect leftLine(theMFCRect.left, theMFCRect.top,theMFCRect.left+itsPenSize.cx,theMFCRect.bottom);
		pDC->InvertRect(&leftLine);
		CRect topLine(theMFCRect.left, theMFCRect.top,theMFCRect.right,theMFCRect.top+itsPenSize.cy);
		pDC->InvertRect(&topLine);
		if(theMFCRect.Width() != itsPenSize.cx) // if equal and drawn-> rectangle dissappears
		{
			CRect rightLine(theMFCRect.right-itsPenSize.cx, theMFCRect.top,theMFCRect.right,theMFCRect.bottom);
			pDC->InvertRect(&rightLine);
		}
		if(theMFCRect.Height() != itsPenSize.cy) // if equal and drawn-> rectangle dissappears
		{
			CRect bottomLine(theMFCRect.left, theMFCRect.bottom-itsPenSize.cy,theMFCRect.right,theMFCRect.bottom);
			pDC->InvertRect(&bottomLine);
		}
	}
	return true;
}

// V‰liaikainen(?) funktio, joka piir‰‰ annetun DC:n sis‰llˆn toolboxiin
void NFmiToolBox::DrawDC(CDC* theDC, const NFmiRect& theRelativeSize)
{
	UpdateClientRect(); // poista t‰m‰ kun voit (tarvitaanko?!?!?)
	CRect theMFCRect;
	ConvertRect(theRelativeSize, theMFCRect);
	pDC->BitBlt(theMFCRect.left
			   ,theMFCRect.top
			   ,theMFCRect.Width()
			   ,theMFCRect.Height()
			   ,theDC
			   ,0
			   ,0
			   ,SRCCOPY);
}

// V‰liaikainen(?) funktio, joka piir‰‰ annetun DC:n sis‰llˆn toolboxiin
void NFmiToolBox::DrawDC(CDC* theDC, const NFmiRect& theRelativeSize, const NFmiRect& theRelativeSourceStart)
{
	UpdateClientRect(); // poista t‰m‰ kun voit (tarvitaanko?!?!?)
	CRect theMFCRect;
	ConvertRect(theRelativeSize, theMFCRect);
	CRect theMFCRectSource;
	ConvertRect(theRelativeSourceStart, theMFCRectSource);
	pDC->BitBlt(theMFCRect.left
			   ,theMFCRect.top
			   ,theMFCRect.Width()
			   ,theMFCRect.Height()
			   ,theDC
			   ,theMFCRectSource.left
			   ,theMFCRectSource.top
			   ,SRCCOPY);
}

// piirt‰‰ l‰j‰n erillisi‰ viivoja samalla pensselill‰
bool NFmiToolBox::DrawValueLineList(NFmiValueLineList* theLineList, NFmiDrawingEnvironment *theEnvi, const NFmiRect& theRelativeSize)
{
	SetUpClipping();

	ConvertEnvironment(theEnvi);
	CPoint mfcPoint1, mfcPoint2;
	NFmiPoint point1, point2;

	if(!pItsPen->CreatePen(itsPenStyle, itsPenSize.cx, itsFrameColor)) //061196/LW oli itsFillColor
	{
		ConvertEnvironment(itsBaseEnvironment);
		EndClipping();
		return false;
	}

	CPen* pOldPen = pDC->SelectObject(pItsPen);
	for(theLineList->Reset(); theLineList->Next(); )
	{
		point1 = theLineList->Current()->Start();
		ConvertPoint(point1, mfcPoint1);
		point2 = theLineList->Current()->End();
		ConvertPoint(point2, mfcPoint2);
	    pDC->MoveTo(mfcPoint1.x, mfcPoint1.y);
	    pDC->LineTo(mfcPoint2.x, mfcPoint2.y);
	}

	pDC->SelectObject(pOldPen);
	pItsPen->DeleteObject();

	ConvertEnvironment(itsBaseEnvironment);

	for(theLineList->PolyReset(); theLineList->PolyNext(); )
		BuildPolyline(theLineList->PolyCurrent());

	EndClipping();
	return true;
}

void NFmiToolBox::FillPolyPolygonPoints(std::list<NFmiPolyline*> &thePolyLineList, std::vector<CPoint> &thePoints, std::vector<int> &thePolygonPointCounts, int &thePolygonCount, const NFmiPoint &theOffSet)
{
	using namespace std;
	thePolygonCount = static_cast<int>(thePolyLineList.size());
	list<NFmiPolyline*>::iterator it = thePolyLineList.begin();
	list<NFmiPolyline*>::iterator endIt = thePolyLineList.end();
	CPoint mfcPoint;
	CPoint mfcOffSet;
	ConvertPoint(theOffSet, mfcOffSet);
	for(; it != endIt; ++it)
	{ 
		const auto &fmiPoints = (*it)->GetPoints();
		thePolygonPointCounts.push_back(static_cast<int>(fmiPoints.size()));
		for(size_t index = 0; index < fmiPoints.size(); index++)
		{
			ConvertPoint(fmiPoints[index], mfcPoint);
			mfcPoint.Offset(mfcOffSet);
			thePoints.push_back(mfcPoint);
		}
	}
}

// oletus: listassa olevat polylinet voivat sis‰lt‰‰ reiki‰ ja saaria, ja ne fillataan sen mukaan
bool NFmiToolBox::DrawMultiPolygon(std::list<NFmiPolyline*> &thePolyLineList, NFmiDrawingEnvironment * theEnvi, const NFmiPoint &theOffSet)
{
	if(thePolyLineList.size() < 1)
		return false;

	if(thePolyLineList.size() == 1)
	{
		std::list<NFmiPolyline*>::iterator it = thePolyLineList.begin();
		NFmiPoint scale;
		DrawPolyline(*it, theOffSet, scale);
	}
	else
	{
		SetUpClipping();
		ConvertEnvironment(theEnvi);
		if(fFilled)
		{
			CPen *oldPen = 0;
			int penStyle = fFramed ? itsPenStyle : PS_NULL;
			int penWidth = fFramed ? 1 : 0;
			COLORREF penColor1 = 0x00000000;
			CPen myPen(penStyle, penWidth, penColor1);
				oldPen = pDC->SelectObject(&myPen); // ei piirret‰ polygonille reunoja

			bool status = false;
			std::list<NFmiPolyline*>::iterator it = thePolyLineList.begin();
			if(itsHatchPattern == -1)
				status = pItsFillPattern->CreateSolidBrush(itsFillColor) != 0; // pDC->GetNearestColor
			else
				status = pItsFillPattern->CreateHatchBrush(itsHatchPattern, itsFillColor) != 0; // pDC->GetNearestColor
			if(!status)
			{
				DeSelectEnvironment(*it);
				EndClipping();
				return false;
			}
			pDC->SetBkMode((*it)->BackGroundMode());
			CBrush* pOldBrush = pDC->SelectObject(pItsFillPattern);

			std::vector<CPoint> points;
			std::vector<int> polCounts;
			int polCount = 0;
			FillPolyPolygonPoints(thePolyLineList, points, polCounts, polCount, theOffSet);
			pDC->PolyPolygon(&points[0], &polCounts[0], polCount);

			pDC->SelectObject(pOldBrush);
			pItsFillPattern->DeleteObject();
				pDC->SelectObject(oldPen);
			ConvertEnvironment(itsBaseEnvironment);
		}
	}
	EndClipping();
	return true;
}

void NFmiToolBox::DrawEllipse(const NFmiRect& theRelativeSize, NFmiDrawingEnvironment *theEnvi)
{
	SetUpClipping();
	ConvertEnvironment(theEnvi);
	if(!pItsPen->CreatePen(fFramed ? itsPenStyle : PS_NULL, itsPenSize.cx, itsFrameColor)) //061196/LW oli itsFillColor
	{
		ConvertEnvironment(itsBaseEnvironment);
		EndClipping();
		return ;
	}
	CPen* pOldPen = pDC->SelectObject(pItsPen);

	CBrush* pOldBrush = 0;
	if(fFilled)
	{
		if(!pItsFillPattern->CreateSolidBrush(itsFillColor)) // pDC->GetNearestColor
		{
			ConvertEnvironment(itsBaseEnvironment);
			EndClipping();
			return ;
		}
		pOldBrush = pDC->SelectObject(pItsFillPattern);
	}
	else
  		pDC->SelectStockObject(NULL_BRUSH);

	CRect theMFCRect;
	ConvertRect(theRelativeSize, theMFCRect);

	pDC->Ellipse(theMFCRect);

	pDC->SelectObject(pOldPen);
	pItsPen->DeleteObject();
	if(fFilled)
	{
	  	pDC->SelectObject(pOldBrush);
		pItsFillPattern->DeleteObject();
	}
	else
		pDC->SelectStockObject(WHITE_BRUSH); // en tajua, miksi t‰nne pit‰‰ laittaa lopuksi valkoinen sivellin, mutta en halua vaihtaa koodia
	ConvertEnvironment(itsBaseEnvironment);
	EndClipping();
}

// 23.2.2001/Marko
void NFmiToolBox::SetDC(CDC* pmyDC, bool fReCalcClientRect)
{
	pDC = pmyDC;
	if(fReCalcClientRect)
		GetDravingRect();
}
// 23.2.2001/Marko
void NFmiToolBox::GetPrintInfo(CPrintInfo* pPrintInfo)
{
	pItsPrintInfo = pPrintInfo ;
	if(pItsPrintInfo)
	{
		mClientRect = pItsPrintInfo->m_rectDraw;
	}
}

void NFmiToolBox::SetUpClipping()
{
	if(fUseClipping)
	{
		CRect mfcClipRect;
		ConvertRect(itsRelativeClipRect, mfcClipRect);
		itsClipRegion.CreateRectRgn(mfcClipRect.left , mfcClipRect.top, mfcClipRect.right, mfcClipRect.bottom);
		pDC->SelectClipRgn(&itsClipRegion);
	}
}

void NFmiToolBox::EndClipping(void)
{
	if(fUseClipping)
	{
		pDC->SelectClipRgn(NULL);
		itsClipRegion.DeleteObject();
	}
}

void NFmiToolBox::ConvertRect(const CRect & absoluteRect, NFmiRect & relativeRect)
{
	relativeRect.Top(itsYDirection * (static_cast<double>(absoluteRect.top) / mClientRect.Height()));
	relativeRect.Left(itsXDirection * (static_cast<double>(absoluteRect.left) / mClientRect.Width()));
	relativeRect.Bottom(itsYDirection * (static_cast<double>(absoluteRect.bottom) / mClientRect.Height()));
	relativeRect.Right(itsXDirection * (static_cast<double>(absoluteRect.right) / mClientRect.Width()));
}

void NFmiToolBox::ConvertPoint(const CPoint& absolutePoint, NFmiPoint& relativePoint)
{
    relativePoint.X(itsXDirection * (static_cast<double>(absolutePoint.x) / mClientRect.Width()));
    relativePoint.Y(itsYDirection * (static_cast<double>(absolutePoint.y) / mClientRect.Height()));
}

void NFmiToolBox::DoSymbolBulkDraw(const NFmiSymbolBulkDrawData& sbdData, bool doStationPlotOnly)
{
	SetUpClipping();
	if(!doStationPlotOnly)
	{
		if(sbdData.drawType() == NFmiSymbolBulkDrawType::Text)
		{
			DoTextBulkDraw(sbdData);
		}
		else if(sbdData.drawType() == NFmiSymbolBulkDrawType::Arrow)
		{
			DoArroyBulkDraw(sbdData);
		}
		else if(sbdData.drawType() == NFmiSymbolBulkDrawType::WindBarb)
		{
			DoWindBarbBulkDraw(sbdData);
		}
		else if(sbdData.drawType() == NFmiSymbolBulkDrawType::HessaaSymbol)
		{
			DoSimpleWeatherBulkkDraw(sbdData);
		}
	}

	DoStationPlotkDraw(sbdData);

	EndClipping();
}

class MfcFontHandler
{
	std::map<int, std::unique_ptr<CFont>> fonts_;
	CDC* pDC_ = nullptr;
	CFont* oldFont_ = nullptr;
public:
	MfcFontHandler(CDC* pDC, const std::vector<NFmiPoint>& fontSizes, bool useBold, const std::wstring &fontName)
		:pDC_(pDC)
	{
		auto differentFontSizes = makeFontSizeSet(fontSizes);
		if(!differentFontSizes.empty())
		{
			for(auto fontSize : differentFontSizes)
			{
				auto insertIter = fonts_.insert(std::make_pair(fontSize, std::make_unique<CFont>()));
				if(insertIter.second)
				{
					insertIter.first->second->CreateFont(
						fontSize // desired height of the font in logical units
						, 0 // width
						, 0 // escapement (angle in 0.1 degrees)
						, 1 // orientation (angle in 0.1 degrees)
						, useBold ? FW_BOLD : FW_NORMAL // weight (FW_NORMAL, FW_BOLD jne.)
						, 0 // italic
						, 0 // underline
						, 0 // strikeout
						, 1 // char set 
						, OUT_TT_PRECIS
						, 1 // nClipPrecision
						, PROOF_QUALITY
						, DEFAULT_PITCH
						, fontName.c_str());
				}
			}
			if(!fonts_.empty())
			{
				oldFont_ = pDC_->SelectObject(fonts_.begin()->second.get());
			}
		}
	}

	~MfcFontHandler()
	{
		pDC_->SelectObject(oldFont_);
		for(auto& fontPtr : fonts_)
		{
			fontPtr.second->DeleteObject();
		}
	}

	void setUsedFont(const NFmiPoint& fontSize)
	{
		auto fontIter = fonts_.find(makeFontSizeConversion(fontSize));
		if(fontIter != fonts_.end())
		{
			pDC_->SelectObject(fontIter->second.get());
		}
	}

private:
	int makeFontSizeConversion(const NFmiPoint& fontSize)
	{
		return boost::math::iround(fontSize.Y());
	}

	std::set<int> makeFontSizeSet(const std::vector<NFmiPoint>& fontSizes)
	{
		std::set<int> finalSizes;
		for(const auto& fontSize : fontSizes)
			finalSizes.insert(makeFontSizeConversion(fontSize));

		return finalSizes;
	}
};

class CdcRestorer
{
	CDC* pDC_ = nullptr;
	int oldBkMode_ = 0;
	COLORREF oldBkColor_ = 0;
	UINT oldTextAlingment_ = 0;
public:
	CdcRestorer(CDC* pDC, bool useTransparentBackground, COLORREF backgroundFillColor, UINT textAlingment)
		:pDC_(pDC)
	{
		oldBkMode_ = pDC_->SetBkMode(TRANSPARENT);
		oldBkColor_ = pDC_->GetBkColor();
		if(!useTransparentBackground)
		{
			pDC_->SetBkMode(OPAQUE);
			pDC_->SetBkColor(backgroundFillColor);
		}
		oldTextAlingment_ = pDC_->SetTextAlign(textAlingment);
	}

	~CdcRestorer()
	{
		pDC_->SetBkColor(oldBkColor_);
		pDC_->SetBkMode(oldBkColor_);
		pDC_->SetTextAlign(oldTextAlingment_);
	}
};

static COLORREF Color2ColorRef(const NFmiColor& theColor)
{
	return RGB(theColor.Red() * 255, theColor.Green() * 255, theColor.Blue() * 255);
}

static bool isMissingValue(const NFmiSymbolBulkDrawData& sbdData, size_t index)
{
	return sbdData.values()[index] == kFloatMissing;
}

// Normipiirto vakioasetuksilla (vakio v‰ri, vakio koko)
static void DoBasicTextBulkDraw(const NFmiSymbolBulkDrawData& sbdData, CDC *pDC, const std::vector<CPoint> &mfcTextPositions)
{
	// Asetetaan k‰ytetty vakio v‰ri ja otetaan vanha v‰ri talteen, jotta se voidaan lopuksi palauttaa.
	auto oldTextColor = pDC->SetTextColor(Color2ColorRef(sbdData.colors().front()));

	for(size_t index = 0; index < mfcTextPositions.size(); index++)
	{
		if(!isMissingValue(sbdData, index))
		{
			const auto& textPosition = mfcTextPositions[index];
			const auto& text = sbdData.drawnTexts()[index];
			pDC->TextOut(textPosition.x, textPosition.y, CString(CA2T(text.c_str())));
		}
	}

	// Siivoa piirtokoodin asetukset eli palauta originaali teksti v‰ri
	pDC->SetTextColor(oldTextColor);
}

// Normi teksti piirto eri v‰reill‰ (vakio koko)
static void DoChangingColorTextBulkDraw(const NFmiSymbolBulkDrawData& sbdData, CDC* pDC, const std::vector<CPoint>& mfcTextPositions)
{
	// Otetaan vain vanha v‰ri talteen, jotta se voidaan lopuksi palauttaa.
	auto oldTextColor = pDC->SetTextColor(0);

	for(size_t index = 0; index < mfcTextPositions.size(); index++)
	{
		if(!isMissingValue(sbdData, index))
		{
			pDC->SetTextColor(Color2ColorRef(sbdData.colors()[index]));
			const auto& textPosition = mfcTextPositions[index];
			const auto& text = sbdData.drawnTexts()[index];
			pDC->TextOut(textPosition.x, textPosition.y, CString(CA2T(text.c_str())));
		}
	}

	// Siivoa piirtokoodin asetukset eli palauta originaali teksti v‰ri
	pDC->SetTextColor(oldTextColor);
}

static void FixChangingFontSizePosition(CPoint& fixedPosition, const NFmiPoint& symbolSize)
{
	fixedPosition.y -= boost::math::iround(symbolSize.Y() / 2.f);
}

// Normi teksti piirto eri fontti koilla (vakio v‰ri)
static void DoChangingFontSizeTextBulkDraw(const NFmiSymbolBulkDrawData& sbdData, CDC* pDC, const std::vector<CPoint>& mfcTextPositions, MfcFontHandler &fontHandler, NFmiToolBox &toolbox)
{
	// Asetetaan k‰ytetty vakio v‰ri ja otetaan vanha v‰ri talteen, jotta se voidaan lopuksi palauttaa.
	auto oldTextColor = pDC->SetTextColor(Color2ColorRef(sbdData.colors().front()));

	for(size_t index = 0; index < mfcTextPositions.size(); index++)
	{
		if(!isMissingValue(sbdData, index))
		{
			auto textPosition = mfcTextPositions[index];
			const auto& text = sbdData.drawnTexts()[index];
			const auto& fontSize = sbdData.symbolSizes()[index];
			fontHandler.setUsedFont(fontSize);
			::FixChangingFontSizePosition(textPosition, fontSize);
			pDC->TextOut(textPosition.x, textPosition.y, CString(CA2T(text.c_str())));
		}
	}

	// Siivoa piirtokoodin asetukset eli palauta originaali teksti v‰ri
	pDC->SetTextColor(oldTextColor);
}

// Teksti piirto eri v‰reill‰ ja fontti koilla
static void DoChangingColorAndSizeTextBulkDraw(const NFmiSymbolBulkDrawData& sbdData, CDC* pDC, const std::vector<CPoint>& mfcTextPositions, MfcFontHandler& fontHandler)
{
	// Otetaan vanha v‰ri talteen, jotta se voidaan lopuksi palauttaa (0 on musta v‰ri, joka asetetaan aluksi k‰yttˆˆn).
	auto oldTextColor = pDC->SetTextColor(0);

	for(size_t index = 0; index < mfcTextPositions.size(); index++)
	{
		if(!isMissingValue(sbdData, index))
		{
			pDC->SetTextColor(Color2ColorRef(sbdData.colors()[index]));
			auto textPosition = mfcTextPositions[index];
			const auto& text = sbdData.drawnTexts()[index];
			const auto& fontSize = sbdData.symbolSizes()[index];
			fontHandler.setUsedFont(fontSize);
			::FixChangingFontSizePosition(textPosition, fontSize);
			pDC->TextOut(textPosition.x, textPosition.y, CString(CA2T(text.c_str())));
		}
	}

	// Siivoa piirtokoodin asetukset eli palauta originaali teksti v‰ri
	pDC->SetTextColor(oldTextColor);
}

void NFmiToolBox::DoTextBulkDraw(const NFmiSymbolBulkDrawData& sbdData)
{
	LoggingProfilerHelper profiler(__FUNCTION__);

	// 1. Tee peruspiirtoasetukset
	// 	   a) Luo fontti, aseta se CDC:lle
	MfcFontHandler fontHandler(pDC, sbdData.symbolSizes(), sbdData.useBoldFont(), sbdData.fontName());

	//	   b) Konvertoi piirtopisteet ja lis‰‰ pisteisiin konvertoitu offset
	CPoint mfcOffSet;
	ConvertPoint(sbdData.relativePositionOffset(), mfcOffSet);
	std::vector<CPoint> mfcTextPositions;
	ConvertPointList(sbdData.relativeStationPointPositions(), mfcTextPositions, mfcOffSet, 0, 0, 0);

	// 	   c) Mieti mit‰ seurauksia on sill‰ ett‰ BuildText:iss‰ otetaan piirtopiste DrawObjectin top-left:ist‰!!!

	// 	   d) Tee v‰riasetuksia CDC:en
	bool useTransparentBackground = true;
	COLORREF backgroundFillColor = RGB(0, 0, 0);
	CdcRestorer cdcRestorer(pDC, useTransparentBackground, backgroundFillColor, TA_CENTER | TA_TOP);

	// 2. Hajauta piirto seuraaviin osiin
	if(sbdData.isChangingColorsAndSizesUsed())
	{
		// a) Muuttuvat v‰rit ja symboli koko
		::DoChangingColorAndSizeTextBulkDraw(sbdData, pDC, mfcTextPositions, fontHandler);
	}
	else if(sbdData.isChangingColorsUsed())
	{
		// a) Muuttuvat v‰rit
		::DoChangingColorTextBulkDraw(sbdData, pDC, mfcTextPositions);
	}
	else if(sbdData.isChangingSizesUsed())
	{
		// b) Muuttuvat symbolikoot
		::DoChangingFontSizeTextBulkDraw(sbdData, pDC, mfcTextPositions, fontHandler, *this);
	}
	else
	{
		// c) Normipiirto vakioasetuksilla
		::DoBasicTextBulkDraw(sbdData, pDC, mfcTextPositions);
	}

}

static void DoPolylineConversion(std::vector<CPoint>& mfcPolylinePositions, float rotationAngle, const CPoint offset)
{
	for(auto& point : mfcPolylinePositions)
	{
		::RotatePoint(point, rotationAngle);
		point.Offset(offset);
	}
}

void NFmiToolBox::DoArroyBulkDraw(const NFmiSymbolBulkDrawData& sbdData)
{
	std::vector<NFmiPoint> arrowPoints{ NFmiPoint(0, 1), NFmiPoint(0, -1), NFmiPoint(-0.5, -0.25), NFmiPoint(0, -1), NFmiPoint(0.5, -0.25) };
	// Samaa kokoskaalaa k‰ytet‰‰n x- ja y-suunnissa
	auto scale = sbdData.symbolSizes().front();

	CPoint mfcOffSet;
	ConvertPoint(sbdData.relativePositionOffset(), mfcOffSet);
	std::vector<CPoint> mfcArrowPositions;
	ConvertPointList(sbdData.relativeStationPointPositions(), mfcArrowPositions, mfcOffSet, 0, 0, 0);
	CPen pen;
	if(pen.CreatePen(PS_SOLID, sbdData.penSize(), ::Color2ColorRef(sbdData.colors().front())))
	{
		auto oldPen = pDC->SelectObject(&pen);
		for(size_t index = 0; index < mfcArrowPositions.size(); index++)
		{
			if(!isMissingValue(sbdData, index))
			{
				auto rotationAngle = sbdData.values()[index];
				const auto& arrowPosition = mfcArrowPositions[index];

				std::vector<CPoint> mfcArrowPoints;
				ConvertPointList(arrowPoints, mfcArrowPoints, arrowPosition, scale.X(), scale.Y(), rotationAngle);

				pDC->Polyline(mfcArrowPoints.data(), static_cast<int>(mfcArrowPoints.size()));
			}
		}
		pDC->SelectObject(oldPen);
		pItsPen->DeleteObject();
	}
}

std::pair<float, float> NFmiToolBox::GetWsAndWdFromWindVector(float windVector)
{
	if(windVector == kFloatMissing)
		return std::make_pair(kFloatMissing, kFloatMissing);
	else
	{
		float WS = static_cast<float>(static_cast<int>(windVector / 100));
		float WD = float(((int)windVector % 100) * 10);
		return std::make_pair(WS, WD);
	}
}

void NFmiToolBox::DoWindBarbBulkDraw(const NFmiSymbolBulkDrawData& sbdData)
{
	NFmiDrawingEnvironment envi;
	envi.SetFillColor(sbdData.colors().front());
	envi.SetFrameColor(sbdData.colors().front());
	envi.SetPenSize(NFmiPoint(sbdData.penSize(), sbdData.penSize()));

	const auto& stationLatlons = sbdData.stationPointLatlons();
	const auto& relativeStationPositions = sbdData.relativeStationPointPositions();
	const auto& relativeObjectSize = sbdData.relativeDrawObjectSize();
	const auto& symbolSize = sbdData.symbolSizes().front();
	NFmiRect relativeSymbolRect;
	relativeSymbolRect.Size(relativeObjectSize);

	// Clippaus pit‰‰ laittaa pois p‰‰lt‰, muuten tulee p‰‰llekk‰isi‰ clippaus 
	// asetuksia NFmiWindBarb olion sis‰ll‰, ja ohjelma kaatuu...
	TurnClippingOffHelper turnClippingOffHelper(this);

	for(size_t index = 0; index < relativeStationPositions.size(); index++)
	{
		if(!isMissingValue(sbdData, index))
		{
			float ws, wd;
			std::tie(ws, wd) = NFmiToolBox::GetWsAndWdFromWindVector(sbdData.values()[index]);
			// Huom1 WindBarb:illa ei voi olla offset:ia, sit‰ ei vain oteta ollenkaan huomioon.
			// Huom2 sijoittelu tapahtuu puhtaasti suhteellisilla koordinaateilla (NFmiWindBarb::Build 
			// tekee sitten lopuksi koordinaattimuunnokset piirrossa).
			relativeSymbolRect.Center(relativeStationPositions[index]);

			NFmiWindBarb(ws
				, wd
				, relativeSymbolRect
				, this
				, stationLatlons[index].Y() < 0
				, symbolSize.X()
				, symbolSize.Y()
				, nullptr
				, &envi).Build();
		}
	}
}

void NFmiToolBox::DoSimpleWeatherBulkkDraw(const NFmiSymbolBulkDrawData& sbdData)
{
	NFmiDrawingEnvironment envi;
	envi.SetFrameColor(NFmiColor(1, 0, 0));
	envi.DisableFill();

	const auto& relativeStationPositions = sbdData.relativeStationPointPositions();
	const auto& relativeObjectSize = sbdData.relativeDrawObjectSize();
	const auto& relativeOffset = sbdData.relativePositionOffset();
	NFmiRect relativeSymbolRect;
	relativeSymbolRect.Size(relativeObjectSize);

	// Clippaus pit‰‰ laittaa pois p‰‰lt‰, muuten tulee p‰‰llekk‰isi‰ clippaus 
	// asetuksia NFmiWindBarb olion sis‰ll‰, ja ohjelma kaatuu...
	TurnClippingOffHelper turnClippingOffHelper(this);

	for(size_t index = 0; index < relativeStationPositions.size(); index++)
	{
		if(!isMissingValue(sbdData, index))
		{
			auto value = sbdData.values()[index];
			auto relativeCenterPoint = relativeStationPositions[index];
			relativeCenterPoint += relativeOffset;
			relativeSymbolRect.Center(relativeCenterPoint);

			NFmiSimpleWeatherSymbol(static_cast<short>(value)
				, relativeSymbolRect
				, this
				, nullptr
				, &envi).Build();
		}
	}
}

#ifdef max
#undef max
#undef min
#endif

// Laita DRAW_STATION_POINT_PIXEL define kommenttiin, kun piirtotestit on ohi
//#define DRAW_STATION_POINT_PIXEL 1

void NFmiToolBox::DoStationPlotkDraw(const NFmiSymbolBulkDrawData& sbdData)
{
	if(sbdData.drawStationPoint())
	{
		COLORREF stationPlotColor = ::Color2ColorRef(sbdData.stationPointColor());
		CBrush brush;
		if(brush.CreateSolidBrush(stationPlotColor))
		{
			auto oldBrush =	pDC->SelectObject(&brush);

			// Normitilanteessa pit‰‰ asemapiste piirt‰‰ CDC::Ellipse metodilla ja
			// sen kanssa pit‰‰ pDC:n kyn‰ laittaa pois p‰‰lt‰.
			auto emptyPen = ::GetStockObject(NULL_PEN);
			auto oldPen = pDC->SelectObject(emptyPen);

			CRect mfcRect;
			ConvertRect(sbdData.baseStationPointRect(), mfcRect);
			// Jos piirt‰‰ CDC::Ellipse metodilla 2x2 kokoista ympyr‰‰ tai pienemp‰‰, ei tule yht‰‰n 
			// mit‰‰n n‰kyviin, siksi ne ovat erikoistapauksia, jolloin piirto tehd‰‰n 1 tai 4 pikselin erillispiirrolla.
			bool doRectangleSpecial = mfcRect.Width() <= 2;
			bool doFourPointSpecial = mfcRect.Width() == 2;

			// CDC::Ellipse piirrossa pit‰‰ piirto rect:i‰ kasvattaa pyk‰l‰ll‰ alas ja oikealle.
			mfcRect.bottom++;
			mfcRect.right++;
			
			// CRect oliolle ei ole SetCenter metodia, vaan CRect:eja siirret‰‰n upper-left
			// kulman suhteen. Siksi joudumme laskemaan laatikon koon suhteen sopivan offsetin,
			// jolla s‰‰det‰‰n laskettavien mfcPoints:ien sijainnit, joilla s‰‰det‰‰n
			// piirrett‰v‰n ellipsin tai laatikon paikkaa.
			auto pixelOffset = -std::max(0, (boost::math::iround(mfcRect.Width() / 2.) - 1));
			CPoint mfcOffSet(pixelOffset, pixelOffset);
			std::vector<CPoint> mfcPoints;
			ConvertPointList(sbdData.relativeStationPointPositions(), mfcPoints, mfcOffSet, 0, 0, 0);
			CPoint zeroOffSet;
			std::vector<CPoint> originalMfcPoints;
			ConvertPointList(sbdData.relativeStationPointPositions(), originalMfcPoints, zeroOffSet, 0, 0, 0);
#ifdef DRAW_STATION_POINT_PIXEL
			COLORREF stationPixelColor = RGB(128, 0, 255);
#endif // DRAW_STATION_POINT_PIXEL
			for(size_t index = 0; index < mfcPoints.size(); index++)
			{

				if(doRectangleSpecial)
				{
					CPoint specialCasePoint = originalMfcPoints[index];
					// Pikselien piirtoon k‰ytet‰‰n CDC::SetPixelV metodia, koska se on nopeampi kuin CDC::SetPixel.
					pDC->SetPixelV(specialCasePoint, stationPlotColor);
					if(doFourPointSpecial)
					{
						specialCasePoint.x--;
						pDC->SetPixelV(specialCasePoint, stationPlotColor);
						specialCasePoint.y--;
						pDC->SetPixelV(specialCasePoint, stationPlotColor);
						specialCasePoint.x++;
						pDC->SetPixelV(specialCasePoint, stationPlotColor);
					}
				}
				else
				{
					mfcRect.MoveToXY(mfcPoints[index]);
					pDC->Ellipse(&mfcRect);
				}

#ifdef DRAW_STATION_POINT_PIXEL
				pDC->SetPixelV(originalMfcPoints[index], stationPixelColor);
#endif // DRAW_STATION_POINT_PIXEL
			}

			// Lopuksi siivotaan...
			pDC->SelectObject(oldBrush);
			pDC->SelectObject(oldPen);
			brush.DeleteObject();
		}
	}
}

#endif


