// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::WindStoryTools
 */
// ======================================================================

#ifndef TEXTGEN_WINDSTORYTOOLS_H
#define TEXTGEN_WINDSTORYTOOLS_H

#include <string>
#include <vector>
#include <calculator/AnalysisSources.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherPeriod.h>
#include <calculator/WeatherResult.h>

namespace TextGen
{
class WeatherResult;
}

namespace TextGen
{
class Sentence;

namespace WindStoryTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Wind direction accuracy class
 */
// ----------------------------------------------------------------------

enum WindDirectionAccuracy
{
  good_accuracy,
  moderate_accuracy,
  bad_accuracy
};

enum CompassType
{
  eight_directions,
  sixteen_directions
};

enum WindSpeedId
{
  TYYNI,
  HEIKKO,       // 0.5...3.5
  KOHTALAINEN,  // 3.5...7.5
  NAVAKKA,      // 7.5...13.5
  KOVA,         // 13.5...20.5
  MYRSKY,       // 20.5...32.5
  HIRMUMYRSKY   // 32.5...
};

enum WindDirectionId
{
  POHJOINEN = 0x1,
  POHJOINEN_KOILLINEN,
  KOILLINEN,
  ITA_KOILLINEN,
  ITA,
  ITA_KAAKKO,
  KAAKKO,
  ETELA_KAAKKO,
  ETELA,
  ETELA_LOUNAS,
  LOUNAS,
  LANSI_LOUNAS,
  LANSI,
  LANSI_LUODE,
  LUODE,
  POHJOINEN_LUODE,
  POHJOISEN_PUOLEINEN,
  KOILLISEN_PUOLEINEN,
  IDAN_PUOLEINEN,
  KAAKON_PUOLEINEN,
  ETELAN_PUOLEINEN,
  LOUNAAN_PUOLEINEN,
  LANNEN_PUOLEINEN,
  LUOTEEN_PUOLEINEN,
  VAIHTELEVA,
  MISSING_WIND_DIRECTION_ID
};

enum WindEventId
{
  TUULI_HEIKKENEE = 0x1,
  TUULI_VOIMISTUU = 0x2,
  TUULI_TYYNTYY = 0x4,
  TUULI_KAANTYY = 0x8,
  TUULI_MUUTTUU_VAIHTELEVAKSI = 0x10,
  TUULI_KAANTYY_JA_HEIKKENEE = 0x9,
  TUULI_KAANTYY_JA_VOIMISTUU = 0xA,
  TUULI_KAANTYY_JA_TYYNTYY = 0xC,
  TUULI_MUUTTUU_VAIHTELEVAKSI_JA_HEIKKENEE = 0x11,
  TUULI_MUUTTUU_VAIHTELEVAKSI_JA_VOIMISTUU = 0x12,
  TUULI_MUUTTUU_VAIHTELEVAKSI_JA_TYYNTYY = 0x14,
  MISSING_WIND_EVENT = 0x0,
  MISSING_WIND_SPEED_EVENT = -0x1,
  MISSING_WIND_DIRECTION_EVENT = -0x2
};

enum WindEventType
{
  WIND_DIRECTION_EVENT,
  WIND_SPEED_EVENT,
  MISSING_EVENT_TYPE
};

#define HEIKKO_LOWER_LIMIT 0.5
#define HEIKKO_UPPER_LIMIT 3.5
#define KOHTALAINEN_LOWER_LIMIT 3.5
#define KOHTALAINEN_UPPER_LIMIT 7.5
#define NAVAKKA_LOWER_LIMIT 7.5
#define NAVAKKA_UPPER_LIMIT 13.5
#define KOVA_LOWER_LIMIT 13.5
#define KOVA_UPPER_LIMIT 20.5
#define GALE_LOWER_LIMIT 17.5
#define MYRSKY_LOWER_LIMIT 20.5
#define MYRSKY_UPPER_LIMIT 32.5
#define HIRMUMYRSKY_LOWER_LIMIT 32.5
#define WEAK_WIND_SPEED_UPPER_LIMIT 5.0

#define TYYNI_WORD "tyyni"
#define HEIKKO_WORD "heikko"
#define KOHTALAINEN_WORD "kohtalainen"
#define NAVAKKA_WORD "navakka"
#define KOVA_WORD "kova"
#define MYRSKY_WORD "myrsky"
#define HIRMUMYRSKY_WORD "hirmumyrsky"

// contains wind speed interval minimum and maximum values
typedef std::vector<std::pair<float, float> > wind_speed_vector;

WindDirectionAccuracy direction_accuracy(double theAccuracy, const std::string& theVariable);
int direction8th(double theDirection);

int direction16th(double theDirection);

Sentence direction_sentence(const TextGen::WeatherResult& theDirection,
                            const std::string& theVariable);

std::string direction_string(const TextGen::WeatherResult& theDirection,
                             const std::string& theVariable);

std::string direction16_string(const TextGen::WeatherResult& theDirection,
                               const std::string& theVariable);

Sentence speed_range_sentence(const TextGen::WeatherResult& theMinSpeed,
                              const TextGen::WeatherResult& theMaxSpeed,
                              const TextGen::WeatherResult& theMeanSpeed,
                              const std::string& theVariable);

Sentence directed_speed_sentence(const TextGen::WeatherResult& theMinSpeed,
                                 const TextGen::WeatherResult& theMaxSpeed,
                                 const TextGen::WeatherResult& theMeanSpeed,
                                 const TextGen::WeatherResult& theDirection,
                                 const std::string& theVariable);

std::string directed_speed_string(const TextGen::WeatherResult& theMeanSpeed,
                                  const TextGen::WeatherResult& theDirection,
                                  const std::string& theVariable);

std::string directed16_speed_string(const TextGen::WeatherResult& theMeanSpeed,
                                    const TextGen::WeatherResult& theDirection,
                                    const std::string& theVariable);

std::string speed_string(const TextGen::WeatherResult& theMeanSpeed);

WindSpeedId wind_speed_id(const TextGen::WeatherResult& theWindSpeed);

WindSpeedId wind_speed_id(float theWindSpeed);

std::string wind_speed_string(const WindSpeedId& theWindSpeedId);

std::pair<int, int> wind_speed_interval(const wind_speed_vector& theWindSpeedVector);

WindDirectionId wind_direction_id(const TextGen::WeatherResult& theWindDirection,
                                  const TextGen::WeatherResult& theMaxWindSpeed,
                                  const std::string& theVariable,
                                  double theWindDirectionMinSpeed);

WindDirectionId plain_wind_direction_id(const TextGen::WeatherResult& theWindDirection,
                                        const TextGen::WeatherResult& theMaxWindSpeed,
                                        const std::string& theVariable,
                                        double theWindDirectionMinSpeed);

WindDirectionId direction_between_id(float theWindDirection);

std::string wind_direction_string(const WindDirectionId& theWindDirectionId);

WeatherResult mode_wind_direction(const AnalysisSources& theSources,
                                  const WeatherArea& theArea,
                                  const WeatherPeriod& thePeriod,
                                  const WeatherResult& theMedianWind,
                                  const WeatherResult& theTopWind,
                                  const std::string& theVar);
void populate_winddirection_distribution_time_series(
    const AnalysisSources& theSources,
    const WeatherArea& theArea,
    const WeatherPeriod& thePeriod,
    const std::string& theVar,
    std::vector<std::pair<float, WeatherResult> >& theWindDirectionDistribution,
    WindStoryTools::CompassType compass_type = sixteen_directions);

}  // namespace WindStoryTools
}  // namespace TextGen

#endif  // TEXTGEN_WINDSTORYTOOLS_H

// ======================================================================
