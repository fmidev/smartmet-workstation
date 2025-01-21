//© Ilmatieteenlaitos/Marko
//  Original 24.09.1998
//
//
//Ver. xx.xx.xxxx/Marko
//
//-------------------------------------------------------------------- NFmiStationTextView.cpp

#include "NFmiStationTextView.h"
#include "NFmiToolBox.h"
#include "NFmiDrawParam.h"
#include "CtrlViewDocumentInterface.h"
#include "boost\math\special_functions\round.hpp"


NFmiStationTextView::NFmiStationTextView (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
								 ,NFmiToolBox * theToolBox
								 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								 ,FmiParameterName theParamId
								 ,NFmiPoint theOffSet
								 ,NFmiPoint theSize
								 ,int theRowIndex
                                 ,int theColumnIndex)
:NFmiStationView(theMapViewDescTopIndex, theArea
				,theToolBox
				,theDrawParam
				,theParamId
				,theOffSet
				,theSize
				,theRowIndex
                ,theColumnIndex)
{
}

NFmiStationTextView::~NFmiStationTextView(void)
{
}

void NFmiStationTextView::ModifyTextEnvironment(void)
{
	itsDrawingEnvironment.SetFrameColor(itsDrawParam->FrameColor());
	itsDrawingEnvironment.DisableFill();
    itsDrawingEnvironment.SetFontSize(CalcFontSize(12, boost::math::iround(MaximumFontSizeFactor() * 45), itsCtrlViewDocumentInterface->Printing()));
}

// n‰ill‰ kertoimilla viel‰ tehd‰‰n viimeistely
NFmiPoint NFmiStationTextView::GetSpaceOutFontFactor(void)
{
	if(itsDrawingEnvironment.GetFontType() == kSynop)
		return NFmiPoint(0.4, 0.6);
	return NFmiStationView::GetSpaceOutFontFactor();
}

NFmiPoint NFmiStationTextView::SbdCalcFixedSymbolSize() const
{
	return SbdBasicSymbolSizeCalculation(12, 45);
}
