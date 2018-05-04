// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiGshhsTools
 */
// ======================================================================

#include "NFmiGshhsTools.h"
#include "NFmiPath.h"
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiSettings.h>
#include <stdexcept>

using namespace std;

// ======================================================================
//				IMPLEMENTATION HIDING DETAILS
// ======================================================================

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Byteswapping subroutine
 *
 * \param The number to byteswap
 */
// ----------------------------------------------------------------------

inline void swab2(short *theValue)
{
  char *ch = reinterpret_cast<char *>(theValue);
  std::swap(ch[0], ch[1]);
}

// ----------------------------------------------------------------------
/*!
 * \brief Byteswapping subroutine
 *
 * \param The number to byteswap
 */
// ----------------------------------------------------------------------

inline void swab4(int *theValue)
{
  char *ch = reinterpret_cast<char *>(theValue);
  std::swap(ch[0], ch[3]);
  std::swap(ch[1], ch[2]);
}

// ----------------------------------------------------------------------
/*!
 * \brief GSHHS header structure
 */
// ----------------------------------------------------------------------

struct GSHHS
{
  int id;               //!< unique polygon id number, starting at 0
  int n;                //!< number of points in this polygon
  int level;            //!< 1 land, 2 lake, 3 island in lake, 4 pond in..
  int west;             //!< min longitude in micro-degrees
  int east;             //!< max longitude in micro-degrees
  int south;            //!< min latitude in micro-degrees
  int north;            //!< max latitude in micro-degrees
  int area;             //!< area of polygon in 1/10 km^2
  short int greenwich;  //!< 1 if Greenwich is crossed
  short int source;     //!< 0 = CIA WDBII, 1 = WVS

};  // struct GSHHS

// ----------------------------------------------------------------------
/*!
 * \brief GSHHS coordinate structure
 */
// ----------------------------------------------------------------------

struct POINT
{
  int x;  //!< x-coordinate
  int y;  //!< y-coordinate
};

}  // namespace anonymous

// ======================================================================
//				ACTUAL INTERFACE IMPLEMENTATION
// ======================================================================

namespace Imagine
{
namespace NFmiGshhsTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Read the path within the given bounding box
 *
 * Throws if the file is unreadable or corrupt.
 *
 * \param theFilename The name of the file containing the GSHHS data
 * \param theMinLongitude The minimum longitude
 * \param theMinLatitude The minimum latitude
 * \param theMaxLongitude The maximum longitude
 * \param theMaxLatitude The maximum latitude
 * \param theMinArea The minimum area, negative implies no minimum
 * \return The path
 */
// ----------------------------------------------------------------------

const NFmiPath ReadPath(const std::string &theFilename,
                        double theMinLongitude,
                        double theMinLatitude,
                        double theMaxLongitude,
                        double theMaxLatitude,
                        double theMinArea)
{
  // The path to be returned

  NFmiPath ret;

  // Handle invalid bounding box by returning empty path

  if (theMinLongitude >= theMaxLongitude || theMinLatitude >= theMaxLatitude) return ret;

  // Open file for reading

  const string gshhs_path = NFmiSettings::Require<string>("imagine::gshhs_path");

  const string filename = NFmiFileSystem::FileComplete(theFilename, gshhs_path);

  FILE *fp = fopen(filename.c_str(), "rb");

  // Throw if failed to open

  if (fp == NULL) throw std::runtime_error("Failed to open " + theFilename + " for reading");

  // Read the first header
  GSHHS header;
  POINT point;
  int n_read = fread(static_cast<void *>(&header), size_t(sizeof(struct GSHHS)), size_t(1), fp);

  // Establish whether byteswapping is needed from first header

  const bool flip = !(header.level > 0 && header.level < 5);

  int max = 270000000;  // initial value for polygon offsetting

  while (n_read == 1)
  {
    if (flip)
    {
      swab4(&header.id);
      swab4(&header.n);
      swab4(&header.level);
      swab4(&header.west);
      swab4(&header.east);
      swab4(&header.south);
      swab4(&header.north);
      swab4(&header.area);
      swab2(&header.greenwich);
      swab2(&header.source);
    }

    const double w = 1.0e-6 * header.west;
    const double e = 1.0e-6 * header.east;
    const double s = 1.0e-6 * header.south;
    const double n = 1.0e-6 * header.north;

    const double area = 0.1 * header.area;  // now in km^2

    const double ww = (w < -180 ? w + 360 : w > 180 ? w - 360 : w);
    const double ee = (e < -180 ? e + 360 : e > 180 ? e - 360 : e);

    bool outside =
        (ww > theMaxLongitude || ee < theMinLongitude || s > theMaxLatitude || n < theMinLatitude);

    // Correct test for Eurasia
    if (ee < ww)
    {
      bool outside1 = (ww > theMaxLongitude || s > theMaxLatitude || n < theMinLatitude);

      bool outside2 = (ee < theMinLongitude || s > theMaxLatitude || n < theMinLatitude);

      outside = (outside1 && outside2);
    }

    if (outside || (area < theMinArea && theMinArea >= 0))
    {
      const size_t skipsize = header.n * sizeof(struct POINT);
      fseek(fp, skipsize, SEEK_CUR);
    }
    else
    {
      for (int k = 0; k < header.n; k++)
      {
        if (fread(static_cast<void *>(&point), size_t(sizeof(struct POINT)), size_t(1), fp) != 1)
          throw std::runtime_error("File " + theFilename + " is corrupt");
        if (flip)
        {
          swab4(&point.x);
          swab4(&point.y);
        }
        const double lon =
            (header.greenwich && point.x > max ? point.x * 1.0e-6 - 360 : point.x * 1.0e-6);
        const double lat = point.y * 1.0e-6;

        if (k == 0)
          ret.MoveTo(lon, lat);
        else
          ret.LineTo(lon, lat);
      }
    }

    max = 180000000;  // only Eurasiafrica needs 270

    n_read = fread(static_cast<void *>(&header), size_t(sizeof(struct GSHHS)), size_t(1), fp);
  }

  fclose(fp);

  ret.Clip(theMinLongitude, theMinLatitude, theMaxLongitude, theMaxLatitude);
  return ret;
}

}  // namespace NFmiGshhsTools

}  // namespace Imagine

// ======================================================================
