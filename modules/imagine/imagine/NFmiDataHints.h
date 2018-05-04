// ======================================================================
/*!
 * \brief Interface of class NFmiDataMatrixInfo
 */
// ======================================================================

#pragma once

#include <newbase/NFmiDataMatrix.h>

#include <boost/shared_ptr.hpp>
#include <list>

namespace Imagine
{
// ----------------------------------------------------------------------
/*!
 * \brief Interface of class NFmiDataHints
 */
// ----------------------------------------------------------------------

class NFmiDataHints
{
 public:
  struct Rectangle
  {
    int x1;
    int y1;
    int x2;
    int y2;
    float minimum;
    float maximum;
    bool hasmissing;
  };

 public:
  typedef std::list<Rectangle> return_type;

  ~NFmiDataHints();

  // Max subgrid size is 10x10
  NFmiDataHints(const NFmiDataMatrix<float>& theData, int theMaxSize = 10);

  return_type rectangles(float theLoLimit, float theHiLimit) const;

 private:
  class Pimple;
  boost::shared_ptr<Pimple> itsPimple;

  NFmiDataHints();
  NFmiDataHints& operator=(const NFmiDataHints& theMatrix);
  NFmiDataHints(const NFmiDataHints& theMatrix);
};

}  // namespace


// ======================================================================
