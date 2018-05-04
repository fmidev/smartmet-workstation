// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace Imagine::NFmiApproximateBezierFit
 */
// ======================================================================
/*!
 * \namespace Imagine::NFmiApproximateBezierFit
 *
 * \brief Calculating an approximate bezier fit
 *
 * The original C-code was published in the Graphics Gems series
 * The code is available from the url
 * <a href="http://www1.acm.org/pubs/tog/GraphicsGems/gems/FitCurves.c">FitCurves.c</a>
 */
// ======================================================================

#include "NFmiApproximateBezierFit.h"
#include "NFmiBezierTools.h"
#include "NFmiCounter.h"
#include "NFmiPath.h"

#include <newbase/NFmiGeoTools.h>
#include <newbase/NFmiPoint.h>

#include <list>
#include <vector>
#include <utility>

using namespace std;

namespace Imagine
{
namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Bezier multiplier 0 = (1-u)^3
 */
// ----------------------------------------------------------------------

double B0(double u)
{
  double tmp = 1 - u;
  return (tmp * tmp * tmp);
}

// ----------------------------------------------------------------------
/*!
 * \brief Bezier multiplier 1 = 3u(1-u)^2
 */
// ----------------------------------------------------------------------

double B1(double u)
{
  double tmp = 1 - u;
  return (3 * u * tmp * tmp);
}

// ----------------------------------------------------------------------
/*!
 * \brief Bezier multiplier 2 = 3u^2(1-u)
 */
// ----------------------------------------------------------------------

double B2(double u)
{
  double tmp = 1 - u;
  return (3 * u * u * tmp);
}

// ----------------------------------------------------------------------
/*!
 * \brief Bezier multiplier 3 = u^3
 */
// ----------------------------------------------------------------------

double B3(double u) { return (u * u * u); }
// ----------------------------------------------------------------------
/*!
 * \brief Establish regular segment orientation
 */
// ----------------------------------------------------------------------

bool IsPositivelyOriented(const NFmiPathData& thePath)
{
  // no orientation for lines
  if (thePath.size() < 3) return true;
  // this calculates 2*polygon area with sign
  double sum = 0;
  for (unsigned int i = 0; i < thePath.size() - 1; i++)
  {
    // Must use doubles to avoid bad errors due to
    // cancellation. Also, the latter formula
    // is more accurate:
    // x1*y2-x2*y1 = x1*dy-dx*y1

    const double x1 = thePath[i].x;
    const double y1 = thePath[i].y;
    const double x2 = thePath[i + 1].x;
    const double y2 = thePath[i + 1].y;
    const double dx = x2 - x1;
    const double dy = y2 - y1;
    sum += x1 * dy - dx * y1;
  }

// for some reasing closing the segments will produce same area every time
// must check area formula validity
#if 0
	  const bool isclosed = (thePath.front().x == thePath.back().x &&
							 thePath.front().y == thePath.back().y);
	  if(!isclosed)
		sum += thePath.back().x*thePath.front().y-thePath.front().x*thePath.back().y;
#endif

  // positive orientation for positive area
  return (sum >= 0.0);
}

// ----------------------------------------------------------------------
/*!
 * \brief Reverse the path segment
 *
 */
// ----------------------------------------------------------------------

NFmiPath Reverse(const NFmiPath& thePath)
{
  const NFmiPathData& path = thePath.Elements();
  NFmiPath out;
  out.MoveTo(path.back().x, path.back().y);
  for (size_t i = path.size() - 1; i > 0; i--)
  {
    out.Add(NFmiPathElement(path[i].op, path[i - 1].x, path[i - 1].y));
  }
  return out;
}

// ----------------------------------------------------------------------
/*!
 * \brief Dot product
 */
// ----------------------------------------------------------------------

double DotProduct(const NFmiPoint& theLhs, const NFmiPoint& theRhs)
{
  return (theLhs.X() * theRhs.X() + theLhs.Y() * theRhs.Y());
}

// ----------------------------------------------------------------------
/*!
 * \brief Rescale tangent vector
 *
 * \param theTangent The tangent vector
 * \param theScale The new scale
 * \return The scaled tangent vector
 */
// ----------------------------------------------------------------------

NFmiPoint ScaleTangent(const NFmiPoint& theTangent, double theScale)
{
  const double x = theTangent.X();
  const double y = theTangent.Y();
  const double len = sqrt(x * x + y * y);
  if (len == 0)
    throw runtime_error("Failed to fit bezier curve due to a 0-tangent");
  else
    return NFmiPoint(x * theScale / len, y * theScale / len);
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct normalized tangent
 *
 * \param theX The x-part of the tangent vector
 * \param theY The y-part of the tangent vector
 * \return Normalized vector
 */
// ----------------------------------------------------------------------

NFmiPoint Tangent(double theX, double theY)
{
  const double len = sqrt(theX * theX + theY * theY);
  if (len == 0)
    return NFmiPoint(theX, theY);
  else
    return NFmiPoint(theX / len, theY / len);
}

// ----------------------------------------------------------------------
/*!
 * \brief Estimate left tangent
 *
 * \param thePath The path from which to estimate
 * \return The tangent estimate
 */
// ----------------------------------------------------------------------

NFmiPoint ComputeLeftTangent(const NFmiPathData& thePath)
{
  const double dx = thePath[1].x - thePath[0].x;
  const double dy = thePath[1].y - thePath[0].y;
  return Tangent(dx, dy);
}

// ----------------------------------------------------------------------
/*!
 * \brief Estimate right tangent
 *
 * \param thePath The path from which to estimate
 * \return The tangent estimate
 */
// ----------------------------------------------------------------------

NFmiPoint ComputeRightTangent(const NFmiPathData& thePath)
{
  const size_t n = thePath.size();
  const double dx = thePath[n - 2].x - thePath[n - 1].x;
  const double dy = thePath[n - 2].y - thePath[n - 1].y;
  return Tangent(dx, dy);
}

// ----------------------------------------------------------------------
/*!
 * \brief Estimate center tangent
 *
 * \param thePath The path from which to estimate
 * \param thePos The position
 * \return The tangent estimate at the position
 */
// ----------------------------------------------------------------------

NFmiPoint ComputeCenterTangent(const NFmiPathData& thePath, unsigned int thePos)
{
  // Assume we have points p2,p3,p4 and p3 is the split
  // point. Then if p2==p4, the tangent estimated at p3
  // is (0,0). Hence we must iterate further to get
  // some tangent estimate, we use p1,p5 and so on until
  // we get a result

  for (size_t d = 1; d < thePath.size(); d++)
  {
    size_t prev = (thePos >= d ? thePos - d : 0);
    size_t next = (thePos + d < thePath.size() ? thePos + d : thePath.size() - 1);

    double dx1 = thePath[prev].x - thePath[thePos].x;
    double dy1 = thePath[prev].y - thePath[thePos].y;
    double dx2 = thePath[thePos].x - thePath[next].x;
    double dy2 = thePath[thePos].y - thePath[next].y;
    double dx = (dx1 + dx2) / 2;
    double dy = (dy1 + dy2) / 2;

    if (dx != 0 || dy != 0) return Tangent(dx, dy);
  }

  // Make some stupid guess in case the entire curve is
  // a big spike.

  return Tangent(1, 0);
}

// ----------------------------------------------------------------------
/*!
 * \brief Estimate initial tangent
 *
 * Note: The code assumes a closed polygon.
 *
 * \param thePath The path from which to estimate
 * \return The tangent estimate
 */
// ----------------------------------------------------------------------

NFmiPoint ComputeInitialTangent(const NFmiPathData& thePath)
{
  const size_t n = thePath.size();

  const double dx1 = thePath[0].x - thePath[n - 2].x;
  const double dy1 = thePath[0].y - thePath[n - 2].y;

  const double dx2 = thePath[1].x - thePath[0].x;
  const double dy2 = thePath[1].y - thePath[0].y;

  const double dx = (dx1 + dx2) / 2;
  const double dy = (dy1 + dy2) / 2;

  if (dx != 0 && dy != 0) return Tangent(dx, dy);

  // We have a spike. We return a normal to the spike direction

  return Tangent(dy1, -dx1);
}

// ----------------------------------------------------------------------
/*!
 * \brief Evaluate a Bezier curve segment
 *
 * \param theBezier The Bezier segment, including the initial point
 * \param theValue The value at which to evaluate
 * \return The curve coordinate
 */
// ----------------------------------------------------------------------

NFmiPoint Bezier(const NFmiPathData& thePath, double theValue)
{
  NFmiPathData tmp = thePath;
  // triangle computation
  const size_t degree = thePath.size() - 1;
  for (size_t i = 1; i <= degree; i++)
    for (size_t j = 0; j <= degree - i; j++)
    {
      tmp[j].x = (1 - theValue) * tmp[j].x + theValue * tmp[j + 1].x;
      tmp[j].y = (1 - theValue) * tmp[j].y + theValue * tmp[j + 1].y;
    }
  return NFmiPoint(tmp[0].x, tmp[0].y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Parameterize a regular path segment
 *
 * Assign parameter values to digitized points using relative
 * distances between points.
 *
 * \param thePath The path data
 * \param theFirst The index of the first point
 * \param theLast The index of the last point
 * \return The parameters to digitized points
 */
// ----------------------------------------------------------------------

vector<double> ChordLengthParameterize(const NFmiPathData& thePath,
                                       unsigned int theFirst,
                                       unsigned int theLast)
{
  vector<double> out(theLast - theFirst + 1, 0);

  out[0] = 0;
  unsigned int i;
  for (i = theFirst + 1; i <= theLast; i++)
  {
    const double dist =
        NFmiGeoTools::Distance(thePath[i].x, thePath[i].y, thePath[i - 1].x, thePath[i - 1].y);
    out[i - theFirst] = out[i - theFirst - 1] + dist;
  }

  for (i = theFirst + 1; i <= theLast; i++)
  {
    out[i - theFirst] = out[i - theFirst] / out[theLast - theFirst];
  }

  return out;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return true if vector contains valid parameterization
 */
// ----------------------------------------------------------------------

bool IsValidParameterization(const vector<double>& theU)
{
  if (theU.size() < 2) return true;

  unsigned int i = 0;
  for (i = 0; i < theU.size(); i++)
  {
    if (theU[i] < 0 || theU[i] > 1) return false;
  }

  for (i = 1; i < theU.size(); i++)
  {
    if (theU[i] <= theU[i - 1]) return false;
  }
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Root finder
 *
 * Use Newton-Raphson iteration to find better root
 *
 * \param theBezier The Bezier curve
 * \param thePoint The point
 * \param theU The parameter value for the point
 * \return The root
 */
// ----------------------------------------------------------------------

double NewtonRaphsonRootFind(const NFmiPathData& thePath,
                             const NFmiPathElement& thePoint,
                             double theU)
{
  unsigned int i;

  // Compute Q(u)
  NFmiPoint Q_u = Bezier(thePath, theU);

  // Generate control vertices for Q'
  NFmiPath Q1;
  for (i = 0; i <= 2; i++)
    Q1.ConicTo(3 * (thePath[i + 1].x - thePath[i].x), 3 * (thePath[i + 1].y - thePath[i].y));

  // Generate control vertices for Q''
  NFmiPath Q2;
  for (i = 0; i <= 1; i++)
    Q2.LineTo(2 * (Q1.Elements()[i + 1].x - Q1.Elements()[i].x),
              2 * (Q1.Elements()[i + 1].y - Q1.Elements()[i].y));

  // Compute Q'(u) and Q''(u)

  NFmiPoint Q1_u = Bezier(Q1.Elements(), theU);
  NFmiPoint Q2_u = Bezier(Q2.Elements(), theU);

  // Compute f(u)/f'(u)

  double numerator = ((Q_u.X() - thePoint.x) * Q1_u.X() + (Q_u.Y() - thePoint.y) * Q1_u.Y());
  double denominator = ((Q1_u.X() * Q1_u.X() + Q1_u.Y() * Q1_u.Y() +
                         (Q_u.X() - thePoint.x) * Q2_u.X() + (Q_u.Y() - thePoint.y) * Q2_u.Y()));

  // u = u - f(u)/f'(u)
  // the safety check against division by zero was missing
  // from Graphics Gems code, and caused a few failures.
  // The original value is a good return value in such
  // cases.

  double uprime;
  if (denominator == 0)
    uprime = theU;
  else
    uprime = theU - numerator / denominator;

  return uprime;
}

// ----------------------------------------------------------------------
/*!
 * \brief Reparameterize
 *
 * Given a set of points and their parameterization, try to find a
 * better parameterization.
 *
 * \param thePath The path
 * \param theFirst The start index
 * \param theLast The end index
 * \param theU Old parameterization
 * \param theBezier The Bezier approximation
 * \return New parameterization
 */
// ----------------------------------------------------------------------

vector<double> Reparameterize(const NFmiPathData& thePath,
                              unsigned int theFirst,
                              unsigned int theLast,
                              const vector<double>& theU,
                              const NFmiPath& theBezier)
{
  const int n = theLast - theFirst + 1;
  vector<double> out;
  out.reserve(n);

  unsigned int i = 0;
  for (i = theFirst; i <= theLast; i++)
  {
    out.push_back(NewtonRaphsonRootFind(theBezier.Elements(), thePath[i], theU[i - theFirst]));
  }

  // Replace invalid parameterizations by linear interpolation
  for (i = 1; i < out.size() - 1; i++)
  {
    if (out[i] < 0 || out[i] > 1 || out[i] < out[i - 1] || out[i] > out[i + 1])
    {
      unsigned int j, k;
      for (j = i - 1; j > 0; j--)
        if (out[j] >= 0 && out[j] <= 1) break;
      for (k = i + 1; k < theLast; k++)
        if (out[k] >= 0 && out[k] <= 1 && out[k] > out[j]) break;
      if (k < theLast) out[i] = (out[j] * (k - i) + out[k] * (i - j)) / (k - j);
    }
  }

  return out;
}

// ----------------------------------------------------------------------
/*!
 * \brief Compute maximum error
 *
 * Find the maximum squared distance of digitized points to fitted curve
 *
 * \param thePath The path
 * \param theFirst The start index of the segment
 * \param theLast The end index of the segment
 * \param theBezier The approximation
 * \param theU The parameterization of points
 * \param theSplitPoint The point where the maximum error is found
 * \return Error estimate
 */
// ----------------------------------------------------------------------

double ComputeMaxError(const NFmiPathData& thePath,
                       unsigned int theFirst,
                       unsigned int theLast,
                       const NFmiPathData& theBezier,
                       const vector<double>& theU,
                       unsigned int& theSplitPoint)
{
  theSplitPoint = (theLast - theFirst + 1) / 2;
  double maxdist = 0.0;
  for (unsigned int i = theFirst + 1; i < theLast; i++)
  {
    NFmiPoint P = Bezier(theBezier, theU[i - theFirst]);
    const double dx = P.X() - thePath[i].x;
    const double dy = P.Y() - thePath[i].y;
    double dist = dx * dx + dy * dy;
    if (dist >= maxdist)
    {
      maxdist = dist;
      theSplitPoint = i;
    }
  }
  return maxdist;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if Bezier length is close match to polyline length
 *
 * We are tolerant here, since the purpose is only to prevent wild
 * control points. Hence we allow any Bezier length less than the
 * actual polyline, since then the control points are not wild,
 * and any Bezier at most twice as long as the polyline. The
 * latter condition then actually suffices.
 *
 * \param thePath The path
 * \param theFirst The start index of the segment
 * \param theLast The end index of the segment
 * \param theBezier The approximation
 * \return True if the fit is close enough
 */
// ----------------------------------------------------------------------

bool BezierLengthMatches(const NFmiPathData& thePath,
                         unsigned int theFirst,
                         unsigned int theLast,
                         const NFmiPath& theBezier)
{
  const double bezlen = NFmiBezierTools::BezierLength(theBezier, 0.01);

  double len = 0;
  for (unsigned int i = theFirst; i <= theLast; i++)
    len += NFmiGeoTools::Distance(thePath[i].x, thePath[i].y, thePath[i + 1].x, thePath[i + 1].y);

  return (bezlen <= 2 * len);
}

// ----------------------------------------------------------------------
/*!
 * \brief Find least-squares Bezier fit
 *
 * Use least-squares method to find Bezier control points for region
 *
 * \param thePath The path to approximate
 * \param theFirst The first path point
 * \param theLast The last path point
 * \param theU The parameter values
 * \param theTangent1 The left tangent
 * \param theTangent2 The right tangent
 * \return The Bezier path
 */
// ----------------------------------------------------------------------

NFmiPath GenerateBezier(const NFmiPathData& thePath,
                        unsigned int theFirst,
                        unsigned int theLast,
                        const vector<double>& theU,
                        const NFmiPoint& theTangent1,
                        const NFmiPoint& theTangent2)
{
  NFmiPath outpath;

  const unsigned int n = theLast - theFirst + 1;

  // Compute the A's

  unsigned int i;

  vector<pair<NFmiPoint, NFmiPoint> > A;
  A.reserve(thePath.size());
  for (i = 0; i < n; i++)
  {
    const NFmiPoint v1 = ScaleTangent(theTangent1, B1(theU[i]));
    const NFmiPoint v2 = ScaleTangent(theTangent2, B2(theU[i]));
    A.push_back(make_pair(v1, v2));
  }

  // Create the C and X matrices

  double C[2][2] = {{0, 0}, {0, 0}};
  double X[2] = {0, 0};

  const NFmiPoint p0(thePath[theFirst].x, thePath[theFirst].y);
  const NFmiPoint p3(thePath[theLast].x, thePath[theLast].y);

  for (i = 0; i < n; i++)
  {
    C[0][0] += DotProduct(A[i].first, A[i].first);
    C[0][1] += DotProduct(A[i].first, A[i].second);
    C[1][0] = C[0][1];
    C[1][1] += DotProduct(A[i].second, A[i].second);

    const double u = theU[i];
    const NFmiPoint p(thePath[theFirst + i].x, thePath[theFirst + i].y);

    NFmiPoint tmp = p - (p0 * (B0(u) + B1(u)) + p3 * (B2(u) + B3(u)));

    X[0] += DotProduct(A[i].first, tmp);
    X[1] += DotProduct(A[i].second, tmp);
  }

  // Compute the determinants of C and X

  double det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
  double det_C0_X = C[0][0] * X[1] - C[0][1] * X[0];
  double det_X_C1 = X[0] * C[1][1] - X[1] * C[0][1];

  // Finally, derive alpha values

  if (det_C0_C1 == 0.0)
  {
    det_C0_C1 = (C[0][0] * C[1][1]) * 10e-12;
  }
  const double alpha_l = det_X_C1 / det_C0_C1;
  const double alpha_r = det_C0_X / det_C0_C1;

  NFmiPoint tan1, tan2;

  //  If alpha negative, use the Wu/Barsky heuristic (see text)
  // (if alpha is 0, you get coincident control points that lead to
  // divide by zero in any subsequent NewtonRaphsonRootFind() call.
  if (alpha_l < 1e-6 || alpha_r < 1e-6)
  {
    // If alpha negative, use the Wu/Barsky heuristic
    const double dist = p0.Distance(p3) / 3;

    tan1 = ScaleTangent(theTangent1, dist);
    tan2 = ScaleTangent(theTangent2, dist);
  }
  else
  {
    // First and last control points of the Bezier curve are
    // positioned exactly at the first and last data points.
    // Control points 1 and 2 are positioned an alpha distance out
    // on the tangent vectors, left and right, respectively.

    tan1 = ScaleTangent(theTangent1, alpha_l);
    tan2 = ScaleTangent(theTangent2, alpha_r);
  }

  outpath.MoveTo(p0.X(), p0.Y());
  outpath.CubicTo(p0.X() + tan1.X(), p0.Y() + tan1.Y());
  outpath.CubicTo(p3.X() + tan2.X(), p3.Y() + tan2.Y());
  outpath.CubicTo(p3.X(), p3.Y());

  return outpath;
}

// ----------------------------------------------------------------------
/*!
 * \brief Utility subroutine for SimpleFit
 *
 * This generates a cubic Bezier fit recursively.
 *
 * \param thePath The path to approximate
 * \param theFirst The first index (0 at the start)
 * \param theLast The last index (path size -1 at the start)
 * \param theTangent1 The left tangent
 * \param theTangent2 The right tangent
 * \param theError The maximum allowed error squared
 * \return The Bezier path
 */
// ----------------------------------------------------------------------

NFmiPath RecursiveFit(const NFmiPathData& thePath,
                      unsigned int theFirst,
                      unsigned int theLast,
                      const NFmiPoint& theTangent1,
                      const NFmiPoint& theTangent2,
                      double theError)
{
  NFmiPath outpath;

  const unsigned int n = theLast - theFirst + 1;

  // Use heuristic if region has only two points in it
  if (n == 2)
  {
    const double x0 = thePath[theFirst].x;
    const double y0 = thePath[theFirst].y;
    const double x3 = thePath[theLast].x;
    const double y3 = thePath[theLast].y;

    const double dist = NFmiGeoTools::Distance(x0, y0, x3, y3) / 3;

    const NFmiPoint t1 = ScaleTangent(theTangent1, dist);
    const NFmiPoint t2 = ScaleTangent(theTangent2, dist);

    outpath.MoveTo(x0, y0);
    outpath.CubicTo(x0 + t1.X(), y0 + t1.Y());
    outpath.CubicTo(x3 + t2.X(), y3 + t2.Y());
    outpath.CubicTo(x3, y3);
    return outpath;
  }

  // Parameterize points and attempt to fit curve

  vector<double> u = ChordLengthParameterize(thePath, theFirst, theLast);

  outpath = GenerateBezier(thePath, theFirst, theLast, u, theTangent1, theTangent2);

  // find max deviation of points to fitted curve
  unsigned int splitpoint;
  double maxerror = ComputeMaxError(thePath, theFirst, theLast, outpath.Elements(), u, splitpoint);

  if (maxerror < theError)
  {
    // Make sure the curve lengths are roughly equal
    // to avoid Bezier fits with wild control points

    if (BezierLengthMatches(thePath, theFirst, theLast, outpath)) return outpath;
  }

  // if error not too large, try some reparameterization and iteration

  const double errorfactor = 4.0;
  const double iterationerror = errorfactor * theError;
  if (maxerror < iterationerror)
  {
    vector<double> uprime;
    const unsigned int maxiterations = 4;

    for (unsigned i = 0; i < maxiterations; i++)
    {
      uprime = Reparameterize(thePath, theFirst, theLast, u, outpath);
      // safety against bad roots - this was missing
      // from Graphics Gems
      if (!IsValidParameterization(uprime))
      {
        break;
      }

      outpath = GenerateBezier(thePath, theFirst, theLast, uprime, theTangent1, theTangent2);
      maxerror =
          ComputeMaxError(thePath, theFirst, theLast, outpath.Elements(), uprime, splitpoint);
      if (maxerror < theError)
        if (BezierLengthMatches(thePath, theFirst, theLast, outpath)) return outpath;
      u = uprime;
    }
  }

  // fitting failed - split at max error point and fit recursively

  NFmiPoint tangent1 = ComputeCenterTangent(thePath, splitpoint);
  NFmiPoint tangent2(-tangent1.X(), -tangent1.Y());

  NFmiPath part1 = RecursiveFit(thePath, theFirst, splitpoint, theTangent1, tangent1, theError);
  NFmiPath part2 = RecursiveFit(thePath, splitpoint, theLast, tangent2, theTangent2, theError);

  // Append all but the first move
  outpath = part1;
  for (int i = 1; i < part2.Size(); i++)
    outpath.Add(part2.Elements()[i]);

  return outpath;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate tolerance bezier approximation for regular path segment
 *
 * A regular path segment starts with a moveto, is followed by
 * several lineto commands and may or may not be closed.
 *
 * The algorithm is from Graphics Gems. The version by Philip J. Schneider
 * was used as the basis for the C++ implementation.
 *
 * \param thePath The path to cardinalize
 * \param theMaxError The maximum allowed error
 * \return The approximated path
 */
// ----------------------------------------------------------------------

NFmiPath SimpleFit(const NFmiPath& thePath, double theMaxError)
{
  // safety against too small paths
  if (thePath.Size() < 3) return thePath;

  const bool isclosed = NFmiBezierTools::IsClosed(thePath);

  const NFmiPathData& path = thePath.Elements();

  // we want to ensure positive orientation during algorithms
  // to guarantee equivalent results for paths which are
  // mirror images of each other

  if (!IsPositivelyOriented(path))
  {
    NFmiPath tmppath = Reverse(thePath);
    tmppath = SimpleFit(tmppath, theMaxError);
    return Reverse(tmppath);
  }

  // Estimate tangent vectors at endpoints of the curve
  // Note that tangents face "inward", i.e., towards the
  // interior of the set of sampled points

  const double squared_max_error = theMaxError * theMaxError;

  NFmiPoint tangent1, tangent2;

  if (!isclosed)
  {
    tangent1 = ComputeLeftTangent(path);
    tangent2 = ComputeRightTangent(path);
  }
  else
  {
    tangent1 = ComputeInitialTangent(path);
    tangent2 = NFmiPoint(-tangent1.X(), -tangent1.Y());
  }

  return RecursiveFit(path, 0, thePath.Size() - 1, tangent1, tangent2, squared_max_error);
}

}  // namespace anonymous

namespace NFmiApproximateBezierFit
{
// ----------------------------------------------------------------------
/*!
 * \brief Calculate a Bezier approximation
 *
 * The given tolerance determines how closely the approximation
 * must follow the original points. The larger the tolerance,
 * the smoother the approximation is likely to be (with less
 * control points in the path).
 *
 * \param thePath The path to approximate
 * \param theMaxError The maximum Euclidian distance error
 * \return The converted path
 */
// ----------------------------------------------------------------------

const NFmiPath Fit(const NFmiPath& thePath, double theMaxError)
{
  using namespace NFmiBezierTools;

  NFmiPath outpath;
  Segments segments = SplitSegments(thePath);
  for (Segments::const_iterator it = segments.begin(); it != segments.end(); ++it)
  {
    if (it->second)
      outpath.Add(SimpleFit(it->first, theMaxError));
    else
      outpath.Add(it->first);
  }

  return outpath;
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate multiple Bezier approximations
 *
 * Note that the different paths may have common subsegments
 * which must be fitted identically, otherwise for example
 * contour plots may show gaps.
 *
 * This function works only for flattened paths, conic
 * or cubic elements are not allowed in the input.
 *
 * The algorithm is:
 *  -# Calculate how many times each point occurs, taking
 *     care not to calculate closed subpath joinpoints twice
 *  -# Split each path to subsegments based on where the
 *     count of each endpoint and adjacent points reveals
 *     a join point between another path.
 *  -# Establish unique order for all the subsegments. For
 *     closed subsegments we must establish a unique starting
 *     point.
 *  -# Fit each subsegment separately.
 *  -# Join the subsegments back
 *
 * \param thePaths Vector of paths to fit
 * \param theMaxError The maximum allowed error
 * \return The fitted paths
 */
// ----------------------------------------------------------------------

const NFmiPaths Fit(const NFmiPaths& thePaths, double theMaxError)
{
  using namespace NFmiBezierTools;

  // Calculate the points

  NFmiCounter<NFmiPoint> counts = VertexCounts(thePaths);

  NFmiPaths outpaths;
  for (NFmiPaths::const_iterator it = thePaths.begin(); it != thePaths.end(); ++it)
  {
    PathList pathlist = SplitPath(*it, counts);
    NFmiPath outpath;
    for (PathList::const_iterator jt = pathlist.begin(); jt != pathlist.end(); ++jt)
    {
      NFmiPath fitpath = Fit(*jt, theMaxError);
      outpath.Add(fitpath);
    }
    outpaths.push_back(outpath);
  }

  return outpaths;
}

}  // namespace NFmiApproximateBezierFit

}  // namespace Imagine

// ======================================================================
