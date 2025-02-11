#include "NFmiGridViewSelectorView.h"
#include "NFmiToolBox.h"
#include "NFmiRectangle.h"
#include "NFmiLine.h"
#include "CtrlViewDocumentInterface.h"

NFmiGridViewSelectorView::NFmiGridViewSelectorView(int theMapViewDescTopIndex, NFmiToolBox * theToolBox
												 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
												 ,const NFmiRect& theRect
												 ,int rows
												 ,int columns)
:NFmiZoomView(theMapViewDescTopIndex, theToolBox, theDrawParam, theRect)
,itsSelectedGridViewSize()
,itsGridViewRows(rows)
,itsGridViewColumns(columns) 
{
    itsSelectedGridViewSize = itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex);
}

void NFmiGridViewSelectorView::SetViewGridSize(int rows, int columns)
{
	itsGridViewRows = rows;
	itsGridViewColumns = columns; 
}

bool NFmiGridViewSelectorView::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	itsSelectedGridViewSize = Relative2GridPoint(thePlace);
	Draw();
	return false;
}

void NFmiGridViewSelectorView::Draw(NFmiToolBox * theGTB)
{
	Draw();
}

void NFmiGridViewSelectorView::Draw(void)
{
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	itsDrawingEnvironment.SetPenSize(NFmiPoint(1,1));
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(NFmiColor(0.9f,0.9f,0.9f));
	DrawFrame(itsDrawingEnvironment);

	NFmiRect viewRect(GetFrame());
	NFmiRect selectedRect(viewRect);
	NFmiPoint selectedSize(selectedRect.Size());
	selectedSize.X(selectedSize.X() * itsSelectedGridViewSize.X()/itsGridViewColumns);
	selectedSize.Y(selectedSize.Y() * itsSelectedGridViewSize.Y()/itsGridViewRows);
	selectedRect.Size(selectedSize);
	itsDrawingEnvironment.SetFillColor(NFmiColor(0.f,0.9f,0.f));
	itsDrawingEnvironment.EnableFill();
	NFmiRectangle selectedRectangle(selectedRect, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&selectedRectangle);

	for(int i=0; i<itsGridViewColumns; i++)
	{
		NFmiPoint startingPoint(i * (viewRect.Width()/itsGridViewColumns), viewRect.Top());
		NFmiPoint endingPoint(i * (viewRect.Width()/itsGridViewColumns), viewRect.Bottom());
		NFmiLine line(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&line);
	}
	for(int j=0; j<itsGridViewRows; j++)
	{
		NFmiPoint startingPoint(viewRect.Left(), j * (viewRect.Height()/itsGridViewRows));
		NFmiPoint endingPoint(viewRect.Right(), j * (viewRect.Height()/itsGridViewRows));
		NFmiLine line(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&line);
	}
}

NFmiPoint NFmiGridViewSelectorView::Relative2GridPoint(const NFmiPoint& theRelativePlace)
{
	NFmiRect viewRect(GetFrame());
	NFmiPoint gridPoint(int((theRelativePlace.X() - viewRect.Left())/viewRect.Width() * itsGridViewColumns) + 1
			, int((theRelativePlace.Y() - viewRect.Top())/viewRect.Height() * itsGridViewRows) + 1);
	return gridPoint;
}

