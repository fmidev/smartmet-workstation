// ======================================================================
/*!
 * \file
 * \brief Implementation of PrecipitationForecast class
 */
// ======================================================================

#include "WeatherStory.h"
#include "CloudinessStory.h"
#include "CloudinessStoryTools.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "NightAndDayPeriodGenerator.h"
#include "PeriodPhraseFactory.h"
#include "PrecipitationStoryTools.h"
#include <calculator/RangeAcceptor.h>
#include "ValueAcceptor.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/TimeTools.h>
#include <calculator/WeatherResult.h>
#include "WeekdayTools.h"
#include <calculator/NullPeriodGenerator.h>
#include <calculator/WeatherPeriodTools.h>
#include "AreaTools.h"
#include <calculator/MathTools.h>
#include "SeasonTools.h"
#include "SubMaskExtractor.h"
#include "PrecipitationForecast.h"
#include "CloudinessForecast.h"
#include "ThunderForecast.h"
#include "DebugTextFormatter.h"

#include <newbase/NFmiMercatorArea.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <vector>
#include <map>
#include <iomanip>

namespace TextGen
{
using namespace Settings;
using namespace TextGen;
using namespace AreaTools;
using namespace TimeTools;
using namespace boost;
using namespace std;

// pouta
#define HUOMENNA_SISAMAASSA_SAA_ON_POUTAINEN_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] saa on poutainen"
#define SISAMAASSA_SAA_ON_POUTAINEN_COMPOSITE_PHRASE "[sisamaassa] saa on poutainen"
#define HUOMENNA_SAA_ON_POUTAINEN_COMPOSITE_PHRASE "[huomenna] saa on poutainen"
#define HUOMENNA_SISAMAASSA_SAA_ON_ENIMMAKSEEN_POUTAINEN_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] saa on enimmakseen poutainen, [yksittaiset sadekuurot mahdollisia]"

// enimmakseen pouta, yksittaiset kuurot mahdollisia
#define SISAMAASSA_SAA_ON_ENIMMAKSEEN_POUTAINEN_COMPOSITE_PHRASE \
  "[sisamaassa] saa on enimmakseen poutainen, [yksittaiset sadekuurot mahdollisia]"
#define HUOMENNA_SAA_ON_ENIMMAKSEEN_POUTAINEN_COMPOSITE_PHRASE \
  "[huomenna] saa on enimmakseen poutainen, [yksittaiset sadekuurot mahdollisia]"

// sade
#define ILTAPAIVALLA_KUUROT_VOIVAT_OLLA_VOIMAKKAITA "[iltapaivalla] kuurot voivat olla voimakkaita"
#define ILTAPAIVALLA_SADE_VOI_OLLA_RUNSASTA "[iltapaivalla] sade voi olla runsasta"
#define HUOMENNA_SISAMAASSA_PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [paikoin] [heikkoa] [sadetta]"
#define HUOMENNA_SISAMAASSA_HEIKKOA_SADETTA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [heikkoa] [sadetta]"
#define HUOMENNA_SISAMAASSA_PAIKOIN_SADETTA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [paikoin] [sadetta]"

#define SISAMAASSA_PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE \
  "[sisamaassa] [paikoin] [heikkoa] [sadetta]"
#define SISAMAASSA_PAIKOIN_SADETTA_COMPOSITE_PHRASE "[sisamaassa] [paikoin] [sadetta]"
#define SISAMAASSA_HEIKKOA_SADETTA_COMPOSITE_PHRASE "[sisamaassa] [heikkoa] [sadetta]"
#define SISAMAASSA_SADETTA_COMPOSITE_PHRASE "[sisamaassa] [sadetta]"

#define HUOMENNA_PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE "[huomenna] [paikoin] [heikkoa] [sadetta]"
#define HUOMENNA_PAIKOIN_SADETTA_COMPOSITE_PHRASE "[huomenna] [paikoin] [sadetta]"
#define HUOMENNA_HEIKKOA_SADETTA_COMPOSITE_PHRASE "[huomenna] [heikkoa] [sadetta]"
#define HUOMENNA_SADETTA_COMPOSITE_PHRASE "[huomenna] [sadetta]"

#define PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE "[paikoin] [heikkoa] [sadetta]"
#define PAIKOIN_SADETTA_COMPOSITE_PHRASE "[paikoin] [sadetta]"
#define HEIKKOA_SADETTA_COMPOSITE_PHRASE "[heikkoa] [sadetta]"

#define HUOMENNA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] yksittaiset vesikuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] yksittaiset vesi- tai rantakuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] yksittaiset vesi- tai lumikuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] yksittaiset sadekuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] yksittaiset rantakuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] yksittaiset ranta- tai vesikuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] yksittaiset lumikuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] yksittaiset lumi- tai rantakuurot mahdollisia"

#define SISAMAASSA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[sisamaassa] yksittaiset vesikuurot mahdollisia"
#define SISAMAASSA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[sisamaassa] yksittaiset vesi- tai rantakuurot mahdollisia"
#define SISAMAASSA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[sisamaassa] yksittaiset vesi- tai lumikuurot mahdollisia"
#define SISAMAASSA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[sisamaassa] yksittaiset sadekuurot mahdollisia"
#define SISAMAASSA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[sisamaassa] yksittaiset rantakuurot mahdollisia"
#define SISAMAASSA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[sisamaassa] yksittaiset ranta- tai vesikuurot mahdollisia"
#define SISAMAASSA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[sisamaassa] yksittaiset lumikuurot mahdollisia"
#define SISAMAASSA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[sisamaassa] yksittaiset lumi- tai rantakuurot mahdollisia"

#define HUOMENNA_SISAMAASSA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] yksittaiset vesikuurot mahdollisia"
#define HUOMENNA_SISAMAASSA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] yksittaiset vesi- tai rantakuurot mahdollisia"
#define HUOMENNA_SISAMAASSA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] yksittaiset vesi- tai lumikuurot mahdollisia"
#define HUOMENNA_SISAMAASSA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] yksittaiset sadekuurot mahdollisia"
#define HUOMENNA_SISAMAASSA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] yksittaiset rantakuurot mahdollisia"
#define HUOMENNA_SISAMAASSA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] yksittaiset ranta- tai vesikuurot mahdollisia"
#define HUOMENNA_SISAMAASSA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] yksittaiset lumikuurot mahdollisia"
#define HUOMENNA_SISAMAASSA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] yksittaiset lumi- tai rantakuurot mahdollisia"

#define HUOMENNA_SISAMAASSA_SADETTA_COMPOSITE_PHRASE "[huomenna] [sisamaassa] [sadetta]"

#define HUOMENNA_SISAMAASSA_PAIKOIN_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [paikoin] [heikkoa] [sadetta], joka voi olla jaatavaa"
#define HUOMENNA_SISAMAASSA_PAIKOIN_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [paikoin] [sadetta], joka voi olla jaatavaa"
#define HUOMENNA_SISAMAASSA_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [heikkoa] [sadetta], joka voi olla jaatavaa"
#define HUOMENNA_SISAMAASSA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [sadetta], joka voi olla jaatavaa"

#define HUOMENNA_PAIKOIN_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[huomenna] [paikoin] [heikkoa] [sadetta], joka voi olla jaatavaa"
#define HUOMENNA_PAIKOIN_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[huomenna] [paikoin] [sadetta], joka voi olla jaatavaa"
#define HUOMENNA_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[huomenna] [heikkoa] [sadetta], joka voi olla jaatavaa"
#define HUOMENNA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[huomenna] [sadetta], joka voi olla jaatavaa"
#define SISAMAASSA_PAIKOIN_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[sisamaassa] [paikoin] [heikkoa] [sadetta], joka voi olla jaatavaa"
#define SISAMAASSA_PAIKOIN_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[sisamaassa] [paikoin] [sadetta], joka voi olla jaatavaa"
#define SISAMAASSA_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[sisamaassa] [heikkoa] [sadetta], joka voi olla jaatavaa"
#define SISAMAASSA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[sisamaassa] [sadetta], joka voi olla jaatavaa"
#define PAIKOIN_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[paikoin] [heikkoa] [sadetta], joka voi olla jaatavaa"
#define PAIKOIN_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[paikoin] [sadetta], joka voi olla jaatavaa"
#define HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE \
  "[heikkoa] [sadetta], joka voi olla jaatavaa"
#define SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE "[sadetta], joka voi olla jaatavaa"

#define HUOMENNA_SISAMAASSA_PAIKOIN_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [paikoin] [heikkoja] [sadekuuroja], jotka voivat olla jaatavia"
#define HUOMENNA_SISAMAASSA_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [heikkoja] [sadekuuroja], jotka voivat olla jaatavia"
#define HUOMENNA_SISAMAASSA_PAIKOIN_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [paikoin] [sadekuuroja], jotka voivat olla jaatavia"

#define HUOMENNA_SISAMAASSA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [sadekuuroja], jotka voivat olla jaatavia"

#define HUOMENNA_PAIKOIN_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[huomenna] [paikoin] [heikkoja] [sadekuuroja], jotka voivat olla jaatavia"
#define HUOMENNA_PAIKOIN_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[huomenna] [paikoin] [sadekuuroja], jotka voivat olla jaatavia"
#define HUOMENNA_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[huomenna] [heikkoja] [sadekuuroja], jotka voivat olla jaatavia"
#define HUOMENNA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[huomenna] [sadekuuroja], jotka voivat olla jaatavia"
#define SISAMAASSA_PAIKOIN_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[sisamaassa] [paikoin] [heikkoja] [sadekuuroja], jotka voivat olla jaatavia"
#define SISAMAASSA_PAIKOIN_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[sisamaassa] [paikoin] [sadekuuroja], jotka voivat olla jaatavia"
#define SISAMAASSA_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[sisamaassa] [heikkoja] [sadekuuroja], jotka voivat olla jaatavia"
#define SISAMAASSA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[sisamaassa] [sadekuuroja], jotka voivat olla jaatavia"
#define PAIKOIN_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[paikoin] [heikkoja] [sadekuuroja], jotka voivat olla jaatavia"
#define PAIKOIN_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[paikoin] [sadekuuroja], jotka voivat olla jaatavia"
#define HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[heikkoja] [sadekuuroja], jotka voivat olla jaatavia"
#define SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE \
  "[sadekuuroja], jotka voivat olla jaatavia"

//#define SAA_POUTAANTUU_COMPOSITE_PHRASE "[iltapaivalla] saa poutaantuu"

#define SAA_POUTAANTUU_VESISATEEN_JALKEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu vesisateen jalkeen"
#define SAA_POUTAANTUU_RANTASATEEN_JALKEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu rantasateen jalkeen"
#define SAA_POUTAANTUU_LUMISATEEN_JALKEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu lumisateen jalkeen"
#define SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu tihkusateen jalkeen"

//#define SAA_POUTAANTUU_JA_ON_SELKEA_COMPOSITE_PHRASE "[iltapaivalla] saa poutaantuu ja on
//[selkea]"

#define SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_SELKEA_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu vesisateen jalkeen ja on selkea"
#define SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_SELKEA_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu rantasateen jalkeen ja on selkea"
#define SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_SELKEA_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu lumisateen jalkeen ja on selkea"
#define SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_SELKEA_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu tihkusateen jalkeen ja on selkea"

#define SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_MELKO_SELKEA_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu vesisateen jalkeen ja on melko selkea"
#define SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_MELKO_SELKEA_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu rantasateen jalkeen ja on melko selkea"
#define SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_MELKO_SELKEA_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu lumisateen jalkeen ja on melko selkea"
#define SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_MELKO_SELKEA_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu tihkusateen jalkeen ja on melko selkea"

#define SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_PUOLIPILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu vesisateen jalkeen ja on puolipilvinen"
#define SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_PUOLIPILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu rantasateen jalkeen ja on puolipilvinen"
#define SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_PUOLIPILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu lumisateen jalkeen ja on puolipilvinen"
#define SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_PUOLIPILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu tihkusateen jalkeen ja on puolipilvinen"

#define SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_VERRATTAIN_PILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu vesisateen jalkeen ja on verrattain pilvinen"
#define SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_VERRATTAIN_PILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu rantasateen jalkeen ja on verrattain pilvinen"
#define SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_VERRATTAIN_PILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu lumisateen jalkeen ja on verrattain pilvinen"
#define SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_VERRATTAIN_PILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu tihkusateen jalkeen ja on verrattain pilvinen"

#define SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_PILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu vesisateen jalkeen ja on pilvinen"
#define SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_PILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu rantasateen jalkeen ja on pilvinen"
#define SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_PILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu lumisateen jalkeen ja on pilvinen"
#define SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_PILVINEN_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu tihkusateen jalkeen ja on pilvinen"

//#define SAA_POUTAANTUU_JA_VAIHTELEE_COMPOSITE_PHRASE "[iltapaivalla] saa poutaantuu ja vaihtelee
// puolipilvisesta pilviseen"

#define SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_VAIHTELEE_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu vesisateen jalkeen ja vaihtelee puolipilvisesta pilviseen"
#define SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_VAIHTELEE_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu rantasateen jalkeen ja vaihtelee puolipilvisesta pilviseen"
#define SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_VAIHTELEE_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu lumisateen jalkeen ja vaihtelee puolipilvisesta pilviseen"
#define SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_VAIHTELEE_COMPOSITE_PHRASE \
  "[iltapaivalla] saa poutaantuu tihkusateen jalkeen ja vaihtelee puolipilvisesta pilviseen"

InPlacesPhrase& get_in_places_phrase()
{
  static boost::thread_specific_ptr<InPlacesPhrase> tls;

  if (!tls.get()) tls.reset(new InPlacesPhrase);

  return *tls;
}

std::ostream& operator<<(std::ostream& theOutput,
                         const PrecipitationDataItemData& thePrecipitationDataItemData)
{
  theOutput << thePrecipitationDataItemData.theObservationTime << ": ";
  theOutput << setw(8) << setfill(' ') << fixed << setprecision(3)
            << thePrecipitationDataItemData.thePrecipitationIntensity;
  theOutput << setw(8) << setfill(' ') << fixed << setprecision(3)
            << thePrecipitationDataItemData.thePrecipitationMaxIntensity;
  theOutput << setw(8) << setfill(' ') << fixed << setprecision(1)
            << thePrecipitationDataItemData.thePrecipitationExtent;
  if (thePrecipitationDataItemData.thePrecipitationIntensity > 0)
  {
    theOutput << " ";
    if (thePrecipitationDataItemData.thePrecipitationFormWater > 0.0)
      theOutput << "   water=" << setw(3) << setfill(' ') << fixed << setprecision(0)
                << thePrecipitationDataItemData.thePrecipitationFormWater;
    if (thePrecipitationDataItemData.thePrecipitationFormDrizzle > 0.0)
      theOutput << " drizzle=" << setw(3) << setfill(' ') << fixed << setprecision(0)
                << thePrecipitationDataItemData.thePrecipitationFormDrizzle;
    if (thePrecipitationDataItemData.thePrecipitationFormSleet > 0.0)
      theOutput << "   sleet=" << setw(3) << setfill(' ') << fixed << setprecision(0)
                << thePrecipitationDataItemData.thePrecipitationFormSleet;
    if (thePrecipitationDataItemData.thePrecipitationFormSnow > 0.0)
      theOutput << "    snow=" << setw(3) << setfill(' ') << fixed << setprecision(0)
                << thePrecipitationDataItemData.thePrecipitationFormSnow;
    if (thePrecipitationDataItemData.thePrecipitationFormFreezing > 0.0)
      theOutput << "freezing=" << setw(3) << setfill(' ') << fixed << setprecision(0)
                << thePrecipitationDataItemData.thePrecipitationFormFreezing;
    if (thePrecipitationDataItemData.thePrecipitationTypeShowers > 0.0)
      theOutput << " showers=" << setw(3) << setfill(' ') << fixed << setprecision(0)
                << thePrecipitationDataItemData.thePrecipitationTypeShowers;
    theOutput << endl;

    /*
    theOutput << " ne=" << fixed << setprecision(0)
                      << thePrecipitationDataItemData.thePrecipitationPercentageNorthEast << " "
                      << "se=" << fixed << setprecision(0)
                      << thePrecipitationDataItemData.thePrecipitationPercentageSouthEast << " "
                      << "sw=" << fixed << setprecision(0)
                      << thePrecipitationDataItemData.thePrecipitationPercentageSouthWest << " "
                      << "nw=" << fixed << setprecision(0)
                      << thePrecipitationDataItemData.thePrecipitationPercentageNorthWest << " ";

    theOutput << " coord=" << setw(3) << setfill(' ') << fixed << setprecision(2)
                      << thePrecipitationDataItemData.thePrecipitationPoint.X()
                      << ", "
                      << setw(3) << setfill(' ') << fixed << setprecision(2)
                      << thePrecipitationDataItemData.thePrecipitationPoint.Y() << endl;
    */
  }
  else
  {
    theOutput << endl;
  }

  return theOutput;
}

std::string operator<<(std::string& theDestinationString, const std::string& theSourceString)
{
  theDestinationString.append(theSourceString);
  //	theDestinationString.append(" ");

  return theDestinationString;
}

InPlacesPhrase::InPlacesPhrase()
    : thePreviousPhrase(NONEXISTENT_PHRASE), thePreventTautologyFlag(false)
{
}

Sentence InPlacesPhrase::getInPlacesPhrase(PhraseType thePhraseType, bool useOllaVerbFlag)
{
  Sentence sentence;

  if (thePreventTautologyFlag && thePreviousPhrase == thePhraseType)
  {
    sentence << EMPTY_STRING;
    return sentence;
  }

  thePreventTautologyFlag = false;

  if (thePhraseType != NONEXISTENT_PHRASE)
  {
    /*
    if(useOllaVerbFlag)
      {
            stringVector.push_back(SAADAAN_WORD);
      }
    */
    thePreviousPhrase = thePhraseType;
    sentence << (thePhraseType == IN_SOME_PLACES_PHRASE ? PAIKOIN_WORD : MONIN_PAIKOIN_WORD);
  }
  else
  {
    sentence << EMPTY_STRING;
  }

  return sentence;
}

bool get_period_start_end_index(const WeatherPeriod& thePeriod,
                                const precipitation_data_vector& theDataVector,
                                unsigned int& theStartIndex,
                                unsigned int& theEndIndex)
{
  theStartIndex = 0;
  theEndIndex = 0;

  bool startFound = false;
  for (unsigned int i = 0; i < theDataVector.size(); i++)
  {
    if (theDataVector.at(i)->theObservationTime >= thePeriod.localStartTime() &&
        theDataVector.at(i)->theObservationTime <= thePeriod.localEndTime())
    {
      if (!startFound)
      {
        startFound = true;
        theStartIndex = i;
      }
      if (startFound) theEndIndex = i;
    }
  }
  return startFound;
}

weather_result_data_item_vector* get_data_vector(wf_story_params& theParameters,
                                                 const weather_result_data_id& theDataId)
{
  weather_result_data_item_vector* retval = 0;

  forecast_area_id areaId(NO_AREA);
  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    areaId = FULL_AREA;
  else if (theParameters.theForecastArea & COASTAL_AREA)
    areaId = COASTAL_AREA;
  else if (theParameters.theForecastArea & INLAND_AREA)
    areaId = INLAND_AREA;

  if (areaId != NO_AREA) retval = (*theParameters.theCompleteData[areaId])[theDataId];

  return retval;
}

bool get_period_start_end_index(const WeatherPeriod& thePeriod,
                                const weather_result_data_item_vector& theDataVector,
                                unsigned int& theStartIndex,
                                unsigned int& theEndIndex)
{
  theStartIndex = 0;
  theEndIndex = 0;

  bool startFound = false;
  for (unsigned int i = 0; i < theDataVector.size(); i++)
  {
    // Note: in weather result vector start and end times are same
    if (theDataVector.at(i)->thePeriod.localStartTime() >= thePeriod.localStartTime() &&
        theDataVector.at(i)->thePeriod.localStartTime() <= thePeriod.localEndTime())
    {
      if (!startFound)
      {
        startFound = true;
        theStartIndex = i;
      }
      if (startFound) theEndIndex = i;
    }
  }
  return startFound;
}

void can_be_freezing_phrase(const bool& theCanBeFreezingFlag,
                            map<string, Sentence>& theCompositePhraseElements,
                            bool thePluralFlag)
{
  if (theCanBeFreezingFlag)
  {
    if (thePluralFlag)
    {
      theCompositePhraseElements[JOTKA_VOIVAT_OLLA_JAATAVIA_PHRASE] << SAA_WORD;
    }
    else
    {
      theCompositePhraseElements[JOKA_VOI_OLLA_JAATAVAA_PHRASE] << SAA_WORD;
    }
  }
}

bool is_dry_weather(const wf_story_params& theParameters,
                    const precipitation_form_id& thePrecipitationForm,
                    const float& thePrecipitationIntensity,
                    const float& thePrecipitationExtent)
{
  if (thePrecipitationExtent == 0.0 || thePrecipitationExtent == kFloatMissing ||
      thePrecipitationIntensity == kFloatMissing)
    return true;

  bool dry_weather = false;

  switch (thePrecipitationForm)
  {
    case WATER_FREEZING_FORM:
    case FREEZING_FORM:
    case WATER_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitWater) dry_weather = true;
      break;
    case SLEET_FREEZING_FORM:
    case SLEET_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitSleet) dry_weather = true;
      break;
    case SNOW_FORM:
    case SNOW_FREEZING_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitSnow) dry_weather = true;
      break;
    case DRIZZLE_FORM:
    case DRIZZLE_FREEZING_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitDrizzle) dry_weather = true;
      break;
    case WATER_DRIZZLE_FREEZING_FORM:
    case WATER_DRIZZLE_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitWater) dry_weather = true;
      break;
    case DRIZZLE_SLEET_FORM:
    case DRIZZLE_SLEET_FREEZING_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitDrizzle &&
          thePrecipitationIntensity < theParameters.theDryWeatherLimitSleet)
        dry_weather = true;
      break;
    case WATER_DRIZZLE_SLEET_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitWater) dry_weather = true;
      break;
    case WATER_SLEET_FREEZING_FORM:
    case WATER_SLEET_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitWater) dry_weather = true;
      break;
    case WATER_SNOW_FREEZING_FORM:
    case WATER_SNOW_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitWater) dry_weather = true;
      break;
    case DRIZZLE_SNOW_FREEZING_FORM:
    case DRIZZLE_SNOW_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitDrizzle) dry_weather = true;
      break;
    case WATER_DRIZZLE_SNOW_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitWater) dry_weather = true;
      break;
    case WATER_SLEET_SNOW_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitWater) dry_weather = true;
      break;
    case DRIZZLE_SLEET_SNOW_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitDrizzle) dry_weather = true;
      break;
    case SLEET_SNOW_FREEZING_FORM:
    case SLEET_SNOW_FORM:
      if (thePrecipitationIntensity < theParameters.theDryWeatherLimitSleet) dry_weather = true;
      break;
    case MISSING_PRECIPITATION_FORM:
      dry_weather = true;
      break;
  }

  return dry_weather;
}

