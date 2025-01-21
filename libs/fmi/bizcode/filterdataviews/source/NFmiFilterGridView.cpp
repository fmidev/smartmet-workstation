// NFmiFilterGridView.cpp: implementation for the NFmiZoomView class.
//
//////////////////////////////////////////////////////////////////////

#include "NFmiFilterGridView.h"
#include "NFmiToolBox.h"
#include "NFmiLine.h"
#include "NFmiRectangle.h"
#include "CtrlViewDocumentInterface.h"

NFmiFilterGridView::NFmiFilterGridView(NFmiToolBox * theToolBox
					 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					 ,const NFmiRect& theRect
					 ,int theIndex)
:NFmiZoomView(0, theToolBox, theDrawParam, theRect)
,itsFilterRelativeAreaRect()
,itsGridXSize(1)
,itsGridYSize(1)
,itsGridXOrigo(0)
,itsGridYOrigo(0)
,itsIndex(theIndex)
,itsFilterAreaRect()
,itsBackgroundColor(0.9f,0.9f,0.9f)
{
	InitGridValues();
}

NFmiFilterGridView::~NFmiFilterGridView()
{
}

void NFmiFilterGridView::InitGridValues(void)
{
    const NFmiRect& ranges = itsCtrlViewDocumentInterface->AreaFilterRangeLimits();
    itsGridXSize = int(ranges.Right() - ranges.Left());
    itsGridYSize = int(ranges.Bottom() - ranges.Top());
    itsGridXOrigo = int(ranges.Left());
    itsGridYOrigo = int(ranges.Top());
    itsFilterAreaRect = itsCtrlViewDocumentInterface->AreaFilterRange(itsIndex);
}

void NFmiFilterGridView::Update(void)
{
}

void NFmiFilterGridView::Draw(NFmiToolBox * theGTB)
{
	InitGridValues();
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	itsDrawingEnvironment.SetPenSize(NFmiPoint(1,1));
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(itsBackgroundColor);
	DrawFrame(itsDrawingEnvironment);
	NFmiRect viewRect(GetFrame());

	int i;
	for(i=0; i<itsGridXSize; i++)
	{
		NFmiPoint startingPoint(i * (viewRect.Width()/itsGridXSize), viewRect.Top());
		NFmiPoint endingPoint(i * (viewRect.Width()/itsGridXSize), viewRect.Bottom());
		NFmiLine line(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&line);
	}
	int j;
	for(j=0; j<itsGridYSize; j++)
	{
		NFmiPoint startingPoint(viewRect.Left(), j * (viewRect.Height()/itsGridYSize));
		NFmiPoint endingPoint(viewRect.Right(), j * (viewRect.Height()/itsGridYSize));
		NFmiLine line(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&line);
	}

// piirretään hilapisteet ruudukkoon
	double sizeX = itsToolBox->SX(4);
	double sizeY = itsToolBox->SY(4);
	NFmiRect gridPointRect(0, 0, sizeX, sizeY);
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(NFmiColor(0,0,0));
	for(j=0; j<=itsGridYSize; j++)
	{
		for(i=0; i<=itsGridXSize; i++)
		{
			NFmiPoint centerOfGrid(i * (viewRect.Width()/itsGridXSize), j * (viewRect.Height()/itsGridYSize));
			gridPointRect.Center(centerOfGrid);
			NFmiRectangle gridPoint(gridPointRect.TopLeft(), gridPointRect.BottomRight(), 0, &itsDrawingEnvironment);
			itsToolBox->Convert(&gridPoint);
		}
	}
// piirretään hilapisteet ruudukkoon


	NFmiRect middleGridPointRect(0, 0, viewRect.Width()/15, viewRect.Height()/15);
	middleGridPointRect.Center(NFmiPoint(0.5, 0.5));
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(NFmiColor(1,0,0));
	NFmiRectangle middlePoint(middleGridPointRect.TopLeft(), middleGridPointRect.BottomRight(), 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&middlePoint);

	NFmiRect filterAreaRect(itsFilterAreaRect);
	
	if(filterAreaRect.Width() == 0 || filterAreaRect.Height() == 0)
		filterAreaRect.Inflate(0.2);
	double x1 = (filterAreaRect.Left() - itsGridXOrigo) * (viewRect.Width()/itsGridXSize);
	double x2 = (filterAreaRect.Right() - itsGridXOrigo) * (viewRect.Width()/itsGridXSize);
	double y1 = (filterAreaRect.Bottom() - itsGridYOrigo) * (viewRect.Height()/itsGridYSize);
	y1 = 1 - y1;
	double y2 = (filterAreaRect.Top() - itsGridYOrigo) * (viewRect.Height()/itsGridYSize);
	y2 = 1 - y2;
	NFmiPoint startingPoint = NFmiPoint(x1,y1);
	NFmiPoint endingPoint = NFmiPoint(x2,y2);
	itsFilterRelativeAreaRect = NFmiRect(startingPoint, endingPoint); // laitetaan relative recti talteen hiiri-tarkasteluja varten
	itsDrawingEnvironment.DisableFill();
	itsDrawingEnvironment.SetPenSize(NFmiPoint(2,2));
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,1));
	NFmiRectangle filterRectangle(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&filterRectangle);
}

