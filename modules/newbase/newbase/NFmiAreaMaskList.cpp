// ======================================================================
/*!
 * \file NFmiAreaMaskList.cpp
 * \brief Implementation of class NFmiAreaMaskList
 */
// ======================================================================
/*!
 * \class NFmiAreaMaskList
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiAreaMaskList.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskList::~NFmiAreaMaskList() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskList::NFmiAreaMaskList() : itsMaskVector(), itsCurrentIndex(-1), fMaskInUse(false) {}

NFmiAreaMaskList::NFmiAreaMaskList(const NFmiAreaMaskList &theOther)

    = default;

boost::shared_ptr<NFmiAreaMaskList> NFmiAreaMaskList::CreateShallowCopy(
    const boost::shared_ptr<NFmiAreaMaskList> &theOther)
{
  if (theOther)
  {
    boost::shared_ptr<NFmiAreaMaskList> copyMaskList(new NFmiAreaMaskList());
    for (theOther->Reset(); theOther->Next();)
    {
      boost::shared_ptr<NFmiAreaMask> copyedMask(theOther->Current()->Clone());
      copyMaskList->Add(copyedMask);
    }
    copyMaskList->CheckIfMaskUsed();
    return copyMaskList;
  }
  else
    return boost::shared_ptr<NFmiAreaMaskList>();
}

unsigned long NFmiAreaMaskList::NumberOfItems()
{
  return static_cast<unsigned long>(itsMaskVector.size());
}

// ----------------------------------------------------------------------
/*!
 * \param theMask Undocumented, unused parameter
 * \return Undocumented
 *
 * \todo Investigate whether theMask parameter can be removed
 */
// ----------------------------------------------------------------------

