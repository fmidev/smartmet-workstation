// ======================================================================
//
// Abstract base class for shapefile ESRI element types
// and their attributes.
//
// ======================================================================

#pragma once

#include "NFmiEsriProjector.h"
#include "NFmiEsriAttribute.h"

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <list>

namespace Imagine
{
enum NFmiEsriElementType
{
  kFmiEsriNull = 0,
  kFmiEsriPoint = 1,
  kFmiEsriPolyLine = 3,
  kFmiEsriPolygon = 5,
  kFmiEsriMultiPoint = 8,
  kFmiEsriPointZ = 11,
  kFmiEsriPolyLineZ = 13,
  kFmiEsriPolygonZ = 15,
  kFmiEsriMultiPointZ = 18,
  kFmiEsriPointM = 21,
  kFmiEsriPolyLineM = 23,
  kFmiEsriPolygonM = 25,
  kFmiEsriMultiPointM = 28,
  kFmiEsriMultiPatch = 31
};

class NFmiEsriBox;

class _FMI_DLL NFmiEsriElement
{
 public:
  virtual ~NFmiEsriElement(void) {}
  NFmiEsriElement(NFmiEsriElementType theType, int theNumber = 0)
      : itsType(theType), itsNumber(theNumber), itsAttributes()
  {
  }

  // Copying

  virtual NFmiEsriElement* Clone() const = 0;

  // Adding an attribute

  void Add(const NFmiEsriAttribute& theAttribute) { itsAttributes.push_back(theAttribute); }
  // Returning an attribute value

  const std::string GetString(const std::string& theName) const;
  int GetInteger(const std::string& theName) const;
  double GetDouble(const std::string& theName) const;
  NFmiEsriAttributeType GetType(const std::string& theName) const;

  // Update given bounding box

  virtual void Update(NFmiEsriBox& theBox) const = 0;

  // Write element as character buffer

  virtual std::ostream& Write(std::ostream& os) const { return os; }
  // Return only the size the character buffer would take

  virtual int StringSize(void) const { return 0; }
  // Data access

  virtual NFmiEsriElementType Type(void) const { return itsType; }
  virtual int Number(void) const { return itsNumber; }
  virtual double X(void) const { return -999.0; }
  virtual double Y(void) const { return -999.0; }
  virtual double Z(void) const { return -999.0; }
  virtual double M(void) const { return -999.0; }
  virtual int NumParts(void) const { return -1; }
  virtual int NumPoints(void) const { return -1; }
  // Projection

  virtual void Project(const NFmiEsriProjector& theProjector) = 0;

 protected:
  NFmiEsriElement(void);
  NFmiEsriElement(const NFmiEsriElement& theElement)
      : itsType(theElement.itsType),
        itsNumber(theElement.itsNumber),
        itsAttributes(theElement.itsAttributes)
  {
  }

  NFmiEsriElement& operator=(const NFmiEsriElement& theElement);

  NFmiEsriElementType itsType;
  int itsNumber;
  std::list<NFmiEsriAttribute> itsAttributes;
};

}  // namespace Imagine

std::ostream& operator<<(std::ostream& os, const Imagine::NFmiEsriElement& theElement);


// ======================================================================
