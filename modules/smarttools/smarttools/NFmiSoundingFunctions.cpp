// ======================================================================
/*!
 * \file NFmiSoundingFunctions.cpp
 *
 * Namespace, jossa erilaisia irto funktioita, joiden avulla lasketaan
 * erilaisia mm. luotauksiin liittviä juttuja.
 */
// ======================================================================

#include "NFmiSoundingFunctions.h"
#include <newbase/NFmiAngle.h>
#include <newbase/NFmiValueString.h>
#include <newbase/NFmiInterpolation.h>

namespace NFmiSoundingFunctions
{
// Funktio laskee kastepisteen (DP) lämpötilan (T) ja suhteellisen
// kosteuden (RH) avulla. Lämpötila ja kastepiste ovat celsiuksina.
double CalcDP(double T, double RH)
{
  static bool firstTime = true;
  const int arraySize = 1001;
  static double gLogTable0_100[arraySize] = {
      0};  // taulukossa kymmenyksen tarkkuudella logaritmit (size 1001)
  if (firstTime)
  {
    for (int i = 0; i < arraySize; i++)
      gLogTable0_100[i] = ::log(i * 0.1f / 100.f);
    firstTime = false;
  }
  const double b = 17.27f;
  const double c = 237.3f;
  //	double x = (log(RH/100.) + b * (T / (T + c))) / b;

  int logIndex = static_cast<int>(round(RH * 10));
  if (T != kFloatMissing && RH != kFloatMissing && logIndex > 0 &&
      logIndex < arraySize)  // logIndex pitää olla > 0 koska 0-kohdassa on not a number!
  {
    double x = (gLogTable0_100[logIndex] + b * (T / (T + c))) / b;
    if (x != 1)  // tarkistus ettei tule nollalla jakoa
    {
      double dp = c * x / (1 - x);
      return dp;
    }
  }
  return kFloatMissing;
}

double ESW(double T)
{
  double ES0 = 6.1078;
  double POL =
      0.99999683 +
      T * (-0.90826951E-02 +
           T * (0.78736169E-04 +
                T * (-0.61117958E-06 +
                     T * (0.43884187E-08 +
                          T * (-0.29883885E-10 +
                               T * (0.21874425E-12 +
                                    T * (-0.17892321E-14 +
                                         T * (0.11112018E-16 + T * (-0.30994571E-19)))))))));
  double ESW = ES0 / (::pow(POL, 8));
  return ESW;
}

double WMR(double T, double P)
{
  double EPS = 0.62197;
  double X = 0.02 * (T - 12.5 + 7500. / P);
  double WFW = 1. + 4.5E-06 * P + 1.4E-03 * X * X;
  double FWESW = WFW * ESW(T);
  double R = EPS * FWESW / (P - FWESW);
  double WMR = 1000. * R;
  return WMR;
}

// http://www.iac.ethz.ch/staff/dominik/idltools/atmos_phys/
double ESAT(double T)  // T kelvineinä tai celsiuksina
{
  double T0 = 0.;
  if (T < 105.)
    T0 = 273.15;

  // Formula with T = temperature in K
  //   esat = exp( -6763.6/(T+T0) - 4.9283*alog((T+T0)) + 54.2190 )

  // Formula close to that of Magnus, 1844 with temperature TC in Celsius
  //    ESAT = 6.1078 * EXP( 17.2693882 * TC / (TC + 237.3) ) ; TC in Celsius

  // or Emanuel's formula (also approximation in form of Magnus' formula,
  // 1844), which was taken from Bolton, Mon. Wea. Rev. 108, 1046-1053, 1980.
  // This formula is very close to Goff and Gratch with differences of
  // less than 0.25% between -50 and 0 deg C (and only 0.4% at -60degC)
  //    esat=6.112*EXP(17.67*TC/(243.5+TC))

  // WMO reference formula is that of Goff and Gratch (1946), slightly
  // modified by Goff in 1965:

  double e1 = 1013.250;
  double TK = 273.15;
  double esat =
      e1 * ::pow(10.,
                 (10.79586 * (1 - TK / (T + T0)) - 5.02808 * ::log10((T + T0) / TK) +
                  1.50474 * 1e-4 * (1 - ::pow(10., (-8.29692 * ((T + T0) / TK - 1)))) +
                  0.42873 * 1e-3 * (::pow(10., (4.76955 * (1 - TK / (T + T0)))) - 1) - 2.2195983));

  return esat;
}

/*
double ESAT2(double Tk) // T kelvineinä (HUOM! kaavassa on jotain vikaa, EI TOIMI)
{
    double T0 = 273.15;
    double esat2 = ::exp( -6763.6/(Tk+T0) - 4.9283* ::log((Tk+T0)) + 54.2190 );
    return esat2;
}

double ESAT3(double Tc) // T celsiuksina
{
    double ESAT3 = 6.1078 * ::exp( 17.2693882 * Tc / (Tc + 237.3) ) ;
    return ESAT3;
}

double ESAT4(double Tc) // T celsiuksina (tällä saadaan parhaat tulokset verrattuna ESAT-funktioon
ja tässä on vain yksi exp kun ESAT:issa on 3xpow ja 1xlog10)
{
    if(Tc > 105.)
        Tc -= 273.15; // muutetaan celsiuksiksi

    double esat4 = 6.112*::exp(17.67*Tc/(243.5+Tc));
    return esat4;
}
*/

// http://www.iac.ethz.ch/staff/dominik/idltools/atmos_phys/
double MIXR_SAT(double T, double P)
{
  double Mw = 18.0160;  // molec mass of water
  double Md = 28.9660;  // molec mass of dry air

  double X = ESAT(T);  // Note: ESAT accepts temperatures in Celsius or K
  double MIXR_SAT = Mw / Md * X * 1000. / (P - X);
  return MIXR_SAT;
}

static const double gTMR_alfa = 0.0498646455;
static const double gTMR_beta = 2.4082965;
static const double gTMR_gamma = 0.0915;
static const double gTMR_gamma2 = 38.9114;
static const double gTMR_gamma3 = 1.2035;

// lähde: http://www.iac.ethz.ch/staff/dominik/idltools/atmos_phys/skewt.pro
//;========================================================================
//;  FUNCTION TO COMPUTE THE TEMPERATURE (KELVIN) OF AIR AT A GIVEN
//;  PRESSURE AND WITH A GIVEN MIXING RATIO.
//;  TMR(KELVIN), W(GRAMS WATER VAPOR/KILOGRAM DRY AIR), P(MILLIBAR)
//      FUNCTION  TMR, W, P
//         X   =  ALOG10 ( W * P / (622.+ W) )
//         TMR = 10. ^ ( .0498646455 * X + 2.4082965 ) - 7.07475 + $
//               38.9114 * ( (10.^( .0915 * X ) - 1.2035 )^2 )
//      RETURN, TMR
//      END
//;========================================================================
double TMR(double W, double P)
{
  double X = ::log10(W * P / (622. + W));
  double TMR = ::pow(10., (gTMR_alfa * X + gTMR_beta)) - 7.07475 +
               gTMR_gamma2 * (::pow((::pow(10., (gTMR_gamma * X)) - gTMR_gamma3), 2));
  return TMR - 273.16;  // HUOM! lopussa muutetaan kuitenkin celsiuksiksi!!
}

// lähde: http://www.iac.ethz.ch/staff/dominik/idltools/atmos_phys/skewt.pro
//;========================================================================
//;  FUNCTION TO COMPUTE SATURATION ADIABATIC TEMP AT 1000 MB GIVEN T & P.
//;  OS AND T (KELVIN or CELSIUS), P (MILLIBARS )
//      FUNCTION  OS, T, P
//	 TK = T + 273.15*(T LT 100.)	; convert to Kelvin if necessary
//         OS = TK * ((1000./P)^.286) / (EXP( -2.6518986*MIXR_SAT(TK,P)/TK) )
//      RETURN, OS
//      END
//;========================================================================
double OS(double T, double P)
{
  double TK = (T < 100) ? T + 273.15 : T;  //; convert to Kelvin if necessary
  double OS = TK * (::pow((1000. / P), .286)) / (::exp(-2.6518986 * MIXR_SAT(TK, P) / TK));
  return OS - 273.16;  // Muutetaan takaisin celsiuksiksi
}

// lähde: http://www.iac.ethz.ch/staff/dominik/idltools/atmos_phys/skewt.pro
//;========================================================================
//;  FUNCTION TO COMPUTE TEMPERATUE (KELVIN) OF A MOIST ADIABAT GIVEN
//;  OS(KELVIN), P(MILLIBARS)
//;  SIGN(A,B) REPLACES THE ALGEBRAIC SIGN OF A WITH THE SIGN OF B
//      FUNCTION TSA, OS, P
//          A  = OS
//          TQ = 253.16
//          D  = 120.
//          FOR  I = 1,12 DO BEGIN	; iterations
//             D = D/2.
//;  IF THE TEMPERATURE DIFFERENCE, X, IS SMALL, EXIT THIS LOOP.
//             X = A * EXP (-2.6518986*MIXR_SAT(TQ,P)/TQ)-TQ*((1000./P)^.286)
//             IF ( ABS(X) LT 0.01 ) THEN GOTO, JUMP2
//	     D = - (X LT 0)*ABS(D)+(X GT 0)*ABS(D)
//             TQ = TQ + D
//          ENDFOR
// JUMP2:    TSA=TQ
//      RETURN, TSA
//      END
//;========================================================================
double TSA(double OS, double P)
{
  if (P < 1.3)
    P = 1.3;  // Marko: TSA-funktio ei toimi esim. P = 1 arvoilla, tällöin laskettu lämpötila saa
              // arvokseen älyttömän suuria arvoja (~99 astetta celsiusta), kokeellisesti saatu
              // minimi P raja oli 1.3
  double A = OS + 273.16;  // muutetaan tässä kelvineiksi
  double TQ = 253.16;
  double D = 120.;
  for (int I = 1; I < 12; I++)
  {
    D = D / 2.;
    double X = A * ::exp(-2.6518986 * MIXR_SAT(TQ, P) / TQ) - TQ * (::pow((1000. / P), .286));
    //;  IF THE TEMPERATURE DIFFERENCE, X, IS SMALL, EXIT THIS LOOP.
    if (::fabs(X) < 0.01)
      break;
    D = (X < 0) ? -fabs(D) : fabs(D);  // hämärää koodia alkuperäisessä kielessä
    TQ = TQ + D;
  }
  double TSA = TQ;
  return TSA - 273.16;  // Muutetaan takaisin celsiuksiksi
}

static const double gTpot2tConstant1 = 0.2854;
static const double gKelvinChange = 273.16;

// Laskee annetun potentiaalilämpötilan ja paineen avulla lämpötilan haluttuun korkeuteen.
// Se on käänteinen funktio CalcThetaT:n verrattuna.
// tpot annetaan celsiuksina.
// Oletus: kaikki parametrit ovat ei-puuttuvia!
double Tpot2t(double tpot, double p)
{
  // HUOM! pot lämpötila muutetaan ensin kelvineiksi ja lopuksi tulos muutetaan takaisin
  // celsiuksiksi
  return ((gKelvinChange + tpot) * ::pow(p / 1000, gTpot2tConstant1)) - gKelvinChange;
}

// Laskee potentiaalilämpötila theta kun annetaan lämpötila ja paine, missä lämpötila on otettu.
// T annetaan celsiuksina.
// Oletus: kaikki parametrit ovat ei-puuttuvia!
double T2tpot(double T, double P)
{
  const double T0 = 273.16;  // kelvin asteikon muunnos
  return ((T + T0) * ::pow(1000 / P, 0.2854)) - T0;
}

// laskee suhteellisen kosteuden lämpötilan ja kastepisteen avulla.
// Oletus: kaikki parametrit ovat ei-puuttuvia!
// Lämpötila ja kastepiste celsiuksina.
double CalcRH(double T, double Td)
{
  double RH = 100 * ::pow((112 - 0.1 * T + Td) / (112 + 0.9 * T), 8);
  return RH;
}

// Laskee kyllästysvesihöyrynpaineen es lämpötilan avulla.
// Oletus: kaikki parametrit ovat ei-puuttuvia!
// Lämpötila celsiuksina.
double CalcEs(double T)
{
  const double T0 = 273.16;        // kelvin asteikon muunnos
  const double Rv = 462;           // vesihöyryn kaasuvakio [J/kg/K]
  const double L = 2.5 * 1000000;  // vesihöyryn tiivistymislämpö [J/kg]
  double es = 6.11 * ::exp(L / Rv * (1 / T0 - 1 / (T + T0)));
  return es;
}

// Laskee kyllästysvesihöyrynpaineen es lämpötilan avulla. Saulin löytämä 'tarkka' kaava.
// Oletus: kaikki parametrit ovat ei-puuttuvia!
// Lämpötila celsiuksina.
double CalcEs2(double Tcelsius)
{
  const double b = 17.2694;
  const double e0 = 6.11;    // 6.11 <- 0.611 [kPa]
  const double T1 = 273.16;  // [K]
  const double T2 = 35.86;   // [K]

  double T = Tcelsius + T1;
  double nume = b * (T - T1);
  double deno = (T - T2);

  double es = e0 * ::exp(nume / deno);
  return es;
}

// Laskee vesihöyryn osapaineen (e)
// Oletus: kaikki parametrit ovat ei-puuttuvia!
double CalcE(double RH, double es)
{
  double e = RH * es / 100;
  return e;
}

// Laskee vesihöyryn osapaineen (e) ja kyllästyspaineen (es)
// avulla suhteellisen kosteuden RH.
// Oletus: kaikki parametrit ovat ei-puuttuvia!
double CalcRH2(double e, double es)
{
  double RH = e / es * 100;
  return RH;
}

// Laskee sekoitussuhteen w (mixing ratio) [g/kg]
// Oletus: kaikki parametrit ovat ei-puuttuvia!
double CalcW(double e, double P)
{
  double w = 0.622 * e / P * 1000;
  return w;
}

// Laskee sekoitussuhteen w (mixing ratio) [g/kg]
// ja paineen avulla vesihöyryn paineen e
// Oletus: kaikki parametrit ovat ei-puuttuvia!
double CalcE2(double w, double P)
{
  double e = w * P / (0.622 * 1000);
  return e;
}

// Laskee sekoitussuhteen w (mixing ratio) [g/kg].
// Oletus: kaikki parametrit ovat ei-puuttuvia!
double CalcMixingRatio(double T, double Td, double P)
{
  double RH = CalcRH(T, Td);
  double es = CalcEs2(T);
  double e = CalcE(RH, es);
  double w = CalcW(e, P);
  return w;
}

// Mixing ratio (r) [kg/kg], notice the difference in units compared to another version!
double CalcMixingRatioUsingKelvinsAndRH(double RH, double Tkelvin, double P)
{
    double Tcelsius = Tkelvin - 273.15;
    double e = RH / 100 * 0.611 * std::exp((17.27 * Tcelsius) / (Tcelsius + 237.3));
    double r = 0.622 * e / (P - e);
    return r;
}

// Laskee kastepisteen.
// Oletus: kaikki parametrit ovat ei-puuttuvia!
double CalcDewPoint(double T, double w, double P)
{
  double es = CalcEs2(T);
  double e = CalcE2(w, P);
  double RH = CalcRH2(e, es);
  double Td = CalcDP(static_cast<float>(T), static_cast<float>(RH));
  return Td;
}

// Laskee virtuaali lämpötilan Tv
// Oletus: kaikki parametrit ovat ei-puuttuvia!
double CalcTv(double T, double Td, double P)
{
  double w = CalcMixingRatio(T, Td, P);
  double Tv = (1. + 0.61 * w) * T;
  return Tv;
}

// Equivalent potential temperature (Theta-E)
double CalcThetaE(double T, double Td, double P)
{
  double tpot = T2tpot(T, P);
  double w = CalcMixingRatio(T, Td, P);
  double thetae = tpot + 3 * w;
  return thetae;
}

// Laskee LCL-levelin T, Td, ja 'aloitus' P:n avulla
double CalcLCLPressure(double T, double Td, double P)
{
  int iterationCount = 0;  // Tämän voi poistaa profiloinnin jälkeen
  double lclPressure = kFloatMissing;
  // 2. Laske sekoitussuhde pinnalla
  double w = CalcMixingRatio(T, Td, P);
  double tpot = T2tpot(T, P);  // pitää laskea mitä lämpötilaa vastaa pinnan 'potentiaalilämpötila'
  // 3. iteroi pinnasta ylöspäin ja laske, milloin mixing-ratio käyrä ja lämpötilan
  // kostea-adiapaatti leikkaavat
  double lastP = P;
  for (double currentP = P; currentP > 100; currentP -= 1)
  {
    iterationCount++;
    double Tw = TMR(w, currentP);
    double Tdry = Tpot2t(tpot, currentP);

    if (Tdry < Tw)
      break;
    lastP = currentP;  // viimeisinta painetta ennen kuin Tw ja Tdry ovat leikanneet, voidaan
                       // käyttää tarkemman LCL painepinnan interpolointiin
  }

  // laske tarkempi paine jos viitsit lastP ja currentP;n avulla interpoloimalla
  lclPressure = lastP;
  return lclPressure;
}

// Laskee newtonin menetelmällä seuraavan P:n arvon funktion ja sen derivaatan avulla.
// Palauttaa myös viimeisimmällä arvolla lasketun erotuksen.
double IterateMixMoistDiffWithNewtonMethod(double W, double Tpot, double P, double &diffOut)
{
  double P2 = P + 0.001;
  double tmr1 = TMR(W, P);
  double tmr2 = TMR(W, P2);
  double Tw1 = Tpot2t(Tpot, P);
  double Tw2 = Tpot2t(Tpot, P2);
  double tmrDeri = (tmr2 - tmr1) / (P2 - P);
  double TwDeri = (Tw2 - Tw1) / (P2 - P);
  double mixMoistDiff = tmr1 - Tw1;
  diffOut = mixMoistDiff;
  double mixMoistDiffDerivate = tmrDeri - TwDeri;
  return P - (mixMoistDiff / mixMoistDiffDerivate);
}

double CalcLCLPressureFast(double T, double Td, double P)
{
  double lastLCL = 900;  // aloitetaan haku jostain korkeudesta

  int iterationCount = 0;  // Tämän voi poistaa profiloinnin jälkeen
  double lclPressure = kFloatMissing;
  // 2. Laske sekoitussuhde pinnalla
  double w = CalcMixingRatio(T, Td, P);
  double tpot = T2tpot(T, P);  // pitää laskea mitä lämpötilaa vastaa pinnan 'potentiaalilämpötila'
  double currentP = lastLCL;
  double diff = 99999;
  int maxIterations = 20;
  // Etsi newtonin menetelmällä LCL pressure
  do
  {
    iterationCount++;
    currentP = IterateMixMoistDiffWithNewtonMethod(w, tpot, currentP, diff);
    if (::fabs(diff) < 0.01)
      break;
    if (currentP < 100)  // most unstable tapauksissa etsintä piste saattaa pompata tosi ylös
    {  // tässä on paineen arvoksi tullut niin pieni että nostetaan sitä takaisin ylös ja jatketaan
      // etsintöjä
      currentP = 100;
    }
  } while (iterationCount < maxIterations);

  // laske tarkempi paine jos viitsit lastP ja currentP;n avulla interpoloimalla
  if (iterationCount < maxIterations && currentP != kFloatMissing)
    lastLCL = currentP;
  else if (iterationCount >= maxIterations)
    currentP = kFloatMissing;
  lclPressure = currentP;
  return lclPressure;
}

// etsii monotonisen 'funktion' juurta kun sille on annettu kaksi funktion pistettä
double FindRoot(double x1, double x2, double y1, double y2)
{
  double x0 = x1 - y1 * ((x1 - x2) / (y1 - y2));
  return x0;
}

// laskee kostea adiapaattisen arvon (iteroimalla), kun annetaan
// lämpötila (celcius) ja paine (mb).
// Oletus: parametrit ovat ei puuttuvia.
// Etsii kostea-adiapaattiarvoa alueelta -40 - 50 astetta celsiusta.
// HUOM!!!!! tämä voi varmaan optimoida jollain puolitus haulla!?!?, luulen että tämä hidastaa
// luotaus piirtoa rankasti!
double CalcMoistT(double T, double P)
{
  double minDiff = 999999.;
  double minDiffMoistT = kFloatMissing;
  double deltaT = 0.1;
  double moistT1 = -40;
  double moistT2 = 50;
  int counter = 0;
  do
  {
    double AOS1 = OS(moistT1, 1000.);  // tämä on hämärä, miksi tämä tehdään, mutta muutetaan
                                       // kostea-lämppäsi jotenkin pintaan
    double ATSA1 = TSA(AOS1, P);  // lasketaan sen avulla lämpötila taas halutulle korkeudelle
    double diff1 = ATSA1 - T;
    double AOS2 = OS(moistT2, 1000.);
    double ATSA2 = TSA(AOS2, P);
    double diff2 = ATSA2 - T;
    double moistT0 = FindRoot(moistT1, moistT2, diff1, diff2);
    double AOS0 = OS(moistT0, 1000.);
    double ATSA0 = TSA(AOS0, P);
    double diff0 = ATSA0 - T;
    if (::fabs(diff0) < minDiff)
    {  // jos laskettu lämpötila oli tarpeeksi lähellä annettua, laitetaan arvo talteen
      minDiff = ::fabs(diff0);
      minDiffMoistT = moistT0;
    }
    if (diff1 < 0 && diff0 < 0)  // katsotaan kumman kanssa etumerkki on sama ja alustetaan
                                 // iterointi alkuarvot sen mukaan että ollaan eri puolella 0:aa
      moistT1 = moistT0;
    else
      moistT2 = moistT0;
    counter++;
  } while (minDiff > deltaT && counter < 10);

  if (minDiff <= deltaT)  // jos lähimmän lasketun lämppärin ero oli tarpeeksi pieni, palautetaan
                          // sitä vastaava kostea-lämpötila
    return minDiffMoistT;
  return kFloatMissing;
}

// Laskee logaritmisessa asteikossa interpoloidun arvon.
// Käytetään esim. logaritmisen paine asteikon kanssa.
// Palauttaa x:ää vastaavan y:n, kun x1 arvoa vastaa y1 ja x2:n arvoa vastaa y2.
float CalcLogInterpolatedValue(float x1, float x2, float x, float y1, float y2)
{
  float y = kFloatMissing;
  if (x1 != kFloatMissing && x2 != kFloatMissing && x != kFloatMissing)
  {
    if (x1 == x2)
      y = y1 != kFloatMissing ? y1 : y2;
    else if (y1 != kFloatMissing && y2 != kFloatMissing)
    {
      float w = (::log(x) - ::log(x1)) / (::log(x2) - ::log(x1));
      y = (1 - w) * y1 + w * y2;
    }
    else if (y1 != kFloatMissing)  // jos toinen -arvoista puuttuu annetaan arvoksi toinen
      y = y1;
    else if (y2 != kFloatMissing)  // jos toinen -arvoista puuttuu annetaan arvoksi toinen
      y = y2;
  }
  return y;
}

// Laskee logaritmisessa asteikossa interpoloidun arvon.
// Käytetään esim. logaritmisen paine asteikon kanssa.
// Palauttaa x:ää vastaavan y:n, kun x1 arvoa vastaa y1 ja x2:n arvoa vastaa y2.
float CalcLogModLinearInterpolatedValue(
    float x1, float x2, float x, float y1, float y2, unsigned int modulo)
{
  float y = kFloatMissing;
  if (x1 != kFloatMissing && x2 != kFloatMissing && x != kFloatMissing)
  {
    if (x1 == x2)
      y = y1 != kFloatMissing ? y1 : y2;
    else if (y1 != kFloatMissing && y2 != kFloatMissing)
    {
      float w = (::log(x) - ::log(x1)) / (::log(x2) - ::log(x1));
      y = static_cast<float>(NFmiInterpolation::ModLinear(w, y1, y2, modulo));
    }
    else if (y1 != kFloatMissing)  // jos toinen -arvoista puuttuu annetaan arvoksi toinen
      y = y1;
    else if (y2 != kFloatMissing)  // jos toinen -arvoista puuttuu annetaan arvoksi toinen
      y = y2;
  }
  return y;
}

float CalcLogInterpolatedWindWectorValue(float x1, float x2, float x, float wv1, float wv2)
{
  float y = kFloatMissing;
  if (wv1 != kFloatMissing && wv2 != kFloatMissing)
  {
    float wd1 = (static_cast<int>(wv1) % 100) * 10.f;
    float ws1 = static_cast<float>(static_cast<int>(wv1) / 100);
    float wd2 = (static_cast<int>(wv2) % 100) * 10.f;
    float ws2 = static_cast<float>(static_cast<int>(wv2) / 100);

    float wdInterp = CalcLogModLinearInterpolatedValue(x1, x2, x, wd1, wd2, 360);
    float wsInterp = CalcLogInterpolatedValue(x1, x2, x, ws1, ws2);
    if (wdInterp != kFloatMissing && wsInterp != kFloatMissing)
    {
      y = static_cast<float>(round(wsInterp) * 100 + round(wdInterp / 10.));
    }
  }
  else if (wv1 != kFloatMissing)
    y = wv1;
  else if (wv2 != kFloatMissing)
    y = wv2;
  return y;
}

// id-vector u-component 'right'
double CalcU_ID_right(double u0_6, double shr_0_6_v_n)
{
  double u_ID = u0_6 + shr_0_6_v_n * 7.5;
  return u_ID;
}

// id-vector v-component 'left'
double CalcV_ID_left(double v0_6, double shr_0_6_u_n)
{
  double v_ID = v0_6 + shr_0_6_u_n * 7.5;
  return v_ID;
}

// id-vector u-component 'left'
double CalcU_ID_left(double u0_6, double shr_0_6_v_n)
{
  double u_ID = u0_6 - shr_0_6_v_n * 7.5;
  return u_ID;
}

// id-vector v-component 'right'
double CalcV_ID_right(double v0_6, double shr_0_6_u_n)
{
  double v_ID = v0_6 - shr_0_6_u_n * 7.5;
  return v_ID;
}

// shear unit u-vector
double Calc_shear_unit_u_vector(double shr_0_6_u, double shr_0_6_v)
{
  double shr_0_6_u_n = shr_0_6_u / ::sqrt((shr_0_6_u * shr_0_6_u + shr_0_6_v * shr_0_6_v));
  return shr_0_6_u_n;
}

// shear unit v-vector
double Calc_shear_unit_v_vector(double shr_0_6_v, double shr_0_6_u)
{
  double shr_0_6_v_n = shr_0_6_v / ::sqrt((shr_0_6_v * shr_0_6_v + shr_0_6_u * shr_0_6_u));
  return shr_0_6_v_n;
}

// u_ID and vID are wind component vector calculated for helicity calculations
// uP1 is wind vector x (along latitude) component from lower layer and uP2 from higher layer
// vP1 are the same but for wind y (along longitude) component.
double CalcSRH(double u_ID, double v_ID, double uP1, double uP2, double vP1, double vP2)
{
  double SRH = ((u_ID - uP1) * (vP1 - vP2)) - ((v_ID - vP1) * (uP1 - uP2));
  return SRH;
}

double FindNearestW(double T, double P)
{
  double minDiff = 999999.;
  double minDiffW = kFloatMissing;
  double deltaW = 0.1;
  double w1 = 0.01;
  double w2 = 50;
  int counter = 0;
  do
  {
    double t1 = TMR(w1, P);
    double diff1 = t1 - T;
    double t2 = TMR(w2, P);
    double diff2 = t2 - T;
    double w0 = FindRoot(w1, w2, diff1, diff2);
    double t0 = TMR(w0, P);
    double diff0 = t0 - T;
    if (::fabs(diff0) < minDiff)
    {  // jos laskettu lämpötila oli tarpeeksi lähellä annettua, laitetaan arvo talteen
      minDiff = ::fabs(diff0);
      minDiffW = w0;
    }
    if (diff1 < 0 && diff0 < 0)  // katsotaan kumman kanssa etumerkki on sama ja alustetaan
                                 // iterointi alkuarvot sen mukaan että ollaan eri puolella 0:aa
      w1 = w0;
    else
      w2 = w0;
    counter++;
  } while (minDiff > deltaW && counter < 50);

  if (minDiff <=
      deltaW)  // jos lähimmän lasketun W:n ero oli tarpeeksi pieni, palautetaan sitä vastaava arvo
    return minDiffW;
  return kFloatMissing;
}

// Laske pisteiden (P1, P2) ja (P3, P4) muodostamien viivojen leikkauspiste.
// Kaavat on haettu http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
MyPoint CalcTwoLineIntersectionPoint(const MyPoint &P1,
                                     const MyPoint &P2,
                                     const MyPoint &P3,
                                     const MyPoint &P4)
{
  if (P1.IsValid() && P2.IsValid() && P3.IsValid() && P4.IsValid())
  {
    double UaDenom = ((P4.x - P3.x) * (P1.y - P3.y) - (P4.y - P3.y) * (P1.x - P3.x));
    double Udividor = ((P4.y - P3.y) * (P2.x - P1.x) - (P4.x - P3.x) * (P2.y - P1.y));
    if (Udividor != 0)  // jakajan 0 tarkastelu
    {
      double Ua = UaDenom / Udividor;

      double UbDenom = ((P2.x - P1.x) * (P1.y - P3.y) - (P2.y - P1.y) * (P1.x - P3.x));
      double Ub = UbDenom / Udividor;

      double x = P1.x + Ua * (P2.x - P1.x);
      double y = P1.y + Ub * (P2.y - P1.y);

      return MyPoint(x, y);
    }
  }

  return MyPoint(kFloatMissing, kFloatMissing);  // virhetilanteissa puuttuvan tiedon piste
}

}  // end of namespace NFmiSoundingFunctions
