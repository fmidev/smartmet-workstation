// ======================================================================
/*!
 * \file NFmiAreaMaskImpl.cpp
 * \brief Implementation of class NFmiAreaMaskImpl
 */
// ======================================================================
/*!
 * \class NFmiAreaMaskImpl
 *
 * A basic NFmiAreaMask implementation, including the possibility for
 * indexable submasks. Supermask is with index 1, submasks have increasing
 * indices.
 *
 * \bug Should define a copy constructor since there are pointer
 *      members.
 */
// ======================================================================

#include "NFmiAreaMaskImpl.h"
#include "NFmiSimpleCondition.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskImpl::NFmiAreaMaskImpl()
    : itsMaskCondition(),
      itsMaskType(kNoType),
      itsDataType(NFmiInfoData::kNoDataType),
      itsCalculationOperationType(NoType),
      itsCalculationOperator(NotOperation),
      itsPostBinaryOperator(kNoValue),
      itsMathFunctionType(NotMathFunction),
      itsFunctionType(NotFunction),
      itsSecondaryFunctionType(NotFunction),
      itsMetFunctionDirection(NoDirection),
      itsIntegrationFunctionType(0),
      itsFunctionArgumentCount(0),
      fHasSubMasks(false),
      fEnabled(true),
      itsSimpleCondition()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theOperation Undocumented
 * \param theMaskType Undocumented
 * \param theDataType Undocumented
 * \param thePostBinaryOperator Undocumented
 */
// ----------------------------------------------------------------------

NFmiAreaMaskImpl::NFmiAreaMaskImpl(const NFmiCalculationCondition &theOperation,
                                   Type theMaskType,
                                   NFmiInfoData::Type theDataType,
                                   BinaryOperator thePostBinaryOperator)
    : itsMaskCondition(theOperation),
      itsMaskType(theMaskType),
      itsDataType(theDataType),
      itsCalculationOperationType(NoType)  // tämä pitää asettaa erikseen funktiosta!
      ,
      itsCalculationOperator(NotOperation),
      itsPostBinaryOperator(thePostBinaryOperator),
      itsMathFunctionType(NotMathFunction),
      itsFunctionType(NotFunction),
      itsSecondaryFunctionType(NotFunction),
      itsMetFunctionDirection(NoDirection),
      itsIntegrationFunctionType(0),
      itsFunctionArgumentCount(0),
      fHasSubMasks(false),
      fEnabled(true),
      itsSimpleCondition()
{
}

