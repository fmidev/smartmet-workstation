// ======================================================================
/*!
 * \file NFmiSaveBaseFactory.cpp
 * \brief Implementation of a factory for newbase objects
 */
// ======================================================================

#include "NFmiSaveBaseFactory.h"
#include "NFmiEquidistArea.h"
#include "NFmiGdalArea.h"
#include "NFmiGnomonicArea.h"
#include "NFmiGrid.h"
#include "NFmiLambertEqualArea.h"
#include "NFmiLatLonArea.h"
#include "NFmiMercatorArea.h"
#include "NFmiQueryData.h"
#include "NFmiRotatedLatLonArea.h"
#include "NFmiStationBag.h"
#include "NFmiStereographicArea.h"
#include "NFmiYKJArea.h"

#include "NFmiVersion.h"

#include <boost/lexical_cast.hpp>

// Having 'extern' and initializatin is no good (gcc gives a warning).
//  -AKa 3-Jun-10
//
// Note that these are GLOBALLY WRITTEN TO in 'NFmiMetBox.cpp' (looks very suspicious...)
//  -AKa 3-Jun-10
//
unsigned short FmiBoxVersion = 3;
unsigned short FmiInfoVersion = 7;

// ----------------------------------------------------------------------
/*!
 * Returns a newbase object based on the given unique numeric ID.
 * The returned object is a void pointer, which makes this a rather
 * strange factory, but it will have to do until a complete redesign.
 *
 * \param classId The ID of the object to be created
 * \return Pointer to the new object
 * \todo Should return an boost::shared_ptr
 */
// ----------------------------------------------------------------------

void *CreateSaveBase(unsigned int classId)
{
  switch (classId)
  {
    case kNFmiGrid:
      return static_cast<void *>(new NFmiGrid);

    case kNFmiLambertEqualArea:
      return static_cast<void *>(new NFmiLambertEqualArea);
    case kNFmiLatLonArea:
      return static_cast<void *>(new NFmiLatLonArea);
    case kNFmiRotatedLatLonArea:
      return static_cast<void *>(new NFmiRotatedLatLonArea);
    case kNFmiStereographicArea:
      return static_cast<void *>(new NFmiStereographicArea);
    case kNFmiYKJArea:
      return static_cast<void *>(new NFmiYKJArea);
    case kNFmiEquiDistArea:
      return static_cast<void *>(new NFmiEquidistArea);
    case kNFmiMercatorArea:
      return static_cast<void *>(new NFmiMercatorArea);
    case kNFmiGnomonicArea:
      return static_cast<void *>(new NFmiGnomonicArea);

    case kNFmiQueryData:
      return static_cast<void *>(new NFmiQueryData);
    case kNFmiQueryInfo:
      return static_cast<void *>(new NFmiQueryInfo);

    case kNFmiLocationBag:
      return static_cast<void *>(new NFmiLocationBag);
    case kNFmiStationBag:
      return static_cast<void *>(new NFmiStationBag);
#ifdef UNIX
    case kNFmiGdalArea:
      return static_cast<void *>(new NFmiGdalArea);
#endif

    default:
      throw std::runtime_error("Newbase: unable to create unknown class " +
                               boost::lexical_cast<std::string>(classId));
  }
}

// ======================================================================
