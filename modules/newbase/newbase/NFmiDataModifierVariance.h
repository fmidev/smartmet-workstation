// ======================================================================
/*!
 * \file NFmiDataModifierVariance.h
 * \brief Interface of class NFmiDataModifierVariance
 */
// ======================================================================

#pragma once

#include "NFmiRegressionModifier.h"

#include <boost/thread.hpp>
typedef boost::shared_mutex MutexType;

//! Undocumented
class _FMI_DLL NFmiDataModifierVariance : public NFmiRegressionModifier
{
 public:
  virtual ~NFmiDataModifierVariance(void);
  NFmiDataModifierVariance(NFmiDataIdent* theDataIdent,
                           NFmiLevel* theLevel,
                           NFmiQueryInfo* theData = 0,
                           float theVarianceLevel = 1.0,
                           bool onlyVariance = false);

  virtual double FloatValue(void);

 protected:
  double GaussianRandom(void);
  bool fVarianceOnly;
  double itsVarianceLevel;
  mutable MutexType itsMutex;

};  // class NFmiDataModifierVariance

// ======================================================================
