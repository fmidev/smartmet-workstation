#include <regression/tframe.h>
#include "UnitFactory.h"
#include "DictionaryFactory.h"
#include "PlainTextFormatter.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace UnitFactoryTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test UnitFactory::create(DegreesCelsius)
 */
// ----------------------------------------------------------------------

void celsius()
{
  using namespace TextGen;
  using namespace UnitFactory;

  shared_ptr<Dictionary> finnish(DictionaryFactory::create("mysql"));
  finnish->init("fi");

  PlainTextFormatter formatter;
  formatter.dictionary(finnish);

  shared_ptr<Sentence> unit;

  Settings::set("textgen::units::celsius::format", "SI");
  unit = create(DegreesCelsius);
  string celsius = string("\xc2\xb0") + "C.";
  if (unit->realize(formatter) != celsius) TEST_FAILED("create() failed to realize " + celsius);

  Settings::set("textgen::units::celsius::format", "phrase");
  unit = create(DegreesCelsius);
  if (unit->realize(formatter) != "Astetta.") TEST_FAILED("create() failed to realize Astetta.");

  Settings::set("textgen::units::celsius::format", "none");
  unit = create(DegreesCelsius);
  if (unit->realize(formatter) != "") TEST_FAILED("create() failed to realize empty celsius.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test UnitFactory::create(MetersPerSecond)
 */
// ----------------------------------------------------------------------

void meterspersecond()
{
  using namespace TextGen;
  using namespace UnitFactory;

  shared_ptr<Dictionary> finnish(DictionaryFactory::create("mysql"));
  finnish->init("fi");

  PlainTextFormatter formatter;
  formatter.dictionary(finnish);

  shared_ptr<Sentence> unit;

  Settings::set("textgen::units::meterspersecond::format", "SI");
  unit = create(MetersPerSecond);
  if (unit->realize(formatter) != "M/s.") TEST_FAILED("create() failed to realize M/s.");

  Settings::set("textgen::units::meterspersecond::format", "phrase");
  unit = create(MetersPerSecond);
  if (unit->realize(formatter) != "Metriä sekunnissa.")
    TEST_FAILED("create() failed to realize Metriä sekunnissa.");

  Settings::set("textgen::units::meterspersecond::format", "none");
  unit = create(MetersPerSecond);
  if (unit->realize(formatter) != "")
    TEST_FAILED("create() failed to realize empty meterspersecond.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test UnitFactory::create(Millimeters)
 */
// ----------------------------------------------------------------------

void millimeters()
{
  using namespace TextGen;
  using namespace UnitFactory;

  shared_ptr<Dictionary> finnish(DictionaryFactory::create("mysql"));
  finnish->init("fi");

  PlainTextFormatter formatter;
  formatter.dictionary(finnish);

  shared_ptr<Sentence> unit;

  Settings::set("textgen::units::millimeters::format", "SI");
  unit = create(Millimeters);
  if (unit->realize(formatter) != "Mm.") TEST_FAILED("create() failed to realize Mm.");

  Settings::set("textgen::units::millimeters::format", "phrase");
  unit = create(Millimeters);
  if (unit->realize(formatter) != "Millimetriä.")
    TEST_FAILED("create() failed to realize Millimetriä.");

  Settings::set("textgen::units::millimeters::format", "none");
  unit = create(Millimeters);
  if (unit->realize(formatter) != "") TEST_FAILED("create() failed to realize empty millimeters.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test UnitFactory::create(Percent)
 */
// ----------------------------------------------------------------------

void percent()
{
  using namespace TextGen;
  using namespace UnitFactory;

  shared_ptr<Dictionary> finnish(DictionaryFactory::create("mysql"));
  finnish->init("fi");

  PlainTextFormatter formatter;
  formatter.dictionary(finnish);

  shared_ptr<Sentence> unit;

  Settings::set("textgen::units::percent::format", "SI");
  unit = create(Percent);
  if (unit->realize(formatter) != "%.") TEST_FAILED("create() failed to realize %.");

  Settings::set("textgen::units::percent::format", "phrase");
  unit = create(Percent);
  if (unit->realize(formatter) != "Prosenttia.")
    TEST_FAILED("create() failed to realize Prosenttia.");

  Settings::set("textgen::units::percent::format", "none");
  unit = create(Percent);
  if (unit->realize(formatter) != "") TEST_FAILED("create() failed to realize empty percent.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test UnitFactory::create(HectoPascal)
 */
// ----------------------------------------------------------------------

void hectopascal()
{
  using namespace TextGen;
  using namespace UnitFactory;

  shared_ptr<Dictionary> finnish(DictionaryFactory::create("mysql"));
  finnish->init("fi");

  PlainTextFormatter formatter;
  formatter.dictionary(finnish);

  shared_ptr<Sentence> unit;

  Settings::set("textgen::units::hectopascal::format", "SI");
  unit = create(HectoPascal);
  if (unit->realize(formatter) != "Hpa.")
    TEST_FAILED("create() failed to realize Hpa, got: " + unit->realize(formatter));

  Settings::set("textgen::units::hectopascal::format", "phrase");
  unit = create(HectoPascal);
  if (unit->realize(formatter) != "Hehtopascalia.")
    TEST_FAILED("create() failed to realize hehtopascal, got :" + unit->realize(formatter));

  Settings::set("textgen::units::hectopascal::format", "none");
  unit = create(HectoPascal);
  if (unit->realize(formatter) != "")
    TEST_FAILED("create() failed to realize empty hehtopascal, got: " + unit->realize(formatter));

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
    TEST(celsius);
    TEST(meterspersecond);
    TEST(millimeters);
    TEST(percent);
    TEST(hectopascal);
  }

};  // class tests

}  // namespace UnitFactoryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  cout << endl << "UnitFactory tester" << endl << "==================" << endl;
  UnitFactoryTest::tests t;
  return t.run();
}