NFmiAreaMaskImpl::NFmiAreaMaskImpl(const NFmiAreaMaskImpl &theOther)
    : itsMaskCondition(theOther.itsMaskCondition),
      itsMaskType(theOther.itsMaskType),
      itsDataType(theOther.itsDataType),
      itsCalculationOperationType(theOther.itsCalculationOperationType),
      itsCalculationOperator(theOther.itsCalculationOperator),
      itsPostBinaryOperator(theOther.itsPostBinaryOperator),
      itsMathFunctionType(theOther.itsMathFunctionType),
      itsFunctionType(theOther.itsFunctionType),
      itsSecondaryFunctionType(theOther.itsSecondaryFunctionType),
      itsMetFunctionDirection(theOther.itsMetFunctionDirection),
      itsIntegrationFunctionType(theOther.itsIntegrationFunctionType),
      itsFunctionArgumentCount(theOther.itsFunctionArgumentCount),
      fHasSubMasks(theOther.fHasSubMasks),
      fEnabled(theOther.fEnabled),
      itsSimpleCondition(theOther.itsSimpleCondition ? theOther.itsSimpleCondition->Clone() : nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskImpl::~NFmiAreaMaskImpl() = default;

void NFmiAreaMaskImpl::Initialize(void) 
{
    if(itsSimpleCondition)
        itsSimpleCondition->Initialize();
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::IsMasked(const NFmiPoint &theLatLon) const
{
  if (!fEnabled)
    return true;  // jos maski ei ole käytössä, on maski aina 'päällä'
  else
  {
    double testValue = CalcValueFromLocation(theLatLon);  // CalcValueFromLocation on virtuaalinen
    return itsMaskCondition.IsMasked(testValue);
  }
}

// ----------------------------------------------------------------------
/*!
 * Laskee arvon 0:n ja 1:n välille riippuen maskista (tarkoitettu
 * laskemaan liukuvia maskeja ja niistä johtuvia kertoimia)
 *
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiAreaMaskImpl::MaskValue(const NFmiPoint &theLatLon) const
{
  if (!fEnabled)
    return 1.;
  else
  {
    double testValue = CalcValueFromLocation(theLatLon);  // CalcValueFromLocation on virtuaalinen
    return itsMaskCondition.MaskValue(testValue);
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatlon Undocumented
 * \param theTime Undocumented, unused
 * \param theTimeIndex Undocumented, unused
 */
// ----------------------------------------------------------------------

double NFmiAreaMaskImpl::Value(const NFmiCalculationParams &theCalculationParams,
                               bool /* fUseTimeInterpolationAlways */)
{
  // useimmille maskiluokille tämä riittää, koska ne eivät
  // ole riippuvaisia ajasta.
  return CalcValueFromLocation(theCalculationParams.itsLatlon);
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::Time(const NFmiMetTime & /* theTime */) { return false; }
// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented, unused
 * \param theLevel Undocumented, unused
 * \return Undocumented, always false
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::IsWantedParam(const NFmiDataIdent & /* theParam */,
                                     const NFmiLevel * /* theLevel */) const
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiAreaMaskImpl::MaskString() const
{
  NFmiString subStr(MakeSubMaskString());
  NFmiString returnValue(itsMaskCondition.MaskString(subStr));
  return returnValue;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiDataIdent *NFmiAreaMaskImpl::DataIdent() const { return nullptr; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiParam *NFmiAreaMaskImpl::Param() const { return nullptr; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiLevel *NFmiAreaMaskImpl::Level() const { return nullptr; }
void NFmiAreaMaskImpl::Level(const NFmiLevel & /* theLevel */) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::UseLevelInfo() const { return false; }
bool NFmiAreaMaskImpl::UsePressureLevelInterpolation() const { return false; }
void NFmiAreaMaskImpl::UsePressureLevelInterpolation(bool /* newValue */) {}
double NFmiAreaMaskImpl::UsedPressureLevelValue() const { return kFloatMissing; }
void NFmiAreaMaskImpl::UsedPressureLevelValue(double /* newValue */) {}
// ----------------------------------------------------------------------
/*!
 * \param theMask Undocumented, unused
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::AddMask(NFmiAreaMask * /* theMask */) { return false; }
// ----------------------------------------------------------------------
/*!
 * Palauttaa joko this:in jos index = 1 ja muuten indeksillä
 * osoitetun 'ali'-maskin, tai 0:n.
 *
 * \param theIndex Unused, undocumented
 * \return Undocumented, always zero
 *
 */
// ----------------------------------------------------------------------

NFmiAreaMask *NFmiAreaMaskImpl::AreaMask(int /* theIndex */) const { return nullptr; }
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented, unused
 * \return Undocumented, always false
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::RemoveSubMask(int /* theIndex*/) { return false; }
// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented, unused
 * \return Undocumented, always kFloatMissing
 */
// ----------------------------------------------------------------------

double NFmiAreaMaskImpl::CalcValueFromLocation(const NFmiPoint & /* theLatLon */) const
{
  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always empty string
 */
// ----------------------------------------------------------------------

const NFmiString NFmiAreaMaskImpl::MakeSubMaskString() const
{
  NFmiString returnVal;
  return returnVal;
}

// ----------------------------------------------------------------------
/*!
 * \param theCondition
 */
// ----------------------------------------------------------------------

void NFmiAreaMaskImpl::Condition(const NFmiCalculationCondition &theCondition)
{
  itsMaskCondition = theCondition;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskImpl::IsRampMask() const { return itsMaskCondition.IsRampMask(); }
// ======================================================================

void NFmiAreaMaskImpl::DoIntegrationCalculations(float value)
{
}

void NFmiAreaMaskImpl::InitializeIntegrationValues()
{
}

bool NFmiAreaMaskImpl::SimpleConditionCheck(const NFmiCalculationParams &theCalculationParams)
{
    if(itsSimpleCondition)
        return itsSimpleCondition->CheckCondition(theCalculationParams, true);
    else
        return true;
}

float NFmiAreaMaskImpl::CalculationPointValue(int theOffsetX, int theOffsetY, const NFmiMetTime &theInterpolationTime, bool useInterpolatedTime)
{
    return kFloatMissing;
}
