#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255 merkkiä joka johtuu 'puretuista' STL-template nimistä)
#endif

#include "NFmiToolBox.h"
#include "NFmiZoomView.h"
#include "NFmiArea.h"
#include "NFmiRectangle.h"
#include "CtrlViewDocumentInterface.h"
#include "MapHandlerInterface.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NFmiZoomView::NFmiZoomView(int theMapViewDescTopIndex, NFmiToolBox *theToolBox
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam)
    :NFmiCtrlView(theMapViewDescTopIndex, CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->GetMapHandlerInterface(theMapViewDescTopIndex)->TotalArea()->XYArea()
        , theToolBox
        , theDrawParam)
    , itsZoomedAreaRect()
    , itsZoomedArea()
{
    itsZoomedArea = GetMapHandlerInterface()->Area();
    itsZoomedAreaRect = CalcZoomedAreaRect();
}

NFmiZoomView::NFmiZoomView(int theMapViewDescTopIndex, NFmiToolBox *theToolBox
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , const NFmiRect& theRect)
    :NFmiCtrlView(theMapViewDescTopIndex, theRect
        , theToolBox
        , theDrawParam)
    , itsZoomedAreaRect()
    , itsZoomedArea()
{
    itsZoomedArea = GetMapHandlerInterface()->Area();
    itsZoomedAreaRect = CalcZoomedAreaRect();
}

NFmiZoomView::~NFmiZoomView()
{
}

void NFmiZoomView::Draw(NFmiToolBox * theGTB)
{
	DrawConstAreaRects();
	DrawZoomedAreaRect();
}

void NFmiZoomView::DrawZoomedAreaRect()
{
	NFmiPoint penSize(3,3);
	DrawInvertRect(itsZoomedAreaRect,penSize);
}

void NFmiZoomView::DrawConstAreaRects(void)
{
}

void NFmiZoomView::DrawInvertRect(const NFmiRect& theRect, const NFmiPoint& thePenSize)
{
	NFmiPoint oldPenSize(itsDrawingEnvironment.GetPenSize());
	itsDrawingEnvironment.SetPenSize(thePenSize);
	itsDrawingEnvironment.EnableInvert();
	NFmiRectangle rect(theRect.TopLeft(),
			   theRect.BottomRight(),
			   0,
			   &itsDrawingEnvironment);
	itsToolBox->Convert(&rect);
	itsDrawingEnvironment.DisableInvert();
	itsDrawingEnvironment.SetPenSize(oldPenSize);
}

void NFmiZoomView::DrawZoomedAreaRect(const NFmiPoint& thePlace)
{
	DrawZoomedAreaRect(); // 'draws' previous rectangle away
	itsZoomedAreaRect.Center(thePlace);
	DrawZoomedAreaRect(); // draws new rectangle with invert colors
}

void NFmiZoomView::DrawZoomedAreaRect(const NFmiRect& newZoomedRect)
{
	DrawZoomedAreaRect(); // 'draws' previous rectangle away
	itsZoomedAreaRect = newZoomedRect;
	DrawZoomedAreaRect(); // draws new rectangle with invert colors
}

void NFmiZoomView::DrawZoomedAreaRect(const NFmiPoint& thePlace, FmiDirection theDragTarget)
{
	if(theDragTarget == kCenter)
		DrawZoomedAreaRect(thePlace);
	else
	{
		DrawZoomedAreaRect(); // 'draws' previous rectangle away
		if(thePlace.X() > itsZoomedAreaRect.Left())
			itsZoomedAreaRect.Right(thePlace.X());
		if(thePlace.Y() > itsZoomedAreaRect.Top())
			itsZoomedAreaRect.Bottom(thePlace.Y());
		AdjustZoomedAreaRect();
		DrawZoomedAreaRect(); // draws new rectangle with invert colors
	}
}

void NFmiZoomView::AdjustZoomedAreaRect(void)
{
    if(itsCtrlViewDocumentInterface->KeepMapAspectRatio())
	{ // HUOM!! Onkohan tämä liian monimutkainen viritys?!?!?
	  // Tarkoitus on pitaa 'zoom'-laatikon aspectratio karttanäytön aspectration mukaisena
		boost::shared_ptr<NFmiArea> totalArea = GetMapHandlerInterface()->TotalArea();
		boost::shared_ptr<NFmiArea> area(totalArea->CreateNewArea(itsZoomedAreaRect));
		if(area)
		{
			boost::shared_ptr<NFmiArea> area2(area->CreateNewArea(itsCtrlViewDocumentInterface->ClientViewXperYRatio(itsMapViewDescTopIndex), kTopLeft, true));
			if(area2)
                itsZoomedAreaRect = totalArea->XYArea(area2.get());
		}
	}
}

