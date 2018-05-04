// ======================================================================
/*!
 * \file NFmiDataMatrix.h
 * \brief Interface of class NFmiDataMatrix
 */
// ======================================================================
/*!
 * \class NFmiDataMatrix
 *
 * NFmiDataMatrix on 2D datamatriisi, joka tukee [x][y] notaatiota.
 *
 * Datalla ei oleteta olevan minkään laisia matemaattisia ominaisuuksia,
 * tarkoitus on ainoastaan säilyttää dataa 2-ulotteisesti.
 *
 * Mallilausekkeita:
 *
 * \code
 *	NFmiDataMatrix<float> matrix(nx,ny);
 *	matrix = kFloatMissing;
 *
 *	NFmiDataMatrix<float> fastermatrix(nx,ny,kFloatMissing);
 *
 *	float arvo = matrix[x][y];
 *	matrix[x][y] = arvo;
 *	float arvo2 = matrix.At(x,y,puuttuvaarvo);

 *	matrix.Resize(new_nx,new_ny,fillvalue);
 *	matrix.RemoveColumn(2);
 *
 *	cout << matrix << endl;
 * \endcode
 *
 * Note: This is the faster way to loop, i in the inside:
 *
 * \code
 *    for (int j = 0; j < matrix.NY() ; j++)
 *      for (int i = 0; i < matrix.NX(); i++)
 *	    matrix[i][j] = 2
 * \endcode
 *
 */
// ======================================================================

#pragma once

// Jos tämä on määritelty, checkedVector tarkistaa aina rajat ja heittää aina poikkeuksen
// virhetilanteessa
// (virheraportin mukana vielä MSVC kääntäjän kanssa call stack).
// Jos tämä (FMI_USE_SECURE_CHECKED_VECTOR) on kommentoitu, toimii checkedVector-luokka (ja
// NFmiDataMatrix
// joka käyttää sitä) siten kuin se olisi normaali std::vector (nopea ja 'vaarallinen').

// #define FMI_USE_SECURE_CHECKED_VECTOR 1

#ifdef FMI_USE_SECURE_CHECKED_VECTOR
#ifdef _MSC_VER
//#include "stdafx.h" // call stack juttuihin tarvitaan windows headereita
//#include "sym_engine.h" // täältä tulee call stack virhe tarkasteluihin
#endif  // _MSC_VER
#endif  // FMI_USE_SECURE_CHECKED_VECTOR

#include "NFmiGlobals.h"  // kFloatMissing
#include "NFmiInterpolation.h"
#include "NFmiParameterName.h"
#include "NFmiPoint.h"  // kFloatMissing
#include "NFmiRect.h"
#include "NFmiStringTools.h"  // kFloatMissing

#include <iostream>
#include <sstream>
#include <vector>

// Laitoin checkedvectorin tähän debuggaus avuksi, koska ei ollu
// boundcheckeriä asennettuna. Koodi on pöllitty muistaakseni
// Breyn??? STL-kirjasta.
#ifndef CHECKVEC_H
#define CHECKVEC_H

template <class T>
class checkedVector : public std::vector<T>  // inherit from std::vector<T>
{
 public:
  typedef typename std::vector<T> base_type;
  typedef typename base_type::size_type size_type;
  typedef typename base_type::difference_type difference_type;
  typedef typename base_type::reference reference;
  typedef typename base_type::const_reference const_reference;

  // type names like iterator etc. are also inherited
  checkedVector() {}
  checkedVector(size_type n, const T& value = T()) : std::vector<T>(n, value) {}
  template <class Iterator>
  checkedVector(Iterator i, Iterator j) : std::vector<T>(i, j)
  {
  }

#ifdef FMI_USE_SECURE_CHECKED_VECTOR
  reference operator[](size_type index)
  {
    try
    {
      return this->at(index);
    }
    catch (std::exception& e)
    {
      DoErrorReporting(e, index);
    }
    throw std::runtime_error(
        "Ei pitäisi mennä tähän, mutta muuten kääntäjä valittaa että funktion pitää palauttaa");
  }

  const_reference operator[](size_type index) const
  {
    try
    {
      return this->at(index);
    }
    catch (std::exception& e)
    {
      DoErrorReporting(e, index);
    }
    throw std::runtime_error(
        "Ei pitäisi mennä tähän, mutta muuten kääntäjä valittaa että funktion pitää palauttaa");
  }

