// NFmiMovingCPLocationView.h: interface for the NFmiZoomView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiFilterGridView.h"
#include "NFmiDataMatrix.h"

class NFmiEditorControlPointManager;

class NFmiMovingCPLocationView : public NFmiFilterGridView
{

public:
	bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey);
	bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
	bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
	bool MouseMove (const NFmiPoint & thePlace, unsigned long theKey);
	void Draw(NFmiToolBox * theGTB);
	NFmiMovingCPLocationView(NFmiToolBox * theToolBox
					 ,NFmiDrawingEnvironment * theDrawingEnvi
					 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					 ,const NFmiRect& theRect
					 ,int theIndex
					 ,NFmiEditorControlPointManager* theCPManager);
	virtual  ~NFmiMovingCPLocationView();
	checkedVector<NFmiPoint> GetRelativeLocationVector(int xDataCount, int yDataCount);

	const NFmiPoint& SplineStart(void);
	const NFmiPoint& SplineMiddle(void);
	const NFmiPoint& SplineEnd(void);

protected:
	virtual void InitGridValues(void);
	void DrawSplineData(void);

	NFmiEditorControlPointManager* itsCPManager; // ei omista
	NFmiPoint itsSplineStart;
	NFmiPoint itsSplineMiddle;
	NFmiPoint itsSplineEnd;
	checkedVector<NFmiPoint> itsLocationVector;
};

