#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>

class NFmiFastQueryInfo;
class NFmiDrawParam;
class NFmiArea;
class NFmiIgnoreStationsData;
class NFmiGriddingProperties;

enum FmiGriddingFunction
{
  kFmiMarkoGriddingFunction = 0,
  kFmiXuGriddingFastLocalFitCalc = 1,
  kFmiXuGriddingLocalFitCalc = 2,
  kFmiXuGriddingTriangulationCalc = 3,
  kFmiXuGriddingLeastSquaresCalc = 4,
  kFmiXuGriddingThinPlateSplineCalc = 5,
  kFmiErrorGriddingFunction  // virhetilanteita varten
};

class NFmiGriddingHelperInterface
{
 public:
  virtual ~NFmiGriddingHelperInterface();
  virtual void MakeDrawedInfoVectorForMapView(
      std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector,
      boost::shared_ptr<NFmiDrawParam> &theDrawParam,
      const boost::shared_ptr<NFmiArea> &theArea) = 0;
  virtual NFmiIgnoreStationsData &IgnoreStationsData() = 0;
  virtual const NFmiGriddingProperties &GriddingProperties(bool getEditingRelatedProperties) = 0;
};
