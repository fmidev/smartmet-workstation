// ======================================================================
//
// Generic map vector data (point,polyline,polygon) container and
// renderer.
//
// History:
//
// 25.09.2001 Mika Heiskanen
//
//	Implemented ESRI mapdata renderer based on libesri
//
// ======================================================================

#pragma once


#include "NFmiEsriShape.h"
#include "NFmiEsriTools.h"
#include "NFmiPath.h"

#ifdef IMAGINE_WITH_CAIRO
#include "ImagineXr.h"
#else
#include "NFmiColorTools.h"
#include "NFmiImage.h"
#include "NFmiFillMap.h"
#include "NFmiDrawable.h"
#endif

#include <newbase/NFmiArea.h>

#include <stdexcept>

namespace Imagine
{
// Implemented shape types

enum NFmiGeoShapeType
{
  kFmiGeoShapeEsri,       // ESRI shapefiles (.shp,.shx,.dbf)
  kFmiGeoShapeShoreLine,  // GMT shoreline databases
  kFmiGeoShapeGMT         // GMT type ASCII databases
};

//! Generic shape error
struct NFmiGeoShapeError : public std::runtime_error
{
  NFmiGeoShapeError(const std::string &s) : std::runtime_error(s) {}
};

class NFmiGeoShape
#ifndef IMAGINE_WITH_CAIRO
    : public NFmiDrawable
#endif
{
 public:
  // Constructor
  NFmiGeoShape() : itsType(kFmiGeoShapeEsri), itsEsriShape(0) {}
  NFmiGeoShape(const std::string &theFilename,
               NFmiGeoShapeType theType = kFmiGeoShapeEsri,
               const std::string &theFilter = "")
      : itsType(theType), itsEsriShape(0)
  {
    Read(theFilename, theType, theFilter);
  }

  // Destructor

  virtual ~NFmiGeoShape() { delete itsEsriShape; }
  // Data-access

  NFmiGeoShapeType Type() const { return itsType; }
  // Project the data

  void ProjectXY(const NFmiArea &theArea);

  // Create a path from the map data

  const NFmiPath Path() const;

// Add the data to a fill map
#ifndef IMAGINE_WITH_CAIRO
  void Add(NFmiFillMap &theMap) const;
#endif

// Stroke onto given image using various Porter-Duff rules

#ifdef IMAGINE_WITH_CAIRO
  void Stroke(ImagineXr &drawing,
              NFmiColorTools::Color theColor,
              NFmiColorTools::NFmiBlendRule theRule = NFmiColorTools::kFmiColorCopy) const;
#else
  void Stroke(NFmiImage &theImage,
              NFmiColorTools::Color theColor,
              NFmiColorTools::NFmiBlendRule theRule = NFmiColorTools::kFmiColorCopy) const;
#endif

// Mark the coordinates

#ifdef IMAGINE_WITH_CAIRO
  void Mark(ImagineXr &drawing,
            const ImagineXr &marker,
            NFmiColorTools::NFmiBlendRule theRule,
            NFmiAlignment theAlignment = kFmiAlignCenter,
            float theAlpha = 1.0) const;
#else
  void Mark(NFmiImage &theImage,
            const NFmiImage &theMarker,
            NFmiColorTools::NFmiBlendRule theRule,
            NFmiAlignment theAlignment = kFmiAlignCenter,
            float theAlpha = 1.0) const;
#endif

  // Write imagemap data to a file

  void WriteImageMap(std::ostream &os, const std::string &theFieldName) const;

  void Read(const std::string &theFilename, NFmiGeoShapeType theType, const std::string &theFilter)
  {
    delete itsEsriShape;
    itsEsriShape = 0;
    itsType = theType;
    switch (itsType)
    {
      case kFmiGeoShapeEsri:
        itsEsriShape = new NFmiEsriShape();
        if (!itsEsriShape->Read(theFilename))
          throw NFmiGeoShapeError(std::string("Failed to read shape ") + theFilename);
        if (!theFilter.empty())
        {
          NFmiEsriShape *tmp = NFmiEsriTools::filter(*itsEsriShape, theFilter);
          delete itsEsriShape;
          itsEsriShape = tmp;
        }

        break;
      case kFmiGeoShapeShoreLine:
        throw NFmiGeoShapeError("kFmiGeoShapeShoreLine not implemented");
      case kFmiGeoShapeGMT:
        throw NFmiGeoShapeError("kFmiGeoShapeFMT not implemented");
    }
  }

#ifdef IMAGINE_WITH_CAIRO
  void Fill(ImagineXr &image,
            Imagine::NFmiColorTools::Color col,
            NFmiColorTools::NFmiBlendRule rule)
#else
  using NFmiDrawable::Fill;
  void Fill(Imagine::NFmiImage &image,
            Imagine::NFmiColorTools::Color col,
            NFmiColorTools::NFmiBlendRule rule) const
#endif
  {
    PathEsri().Fill(image, col, rule);
  }

 private:
  // Path creation

  const NFmiPath PathEsri() const;

// Stroking

#ifdef IMAGINE_WITH_CAIRO
  void StrokeEsri(ImagineXr &drawing,
                  NFmiColorTools::Color theColor,
                  NFmiColorTools::NFmiBlendRule theRule = NFmiColorTools::kFmiColorCopy) const;
#else
  void StrokeEsri(Imagine::NFmiImage &theImage,
                  NFmiColorTools::Color theColor,
                  NFmiColorTools::NFmiBlendRule theRule = NFmiColorTools::kFmiColorCopy) const;
#endif

// Adding to a fillmap
#ifndef IMAGINE_WITH_CAIRO
  void AddEsri(NFmiFillMap &theMap) const;
#endif

// Mark the coordinates

#ifdef IMAGINE_WITH_CAIRO
  void MarkEsri(ImagineXr &drawing,
                const ImagineXr &marker,
                NFmiColorTools::NFmiBlendRule theRule,
                NFmiAlignment theAlignment = kFmiAlignCenter,
                float theAlpha = 1.0) const;
#else
  void MarkEsri(Imagine::NFmiImage &theImage,
                const Imagine::NFmiImage &marker,
                NFmiColorTools::NFmiBlendRule theRule,
                NFmiAlignment theAlignment = kFmiAlignCenter,
                float theAlpha = 1.0) const;
#endif

  // Write imagemap data to a file

  void WriteImageMapEsri(std::ostream &os, const std::string &theFieldName) const;

  // Data-part uses pointers, so that we do not have to initialize
  // all different map data elements, since only one can be used.

  NFmiGeoShapeType itsType;
  NFmiEsriShape *itsEsriShape;
};

}  // namespace Imagine


// ======================================================================
