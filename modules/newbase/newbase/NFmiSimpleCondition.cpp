#include <NFmiSimpleCondition.h>
#include <NFmiFastQueryInfo.h>

namespace
{
bool CheckForStationaryData(const boost::shared_ptr<NFmiAreaMask> &mask)
{
  if (mask)
  {
    if (mask->GetDataType() == NFmiInfoData::kStationary) return true;
  }
  return false;
}

bool UseTimeInterpolation(bool maskIsStationaryData, bool normalInterpolationCondition)
{
  if (maskIsStationaryData)
    return false;
  else
    return normalInterpolationCondition;
}

double CalculateValue(double value1,
                      double value2,
                      NFmiAreaMask::CalculationOperator calculationOperator)
{
  if (value1 == kFloatMissing || value2 == kFloatMissing)
    return kFloatMissing;
  else
  {
    switch (calculationOperator)
    {
      case NFmiAreaMask::Add:
        return value1 + value2;
      case NFmiAreaMask::Sub:
        return value1 - value2;
      case NFmiAreaMask::Mul:
        return value1 * value2;
      case NFmiAreaMask::Div:
      {
        if (value2 == 0)
          return kFloatMissing;
        else
          return value1 / value2;
      }
      case NFmiAreaMask::Mod:
      {
        if (value2 == 0)
          return kFloatMissing;
        else
          return std::fmod(value1, value2);
      }
      case NFmiAreaMask::Pow:
        return std::pow(value1, value2);
      default:
        throw std::runtime_error(
            "Internal program error with SimpleCondition's calculation operator - unknown "
            "operator");
    }
  }
}

double GetPressureValue(boost::shared_ptr<NFmiAreaMask> &mask, double pressure, const NFmiCalculationParams &calculationParams, bool useTimeInterpolation)
{
  if (mask && mask->Info() && mask->Info()->SizeLevels() <= 1)
  {
    // jos käytössä on 1 levelinen pintadata, pyydetään vain pinta-arvoa
    return mask->Value(calculationParams, useTimeInterpolation);
  }
  else
      return mask->PressureValue(pressure, calculationParams);
}

double GetHeightValue(boost::shared_ptr<NFmiAreaMask> &mask,
                      double height,
                      const NFmiCalculationParams &calculationParams,
                      bool useTimeInterpolation)
{
  if (mask && mask->Info() && mask->Info()->SizeLevels() <= 1)
  {
    // jos käytössä on 1 levelinen pintadata, pyydetään vain pinta-arvoa
    return mask->Value(calculationParams, useTimeInterpolation);
  }
  else
    return mask->HeightValue(height, calculationParams);
}

}
// namespace
   // *****************************************************************
// **************   NFmiSimpleConditionPart   **********************
// *****************************************************************

    NFmiSimpleConditionPart::~NFmiSimpleConditionPart() = default;

NFmiSimpleConditionPart::NFmiSimpleConditionPart(boost::shared_ptr<NFmiAreaMask> &mask1,
    NFmiAreaMask::CalculationOperator calculationOperator,
    boost::shared_ptr<NFmiAreaMask> &mask2)
    :itsMask1(mask1)
    , itsCalculationOperator(calculationOperator)
    , itsMask2(mask2)
{
}

NFmiSimpleConditionPart::NFmiSimpleConditionPart(const NFmiSimpleConditionPart &theOther)
    :itsMask1(theOther.itsMask1 ? theOther.itsMask1->Clone() : nullptr)
    , isMask1StationaryData(theOther.isMask1StationaryData)
    , itsCalculationOperator(theOther.itsCalculationOperator)
    , itsMask2(theOther.itsMask2 ? theOther.itsMask2->Clone() : nullptr)
    , isMask2StationaryData(theOther.isMask2StationaryData)
    , itsPreviousValue(theOther.itsPreviousValue)
{
}

void NFmiSimpleConditionPart::Initialize()
{
    isMask1StationaryData = ::CheckForStationaryData(itsMask1);
    isMask2StationaryData = ::CheckForStationaryData(itsMask2);
}

