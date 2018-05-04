// ======================================================================
/*!
 * \file NFmiDataModifierWindChill.h
 * \brief Interface of class NFmiDataModifierWindChill
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiPoint.h"

class NFmiQueryInfo;

class _FMI_DLL NFmiDataModifierWindChill : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierWindChill(void);

  // anna se queryinfo mistä halutaan laskea windchilliä konstruktorissa!
  NFmiDataModifierWindChill(NFmiQueryInfo* theQueryInfo,
                            NFmiDataModifier* theDataModifier,
                            const NFmiPoint& theLatLonPoint,
                            int theTeperatureParamId = 4,
                            int theWindSpeedParamId = 21);

  void Clear(void);

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult(void);

  const NFmiPoint& LatLon(void) const;
  void LatLon(const NFmiPoint& newLatLon);

 private:
  NFmiDataModifierWindChill(const NFmiDataModifierWindChill& theMod);
  NFmiDataModifierWindChill& operator=(const NFmiDataModifierWindChill& theMod);

  int itsTemperatureParamId;
  int itsWindSpeedParamId;
  NFmiPoint itsLatLonPoint;           // luokka tekee kaiken interpoloituna tähän pisteeseen!!!!
  NFmiQueryInfo* itsQueryInfo;        // ei omista!!!, käytä 'juoksutettua' infoa
  NFmiDataModifier* itsDataModifier;  // ei omista!!!

};  // class NFmiDataModifierWindChill

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint& NFmiDataModifierWindChill::LatLon(void) const { return itsLatLonPoint; }
// ----------------------------------------------------------------------
/*!
 * \param newLatLon Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifierWindChill::LatLon(const NFmiPoint& newLatLon)
{
  itsLatLonPoint = newLatLon;
}

// ======================================================================