NFmiRect NFmiZoomView::CalcZoomedAreaRect()
{
    return GetMapHandlerInterface()->TotalArea()->XYArea(itsZoomedArea.get());
}

NFmiRect NFmiZoomView::CalcHandlersZoomedAreaRect()
{
    auto mapHandlerInterface = GetMapHandlerInterface();
    return mapHandlerInterface->TotalArea()->XYArea(mapHandlerInterface->Area().get());
}

void NFmiZoomView::CalcZoomedArea(void)
{
    auto mapHandlerInterface = GetMapHandlerInterface();
    // tässä estetään latlon maailma kartan 'ylivuotoa', joka kaataa MetEditorin eli ei anneta latlon area zoomin mennä yli reunojen
	itsZoomedAreaRect = mapHandlerInterface->TotalArea()->XYArea().Intersection(itsZoomedAreaRect);

	itsZoomedArea = boost::shared_ptr<NFmiArea>(mapHandlerInterface->TotalArea()->CreateNewArea(itsZoomedAreaRect));
}

boost::shared_ptr<NFmiArea> NFmiZoomView::ZoomedArea(void)
{
	CalcZoomedArea(); // I couldn't make better place for this but this way area is supposed
					  // to be calculated only when its needed
	return itsZoomedArea;
}

bool NFmiZoomView::LeftButtonDown(const NFmiPoint &thePlace, unsigned long /* theKey */)
{
	NFmiPoint point(itsZoomedAreaRect.BottomRight());
	double xDiff = fabs(point.X()-thePlace.X());
	double yDiff = fabs(point.Y()-thePlace.Y());
	if(xDiff < 0.02 && yDiff < 0.02)
	{
		itsDragTarget = kBottomRight;
		return true;
	}
	else if(itsZoomedAreaRect.IsInside(thePlace))
	{
		itsDragTarget = kCenter;
		return true;
	}
	return false; // tässä palauttaa arvon, laitetaanko mouse drag päälle vai ei?
}

bool NFmiZoomView::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(theKey & kCtrlKey) // select constant area
	{
		if(theKey & kShiftKey) // select total area jos ctrl+shift pohjassa
		{
            auto mapHandlerInterface = GetMapHandlerInterface();
            mapHandlerInterface->SetMaxArea();
            mapHandlerInterface->SetMakeNewBackgroundBitmap(true);
			DrawZoomedAreaRect(mapHandlerInterface->Position());
			return true;
		}
		else // no shift
		{
			ScaleZoomedAreaRect(0.6);
			DrawZoomedAreaRect();
			return true;
		}
	}
	else
	{
		if(itsDragTarget == kBottomRight) // 1999.08.30/Marko
			itsDragTarget = kCenter; // 1999.08.30/Marko
		else if(itsDragTarget == kCenter)
			DrawZoomedAreaRect(thePlace); // this calculates new rect also
		return true;
	}
}

bool NFmiZoomView::RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(theKey & kCtrlKey)
	{
		ScaleZoomedAreaRect(1.4);
		DrawZoomedAreaRect();
		return true;
	}
	return false;
}

void NFmiZoomView::ScaleZoomedAreaRect(double theScaleFactor)
{
	NFmiPoint center(itsZoomedAreaRect.Center());
	double width = itsZoomedAreaRect.Width() * theScaleFactor;
	double height = itsZoomedAreaRect.Height() * theScaleFactor;
	itsZoomedAreaRect.Size(NFmiPoint(width, height));
	itsZoomedAreaRect.Center(center);
}

bool NFmiZoomView::MouseMove(const NFmiPoint & thePlace, unsigned long /* theKey */)
{
	DrawZoomedAreaRect(thePlace,itsDragTarget); // this calculates new rect also
	return false;
}

void NFmiZoomView::Update(void)
{
	static bool firstTime = true;
	static int lastMapViewDescTopIndex = -1;
	static int lastUsedMapIndex = -1;
	
    auto mapHandlerInterface = GetMapHandlerInterface();
    int currentMapIndex = mapHandlerInterface->UsedMapIndex();
	bool reCalcArea = mapHandlerInterface->MapReallyChanged();
    mapHandlerInterface->MapReallyChanged(false); // tämä nollataan tässä, se on laitettu päälle NFmiGdiPlusImageMapHandler::Area -metodissa
	if(firstTime || reCalcArea || (lastMapViewDescTopIndex != itsMapViewDescTopIndex) || (lastUsedMapIndex != currentMapIndex))
	{
		firstTime = false;
		lastMapViewDescTopIndex = itsMapViewDescTopIndex;
		lastUsedMapIndex = currentMapIndex;

		itsZoomedAreaRect = CalcHandlersZoomedAreaRect();
	}
	DrawZoomedAreaRect();
}