void PrecipitationForecast::waterAndSnowShowersPhrase(
    const float& thePrecipitationIntensity,
    const float thePrecipitationIntensityAbsoluteMax,
    const float& theWaterDrizzleSleetShare,
    const bool& theCanBeFreezingFlag,
    map<string, Sentence>& theCompositePhraseElements) const
{
  // when showers, dont use sleet, use water and snow instead (Sari Hartonen)
  // and if there is no heavy rain
  if (thePrecipitationIntensity < theParameters.theWeakPrecipitationLimitSnow &&
      thePrecipitationIntensityAbsoluteMax < theParameters.theHeavyPrecipitationLimitSnow)
  {
    theCompositePhraseElements[INTENSITY_PARAMETER] << HEIKKOJA_WORD;
  }
  else if (thePrecipitationIntensity >= theParameters.theHeavyPrecipitationLimitSnow)
  {
    theCompositePhraseElements[INTENSITY_PARAMETER] << RUNSAITA_WORD;
  }
  else
  {
    theCheckHeavyIntensityFlag = SHOWERS;
  }

  if (theCanBeFreezingFlag)
  {
    can_be_freezing_phrase(theCanBeFreezingFlag, theCompositePhraseElements, true);

    theCompositePhraseElements[PRECIPITATION_PARAMETER] << LUMI_TAI_VESIKUUROJA_PHRASE;
  }
  else
  {
    if (theWaterDrizzleSleetShare > MAJORITY_LIMIT)
    {
      theCompositePhraseElements[PRECIPITATION_PARAMETER] << VESI_TAI_LUMIKUUROJA_PHRASE;
    }
    else
    {
      theCompositePhraseElements[PRECIPITATION_PARAMETER] << LUMI_TAI_VESIKUUROJA_PHRASE;
    }
  }
}

void PrecipitationForecast::mostlyDryWeatherPhrase(
    const bool& theIsShowersFlag,
    const WeatherPeriod& thePeriod,
    const char* thePhrase,
    map<string, Sentence>& theCompositePhraseElements) const
{
  if (!theDryPeriodTautologyFlag)
  {
    theCompositePhraseElements[HUOMENNA_SISAMAASSA_SAA_ON_ENIMMAKSEEN_POUTAINEN_COMPOSITE_PHRASE]
        << SAA_WORD;
  }
  else
  {
    if (strcmp(thePhrase, YKSITTAISET_SADEKUUROT_MAHDOLLISIA_PHRASE) == 0)
      theCompositePhraseElements[HUOMENNA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE]
          << SAA_WORD;
    else if (strcmp(thePhrase, YKSITTAISET_VESIKUUROT_MAHDOLLISIA_PHRASE) == 0)
      theCompositePhraseElements[HUOMENNA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE]
          << SAA_WORD;
    else if (strcmp(thePhrase, YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_PHRASE) == 0)
      theCompositePhraseElements[HUOMENNA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE]
          << SAA_WORD;
    else if (strcmp(thePhrase, YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_PHRASE) == 0)
      theCompositePhraseElements[HUOMENNA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE]
          << SAA_WORD;
    else if (strcmp(thePhrase, YKSITTAISET_VESI_RANTA_KUUROT_MAHDOLLISIA_PHRASE) == 0)
      theCompositePhraseElements
              [HUOMENNA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE]
          << SAA_WORD;
    else if (strcmp(thePhrase, YKSITTAISET_RANTA_VESI_KUUROT_MAHDOLLISIA_PHRASE) == 0)
      theCompositePhraseElements
              [HUOMENNA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE]
          << SAA_WORD;
    else if (strcmp(thePhrase, YKSITTAISET_LUMI_RANTA_KUUROT_MAHDOLLISIA_PHRASE) == 0)
      theCompositePhraseElements
              [HUOMENNA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE]
          << SAA_WORD;
    else if (strcmp(thePhrase, YKSITTAISET_VESI_LUMI_KUUROT_MAHDOLLISIA_PHRASE) == 0)
      theCompositePhraseElements
              [HUOMENNA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE]
          << SAA_WORD;
    else
      theCompositePhraseElements[PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE] << SAA_WORD;
  }

  theCompositePhraseElements[PRECIPITATION_PARAMETER] << thePhrase;

  theDryPeriodTautologyFlag = true;
}

void PrecipitationForecast::getTransformationPhraseElements(
    const WeatherPeriod& thePeriod,
    const float& thePrecipitationExtent,
    const precipitation_form_transformation_id& theTransformId,
    map<string, Sentence>& theCompositePhraseElements) const
{
  const bool in_some_places = thePrecipitationExtent > theParameters.theInSomePlacesLowerLimit &&
                              thePrecipitationExtent <= theParameters.theInSomePlacesUpperLimit;
  const bool in_many_places = thePrecipitationExtent > theParameters.theInManyPlacesLowerLimit &&
                              thePrecipitationExtent <= theParameters.theInManyPlacesUpperLimit;

  // check if intensity is heavy
  theCheckHeavyIntensityFlag = CONTINUOUS;

  InPlacesPhrase& inPlacesPhraseMaker = get_in_places_phrase();
  InPlacesPhrase::PhraseType phraseType(InPlacesPhrase::NONEXISTENT_PHRASE);
  if (in_some_places)
    phraseType = InPlacesPhrase::IN_SOME_PLACES_PHRASE;
  else if (in_many_places)
    phraseType = InPlacesPhrase::IN_MANY_PLACES_PHRASE;

  theCompositePhraseElements[IN_PLACES_PARAMETER]
      << inPlacesPhraseMaker.getInPlacesPhrase(phraseType, theUseOllaVerbFlag);

  std::string jokaMuuttuuString;

  switch (theTransformId)
  {
    case WATER_TO_SNOW:
    case WATER_TO_DRIZZLE:
    case WATER_TO_SLEET:
    {
      if (theTransformId == WATER_TO_SNOW)
        jokaMuuttuuString = JOKA_MUUTTUU_LUMISATEEKSI_PHRASE;
      else if (theTransformId == WATER_TO_DRIZZLE)
        jokaMuuttuuString = JOKA_MUUTTUU_TIHKUSATEEKSI_PHRASE;
      else
        jokaMuuttuuString = JOKA_MUUTTUU_RANTASATEEKSI_PHRASE;

      theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE]
          << VESISADETTA_WORD << Delimiter(COMMA_PUNCTUATION_MARK) << jokaMuuttuuString;
    }
    break;
    case SNOW_TO_WATER:
    case SNOW_TO_DRIZZLE:
    case SNOW_TO_SLEET:
    {
      if (theTransformId == SNOW_TO_WATER)
        jokaMuuttuuString = JOKA_MUUTTUU_VESISATEEKSI_PHRASE;
      else if (theTransformId == SNOW_TO_DRIZZLE)
        jokaMuuttuuString = JOKA_MUUTTUU_TIHKUSATEEKSI_PHRASE;
      else
        jokaMuuttuuString = JOKA_MUUTTUU_RANTASATEEKSI_PHRASE;

      theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE]
          << LUMISADETTA_WORD << Delimiter(COMMA_PUNCTUATION_MARK) << jokaMuuttuuString;
    }
    break;
    case DRIZZLE_TO_WATER:
    case DRIZZLE_TO_SNOW:
    case DRIZZLE_TO_SLEET:
    {
      if (theTransformId == DRIZZLE_TO_WATER)
        jokaMuuttuuString = JOKA_MUUTTUU_VESISATEEKSI_PHRASE;
      else if (theTransformId == DRIZZLE_TO_SNOW)
        jokaMuuttuuString = JOKA_MUUTTUU_LUMISATEEKSI_PHRASE;
      else
        jokaMuuttuuString = JOKA_MUUTTUU_RANTASATEEKSI_PHRASE;

      theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE]
          << TIHKUSADETTA_WORD << Delimiter(COMMA_PUNCTUATION_MARK) << jokaMuuttuuString;
    }
    break;
    case SLEET_TO_WATER:
    case SLEET_TO_DRIZZLE:
    case SLEET_TO_SNOW:
    {
      if (theTransformId == SLEET_TO_WATER)
        jokaMuuttuuString = JOKA_MUUTTUU_VESISATEEKSI_PHRASE;
      else if (theTransformId == SLEET_TO_DRIZZLE)
        jokaMuuttuuString = JOKA_MUUTTUU_TIHKUSATEEKSI_PHRASE;
      else
        jokaMuuttuuString = JOKA_MUUTTUU_LUMISATEEKSI_PHRASE;

      theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE]
          << RANTASADETTA_WORD << Delimiter(COMMA_PUNCTUATION_MARK) << jokaMuuttuuString;
    }
    break;
    default:
      break;
  }
}

void PrecipitationForecast::getPrecipitationPhraseElements(
    const WeatherPeriod& thePeriod,
    const precipitation_form_id& thePrecipitationForm,
    const float& thePrecipitationIntensity,
    const float thePrecipitationIntensityAbsoluteMax,
    const float& thePrecipitationExtent,
    const float& thePrecipitationFormWater,
    const float& thePrecipitationFormDrizzle,
    const float& thePrecipitationFormSleet,
    const float& thePrecipitationFormSnow,
    const float& thePrecipitationFormFreezing,
    const precipitation_type& thePrecipitationType,
    const TextGenPosixTime& theTypeChangeTime,
    map<string, Sentence>& theCompositePhraseElements) const
{
  theCheckHeavyIntensityFlag = MISSING_PRECIPITATION_TYPE;

  bool dry_weather = is_dry_weather(
      theParameters, thePrecipitationForm, thePrecipitationIntensity, thePrecipitationExtent);

  if (dry_weather)
  {
    theCompositePhraseElements[SAA_ON_POUTAINEN_PHRASE] << SAA_ON_POUTAINEN_PHRASE;

    theDryPeriodTautologyFlag = true;
  }
  else
  {
    if (thePrecipitationForm == MISSING_PRECIPITATION_FORM) return;

    // use the summer phrase if it is summertime and no more than one precipitation form is involved
    bool use_summer_phrase =
        SeasonTools::isSummerHalf(thePeriod.localStartTime(), theParameters.theVariable) &&
        theSinglePrecipitationFormFlag;

    const bool is_showers = thePrecipitationType == SHOWERS;
    const bool mostly_dry_weather =
        thePrecipitationExtent <= theParameters.theMostlyDryWeatherLimit;
    const bool in_some_places = thePrecipitationExtent > theParameters.theInSomePlacesLowerLimit &&
                                thePrecipitationExtent <= theParameters.theInSomePlacesUpperLimit;
    const bool in_many_places = thePrecipitationExtent > theParameters.theInManyPlacesLowerLimit &&
                                thePrecipitationExtent <= theParameters.theInManyPlacesUpperLimit;
    InPlacesPhrase::PhraseType phraseType(InPlacesPhrase::NONEXISTENT_PHRASE);
    if (in_some_places)
      phraseType = InPlacesPhrase::IN_SOME_PLACES_PHRASE;
    else if (in_many_places)
      phraseType = InPlacesPhrase::IN_MANY_PLACES_PHRASE;

    bool can_be_freezing =
        thePrecipitationFormFreezing > theParameters.theFreezingPrecipitationLimit;

    InPlacesPhrase& inPlacesPhraseMaker = get_in_places_phrase();

    theParameters.theLog << "Precipitation form is "
                         << precipitation_form_string(thePrecipitationForm) << endl;
    theParameters.theLog << "Precipitation extent is " << thePrecipitationExtent << endl;

    if (!(thePrecipitationIntensity >= theParameters.theDryWeatherLimitSnow && mostly_dry_weather))
      theCompositePhraseElements[PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE] << SAA_WORD;

    switch (thePrecipitationForm)
    {
      case WATER_FREEZING_FORM:
      case FREEZING_FORM:
      case WATER_FORM:
      {
        if (thePrecipitationIntensity >= theParameters.theDryWeatherLimitWater)
        {
          if (mostly_dry_weather)
          {
            mostlyDryWeatherPhrase(is_showers,
                                   thePeriod,
                                   (use_summer_phrase ? YKSITTAISET_SADEKUUROT_MAHDOLLISIA_PHRASE
                                                      : YKSITTAISET_VESIKUUROT_MAHDOLLISIA_PHRASE),
                                   theCompositePhraseElements);
          }
          else
          {
            theCompositePhraseElements[IN_PLACES_PARAMETER]
                << inPlacesPhraseMaker.getInPlacesPhrase(phraseType, theUseOllaVerbFlag);

            if (is_showers)
            {
              /*
                    // heikot kuurot poistettu 17.8.2011
              if(thePrecipitationIntensity < theParameters.theWeakPrecipitationLimitWater &&
                     thePrecipitationIntensityAbsoluteMax <
              theParameters.theHeavyPrecipitationLimitWater)
                    {
                      theCompositePhraseElements[PRECIPITATION_PARAMETER] <<
                            (use_summer_phrase ? HEIKKOJA_SADEKUUROJA_PHRASE :
              HEIKKOJA_VESIKUUROJA_PHRASE);
                    }
              else
              */
              if (thePrecipitationIntensity >= theParameters.theHeavyPrecipitationLimitWater)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER]
                    << (use_summer_phrase ? VOIMAKKAITA_SADEKUUROJA_PHRASE
                                          : VOIMAKKAITA_VESIKUUROJA_PHRASE);
              }
              else
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER]
                    << (use_summer_phrase ? SADEKUUROJA_WORD : VESIKUUROJA_WORD);
                theCheckHeavyIntensityFlag = SHOWERS;
              }
              can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, true);
            }
            else
            {
              if (thePrecipitationIntensity < theParameters.theWeakPrecipitationLimitWater &&
                  thePrecipitationIntensityAbsoluteMax <
                      theParameters.theHeavyPrecipitationLimitWater)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER]
                    << (use_summer_phrase ? HEIKKOA_SADETTA_PHRASE : HEIKKOA_VESISADETTA_PHRASE);
              }
              else if (thePrecipitationIntensity >= theParameters.theHeavyPrecipitationLimitWater)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER]
                    << (use_summer_phrase ? RUNSASTA_SADETTA_PHRASE : RUNSASTA_VESISADETTA_PHRASE);
              }
              else
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER]
                    << (use_summer_phrase ? SADETTA_WORD : VESISADETTA_WORD);
                theCheckHeavyIntensityFlag = CONTINUOUS;
              }

              can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, false);
            }
            theDryPeriodTautologyFlag = false;
          }
        }
        else
        {
          theCompositePhraseElements[SAA_ON_POUTAINEN_PHRASE] << SAA_ON_POUTAINEN_PHRASE;
          theDryPeriodTautologyFlag = true;
        }
        break;
      }

      case SLEET_FREEZING_FORM:
      case SLEET_FORM:
      {
        if (thePrecipitationIntensity >= theParameters.theDryWeatherLimitSleet)
        {
          if (mostly_dry_weather)
          {
            mostlyDryWeatherPhrase(is_showers,
                                   thePeriod,
                                   YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_PHRASE,
                                   theCompositePhraseElements);
          }
          else
          {
            theCompositePhraseElements[IN_PLACES_PARAMETER]
                << inPlacesPhraseMaker.getInPlacesPhrase(phraseType, theUseOllaVerbFlag);

            if (is_showers)
            {
              theCompositePhraseElements[PRECIPITATION_PARAMETER] << RANTAKUUROJA_WORD;
              theCheckHeavyIntensityFlag = SHOWERS;
            }
            else
            {
              theCompositePhraseElements[PRECIPITATION_PARAMETER] << RANTASADETTA_WORD;
              theCheckHeavyIntensityFlag = CONTINUOUS;
              if (can_be_freezing)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << JA_WORD
                                                                    << JAATAVAA_VESISADETTA_PHRASE;
              }
            }
            theDryPeriodTautologyFlag = false;
          }
        }
        else
        {
          theCompositePhraseElements[SAA_ON_POUTAINEN_PHRASE] << SAA_ON_POUTAINEN_PHRASE;
          theDryPeriodTautologyFlag = true;
        }
        break;
      }

      case SNOW_FORM:
      case SNOW_FREEZING_FORM:
      {
        if (thePrecipitationIntensity >= theParameters.theDryWeatherLimitSnow)
        {
          if (mostly_dry_weather)
          {
            mostlyDryWeatherPhrase(is_showers,
                                   thePeriod,
                                   YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_PHRASE,
                                   theCompositePhraseElements);
          }
          else
          {
            theCompositePhraseElements[IN_PLACES_PARAMETER]
                << inPlacesPhraseMaker.getInPlacesPhrase(phraseType, theUseOllaVerbFlag);

            if (is_showers)
            {
              /*
                    heikot kuurot poistettu 17.8.2011
              if(thePrecipitationIntensity < theParameters.theWeakPrecipitationLimitSnow &&
                     thePrecipitationIntensityAbsoluteMax <
              theParameters.theHeavyPrecipitationLimitSnow)
                    {
                      theCompositePhraseElements[INTENSITY_PARAMETER] << HEIKKOJA_WORD;
                    }
              else
              */
              if (thePrecipitationIntensity >= theParameters.theHeavyPrecipitationLimitSnow)
              {
                theCompositePhraseElements[INTENSITY_PARAMETER] << SAKEITA_WORD;
              }
              else
              {
                theCheckHeavyIntensityFlag = SHOWERS;
              }

              theCompositePhraseElements[PRECIPITATION_PARAMETER] << LUMIKUUROJA_WORD;
            }
            else
            {
              if (thePrecipitationIntensity < theParameters.theWeakPrecipitationLimitSnow &&
                  thePrecipitationIntensityAbsoluteMax <
                      theParameters.theHeavyPrecipitationLimitSnow)
              {
                theCompositePhraseElements[INTENSITY_PARAMETER] << HEIKKOA_WORD;
              }
              else if (thePrecipitationIntensity >= theParameters.theHeavyPrecipitationLimitSnow)
              {
                theCompositePhraseElements[INTENSITY_PARAMETER] << SAKEAA_WORD;
              }
              else
              {
                theCheckHeavyIntensityFlag = CONTINUOUS;
              }

              theCompositePhraseElements[PRECIPITATION_PARAMETER] << LUMISADETTA_WORD;

              if (can_be_freezing)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << JA_WORD
                                                                    << JAATAVAA_VESISADETTA_PHRASE;
              }
            }
            theDryPeriodTautologyFlag = false;
          }
        }
        else
        {
          theCompositePhraseElements[SAA_ON_POUTAINEN_PHRASE] << SAA_ON_POUTAINEN_PHRASE;
          theDryPeriodTautologyFlag = true;
        }
        break;
      }

      case DRIZZLE_FORM:
      case DRIZZLE_FREEZING_FORM:

      {
        if (thePrecipitationIntensity >= theParameters.theDryWeatherLimitDrizzle)
        {
          if (mostly_dry_weather)
          {
            mostlyDryWeatherPhrase(is_showers,
                                   thePeriod,
                                   (use_summer_phrase ? YKSITTAISET_SADEKUUROT_MAHDOLLISIA_PHRASE
                                                      : YKSITTAISET_VESIKUUROT_MAHDOLLISIA_PHRASE),
                                   theCompositePhraseElements);
          }
          else
          {
            theCompositePhraseElements[IN_PLACES_PARAMETER]
                << inPlacesPhraseMaker.getInPlacesPhrase(phraseType, theUseOllaVerbFlag);
            if (is_showers)
            {
              theCompositePhraseElements[PRECIPITATION_PARAMETER]
                  << (use_summer_phrase ? SADEKUUROJA_WORD : VESIKUUROJA_WORD);
              theCheckHeavyIntensityFlag = SHOWERS;

              can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, true);
            }
            else
            {
              theCompositePhraseElements[PRECIPITATION_PARAMETER] << TIHKUSADETTA_WORD;

              can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, false);
            }
            theDryPeriodTautologyFlag = false;
          }
        }
        else
        {
          theCompositePhraseElements[SAA_ON_POUTAINEN_PHRASE] << SAA_ON_POUTAINEN_PHRASE;
          theDryPeriodTautologyFlag = true;
        }
        break;
      }

      case WATER_DRIZZLE_FREEZING_FORM:
      case WATER_DRIZZLE_FORM:
      {
        if (thePrecipitationIntensity >= theParameters.theDryWeatherLimitDrizzle)
        {
          if (mostly_dry_weather)
          {
            mostlyDryWeatherPhrase(is_showers,
                                   thePeriod,
                                   (use_summer_phrase ? YKSITTAISET_SADEKUUROT_MAHDOLLISIA_PHRASE
                                                      : YKSITTAISET_VESIKUUROT_MAHDOLLISIA_PHRASE),
                                   theCompositePhraseElements);
          }
          else
          {
            theCompositePhraseElements[IN_PLACES_PARAMETER]
                << inPlacesPhraseMaker.getInPlacesPhrase(phraseType, theUseOllaVerbFlag);
            if (is_showers)
            {
              theCompositePhraseElements[PRECIPITATION_PARAMETER]
                  << (use_summer_phrase ? SADEKUUROJA_WORD : VESIKUUROJA_WORD);
              theCheckHeavyIntensityFlag = SHOWERS;

              can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, true);
            }
            else
            {
              if (thePrecipitationFormDrizzle < 70.0)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER]
                    << (use_summer_phrase ? SADETTA_WORD : VESISADETTA_WORD);
              }
              else
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << TIHKUSADETTA_WORD;
              }
              theCheckHeavyIntensityFlag = CONTINUOUS;
              can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, false);
            }
            theDryPeriodTautologyFlag = false;
          }
        }
        else
        {
          theCompositePhraseElements[SAA_ON_POUTAINEN_PHRASE] << SAA_ON_POUTAINEN_PHRASE;
          theDryPeriodTautologyFlag = true;
        }
        break;
      }

      case DRIZZLE_SLEET_FORM:
      case DRIZZLE_SLEET_FREEZING_FORM:
      case WATER_DRIZZLE_SLEET_FORM:
      case WATER_SLEET_FREEZING_FORM:
      case WATER_SLEET_FORM:
      {
        if (thePrecipitationIntensity >= theParameters.theDryWeatherLimitDrizzle)
        {
          if (mostly_dry_weather)
          {
            mostlyDryWeatherPhrase(is_showers,
                                   thePeriod,
                                   (thePrecipitationFormWater >= thePrecipitationFormSleet
                                        ? YKSITTAISET_VESI_RANTA_KUUROT_MAHDOLLISIA_PHRASE
                                        : YKSITTAISET_RANTA_VESI_KUUROT_MAHDOLLISIA_PHRASE),
                                   theCompositePhraseElements);
          }
          else
          {
            theCompositePhraseElements[IN_PLACES_PARAMETER]
                << inPlacesPhraseMaker.getInPlacesPhrase(phraseType, theUseOllaVerbFlag);

            if (is_showers)
            {
              if (thePrecipitationFormWater >= thePrecipitationFormSleet && !can_be_freezing)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << VESI_TAI_RANTAKUUROJA_PHRASE;
              }
              else
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << RANTA_TAI_VESIKUUROJA_PHRASE;
              }

              theCheckHeavyIntensityFlag = SHOWERS;
              can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, true);
            }
            else
            {
              if (thePrecipitationFormWater >= thePrecipitationFormSleet)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << VESI_TAI_RANTASADETTA_PHRASE;
                theCheckHeavyIntensityFlag = CONTINUOUS;
              }
              else
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << RANTA_TAI_VESISADETTA_PHRASE;
                theCheckHeavyIntensityFlag = CONTINUOUS;
              }
              can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, false);
            }
            theDryPeriodTautologyFlag = false;
          }
        }
        else
        {
          theCompositePhraseElements[SAA_ON_POUTAINEN_PHRASE] << SAA_ON_POUTAINEN_PHRASE;
          theDryPeriodTautologyFlag = true;
        }
        break;
      }

      case WATER_SNOW_FORM:
      case DRIZZLE_SNOW_FORM:
      case WATER_DRIZZLE_SNOW_FORM:
      case DRIZZLE_SLEET_SNOW_FORM:
      case WATER_SLEET_SNOW_FORM:
      case DRIZZLE_SNOW_FREEZING_FORM:
      case WATER_SNOW_FREEZING_FORM:
      {
        if (thePrecipitationIntensity >= theParameters.theDryWeatherLimitDrizzle)
        {
          if (mostly_dry_weather)
          {
            mostlyDryWeatherPhrase(is_showers,
                                   thePeriod,
                                   YKSITTAISET_VESI_LUMI_KUUROT_MAHDOLLISIA_PHRASE,
                                   theCompositePhraseElements);
          }
          else
          {
            theCompositePhraseElements[IN_PLACES_PARAMETER]
                << inPlacesPhraseMaker.getInPlacesPhrase(phraseType, theUseOllaVerbFlag);

            if (is_showers)
            {
              waterAndSnowShowersPhrase(thePrecipitationIntensity,
                                        thePrecipitationIntensityAbsoluteMax,
                                        thePrecipitationFormWater + thePrecipitationFormDrizzle +
                                            thePrecipitationFormSleet,
                                        can_be_freezing,
                                        theCompositePhraseElements);
            }
            else
            {
              /*
            IF vesi on suunnilleen yhta paljon kuin lumi AND ranta < vesi+lumi, THEN
            vesi- tai lumisadetta.
              */

              if (thePrecipitationIntensity < theParameters.theWeakPrecipitationLimitSnow &&
                  thePrecipitationIntensityAbsoluteMax <
                      theParameters.theHeavyPrecipitationLimitSnow)
              {
                theCompositePhraseElements[INTENSITY_PARAMETER] << HEIKKOA_WORD;
              }
              else if (thePrecipitationIntensity >= theParameters.theHeavyPrecipitationLimitSnow)
              {
                theCompositePhraseElements[INTENSITY_PARAMETER] << KOVAA_WORD;
              }
              else
              {
                theCheckHeavyIntensityFlag = CONTINUOUS;
              }

              if (thePrecipitationFormWater >= thePrecipitationFormSnow)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << VESI_TAI_LUMISADETTA_PHRASE;
              }
              else
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << LUMI_TAI_VESISADETTA_PHRASE;

                can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, false);
              }
            }
            theDryPeriodTautologyFlag = false;
          }
        }
        else
        {
          theCompositePhraseElements[SAA_ON_POUTAINEN_PHRASE] << SAA_ON_POUTAINEN_PHRASE;
          theDryPeriodTautologyFlag = true;
        }
        break;
      }

      case SLEET_SNOW_FREEZING_FORM:
      case SLEET_SNOW_FORM:
      {
        if (thePrecipitationIntensity >= theParameters.theDryWeatherLimitSleet)
        {
          if (mostly_dry_weather)
          {
            mostlyDryWeatherPhrase(is_showers,
                                   thePeriod,
                                   YKSITTAISET_LUMI_RANTA_KUUROT_MAHDOLLISIA_PHRASE,
                                   theCompositePhraseElements);
          }
          else
          {
            theCompositePhraseElements[IN_PLACES_PARAMETER]
                << inPlacesPhraseMaker.getInPlacesPhrase(phraseType, theUseOllaVerbFlag);

            if (is_showers)
            {
              waterAndSnowShowersPhrase(thePrecipitationIntensity,
                                        thePrecipitationIntensityAbsoluteMax,
                                        thePrecipitationFormWater + thePrecipitationFormDrizzle +
                                            thePrecipitationFormSleet,
                                        can_be_freezing,
                                        theCompositePhraseElements);
            }
            else
            {
              if (thePrecipitationIntensity < theParameters.theWeakPrecipitationLimitSnow &&
                  thePrecipitationIntensityAbsoluteMax <
                      theParameters.theHeavyPrecipitationLimitSnow)
              {
                theCompositePhraseElements[INTENSITY_PARAMETER] << HEIKKOA_WORD;
              }
              else if (thePrecipitationIntensity >= theParameters.theHeavyPrecipitationLimitSnow)
              {
                theCompositePhraseElements[INTENSITY_PARAMETER] << SAKEAA_WORD;
              }
              else
              {
                theCheckHeavyIntensityFlag = CONTINUOUS;
              }

              if (thePrecipitationFormSnow >= thePrecipitationFormSleet)
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << LUMI_TAI_RANTASADETTA_PHRASE;

                can_be_freezing_phrase(can_be_freezing, theCompositePhraseElements, false);
              }
              else
              {
                theCompositePhraseElements[PRECIPITATION_PARAMETER] << RANTA_TAI_LUMISADETTA_PHRASE;
              }
            }
            theDryPeriodTautologyFlag = false;
          }
        }
        else
        {
          theCompositePhraseElements[SAA_ON_POUTAINEN_PHRASE] << SAA_ON_POUTAINEN_PHRASE;
          theDryPeriodTautologyFlag = true;
        }
        break;
      }
      default:
        break;
    }
  }
}

