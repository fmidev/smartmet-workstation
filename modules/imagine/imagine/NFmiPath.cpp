// ======================================================================
//
// Definition of a PostScript style path, with a ghostline extension.
// ======================================================================

#include "NFmiPath.h"
#include "NFmiContourTree.h"
#include "NFmiCounter.h"
#include "NFmiEsriBox.h"

#include <newbase/NFmiGrid.h>
#include <newbase/NFmiValueString.h>

#ifdef IMAGINE_WITH_CAIRO
#include "ImagineXr.h"
#define CAIRO_LINE_WIDTH (1.0)
#endif

#include <algorithm>
#include <stdexcept>
#include <set>
#include <sstream>

// ======================================================================
//				HIDDEN INTERNAL FUNCTIONS
// ======================================================================

namespace
{
// Inside-out inversion limit
const double inside_out_limit = 1e8;

//! The number identifying the region within the rectangle
const int central_quadrant = 4;

//! Test the position of given point with respect to a rectangle.
int quadrant(double x, double y, double x1, double y1, double x2, double y2, double margin)
{
  int value = central_quadrant;
  if (x < x1 - margin)
    value--;
  else if (x > x2 + margin)
    value++;
  if (y < y1 - margin)
    value -= 3;
  else if (y > y2 + margin)
    value += 3;
  return value;
}

//! Test whether two rectangles intersect
bool intersects(
    double x1, double y1, double x2, double y2, double X1, double Y1, double X2, double Y2)
{
  bool xoutside = (x1 > X2 || x2 < X1);
  bool youtside = (y1 > Y2 || y2 < Y1);
  return (!xoutside && !youtside);
}

std::string numstring(double value, int precision)
{
  return NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, precision).CharPtr();
}

}  // anonymous namespace

