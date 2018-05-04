#include <regression/tframe.h>
#include "InlandMaskSource.h"
#include <calculator/UserWeatherSource.h>
#include <calculator/WeatherArea.h>

#include <newbase/NFmiIndexMask.h>
#include <newbase/NFmiStringTools.h>
#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace boost;

namespace InlandMaskSourceTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test RegularMaskSource::mask
 */
// ----------------------------------------------------------------------

void mask()
{
  using namespace TextGen;
  using NFmiStringTools::Convert;
  typedef InlandMaskSource::mask_type mask_type;

  const string datafile = "data/skandinavia_pinta.sqd";

  const WeatherArea rannikko("data/rannikko.svg:15");
  const WeatherArea uusimaa("data/uusimaa.svg");
  const WeatherArea ahvenanmaa("data/ahvenanmaa.svg");
  const WeatherArea ahvenanmaa15("data/ahvenanmaa.svg:15");
  const WeatherArea ahvenanmaa30("data/ahvenanmaa.svg:30");
  const WeatherArea pohjois_lappi("data/pohjois-lappi.svg");

  // Initialize all the data

  shared_ptr<NFmiQueryData> sqd(new NFmiQueryData(datafile));

  UserWeatherSource wsource;
  wsource.insert("data", sqd);

  // The mask source itself

  InlandMaskSource source(rannikko);

  // Begin testing

  // Half of uusimaa (4/8) is coastal
  {
    mask_type m = source.mask(uusimaa, "data", wsource);
    if (m->size() != 4) TEST_FAILED("Size of mask uusimaa should be 4, not " + Convert(m->size()));
  }

  // no points inside, hence no points inland either
  {
    mask_type m = source.mask(ahvenanmaa, "data", wsource);
    if (m->size() != 0)
      TEST_FAILED("Size of mask ahvenanmaa should be 0, not " + Convert(m->size()));
  }

  // expand by 15, hence all 5 points are also coastal, but still no inland
  {
    mask_type m = source.mask(ahvenanmaa15, "data", wsource);
    if (m->size() != 0)
      TEST_FAILED("Size of mask ahvenanmaa:15 should be 0, not " + Convert(m->size()));
  }

  // expand by 30, then we finally get some inland points
  {
    mask_type m = source.mask(ahvenanmaa30, "data", wsource);
    if (m->size() != 4)
      TEST_FAILED("Size of mask ahvenanmaa:30 should be 4, not " + Convert(m->size()));
  }

  // not even near the coast
  {
    mask_type m = source.mask(pohjois_lappi, "data", wsource);
    if (m->size() != 26)
      TEST_FAILED("Size of mask pohjois_lappi should be 26, not " + Convert(m->size()));
  }

  TEST_PASSED();
}

//! The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void) { TEST(mask); }
};  // class tests

}  // namespace InlandMaskSourceTest

int main(void)
{
  cout << endl << "InlandMaskSource tester" << endl << "=======================" << endl;
  InlandMaskSourceTest::tests t;
  return t.run();
}
