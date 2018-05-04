// ======================================================================
/*!
 * \brief Implementation of class NFmiDataHints
 */
// ======================================================================

#include "NFmiDataHints.h"
#include <newbase/NFmiGlobals.h>

using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
/*!
 * \brief Recursive grid information
 */
// ----------------------------------------------------------------------

struct RecursiveInfo
{
  NFmiDataHints::Rectangle itsRectangle;
  boost::shared_ptr<RecursiveInfo> itsLeft;
  boost::shared_ptr<RecursiveInfo> itsRight;
};

// ----------------------------------------------------------------------
/*!
 * \brief Pimple
 */
// ----------------------------------------------------------------------

class NFmiDataHints::Pimple
{
 public:
  Pimple(const NFmiDataMatrix<float>& theData, int theMaxSize);
  return_type rectangles(float theLoLimit, float theHiLimit) const;

 private:
  typedef boost::shared_ptr<RecursiveInfo> node_type;

  void recurse(node_type& theInfo,
               const NFmiDataMatrix<float>& theData,
               int x1,
               int y1,
               int x2,
               int y2,
               int theMaxSize);

  bool find(return_type& theRectangles,
            const node_type& theInfo,
            float theLoLimit,
            float theHiLimit) const;

  node_type itsRoot;

  Pimple();
  Pimple(const Pimple& thePimple);
  Pimple& operator=(const Pimple& thePimple);
};

// ----------------------------------------------------------------------
/*!
 * \brief Construct the pimple
 */
// ----------------------------------------------------------------------

NFmiDataHints::Pimple::Pimple(const NFmiDataMatrix<float>& theData, int theMaxSize)
    : itsRoot(new RecursiveInfo())
{
  if (theMaxSize < 4) throw runtime_error("Too small maxsize in NFmiDataHints constructor");

  recurse(itsRoot, theData, 0, 0, theData.NX() - 1, theData.NY() - 1, theMaxSize);
}

// ----------------------------------------------------------------------
/*!
 * \brief Feed matrix information recursively
 */
// ----------------------------------------------------------------------

