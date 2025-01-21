//© Ilmatieteenlaitos/ Persa  
//  Original 28.04.1997
// 
// 
//                                  
//Ver. 28.04.1997/Persa
//-------------------------------------------------------------- nareavie.cpp


#include "NFmiAreaView.h"
#include "NFmiArea.h"

//______________________________________________________________ NFmiAreaView

NFmiAreaView :: NFmiAreaView(int theMapViewDescTopIndex, const boost::shared_ptr<NFmiArea> &theArea
							, NFmiToolBox *theToolBox
							, boost::shared_ptr<NFmiDrawParam> &theDrawParam
                            , int viewGridRowNumber
                            , int viewGridColumnNumber)
:NFmiCtrlView(theMapViewDescTopIndex
			 , theArea ? theArea->XYArea() : NFmiRect(0,0,1,1)
			 , theToolBox
			 , theDrawParam
             , viewGridRowNumber
             , viewGridColumnNumber)
{
	itsArea = theArea;
}

//----------------------------------------------------------------------

NFmiAreaView :: NFmiAreaView(const NFmiAreaView &theView )
:NFmiCtrlView(theView)			 
{
	itsArea = theView.itsArea;
}
//----------------------------------------------------------------------

NFmiAreaView::NFmiAreaView() = default;

NFmiAreaView::~NFmiAreaView() = default;

boost::shared_ptr<NFmiArea> NFmiAreaView::GetArea() const
{
	return itsArea;
}

void NFmiAreaView::SetArea(const boost::shared_ptr<NFmiArea> &theArea)
{
	if(theArea)
	{
		itsArea = theArea;
		itsRect = theArea->XYArea();
	}
}
