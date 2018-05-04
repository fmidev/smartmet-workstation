// ======================================================================
/*!
 * \file NFmiTimeList.cpp
 * \brief Implementation of class NFmiTimeList
 */
// ======================================================================
/*!
 * \class NFmiTimeList
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiTimeList.h"
#include "NFmiMetTime.h"
#include "NFmiTimeBag.h"
#include <algorithm>
#include <cmath>
#include <functional>

#ifdef UNIX
long int abs(long int theValue);
long int abs(long int theValue) { return (theValue < 0 ? -theValue : theValue); }
#endif

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

using namespace std;

NFmiTimeList::~NFmiTimeList() { Clear(true); }
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theList The object being copied
 */
// ----------------------------------------------------------------------

NFmiTimeList::NFmiTimeList(const NFmiTimeList &theList)
    : itsVectorList(theList.itsVectorList),
      itsIndex(theList.itsIndex),
      itsIsReset(theList.itsIsReset)
{
  int vecSize = itsVectorList.size();
  for (int i = 0; i < vecSize; i++)
    itsVectorList[i] = new NFmiMetTime(*theList.itsVectorList[i]);
}

// ----------------------------------------------------------------------
/*!
 * Constructor that produces timelist from timebag.
 *
 * \param theTimes The object being copied
 */
// ----------------------------------------------------------------------

