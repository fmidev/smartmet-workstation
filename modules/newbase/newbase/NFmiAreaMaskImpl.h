// ======================================================================
/*!
 * \file NFmiAreaMaskImpl.h
 * \brief Interface for class NFmiAreaMaskImpl
 */
// ======================================================================

#pragma once

#include "NFmiAreaMask.h"
#include "NFmiCalculationCondition.h"
#include "NFmiInfoData.h"

class NFmAreaMaskList;

//! A basic area mask implementation
class _FMI_DLL NFmiAreaMaskImpl : public NFmiAreaMask
{
 public:
  virtual ~NFmiAreaMaskImpl(void);
  NFmiAreaMaskImpl(void);
  NFmiAreaMaskImpl(const NFmiCalculationCondition &theOperation,
                   Type theMaskType,
                   NFmiInfoData::Type theDataType,
                   BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiAreaMaskImpl(const NFmiAreaMaskImpl &theOther);
  void Initialize(void);

  bool IsMasked(int theIndex) const;
  bool IsMasked(const NFmiPoint &theLatLon) const;
  double MaskValue(const NFmiPoint &theLatLon) const;
  double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways);
  // oletuksenä HeightValue palauttaa saman kuin Value-metodi, homma overridataan vain
  // NFmiInfoAreaMask-luokassa
  double HeightValue(double /* theHeight */, const NFmiCalculationParams &theCalculationParams)
  {
    // Poikkileikkauslaskuissa pitää aina käyttää 'aikainterpolaatiota', muuten ei (koska laskussa käytetyt ajat on jo asetettu muualla)
    bool useTimeInterpolationAlways = theCalculationParams.fCrossSectionCase;
    return Value(theCalculationParams, useTimeInterpolationAlways);
  }
  // oletuksenä PressureValue palauttaa saman kuin Value-metodi, homma overridataan vain
  // NFmiInfoAreaMask-luokassa
  double PressureValue(double /* thePressure */, const NFmiCalculationParams &theCalculationParams)
  {
    // Poikkileikkauslaskuissa pitää aina käyttää 'aikainterpolaatiota', muuten ei (koska laskussa käytetyt ajat on jo asetettu muualla)
    bool useTimeInterpolationAlways = theCalculationParams.fCrossSectionCase;
    return Value(theCalculationParams, useTimeInterpolationAlways);
  }

  bool IsEnabled(void) const;
  void Enable(bool theNewState);

  bool Time(const NFmiMetTime &theTime);
  void Condition(const NFmiCalculationCondition &theCondition);
  const NFmiCalculationCondition &Condition(void) const;