  const_reference at(size_type index) const
  {
    try
    {
      return std::vector<T>::at(index);
    }
    catch (std::exception& e)
    {
      DoErrorReporting(e, index);
    }
    throw std::runtime_error(
        "Ei pitäisi mennä tähän, mutta muuten kääntäjä valittaa että funktion pitää palauttaa");
  }

  reference at(size_type index)
  {
    try
    {
      return std::vector<T>::at(index);
    }
    catch (std::exception& e)
    {
      DoErrorReporting(e, index);
    }
    throw std::runtime_error(
        "Ei pitäisi mennä tähän, mutta muuten kääntäjä valittaa että funktion pitää palauttaa");
  }
#endif  // FMI_USE_SECURE_CHECKED_VECTOR

  void DoErrorReporting(std::exception& e, difference_type index) const
  {
    std::string indexStr("checkedVector size: ");
    indexStr += NFmiStringTools::Convert<unsigned int>(this->size());
    indexStr += " and index: ";
    indexStr += NFmiStringTools::Convert<difference_type>(index);
    indexStr += "\n";
    throw std::runtime_error(e.what() + std::string("\n") + indexStr);
  }
};
#endif  // CHECKVEC_H

static const NFmiRect g_defaultCoordinates(0, 0, 1, 1);

//! A 2D data container
template <class T>  // miten annetaan containeri template parametrina??????
// class _FMI_DLL NFmiDataMatrix : public std::vector<std::vector<T> >
class _FMI_DLL NFmiDataMatrix
    : public checkedVector<checkedVector<T> >  // tämä on ns. debug versio, jos tarvetta
{
 public:
  typedef typename checkedVector<T>::size_type size_type;

 protected:
  size_type itsNX;  //!< Matrix width
  size_type itsNY;  //!< Matrix height

  // Following two options are for debug purpose print out, used only with operator<< -funktion.
  bool fPrintYInverted;   // if true, write rows in inverted order
  bool fPrintIndexAxies;  // prints row number and column number indexies

 public:
  //! Constructor

  NFmiDataMatrix(size_type nx = 0, size_type ny = 0, const T& theValue = T())
      //    : std::vector<std::vector<T> >(nx, std::vector<T>(ny,theValue))
      : checkedVector<checkedVector<T> >(
            nx, checkedVector<T>(ny, theValue))  // tämä on ns. debug versio, jos tarvetta
        ,
        itsNX(nx),
        itsNY(ny),
        fPrintYInverted(false),
        fPrintIndexAxies(false)
  {
  }

  //! Return matrix width.

  size_type NX() const { return itsNX; }
  //! Return matrix height.

  size_type NY() const { return itsNY; }
  bool PrintYInverted(void) const { return fPrintYInverted; }
  void PrintYInverted(bool newValue) { fPrintYInverted = newValue; }
  bool PrintIndexAxies(void) const { return fPrintIndexAxies; }
  void PrintIndexAxies(bool newValue) { fPrintIndexAxies = newValue; }
  //! Matrix value at given location, or given value outside matrix

  const T& At(int i, int j, const T& missingvalue) const
  {
    if (i < 0 || j < 0 || static_cast<size_type>(i) >= itsNX || static_cast<size_type>(j) >= itsNY)
      return missingvalue;
    else
      return this->operator[](i)[j];
  }

  const T& GetValue(int theLongIndex, const T& missingvalue) const
  {
    return At(theLongIndex % static_cast<int>(itsNX),
              theLongIndex / static_cast<int>(itsNX),
              missingvalue);
  }

  void SetValue(int theLongIndex, const T& value)
  {
    int i = theLongIndex % static_cast<int>(itsNX);
    int j = theLongIndex / static_cast<int>(itsNX);
    if (i >= 0 && j >= 0 && static_cast<size_type>(i) < itsNX && static_cast<size_type>(j) < itsNY)
    {
      this->operator[](i)[j] = value;
    }
  }

  //! Matrix value at given location, it returns reference so you can modify it, or if out-of-bounds
  //! indexies => throws

  const T& At(int i, int j) const
  {
    try
    {
      return this->at(i).at(j);
    }
    catch (std::exception& e)
    {
      DoErrorReporting(e, i, j);
    }
    throw std::runtime_error(
        "Ei pitäisi mennä tähän, mutta muuten kääntäjä valittaa että funktion pitää palauttaa");
  }

  //! Matrix value at given location, it returns reference so you can modify it, or if out-of-bounds
  //! indexies => throws

  T& At(int i, int j)
  {
    try
    {
      return this->at(i).at(j);
    }
    catch (std::exception& e)
    {
      DoErrorReporting(e, i, j);
    }
    throw std::runtime_error(
        "Ei pitäisi mennä tähän, mutta muuten kääntäjä valittaa että funktion pitää palauttaa");
  }

  // Tämä IsEqualEnough-funktio piti kopsata NFmiQueryDataUtil.h -luokasta, koska sen includointi
  // menee ristiin tämän headerin kanssa
  template <typename X>
  static bool IsEqualEnough(X value1, X value2, X usedEpsilon)
  {
    if (::fabs(static_cast<double>(value1 - value2)) < usedEpsilon) return true;
    return false;
  }

  double FixIndexOnEdges(double index, size_type matrixSize) const
  {
    const double epsilon = 0.000001;
    if (index < 0 && IsEqualEnough(index, 0., epsilon))
      index = 0;
    else
    {
      double xMaxLimit = matrixSize - 1.;
      if (index >= xMaxLimit && IsEqualEnough(index, xMaxLimit, epsilon))
        index = xMaxLimit - epsilon;
    }
    return index;
  }

  // Tämä funktio laskee interpoloidun arvon matriisin datasta.
  // Annettu piste on halutussa suhteellisessa koordinaatistossa (esim. 0,0  -  1,1 maailmassa)
  // ja kyseinen koordinaatisto (rect-olio) pitää antaa parametrina.
  // HUOM! rect-olio ja matriisi ovat y-akselin suhteen käänteisiä!
  T InterpolatedValue(const NFmiPoint& thePoint,
                      const NFmiRect& theRelativeCoords,
                      FmiParameterName theParamId,
                      bool fDontInvertY = false,
                      FmiInterpolationMethod interp = kLinearly) const
  {
    T value = kFloatMissing;
    double xInd =
        ((NX() - 1) * (thePoint.X() - theRelativeCoords.Left())) / theRelativeCoords.Width();
    // yInd-laskussa pitää y-akseli kääntää
    double yInd =
        fDontInvertY
            ? ((NY() - 1) * (thePoint.Y() - theRelativeCoords.Top())) / theRelativeCoords.Height()
            : ((NY() - 1) *
               (theRelativeCoords.Height() - (thePoint.Y() - theRelativeCoords.Top()))) /
                  theRelativeCoords.Height();
    // xInd ja yInd voivat mennä juuri pikkuisen yli rajojen ja laskut menevät muuten pieleen, mutta
    // tässä korjataan indeksejä juuri pikkuisen, että laskut menevät näissä tapauksissa läpi ja
    // riittävän oikein arvoin.
    xInd = FixIndexOnEdges(xInd, NX());
    yInd = FixIndexOnEdges(yInd, NY());

    int x1 = static_cast<int>(std::floor(xInd));
    int y1 = static_cast<int>(std::floor(yInd));
    int x2 = x1 + 1;
    int y2 = y1 + 1;
    if (x1 >= 0 && x2 < static_cast<int>(NX()) && y1 >= 0 && y2 < static_cast<int>(NY()))
    {  // lasketaan tulos vain jos ollaan matriisin sisällä, tähän voisi reunoille laskea erikois
       // arvoja jos haluaa
      double xFraction = xInd - x1;
      double yFraction = yInd - y1;
      if (interp == kNearestPoint)
        return At(FmiRound(xInd), FmiRound(yInd));
      else
      {
        if (theParamId == kFmiWindDirection || theParamId == kFmiWaveDirection)
          value = static_cast<float>(NFmiInterpolation::ModBiLinear(
              xFraction, yFraction, At(x1, y2), At(x2, y2), At(x1, y1), At(x2, y1), 360));
        else if (theParamId == kFmiWindVectorMS)
          value = static_cast<float>(NFmiInterpolation::WindVector(
              xFraction, yFraction, At(x1, y2), At(x2, y2), At(x1, y1), At(x2, y1)));
        else
          value = static_cast<float>(NFmiInterpolation::BiLinear(
              xFraction, yFraction, At(x1, y2), At(x2, y2), At(x1, y1), At(x2, y1)));
      }
    }
    return value;
  }

  // Tämä funktio laskee interpoloidun arvon matriisin datasta.
  // Oletus annettu piste on aina 0,0  -  1,1 maailmassa ja lasketaan siihen halutut indeksit.
  T InterpolatedValue(const NFmiPoint& thePoint,
                      FmiParameterName theParamId,
                      bool fDontInvertY = false,
                      FmiInterpolationMethod interp = kLinearly) const
  {
    return InterpolatedValue(thePoint, g_defaultCoordinates, theParamId, fDontInvertY, interp);
    /*
                    T value = kFloatMissing;
                    double xInd = (NX()-1) * thePoint.X();
                    double yInd = (NY()-1) * (1. - thePoint.Y()); // y-koordinaatin kääntö!
                    int x1 = static_cast<int>(std::floor(xInd));
                    int y1 = static_cast<int>(std::floor(yInd));
                    int x2 = x1 + 1;
                    int y2 = y1 + 1;
                    if(x1 >= 0 && x2 < static_cast<int>(NX()) && y1 >= 0 && y2 <
       static_cast<int>(NY()))
                    { // lasketaan tulos vain jos ollaan matriisin sisällä, tähän voisi reunoille
       laskea erikois arvoja jos haluaa
                            double xFraction = xInd - x1;
                            double yFraction = yInd - y1;
                            value =
       static_cast<float>(NFmiInterpolation::BiLinear(xFraction,yFraction,At(x1, y2),At(x2,
       y2),At(x1, y1),At(x2, y1)));
                    }
                    return value;
    */
  }

  void DoErrorReporting(std::exception& e, int i, int j) const
  {
    std::string indexStr("NFmiDataMatrix size: ");
    indexStr += NFmiStringTools::Convert(itsNX);
    indexStr += " x ";
    indexStr += NFmiStringTools::Convert(itsNY);
    indexStr += " and indexies [x][y]: ";
    indexStr += NFmiStringTools::Convert(i);
    indexStr += " and ";
    indexStr += NFmiStringTools::Convert(j);
    indexStr += "\n";
    throw std::runtime_error(e.what() + std::string("\n") + indexStr);
  }

  //! Resize matrix to desired size, with given value for new elements.

  void Resize(size_type theNX, size_type theNY, const T& theValue = T())
  {
    if (itsNY == theNY && itsNX == theNX) return;

    itsNY = theNY;
    itsNX = theNX;
    this->resize(itsNX);
    for (size_type i = 0; i < itsNX; i++)
      this->operator[](i).resize(itsNY, theValue);
  }

  //! Remove the given row, e.g, matrix[*][j]

  void RemoveRow(size_type theY)
  {
    for (size_type i = 0; i < itsNX; i++)
      this->operator[](i).erase((this->operator[](i)).begin() + theY);
    itsNY--;
  }

  //! Remove the given column, e.g, matrix[i][*]

  void RemoveColumn(size_type theX)
  {
    typename NFmiDataMatrix::iterator it = this->begin();
    it += theX;
    this->erase(it);
    itsNX--;
  }

  //! Assignment operator : matrix = matrix

  NFmiDataMatrix<T>& operator=(const NFmiDataMatrix<T>& theMatrix)
  {
    Resize(theMatrix.NX(), theMatrix.NY());
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = theMatrix[i][j];
    return *this;
  }

  //! Assignment operator: matrix = value

  NFmiDataMatrix<T>& operator=(const T& theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = theValue;
    return *this;
  }

  //! Data replacement operator: matrix.Replace(source,target)

  void Replace(const T& theSourceValue, const T& theTargetValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        if (this->operator[](i)[j] == theSourceValue) this->operator[](i)[j] = theTargetValue;
  }

  //! Addition operator matrix += matrix

  NFmiDataMatrix<float>& operator+=(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatAdd(this->operator[](i)[j], theMatrix[i][j]);
    return *this;
  }

  //! Addition operator matrix += value
  NFmiDataMatrix<float>& operator+=(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatAdd(this->operator[](i)[j], theValue);
    return *this;
  }

  NFmiDataMatrix<float>& operator-=(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatAdd(this->operator[](i)[j], -theMatrix[i][j]);
    return *this;
  }

  NFmiDataMatrix<float>& operator-=(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatAdd(this->operator[](i)[j], -theValue);
    return *this;
  }

  NFmiDataMatrix<float>& operator*=(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMul(this->operator[](i)[j], theMatrix[i][j]);
    return *this;
  }

  NFmiDataMatrix<float>& operator*=(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMul(this->operator[](i)[j], theValue);
    return *this;
  }

  NFmiDataMatrix<float>& operator/=(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatDiv(this->operator[](i)[j], theMatrix[i][j]);
    return *this;
  }

  NFmiDataMatrix<float>& operator/=(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatDiv(this->operator[](i)[j], theValue);
    return *this;
  }

  void LinearCombination(const NFmiDataMatrix<float>& theMatrix, float weight1, float weight2)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] =
            FloatAdd(FloatMul(this->operator[](i)[j], weight1), FloatMul(theMatrix[i][j], weight2));
  }

  void Min(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMin(this->operator[](i)[j], theMatrix[i][j]);
  }

  void Min(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMin(this->operator[](i)[j], theValue);
  }

  void Max(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMax(this->operator[](i)[j], theMatrix[i][j]);
  }

  void Max(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMax(this->operator[](i)[j], theValue);
  }

  void SetMinMax(float theMinValue, float theMaxValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMinMax(this->operator[](i)[j], theMinValue, theMaxValue);
  }

 private:
  float FloatAdd(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing ||
        b == -kFloatMissing)  // HUOM! pitää ottaa huomioon myös negatiivinen puuttuva, koska esim.
                              // NFmiDataMatrix operator-= antaa b-arvot tänne negatiivisina
      return kFloatMissing;
    else
      return a + b;
  }

  float FloatMul(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing)
      return kFloatMissing;
    else
      return a * b;
  }

  float FloatDiv(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing || b == 0.0)
      return kFloatMissing;
    else
      return a / b;
  }

  float FloatMin(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing)
      return kFloatMissing;
    else
      return (a < b ? a : b);
  }

  float FloatMax(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing)
      return kFloatMissing;
    else
      return (a > b ? a : b);
  }

  float FloatMinMax(float a, float minLimit, float maxLimit) const
  {
    if (a == kFloatMissing || minLimit == kFloatMissing || maxLimit == kFloatMissing)
      return kFloatMissing;
    else
    {
      a = (a > maxLimit ? maxLimit : a);
      return (a < minLimit ? minLimit : a);
    }
  }
};