void PrecipitationForecast::selectPrecipitationSentence(
    const WeatherPeriod& thePeriod,
    const precipitation_form_id& thePrecipitationForm,
    const float thePrecipitationIntensity,
    const float thePrecipitationAbsoluteMax,
    const float thePrecipitationExtent,
    const float thePrecipitationFormWater,
    const float thePrecipitationFormDrizzle,
    const float thePrecipitationFormSleet,
    const float thePrecipitationFormSnow,
    const float thePrecipitationFormFreezing,
    const precipitation_type& thePrecipitationType,
    const TextGenPosixTime& theTypeChangeTime,
    const precipitation_form_transformation_id& theTransformationId,
    map<string, Sentence>& theCompositePhraseElements) const
{
  if (theTransformationId == NO_FORM_TRANSFORMATION)
  {
    getPrecipitationPhraseElements(thePeriod,
                                   thePrecipitationForm,
                                   thePrecipitationIntensity,
                                   thePrecipitationAbsoluteMax,
                                   thePrecipitationExtent,
                                   thePrecipitationFormWater,
                                   thePrecipitationFormDrizzle,
                                   thePrecipitationFormSleet,
                                   thePrecipitationFormSnow,
                                   thePrecipitationFormFreezing,
                                   thePrecipitationType,
                                   theTypeChangeTime,
                                   theCompositePhraseElements);
  }
  else
  {
    getTransformationPhraseElements(
        thePeriod, thePrecipitationExtent, theTransformationId, theCompositePhraseElements);
  }
}

bool PrecipitationForecast::getDryPeriods(const WeatherPeriod& theSourcePeriod,
                                          vector<WeatherPeriod>& theDryPeriods) const
{
  const vector<WeatherPeriod>* precipitationPeriods = 0;

  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    precipitationPeriods = &thePrecipitationPeriodsFull;
  else if (theParameters.theForecastArea & INLAND_AREA)
    precipitationPeriods = &thePrecipitationPeriodsInland;
  else if (theParameters.theForecastArea & COASTAL_AREA)
    precipitationPeriods = &thePrecipitationPeriodsCoastal;

  if (precipitationPeriods->size() == 0)
  {
    theDryPeriods.push_back(
        WeatherPeriod(theSourcePeriod.localStartTime(), theSourcePeriod.localEndTime()));
    return true;
  }

  theParameters.theLog << "START TIME: " << theSourcePeriod.localStartTime() << endl;
  theParameters.theLog << "END TIME: " << theSourcePeriod.localEndTime() << endl;

  bool overlappingPeriods = false;

  for (unsigned i = 0; i < precipitationPeriods->size(); i++)
  {
    theParameters.theLog << "precipitation period i start: "
                         << precipitationPeriods->at(i).localStartTime() << endl;
    theParameters.theLog << "precipitation period i end: "
                         << precipitationPeriods->at(i).localEndTime() << endl;

    if ((precipitationPeriods->at(i).localStartTime() >= theSourcePeriod.localStartTime() &&
         precipitationPeriods->at(i).localStartTime() <= theSourcePeriod.localEndTime()) ||
        (precipitationPeriods->at(i).localEndTime() >= theSourcePeriod.localStartTime() &&
         precipitationPeriods->at(i).localEndTime() <= theSourcePeriod.localStartTime()))
      overlappingPeriods = true;

    if (i == 0)
    {
      if (precipitationPeriods->at(i).localStartTime() >= theSourcePeriod.localStartTime() &&
          (precipitationPeriods->at(i).localStartTime() <= theSourcePeriod.localEndTime()))
      {
        theDryPeriods.push_back(WeatherPeriod(theSourcePeriod.localStartTime(),
                                              precipitationPeriods->at(i).localStartTime()));
      }
    }
    else
    {
      WeatherPeriod dryPeriod(precipitationPeriods->at(i - 1).localEndTime(),
                              precipitationPeriods->at(i).localStartTime());
      if (dryPeriod.localStartTime() >= theSourcePeriod.localStartTime() &&
          dryPeriod.localStartTime() < theSourcePeriod.localEndTime())
      {
        if (dryPeriod.localEndTime() > theSourcePeriod.localStartTime() &&
            dryPeriod.localEndTime() <= theSourcePeriod.localEndTime())
        {
          theDryPeriods.push_back(dryPeriod);
        }
        else
        {
          theDryPeriods.push_back(
              WeatherPeriod(dryPeriod.localStartTime(), theSourcePeriod.localEndTime()));
        }
      }
    }
  }

  if (!overlappingPeriods) theDryPeriods.push_back(theSourcePeriod);

  return theDryPeriods.size() > 0;
}

bool PrecipitationForecast::getPrecipitationPeriods(
    const WeatherPeriod& theSourcePeriod, vector<WeatherPeriod>& thePrecipitationPeriods) const
{
  const vector<WeatherPeriod>* precipitationPeriods = 0;

  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    precipitationPeriods = &thePrecipitationPeriodsFull;
  else if (theParameters.theForecastArea & INLAND_AREA)
    precipitationPeriods = &thePrecipitationPeriodsInland;
  else if (theParameters.theForecastArea & COASTAL_AREA)
    precipitationPeriods = &thePrecipitationPeriodsCoastal;

  if (precipitationPeriods->size() == 0)
  {
    return false;
  }

  for (unsigned i = 0; i < precipitationPeriods->size(); i++)
  {
    WeatherPeriod notDryPeriod(precipitationPeriods->at(i).localStartTime(),
                               precipitationPeriods->at(i).localEndTime());

    if (notDryPeriod.localStartTime() >= theSourcePeriod.localStartTime() &&
        notDryPeriod.localStartTime() <= theSourcePeriod.localEndTime())
    {
      if (notDryPeriod.localEndTime() >= theSourcePeriod.localStartTime() &&
          notDryPeriod.localEndTime() <= theSourcePeriod.localEndTime())
      {
        thePrecipitationPeriods.push_back(notDryPeriod);
      }
      else
      {
        thePrecipitationPeriods.push_back(
            WeatherPeriod(notDryPeriod.localStartTime(), theSourcePeriod.localEndTime()));
      }
    }
    else if (notDryPeriod.localStartTime() < theSourcePeriod.localStartTime() &&
             notDryPeriod.localEndTime() > theSourcePeriod.localStartTime())
    {
      if (notDryPeriod.localEndTime() <= theSourcePeriod.localEndTime())
      {
        thePrecipitationPeriods.push_back(
            WeatherPeriod(theSourcePeriod.localStartTime(), notDryPeriod.localEndTime()));
      }
      else
      {
        thePrecipitationPeriods.push_back(theSourcePeriod);
      }
    }
  }

  return thePrecipitationPeriods.size() > 0;
}

bool PrecipitationForecast::getIntensityFormExtent(const WeatherPeriod& theWeatherPeriod,
                                                   const unsigned short theForecastArea,
                                                   float& theIntensity,
                                                   precipitation_form_id& theForm,
                                                   float& theExtent) const
{
  const precipitation_data_vector& dataVector = getPrecipitationDataVector(theForecastArea);

  precipitation_form_id precipitationForm = MISSING_PRECIPITATION_FORM;
  theIntensity = getMean(dataVector, PRECIPITATION_MEAN_DATA, theWeatherPeriod);

  for (unsigned int i = 0; i < dataVector.size(); i++)
    if (dataVector.at(i)->theObservationTime >= theWeatherPeriod.localStartTime() &&
        dataVector.at(i)->theObservationTime <= theWeatherPeriod.localEndTime() &&
        dataVector.at(i)->thePrecipitationForm != MISSING_PRECIPITATION_FORM)
    {
      if (precipitationForm == MISSING_PRECIPITATION_FORM)
        precipitationForm = dataVector.at(i)->thePrecipitationForm;
      else if (dataVector.at(i)->thePrecipitationForm < precipitationForm)
        precipitationForm = dataVector.at(i)->thePrecipitationForm;
    }

  theForm = precipitationForm;

  theExtent = getMean(dataVector, PRECIPITATION_EXTENT_DATA, theWeatherPeriod);

  return theForm != MISSING_PRECIPITATION_FORM;
}

bool PrecipitationForecast::isMostlyDryPeriod(const WeatherPeriod& theWeatherPeriod,
                                              const unsigned short theForecastArea) const
{
  const precipitation_data_vector& dataVector = getPrecipitationDataVector(theForecastArea);

  return (getMean(dataVector, PRECIPITATION_EXTENT_DATA, theWeatherPeriod) <=
          theParameters.theMostlyDryWeatherLimit);
}

bool PrecipitationForecast::isDryPeriod(const WeatherPeriod& theWeatherPeriod,
                                        const unsigned short theForecastArea) const
{
  float precipitationIntensity(0.0);
  precipitation_form_id precipitationForm = MISSING_PRECIPITATION_FORM;
  float precipitationExtent(0.0);
  bool dry_weather = true;

  if (getIntensityFormExtent(theWeatherPeriod,
                             theForecastArea,
                             precipitationIntensity,
                             precipitationForm,
                             precipitationExtent))
  {
    dry_weather = is_dry_weather(
        theParameters, precipitationForm, precipitationIntensity, precipitationExtent);
  }

  return dry_weather;
}

float PrecipitationForecast::getStat(const precipitation_data_vector& theData,
                                     const weather_result_data_id& theDataId,
                                     const WeatherPeriod& theWeatherPeriod,
                                     const stat_func_id& theStatFunc) const
{
  float cumulativeSum = 0.0;
  float minValue = 0.0;
  float maxValue = 0.0;
  int count = 0;

  for (unsigned int i = 0; i < theData.size(); i++)
  {
    if (theData[i]->theObservationTime < theWeatherPeriod.localStartTime()) continue;
    if (theData[i]->theObservationTime > theWeatherPeriod.localEndTime()) break;

    switch (theDataId)
    {
      case PRECIPITATION_MAX_DATA:
      {
        if (count == 0 || minValue > theData[i]->thePrecipitationMaxIntensity)
          minValue = theData[i]->thePrecipitationIntensity;
        if (count == 0 || maxValue < theData[i]->thePrecipitationMaxIntensity)
          maxValue = theData[i]->thePrecipitationMaxIntensity;
        cumulativeSum += theData[i]->thePrecipitationMaxIntensity;
        count++;
      }
      break;
      case PRECIPITATION_MEAN_DATA:
      {
        if (count == 0 || minValue > theData[i]->thePrecipitationIntensity)
          minValue = theData[i]->thePrecipitationIntensity;
        if (count == 0 || maxValue < theData[i]->thePrecipitationIntensity)
          maxValue = theData[i]->thePrecipitationIntensity;
        cumulativeSum += theData[i]->thePrecipitationIntensity;
        count++;
      }
      break;
      case PRECIPITATION_EXTENT_DATA:
      {
        if (count == 0 || minValue > theData[i]->thePrecipitationExtent)
          minValue = theData[i]->thePrecipitationExtent;
        if (count == 0 || maxValue < theData[i]->thePrecipitationExtent)
          maxValue = theData[i]->thePrecipitationExtent;
        cumulativeSum += theData[i]->thePrecipitationExtent;
        count++;
      }
      break;
      case PRECIPITATION_FORM_WATER_DATA:
      {
        if (theData[i]->thePrecipitationFormWater != kFloatMissing)
        {
          if (count == 0 || minValue > theData[i]->thePrecipitationFormWater)
            minValue = theData[i]->thePrecipitationFormWater;
          if (count == 0 || maxValue < theData[i]->thePrecipitationFormWater)
            maxValue = theData[i]->thePrecipitationFormWater;
          cumulativeSum += theData[i]->thePrecipitationFormWater;
          count++;
        }
      }
      break;
      case PRECIPITATION_FORM_DRIZZLE_DATA:
      {
        if (theData[i]->thePrecipitationFormDrizzle != kFloatMissing)
        {
          if (count == 0 || minValue > theData[i]->thePrecipitationFormDrizzle)
            minValue = theData[i]->thePrecipitationFormDrizzle;
          if (count == 0 || maxValue < theData[i]->thePrecipitationFormDrizzle)
            maxValue = theData[i]->thePrecipitationFormDrizzle;
          cumulativeSum += theData[i]->thePrecipitationFormDrizzle;
          count++;
        }
      }
      break;
      case PRECIPITATION_FORM_SLEET_DATA:
      {
        if (theData[i]->thePrecipitationFormSleet != kFloatMissing)
        {
          if (count == 0 || minValue > theData[i]->thePrecipitationFormSleet)
            minValue = theData[i]->thePrecipitationFormSleet;
          if (count == 0 || maxValue < theData[i]->thePrecipitationFormSleet)
            maxValue = theData[i]->thePrecipitationFormSleet;
          cumulativeSum += theData[i]->thePrecipitationFormSleet;
          count++;
        }
      }
      break;
      case PRECIPITATION_FORM_SNOW_DATA:
      {
        if (theData[i]->thePrecipitationFormSnow != kFloatMissing)
        {
          if (count == 0 || minValue > theData[i]->thePrecipitationFormSnow)
            minValue = theData[i]->thePrecipitationFormSnow;
          if (count == 0 || maxValue < theData[i]->thePrecipitationFormSnow)
            maxValue = theData[i]->thePrecipitationFormSnow;
          cumulativeSum += theData[i]->thePrecipitationFormSnow;
          count++;
        }
      }
      break;
      case PRECIPITATION_FORM_FREEZING_DATA:
      {
        if (theData[i]->thePrecipitationFormFreezing != kFloatMissing)
        {
          if (count == 0 || minValue > theData[i]->thePrecipitationFormFreezing)
            minValue = theData[i]->thePrecipitationFormFreezing;
          if (count == 0 || maxValue < theData[i]->thePrecipitationFormFreezing)
            maxValue = theData[i]->thePrecipitationFormFreezing;
          cumulativeSum += theData[i]->thePrecipitationFormFreezing;
          count++;
        }
      }
      break;
      case PRECIPITATION_TYPE_DATA:
      {
        if (theData[i]->thePrecipitationTypeShowers != kFloatMissing)
        {
          if (count == 0 || minValue > theData[i]->thePrecipitationTypeShowers)
            minValue = theData[i]->thePrecipitationTypeShowers;
          if (count == 0 || maxValue < theData[i]->thePrecipitationTypeShowers)
            maxValue = theData[i]->thePrecipitationTypeShowers;
          cumulativeSum += theData[i]->thePrecipitationTypeShowers;
          count++;
        }
      }
      break;
      default:
        break;
    }
  }

  float retval = 0.0;
  switch (theStatFunc)
  {
    case MIN:
      retval = minValue;
      break;
    case MAX:
      retval = maxValue;
      break;
    case MEAN:
    {
      if (count > 0) retval = (cumulativeSum / count);
    }
    break;
  }

  return retval;
}