namespace Imagine
{
// ----------------------------------------------------------------------
// Append a path using a line of desired type
// ----------------------------------------------------------------------

void NFmiPath::Add(const NFmiPath &thePath, bool fExact)
{
  // We cannot simply copy, since we wish to change the first
  // moveto in the path being appended into a lineto of the
  // type implied by fExact

  NFmiPathData::const_iterator iter = thePath.itsElements.begin();

  for (; iter != thePath.itsElements.end(); ++iter)
  {
    if (iter == thePath.itsElements.begin())
      Add(fExact ? kFmiLineTo : kFmiGhostLineTo, iter->x, iter->y);
    else
      Add(*iter);
  }
}

// ----------------------------------------------------------------------
// Insert a path using a line of desired type
// This requires using a loop, using insert for the list
// as a whole would insert the operations in their current
// order, but we actually want to reverse the path.
// ----------------------------------------------------------------------

void NFmiPath::Insert(const NFmiPath &thePath, bool fExact)
{
  // And insert one by one to get correct inverse order

  NFmiPathData::const_iterator iter = thePath.itsElements.begin();

  bool first = true;
  for (; iter != thePath.itsElements.end(); ++iter)
  {
    if ((first && fExact) || (!first && iter->op == kFmiLineTo))
      InsertLineTo(iter->x, iter->y);
    else
      InsertGhostLineTo(iter->x, iter->y);
    first = false;
  }
}

// ----------------------------------------------------------------------
// Append a reversed path using a line of desired type
// This is tricky, since the correct operation for
// a point must be taken from the adjacent element.
//
// First point to be appended gets the type of the edge
// Second point to be appended gets the type of the first point appended
//
// Note that the first point to be appended is at the end of the
// list, which means we must use reverse iterators.
// ----------------------------------------------------------------------

void NFmiPath::AddReverse(const NFmiPath &thePath, bool fExact)
{
  // Holder for the next op to be added

  NFmiPathOperation op = (fExact ? kFmiLineTo : kFmiGhostLineTo);

  // Iterate

  NFmiPathData::const_reverse_iterator iter = thePath.itsElements.rbegin();
  for (; iter != thePath.itsElements.rend(); ++iter)
  {
    Add(op, iter->x, iter->y);
    op = iter->op;
  }
}

// ----------------------------------------------------------------------
// Close the last subpath in the path with the given type of line operation.
// (kFmiLineTo or kFmiGhostLineTo).
// Algorithm: Find the last kFmiMoveTo coordinate, draw line to it
// ----------------------------------------------------------------------

void NFmiPath::DoCloseLineTo(NFmiPathOperation theOper)
{
  NFmiPathData::reverse_iterator iter;
  for (iter = itsElements.rbegin(); iter != itsElements.rend(); ++iter)
  {
    if (iter->op == kFmiMoveTo)
    {
      // The element to be added

      NFmiPathElement tmp(theOper, iter->x, iter->y);

      // Don't add if the last element is exactly the same

      if (!(itsElements.back() == tmp)) Add(tmp);
      break;
    }
  }
}

// ----------------------------------------------------------------------
// Append a path in string format.
//
// The expected format is
//
//      <operator> <x> <y>
//
// repeated over and over. The recognized operators are
//
//	M = moveto		m = relative moveto
//	L = lineto		l = relative lineto
//	G = ghostlineto		g = relative lineto
//	Q = conicto		q = relative conicto
//	C = cubicto		c = relative cubicto
//
// If the operator is omitted, the previous operator is assumed
// to be repeated. The previous operator is initialized to be
// a moveto before any parsing occurs.
//
// The following are valid examples which represents the same rectangle
//
//   M10,10 l-20,0 0,-20 20,0 0,20
//   M10,10 L-10,10 -10,-10 10,-10 10,10
//   M10,10 L-10,10 L-10,-10 L10,-10 L10,10
//
// Anything after a '#' character is ignored until a newline character
// ----------------------------------------------------------------------

using namespace std;

void NFmiPath::Add(const string &theString)
{
  NFmiPathOperation previous_op = kFmiMoveTo;
  double previous_x = 0;
  double previous_y = 0;
  bool previous_relative = false;

  unsigned int pos = 0;  // parser position

  int phase = 0;  // 0 = op, 1 = x, 2 = y

  double x = 0.0f;
  double y = 0.0f;
  bool relative = false;
  NFmiPathOperation op = kFmiMoveTo;

  while (pos < theString.size())
  {
    // Skip leading whitespace

    if (isspace(theString[pos]))
    {
      pos++;
      continue;
    }

    // Skip comments

    if (theString[pos] == '#')
    {
      while (pos < theString.size() && theString[pos] != '\n')
        pos++;
      continue;
    }

    // Parse operator

    if (phase == 0)
    {
      // Initialize operator to be the previous one

      relative = previous_relative;
      op = previous_op;

      // Assume there is an operator to be found

      char ch = theString[pos++];

      switch (toupper(ch))
      {
        case 'L':
          op = kFmiLineTo;
          relative = (ch != toupper(ch));
          break;
        case 'G':
          op = kFmiGhostLineTo;
          relative = (ch != toupper(ch));
          break;
        case 'M':
          op = kFmiMoveTo;
          relative = (ch != toupper(ch));
          break;
        case 'Q':
          op = kFmiConicTo;
          relative = (ch != toupper(ch));
          break;
        case 'C':
          op = kFmiCubicTo;
          relative = (ch != toupper(ch));
          break;
        default:
          pos--;  // correct backwards - there was no operator
      }
    }

    // Parse coordinates

    else
    {
      // Expecting some float, either x or y

      string asciinumber;
      while (pos < theString.size() && !isspace(theString[pos]) && theString[pos] != ',')
        asciinumber += theString[pos++];

      // Skip the possible comma we may have found

      if (theString[pos] == ',') pos++;

      double number = atof(asciinumber.c_str());

      if (phase == 1)
        x = number;
      else
        y = number;
    }

    // Output movement

    if (phase == 2)
    {
      if (relative)
      {
        x += previous_x;
        y += previous_y;
      }
      Add(op, x, y);
      previous_op = op;
      previous_x = x;
      previous_y = y;
      previous_relative = relative;
    }
    phase = (phase + 1) % 3;
  }
}

// ----------------------------------------------------------------------
// Simplify a path using the Douglas-Peucker algorithm.
// The input specifies the maximum allowed error in the recursive
// algorithm for deciding when to discard a point. A zero value
// is allowed, and means that no error is allowed. Simplifications
// may however occur, since we may have two moves which form a
// straight line, and hence can be simplified into a single move.
//
// Note that a sequence of moves can be simplified if and only
// if the type of the simplified movement is the same as of the
// original more complicated movement. That is, if a line changes
// into a ghostline, we must simplify the line part and the ghostline
// parts separately. Similarly a moveto *always* acts as a separator
// of different paths to be simplified.
// ----------------------------------------------------------------------

void NFmiPath::Simplify(double epsilon)
{
  if (epsilon < 0.0) return;

  // Count the important points

  NFmiCounter<pair<double, double> > counter;

  NFmiPathData::const_iterator iter;
  for (iter = Elements().begin(); iter != Elements().end(); ++iter)
  {
    switch (iter->op)
    {
      case kFmiMoveTo:
      case kFmiLineTo:
      case kFmiGhostLineTo:
        counter.Add(make_pair(iter->x, iter->y));
        break;
      default:
        break;
    }
  }

  // Then simplify all linear segments of equal type
  //
  // a) Optional one moveto + lineto's
  // b) Optional one moveto + ghostlineto's
}

// ----------------------------------------------------------------------
// Translate the path by the given amount
// ----------------------------------------------------------------------

void NFmiPath::Translate(double theX, double theY)
{
  NFmiPathData::iterator iter;

  for (iter = itsElements.begin(); iter != itsElements.end(); ++iter)
  {
    iter->x += theX;
    iter->y += theY;

    //(*iter).X((*iter).X()+theX);	// X += theX
    //(*iter).Y((*iter).Y()+theY);	// Y += theY
  }
}

// ----------------------------------------------------------------------
// Scale the path by the given amount
// ----------------------------------------------------------------------

void NFmiPath::Scale(double theScale) { Scale(theScale, theScale); }
// ----------------------------------------------------------------------
// Scale the path by the given amounts in x- and y-directions
// ----------------------------------------------------------------------

void NFmiPath::Scale(double theXScale, double theYScale)
{
  NFmiPathData::iterator iter;

  for (iter = itsElements.begin(); iter != itsElements.end(); ++iter)
  {
    iter->x *= theXScale;
    iter->y *= theYScale;

    //(*iter).X((*iter).X()*theXScale);
    //(*iter).Y((*iter).Y()*theYScale);
  }
}

// ----------------------------------------------------------------------
// Rotate the path by the given amount
// ----------------------------------------------------------------------

void NFmiPath::Rotate(double theAngle)
{
  const double pi = 3.14159265358979f;

  NFmiPathData::iterator iter;

  double cosa = cos(theAngle * pi / 180);
  double sina = sin(theAngle * pi / 180);

  for (iter = itsElements.begin(); iter != itsElements.end(); ++iter)
  {
    double x = iter->x;
    double y = iter->y;
    iter->x = x * cosa + y * sina;
    iter->y = -x * sina + y * cosa;
  }
}

// ----------------------------------------------------------------------
// Apply affine transformation on the path
// ----------------------------------------------------------------------

#if 0
  void NFmiPath::Transform(NFmiAffine & theAffine)
  {
	NFmiPathData::iterator iter;
	
	for(iter=itsElements.begin(); iter!=itsElements.end(); ++iter)
	  {
		double x = iter->x;
		double y = iter->y;
		iter->x= theAffine.X( x,y );
		iter->y= theAffine.Y( x,y );
	  }
  }
#endif

// ----------------------------------------------------------------------
// Apply alignment on the path
// ----------------------------------------------------------------------

void NFmiPath::Align(NFmiAlignment theAlignment, double theX, double theY)
{
  NFmiEsriBox box = BoundingBox();

  double xfactor = XAlignmentFactor(theAlignment);
  double yfactor = YAlignmentFactor(theAlignment);

  Translate(theX - (box.Xmin() * (1 - xfactor) + box.Xmax() * xfactor),
            theY - (box.Ymin() * (1 - yfactor) + box.Ymax() * yfactor));
}

// ----------------------------------------------------------------------
// Apply a projection to a path
// Note: The path and the area may be in different views
// ----------------------------------------------------------------------

void NFmiPath::Project(const NFmiArea *const theArea)
{
  if (!theArea) return;

  bool path_is_pacific = IsPacificView();
  bool area_is_pacific = theArea->PacificView();

  if (path_is_pacific && !area_is_pacific)
  {
    NFmiPath p = AtlanticView(true);
    itsElements.swap(p.itsElements);
  }
  else if (!path_is_pacific && area_is_pacific)
  {
    NFmiPath p = PacificView(true);
    itsElements.swap(p.itsElements);
  }

  NFmiPathData::iterator iter;
  for (iter = itsElements.begin(); iter != itsElements.end(); ++iter)
  {
    NFmiPoint pt = theArea->ToXY(NFmiPoint(iter->x, iter->y));
    iter->x = pt.X();
    iter->y = pt.Y();
  }
}

// ----------------------------------------------------------------------
// Apply an inverse projection to a path
// ----------------------------------------------------------------------

void NFmiPath::InvProject(const NFmiArea *const theArea)
{
  if (theArea != 0)
  {
    NFmiPathData::iterator iter;
    for (iter = itsElements.begin(); iter != itsElements.end(); ++iter)
    {
      NFmiPoint pt = theArea->ToLatLon(NFmiPoint(iter->x, iter->y));
      iter->x = pt.X();
      iter->y = pt.Y();
    }
  }
}

// ----------------------------------------------------------------------
// Apply an grid projection to a path
// ----------------------------------------------------------------------

void NFmiPath::InvGrid(const NFmiGrid *const theGrid)
{
  if (theGrid != 0)
  {
    NFmiPathData::iterator iter;
    for (iter = itsElements.begin(); iter != itsElements.end(); ++iter)
    {
      NFmiPoint pt = theGrid->GridToLatLon(iter->x, iter->y);
      iter->x = pt.X();
      iter->y = pt.Y();
    }
  }
}

// ----------------------------------------------------------------------
// Return the bounding box of the path.
// Note: Bezier curve bounding boxes not implemented yet
// ----------------------------------------------------------------------

NFmiEsriBox NFmiPath::BoundingBox() const
{
  NFmiEsriBox box;

  if (itsInsideOut)
  {
    box.Update(-inside_out_limit, -inside_out_limit);
    box.Update(inside_out_limit, inside_out_limit);
  }
  else
  {
    NFmiPathData::const_iterator iter;

    for (iter = itsElements.begin(); iter != itsElements.end(); ++iter)
    {
      switch (iter->op)
      {
        case kFmiMoveTo:
        case kFmiLineTo:
        case kFmiGhostLineTo:
        case kFmiConicTo:
        case kFmiCubicTo:
          box.Update(iter->x, iter->y);
          break;
      }
    }
  }

  return box;
}

// ----------------------------------------------------------------------
// Simplify by joining colinear consecutive line segments into
// one longer line segment, provided the points are monotonously
// ordered.
//
// Note: While examining any coordinate, we add the fixed offset
//       given by the user to any value. In particular, when
//       the coordinates are expected to be say from 0 to 400,
//       it may happen that due to rounding errors coordinates
//       near zero might not be considered colinear. Using
//       offset=1 should in general fix the problem. Using
//	 offset=0 will use the exact original coordinates.
//
// This simplification ignores possible important points.
// ----------------------------------------------------------------------

void NFmiPath::SimplifyLines(double theOffset)
{
  NFmiPathData::iterator iter = itsElements.begin();
  NFmiPathData::iterator end = itsElements.end();

  NFmiPathData newelements;

  // Cached previous path element:

  NFmiPathOperation oper1, oper2, oper3;
  double x1, y1, x2, y2, x3, y3;

  oper1 = oper2 = oper3 = kFmiMoveTo;
  x1 = y1 = x2 = y2 = x3 = y3 = kFloatMissing;

  int cachesize = 0;

  while (iter != end)
  {
    // Shift old data backwards

    oper1 = oper2;
    oper2 = oper3;
    x1 = x2;
    x2 = x3;
    y1 = y2;
    y2 = y3;

    // Establish new data

    newelements.push_back(*iter);

    oper3 = iter->op;
    x3 = theOffset + iter->x;
    y3 = theOffset + iter->y;

    // Delete-iterator is current element,

    ++iter;

    // Get atleast 3 points before doing anything

    //      cachesize = min(3,cachesize+1);
    cachesize = FmiMin(3, cachesize + 1);

    if (cachesize < 3) continue;

    // Now, if the last 2 operations are of equal lineto-type,
    // we may simplify the sequence of 3 points:
    //
    // A--B--C becomes A---C if B is somewhere on the
    // line connecting A and C.

    if (oper2 == oper3 && (oper2 == kFmiLineTo || oper2 == kFmiGhostLineTo))
    {
      // The line cannot be straight unless it is monotonous

      if (x1 < x2 && x2 > x3) continue;
      if (x1 > x2 && x2 < x3) continue;
      if (y1 < y2 && y2 > y3) continue;
      if (y1 > y2 && y2 < y3) continue;

      // Vertical and horizontal lines are easily tested
      // General case lines are compared based on their
      // angles: dy/dx must be equal so that
      //       (y2-y1)/(x2-x1) == (y3-y2)/(x3-x2)
      // <==>  (y2-y1)(x3-x2) == (y3-y2)(x2-x1)
      // We do not allow for rounding errors in the test,
      // it's probably not worth the trouble.

      if ((x1 == x2 && x2 == x3) || (y1 == y2 && y2 == y3) ||
          ((x3 - x1) * (y2 - y1) - (y3 - y1) * (x2 - x1) == 0.0))
      {
        NFmiPathElement last = newelements.back();
        newelements.pop_back();
        newelements.pop_back();
        newelements.push_back(last);
        cachesize--;

        oper2 = oper1;
        x2 = x1;
        y2 = y1;
      }
    }
  }

  swap(itsElements, newelements);
}

// ----------------------------------------------------------------------
// Output path in textual form
// ----------------------------------------------------------------------

std::ostream &operator<<(std::ostream &os, const NFmiPath &thePath)
{
  NFmiPathData::const_iterator iter = thePath.Elements().begin();

  if (thePath.IsInsideOut()) os << "INSIDEOUT ";

  for (; iter != thePath.Elements().end(); ++iter)
  {
    // Special code for first move

    if (iter != thePath.Elements().begin()) os << " ";

    if (iter->op == kFmiMoveTo)
      os << 'M';
    else if (iter->op == kFmiLineTo)
      os << 'L';
    else if (iter->op == kFmiGhostLineTo)
      os << 'G';
    else if (iter->op == kFmiConicTo)
      os << 'Q';
    else if (iter->op == kFmiCubicTo)
      os << 'C';
    else
      os << '?';

    os << iter->x << "," << iter->y;
  }
  return os;
}

// ----------------------------------------------------------------------
// Return a SVG-string representation of the path
// If relative_moves=true, relative movements are preferred over
// absolute moves. This usually generates shorter SVG.
// Tulostetaan polku vain kolmen desimaalin tarkkuudella (kaksi desimaali
// ei riittänyt, kun tuli pieni väli maiden välille?).
// ----------------------------------------------------------------------

string NFmiPath::SVG(bool relative_moves, bool removeghostlines) const
{
  // Note: Do NOT use output string streams, atleast not with
  //       g++ v2.92. The implementation is broken, and will
  //       mess up the output string. Instead we reserve a reasonable
  //       amount of space for the string from the beginning in the
  //       hope that output will be reasonably fast.

  string os;

  double last_x, last_y;
  double last_out_x, last_out_y;
  NFmiPathOperation last_op = kFmiMoveTo;

  last_x = last_y = kFloatMissing;
  last_out_x = last_out_y = kFloatMissing;

  int count_conic = 0;
  int count_cubic = 0;

  NFmiPathData::const_iterator iter = Elements().begin();

  for (; iter != Elements().end(); ++iter)
  {
#ifdef _MSC_VER  // MSVC:n stringi on paska kun se täyttyy ei sitä kasvateta tarpeeksi
    if (os.size() > 0.9 * os.capacity()) os.reserve(os.size() * 2);
#endif
    const double x = iter->x;
    const double y = iter->y;
    const NFmiPathOperation op = iter->op;

    switch (op)
    {
      case kFmiConicTo:
        count_cubic = 0;
        count_conic++;
        break;
      case kFmiCubicTo:
        count_conic = 0;
        count_cubic++;
        break;
      default:
        count_conic = 0;
        count_cubic = 0;
    }

    // Special code for first move

    bool out_ok = true;
    if (removeghostlines && op == kFmiGhostLineTo)
    {
      out_ok = false;
    }
    else
    {
      // If ghostlines are being ignored, we must output a moveto
      // when the ghostlines end and next operation is not moveto
      if (removeghostlines && (last_op == kFmiGhostLineTo) &&
          (op != kFmiGhostLineTo && op != kFmiMoveTo))
      {
        if (relative_moves)
        {
          if (last_out_x == kFloatMissing && last_out_y == kFloatMissing)
          {
            os += 'm';
            os += numstring(last_x, 3) + string(",");
            os += numstring(last_y, 3);
          }
          else
          {
            os += " m";
            os += numstring(last_x - last_out_x, 3) + string(",");
            os += numstring(last_y - last_out_y, 3);
          }
        }
        else
        {
          os += " M";
          os += numstring(last_x, 3) + string(",");
          os += numstring(last_y, 3);
        }
        last_op = kFmiMoveTo;
        last_out_x = last_x;
        last_out_y = last_y;
      }

      if (iter == Elements().begin())
      {
        os += (relative_moves ? "m" : "M");
        os += numstring(x, 3) + string(",");
        os += numstring(y, 3);
      }

      // Relative moves are "m dx dy" and "l dx dy" etc
      else if (relative_moves)
      {
        switch (op)
        {
          case kFmiMoveTo:
            os += (last_op == kFmiMoveTo ? " " : " m");
            break;
          case kFmiLineTo:
            os += ((last_op == kFmiLineTo || last_op == kFmiGhostLineTo) ? " " : " l");
            break;
          case kFmiGhostLineTo:
            if (!removeghostlines)
              os += ((last_op == kFmiLineTo || last_op == kFmiGhostLineTo) ? " " : " l");
            else
              os += ' ';
            break;
          case kFmiConicTo:
            os += (last_op == kFmiConicTo ? " " : " q");
            out_ok = (count_conic > 1);
            break;
          case kFmiCubicTo:
            os += (last_op == kFmiCubicTo ? " " : " c");
            out_ok = (count_conic > 2);
            break;
        }

        os += numstring(x - last_out_x, 3) + string(",");
        os += numstring(y - last_out_y, 3);
      }

      // Absolute moves are "M x y" and "L x y" etc
      else
      {
        switch (op)
        {
          case kFmiMoveTo:
            os += (last_op == kFmiMoveTo ? " " : " M");
            break;
          case kFmiLineTo:
            os += ((last_op == kFmiLineTo || last_op == kFmiGhostLineTo) ? " " : " L");
            break;
          case kFmiGhostLineTo:
            if (!removeghostlines)
              os += ((last_op == kFmiLineTo || last_op == kFmiGhostLineTo) ? " " : " L");
            else
              os += ' ';
          case kFmiConicTo:
            os += (last_op == kFmiConicTo ? " " : " Q");
            break;
          case kFmiCubicTo:
            os += (last_op == kFmiCubicTo ? " " : " C");
            break;
        }

        os += numstring(x, 3) + string(",");
        os += numstring(y, 3);
      }
    }

    last_op = op;

    last_x = x;
    last_y = y;
    if (out_ok)
    {
      last_out_x = x;
      last_out_y = y;
    }
  }

  if (!removeghostlines && itsInsideOut)
  {
    if (inside_out_limit != 1e8) throw runtime_error("Internal error in NFmiPath inside_out_limit");
    os += "M -1e8,-1e8 L -1e8,1e8 L 1e8,1e8 L 1e8,-1e8 Z";
  }

  return os;
}

// ----------------------------------------------------------------------
// Convert floating point number to string for the benefit of SVG() method
// ----------------------------------------------------------------------

string NFmiPath::ftoa(double theValue) const
{
  ostringstream str;
  str << theValue;
  return str.str();
}

// ----------------------------------------------------------------------
// Add the path to a fill map
// ----------------------------------------------------------------------

#ifndef IMAGINE_WITH_CAIRO
void NFmiPath::Add(NFmiFillMap &theMap) const
{
  // Data holders for moves. 1 is the newest, 4 the oldest

  NFmiPathOperation op1 = kFmiMoveTo;
  NFmiPathOperation op2 = kFmiMoveTo;
  NFmiPathOperation op3 = kFmiMoveTo;
  double x1 = kFloatMissing;
  double x2 = kFloatMissing;
  double x3 = kFloatMissing;
  double x4 = kFloatMissing;
  double y1 = kFloatMissing;
  double y2 = kFloatMissing;
  double y3 = kFloatMissing;
  double y4 = kFloatMissing;

  // The iterator for traversing the data

  NFmiPathData::const_iterator iter = Elements().begin();

  for (; iter != Elements().end(); ++iter)
  {
    x1 = iter->x;
    y1 = iter->y;
    op1 = iter->op;

    switch (iter->op)
    {
      case kFmiMoveTo:
        break;

      case kFmiLineTo:
      case kFmiGhostLineTo:
      {
        switch (op2)
        {
          case kFmiConicTo:
            theMap.AddConic(x3, y3, x2, y1, x1, y1);  // Conic segment
            break;
          case kFmiCubicTo:
            if (op3 == kFmiCubicTo)
              theMap.AddCubic(
                  x4, y4, x4, y3, x2, y2, x1, y1);  // AKa 7-Aug-08: clearly a BUG: x4->x3
            break;
          default:
            theMap.Add(x2, y2, x1, y1);  // Line segment
            break;
        }
      }

      case kFmiConicTo:
        break;
      case kFmiCubicTo:
        break;
    }

    // Update movement history

    op3 = op2;
    op2 = op1;
    x4 = x3;
    x3 = x2;
    x2 = x1;
    y4 = y3;
    y3 = y2;
    y2 = y1;
  }

  if (itsInsideOut)
  {
    theMap.Add(-inside_out_limit, -inside_out_limit, -inside_out_limit, inside_out_limit);
    theMap.Add(-inside_out_limit, inside_out_limit, inside_out_limit, inside_out_limit);
    theMap.Add(inside_out_limit, inside_out_limit, inside_out_limit, -inside_out_limit);
    theMap.Add(inside_out_limit, -inside_out_limit, -inside_out_limit, -inside_out_limit);
  }
}
#endif

// ----------------------------------------------------------------------
// Stroke onto given image using various Porter-Duff rules
// ----------------------------------------------------------------------

#ifndef IMAGINE_WITH_CAIRO
void NFmiPath::Stroke(ImagineXr_or_NFmiImage &img,
                      NFmiColorTools::Color theColor,
                      NFmiColorTools::NFmiBlendRule theRule) const
{
  // Quick exit if color is not real

  if (theColor == NFmiColorTools::NoColor) return;

  // Current point is not defined yet

  double lastX = kFloatMissing;
  double lastY = kFloatMissing;

  NFmiPathData::const_iterator iter = Elements().begin();

  for (; iter != Elements().end(); ++iter)
  {
    // Next point

    double nextX = iter->x;
    double nextY = iter->y;

    if (iter->op == kFmiConicTo || iter->op == kFmiCubicTo)
      throw std::runtime_error(
          "Conic and Cubic control points not supported in NFmiPath::Stroke()");

    // Only LineTo operations get rendered

    if (iter->op == kFmiLineTo)
      if (lastX != kFloatMissing && lastY != kFloatMissing && nextX != kFloatMissing &&
          nextY != kFloatMissing)
      {
        img.StrokeBasic(lastX, lastY, nextX, nextY, theColor, theRule);
      }

    // New last point

    lastX = nextX;
    lastY = nextY;
  }
}
#endif

// ----------------------------------------------------------------------
// Stroke onto given image using various Porter-Duff rules
// ----------------------------------------------------------------------

void NFmiPath::Stroke(ImagineXr_or_NFmiImage &img,
                      double theWidth,
                      NFmiColorTools::Color theColor,
                      NFmiColorTools::NFmiBlendRule theRule) const
{
  // Quick exit if color is not real

  if (theColor == NFmiColorTools::NoColor) return;

  if (theWidth <= 0) return;

/*
* Important that Cairo drawings are done the whole path at once;
* cutting to thousands of moveto/lineto segments kills performance,
* especially when creating PDF output (Cairo 1.6.4).
*/
#ifdef IMAGINE_WITH_CAIRO
  img.Stroke(itsElements, theWidth, theColor, theRule);
#else
  // Current point is not defined yet

  double lastX = kFloatMissing;
  double lastY = kFloatMissing;

  NFmiPathData::const_iterator iter = Elements().begin();

  for (; iter != Elements().end(); ++iter)
  {
    // Next point

    double nextX = iter->x;
    double nextY = iter->y;

    if (iter->op == kFmiConicTo || iter->op == kFmiCubicTo)
      throw std::runtime_error(
          "Conic and Cubic control points not supported in NFmiPath::Stroke()");

    // Only LineTo operations get rendered

    if (iter->op == kFmiLineTo)
      if (lastX != kFloatMissing && lastY != kFloatMissing && nextX != kFloatMissing &&
          nextY != kFloatMissing)
      {
        // Calculate wide line box coordinates

        if (lastX != nextX || lastY != nextY)
        {
          // Emulate thick line with a box
          NFmiPath box;
          double dx = nextX - lastX;
          double dy = nextY - lastY;
          double alpha = atan2(dy, dx);

          box.MoveTo(lastX - theWidth / 2 * sin(alpha), lastY + theWidth / 2 * cos(alpha));
          box.LineTo(lastX + theWidth / 2 * sin(alpha), lastY - theWidth / 2 * cos(alpha));
          box.LineTo(nextX + theWidth / 2 * sin(alpha), nextY - theWidth / 2 * cos(alpha));
          box.LineTo(nextX - theWidth / 2 * sin(alpha), nextY + theWidth / 2 * cos(alpha));
          box.CloseLineTo();
          box.Fill(img, theColor, theRule);
        }
      }

    // New last point

    lastX = nextX;
    lastY = nextY;
  }
#endif
}

#ifndef IMAGINE_WITH_CAIRO
NFmiPath NFmiPath::Clip(
    double theX1, double theY1, double theX2, double theY2, double theMargin) const
{
  if (itsElements.empty()) return *this;

  NFmiPath outPath;
  NFmiPath tmpPath;

  const NFmiPathData::const_iterator begin = Elements().begin();
  const NFmiPathData::const_iterator end = Elements().end();
  int last_quadrant = 0;
  int this_quadrant = 0;

  // Initialize the new bounding box
  double minx = 0;
  double miny = 0;
  double maxx = 0;
  double maxy = 0;

  double lastX = 0;
  double lastY = 0;
  NFmiPathOperation lastOp = kFmiMoveTo;

  bool last_ignored = false;

  for (NFmiPathData::const_iterator iter = begin; iter != end;)
  {
    double X = iter->x;
    double Y = iter->y;
    NFmiPathOperation op = iter->op;
    ++iter;

    this_quadrant = quadrant(X, Y, theX1, theY1, theX2, theY2, theMargin);

    switch (op)
    {
      case kFmiMoveTo:
      {
        if (tmpPath.Size() > 0 && intersects(minx,
                                             miny,
                                             maxx,
                                             maxy,
                                             theX1 - theMargin,
                                             theY1 - theMargin,
                                             theX2 + theMargin,
                                             theY2 + theMargin))
        {
          outPath.Add(tmpPath);
        }
        tmpPath.Clear();
        tmpPath.Add(op, X, Y);
        minx = maxx = X;
        miny = maxy = Y;
        lastOp = op;
        lastX = X;
        lastY = Y;
        last_quadrant = this_quadrant;
        last_ignored = false;
        break;
      }
      case kFmiLineTo:
      case kFmiGhostLineTo:
      {
        if (this_quadrant == central_quadrant || this_quadrant != last_quadrant)
        {
          if (last_ignored) tmpPath.Add(lastOp, lastX, lastY);
          tmpPath.Add(op, X, Y);
          last_ignored = false;
        }
        else
          last_ignored = true;

        minx = min(minx, X);
        miny = min(miny, Y);
        maxx = max(maxx, X);
        maxy = max(maxy, Y);
        lastOp = op;
        lastX = X;
        lastY = Y;
        last_quadrant = this_quadrant;
        break;
      }
      case kFmiConicTo:
      {
        double X2 = iter->x;
        double Y2 = iter->y;
        ++iter;

        int end_quadrant = quadrant(X2, Y2, theX1, theY1, theX2, theY2, theMargin);
        if (end_quadrant == central_quadrant || end_quadrant != this_quadrant ||
            end_quadrant != last_quadrant)
        {
          if (last_ignored) tmpPath.Add(lastOp, lastX, lastY);
          tmpPath.Add(op, X, Y);
          tmpPath.Add(op, X2, Y2);
          last_ignored = false;
        }
        else
          last_ignored = true;
        lastOp = kFmiLineTo;
        lastX = X2;
        lastY = Y2;
        last_quadrant = end_quadrant;
        minx = min(minx, X);
        minx = min(minx, X2);
        miny = min(miny, Y);
        miny = min(miny, Y2);
        maxx = max(maxx, X);
        maxx = max(maxx, X2);
        maxy = max(maxy, Y);
        maxy = max(maxy, Y2);

        break;
      }
      case kFmiCubicTo:
      {
        double X2 = iter->x;
        double Y2 = iter->y;
        ++iter;

        double X3 = iter->x;
        double Y3 = iter->y;
        ++iter;

        int middle_quadrant = quadrant(X2, Y2, theX1, theY1, theX2, theY2, theMargin);
        int end_quadrant = quadrant(X3, Y3, theX1, theY1, theX2, theY2, theMargin);
        if (end_quadrant == central_quadrant || end_quadrant != this_quadrant ||
            end_quadrant != middle_quadrant || end_quadrant != last_quadrant)
        {
          if (last_ignored) tmpPath.Add(lastOp, lastX, lastY);
          tmpPath.Add(op, X, Y);
          tmpPath.Add(op, X2, Y2);
          tmpPath.Add(op, X3, Y3);
          last_ignored = false;
        }
        else
          last_ignored = true;
        lastOp = kFmiLineTo;
        lastX = X3;
        lastY = Y3;
        last_quadrant = end_quadrant;
        minx = min(minx, X);
        minx = min(minx, X2);
        minx = min(minx, X3);
        miny = min(miny, Y);
        miny = min(miny, Y2);
        miny = min(miny, Y3);
        maxx = max(maxx, X);
        maxx = max(maxx, X2);
        maxx = max(maxx, X3);
        maxy = max(maxy, Y);
        maxy = max(maxy, Y2);
        maxy = max(maxy, Y3);
        break;
      }
    }
  }

  if (tmpPath.Size() > 0 && intersects(minx,
                                       miny,
                                       maxx,
                                       maxy,
                                       theX1 - theMargin,
                                       theY1 - theMargin,
                                       theX2 + theMargin,
                                       theY2 + theMargin))
  {
    outPath.Add(tmpPath);
  }

  return outPath;
}
#endif

static bool IsInside(const NFmiArea *const theArea, const NFmiPathElement &theElement)
{
  return theArea->IsInside(NFmiPoint(theElement.x, theElement.y));
}

static void AddElementToCutPath(NFmiPath &thePath,
                                bool prevInside,
                                bool currentInside,
                                const NFmiPathElement &theElem,
                                NFmiPathOperation oper,
                                bool lastOperation)
{
  if (oper == kFmiMoveTo)
  {
    if (prevInside == false && currentInside)  // ulkoa sisälle
      ;                                        // moveto sisältä ulos, ei lisätä uuteen path:iin
    else if (prevInside && currentInside == false)  // sisältä ulos
      thePath.Add(theElem);
    else if (prevInside && currentInside)  // sisällä kokonaan
      thePath.Add(theElem);
    else  // ulkona
    {
    }  // moveto ulkona, ei lisätä uuteen path:iin
  }
  else
  {  // joku viiva tyyppi, oletetaan että lineto (ei oikeastaan väliä)
    if (prevInside == false && currentInside)  // ulkoa sisälle
    {
      if (lastOperation)  // tämä on ainoa poikkeus käsittely, kun kyseessä on viimeisestä viivan
                          // pätkästä
        thePath.Add(theElem);
      else
      {
        NFmiPathElement elem(theElem);
        elem.op = kFmiMoveTo;  // pitää muuttaa moveto-tyypiksi
        thePath.Add(elem);
      }
    }
    else if (prevInside && currentInside == false)  // sisältä ulos
      thePath.Add(theElem);
    else if (prevInside && currentInside)  // sisällä kokonaan
      thePath.Add(theElem);
    else  // ulkona
    {
    }  // moveto ulkona, ei lisätä uuteen path:iin
  }
}

// ----------------------------------------------------------------------
// Create new path that is cut inside the given area. But so that the lines
// at the edges goes out of area (by one point along the polyline).
// HUOM! Tämä on raakile versio ja toimii vain rajojen katkaisijana,
// pitäisi tehdä sellainen versio joka osaisi tehdä alue leikkaukset
// ghost-viivojen kanssa.
// ----------------------------------------------------------------------

NFmiPath NFmiPath::Clip(const NFmiArea *const theArea) const
{
  NFmiPath path;
  if (theArea && itsElements.size() > 1)
  {
    NFmiPathData::const_iterator iter = itsElements.begin();
    NFmiPathData::const_iterator prevIter = iter++;
    bool prevInside = IsInside(theArea, *prevIter);
    bool currentInside = prevInside;
    for (; iter != itsElements.end(); ++iter)
    {
      prevInside = currentInside;  // tämä pitää tehdä loopin alussa, jotta loopin lopuksi on tiedot
                                   // kahden viimeisen pisteen tilasta
      currentInside = IsInside(theArea, *iter);
      AddElementToCutPath(path, prevInside, currentInside, *prevIter, iter->op, false);
      prevIter = iter;
    }
    // lopuksi pitää lisätä tarvittaessa vielä viimeinen piste
    AddElementToCutPath(path,
                        prevInside,
                        currentInside,
                        *prevIter,
                        prevIter->op,
                        true);  // huom! tässä on prevIter->op
  }
  return path;
}

void add_cuts(NFmiContourTree &theTree, std::set<double> &theCuts, double theLon1, double theLon2)
{
  std::set<double>::const_iterator iter = theCuts.begin();
  std::set<double>::const_iterator end = theCuts.end();

  while (iter != end)
  {
    double lat1 = *iter;
    if (++iter != end)
    {
      double lat2 = *iter;
      ++iter;
      theTree.Add(NFmiEdge(theLon1, lat1, theLon1, lat2, true, false));
      theTree.Add(NFmiEdge(theLon2, lat1, theLon2, lat2, true, false));
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Utility subroutine for PacificView
 */
// ----------------------------------------------------------------------

void make_pacific(const NFmiPath &thePath,
                  const NFmiEsriBox &theBox,
                  NFmiPath &theOutPath,
                  NFmiContourTree &theTree,
                  std::set<double> &theCuts,
                  bool theDateLine)
{
  if (thePath.Empty()) return;

  if (theBox.Xmin() >= 0 && !theDateLine)
  {
    theOutPath.Add(thePath);  // already in range 0-360
    return;
  }

  if (theBox.Xmax() < 0 && !theDateLine)
  {
    // Only shift from Atlantic to Pacific
    for (NFmiPathData::const_iterator iter = thePath.Elements().begin(),
                                      end = thePath.Elements().end();
         iter != end;
         ++iter)
    {
      theOutPath.Add(NFmiPathElement(iter->op, iter->x + 360, iter->y));
    }
    return;
  }

  // Now splitting some lines in half may be necessary. We also omit vertical lines
  // at the dateline boundary (-180 or 180) except near the poles, where they are
  // needed to include the poles themselves in the path. We also need to create
  // the lines at 0 and 360 to reconnect the polygons

  double lastX = kFloatMissing;
  double lastY = kFloatMissing;

  for (NFmiPathData::const_iterator iter = thePath.Elements().begin(),
                                    end = thePath.Elements().end();
       iter != end;
       ++iter)
  {
    double X = iter->x;
    double Y = iter->y;
    NFmiPathOperation op = iter->op;

    switch (op)
    {
      case kFmiMoveTo:
      {
        break;
      }
      case kFmiLineTo:
      case kFmiGhostLineTo:
      {
        // Omit vertical lines at the dateline boundary

        if ((lastX == -180 || lastX == 180) && (X == -180 || X == 180) &&
            (lastY > -80 && lastY < 75 && Y > -80 &&
             Y < 75))  // works for the Antarctic and Chukotski Peninsula
          break;

        bool exact = (op == kFmiLineTo);

        if (lastX < 0 && X < 0)
        {
          theTree.Add(NFmiEdge(lastX + 360, lastY, X + 360, Y, exact, true));
        }
        else if (lastX > 0 && X > 0)
        {
          theTree.Add(NFmiEdge(lastX, lastY, X, Y, exact, true));
        }
        else if (lastX == X)
        {
          theTree.Add(NFmiEdge(lastX, lastY, X, Y, exact, true));
        }
        else if (lastX < X)
        {
          // now lastX < 0 and X >= 0
          double s = (0 - lastX) / (X - lastX);
          double ymid = lastY + s * (Y - lastY);
          theTree.Add(NFmiEdge(lastX + 360, lastY, 360, ymid, exact, true));
          theTree.Add(NFmiEdge(0, ymid, X, Y, exact, true));
          theCuts.insert(ymid);
        }
        else
        {
          // now lastX >= 0 and X < 0
          double s = (0 - X) / (lastX - X);
          double ymid = Y + s * (lastY - Y);
          theTree.Add(NFmiEdge(lastX, lastY, 0, ymid, exact, true));
          theTree.Add(NFmiEdge(360, ymid, X + 360, Y, exact, true));
          theCuts.insert(ymid);
        }
        break;
      }
      default:;
    }

    lastX = X;
    lastY = Y;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Utility subroutine for AtlanticView
 */
// ----------------------------------------------------------------------

void make_atlantic(const NFmiPath &thePath,
                   const NFmiEsriBox &theBox,
                   NFmiPath &theOutPath,
                   NFmiContourTree &theTree,
                   std::set<double> &theCuts,
                   bool theDateLine)
{
  if (thePath.Empty()) return;

  // Note: We require Xmin to be >=0 in this easy cases since some paths
  // may look Pacific even though the coordinates are not. Example: Chuchki data

  if (theBox.Xmin() >= 0 && theBox.Xmax() <= 180 && !theDateLine)
  {
    theOutPath.Add(thePath);  // already in range 0...180
    return;
  }

  if (theBox.Xmin() >= 180 && !theDateLine)
  {
    // Only shift from Pacific to Atlantic
    for (NFmiPathData::const_iterator iter = thePath.Elements().begin(),
                                      end = thePath.Elements().end();
         iter != end;
         ++iter)
    {
      theOutPath.Add(NFmiPathElement(iter->op, iter->x - 360, iter->y));
    }
    return;
  }

  // Now splitting some lines in half may be necessary. We also omit vertical lines
  // at the dateline boundary (0 or 360) except near the poles, where they are
  // needed to include the poles themselves in the path.

  double lastX = kFloatMissing;
  double lastY = kFloatMissing;

  for (NFmiPathData::const_iterator iter = thePath.Elements().begin(),
                                    end = thePath.Elements().end();
       iter != end;
       ++iter)
  {
    double X = iter->x;
    double Y = iter->y;
    NFmiPathOperation op = iter->op;

    switch (op)
    {
      case kFmiMoveTo:
      {
        break;
      }
      case kFmiLineTo:
      case kFmiGhostLineTo:
      {
        // Omit vertical lines at the dateline boundary

        if ((lastX == 0 || lastX == 360) && (X == 0 || X == 360) &&
            (lastY > -80 && lastY < 75 && Y > -80 &&
             Y < 75))  // works for the Antarctic and Chukotski Peninsula
          break;

        bool exact = (op == kFmiLineTo);

        if (lastX > 180 && X > 180)
        {
          theTree.Add(NFmiEdge(lastX - 360, lastY, X - 360, Y, exact, true));
        }
        else if (lastX >= 0 && lastX < 180 && X >= 0 && X < 180)
        {
          theTree.Add(NFmiEdge(lastX, lastY, X, Y, exact, true));
        }
        else if (lastX >= -180 && lastX <= 0 && X >= -180 && X <= 0)
        {
          theTree.Add(NFmiEdge(lastX, lastY, X, Y, exact, true));
        }
        else if (lastX == X)
        {
          theTree.Add(NFmiEdge(lastX, lastY, X, Y, exact, true));
        }
        else if (lastX < -90 && X > 90)
        {
          double x = lastX + 360;
          double s = (180 - x) / (X - x);
          double ymid = lastY + s * (Y - lastY);
          theTree.Add(NFmiEdge(lastX, lastY, -180, ymid, exact, true));
          theTree.Add(NFmiEdge(180, ymid, X, Y, exact, true));
          theCuts.insert(ymid);
        }
        else if (lastX > 90 && X < -90)
        {
          double x = X + 360;
          double s = (180 - lastX) / (x - lastX);
          double ymid = lastY + s * (Y - lastY);
          theTree.Add(NFmiEdge(lastX, lastY, 180, ymid, exact, true));
          theTree.Add(NFmiEdge(-180, ymid, X, Y, exact, true));
          theCuts.insert(ymid);
        }
        else if (lastX < X)
        {
          // now lastX < 180 and X >= 180
          double s = (180 - lastX) / (X - lastX);
          double ymid = lastY + s * (Y - lastY);
          theTree.Add(NFmiEdge(lastX, lastY, 180, ymid, exact, true));
          theTree.Add(NFmiEdge(-180, ymid, X - 360, Y, exact, true));
          theCuts.insert(ymid);
        }
        else
        {
          // now lastX >= 180 and X < 180
          double s = (180 - X) / (lastX - X);
          double ymid = Y + s * (lastY - Y);
          theTree.Add(NFmiEdge(lastX - 360, lastY, -180, ymid, exact, true));
          theTree.Add(NFmiEdge(180, ymid, X, Y, exact, true));
          theCuts.insert(ymid);
        }
        break;
      }
      default:;
    }

    lastX = X;
    lastY = Y;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Rebuild the path into a Pacific view if so requested
 */
// ----------------------------------------------------------------------

NFmiPath NFmiPath::PacificView(bool pacific) const
{
  if (!pacific) return *this;
  if (itsElements.empty()) return *this;
  if (IsPacificView()) return *this;

  NFmiPath outpath;
  NFmiPath currentpath;

  NFmiContourTree tree(0, 0);
  NFmiEsriBox box;
  bool dateline = false;

  std::set<double> cuts;

  // Iterate over subsegments, calculating the bounding box simultaneously

  for (NFmiPathData::const_iterator iter = Elements().begin(), end = Elements().end(); iter != end;
       ++iter)
  {
    if (iter->op == kFmiMoveTo)
    {
      make_pacific(currentpath, box, outpath, tree, cuts, dateline);
      currentpath.Clear();
      box = NFmiEsriBox();
      dateline = false;
    }

    currentpath.Add(*iter);
    box.Update(iter->x, iter->y);

    if (iter->x == -180 || iter->x == 180) dateline = true;
  }

  make_pacific(currentpath, box, outpath, tree, cuts, dateline);

  add_cuts(tree, cuts, 0, 360);

  outpath.Add(tree.Path());
  return outpath;
}

// ----------------------------------------------------------------------
/*!
 * \brief Rebuild the path into a Atlantic view if so requested
 */
// ----------------------------------------------------------------------

NFmiPath NFmiPath::AtlanticView(bool atlantic) const
{
  if (!atlantic) return *this;
  if (itsElements.empty()) return *this;
  if (!IsPacificView()) return *this;

  NFmiPath outpath;
  NFmiPath currentpath;

  NFmiContourTree tree(0, 0);
  NFmiEsriBox box;
  bool dateline = false;

  std::set<double> cuts;

  // Iterate over subsegments, calculating the bounding box simultaneously

  for (NFmiPathData::const_iterator iter = Elements().begin(), end = Elements().end(); iter != end;
       ++iter)
  {
    if (iter->op == kFmiMoveTo)
    {
      make_atlantic(currentpath, box, outpath, tree, cuts, dateline);
      currentpath.Clear();
      box = NFmiEsriBox();
      dateline = false;
    }

    currentpath.Add(*iter);
    box.Update(iter->x, iter->y);

    if (iter->x == 0 || iter->x == 360) dateline = true;
  }

  make_atlantic(currentpath, box, outpath, tree, cuts, dateline);

  add_cuts(tree, cuts, -180, 180);

  outpath.Add(tree.Path());
  return outpath;
}

// ----------------------------------------------------------------------
// Test whether any path segment looks like the data comes from a Pacific view
// ----------------------------------------------------------------------

bool NFmiPath::IsPacificView() const
{
  // cntry06 data has longitudes such as 180.00000033527612686
  const double eps = 0.001;

  if (Empty()) return false;

  double lastX = kFloatMissing;

  NFmiPathData::const_iterator iter;

  for (iter = itsElements.begin(); iter != itsElements.end(); ++iter)
  {
    switch (iter->op)
    {
      case kFmiLineTo:
      case kFmiGhostLineTo:
      {
        // Ignore long lines at the poles
        if (iter->y != -90 && iter->y != 90)
        {
          // if line is all together over 180 + eps, its pacific
          if (lastX > 180 + eps && iter->x > 180 + eps) return true;
          if (lastX < 180 && iter->x > 180 + eps) return true;
          if (lastX > 180 + eps && iter->x < 180) return true;
          // Or looks like it should be made into a Pacific view
          // when there are lines longer than half the world
          if (lastX < -90 && iter->x > 90) return true;
          if (lastX > 90 && iter->x < -90) return true;

          break;
        }
      }
      default:
        break;
    }
    lastX = iter->x;
  }

  return false;
}

}  // namespace Imagine

// ======================================================================
