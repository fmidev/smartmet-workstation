// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::CoastMaskSource
 */
// ======================================================================
/*!
 * \class TextGen::CoastMaskSource
 *
 * \brief Provides mask services to clients (masked to coastal areas)
 *
 * The CoastMaskSource class provides access to masks calculated from
 * named SVG paths which represent geographic areas. This class is
 * differentiated from RegularMaskSource by the fact that any mask
 * is restricted to remain close to the coast. The acceptable coast
 * area is specified by giving the name of the coastal line
 * and the acceptable distance in kilometers from the coastal line.
 *
 */
// ======================================================================

#include "CoastMaskSource.h"

#include <calculator/TextGenError.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherSource.h>

#include <newbase/NFmiGrid.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiQueryData.h>
#include <newbase/NFmiIndexMask.h>
#include <newbase/NFmiIndexMaskSource.h>
#include <newbase/NFmiIndexMaskTools.h>

#include <map>

using namespace std;
using namespace boost;
using namespace NFmiIndexMaskTools;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Utility structure to each cache management
 */
// ----------------------------------------------------------------------

struct WeatherAreaAndID
{
  WeatherId itsID;
  WeatherArea itsArea;

  WeatherAreaAndID(const WeatherId& theID, const WeatherArea& theArea)
      : itsID(theID), itsArea(theArea)
  {
  }

  bool operator==(const WeatherAreaAndID& theOther) const
  {
    return (itsID == theOther.itsID && itsArea == theOther.itsArea);
  }

  bool operator<(const WeatherAreaAndID& theOther) const
  {
    return (itsID != theOther.itsID ? itsID < theOther.itsID : itsArea < theOther.itsArea);
  }
};

// ----------------------------------------------------------------------
/*!
 * \brief Implementation hiding detail for TextGen::CoastMaskSource
 */
// ----------------------------------------------------------------------

class CoastMaskSource::Pimple
{
 public:
  Pimple(const WeatherArea& theCoast);

  typedef map<WeatherAreaAndID, mask_type> mask_storage;
  typedef map<WeatherAreaAndID, masks_type> masks_storage;

  const WeatherArea itsCoast;

  mutable mask_storage itsMaskStorage;
  mutable masks_storage itsMasksStorage;

  mask_type find(const WeatherId& theID, const WeatherArea& theArea) const;

  void insert(const WeatherId& theID, const WeatherArea& theArea, const mask_type& theMask) const;

  mask_type create_mask(const WeatherArea& theArea,
                        const std::string& theData,
                        const WeatherSource& theWeatherSource) const;

};  // class CoastMaskSource::Pimple

// ----------------------------------------------------------------------
/*!
 * \brief Pimple constructor
 */
// ----------------------------------------------------------------------

CoastMaskSource::Pimple::Pimple(const WeatherArea& theCoast)
    : itsCoast(theCoast), itsMaskStorage(), itsMasksStorage()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Find mask from cache
 *
 * Returns a 0-shared pointer if mask is not found
 *
 * \param theID The weather ID
 * \param theArea The weather area
 * \return shared_ptr to mask or 0
 */
// ----------------------------------------------------------------------

CoastMaskSource::mask_type CoastMaskSource::Pimple::find(const WeatherId& theID,
                                                         const WeatherArea& theArea) const
{
  static boost::shared_ptr<NFmiIndexMask> dummy;

  WeatherAreaAndID key(theID, theArea);

  mask_storage::const_iterator it = itsMaskStorage.find(key);
  if (it == itsMaskStorage.end()) return dummy;

  return it->second;
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert a new mask into the cache
 *
 * \param theID The data ID
 * \param theArea The weather area
 * \param theMask The mask itself
 */
// ----------------------------------------------------------------------

void CoastMaskSource::Pimple::insert(const WeatherId& theID,
                                     const WeatherArea& theArea,
                                     const mask_type& theMask) const
{
  typedef mask_storage::value_type value_type;

  WeatherAreaAndID key(theID, theArea);

  itsMaskStorage.insert(value_type(key, theMask));

  if (itsMaskStorage.insert(value_type(key, theMask)).second)
    throw TextGenError("Could not cache mask for " + theArea.name());
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a new weather area
 *
 * \param theArea The area
 * \param theData The data name
 * \param theWeatherSource The weather source
 * \return The mask
 */
// ----------------------------------------------------------------------

CoastMaskSource::mask_type CoastMaskSource::Pimple::create_mask(
    const WeatherArea& theArea,
    const std::string& theData,
    const WeatherSource& theWeatherSource) const
{
  // Establish the grid which to mask

  boost::shared_ptr<NFmiQueryData> qdata = theWeatherSource.data(theData);
  NFmiFastQueryInfo qi = NFmiFastQueryInfo(qdata.get());
  if (!qi.IsGrid())
    throw TextGenError("The data in " + theData + " is not gridded - cannot generate mask for it");

  // First build the area mask

  const NFmiSvgPath svg = theArea.path();
  const float radius = theArea.radius();
  mask_type areamask(new NFmiIndexMask(MaskExpand(*(qi.Grid()), svg, radius)));

  // Then build the coast mask

  WeatherId id = theWeatherSource.id(theData);
  mask_type coastmask = find(id, itsCoast);
  if (coastmask.get() == 0)
  {
    const NFmiSvgPath& csvg = itsCoast.path();
    const float cdistance = itsCoast.radius();
    coastmask.reset(new NFmiIndexMask(MaskDistance(*(qi.Grid()), csvg, cdistance)));
    insert(id, itsCoast, coastmask);
  }

  // The intersection is the coastal area

  *areamask &= *coastmask;

  return areamask;
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

CoastMaskSource::CoastMaskSource(const WeatherArea& theCoast) : itsPimple(new Pimple(theCoast)) {}
// ----------------------------------------------------------------------
/*!
 * \brief Return the mask for the given area
 *
 * \param theArea The weather area
 * \param theData The data name
 * \param theWeatherSource The source for weather data
 */
// ----------------------------------------------------------------------

CoastMaskSource::mask_type CoastMaskSource::mask(const WeatherArea& theArea,
                                                 const std::string& theData,
                                                 const WeatherSource& theWeatherSource) const
{
  if (theArea.isPoint()) throw TextGenError("Trying to generate mask for point");

  // Establish the ID for the data

  WeatherId id = theWeatherSource.id(theData);

  // Try to find cached mask first

  mask_type areamask = itsPimple->find(id, theArea);

  if (areamask.get() != 0) return areamask;

  // Calculate new mask and cache it

  areamask = itsPimple->create_mask(theArea, theData, theWeatherSource);
  itsPimple->insert(id, theArea, areamask);

  return areamask;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the mask source for the given area
 *
 * \param theArea The weather area
 * \param theData The data name
 * \param theWeatherSource The source for weather data
 */
// ----------------------------------------------------------------------

CoastMaskSource::masks_type CoastMaskSource::masks(const WeatherArea& theArea,
                                                   const std::string& theData,
                                                   const WeatherSource& theWeatherSource) const
{
  throw TextGenError("CoastMaskSource::masks not implemented");
}

}  // namespace TextGen

// ======================================================================