float PrecipitationForecast::getMin(const precipitation_data_vector& theData,
                                    const weather_result_data_id& theDataId,
                                    const WeatherPeriod& theWeatherPeriod) const
{
  return getStat(theData, theDataId, theWeatherPeriod, MIN);
}

float PrecipitationForecast::getMax(const precipitation_data_vector& theData,
                                    const weather_result_data_id& theDataId,
                                    const WeatherPeriod& theWeatherPeriod) const
{
  return getStat(theData, theDataId, theWeatherPeriod, MAX);
}

float PrecipitationForecast::getMean(const precipitation_data_vector& theData,
                                     const weather_result_data_id& theDataId,
                                     const WeatherPeriod& theWeatherPeriod) const
{
  return getStat(theData, theDataId, theWeatherPeriod, MEAN);
}

precipitation_type PrecipitationForecast::getPrecipitationType(
    const WeatherPeriod& thePeriod, const unsigned short theForecastArea) const
{
  const precipitation_data_vector& theData = getPrecipitationDataVector(theForecastArea);

  unsigned int showers_counter(0);
  unsigned int continuous_counter(0);

  for (unsigned int i = 0; i < theData.size(); i++)
  {
    if (theData[i]->theObservationTime < thePeriod.localStartTime()) continue;
    if (theData[i]->theObservationTime > thePeriod.localEndTime()) break;

    if (theData[i]->thePrecipitationType != MISSING_PRECIPITATION_TYPE)
    {
      if (theData[i]->thePrecipitationType == SHOWERS)
        showers_counter++;
      else
        continuous_counter++;
    }
  }

  return (continuous_counter >= showers_counter ? CONTINUOUS : SHOWERS);
}

// Logic:
// 1) save the type of the first hour
// 2) Iterate the precipitation period through hour by hour and check
// if the type changes
// 3) If the type changes (i.e. is different from the original type), check if the new type
// is the prevailing type for the rest of the period
// returns 0, if there is no change, index in the data vector otherwise
unsigned int PrecipitationForecast::getPrecipitationTypeChange(
    const precipitation_data_vector& theData, const WeatherPeriod& thePeriod) const
{
  WeatherPeriod periodToCheck(thePeriod.localStartTime(),
                              thePeriod.localEndTime().GetYear() == 2037
                                  ? theParameters.theForecastPeriod.localEndTime()
                                  : thePeriod.localEndTime());

  bool firstValue = true;
  precipitation_type originalPrecipitationType(MISSING_PRECIPITATION_TYPE);
  for (unsigned int i = 0; i < theData.size(); i++)
  {
    if (theData[i]->theObservationTime < periodToCheck.localStartTime()) continue;
    if (theData[i]->theObservationTime > periodToCheck.localEndTime()) break;

    if (firstValue)
    {
      // store the original type
      originalPrecipitationType = theData[i]->thePrecipitationType;
      firstValue = false;
    }
    else
    {
      // different from the original type
      if (originalPrecipitationType != theData[i]->thePrecipitationType)
      {
        // check the type for the rest of the precipitation period
        for (unsigned int k = i; k < theData.size(); k++)
        {
          // the type changes again
          if (theData[k]->thePrecipitationType != theData[i]->thePrecipitationType) return 0;
        }
        // if type changes in the beginning or in the end ignore it
        if (abs(theData[i]->theObservationTime.DifferenceInHours(periodToCheck.localStartTime())) <
                3 ||
            abs(theData[i]->theObservationTime.DifferenceInHours(periodToCheck.localEndTime())) < 3)
          return 0;

        return i;
      }
    }
  }

  return 0;
}

precipitation_form_id PrecipitationForecast::getPrecipitationForm(
    const WeatherPeriod& thePeriod, const unsigned short theForecastArea) const
{
  const precipitation_data_vector& theDataVector = getPrecipitationDataVector(theForecastArea);

  float precipitationFormWater = getMean(theDataVector, PRECIPITATION_FORM_WATER_DATA, thePeriod);
  float precipitationFormDrizzle =
      getMean(theDataVector, PRECIPITATION_FORM_DRIZZLE_DATA, thePeriod);
  float precipitationFormSleet = getMean(theDataVector, PRECIPITATION_FORM_SLEET_DATA, thePeriod);
  float precipitationFormSnow = getMean(theDataVector, PRECIPITATION_FORM_SNOW_DATA, thePeriod);
  float precipitationFormFreezing =
      getMean(theDataVector, PRECIPITATION_FORM_FREEZING_DATA, thePeriod);

  return get_complete_precipitation_form(theParameters.theVariable,
                                         precipitationFormWater,
                                         precipitationFormDrizzle,
                                         precipitationFormSleet,
                                         precipitationFormSnow,
                                         precipitationFormFreezing);
}

unsigned int PrecipitationForecast::getPrecipitationCategory(const float& thePrecipitation,
                                                             const unsigned int& theType) const
{
  unsigned int retval(DRY_WEATHER_CATEGORY);

  switch (theType)
  {
    case WATER_FORM:
    case WATER_DRIZZLE_FORM:
    case WATER_SLEET_FORM:
    case WATER_SNOW_FORM:
    case WATER_FREEZING_FORM:
    case WATER_DRIZZLE_SLEET_FORM:
    case WATER_DRIZZLE_SNOW_FORM:
    case WATER_DRIZZLE_FREEZING_FORM:
    case WATER_SLEET_SNOW_FORM:
    case WATER_SLEET_FREEZING_FORM:
    case WATER_SNOW_FREEZING_FORM:
    case DRIZZLE_FORM:
    case DRIZZLE_SLEET_FORM:
    case DRIZZLE_SLEET_SNOW_FORM:
    case DRIZZLE_SLEET_FREEZING_FORM:
    case DRIZZLE_FREEZING_FORM:
    case DRIZZLE_SNOW_FORM:
    case DRIZZLE_SNOW_FREEZING_FORM:
    case FREEZING_FORM:
    {
      if (thePrecipitation <= theParameters.theDryWeatherLimitWater)
        retval = DRY_WEATHER_CATEGORY;
      else if (thePrecipitation >= theParameters.theWeakPrecipitationLimitWater &&
               thePrecipitation < theParameters.theHeavyPrecipitationLimitWater)
        retval = MODERATE_WATER_PRECIPITATION;
      else if (thePrecipitation > theParameters.theHeavyPrecipitationLimitWater)
        retval = HEAVY_WATER_PRECIPITATION;
    }
    break;
    case SLEET_FORM:
    case SLEET_FREEZING_FORM:
    case SLEET_SNOW_FREEZING_FORM:
    {
      if (thePrecipitation <= theParameters.theDryWeatherLimitSleet)
        retval = DRY_WEATHER_CATEGORY;
      else if (thePrecipitation >= theParameters.theWeakPrecipitationLimitSleet &&
               thePrecipitation < theParameters.theHeavyPrecipitationLimitSleet)
        retval = MODERATE_WATER_PRECIPITATION;
      else if (thePrecipitation > theParameters.theHeavyPrecipitationLimitSleet)
        retval = HEAVY_WATER_PRECIPITATION;
    }
    break;
    case SNOW_FORM:
    case SNOW_FREEZING_FORM:
    {
      if (thePrecipitation <= theParameters.theDryWeatherLimitSnow)
        retval = DRY_WEATHER_CATEGORY;
      else if (thePrecipitation >= theParameters.theWeakPrecipitationLimitSnow &&
               thePrecipitation < theParameters.theHeavyPrecipitationLimitSnow)
        retval = MODERATE_WATER_PRECIPITATION;
      else if (thePrecipitation > theParameters.theHeavyPrecipitationLimitSnow)
        retval = HEAVY_WATER_PRECIPITATION;
    }
    break;
    default:
      break;
  }

  return retval;
}

// this should be more detailed
bool PrecipitationForecast::reportPrecipitationFormsSeparately(
    const precipitation_form_id& form1, const precipitation_form_id& form2) const
{
  if ((form1 == WATER_FORM &&
       (form2 == SLEET_FORM || form2 == SNOW_FORM || form2 == FREEZING_FORM)) ||
      (form2 == WATER_FORM &&
       (form1 == SLEET_FORM || form1 == SNOW_FORM || form1 == FREEZING_FORM)))
    return true;
  else if ((form1 == SLEET_FORM &&
            (form2 == WATER_FORM || form2 == SNOW_FORM || form2 == FREEZING_FORM)) ||
           (form2 == SLEET_FORM &&
            (form1 == WATER_FORM || form1 == SNOW_FORM || form1 == FREEZING_FORM)))
    return true;
  else if ((form1 == SNOW_FORM &&
            (form2 == WATER_FORM || form2 == SLEET_FORM || form2 == FREEZING_FORM)) ||
           (form2 == SNOW_FORM &&
            (form1 == WATER_FORM || form1 == SLEET_FORM || form1 == FREEZING_FORM)))
    return true;
  else if ((form1 == FREEZING_FORM &&
            (form2 == WATER_FORM || form2 == SLEET_FORM || form2 == SNOW_FORM)) ||
           (form2 == FREEZING_FORM &&
            (form1 == WATER_FORM || form1 == SLEET_FORM || form1 == SNOW_FORM)))
    return true;

  return false;
}

bool PrecipitationForecast::separateCoastInlandPrecipitation(
    const WeatherPeriod& theWeatherPeriod) const
{
  if (!(theParameters.theForecastArea & INLAND_AREA &&
        theParameters.theForecastArea & COASTAL_AREA) ||
      theParameters.theCoastalAndInlandTogetherFlag == true)
    return false;

  float coastalPrecipitation = getMean(theCoastalData, PRECIPITATION_MEAN_DATA, theWeatherPeriod);
  float inlandPrecipitation = getMean(theInlandData, PRECIPITATION_MEAN_DATA, theWeatherPeriod);

  precipitation_form_id coastalPrecipitationForm =
      getPrecipitationForm(theWeatherPeriod, COASTAL_AREA);
  precipitation_form_id inlandPrecipitationForm =
      getPrecipitationForm(theWeatherPeriod, INLAND_AREA);

  bool precipitationFormsDiffer =
      reportPrecipitationFormsSeparately(inlandPrecipitationForm, coastalPrecipitationForm);
  bool retval;

  if (((coastalPrecipitation < 0.04 && inlandPrecipitation > 0.4) ||
       (coastalPrecipitation > 0.4 && inlandPrecipitation < 0.04)) ||
      ((inlandPrecipitation > 0.04 && coastalPrecipitation > 0.04) && precipitationFormsDiffer))
  {
    retval = true;
    theParameters.theLog << "Coastal and Inland precipitation reported separately:" << endl;
  }
  else
  {
    retval = false;
    theParameters.theLog << "Coastal and Inland precipitation reported together:" << endl;
  }

  theParameters.theLog << "Coastal: " << coastalPrecipitation << "("
                       << precipitation_form_string(coastalPrecipitationForm) << ")"
                       << "; Inland: " << inlandPrecipitation << "("
                       << precipitation_form_string(inlandPrecipitationForm) << ")" << endl;

  return retval;
}

void PrecipitationForecast::printOutPrecipitationVector(
    std::ostream& theOutput, const precipitation_data_vector& thePrecipitationDataVector) const
{
  for (unsigned int i = 0; i < thePrecipitationDataVector.size(); i++)
  {
    theOutput << *(thePrecipitationDataVector[i]);
  }
}

void PrecipitationForecast::printOutPrecipitationData(std::ostream& theOutput) const
{
  theOutput << "** PRECIPITATION DATA **" << endl;
  theOutput << "time:**mean intensity**max "
               "intensity**extent**water**drizzle**sleet**snow**freezing**showers**northeast**"
               "southeast**southwest**northwest)"
            << endl;

  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
  {
    theOutput << "Full area precipitation: " << endl;
    printOutPrecipitationVector(theOutput, theFullData);
  }
  if (theParameters.theForecastArea & COASTAL_AREA)
  {
    theOutput << "Coastal precipitation: " << endl;
    printOutPrecipitationVector(theOutput, theCoastalData);
  }
  if (theParameters.theForecastArea & INLAND_AREA)
  {
    theOutput << "Inland precipitation: " << endl;
    printOutPrecipitationVector(theOutput, theInlandData);
  }
}

void PrecipitationForecast::gatherPrecipitationData()
{
  if (theParameters.theForecastArea & COASTAL_AREA) fillInPrecipitationDataVector(COASTAL_AREA);
  if (theParameters.theForecastArea & INLAND_AREA) fillInPrecipitationDataVector(INLAND_AREA);
  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    fillInPrecipitationDataVector(FULL_AREA);
}

void PrecipitationForecast::fillInPrecipitationDataVector(const forecast_area_id& theAreaId)
{
  weather_result_data_item_vector* precipitationMaxHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_MAX_DATA];
  weather_result_data_item_vector* precipitationMeanHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_MEAN_DATA];
  weather_result_data_item_vector* precipitationExtentHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_EXTENT_DATA];
  weather_result_data_item_vector* precipitationFormWaterHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_FORM_WATER_DATA];
  weather_result_data_item_vector* precipitationFormDrizzleHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_FORM_DRIZZLE_DATA];
  weather_result_data_item_vector* precipitationFormSleetHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_FORM_SLEET_DATA];
  weather_result_data_item_vector* precipitationFormSnowHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_FORM_SNOW_DATA];
  weather_result_data_item_vector* precipitationFormFreezingHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_FORM_FREEZING_DATA];
  weather_result_data_item_vector* precipitationTypeHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_TYPE_DATA];
  weather_result_data_item_vector* precipitationNorthEastShareHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_NORTHEAST_SHARE_DATA];
  weather_result_data_item_vector* precipitationSouthEastShareHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_SOUTHEAST_SHARE_DATA];
  weather_result_data_item_vector* precipitationSouthWestShareHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_SOUTHWEST_SHARE_DATA];
  weather_result_data_item_vector* precipitationNorthWestShareHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_NORTHWEST_SHARE_DATA];
  weather_result_data_item_vector* precipitationPointHourly =
      (*theParameters.theCompleteData[theAreaId])[PRECIPITATION_POINT_DATA];

  precipitation_data_vector& dataVector =
      (theAreaId == COASTAL_AREA ? theCoastalData
                                 : (theAreaId == INLAND_AREA ? theInlandData : theFullData));

  for (unsigned int i = 0; i < precipitationMaxHourly->size(); i++)
  {
    float precipitationMaxIntesity = (*precipitationMaxHourly)[i]->theResult.value();
    float precipitationMeanIntesity = (*precipitationMeanHourly)[i]->theResult.value();
    float precipitationExtent = (*precipitationExtentHourly)[i]->theResult.value();
    float precipitationFormWater = (*precipitationFormWaterHourly)[i]->theResult.value();
    float precipitationFormDrizzle = (*precipitationFormDrizzleHourly)[i]->theResult.value();
    float precipitationFormSleet = (*precipitationFormSleetHourly)[i]->theResult.value();
    float precipitationFormSnow = (*precipitationFormSnowHourly)[i]->theResult.value();
    float precipitationFormFreezing = (*precipitationFormFreezingHourly)[i]->theResult.value();
    float precipitationTypeShowers = (*precipitationTypeHourly)[i]->theResult.value();

    const precipitation_form_id precipitationForm =
        get_complete_precipitation_form(theParameters.theVariable,
                                        precipitationFormWater,
                                        precipitationFormDrizzle,
                                        precipitationFormSleet,
                                        precipitationFormSnow,
                                        precipitationFormFreezing);

    PrecipitationDataItemData* item =
        new PrecipitationDataItemData(theParameters,
                                      precipitationForm,
                                      (precipitationMaxIntesity + precipitationMeanIntesity) / 2.0,
                                      precipitationMaxIntesity,
                                      precipitationExtent,
                                      precipitationFormWater,
                                      precipitationFormDrizzle,
                                      precipitationFormSleet,
                                      precipitationFormSnow,
                                      precipitationFormFreezing,
                                      precipitationTypeShowers,
                                      MISSING_WEATHER_EVENT,
                                      0.0,
                                      (*precipitationMaxHourly)[i]->thePeriod.localStartTime());

    item->thePrecipitationPercentageNorthEast =
        (*precipitationNorthEastShareHourly)[i]->theResult.value();
    item->thePrecipitationPercentageSouthEast =
        (*precipitationSouthEastShareHourly)[i]->theResult.value();
    item->thePrecipitationPercentageSouthWest =
        (*precipitationSouthWestShareHourly)[i]->theResult.value();
    item->thePrecipitationPercentageNorthWest =
        (*precipitationNorthWestShareHourly)[i]->theResult.value();
    item->thePrecipitationPoint.X((*precipitationPointHourly)[i]->theResult.value());
    item->thePrecipitationPoint.Y((*precipitationPointHourly)[i]->theResult.error());

    dataVector.push_back(item);
  }
}

void PrecipitationForecast::findOutPrecipitationPeriods()
{
  if (theParameters.theForecastArea & COASTAL_AREA) findOutPrecipitationPeriods(COASTAL_AREA);
  if (theParameters.theForecastArea & INLAND_AREA) findOutPrecipitationPeriods(INLAND_AREA);
  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    findOutPrecipitationPeriods(FULL_AREA);
}

// join periods if there is only one hour "dry" period between
// and if type stays the same
void PrecipitationForecast::joinPrecipitationPeriods(
    vector<WeatherPeriod>& thePrecipitationPeriodVector)
{
  if (thePrecipitationPeriodVector.size() < 2) return;

  for (unsigned int i = thePrecipitationPeriodVector.size() - 1; i > 0; i--)
  {
    TextGenPosixTime currentPeriodStartTime(thePrecipitationPeriodVector.at(i).localStartTime());
    TextGenPosixTime previousPeriodEndTime(thePrecipitationPeriodVector.at(i - 1).localEndTime());

    if (currentPeriodStartTime.DifferenceInHours(previousPeriodEndTime) <= 2)
    {
      WeatherPeriod joinedPeriod(thePrecipitationPeriodVector.at(i - 1).localStartTime(),
                                 thePrecipitationPeriodVector.at(i).localEndTime());

      if (isDryPeriod(joinedPeriod, theParameters.theForecastArea)) continue;

      theParameters.theLog << "Joining precipitation periods: "
                           << thePrecipitationPeriodVector.at(i - 1).localStartTime() << "..."
                           << thePrecipitationPeriodVector.at(i - 1).localEndTime() << " and "
                           << thePrecipitationPeriodVector.at(i).localStartTime() << "..."
                           << thePrecipitationPeriodVector.at(i).localEndTime();

      thePrecipitationPeriodVector.erase(thePrecipitationPeriodVector.begin() + i);
      thePrecipitationPeriodVector.erase(thePrecipitationPeriodVector.begin() + i - 1);
      thePrecipitationPeriodVector.insert(thePrecipitationPeriodVector.begin() + i - 1,
                                          joinedPeriod);
    }
  }
}

void PrecipitationForecast::findOutPrecipitationPeriods(const forecast_area_id& theAreaId)
{
  precipitation_data_vector* dataSourceVector = 0;
  vector<WeatherPeriod>* dataDestinationVector = 0;

  if (theAreaId & FULL_AREA)
  {
    dataSourceVector = &theFullData;
    dataDestinationVector = &thePrecipitationPeriodsFull;
  }
  else if (theAreaId & INLAND_AREA)
  {
    dataSourceVector = &theInlandData;
    dataDestinationVector = &thePrecipitationPeriodsInland;
  }
  else if (theAreaId & COASTAL_AREA)
  {
    dataSourceVector = &theCoastalData;
    dataDestinationVector = &thePrecipitationPeriodsCoastal;
  }

  if (!dataSourceVector) return;

  bool isDryPrevious = true;
  bool isDryCurrent = true;

  unsigned int periodStartIndex = 0;
  for (unsigned int i = 1; i < dataSourceVector->size(); i++)
  {
    isDryPrevious = is_dry_weather(theParameters,
                                   (*dataSourceVector)[i - 1]->thePrecipitationForm,
                                   (*dataSourceVector)[i - 1]->thePrecipitationIntensity,
                                   (*dataSourceVector)[i - 1]->thePrecipitationExtent);
    isDryCurrent = is_dry_weather(theParameters,
                                  (*dataSourceVector)[i]->thePrecipitationForm,
                                  (*dataSourceVector)[i]->thePrecipitationIntensity,
                                  (*dataSourceVector)[i]->thePrecipitationExtent);
    if (isDryPrevious != isDryCurrent)
    {
      if (!isDryPrevious)
      {
        TextGenPosixTime startTime((*dataSourceVector)[periodStartIndex]->theObservationTime);
        if (periodStartIndex == 0)
          startTime.SetDate(1970, 1, 1);  // precipitation starts before forecast period
        TextGenPosixTime endTime((*dataSourceVector)[i - 1]->theObservationTime);
        dataDestinationVector->push_back(WeatherPeriod(startTime, endTime));
      }
      periodStartIndex = i;
    }
  }
  if (!isDryPrevious && periodStartIndex != dataSourceVector->size() - 1)
  {
    TextGenPosixTime startTime((*dataSourceVector)[periodStartIndex]->theObservationTime);
    if (periodStartIndex == 0)
      startTime.SetDate(1970, 1, 1);  // precipitation starts before forecast period
    TextGenPosixTime endTime((*dataSourceVector)[dataSourceVector->size() - 1]->theObservationTime);
    if (endTime == theParameters.theForecastPeriod.localEndTime())
      endTime.SetDate(2037, 1, 1);  // precipitation continues when forecast period ends

    // If type changes, split one period into two
    WeatherPeriod precipitationPeriod(startTime, endTime);
    unsigned int typeChangeIndex =
        getPrecipitationTypeChange(*dataSourceVector, precipitationPeriod);
    if (typeChangeIndex > 0)
    {
      theParameters.theLog << "Split one precipitation period into two, because type changes at ";
      theParameters.theLog << (*dataSourceVector)[typeChangeIndex]->theObservationTime;
      if ((*dataSourceVector)[typeChangeIndex]->thePrecipitationType == SHOWERS)
        theParameters.theLog << " from continuous to showers";
      else
        theParameters.theLog << " from showers to continuous";

      WeatherPeriod period1(startTime,
                            (*dataSourceVector)[typeChangeIndex - 1]->theObservationTime);
      WeatherPeriod period2((*dataSourceVector)[typeChangeIndex]->theObservationTime, endTime);
      dataDestinationVector->push_back(period1);
      dataDestinationVector->push_back(period2);
    }
    else
    {
      dataDestinationVector->push_back(precipitationPeriod);
    }
  }
  //	joinPrecipitationPeriods(*dataDestinationVector);
}

