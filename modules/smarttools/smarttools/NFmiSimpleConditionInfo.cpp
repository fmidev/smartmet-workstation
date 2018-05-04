#include <NFmiSimpleConditionInfo.h>

NFmiSingleConditionInfo::NFmiSingleConditionInfo(void)
:itsPart1()
,itsConditionOperand1(kFmiNoMaskOperation)
,itsPart2()
,itsConditionOperand2(kFmiNoMaskOperation)
,itsPart3()
{
}

NFmiSingleConditionInfo::~NFmiSingleConditionInfo(void) = default;

NFmiSingleConditionInfo::NFmiSingleConditionInfo(const boost::shared_ptr<NFmiSimpleConditionPartInfo> &part1,
    FmiMaskOperation conditionOperand1,
    const boost::shared_ptr<NFmiSimpleConditionPartInfo> &part2,
    FmiMaskOperation conditionOperand2,
    const boost::shared_ptr<NFmiSimpleConditionPartInfo> &part3)
    :itsPart1(part1)
    , itsConditionOperand1(conditionOperand1)
    , itsPart2(part2)
    , itsConditionOperand2(conditionOperand2)
    , itsPart3(part3)
{
}


NFmiSimpleConditionInfo::NFmiSimpleConditionInfo(void)
{
}

NFmiSimpleConditionInfo::~NFmiSimpleConditionInfo(void) = default;

NFmiSimpleConditionInfo::NFmiSimpleConditionInfo(const boost::shared_ptr<NFmiSingleConditionInfo> &condition1,
    NFmiAreaMask::BinaryOperator conditionOperator,
    const boost::shared_ptr<NFmiSingleConditionInfo> &condition2)
    :itsCondition1(condition1)
    , itsConditionOperator(conditionOperator)
    , itsCondition2(condition2)
{
}
