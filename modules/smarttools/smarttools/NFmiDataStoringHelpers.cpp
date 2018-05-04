//© Ilmatieteenlaitos/Marko.
// Original 4.11.2011
//
// namespacessa on muutamia helper funktioita ja 'ylimääräisen' datan talletus luokka.
//---------------------------------------------------------- NFmiDataStoringHelpers.cpp

#include "NFmiDataStoringHelpers.h"
#include <newbase/NFmiStringTools.h>

namespace
{
NFmiMetTime itsViewMacroTime =
    NFmiMetTime(1900, 0, 0, 0, 0, 0);  // NFmiMetTime::gMissingTime; EI TOIMI!!!
// Ei toiminut gMissingTime-arvon käyttö ainakaan VC++2008 kääntäjällä.
// itsViewMacroTime sai arvokseen 0 v 0 kk 0 pv 0 h 0 min 0 sec
// Ilmeisesti anonymous namespace alustetaan ensin ja sitten vasta NFmiMetTime-luokan staattinen
// const dataosa.
}

void NFmiDataStoringHelpers::WriteTimeWithOffsets(const NFmiMetTime &theUsedCurrentTime,
                                                  const NFmiMetTime &theTime,
                                                  std::ostream &os)
{
  short utcHour = theTime.GetHour();
  short utcMinute = theTime.GetMin();
  NFmiMetTime aTime(theUsedCurrentTime);
  aTime.SetHour(0);
  aTime.SetMin(0);
  aTime.SetSec(0);
  long hourShift = aTime.DifferenceInHours(theTime);
  long usedDayShift = hourShift / 24;
  if (hourShift > 0)
    usedDayShift++;
  os << utcHour << " " << utcMinute << " " << usedDayShift << std::endl;
}

void NFmiDataStoringHelpers::ReadTimeWithOffsets(const NFmiMetTime &theUsedCurrentTime,
                                                 NFmiMetTime &theTime,
                                                 std::istream &is)
{
  short utcHour = 0;
  short utcMinute = 0;
  long dayShift = 0;
  is >> utcHour >> utcMinute >> dayShift;

  NFmiMetTime aTime(theUsedCurrentTime);
  aTime.SetHour(0);
  aTime.SetMin(0);
  aTime.SetSec(0);
  aTime.ChangeByDays(-dayShift);
  aTime.SetHour(utcHour);
  aTime.SetMin(utcMinute);
  // alkuperäisessä tallennuskoodissa on ollut bugi, kun ei olla nollattu minuutteja eikä
  // sekunteja ennen dayshift-laskuja.
  // En voi täysin korjata tätä, koska se saattaa sekoittaa pakkaa enemmän kuin tarpeen.
  // Siksi jos utc-tunti oli 0 ja dayShift oli positiivinen, pitää lopullista aikaa siirtää päivällä
  // eteenpäin
  bool uglyAfterFix = ((utcHour == 0) && (dayShift > 0));
  if (uglyAfterFix)
    aTime.ChangeByDays(1);
  theTime = aTime;
}

void NFmiDataStoringHelpers::WriteTimeBagWithOffSets(const NFmiMetTime &theUsedCurrentTime,
                                                     const NFmiTimeBag &theTimeBag,
                                                     std::ostream &os)
{
  NFmiDataStoringHelpers::WriteTimeWithOffsets(theUsedCurrentTime, theTimeBag.FirstTime(), os);
  os << " ";
  NFmiDataStoringHelpers::WriteTimeWithOffsets(theUsedCurrentTime, theTimeBag.LastTime(), os);
  os << " ";
  os << static_cast<long>(theTimeBag.Resolution()) << std::endl;
}

void NFmiDataStoringHelpers::ReadTimeBagWithOffSets(const NFmiMetTime &theUsedCurrentTime,
                                                    NFmiTimeBag &theTimeBag,
                                                    std::istream &is)
{
  NFmiMetTime firstTime;
  NFmiDataStoringHelpers::ReadTimeWithOffsets(theUsedCurrentTime, firstTime, is);
  NFmiMetTime lastTime;
  NFmiDataStoringHelpers::ReadTimeWithOffsets(theUsedCurrentTime, lastTime, is);
  long period = 60;
  is >> period;

  theTimeBag = NFmiTimeBag(firstTime, lastTime, period);
}