void PrecipitationForecast::findOutPrecipitationWeatherEvents()
{
  if (theParameters.theForecastArea & COASTAL_AREA)
    findOutPrecipitationWeatherEvents(
        thePrecipitationPeriodsCoastal, COASTAL_AREA, thePrecipitationWeatherEventsCoastal);
  if (theParameters.theForecastArea & INLAND_AREA)
    findOutPrecipitationWeatherEvents(
        thePrecipitationPeriodsInland, INLAND_AREA, thePrecipitationWeatherEventsInland);
  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    findOutPrecipitationWeatherEvents(
        thePrecipitationPeriodsFull, FULL_AREA, thePrecipitationWeatherEventsFull);
}

void PrecipitationForecast::findOutPrecipitationWeatherEvents(
    const vector<WeatherPeriod>& thePrecipitationPeriods,
    const unsigned short theForecastArea,
    weather_event_id_vector& thePrecipitationWeatherEvents)
{
  for (unsigned int i = 0; i < thePrecipitationPeriods.size(); i++)
  {
    TextGenPosixTime precipitationStartTime(thePrecipitationPeriods.at(i).localStartTime());
    TextGenPosixTime precipitationEndTime(thePrecipitationPeriods.at(i).localEndTime());
    if (precipitationEndTime.DifferenceInHours(precipitationStartTime) >= 6)
    {
      // start year 1970 indicates that rain starts on previous period
      if (precipitationStartTime.GetYear() != 1970 &&
          precipitationStartTime >= theParameters.theForecastPeriod.localStartTime() &&
          precipitationStartTime <= theParameters.theForecastPeriod.localEndTime())
      {
        thePrecipitationWeatherEvents.push_back(make_pair(precipitationStartTime, SADE_ALKAA));
        // check here if type cahanes during the period (from continuous to showers or vice versa)
      }

      if (precipitationEndTime.GetYear() != 2037 &&
          precipitationEndTime >= theParameters.theForecastPeriod.localStartTime() &&
          precipitationEndTime <= theParameters.theForecastPeriod.localEndTime())
      {
        bool mahdollisestiPoutaantuu = true;
        if (i < thePrecipitationPeriods.size() - 1)
        {
          if (thePrecipitationPeriods.at(i + 1).localStartTime().DifferenceInHours(
                  precipitationEndTime) <= 2)
            mahdollisestiPoutaantuu = false;
        }

        if (mahdollisestiPoutaantuu)
        {
          if (getPrecipitationExtent(thePrecipitationPeriods.at(i), theForecastArea) >
              MOSTLY_DRY_WEATHER_LIMIT)
            thePrecipitationWeatherEvents.push_back(make_pair(precipitationEndTime, POUTAANTUU));
          else
            thePrecipitationWeatherEvents.push_back(
                make_pair(precipitationEndTime, POUTAANTUU_WHEN_EXTENT_SMALL));
        }
      }
    }
  }
}

float PrecipitationForecast::getPrecipitationExtent(const WeatherPeriod& thePeriod,
                                                    const unsigned short theForecastArea) const
{
  const precipitation_data_vector& dataVector = getPrecipitationDataVector(theForecastArea);

  float extent = getMean(dataVector, PRECIPITATION_EXTENT_DATA, thePeriod);

  return extent;
}

float PrecipitationForecast::getMaxIntensity(const WeatherPeriod& thePeriod,
                                             const unsigned short theForecastArea) const
{
  const precipitation_data_vector& dataVector = getPrecipitationDataVector(theForecastArea);

  float intensity = getMean(dataVector, PRECIPITATION_MAX_DATA, thePeriod);

  return intensity;
}

float PrecipitationForecast::getMeanIntensity(const WeatherPeriod& thePeriod,
                                              const unsigned short theForecastArea) const
{
  const precipitation_data_vector& dataVector = getPrecipitationDataVector(theForecastArea);

  float intensity = getMean(dataVector, PRECIPITATION_MEAN_DATA, thePeriod);

  return intensity;
}

void PrecipitationForecast::removeRedundantWeatherEvents(
    weather_event_id_vector& thePrecipitationWeatherEvents, const vector<int>& theRemoveIndexes)
{
  if (theRemoveIndexes.size() > 0)
  {
    for (unsigned int i = theRemoveIndexes.size(); i > 0; i--)
    {
      thePrecipitationWeatherEvents.erase(thePrecipitationWeatherEvents.begin() +
                                          theRemoveIndexes[i - 1]);
    }
  }
}

void PrecipitationForecast::removeDuplicatePrecipitationWeatherEvents(
    weather_event_id_vector& thePrecipitationWeatherEvents)
{
  vector<int> removeIndexes;

  for (unsigned int i = 1; i < thePrecipitationWeatherEvents.size(); i++)
  {
    weather_event_id previousPeriodWeatherEventId(thePrecipitationWeatherEvents[i - 1].second);
    weather_event_id currentPeriodWeatherEventId(thePrecipitationWeatherEvents[i].second);
    if (previousPeriodWeatherEventId == currentPeriodWeatherEventId) removeIndexes.push_back(i);
  }
  removeRedundantWeatherEvents(thePrecipitationWeatherEvents, removeIndexes);
}

// take into account the continuous precipitation periods
void PrecipitationForecast::cleanUpPrecipitationWeatherEvents(
    weather_event_id_vector& thePrecipitationWeatherEvents)
{
  vector<int> removeIndexes;

  for (unsigned int i = 1; i < thePrecipitationWeatherEvents.size(); i++)
  {
    TextGenPosixTime previousPeriodEndTime(thePrecipitationWeatherEvents[i - 1].first);
    TextGenPosixTime currentPeriodStartTime(thePrecipitationWeatherEvents[i].first);
    weather_event_id previousPeriodWeatherEventId(thePrecipitationWeatherEvents[i - 1].second);
    weather_event_id currentPeriodWeatherEventId(thePrecipitationWeatherEvents[i].second);
    if (abs(previousPeriodEndTime.DifferenceInHours(currentPeriodStartTime)) < 2 &&
        previousPeriodWeatherEventId == SADE_ALKAA &&
        (currentPeriodWeatherEventId == POUTAANTUU ||
         currentPeriodWeatherEventId == POUTAANTUU_WHEN_EXTENT_SMALL))
      removeIndexes.push_back(i - 1);
  }
  removeRedundantWeatherEvents(thePrecipitationWeatherEvents, removeIndexes);
  removeDuplicatePrecipitationWeatherEvents(thePrecipitationWeatherEvents);
}

void PrecipitationForecast::printOutPrecipitationDistribution(std::ostream& theOutput) const
{
  theOutput << "** PRECIPITATION DISTRIBUTION **" << endl;
  if (theCoastalData.size() > 0)
  {
    theOutput << "Coastal distribution: " << endl;
    printOutPrecipitationDistribution(theOutput, theCoastalData);
  }
  if (theInlandData.size() > 0)
  {
    theOutput << "Inland distribution: " << endl;
    printOutPrecipitationDistribution(theOutput, theInlandData);
  }
  if (theFullData.size() > 0)
  {
    theOutput << "Full area distribution: " << endl;
    printOutPrecipitationDistribution(theOutput, theFullData);
  }
}

void PrecipitationForecast::printOutPrecipitationDistribution(
    std::ostream& theOutput, const precipitation_data_vector& theDataVector) const
{
  for (unsigned int i = 0; i < theDataVector.size(); i++)
  {
    theOutput << "distribution(ne,se,sw,nw,n,s,e,w): " << theDataVector[i]->theObservationTime
              << ", " << theDataVector[i]->thePrecipitationPercentageNorthEast << ", "
              << theDataVector[i]->thePrecipitationPercentageSouthEast << ", "
              << theDataVector[i]->thePrecipitationPercentageSouthWest << ", "
              << theDataVector[i]->thePrecipitationPercentageNorthWest << ", "
              << theDataVector[i]->precipitationPercentageNorth() << ", "
              << theDataVector[i]->precipitationPercentageSouth() << ", "
              << theDataVector[i]->precipitationPercentageEast() << ", "
              << theDataVector[i]->precipitationPercentageWest() << endl;
  }
}

void PrecipitationForecast::printOutPrecipitationWeatherEvents(std::ostream& theOutput) const
{
  theOutput << "** PRECIPITATION WEATHER EVENTS **" << endl;
  bool isWeatherEvents = false;
  if (thePrecipitationWeatherEventsCoastal.size() > 0)
  {
    theOutput << "Coastal precipitation weather events: " << endl;
    print_out_weather_event_vector(theOutput, thePrecipitationWeatherEventsCoastal);
    isWeatherEvents = true;
  }
  if (thePrecipitationWeatherEventsInland.size() > 0)
  {
    theOutput << "Inland precipitation weather events: " << endl;
    print_out_weather_event_vector(theOutput, thePrecipitationWeatherEventsInland);
    isWeatherEvents = true;
  }
  if (thePrecipitationWeatherEventsFull.size() > 0)
  {
    theOutput << "Full area precipitation weather events: " << endl;
    print_out_weather_event_vector(theOutput, thePrecipitationWeatherEventsFull);
    isWeatherEvents = true;
  }

  if (!isWeatherEvents) theOutput << "No weather events!" << endl;
}

void PrecipitationForecast::printOutPrecipitationPeriods(std::ostream& theOutput,
                                                         const bool& isPoint) const
{
  theOutput << "** PRECIPITATION PERIODS **" << endl;
  bool found = false;
  if (thePrecipitationPeriodsCoastal.size() > 0)
  {
    theOutput << "Coastal precipitation periods: " << endl;
    found = printOutPrecipitationPeriods(
        theOutput, thePrecipitationPeriodsCoastal, theCoastalData, isPoint);
  }
  if (thePrecipitationPeriodsInland.size() > 0)
  {
    theOutput << "Inland precipitation periods: " << endl;
    found = printOutPrecipitationPeriods(
        theOutput, thePrecipitationPeriodsInland, theInlandData, isPoint);
  }
  if (thePrecipitationPeriodsFull.size() > 0)
  {
    theOutput << "Full precipitation periods: " << endl;
    found =
        printOutPrecipitationPeriods(theOutput, thePrecipitationPeriodsFull, theFullData, isPoint);
  }

  if (!found) theOutput << "No precipitation periods!" << endl;
}

bool PrecipitationForecast::printOutPrecipitationPeriods(
    std::ostream& theOutput,
    const vector<WeatherPeriod>& thePrecipitationPeriods,
    const precipitation_data_vector& theDataVector,
    const bool& isPoint) const
{
  bool retval = false;

  for (unsigned int i = 0; i < thePrecipitationPeriods.size(); i++)
  {
    bool intersectionPeriodFound(false);
    WeatherPeriod period = get_intersection_period(
        thePrecipitationPeriods.at(i), theParameters.theForecastPeriod, intersectionPeriodFound);

    if (!intersectionPeriodFound)
    {
      continue;
    }
    theOutput << period.localStartTime() << "..." << period.localEndTime() << endl;

    theOutput  //<< ": "
        << " min_of_max = " << setw(7) << setfill(' ') << setprecision(3)
        << getMin(theDataVector, PRECIPITATION_MAX_DATA, period) << endl
        << " min_of_mean = " << setw(7) << setfill(' ') << setprecision(3)
        << getMin(theDataVector, PRECIPITATION_MEAN_DATA, period) << endl
        << " mean_of_max = " << setw(7) << setfill(' ') << setprecision(3)
        << getMean(theDataVector, PRECIPITATION_MAX_DATA, period) << endl
        << " mean_of_mean = " << setw(7) << setfill(' ') << setprecision(3)
        << getMean(theDataVector, PRECIPITATION_MEAN_DATA, period) << endl
        << " max_of_max = " << setprecision(3)
        << getMax(theDataVector, PRECIPITATION_MAX_DATA, period) << endl
        << " max_of_mean = " << setprecision(3)
        << getMax(theDataVector, PRECIPITATION_MEAN_DATA, period) << endl;

    if (!isPoint)
    {
      precipitation_traverse_id traverseId = getPrecipitationTraverseId(period);
      if (traverseId != MISSING_TRAVERSE_ID)
      {
        theOutput << " movement=" << precipitation_traverse_string(traverseId);
      }
    }

    theOutput << endl;
    retval = true;
  }

  return retval;
}

void PrecipitationForecast::printOutPrecipitationDistribution(std::ostream& theOutput,
                                                              const WeatherPeriod& thePeriod)
{
  //	if(!isDryPeriod(thePeriod, theParameters.theForecastArea))
  {
    float northPercentage(0.0);
    float southPercentage(0.0);
    float eastPercentage(0.0);
    float westPercentage(0.0);
    float northEastPercentage(0.0);
    float southEastPercentage(0.0);
    float southWestPercentage(0.0);
    float northWestPercentage(0.0);

    getPrecipitationDistribution(thePeriod,
                                 northPercentage,
                                 southPercentage,
                                 eastPercentage,
                                 westPercentage,
                                 northEastPercentage,
                                 southEastPercentage,
                                 southWestPercentage,
                                 northWestPercentage);

    theOutput << "precipitation north: " << northPercentage << endl;
    theOutput << "precipitation south: " << southPercentage << endl;
    theOutput << "precipitation east: " << eastPercentage << endl;
    theOutput << "precipitation west: " << westPercentage << endl;
    theOutput << "precipitation northeast: " << northEastPercentage << endl;
    theOutput << "precipitation southeast: " << southEastPercentage << endl;
    theOutput << "precipitation southwest: " << southWestPercentage << endl;
    theOutput << "precipitation northwest: " << northWestPercentage << endl;
  }
}

bool PrecipitationForecast::getPrecipitationPeriod(const TextGenPosixTime& theTimestamp,
                                                   TextGenPosixTime& theStartTime,
                                                   TextGenPosixTime& theEndTime) const
{
  const vector<WeatherPeriod>* precipitationPeriodVector = 0;

  if (theParameters.theForecastArea & FULL_AREA)
    precipitationPeriodVector = &thePrecipitationPeriodsFull;
  else if (theParameters.theForecastArea & COASTAL_AREA)
    precipitationPeriodVector = &thePrecipitationPeriodsCoastal;
  else if (theParameters.theForecastArea & INLAND_AREA)
    precipitationPeriodVector = &thePrecipitationPeriodsInland;

  if (!precipitationPeriodVector) return false;

  for (unsigned int i = 0; i < precipitationPeriodVector->size(); i++)
  {
    if (is_inside(theTimestamp, precipitationPeriodVector->at(i)))
    {
      theStartTime = precipitationPeriodVector->at(i).localStartTime();
      theEndTime = precipitationPeriodVector->at(i).localEndTime();
      //			theWeatherPeriod = precipitationPeriodVector->at(i);
      return true;
    }
  }
  return false;
}
Sentence PrecipitationForecast::precipitationPoutaantuuAndCloudiness(
    const Sentence& thePeriodPhrase, const cloudiness_id& theCloudinessId) const
{
  Sentence sentence;

  precipitation_form_id previousPrecipitationForm = getPoutaantuuPrecipitationForm();

  if (theCloudinessId == PUOLIPILVINEN_JA_PILVINEN)
  {
    switch (previousPrecipitationForm)
    {
      case DRIZZLE_FORM:
        sentence << SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_VAIHTELEE_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SLEET_FORM:
        sentence << SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_VAIHTELEE_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SNOW_FORM:
        sentence << SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_VAIHTELEE_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      default:
        sentence << SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_VAIHTELEE_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
    }
  }
  else if (theCloudinessId == PILVINEN)
  {
    switch (previousPrecipitationForm)
    {
      case DRIZZLE_FORM:
        sentence << SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_PILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SLEET_FORM:
        sentence << SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_PILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SNOW_FORM:
        sentence << SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_PILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      default:
        sentence << SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_PILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
    }
  }
  else if (theCloudinessId == VERRATTAIN_PILVINEN)
  {
    switch (previousPrecipitationForm)
    {
      case DRIZZLE_FORM:
        sentence << SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_VERRATTAIN_PILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SLEET_FORM:
        sentence << SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_VERRATTAIN_PILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SNOW_FORM:
        sentence << SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_VERRATTAIN_PILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      default:
        sentence << SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_VERRATTAIN_PILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
    }
  }
  else if (theCloudinessId == PUOLIPILVINEN)
  {
    switch (previousPrecipitationForm)
    {
      case DRIZZLE_FORM:
        sentence << SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_PUOLIPILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SLEET_FORM:
        sentence << SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_PUOLIPILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SNOW_FORM:
        sentence << SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_PUOLIPILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      default:
        sentence << SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_PUOLIPILVINEN_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
    }
  }
  else if (theCloudinessId == MELKO_SELKEA)
  {
    switch (previousPrecipitationForm)
    {
      case DRIZZLE_FORM:
        sentence << SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_MELKO_SELKEA_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SLEET_FORM:
        sentence << SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_MELKO_SELKEA_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SNOW_FORM:
        sentence << SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_MELKO_SELKEA_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      default:
        sentence << SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_MELKO_SELKEA_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
    }
  }
  else if (theCloudinessId == SELKEA)
  {
    switch (previousPrecipitationForm)
    {
      case DRIZZLE_FORM:
        sentence << SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_JA_ON_SELKEA_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SLEET_FORM:
        sentence << SAA_POUTAANTUU_RANTASATEEN_JALKEEN_JA_ON_SELKEA_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      case SNOW_FORM:
        sentence << SAA_POUTAANTUU_LUMISATEEN_JALKEEN_JA_ON_SELKEA_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
      default:
        sentence << SAA_POUTAANTUU_VESISATEEN_JALKEEN_JA_ON_SELKEA_COMPOSITE_PHRASE
                 << thePeriodPhrase;
        break;
    }
  }

  setDryPeriodTautologyFlag(true);

  return sentence;
}

Sentence PrecipitationForecast::precipitationChangeSentence(
    const WeatherPeriod& thePeriod,
    const Sentence& thePeriodPhrase,
    const weather_event_id& theWeatherEvent,
    std::vector<Sentence>& theAdditionalSentences) const
{
  Sentence sentence;

  if (theWeatherEvent == POUTAANTUU || theWeatherEvent == POUTAANTUU_WHEN_EXTENT_SMALL)
  {
    switch (getPoutaantuuPrecipitationForm())
    {
      case DRIZZLE_FORM:
        sentence << SAA_POUTAANTUU_TIHKUSATEEN_JALKEEN_COMPOSITE_PHRASE << thePeriodPhrase;
        break;
      case SLEET_FORM:
        sentence << SAA_POUTAANTUU_RANTASATEEN_JALKEEN_COMPOSITE_PHRASE << thePeriodPhrase;
        break;
      case SNOW_FORM:
        sentence << SAA_POUTAANTUU_LUMISATEEN_JALKEEN_COMPOSITE_PHRASE << thePeriodPhrase;
        break;
      default:
        sentence << SAA_POUTAANTUU_VESISATEEN_JALKEEN_COMPOSITE_PHRASE << thePeriodPhrase;
        break;
    }

    theDryPeriodTautologyFlag = true;
  }
  else  // sade alkaa
  {
    sentence << precipitationSentence(thePeriod, thePeriodPhrase, theAdditionalSentences);
  }

  return sentence;
}

void PrecipitationForecast::getPrecipitationDistribution(const WeatherPeriod& thePeriod,
                                                         float& theNorthPercentage,
                                                         float& theSouthPercentage,
                                                         float& theEastPercentage,
                                                         float& theWestPercentage,
                                                         float& theNorthEastPercentage,
                                                         float& theSouthEastPercentage,
                                                         float& theSouthWestPercentage,
                                                         float& theNorthWestPercentage) const
{
  const precipitation_data_vector* precipitationDataVector =
      ((theParameters.theForecastArea & FULL_AREA)
           ? &theFullData
           : ((theParameters.theForecastArea & INLAND_AREA) ? &theInlandData : &theCoastalData));

  theNorthPercentage = 0.0;
  theSouthPercentage = 0.0;
  theEastPercentage = 0.0;
  theWestPercentage = 0.0;
  theNorthEastPercentage = 0.0;
  theSouthEastPercentage = 0.0;
  theSouthWestPercentage = 0.0;
  theNorthWestPercentage = 0.0;

  unsigned int count = 0;
  for (unsigned int i = 0; i < precipitationDataVector->size(); i++)
  {
    if (precipitationDataVector->at(i)->thePrecipitationIntensity > 0)
    {
      theNorthEastPercentage += precipitationDataVector->at(i)->thePrecipitationPercentageNorthEast;
      theSouthEastPercentage += precipitationDataVector->at(i)->thePrecipitationPercentageSouthEast;
      theSouthWestPercentage += precipitationDataVector->at(i)->thePrecipitationPercentageSouthWest;
      theNorthWestPercentage += precipitationDataVector->at(i)->thePrecipitationPercentageNorthWest;
      count++;
    }
  }
  if (count > 0)
  {
    theNorthEastPercentage /= count;
    theSouthEastPercentage /= count;
    theSouthWestPercentage /= count;
    theNorthWestPercentage /= count;
    theNorthPercentage = (theNorthEastPercentage + theNorthWestPercentage);
    theSouthPercentage = (theSouthEastPercentage + theSouthWestPercentage);
    theEastPercentage = (theNorthEastPercentage + theSouthEastPercentage);
    theWestPercentage = (theSouthEastPercentage + theSouthWestPercentage);
  }
}

