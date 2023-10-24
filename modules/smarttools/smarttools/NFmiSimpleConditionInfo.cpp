#include <NFmiAreaMaskInfo.h>
#include <NFmiSimpleConditionInfo.h>

void NFmiSimpleConditionPartInfo::SetStationDataUsage(const NFmiProducer &mainFunctionProducer)
{
  SetMaskStationDataUsage(itsMask1, mainFunctionProducer);
  SetMaskStationDataUsage(itsMask2, mainFunctionProducer);
}

void NFmiSimpleConditionPartInfo::SetMaskStationDataUsage(boost::shared_ptr<NFmiAreaMaskInfo> &mask,
                                                          const NFmiProducer &mainFunctionProducer)
{
  if (mask)
  {
    if (mask->GetDataIdent().GetProducer()->GetIdent() == mainFunctionProducer.GetIdent())
    {
      // Jos tuottajat olivat samoja, merkitään että kyseistä infoa käytetään kuten asemadataa, jos
      // kyse on sitten oikeasti asemadatasta (ei tiedä tässä vaiheessa)
      mask->SetSecondaryFunctionType(NFmiAreaMask::SimpleConditionUsedAsStationData);
    }
  }
}

NFmiSingleConditionInfo::NFmiSingleConditionInfo()
    : itsPart1(),
      itsConditionOperand1(kFmiNoMaskOperation),
      itsPart2(),
      itsConditionOperand2(kFmiNoMaskOperation),
      itsPart3()
{
}

NFmiSingleConditionInfo::~NFmiSingleConditionInfo() = default;

NFmiSingleConditionInfo::NFmiSingleConditionInfo(
    const boost::shared_ptr<NFmiSimpleConditionPartInfo> &part1,
    FmiMaskOperation conditionOperand1,
    const boost::shared_ptr<NFmiSimpleConditionPartInfo> &part2,
    FmiMaskOperation conditionOperand2,
    const boost::shared_ptr<NFmiSimpleConditionPartInfo> &part3)
    : itsPart1(part1),
      itsConditionOperand1(conditionOperand1),
      itsPart2(part2),
      itsConditionOperand2(conditionOperand2),
      itsPart3(part3)
{
}

void NFmiSingleConditionInfo::SetStationDataUsage(const NFmiProducer &mainFunctionProducer)
{
  if (itsPart1)
    itsPart1->SetStationDataUsage(mainFunctionProducer);
  if (itsPart2)
    itsPart2->SetStationDataUsage(mainFunctionProducer);
  if (itsPart3)
    itsPart3->SetStationDataUsage(mainFunctionProducer);
}

NFmiSimpleConditionInfo::NFmiSimpleConditionInfo() {}

NFmiSimpleConditionInfo::~NFmiSimpleConditionInfo() = default;

NFmiSimpleConditionInfo::NFmiSimpleConditionInfo(
    const boost::shared_ptr<NFmiSingleConditionInfo> &condition1,
    NFmiAreaMask::BinaryOperator conditionOperator,
    const boost::shared_ptr<NFmiSingleConditionInfo> &condition2)
    : itsCondition1(condition1), itsConditionOperator(conditionOperator), itsCondition2(condition2)
{
}

void NFmiSimpleConditionInfo::SetStationDataUsage(const NFmiProducer &mainFunctionProducer)
{
  if (itsCondition1)
    itsCondition1->SetStationDataUsage(mainFunctionProducer);
  if (itsCondition2)
    itsCondition2->SetStationDataUsage(mainFunctionProducer);
}