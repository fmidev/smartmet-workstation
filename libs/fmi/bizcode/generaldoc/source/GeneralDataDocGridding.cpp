#include "GeneralDataDocGridding.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "CombinedMapHandlerInterface.h"
#ifndef UNIX
#include "NFmiApplicationWinRegistry.h"
#else
#include <stdexcept>
#endif

void GeneralDataDocGridding::MakeDrawedInfoVectorForMapView(std::vector<std::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, std::shared_ptr<NFmiDrawParam> &theDrawParam, const std::shared_ptr<NFmiArea> &theArea)
{
    itsDoc->GetCombinedMapHandler()->makeDrawedInfoVectorForMapView(theInfoVector, theDrawParam, theArea);
}

NFmiIgnoreStationsData& GeneralDataDocGridding::IgnoreStationsData()
{
    return itsDoc->IgnoreStationsData();
}

const NFmiGriddingProperties& GeneralDataDocGridding::GriddingProperties(bool getEditingRelatedProperties)
{
#ifndef UNIX
    return itsDoc->ApplicationWinRegistry().GriddingProperties(getEditingRelatedProperties);
#else
    throw std::runtime_error("GeneralDataDocGridding::GriddingProperties not implemented on Linux");
#endif
}

