// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::FireWarnings
 */
// ======================================================================

#include "FireWarnings.h"
#include <calculator/TextGenError.h>
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiStringTools.h>
#include <newbase/NFmiStaticTime.h>
#include <fstream>

using namespace std;

namespace TextGen
{
//! Firewarning areas run from 1 to 46

const int MaxAreaCode = 46;

// ----------------------------------------------------------------------
/*!
 * \brief FireWarnings constructor
 */
// ----------------------------------------------------------------------

FireWarnings::FireWarnings(const string& theDirectory, const TextGenPosixTime& theTime)
    : itsTime(theTime), itsWarnings(MaxAreaCode + 1, Undefined)
{
  if (!NFmiFileSystem::DirectoryExists(theDirectory))
    throw TextGenError("Directory '" + theDirectory +
                       "' required by class FireWarnings does not exist");

  // Form the expected filename of form YYYYMMDD.palot_koodina
  // If we cannot find the file, an exception is thrown

  string filename = (theDirectory + '/' + theTime.ToStr(kYYYYMMDD) + ".palot_koodina");
  if (!NFmiFileSystem::FileExists(filename))
  {
    TextGenPosixTime tmp = theTime;
    tmp.ChangeByDays(-1);
    filename = (theDirectory + '/' + tmp.ToStr(kYYYYMMDD) + ".palot_koodina");
    if (!NFmiFileSystem::FileExists(filename))
      throw TextGenError("Cannot find warnings from '" + theDirectory + "'");
  }

  // Read the file

  ifstream input(filename.c_str(), ios::in);
  if (!input) throw TextGenError("Failed to open '" + filename + "' for reading");

  // Skip the date
  string tmp;
  input >> tmp;

  // Read the areacode - state pairs

  int areacode;
  int areastate;
  while (input >> areacode >> areastate)
  {
    if (areacode < 1 || areacode > MaxAreaCode)
      throw TextGenError("File '" + filename + "' contains invalid areacode " +
                         NFmiStringTools::Convert(areacode));
    switch (State(areastate))
    {
      case None:
      case GrassFireWarning:
      case FireWarning:
        itsWarnings[areacode] = State(areastate);
        break;
      default:
        throw TextGenError("File '" + filename + "' contains invalid warningcode " +
                           NFmiStringTools::Convert(areastate));
    }
  }
  input.close();
}

// ----------------------------------------------------------------------
/*!
 * \brief get the warning state for the given area code
 *
 * \param theArea The area code (1...46)
 * \return The state
 */
// ----------------------------------------------------------------------

FireWarnings::State FireWarnings::state(int theArea) const
{
  if (theArea < 1 || theArea > MaxAreaCode)
    throw TextGenError("Only area codes 1...46 are allowed in FireWarnings");
  return itsWarnings[theArea];
}

}  // namespace TextGen

// ======================================================================
