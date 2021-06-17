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
#include "NFmiAreaMaskHelperStructures.h"

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
                   BinaryOperator thePostBinaryOperator);
  NFmiAreaMaskImpl(const NFmiAreaMaskImpl &theOther);
  void Initialize(void) override;

  bool IsMasked(int theIndex) const override;
  bool IsMasked(const NFmiPoint &theLatLon) const override;
  double MaskValue(const NFmiPoint &theLatLon) const override;
  double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways) override;
  // oletuksenä HeightValue palauttaa saman kuin Value-metodi, homma overridataan vain
  // NFmiInfoAreaMask-luokassa
  double HeightValue(double /* theHeight */, const NFmiCalculationParams &theCalculationParams) override
  {
    // Poikkileikkauslaskuissa pitää aina käyttää 'aikainterpolaatiota', muuten ei (koska laskussa käytetyt ajat on jo asetettu muualla)
    bool useTimeInterpolationAlways = theCalculationParams.fCrossSectionCase;
    return Value(theCalculationParams, useTimeInterpolationAlways);
  }
  // oletuksenä PressureValue palauttaa saman kuin Value-metodi, homma overridataan vain
  // NFmiInfoAreaMask-luokassa
  double PressureValue(double /* thePressure */, const NFmiCalculationParams &theCalculationParams) override
  {
    // Poikkileikkauslaskuissa pitää aina käyttää 'aikainterpolaatiota', muuten ei (koska laskussa käytetyt ajat on jo asetettu muualla)
    bool useTimeInterpolationAlways = theCalculationParams.fCrossSectionCase;
    return Value(theCalculationParams, useTimeInterpolationAlways);
  }

  bool IsEnabled(void) const override;
  void Enable(bool theNewState) override;

  bool Time(const NFmiMetTime &theTime) override;
  void Condition(const NFmiCalculationCondition &theCondition) override;
  const NFmiCalculationCondition &Condition(void) const override;

  bool IsRampMask(void) const override;
  bool IsWantedParam(const NFmiDataIdent &theParam, const NFmiLevel *theLevel = 0) const override;
  const NFmiString MaskString(void) const override;
  boost::shared_ptr<NFmiFastQueryInfo> Info(void) override;
  void UpdateInfo(boost::shared_ptr<NFmiFastQueryInfo> &theInfo) override;
  const NFmiDataIdent *DataIdent(void) const override;
  const NFmiParam *Param(void) const override;
  const NFmiLevel *Level(void) const override;
  void Level(const NFmiLevel &theLevel) override;
  NFmiInfoData::Type GetDataType(void) const override;
  void SetDataType(NFmiInfoData::Type theType) override;
  bool UseLevelInfo(void) const override;
  bool UsePressureLevelInterpolation(void) const override;
  void UsePressureLevelInterpolation(bool newValue) override;
  double UsedPressureLevelValue(void) const override;
  void UsedPressureLevelValue(double newValue) override;

  void LowerLimit(double theLowerLimit) override;
  void UpperLimit(double theUpperLimit) override;
  double LowerLimit(void) const override;
  double UpperLimit(void) const override;
  void MaskOperation(FmiMaskOperation theMaskOperation) override;
  FmiMaskOperation MaskOperation(void) const override;

  bool AddMask(NFmiAreaMask *theMask) override;
  NFmiAreaMask *AreaMask(int theIndex) const override;
  bool RemoveSubMask(int theIndex) override;
  void MaskType(Type theType) override;
  Type MaskType(void) const override;
  NFmiAreaMask *Clone(void) const override;
  void PostBinaryOperator(BinaryOperator newOperator) override;
  BinaryOperator PostBinaryOperator(void) const override;
  CalculationOperationType GetCalculationOperationType(void) const override;
  void SetCalculationOperationType(CalculationOperationType newValue) override;
  CalculationOperator GetCalculationOperator(void) const override;
  void SetCalculationOperator(CalculationOperator newValue) override;
  MathFunctionType GetMathFunctionType(void) const override;
  void SetMathFunctionType(MathFunctionType newValue) override;
  FunctionType GetFunctionType(void) const override { return itsFunctionType; }
  void SetFunctionType(FunctionType newType) override { itsFunctionType = newType; }
  FunctionType GetSecondaryFunctionType(void) const override { return itsSecondaryFunctionType; }
  void SetSecondaryFunctionType(FunctionType newType) override { itsSecondaryFunctionType = newType; }
  MetFunctionDirection GetMetFunctionDirection(void) const override { return itsMetFunctionDirection; }
  void GetMetFunctionDirection(MetFunctionDirection newValue) override
  {
    itsMetFunctionDirection = newValue;
  }
  int IntegrationFunctionType(void) const override { return itsIntegrationFunctionType; }
  void IntegrationFunctionType(int newValue) override { itsIntegrationFunctionType = newValue; }
  void SetArguments(std::vector<float> & /* theArgumentVector */) override {};
  int FunctionArgumentCount(void) const override { return itsFunctionArgumentCount; }
  void FunctionArgumentCount(int newValue) override { itsFunctionArgumentCount = newValue; }
  // HUOM! seuraavat toimivat oikeasti vain NFmiBinaryMask:in kanssa. Tässä vain tyhjät oletus
  // toteutukset.
  void SetAll(bool /* theNewState */) override {};
  void Mask(int /* theIndex */, bool /* newStatus */) override {};
  const boost::shared_ptr<NFmiSimpleCondition>& SimpleCondition() const override { return itsSimpleCondition; }
  void SimpleCondition(boost::shared_ptr<NFmiSimpleCondition> &theSimpleCondition) override { itsSimpleCondition = theSimpleCondition; }
  float FunctionDataTimeOffsetInHours() const override { return itsFunctionDataTimeOffsetInHours; }
  void FunctionDataTimeOffsetInHours(float newValue) override { itsFunctionDataTimeOffsetInHours = newValue; }

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
  // Jossain tilanteissa smarttool funktion datalle voidaan haluta tehdä aikasiirto,
  // jolloin käytetään tämän arvoa tekemään siirto, esim. peekxy3(T_ec[-3h] 10 0)
  // kutsussa tehdään funktiole annettavalle parametrille T_ec 3 tunnin siirto taaksepäin.
  float itsFunctionDataTimeOffsetInHours = 0;
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
