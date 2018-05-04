// ======================================================================
//
// See documentation in NFmiEdge.cpp
//
// History:
//
// 12.08.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once

#include <newbase/NFmiGlobals.h>

namespace Imagine
{
// ----------------------------------------------------------------------
// A working class, holding an unique edge
// ----------------------------------------------------------------------

class _FMI_DLL NFmiEdge
{
 public:
  // The constructor will sort the vertices

  NFmiEdge(float theX1, float theY1, float theX2, float theY2, bool exact, bool fixed)
      : itsX1(theX1), itsY1(theY1), itsX2(theX2), itsY2(theY2), fExact(exact), fFixed(fixed)
  {
    if (itsX2 < itsX1 || (itsX2 == itsX1 && itsY2 < itsY1))
    {
      itsX1 = theX2;
      itsX2 = theX1;
      itsY1 = theY2;
      itsY2 = theY1;
    }
  };

  // Getting data values

  float GetX1() const { return itsX1; }
  float GetY1() const { return itsY1; }
  float GetX2() const { return itsX2; }
  float GetY2() const { return itsY2; }
  bool Exact() const { return fExact; }
  bool Fixed() const { return fFixed; }
  // Setting data values

  void SetX1(float theX) { itsX1 = theX; }
  void SetY1(float theY) { itsY1 = theY; }
  void SetX2(float theX) { itsX2 = theX; }
  void SetY2(float theY) { itsY2 = theY; }
  // Equality comparison for our own purposes and for set find methods.

  bool operator==(const NFmiEdge &theEdge) const
  {
    return (itsX1 == theEdge.itsX1 && itsY1 == theEdge.itsY1 && itsX2 == theEdge.itsX2 &&
            itsY2 == theEdge.itsY2 && fExact == theEdge.fExact && fFixed == theEdge.fFixed);
  }

  // Less-than comparison needed for set<NFmiEdge>

  bool operator<(const NFmiEdge &theEdge) const
  {
    if (itsX1 != theEdge.itsX1) return (itsX1 < theEdge.itsX1);
    if (itsY1 != theEdge.itsY1) return (itsY1 < theEdge.itsY1);
    if (itsX2 != theEdge.itsX2) return (itsX2 < theEdge.itsX2);
    return (itsY2 < theEdge.itsY2);
  }

 private:
  // Protect from misuse

  NFmiEdge();

  // Data elements

  float itsX1;  // Start point coordinates
  float itsY1;
  float itsX2;  // End point coordinates
  float itsY2;
  bool fExact;  // Is the edge exactly on a contour?
  bool fFixed;  // False if edge may be deleted when it is a duplicate
};

}  // namespace Imagine

// ----------------------------------------------------------------------

