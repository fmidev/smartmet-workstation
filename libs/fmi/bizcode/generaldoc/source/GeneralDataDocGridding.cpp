#include "GeneralDataDocGridding.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "CombinedMapHandlerInterface.h"
#include "NFmiApplicationWinRegistry.h"

void GeneralDataDocGridding::MakeDrawedInfoVectorForMapView(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea)
{
    itsDoc->GetCombinedMapHandler()->makeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea);
}

NFmiIgnoreStationsData& GeneralDataDocGridding::IgnoreStationsData()
{
    return itsDoc->IgnoreStationsData();
}

const NFmiGriddingProperties& GeneralDataDocGridding::GriddingProperties(bool getEditingRelatedProperties)
{
    return itsDoc->ApplicationWinRegistry().GriddingProperties(getEditingRelatedProperties);
}

