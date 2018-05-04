// ======================================================================
/*!
 * \file NFmiInfoModifier.cpp
 * \brief Implementation for class NFmiInfoModifier
 */
// ======================================================================
/*!
 * \class NFmiInfoModifier
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiInfoModifier.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiInfoModifier::~NFmiInfoModifier() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \param theData Undocumented
 * \param theJoinOperator Undocumented
 */
// ----------------------------------------------------------------------

NFmiInfoModifier::NFmiInfoModifier(NFmiDataIdent* theParam,
                                   NFmiLevel* theLevel,
                                   NFmiQueryInfo* theData,
                                   FmiJoinOperator theJoinOperator)
    : NFmiParamDataModifier(theParam, theLevel, theJoinOperator), itsData(theData)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiInfoModifier::FloatValue() { return itsData->FloatValue(); }
// ----------------------------------------------------------------------
/*!
 * \param dx Undocumented
 * \param dy Undocumented
 * \param dt Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiInfoModifier::PeekValue(long dx, long dy, long dt)
{
  return itsData->PeekValue(dt, dx, dy);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo* NFmiInfoModifier::Info() { return itsData; }
// ======================================================================
