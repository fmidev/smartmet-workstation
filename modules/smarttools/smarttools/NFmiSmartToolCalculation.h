#pragma once
//**********************************************************
// C++ Class Name : NFmiSmartToolCalculation
// ---------------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Thur - Jun 20, 2002
//
// Tämä luokka hoitaa yhden laskurivin esim. T = T + 1
//**********************************************************

#include <boost/shared_ptr.hpp>
#include <newbase/NFmiAreaMask.h>
#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiPoint.h>

class NFmiFastQueryInfo;
class NFmiDataModifier;

class NFmiSmartToolCalculation
{
 public:
  bool IsMasked(const NFmiCalculationParams &theCalculationParams);
  void Calculate(const NFmiCalculationParams &theCalculationParams,
                 NFmiMacroParamValue &theMacroParamValue);
  void Calculate_ver2(const NFmiCalculationParams &theCalculationParams);
  void Time(const NFmiMetTime &theTime);  // optimointia laskuja varten

  NFmiSmartToolCalculation();
  NFmiSmartToolCalculation(const NFmiSmartToolCalculation &theOther);
  ~NFmiSmartToolCalculation();
  static std::vector<boost::shared_ptr<NFmiSmartToolCalculation> > DoShallowCopy(
      const std::vector<boost::shared_ptr<NFmiSmartToolCalculation> > &theCalculationVector);

  void SetResultInfo(const boost::shared_ptr<NFmiFastQueryInfo> &value)
  {
    itsResultInfo = value;
    CheckIfModularParameter();
  }
  boost::shared_ptr<NFmiFastQueryInfo> GetResultInfo() { return itsResultInfo; }
  std::vector<boost::shared_ptr<NFmiAreaMask> > &GetCalculations() { return itsCalculations; }
  void AddCalculation(const boost::shared_ptr<NFmiAreaMask> &theCalculation);
  const std::string &GetCalculationText() { return itsCalculationText; }
  void SetCalculationText(const std::string &theText) { itsCalculationText = theText; }
  void SetLimits(float theLowerLimit, float theUpperLimit, bool theDoLimitCheck);
  bool AllowMissingValueAssignment() { return fAllowMissingValueAssignment; };
  void AllowMissingValueAssignment(bool newState) { fAllowMissingValueAssignment = newState; };

 private:
  std::string itsCalculationText;  // originaali teksti, mistä tämä lasku on tulkittu
  typedef std::vector<boost::shared_ptr<NFmiAreaMask> >::iterator CalcIter;

  float GetInsideLimitsValue(float theValue);
  float itsLowerLimit;  // näiden avulla kontrolloidaan mahdollisia min ja max arvoja
  float itsUpperLimit;
  bool
      fDoLimitCheck;  // kaikille parametreille ei tehdä rajojen tarkistusta, esim. TotalWind ja W&C

  // eval_exp-metodit otettu H. Schilbertin  C++: the Complete Refeference third ed.
  // jouduin muuttamaan niitä vähän sopimaan tähän ympäristöön.
  double eval_exp(const NFmiCalculationParams &theCalculationParams);
  void eval_exp1(double &result, const NFmiCalculationParams &theCalculationParams);
  void eval_exp2(double &result, const NFmiCalculationParams &theCalculationParams);
  void eval_exp3(double &result, const NFmiCalculationParams &theCalculationParams);
  void eval_exp4(double &result, const NFmiCalculationParams &theCalculationParams);
  void eval_exp5(double &result, const NFmiCalculationParams &theCalculationParams);
  void eval_exp6(double &result, const NFmiCalculationParams &theCalculationParams);
  void eval_math_function(double &result, int theFunction);
  void eval_ThreeArgumentFunction(double &result,
                                  double argument1,
                                  double argument2,
                                  NFmiAreaMask::FunctionType func,
                                  int theIntegrationFunctionType,
                                  const NFmiCalculationParams &theCalculationParams);
  void eval_ThreeArgumentFunctionZ(double &result,
                                   double argument1,
                                   double argument2,
                                   NFmiAreaMask::FunctionType func,
                                   int theIntegrationFunctionType,
                                   const NFmiCalculationParams &theCalculationParams);
  void atom(double &result, const NFmiCalculationParams &theCalculationParams);
  void get_token();
  boost::shared_ptr<NFmiAreaMask> token;  // tässä on kulloinenkin laskun osa tarkastelussa
  CalcIter itsCalcIterator;               // get_token siirtää tätä
  // Lisäksi piti maskia varten binääri versio evaluaatio systeemistä
  bool bin_eval_exp(const NFmiCalculationParams &theCalculationParams);
  void bin_eval_exp1(bool &maskresult,
                     double &result,
                     const NFmiCalculationParams &theCalculationParams);
  void bin_eval_exp1_1(bool &maskresult,
                       double &result,
                       const NFmiCalculationParams &theCalculationParams);
  void bin_eval_exp1_2(bool &maskresult,
                       double &result,
                       const NFmiCalculationParams &theCalculationParams);
  void bin_eval_exp2(bool &maskresult,
                     double &result,
                     const NFmiCalculationParams &theCalculationParams);
  void bin_eval_exp3(bool &maskresult,
                     double &result,
                     const NFmiCalculationParams &theCalculationParams);
  void bin_eval_exp4(bool &maskresult,
                     double &result,
                     const NFmiCalculationParams &theCalculationParams);
  void bin_eval_exp5(bool &maskresult,
                     double &result,
                     const NFmiCalculationParams &theCalculationParams);
  void bin_eval_exp6(bool &maskresult,
                     double &result,
                     const NFmiCalculationParams &theCalculationParams);
  void bin_atom(bool &maskresult,
                double &result,
                const NFmiCalculationParams &theCalculationParams);
  void CalcThreeArgumentFunction(double &result, const NFmiCalculationParams &theCalculationParams);
  void CalcVertFunction(double &result, const NFmiCalculationParams &theCalculationParams);

  boost::shared_ptr<NFmiFastQueryInfo> itsResultInfo;             // omistaa+tuhoaa
  std::vector<boost::shared_ptr<NFmiAreaMask> > itsCalculations;  // omistaa+tuhoaa
  float itsHeightValue;
  float itsPressureHeightValue;

  bool fUseTimeInterpolationAlways;  // uudet MINT, MAXT, jne vaativat aina aikainterpolointia, ja
                                     // tämä flagi laitetaan silloin päälle
  // (tämä on jo käytössä olevan optimoinnin toimivuuden kannalta pakko tehdä näin)
  bool fUseHeightCalculation;  // atom-metodi kutsuu HeightValue-metodia, jos tämä on päällä
  bool
      fUsePressureLevelCalculation;  // atom-metodi kutsuu PressureValue-metodia, jos tämä on päällä
  bool fAllowMissingValueAssignment;

  // tuulen suuntaa varten pitää tehdä virityksiä, että esim. 350 + 20 olisi 10 eikä 360 (eli
  // maksimi) jne.
  bool fCircularValue;
  double itsCircularValueModulor;
  void CheckIfModularParameter();
  double FixCircularValues(double theValue);
  bool IsSpecialCalculationVariableCase(const NFmiCalculationParams &theCalculationParams);
  double SpecialCalculationVariableCaseValue(const NFmiCalculationParams &theCalculationParams);
};
