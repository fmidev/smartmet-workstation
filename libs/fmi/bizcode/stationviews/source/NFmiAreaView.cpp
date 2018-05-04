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
							, NFmiDrawingEnvironment* theDrawingEnvi
							, boost::shared_ptr<NFmiDrawParam> &theDrawParam
                            , int viewGridRowNumber
                            , int viewGridColumnNumber)
:NFmiCtrlView(theMapViewDescTopIndex
			 , theArea ? theArea->XYArea() : NFmiRect(0,0,1,1)
			 , theToolBox
			 , theDrawingEnvi
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

NFmiAreaView :: ~NFmiAreaView( void )
{
} 

void NFmiAreaView::Area(const boost::shared_ptr<NFmiArea> &theArea)
{
	if(theArea)
	{
		itsArea = theArea;
		itsRect = theArea->XYArea(); // pitääkö tämä tehdä 1999.09.10/Marko
	}
}