template <class T>
inline std::ostream& operator<<(std::ostream& s, const NFmiDataMatrix<T>& m)
{
  typedef typename NFmiDataMatrix<T>::size_type sz_type;
  sz_type rows = m.NY();
  sz_type columns = m.NX();

  s << static_cast<unsigned int>(columns) << " " << static_cast<unsigned int>(rows) << std::endl;

  if (m.PrintYInverted() == false)
  {
    for (sz_type j = 0; j < rows; j++)
    {
      for (sz_type i = 0; i < columns; i++)
        s << m[i][j] << " ";
      s << std::endl;
    }
  }
  else
  {  // tulostus käänteisessä rivi-järjestyksessä
    for (sz_type j = rows - 1; j >= 0; j--)
    {
      if (m.PrintIndexAxies()) s << j << "\t";
      for (sz_type i = 0; i < columns; i++)
        s << m[i][j] << " ";
      s << std::endl;

      if (j == 0)  // luulen että koska j on unsigned tyyppinen, pitää tässä tarkastella 0-riviä,
                   // koska for-loopissa j-- lauseke ei ikinä vie j:n arvoa negatiiviseksi
                   // (kierähtää 0:sta tosi isoksi luvuksi)
      {
        if (m.PrintIndexAxies())
        {
          for (sz_type i = 0; i < columns; i++)
            s << i << " ";
        }
        break;
      }
    }
  }
  return s;
}

template <class T>
inline std::istream& operator>>(std::istream& s, NFmiDataMatrix<T>& m)
{
  typedef typename NFmiDataMatrix<T>::size_type sz_type;
  sz_type rows = 0;
  sz_type columns = 0;

  s >> columns >> rows;
  m.Resize(columns, rows);
  for (sz_type j = 0; j < rows; j++)
  {
    for (sz_type i = 0; i < columns; i++)
      s >> m[i][j];
  }
  return s;
}

// ======================================================================