  bool IsRampMask(void) const;
  bool IsWantedParam(const NFmiDataIdent &theParam, const NFmiLevel *theLevel = 0) const;
  const NFmiString MaskString(void) const;
  boost::shared_ptr<NFmiFastQueryInfo> Info(void);
  void UpdateInfo(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
  const NFmiDataIdent *DataIdent(void) const;
  const NFmiParam *Param(void) const;
  const NFmiLevel *Level(void) const;
  void Level(const NFmiLevel &theLevel);
  NFmiInfoData::Type GetDataType(void) const;
  void SetDataType(NFmiInfoData::Type theType);
  bool UseLevelInfo(void) const;
  bool UsePressureLevelInterpolation(void) const;
  void UsePressureLevelInterpolation(bool newValue);
  double UsedPressureLevelValue(void) const;
  void UsedPressureLevelValue(double newValue);

  void LowerLimit(double theLowerLimit);
  void UpperLimit(double theUpperLimit);
  double LowerLimit(void) const;
  double UpperLimit(void) const;
  void MaskOperation(FmiMaskOperation theMaskOperation);
  FmiMaskOperation MaskOperation(void) const;

  bool AddMask(NFmiAreaMask *theMask);
  NFmiAreaMask *AreaMask(int theIndex) const;
  bool RemoveSubMask(int theIndex);
  void MaskType(Type theType);
  Type MaskType(void) const;
  NFmiAreaMask *Clone(void) const;
  void PostBinaryOperator(BinaryOperator newOperator);
  BinaryOperator PostBinaryOperator(void) const;
  CalculationOperationType GetCalculationOperationType(void) const;
  void SetCalculationOperationType(CalculationOperationType newValue);
  CalculationOperator GetCalculationOperator(void) const;
  void SetCalculationOperator(CalculationOperator newValue);
  MathFunctionType GetMathFunctionType(void) const;
  void SetMathFunctionType(MathFunctionType newValue);
  FunctionType GetFunctionType(void) const { return itsFunctionType; }
  void SetFunctionType(FunctionType newType) { itsFunctionType = newType; }
  FunctionType GetSecondaryFunctionType(void) const { return itsSecondaryFunctionType; }
  void SetSecondaryFunctionType(FunctionType newType) { itsSecondaryFunctionType = newType; }
  MetFunctionDirection GetMetFunctionDirection(void) const { return itsMetFunctionDirection; }
  void GetMetFunctionDirection(MetFunctionDirection newValue)
  {
    itsMetFunctionDirection = newValue;
  }
  int IntegrationFunctionType(void) const { return itsIntegrationFunctionType; }
  void IntegrationFunctionType(int newValue) { itsIntegrationFunctionType = newValue; }
  void SetArguments(std::vector<float> & /* theArgumentVector */){};
  int FunctionArgumentCount(void) const { return itsFunctionArgumentCount; }
  void FunctionArgumentCount(int newValue) { itsFunctionArgumentCount = newValue; }
  // HUOM! seuraavat toimivat oikeasti vain NFmiBinaryMask:in kanssa. Tässä vain tyhjät oletus
  // toteutukset.
  void SetAll(bool /* theNewState */){};
  void Mask(int /* theIndex */, bool /* newStatus */){};
  const boost::shared_ptr<NFmiSimpleCondition>& SimpleCondition() const { return itsSimpleCondition; }
  void SimpleCondition(boost::shared_ptr<NFmiSimpleCondition> &theSimpleCondition) { itsSimpleCondition = theSimpleCondition; }

 protected:
  virtual double CalcValueFromLocation(const NFmiPoint &theLatLon) const;
  virtual const NFmiString MakeSubMaskString(void) const;
  // Seuraavat virtuaali funktiot liittyvät integraatio funktioihin ja niiden mahdollisiin Simplecondition tarkasteluihin
  virtual void DoIntegrationCalculations(float value);
  virtual void InitializeIntegrationValues();
  virtual bool SimpleConditionCheck(const NFmiCalculationParams &theCalculationParams);
  virtual float CalculationPointValue(int theOffsetX, int theOffsetY, const NFmiMetTime &theInterpolationTime, bool useInterpolatedTime);

 protected:
  NFmiCalculationCondition itsMaskCondition;
  Type itsMaskType;
  NFmiInfoData::Type itsDataType;
  CalculationOperationType itsCalculationOperationType;
  // Jos maskeja lista, tämän operaation mukaan lasketaan maskit yhteen esim. NOT, AND ja OR
  CalculationOperator
      itsCalculationOperator;  // myös smarttool systeemi, pitäisi suunnitella uusiksi!
  BinaryOperator itsPostBinaryOperator;
  MathFunctionType itsMathFunctionType;
  FunctionType itsFunctionType;  // onko mahd. funktio esim. min, max jne. (ei matemaattisia
                                 // funktioita kuten sin, cos, pow, jne.)
  FunctionType itsSecondaryFunctionType;  // tässä on ainakin vertikaali funktioiden lasku tapa
                                          // VertP, VertZ, VertFL ja VertHyb eli missä vertikaali
                                          // asteikossa operoidaan
  MetFunctionDirection itsMetFunctionDirection;  // grad, adv, div rot ja lap -funktioille (ja
  // näiden 2-versioille) määrätään myös suunta, joka
  // voi olla X, Y tai molemmat
  int itsIntegrationFunctionType;  // 1=SumT tyylinen ja 2=SumZ tyylinen ja 3=MinH tyylinen funktio
  int itsFunctionArgumentCount;
  bool fHasSubMasks;
  bool fEnabled;
  boost::shared_ptr<NFmiSimpleCondition> itsSimpleCondition;
};  // class NFmiAreaMaskImpl

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiAreaMaskImpl::IsMasked(int /* theIndex */) const { return false; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiAreaMaskImpl::IsEnabled(void) const { return fEnabled; }
// ----------------------------------------------------------------------
/*!
 * \param theNewState Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::Enable(bool theNewState) { fEnabled = theNewState; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiCalculationCondition &NFmiAreaMaskImpl::Condition(void) const
{
  return itsMaskCondition;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline boost::shared_ptr<NFmiFastQueryInfo> NFmiAreaMaskImpl::Info(void)
{
  return boost::shared_ptr<NFmiFastQueryInfo>();
}

inline void NFmiAreaMaskImpl::UpdateInfo(boost::shared_ptr<NFmiFastQueryInfo> & /* theInfo */) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiInfoData::Type NFmiAreaMaskImpl::GetDataType(void) const { return itsDataType; }
// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::SetDataType(NFmiInfoData::Type theType) { itsDataType = theType; }
// ----------------------------------------------------------------------
/*!
 * \param theType Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::MaskType(Type theType) { itsMaskType = theType; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::Type NFmiAreaMaskImpl::MaskType(void) const { return itsMaskType; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMask *NFmiAreaMaskImpl::Clone(void) const { return new NFmiAreaMaskImpl(*this); }
// ----------------------------------------------------------------------
/*!
 * \param newOperator Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::PostBinaryOperator(BinaryOperator newOperator)
{
  itsPostBinaryOperator = newOperator;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::BinaryOperator NFmiAreaMaskImpl::PostBinaryOperator(void) const
{
  return itsPostBinaryOperator;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::CalculationOperationType NFmiAreaMaskImpl::GetCalculationOperationType(
    void) const
{
  return itsCalculationOperationType;
}

// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::SetCalculationOperationType(CalculationOperationType newValue)
{
  itsCalculationOperationType = newValue;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::CalculationOperator NFmiAreaMaskImpl::GetCalculationOperator(void) const
{
  return itsCalculationOperator;
}

// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::SetCalculationOperator(CalculationOperator newValue)
{
  itsCalculationOperator = newValue;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMaskImpl::MathFunctionType NFmiAreaMaskImpl::GetMathFunctionType(void) const
{
  return itsMathFunctionType;
}

// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::SetMathFunctionType(MathFunctionType newValue)
{
  itsMathFunctionType = newValue;
}

// ----------------------------------------------------------------------
/*!
 * \param theLowerLimit Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::LowerLimit(double theLowerLimit)
{
  itsMaskCondition.LowerLimit(theLowerLimit);
}

// ----------------------------------------------------------------------
/*!
 * \param theUpperLimit Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::UpperLimit(double theUpperLimit)
{
  itsMaskCondition.UpperLimit(theUpperLimit);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAreaMaskImpl::LowerLimit(void) const { return itsMaskCondition.LowerLimit(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAreaMaskImpl::UpperLimit(void) const { return itsMaskCondition.UpperLimit(); }
// ----------------------------------------------------------------------
/*!
 * \param theMaskOperation Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiAreaMaskImpl::MaskOperation(FmiMaskOperation theMaskOperation)
{
  itsMaskCondition.Condition(theMaskOperation);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiMaskOperation NFmiAreaMaskImpl::MaskOperation(void) const
{
  return itsMaskCondition.Condition();
}

// ======================================================================
