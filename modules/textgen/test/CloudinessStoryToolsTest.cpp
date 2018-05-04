#include <regression/tframe.h>
#include "DebugDictionary.h"
#include "CloudinessStoryTools.h"
#include <newbase/NFmiSettings.h>
#include "PlainTextFormatter.h"
#include "Sentence.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace CloudinessStoryToolsTest
{
// ----------------------------------------------------------------------
/*!
 * \brief Test CloudinessStoryTools::cloudiness_type
 */
// ----------------------------------------------------------------------

void cloudiness_type()
{
  // Default values based on documentation:
  //
  // single_class_limit = 90
  // mostly_class_limit = 80
  // no_class_limit     = 20
  // trend_limit        = 80

  using namespace TextGen;
  using namespace TextGen::CloudinessStoryTools;

  // running cloudiness
  if (CloudinessStoryTools::cloudiness_type("", 95, 0, 0) != Cloudy)
    TEST_FAILED("Failed to return Cloudy for 95% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 90, 0, 0) != Cloudy)
    TEST_FAILED("Failed to return Cloudy for 90% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 85, 0, 0) != MostlyCloudy)
    TEST_FAILED("Failed to return MostlyCloudy for 85% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 80, 0, 0) != MostlyCloudy)
    TEST_FAILED("Failed to return MostlyCloudy for 80% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 75, 0, 0) != CloudyOrPartlyCloudy)
    TEST_FAILED("Failed to return CloudyOrPartlyCloudy for 75% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 60, 0, 0) != CloudyOrPartlyCloudy)
    TEST_FAILED("Failed to return CloudyOrPartlyCloudy for 60% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 50, 0, 0) != CloudyOrPartlyCloudy)
    TEST_FAILED("Failed to return CloudyOrPartlyCloudy for 50% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 40, 0, 0) != CloudyOrPartlyCloudy)
    TEST_FAILED("Failed to return CloudyOrPartlyCloudy for 40% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 30, 0, 0) != CloudyOrPartlyCloudy)
    TEST_FAILED("Failed to return CloudyOrPartlyCloudy for 30% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 20, 0, 0) != MostlyPartlyCloudy)
    TEST_FAILED("Failed to return MostlyPartlyCloudy for 20% cloudy, 0% clear");
  if (CloudinessStoryTools::cloudiness_type("", 10, 0, 0) != PartlyCloudy)
    TEST_FAILED("Failed to return PartlyCloudy for 10% cloudy, 0% clear");

  // running clear
  if (CloudinessStoryTools::cloudiness_type("", 0, 95, 0) != Clear)
    TEST_FAILED("Failed to return Clear for 95% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 90, 0) != Clear)
    TEST_FAILED("Failed to return Clear for 90% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 85, 0) != MostlyClear)
    TEST_FAILED("Failed to return MostlyClear for 85% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 80, 0) != MostlyClear)
    TEST_FAILED("Failed to return MostlyClear for 80% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 75, 0) != ClearOrPartlyCloudy)
    TEST_FAILED("Failed to return ClearOrPartlyCloudy for 75% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 60, 0) != ClearOrPartlyCloudy)
    TEST_FAILED("Failed to return ClearOrPartlyCloudy for 60% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 50, 0) != ClearOrPartlyCloudy)
    TEST_FAILED("Failed to return ClearOrPartlyCloudy for 50% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 40, 0) != ClearOrPartlyCloudy)
    TEST_FAILED("Failed to return ClearOrPartlyCloudy for 40% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 30, 0) != ClearOrPartlyCloudy)
    TEST_FAILED("Failed to return ClearOrPartlyCloudy for 30% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 20, 0) != MostlyPartlyCloudy)
    TEST_FAILED("Failed to return MostlyPartlyCloudy for 20% clear, 0% cloudy");
  if (CloudinessStoryTools::cloudiness_type("", 0, 10, 0) != PartlyCloudy)
    TEST_FAILED("Failed to return PartlyCloudy for 10% clear, 0% cloudy");

  // fixed clear, running cloudy

  if (CloudinessStoryTools::cloudiness_type("", 80, 20, 0) != MostlyCloudy)
    TEST_FAILED("Failed to return MostlyCloudy for 80% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 75, 20, 0) != VariableCloudiness)
    TEST_FAILED("Failed to return VariableCloudiness for 75% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 60, 20, 0) != VariableCloudiness)
    TEST_FAILED("Failed to return VariableCloudiness for 60% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 50, 20, 0) != VariableCloudiness)
    TEST_FAILED("Failed to return VariableCloudiness for 50% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 40, 20, 0) != VariableCloudiness)
    TEST_FAILED("Failed to return VariableCloudiness for 40% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 30, 20, 0) != VariableCloudiness)
    TEST_FAILED("Failed to return VariableCloudiness for 30% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 20, 20, 0) != VariableCloudiness)
    TEST_FAILED("Failed to return VariableCloudiness for 20% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 10, 20, 0) != ClearOrPartlyCloudy)
    TEST_FAILED("Failed to return ClearOrPartlyCloudy for 10% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 0, 20, 0) != MostlyPartlyCloudy)
    TEST_FAILED("Failed to return MostlyPartlyCloudy for 0% cloudy, 20% clear");

  // fixed clear, running cloudy, with big trend

  if (CloudinessStoryTools::cloudiness_type("", 80, 20, 90) != MostlyCloudy)
    TEST_FAILED("Failed to return MostlyCloudy for 80% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 70, 20, 90) != IncreasingCloudiness)
    TEST_FAILED("Failed to return IncreasingCloudiness for 70% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 60, 20, 90) != IncreasingCloudiness)
    TEST_FAILED("Failed to return IncreasingCloudiness for 60% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 50, 20, 90) != IncreasingCloudiness)
    TEST_FAILED("Failed to return IncreasingCloudiness for 50% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 40, 20, 90) != IncreasingCloudiness)
    TEST_FAILED("Failed to return IncreasingCloudiness for 40% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 30, 20, 90) != IncreasingCloudiness)
    TEST_FAILED("Failed to return IncreasingCloudiness for 30% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 20, 20, 90) != IncreasingCloudiness)
    TEST_FAILED("Failed to return IncreasingCloudiness for 20% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 20, 20, 90) != IncreasingCloudiness)
    TEST_FAILED("Failed to return IncreasingCloudiness for 10% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 0, 20, 90) != MostlyPartlyCloudy)
    TEST_FAILED("Failed to return MostlyPartlyCloudy for 0% cloudy, 20% clear");

  // fixed clear, running cloudy, with big decreasing trend

  if (CloudinessStoryTools::cloudiness_type("", 80, 20, -90) != MostlyCloudy)
    TEST_FAILED("Failed to return MostlyCloudy for 80% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 70, 20, -90) != DecreasingCloudiness)
    TEST_FAILED("Failed to return DecreasingCloudiness for 70% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 60, 20, -90) != DecreasingCloudiness)
    TEST_FAILED("Failed to return DecreasingCloudiness for 60% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 50, 20, -90) != DecreasingCloudiness)
    TEST_FAILED("Failed to return DecreasingCloudiness for 50% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 40, 20, -90) != DecreasingCloudiness)
    TEST_FAILED("Failed to return DecreasingCloudiness for 40% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 30, 20, -90) != DecreasingCloudiness)
    TEST_FAILED("Failed to return DecreasingCloudiness for 30% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 20, 20, -90) != DecreasingCloudiness)
    TEST_FAILED("Failed to return DecreasingCloudiness for 20% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 20, 20, -90) != DecreasingCloudiness)
    TEST_FAILED("Failed to return DecreasingCloudiness for 10% cloudy, 20% clear");
  if (CloudinessStoryTools::cloudiness_type("", 0, 20, -90) != MostlyPartlyCloudy)
    TEST_FAILED("Failed to return MostlyPartlyCloudy for 0% cloudy, 20% clear");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test CloudinessStoryTools::similar_type
 */
// ----------------------------------------------------------------------

void similar_type()
{
  using namespace TextGen;
  using namespace TextGen::CloudinessStoryTools;

  // these tests hardcode the table in the documentation
  // of the cloudiness_overview story

  if (!similar_type(Cloudy, Cloudy).second) TEST_FAILED("Cloudy is similar to Cloudy");
  if (!similar_type(Cloudy, PartlyCloudy).second) TEST_FAILED("Cloudy is similar to PartlyCloudy");
  if (similar_type(Cloudy, Clear).second) TEST_FAILED("Cloudy is not similar to Clear");
  if (!similar_type(Cloudy, MostlyCloudy).second) TEST_FAILED("Cloudy is similar to MostlyCloudy");
  if (!similar_type(Cloudy, MostlyPartlyCloudy).second)
    TEST_FAILED("Cloudy is similar to MostlyPartlyCloudy");
  if (similar_type(Cloudy, MostlyClear).second) TEST_FAILED("Cloudy is not similar to MostlyClear");
  if (!similar_type(Cloudy, CloudyOrPartlyCloudy).second)
    TEST_FAILED("Cloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(Cloudy, ClearOrPartlyCloudy).second)
    TEST_FAILED("Cloudy is not similar to ClearOrPartlyCloudy");
  if (similar_type(Cloudy, DecreasingCloudiness).second)
    TEST_FAILED("Cloudy is not similar to DecreasingCloudiness");
  if (similar_type(Cloudy, IncreasingCloudiness).second)
    TEST_FAILED("Cloudy is not similar to IncreasingCloudiness");
  if (similar_type(Cloudy, VariableCloudiness).second)
    TEST_FAILED("Cloudy is not similar to VariableCloudiness");

  if (!similar_type(PartlyCloudy, Cloudy).second) TEST_FAILED("PartlyCloudy is similar to Cloudy");
  if (!similar_type(PartlyCloudy, PartlyCloudy).second)
    TEST_FAILED("PartlyCloudy is similar to PartlyCloudy");
  if (!similar_type(PartlyCloudy, Clear).second) TEST_FAILED("PartlyCloudy is similar to Clear");
  if (!similar_type(PartlyCloudy, MostlyCloudy).second)
    TEST_FAILED("PartlyCloudy is similar to MostlyCloudy");
  if (!similar_type(PartlyCloudy, MostlyPartlyCloudy).second)
    TEST_FAILED("PartlyCloudy is similar to MostlyPartlyCloudy");
  if (!similar_type(PartlyCloudy, MostlyClear).second)
    TEST_FAILED("PartlyCloudy is similar to MostlyClear");
  if (!similar_type(PartlyCloudy, CloudyOrPartlyCloudy).second)
    TEST_FAILED("PartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (!similar_type(PartlyCloudy, ClearOrPartlyCloudy).second)
    TEST_FAILED("PartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(PartlyCloudy, DecreasingCloudiness).second)
    TEST_FAILED("PartlyCloudy is not similar to DecreasingCloudiness");
  if (similar_type(PartlyCloudy, IncreasingCloudiness).second)
    TEST_FAILED("PartlyCloudy is not similar to IncreasingCloudiness");
  if (!similar_type(PartlyCloudy, VariableCloudiness).second)
    TEST_FAILED("PartlyCloudy is similar to VariableCloudiness");

  if (similar_type(Clear, Cloudy).second) TEST_FAILED("Clear is not similar to Cloudy");
  if (!similar_type(Clear, PartlyCloudy).second) TEST_FAILED("Clear is similar to PartlyCloudy");
  if (!similar_type(Clear, Clear).second) TEST_FAILED("Clear is similar to Clear");
  if (similar_type(Clear, MostlyCloudy).second) TEST_FAILED("Clear is not similar to MostlyCloudy");
  if (!similar_type(Clear, MostlyPartlyCloudy).second)
    TEST_FAILED("Clear is similar to MostlyPartlyCloudy");
  if (!similar_type(Clear, MostlyClear).second) TEST_FAILED("Clear is similar to MostlyClear");
  if (similar_type(Clear, CloudyOrPartlyCloudy).second)
    TEST_FAILED("Clear is not similar to CloudyOrPartlyCloudy");
  if (!similar_type(Clear, ClearOrPartlyCloudy).second)
    TEST_FAILED("Clear is similar to ClearOrPartlyCloudy");
  if (similar_type(Clear, DecreasingCloudiness).second)
    TEST_FAILED("Clear is not similar to DecreasingCloudiness");
  if (similar_type(Clear, IncreasingCloudiness).second)
    TEST_FAILED("Clear is not similar to IncreasingCloudiness");
  if (similar_type(Clear, VariableCloudiness).second)
    TEST_FAILED("Clear is not similar to VariableCloudiness");

  if (!similar_type(MostlyCloudy, Cloudy).second) TEST_FAILED("MostlyCloudy is similar to Cloudy");
  if (!similar_type(MostlyCloudy, PartlyCloudy).second)
    TEST_FAILED("MostlyCloudy is similar to PartlyCloudy");
  if (similar_type(MostlyCloudy, Clear).second) TEST_FAILED("MostlyCloudy is not similar to Clear");
  if (!similar_type(MostlyCloudy, MostlyCloudy).second)
    TEST_FAILED("MostlyCloudy is similar to MostlyCloudy");
  if (!similar_type(MostlyCloudy, MostlyPartlyCloudy).second)
    TEST_FAILED("MostlyCloudy is similar to MostlyPartlyCloudy");
  if (similar_type(MostlyCloudy, MostlyClear).second)
    TEST_FAILED("MostlyCloudy is not similar to MostlyClear");
  if (!similar_type(MostlyCloudy, CloudyOrPartlyCloudy).second)
    TEST_FAILED("MostlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(MostlyCloudy, ClearOrPartlyCloudy).second)
    TEST_FAILED("MostlyCloudy is not similar to ClearOrPartlyCloudy");
  if (similar_type(MostlyCloudy, DecreasingCloudiness).second)
    TEST_FAILED("MostlyCloudy is not similar to DecreasingCloudiness");
  if (similar_type(MostlyCloudy, IncreasingCloudiness).second)
    TEST_FAILED("MostlyCloudy is not similar to IncreasingCloudiness");
  if (similar_type(MostlyCloudy, VariableCloudiness).second)
    TEST_FAILED("MostlyCloudy is not similar to VariableCloudiness");

  if (!similar_type(MostlyPartlyCloudy, Cloudy).second)
    TEST_FAILED("MostlyPartlyCloudy is similar to Cloudy");
  if (!similar_type(MostlyPartlyCloudy, PartlyCloudy).second)
    TEST_FAILED("MostlyPartlyCloudy is similar to PartlyCloudy");
  if (!similar_type(MostlyPartlyCloudy, Clear).second)
    TEST_FAILED("MostlyPartlyCloudy is similar to Clear");
  if (!similar_type(MostlyPartlyCloudy, MostlyCloudy).second)
    TEST_FAILED("MostlyPartlyCloudy is similar to MostlyCloudy");
  if (!similar_type(MostlyPartlyCloudy, MostlyPartlyCloudy).second)
    TEST_FAILED("MostlyPartlyCloudy is similar to MostlyPartlyCloudy");
  if (!similar_type(MostlyPartlyCloudy, MostlyClear).second)
    TEST_FAILED("MostlyPartlyCloudy is similar to MostlyClear");
  if (!similar_type(MostlyPartlyCloudy, CloudyOrPartlyCloudy).second)
    TEST_FAILED("MostlyPartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (!similar_type(MostlyPartlyCloudy, ClearOrPartlyCloudy).second)
    TEST_FAILED("MostlyPartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(MostlyPartlyCloudy, DecreasingCloudiness).second)
    TEST_FAILED("MostlyPartlyCloudy is not similar to DecreasingCloudiness");
  if (similar_type(MostlyPartlyCloudy, IncreasingCloudiness).second)
    TEST_FAILED("MostlyPartlyCloudy is not similar to IncreasingCloudiness");
  if (!similar_type(MostlyPartlyCloudy, VariableCloudiness).second)
    TEST_FAILED("MostlyPartlyCloudy is similar to VariableCloudiness");

  if (similar_type(MostlyClear, Cloudy).second) TEST_FAILED("MostlyClear is not similar to Cloudy");
  if (!similar_type(MostlyClear, PartlyCloudy).second)
    TEST_FAILED("MostlyClear is similar to PartlyCloudy");
  if (!similar_type(MostlyClear, Clear).second) TEST_FAILED("MostlyClear is similar to Clear");
  if (similar_type(MostlyClear, MostlyCloudy).second)
    TEST_FAILED("MostlyClear is not similar to MostlyCloudy");
  if (!similar_type(MostlyClear, MostlyPartlyCloudy).second)
    TEST_FAILED("MostlyClear is similar to MostlyPartlyCloudy");
  if (!similar_type(MostlyClear, MostlyClear).second)
    TEST_FAILED("MostlyClear is similar to MostlyClear");
  if (similar_type(MostlyClear, CloudyOrPartlyCloudy).second)
    TEST_FAILED("MostlyClear is not similar to CloudyOrPartlyCloudy");
  if (!similar_type(MostlyClear, ClearOrPartlyCloudy).second)
    TEST_FAILED("MostlyClear is similar to ClearOrPartlyCloudy");
  if (similar_type(MostlyClear, DecreasingCloudiness).second)
    TEST_FAILED("MostlyClear is not similar to DecreasingCloudiness");
  if (similar_type(MostlyClear, IncreasingCloudiness).second)
    TEST_FAILED("MostlyClear is not similar to IncreasingCloudiness");
  if (similar_type(MostlyClear, VariableCloudiness).second)
    TEST_FAILED("MostlyClear is not similar to VariableCloudiness");

  if (!similar_type(CloudyOrPartlyCloudy, Cloudy).second)
    TEST_FAILED("CloudyOrPartlyCloudy is similar to Cloudy");
  if (!similar_type(CloudyOrPartlyCloudy, PartlyCloudy).second)
    TEST_FAILED("CloudyOrPartlyCloudy is similar to PartlyCloudy");
  if (similar_type(CloudyOrPartlyCloudy, Clear).second)
    TEST_FAILED("CloudyOrPartlyCloudy is not similar to Clear");
  if (!similar_type(CloudyOrPartlyCloudy, MostlyCloudy).second)
    TEST_FAILED("CloudyOrPartlyCloudy is similar to MostlyCloudy");
  if (!similar_type(CloudyOrPartlyCloudy, MostlyPartlyCloudy).second)
    TEST_FAILED("CloudyOrPartlyCloudy is similar to MostlyPartlyCloudy");
  if (similar_type(CloudyOrPartlyCloudy, MostlyClear).second)
    TEST_FAILED("CloudyOrPartlyCloudy is not similar to MostlyClear");
  if (!similar_type(CloudyOrPartlyCloudy, CloudyOrPartlyCloudy).second)
    TEST_FAILED("CloudyOrPartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (!similar_type(CloudyOrPartlyCloudy, ClearOrPartlyCloudy).second)
    TEST_FAILED("CloudyOrPartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(CloudyOrPartlyCloudy, DecreasingCloudiness).second)
    TEST_FAILED("CloudyOrPartlyCloudy is not similar to DecreasingCloudiness");
  if (similar_type(CloudyOrPartlyCloudy, IncreasingCloudiness).second)
    TEST_FAILED("CloudyOrPartlyCloudy is not similar to IncreasingCloudiness");
  if (!similar_type(CloudyOrPartlyCloudy, VariableCloudiness).second)
    TEST_FAILED("CloudyOrPartlyCloudy is similar to VariableCloudiness");

  if (similar_type(ClearOrPartlyCloudy, Cloudy).second)
    TEST_FAILED("ClearOrPartlyCloudy is not similar to Cloudy");
  if (!similar_type(ClearOrPartlyCloudy, PartlyCloudy).second)
    TEST_FAILED("ClearOrPartlyCloudy is similar to PartlyCloudy");
  if (!similar_type(ClearOrPartlyCloudy, Clear).second)
    TEST_FAILED("ClearOrPartlyCloudy is similar to Clear");
  if (!similar_type(ClearOrPartlyCloudy, MostlyCloudy).second)
    TEST_FAILED("ClearOrPartlyCloudy is similar to MostlyCloudy");
  if (!similar_type(ClearOrPartlyCloudy, MostlyPartlyCloudy).second)
    TEST_FAILED("ClearOrPartlyCloudy is similar to MostlyPartlyCloudy");
  if (!similar_type(ClearOrPartlyCloudy, MostlyClear).second)
    TEST_FAILED("ClearOrPartlyCloudy is similar to MostlyClear");
  if (!similar_type(ClearOrPartlyCloudy, CloudyOrPartlyCloudy).second)
    TEST_FAILED("ClearOrPartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (!similar_type(ClearOrPartlyCloudy, ClearOrPartlyCloudy).second)
    TEST_FAILED("ClearOrPartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(ClearOrPartlyCloudy, DecreasingCloudiness).second)
    TEST_FAILED("ClearOrPartlyCloudy is not similar to DecreasingCloudiness");
  if (similar_type(ClearOrPartlyCloudy, IncreasingCloudiness).second)
    TEST_FAILED("ClearOrPartlyCloudy is not similar to IncreasingCloudiness");
  if (!similar_type(ClearOrPartlyCloudy, VariableCloudiness).second)
    TEST_FAILED("ClearOrPartlyCloudy is similar to VariableCloudiness");

  if (similar_type(DecreasingCloudiness, Cloudy).second)
    TEST_FAILED("DecreasingCloudiness is not similar to Cloudy");
  if (similar_type(DecreasingCloudiness, PartlyCloudy).second)
    TEST_FAILED("DecreasingCloudiness is not similar to PartlyCloudy");
  if (!similar_type(DecreasingCloudiness, Clear).second)
    TEST_FAILED("DecreasingCloudiness is similar to Clear");
  if (similar_type(DecreasingCloudiness, MostlyCloudy).second)
    TEST_FAILED("DecreasingCloudiness is not similar to MostlyCloudy");
  if (similar_type(DecreasingCloudiness, MostlyPartlyCloudy).second)
    TEST_FAILED("DecreasingCloudiness is not similar to MostlyPartlyCloudy");
  if (!similar_type(DecreasingCloudiness, MostlyClear).second)
    TEST_FAILED("DecreasingCloudiness is similar to MostlyClear");
  if (similar_type(DecreasingCloudiness, CloudyOrPartlyCloudy).second)
    TEST_FAILED("DecreasingCloudiness is not similar to CloudyOrPartlyCloudy");
  if (similar_type(DecreasingCloudiness, ClearOrPartlyCloudy).second)
    TEST_FAILED("DecreasingCloudiness is not similar to ClearOrPartlyCloudy");
  if (!similar_type(DecreasingCloudiness, DecreasingCloudiness).second)
    TEST_FAILED("DecreasingCloudiness is similar to DecreasingCloudiness");
  if (similar_type(DecreasingCloudiness, IncreasingCloudiness).second)
    TEST_FAILED("DecreasingCloudiness is not similar to IncreasingCloudiness");
  if (similar_type(DecreasingCloudiness, VariableCloudiness).second)
    TEST_FAILED("DecreasingCloudiness is not similar to VariableCloudiness");

  if (!similar_type(IncreasingCloudiness, Cloudy).second)
    TEST_FAILED("IncreasingCloudiness is similar to Cloudy");
  if (similar_type(IncreasingCloudiness, PartlyCloudy).second)
    TEST_FAILED("IncreasingCloudiness is not similar to PartlyCloudy");
  if (similar_type(IncreasingCloudiness, Clear).second)
    TEST_FAILED("IncreasingCloudiness is not similar to Clear");
  if (!similar_type(IncreasingCloudiness, MostlyCloudy).second)
    TEST_FAILED("IncreasingCloudiness is similar to MostlyCloudy");
  if (similar_type(IncreasingCloudiness, MostlyPartlyCloudy).second)
    TEST_FAILED("IncreasingCloudiness is not similar to MostlyPartlyCloudy");
  if (similar_type(IncreasingCloudiness, MostlyClear).second)
    TEST_FAILED("IncreasingCloudiness is not similar to MostlyClear");
  if (similar_type(IncreasingCloudiness, CloudyOrPartlyCloudy).second)
    TEST_FAILED("IncreasingCloudiness is not similar to CloudyOrPartlyCloudy");
  if (similar_type(IncreasingCloudiness, ClearOrPartlyCloudy).second)
    TEST_FAILED("IncreasingCloudiness is not similar to ClearOrPartlyCloudy");
  if (similar_type(IncreasingCloudiness, DecreasingCloudiness).second)
    TEST_FAILED("IncreasingCloudiness is not similar to DecreasingCloudiness");
  if (!similar_type(IncreasingCloudiness, IncreasingCloudiness).second)
    TEST_FAILED("IncreasingCloudiness is similar to IncreasingCloudiness");
  if (!similar_type(IncreasingCloudiness, VariableCloudiness).second)
    TEST_FAILED("IncreasingCloudiness is similar to VariableCloudiness");

  if (similar_type(VariableCloudiness, Cloudy).second)
    TEST_FAILED("VariableCloudiness is not similar to Cloudy");
  if (!similar_type(VariableCloudiness, PartlyCloudy).second)
    TEST_FAILED("VariableCloudiness is similar to PartlyCloudy");
  if (similar_type(VariableCloudiness, Clear).second)
    TEST_FAILED("VariableCloudiness is not similar to Clear");
  if (!similar_type(VariableCloudiness, MostlyCloudy).second)
    TEST_FAILED("VariableCloudiness is similar to MostlyCloudy");
  if (!similar_type(VariableCloudiness, MostlyPartlyCloudy).second)
    TEST_FAILED("VariableCloudiness is similar to MostlyPartlyCloudy");
  if (similar_type(VariableCloudiness, MostlyClear).second)
    TEST_FAILED("VariableCloudiness is not similar to MostlyClear");
  if (!similar_type(VariableCloudiness, CloudyOrPartlyCloudy).second)
    TEST_FAILED("VariableCloudiness is similar to CloudyOrPartlyCloudy");
  if (!similar_type(VariableCloudiness, ClearOrPartlyCloudy).second)
    TEST_FAILED("VariableCloudiness is similar to ClearOrPartlyCloudy");
  if (similar_type(VariableCloudiness, DecreasingCloudiness).second)
    TEST_FAILED("VariableCloudiness is not similar to DecreasingCloudiness");
  if (!similar_type(VariableCloudiness, IncreasingCloudiness).second)
    TEST_FAILED("VariableCloudiness is similar to IncreasingCloudiness");
  if (!similar_type(VariableCloudiness, VariableCloudiness).second)
    TEST_FAILED("VariableCloudiness is similar to VariableCloudiness");

  // test that the combination is always correct

  if (similar_type(Cloudy, Cloudy).first != Cloudy)
    TEST_FAILED("Cloudy+Cloudy is similar to Cloudy");
  if (similar_type(Cloudy, PartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("Cloudy+PartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(Cloudy, MostlyCloudy).first != MostlyCloudy)
    TEST_FAILED("Cloudy+MostlyCloudy is similar to MostlyCloudy");
  if (similar_type(Cloudy, MostlyPartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("Cloudy+MostlyPartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(Cloudy, CloudyOrPartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("Cloudy+CloudyOrPartlyCloudy is similar to CloudyOrPartlyCloudy");

  if (similar_type(PartlyCloudy, Cloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("PartlyCloudy+Cloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(PartlyCloudy, PartlyCloudy).first != PartlyCloudy)
    TEST_FAILED("PartlyCloudy+PartlyCloudy is similar to PartlyCloudy");
  if (similar_type(PartlyCloudy, Clear).first != ClearOrPartlyCloudy)
    TEST_FAILED("PartlyCloudy+Clear is similar to ClearOrPartlyCloudy");
  if (similar_type(PartlyCloudy, MostlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("PartlyCloudy+MostlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(PartlyCloudy, MostlyPartlyCloudy).first != MostlyPartlyCloudy)
    TEST_FAILED("PartlyCloudy+MostlyPartlyCloudy is similar to MostlyPartlyCloudy");
  if (similar_type(PartlyCloudy, MostlyClear).first != ClearOrPartlyCloudy)
    TEST_FAILED("PartlyCloudy+MostlyClear is similar to ClearOrPartlyCloudy");
  if (similar_type(PartlyCloudy, CloudyOrPartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("PartlyCloudy+CloudyOrPartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(PartlyCloudy, ClearOrPartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("PartlyCloudy+ClearOrPartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(PartlyCloudy, VariableCloudiness).first != VariableCloudiness)
    TEST_FAILED("PartlyCloudy+VariableCloudiness is similar to VariableCloudiness");

  if (similar_type(Clear, PartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("Clear+PartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(Clear, Clear).first != Clear) TEST_FAILED("Clear+Clear is similar to Clear");
  if (similar_type(Clear, MostlyPartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("Clear+MostlyPartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(Clear, MostlyClear).first != MostlyClear)
    TEST_FAILED("Clear+MostlyClear is similar to MostlyClear");
  if (similar_type(Clear, ClearOrPartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("Clear+ClearOrPartlyCloudy is similar to ClearOrPartlyCloudy");

  if (similar_type(MostlyCloudy, Cloudy).first != MostlyCloudy)
    TEST_FAILED("MostlyCloudy+Cloudy is similar to MostlyCloudy");
  if (similar_type(MostlyCloudy, PartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("MostlyCloudy+PartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(MostlyCloudy, MostlyCloudy).first != MostlyCloudy)
    TEST_FAILED("MostlyCloudy+MostlyCloudy is similar to MostlyCloudy");
  if (similar_type(MostlyCloudy, MostlyPartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("MostlyCloudy+MostlyPartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(MostlyCloudy, CloudyOrPartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("MostlyCloudy+CloudyOrPartlyCloudy is similar to CloudyOrPartlyCloudy");

  if (similar_type(MostlyPartlyCloudy, Cloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("MostlyPartlyCloudy+Cloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(MostlyPartlyCloudy, PartlyCloudy).first != MostlyPartlyCloudy)
    TEST_FAILED("MostlyPartlyCloudy+PartlyCloudy is similar to MostlyPartlyCloudy");
  if (similar_type(MostlyPartlyCloudy, Clear).first != ClearOrPartlyCloudy)
    TEST_FAILED("MostlyPartlyCloudy+Clear is similar to ClearOrPartlyCloudy");
  if (similar_type(MostlyPartlyCloudy, MostlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("MostlyPartlyCloudy+MostlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(MostlyPartlyCloudy, MostlyPartlyCloudy).first != MostlyPartlyCloudy)
    TEST_FAILED("MostlyPartlyCloudy+MostlyPartlyCloudy is similar to MostlyPartlyCloudy");
  if (similar_type(MostlyPartlyCloudy, MostlyClear).first != ClearOrPartlyCloudy)
    TEST_FAILED("MostlyPartlyCloudy+MostlyClear is similar to ClearOrPartlyCloudy");
  if (similar_type(MostlyPartlyCloudy, CloudyOrPartlyCloudy).first != VariableCloudiness)
    TEST_FAILED("MostlyPartlyCloudy+CloudyOrPartlyCloudy is similar to VariableCloudiness");
  if (similar_type(MostlyPartlyCloudy, ClearOrPartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("MostlyPartlyCloudy+ClearOrPartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(MostlyPartlyCloudy, VariableCloudiness).first != VariableCloudiness)
    TEST_FAILED("MostlyPartlyCloudy+VariableCloudiness is similar to VariableCloudiness");

  if (similar_type(MostlyClear, PartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("MostlyClear+PartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(MostlyClear, Clear).first != MostlyClear)
    TEST_FAILED("MostlyClear+Clear is similar to MostlyClear");
  if (similar_type(MostlyClear, MostlyPartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("MostlyClear+MostlyPartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(MostlyClear, MostlyClear).first != MostlyClear)
    TEST_FAILED("MostlyClear+MostlyClear is similar to MostlyClear");
  if (similar_type(MostlyClear, ClearOrPartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("MostlyClear+ClearOrPartlyCloudy is similar to ClearOrPartlyCloudy");

  if (similar_type(CloudyOrPartlyCloudy, Cloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("CloudyOrPartlyCloudy+Cloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(CloudyOrPartlyCloudy, PartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("CloudyOrPartlyCloudy+PartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(CloudyOrPartlyCloudy, MostlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("CloudyOrPartlyCloudy+MostlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(CloudyOrPartlyCloudy, MostlyPartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("CloudyOrPartlyCloudy+MostlyPartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(CloudyOrPartlyCloudy, CloudyOrPartlyCloudy).first != CloudyOrPartlyCloudy)
    TEST_FAILED("CloudyOrPartlyCloudy+CloudyOrPartlyCloudy is similar to CloudyOrPartlyCloudy");
  if (similar_type(CloudyOrPartlyCloudy, ClearOrPartlyCloudy).first != VariableCloudiness)
    TEST_FAILED("CloudyOrPartlyCloudy+ClearOrPartlyCloudy is similar to VariableCloudiness");
  if (similar_type(CloudyOrPartlyCloudy, VariableCloudiness).first != VariableCloudiness)
    TEST_FAILED("CloudyOrPartlyCloudy+VariableCloudiness is similar to VariableCloudiness");

  if (similar_type(ClearOrPartlyCloudy, PartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("ClearOrPartlyCloudy+PartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(ClearOrPartlyCloudy, Clear).first != ClearOrPartlyCloudy)
    TEST_FAILED("ClearOrPartlyCloudy+Clear is similar to ClearOrPartlyCloudy");
  if (similar_type(ClearOrPartlyCloudy, MostlyCloudy).first != VariableCloudiness)
    TEST_FAILED("ClearOrPartlyCloudy+MostlyCloudy is similar to VariableCloudiness");
  if (similar_type(ClearOrPartlyCloudy, MostlyPartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("ClearOrPartlyCloudy+MostlyPartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(ClearOrPartlyCloudy, MostlyClear).first != ClearOrPartlyCloudy)
    TEST_FAILED("ClearOrPartlyCloudy+MostlyClear is similar to ClearOrPartlyCloudy");
  if (similar_type(ClearOrPartlyCloudy, CloudyOrPartlyCloudy).first != VariableCloudiness)
    TEST_FAILED("ClearOrPartlyCloudy+CloudyOrPartlyCloudy is similar to VariableCloudiness");
  if (similar_type(ClearOrPartlyCloudy, ClearOrPartlyCloudy).first != ClearOrPartlyCloudy)
    TEST_FAILED("ClearOrPartlyCloudy+ClearOrPartlyCloudy is similar to ClearOrPartlyCloudy");
  if (similar_type(ClearOrPartlyCloudy, VariableCloudiness).first != VariableCloudiness)
    TEST_FAILED("ClearOrPartlyCloudy+VariableCloudiness is similar to VariableCloudiness");

  if (similar_type(DecreasingCloudiness, Clear).first != DecreasingCloudiness)
    TEST_FAILED("DecreasingCloudiness+Clear is similar to DecreasingCloudiness");
  if (similar_type(DecreasingCloudiness, MostlyClear).first != DecreasingCloudiness)
    TEST_FAILED("DecreasingCloudiness+MostlyClear is similar to DecreasingCloudiness");
  if (similar_type(DecreasingCloudiness, DecreasingCloudiness).first != DecreasingCloudiness)
    TEST_FAILED("DecreasingCloudiness+DecreasingCloudiness is similar to DecreasingCloudiness");

  if (similar_type(IncreasingCloudiness, Cloudy).first != IncreasingCloudiness)
    TEST_FAILED("IncreasingCloudiness+Cloudy is similar to IncreasingCloudiness");
  if (similar_type(IncreasingCloudiness, MostlyCloudy).first != IncreasingCloudiness)
    TEST_FAILED("IncreasingCloudiness+MostlyCloudy is similar to IncreasingCloudiness");
  if (similar_type(IncreasingCloudiness, IncreasingCloudiness).first != IncreasingCloudiness)
    TEST_FAILED("IncreasingCloudiness+IncreasingCloudiness is similar to IncreasingCloudiness");
  if (similar_type(IncreasingCloudiness, VariableCloudiness).first != VariableCloudiness)
    TEST_FAILED("IncreasingCloudiness+VariableCloudiness is similar to VariableCloudiness");

  if (similar_type(VariableCloudiness, PartlyCloudy).first != VariableCloudiness)
    TEST_FAILED("VariableCloudiness+PartlyCloudy is similar to VariableCloudiness");
  if (similar_type(VariableCloudiness, MostlyCloudy).first != VariableCloudiness)
    TEST_FAILED("VariableCloudiness+MostlyCloudy is similar to VariableCloudiness");
  if (similar_type(VariableCloudiness, MostlyPartlyCloudy).first != VariableCloudiness)
    TEST_FAILED("VariableCloudiness+MostlyPartlyCloudy is similar to VariableCloudiness");
  if (similar_type(VariableCloudiness, CloudyOrPartlyCloudy).first != VariableCloudiness)
    TEST_FAILED("VariableCloudiness+CloudyOrPartlyCloudy is similar to VariableCloudiness");
  if (similar_type(VariableCloudiness, ClearOrPartlyCloudy).first != VariableCloudiness)
    TEST_FAILED("VariableCloudiness+ClearOrPartlyCloudy is similar to VariableCloudiness");
  if (similar_type(VariableCloudiness, IncreasingCloudiness).first != VariableCloudiness)
    TEST_FAILED("VariableCloudiness+IncreasingCloudiness is similar to VariableCloudiness");
  if (similar_type(VariableCloudiness, VariableCloudiness).first != VariableCloudiness)
    TEST_FAILED("VariableCloudiness+VariableCloudiness is similar to VariableCloudiness");

  // test combinations

  std::vector<CloudinessType> vec;
  vec.push_back(PartlyCloudy);
  vec.push_back(MostlyPartlyCloudy);
  if (similar_type(vec).first != MostlyPartlyCloudy)
    TEST_FAILED("PartlyCloudy+MostlyPartlyCloudy = MostlyPartlyCloudy");

  vec.push_back(MostlyClear);
  if (similar_type(vec).first != ClearOrPartlyCloudy)
    TEST_FAILED("PartlyCloudy+MostlyPartlyCloudy+MostlyClear = ClearOrPartlyCloudy");

  vec.push_back(MostlyCloudy);
  if (similar_type(vec).first != VariableCloudiness)
    TEST_FAILED("PartlyCloudy+MostlyPartlyCloudy+MostlyClear+MostlyCloudy = VariableCloudiness");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test CloudinessStoryTools::cloudiness_phrase()
 */
// ----------------------------------------------------------------------

void cloudiness_phrase()
{
  using namespace TextGen;
  using namespace TextGen::CloudinessStoryTools;

  TextGen::PlainTextFormatter formatter;
  shared_ptr<Dictionary> dict(new TextGen::DebugDictionary);
  formatter.dictionary(dict);

  string result, expected;
  Sentence sentence;

#define PHRASETEST(input, output)                            \
  expected = output;                                         \
  sentence = CloudinessStoryTools::cloudiness_phrase(input); \
  if ((result = sentence.realize(formatter)) != expected)    \
    TEST_FAILED("Conversion of " + string(#input) + " to " + #output + " incorrect: " + result);

  PHRASETEST(Cloudy, "Pilvista.");
  PHRASETEST(PartlyCloudy, "Puolipilvista.");
  PHRASETEST(Clear, "Selkeaa.");
  PHRASETEST(MostlyCloudy, "Enimmakseen pilvista.");
  PHRASETEST(MostlyPartlyCloudy, "Enimmakseen puolipilvista.");
  PHRASETEST(MostlyClear, "Enimmakseen selkeaa.");
  PHRASETEST(CloudyOrPartlyCloudy, "Pilvista tai puolipilvista.");
  PHRASETEST(ClearOrPartlyCloudy, "Selkeaa tai puolipilvista.");
  PHRASETEST(DecreasingCloudiness, "Selkenevaa.");
  PHRASETEST(IncreasingCloudiness, "Pilvistyvaa.");
  PHRASETEST(VariableCloudiness, "Vaihtelevaa pilvisyytta.");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief The actual test driver
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(cloudiness_type);
    TEST(similar_type);
    TEST(cloudiness_phrase);
  }

};  // class tests

}  // namespace CloudinessStoryToolsTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");

  using namespace CloudinessStoryToolsTest;

  cout << endl << "CloudinessStoryTools tests" << endl << "==========================" << endl;

  tests t;
  return t.run();
}