precipitation_form_transformation_id PrecipitationForecast::getPrecipitationTransformationId(
    const WeatherPeriod& thePeriod, const unsigned short theForecastArea) const
{
  if (thePeriod.localEndTime().DifferenceInHours(thePeriod.localStartTime()) < 5)
    return NO_FORM_TRANSFORMATION;

  TextGenPosixTime atStartBeg(thePeriod.localStartTime());
  if (atStartBeg.GetYear() == 1970) atStartBeg = theParameters.theForecastPeriod.localStartTime();
  TextGenPosixTime atStartEnd(atStartBeg);
  atStartEnd.ChangeByHours(2);
  WeatherPeriod atStartPeriod(atStartBeg, atStartEnd);
  TextGenPosixTime atEndBeg(thePeriod.localEndTime());
  if (atEndBeg.GetYear() == 2037) atEndBeg = theParameters.theForecastPeriod.localEndTime();
  TextGenPosixTime atEndEnd(atEndBeg);
  atEndBeg.ChangeByHours(-2);
  WeatherPeriod atEndPeriod(atEndBeg, atEndEnd);

  const precipitation_data_vector& theDataVector = getPrecipitationDataVector(theForecastArea);

  float precipitationFormWaterBeg =
      getMean(theDataVector, PRECIPITATION_FORM_WATER_DATA, WeatherPeriod(atStartBeg, atStartBeg));
  float precipitationFormDrizzleBeg = getMean(
      theDataVector, PRECIPITATION_FORM_DRIZZLE_DATA, WeatherPeriod(atStartBeg, atStartBeg));
  float precipitationFormSleetBeg =
      getMean(theDataVector, PRECIPITATION_FORM_SLEET_DATA, WeatherPeriod(atStartBeg, atStartBeg));
  float precipitationFormSnowBeg =
      getMean(theDataVector, PRECIPITATION_FORM_SNOW_DATA, WeatherPeriod(atStartBeg, atStartBeg));
  float precipitationFormWaterEnd =
      getMean(theDataVector, PRECIPITATION_FORM_WATER_DATA, WeatherPeriod(atEndEnd, atEndEnd));
  float precipitationFormDrizzleEnd =
      getMean(theDataVector, PRECIPITATION_FORM_DRIZZLE_DATA, WeatherPeriod(atEndEnd, atEndEnd));
  float precipitationFormSleetEnd =
      getMean(theDataVector, PRECIPITATION_FORM_SLEET_DATA, WeatherPeriod(atEndEnd, atEndEnd));
  float precipitationFormSnowEnd =
      getMean(theDataVector, PRECIPITATION_FORM_SNOW_DATA, WeatherPeriod(atEndEnd, atEndEnd));

  precipitation_form_id precipitationFormBeg(MISSING_PRECIPITATION_FORM);
  precipitation_form_id precipitationFormEnd(MISSING_PRECIPITATION_FORM);

  if (precipitationFormWaterBeg >= 90)
    precipitationFormBeg = WATER_FORM;
  else if (precipitationFormDrizzleBeg >= 90)
    precipitationFormBeg = DRIZZLE_FORM;
  else if (precipitationFormSleetBeg >= 90)
    precipitationFormBeg = SLEET_FORM;
  else if (precipitationFormSnowBeg >= 90)
    precipitationFormBeg = SNOW_FORM;

  if (precipitationFormWaterEnd >= 90)
    precipitationFormEnd = WATER_FORM;
  else if (precipitationFormDrizzleEnd >= 90)
    precipitationFormEnd = DRIZZLE_FORM;
  else if (precipitationFormSleetEnd >= 90)
    precipitationFormEnd = SLEET_FORM;
  else if (precipitationFormSnowEnd >= 90)
    precipitationFormEnd = SNOW_FORM;

  if (precipitationFormBeg == precipitationFormEnd) return NO_FORM_TRANSFORMATION;

  const weather_result_data_item_vector* precipitationFormWaterHourly =
      get_data_vector(theParameters, PRECIPITATION_FORM_WATER_DATA);
  const weather_result_data_item_vector* precipitationFormSnowHourly =
      get_data_vector(theParameters, PRECIPITATION_FORM_SNOW_DATA);
  const weather_result_data_item_vector* precipitationFormDrizzleHourly =
      get_data_vector(theParameters, PRECIPITATION_FORM_DRIZZLE_DATA);
  const weather_result_data_item_vector* precipitationFormSleetHourly =
      get_data_vector(theParameters, PRECIPITATION_FORM_SLEET_DATA);

  unsigned int startIndex, endIndex;
  get_period_start_end_index(thePeriod, *precipitationFormWaterHourly, startIndex, endIndex);

  double pearson_co_water =
      get_pearson_coefficient(*precipitationFormWaterHourly, startIndex, endIndex);
  double pearson_co_snow =
      get_pearson_coefficient(*precipitationFormSnowHourly, startIndex, endIndex);
  double pearson_co_drizzle =
      get_pearson_coefficient(*precipitationFormDrizzleHourly, startIndex, endIndex);
  double pearson_co_sleet =
      get_pearson_coefficient(*precipitationFormSleetHourly, startIndex, endIndex);

  if (precipitationFormBeg == WATER_FORM)
  {
    if (precipitationFormEnd == SNOW_FORM)
    {
      if (pearson_co_water <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_snow >= PEARSON_CO_FORM_TRANSFORM)
        return WATER_TO_SNOW;
    }
    else if (precipitationFormEnd == DRIZZLE_FORM)
    {
      if (pearson_co_water <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_drizzle >= PEARSON_CO_FORM_TRANSFORM)
        return WATER_TO_DRIZZLE;
    }
    else if (precipitationFormEnd == SLEET_FORM)
    {
      if (pearson_co_water <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_sleet >= PEARSON_CO_FORM_TRANSFORM)
        return WATER_TO_SLEET;
    }
  }
  else if (precipitationFormBeg == DRIZZLE_FORM)
  {
    if (precipitationFormEnd == SNOW_FORM)
    {
      if (pearson_co_drizzle <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_snow >= PEARSON_CO_FORM_TRANSFORM)
        return DRIZZLE_TO_SNOW;
    }
    else if (precipitationFormEnd == WATER_FORM)
    {
      if (pearson_co_drizzle <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_water >= PEARSON_CO_FORM_TRANSFORM)
        return DRIZZLE_TO_WATER;
    }
    else if (precipitationFormEnd == SLEET_FORM)
    {
      if (pearson_co_drizzle <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_sleet >= PEARSON_CO_FORM_TRANSFORM)
        return DRIZZLE_TO_SLEET;
    }
  }
  else if (precipitationFormBeg == SLEET_FORM)
  {
    if (precipitationFormEnd == SNOW_FORM)
    {
      if (pearson_co_sleet <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_snow >= PEARSON_CO_FORM_TRANSFORM)
        return SLEET_TO_SNOW;
    }
    else if (precipitationFormEnd == DRIZZLE_FORM)
    {
      if (pearson_co_sleet <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_drizzle >= PEARSON_CO_FORM_TRANSFORM)
        return SLEET_TO_DRIZZLE;
    }
    else if (precipitationFormEnd == WATER_FORM)
    {
      if (pearson_co_sleet <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_water >= PEARSON_CO_FORM_TRANSFORM)
        return SLEET_TO_WATER;
    }
  }
  else if (precipitationFormBeg == SNOW_FORM)
  {
    if (precipitationFormEnd == WATER_FORM)
    {
      if (pearson_co_snow <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_water >= PEARSON_CO_FORM_TRANSFORM)
        return SNOW_TO_WATER;
    }
    else if (precipitationFormEnd == DRIZZLE_FORM)
    {
      if (pearson_co_snow <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_drizzle >= PEARSON_CO_FORM_TRANSFORM)
        return SNOW_TO_DRIZZLE;
    }
    else if (precipitationFormEnd == SLEET_FORM)
    {
      if (pearson_co_snow <= -PEARSON_CO_FORM_TRANSFORM &&
          pearson_co_sleet >= PEARSON_CO_FORM_TRANSFORM)
        return SNOW_TO_SLEET;
    }
  }

  return NO_FORM_TRANSFORMATION;
}

WeatherPeriod PrecipitationForecast::getHeavyPrecipitationPeriod(
    const WeatherPeriod& thePeriod, const precipitation_data_vector& theDataVector) const
{
  vector<WeatherPeriod> heavyPrecipitationPeriods;

  // First find out all heavy precipitation periods
  int heavyPrecipitationStartIndex(-1);
  int heavyPrecipitationEndIndex(-1);
  for (unsigned int i = 0; i < theDataVector.size(); i++)
  {
    const PrecipitationDataItemData* precipitationItem(theDataVector[i]);

    if (precipitationItem->theObservationTime < thePeriod.localStartTime()) continue;
    if (precipitationItem->theObservationTime > thePeriod.localEndTime()) break;

    precipitation_form_id precipitationForm(
        get_complete_precipitation_form(theParameters.theVariable,
                                        precipitationItem->thePrecipitationFormWater,
                                        precipitationItem->thePrecipitationFormDrizzle,
                                        precipitationItem->thePrecipitationFormSleet,
                                        precipitationItem->thePrecipitationFormSnow,
                                        precipitationItem->thePrecipitationFormFreezing));
    float lowerLimit(kFloatMissing), upperLimit(kFloatMissing);
    get_precipitation_limit_value(
        theParameters, precipitationForm, HEAVY_PRECIPITATION, lowerLimit, upperLimit);

    if (theDataVector[i]->thePrecipitationMaxIntensity >= lowerLimit)
    {
      if (heavyPrecipitationStartIndex == -1)
        heavyPrecipitationStartIndex = i;  // heavy precipitation starts
      heavyPrecipitationEndIndex = i;
    }
    else if (heavyPrecipitationStartIndex != -1)
    {
      WeatherPeriod heavyPeriod(theDataVector[heavyPrecipitationStartIndex]->theObservationTime,
                                theDataVector[heavyPrecipitationEndIndex]->theObservationTime);
      // heavy precipitation ended
      heavyPrecipitationPeriods.push_back(heavyPeriod);
      heavyPrecipitationStartIndex = -1;
      heavyPrecipitationEndIndex = -1;
    }
  }
  if (heavyPrecipitationStartIndex != -1 && heavyPrecipitationPeriods.size() == 0)
  {
    WeatherPeriod heavyPeriod(theDataVector[heavyPrecipitationStartIndex]->theObservationTime,
                              theDataVector[heavyPrecipitationEndIndex]->theObservationTime);
    heavyPrecipitationPeriods.push_back(heavyPeriod);
  }

  // then find out if you can merge periods (if there is a smaall gap between them)
  vector<WeatherPeriod> mergedHeavyPrecipitationPeriods;
  if (heavyPrecipitationPeriods.size() > 0)
  {
    for (int i = heavyPrecipitationPeriods.size() - 2; i >= 0; i--)
    {
      TextGenPosixTime currentPeriodStartTime(heavyPrecipitationPeriods[i].localEndTime());
      TextGenPosixTime nextPeriodStartTime(heavyPrecipitationPeriods[i + 1].localStartTime());
      if (abs(nextPeriodStartTime.DifferenceInHours(currentPeriodStartTime)) <= 2 &&
          (get_period_length(heavyPrecipitationPeriods[i]) >= 2 ||
           get_period_length(heavyPrecipitationPeriods[i + 1]) >= 2))
      {
        // merge
        heavyPrecipitationPeriods[i] =
            WeatherPeriod(heavyPrecipitationPeriods[i].localStartTime(),
                          heavyPrecipitationPeriods[i + 1].localEndTime());
      }
      else
      {
        mergedHeavyPrecipitationPeriods.push_back(heavyPrecipitationPeriods[i + 1]);
      }
    }
    mergedHeavyPrecipitationPeriods.push_back(heavyPrecipitationPeriods[0]);
    sort(mergedHeavyPrecipitationPeriods.begin(), mergedHeavyPrecipitationPeriods.end());
  }

  // find out the longest heavy precipitation period and return it
  int heavyIndex = mergedHeavyPrecipitationPeriods.size() > 0 ? 0 : -1;
  for (unsigned int i = 1; i < mergedHeavyPrecipitationPeriods.size(); i++)
  {
    if (get_period_length(mergedHeavyPrecipitationPeriods[i]) >
        get_period_length(mergedHeavyPrecipitationPeriods[heavyIndex]))
      heavyIndex = i;
  }

  if (heavyIndex >= 0)
    return mergedHeavyPrecipitationPeriods[heavyIndex];
  else
    return WeatherPeriod(thePeriod.localStartTime(), thePeriod.localStartTime());
}

void PrecipitationForecast::calculatePrecipitationParameters(
    const WeatherPeriod& thePeriod,
    const precipitation_data_vector& theDataVector,
    float& thePrecipitationIntensity,
    float& thePrecipitationAbsoluteMaxIntensity,
    float& thePrecipitationExtent,
    float& thePrecipitationFormWater,
    float& thePrecipitationFormDrizzle,
    float& thePrecipitationFormSleet,
    float& thePrecipitationFormSnow,
    float& thePrecipitationFormFreezing) const
{
  // thePrecipitationIntensity contains mean of the maximum and mean precipitation
  thePrecipitationIntensity = getMean(theDataVector, PRECIPITATION_MEAN_DATA, thePeriod);
  thePrecipitationAbsoluteMaxIntensity = getMax(theDataVector, PRECIPITATION_MAX_DATA, thePeriod);
  thePrecipitationExtent = getMean(theDataVector, PRECIPITATION_EXTENT_DATA, thePeriod);
  thePrecipitationFormWater = getMean(theDataVector, PRECIPITATION_FORM_WATER_DATA, thePeriod);
  thePrecipitationFormDrizzle = getMean(theDataVector, PRECIPITATION_FORM_DRIZZLE_DATA, thePeriod);
  thePrecipitationFormSleet = getMean(theDataVector, PRECIPITATION_FORM_SLEET_DATA, thePeriod);
  thePrecipitationFormSnow = getMean(theDataVector, PRECIPITATION_FORM_SNOW_DATA, thePeriod);
  thePrecipitationFormFreezing =
      getMean(theDataVector, PRECIPITATION_FORM_FREEZING_DATA, thePeriod);
}

