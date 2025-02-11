// NFmiMovingCPLocationView.cpp: implementation for the NFmiMovingCPLocationView class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiMovingCPLocationView.h"
#include "NFmiToolBox.h"
#include "NFmiRectangle.h"
#include "BSPLINE.h"
#include "NFmiEditorControlPointManager.h"


NFmiMovingCPLocationView::NFmiMovingCPLocationView(NFmiToolBox * theToolBox
												 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
												 ,const NFmiRect& theRect
												 ,int theIndex
												 ,NFmiEditorControlPointManager* theCPManager)
:NFmiFilterGridView(theToolBox,theDrawParam,theRect,theIndex)
,itsCPManager(theCPManager)
,itsSplineStart(0.5,0.5)
,itsSplineMiddle(0.5,0.5)
,itsSplineEnd(0.5,0.5)
,itsLocationVector()
{
	if(itsCPManager && itsCPManager->IsCPMovingInTime())
	{
		NFmiEditorControlPointManager::ThreePoints points(itsCPManager->CPMovingInTimeHelpPoints());
		itsSplineStart = points.itsStartPoint;
		itsSplineMiddle = points.itsMiddlePoint;
		itsSplineEnd = points.itsEndPoint;
	}
}

NFmiMovingCPLocationView::~NFmiMovingCPLocationView()
{
}

void NFmiMovingCPLocationView::Draw(NFmiToolBox * theGTB)
{
	itsBackgroundColor = NFmiColor(0.8f,0.84f,0.99f);
	NFmiFilterGridView::Draw(theGTB);

	DrawSplineData();

	itsDrawingEnvironment.SetPenSize(NFmiPoint(3,3));
	NFmiRect ellipseRect(0,0,0.08,0.08);
	ellipseRect.Center(itsSplineStart);
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	itsToolBox->DrawEllipse(ellipseRect, &itsDrawingEnvironment);

	ellipseRect.Center(itsSplineMiddle);
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.5f,0.5f,0.5f));
	itsToolBox->DrawEllipse(ellipseRect, &itsDrawingEnvironment);

	ellipseRect.Center(itsSplineEnd);
	itsDrawingEnvironment.SetFrameColor(NFmiColor(1.f,1.f,1.f));
	itsToolBox->DrawEllipse(ellipseRect, &itsDrawingEnvironment);

}

void NFmiMovingCPLocationView::InitGridValues(void)
{
	NFmiFilterGridView::InitGridValues();
	itsGridXSize = 18;
	itsGridYSize = 18;
	itsGridXOrigo = 0;
	itsGridYOrigo = 0;
	itsFilterAreaRect = NFmiRect(0,0,0,0);
}

bool NFmiMovingCPLocationView::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	return false;
}

bool NFmiMovingCPLocationView::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	itsSplineMiddle = thePlace;
	return true;
}

bool NFmiMovingCPLocationView::RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey)
{
	itsSplineEnd = thePlace;
	return true;
}

bool NFmiMovingCPLocationView::MouseMove(const NFmiPoint & thePlace, unsigned long theKey)
{
	return false;
}

// TÄMÄ on kammottavaa koodia, onneksi kukaan ei kai oikein käytä 
// liikkuvia kontrollipisteitä editorissa.
void NFmiMovingCPLocationView::DrawSplineData(void)
{
	int n=2;          // number of control points = n+1
	int t=3;           // degree of polynomial = t-1

	point *pts;          // allocate our control point array
	pts=new point[n+1];

	pts[0].x=0.5;  pts[0].y=0.5;  pts[0].z=0;
	
	pts[1].x=itsSplineMiddle.X();  
	pts[1].y=itsSplineMiddle.Y();  
	pts[1].z=0;
	pts[2].x=itsSplineEnd.X();  
	pts[2].y=itsSplineEnd.Y();  
	pts[2].z=0;

	int resolution = itsCPManager->TimeDescriptor().Size();  // how many points our in our output array
	point *out_pts;
	out_pts = new point[resolution];

	bspline(n, t, pts, out_pts, resolution);
	itsDrawingEnvironment.SetPenSize(NFmiPoint(1,1));
	float grayShadeDelta  = 1.f/(resolution-1);
	float currentGrayShade  = 0.f;

	NFmiRect CPRect(NFmiPoint(0, 0), NFmiPoint(0.04, 0.04));
	int i;
	for(i=0; i<resolution; i++)
	{
		itsDrawingEnvironment.SetFrameColor(NFmiColor(currentGrayShade, currentGrayShade, currentGrayShade));
		NFmiPoint xy(out_pts[i].x, out_pts[i].y);
		CPRect.Center(xy);
		NFmiRectangle rectangle(CPRect, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&rectangle);
		currentGrayShade += grayShadeDelta;
	}
	delete [] pts;
// TÄYTÄ LOCATION VECTOR!!!!!!
	itsLocationVector.resize(resolution);
	for(i = 0; i < resolution; i++)
		itsLocationVector[i] = NFmiPoint(out_pts[i].x, out_pts[i].y);
	delete [] out_pts;
}

// laskee omasta location vektoristaan suhteellisen location vectorin
// lasketaan 0,0 - 1,1 avaruudessa datan hilapisteiden ja itsGridXSize:n
// ja itsGridYSize avulla

std::vector<NFmiPoint> NFmiMovingCPLocationView::GetRelativeLocationVector(int xDataCount, int yDataCount)
{
	size_t size = itsLocationVector.size();
	if(size <= 0)
		return std::vector<NFmiPoint>();
	std::vector<NFmiPoint> relativeLocationVector(size);
	NFmiPoint startingRelativePoint(itsCPManager->StartingRelativeLocation());
	relativeLocationVector[0] = startingRelativePoint;
	for(size_t i = 1; i < size; i++)
	{
		double xRelLoc = (-itsSplineStart.X() + itsLocationVector[i].X()) * ((xDataCount - itsGridXSize)/double(xDataCount));
		double yRelLoc = -(-itsSplineStart.Y() + itsLocationVector[i].Y()) * ((yDataCount - itsGridYSize)/double(yDataCount));
		NFmiPoint curPoint(startingRelativePoint.X() + xRelLoc, startingRelativePoint.Y() + yRelLoc);
		relativeLocationVector[i] = curPoint;
	}
	return relativeLocationVector;
}

const NFmiPoint& NFmiMovingCPLocationView::SplineStart(void)
{
	return itsSplineStart;
}
const NFmiPoint& NFmiMovingCPLocationView::SplineMiddle(void)
{
	return itsSplineMiddle;
}
const NFmiPoint& NFmiMovingCPLocationView::SplineEnd(void)
{
	return itsSplineEnd;
}
