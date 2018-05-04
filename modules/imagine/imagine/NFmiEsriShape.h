// ======================================================================
//
// Definition of a shape class suitable for storing information in
// ESRI shape files (*.shp) and the respective dBASE files (*.dbf).
//
// Sample use:
//
//	NFmiEsriShape shp;
//
//	shp.Read("filename");			// reads .shp, .dbf
//	shp.Read("filename",false);		// reads .shp only
//
//	shp.Write("filename");			// writes .shp .shx .dbf
//	shp.Write("filename",false);		// writes .shp .shx
//	shp.Write("filename",false,false)	// writes .shp
//
// History:
//
// 30.08.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once

// ESRI Shapefile Technical Description, page 4
//
// These are the element types, which can be in a shapefile

#include "NFmiEsriElement.h"
#include "NFmiEsriBox.h"
#include "NFmiEsriAttributeName.h"

#include <vector>

namespace Imagine
{
// ESRI Shapefile Technical Description, page 2
// Any number smaller than this limit is considered "no-data"

static const double SHP_NODATA_LIMIT = -1e38;

static const int kFmiEsriHeaderSize = 100;

static const int kFmiEsriMagicNumber = 9994;
static const int kFmiEsriVersion = 1000;

// Header element positions:

static const int kFmiEsriPosMagic = 0;  // Big endian
// unused gap
static const int kFmiEsriPosFileLength = 24;  // Big endian
static const int kFmiEsriPosVersion = 28;     // Little endian
static const int kFmiEsriPosType = 32;        // Little endian
static const int kFmiEsriPosXmin = 36;        // Little endian
static const int kFmiEsriPosYmin = 44;        // Little endian
static const int kFmiEsriPosXmax = 52;        // Little endian
static const int kFmiEsriPosYmax = 60;        // Little endian
static const int kFmiEsriPosZmin = 68;        // Little endian *
static const int kFmiEsriPosZmax = 76;        // Little endian *
static const int kFmiEsriPosMmin = 84;        // Little endian *
static const int kFmiEsriPosMmax = 92;        // Little endian *

// (*) Unused, with value 0.0, if not of M or Z type

static const int kFmiEsriRecordHeaderSize = 8;
static const int kFmiEsriRecordHeaderPosNum = 0;  // Big endian
static const int kFmiEsriRecordHeaderPosLen = 4;  // Big endian

// xBASE constants

static const int kFmixBaseHeaderSize = 32;

static const int kFmixBaseSignaturePos = 0;
static const int kFmixBaseDatePos = 1;
static const int kFmixBaseNumRecordsPos = 4;
static const int kFmixBaseHeaderLengthPos = 8;
static const int kFmixBaseRecordLengthPos = 10;

static const int kFmixBaseFieldSize = 32;

static const int kFmixBaseFieldNamePos = 0;
static const int kFmixBaseFieldTypePos = 11;
static const int kFmixBaseFieldLengthPos = 16;
static const int kFmixBaseFieldDecimalPos = 17;

static const char kFmixBaseFieldNumber = 'N';
static const char kFmixBaseFieldFloat = 'F';
static const char kFmixBaseFieldChar = 'C';
static const char kFmixBaseFieldInt = 'I';

class _FMI_DLL NFmiEsriShape
{
 public:
  // Typedefs to aid iterating

  typedef std::vector<NFmiEsriElement *> elements_type;
  typedef std::vector<NFmiEsriAttributeName *> attributes_type;

  typedef elements_type::const_iterator const_iterator;
  typedef elements_type::iterator iterator;

  const attributes_type &Attributes(void) const { return itsAttributeNames; }
  // Constructor, destructor

  NFmiEsriShape(NFmiEsriElementType theType) : itsShapeType(theType) {}
  NFmiEsriShape(void) {}
  ~NFmiEsriShape(void) { Init(); }
  // Access to data members

  NFmiEsriElementType Type(void) const { return itsShapeType; }
  const NFmiEsriBox &Box(void) const { return itsBox; }
  const elements_type &Elements(void) const { return itsElements; }
  // Add an element

  void Add(NFmiEsriElement *theElement);

  // Add an attribute name

  void Add(NFmiEsriAttributeName *theAttributeName);

  // Reading and writing data

  bool Read(const std::string &theFilename, bool fDBF = true);
  bool Write(const std::string &theFilename, bool fDBF = true, bool fSHX = true) const;

  bool WriteSHP(const std::string &theFilename) const;
  bool WriteSHX(const std::string &theFilename) const;
  bool WriteDBF(const std::string &theFilename) const;

  // Projecting

  void Project(const NFmiEsriProjector &theProjector);

  // Return desired attributename, or null pointer if not found

  NFmiEsriAttributeName *AttributeName(const std::string &theFieldName) const;

 private:
  NFmiEsriShape(const NFmiEsriShape &theShape);
  NFmiEsriShape &operator=(const NFmiEsriShape);

  // This one initializes by destroying possible old contents

  void Init(void);

  // Adding a stringed element

  bool Add(int theRecordNumber, const std::string &theBuffer, int thePos);

  int CountRecords(const std::string &theBuffer) const;

  // Header writing utility

  void WriteHeader(std::ostream &os, int theFileLength) const;

  // Data members

  NFmiEsriElementType itsShapeType;  // The shape type
  NFmiEsriBox itsBox;                // The bounding box

  elements_type itsElements;  // the actual data

  // This one is special, we store all the different EsriAttributeNames
  // into this list, so that when the shape is destructed we can
  // destroy all the AttributeNames used in the shape. The individual
  // elements will user pointers to these elements, and do not ever
  // attempt to destroy them.

  attributes_type itsAttributeNames;

  // Constants related to headers:
};

}  // namespace Imagine

// ======================================================================
