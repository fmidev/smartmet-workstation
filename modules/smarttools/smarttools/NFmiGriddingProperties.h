#pragma once

#include "NFmiGriddingHelperInterface.h"

class NFmiGriddingProperties
{
  bool toolMasterAvailable_;
  // Which ToolMaster function is in use, options are: FastLocalFitCalc, LocalFitCalc,
  // TriangulationCalc, LeastSquaresCalc, ThinPlateSplineCalc
  FmiGriddingFunction function_ = kFmiXuGriddingFastLocalFitCalc;
  // To how far grid points is CP value allowed to affect [km]
  double rangeLimitInKm_ = 0.;
  // Interpolation method: default is XuWEIGHTED_AVERAGE 1, other options: XuBILINEAR 2,
  // XuBILINEAR_STRICT 3, XuWEIGHTED_AVERAGE_2Q 4, XuWEIGHTED_AVERAGE_3Q 5, XuWEIGHTED_AVERAGE_4Q 6
  int localFitMethod_ = 1;
  // If grid-point is sufficently close enough to CP point, that grid-point's value will be set to
  // that CP's value. I don't know the unit, but the default is 0.5 and I don't understand it.
  double localFitDelta_ = 0.5;
  // How many times is grid recalculated for smoother results.
  // Default is 0, which means that calculations are done only the initial time.
  // All the extra smooth calcualtion rounds off course slow operations down.
  int smoothLevel_ = 0;
  // Quadratic interpolation factor for LocalFit -calculations, default is 1.25, should be > 1.0.
  double localFitFilterRadius_ = 1.25;
  // Quadratic interpolation factor for LocalFit -calculations, default is 0.15, should be > 0.0.
  double localFitFilterFactor_ = 0.15;

 public:
  NFmiGriddingProperties(bool toolMasterAvailable = false);

  std::string toString() const;
  bool fromString(const std::string &str);
  bool toolMasterAvailable() const { return toolMasterAvailable_; }
  void toolMasterAvailable(bool toolMasterAvailable) { toolMasterAvailable_ = toolMasterAvailable; }
  FmiGriddingFunction function() const;
  void function(FmiGriddingFunction function) { function_ = function; }
  double rangeLimitInKm() const { return rangeLimitInKm_; }
  void rangeLimitInKm(double rangeLimitInKm) { rangeLimitInKm_ = rangeLimitInKm; }
  int localFitMethod() const { return localFitMethod_; }
  void localFitMethod(int localFitMethod) { localFitMethod_ = localFitMethod; }
  double localFitDelta() const { return localFitDelta_; }
  void localFitDelta(double localFitDelta) { localFitDelta_ = localFitDelta; }
  int smoothLevel() const { return smoothLevel_; }
  void smoothLevel(int smoothLevel) { smoothLevel_ = smoothLevel; }
  double localFitFilterRadius() const { return localFitFilterRadius_; }
  void localFitFilterRadius(double localFitFilterRadius)
  {
    localFitFilterRadius_ = localFitFilterRadius;
  }
  double localFitFilterFactor() const { return localFitFilterFactor_; }
  void localFitFilterFactor(double localFitFilterFactor)
  {
    localFitFilterFactor_ = localFitFilterFactor;
  }

  static double ConvertLengthInKmToRelative(double lengthInKm, const NFmiArea *area);
};
