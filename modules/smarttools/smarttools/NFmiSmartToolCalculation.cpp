// ======================================================================
/*!
 *
 * C++ Class Name : NFmiSmartToolCalculation2
 * ---------------------------------------------------------
 * Filetype: (SOURCE)
 * Filepath: G:/siirto/marko/oc/NFmiSmartToolCalculation2.cpp
 *
 *
 * GDPro Properties
 * ---------------------------------------------------
 *  - GD Symbol Type    : CLD_Class
 *  - GD Method         : UML ( 4.0 )
 *  - GD System Name    : aSmartTools
 *  - GD View Type      : Class Diagram
 *  - GD View Name      : smarttools 1
 * ---------------------------------------------------
 *  Author         : pietarin
 *  Creation Date  : Thur - Jun 20, 2002
 *
 *  Change Log     :
 *
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4786)  // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiSmartToolCalculation.h"
#include "NFmiAreaMaskInfo.h"
#include "NFmiCalculationConstantValue.h"
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiDataModifierClasses.h>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include "NFmiDictionaryFunction.h"

using namespace std;

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiSmartToolCalculation::NFmiSmartToolCalculation(void)
    : itsCalculationText(),
      itsLowerLimit(0),
      itsUpperLimit(1),
      fDoLimitCheck(true),
      token(),
      itsCalcIterator(),
      itsResultInfo(),
      itsCalculations(),
      itsHeightValue(0),
      itsPressureHeightValue(1000),
      fUseTimeInterpolationAlways(false),
      fUseHeightCalculation(false),
      fUsePressureLevelCalculation(false),
      fAllowMissingValueAssignment(false),
      fCircularValue(false),
      itsCircularValueModulor(kFloatMissing)
{
}

NFmiSmartToolCalculation::NFmiSmartToolCalculation(const NFmiSmartToolCalculation &theOther)
    : itsCalculationText(theOther.itsCalculationText),
      itsLowerLimit(theOther.itsLowerLimit),
      itsUpperLimit(theOther.itsUpperLimit),
      fDoLimitCheck(theOther.fDoLimitCheck),
      token()  // tätä ei mielestäni pidä kopioida
      ,
      itsCalcIterator()  // tätä ei mielestäni pidä kopioida
      ,
      itsResultInfo(NFmiAreaMask::DoShallowCopy(theOther.itsResultInfo)),
      itsCalculations(NFmiAreaMask::DoShallowCopy(theOther.itsCalculations)),
      itsHeightValue(theOther.itsHeightValue),
      itsPressureHeightValue(theOther.itsPressureHeightValue),
      fUseTimeInterpolationAlways(theOther.fUseTimeInterpolationAlways),
      fUseHeightCalculation(theOther.fUseHeightCalculation),
      fUsePressureLevelCalculation(theOther.fUsePressureLevelCalculation),
      fAllowMissingValueAssignment(theOther.fAllowMissingValueAssignment),
      fCircularValue(theOther.fCircularValue),
      itsCircularValueModulor(theOther.itsCircularValueModulor)
{
}

NFmiSmartToolCalculation::~NFmiSmartToolCalculation(void)
{
}

//--------------------------------------------------------
// Calculate
//--------------------------------------------------------
void NFmiSmartToolCalculation::Calculate(const NFmiCalculationParams &theCalculationParams,
                                         NFmiMacroParamValue &theMacroParamValue)
{
  if (theMacroParamValue.fDoCrossSectionCalculations)
  {
    fUsePressureLevelCalculation = true;
    itsPressureHeightValue = theMacroParamValue.itsPressureHeight;
  }
  double value = eval_exp(theCalculationParams);

  //	value = FmiMakeValidNumber(value); // NFmiQueryInfo::FloatValue(value) -metodi tarkastaa
  // nykyään onko kyseessä sallittu luku, inf ja nan -arvojen asetus on nykyään estetty
  if (value != kFloatMissing)  // tuli ongelmia missing asetuksissa, pitää mietti vaikka jokin
  // funktio, jolla asetetaan puuttuva arvo // pitää pystyä sittenkin
  // asettamaan arvoksi kFloatMissing:in!!!
  {
    itsResultInfo->LocationIndex(theCalculationParams.itsLocationIndex);  // kohde dataa
                                                                          // juoksutetaan, joten
                                                                          // lokaatio indeksien
    // pitää olla synkassa!!!
    value = FixCircularValues(value);  // ensin tehdään circular tarkistus ja sitten vasta min/max
    value = GetInsideLimitsValue(static_cast<float>(value));  // asetetaan value vielä drawparamista
                                                              // satuihin rajoihin, ettei esim. RH
                                                              // voi olla alle 0 tai yli 100 %

    itsResultInfo->FloatValue(
        static_cast<float>(value));  // miten info saadaan osoittamaan oikeaan kohtaan?!?
  }
  else
  {
    if (fAllowMissingValueAssignment)
    {
      itsResultInfo->LocationIndex(theCalculationParams.itsLocationIndex);  // kohde dataa
                                                                            // juoksutetaan, joten
                                                                            // lokaatio indeksien
                                                                            // pitää olla
                                                                            // synkassa!!!
      itsResultInfo->FloatValue(
          static_cast<float>(value));  // nyt voidaan asettaa puuttuva arvo dataan
    }
  }
  // lopuksi asetetaan macroParamValue, jos niin on haluttu ja resultInfo on macroParam-tyyppiä
  NFmiInfoData::Type dataType = itsResultInfo->DataType();
  if (theMacroParamValue.fSetValue &&
      (dataType == NFmiInfoData::kMacroParam || dataType == NFmiInfoData::kCrossSectionMacroParam ||
       dataType == NFmiInfoData::kScriptVariableData))
    theMacroParamValue.itsValue = static_cast<float>(value);
}

void NFmiSmartToolCalculation::Calculate_ver2(const NFmiCalculationParams &theCalculationParams)
{
  double value = eval_exp(theCalculationParams);

  //	value = FmiMakeValidNumber(value); // NFmiQueryInfo::FloatValue(value) -metodi tarkastaa
  // nykyään onko kyseessä sallittu luku, inf ja nan -arvojen asetus on nykyään estetty
  if (value != kFloatMissing)  // tuli ongelmia missing asetuksissa, pitää mietti vaikka jokin
  // funktio, jolla asetetaan puuttuva arvo // pitää pystyä sittenkin
  // asettamaan arvoksi kFloatMissing:in!!!
  {
    itsResultInfo->LocationIndex(theCalculationParams.itsLocationIndex);  // kohde dataa
                                                                          // juoksutetaan, joten
                                                                          // lokaatio indeksien
    // pitää olla synkassa!!!
    value = FixCircularValues(value);  // ensin tehdään circular tarkistus ja sitten vasta min/max
    value = GetInsideLimitsValue(static_cast<float>(value));  // asetetaan value vielä drawparamista
                                                              // satuihin rajoihin, ettei esim. RH
                                                              // voi olla alle 0 tai yli 100 %

    itsResultInfo->FloatValue(
        static_cast<float>(value));  // miten info saadaan osoittamaan oikeaan kohtaan?!?
  }
  else
  {
    if (fAllowMissingValueAssignment)
    {
      itsResultInfo->LocationIndex(theCalculationParams.itsLocationIndex);  // kohde dataa
                                                                            // juoksutetaan, joten
                                                                            // lokaatio indeksien
                                                                            // pitää olla
                                                                            // synkassa!!!
      itsResultInfo->FloatValue(
          static_cast<float>(value));  // nyt voidaan asettaa puuttuva arvo dataan
    }
  }
}

// ei ota huomioon missing arvoa, koska se pitää ottaa huomioon jo ennen tämän kutsua.
float NFmiSmartToolCalculation::GetInsideLimitsValue(float theValue)
{
  if (theValue == kFloatMissing)
    return theValue;

  if (fDoLimitCheck)
  {
    if (itsLowerLimit != kFloatMissing && theValue < itsLowerLimit)
      return itsLowerLimit;
    else if (itsUpperLimit != kFloatMissing && theValue > itsUpperLimit)
      return itsUpperLimit;
  }
  return theValue;
}

void NFmiSmartToolCalculation::SetLimits(float theLowerLimit,
                                         float theUpperLimit,
                                         bool theDoLimitCheck)
{
  fDoLimitCheck = theDoLimitCheck;
  if (fDoLimitCheck)
  {
    if ((theLowerLimit != kFloatMissing && theUpperLimit != kFloatMissing) &&
        theLowerLimit >= theUpperLimit)
      throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorMinMaxLimits"));
    else
    {
      itsLowerLimit = theLowerLimit;
      itsUpperLimit = theUpperLimit;
    }
  }
}

bool NFmiSmartToolCalculation::IsMasked(const NFmiCalculationParams &theCalculationParams)
{
  return bin_eval_exp(theCalculationParams);
}

void NFmiSmartToolCalculation::AddCalculation(const boost::shared_ptr<NFmiAreaMask> &theCalculation)
{
  if (theCalculation)
  {
    itsCalculations.push_back(theCalculation);
  }
}

// globaali asetus luokka for_each-funktioon
struct TimeSetter
{
  TimeSetter(const NFmiMetTime &theTime) : itsTime(theTime) {}
  void operator()(boost::shared_ptr<NFmiAreaMask> &theMask) { theMask->Time(itsTime); }
  const NFmiMetTime &itsTime;
};

void NFmiSmartToolCalculation::Time(const NFmiMetTime &theTime)
{
  if (itsResultInfo)  // maskin tapauksessa tämä on 0-pointteri (ja tulevaisuudessa tämä sijoitus
                      // info poistuu kokonaisuudessaan)
    itsResultInfo->Time(theTime);  // tämän ajan asetuksen pitää onnistua editoitavalle datalle

  std::for_each(itsCalculations.begin(), itsCalculations.end(), TimeSetter(theTime));
}

// eval_exp-metodit otettu H. Schilbertin  C++: the Complete Refeference third ed.
// jouduin muuttamaan niitä vähän sopimaan tähän ympäristöön.
double NFmiSmartToolCalculation::eval_exp(const NFmiCalculationParams &theCalculationParams)
{
  double result = kFloatMissing;

  token = boost::shared_ptr<NFmiAreaMask>();  // nollataan aluksi 'token'
  itsCalcIterator = itsCalculations.begin();

  get_token();
  if (!token || token->GetCalculationOperationType() == NFmiAreaMask::NoType ||
      token->GetCalculationOperationType() == NFmiAreaMask::EndOfOperations)
    throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorNoCalculations") + ":\n" +
                        GetCalculationText());

  eval_exp1(result, theCalculationParams);
  if (token->GetCalculationOperationType() != NFmiAreaMask::EndOfOperations)
    throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidEnding") + ":\n" +
                        GetCalculationText());
  return result;
}

// Process an assignment.
// Tämä on jo hoidettu toisella tavalla omassa koodissa, joten ohitan tämän,
// mutta jätän kommentteihin, jos tarvitsen tulevaisuudessa.
void NFmiSmartToolCalculation::eval_exp1(double &result,
                                         const NFmiCalculationParams &theCalculationParams)
{
  eval_exp2(result, theCalculationParams);
}

// Add or subtract two terms.
void NFmiSmartToolCalculation::eval_exp2(double &result,
                                         const NFmiCalculationParams &theCalculationParams)
{
  NFmiAreaMask::CalculationOperator op;
  double temp;

  eval_exp3(result, theCalculationParams);
  while ((op = token->GetCalculationOperator()) == NFmiAreaMask::Add || op == NFmiAreaMask::Sub)
  {
    get_token();
    eval_exp3(temp, theCalculationParams);

    if (result == kFloatMissing || temp == kFloatMissing)
      result = kFloatMissing;
    else if (op == NFmiAreaMask::Sub)
      result = result - temp;
    else  // Add
      result = result + temp;
  }
}

// Multiply or divide two factors.
void NFmiSmartToolCalculation::eval_exp3(double &result,
                                         const NFmiCalculationParams &theCalculationParams)
{
  NFmiAreaMask::CalculationOperator op;
  double temp;

  eval_exp4(result, theCalculationParams);
  while ((op = token->GetCalculationOperator()) == NFmiAreaMask::Mul || op == NFmiAreaMask::Div ||
         op == NFmiAreaMask::Mod)
  {
    get_token();
    eval_exp4(temp, theCalculationParams);

    if (result == kFloatMissing || temp == kFloatMissing)
      result = kFloatMissing;
    else if (op == NFmiAreaMask::Mul)
      result = result * temp;
    else if (op == NFmiAreaMask::Div)
      result = (temp == 0 ? kFloatMissing : result / temp);
    else  // NFmiAreaMask::Mod:
      result = temp == 0 ? kFloatMissing : static_cast<int>(result) % static_cast<int>(temp);
  }
}

// Process an exponent
void NFmiSmartToolCalculation::eval_exp4(double &result,
                                         const NFmiCalculationParams &theCalculationParams)
{
  double temp;

  eval_exp5(result, theCalculationParams);
  if (token->GetCalculationOperator() == NFmiAreaMask::Pow)
  {
    get_token();
    eval_exp4(temp, theCalculationParams);
    if (temp == 0.0)
    {
      result = 1.0;
      return;
    }
    if (result == kFloatMissing || temp == kFloatMissing)
      result = kFloatMissing;
    else
      result = pow(result, temp);
  }
}

// Evaluate a unary + or -.
void NFmiSmartToolCalculation::eval_exp5(double &result,
                                         const NFmiCalculationParams &theCalculationParams)
{
  NFmiAreaMask::CalculationOperator op = token->GetCalculationOperator();
  if (op == NFmiAreaMask::Add || op == NFmiAreaMask::Sub)
    get_token();
  eval_exp6(result, theCalculationParams);

  if (op == NFmiAreaMask::Sub && result != kFloatMissing)
    result = -result;
}

#if 0
static void DEBUGOUT2(bool doOutPut, const string &theComment, double theValue)
{
	static bool firstTime = true;
	static ofstream out;
	if(firstTime)
	{
		out.open("d:\\debugout.txt");
		firstTime = false;
	}
	if(doOutPut)
	{
		out << theComment << " " << NFmiStringTools::Convert<double>(theValue) << endl;
	}
}
#endif

// Process a parenthesized expression.
void NFmiSmartToolCalculation::eval_exp6(double &result,
                                         const NFmiCalculationParams &theCalculationParams)
{
  if (token->GetCalculationOperationType() == NFmiAreaMask::StartParenthesis)
  {
    get_token();
    eval_exp2(result, theCalculationParams);
    if (token->GetCalculationOperationType() != NFmiAreaMask::EndParenthesis)
      throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidParenthesis") +
                          ":\n" + GetCalculationText());
    get_token();
  }
  else if (token->GetCalculationOperationType() == NFmiAreaMask::MathFunctionStart)
  {
    NFmiAreaMask::MathFunctionType function = token->GetMathFunctionType();
    get_token();
    eval_exp2(result, theCalculationParams);
    if (token->GetCalculationOperationType() !=
        NFmiAreaMask::EndParenthesis)  // MathFunctionStart:in päättää normaali lopetus sulku!
      throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidMathParenthesis") +
                          ":\n" + GetCalculationText());
    eval_math_function(result, function);
    get_token();
  }
  else if (token->GetCalculationOperationType() == NFmiAreaMask::ThreeArgumentFunctionStart)
    CalcThreeArgumentFunction(result, theCalculationParams);
  else if (token->GetCalculationOperationType() == NFmiAreaMask::VertFunctionStart)
    CalcVertFunction(result, theCalculationParams);
  else
    atom(result, theCalculationParams);
}

void NFmiSmartToolCalculation::eval_ThreeArgumentFunction(
    double &result,
    double argument1,
    double argument2,
    NFmiAreaMask::FunctionType func,
    int theIntegrationFunctionType,
    const NFmiCalculationParams &theCalculationParams)
{
  result = kFloatMissing;
  if (theIntegrationFunctionType == 2 || theIntegrationFunctionType == 3)
    eval_ThreeArgumentFunctionZ(
        result, argument1, argument2, func, theIntegrationFunctionType, theCalculationParams);
  else
  {
    if (argument1 != kFloatMissing && argument2 != kFloatMissing)
    {
      double value = kFloatMissing;
      // 1. ota talteen token tai se iteraattori, että samoja laskuja voidaan käydä läpi uudestaan
      // ja uudestaan
      CalcIter startCalcIterator =
          itsCalcIterator;  // pitääkö olla edellinen??, pitää olla epäselvää, mutta pakko
      // 2. katso onko kyseessä aika- vai korkeus lasku ja haaraudu
      // 3. jos aikalasku, laske alkuaika ja loppu aika
      NFmiMetTime startTime(theCalculationParams.itsTime);
      startTime.ChangeByHours(static_cast<long>(argument1));
      NFmiMetTime endTime(theCalculationParams.itsTime);
      endTime.ChangeByHours(static_cast<long>(argument2));
      NFmiMetTime currentTime(startTime);
      // 4. mieti mikä on aikahyppy (oletus 1h) jos pelkkää EC-dataa, voisi aikahyppy olla 3h tai 6h
      int usedTimeResolutionInMinutes = 60;
      if (startTime <= endTime)
      {
        if (endTime.DifferenceInMinutes(startTime) / usedTimeResolutionInMinutes > 250)
          throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorTimeCalcOverRun"));
        // 5. funktiosta riippuva datamodifier min, max jne.
        boost::shared_ptr<NFmiDataModifier> modifier =
            NFmiInfoAreaMask::CreateIntegrationFuction(func);  // tämä palauttaa aina jotain, tai heittää poikkeuksen
        try
        {
          NFmiCalculationParams modifiedCalculationParams(theCalculationParams);
          fUseTimeInterpolationAlways = true;
          do
          {
            modifiedCalculationParams.itsTime = currentTime;
            // 6. muista aina asettaa token/laskuiteraattori 'alkuun'
            itsCalcIterator = startCalcIterator;
            get_token();  // pitää tehdä vielä tämä, muuten osoittaa edelliseen pilkku-operaattoriin
            // 7. käy aika-loopissa läpi eval_exp2-laskut
            eval_exp2(value, modifiedCalculationParams);  // theTimeIndex on nyt puppua
            // 8. sijoita tulos datamodifier:iin
            modifier->Calculate(static_cast<float>(value));
            // 9. 'next'
            currentTime.ChangeByMinutes(usedTimeResolutionInMinutes);
          } while (currentTime <= endTime);
          // 10. loopin lopuksi pyydä result datamodifier:ilta
          result = modifier->CalculationResult();
          fUseTimeInterpolationAlways = false;
        }
        catch (...)
        {
          fUseTimeInterpolationAlways = false;
          throw;
        }
      }
      // Jos kyse level laskuista, juoksuta korkeuksia/leveleitä jotenkin ja tee samaa
    }
  }
}

static float GetCurrentHeightStep(float theHeight)
{
  float step = 100;
  if (theHeight < 150)
    step = 30;
  else if (theHeight < 300)
    step = 50;
  else if (theHeight < 1000)
    step = 100;
  else if (theHeight < 3000)
    step = 200;
  else if (theHeight < 10000)
    step = 500;
  else if (theHeight < 16000)
    step = 1000;
  else
    step = 2000;

  return step;
}

template <typename T>
static bool IsEqualEnough(T value1, T value2, T usedEpsilon)
{
  if (::fabs(static_cast<double>(value1 - value2)) < usedEpsilon)
    return true;
  return false;
}
void NFmiSmartToolCalculation::eval_ThreeArgumentFunctionZ(
    double &result,
    double argument1,
    double argument2,
    NFmiAreaMask::FunctionType func,
    int theIntegrationFunctionType,
    const NFmiCalculationParams &theCalculationParams)
{
  result = kFloatMissing;
  if (argument1 != kFloatMissing && argument2 != kFloatMissing)
  {
    double value = kFloatMissing;
    double heightValue =
        kFloatMissing;  // tähän talletetaan h-funktion tapauksessa minimin/maksimin korkeus
    // 1. ota talteen token tai se iteraattori, että samoja laskuja voidaan käydä läpi uudestaan ja
    // uudestaan
    CalcIter startCalcIterator =
        itsCalcIterator;  // pitääkö olla edellinen??, pitää olla epäselvää, mutta pakko
    // 3. jos korkeus lasku
    itsHeightValue = static_cast<float>(argument1);
    // 4. mieti mikä on aikahyppy (oletus 1h) jos pelkkää EC-dataa, voisi aikahyppy olla 3h tai 6h
    // HUOM!! Muuta resoluutio siten, että 50, kun korkeus alle 500 m, 100, kun korkeus alle 3000 m,
    // 200 kun korkeus alle 10000 m ja 500 kun korkeus yli sen
    float usedHeightResolution = 100;
    if (argument1 >= 0 && argument1 <= argument2)
    {
      if ((argument2 - argument1) > 35000)
        throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorHeightCalcOverRun"));
      // 5. funktiosta riippuva datamodifier min, max jne.
      boost::shared_ptr<NFmiDataModifier> modifier =
          NFmiInfoAreaMask::CreateIntegrationFuction(func);  // tämä palauttaa aina jotain, tai heittää poikkeuksen
      try
      {
        fUseHeightCalculation = true;
        do
        {
          usedHeightResolution = GetCurrentHeightStep(itsHeightValue);
          NFmiCalculationDeltaZValue::SetDeltaZValue(usedHeightResolution);
          // 6. muista aina asettaa token/laskuiteraattori 'alkuun'
          itsCalcIterator = startCalcIterator;
          get_token();  // pitää tehdä vielä tämä, muuten osoittaa edelliseen pilkku-operaattoriin
          // 7. käy aika-loopissa läpi eval_exp2-laskut
          eval_exp2(value, theCalculationParams);  // theTimeIndex on nyt puppua
          // 8. sijoita tulos datamodifier:iin
          modifier->Calculate(static_cast<float>(value));
          if (theIntegrationFunctionType == 3)
          {
            double calculationResult = modifier->CalculationResult();

            // HUOM!! Tässä value vs. calculationResult vertailu pitää tehdä virherajan sisällä,
            // koska modifier käsittelee arvoja floateilla ja muu ysteemi doubleina ja
            // siitä seuraa ongelmia tarkkuuden kanssa.
            // MSVC++ 7.1 debug toimi == operaatorin kanssa, MUTTA release versio EI!!!
            // Bugin metsästykseen on mennyt arviolta eri aikoina yhteensä 10 tuntia!!!!
            // Ja ainakin neljä kertaa kun olen esitellyt SmartTool-kielen maxh-funktiota
            // olen ihmetellyt että demossa ei toimi, mutta omassa koneessa toimi (debug versiona!)
            if (value != kFloatMissing && ::IsEqualEnough(value, calculationResult, 0.000001))
            {
              heightValue = itsHeightValue;
            }
          }
          // 9. 'next'
          itsHeightValue += usedHeightResolution;
        } while (itsHeightValue <= argument2);
        // 10. loopin lopuksi pyydä result datamodifier:ilta
        if (theIntegrationFunctionType == 3)
          result = heightValue;  // eli sijoitetaan min/max arvon korkeus tulokseen jos kyseessä oli
                                 // minh/maxh -funktio
        else
          result = modifier->CalculationResult();
        fUseHeightCalculation = false;
      }
      catch (...)
      {
        fUseHeightCalculation = false;
        throw;
      }
    }
    // Jos kyse level laskuista, juoksuta korkeuksia/leveleitä jotenkin ja tee samaa
  }
}

// HUOM! trigonometriset funktiot tehdään asteille, joten annettu luku pitää konvertoida
// c++ funktioille jotka odottavat kulmaa radiaaneille.
void NFmiSmartToolCalculation::eval_math_function(double &result, int theFunction)
{
  static const double trigFactor = 2 * kPii / 360;

  if (result != kFloatMissing)
  {
    switch (NFmiAreaMask::MathFunctionType(theFunction))
    {
      case NFmiAreaMask::Exp:
        result = exp(result);
        break;
      case NFmiAreaMask::Sqrt:
        if (result >= 0)  // neliöjuurta ei saa ottaa negatiivisesta luvusta
          result = sqrt(result);
        else
          result = kFloatMissing;
        break;
      case NFmiAreaMask::Log:
        if (result > 0)  // logaritmi pitää ottaa positiivisesta luvusta
          result = log(result);
        else
          result = kFloatMissing;
        break;
      case NFmiAreaMask::Log10:
        if (result > 0)  // logaritmi pitää ottaa positiivisesta luvusta
          result = log10(result);
        else
          result = kFloatMissing;
        break;
      case NFmiAreaMask::Sin:
        result = sin(result * trigFactor);  // konversio asteista radiaaneiksi tehtävä
        break;
      case NFmiAreaMask::Cos:
        result = cos(result * trigFactor);  // konversio asteista radiaaneiksi tehtävä
        break;
      case NFmiAreaMask::Tan:
        result = tan(result * trigFactor);  // konversio asteista radiaaneiksi tehtävä
        break;
      case NFmiAreaMask::Sinh:
        result = sinh(result);
        break;
      case NFmiAreaMask::Cosh:
        result = cosh(result);
        break;
      case NFmiAreaMask::Tanh:
        result = tanh(result);
        break;
      case NFmiAreaMask::Asin:
        if (result >= -1 && result <= 1)
          result = asin(result) / trigFactor;  // konversio radiaaneista asteiksi tehtävä
        else
          result = kFloatMissing;
        break;
      case NFmiAreaMask::Acos:
        if (result >= -1 && result <= 1)
          result = acos(result) / trigFactor;  // konversio radiaaneista asteiksi tehtävä
        else
          result = kFloatMissing;
        break;
      case NFmiAreaMask::Atan:
        result = atan(result) / trigFactor;  // konversio radiaaneista asteiksi tehtävä
        break;
      case NFmiAreaMask::Ceil:
        result = ceil(result);
        break;
      case NFmiAreaMask::Floor:
        result = floor(result);
        break;
      case NFmiAreaMask::Round:
        result = round(result);
        break;
      case NFmiAreaMask::Abs:
        result = fabs(result);
        break;
      case NFmiAreaMask::Rand:
        result = (static_cast<double>(rand()) / RAND_MAX) *
                 result;  // palauttaa luvun 0 ja result:in väliltä
        break;
      default:
        throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorMathFunction") + ":\n" +
                            GetCalculationText());
    }
  }
}

bool NFmiSmartToolCalculation::IsCrossSectionVariableCase(const NFmiCalculationParams &theCalculationParams)
{
    return (theCalculationParams.fCrossSectionCase && token->GetDataType() == NFmiInfoData::Type::kScriptVariableData);
}

// Oletus: ensin tarkistetaan IsCrossSectionVariableCase metodilla onko tarvetta tälle operaatiolle.
double NFmiSmartToolCalculation::CrossSectionVariableCaseValue(const NFmiCalculationParams &theCalculationParams)
{
    // Erikoistapaus: poikkileikkaus macroParam laskuissa var -muuttuja pitää ottaa vain suoraan infosta ilman mitään interpolaatioita
    auto info = token->Info();
    if(info)
    {
        info->TimeIndex(theCalculationParams.itsTimeIndex);
        info->LocationIndex(theCalculationParams.itsLocationIndex);
        return info->FloatValue();
    }
    return kFloatMissing; // Ei pitäisi mennä tänne
}

void NFmiSmartToolCalculation::atom(double &result,
                                    const NFmiCalculationParams &theCalculationParams)
{
  if(IsCrossSectionVariableCase(theCalculationParams))
    result = CrossSectionVariableCaseValue(theCalculationParams);
  else if (fUseHeightCalculation)
    result = token->HeightValue(itsHeightValue, theCalculationParams);
  else if (fUsePressureLevelCalculation)
    result = token->PressureValue(itsPressureHeightValue, theCalculationParams);
  else
    result = token->Value(theCalculationParams, fUseTimeInterpolationAlways);
  get_token();
}

// ottaa seuraavan 'tokenin' kohdalle, mutta koska aluksi
// itsCalcIterator osoittaa jo 1. tokeniin, tehdään ensin
// sijoitus tokeniin ja sitten siirretään iteraattoria eteenpäin.
// Tällä lailla C++: Compl. Ref. kirjasta kopioitu koodi toimii 'sellaisenaan'.
void NFmiSmartToolCalculation::get_token(void)
{
  if (itsCalcIterator != itsCalculations.end())
  {
    token = *itsCalcIterator;
    ++itsCalcIterator;
  }
  else
  {
    throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorWrongEnding") + ":\n" +
                        GetCalculationText());
  }
}

// Laskun alustus, alustetaan iteraattori ja token.
bool NFmiSmartToolCalculation::bin_eval_exp(const NFmiCalculationParams &theCalculationParams)
{
  bool maskresult = true;
  double result = kFloatMissing;

  token = boost::shared_ptr<NFmiAreaMask>();  // nollataan aluksi 'token'
  itsCalcIterator = itsCalculations.begin();

  get_token();
  if (!token || token->GetCalculationOperationType() == NFmiAreaMask::NoType ||
      token->GetCalculationOperationType() == NFmiAreaMask::EndOfOperations)
    throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorNoCalculations") + ":\n" +
                        GetCalculationText());

  bin_eval_exp1(maskresult, result, theCalculationParams);
  if (token->GetCalculationOperationType() != NFmiAreaMask::EndOfOperations)
    throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorConditionalEnding") +
                        ":\n" + GetCalculationText());
  return maskresult;
}

// Process an assignment.
// Tämä on jo hoidettu toisella tavalla omassa koodissa, joten ohitan tämän,
// mutta jätän kommentteihin, jos tarvitsen tulevaisuudessa.
void NFmiSmartToolCalculation::bin_eval_exp1(bool &maskresult,
                                             double &result,
                                             const NFmiCalculationParams &theCalculationParams)
{
  bin_eval_exp1_1(maskresult, result, theCalculationParams);
}

// Tämä pitää siirtää prioriteetissa alle yhteen laskun
// Process a binary expression. && ja || eli AND ja OR
void NFmiSmartToolCalculation::bin_eval_exp1_1(bool &maskresult,
                                               double &result,
                                               const NFmiCalculationParams &theCalculationParams)
{
  NFmiAreaMask::BinaryOperator op;
  bool tempmask;
  double temp;

  bin_eval_exp1_2(maskresult, result, theCalculationParams);
  while ((op = token->PostBinaryOperator()) == NFmiAreaMask::kAnd || op == NFmiAreaMask::kOr)
  {
    get_token();
    bin_eval_exp1_2(tempmask, temp, theCalculationParams);
    switch (op)
    {
      case NFmiAreaMask::kAnd:
        maskresult = (maskresult && tempmask);
        break;
      case NFmiAreaMask::kOr:
        maskresult = (maskresult || tempmask);
        break;
      default:
        throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorStrangeConditional") +
                            ":\n" + GetCalculationText());
    }
  }
}

// Tämä pitää siirtää prioriteetissa alle yhteen laskun
// Process a comparison expression <, >, ==, <=, >=
void NFmiSmartToolCalculation::bin_eval_exp1_2(bool &maskresult,
                                               double &result,
                                               const NFmiCalculationParams &theCalculationParams)
{
  FmiMaskOperation op;
  bool tempmask;
  double temp;
  NFmiAreaMask::CalculationOperationType opType1 = token->GetCalculationOperationType();

  bin_eval_exp2(maskresult, result, theCalculationParams);
  while ((op = token->Condition().Condition()) != kFmiNoMaskOperation)
  {
    get_token();
    NFmiAreaMask::CalculationOperationType opType2 = token->GetCalculationOperationType();
    bin_eval_exp2(tempmask, temp, theCalculationParams);

    // resultit eivät saa olla missin-arvoja, paitsi jos ne ovat Constant-maskista, eli halutaan
    // nimenomaan verrata jotain missing-arvoon
    bool allowMissingComparison = (result == kFloatMissing && opType1 == NFmiAreaMask::Constant) ||
                                  (temp == kFloatMissing && opType2 == NFmiAreaMask::Constant);
    bool missingValuesExist = (result == kFloatMissing) || (temp == kFloatMissing);
    if ((!allowMissingComparison) && missingValuesExist)
      maskresult = false;
    else
    {
      switch (op)
      {
        case kFmiMaskEqual:
          maskresult = (result == temp);
          break;
        case kFmiMaskGreaterThan:
          maskresult = (result > temp);
          break;
        case kFmiMaskLessThan:
          maskresult = (result < temp);
          break;
        case kFmiMaskGreaterOrEqualThan:
          maskresult = (result >= temp);
          break;
        case kFmiMaskLessOrEqualThan:
          maskresult = (result <= temp);
          break;
        case kFmiMaskNotEqual:
          maskresult = (result != temp);
          break;
        default:
          throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorStrangeComparison") +
                              ":\n" + GetCalculationText());
      }
    }
  }
}

// Add or subtract two terms.
void NFmiSmartToolCalculation::bin_eval_exp2(bool &maskresult,
                                             double &result,
                                             const NFmiCalculationParams &theCalculationParams)
{
  NFmiAreaMask::CalculationOperator op;
  double temp;

  bin_eval_exp3(maskresult, result, theCalculationParams);
  while ((op = token->GetCalculationOperator()) == NFmiAreaMask::Add || op == NFmiAreaMask::Sub)
  {
    get_token();
    bin_eval_exp3(maskresult, temp, theCalculationParams);
    if (result == kFloatMissing || temp == kFloatMissing)
      result = kFloatMissing;
    else if (op == NFmiAreaMask::Sub)
      result = result - temp;
    else  // NFmiAreaMask::Add
      result = result + temp;
  }
}

// Multiply or divide two factors.
void NFmiSmartToolCalculation::bin_eval_exp3(bool &maskresult,
                                             double &result,
                                             const NFmiCalculationParams &theCalculationParams)
{
  NFmiAreaMask::CalculationOperator op;
  double temp;

  bin_eval_exp4(maskresult, result, theCalculationParams);
  while ((op = token->GetCalculationOperator()) == NFmiAreaMask::Mul || op == NFmiAreaMask::Div ||
         op == NFmiAreaMask::Mod)
  {
    get_token();
    bin_eval_exp4(maskresult, temp, theCalculationParams);
    if (result == kFloatMissing || temp == kFloatMissing)
      result = kFloatMissing;
    else if (op == NFmiAreaMask::Mul)
      result = result * temp;
    else if (op == NFmiAreaMask::Div)
      result = (temp == 0 ? kFloatMissing : result / temp);
    else  // NFmiAreaMask::Mod
      result = temp == 0 ? kFloatMissing : static_cast<int>(result) % static_cast<int>(temp);
  }
}

// Process an exponent
void NFmiSmartToolCalculation::bin_eval_exp4(bool &maskresult,
                                             double &result,
                                             const NFmiCalculationParams &theCalculationParams)
{
  double temp;

  bin_eval_exp5(maskresult, result, theCalculationParams);
  if (token->GetCalculationOperator() == NFmiAreaMask::Pow)
  {
    get_token();
    bin_eval_exp4(maskresult, temp, theCalculationParams);
    if (temp == 0.0)
    {
      result = 1.0;
      return;
    }
    if (result == kFloatMissing || temp == kFloatMissing)
      result = kFloatMissing;
    else
      result = pow(result, temp);
  }
}

// Evaluate a unary + or -.
void NFmiSmartToolCalculation::bin_eval_exp5(bool &maskresult,
                                             double &result,
                                             const NFmiCalculationParams &theCalculationParams)
{
  NFmiAreaMask::CalculationOperator op = token->GetCalculationOperator();
  if (op == NFmiAreaMask::Add || op == NFmiAreaMask::Sub)
    get_token();
  bin_eval_exp6(maskresult, result, theCalculationParams);

  if (op == NFmiAreaMask::Sub && result != kFloatMissing)
    result = -result;
}

void NFmiSmartToolCalculation::CalcThreeArgumentFunction(
    double &result, const NFmiCalculationParams &theCalculationParams)
{
  // huom! tässä ei käytetä bin_eval-kutsuja, koska tässä lasketaan vain yksi luku, mikä palautetaan
  // bin_eval-systeemiin
  NFmiAreaMask::FunctionType func =
      token->GetFunctionType();  // eli onko kyse min, max vai mistä funktiosta
  int integrationFunctionType = token->IntegrationFunctionType();  // tämä kertoo onko kyse
                                                                   // vertikaali vai ajallisesta
                                                                   // integroinnista kyse
  get_token();
  double argument1 = kFloatMissing;
  eval_exp2(argument1, theCalculationParams);
  if (token->GetCalculationOperationType() !=
      NFmiAreaMask::CommaOperator)  // näiden funktioiden argumentit erotetaan pilkuilla
    throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidParamSeparation") +
                        ":\n" + GetCalculationText());
  get_token();
  double argument2 = kFloatMissing;
  eval_exp2(argument2, theCalculationParams);
  if (token->GetCalculationOperationType() !=
      NFmiAreaMask::CommaOperator)  // näiden funktioiden argumentit erotetaan pilkuilla
    throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidParamSeparation") +
                        ":\n" + GetCalculationText());
  eval_ThreeArgumentFunction(
      result, argument1, argument2, func, integrationFunctionType, theCalculationParams);

  if (token->GetCalculationOperationType() !=
      NFmiAreaMask::EndParenthesis)  // MathFunctionStart:in päättää normaali lopetus sulku!
    throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidMathParenthesis") +
                        ":\n" + GetCalculationText());
  get_token();
}

void NFmiSmartToolCalculation::CalcVertFunction(double &result,
                                                const NFmiCalculationParams &theCalculationParams)
{
  boost::shared_ptr<NFmiAreaMask> verticalFunctionToken =
      token;  // tähän otetaan talteen vertikaalilasku olio
  int trueArgumentCount = verticalFunctionToken->FunctionArgumentCount() - 1;
  std::vector<float> argumentVector;  // tässä vektorissa annetaan vertikaali laskuissa tarvittavat
                                      // argumentti arvot (Paitsi 1. joka tulee AreaMask-olion
                                      // omasta itsInfo-datasta)
  for (int argumentCounter = 0; argumentCounter < trueArgumentCount; argumentCounter++)
  {
    get_token();
    double argument = kFloatMissing;
    eval_exp2(argument, theCalculationParams);
    bool commaOperator = (token->GetCalculationOperationType() == NFmiAreaMask::CommaOperator);
    bool endParethesisOperator =
        (argumentCounter >= trueArgumentCount - 1) &&
        (token->GetCalculationOperationType() == NFmiAreaMask::EndParenthesis);
    if (!(commaOperator || endParethesisOperator))  // näiden funktioiden argumentit erotetaan
                                                    // pilkuilla tai ne loppuu sulkuun
      throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidParamSeparation") +
                          ":\n" + GetCalculationText());
    argumentVector.push_back(static_cast<float>(argument));
  }

  verticalFunctionToken->SetArguments(argumentVector);
  result = verticalFunctionToken->Value(theCalculationParams, false);

  if (token->GetCalculationOperationType() !=
      NFmiAreaMask::EndParenthesis)  // MathFunctionStart:in päättää normaali lopetus sulku!
    throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidMathParenthesis") +
                        ":\n" + GetCalculationText());
  get_token();
}

// Process a parenthesized expression.
void NFmiSmartToolCalculation::bin_eval_exp6(bool &maskresult,
                                             double &result,
                                             const NFmiCalculationParams &theCalculationParams)
{
  if (token->GetCalculationOperationType() == NFmiAreaMask::StartParenthesis)
  {
    get_token();
    bin_eval_exp1_1(maskresult, result, theCalculationParams);
    if (token->GetCalculationOperationType() != NFmiAreaMask::EndParenthesis)
      throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidParenthesis") +
                          ":\n" + GetCalculationText());
    get_token();
  }
  else if (token->GetCalculationOperationType() == NFmiAreaMask::MathFunctionStart)
  {
    NFmiAreaMask::MathFunctionType function = token->GetMathFunctionType();
    get_token();
    bin_eval_exp1_1(maskresult, result, theCalculationParams);
    if (token->GetCalculationOperationType() !=
        NFmiAreaMask::EndParenthesis)  // MathFunctionStart:in päättää normaali lopetus sulku!
      throw runtime_error(::GetDictionaryString("SmartToolCalculationErrorInvalidMathParenthesis") +
                          ":\n" + GetCalculationText());
    eval_math_function(result, function);
    get_token();
  }
  else if (token->GetCalculationOperationType() == NFmiAreaMask::ThreeArgumentFunctionStart)
    CalcThreeArgumentFunction(result, theCalculationParams);
  else if (token->GetCalculationOperationType() == NFmiAreaMask::VertFunctionStart)
    CalcVertFunction(result, theCalculationParams);
  else
    bin_atom(maskresult, result, theCalculationParams);
}

void NFmiSmartToolCalculation::bin_atom(bool &maskresult,
                                        double &result,
                                        const NFmiCalculationParams &theCalculationParams)
{
  if(IsCrossSectionVariableCase(theCalculationParams))
    result = CrossSectionVariableCaseValue(theCalculationParams);
  else if (fUseHeightCalculation)
    result = token->HeightValue(itsHeightValue, theCalculationParams);
  else if (fUsePressureLevelCalculation)
    result = token->PressureValue(itsPressureHeightValue, theCalculationParams);
  else
    result = token->Value(theCalculationParams, fUseTimeInterpolationAlways);
  get_token();
}

// tarkistaa onko resultinfon aktiivinen parametri kuten tuulen suunta
// ja tekee tarvittavat asetukset
void NFmiSmartToolCalculation::CheckIfModularParameter(void)
{
  fCircularValue = false;
  itsCircularValueModulor = kFloatMissing;
  if (itsResultInfo)
  {
    if (itsResultInfo->Param().GetParamIdent() == kFmiWindDirection)
    {
      fCircularValue = true;
      itsCircularValueModulor = 360;
    }
  }
}

double NFmiSmartToolCalculation::FixCircularValues(double theValue)
{
  if (fCircularValue && theValue != kFloatMissing)
  {
    if (theValue < 0)
      return itsCircularValueModulor - fmod(-theValue, itsCircularValueModulor);
    else
      return fmod(theValue, itsCircularValueModulor);
  }
  return theValue;
}

checkedVector<boost::shared_ptr<NFmiSmartToolCalculation> > NFmiSmartToolCalculation::DoShallowCopy(
    const checkedVector<boost::shared_ptr<NFmiSmartToolCalculation> > &theCalculationVector)
{
  checkedVector<boost::shared_ptr<NFmiSmartToolCalculation> > returnVector(
      theCalculationVector.size());
  for (size_t i = 0; i < theCalculationVector.size(); i++)
    returnVector[i] = boost::shared_ptr<NFmiSmartToolCalculation>(
        new NFmiSmartToolCalculation(*theCalculationVector[i]));
  return returnVector;
}
