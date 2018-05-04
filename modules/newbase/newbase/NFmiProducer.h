// ======================================================================
/*!
 * \file NFmiProducer.h
 * \brief Implementation of class NFmiProducer
 */
// ======================================================================
/*!
 * \class NFmiProducer
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiIndividual.h"

//! Undocumented
class _FMI_DLL NFmiProducer : public NFmiIndividual
{
 public:
  NFmiProducer(void);
  NFmiProducer(const NFmiProducer& theProducer);
  explicit NFmiProducer(unsigned long theIdent, const NFmiString& theName = "Kennel");

  NFmiProducer& operator=(const NFmiProducer& theProducer);
  bool operator==(const NFmiProducer& theProducer) const;

  virtual const char* ClassName(void) const;

 private:
};  // class NFmiProducer

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiProducer::NFmiProducer(void) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theIdent Undocumented
 * \param theName Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiProducer::NFmiProducer(unsigned long theIdent, const NFmiString& theName)
    : NFmiIndividual(theIdent, theName)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theProducer The object being copied
 */
// ----------------------------------------------------------------------

inline NFmiProducer::NFmiProducer(const NFmiProducer& theProducer) : NFmiIndividual(theProducer) {}
// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theProducer The object to compare with
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

inline bool NFmiProducer::operator==(const NFmiProducer& theProducer) const
{
  return GetIdent() == theProducer.GetIdent();
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theProducer The object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

inline NFmiProducer& NFmiProducer::operator=(const NFmiProducer& theProducer)
{
  NFmiIndividual::operator=(theProducer);
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiProducer::ClassName(void) const { return "NFmiProducer"; }

// ======================================================================
