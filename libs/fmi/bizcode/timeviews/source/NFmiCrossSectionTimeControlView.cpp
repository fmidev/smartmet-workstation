#include "NFmiCrossSectionTimeControlView.h"

#include "NFmiDrawParam.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiCrossSectionSystem.h"
#include "GraphicalInfo.h"

NFmiCrossSectionTimeControlView::NFmiCrossSectionTimeControlView(int theMapViewDescTopIndex, const NFmiRect& theRect
											,NFmiToolBox *theToolBox
											,boost::shared_ptr<NFmiDrawParam> &theDrawParam
											,bool theShowResolutionChangerBox
											,bool theDrawViewInMouseMove
											,bool theShowEditTimeHandles
											,bool theShowSelectedTimes
											,double theAxisShrinkFactor)

:NFmiTimeControlView(theMapViewDescTopIndex, theRect
					,theToolBox
					,theDrawParam
					,theShowResolutionChangerBox
					,theDrawViewInMouseMove
					,theShowEditTimeHandles
					,theShowSelectedTimes
					,true
					,theAxisShrinkFactor)
{
}

NFmiCrossSectionTimeControlView::~NFmiCrossSectionTimeControlView(void)
{
}

NFmiPoint NFmiCrossSectionTimeControlView::GetViewSizeInPixels(void)
{
	return itsCtrlViewDocumentInterface->CrossSectionViewSizeInPixels();
}

const NFmiTimeBag& NFmiCrossSectionTimeControlView::GetUsedTimeBag(void)
{
	return itsCtrlViewDocumentInterface->CrossSectionSystem()->CrossSectionTimeControlTimeBag();
}

void NFmiCrossSectionTimeControlView::SetUsedTimeBag(const NFmiTimeBag &newTimeBag)
{
    itsCtrlViewDocumentInterface->CrossSectionSystem()->CrossSectionTimeControlTimeBag(newTimeBag);
}

bool NFmiCrossSectionTimeControlView::IsTimeFiltersDrawn(void)
{
	return false; // ei piirretä ollenkaan aikajanaa
}

bool NFmiCrossSectionTimeControlView::LeftButtonUp(const NFmiPoint & thePlace,unsigned long theKey)
{
	bool status = NFmiTimeControlView::LeftButtonUp(thePlace, theKey);
    itsCtrlViewDocumentInterface->MouseCapturedInTimeWindow(false);
	return status;
}

CtrlViewUtils::GraphicalInfo& NFmiCrossSectionTimeControlView::GetGraphicalInfo(void)
{
	return itsCtrlViewDocumentInterface->CrossSectionSystem()->GetGraphicalInfo();
}

