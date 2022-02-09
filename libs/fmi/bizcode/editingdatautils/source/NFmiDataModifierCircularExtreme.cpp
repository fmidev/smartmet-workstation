#include "NFmiDataModifierCircularExtreme.h"
#include "boost/math/special_functions/round.hpp"

const float kCircularModula = 360.f;
const float kDistributionSectorAngle = 30.f;

// ****************************************************************************
// ********************  NFmiDistributionSectorData  **************************
// ****************************************************************************

NFmiDistributionSectorData::NFmiDistributionSectorData() = default;

void NFmiDistributionSectorData::AddValue(float value)
{
  weight++;

  if (lowValue == kFloatMissing)
    lowValue = value;
  else if (lowValue > value)
    lowValue = value;

  if (highValue == kFloatMissing)
    highValue = value;
  else if (highValue < value)
    highValue = value;
}


// ****************************************************************************
// *****************  NFmiDataModifierCircularExtreme  ************************
// ****************************************************************************

  NFmiDataModifierCircularExtreme::~NFmiDataModifierCircularExtreme() = default;
  NFmiDataModifierCircularExtreme::NFmiDataModifierCircularExtreme() = default;
  NFmiDataModifierCircularExtreme::NFmiDataModifierCircularExtreme(
      const NFmiDataModifierCircularExtreme& theOther) = default;

  void NFmiDataModifierCircularExtreme::Calculate(float theValue)
  {
    if (theValue != kFloatMissing)
    {
      // Varmistetaan että annetut arvot ovat 0-360 välillä
      float finalValue = std::fmod(theValue, kCircularModula);
      if (finalValue < 0)
      {
        finalValue += kCircularModula;
      }
      itsValues.insert(finalValue);
      fCachedValueCalculated = false;
      AddToDistribution(finalValue);
    }
  }

  void NFmiDataModifierCircularExtreme::Clear()
  {
    itsValues.clear();
    itsCachedValue = kFloatMissing;
    fCachedValueCalculated = false;
  }

 float NFmiDataModifierCircularExtreme::CalcMinimumCircularDiff(float value1, float value2)
 {
   if (value1 != kFloatMissing && value2 != kFloatMissing)
   {
     if (value2 > value1)
     {
       std::swap(value1, value2);
     }
     auto diff1 = value1 - value2;
     auto diff2 = std::fabs(value2 - (value1 - 360));
     return (diff1 < diff2) ? diff1 : diff2;
   }
   return kFloatMissing;
 }
 
 void NFmiDataModifierCircularExtreme::InitializeDistributionVector()
 {
   itsDistributionSectorData.clear();
   itsDistributionSectorData.resize(
       boost::math::iround(kCircularModula / kDistributionSectorAngle), NFmiDistributionSectorData());
 }

 int NFmiDataModifierCircularExtreme::CalcDistributionSectorIndex(float value) const
 {
   int index = boost::math::iround(value / kDistributionSectorAngle);
   if (index >= itsDistributionSectorData.size())
     index = 0;
   else if (index < 0)
     index = 0;
   return index;
 }


 void NFmiDataModifierCircularExtreme::AddToDistribution(float value)
 {
   int index = CalcDistributionSectorIndex(value);
   itsDistributionSectorData[index].AddValue(value);
 }

 // ****************************************************************************
 // ******************  NFmiDataModifierCircularMin  **************************
 // ****************************************************************************

  NFmiDataModifierCircularMin::~NFmiDataModifierCircularMin() = default;
 NFmiDataModifierCircularMin::NFmiDataModifierCircularMin() = default;
  NFmiDataModifierCircularMin::NFmiDataModifierCircularMin(
     const NFmiDataModifierCircularMin& theOther) = default;

  NFmiDataModifier* NFmiDataModifierCircularMin::Clone() const
  {
    return new NFmiDataModifierCircularMin(*this);
  }

  float NFmiDataModifierCircularMin::CalculationResult()
  {
      return kFloatMissing;
  }


// ****************************************************************************
  // ******************  NFmiDataModifierCircularMax  **************************
  // ****************************************************************************

  NFmiDataModifierCircularMax::~NFmiDataModifierCircularMax() = default;
  NFmiDataModifierCircularMax::NFmiDataModifierCircularMax() = default;
  NFmiDataModifierCircularMax::NFmiDataModifierCircularMax(
      const NFmiDataModifierCircularMax& theOther) = default;

  NFmiDataModifier* NFmiDataModifierCircularMax::Clone() const
  {
    return new NFmiDataModifierCircularMax(*this);
  }
  
  float NFmiDataModifierCircularMax::CalculationResult() 
  { 
      return kFloatMissing; 
  }