Sentence PrecipitationForecast::parseFinalSentence(
    map<string, Sentence>& theCompositePhraseElements,
    const Sentence& thePeriodPhrase,
    const std::string& theAreaPhrase) const
{
  Sentence sentence;
  Sentence periodPhrase;
  Sentence areaPhrase;
  Sentence inPlacesPhrase;
  Sentence intensity;
  Sentence precipitation;
  bool periodPhraseEmpty(thePeriodPhrase.size() == 0);
  bool areaPhraseEmpty(theAreaPhrase.size() == 0 || theAreaPhrase.compare(EMPTY_STRING) == 0);
  bool inPlacesPhraseEmpty(theCompositePhraseElements.find(IN_PLACES_PARAMETER) ==
                           theCompositePhraseElements.end());
  bool intensityEmpty(theCompositePhraseElements.find(INTENSITY_PARAMETER) ==
                      theCompositePhraseElements.end());

  if (theAreaPhrase.size() == 0)
  {
    areaPhrase << EMPTY_STRING;
  }
  else
  {
    areaPhrase << theAreaPhrase;
  }

  if (thePeriodPhrase.size() == 0)
  {
    periodPhrase << EMPTY_STRING;
  }
  else
  {
    periodPhrase << thePeriodPhrase;
  }

  if (theCompositePhraseElements.find(IN_PLACES_PARAMETER) == theCompositePhraseElements.end())
  {
    inPlacesPhrase << EMPTY_STRING;
  }
  else
  {
    inPlacesPhrase << theCompositePhraseElements[IN_PLACES_PARAMETER];
  }

  if (theCompositePhraseElements.find(INTENSITY_PARAMETER) == theCompositePhraseElements.end())
  {
    intensity << EMPTY_STRING;
  }
  else
  {
    intensity << theCompositePhraseElements[INTENSITY_PARAMETER];
  }

  if (theCompositePhraseElements.find(PRECIPITATION_PARAMETER) == theCompositePhraseElements.end())
  {
    precipitation << EMPTY_STRING;
  }
  else
  {
    precipitation << theCompositePhraseElements[PRECIPITATION_PARAMETER];
  }

  if (theCompositePhraseElements.find(PLAIN_PRECIPITATION_PHRASE) !=
      theCompositePhraseElements.end())
  {
    if (periodPhraseEmpty && areaPhraseEmpty)
    {
      sentence << PAIKOIN_SADETTA_COMPOSITE_PHRASE << inPlacesPhrase
               << theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE];
    }
    else if ((periodPhraseEmpty && !areaPhraseEmpty) || (!periodPhraseEmpty && areaPhraseEmpty))
    {
      if (periodPhraseEmpty)
      {
        if (inPlacesPhraseEmpty)
          sentence << SISAMAASSA_SADETTA_COMPOSITE_PHRASE << areaPhrase
                   << theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE];
        else
          sentence << SISAMAASSA_PAIKOIN_SADETTA_COMPOSITE_PHRASE << areaPhrase << inPlacesPhrase
                   << theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE];
      }
      else
      {
        if (inPlacesPhraseEmpty)
          sentence << HUOMENNA_SADETTA_COMPOSITE_PHRASE << periodPhrase
                   << theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE];
        else
          sentence << HUOMENNA_PAIKOIN_SADETTA_COMPOSITE_PHRASE << periodPhrase << inPlacesPhrase
                   << theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE];
      }
    }
    else
    {
      if (inPlacesPhraseEmpty)
        sentence << HUOMENNA_SISAMAASSA_SADETTA_COMPOSITE_PHRASE << periodPhrase << areaPhrase
                 << theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE];
      else
        sentence << HUOMENNA_SISAMAASSA_PAIKOIN_SADETTA_COMPOSITE_PHRASE << periodPhrase
                 << areaPhrase << inPlacesPhrase
                 << theCompositePhraseElements[PLAIN_PRECIPITATION_PHRASE];
    }
  }
  else
  {
    theParameters.theLog << "periodPhrase: ";
    theParameters.theLog << periodPhrase;
    theParameters.theLog << "areaPhrase: ";
    theParameters.theLog << areaPhrase;
    theParameters.theLog << "intensity: ";
    theParameters.theLog << intensity;
    theParameters.theLog << "precipitation: ";
    theParameters.theLog << precipitation;
    theParameters.theLog << "inPlacesPhrase: ";
    theParameters.theLog << inPlacesPhrase;
    theParameters.theLog << "periodPhraseEmpty: ";
    theParameters.theLog << (periodPhraseEmpty ? "yes" : "no") << endl;
    theParameters.theLog << "areaPhraseEmpty: ";
    theParameters.theLog << (areaPhraseEmpty ? "yes" : "no") << endl;

    if (theCompositePhraseElements.find(
            HUOMENNA_SISAMAASSA_SAA_ON_ENIMMAKSEEN_POUTAINEN_COMPOSITE_PHRASE) !=
        theCompositePhraseElements.end())
    {
      if (periodPhraseEmpty && areaPhraseEmpty)
      {
        sentence << SAA_ON_ENIMMAKSEEN_POUTAISTA_PHRASE << Delimiter(COMMA_PUNCTUATION_MARK)
                 << precipitation;
      }
      else if ((periodPhraseEmpty && !areaPhraseEmpty) || (!periodPhraseEmpty && areaPhraseEmpty))
      {
        if (periodPhraseEmpty)
          sentence << SISAMAASSA_SAA_ON_ENIMMAKSEEN_POUTAINEN_COMPOSITE_PHRASE << areaPhrase
                   << precipitation;
        else
          sentence << HUOMENNA_SAA_ON_ENIMMAKSEEN_POUTAINEN_COMPOSITE_PHRASE << periodPhrase
                   << precipitation;
      }
      else
      {
        sentence << HUOMENNA_SISAMAASSA_SAA_ON_ENIMMAKSEEN_POUTAINEN_COMPOSITE_PHRASE
                 << periodPhrase << areaPhrase << precipitation;
      }
    }
    else if (theCompositePhraseElements.find(
                 HUOMENNA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end() ||
             theCompositePhraseElements.find(
                 HUOMENNA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end() ||
             theCompositePhraseElements.find(
                 HUOMENNA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end() ||
             theCompositePhraseElements.find(
                 HUOMENNA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end() ||
             theCompositePhraseElements.find(
                 HUOMENNA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end() ||
             theCompositePhraseElements.find(
                 HUOMENNA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end() ||
             theCompositePhraseElements.find(
                 HUOMENNA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end() ||
             theCompositePhraseElements.find(
                 HUOMENNA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
    {
      if (periodPhraseEmpty && areaPhraseEmpty)
      {
        sentence << precipitation;
      }
      else if ((periodPhraseEmpty && !areaPhraseEmpty) || (!periodPhraseEmpty && areaPhraseEmpty))
      {
        if (theCompositePhraseElements.find(
                HUOMENNA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
            theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty)
            sentence << SISAMAASSA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << areaPhrase;
          else
            sentence << HUOMENNA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << periodPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty)
            sentence << SISAMAASSA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << areaPhrase;
          else
            sentence << HUOMENNA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << periodPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty)
            sentence << SISAMAASSA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << areaPhrase;
          else
            sentence << HUOMENNA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << periodPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty)
            sentence << SISAMAASSA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << areaPhrase;
          else
            sentence << HUOMENNA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << periodPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty)
            sentence << SISAMAASSA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << areaPhrase;
          else
            sentence << HUOMENNA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << periodPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty)
            sentence << SISAMAASSA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << areaPhrase;
          else
            sentence << HUOMENNA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << periodPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty)
            sentence << SISAMAASSA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << areaPhrase;
          else
            sentence << HUOMENNA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << periodPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty)
            sentence << SISAMAASSA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << areaPhrase;
          else
            sentence << HUOMENNA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                     << periodPhrase;
        }
      }
      else
      {
        if (theCompositePhraseElements.find(
                HUOMENNA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
            theCompositePhraseElements.end())
        {
          sentence << HUOMENNA_SISAMAASSA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                   << periodPhrase << areaPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          sentence
              << HUOMENNA_SISAMAASSA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
              << periodPhrase << areaPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          sentence
              << HUOMENNA_SISAMAASSA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
              << periodPhrase << areaPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          sentence << HUOMENNA_SISAMAASSA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                   << periodPhrase << areaPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          sentence << HUOMENNA_SISAMAASSA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                   << periodPhrase << areaPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          sentence
              << HUOMENNA_SISAMAASSA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
              << periodPhrase << areaPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          sentence << HUOMENNA_SISAMAASSA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
                   << periodPhrase << areaPhrase;
        }
        else if (theCompositePhraseElements.find(
                     HUOMENNA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          sentence
              << HUOMENNA_SISAMAASSA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE
              << periodPhrase << areaPhrase;
        }
      }

      /*
#define HUOMENNA_YKSITTAISET_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE "[huomenna] yksittaiset
vesikuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_VESI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE "[huomenna]
yksittaiset vesi- tai rantakuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_VESI_TAI_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE "[huomenna]
yksittaiset vesi- tai lumikuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_SADEKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE "[huomenna] yksittaiset
sadekuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE "[huomenna] yksittaiset
rantakuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_RANTA_TAI_VESIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE "[huomenna]
yksittaiset ranta- tai vesikuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_LUMIKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE "[huomenna] yksittaiset
lumikuurot mahdollisia"
#define HUOMENNA_YKSITTAISET_LUMI_TAI_RANTAKUUROT_MAHDOLLISIA_COMPOSITE_PHRASE "[huomenna]
yksittaiset lumi- tai rantakuurot mahdollisia"

       */
    }
    else
    {
      if (theCompositePhraseElements.find(SAA_ON_POUTAINEN_PHRASE) !=
          theCompositePhraseElements.end())
      {
        if (periodPhraseEmpty && areaPhraseEmpty)
        {
          sentence << SAA_ON_POUTAINEN_PHRASE;
        }
        else if ((periodPhraseEmpty && !areaPhraseEmpty) || (!periodPhraseEmpty && areaPhraseEmpty))
        {
          if (periodPhraseEmpty)
          {
            sentence << SISAMAASSA_SAA_ON_POUTAINEN_COMPOSITE_PHRASE << areaPhrase;
          }
          else
          {
            sentence << HUOMENNA_SAA_ON_POUTAINEN_COMPOSITE_PHRASE << periodPhrase;
          }
        }
        else
        {
          sentence << HUOMENNA_SISAMAASSA_SAA_ON_POUTAINEN_COMPOSITE_PHRASE << periodPhrase
                   << areaPhrase;
        }
      }
      else
      {
        if (theCompositePhraseElements.find(JOKA_VOI_OLLA_JAATAVAA_PHRASE) !=
            theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty && areaPhraseEmpty)
          {
            if (inPlacesPhraseEmpty)
            {
              if (intensityEmpty)
                sentence << SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE << precipitation;
              else
                sentence << HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE << intensity
                         << precipitation;
            }
            else
            {
              if (intensityEmpty)
                sentence << PAIKOIN_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                         << inPlacesPhrase << precipitation;
              else
                sentence << PAIKOIN_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                         << inPlacesPhrase << intensity << precipitation;
            }
          }
          else if ((periodPhraseEmpty && !areaPhraseEmpty) ||
                   (!periodPhraseEmpty && areaPhraseEmpty))
          {
            if (periodPhraseEmpty)
            {
              if (inPlacesPhraseEmpty)
              {
                if (intensityEmpty)
                  sentence << SISAMAASSA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                           << areaPhrase << precipitation;
                else
                  sentence << SISAMAASSA_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                           << areaPhrase << intensity << precipitation;
              }
              else
              {
                if (intensityEmpty)
                  sentence << SISAMAASSA_PAIKOIN_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                           << areaPhrase << inPlacesPhrase << precipitation;
                else
                  sentence
                      << SISAMAASSA_PAIKOIN_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                      << areaPhrase << inPlacesPhrase << intensity << precipitation;
              }
            }
            else
            {
              if (inPlacesPhraseEmpty)
              {
                if (intensityEmpty)
                  sentence << HUOMENNA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                           << periodPhrase << precipitation;
                else
                  sentence << HUOMENNA_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                           << periodPhrase << intensity << precipitation;
              }
              else
              {
                if (intensityEmpty)
                  sentence << HUOMENNA_PAIKOIN_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                           << periodPhrase << inPlacesPhrase << precipitation;
                else
                  sentence
                      << HUOMENNA_PAIKOIN_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                      << periodPhrase << inPlacesPhrase << intensity << precipitation;
              }
            }
          }
          else
          {
            if (inPlacesPhraseEmpty)
            {
              if (intensityEmpty)
                sentence << HUOMENNA_SISAMAASSA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                         << periodPhrase << areaPhrase << precipitation;
              else
                sentence
                    << HUOMENNA_SISAMAASSA_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                    << periodPhrase << areaPhrase << intensity << precipitation;
            }
            else
            {
              if (intensityEmpty)
                sentence
                    << HUOMENNA_SISAMAASSA_PAIKOIN_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                    << periodPhrase << areaPhrase << inPlacesPhrase << precipitation;
              else
                sentence
                    << HUOMENNA_SISAMAASSA_PAIKOIN_HEIKKOA_SADETTA_JOKA_VOI_OLLA_JAATAVAA_COMPOSITE_PHRASE
                    << periodPhrase << areaPhrase << inPlacesPhrase << intensity << precipitation;
            }
          }
        }
        else if (theCompositePhraseElements.find(JOTKA_VOIVAT_OLLA_JAATAVIA_PHRASE) !=
                 theCompositePhraseElements.end())
        {
          if (periodPhraseEmpty && areaPhraseEmpty)
          {
            if (inPlacesPhraseEmpty)
            {
              if (intensityEmpty)
                sentence << SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                         << precipitation;
              else
                sentence << HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                         << intensity << precipitation;
            }
            else
            {
              if (intensityEmpty)
                sentence << PAIKOIN_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                         << inPlacesPhrase << precipitation;
              else
                sentence << PAIKOIN_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                         << inPlacesPhrase << intensity << precipitation;
            }
          }
          else if ((periodPhraseEmpty && !areaPhraseEmpty) ||
                   (!periodPhraseEmpty && areaPhraseEmpty))
          {
            if (periodPhraseEmpty)
            {
              if (inPlacesPhraseEmpty)
              {
                if (intensityEmpty)
                  sentence << SISAMAASSA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                           << areaPhrase << precipitation;
                else
                  sentence
                      << SISAMAASSA_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                      << areaPhrase << intensity << precipitation;
              }
              else
              {
                if (intensityEmpty)
                  sentence
                      << SISAMAASSA_PAIKOIN_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                      << areaPhrase << inPlacesPhrase << precipitation;
                else
                  sentence
                      << SISAMAASSA_PAIKOIN_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                      << areaPhrase << inPlacesPhrase << intensity << precipitation;
              }
            }
            else
            {
              if (inPlacesPhraseEmpty)
              {
                if (intensityEmpty)
                  sentence << HUOMENNA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                           << periodPhrase << precipitation;
                else
                  sentence
                      << HUOMENNA_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                      << periodPhrase << intensity << precipitation;
              }
              else
              {
                if (intensityEmpty)
                  sentence
                      << HUOMENNA_PAIKOIN_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                      << periodPhrase << inPlacesPhrase << precipitation;
                else
                  sentence
                      << HUOMENNA_PAIKOIN_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                      << periodPhrase << inPlacesPhrase << intensity << precipitation;
              }
            }
          }
          else
          {
            if (inPlacesPhraseEmpty)
            {
              if (intensityEmpty)
                sentence
                    << HUOMENNA_SISAMAASSA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                    << periodPhrase << areaPhrase << precipitation;
              else
                sentence
                    << HUOMENNA_SISAMAASSA_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                    << periodPhrase << areaPhrase << intensity << precipitation;
            }
            else
            {
              if (intensityEmpty)
                sentence
                    << HUOMENNA_SISAMAASSA_PAIKOIN_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                    << periodPhrase << areaPhrase << inPlacesPhrase << precipitation;
              else
                sentence
                    << HUOMENNA_SISAMAASSA_PAIKOIN_HEIKKOJA_SADEKUUROJA_JOTKA_VOIVAT_OLLA_JAATAVIA_COMPOSITE_PHRASE
                    << periodPhrase << areaPhrase << inPlacesPhrase << intensity << precipitation;
            }
          }
        }
        else
        {
          if (periodPhraseEmpty && areaPhraseEmpty)
          {
            if (inPlacesPhraseEmpty)
            {
              if (intensityEmpty)
                sentence << precipitation;
              else
                sentence << HEIKKOA_SADETTA_COMPOSITE_PHRASE << intensity << precipitation;
            }
            else
            {
              if (intensityEmpty)
                sentence << PAIKOIN_SADETTA_COMPOSITE_PHRASE << inPlacesPhrase << precipitation;
              else
                sentence << PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE << inPlacesPhrase << intensity
                         << precipitation;
            }
          }

          else if ((periodPhraseEmpty && !areaPhraseEmpty) ||
                   (!periodPhraseEmpty && areaPhraseEmpty))
          {
            if (periodPhraseEmpty)
            {
              if (inPlacesPhraseEmpty)
              {
                if (intensityEmpty)
                  sentence << SISAMAASSA_SADETTA_COMPOSITE_PHRASE << areaPhrase << precipitation;
                else
                  sentence << SISAMAASSA_HEIKKOA_SADETTA_COMPOSITE_PHRASE << areaPhrase << intensity
                           << precipitation;
              }
              else
              {
                if (intensityEmpty)
                  sentence << SISAMAASSA_PAIKOIN_SADETTA_COMPOSITE_PHRASE << areaPhrase
                           << inPlacesPhrase << precipitation;
                else
                  sentence << SISAMAASSA_PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE << areaPhrase
                           << inPlacesPhrase << intensity << precipitation;
              }
            }
            else
            {
              if (inPlacesPhraseEmpty)
              {
                if (intensityEmpty)
                  sentence << HUOMENNA_SADETTA_COMPOSITE_PHRASE << periodPhrase << precipitation;
                else
                  sentence << HUOMENNA_HEIKKOA_SADETTA_COMPOSITE_PHRASE << periodPhrase << intensity
                           << precipitation;
              }
              else
              {
                if (intensityEmpty)
                  sentence << HUOMENNA_PAIKOIN_SADETTA_COMPOSITE_PHRASE << periodPhrase
                           << inPlacesPhrase << precipitation;
                else
                  sentence << HUOMENNA_PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE << periodPhrase
                           << inPlacesPhrase << intensity << precipitation;
              }
            }
          }
          else
          {
            if (inPlacesPhraseEmpty)
            {
              if (intensityEmpty)
                sentence << HUOMENNA_SISAMAASSA_SADETTA_COMPOSITE_PHRASE << periodPhrase
                         << areaPhrase << precipitation;
              else
                sentence << HUOMENNA_SISAMAASSA_HEIKKOA_SADETTA_COMPOSITE_PHRASE << periodPhrase
                         << areaPhrase << intensity << precipitation;
            }
            else
            {
              if (intensityEmpty)
                sentence << HUOMENNA_SISAMAASSA_PAIKOIN_SADETTA_COMPOSITE_PHRASE << periodPhrase
                         << areaPhrase << inPlacesPhrase << precipitation;
              else
                sentence << HUOMENNA_SISAMAASSA_PAIKOIN_HEIKKOA_SADETTA_COMPOSITE_PHRASE
                         << periodPhrase << areaPhrase << inPlacesPhrase << intensity
                         << precipitation;
            }
          }
        }
      }
    }
  }
  theParameters.theLog << "FINAL SENTENCE: ";
  theParameters.theLog << sentence;

  return sentence;
}

Sentence PrecipitationForecast::constructPrecipitationSentence(
    const WeatherPeriod& thePeriod,
    const Sentence& thePeriodPhrase,
    const unsigned short& theForecastAreaId,
    const std::string& theAreaPhrase,
    std::vector<Sentence>& theAdditionalSentences) const
{
  Sentence sentence;

  const precipitation_data_vector* dataVector = 0;

  float precipitationIntensity = kFloatMissing;
  float precipitationAbsoluteMaxIntensity = kFloatMissing;
  float precipitationExtent = kFloatMissing;
  float precipitationFormWater = kFloatMissing;
  float precipitationFormDrizzle = kFloatMissing;
  float precipitationFormSleet = kFloatMissing;
  float precipitationFormSnow = kFloatMissing;
  float precipitationFormFreezing = kFloatMissing;

  if (theForecastAreaId & INLAND_AREA && theForecastAreaId & COASTAL_AREA)
  {
    if (separateCoastInlandPrecipitation(thePeriod))
    {
      sentence << constructPrecipitationSentence(
          thePeriod, thePeriodPhrase, INLAND_AREA, INLAND_PHRASE, theAdditionalSentences);

      // ARE 22.02.2011: this is to prevent tautology e.g. sisamaassa moinin paikoin rantasadetta,
      // rannikolla monin paikoin vesisadetta
      get_in_places_phrase().preventTautology(true);

      sentence << Delimiter(COMMA_PUNCTUATION_MARK);

      sentence << constructPrecipitationSentence(
          thePeriod, thePeriodPhrase, COASTAL_AREA, COAST_PHRASE, theAdditionalSentences);

      get_in_places_phrase().preventTautology(false);
    }
    else
    {
      dataVector = &theFullData;
    }
  }
  else if (theForecastAreaId & INLAND_AREA)
  {
    dataVector = &theInlandData;

    /*
    // This block is here to find out, whether more than one precipitation form is included in the
    sentence
    // when both coast and inland are included. That information we need to choose
    // the right phrase for example vesisadetta vs. sadetta:
    // for example sisamaassa sadetta, rannikolla sadekuuroja vs.
    // sisamaassa vesisadetta, rannikolla lumi- tai rantakuuroja
    if(separateCoastInlandPrecipitation(thePeriod))
      {
            calculatePrecipitationParameters(thePeriod,
                                                                             theCoastalData,
                                                                             precipitationIntensity,
                                                                             precipitationAbsoluteMaxIntensity,
                                                                             precipitationExtent,
                                                                             precipitationFormWater,
                                                                             precipitationFormDrizzle,
                                                                             precipitationFormSleet,
                                                                             precipitationFormSnow,
                                                                             precipitationFormFreezing);
      }
    */
  }
  else if (theForecastAreaId & COASTAL_AREA)
  {
    dataVector = &theCoastalData;

    /*
    if(separateCoastInlandPrecipitation(thePeriod))
      {
            calculatePrecipitationParameters(thePeriod,
                                                                             theInlandData,
                                                                             precipitationIntensity,
                                                                             precipitationAbsoluteMaxIntensity,
                                                                             precipitationExtent,
                                                                             precipitationFormWater,
                                                                             precipitationFormDrizzle,
                                                                             precipitationFormSleet,
                                                                             precipitationFormSnow,
                                                                             precipitationFormFreezing);
      }
    */
  }

  if (dataVector)
  {
    calculatePrecipitationParameters(thePeriod,
                                     *dataVector,
                                     precipitationIntensity,
                                     precipitationAbsoluteMaxIntensity,
                                     precipitationExtent,
                                     precipitationFormWater,
                                     precipitationFormDrizzle,
                                     precipitationFormSleet,
                                     precipitationFormSnow,
                                     precipitationFormFreezing);

    theParameters.theLog << "Period: " << thePeriod.localStartTime() << "..."
                         << thePeriod.localEndTime() << endl;
    theParameters.theLog << "Mean intensity (max+mean/2.0): " << precipitationIntensity << endl;
    theParameters.theLog << "Maximum intensity: " << precipitationAbsoluteMaxIntensity << endl;

    precipitation_type precipitationType(
        getPrecipitationType(thePeriod, theParameters.theForecastArea));

    unsigned int typeChangeIndex = getPrecipitationTypeChange(*dataVector, thePeriod);

    precipitation_form_id precipitationForm =
        get_complete_precipitation_form(theParameters.theVariable,
                                        precipitationFormWater,
                                        precipitationFormDrizzle,
                                        precipitationFormSleet,
                                        precipitationFormSnow,
                                        precipitationFormFreezing);

    thePrecipitationFormBeforeDryPeriod = precipitationForm;

    map<string, Sentence> compositePhraseElements;

    selectPrecipitationSentence(thePeriod,
                                precipitationForm,
                                precipitationIntensity,
                                precipitationAbsoluteMaxIntensity,
                                precipitationExtent,
                                precipitationFormWater,
                                precipitationFormDrizzle,
                                precipitationFormSleet,
                                precipitationFormSnow,
                                precipitationFormFreezing,
                                precipitationType,
                                dataVector->at(typeChangeIndex)->theObservationTime,
                                getPrecipitationTransformationId(thePeriod, theForecastAreaId),
                                compositePhraseElements);

    sentence << parseFinalSentence(compositePhraseElements, thePeriodPhrase, theAreaPhrase);

    bool dry_weather = is_dry_weather(
        theParameters, precipitationForm, precipitationIntensity, precipitationExtent);

    if (!dry_weather)
    {
      // sentence << areaSpecificSentence(thePeriod);

      Sentence thunderSentence;
      thunderSentence << getThunderSentence(
          thePeriod, theForecastAreaId, theParameters.theVariable);

      if (thunderSentence.size() > 0)
      {
        sentence << thunderSentence;
      }

      if (theCheckHeavyIntensityFlag != MISSING_PRECIPITATION_TYPE)
      {
        WeatherPeriod heavyPrecipitationPeriod(getHeavyPrecipitationPeriod(thePeriod, *dataVector));

        if (get_period_length(heavyPrecipitationPeriod) >= 2)
        {
          part_of_the_day_id partOfTheDayIdStart(
              get_part_of_the_day_id(heavyPrecipitationPeriod.localStartTime()));
          part_of_the_day_id partOfTheDayIdEnd(
              get_part_of_the_day_id(heavyPrecipitationPeriod.localEndTime()));
          part_of_the_day_id partOfTheDayId(MISSING_PART_OF_THE_DAY_ID);
          time_phrase_format timePhraseFormat(AT_FORMAT);

          if (heavyPrecipitationPeriod.localStartTime() == thePeriod.localStartTime())
          {
            // heavy precipitation in the beginning
            partOfTheDayId = get_part_of_the_day_id(heavyPrecipitationPeriod.localEndTime());
            timePhraseFormat = TILL_FORMAT;
          }
          else if (heavyPrecipitationPeriod.localEndTime() == thePeriod.localEndTime())
          {
            // heavy precipitation in the end
            partOfTheDayId = get_part_of_the_day_id(heavyPrecipitationPeriod.localStartTime());
            timePhraseFormat = FROM_FORMAT;
          }
          else
          {
            partOfTheDayId = get_part_of_the_day_id_large(heavyPrecipitationPeriod);
            timePhraseFormat = AT_FORMAT;
          }

          if (timePhraseFormat == TILL_FORMAT && partOfTheDayIdStart == partOfTheDayIdEnd)
          {
            timePhraseFormat = AT_FORMAT;
          }

          std::string numberString;
          Sentence heavyPrecipitationSentence;
          if (partOfTheDayId != MISSING_PART_OF_THE_DAY_ID)
          {
            Sentence todayPhrase;
            todayPhrase << PeriodPhraseFactory::create("today",
                                                       theParameters.theVariable,
                                                       theParameters.theForecastTime,
                                                       heavyPrecipitationPeriod,
                                                       theParameters.theArea);
            bool samedayStart =
                isSameDay(theParameters.theForecastTime, heavyPrecipitationPeriod.localStartTime());
            bool samedayEnd =
                isSameDay(theParameters.theForecastTime, heavyPrecipitationPeriod.localEndTime());
            bool dontUseTodayPhrase =
                (get_period_length(theParameters.theForecastPeriod) <= 24 ||
                 (timePhraseFormat == TILL_FORMAT && samedayEnd) ||
                 (timePhraseFormat == FROM_FORMAT && samedayStart) ||
                 (timePhraseFormat == AT_FORMAT && samedayStart && samedayEnd));

            if (!todayPhrase.empty() && !dontUseTodayPhrase)
            {
              DebugTextFormatter dtf;
              boost::shared_ptr<Dictionary> dict(new DebugDictionary());
              dtf.dictionary(dict);
              std::string todayString(dtf.format(todayPhrase));
              if (todayString.find('-') != string::npos)
                numberString = todayString.substr(0, todayString.find('-') + 1);
              else
                heavyPrecipitationSentence << todayPhrase;
            }

            if (theCheckHeavyIntensityFlag == SHOWERS)
              heavyPrecipitationSentence << ILTAPAIVALLA_KUUROT_VOIVAT_OLLA_VOIMAKKAITA;
            else
              heavyPrecipitationSentence << ILTAPAIVALLA_SADE_VOI_OLLA_RUNSASTA;

            heavyPrecipitationSentence
                << (numberString + getTimePhrase(partOfTheDayId, timePhraseFormat));
            theAdditionalSentences.push_back(heavyPrecipitationSentence);

            theParameters.theLog << "HEAVY PRECIPITATION PERIOD: "
                                 << heavyPrecipitationPeriod.localStartTime() << "..."
                                 << heavyPrecipitationPeriod.localEndTime() << " ("
                                 << getTimePhrase(partOfTheDayId, timePhraseFormat) << ")" << endl;
          }
        }
      }
    }
  }

  return sentence;
}

string PrecipitationForecast::getTimePhrase(const part_of_the_day_id& thePartOfTheDayId,
                                            const time_phrase_format& theTimePhraseFormat) const
{
  switch (thePartOfTheDayId)
  {
    case AAMU:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return AAMUSTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return AAMUUN_ASTI_PHRASE;
      else
        return AAMULLA_WORD;
      break;
    }
    case AAMUPAIVA:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return AAMUPAIVASTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return AAMUPAIVAAN_ASTI_PHRASE;
      else
        return AAMUPAIVALLA_WORD;
      break;
    }
    case ILTAPAIVA:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return ILTAPAIVASTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return ILTAPAIVAAN_ASTI_PHRASE;
      else
        return ILTAPAIVALLA_WORD;
      break;
    }
    case ILTA:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return ILLASTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return ILTAAN_ASTI_PHRASE;
      else
        return ILLALLA_WORD;
      break;
    }
    case ILTAYO:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return ILTAYOSTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return ILTAYOHON_ASTI_PHRASE;
      else
        return ILTAYOLLA_WORD;
      break;
    }
    case KESKIYO:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return KESKIYOSTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return KESKIYOHON_ASTI_PHRASE;
      else
        return KESKIYOLLA_WORD;
      break;
    }
    case AAMUYO:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return AAMUYOSTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return AAMUYOHON_ASTI_PHRASE;
      else
        return AAMUYOLLA_WORD;
      break;
    }
    case PAIVA:
    {
      if (theTimePhraseFormat == AT_FORMAT) return PAIVALLA_WORD;
      break;
    }
    case YO:
    {
      if (theTimePhraseFormat == AT_FORMAT) return YOLLA_WORD;
      break;
    }
    case AAMU_JA_AAMUPAIVA:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return AAMUSTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return AAMUPAIVAAN_ASTI_PHRASE;
      else
        return AAMULLA_JA_AAMUPAIVALLA_PHRASE;
      break;
    }
    case ILTAPAIVA_JA_ILTA:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return ILTAPAIVASTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return ILTAAN_ASTI_PHRASE;
      else
        return ILTAPAIVALLA_JA_ILLALLA_PHRASE;
      break;
    }
    case ILTA_JA_ILTAYO:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return ILLASTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return ILTAYOHON_ASTI_PHRASE;
      else
        return ILLALLA_JA_ILTAYOLLA_PHRASE;
      break;
    }
    case ILTAYO_JA_KESKIYO:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return ILTAYOSTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return KESKIYOHON_ASTI_PHRASE;
      else
        return ILTAYOLLA_JA_KESKIYOLLA_PHRASE;
      break;
    }
    case KESKIYO_JA_AAMUYO:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return KESKIYOSTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return AAMUYOHON_ASTI_PHRASE;
      else
        return KESKIYOLLA_JA_AAMUYOLLA_PHRASE;
      break;
    }
    case AAMUYO_JA_AAMU:
    {
      if (theTimePhraseFormat == FROM_FORMAT)
        return AAMUYOSTA_ALKAEN_PHRASE;
      else if (theTimePhraseFormat == TILL_FORMAT)
        return AAMUUN_ASTI_PHRASE;
      else
        return AAMUYOLLA_JA_AAMULLA_PHRASE;
      break;
    }
    case MISSING_PART_OF_THE_DAY_ID:
      return "missing part of the day";
  };

  return "missing part of the day";
}