bool NFmiFilterGridView::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	NFmiPoint topRight(itsFilterRelativeAreaRect.TopRight());
	double xDiff = fabs(topRight.X()-thePlace.X());
	double yDiff = fabs(topRight.Y()-thePlace.Y());
	if(xDiff < 0.05 && yDiff < 0.05)
	{
		itsDragTarget = kTopRight;
		return true;
	}
	NFmiPoint bottomRight(itsFilterRelativeAreaRect.BottomRight());
	xDiff = fabs(bottomRight.X()-thePlace.X());
	yDiff = fabs(bottomRight.Y()-thePlace.Y());
	if(xDiff < 0.05 && yDiff < 0.05)
	{
		itsDragTarget = kBottomRight;
		return true;
	}
	NFmiPoint bottomLeft(itsFilterRelativeAreaRect.BottomLeft());
	xDiff = fabs(bottomLeft.X()-thePlace.X());
	yDiff = fabs(bottomLeft.Y()-thePlace.Y());
	if(xDiff < 0.05 && yDiff < 0.05)
	{
		itsDragTarget = kBottomLeft;
		return true;
	}
	NFmiPoint topLeft(itsFilterRelativeAreaRect.TopLeft());
	xDiff = fabs(topLeft.X()-thePlace.X());
	yDiff = fabs(topLeft.Y()-thePlace.Y());
	if(xDiff < 0.05 && yDiff < 0.05)
	{
		itsDragTarget = kTopLeft;
		return true;
	}
	if(itsFilterRelativeAreaRect.IsInside(thePlace))
	{
		itsDragTarget = kCenter;
		return true;
	}
	itsDragTarget = kBase; // tarkoittaa, että kursori oli sinisen laatikon ulkopuolella
	return true;
}

bool NFmiFilterGridView::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(theKey & kCtrlKey && theKey & kShiftKey)
	{
		NFmiRect rect(itsCtrlViewDocumentInterface->AreaFilterRangeLimits());
		if(rect.Width() > 3 && rect.Height() > 3)
		{
			rect.Left(rect.Left()+1);
			rect.Right(rect.Right()-1);
			rect.Top(rect.Top()+1);
			rect.Bottom(rect.Bottom()-1);
            itsCtrlViewDocumentInterface->AreaFilterRangeLimits(rect);
		}
	}
	else if(theKey & kCtrlKey)
	{
		NFmiRect filterAreaRect(itsCtrlViewDocumentInterface->AreaFilterRange(itsIndex));
		if(filterAreaRect.Width() >= 1)
			filterAreaRect.Left(filterAreaRect.Left()+1);
		if(filterAreaRect.Width() >= 1)
			filterAreaRect.Right(filterAreaRect.Right()-1);
		if(filterAreaRect.Height() >= 1)
			filterAreaRect.Top(filterAreaRect.Top()+1);
		if(filterAreaRect.Height() >= 1)
			filterAreaRect.Bottom(filterAreaRect.Bottom()-1);
        itsCtrlViewDocumentInterface->AreaFilterRange(itsIndex, filterAreaRect);
	}
	else
	{
		if(itsDragTarget == kBottomRight) // 1999.08.30/Marko
			itsDragTarget = kBase; // 1999.08.30/Marko
		else if(itsDragTarget == kBase)
		{
			MoveFilterAreaRect(thePlace);
			return true;
		}
	}
	return true;
}

bool NFmiFilterGridView::RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey)
{
	if(theKey & kCtrlKey && theKey & kShiftKey)
	{
		NFmiRect rect(itsCtrlViewDocumentInterface->AreaFilterRangeLimits());
		if(rect.Width() < 30 && rect.Height() < 30)
		{
			rect.Left(rect.Left()-1);
			rect.Right(rect.Right()+1);
			rect.Top(rect.Top()-1);
			rect.Bottom(rect.Bottom()+1);
            itsCtrlViewDocumentInterface->AreaFilterRangeLimits(rect);
		}
	}
	else if(theKey & kCtrlKey) // select constant area
	{
		NFmiRect filterAreaRect(itsCtrlViewDocumentInterface->AreaFilterRange(itsIndex));
		filterAreaRect.Left(filterAreaRect.Left()-1);
		filterAreaRect.Right(filterAreaRect.Right()+1);
		filterAreaRect.Top(filterAreaRect.Top()-1);
		filterAreaRect.Bottom(filterAreaRect.Bottom()+1);
        itsCtrlViewDocumentInterface->AreaFilterRange(itsIndex, filterAreaRect);
		return true;
	}
	return false;
}

