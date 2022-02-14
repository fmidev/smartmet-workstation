// ======================================================================
/*!
 * \file NFmiDataModifierCircularExtreme.h
 * \brief abstrakti luokka tuulen suunnan ja erilaisten aaltojen suunta  
 *  parametrien min/max:ia varten, niin että arvojen 360 asteen kehä otetaan
 *  huomioon lopputuloksessa. Katsotaan miten arvot ovat läjässä ja
 *  katsotaan tuloksia seuraavaan tyyliin:
 *  Esim1: Arvot 70, 80, 110, 120 ja 150 -> min 70 ja max 150 (läjä 110 tienoilla)
 *  Esim2: Arvot 10, 20, 50, 320 ja 350 -> min 320 ja max 50 (läjä 10 tienoilla, 
 *         menee 360 rajan molemmin puolin)
 *  Esim3: Arvot 0, 90, 180 ja 270 -> min 0 ja max 270 (arvot jakautuneet, ei läjää)
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include <set>

class NFmiDistributionSectorData
{
  float lowValue = kFloatMissing;
  float highValue = kFloatMissing;
  float weight = 0;

 public:
  NFmiDistributionSectorData();
  void AddValue(float value);
};

//! Undocumented
class NFmiDataModifierCircularExtreme : public NFmiDataModifier
{
 protected:
  std::multiset<float> itsValues;
  float itsCachedValue = kFloatMissing;
  bool fCachedValueCalculated = false;
  std::vector<NFmiDistributionSectorData> itsDistributionSectorData;

 public:
  ~NFmiDataModifierCircularExtreme() override;
  NFmiDataModifierCircularExtreme();
  NFmiDataModifierCircularExtreme(const NFmiDataModifierCircularExtreme &theOther);

  void Calculate(float theValue) override;
  void Clear();
  int CalcDistributionSectorIndex(float value) const;

 protected:
 float CalcMinimumCircularDiff(float value1, float value2);
 void InitializeDistributionVector();
 void AddToDistribution(float value);

};  // class NFmiDataModifierCircularExtreme

class NFmiDataModifierCircularMin : public NFmiDataModifierCircularExtreme
{
 public:
  ~NFmiDataModifierCircularMin() override;
  NFmiDataModifierCircularMin();
  NFmiDataModifierCircularMin(const NFmiDataModifierCircularMin &theOther);
  NFmiDataModifier* Clone() const override;
  NFmiDataModifierCircularMin& operator=(const NFmiDataModifierCircularMin& ) = delete;

  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

};  // class NFmiDataModifierCircularMin

class NFmiDataModifierCircularMax : public NFmiDataModifierCircularExtreme
{
 public:
  ~NFmiDataModifierCircularMax() override;
  NFmiDataModifierCircularMax();
  NFmiDataModifierCircularMax(const NFmiDataModifierCircularMax &theOther);
  NFmiDataModifier* Clone() const override;
  NFmiDataModifierCircularMax& operator=(const NFmiDataModifierCircularMax&) = delete;

  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

};  // class NFmiDataModifierCircularMax

// ======================================================================
