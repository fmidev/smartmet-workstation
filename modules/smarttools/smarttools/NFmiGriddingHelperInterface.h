#pragma once

#include <newbase/NFmiDataMatrix.h>
#include <boost/shared_ptr.hpp>

class NFmiFastQueryInfo;
class NFmiDrawParam;
class NFmiArea;
class NFmiIgnoreStationsData;

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
      checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector,
      boost::shared_ptr<NFmiDrawParam> &theDrawParam,
      const boost::shared_ptr<NFmiArea> &theArea) = 0;
  virtual NFmiIgnoreStationsData &IgnoreStationsData() = 0;
  virtual FmiGriddingFunction GriddingFunction() = 0;
};