NFmiTimeList::NFmiTimeList(const NFmiTimeBag &theTimes)
    : itsVectorList(theTimes.GetSize()),
      itsIndex(theTimes.CurrentIndex()),
      itsIsReset(theTimes.CurrentIndex() >= 0 ? false : true)
{
  NFmiTimeBag tmpBag(theTimes);
  tmpBag.Reset();
  int vecSize = itsVectorList.size();
  for (int i = 0; i < vecSize; i++)
  {
    tmpBag.Next();
    itsVectorList[i] = new NFmiMetTime(tmpBag.CurrentTime());
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------
// Marko: en ole testannut funktiota, enkä tajua mihin sitä voisi tarvita, joten varokaa.
bool NFmiTimeList::Next(NFmiMetTime **theItem) const
{
  *theItem = Current();
  if (*theItem)
  {
    Next();
    return true;
  }

  return false;
  /*
  *theItem=Current();
  if(*theItem)
	{						// VAROITUS !
	  itsIter->Next();		// Viimeisen alkion jälkeen mennään listan ulkopuolelle true:lla;
	  return true;			// vasta seuraavalla kerralla tämä Next palauttaa false !!	
	  // return Next();		<-- Näin kursori jääsi osoittamaan viimeistä itemiä, mutta toisaalta nyt
	}						// return false on harhaan johtava, sillä onhan saatu mielekäs theItem.

	return false;			// Suosittelen metodien Next(void) & Current() käyttöä, jolloin ei voi joutua ulos listalta
*/  // viljo 12.05.-97
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::Next() const
{
  if (itsIsReset)
    return First();
  else
  {
    itsIndex++;
    return IndexOk(itsIndex);
  }
}

bool NFmiTimeList::IndexOk(int theIndex) const
{
  if (theIndex >= 0 && static_cast<unsigned int>(theIndex) < itsVectorList.size()) return true;
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::Previous() const
{
  if (itsIsReset)
    return false;
  else
  {
    itsIndex--;
    return IndexOk(itsIndex);
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetTime *NFmiTimeList::Current() const
{
  if (IndexOk(itsIndex)) return itsVectorList[itsIndex];
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::Reset() const
{
  itsIsReset = true;
  itsIndex = -1;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::First() const
{
  if (itsVectorList.empty())
  {
    Reset();
    return false;
  }
  else
  {
    itsIsReset = false;
    itsIndex = 0;
    return true;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Lisää uusi kellonaika aikalistaan
 *
 * Oletusarvoisesti aikalista on sortattu kasvavaan järjestykseen.
 *
 * Add ei lisää annettua aikaa, jos se löytyy jo listasta ennestään.
 * Lisäksi objekti deletoidaan, kuten destruktorikin tekisi.
 *
 * Jos aikaa ei ole lisätty aiemmin, lisätään se aikajärjestyksessä
 * oikeaan kohtaan.
 *
 * \param theItem Pointteri lisättävään aikaan
 */
// ----------------------------------------------------------------------

void NFmiTimeList::Add(NFmiMetTime *theItem, bool fAllowDuplicates, bool fAddEnd)
{
  // etsitään ensimmäinen kohta, jossa vanha aika >= uusi aika

  auto it(itsVectorList.begin());

  if (fAddEnd)  // pakko optimoida, koska salamadatassa on jumalattomasti aikoja ja ne ovat jo
                // järjestyksessä!!!
    itsVectorList.push_back(theItem);
  else
  {
    for (; it != itsVectorList.end(); ++it)
      if (**it >= *theItem) break;

    // jos kaikki ajat olivat pienempiä, liitetään vain perään
    if (it == itsVectorList.end()) itsVectorList.push_back(theItem);

    // jos löytyi sama aika, ei insertoida vaan deletoidaan pois (paitsi jos duplikaatit sallitaan)
    else if ((**it == *theItem) && (!fAllowDuplicates))
      delete theItem;

    // muuten insertoidaan oikeaan kohtaan
    else
      itsVectorList.insert(it, theItem);
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTimeList::AddOver(NFmiMetTime *theItem)
{
  if (Find(*theItem))
  {
    delete theItem;
    return;
  }
  Add(theItem);
}

// ----------------------------------------------------------------------
/*!
 * \param theList Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTimeList::Add(NFmiTimeList *theList)
{
  int vecSize = theList->itsVectorList.size();
  for (int i = 0; i < vecSize; i++)
    itsVectorList.push_back(new NFmiMetTime(*theList->itsVectorList[i]));
  // vanha versio resetoi lopuksi, en näe mitää syytä moiseen/Marko

  /*
    *itsList+=(*theList->itsList);
    delete itsIter;
    itsIter = new NFmiVoidPtrIterator(itsList);
    itsIter->Reset();
    */
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented
 */
// ----------------------------------------------------------------------

void NFmiTimeList::Clear(bool fDeleteData)
{
  if (fDeleteData)
  {
    int vecSize = itsVectorList.size();
    for (int i = 0; i < vecSize; i++)
      delete itsVectorList[i];
  }
  checkedVector<NFmiMetTime *>().swap(itsVectorList);
  Reset();  // vanha laittoi tyhjennyksen jälkeen Firstiin, missä ei ole järkeä

  /*
    if( fDeleteData )
          {
            NFmiMetTime * aItem = 0;
            First();
            while(Next(&aItem))
                  delete aItem;
          }
    if(itsList)
          itsList->Clear();
    First();
    */
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theList The other object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiTimeList &NFmiTimeList::operator=(const NFmiTimeList &theList)
{
  Clear(true);  // vanha vuoti, koska ei tuhonnut metTimeja listasta, saattaa kaataa ohjelmia, jotka
                // luottavat tähän ominaisuuteen
  int vecSize = theList.itsVectorList.size();
  for (int i = 0; i < vecSize; i++)
    itsVectorList.push_back(new NFmiMetTime(*theList.itsVectorList[i]));

  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param theList Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::operator==(const NFmiTimeList &theList) const
{
  bool retVal = false;
  for (this->Reset(), theList.Reset(); this->Next() && theList.Next();)
  {
    if (!(this->Current()->IsEqual(*(theList.Current())))) return false;
    retVal = true;
  }
  return retVal;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::Index(int theIndex) const
{
  if (IndexOk(theIndex))
  {
    itsIndex = theIndex;
    return true;
  }
  return false;
  //  return itsIter?itsIter->Index(theIndex):false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

int NFmiTimeList::Index() const
{
  return itsIndex;
  //  return itsIter?itsIter->Index():false;		// index = -1 out of list
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiTimeList::Write(std::ostream &file) const
{
  long items = NumberOfItems();
  file << items << std::endl;
  First();
  NFmiMetTime *aItem;
  for (long i = 0; i < items; i++)
  {
    Next(&aItem);

    file << aItem->GetYear() << " " << aItem->GetMonth() << " " << aItem->GetDay() << " "
         << aItem->GetHour() << " " << aItem->GetMin() << " " << aItem->GetSec() << std::endl;
  }
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

std::istream &NFmiTimeList::Read(std::istream &file)
{
  int items;
  short year, month, day, hour, min, sec;

  file >> items;
  First();
  NFmiMetTime *aItem;
  for (long i = 0; i < items; i++)
  {
    file >> year >> month >> day >> hour >> min >> sec;
    aItem = new NFmiMetTime(year, month, day, hour, min, sec, 1);
    if (sec) aItem->SetSec(sec);

    Add(aItem, true);  // true= sallitaan duplikaatit
  }
  return file;
}

// apu-funktori, jolla vertaillaan aikalistalla olevia aika-pointtereita
template <typename T>
struct ComparePtrs : public binary_function<T, T, bool>
{
  bool operator()(const T *lhs, const T *rhs) { return *lhs < *rhs; }
};

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::Find(const NFmiMetTime &theTime)
{
  itsIndex = -1;
  if (itsVectorList.empty()) return false;
  auto pos = std::lower_bound(
      itsVectorList.begin(), itsVectorList.end(), &theTime, ComparePtrs<NFmiMetTime>());
  if (pos != itsVectorList.end())
  {
    if (theTime == *(*pos))  // tässä pitää vielä tarkistaa löytyikö varmasti oikea aika!
    {
      itsIndex = std::distance(itsVectorList.begin(), pos);
      return true;
    }
  }
  return false;

  /*
    if(First())
          {
            do
                  {
                    if(*Current() == theTime)
                          return true;
                  }
            while(Next());
          }
    return false;
  */
}

// ----------------------------------------------------------------------
/*!
 * \param theTime is searched time
 * \param theDirection tells which direction search is done. If its kBackward, search
 *  only backwards from theTime. If kCenter, search in both directions and if kForward,
 *  search only forward from theTime.
 * \param theTimeRangeInMinutes tells how far you are allowed to search in wanted direction.
 * If theTimeRangeInMinutes value is kUnsignedLongMissing, there is no time limit in search.
 * \return Undocumented
 * Assumption: times in the itsVectorList are in rising order.
 */
// ----------------------------------------------------------------------

bool NFmiTimeList::FindNearestTime(const NFmiMetTime &theTime,
                                   FmiDirection theDirection,
                                   unsigned long theTimeRangeInMinutes)
{
  if (itsVectorList.empty()) return false;

  auto firstNotLess = std::lower_bound(
      itsVectorList.begin(), itsVectorList.end(), &theTime, ComparePtrs<NFmiMetTime>());
  if (firstNotLess != itsVectorList.end() && *(*firstNotLess) == theTime)
  {
    // Searched time was found from time-vector
    itsIndex = CalcTimeListIndex(firstNotLess);
    return true;
  }

  if (theDirection == kBackward)
    return FindNearestBackwardTime(firstNotLess, theTime, theTimeRangeInMinutes);
  else if (theDirection == kForward)
    return FindNearestForwardTime(firstNotLess, theTime, theTimeRangeInMinutes);
  else
    return FindNearestTime(firstNotLess, theTime, theTimeRangeInMinutes);
}

// Assumption: firstNotLess -iterator is from the itsVectorList.
bool NFmiTimeList::FindNearestBackwardTime(checkedVector<NFmiMetTime *>::iterator &firstNotLess,
                                           const NFmiMetTime &theTime,
                                           unsigned long theTimeRangeInMinutes)
{
  if (firstNotLess == itsVectorList.begin())
    return false;  // All times in itsVectorList were bigger than theTime
  else
  {
    firstNotLess--;  // Lets move to previous time which is what we are searching here (parameter's
                     // descriptive name false after this)
    return CheckFoundTimeIter(firstNotLess, theTime, theTimeRangeInMinutes);
  }
}

// Assumption: firstNotLess -iterator is from the itsVectorList.
bool NFmiTimeList::FindNearestForwardTime(checkedVector<NFmiMetTime *>::iterator &firstNotLess,
                                          const NFmiMetTime &theTime,
                                          unsigned long theTimeRangeInMinutes)
{
  if (firstNotLess == itsVectorList.end())
    return false;  // All times in itsVectorList were less than theTime
  else
  {
    return CheckFoundTimeIter(firstNotLess, theTime, theTimeRangeInMinutes);
  }
}

// Assumption: firstNotLess -iterator is from the itsVectorList.
bool NFmiTimeList::FindNearestTime(checkedVector<NFmiMetTime *>::iterator &firstNotLess,
                                   const NFmiMetTime &theTime,
                                   unsigned long theTimeRangeInMinutes)
{
  if (firstNotLess == itsVectorList.begin())
  {
    // Only list's first time is possible
    return CheckFoundTimeIter(firstNotLess, theTime, theTimeRangeInMinutes);
  }
  else if (firstNotLess == itsVectorList.end())
  {
    // Only list's last time is possible
    firstNotLess--;
    return CheckFoundTimeIter(firstNotLess, theTime, theTimeRangeInMinutes);
  }
  else
  {
    // Must check the first not-less time and the previous time
    auto timeIter2 = firstNotLess;
    double diff2 = std::fabs(theTime.DifferenceInMinutes(*(*timeIter2)));
    firstNotLess--;
    auto timeIter1 = firstNotLess;
    double diff1 = std::fabs(theTime.DifferenceInMinutes(*(*timeIter1)));
    // first time in the list has precedence if difference is equal
    if (diff1 <= diff2)
      return CheckFoundTimeIter(timeIter1, theTime, theTimeRangeInMinutes);
    else
      return CheckFoundTimeIter(timeIter2, theTime, theTimeRangeInMinutes);
  }
}

// Assumption: theIter -iterator is from the itsVectorList.
int NFmiTimeList::CalcTimeListIndex(const checkedVector<NFmiMetTime *>::iterator &theIter)
{
  return static_cast<int>(std::distance(itsVectorList.begin(), theIter));
}

bool NFmiTimeList::IsSearchedTimeInRange(checkedVector<NFmiMetTime *>::iterator &foundTimeIter,
                                         const NFmiMetTime &theTime,
                                         unsigned long theTimeRangeInMinutes)
{
  if (theTimeRangeInMinutes == kUnsignedLongMissing)
    return true;
  else if (theTimeRangeInMinutes >= std::fabs(theTime.DifferenceInMinutes(*(*foundTimeIter))))
    return true;
  else
    return false;
}

bool NFmiTimeList::CheckFoundTimeIter(checkedVector<NFmiMetTime *>::iterator &foundTimeIter,
                                      const NFmiMetTime &theTime,
                                      unsigned long theTimeRangeInMinutes)
{
  if (IsSearchedTimeInRange(foundTimeIter, theTime, theTimeRangeInMinutes))
  {
    itsIndex = CalcTimeListIndex(foundTimeIter);
    return true;
  }
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \param theTimeRangeInMinutes Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// apu funktio nearestTime:en
// onko annettu aika tietyn rajan sisällä currentista ajasta?

bool NFmiTimeList::TimeInSearchRange(const NFmiMetTime &theTime,
                                     unsigned long theTimeRangeInMinutes)
{
  if (theTimeRangeInMinutes == kUnsignedLongMissing ||
      static_cast<unsigned long>(::abs(Current()->DifferenceInMinutes(theTime))) <
          theTimeRangeInMinutes)
    return true;
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theList This-otus ja theList yhdistetään halutulla tavalla.
 * \param theStartTimeFunction Mistä otetaan alkuaika, jos 0, pienempi,
 *				jos 1 otetaan this:istä ja jos 2 otetaan theBag:ista.
 * \param theEndTimeFunction Mistä otetaan loppuaika, jos 0, suurempi,
 *				jos 1 otetaan this:istä ja jos 2 otetaan theBag:ista.
 * \return Palautetaan yhdistetty timelist.
 */
// ----------------------------------------------------------------------

const NFmiTimeList NFmiTimeList::Combine(NFmiTimeList &theList,
                                         int theStartTimeFunction,
                                         int theEndTimeFunction)
{
  NFmiTimeList combinedList(*this);
  for (theList.Reset(); theList.Next();)
  {
    auto *tempTime = new NFmiMetTime(*theList.Current());
    combinedList.Add(tempTime, false, false);  // 1. false ei salli duplikaatteja, 2. false etsii
                                               // ajan paikan olemassa olevasta listasta
  }
  if (theStartTimeFunction == 0 && theEndTimeFunction == 0) return combinedList;

  // pitää mahdollisesti karsia aikoja, en optimoinut ollenkaan
  NFmiMetTime startTime;
  if (theStartTimeFunction == 0)
    startTime = FirstTime() < theList.FirstTime() ? FirstTime() : theList.FirstTime();
  else if (theStartTimeFunction == 1)
    startTime = FirstTime();
  else  // tässä pitäisi olla 2, mutta en jaksa tarkistaa, ettei tarvitse tehdä virhe käsittelyä
    startTime = theList.FirstTime();

  NFmiMetTime endTime;
  if (theEndTimeFunction == 0)
    endTime = LastTime() > theList.LastTime() ? LastTime() : theList.LastTime();
  else if (theEndTimeFunction == 1)
    endTime = LastTime();
  else  // tässä pitäisi olla 2, mutta en jaksa tarkistaa, ettei tarvitse tehdä virhe käsittelyä
    endTime = theList.LastTime();

  return combinedList.GetIntersection(startTime, endTime);
}

// ======================================================================

const NFmiMetTime &NFmiTimeList::FirstTime() const
{
  static NFmiMetTime dummy;
  if (IndexOk(0)) return *itsVectorList[0];
  return dummy;
}

const NFmiMetTime &NFmiTimeList::LastTime() const
{
  static NFmiMetTime dummy;
  int index = NumberOfItems() - 1;
  if (IndexOk(index)) return *itsVectorList[index];
  return dummy;
}

int NFmiTimeList::CurrentResolution() const
{
  if (itsIndex > 0 && itsIndex < static_cast<int>(itsVectorList.size()))
    return itsVectorList[itsIndex]->DifferenceInMinutes(*itsVectorList[itsIndex - 1]);
  else if (itsIndex == 0 && itsVectorList.size() > 1)
    return itsVectorList[itsIndex + 1]->DifferenceInMinutes(*itsVectorList[itsIndex]);
  // HUOM! loppu osa koodia on poikeus tapausten tarkastelua ja pitäisi heittää esim. poikkeus.
  // Mutta laitoin koodin toimimaan kuten timebag:in Resolution-metodi toimisi
  // (palauttaa aina arvon), ettei tule ongelmia erilaisissa koodeissa nyt kun
  // käytetään enemmän timelist-dataa.
  else if (itsVectorList.size() >
           1)  // jos esim reset-tilassa, palauttaa 1. ja 2. ajan välisen resoluution
    return itsVectorList[1]->DifferenceInMinutes(*itsVectorList[0]);
  else
    return 60;  // jos listassa on vain yksi tai nolla aikaa palautetaan 60 minuuttia
}

// etsii annettuun aikaan lähimmän edellisen ja lähimmän seuraavan ajan ja palauttaa ne
// parametreina.
// Palauttaa ensimmäisen ajan indeksin jos löytyi. Oletus on että seuraavan ajan indeksi on
// palautettu arvo + 1.
// Jos homma ei onnistunut, palauttaa -1.
int NFmiTimeList::FindNearestTimes(const NFmiMetTime &theTime,
                                   int theMaxMinuteRange,
                                   NFmiMetTime &theTime1,
                                   NFmiMetTime &theTime2)
{
  int oldIndex = itsIndex;
  int returnIndex = -1;
  if (FindNearestTime(theTime, kBackward, theMaxMinuteRange))
  {
    if (itsIndex >= 0 && itsIndex < static_cast<int>(itsVectorList.size()) - 1)
    {
      returnIndex = itsIndex;
      theTime1 = *itsVectorList[itsIndex];
      theTime2 = *itsVectorList[itsIndex + 1];
    }
  }
  itsIndex = oldIndex;  // palautetaan indeksi osoittamaan varmuuden vuoksi takaisin
  return returnIndex;
}

const NFmiMetTime *NFmiTimeList::Time(int theIndex) const
{
  if (IndexOk(theIndex)) return itsVectorList[theIndex];
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * Laskee this timelistasta leikkauksen siten, että otetaan this-listasta
 * ne ajat jotka ovat annettujen rajojen sisältä/rajalta ja laitetaan ne
 * uuteen timelistiin joka palautetaan ulos.
 * Jos rajat ovat ohi kokonaan timelististä ja vielä samaan suuntaan,
 * palautetaan tyhjä timebagi. Samoin käy jos rajojen sisään ei sovi yhtään
 * timelistin aikaa.
 * \param theStartLimit Leikkausta rajoittava alkuaika.
 * \param theEndLimit Leikkausta rajoittava loppuaika.
 * \return Palauttaa tulos timelistin.
 */
// ----------------------------------------------------------------------

const NFmiTimeList NFmiTimeList::GetIntersection(const NFmiMetTime &theStartLimit,
                                                 const NFmiMetTime &theEndLimit)
{
  NFmiTimeList returnTimeList;
  NFmiMetTime tmpTime;
  for (Reset(); Next();)
  {
    tmpTime = *Current();
    if (tmpTime >= theStartLimit && tmpTime <= theEndLimit)
      returnTimeList.Add(new NFmiMetTime(tmpTime));
  }
  return returnTimeList;
}

// ----------------------------------------------------------------------
/*!
 * Tarkistaa onko annettu aika aikalistan sisällä. Oletus: aika-
 * listassa olevat ajat ovat järjestyksessä
 * \param theTime Tarkistettava aika.
 * \return Palauttaa true jos theTime on listan sisällä tai reunalla, muuten false.
 */
// ----------------------------------------------------------------------
bool NFmiTimeList::IsInside(const NFmiMetTime &theTime) const
{
  int timeCount = NumberOfItems();
  if (timeCount < 1)
    return false;
  else if (timeCount == 1)
    return *itsVectorList[0] == theTime;
  else
    return *itsVectorList[0] <= theTime && theTime <= *itsVectorList[timeCount - 1];
}

// ======================================================================

// template<typename T>
struct PointerDestroyer
{
  template <typename T>
  void operator()(T *thePtr)
  {
    delete thePtr;
  }
};

// ----------------------------------------------------------------------
/*!
 * Karsii halutut ylimääräiset ajat pois
 * \param theMaxTimeCount Kertoo kuinka monta aikaa saa olla maksimissaan.
 * \param fFromEnd kertoo, mistä päästä karsitaan ylimääräiset pois.
 * \todo EN OLE SITTEN TESTANNUT T:Marko
 */
// ----------------------------------------------------------------------
void NFmiTimeList::PruneTimes(int theMaxTimeCount, bool fFromEnd)
{
  if (NumberOfItems() > theMaxTimeCount)
  {
    if (fFromEnd)
    {
      checkedVector<NFmiMetTime *> tmpList(theMaxTimeCount);
      std::copy(itsVectorList.begin(), itsVectorList.begin() + theMaxTimeCount, tmpList.begin());
      std::for_each(
          itsVectorList.begin() + theMaxTimeCount, itsVectorList.end(), PointerDestroyer());
      itsVectorList.swap(tmpList);
    }
    else
    {
      checkedVector<NFmiMetTime *> tmpList(theMaxTimeCount);
      std::copy(itsVectorList.end() - theMaxTimeCount, itsVectorList.end(), tmpList.begin());
      std::for_each(
          itsVectorList.begin(), itsVectorList.end() - theMaxTimeCount, PointerDestroyer());
      itsVectorList.swap(tmpList);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * Muuttaa timelistin aikoja niin että annettu aika laitetaan ensimmäiseksi,
 * ja kaikille muille arvoille lasketaan siirtymä, että ajat ovat toisiinsa nähden
 * yhtä etäällä kuin aiemminkin. Timebagin rakenne ja 'resoluutio' säilyvät.
 * Lisäksi timelisti laitetaan osoittamaan 1. aikaa.
 * \param theTime on timebagin uusi firstTime.
 */
// ----------------------------------------------------------------------
void NFmiTimeList::SetNewStartTime(const NFmiMetTime &theTime)
{
  if (NumberOfItems() > 0)
  {
    long diffInMinutes = theTime.DifferenceInMinutes(*itsVectorList[0]);
    for (size_t i = 0; i < itsVectorList.size(); i++)
      itsVectorList[i]->ChangeByMinutes(diffInMinutes);
  }
}