void NFmiDataHints::Pimple::recurse(node_type& theNode,
                                    const NFmiDataMatrix<float>& theData,
                                    int x1,
                                    int y1,
                                    int x2,
                                    int y2,
                                    int theMaxSize)
{
  theNode->itsRectangle.x1 = x1;
  theNode->itsRectangle.y1 = y1;
  theNode->itsRectangle.x2 = x2;
  theNode->itsRectangle.y2 = y2;

  int width = x2 - x1;
  int height = y2 - y1;

  if ((width <= theMaxSize && height <= theMaxSize) || (width <= 1 || height <= 1))
  {
    // The rectangle is small enough now, find the extrema from it
    float minimum = kFloatMissing;
    float maximum = kFloatMissing;

    theNode->itsRectangle.hasmissing = false;
    for (int j = y1; j <= y2; j++)
      for (int i = x1; i <= x2; i++)
      {
        float value = theData[i][j];
        if (value == kFloatMissing)
          theNode->itsRectangle.hasmissing = true;
        else
        {
          if (minimum == kFloatMissing)
          {
            minimum = value;
            maximum = value;
          }
          else
          {
            minimum = min(value, minimum);
            maximum = max(value, maximum);
          }
        }
      }

    theNode->itsRectangle.minimum = minimum;
    theNode->itsRectangle.maximum = maximum;
  }
  else
  {
    theNode->itsLeft.reset(new RecursiveInfo());
    theNode->itsRight.reset(new RecursiveInfo());

    // Recurse the longer edge first
    if (width > height)
    {
      int x = (x1 + x2) / 2;
      recurse(theNode->itsLeft, theData, x1, y1, x, y2, theMaxSize);
      recurse(theNode->itsRight, theData, x, y1, x2, y2, theMaxSize);
    }
    else
    {
      int y = (y1 + y2) / 2;
      recurse(theNode->itsLeft, theData, x1, y1, x2, y, theMaxSize);
      recurse(theNode->itsRight, theData, x1, y, x2, y2, theMaxSize);
    }

    theNode->itsRectangle.hasmissing =
        (theNode->itsLeft->itsRectangle.hasmissing | theNode->itsRight->itsRectangle.hasmissing);

    float min1 = theNode->itsLeft->itsRectangle.minimum;
    float max1 = theNode->itsLeft->itsRectangle.maximum;

    float min2 = theNode->itsRight->itsRectangle.minimum;
    float max2 = theNode->itsRight->itsRectangle.maximum;

    if (min1 == kFloatMissing)
    {
      theNode->itsRectangle.minimum = min2;
      theNode->itsRectangle.maximum = max2;
    }
    else if (min2 == kFloatMissing)
    {
      theNode->itsRectangle.minimum = min1;
      theNode->itsRectangle.maximum = max1;
    }
    else
    {
      theNode->itsRectangle.minimum = min(min1, min2);
      theNode->itsRectangle.maximum = max(max1, max2);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the rectangles
 *
 * \param theLoLimit The lower limit
 * \param theHiLimit The upper limit
 * \return List of rectangles
 */
// ----------------------------------------------------------------------

NFmiDataHints::return_type NFmiDataHints::Pimple::rectangles(float theLoLimit,
                                                             float theHiLimit) const
{
  return_type ret;
  if (find(ret, itsRoot, theLoLimit, theHiLimit)) ret.push_back(itsRoot->itsRectangle);
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return true if rectangle intersects searched range
 */
// ----------------------------------------------------------------------

bool rectangle_intersects(const NFmiDataHints::Rectangle& theRectangle,
                          float theLoLimit,
                          float theHiLimit)
{
  const float& nodemin = theRectangle.minimum;
  const float& nodemax = theRectangle.maximum;
  const bool nodemissing = (nodemin == kFloatMissing);  // no valid values?

  if (theLoLimit != kFloatMissing)
  {
    if (theHiLimit != kFloatMissing)  // searched range: x..y
    {
      if (nodemissing) return false;
      if (max(theLoLimit, nodemin) <= min(theHiLimit, nodemax)) return true;
      return false;
    }
    else  // searched range: x..inf
    {
      if (nodemissing) return false;
      if (nodemax >= theLoLimit) return true;
      return false;
    }
  }
  else
  {
    if (theHiLimit != kFloatMissing)  // searched range: -inf..y
    {
      if (nodemissing) return false;
      if (nodemin <= theHiLimit) return true;
      return false;
    }
    else  // searched range: -inf..inf
    {
      if (!nodemissing) return true;
      return false;
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Find intersecting set of rectangles
 */
// ----------------------------------------------------------------------

bool NFmiDataHints::Pimple::find(return_type& theValues,
                                 const node_type& theNode,
                                 float theLoLimit,
                                 float theHiLimit) const
{
  bool haschildren = (theNode->itsLeft.get() != 0 && theNode->itsRight.get() != 0);

  // Quick exit if the rectangle does not intersect at all

  bool ok = rectangle_intersects(theNode->itsRectangle, theLoLimit, theHiLimit);

  if (!ok) return false;

  if (!haschildren)
  {
    return true;
  }
  else
  {
    bool leftok = find(theValues, theNode->itsLeft, theLoLimit, theHiLimit);
    bool rightok = find(theValues, theNode->itsRight, theLoLimit, theHiLimit);
    if (leftok && rightok) return true;
    if (leftok) theValues.push_back(theNode->itsLeft->itsRectangle);
    if (rightok) theValues.push_back(theNode->itsRight->itsRectangle);
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiDataHints::~NFmiDataHints() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

NFmiDataHints::NFmiDataHints(const NFmiDataMatrix<float>& theData, int theMaxSize)
    : itsPimple(new Pimple(theData, theMaxSize))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return list of rectangles covering the desired value range
 *
 * Lower limit kFloatMissing implies -infinity.
 * Lower limit kFloatMissing implies +infinity.
 * If both limits are missing, any valid value is accepted.
 *
 * \param theLoLimit The lower limit
 * \param theHiLimit The upper limit
 * \return List of rectangles
 */
// ----------------------------------------------------------------------

NFmiDataHints::return_type NFmiDataHints::rectangles(float theLoLimit, float theHiLimit) const
{
  // Call the actual implemenetation
  return itsPimple->rectangles(theLoLimit, theHiLimit);
}

}  // namespace Imagine

// ======================================================================