bool PrecipitationForecast::shortTermPrecipitationExists(const WeatherPeriod& thePeriod) const
{
  const vector<WeatherPeriod>* precipitationPeriodVector = 0;

  if (theParameters.theForecastArea & FULL_AREA)
    precipitationPeriodVector = &thePrecipitationPeriodsFull;
  else if (theParameters.theForecastArea & COASTAL_AREA)
    precipitationPeriodVector = &thePrecipitationPeriodsCoastal;
  else if (theParameters.theForecastArea & INLAND_AREA)
    precipitationPeriodVector = &thePrecipitationPeriodsInland;

  if (precipitationPeriodVector)
  {
    for (unsigned int i = 0; i < precipitationPeriodVector->size(); i++)
    {
      TextGenPosixTime startTime(precipitationPeriodVector->at(i).localStartTime());
      TextGenPosixTime endTime(precipitationPeriodVector->at(i).localEndTime());

      if (endTime.DifferenceInHours(startTime) < 6 && is_inside(startTime, thePeriod) &&
          is_inside(endTime, thePeriod))
      {
        return true;
      }
    }
  }

  return false;
}

Sentence PrecipitationForecast::shortTermPrecipitationSentence(
    const WeatherPeriod& thePeriod, const Sentence& thePeriodPhrase) const
{
  Sentence sentence;

  std::vector<Sentence> theAdditionalSentences;

  sentence << constructPrecipitationSentence(thePeriod,
                                             thePeriodPhrase,
                                             theParameters.theForecastArea,
                                             EMPTY_STRING,
                                             theAdditionalSentences);

  theParameters.theLog << "Short term precipitation sentence: ";
  theParameters.theLog << sentence;

  return sentence;
}

Sentence PrecipitationForecast::precipitationSentence(
    const WeatherPeriod& thePeriod,
    const Sentence& thePeriodPhrase,
    std::vector<Sentence>& theAdditionalSentences) const
{
  Sentence sentence;

  sentence << constructPrecipitationSentence(thePeriod,
                                             thePeriodPhrase,
                                             theParameters.theForecastArea,
                                             EMPTY_STRING,
                                             theAdditionalSentences);

  return sentence;
}

Sentence PrecipitationForecast::areaSpecificSentence(const WeatherPeriod& thePeriod) const
{
  Sentence sentence;

  // If the area contains both coast and inland, we don't use area specific sentence
  if (theParameters.theForecastArea & FULL_AREA) return sentence;

  float north(0.0);
  float south(0.0);
  float east(0.0);
  float west(0.0);
  float northEast(0.0);
  float southEast(0.0);
  float southWest(0.0);
  float northWest(0.0);

  getPrecipitationDistribution(
      thePeriod, north, south, east, west, northEast, southEast, southWest, northWest);

  area_specific_sentence_id sentenceId = get_area_specific_sentence_id(
      north, south, east, west, northEast, southEast, southWest, northWest);

  Rect areaRect(theParameters.theArea);
  NFmiMercatorArea mercatorArea(areaRect.getBottomLeft(), areaRect.getTopRight());
  float areaHeightWidthRatio = mercatorArea.WorldRect().Height() / mercatorArea.WorldRect().Width();

  Sentence areaSpecificSentence;
  areaSpecificSentence << area_specific_sentence(
      north, south, east, west, northEast, southEast, southWest, northWest);

  // If the area is too cigar-shaped, we can use only north-south/east-west specifier
  if ((areaHeightWidthRatio >= 0.6 && areaHeightWidthRatio <= 1.5) ||
      (areaHeightWidthRatio < 0.6 &&
       (sentenceId == ALUEEN_ITAOSASSA || sentenceId == ALUEEN_LANSIOSASSA ||
        sentenceId == ENIMMAKSEEN_ALUEEN_ITAOSASSA ||
        sentenceId == ENIMMAKSEEN_ALUEEN_LANSIOSASSA)) ||
      (areaHeightWidthRatio > 1.5 &&
       (sentenceId == ALUEEN_POHJOISOSASSA || sentenceId == ALUEEN_ETELAOSASSA ||
        sentenceId == ENIMMAKSEEN_ALUEEN_POHJOISOSASSA ||
        sentenceId == ENIMMAKSEEN_ALUEEN_ETELAOSASSA)))
  {
    sentence << areaSpecificSentence;
  }

  return sentence;
}

void PrecipitationForecast::getWeatherEventIdVector(
    weather_event_id_vector& thePrecipitationWeatherEvents) const
{
  const weather_event_id_vector* vectorToRefer = 0;

  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    vectorToRefer = &thePrecipitationWeatherEventsFull;
  else if (theParameters.theForecastArea & COASTAL_AREA)
    vectorToRefer = &thePrecipitationWeatherEventsCoastal;
  else if (theParameters.theForecastArea & INLAND_AREA)
    vectorToRefer = &thePrecipitationWeatherEventsInland;

  if (vectorToRefer) thePrecipitationWeatherEvents = *vectorToRefer;
}

Rect PrecipitationForecast::getPrecipitationRect(const TextGenPosixTime& theTimestamp,
                                                 const float& theLowerLimit,
                                                 const float& theUpperLimit) const
{
  NFmiIndexMask indexMask;
  RangeAcceptor precipitationlimits;
  precipitationlimits.lowerLimit(theLowerLimit);
  precipitationlimits.upperLimit(theUpperLimit);

  // precipitation in the beginning
  ExtractMask(theParameters.theSources,
              Precipitation,
              theParameters.theArea,
              WeatherPeriod(theTimestamp, theTimestamp),
              precipitationlimits,
              indexMask);

  return Rect(theParameters.theSources, Precipitation, indexMask);
}

direction_id PrecipitationForecast::getPrecipitationLeavingDirection(
    const WeatherPeriod& thePeriod) const
{
  direction_id retval(NO_DIRECTION);

  TextGenPosixTime startTime(thePeriod.localEndTime());
  TextGenPosixTime endTime(thePeriod.localEndTime());
  endTime.ChangeByHours(1);
  startTime.ChangeByHours(1);
  WeatherPeriod checkPeriod(startTime, endTime);

  unsigned int startIndex, endIndex;
  weather_result_data_item_vector* northeast_data =
      get_data_vector(theParameters, PRECIPITATION_NORTHEAST_SHARE_DATA);
  weather_result_data_item_vector* southeast_data =
      get_data_vector(theParameters, PRECIPITATION_SOUTHEAST_SHARE_DATA);
  weather_result_data_item_vector* southwest_data =
      get_data_vector(theParameters, PRECIPITATION_SOUTHWEST_SHARE_DATA);
  weather_result_data_item_vector* northwest_data =
      get_data_vector(theParameters, PRECIPITATION_NORTHWEST_SHARE_DATA);

  if (!get_period_start_end_index(checkPeriod, *northeast_data, startIndex, endIndex))
    return retval;

  float northeast = northeast_data->at(startIndex)->theResult.value();
  float southeast = southeast_data->at(startIndex)->theResult.value();
  float southwest = southwest_data->at(startIndex)->theResult.value();
  float northwest = northwest_data->at(startIndex)->theResult.value();
  float north = northeast + northwest;
  float south = southeast + southwest;
  float east = northeast + southeast;
  float west = southwest + northwest;

  if (north >= 80)
    retval = SOUTH;
  else if (south >= 80)
    retval = NORTH;
  else if (east >= 80)
    retval = WEST;
  else if (west >= 80)
    retval = EAST;

  /*
  Rect areaRect(theParameters.theArea);
  Rect precipitationRect(getPrecipitationRect(thePeriod.localEndTime(), 0.0, 0.02));

  retval = getDirection(areaRect, precipitationRect);

  */

  return retval;
}

direction_id PrecipitationForecast::getPrecipitationArrivalDirection(
    const WeatherPeriod& thePeriod) const
{
  direction_id retval(NO_DIRECTION);

  TextGenPosixTime startTime(thePeriod.localStartTime());
  startTime.ChangeByHours(-1);
  TextGenPosixTime endTime(thePeriod.localStartTime());
  WeatherPeriod checkPeriod(startTime, endTime);

  unsigned int startIndex, endIndex;
  weather_result_data_item_vector* northeast_data =
      get_data_vector(theParameters, PRECIPITATION_NORTHEAST_SHARE_DATA);
  weather_result_data_item_vector* southeast_data =
      get_data_vector(theParameters, PRECIPITATION_SOUTHEAST_SHARE_DATA);
  weather_result_data_item_vector* southwest_data =
      get_data_vector(theParameters, PRECIPITATION_SOUTHWEST_SHARE_DATA);
  weather_result_data_item_vector* northwest_data =
      get_data_vector(theParameters, PRECIPITATION_NORTHWEST_SHARE_DATA);

  if (!get_period_start_end_index(checkPeriod, *northeast_data, startIndex, endIndex))
    return retval;

  float northeast = northeast_data->at(startIndex)->theResult.value();
  float southeast = southeast_data->at(startIndex)->theResult.value();
  float southwest = southwest_data->at(startIndex)->theResult.value();
  float northwest = northwest_data->at(startIndex)->theResult.value();
  float north = northeast + northwest;
  float south = southeast + southwest;
  float east = northeast + southeast;
  float west = southwest + northwest;

  if (northeast >= 80)
    retval = NORTHEAST;
  else if (southeast >= 80)
    retval = SOUTHEAST;
  else if (southwest >= 80)
    retval = SOUTHWEST;
  else if (northwest >= 80)
    retval = NORTHWEST;
  else if (north >= 80)
    retval = NORTH;
  else if (south >= 80)
    retval = SOUTH;
  else if (east >= 80)
    retval = EAST;
  else if (west >= 80)
    retval = WEST;

  /*
  Rect areaRect(theParameters.theArea);
  Rect precipitationRect(getPrecipitationRect(thePeriod.localStartTime(), 0.02, 100.0));

  retval = getDirection(areaRect, precipitationRect);

  if(retval != NO_DIRECTION)
    {
          theParameters.theLog << "Whole area: " << (string)areaRect << endl;
          theParameters.theLog << "Precipitation area: " << (string)precipitationRect << endl;
          theParameters.theLog << "West: " <<	(string)areaRect.subRect(WEST) << endl;
          theParameters.theLog << "East: " <<	(string)areaRect.subRect(EAST) << endl;

          theParameters.theLog << "Forecast period: "
                                                   <<
  theParameters.theForecastPeriod.localStartTime()
                                                   << " ... "
                                                   << theParameters.theForecastPeriod.localEndTime()
                                                   << endl;
          theParameters.theLog << "Rain direction check period: "
                                                   << thePeriod.localStartTime()
                                                   << " ... "
                                                   << thePeriod.localStartTime()
                                                   << endl;
          theParameters.theLog << getDirectionString(retval) << endl;
    }
  */
  return retval;
}

NFmiPoint PrecipitationForecast::getPrecipitationRepresentativePoint(
    const WeatherPeriod& thePeriod) const
{
  NFmiPoint retval(kFloatMissing, kFloatMissing);

  unsigned int startIndex, endIndex;
  weather_result_data_item_vector* coordinates =
      get_data_vector(theParameters, PRECIPITATION_POINT_DATA);
  if (!get_period_start_end_index(thePeriod, *coordinates, startIndex, endIndex)) return retval;

  float lon = 0.0;
  float lat = 0.0;
  unsigned int count = 0;
  for (unsigned int i = startIndex; i <= endIndex; i++)
  {
    if (coordinates->at(i)->theResult.value() != kFloatMissing)
    {
      lon += coordinates->at(i)->theResult.value();
      lat += coordinates->at(i)->theResult.error();
      count++;
    }
  }

  if (count > 0)
  {
    retval.X(lon / count);
    retval.Y(lat / count);
  }

  return retval;
}

precipitation_traverse_id PrecipitationForecast::getPrecipitationTraverseId(
    const WeatherPeriod& thePeriod) const
{
  precipitation_traverse_id retval(MISSING_TRAVERSE_ID);

  const weather_result_data_item_vector* dataVector =
      get_data_vector(theParameters, PRECIPITATION_MEAN_DATA);

  unsigned int startIndex, endIndex;

  if (!get_period_start_end_index(thePeriod, *dataVector, startIndex, endIndex)) return retval;

  NFmiIndexMask begIndexMask;
  NFmiIndexMask endIndexMask;
  RangeAcceptor precipitationlimits;
  precipitationlimits.lowerLimit(0.02);

  ExtractMask(theParameters.theSources,
              Precipitation,
              theParameters.theArea,
              WeatherPeriod(thePeriod.localStartTime(), thePeriod.localStartTime()),
              precipitationlimits,
              begIndexMask);
  ExtractMask(theParameters.theSources,
              Precipitation,
              theParameters.theArea,
              WeatherPeriod(thePeriod.localEndTime(), thePeriod.localEndTime()),
              precipitationlimits,
              endIndexMask);

  Rect areaRect(theParameters.theArea);
  Rect begRect(theParameters.theSources, Precipitation, begIndexMask);
  Rect endRect(theParameters.theSources, Precipitation, endIndexMask);

  direction_id begDirection = getDirection(areaRect, begRect);
  direction_id endDirection = getDirection(areaRect, endRect);

  weather_result_data_item_vector* coordinates =
      get_data_vector(theParameters, PRECIPITATION_POINT_DATA);
  get_period_start_end_index(thePeriod, *coordinates, startIndex, endIndex);

  double pearson_co_lon = get_pearson_coefficient(*coordinates, startIndex, endIndex, false);
  double pearson_co_lat = get_pearson_coefficient(*coordinates, startIndex, endIndex, true);

  if (begDirection == EAST && endDirection == WEST && pearson_co_lon <= -0.85)
  {
    retval = FROM_EAST_TO_WEST;
  }
  else if (begDirection == WEST && endDirection == EAST && pearson_co_lon >= 0.85)
  {
    retval = FROM_WEST_TO_EAST;
  }
  else if (begDirection == SOUTH && endDirection == NORTH && pearson_co_lat >= 0.85)
  {
    retval = FROM_SOUTH_TO_NORTH;
  }
  else if (begDirection == NORTH && endDirection == SOUTH && pearson_co_lat <= -0.85)
  {
    retval = FROM_NORTH_TO_SOUTH;
  }
  else if (begDirection == NORTHEAST && endDirection == SOUTHWEST && pearson_co_lon <= -0.85 &&
           pearson_co_lat <= -0.85)
  {
    retval = FROM_NORTHEAST_TO_SOUTHWEST;
  }
  else if (begDirection == SOUTHWEST && endDirection == NORTHEAST && pearson_co_lon >= 0.85 &&
           pearson_co_lat >= 0.85)
  {
    retval = FROM_SOUTHWEST_TO_NORTHEAST;
  }
  else if (begDirection == NORTHWEST && endDirection == SOUTHEAST && pearson_co_lon >= 0.85 &&
           pearson_co_lat <= -0.85)
  {
    retval = FROM_NORTHWEST_TO_SOUTHEAST;
  }
  else if (begDirection == SOUTHEAST && endDirection == NORTHWEST && pearson_co_lon <= -0.85 &&
           pearson_co_lat >= 0.85)
  {
    retval = FROM_SOUTHEAST_TO_NORTHWEST;
  }

  if (retval != MISSING_TRAVERSE_ID)
  {
    theParameters.theLog << "Sadealue kulkee " << precipitation_traverse_string(retval) << endl;
  }

  return retval;
}

const precipitation_data_vector& PrecipitationForecast::getPrecipitationDataVector(
    const unsigned short theForecastArea) const
{
  if (theForecastArea & INLAND_AREA && theForecastArea & COASTAL_AREA)
  {
    return theFullData;
  }
  else if (theForecastArea & COASTAL_AREA)
  {
    return theCoastalData;
  }
  else
  {
    return theInlandData;
  }
}

PrecipitationForecast::PrecipitationForecast(wf_story_params& parameters)
    : theParameters(parameters),
      theUseOllaVerbFlag(false),
      theDryPeriodTautologyFlag(false),
      theSinglePrecipitationFormFlag(true),
      thePrecipitationFormBeforeDryPeriod(MISSING_PRECIPITATION_FORM),
      theCheckHeavyIntensityFlag(MISSING_PRECIPITATION_TYPE)
{
  gatherPrecipitationData();
  findOutPrecipitationPeriods();
  findOutPrecipitationWeatherEvents();
}

PrecipitationForecast::~PrecipitationForecast()
{
  theCoastalData.clear();
  theInlandData.clear();
  theFullData.clear();
}

bool PrecipitationForecast::singleForm(const precipitation_form_id& thePrecipitationForm)
{
  return thePrecipitationForm == WATER_FORM || thePrecipitationForm == WATER_FREEZING_FORM ||
         thePrecipitationForm == DRIZZLE_FORM || thePrecipitationForm == DRIZZLE_FREEZING_FORM ||
         thePrecipitationForm == SLEET_FORM || thePrecipitationForm == SNOW_FORM ||
         thePrecipitationForm == FREEZING_FORM ||
         thePrecipitationForm == WATER_DRIZZLE_FREEZING_FORM ||
         thePrecipitationForm == WATER_DRIZZLE_FORM ||
         thePrecipitationForm == MISSING_PRECIPITATION_FORM;
}

// this function is set up because of language versions, for eample in swedish: regnet/snöfallet
// upphör
precipitation_form_id PrecipitationForecast::getPoutaantuuPrecipitationForm() const
{
  precipitation_form_id retval(MISSING_PRECIPITATION_FORM);

  switch (thePrecipitationFormBeforeDryPeriod)
  {
    case MISSING_PRECIPITATION_FORM:
    case WATER_SLEET_SNOW_FORM:
    case WATER_SNOW_FREEZING_FORM:
    case WATER_SNOW_FORM:
    case WATER_DRIZZLE_SNOW_FORM:
    case WATER_SLEET_FREEZING_FORM:
    case WATER_SLEET_FORM:
    case WATER_DRIZZLE_SLEET_FORM:
    case WATER_DRIZZLE_FREEZING_FORM:
    case WATER_DRIZZLE_FORM:
    case WATER_FREEZING_FORM:
    case FREEZING_FORM:
    case WATER_FORM:
      retval = WATER_FORM;
      break;
    case SLEET_SNOW_FREEZING_FORM:
    case SLEET_SNOW_FORM:
    case SLEET_FREEZING_FORM:
    case SLEET_FORM:
      retval = SLEET_FORM;
      break;
    case SNOW_FORM:
    case SNOW_FREEZING_FORM:
      retval = SNOW_FORM;
      break;
    case DRIZZLE_SNOW_FREEZING_FORM:
    case DRIZZLE_SNOW_FORM:
    case DRIZZLE_SLEET_SNOW_FORM:
    case DRIZZLE_FORM:
    case DRIZZLE_FREEZING_FORM:
    case DRIZZLE_SLEET_FORM:
    case DRIZZLE_SLEET_FREEZING_FORM:
      retval = DRIZZLE_FORM;
      break;
  }

  return retval;
}

Sentence PrecipitationForecast::getThunderSentence(const WeatherPeriod& thePeriod,
                                                   const unsigned short& theForecastAreaId,
                                                   const string& theVariable) const
{
  Sentence thunderSentence;

  forecast_area_id theAreaId = NO_AREA;

  if (theForecastAreaId & FULL_AREA)
    theAreaId = FULL_AREA;
  else if (theForecastAreaId & INLAND_AREA)
    theAreaId = INLAND_AREA;
  else if (theForecastAreaId & COASTAL_AREA)
    theAreaId = COASTAL_AREA;

  thunderSentence << theParameters.theThunderForecast->thunderSentence(
      thePeriod, theAreaId, theVariable);

  return thunderSentence;
}

bool PrecipitationForecast::thunderExists(const WeatherPeriod& thePeriod,
                                          const unsigned short& theForecastAreaId,
                                          const string& theVariable) const
{
  Sentence thunderSentence;

  thunderSentence << getThunderSentence(thePeriod, theForecastAreaId, theVariable);

  return (thunderSentence.size() > 0);
}

}  // namespace TextGen

// ======================================================================