NFmiSimpleConditionPart* NFmiSimpleConditionPart::Clone() const
{
    return new NFmiSimpleConditionPart(*this);
}

double NFmiSimpleConditionPart::Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways)
{
    double value1 = itsMask1->Value(theCalculationParams, ::UseTimeInterpolation(isMask1StationaryData, fUseTimeInterpolationAlways));
    if(!itsMask2)
        return value1;
    else
    {
        double value2 = itsMask2->Value(theCalculationParams, ::UseTimeInterpolation(isMask2StationaryData, fUseTimeInterpolationAlways));
        return ::CalculateValue(value1, value2, itsCalculationOperator);
    }
}

double NFmiSimpleConditionPart::PressureValue(double thePressure,
                                              const NFmiCalculationParams &theCalculationParams)
{
  auto doTimeInterp = ::UseTimeInterpolation(isMask1StationaryData, true);
  double value1 = ::GetPressureValue(itsMask1, thePressure, theCalculationParams, doTimeInterp);
  if (!itsMask2)
    return value1;
  else
  {
    double value2 = ::GetPressureValue(itsMask2, thePressure, theCalculationParams, doTimeInterp);
    return ::CalculateValue(value1, value2, itsCalculationOperator);
  }
}

double NFmiSimpleConditionPart::HeightValue(double theHeight,
                                            const NFmiCalculationParams &theCalculationParams)
{
  auto doTimeInterp = ::UseTimeInterpolation(isMask1StationaryData, true);
  double value1 = ::GetHeightValue(itsMask1, theHeight, theCalculationParams, doTimeInterp);
  if (!itsMask2)
    return value1;
  else
  {
    double value2 = ::GetHeightValue(itsMask2, theHeight, theCalculationParams, doTimeInterp);
    return ::CalculateValue(value1, value2, itsCalculationOperator);
  }
}

double NFmiSimpleConditionPart::PreviousValue(double newPreviousValue) 
{
  auto returnValue = itsPreviousValue;
  itsPreviousValue = newPreviousValue;
  return returnValue;
}

void NFmiSimpleConditionPart::ResetPreviousValue() { itsPreviousValue = kFloatMissing; }


// *****************************************************************
// ****************   NFmiSingleCondition   ************************
// *****************************************************************

NFmiSingleCondition::~NFmiSingleCondition() = default;

NFmiSingleCondition::NFmiSingleCondition(const boost::shared_ptr<NFmiSimpleConditionPart> &thePart1,
    FmiMaskOperation theConditionOperand1,
    const boost::shared_ptr<NFmiSimpleConditionPart> &thePart2,
    FmiMaskOperation theConditionOperand2,
    const boost::shared_ptr<NFmiSimpleConditionPart> &thePart3)
    :part1(thePart1)
    , conditionOperand1(theConditionOperand1)
    , part2(thePart2)
    ,conditionOperand2(theConditionOperand2)
    , part3(thePart3)
{}

NFmiSingleCondition::NFmiSingleCondition(const NFmiSingleCondition &theOther)
    :part1(theOther.part1 ? theOther.part1->Clone() : nullptr)
    , conditionOperand1(theOther.conditionOperand1)
    , part2(theOther.part2 ? theOther.part2->Clone() : nullptr)
    , conditionOperand2(theOther.conditionOperand2)
    , part3(theOther.part3 ? theOther.part3->Clone() : nullptr)
{}

static void InitializePart(boost::shared_ptr<NFmiSimpleConditionPart> &part)
{
    if(part)
        part->Initialize();
}

void NFmiSingleCondition::Initialize()
{
    InitializePart(part1);
    InitializePart(part2);
    InitializePart(part3);
}

NFmiSingleCondition* NFmiSingleCondition::Clone() const
{
    return new NFmiSingleCondition(*this);
}

