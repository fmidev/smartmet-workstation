//© Ilmatieteenlaitos/software by Marko
//  Original 5.05.2004
// 
// Luokka piirtää poikkileikkaus-näytön aikaleikkaus moodissa
// aika-akselia, jossa myös poikkileikkauksen alku/loppuaikojen säätimet. 
//-------------------------------------------------------------------- NFmiCrossSectionTimeControlView.h

#pragma once

//_________________________________________________________ NFmiCrossSectionTimeControlView

#include "NFmiTimeControlView.h"

namespace CtrlViewUtils
{
    struct GraphicalInfo;
}

class NFmiCrossSectionTimeControlView : public NFmiTimeControlView
{

public:
	NFmiCrossSectionTimeControlView(int theMapViewDescTopIndex, const NFmiRect & theRect
						,NFmiToolBox * theToolBox
						,boost::shared_ptr<NFmiDrawParam> &theDrawParam
						,bool theShowResolutionChangerBox
						,bool theDrawViewInMouseMove
						,bool theShowEditTimeHandles
						,bool theShowSelectedTimes
						,double theAxisShrinkFactor);
	virtual  ~NFmiCrossSectionTimeControlView(void);

	bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
    CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(void);

protected:
	NFmiPoint GetViewSizeInPixels(void);
	const NFmiTimeBag& GetUsedTimeBag(void);
	void SetUsedTimeBag(const NFmiTimeBag &newTimeBag);
	bool IsTimeFiltersDrawn(void);

private:

};

