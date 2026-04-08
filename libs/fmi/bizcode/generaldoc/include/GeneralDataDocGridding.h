#include "NFmiGriddingHelperInterface.h"

class NFmiEditMapGeneralDataDoc;

class GeneralDataDocGridding : public NFmiGriddingHelperInterface
{
    NFmiEditMapGeneralDataDoc *itsDoc;
public:
    GeneralDataDocGridding(NFmiEditMapGeneralDataDoc *theDoc)
        :itsDoc(theDoc)
    {}

    void MakeDrawedInfoVectorForMapView(std::vector<std::shared_ptr<NFmiFastQueryInfo> >& theInfoVector, std::shared_ptr<NFmiDrawParam>& theDrawParam, const std::shared_ptr<NFmiArea>& theArea) override;
    NFmiIgnoreStationsData& IgnoreStationsData() override;
    const NFmiGriddingProperties& GriddingProperties(bool getEditingRelatedProperties) override;

};
