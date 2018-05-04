#pragma once

#include <newbase/NFmiProducer.h>
#include <newbase/NFmiLevelType.h>
#include <newbase/NFmiDataMatrix.h>

#include "boost/shared_ptr.hpp"

class NFmiFastQueryInfo;
class NFmiInfoOrganizer;
class NFmiArea;

// Kun smarttool:ia tulkitaan, siin‰ saattaa olla osia, joita voi k‰ytt‰‰ vain macroParamien
// yhteydess‰.
// T‰h‰n luokkaan talletetaan kaikkea, mit‰ extra tietoa voi lˆyty‰ annetusta skriptist‰.
class NFmiExtraMacroParamData
{
 public:
  NFmiExtraMacroParamData();

  void FinalizeData(NFmiInfoOrganizer &theInfoOrganizer);
  bool UseSpecialResolution() const;
  static void SetUsedAreaForData(boost::shared_ptr<NFmiFastQueryInfo> &theData,
                                 const NFmiArea *theUsedArea);

  bool UseEditedDataForResolution() const { return fUseEditedDataForResolution; }
  void UseEditedDataForResolution(bool newValue) { fUseEditedDataForResolution = newValue; }
  float GivenResolutionInKm() const { return itsGivenResolutionInKm; }
  void GivenResolutionInKm(float newValue) { itsGivenResolutionInKm = newValue; }
  const NFmiProducer &Producer() const { return itsProducer; }
  void Producer(const NFmiProducer &newValue) { itsProducer = newValue; }
  FmiLevelType LevelType() const { return itsLevelType; }
  void LevelType(FmiLevelType newValue) { itsLevelType = newValue; }
  boost::shared_ptr<NFmiFastQueryInfo> ResolutionMacroParamData()
  {
    return itsResolutionMacroParamData;
  }
  static void AdjustValueMatrixToMissing(const boost::shared_ptr<NFmiFastQueryInfo> &theData,
                                         NFmiDataMatrix<float> &theValueMatrix);

  void AddCalculationPoint(const NFmiPoint &latlon) { itsCalculationPoints.push_back(latlon); }
  const std::vector<NFmiPoint> &CalculationPoints() const { return itsCalculationPoints; }
  bool UseCalculationPoints() const { return !itsCalculationPoints.empty(); }
  const NFmiProducer &CalculationPointProducer() const { return itsCalculationPointProducer; }
  void CalculationPointProducer(const NFmiProducer &theProducer)
  {
    itsCalculationPointProducer = theProducer;
  }

  float ObservationRadiusInKm() const { return itsObservationRadiusInKm; }
  void ObservationRadiusInKm(float newValue) { itsObservationRadiusInKm = newValue; }
  float ObservationRadiusRelative() const { return itsObservationRadiusRelative; }
  void ObservationRadiusRelative(float newValue) { itsObservationRadiusRelative = newValue; }
 private:
  void InitializeResolutionWithEditedData(NFmiInfoOrganizer &theInfoOrganizer);
  void InitializeResolutionData(NFmiInfoOrganizer &theInfoOrganizer, float usedResolutionInKm);
  void InitializeDataBasedResolutionData(NFmiInfoOrganizer &theInfoOrganizer,
                                         const NFmiProducer &theProducer,
                                         FmiLevelType theLevelType);
  void UseDataForResolutionCalculations(NFmiInfoOrganizer &theInfoOrganizer,
                                        boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
  void InitializeRelativeObservationRange(NFmiInfoOrganizer &theInfoOrganizer, float usedRangeInKm);
  void AddCalculationPointsFromData(NFmiInfoOrganizer &theInfoOrganizer,
                                    const NFmiProducer &theProducer);

  // Jos skriptiss‰ on resolution = edited, k‰ytet‰‰n editoitua dataa resoluutio laskuissa.
  bool fUseEditedDataForResolution;
  // Jos skriptiss‰ on annettu haluttu laskenta resoluutio tyyliin "resolution = 12.5", talletetaan
  // kyseinen luku t‰h‰n.
  // Jos ei ole asetettu, on arvo missing.
  float itsGivenResolutionInKm;
  // Jos skriptiss‰ on annettu resoluutio data muodossa tyyliin "resolution = hir_pressure",
  // talletetaan lausekkeen tuottaja ja level tyyppi n‰ihin kahteen arvoon.
  NFmiProducer itsProducer;   // puuttuva arvo on kun id = 0
  FmiLevelType itsLevelType;  // puuttuva arvo on kFmiNoLevelType eli 0

  // t‰h‰n lasketaan datasta haluttu resoluutio  makro-parametrien laskuja varten pit‰‰ pit‰‰ yll‰
  // yhden
  // hilan kokoista dataa  (yksi aika,param ja level, editoitavan datan hplaceDesc). T‰h‰n
  // dataan on laskettu haluttu resoluutio t‰m‰n macroParamin laskujen ajaksi.
  float itsDataBasedResolutionInKm;
  boost::shared_ptr<NFmiFastQueryInfo> itsResolutionMacroParamData;

  // CalculationPoint listassa on pisteet jos niit‰ on annettu "CalculationPoint = lat,lon"
  // -lausekkeilla.
  // T‰m‰ muuttaa macroParamin laskuja niin ett‰ lopullinen laskettu matriisi nollataan muiden
  // pisteiden
  // kohdalta paitsi n‰iden pisteiden l‰himmiss‰ hilapisteiss‰.
  std::vector<NFmiPoint> itsCalculationPoints;
  // Jos skriptiss‰ on annettu CalculationPoint on muodossa "CalculationPoint = synop",
  // talletetaan lausekkeen tuottaja t‰nne ja kyseisen datan asemat lis‰t‰‰n itsCalculationPoints
  // -listaan.
  NFmiProducer itsCalculationPointProducer;  // puuttuva arvo on kun id = 0

  // Jos halutaan ett‰ havaintojen k‰yttˆ‰ laskuissa rajoitetaan laskentas‰teell‰, annetaan se
  // t‰h‰n kilometreiss‰. Jos t‰m‰ on kFloatMissing, k‰ytet‰‰n laskuissa havaintoja rajattomasti.
  float itsObservationRadiusInKm;
  // T‰h‰n lasketaan k‰ytetyn kartta-alueen mukainen relatiivinen et‰isyys (jota k‰ytet‰‰n itse
  // laskuissa)
  float itsObservationRadiusRelative;
};
