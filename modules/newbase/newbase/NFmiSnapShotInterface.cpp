// ======================================================================
/*!
 * \file NFmiSnapShotInterface.cpp
 * \brief Implementation of class NFmiSnapShortInterface
 */
// ======================================================================

#include "NFmiSnapShotInterface.h"
#include <ctime>
#include <fstream>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiSnapShotInterface::~NFmiSnapShotInterface()
{
  delete itsData;
  delete itsInfo;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theDataFileName Undocumented
 * \param theWorkingDirectory Undocumented
 * \param theSourceDirectory Undocumented
 * \param theUpdateInterval Undocumented
 */
// ----------------------------------------------------------------------

NFmiSnapShotInterface::NFmiSnapShotInterface(NFmiString theDataFileName,
                                             NFmiString theWorkingDirectory,
                                             NFmiString theSourceDirectory,
                                             time_t theUpdateInterval)
    : fIsValid(),
      itsUpdateInterval(theUpdateInterval),
      itsInfo(nullptr),
      itsData(nullptr),
      itsDataFileName(),
      itsSourceDirectory(theSourceDirectory),
      itsWorkingDirectory(theWorkingDirectory),
      itsStartingTime(time(nullptr))
{
  theDataFileName.UpperCase();
  if (theDataFileName == NFmiString("ECMWF"))
  {
    itsDataFileName = "ECMWF_Suomi_192_3_uusin.sqd";
  }
  else if (theDataFileName == NFmiString("HIRLAM"))
  {
    itsDataFileName = "HIRLAM_Suomi_48_1_uusin.sqd";
  }
  else if (theDataFileName == NFmiString("KEPA"))
  {
    itsDataFileName = "kepa_suomi_168_1_uusin.sqd";
  }
  else if (theDataFileName == NFmiString("KEPALYHYT"))
  {
    itsDataFileName = "Kepa_LightBoy_Uusin.sqd";
  }
  else if (theDataFileName == NFmiString("KEPAPITKÄ"))
  {
    itsDataFileName = "Kepa_FatBoy_Uusin.sqd";
  }
  else if (theDataFileName == NFmiString("SYNOP"))
  {
    itsDataFileName = "Havainto_uusin.fqd";
  }
  else if (theDataFileName == NFmiString("TUTKA"))
  {
    itsDataFileName = "RadRain1h_ennuste4h10km.sqd";
  }
  else if (theDataFileName == NFmiString("TUTKATIHEÄ"))
  {
    itsDataFileName = "Sadekertyma-1_uusin.sqd";
  }
  else if (theDataFileName == NFmiString("TOPOGRAFIA"))
  {
    itsDataFileName = R"(Mask\topography_europe_400x399_polster_text.fqd)";
  }
  else if (theDataFileName == NFmiString("TIEPIIRIT"))
  {
    itsDataFileName = R"(Mask\tiealueet2.sqd)";
  }
  else
  {
    itsDataFileName = theDataFileName;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theInfo Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSnapShotInterface::Update(NFmiQueryInfo** theInfo)
{
  bool retBool = false;
  if (IsValid())
    return retBool;

  else if (itsInfo)
  {
    retBool = true;
    if (theInfo) delete theInfo;
  }
  else if (ReadData())
  {
    retBool = true;
  }
  if (itsInfo) *theInfo = new NFmiQueryInfo(*itsInfo);
  return retBool;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSnapShotInterface::IsValid()
{
  if (!itsInfo || time(nullptr) - itsStartingTime > itsUpdateInterval)
  {
    return false;
  }
  else
    return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSnapShotInterface::ReadData()
{
  // Kopioidaan tiedosto paikalliseksi
  NFmiString command = NFmiString("clone.exe ");
  command += itsSourceDirectory;
  command += NFmiString(" ");
  command += itsDataFileName;
  command += NFmiString(" ");
  command += itsWorkingDirectory;

  // Mika 08.11.2001 Kayttamaton paluuarvo antaa varoituksia.
  // int aRet = system(command);
  // if( aRet != 0 )
  // return false;
  system(command);

  delete itsData;
  delete itsInfo;
  itsData = nullptr;
  itsInfo = nullptr;
  NFmiString fileName = NFmiString(itsWorkingDirectory) += itsDataFileName;
  ifstream localFile;
  localFile.open(fileName, ios::in | ios::binary);
  if (localFile)
  {
    itsData = new NFmiQueryData();
    localFile >> *itsData;
    localFile.close();
  }
  else
  {
    ifstream remoteFile;
    fileName = NFmiString(itsSourceDirectory) += itsDataFileName;
    remoteFile.open(fileName, ios::in | ios::binary);
    if (remoteFile)
    {
      itsData = new NFmiQueryData();
      remoteFile >> *itsData;
      remoteFile.close();
    }
    else
      return false;
  }
  itsInfo = new NFmiQueryInfo(itsData);
  return true;
}

// ======================================================================
