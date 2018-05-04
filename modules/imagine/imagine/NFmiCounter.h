// ======================================================================
/*!
 * \file NFmiCounter.h
 * \brief Description of a class to count unique objects.
 *
 * \b History:
 *
 * \li 07.11.2001 Mika Heiskanen\par
 * Implemented
 *
 */
// ======================================================================
/*!
 * \class NFmiCounter
 *
 * A counter is used to count the number of times each unique object
 * occurs. It can be used for example to identify all unique NFmiPoints,
 * and to get their occurrance counts. This can in turn be used for example
 * when simplifying geographic polygons, such as national borders, where
 * identifying important points (polygon intersections) is important.
 *
 * A basic requirement is that the object must support equality
 * and less-than operators.
 *
 * Sample usage:
 *
 * \code
 * #include "NFmiPoint.h"
 * NFmiCounter<NFmiPoint> counter;
 * int newcount = counter.Add(point1);
 * int count = counter.Count(point2);
 * \endcode
 *
 * Instead of depending on newbase, one may for example also use
 *
 * \code
 * NFmiCounter<pair<float,float> > counter;
 * int newcount = counter.Add(make_pair(x,y));
 * int count = counter.Count(make_pair(x,y));
 * \endcode
 *
 */
// ======================================================================

#pragma once

#include <newbase/NFmiDef.h>
#include <map>

namespace Imagine
{
//! An unique object counter.
template <class T>
class _FMI_DLL NFmiCounter
{
 protected:
  //! The counted data is held in this type containers.
  typedef std::map<T, unsigned long> NFmiCounterData;

  //! The data counted so far, along with the counts.
  NFmiCounterData itsData;

 public:
  typedef typename NFmiCounterData::const_iterator const_iterator;

  //! Constructor
  NFmiCounter(void) {}
  //! Destructor
  ~NFmiCounter(void) {}
  //! Reset the counter to contain no data
  void Clear(void) { itsData.clear(); }
  //! Add a new object to the counter.
  /*!
   * This adds the object to a map with the object as a key.
   * If the object already exists in the map, its counter is increased.
   * The count of the object after insertion is returned, hence it
   * is always atleast one.
   */

  long Add(T theElement)
  {
#ifndef UNIX
    std::pair<NFmiCounterData::iterator, bool> result =
        itsData.insert(std::make_pair(theElement, 1));
#else
    std::pair<typename NFmiCounterData::iterator, bool> result =
        itsData.insert(typename NFmiCounterData::value_type(theElement, 1));
#endif
    if (result.second)
      return 1;
    else
      return ++(result.first->second);
  }

  //! Query the count of an object.
  /*!
   * The count of the object is the number of times Add has been used
   * to add the object into the counter. The count is zero if the
   * object has not been added into the counter.
   */

  long Count(T theElement) const
  {
    typename NFmiCounterData::const_iterator iter = itsData.find(theElement);
    if (iter == itsData.end())
      return 0L;
    else
      return iter->second;
  }

  const_iterator begin() const { return itsData.begin(); }
  const_iterator end() const { return itsData.end(); }
};

}  // namespace Imagine


// ======================================================================