std::string NFmiDataStoringHelpers::GetTimeBagOffSetStr(const NFmiMetTime &theUsedCurrentTime,
                                                        const NFmiTimeBag &theTimeBag)
{
  std::stringstream out;
  NFmiDataStoringHelpers::WriteTimeWithOffsets(theUsedCurrentTime, theTimeBag.FirstTime(), out);
  out << " ";
  NFmiDataStoringHelpers::WriteTimeWithOffsets(theUsedCurrentTime, theTimeBag.LastTime(), out);
  out << " ";
  out << static_cast<long>(theTimeBag.Resolution());
  return out.str();
}

NFmiTimeBag NFmiDataStoringHelpers::GetTimeBagOffSetFromStr(const NFmiMetTime &theUsedCurrentTime,
                                                            const std::string &theTimeBagStr)
{
  std::stringstream in(theTimeBagStr);
  NFmiMetTime firstTime;
  NFmiDataStoringHelpers::ReadTimeWithOffsets(theUsedCurrentTime, firstTime, in);
  NFmiMetTime lastTime;
  NFmiDataStoringHelpers::ReadTimeWithOffsets(theUsedCurrentTime, lastTime, in);
  long period = 60;
  in >> period;

  return NFmiTimeBag(firstTime, lastTime, period);
}

void NFmiDataStoringHelpers::SetUsedViewMacroTime(const NFmiMetTime &theTime)
{
  itsViewMacroTime = theTime;
}

NFmiMetTime NFmiDataStoringHelpers::GetUsedViewMacroTime(void)
{
  if (itsViewMacroTime == NFmiMetTime::gMissingTime)
    return NFmiMetTime(1);
  else
    return itsViewMacroTime;
}

void NFmiDataStoringHelpers::NFmiExtraDataStorage::Clear(void)
{
  itsDoubleValues.clear();
  itsStringValues.clear();
}

void NFmiDataStoringHelpers::NFmiExtraDataStorage::Add(double theValue)
{
  itsDoubleValues.push_back(theValue);
}

void NFmiDataStoringHelpers::NFmiExtraDataStorage::Add(const std::string &theValue)
{
  itsStringValues.push_back(theValue);
}

void NFmiDataStoringHelpers::NFmiExtraDataStorage::Write(std::ostream &os) const
{
  size_t ssize = itsDoubleValues.size();
  os << ssize << std::endl;
  size_t i = 0;
  for (i = 0; i < ssize; i++)
  {
    if (i > 0)  // tämän avulla viimeisen arvon jälkeen ei tule spacea
      os << " ";
    os << itsDoubleValues[i];
  }
  if (ssize > 0)
    os << std::endl;

  ssize = itsStringValues.size();
  os << ssize << std::endl;
  for (i = 0; i < ssize; i++)
  {
    // muutetaan std::string NFmiString:iksi että saadaan stringin pituus mukaan kirjoitukseen
    // luku vaiheessa muuten hömma tökkää ensimmäiseen white spaceen
    NFmiString tmpStr(itsStringValues[i]);
    os << tmpStr;  // NFmiString heittää itse endl:in perään.
  }
  if (ssize > 0)
    os << std::endl;
}

void NFmiDataStoringHelpers::NFmiExtraDataStorage::Read(std::istream &is)
{
  static size_t maxAllowedVectorSize = 200;
  Clear();

  size_t ssize = 0;
  is >> ssize;
  if (ssize > maxAllowedVectorSize)
    throw std::runtime_error(std::string("NFmiExtraDataStorage::Read double vector size is over ") +
                             NFmiStringTools::Convert<int>(200) +
                             ", propably error failing read...");
  itsDoubleValues.resize(ssize);
  size_t i = 0;
  for (i = 0; i < ssize; i++)
  {
    is >> itsDoubleValues[i];
  }

  is >> ssize;
  if (ssize > maxAllowedVectorSize)
    throw std::runtime_error(std::string("NFmiExtraDataStorage::Read string vector size is over ") +
                             NFmiStringTools::Convert<int>(200) +
                             ", propably error failing read...");
  itsStringValues.resize(ssize);
  for (i = 0; i < ssize; i++)
  {
    // Luetaan NFmiString otus sisään ja muutetaan se std::string:iksi
    // että saadaan stringin pituus mukaan kirjoitukseen
    // luku vaiheessa muuten hömma tökkää ensimmäiseen white spaceen
    NFmiString tmpStr;
    is >> tmpStr;
    itsStringValues[i] = tmpStr;
  }
}
