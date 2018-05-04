// ======================================================================
/*!
 * \file NFmiLagrange.h
 * \brief Interface of class NFmiLagrange
 */
// ======================================================================

#include "NFmiDataMatrix.h"  // for kFloatMissing
#include "NFmiDef.h"
#include "NFmiGlobals.h"  // for kFloatMissing

#include <cmath>
#include <string>

#pragma once

//! Undocumented
class _FMI_DLL NFmiLagrange
{
 public:
  ~NFmiLagrange(void);
  NFmiLagrange(void);
  NFmiLagrange(const NFmiLagrange& theLagrange);
  NFmiLagrange(const double* si, const double* ti, double* yij, int sn, int tn);
  NFmiLagrange(const double* si, const double* yij, int sn);

  void Si(const double* si, bool isJustCopyNewInputData = false);
  void Ti(const double* ti, bool isJustCopyNewInputData = false);
  void Yij(const double* yij, bool isJustCopyNewInputData = false);

  // 2D interpolation:
  bool Init(const double* si, const double* ti, double* yij, int sn, int tn);
  bool UpdateData(const double* si, const double* ti, const double* yij);
  double Interpolate(double s, double t);

  // 1D interpolation:
  bool Init(const double* si, const double* yi, int sn);
  bool UpdateData(const double* si, const double* yi);
  double Interpolate(double s);

 private:
  double* Si(void);
  double* Ti(void);
  double* Yij(void);

  void Denominator(const checkedVector<double>& xi, checkedVector<double>& denominator, int n);
  void SiDenominator(void);
  void TiDenominator(void);

  double L(
      const checkedVector<double>& xi, double x, checkedVector<double>& denominator, int i, int n);
  double Ls(double s, int i);
  double Lt(double t, int i);

  int itsSn;
  int itsTn;

  checkedVector<double> itsSi;
  checkedVector<double> itsTi;
  checkedVector<double> itsYij;

  checkedVector<double> itsSiDenominator;
  checkedVector<double> itsTiDenominator;

  checkedVector<double> itsLs;
  checkedVector<double> itsLt;

};  // class NFmiLagrange

// ======================================================================
