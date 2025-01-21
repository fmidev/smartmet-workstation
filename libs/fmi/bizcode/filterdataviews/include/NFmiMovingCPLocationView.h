// NFmiMovingCPLocationView.h: interface for the NFmiZoomView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiFilterGridView.h"

class NFmiEditorControlPointManager;

class NFmiMovingCPLocationView : public NFmiFilterGridView
{

public:
	bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool MouseMove (const NFmiPoint & thePlace, unsigned long theKey) override;
	void Draw(NFmiToolBox * theGTB) override;
	NFmiMovingCPLocationView(NFmiToolBox * theToolBox
					 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					 ,const NFmiRect& theRect
					 ,int theIndex
					 ,NFmiEditorControlPointManager* theCPManager);
	~NFmiMovingCPLocationView();
	std::vector<NFmiPoint> GetRelativeLocationVector(int xDataCount, int yDataCount);

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
	std::vector<NFmiPoint> itsLocationVector;
};

