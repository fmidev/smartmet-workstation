// ======================================================================
/*!
 * \file NFmiIndividual.cpp
 * \brief Implementation of class NFmiIndividual
 */
// ======================================================================
/*!
 * \class NFmiIndividual
 *
 * NFmiIndividual whose purpose is to provide a derived class with
 * a name and a number.
 *
 * The class is old fashioned and should not be used unless necessary.
 *
 */
// ======================================================================

#include "NFmiIndividual.h"
#include "NFmiVersion.h"
#include <boost/functional/hash.hpp>
#include <fstream>

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theIndividual The other object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiIndividual& NFmiIndividual::operator=(const NFmiIndividual& theIndividual)
{
  itsName = theIndividual.itsName;
  itsIdent = theIndividual.itsIdent;
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiIndividual::Write(std::ostream& file) const
{
  file << itsIdent << std::endl;
  if (FmiBoxVersion >= 2)
  {
    file << itsName;
  }
  else
  {
    file << itsIdent << " " << itsName.GetCharPtr() << " ";
  }
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 * \todo Kill the cat
 */
// ----------------------------------------------------------------------

std::istream& NFmiIndividual::Read(std::istream& file)
{
  file >> itsIdent;

  if (FmiBoxVersion >= 2)
  {
    file >> itsName;
  }
  else
  {
    char tmp[255];
    file >> tmp;

    itsName = tmp;
  }

  return file;
}

void NFmiIndividual::SetContents(long ident, NFmiString name)
{
  itsIdent = ident;
  itsName = name;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the individual
 */
// ----------------------------------------------------------------------

std::size_t NFmiIndividual::HashValue() const
{
  std::size_t hash = itsName.HashValue();
  boost::hash_combine(hash, boost::hash_value(itsIdent));
  return hash;
}
