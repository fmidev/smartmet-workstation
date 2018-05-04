#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Phrase.h"
#include <calculator/Settings.h>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace HeaderPhrasesTest
{
shared_ptr<TextGen::Dictionary> dict;

void require(const string& theKey, const string& theValue)
{
  using namespace TextGen;
  Phrase p(theKey);
  const string result = p.realize(*dict);
  if (result != theValue)
  {
    const string msg = ("'" + theKey + "'" + " = '" + p.realize(*dict) + "' is wrong, should be '" +
                        theValue + "'");
    TEST_FAILED(msg.c_str());
  }
}

void require(const string& theKey) { require(theKey, theKey); }
// ----------------------------------------------------------------------
/*!
 * \brief HeaderFactory::header_until() requirements
 */
// ----------------------------------------------------------------------

void header_until()
{
  require("odotettavissa");

  require("1-aamuun", "maanantaiaamuun");
  require("2-aamuun", "tiistaiaamuun");
  require("3-aamuun", "keskiviikkoaamuun");
  require("4-aamuun", "torstaiaamuun");
  require("5-aamuun", "perjantaiaamuun");
  require("6-aamuun", "lauantaiaamuun");
  require("7-aamuun", "sunnuntaiaamuun");

  require("1-iltaan", "maanantai-iltaan");
  require("2-iltaan", "tiistai-iltaan");
  require("3-iltaan", "keskiviikkoiltaan");
  require("4-iltaan", "torstai-iltaan");
  require("5-iltaan", "perjantai-iltaan");
  require("6-iltaan", "lauantai-iltaan");
  require("7-iltaan", "sunnuntai-iltaan");

  require("asti");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief HeaderFactory::header_from_until() requirements
 */
// ----------------------------------------------------------------------

void header_from_until()
{
  require("odotettavissa");

  require("1-aamusta", "maanantaiaamusta");
  require("2-aamusta", "tiistaiaamusta");
  require("3-aamusta", "keskiviikkoaamusta");
  require("4-aamusta", "torstaiaamusta");
  require("5-aamusta", "perjantaiaamusta");
  require("6-aamusta", "lauantaiaamusta");
  require("7-aamusta", "sunnuntaiaamusta");

  require("1-illasta", "maanantai-illasta");
  require("2-illasta", "tiistai-illasta");
  require("3-illasta", "keskiviikkoillasta");
  require("4-illasta", "torstai-illasta");
  require("5-illasta", "perjantai-illasta");
  require("6-illasta", "lauantai-illasta");
  require("7-illasta", "sunnuntai-illasta");

  require("1-aamuun", "maanantaiaamuun");
  require("2-aamuun", "tiistaiaamuun");
  require("3-aamuun", "keskiviikkoaamuun");
  require("4-aamuun", "torstaiaamuun");
  require("5-aamuun", "perjantaiaamuun");
  require("6-aamuun", "lauantaiaamuun");
  require("7-aamuun", "sunnuntaiaamuun");

  require("1-iltaan", "maanantai-iltaan");
  require("2-iltaan", "tiistai-iltaan");
  require("3-iltaan", "keskiviikkoiltaan");
  require("4-iltaan", "torstai-iltaan");
  require("5-iltaan", "perjantai-iltaan");
  require("6-iltaan", "lauantai-iltaan");
  require("7-iltaan", "sunnuntai-iltaan");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief HeaderFactory::header_several_days() requirements
 */
// ----------------------------------------------------------------------

void header_several_days()
{
  require("1-aamusta", "maanantaiaamusta");
  require("2-aamusta", "tiistaiaamusta");
  require("3-aamusta", "keskiviikkoaamusta");
  require("4-aamusta", "torstaiaamusta");
  require("5-aamusta", "perjantaiaamusta");
  require("6-aamusta", "lauantaiaamusta");
  require("7-aamusta", "sunnuntaiaamusta");

  require("1-illasta", "maanantai-illasta");
  require("2-illasta", "tiistai-illasta");
  require("3-illasta", "keskiviikkoillasta");
  require("4-illasta", "torstai-illasta");
  require("5-illasta", "perjantai-illasta");
  require("6-illasta", "lauantai-illasta");
  require("7-illasta", "sunnuntai-illasta");

  require("alkavan");

  require("2-vuorokauden saa", "kahden vuorokauden sää");
  require("3-vuorokauden saa", "kolmen vuorokauden sää");
  require("4-vuorokauden saa", "neljän vuorokauden sää");
  require("5-vuorokauden saa", "viiden vuorokauden sää");
  require("6-vuorokauden saa", "kuuden vuorokauden sää");
  require("7-vuorokauden saa", "seitsemän vuorokauden sää");
  require("8-vuorokauden saa", "kahdeksan vuorokauden sää");
  require("9-vuorokauden saa", "yhdeksän vuorokauden sää");
  require("10-vuorokauden saa", "kymmenen vuorokauden sää");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief HeaderFactory::header_report_area() requirements
 */
// ----------------------------------------------------------------------

void header_report_area()
{
  require("kello");
  require("o'clock", "");

  require("1-na", "maanantaina");
  require("2-na", "tiistaina");
  require("3-na", "keskiviikkona");
  require("4-na", "torstaina");
  require("5-na", "perjantaina");
  require("6-na", "lauantaina");
  require("7-na", "sunnuntaina");

  require("aland:lle", "Ahvenanmaalle");
  require("etela-karjala:lle", "Etelä-Karjalaan");
  require("etela-pohjanmaa:lle", "Etelä-Pohjanmaalle");
  require("etela-savo:lle", "Etelä-Savoon");
  require("hame:lle", "Hämeeseen");
  require("kainuu:lle", "Kainuuseen");
  require("keski-pohjanmaa:lle", "Keski-Pohjanmaalle");
  require("keski-suomi:lle", "Keski-Suomeen");
  require("koillismaa:lle", "Koillismaalle");
  require("kymenlaakso:lle", "Kymenlaaksoon");
  require("lansi-lappi:lle", "Länsi-Lappiin");
  require("nyland:lle", "Uudellemaalle");
  require("osterbotten:lle", "Pohjanmaalle");
  require("paijat-hame:lle", "Päijät-Hämeeseen");
  require("pirkanmaa:lle", "Pirkanmaalle");
  require("pohjois-karjala:lle", "Pohjois-Karjalaan");
  require("pohjois-lappi:lle", "Pohjois-Lappiin");
  require("pohjois-pohjanmaa:lle", "Pohjois-Pohjanmaalle");
  require("pohjois-savo:lle", "Pohjois-Savoon");
  require("satakunta:lle", "Satakuntaan");
  require("uusimaa:lle", "Uudellemaalle");
  require("varsinais-suomi:lle", "Varsinais-Suomeen");

  require("tiepiiri-hame:lle", "Hämeen tiepiiriin");
  require("tiepiiri-kaakkois-lappi:lle", "Kaakkois-Lappiin");
  require("tiepiiri-kaakkois-suomi:lle", "Kaakkois-Suomen tiepiiriin");
  require("tiepiiri-kasivarsi:lle", "Käsivarteen");
  require("tiepiiri-keski-lappi:lle", "Keski-Lappiin");
  require("tiepiiri-keski-suomi:lle", "Keski-Suomen tiepiiriin");
  require("tiepiiri-lounais-lappi:lle", "Lounais-Lappiin");
  require("tiepiiri-oulu-ita:lle", "Oulun tiepiirin itäosaan");
  require("tiepiiri-oulu-lansi:lle", "Oulun tiepiirin länsiosaan");
  require("tiepiiri-savo-karjala:lle", "Savo-Karjalan tiepiiriin");
  require("tiepiiri-turku:lle", "Turun tiepiiriin");
  require("tiepiiri-uusimaa:lle", "Uudenmaan tiepiiriin");
  require("tiepiiri-vaasa:lle", "Vaasan tiepiiriin");
  require("tiepiiri-yla-lappi:lle", "Ylä-Lappiin");

  TEST_PASSED();
}

//! The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(header_until);
    TEST(header_from_until);
    TEST(header_several_days);
    TEST(header_report_area);
  }

};  // class tests

}  // namespace HeaderPhrasesTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  cout << endl << "HeaderFactory requirements" << endl << "==========================" << endl;

  HeaderPhrasesTest::dict.reset(TextGen::DictionaryFactory::create("mysql"));
  HeaderPhrasesTest::dict->init("fi");

  HeaderPhrasesTest::tests t;
  return t.run();
}