void NFmiAreaMaskList::Add(boost::shared_ptr<NFmiAreaMask> &theMask)
{
  itsMaskVector.push_back(theMask);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Reset()
{
  itsCurrentIndex = -1;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Next()
{
  itsCurrentIndex++;
  if (static_cast<std::size_t>(itsCurrentIndex) < itsMaskVector.size())
    return true;
  else
    return false;
}

// Tämä tarkistaa vektori-taulukkoon osoittavia indeksejä, jotka alkavat 0:sta.
bool NFmiAreaMaskList::IsValidIndex(int theIndex)
{
  if (theIndex >= 0 && static_cast<std::size_t>(theIndex) < itsMaskVector.size())
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always zero pointer
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

boost::shared_ptr<NFmiAreaMask> NFmiAreaMaskList::Current()
{
  if (IsValidIndex(itsCurrentIndex))
    return itsMaskVector[itsCurrentIndex];
  else
  {
    // jos indeksi on vektorin ulkopuolella, palautetaan 0-pointteri (shared_ptr tyhjänä)
    static boost::shared_ptr<NFmiAreaMask> dummy;
    return dummy;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Remove()
{
  if (IsValidIndex(itsCurrentIndex))
  {
    itsMaskVector.erase(itsMaskVector.begin() + itsCurrentIndex);
    return true;
  }
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 *  Käy läpi listassa olevat maski-oliot ja kysyy
 *  niiltä IsMasked ja tekee tulosta AND operaation kanssa.
 * \param theLatLon Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::IsMasked(const NFmiPoint &theLatLon)
{
  if (fMaskInUse)  // 1999.09.24/Marko Muutin tämän katsomaan ensin onko maski käytössä
  {                // jos on, katsotaan onko maskattu, muuten on aina maskattu
                   // näin pääsee eroon muutamasta ikävästä if-lause testeistä
    for (auto &index : itsMaskVector)
    {
      if (index->IsEnabled())
      {
        if (!(index->IsMasked(theLatLon)))
        {
          return false;
        }
      }
    }
    return true;
  }
  else
    return true;
}

// ----------------------------------------------------------------------
/*!
 * käy kaikki listan maskit läpi ja katsoo, ettei yhdenkään maskin kerroin ole 0,
 * tällöin palautetaan 0. Jos ei ole yhtään 0-kerrointa, lasketaan ramppimaskien
 * keskiarvo, joka palautetaan (tavallisia maskeja ei lasketa tässä koska ne
 * mukaan lukien tulisi liian rankkoja muutoksia). Jos vain tavallisia maskeja, palautetaan arvo 1.
 * \param theLatLon Undocumented, unused
 * \return Undocumented, always kFloatMissing
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

double NFmiAreaMaskList::MaskValue(const NFmiPoint &theLatLon)
{
  if (fMaskInUse)  // 1999.09.24/Marko Muutin tämän katsomaan ensin onko maski käytössä
  {                // jos on, katsotaan onko maskattu, muuten on aina maskattu
                   // näin pääsee eroon muutamasta ikävästä if-lause testeistä
    double sum = 0, tempValue = 0;
    int count = 0;
    for (auto &index : itsMaskVector)
    {
      if (index->IsEnabled())
      {
        tempValue = index->MaskValue(theLatLon);
        if (!tempValue)
        {
          return 0.;
        }
        else
        {
          if (index->IsRampMask())
          {
            sum += tempValue;
            count++;
          }
        }
      }
    }
    if (count) return sum / count;
  }
  return 1.;
}

// ----------------------------------------------------------------------
/*!
 *  Etsii annetun indeksin mukaisen otuksen listasta ja asettaa itsCurrentIndex:in osoittamaan
 * siihen.
 *  HUOM! theIndex on 1:sta alkava indeksi, sisäinen muuttuja itsCurrentIndex on taas 0:sta alkava
 * \param theIndex 1:llä alkava indeksi jota etsitään maski vektorista.
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Find(unsigned long theIndex)
{
  int vectorIndex = static_cast<int>(theIndex) - 1;
  if (IsValidIndex(vectorIndex))
  {
    itsCurrentIndex = vectorIndex;
    return true;
  }
  else
  {
    itsCurrentIndex = -1;
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented, unused
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

void NFmiAreaMaskList::Clear() { itsMaskVector.clear(); }
// ----------------------------------------------------------------------
/*!
 *  Tarkistaa listassa olevilta maskeilta, onko
 *  yhtään maskia käytössä (enabled). Jos on, asetetaan
 *  fMaskInUse attribuutti true:ksi jos ei asetetaan
 *  se false:ksi. Funktio palauttaa fMaskInUse:in arvon.
 *  Jatkossa listalta voidaan kysyä metodilla UseMask
 *  fMaskInUse:n tilaa. (tämä on optimointia varten tehty
 *  viritelmä)
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::CheckIfMaskUsed()
{
  fMaskInUse = false;
  for (auto &index : itsMaskVector)
  {
    if (index->IsEnabled())
    {
      fMaskInUse = true;
    }
  }
  return fMaskInUse;
}

// ----------------------------------------------------------------------
/*!
 *  Käy enabloidut maskit läpi antaa theTime:n
 *  niiden Time-metodille.
 * \param theTime Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::SyncronizeMaskTime(const NFmiMetTime &theTime)
{
  for (auto &index : itsMaskVector)
  {
    if (index->IsEnabled()) index->Time(theTime);
  }
  return true;
}

// ----------------------------------------------------------------------
/*!
 *  HUOM! theIndex on 1:sta alkava indeksi, sisäinen muuttuja itsCurrentIndex on taas 0:sta alkava
 * \param theIndex theIndex alkaa 1:stä
 * \return jos löytyi halutulla indeksillä oleva maski, palauttaa true
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Index(unsigned long theIndex) { return Find(theIndex); }
// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Find(const NFmiDataIdent &theParam)
{
  for (auto &index : itsMaskVector)
  {
    if (index->IsWantedParam(theParam)) return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented, unused
 * \param theLevel Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Find(const NFmiDataIdent &theParam, const NFmiLevel *theLevel)
{
  for (auto &index : itsMaskVector)
  {
    if (index->IsWantedParam(theParam, theLevel)) return true;
  }
  return false;
}

// ======================================================================
