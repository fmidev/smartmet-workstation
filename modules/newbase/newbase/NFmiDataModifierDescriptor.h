// ======================================================================
/*!
 * \file NFmiDataModifierDescriptor.h
 * \brief Interface of class NFmiDataModifierDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"

class NFmiParamDataModifierList;
class NFmiParamDataModifier;
class NFmiDataIdent;
class NFmiLevel;

//! Undocumented
class _FMI_DLL NFmiDataModifierDescriptor : public NFmiDataDescriptor
{
 public:
  virtual ~NFmiDataModifierDescriptor(void);
  NFmiDataModifierDescriptor(void);

  NFmiParamDataModifier* VarianceModifier(const NFmiDataIdent& theParam, const NFmiLevel* theLevel);
  NFmiParamDataModifierList* GetVarianceModifiers(void);
  void SetVarianceModifiers(NFmiParamDataModifierList* theList);

  // seuraavat on pakko kirjoittaa, koska ne on määritelty pure virtualiksi emossa
  virtual unsigned long Index(void) const;
  virtual unsigned long Size(void) const;
  virtual bool IsActive(void) const;
  virtual bool SetActivity(bool theActivityState);
  virtual bool NextActive(void);
  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 protected:
  NFmiParamDataModifierList* itsSystematicModifiers;  // Omistaa listan
  NFmiParamDataModifierList* itsVarianceModifiers;    // Omistaa listan

 private:
  NFmiDataModifierDescriptor(const NFmiDataModifierDescriptor& theDesc);
  NFmiDataModifierDescriptor& operator=(const NFmiDataModifierDescriptor& theDesc);

};  // class NFmiDataModifierDescriptor

// ======================================================================
