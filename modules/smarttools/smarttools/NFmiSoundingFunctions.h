// ======================================================================
/*!
 * \file NFmiSoundingFunctions.h
 *
 * Namespace, jossa erilaisia irto funktioita, joiden avulla lasketaan
 * erilaisia mm. luotauksiin liittviä juttuja.
 * Julkisia funktioita on vain osa, cpp-filessa on paljon lisää tavaraa.
 */
// ======================================================================

#pragma once

#include <newbase/NFmiGlobals.h>
#include <cmath>

namespace NFmiSoundingFunctions
{
double FindNearestW(double T, double P);
double Tpot2t(double tpot, double p);
double CalcRH(double T, double Td);
double TMR(double W, double P);
double OS(double T, double P);
double TSA(double OS, double P);
double T2tpot(double T, double P);
double CalcMoistT(double T, double P);
double CalcThetaE(double T, double Td, double P);
double CalcMixingRatio(double T, double Td, double P);
double CalcMixingRatioUsingKelvinsAndRH(double RH, double T, double P);
double CalcDewPoint(double T, double w, double P);
double CalcDP(double T, double RH);
double CalcLCLPressure(double T, double Td, double P);
double CalcLCLPressureFast(double T, double Td, double P);
double Calc_shear_unit_v_vector(double shr_0_6_v, double shr_0_6_u);
double CalcU_ID_left(double u0_6, double shr_0_6_v_n);
double CalcV_ID_left(double v0_6, double shr_0_6_u_n);
double CalcU_ID_right(double u0_6, double shr_0_6_v_n);
double CalcV_ID_right(double v0_6, double shr_0_6_u_n);
double CalcSRH(double u_ID, double v_ID, double uP1, double uP2, double vP1, double vP2);

float CalcLogInterpolatedValue(float x1, float x2, float x, float y1, float y2);
float CalcLogModLinearInterpolatedValue(
    float x1, float x2, float x, float y1, float y2, unsigned int modulo);
float CalcLogInterpolatedWindWectorValue(float x1, float x2, float x, float wv1, float wv2);

template <typename T>
bool IsEqualEnough(T value1, T value2, T usedEpsilon)
{
  if (::fabs(static_cast<double>(value1 - value2)) < usedEpsilon)
    return true;
  return false;
}

class MyPoint
{
 public:
  MyPoint(double theX, double theY) : x(theX), y(theY) {}
  bool IsValid(void) const { return (x != kFloatMissing) && (y != kFloatMissing); }
  double x;
  double y;
};
MyPoint CalcTwoLineIntersectionPoint(const MyPoint &P1,
                                     const MyPoint &P2,
                                     const MyPoint &P3,
                                     const MyPoint &P4);
}