bool NFmiFilterGridView::MouseMove(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(itsDragTarget == kBottomRight || itsDragTarget == kTopRight || itsDragTarget == kTopLeft || itsDragTarget == kBottomLeft)
	{
		ChangeFilterAreaRect(thePlace);
	}
	else //if(itsDragTarget == kBase)
	{
		MoveFilterAreaRect(thePlace);
	}
	return true;
}

void NFmiFilterGridView::ChangeFilterAreaRect(const NFmiPoint & thePlace)
{
	NFmiRect filterAreaRect(itsCtrlViewDocumentInterface->AreaFilterRange(itsIndex));
	NFmiPoint gridPoint(Relative2FolterGridPoint(thePlace));

	switch (itsDragTarget)
	{
	case kBottomRight: // tämä näyttää säätävän oikeasti oikeaayläkulmaa?!?!?
		filterAreaRect.Top(gridPoint.Y());
		filterAreaRect.Right(gridPoint.X());
		break;
	case kTopRight: // tämä näyttää säätävän oikeasti oikeaaalakulmaa?!?!?
		filterAreaRect.Bottom(gridPoint.Y());
		filterAreaRect.Right(gridPoint.X());
		break;
	case kTopLeft: // tämä näyttää säätävän oikeasti oikeaaalakulmaa?!?!?
		filterAreaRect.Bottom(gridPoint.Y());
		filterAreaRect.Left(gridPoint.X());
		break;
	case kBottomLeft: // tämä näyttää säätävän oikeasti oikeaaalakulmaa?!?!?
		filterAreaRect.Top(gridPoint.Y());
		filterAreaRect.Left(gridPoint.X());
		break;
	}
	AdjustFilterGridRect(filterAreaRect, true);
    itsCtrlViewDocumentInterface->AreaFilterRange(itsIndex, filterAreaRect);
}

void NFmiFilterGridView::MoveFilterAreaRect(const NFmiPoint & thePlace)
{
	NFmiRect filterAreaRect(itsCtrlViewDocumentInterface->AreaFilterRange(itsIndex));
	NFmiPoint gridPoint(Relative2FolterGridPoint(thePlace));
	filterAreaRect.Center(gridPoint);
	AdjustFilterGridRect(filterAreaRect, true);
    itsCtrlViewDocumentInterface->AreaFilterRange(itsIndex, filterAreaRect);
}

NFmiPoint NFmiFilterGridView::Relative2FolterGridPoint(const NFmiPoint& thrRelativePoint)
{
	NFmiRect relativeRect(GetFrame());
	NFmiPoint returnPoint(thrRelativePoint.X() * itsGridXSize / relativeRect.Width(),
						  (relativeRect.Height() - thrRelativePoint.Y()) * itsGridYSize / relativeRect.Height());
	returnPoint.X(returnPoint.X() + itsGridXOrigo);
	returnPoint.Y(returnPoint.Y() + itsGridYOrigo);
	return returnPoint;
}

// katsoo, että annettu rect:i on hilanäytön sisällä ja tarpeen mukaan muokkaa/siirtää rect:iä
void NFmiFilterGridView::AdjustFilterGridRect(NFmiRect& theRect, bool fKeepSize, FmiDirection theSolidCorner)
{
	if(fKeepSize)
	{
		NFmiPoint place(theRect.Place());
		place.X(round(place.X()));
		place.Y(round(place.Y()));
		NFmiPoint size(theRect.Size());
		size.X(round(size.X()));
		size.Y(round(size.Y()));
		theRect.Place(place);
		theRect.Size(size);

		const NFmiRect& ranges = itsCtrlViewDocumentInterface->AreaFilterRangeLimits();
		double xMovement = 0;
		double yMovement = 0;
		if(theRect.Left() < ranges.Left())
			xMovement = ranges.Left() - theRect.Left();
		else if(theRect.Right() > ranges.Right())
			xMovement = ranges.Right() - theRect.Right();

		if(theRect.Top() < ranges.Top())
			yMovement = ranges.Top() - theRect.Top();
		else if(theRect.Bottom() > ranges.Bottom())
			yMovement = ranges.Bottom() - theRect.Bottom();

		NFmiPoint centerPoint(theRect.Center());
		centerPoint.X(centerPoint.X() + xMovement);
		centerPoint.Y(centerPoint.Y() + yMovement);
		theRect.Center(centerPoint);
	}
}