static bool EvaluateCondition(double value1, FmiMaskOperation operand, double value2)
{
    if(value1 == kFloatMissing || value2 == kFloatMissing)
        return false;
    else if(operand == kFmiNoMaskOperation)
        return false;
    else
    {
        switch(operand)
        {
        case kFmiMaskEqual:
            return value1 == value2;
        case kFmiMaskGreaterThan:
            return value1 > value2;
        case kFmiMaskLessThan:
            return value1 < value2;
        case kFmiMaskGreaterOrEqualThan:
            return value1 >= value2;
        case kFmiMaskLessOrEqualThan:
            return value1 <= value2;
        case kFmiMaskNotEqual:
            return value1 != value2;
        default:
            throw std::runtime_error("Error in SimpleCondition's EvaluateCondition function: unsupported condition operand");
        }
    }
}

static bool IsGreaterOperand(FmiMaskOperation operand)
{
    return (operand == kFmiMaskGreaterThan || operand == kFmiMaskGreaterOrEqualThan);
}

// Note: only range option accepted are:
// 1. In between case e.g. "limit1 < x < limit2", where operands can be < or <= 
// 2. Outside case e.g. "limit1 > x > limit2", where operands can be > or >= 
static bool EvaluateRangeCondition(double value1, FmiMaskOperation operand1, double value2, FmiMaskOperation operand2, double value3)
{
    bool condition1 = ::EvaluateCondition(value1, operand1, value2);
    bool condition2 = ::EvaluateCondition(value2, operand2, value3);
    if(::IsGreaterOperand(operand1))
    {
        // Outside limits case e.g. "-5 > T > 0", only other condition is true
        return condition1 ^ condition2;
    }
    else
    {
        // In between case e.g. "-5 < T < 0", both conditions must be true
        return condition1 && condition2;
    }
}

// value2:n pitää olla jotenkin value1:n ja previousValue1:n välissä
static bool EvaluateContinuousEqualCase(double value1,
                                          double value2,
                                          double previousValue1) 
{
  if (value1 != kFloatMissing && value2 != kFloatMissing && previousValue1 != kFloatMissing) 
  {
    if (value1 >= value2 && previousValue1 <= value2) return true;
    if (previousValue1 >= value2 && value1 <= value2) return true;
  }
  return false;
}

bool NFmiSingleCondition::CheckCondition(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways)
{
    double value1 = part1->Value(theCalculationParams, fUseTimeInterpolationAlways);
    double value2 = part2->Value(theCalculationParams, fUseTimeInterpolationAlways);
    if (conditionOperand1 == kFmiMaskContinuousEqual)
    {
      return ::EvaluateContinuousEqualCase(value1, value2, part1->PreviousValue(value1));
    }
    else if(!part3)
    { 
        return ::EvaluateCondition(value1, conditionOperand1, value2);
    }
    else
    {
        double value3 = part3->Value(theCalculationParams, fUseTimeInterpolationAlways);
        return ::EvaluateRangeCondition(value1, conditionOperand1, value2, conditionOperand2, value3);
    }
}

bool NFmiSingleCondition::CheckPressureCondition(double thePressure, const NFmiCalculationParams &theCalculationParams)
{
    double value1 = part1->PressureValue(thePressure, theCalculationParams);
    double value2 = part2->PressureValue(thePressure, theCalculationParams);
    if (conditionOperand1 == kFmiMaskContinuousEqual)
    {
      return ::EvaluateContinuousEqualCase(value1, value2, part1->PreviousValue(value1));
    }
    else if (!part3)
    {
        return ::EvaluateCondition(value1, conditionOperand1, value2);
    }
    else
    {
        double value3 = part3->PressureValue(thePressure, theCalculationParams);
        return ::EvaluateRangeCondition(value1, conditionOperand1, value2, conditionOperand2, value3);
    }
}

bool NFmiSingleCondition::CheckHeightCondition(double theHeight, const NFmiCalculationParams &theCalculationParams)
{
    double value1 = part1->HeightValue(theHeight, theCalculationParams);
    double value2 = part2->HeightValue(theHeight, theCalculationParams);
    if (conditionOperand1 == kFmiMaskContinuousEqual)
    {
      return ::EvaluateContinuousEqualCase(value1, value2, part1->PreviousValue(value1));
    }
    else if (!part3)
    {
        return ::EvaluateCondition(value1, conditionOperand1, value2);
    }
    else
    {
        double value3 = part3->HeightValue(theHeight, theCalculationParams);
        return ::EvaluateRangeCondition(value1, conditionOperand1, value2, conditionOperand2, value3);
    }
}

