// ======================================================================
/*!
 * \class NFmiAffine
 *
 * Mathematically, all transformations can be represented as 3x3
 * transformation matrices of the following form:
 * \f[
 * xyz
 * \f]
 *
 * Since only six values are used in the above 3x3 matrix, a
 * transformation matrix is also expressed as a vector:
 * \f$\left[a b c d e f\right]\f$.
 *
 * Transformations map coordinates and lengths from a new coordinate
 * system into a previous coordinate system:
 * \f[
 *    xyz
 * \f]
 * For more documentation on affine transformations see for example
 * the SVG specifications at W3C.
 */
// ======================================================================

#include "NFmiAffine.h"
#include <cmath>
#ifdef __BORLANDC__
using std::sin;
using std::cos;
#endif

namespace Imagine
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiAffine::~NFmiAffine() {}
// ----------------------------------------------------------------------
/*!
 * \brief Default constructor creates an identify transformation
 */
// ----------------------------------------------------------------------

NFmiAffine::NFmiAffine() : itsA(1.0), itsB(0.0), itsC(0.0), itsD(1.0), itsE(0.0), itsF(0.0) {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor for all 6 matrix elements
 */
// ----------------------------------------------------------------------

NFmiAffine::NFmiAffine(double theA, double theB, double theC, double theD, double theE, double theF)
    : itsA(theA), itsB(theB), itsC(theC), itsD(theD), itsE(theE), itsF(theF)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 */
// ----------------------------------------------------------------------

NFmiAffine::NFmiAffine(const NFmiAffine& theAffine)
    : itsA(theAffine.itsA),
      itsB(theAffine.itsB),
      itsC(theAffine.itsC),
      itsD(theAffine.itsD),
      itsE(theAffine.itsE),
      itsF(theAffine.itsE)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 */
// ----------------------------------------------------------------------

NFmiAffine& NFmiAffine::operator=(const NFmiAffine& theAffine)
{
  if (this != &theAffine)
  {
    itsA = theAffine.itsA;
    itsB = theAffine.itsB;
    itsC = theAffine.itsC;
    itsD = theAffine.itsD;
    itsE = theAffine.itsE;
    itsF = theAffine.itsF;
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add translation to the transformation
 */
// ----------------------------------------------------------------------

void NFmiAffine::Translate(double tx, double ty)
{
  // Multiply(1,0,0,1,tx,ty)

  itsE += itsA * tx + itsC * ty;
  itsF += itsB * tx + itsD * ty;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add uniform scaling to the transformation
 */
// ----------------------------------------------------------------------

void NFmiAffine::Scale(double s)
{
  // Multiply(NFmiAffine(s,0,0,s,0,0));
  itsA *= s;
  itsB *= s;
  itsC *= s;
  itsD *= s;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add general scaling to the transformation
 */
// ----------------------------------------------------------------------

void NFmiAffine::Scale(double sx, double sy)
{
  // Multiply(NFmiAffine(sx,0,0,sy,0,0));
  itsA *= sx;
  itsB *= sx;
  itsC *= sy;
  itsD *= sy;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add rotation to the transformation
 */
// ----------------------------------------------------------------------

void NFmiAffine::Rotate(double a)
{
  // Multiply(NFmiAffine(cos(a),sin(a),-sin(a),cos(a),0,0))

  const double pi180 = 3.141592658579323846 / 180.0;
  double ca = cos(a * pi180);
  double sa = sin(a * pi180);

  double oldA = itsA;
  double oldB = itsB;
  double oldC = itsC;
  double oldD = itsD;

  itsA = oldA * ca + oldC * sa;
  itsB = oldB * ca + oldD * sa;
  itsC = -oldA * sa + oldC * ca;
  itsD = -oldB * sa + oldD * ca;
}

// ----------------------------------------------------------------------
/*!
 * \brief Multiplication of affine transformations are done
 *        by post-multiplication
 */
// ----------------------------------------------------------------------

void NFmiAffine::Multiply(const NFmiAffine& theAffine)
{
  double oldA = itsA;
  double oldB = itsB;
  double oldC = itsC;
  double oldD = itsD;
  double oldE = itsE;
  double oldF = itsF;

  itsA = oldA * theAffine.itsA + oldC * theAffine.itsB;
  itsB = oldB * theAffine.itsA + oldD * theAffine.itsB;
  itsC = oldA * theAffine.itsC + oldC * theAffine.itsD;
  itsD = oldB * theAffine.itsC + oldD * theAffine.itsD;
  itsE = oldA * theAffine.itsE + oldC * theAffine.itsF + oldE;
  itsF = oldB * theAffine.itsE + oldD * theAffine.itsF + oldF;
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate transformed X-coordinate
 */
// ----------------------------------------------------------------------

double NFmiAffine::X(double x, double y) { return itsA * x + itsC * y + itsE; }
// ----------------------------------------------------------------------
/*!
 * \brief Calculate transformed Y-coordinate
 */
// ----------------------------------------------------------------------

double NFmiAffine::Y(double x, double y) { return itsB * x + itsD * y + itsF; }
}  // namespace Imagine

// ======================================================================
