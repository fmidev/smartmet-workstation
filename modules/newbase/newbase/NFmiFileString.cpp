// ======================================================================
/*!
 * \file NFmiFileString.cpp
 * \brief Implementation of class NFmiFileString
 */
// ======================================================================
/*!
 * \class NFmiFileString
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiFileString.h"
#include <algorithm>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiFileString::~NFmiFileString() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiFileString::NFmiFileString() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theStr Undocumented
 */
// ----------------------------------------------------------------------

NFmiFileString::NFmiFileString(const NFmiString &theStr) : NFmiString(theStr)
{
  NormalizeDelimiter();
}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 */
// ----------------------------------------------------------------------

NFmiFileString::NFmiFileString(const NFmiFileString &theFileStr) : NFmiString(theFileStr)
{
  NormalizeDelimiter();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFileString::HasExtension() const
{
  unsigned char find[2] = ".";
#ifndef UNIX
  unsigned char slash[2] = "\\";
#else
  unsigned char slash[2] = "/";
#endif

  unsigned long lastSlash = SearchLast(slash);
  return Search(find, std::max(lastSlash, 1ul)) > 0 ? true : false;
}

bool NFmiFileString::IsAbsolutePath() const
{
  if (fChar[0] == '/' || fChar[0] == '\\' || Device() != NFmiString("")) return true;
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFileString::Extension() const
{
  unsigned long last;
  unsigned char find[2] = ".";

  last = SearchLast(find);
  if (last)
    return GetChars(last + 1, GetLen() - last);
  else
    return "";  // jos last oli 0, ei löytynyt .-merkkiä  ja ei ole extensiota
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFileString::FileName() const
{
  unsigned long last;
  unsigned char find[2] = {kFmiDirectorySeparator, '\0'};

  last = SearchLast(find);

  return GetChars(last + 1, GetLen() - last);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFileString::Header() const
{
  unsigned long first, last;
  unsigned char find1[2] = {kFmiDirectorySeparator, '\0'};
  unsigned char find2[2] = ".";

  first = SearchLast(find1);

  last = HasExtension() ? SearchLast(find2) : 0;
  // last  = SearchLast(find2);

  if (last == 0) last = GetLen() + 1;

  return GetChars(first + 1, last - (first + 1));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Ei hoida tilannetta jos filestringin sisältö on:
 *  polku/tiedosto.ext (ilman ./ alkua)
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFileString::Path() const
{
  unsigned long first, last;
  unsigned char find[2] = R"(\)";
  unsigned char find2[2] = "/";

  first = Search(find);
  last = SearchLast(find);

  if (first == 0 && last == 0)  // kokeillaan vielä kenoja toisin päin
  {
    first = Search(find2);
    last = SearchLast(find2);
  }

  if (first)
  {
    if (fChar[0] == '.')  // Lasse 23.9.98 suht polku
      return GetChars(1, last);
    else if (IsAbsolutePath() == false)  // suhteellinen polku ilman .-merkkiä alussa
      return GetChars(1, last);
    else
      return GetChars(first, (last + 1) - first);
  }

  return NFmiString();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFileString::Device() const
{
#ifdef UNIX
  return NFmiString();  // linux/unix/mac ei ole devicea, joten palautetaan tyhjä
#else                   // windowsissa on device
  unsigned long last;
  unsigned char find[2] = ":";

  last = Search(find);

  if (last) return GetChars(1, last);

  return NFmiString();
#endif
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFileString::PathWithoutLastDirectory()
{
  unsigned long first, last, nextLast, pos;
  unsigned char find[2] = {kFmiDirectorySeparator, '\0'};

  first = Search(find);
  last = first;
  nextLast = last;
  pos = Search(find, first + 1);
  while (pos > 0)
  {
    nextLast = last;
    last = pos;
    pos = Search(find, ++pos);
  }

  if (first)
  {
    if (fChar[0] == '.')
      return GetChars(1, nextLast);
    else
      return GetChars(first, (nextLast + 1) - first);
  }

  return NFmiString();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiFileString::Directory() const
{
  unsigned long first, last;
  unsigned char backSlash[2] = {kFmiDirectorySeparator, '\0'};

  unsigned long pos = 1;
  pos = Search(backSlash, pos);
  first = pos;
  last = pos;
  pos = Search(backSlash, pos);
  while (pos > 0)
  {
    first = last;
    last = pos;
    pos = Search(backSlash, ++pos);
  }
  if (last > first)
    return GetChars(first + 1, last - (first + 1));
  else
    return NFmiString();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::NormalizeDelimiter()
{
// 31.8.98 Lehtiohjelmassa on ainakin tarvetta/LW
#ifndef UNIX
  unsigned char slash[2] = "/";
  unsigned char backSlash[2] = "\\";
  ReplaceChars(slash, backSlash);
#endif
}

// ----------------------------------------------------------------------
/*!
 * \param theDirectory Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::AddDirectory(const NFmiString &theDirectory)
{
  NFmiString newStr(40);

  newStr = Device();
  newStr += Path();
  newStr += theDirectory;
  newStr += kFmiDirectorySeparator;
  newStr += Header();
  if (HasExtension())
  {
    newStr += NFmiString(".");
    newStr += Extension();
  }
  *this = newStr;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiFileString::DeleteDirectory()
{
  NFmiString newStr(40);

  newStr = Device();
  newStr += PathWithoutLastDirectory();
  newStr += Header();
  if (HasExtension())
  {
    newStr += NFmiString(".");
    newStr += Extension();
  }
  *this = newStr;
}

// ----------------------------------------------------------------------
/*!
 * \param theDirectory Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::ReplaceDirectory(const NFmiString &theDirectory)
{
  NFmiString newStr(60);

  newStr = Device();
  newStr += PathWithoutLastDirectory();
  newStr += theDirectory;
  newStr += kFmiDirectorySeparator;
  newStr += Header();
  if (HasExtension())
  {
    newStr += NFmiString(".");
    newStr += Extension();
  }
  *this = newStr;
}

// ----------------------------------------------------------------------
/*!
 * \param theExt Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::Extension(const char *theExt)
{
  NFmiString newStr(40);

  newStr = Device();
  newStr += Path();
  newStr += Header();
  newStr += ".";
  newStr += theExt;
  *this = newStr;
}

// ----------------------------------------------------------------------
/*!
 * \param theExtension Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::FileName(const NFmiString &theExtension)
{
  FileName(static_cast<char *>(theExtension));
}

// ----------------------------------------------------------------------
/*!
 * \param theFileName Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::FileName(const char *theFileName)
{
  NFmiString newStr(40);

  newStr = Device();
  newStr += Path();
  newStr += theFileName;
  *this = newStr;
}

// ----------------------------------------------------------------------
/*!
 * \param theHeader Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::Header(const NFmiString &theExtension) { Header(theExtension.CharPtr()); }
void NFmiFileString::Header(const char *theHeader)
{
  NFmiString newStr(40);

  newStr = Device();
  newStr += Path();
  newStr += theHeader;
  if (HasExtension())
  {
    newStr += ".";
    newStr += Extension();
  }
  *this = newStr;
}

// ----------------------------------------------------------------------
/*!
 * \param thePath Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::Path(const char *thePath)
{
  NFmiString newStr(40);

  newStr = Device();
  newStr += thePath;
  if (newStr.GetChars(newStr.GetLen(), 1) != NFmiString(kFmiDirectorySeparator))
    newStr += kFmiDirectorySeparator;
  newStr += FileName();
  *this = newStr;
}

// ----------------------------------------------------------------------
/*!
 * \param theDevice Undocumented
 */
// ----------------------------------------------------------------------

void NFmiFileString::Device(const char *theDevice)
{
  NFmiString newStr(40);

  newStr = theDevice;
  newStr += Path();
  newStr += FileName();
  *this = newStr;
}

void NFmiFileString::Device(const NFmiString &theExtension) { Device(theExtension.CharPtr()); }
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiFileString::ChangeScandinavian()
{
  NFmiString skand("äöåÄÖÅ");
  NFmiString notSkand("aoaAOA");
  for (unsigned int ind = 1; ind <= skand.GetLen(); ind++)
    ReplaceChars(reinterpret_cast<unsigned char *>(skand.GetCharsPtr(ind, 1)),
                 reinterpret_cast<unsigned char *>(notSkand.GetCharsPtr(ind, 1)));
}

// ======================================================================