void NFmiSingleCondition::ResetPreviousValue() 
{
  if (part1) part1->ResetPreviousValue();
  if (part2) part2->ResetPreviousValue();
  if (part3) part3->ResetPreviousValue();
}

// *****************************************************************
// ****************   NFmiSimpleCondition   ************************
// *****************************************************************

NFmiSimpleCondition::~NFmiSimpleCondition() = default;

NFmiSimpleCondition::NFmiSimpleCondition(const boost::shared_ptr<NFmiSingleCondition> &theCondition1,
    NFmiAreaMask::BinaryOperator theConditionOperator,
    const boost::shared_ptr<NFmiSingleCondition> &theCondition2)
    :condition1(theCondition1)
    , conditionOperator(theConditionOperator)
    , condition2(theCondition2)
{
}

NFmiSimpleCondition::NFmiSimpleCondition(const NFmiSimpleCondition &theOther)
    :condition1(theOther.condition1 ? theOther.condition1->Clone() : nullptr)
    , conditionOperator(theOther.conditionOperator)
    , condition2(theOther.condition2 ? theOther.condition2->Clone() : nullptr)
{
}

static void InitializePart(boost::shared_ptr<NFmiSingleCondition> &condition)
{
    if(condition)
        condition->Initialize();
}

// Tätä kutsutaan konstruktorin jälkeen, tässä alustetaan ainakin tieto siitä onko maski ns. stationaaristä dataa
void NFmiSimpleCondition::Initialize()
{
    ::InitializePart(condition1);
    ::InitializePart(condition2);
}

NFmiSimpleCondition* NFmiSimpleCondition::Clone() const
{
    return new NFmiSimpleCondition(*this);
}

static bool EvaluateBinaryCondition(bool condition1, NFmiAreaMask::BinaryOperator conditionOperator, bool condition2)
{
    switch(conditionOperator)
    {
    case NFmiAreaMask::kAnd:
        return condition1 && condition2;
    case NFmiAreaMask::kOr:
        return condition1 || condition2;
    case NFmiAreaMask::kXor:
        return condition1 ^ condition2;
    default:
        throw std::runtime_error("Error in SimpleCondition's EvaluateCondition function: unsupported binary operator");
    }
}


// Eri tapauksia varten omat tarkastelu funktiot
bool NFmiSimpleCondition::CheckCondition(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways)
{
    bool conditionValue1 = condition1->CheckCondition(theCalculationParams, fUseTimeInterpolationAlways);
    if(!condition2)
        return conditionValue1;
    else
    {
        bool conditionValue2 = condition2->CheckCondition(theCalculationParams, fUseTimeInterpolationAlways);
        return ::EvaluateBinaryCondition(conditionValue1, conditionOperator, conditionValue2);
    }
}

bool NFmiSimpleCondition::CheckPressureCondition(double thePressure, const NFmiCalculationParams &theCalculationParams)
{
    bool conditionValue1 = condition1->CheckPressureCondition(thePressure, theCalculationParams);
    if(!condition2)
        return conditionValue1;
    else
    {
        bool conditionValue2 = condition2->CheckPressureCondition(thePressure, theCalculationParams);
        return ::EvaluateBinaryCondition(conditionValue1, conditionOperator, conditionValue2);
    }
}

bool NFmiSimpleCondition::CheckHeightCondition(double theHeight, const NFmiCalculationParams &theCalculationParams)
{
    bool conditionValue1 = condition1->CheckHeightCondition(theHeight, theCalculationParams);
    if(!condition2)
        return conditionValue1;
    else
    {
        bool conditionValue2 = condition2->CheckHeightCondition(theHeight, theCalculationParams);
        return ::EvaluateBinaryCondition(conditionValue1, conditionOperator, conditionValue2);
    }
}

void NFmiSimpleCondition::ResetPreviousValue()
{
  if (condition1) condition1->ResetPreviousValue();
  if (condition2) condition2->ResetPreviousValue();
}
