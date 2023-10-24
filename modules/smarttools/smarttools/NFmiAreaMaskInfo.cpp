//**********************************************************
// C++ Class Name : NFmiAreaMaskInfo
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: G:/siirto/marko/oc/NFmiAreaMaskInfo2.cpp
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 4.0 )
//  - GD System Name    : aSmartTools
//  - GD View Type      : Class Diagram
//  - GD View Name      : smarttools 1
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Thur - Jun 20, 2002
//
//  Change Log     :
//
//**********************************************************
#ifdef _MSC_VER
#pragma warning(disable : 4786)  // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiAreaMaskInfo.h"
#include "NFmiSimpleConditionInfo.h"
#include <newbase/NFmiLevel.h>

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiAreaMaskInfo::NFmiAreaMaskInfo(const std::string &theOrigLineText)
    : itsDataIdent(),
      fUseDefaultProducer(true),
      itsMaskCondition(),
      itsOperationType(NFmiAreaMask::NoType),
      itsCalculationOperator(NFmiAreaMask::NotOperation),
      itsBinaryOperator(NFmiAreaMask::kNoValue),
      itsDataType(NFmiInfoData::kNoDataType),
      itsLevel(nullptr),
      itsMaskText(),
      itsOrigLineText(theOrigLineText),
      itsFunctionType(NFmiAreaMask::NotFunction),
      itsSecondaryFunctionType(NFmiAreaMask::NotFunction),
      itsMetFunctionDirection(NFmiAreaMask::NoDirection),
      itsOffsetPoint1(),
      itsOffsetPoint2(),
      itsMathFunctionType(NFmiAreaMask::NotMathFunction),
      itsIntegrationFunctionType(0),
      itsFunctionArgumentCount(0),
      itsSoundingParameter(kSoundingParNone),
      itsModelRunIndex(1),
      itsSimpleConditionInfo()
{
}

NFmiAreaMaskInfo::NFmiAreaMaskInfo(const NFmiAreaMaskInfo &theOther)
    : itsDataIdent(theOther.itsDataIdent),
      fUseDefaultProducer(theOther.fUseDefaultProducer),
      itsMaskCondition(theOther.itsMaskCondition),
      itsOperationType(theOther.itsOperationType),
      itsCalculationOperator(theOther.itsCalculationOperator),
      itsBinaryOperator(theOther.itsBinaryOperator),
      itsDataType(theOther.itsDataType),
      itsLevel(theOther.itsLevel ? new NFmiLevel(*theOther.itsLevel) : nullptr),
      itsMaskText(theOther.itsMaskText),
      itsOrigLineText(theOther.itsOrigLineText),
      itsFunctionType(theOther.itsFunctionType),
      itsSecondaryFunctionType(theOther.itsSecondaryFunctionType),
      itsMetFunctionDirection(theOther.itsMetFunctionDirection),
      itsOffsetPoint1(theOther.itsOffsetPoint1),
      itsOffsetPoint2(theOther.itsOffsetPoint2),
      itsMathFunctionType(theOther.itsMathFunctionType),
      itsIntegrationFunctionType(theOther.itsIntegrationFunctionType),
      itsFunctionArgumentCount(theOther.itsFunctionArgumentCount),
      itsSoundingParameter(theOther.itsSoundingParameter),
      itsModelRunIndex(theOther.itsModelRunIndex),
      itsSimpleConditionInfo(theOther.itsSimpleConditionInfo),
      itsTimeOffsetInHours(theOther.itsTimeOffsetInHours),
      itsSecondaryParam(theOther.itsSecondaryParam),
      itsSecondaryParamLevel(theOther.itsSecondaryParamLevel
                                 ? new NFmiLevel(*theOther.itsSecondaryParamLevel)
                                 : nullptr),
      itsSecondaryParamDataType(theOther.itsSecondaryParamDataType),
      fSecondaryParamUseDefaultProducer(theOther.fSecondaryParamUseDefaultProducer)
{
}

NFmiAreaMaskInfo::~NFmiAreaMaskInfo()
{
  delete itsLevel;
}

void NFmiAreaMaskInfo::SetLevel(NFmiLevel *theLevel)
{
  delete itsLevel;
  itsLevel = theLevel ? new NFmiLevel(*theLevel) : nullptr;
}

bool NFmiAreaMaskInfo::AllowSimpleCondition() const
{
  return (itsSimpleConditionRule == NFmiAreaMask::SimpleConditionRule::Allowed ||
          itsSimpleConditionRule == NFmiAreaMask::SimpleConditionRule::MustHave);
}

void NFmiAreaMaskInfo::SimpleConditionInfo(
    boost::shared_ptr<NFmiSimpleConditionInfo> &theSimpleConditionInfo)
{
  itsSimpleConditionInfo = theSimpleConditionInfo;
  if (itsSimpleConditionInfo)
  {
    itsSimpleConditionInfo->SetStationDataUsage(*itsDataIdent.GetProducer());
  }
}

void NFmiAreaMaskInfo::SetSecondaryParamLevel(NFmiLevel *theLevel)
{
  delete itsSecondaryParamLevel;
  itsSecondaryParamLevel = theLevel ? new NFmiLevel(*theLevel) : nullptr;
}
