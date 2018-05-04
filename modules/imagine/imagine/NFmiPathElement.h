// ======================================================================
//
// Definition of a PostScript style path element, with a ghostline extension.
//
// There is no respective NFmiPathElement.cpp file, all tasks needed so far
// are inlined here.
//
// History:
//
// 16.07.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once


namespace Imagine
{
// ----------------------------------------------------------------------
// Path operations
// ----------------------------------------------------------------------

enum NFmiPathOperation
{
  kFmiMoveTo,
  kFmiLineTo,
  kFmiGhostLineTo,
  kFmiConicTo,
  kFmiCubicTo
};

/* The struct is basically the same, a _bit_ shorter... then old (below)
*/
struct NFmiPathElement
{
  enum NFmiPathOperation op;
  double x, y;

  NFmiPathElement(enum NFmiPathOperation op_, double x_, double y_) : op(op_), x(x_), y(y_) {}
  bool operator==(const NFmiPathElement &other) const
  {
    return op == other.op && x == other.x && y == other.y;
  }

/* Verbose alternatives for old code's compatibility
*/
#if 1
  enum NFmiPathOperation Oper() const { return op; }
  double X() const { return x; }
  double Y() const { return y; }
  void Oper(enum NFmiPathOperation v) { op = v; }
  void X(double v) { x = v; }
  void Y(double v) { y = v; }
#endif
};

}  // namespace Imagine


// ======================================================================
