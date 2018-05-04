// ======================================================================
/*!
 * \file NFmiSatel.h
 * \brief Interface of class NFmiSatel
 */
// ======================================================================
/*!
 * \class NFmiSatel
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiString.h"
#include <sys/stat.h>
#include <iostream>

//! Undocumented
class _FMI_DLL NFmiSatel
{
 public:
  virtual ~NFmiSatel(void);
  NFmiSatel(void);
  NFmiSatel(const NFmiSatel& theSatel);
  NFmiSatel(char* theSatelFileName);

  virtual const NFmiString GetName(void) const;
  virtual unsigned long GetSize(void);
  virtual const char* GetData(void);

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 private:
  NFmiString itsSatelName;
  char* itsSatelData;
  unsigned long itsSatelDataSize;

};  // class NFmiSatel

// ----------------------------------------------------------------------
/*!
 * Void constructor
 * \todo Use initialization lists
 */
// ----------------------------------------------------------------------

inline NFmiSatel::NFmiSatel(void)
{
  itsSatelName = "NONE";
  itsSatelData = 0;
  itsSatelDataSize = 0;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theSatelFileName Undocumented
 * \todo Use initialization lists
 */
// ----------------------------------------------------------------------

inline NFmiSatel::NFmiSatel(char* theSatelFileName)
{
  itsSatelName = theSatelFileName;
  itsSatelData = 0;
  itsSatelDataSize = 0;
  std::ifstream datain(theSatelFileName, std::ios::in | std::ios::binary);
  if (datain.is_open())
  {
    struct _stat buf;
    _stat(theSatelFileName, &buf);

    itsSatelDataSize = buf.st_size;
    itsSatelData = new char[itsSatelDataSize];
    datain.read(itsSatelData, itsSatelDataSize);
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theSatel The object to be copied
 * \todo Use initialization lists
 */
// ----------------------------------------------------------------------

inline NFmiSatel::NFmiSatel(const NFmiSatel& theSatel)
{
  itsSatelName = theSatel.itsSatelName;
  itsSatelDataSize = theSatel.itsSatelDataSize;
  itsSatelData = new char[itsSatelDataSize];
  strcpy(itsSatelData, theSatel.itsSatelData);
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiSatel::~NFmiSatel(void)
{
  if (itsSatelData) delete itsSatelData;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiString NFmiSatel::GetName(void) const { return itsSatelName; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiSatel::GetSize(void) { return itsSatelDataSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiSatel::GetData(void) { return itsSatelData ? itsSatelData : ""; }
// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream& NFmiSatel::Write(std::ostream& file) const
{
  file << itsSatelName;
  file << itsSatelDataSize << std::endl;
  file.write(itsSatelData, itsSatelDataSize);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream& NFmiSatel::Read(std::istream& file)
{
  file >> itsSatelName;
  file >> itsSatelDataSize;
  char buff;
  file.get(buff);  // read (lf) from stream

  itsSatelData = new char[itsSatelDataSize];
  file.read(itsSatelData, itsSatelDataSize);
  return file;
}

// ======================================================================
