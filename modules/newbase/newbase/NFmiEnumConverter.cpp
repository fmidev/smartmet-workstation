// ======================================================================
/*!
 * \file NFmiEnumConverter.cpp
 * \brief Implementation of class NFmiEnumConverter
 */
// ======================================================================
/*!
 * \class NFmiEnumConverter
 *
 * A parameter descriptor type conversion class.
 *
 * Note that the single data member is made static and is hidden
 * in the .cpp file.
 *
 * Correct usage:
 *
 * \code
 *  NFmiParamConverter myconverter;
 *  NFmiParameterName temp = myconverter.ToName("Temperature");
 *  string str = myconverter.ToString(kNFmiTemperature);
 * \endcode
 *
 * Available converters are created with
 *
 * \code
 *  NFmiParamConverter myconverter(enumname);
 * \endcode
 *
 * where enumname is one of
 *
 * \code
 *     kParamNames (default)
 *     kRoadRegions
 *     kPressRegions
 * \endcode
 *
 * Works, but is slow if the converter is used multiple times:
 *
 * \code
 *	FmiParameterName temp = NFmiParamConverter().ToName("Temperature");
 *	string str = NFmiParamConverter().ToString(kNFmiTemperature);
 * \endcode
 *
 * The former returns kNFmiBadParameter if the string is unidentified,
 * the latter an empty string
 *
 * \todo Make the access methods const by using mutable if necessary
 */
// ======================================================================

#include "NFmiEnumConverter.h"
#include "NFmiDef.h"
#include "NFmiPressMasks.h"
#include "NFmiTiesaaAlueet.h"

#define PARAMINSERT(A, B) theData[A] = (B)

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 *
 * \todo Move this to the cpp file
 */
// ----------------------------------------------------------------------

NFmiEnumConverter::~NFmiEnumConverter() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theEnumspace Undocumented
 *
 * \todo Move this to the cpp file
 */
// ----------------------------------------------------------------------

NFmiEnumConverter::NFmiEnumConverter(FmiEnumSpace theEnumspace)
    : itsEnumSpace(theEnumspace), itsData(), itsBadEnum()
{
}

// ----------------------------------------------------------------------
/*!
 * Convert parameter string to parameter name.
 *
 * Return value is kFmiBadParameter if string is unknown.
 *
 * This method is fast, once it has been used atleast once so that
 * the parameter table has been initialized.
 *
 * \param theString The name of the paramter
 * \return The enumeration value
 *
 * \todo Use const_iterator internally
 */
// ----------------------------------------------------------------------

int NFmiEnumConverter::ToEnum(const char *s)
{
  EnumTableInit();

  auto iter = itsData.find(s);

  if (iter == itsData.end())
    return itsBadEnum;
  else
    return iter->second;
}

// ----------------------------------------------------------------------
/*!
 * Convert parameter name to parameter string
 */
// ----------------------------------------------------------------------

const std::string NFmiEnumConverter::ToString(int theValue)
{
  const char *ptr = ToCharPtr(theValue);
  if (ptr == nullptr)
    return "";
  else
    return ptr;
}

// ----------------------------------------------------------------------
/*!
 * Convert parameter name to parameter string.
 *
 * Returns an empty string if the value is invalid.
 *
 * This method is relatively slow, as it uses a linear search
 * in a large collection of parameter value-name pairs.
 *
 * \param theName The enumeration value to search for
 * \returns The named enumeration as a string
 *
 * \todo Use const_iterator internally
 */
// ----------------------------------------------------------------------

const char *NFmiEnumConverter::ToCharPtr(int theName)
{
  EnumTableInit();

  for (auto &iter : itsData)
  {
    if (iter.second == theName) return iter.first;
  }
  return nullptr;
}

/* Was (AKa 9-Apr-09):
const string & NFmiEnumConverter::ToString(int theName)
{
  EnumTableInit();

  // Search for the value. We avoid using a functor with find_if
  // on purpose, our own loop is just as good and more readable.

  storage_type::iterator iter;

  for(iter=itsData.begin() ; iter!=itsData.end(); ++iter)
    {
      if(iter->second == theName)
                return iter->first;
    }

  static string BadParameter("");
  return BadParameter;
}
*/

// ----------------------------------------------------------------------
/*!
 * Return the registered names in a list.
 *
 * \return List of parameter names
 */
// ----------------------------------------------------------------------

list<string> NFmiEnumConverter::Names()
{
  EnumTableInit();

  list<string> out;

  storage_type::const_iterator iter;

  for (iter = itsData.begin(); iter != itsData.end(); ++iter)
    out.emplace_back(iter->first);

  return out;
}

// ----------------------------------------------------------------------

static void InitRoadRegions(NFmiEnumConverter::storage_type &theData)
{
  PARAMINSERT("None", kTieAlueNone);
  PARAMINSERT("U1", kTieAlueU1);
  PARAMINSERT("U2", kTieAlueU2);
  PARAMINSERT("U3", kTieAlueU3);
  PARAMINSERT("U4", kTieAlueU4);
  PARAMINSERT("U5", kTieAlueU5);
  PARAMINSERT("T1", kTieAlueT1);
  PARAMINSERT("T2", kTieAlueT2);
  PARAMINSERT("T3", kTieAlueT3);
  PARAMINSERT("T4", kTieAlueT4);
  PARAMINSERT("T5", kTieAlueT5);
  PARAMINSERT("H1", kTieAlueH1);
  PARAMINSERT("H2", kTieAlueH2);
  PARAMINSERT("H3", kTieAlueH3);
  PARAMINSERT("H4", kTieAlueH4);
  PARAMINSERT("H5", kTieAlueH5);
  PARAMINSERT("K1", kTieAlueK1);
  PARAMINSERT("K2", kTieAlueK2);
  PARAMINSERT("K3", kTieAlueK3);
  PARAMINSERT("K4", kTieAlueK4);
  PARAMINSERT("S1", kTieAlueS1);
  PARAMINSERT("S2", kTieAlueS2);
  PARAMINSERT("S3", kTieAlueS3);
  PARAMINSERT("S4", kTieAlueS4);
  PARAMINSERT("S5", kTieAlueS5);
  PARAMINSERT("V1", kTieAlueV1);
  PARAMINSERT("V2", kTieAlueV2);
  PARAMINSERT("V4", kTieAlueV4);
  PARAMINSERT("V5", kTieAlueV5);
  PARAMINSERT("O1e", kTieAlueO1e);
  PARAMINSERT("O1p", kTieAlueO1p);
  PARAMINSERT("O2", kTieAlueO2);
  PARAMINSERT("O3", kTieAlueO3);
  PARAMINSERT("O4", kTieAlueO4);
  PARAMINSERT("O5", kTieAlueO5);
  PARAMINSERT("L1", kTieAlueL1);
  PARAMINSERT("L2", kTieAlueL2);
  PARAMINSERT("L3", kTieAlueL3);
  PARAMINSERT("L4", kTieAlueL4);
  PARAMINSERT("L5", kTieAlueL5);
  PARAMINSERT("Ke1", kTieAlueKe1);
  PARAMINSERT("Ke2", kTieAlueKe2);
}

static void InitPressRegions(NFmiEnumConverter::storage_type &theData)
{
  PARAMINSERT("None", kPressMaskNone);
  PARAMINSERT("TampereenAlue", kPressTampereenAlue);
}

static void InitParamNames(NFmiEnumConverter::storage_type &theData)
{
  PARAMINSERT("1CloudBase", kFmi1CloudBase);
  PARAMINSERT("1CloudCover", kFmi1CloudCover);
  PARAMINSERT("1CloudTop", kFmi1CloudTop);
  PARAMINSERT("1CloudType", kFmi1CloudType);
  PARAMINSERT("2CloudBase", kFmi2CloudBase);
  PARAMINSERT("2CloudCover", kFmi2CloudCover);
  PARAMINSERT("2CloudTop", kFmi2CloudTop);
  PARAMINSERT("2CloudType", kFmi2CloudType);
  PARAMINSERT("3CloudBase", kFmi3CloudBase);
  PARAMINSERT("3CloudCover", kFmi3CloudCover);
  PARAMINSERT("3CloudTop", kFmi3CloudTop);
  PARAMINSERT("3CloudType", kFmi3CloudType);
  PARAMINSERT("4CloudBase", kFmi4CloudBase);
  PARAMINSERT("4CloudCover", kFmi4CloudCover);
  PARAMINSERT("4CloudTop", kFmi4CloudTop);
  PARAMINSERT("4CloudType", kFmi4CloudType);
  PARAMINSERT("AbsoluteHumidity", kFmiAbsoluteHumidity);
  PARAMINSERT("AQIndex", kFmiAQIndex);
  PARAMINSERT("AccumulationTime", kFmiAccumulationTime);
  PARAMINSERT("Albedo", kFmiAlbedo);
  PARAMINSERT("AreaOfRisk", kFmiAreaOfRisk);
  PARAMINSERT("AshConcentration", kFmiAshConcentration);
  PARAMINSERT("AshOnOff", kFmiAshOnOff);
  PARAMINSERT("AvailableEnsembleMemberCount", kFmiAvailableEnsembleMemberCount);
  PARAMINSERT("AverageHumidity", kFmiAverageHumidity);
  PARAMINSERT("AveragePressure", kFmiAveragePressure);
  PARAMINSERT("AverageTemperature", kFmiAverageTemperature);
  PARAMINSERT("AverageWindDirection", kFmiAverageWindDirection);
  PARAMINSERT("AverageWindSpeedMS", kFmiAverageWindSpeedMS);
  PARAMINSERT("AviVis", kFmiAviationVisibility);  // SPELLING!
  PARAMINSERT("AviationWeather1", kFmiAviationWeather1);
  PARAMINSERT("AviationWeather2", kFmiAviationWeather2);
  PARAMINSERT("AviationWeather3", kFmiAviationWeather3);
  PARAMINSERT("AzimuthalLayer", kFmiAzimuthalLayer);
  PARAMINSERT("BaseOfTurbulence", kFmiBaseOfTurbulence);
  PARAMINSERT("BatteryVoltage", kFmiBatteryVoltage);
  PARAMINSERT("BioMassSum", kFmiBioMassSum);
  PARAMINSERT("BioMassSumChange", kFmiBioMassSumChange);
  PARAMINSERT("BioMassSumChangeP", kFmiBioMassSumChangeP);
  PARAMINSERT("BoundaryLayerDissipation", kFmiBoundaryLayerDissipation);
  PARAMINSERT("BoundaryLayerTurbulence", kFmiBoundaryLayerTurbulence);
  PARAMINSERT("CAPE", kFmiCAPE);
  PARAMINSERT("CAPEF0", kFmiCAPEF0);
  PARAMINSERT("CAPEF1", kFmiCAPEF1);
  PARAMINSERT("CAPEF2.5", kFmiCAPEF2_5);
  PARAMINSERT("CAPEF5", kFmiCAPEF5);
  PARAMINSERT("CAPEF12.5", kFmiCAPEF12_5);
  PARAMINSERT("CAPEF50", kFmiCAPEF50);
  PARAMINSERT("CAPEF87.5", kFmiCAPEF87_5);
  PARAMINSERT("CAPEF95", kFmiCAPEF95);
  PARAMINSERT("CAPEF97.5", kFmiCAPEF97_5);
  PARAMINSERT("CAPEF99", kFmiCAPEF99);
  PARAMINSERT("CAPEF100", kFmiCAPEF100);
  PARAMINSERT("CAPPIHeight", kFmiCAPPIHeight);
  PARAMINSERT("CH112", kFmiCH112);
  PARAMINSERT("CIN", kFmiCIN);
  PARAMINSERT("COContents", kFmiCOContents);
  PARAMINSERT("CanopyWater", kFmiCanopyWater);
  PARAMINSERT("CbCloudBase", kFmiCbCloudBase);
  PARAMINSERT("CbCloudCover", kFmiCbCloudCover);
  PARAMINSERT("CbCloudTop", kFmiCbCloudTop);
  PARAMINSERT("ChillFactor", kFmiChillFactor);
  PARAMINSERT("CloudBase", kFmiCloudBase);
  PARAMINSERT("CloudBase2", kFmiCloudBase2);
  PARAMINSERT("CloudCeilingHFT", kFmiCloudCeilingHFT);
  PARAMINSERT("CloudCondensate", kFmiCloudCondensate);
  PARAMINSERT("CloudHeight", kFmiCloudHeight);
  PARAMINSERT("CloudIce", kFmiCloudIce);
  PARAMINSERT("CloudSymbol", kFmiCloudSymbol);
  PARAMINSERT("CloudTop", kFmiCloudTop);
  PARAMINSERT("CloudTop2", kFmiCloudTop2);
  PARAMINSERT("CloudTopBrightnessTemperature", kFmiCloudTopBrightnessTemperature);
  PARAMINSERT("CloudWater", kFmiCloudWater);
  PARAMINSERT("CloudWaterReflectivity", kFmiCloudWaterReflectivity);
  PARAMINSERT("ClusterGeopHeight1", kFmiClusterGeopHeight1);
  PARAMINSERT("ClusterGeopHeight2", kFmiClusterGeopHeight2);
  PARAMINSERT("ClusterGeopHeight3", kFmiClusterGeopHeight3);
  PARAMINSERT("ClusterGeopHeight4", kFmiClusterGeopHeight4);
  PARAMINSERT("ClusterGeopHeight5", kFmiClusterGeopHeight5);
  PARAMINSERT("ClusterGeopHeight6", kFmiClusterGeopHeight6);
  PARAMINSERT("ClusterTemperature1", kFmiClusterTemperature1);
  PARAMINSERT("ClusterTemperature2", kFmiClusterTemperature2);
  PARAMINSERT("ClusterTemperature3", kFmiClusterTemperature3);
  PARAMINSERT("ClusterTemperature4", kFmiClusterTemperature4);
  PARAMINSERT("ClusterTemperature5", kFmiClusterTemperature5);
  PARAMINSERT("ClusterTemperature6", kFmiClusterTemperature6);
  PARAMINSERT("ColumnNumber", kFmiColumnNumber);
  PARAMINSERT("Conductivity1", kFmiConductivity1);
  PARAMINSERT("Conductivity2", kFmiConductivity2);
  PARAMINSERT("ConvCloudSymbol", kFmiConvCloudSymbol);
  PARAMINSERT("ConvectiveAvailablePotentialEnergy", kFmiConvectiveAvailablePotentialEnergy);
  PARAMINSERT("ConvectiveCloudCover", kFmiConvectiveCloudCover);
  PARAMINSERT("ConvectiveSnow", kFmiConvectiveSnow);
  PARAMINSERT("ConvectiveSnowFallRate", kFmiConvectiveSnowFallRate);
  PARAMINSERT("CorrectedReflectivity", kFmiCorrectedReflectivity);
  PARAMINSERT("CrossSectionDirection", kFmiCrossSectionDirection);
  PARAMINSERT("CurrentDirection", kFmiCurrentDirection);
  PARAMINSERT("CurrentSpeed", kFmiCurrentSpeed);
  PARAMINSERT("CyanosSum", kFmiCyanosSum);
  PARAMINSERT("CyanosSumChange", kFmiCyanosSumChange);
  PARAMINSERT("CyanosSumChangeP", kFmiCyanosSumChangeP);
  PARAMINSERT("DailyDiffuseRadiation", kFmiDailyDiffuseRadiation);
  PARAMINSERT("DailyGlobalRadiation", kFmiDailyGlobalRadiation);
  PARAMINSERT("DailyMeanTemperature", kFmiDailyMeanTemperature);
  PARAMINSERT("DailyNetRadiation", kFmiDailyNetRadiation);
  PARAMINSERT("DailyPrecipitationCode", kFmiDailyPrecipitationCode);
  PARAMINSERT("DailyReflectedRadiation", kFmiDailyReflectedRadiation);
  PARAMINSERT("Day", kFmiDay);
  PARAMINSERT("DeepRoadTemperature", kFmiDeepRoadTemperature);
  PARAMINSERT("DeepRoadTemperature2", kFmiDeepRoadTemperature2);
  PARAMINSERT("DeepSoilMoistureContent", kFmiDeepSoilMoistureContent);
  PARAMINSERT("DeepSoilTemperature", kFmiDeepSoilTemperature);
  PARAMINSERT("DegreeDays", kFmiDegreeDays);
  PARAMINSERT("DeIcingClass0Probability", kFmiDeIcingClass0Probability);
  PARAMINSERT("DeIcingClass1Probability", kFmiDeIcingClass1Probability);
  PARAMINSERT("DeIcingClass2Probability", kFmiDeIcingClass2Probability);
  PARAMINSERT("DeIcingClass3Probability", kFmiDeIcingClass3Probability);
  PARAMINSERT("DeIcingClass4Probability", kFmiDeIcingClass4Probability);
  PARAMINSERT("DeltaTime", kFmiDeltaTime);
  PARAMINSERT("Density", kFmiDensity);
  PARAMINSERT("Detectability", kFmiDetectability);
  PARAMINSERT("DewPoint", kFmiDewPoint);
  PARAMINSERT("DewPoint2M", kFmiDewPoint2M);
  PARAMINSERT("DewPointDeficit", kFmiDewPointDeficit);
  PARAMINSERT("DewPointF0", kFmiDewPointF0);
  PARAMINSERT("DewPointF1", kFmiDewPointF1);
  PARAMINSERT("DewPointF2.5", kFmiDewPointF2_5);
  PARAMINSERT("DewPointF5", kFmiDewPointF5);
  PARAMINSERT("DewPointF12.5", kFmiDewPointF12_5);
  PARAMINSERT("DewPointF50", kFmiDewPointF50);
  PARAMINSERT("DewPointF87.5", kFmiDewPointF87_5);
  PARAMINSERT("DewPointF95", kFmiDewPointF95);
  PARAMINSERT("DewPointF97.5", kFmiDewPointF97_5);
  PARAMINSERT("DewPointF99", kFmiDewPointF99);
  PARAMINSERT("DewPointF100", kFmiDewPointF100);
  PARAMINSERT("DifferentialPhase", kFmiDifferentialPhase);
  PARAMINSERT("DifferentialReflectivity", kFmiDifferentialReflectivity);
  PARAMINSERT("DivergenceAbsolute", kFmiDivergenceAbsolute);
  PARAMINSERT("DivergenceRelative", kFmiDivergenceRelative);
  PARAMINSERT("DragCoefficient", kFmiDragCoefficient);
  PARAMINSERT("DrySnowLoad", kFmiDrySnowLoad);
  PARAMINSERT("EFITemperature", kFmiEFITemperature);
  PARAMINSERT("EFIWindGust", kFmiEFIWindGust);
  PARAMINSERT("EFIWindPrecipitation", kFmiEFIWindPrecipitation);
  PARAMINSERT("EFIWindSpeed", kFmiEFIWindSpeed);
  PARAMINSERT("EchoTop", kFmiEchoTop);
  PARAMINSERT("EffectiveTemperatureSum", kFmiEffectiveTemperatureSum);
  PARAMINSERT("EffectiveTemperatureSumDeviation", kFmiEffectiveTemperatureSumDeviation);
  PARAMINSERT("ElevationAngle", kFmiElevationAngle);
  PARAMINSERT("Evaporation", kFmiEvaporation);
  PARAMINSERT("Evaporation18", kFmiEvaporation18);
  PARAMINSERT("ExceptionalPrecipitation", kFmiExceptionalPrecipitation);
  PARAMINSERT("FL1Base", kFmi_FL_1_Base);            // SPELLING!
  PARAMINSERT("FL1CloudType", kFmi_FL_1_CloudType);  // SPELLING!
  PARAMINSERT("FL1Cover", kFmi_FL_1_Cover);          // SPELLING!
  PARAMINSERT("FL1Top", kFmi_FL_1_Top);              // SPELLING!
  PARAMINSERT("FL2Base", kFmi_FL_2_Base);            // SPELLING!
  PARAMINSERT("FL2CloudType", kFmi_FL_2_CloudType);  // SPELLING!
  PARAMINSERT("FL2Cover", kFmi_FL_2_Cover);          // SPELLING!
  PARAMINSERT("FL2Top", kFmi_FL_2_Top);              // SPELLING!
  PARAMINSERT("FL3Base", kFmi_FL_3_Base);            // SPELLING!
  PARAMINSERT("FL3CloudType", kFmi_FL_3_CloudType);  // SPELLING!
  PARAMINSERT("FL3Cover", kFmi_FL_3_Cover);          // SPELLING!
  PARAMINSERT("FL3Top", kFmi_FL_3_Top);              // SPELLING!
  PARAMINSERT("FL4Base", kFmi_FL_4_Base);            // SPELLING!
  PARAMINSERT("FL4CloudType", kFmi_FL_4_CloudType);  // SPELLING!
  PARAMINSERT("FL4Cover", kFmi_FL_4_Cover);          // SPELLING!
  PARAMINSERT("FL4Top", kFmi_FL_4_Top);              // SPELLING!
  PARAMINSERT("FL5Base", kFmi_FL_5_Base);            // SPELLING!
  PARAMINSERT("FL5CloudType", kFmi_FL_5_CloudType);  // SPELLING!
  PARAMINSERT("FL5Cover", kFmi_FL_5_Cover);          // SPELLING!
  PARAMINSERT("FL5Top", kFmi_FL_5_Top);              // SPELLING!
  PARAMINSERT("FL6Base", kFmi_FL_6_Base);            // SPELLING!
  PARAMINSERT("FL6CloudType", kFmi_FL_6_CloudType);  // SPELLING!
  PARAMINSERT("FL6Cover", kFmi_FL_6_Cover);          // SPELLING!
  PARAMINSERT("FL6Top", kFmi_FL_6_Top);              // SPELLING!
  PARAMINSERT("FL7Base", kFmi_FL_7_Base);            // SPELLING!
  PARAMINSERT("FL7CloudType", kFmi_FL_7_CloudType);  // SPELLING!
  PARAMINSERT("FL7Cover", kFmi_FL_7_Cover);          // SPELLING!
  PARAMINSERT("FL7Top", kFmi_FL_7_Top);              // SPELLING!
  PARAMINSERT("FL8Base", kFmi_FL_8_Base);            // SPELLING!
  PARAMINSERT("FL8CloudType", kFmi_FL_8_CloudType);  // SPELLING!
  PARAMINSERT("FL8Cover", kFmi_FL_8_Cover);          // SPELLING!
  PARAMINSERT("FL8Top", kFmi_FL_8_Top);              // SPELLING!
  PARAMINSERT("FLCbBase", kFmi_FL_Cb_Base);          // SPELLING!
  PARAMINSERT("FLCbCover", kFmi_FL_Cb_Cover);        // SPELLING!
  PARAMINSERT("FLMaxBase", kFmi_FL_Max_Base);        // SPELLING!
  PARAMINSERT("FLMinBase", kFmi_FL_Min_Base);        // SPELLING!
  PARAMINSERT("FieldGroundHumidity", kFmiFieldGroundHumidity);
  PARAMINSERT("FlAltitude", kFmiFlAltitude);
  PARAMINSERT("FlagDate", kFmiFlagDate);
  PARAMINSERT("FlashAccuracy", kFmiFlashAccuracy);
  PARAMINSERT("FlashDirection", kFmiFlashDirection);
  PARAMINSERT("FlashDuration", kFmiFlashDuration);
  PARAMINSERT("FlashLength", kFmiFlashLength);
  PARAMINSERT("FlashMultiplicity", kFmiFlashMultiplicity);
  PARAMINSERT("FlashStrength", kFmiFlashStrength);
  PARAMINSERT("FlashWidth", kFmiFlashWidth);
  PARAMINSERT("FogIntensity", kFmiFogIntensity);
  PARAMINSERT("FogPercentage", kFmiFogPercentage);
  PARAMINSERT("FogSymbol", kFmiFogSymbol);
  PARAMINSERT("ForecastPeriod", kFmiForecastPeriod);
  PARAMINSERT("ForecasterCode", kFmiForecasterCode);
  PARAMINSERT("ForestFireWarning", kFmiForestFireWarning);
  PARAMINSERT("ForestFraction", kFmiForestFraction);
  PARAMINSERT("ForestGroundHumidity", kFmiForestGroundHumidity);
  PARAMINSERT("FreezingLevel", kFmiFreezingLevel);
  PARAMINSERT("FreezingPoint1", kFmiFreezingPoint1);
  PARAMINSERT("FreezingPoint2", kFmiFreezingPoint2);
  PARAMINSERT("Friction", kFmiFriction);
  PARAMINSERT("Friction3", kFmiFriction3);
  PARAMINSERT("FrontSymbol", kFmiFrontSymbol);
  PARAMINSERT("FrostLayerBottom", kFmiFrostLayerBottom);
  PARAMINSERT("FrostLayerTop", kFmiFrostLayerTop);
  PARAMINSERT("FrostPoint", kFmiFrostPoint);
  PARAMINSERT("FrostProbability", kFmiFrostProbability);
  PARAMINSERT("FrostSum", kFmiFrostSum);
  PARAMINSERT("FrozenSnowLoad", kFmiFrozenSnowLoad);
  PARAMINSERT("GeomHeight", kFmiGeomHeight);
  PARAMINSERT("GeopHeight", kFmiGeopHeight);
  PARAMINSERT("GeopHeightF0", kFmiGeopHeightF0);
  PARAMINSERT("GeopHeightF1", kFmiGeopHeightF1);
  PARAMINSERT("GeopHeightF2.5", kFmiGeopHeightF2_5);
  PARAMINSERT("GeopHeightF5", kFmiGeopHeightF5);
  PARAMINSERT("GeopHeightF12.5", kFmiGeopHeightF12_5);
  PARAMINSERT("GeopHeightF50", kFmiGeopHeightF50);
  PARAMINSERT("GeopHeightF87.5", kFmiGeopHeightF87_5);
  PARAMINSERT("GeopHeightF95", kFmiGeopHeightF95);
  PARAMINSERT("GeopHeightF97.5", kFmiGeopHeightF97_5);
  PARAMINSERT("GeopHeightF99", kFmiGeopHeightF99);
  PARAMINSERT("GeopHeightF100", kFmiGeopHeightF100);
  PARAMINSERT("GeopotentialHeightAnomaly", kFmiGeopotentialHeightAnomaly);
  PARAMINSERT("GolfIndex", kFmiGolfIndex);
  PARAMINSERT("GraupelMixingRatio", kFmiGraupelMixingRatio);
  PARAMINSERT("GridOrientation", kFmiGridOrientation);
  PARAMINSERT("GroundFrost", kFmiGroundFrost);
  PARAMINSERT("GroundHumidityForest", kFmiGroundHumidityForest);
  PARAMINSERT("GroundTemperature", kFmiGroundTemperature);
  PARAMINSERT("GrowthPeriodDeviationInDays", kFmiGrowthPeriodDeviationInDays);
  PARAMINSERT("GrowthPeriodDeviationInPrcnt", kFmiGrowthPeriodDeviationInPrcnt);
  PARAMINSERT("GrowthPeriodEnded", kFmiGrowthPeriodEnded);
  PARAMINSERT("GrowthPeriodEvaporationSum", kFmiGrowthPeriodEvaporationSum);
  PARAMINSERT("GrowthPeriodOnOff", kFmiGrowthPeriodOnOff);
  PARAMINSERT("GrowthPeriodPentadEvaporation", kFmiGrowthPeriodPentadEvaporation);
  PARAMINSERT("GrowthPeriodPentadFrostNights", kFmiGrowthPeriodPentadFrostNights);
  PARAMINSERT("GrowthPeriodPentadPrecipitation", kFmiGrowthPeriodPentadPrecipitation);
  PARAMINSERT("GrowthPeriodPentadPrecipitationDays01mm",
              kFmiGrowthPeriodPentadPrecipitationDays01mm);
  PARAMINSERT("GrowthPeriodPentadPrecipitationDays1mm", kFmiGrowthPeriodPentadPrecipitationDays1mm);
  PARAMINSERT("GrowthPeriodPentadPrecipitationDeviation",
              kFmiGrowthPeriodPentadPrecipitationDeviation);
  PARAMINSERT("GrowthPeriodPentadRelativeHumidityDeviation",
              kFmiGrowthPeriodPentadRelativeHumidityDeviation);
  PARAMINSERT("GrowthPeriodPentadRelativeHumidityMax", kFmiGrowthPeriodPentadRelativeHumidityMax);
  PARAMINSERT("GrowthPeriodPentadRelativeHumidityMean", kFmiGrowthPeriodPentadRelativeHumidityMean);
  PARAMINSERT("GrowthPeriodPentadRelativeHumidityMin", kFmiGrowthPeriodPentadRelativeHumidityMin);
  PARAMINSERT("GrowthPeriodPentadSurfaceTemperatureMin",
              kFmiGrowthPeriodPentadSurfaceTemperatureMin);
  PARAMINSERT("GrowthPeriodPentadSurfaceTemperatureMin025Perc",
              kFmiGrowthPeriodPentadSurfaceTemperatureMin025Perc);
  PARAMINSERT("GrowthPeriodPentadTemperatureMax", kFmiGrowthPeriodPentadTemperatureMax);
  PARAMINSERT("GrowthPeriodPentadTemperatureMaxDeviation",
              kFmiGrowthPeriodPentadTemperatureMaxDeviation);
  PARAMINSERT("GrowthPeriodPentadTemperatureMean", kFmiGrowthPeriodPentadTemperatureMean);
  PARAMINSERT("GrowthPeriodPentadTemperatureMeanDeviation",
              kFmiGrowthPeriodPentadTemperatureMeanDeviation);
  PARAMINSERT("GrowthPeriodPentadTemperatureMin", kFmiGrowthPeriodPentadTemperatureMin);
  PARAMINSERT("GrowthPeriodPentadTemperatureMinDeviation",
              kFmiGrowthPeriodPentadTemperatureMinDeviation);
  PARAMINSERT("GrowthPeriodPrecipitation", kFmiGrowthPeriodPrecipitation);
  PARAMINSERT("GrowthPeriodPrecipitationSum", kFmiGrowthPeriodPrecipitationSum);
  PARAMINSERT("GrowthPeriodPrecipitationSumDeviationInPrcnt",
              kFmiGrowthPeriodPrecipitationSumDeviationInPrcnt);
  PARAMINSERT("GrowthPeriodStarted", kFmiGrowthPeriodStarted);
  PARAMINSERT("HakeMessageType", kFmiHakeMessageType);
  PARAMINSERT("HaKeTaskCode", kFmiHaKeTaskCode);
  PARAMINSERT("HeightMinus20C", kFmiHeightMinus20C);
  PARAMINSERT("Helicity", kFmiHelicity);
  PARAMINSERT("HighCloudCover", kFmiHighCloudCover);
  PARAMINSERT("HighCloudSymbol", kFmiHighCloudSymbol);
  PARAMINSERT("HighCloudType", kFmiHighCloudType);
  PARAMINSERT("HoarFrostLoad", kFmiHoarFrostLoad);
  PARAMINSERT("HorizontalResolution", kFmiHorizontalResolution);
  PARAMINSERT("Hour", kFmiHour);
  PARAMINSERT("HourlyDiffuseRadiation", kFmiHourlyDiffuseRadiation);
  PARAMINSERT("HourlyGlobalRadiation", kFmiHourlyGlobalRadiation);
  PARAMINSERT("HourlyMaxRainIntensity", kFmiHourlyMaxRainIntensity);
  PARAMINSERT("HourlyMaximumGust", kFmiHourlyMaximumGust);
  PARAMINSERT("HourlyMaximumGustKT", kFmiHourlyMaximumGustKT);
  PARAMINSERT("HourlyMaximumTemperature", kFmiHourlyMaximumTemperature);
  PARAMINSERT("HourlyMaximumWindSpeed", kFmiHourlyMaximumWindSpeed);
  PARAMINSERT("HourlyMinimumTemperature", kFmiHourlyMinimumTemperature);
  PARAMINSERT("HourlyMinimumWindSpeed", kFmiHourlyMinimumWindSpeed);
  PARAMINSERT("HourlyNetRadiation", kFmiHourlyNetRadiation);
  PARAMINSERT("HourlyPressure", kFmiHourlyPressure);
  PARAMINSERT("HourlyReflectedRadiation", kFmiHourlyReflectedRadiation);
  PARAMINSERT("HourlyRelativeHumidity", kFmiHourlyRelativeHumidity);
  PARAMINSERT("HourlySunShine", kFmiHourlySunShine);
  PARAMINSERT("HourlyTemperature", kFmiHourlyTemperature);
  PARAMINSERT("HourlyWaterTemperature", kFmiHourlyWaterTemperature);
  PARAMINSERT("HourlyWindDirection", kFmiHourlyWindDirection);
  PARAMINSERT("HourlyWindSpeed", kFmiHourlyWindSpeed);
  PARAMINSERT("Humidity", kFmiHumidity);
  PARAMINSERT("Humidity2M", kFmiHumidity2M);
  PARAMINSERT("HumidityCode", kFmiHumidityCode);
  PARAMINSERT("HumidityDeviation", kFmiHumidityDeviation);
  PARAMINSERT("HumidityRelativeToIce", kFmiHumidityRelativeToIce);
  PARAMINSERT("HumidityVoltage", kFmiHumidityVoltage);
  PARAMINSERT("IR", kFmiIR);
  PARAMINSERT("IceConcentration", kFmiIceConcentration);
  PARAMINSERT("IceContent", kFmiIceContent);
  PARAMINSERT("IceCover", kFmiIceCover);
  PARAMINSERT("IceCoverF0", kFmiIceCoverF0);
  PARAMINSERT("IceCoverF1", kFmiIceCoverF1);
  PARAMINSERT("IceCoverF2.5", kFmiIceCoverF2_5);
  PARAMINSERT("IceCoverF5", kFmiIceCoverF5);
  PARAMINSERT("IceCoverF12.5", kFmiIceCoverF12_5);
  PARAMINSERT("IceCoverF50", kFmiIceCoverF50);
  PARAMINSERT("IceCoverF87.5", kFmiIceCoverF87_5);
  PARAMINSERT("IceCoverF95", kFmiIceCoverF95);
  PARAMINSERT("IceCoverF97.5", kFmiIceCoverF97_5);
  PARAMINSERT("IceCoverF99", kFmiIceCoverF99);
  PARAMINSERT("IceCoverF100", kFmiIceCoverF100);
  PARAMINSERT("IceDegreeOfRidging", kFmiIceDegreeOfRidging);
  PARAMINSERT("IceDirection", kFmiIceDirection);
  PARAMINSERT("IceMass", kFmiIceMass);
  PARAMINSERT("IceMaxThickness", kFmiIceMaxThickness);
  PARAMINSERT("IceMinThickness", kFmiIceMinThickness);
  PARAMINSERT("IceSpeed", kFmiIceSpeed);
  PARAMINSERT("IceThickness", kFmiIceThickness);
  PARAMINSERT("IcingRate", kFmiIcingRate);
  PARAMINSERT("EastwardIceVelocity", kFmiIceVelocityU);
  PARAMINSERT("NorthwardIceVelocity", kFmiIceVelocityV);
  PARAMINSERT("Icing", kFmiIcing);
  PARAMINSERT("IcingWarningIndex", kFmiIcingWarningIndex);
  PARAMINSERT("KIndex", kFmiKIndex);
  PARAMINSERT("LCL", kFmiLCL);
  PARAMINSERT("LNB", kFmiLNB);
  PARAMINSERT("LandCover", kFmiLandCover);
  PARAMINSERT("LandPercentage", kFmiLandPercentage);
  PARAMINSERT("LandSeaMask", kFmiLandSeaMask);
  PARAMINSERT("LapseRate", kFmiLapseRate);
  PARAMINSERT("LapseRate2", kFmiLapseRate2);
  PARAMINSERT("LargeScaleSnow", kFmiLargeScaleSnow);
  PARAMINSERT("LargeScaleSnowFallRate", kFmiLargeScaleSnowFallRate);
  PARAMINSERT("LatentHeatFlux", kFmiLatentHeatFlux);
  PARAMINSERT("Latitude", kFmiLatitude);
  PARAMINSERT("LocationId", kFmiLocationId);
  PARAMINSERT("LogNatOfPressure", kFmiLogNatOfPressure);
  PARAMINSERT("Longitude", kFmiLongitude);
  PARAMINSERT("LowCloudCover", kFmiLowCloudCover);
  PARAMINSERT("LowCloudSymbol", kFmiLowCloudSymbol);
  PARAMINSERT("LowCloudType", kFmiLowCloudType);
  PARAMINSERT("MaxElevation", kFmiMaxElevation);
  PARAMINSERT("MaxLayerHeight", kFmiMaxLayerHeight);
  PARAMINSERT("MaxRange", kFmiMaxRange);
  PARAMINSERT("MaxWaveHeight", kFmiMaxWaveHeight);
  PARAMINSERT("MaxWavePeriod", kFmiMaxWavePeriod);
  PARAMINSERT("MaximumHumidity", kFmiMaximumHumidity);
  PARAMINSERT("MaximumIcingIndex", kFmiMaximumIcingIndex);
  PARAMINSERT("MaximumPressure", kFmiMaximumPressure);
  PARAMINSERT("MaximumTemperature", kFmiMaximumTemperature);
  PARAMINSERT("MaximumTemperatureF0", kFmiMaximumTemperatureF0);
  PARAMINSERT("MaximumTemperatureF1", kFmiMaximumTemperatureF1);
  PARAMINSERT("MaximumTemperatureF2.5", kFmiMaximumTemperatureF2_5);
  PARAMINSERT("MaximumTemperatureF5", kFmiMaximumTemperatureF5);
  PARAMINSERT("MaximumTemperatureF12.5", kFmiMaximumTemperatureF12_5);
  PARAMINSERT("MaximumTemperatureF50", kFmiMaximumTemperatureF50);
  PARAMINSERT("MaximumTemperatureF87.5", kFmiMaximumTemperatureF87_5);
  PARAMINSERT("MaximumTemperatureF95", kFmiMaximumTemperatureF95);
  PARAMINSERT("MaximumTemperatureF97.5", kFmiMaximumTemperatureF97_5);
  PARAMINSERT("MaximumTemperatureF99", kFmiMaximumTemperatureF99);
  PARAMINSERT("MaximumTemperatureF100", kFmiMaximumTemperatureF100);
  PARAMINSERT("MaximumTemperature06", kFmiMaximumTemperature06);
  PARAMINSERT("MaximumTemperature18", kFmiMaximumTemperature18);
  PARAMINSERT("MaximumTemperature24h", kFmiMaximumTemperature24h);
  PARAMINSERT("MaximumTemperature2m", kFmiMaximumTemperature2m);
  PARAMINSERT("MaximumWind", kFmiMaximumWind);
  PARAMINSERT("MaximumWindDirection", kFmiMaximumWindDirection);
  PARAMINSERT("MeanIceThickness", kFmiMeanIceThickness);
  PARAMINSERT("MediumCloudCover", kFmiMediumCloudCover);
  PARAMINSERT("MediumCloudSymbol", kFmiMediumCloudSymbol);
  PARAMINSERT("MetarObservationType", kFmiMetarObservationType);
  PARAMINSERT("MiddleAndLowCloudCover", kFmiMiddleAndLowCloudCover);
  PARAMINSERT("MiddleCloudType", kFmiMiddleCloudType);
  PARAMINSERT("MinRange", kFmiMinRange);
  PARAMINSERT("MindBZ", kFmiMindBZ);
  PARAMINSERT("MinimumGroundTemperature06", kFmiMinimumGroundTemperature06);
  PARAMINSERT("MinimumHumidity", kFmiMinimumHumidity);
  PARAMINSERT("MinimumPressure", kFmiMinimumPressure);
  PARAMINSERT("MinimumTemperature", kFmiMinimumTemperature);
  PARAMINSERT("MinimumTemperatureF0", kFmiMinimumTemperatureF0);
  PARAMINSERT("MinimumTemperatureF1", kFmiMinimumTemperatureF1);
  PARAMINSERT("MinimumTemperatureF2.5", kFmiMinimumTemperatureF2_5);
  PARAMINSERT("MinimumTemperatureF5", kFmiMinimumTemperatureF5);
  PARAMINSERT("MinimumTemperatureF12.5", kFmiMinimumTemperatureF12_5);
  PARAMINSERT("MinimumTemperatureF50", kFmiMinimumTemperatureF50);
  PARAMINSERT("MinimumTemperatureF87.5", kFmiMinimumTemperatureF87_5);
  PARAMINSERT("MinimumTemperatureF95", kFmiMinimumTemperatureF95);
  PARAMINSERT("MinimumTemperatureF97.5", kFmiMinimumTemperatureF97_5);
  PARAMINSERT("MinimumTemperatureF99", kFmiMinimumTemperatureF99);
  PARAMINSERT("MinimumTemperatureF100", kFmiMinimumTemperatureF100);
  PARAMINSERT("MinimumTemperature06", kFmiMinimumTemperature06);
  PARAMINSERT("MinimumTemperature18", kFmiMinimumTemperature18);
  PARAMINSERT("MinimumTemperature24h", kFmiMinimumTemperature24h);
  PARAMINSERT("MinimumTemperature2m", kFmiMinimumTemperature2m);
  PARAMINSERT("MinimumWind", kFmiMinimumWind);
  PARAMINSERT("MinimumWindDirection", kFmiMinimumWindDirection);
  PARAMINSERT("Minute", kFmiMinute);
  PARAMINSERT("Mist", kFmiMist);
  PARAMINSERT("MixedLayerDepth", kFmiMixedLayerDepth);
  PARAMINSERT("MixingRatio", kFmiMixingRatio);
  PARAMINSERT("ModelLevel", kFmiModelLevel);
  PARAMINSERT("MoistureDivergenceHoriz", kFmiMoistureDivergenceHoriz);
  PARAMINSERT("MoninObukhovLength", kFmiMoninObukhovLength);
  PARAMINSERT("Month", kFmiMonth);
  PARAMINSERT("Monthly15SnowDepth3160", kFmiMonthly15SnowDepth3160);
  PARAMINSERT("Monthly15SnowDepth6190", kFmiMonthly15SnowDepth6190);
  PARAMINSERT("MonthlyMeanCloudiness3160", kFmiMonthlyMeanCloudiness3160);
  PARAMINSERT("MonthlyMeanCloudiness6190", kFmiMonthlyMeanCloudiness6190);
  PARAMINSERT("MonthlyMeanPrecipitation3160", kFmiMonthlyMeanPrecipitation3160);
  PARAMINSERT("MonthlyMeanPrecipitation6190", kFmiMonthlyMeanPrecipitation6190);
  PARAMINSERT("MonthlyMeanTemperature", kFmiMonthlyMeanTemperature);
  PARAMINSERT("MonthlyMeanTemperature3160", kFmiMonthlyMeanTemperature3160);
  PARAMINSERT("MonthlyMeanTemperature6190", kFmiMonthlyMeanTemperature6190);
  PARAMINSERT("MonthlyPrecipitation", kFmiMonthlyPrecipitation);
  PARAMINSERT("NOContents", kFmiNOContents);
  PARAMINSERT("NO2Contents", kFmiNO2Contents);
  PARAMINSERT("NormalGroundMinTemperature", kFmiNormalGroundMinTemperature);
  PARAMINSERT("NormalGroundMinTemperatureF02", kFmiNormalGroundMinTemperatureF02);
  PARAMINSERT("NormalGroundMinTemperatureF12", kFmiNormalGroundMinTemperatureF12);
  PARAMINSERT("NormalGroundMinTemperatureF37", kFmiNormalGroundMinTemperatureF37);
  PARAMINSERT("NormalGroundMinTemperatureF50", kFmiNormalGroundMinTemperatureF50);
  PARAMINSERT("NormalGroundMinTemperatureF63", kFmiNormalGroundMinTemperatureF63);
  PARAMINSERT("NormalGroundMinTemperatureF88", kFmiNormalGroundMinTemperatureF88);
  PARAMINSERT("NormalGroundMinTemperatureF98", kFmiNormalGroundMinTemperatureF98);
  PARAMINSERT("NormalMaxTemperature", kFmiNormalMaxTemperature);
  PARAMINSERT("NormalMaxTemperatureF02", kFmiNormalMaxTemperatureF02);
  PARAMINSERT("NormalMaxTemperatureF12", kFmiNormalMaxTemperatureF12);
  PARAMINSERT("NormalMaxTemperatureF37", kFmiNormalMaxTemperatureF37);
  PARAMINSERT("NormalMaxTemperatureF50", kFmiNormalMaxTemperatureF50);
  PARAMINSERT("NormalMaxTemperatureF63", kFmiNormalMaxTemperatureF63);
  PARAMINSERT("NormalMaxTemperatureF88", kFmiNormalMaxTemperatureF88);
  PARAMINSERT("NormalMaxTemperatureF98", kFmiNormalMaxTemperatureF98);
  PARAMINSERT("NormalMeanTemperature", kFmiNormalMeanTemperature);
  PARAMINSERT("NormalMeanTemperatureF02", kFmiNormalMeanTemperatureF02);
  PARAMINSERT("NormalMeanTemperatureF12", kFmiNormalMeanTemperatureF12);
  PARAMINSERT("NormalMeanTemperatureF37", kFmiNormalMeanTemperatureF37);
  PARAMINSERT("NormalMeanTemperatureF50", kFmiNormalMeanTemperatureF50);
  PARAMINSERT("NormalMeanTemperatureF63", kFmiNormalMeanTemperatureF63);
  PARAMINSERT("NormalMeanTemperatureF88", kFmiNormalMeanTemperatureF88);
  PARAMINSERT("NormalMeanTemperatureF98", kFmiNormalMeanTemperatureF98);
  PARAMINSERT("NormalMinTemperature", kFmiNormalMinTemperature);
  PARAMINSERT("NormalMinTemperatureF02", kFmiNormalMinTemperatureF02);
  PARAMINSERT("NormalMinTemperatureF12", kFmiNormalMinTemperatureF12);
  PARAMINSERT("NormalMinTemperatureF37", kFmiNormalMinTemperatureF37);
  PARAMINSERT("NormalMinTemperatureF50", kFmiNormalMinTemperatureF50);
  PARAMINSERT("NormalMinTemperatureF63", kFmiNormalMinTemperatureF63);
  PARAMINSERT("NormalMinTemperatureF88", kFmiNormalMinTemperatureF88);
  PARAMINSERT("NormalMinTemperatureF98", kFmiNormalMinTemperatureF98);
  PARAMINSERT("O3Contents", kFmiO3Contents);
  PARAMINSERT("OceanIcing", kFmiOceanIcing);
  PARAMINSERT("OldVisibility", kFmiOldVisibility);
  PARAMINSERT("OpenLandSnowDepth", kFmiOpenLandSnowDepth);
  PARAMINSERT("OzoneAnomaly", kFmiOzoneAnomaly);
  PARAMINSERT("PackedAviationCloud1", kFmiPackedAviationCloud1);
  PARAMINSERT("PackedAviationCloud2", kFmiPackedAviationCloud2);
  PARAMINSERT("PackedAviationCloud3", kFmiPackedAviationCloud3);
  PARAMINSERT("PackedAviationCloud4", kFmiPackedAviationCloud4);
  PARAMINSERT("PastWeather1", kFmiPastWeather1);
  PARAMINSERT("PastWeather2", kFmiPastWeather2);
  PARAMINSERT("PedestrianIndex", kFmiPedestrianIndex);
  PARAMINSERT("PhaseOfFlight", kFmiPhaseOfFlight);
  PARAMINSERT("PM10Contents", kFmiPM10Contents);
  PARAMINSERT("PM25Contents", kFmiPM25Contents);
  PARAMINSERT("PoP", kFmiPoP);
  PARAMINSERT("PoP06", kFmiPoP06);
  PARAMINSERT("PoP18", kFmiPoP18);
  PARAMINSERT("PoP24h", kFmiPoP24h);
  PARAMINSERT("PollenIndexAlder", kFmiPollenIndexAlder);
  PARAMINSERT("PollenIndexBirch", kFmiPollenIndexBirch);
  PARAMINSERT("PollenIndexHay", kFmiPollenIndexHay);
  PARAMINSERT("PollenIndexMould", kFmiPollenIndexMould);
  PARAMINSERT("PollenIndexMugwort", kFmiPollenIndexMugwort);
  PARAMINSERT("PollenIndexNut", kFmiPollenIndexNut);
  PARAMINSERT("PollenIndexSum", kFmiPollenIndexSum);
  PARAMINSERT("PotentialPrecipitationForm", kFmiPotentialPrecipitationForm);
  PARAMINSERT("PotentialPrecipitationType", kFmiPotentialPrecipitationType);
  PARAMINSERT("PotentialTemperature", kFmiPotentialTemperature);
  PARAMINSERT("PrecipitableWater", kFmiPrecipitableWater);
  PARAMINSERT("PrecipitableWater2", kFmiPrecipitableWater2);
  PARAMINSERT("Precipitation06", kFmiPrecipitation06);
  PARAMINSERT("Precipitation10Days", kFmiPrecipitation10Days);
  PARAMINSERT("Precipitation12h", kFmiPrecipitation12h);
  PARAMINSERT("Precipitation18", kFmiPrecipitation18);
  PARAMINSERT("Precipitation1h", kFmiPrecipitation1h);
  PARAMINSERT("Precipitation24h", kFmiPrecipitation24h);
  PARAMINSERT("Precipitation30Days", kFmiPrecipitation30Days);
  PARAMINSERT("Precipitation3h", kFmiPrecipitation3h);
  PARAMINSERT("Precipitation3hF0", kFmiPrecipitation3hF0);
  PARAMINSERT("Precipitation3hF1", kFmiPrecipitation3hF1);
  PARAMINSERT("Precipitation3hF10", kFmiPrecipitation3hF10);
  PARAMINSERT("Precipitation3hF100", kFmiPrecipitation3hF100);
  PARAMINSERT("Precipitation3hF12", kFmiPrecipitation3hF12);
  PARAMINSERT("Precipitation3hF12.5", kFmiPrecipitation3hF12_5);
  PARAMINSERT("Precipitation3hF2", kFmiPrecipitation3hF2);
  PARAMINSERT("Precipitation3hF2.5", kFmiPrecipitation3hF2_5);
  PARAMINSERT("Precipitation3hF20", kFmiPrecipitation3hF20);
  PARAMINSERT("Precipitation3hF25", kFmiPrecipitation3hF25);
  PARAMINSERT("Precipitation3hF30", kFmiPrecipitation3hF30);
  PARAMINSERT("Precipitation3hF37", kFmiPrecipitation3hF37);
  PARAMINSERT("Precipitation3hF40", kFmiPrecipitation3hF40);
  PARAMINSERT("Precipitation3hF5", kFmiPrecipitation3hF5);
  PARAMINSERT("Precipitation3hF50", kFmiPrecipitation3hF50);
  PARAMINSERT("Precipitation3hF6", kFmiPrecipitation3hF6);
  PARAMINSERT("Precipitation3hF60", kFmiPrecipitation3hF60);
  PARAMINSERT("Precipitation3hF63", kFmiPrecipitation3hF63);
  PARAMINSERT("Precipitation3hF7", kFmiPrecipitation3hF7);
  PARAMINSERT("Precipitation3hF70", kFmiPrecipitation3hF70);
  PARAMINSERT("Precipitation3hF75", kFmiPrecipitation3hF75);
  PARAMINSERT("Precipitation3hF8", kFmiPrecipitation3hF8);
  PARAMINSERT("Precipitation3hF80", kFmiPrecipitation3hF80);
  PARAMINSERT("Precipitation3hF87.5", kFmiPrecipitation3hF87_5);
  PARAMINSERT("Precipitation3hF88", kFmiPrecipitation3hF88);
  PARAMINSERT("Precipitation3hF9", kFmiPrecipitation3hF9);
  PARAMINSERT("Precipitation3hF90", kFmiPrecipitation3hF90);
  PARAMINSERT("Precipitation3hF95", kFmiPrecipitation3hF95);
  PARAMINSERT("Precipitation3hF97.5", kFmiPrecipitation3hF97_5);
  PARAMINSERT("Precipitation3hF98", kFmiPrecipitation3hF98);
  PARAMINSERT("Precipitation3hF99", kFmiPrecipitation3hF99);
  PARAMINSERT("Precipitation4h", kFmiPrecipitation4h);
  PARAMINSERT("Precipitation5d", kFmiPrecipitation5d);
  PARAMINSERT("Precipitation6h", kFmiPrecipitation6h);
  PARAMINSERT("Precipitation7Days", kFmiPrecipitation7Days);
  PARAMINSERT("PrecipitationAmount", kFmiPrecipitationAmount);
  PARAMINSERT("PrecipitationAmount2", kFmiPrecipitationAmount2);
  PARAMINSERT("PrecipitationAnomaly10", kFmiPrecipitationAnomaly10);
  PARAMINSERT("PrecipitationAnomaly20", kFmiPrecipitationAnomaly20);
  PARAMINSERT("PrecipitationConv", kFmiPrecipitationConv);
  PARAMINSERT("PrecipitationForm", kFmiPrecipitationForm);
  PARAMINSERT("PrecipitationForm2", kFmiPrecipitationForm2);
  PARAMINSERT("PrecipitationForm3", kFmiPrecipitationForm3);
  PARAMINSERT("PrecipitationGraupel", kFmiPrecipitationGraupel);
  PARAMINSERT("PrecipitationInstantWater", kFmiPrecipitationInstantWater);
  PARAMINSERT("PrecipitationInstantGraupel", kFmiPrecipitationInstantGraupel);
  PARAMINSERT("PrecipitationInstantSnow", kFmiPrecipitationInstantSnow);
  PARAMINSERT("PrecipitationInstantSolid", kFmiPrecipitationInstantSolid);
  PARAMINSERT("PrecipitationInstantTotal", kFmiPrecipitationInstantTotal);
  PARAMINSERT("PrecipitationLarge", kFmiPrecipitationLarge);
  PARAMINSERT("PrecipitationPeriod", kFmiPrecipitationPeriod);
  PARAMINSERT("PrecipitationRate", kFmiPrecipitationRate);
  PARAMINSERT("PrecipitationRate2", kFmiPrecipitationRate2);
  PARAMINSERT("PrecipitationRateConv", kFmiPrecipitationRateConv);
  PARAMINSERT("PrecipitationRateGraupel", kFmiPrecipitationRateGraupel);
  PARAMINSERT("PrecipitationRateLarge", kFmiPrecipitationRateLarge);
  PARAMINSERT("PrecipitationRateSolid", kFmiPrecipitationRateSolid);
  PARAMINSERT("PrecipitationReliability", kFmiPrecipitationReliability);
  PARAMINSERT("PrecipitationSleet", kFmiPrecipitationSleet);
  PARAMINSERT("PrecipitationSnow", kFmiPrecipitationSnow);
  PARAMINSERT("PrecipitationSolid", kFmiPrecipitationSolid);
  PARAMINSERT("PrecipitationType", kFmiPrecipitationType);
  PARAMINSERT("PrecipitationWater", kFmiPrecipitationWater);
  PARAMINSERT("PresentWeather", kFmiPresentWeather);
  PARAMINSERT("Pressure", kFmiPressure);
  PARAMINSERT("PressureAnomaly", kFmiPressureAnomaly);
  PARAMINSERT("PressureAtStationLevel", kFmiPressureAtStationLevel);
  PARAMINSERT("PressureChange", kFmiPressureChange);
  PARAMINSERT("PressureCode", kFmiPressureCode);
  PARAMINSERT("PressureDeviation", kFmiPressureDeviation);
  PARAMINSERT("PressureF0", kFmiPressureF0);
  PARAMINSERT("PressureF1", kFmiPressureF1);
  PARAMINSERT("PressureF2.5", kFmiPressureF2_5);
  PARAMINSERT("PressureF5", kFmiPressureF5);
  PARAMINSERT("PressureF10", kFmiPressureF10);
  PARAMINSERT("PressureF12.5", kFmiPressureF12_5);
  PARAMINSERT("PressureF25", kFmiPressureF25);
  PARAMINSERT("PressureF50", kFmiPressureF50);
  PARAMINSERT("PressureF75", kFmiPressureF75);
  PARAMINSERT("PressureF87.5", kFmiPressureF87_5);
  PARAMINSERT("PressureF90", kFmiPressureF90);
  PARAMINSERT("PressureF95", kFmiPressureF95);
  PARAMINSERT("PressureF97.5", kFmiPressureF97_5);
  PARAMINSERT("PressureF99", kFmiPressureF99);
  PARAMINSERT("PressureF100", kFmiPressureF100);
  PARAMINSERT("PressureReduced", kFmiPressureReduced);
  PARAMINSERT("PressureTendency", kFmiPressureTendency);
  PARAMINSERT("PressureVoltage", kFmiPressureVoltage);
  PARAMINSERT("ProbabilityDryRoad", kFmiProbabilityDryRoad);
  PARAMINSERT("ProbabilityFrostyRoad", kFmiProbabilityFrostyRoad);
  PARAMINSERT("ProbabilityIcyRoad", kFmiProbabilityIcyRoad);
  PARAMINSERT("ProbabilityMoistRoad", kFmiProbabilityMoistRoad);
  PARAMINSERT("ProbabilityOfCAPE1", kFmiProbabilityOfCAPE1);
  PARAMINSERT("ProbabilityOfCAPE2", kFmiProbabilityOfCAPE2);
  PARAMINSERT("ProbabilityOfCAPE3", kFmiProbabilityOfCAPE3);
  PARAMINSERT("ProbabilityOfCAPE4", kFmiProbabilityOfCAPE4);
  PARAMINSERT("ProbabilityOfColdLimit1", kFmiProbabilityOfColdLimit1);
  PARAMINSERT("ProbabilityOfColdLimit2", kFmiProbabilityOfColdLimit2);
  PARAMINSERT("ProbabilityOfColdLimit3", kFmiProbabilityOfColdLimit3);
  PARAMINSERT("ProbabilityOfColdLimit4", kFmiProbabilityOfColdLimit4);
  PARAMINSERT("ProbabilityOfColdLimit5", kFmiProbabilityOfColdLimit5);
  PARAMINSERT("ProbabilityOfConvectivePrecipitationLimit1",
              kFmiProbabilityOfConvectivePrecipitationLimit1);
  PARAMINSERT("ProbabilityOfConvectivePrecipitationLimit2",
              kFmiProbabilityOfConvectivePrecipitationLimit2);
  PARAMINSERT("ProbabilityOfConvectivePrecipitationLimit3",
              kFmiProbabilityOfConvectivePrecipitationLimit3);
  PARAMINSERT("ProbabilityOfConvectivePrecipitationLimit4",
              kFmiProbabilityOfConvectivePrecipitationLimit4);
  PARAMINSERT("ProbabilityOfConvectivePrecipitationLimit5",
              kFmiProbabilityOfConvectivePrecipitationLimit5);
  PARAMINSERT("ProbabilityOfConvectivePrecipitationLimit6",
              kFmiProbabilityOfConvectivePrecipitationLimit6);
  PARAMINSERT("ProbabilityOfGustLimit1", kFmiProbabilityOfGustLimit1);
  PARAMINSERT("ProbabilityOfGustLimit2", kFmiProbabilityOfGustLimit2);
  PARAMINSERT("ProbabilityOfGustLimit3", kFmiProbabilityOfGustLimit3);
  PARAMINSERT("ProbabilityOfGustLimit4", kFmiProbabilityOfGustLimit4);
  PARAMINSERT("ProbabilityOfGustLimit5", kFmiProbabilityOfGustLimit5);
  PARAMINSERT("ProbabilityOfHeatLimit1", kFmiProbabilityOfHeatLimit1);
  PARAMINSERT("ProbabilityOfHeatLimit2", kFmiProbabilityOfHeatLimit2);
  PARAMINSERT("ProbabilityOfHeatLimit3", kFmiProbabilityOfHeatLimit3);
  PARAMINSERT("ProbabilityOfPr24Limit1", kFmiProbabilityOfPr24Limit1);
  PARAMINSERT("ProbabilityOfPr24Limit2", kFmiProbabilityOfPr24Limit2);
  PARAMINSERT("ProbabilityOfPr24Limit3", kFmiProbabilityOfPr24Limit3);
  PARAMINSERT("ProbabilityOfPr24Limit4", kFmiProbabilityOfPr24Limit4);
  PARAMINSERT("ProbabilityOfPr24Limit5", kFmiProbabilityOfPr24Limit5);
  PARAMINSERT("ProbabilityOfPr6Limit1", kFmiProbabilityOfPr6Limit1);
  PARAMINSERT("ProbabilityOfPr6Limit2", kFmiProbabilityOfPr6Limit2);
  PARAMINSERT("ProbabilityOfPr6Limit3", kFmiProbabilityOfPr6Limit3);
  PARAMINSERT("ProbabilityOfPr6Limit4", kFmiProbabilityOfPr6Limit4);
  PARAMINSERT("ProbabilityOfPr6Limit5", kFmiProbabilityOfPr6Limit5);
  PARAMINSERT("ProbabilityOfPr6Limit6", kFmiProbabilityOfPr6Limit6);
  PARAMINSERT("ProbabilityOfPrec", kFmiProbabilityOfPrec);
  PARAMINSERT("ProbabilityOfPrecFormDrizzle", kFmiProbabilityOfPrecFormDrizzle);
  PARAMINSERT("ProbabilityOfPrecFormFreezingDrizzle", kFmiProbabilityOfPrecFormFreezingDrizzle);
  PARAMINSERT("ProbabilityOfPrecFormFreezingRain", kFmiProbabilityOfPrecFormFreezingRain);
  PARAMINSERT("ProbabilityOfPrecFormRain", kFmiProbabilityOfPrecFormRain);
  PARAMINSERT("ProbabilityOfPrecFormSleet", kFmiProbabilityOfPrecFormSleet);
  PARAMINSERT("ProbabilityOfPrecFormSnow", kFmiProbabilityOfPrecFormSnow);
  PARAMINSERT("ProbabilityOfPrecLimit1", kFmiProbabilityOfPrecLimit1);
  PARAMINSERT("ProbabilityOfPrecLimit2", kFmiProbabilityOfPrecLimit2);
  PARAMINSERT("ProbabilityOfPrecLimit3", kFmiProbabilityOfPrecLimit3);
  PARAMINSERT("ProbabilityOfPrecLimit4", kFmiProbabilityOfPrecLimit4);
  PARAMINSERT("ProbabilityOfPrecLimit5", kFmiProbabilityOfPrecLimit5);
  PARAMINSERT("ProbabilityOfPrecLimit6", kFmiProbabilityOfPrecLimit6);
  PARAMINSERT("ProbabilityOfPrecLimit7", kFmiProbabilityOfPrecLimit7);
  PARAMINSERT("ProbabilityOfPrecLimit8", kFmiProbabilityOfPrecLimit8);
  PARAMINSERT("ProbabilityOfPrecLimit9", kFmiProbabilityOfPrecLimit9);
  PARAMINSERT("ProbabilityOfPrecLimit10", kFmiProbabilityOfPrecLimit10);
  PARAMINSERT("ProbabilityOfPrecipitation3h01mm", kFmiProbabilityOfPrecipitation3h01mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h03mm", kFmiProbabilityOfPrecipitation3h03mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h05mm", kFmiProbabilityOfPrecipitation3h05mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h0mm", kFmiProbabilityOfPrecipitation3h0mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h100mm", kFmiProbabilityOfPrecipitation3h100mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h10mm", kFmiProbabilityOfPrecipitation3h10mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h12mm", kFmiProbabilityOfPrecipitation3h12mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h14mm", kFmiProbabilityOfPrecipitation3h14mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h150mm", kFmiProbabilityOfPrecipitation3h150mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h16mm", kFmiProbabilityOfPrecipitation3h16mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h18mm", kFmiProbabilityOfPrecipitation3h18mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h1mm", kFmiProbabilityOfPrecipitation3h1mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h200mm", kFmiProbabilityOfPrecipitation3h200mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h20mm", kFmiProbabilityOfPrecipitation3h20mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h25mm", kFmiProbabilityOfPrecipitation3h25mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h2mm", kFmiProbabilityOfPrecipitation3h2mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h30mm", kFmiProbabilityOfPrecipitation3h30mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h35mm", kFmiProbabilityOfPrecipitation3h35mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h3mm", kFmiProbabilityOfPrecipitation3h3mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h40mm", kFmiProbabilityOfPrecipitation3h40mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h45mm", kFmiProbabilityOfPrecipitation3h45mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h4mm", kFmiProbabilityOfPrecipitation3h4mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h500mm", kFmiProbabilityOfPrecipitation3h500mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h50mm", kFmiProbabilityOfPrecipitation3h50mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h5mm", kFmiProbabilityOfPrecipitation3h5mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h60mm", kFmiProbabilityOfPrecipitation3h60mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h6mm", kFmiProbabilityOfPrecipitation3h6mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h70mm", kFmiProbabilityOfPrecipitation3h70mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h7mm", kFmiProbabilityOfPrecipitation3h7mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h80mm", kFmiProbabilityOfPrecipitation3h80mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h8mm", kFmiProbabilityOfPrecipitation3h8mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h90mm", kFmiProbabilityOfPrecipitation3h90mm);
  PARAMINSERT("ProbabilityOfPrecipitation3h9mm", kFmiProbabilityOfPrecipitation3h9mm);
  PARAMINSERT("ProbabilityOfSigWaveHeightLimit1", kFmiProbabilityOfSigWaveHeightLimit1);
  PARAMINSERT("ProbabilityOfSigWaveHeightLimit2", kFmiProbabilityOfSigWaveHeightLimit2);
  PARAMINSERT("ProbabilityOfSigWaveHeightLimit3", kFmiProbabilityOfSigWaveHeightLimit3);
  PARAMINSERT("ProbabilityOfSigWaveHeightLimit4", kFmiProbabilityOfSigWaveHeightLimit4);
  PARAMINSERT("ProbabilityOfSn24Limit1", kFmiProbabilityOfSn24Limit1);
  PARAMINSERT("ProbabilityOfSn24Limit2", kFmiProbabilityOfSn24Limit2);
  PARAMINSERT("ProbabilityOfSn24Limit3", kFmiProbabilityOfSn24Limit3);
  PARAMINSERT("ProbabilityOfSn24Limit4", kFmiProbabilityOfSn24Limit4);
  PARAMINSERT("ProbabilityOfSn24Limit5", kFmiProbabilityOfSn24Limit5);
  PARAMINSERT("ProbabilityOfSn24Limit6", kFmiProbabilityOfSn24Limit6);
  PARAMINSERT("ProbabilityOfSn6Limit1", kFmiProbabilityOfSn6Limit1);
  PARAMINSERT("ProbabilityOfSn6Limit2", kFmiProbabilityOfSn6Limit2);
  PARAMINSERT("ProbabilityOfSn6Limit3", kFmiProbabilityOfSn6Limit3);
  PARAMINSERT("ProbabilityOfSn6Limit4", kFmiProbabilityOfSn6Limit4);
  PARAMINSERT("ProbabilityOfSn6Limit5", kFmiProbabilityOfSn6Limit5);
  PARAMINSERT("ProbabilityOfSn6Limit6", kFmiProbabilityOfSn6Limit6);
  PARAMINSERT("ProbabilityOfSnow", kFmiProbabilityOfSnow);
  PARAMINSERT("ProbabilityOfTemp", kFmiProbabilityOfTemp);
  PARAMINSERT("ProbabilityOfTempLimit1", kFmiProbabilityOfTempLimit1);
  PARAMINSERT("ProbabilityOfTempLimit2", kFmiProbabilityOfTempLimit2);
  PARAMINSERT("ProbabilityOfTempLimit3", kFmiProbabilityOfTempLimit3);
  PARAMINSERT("ProbabilityOfTempLimit4", kFmiProbabilityOfTempLimit4);
  PARAMINSERT("ProbabilityOfWind", kFmiProbabilityOfWind);
  PARAMINSERT("ProbabilityOfWindLimit1", kFmiProbabilityOfWindLimit1);
  PARAMINSERT("ProbabilityOfWindLimit2", kFmiProbabilityOfWindLimit2);
  PARAMINSERT("ProbabilityOfWindLimit3", kFmiProbabilityOfWindLimit3);
  PARAMINSERT("ProbabilityOfWindLimit4", kFmiProbabilityOfWindLimit4);
  PARAMINSERT("ProbabilityOfWindLimit5", kFmiProbabilityOfWindLimit5);
  PARAMINSERT("ProbabilityOfWindLimit6", kFmiProbabilityOfWindLimit6);
  PARAMINSERT("ProbabilitySnowyRoad", kFmiProbabilitySnowyRoad);
  PARAMINSERT("ProbabilityThunderstorm", kFmiProbabilityThunderstorm);
  PARAMINSERT("ProbabilityThunderstorm2", kFmiProbabilityThunderstorm2);
  PARAMINSERT("ProbabilityWetRoad", kFmiProbabilityWetRoad);
  PARAMINSERT("ProjectionID", kFmiProjectionID);
  PARAMINSERT("PseudoAdiabaticPotentialTemperature", kFmiPseudoAdiabaticPotentialTemperature);
  PARAMINSERT("QNH", kFmiQNH);
  PARAMINSERT("QOL", kFmiQOL);
  PARAMINSERT("RadarBorder", kFmiRadarBorder);
  PARAMINSERT("RadarLayer", kFmiRadarLayer);
  PARAMINSERT("RadarLevelType", kFmiRadarLevelType);
  PARAMINSERT("RadarParameter", kFmiRadarParameter);
  PARAMINSERT("RadarProducer", kFmiRadarProducer);
  PARAMINSERT("RadarRadius", kFmiRadarRadius);
  PARAMINSERT("RadialVelocity", kFmiRadialVelocity);
  PARAMINSERT("RadialWind", kFmiRadialWind);
  PARAMINSERT("RadiationDiffuseAccumulation", kFmiRadiationDiffuseAccumulation);
  PARAMINSERT("RadiationGlobal", kFmiRadiationGlobal);
  PARAMINSERT("RadiationGlobalAccumulation", kFmiRadiationGlobalAccumulation);
  PARAMINSERT("RadiationInSW", kFmiRadiationInSW);
  PARAMINSERT("RadiationInSW2", kFmiRadiationInSW2);
  PARAMINSERT("RadiationLW", kFmiRadiationLW);
  PARAMINSERT("RadiationLWAccumulation", kFmiRadiationLWAccumulation);
  PARAMINSERT("RadiationNetSurfaceLW", kFmiRadiationNetSurfaceLW);
  PARAMINSERT("RadiationNetSurfaceLWAccumulation", kFmiRadiationNetSurfaceLWAccumulation);
  PARAMINSERT("RadiationNetSurfaceSW", kFmiRadiationNetSurfaceSW);
  PARAMINSERT("RadiationNetSurfaceSWAccumulation", kFmiRadiationNetSurfaceSWAccumulation);
  PARAMINSERT("RadiationNetTopAtmLW", kFmiRadiationNetTopAtmLW);
  PARAMINSERT("RadiationNetTopAtmSW", kFmiRadiationNetTopAtmSW);
  PARAMINSERT("RadiationOutLW", kFmiRadiationOutLW);
  PARAMINSERT("RadiationOutLW2", kFmiRadiationOutLW2);
  PARAMINSERT("RadiationOutSW", kFmiRadiationOutSW);
  PARAMINSERT("RadiationOutSW2", kFmiRadiationOutSW2);
  PARAMINSERT("RadiationReflected", kFmiRadiationReflected);
  PARAMINSERT("RadiationSW", kFmiRadiationSW);
  PARAMINSERT("RaftIceConcentration", kFmiRaftIceConcentration);
  PARAMINSERT("RaftIceThickness", kFmiRaftIceThickness);
  PARAMINSERT("RainIntensityWeather", kFmiRainIntensityWeather);
  PARAMINSERT("RainMinutes", kFmiRainMinutes);
  PARAMINSERT("RainOnOff", kFmiRainOnOff);
  PARAMINSERT("RainfallDepth", kFmiRainfallDepth);
  PARAMINSERT("RainWaterMixingRatio", kFmiRainWaterMixingRatio);
  PARAMINSERT("RawRadarData", kFmiRawRadarData);
  PARAMINSERT("Reflectivity", kFmiReflectivity);
  PARAMINSERT("ReflectivityCorrelation", kFmiReflectivityCorrelation);
  PARAMINSERT("RidgeIceConcentration", kFmiRidgeIceConcentration);
  PARAMINSERT("RoadCondition", kFmiRoadCondition);
  PARAMINSERT("RoadConditionAlternative", kFmiRoadConditionAlternative);
  PARAMINSERT("RoadConditionSeverity", kFmiRoadConditionSeverity);
  PARAMINSERT("RoadFrostCover", kFmiRoadFrostCover);
  PARAMINSERT("RoadIceCover", kFmiRoadIceCover);
  PARAMINSERT("RoadIceCoverOnVehiclePath", kFmiRoadIceCoverOnVehiclePath);
  PARAMINSERT("RoadMaintenanceMeasure", kFmiRoadMaintenanceMeasure);
  PARAMINSERT("RoadNotification", kFmiRoadNotification);
  PARAMINSERT("RoadReasoning", kFmiRoadReasoning);
  PARAMINSERT("RoadSnowCover", kFmiRoadSnowCover);
  PARAMINSERT("RoadStatus", kFmiRoadStatus);
  PARAMINSERT("RoadT1Derivative", kFmiRoadT1Derivative);
  PARAMINSERT("RoadT2Derivative", kFmiRoadT2Derivative);
  PARAMINSERT("RoadTemperature", kFmiRoadTemperature);
  PARAMINSERT("RoadTemperature2", kFmiRoadTemperature2);
  PARAMINSERT("RoadWarning", kFmiRoadWarning);
  PARAMINSERT("RoadWarning1", kFmiRoadWarning1);
  PARAMINSERT("RoadWarning2", kFmiRoadWarning2);
  PARAMINSERT("RoadWarning3", kFmiRoadWarning3);
  PARAMINSERT("RoadWarning4", kFmiRoadWarning4);
  PARAMINSERT("RoadWarning5", kFmiRoadWarning5);
  PARAMINSERT("RoadWarning6", kFmiRoadWarning6);
  PARAMINSERT("Roughness", kFmiRoughness);
  PARAMINSERT("RowNumber", kFmiRowNumber);
  PARAMINSERT("SO2Contents", kFmiSO2Contents);
  PARAMINSERT("Salinity", kFmiSalinity);
  PARAMINSERT("SaltConcentration1", kFmiSaltConcentration1);
  PARAMINSERT("SaltConcentration2", kFmiSaltConcentration2);
  PARAMINSERT("SaltSpread1", kFmiSaltSpread1);
  PARAMINSERT("SaltSpread2", kFmiSaltSpread2);
  PARAMINSERT("SatelCh1", kFmiSatelCh1);
  PARAMINSERT("SatelCh10", kFmiSatelCh10);
  PARAMINSERT("SatelCh11", kFmiSatelCh11);
  PARAMINSERT("SatelCh112", kFmiSatelCh112);
  PARAMINSERT("SatelCh12", kFmiSatelCh12);
  PARAMINSERT("SatelCh124", kFmiSatelCh124);
  PARAMINSERT("SatelCh129", kFmiSatelCh129);
  PARAMINSERT("SatelCh164", kFmiSatelCh164);
  PARAMINSERT("SatelCh2", kFmiSatelCh2);
  PARAMINSERT("SatelCh239", kFmiSatelCh239);
  PARAMINSERT("SatelCh3", kFmiSatelCh3);
  PARAMINSERT("SatelCh321", kFmiSatelCh321);
  PARAMINSERT("SatelCh345", kFmiSatelCh345);
  PARAMINSERT("SatelCh4", kFmiSatelCh4);
  PARAMINSERT("SatelCh4910", kFmiSatelCh4910);
  PARAMINSERT("SatelCh5", kFmiSatelCh5);
  PARAMINSERT("SatelCh6", kFmiSatelCh6);
  PARAMINSERT("SatelCh7", kFmiSatelCh7);
  PARAMINSERT("SatelCh8", kFmiSatelCh8);
  PARAMINSERT("SatelCh9", kFmiSatelCh9);
  PARAMINSERT("SatoKierto", kFmiSatoKierto);
  PARAMINSERT("SaturationDeficit", kFmiSaturationDeficit);
  PARAMINSERT("SeaLevel", kFmiSeaLevel);
  PARAMINSERT("SeaLevelAnomaly", kFmiSeaLevelAnomaly);
  PARAMINSERT("SeaLevelMaximum", kFmiSeaLevelMaximum);
  PARAMINSERT("SeaLevelMinimum", kFmiSeaLevelMinimum);
  PARAMINSERT("SeaPercentage", kFmiSeaPercentage);
  PARAMINSERT("SeaSprayIcing", kFmiSeaSprayIcing);
  PARAMINSERT("SeaWaterVelocityU", kFmiSeaWaterVelocityU);
  PARAMINSERT("SeaWaterVelocityV", kFmiSeaWaterVelocityV);
  PARAMINSERT("SeaWaterVelocityW", kFmiSeaWaterVelocityW);
  PARAMINSERT("Second", kFmiSecond);
  PARAMINSERT("SensibleHeatFlux", kFmiSensibleHeatFlux);
  PARAMINSERT("SensorFault", kFmiSensorFault);
  PARAMINSERT("SensorOrdinal", kFmiSensorOrdinal);
  PARAMINSERT("SevereFrostProbability", kFmiSevereFrostProbability);
  PARAMINSERT("ShipDirection", kFmiShipDirection);
  PARAMINSERT("ShipSpeed", kFmiShipSpeed);
  PARAMINSERT("SignalQualityIndex", kFmiSignalQualityIndex);
  PARAMINSERT("SigWaveHeight", kFmiSigWaveHeight);
  PARAMINSERT("SigWaveHeightBandB", kFmiSigWaveHeightBandB);
  PARAMINSERT("SigWaveHeightBandC", kFmiSigWaveHeightBandC);
  PARAMINSERT("SigWaveHeightSwell0", kFmiSigWaveHeightSwell0);
  PARAMINSERT("SigWaveHeightSwell1", kFmiSigWaveHeightSwell1);
  PARAMINSERT("SigWaveHeightSwell2", kFmiSigWaveHeightSwell2);
  PARAMINSERT("SigWaveLength", kFmiSigWaveLength);
  PARAMINSERT("SigWavePeriod", kFmiSigWavePeriod);
  PARAMINSERT("SigWavePeriodBandB", kFmiSigWavePeriodBandB);
  PARAMINSERT("SigWavePeriodBandC", kFmiSigWavePeriodBandC);
  PARAMINSERT("SigWavePeriodSwell0", kFmiSigWavePeriodSwell0);
  PARAMINSERT("SigWavePeriodSwell1", kFmiSigWavePeriodSwell1);
  PARAMINSERT("SigWavePeriodSwell2", kFmiSigWavePeriodSwell2);
  PARAMINSERT("SimpleWeather", kFmiSimpleWeather);
  PARAMINSERT("SkinTemperature", kFmiSkinTemperature);
  PARAMINSERT("SmogIndex", kFmiSmogIndex);
  PARAMINSERT("SmogSymbol", kFmiSmogSymbol);
  PARAMINSERT("SnowAccumulation", kFmiSnowAccumulation);
  PARAMINSERT("SnowAccumulationF0", kFmiSnowAccumulationF0);
  PARAMINSERT("SnowAccumulationF1", kFmiSnowAccumulationF1);
  PARAMINSERT("SnowAccumulationF2.5", kFmiSnowAccumulationF2_5);
  PARAMINSERT("SnowAccumulationF5", kFmiSnowAccumulationF5);
  PARAMINSERT("SnowAccumulationF12.5", kFmiSnowAccumulationF12_5);
  PARAMINSERT("SnowAccumulationF50", kFmiSnowAccumulationF50);
  PARAMINSERT("SnowAccumulationF87.5", kFmiSnowAccumulationF87_5);
  PARAMINSERT("SnowAccumulationF95", kFmiSnowAccumulationF95);
  PARAMINSERT("SnowAccumulationF97.5", kFmiSnowAccumulationF97_5);
  PARAMINSERT("SnowAccumulationF99", kFmiSnowAccumulationF99);
  PARAMINSERT("SnowAccumulationF100", kFmiSnowAccumulationF100);
  PARAMINSERT("SnowContent", kFmiSnowContent);
  PARAMINSERT("SnowDensity", kFmiSnowDensity);
  PARAMINSERT("SnowDepth", kFmiSnowDepth);
  PARAMINSERT("SnowDepthF0", kFmiSnowDepthF0);
  PARAMINSERT("SnowDepthF1", kFmiSnowDepthF1);
  PARAMINSERT("SnowDepthF2.5", kFmiSnowDepthF2_5);
  PARAMINSERT("SnowDepthF5", kFmiSnowDepthF5);
  PARAMINSERT("SnowDepthF12.5", kFmiSnowDepthF12_5);
  PARAMINSERT("SnowDepthF50", kFmiSnowDepthF50);
  PARAMINSERT("SnowDepthF87.5", kFmiSnowDepthF87_5);
  PARAMINSERT("SnowDepthF95", kFmiSnowDepthF95);
  PARAMINSERT("SnowDepthF97.5", kFmiSnowDepthF97_5);
  PARAMINSERT("SnowDepthF99", kFmiSnowDepthF99);
  PARAMINSERT("SnowDepthF100", kFmiSnowDepthF100);
  PARAMINSERT("SnowDepth06", kFmiSnowDepth06);
  PARAMINSERT("SnowDepth18", kFmiSnowDepth18);
  PARAMINSERT("SnowDepth2", kFmiSnowDepth2);
  PARAMINSERT("SnowLoad", kFmiSnowLoad);
  PARAMINSERT("SnowLoadType", kFmiSnowLoadType);
  PARAMINSERT("SnowMixingRatio", kFmiSnowMixingRatio);
  PARAMINSERT("SnowPercentage", kFmiSnowPercentage);
  PARAMINSERT("SnowWarning", kFmiSnowWarning);
  PARAMINSERT("SnowWaterRatio", kFmiSnowWaterRatio);
  PARAMINSERT("SnowfallRate", kFmiSnowfallRate);
  PARAMINSERT("SoilMoistureContent", kFmiSoilMoistureContent);
  PARAMINSERT("SoilTemperature", kFmiSoilTemperature);
  PARAMINSERT("SoilType", kFmiSoilType);
  PARAMINSERT("SolarElevation", kFmiSolarElevation);
  PARAMINSERT("SoundingLCLPressureSurface", kFmiSoundingLCLSur);
  PARAMINSERT("SoundingLFCPressureSurface", kFmiSoundingLFCSur);
  PARAMINSERT("SoundingELPressureSurface", kFmiSoundingELSur);
  PARAMINSERT("SoundingCAPESurface", kFmiSoundingCAPESur);
  PARAMINSERT("SoundingCAPE3kmSurface", kFmiSoundingCAPE0_3kmSur);
  PARAMINSERT("SoundingCAPE1040Surface", kFmiSoundingCAPE_TT_Sur);
  PARAMINSERT("SoundingCINSurface", kFmiSoundingCINSur);
  PARAMINSERT("SoundingLCLHeightSurface", kFmiSoundingLCLHeightSur);
  PARAMINSERT("SoundingLFCHeightSurface", kFmiSoundingLFCHeightSur);
  PARAMINSERT("SoundingELHeightSurface", kFmiSoundingELHeightSur);
  PARAMINSERT("SoundingLCLPressure500m", kFmiSoundingLCL500m);
  PARAMINSERT("SoundingLFCPressure500m", kFmiSoundingLFC500m);
  PARAMINSERT("SoundingELPressure500m", kFmiSoundingEL500m);
  PARAMINSERT("SoundingCAPE500m", kFmiSoundingCAPE500m);
  PARAMINSERT("SoundingCAPE3km500m", kFmiSoundingCAPE0_3km500m);
  PARAMINSERT("SoundingCAPE1040500m", kFmiSoundingCAPE_TT_500m);
  PARAMINSERT("SoundingCIN500m", kFmiSoundingCIN500m);
  PARAMINSERT("SoundingLCLHeight500m", kFmiSoundingLCLHeight500m);
  PARAMINSERT("SoundingLFCHeight500m", kFmiSoundingLFCHeight500m);
  PARAMINSERT("SoundingELHeightS500m", kFmiSoundingELHeight500m);
  PARAMINSERT("SoundingLCLPressureMostUnstable", kFmiSoundingLCLMostUn);
  PARAMINSERT("SoundingLFCPressureMostUnstable", kFmiSoundingLFCMostUn);
  PARAMINSERT("SoundingELPressureMostUnstable", kFmiSoundingELMostUn);
  PARAMINSERT("SoundingCAPEMostUnstable", kFmiSoundingCAPEMostUn);
  PARAMINSERT("SoundingCAPE3kmMostUnstable", kFmiSoundingCAPE0_3kmMostUn);
  PARAMINSERT("SoundingCAPE1040MostUnstable", kFmiSoundingCAPE_TT_MostUn);
  PARAMINSERT("SoundingCINMostUnstable", kFmiSoundingCINMostUn);
  PARAMINSERT("SoundingLCLHeightMostUnstable", kFmiSoundingLCLHeightMostUn);
  PARAMINSERT("SoundingLFCHeightMostUnstable", kFmiSoundingLFCHeightMostUn);
  PARAMINSERT("SoundingELHeightMostUnstable", kFmiSoundingELHeightMostUn);
  PARAMINSERT("SoundingShowalterIndex", kFmiSoundingSHOW);
  PARAMINSERT("SoundingLiftedIndex", kFmiSoundingLIFT);
  PARAMINSERT("SoundingKIndex", kFmiSoundingKINX);
  PARAMINSERT("SoundingCrossTotalsIndex", kFmiSoundingCTOT);
  PARAMINSERT("SoundingVerticalTotalsIndex", kFmiSoundingVTOT);
  PARAMINSERT("SoundingTotalTotalsIndex", kFmiSoundingTOTL);
  PARAMINSERT("SoundingBulkShear6km", kFmiSoundingBS0_6km);
  PARAMINSERT("SoundingBulkShear1km", kFmiSoundingBS0_1km);
  PARAMINSERT("SoundingStormRelativeHelicity3km", kFmiSoundingSRH0_3km);
  PARAMINSERT("SoundingStormRelativeHelicity1km", kFmiSoundingSRH0_1km);
  PARAMINSERT("SoundingWindSpeed1500m", kFmiSoundingWS1500m);
  PARAMINSERT("SoundingThetaE3km", kFmiSoundingThetaE0_3km);
  PARAMINSERT("SpecificDifferentialPhase", kFmiSpecificDifferentialPhase);
  PARAMINSERT("SpecificHumidity", kFmiSpecificHumidity);
  PARAMINSERT("SpecificHumidityF0", kFmiSpecificHumidityF0);
  PARAMINSERT("SpecificHumidityF1", kFmiSpecificHumidityF1);
  PARAMINSERT("SpecificHumidityF2.5", kFmiSpecificHumidityF2_5);
  PARAMINSERT("SpecificHumidityF5", kFmiSpecificHumidityF5);
  PARAMINSERT("SpecificHumidityF12.5", kFmiSpecificHumidityF12_5);
  PARAMINSERT("SpecificHumidityF50", kFmiSpecificHumidityF50);
  PARAMINSERT("SpecificHumidityF87.5", kFmiSpecificHumidityF87_5);
  PARAMINSERT("SpecificHumidityF95", kFmiSpecificHumidityF95);
  PARAMINSERT("SpecificHumidityF97.5", kFmiSpecificHumidityF97_5);
  PARAMINSERT("SpecificHumidityF99", kFmiSpecificHumidityF99);
  PARAMINSERT("SpecificHumidityF100", kFmiSpecificHumidityF100);
  PARAMINSERT("SpectralWidth", kFmiSpectralWidth);
  PARAMINSERT("StabilityIndex", kFmiStabilityIndex);
  PARAMINSERT("StateOfGround", kFmiStateOfGround);
  PARAMINSERT("StationType", kFmiStationType);
  PARAMINSERT("StdDevHighCld", kFmiStdDevHighCld);
  PARAMINSERT("StdDevLMCld", kFmiStdDevLMCld);
  PARAMINSERT("StdDevTotalCloud", kFmiStdDevTotalCloud);
  PARAMINSERT("StormRelativeHelicity", kFmiStormRelativeHelicity);
  PARAMINSERT("StreamFunction", kFmiStreamFunction);
  PARAMINSERT("SunGlintPercentage", kFmiSunGlintPercentage);
  PARAMINSERT("SunHours", kFmiSunHours);
  PARAMINSERT("SunOnOff", kFmiSunOnOff);
  PARAMINSERT("SunSeconds", kFmiSunSeconds);
  PARAMINSERT("Surface6hAccumulatedNOxDryDeposition", kFmiSurface6hAccumulatedNOxDryDeposition);
  PARAMINSERT("Surface6hAccumulatedNOxWetDeposition", kFmiSurface6hAccumulatedNOxWetDeposition);
  PARAMINSERT("Surface6hAccumulatedNHxDryDeposition", kFmiSurface6hAccumulatedNHxDryDeposition);
  PARAMINSERT("Surface6hAccumulatedNHxWetDeposition", kFmiSurface6hAccumulatedNHxWetDeposition);
  PARAMINSERT("Surface6hAccumulatedSDryDeposition", kFmiSurface6hAccumulatedSDryDeposition);
  PARAMINSERT("Surface6hAccumulatedSWetDeposition", kFmiSurface6hAccumulatedSWetDeposition);
  PARAMINSERT("Surface24hAccumulatedNOxDryDeposition", kFmiSurface24hAccumulatedNOxDryDeposition);
  PARAMINSERT("Surface24hAccumulatedNOxWetDeposition", kFmiSurface24hAccumulatedNOxWetDeposition);
  PARAMINSERT("Surface24hAccumulatedNHxDryDeposition", kFmiSurface24hAccumulatedNHxDryDeposition);
  PARAMINSERT("Surface24hAccumulatedNHxWetDeposition", kFmiSurface24hAccumulatedNHxWetDeposition);
  PARAMINSERT("Surface24hAccumulatedSDryDeposition", kFmiSurface24hAccumulatedSDryDeposition);
  PARAMINSERT("Surface24hAccumulatedSWetDeposition", kFmiSurface24hAccumulatedSWetDeposition);
  PARAMINSERT("SurfaceRoughness", kFmiSurfaceRoughness);
  PARAMINSERT("SurfaceRoughnessAtSea", kFmiSurfaceRoughnessAtSea);
  PARAMINSERT("SurfaceSoilWetness", kFmiSurfaceSoilWetness);
  PARAMINSERT("SurfaceTypeFraction", kFmiSurfaceTypeFraction);
  PARAMINSERT("SurfaceWaterPhase", kFmiSurfaceWaterPhase);
  PARAMINSERT("TOL", kFmiTOL);
  PARAMINSERT("Temperature", kFmiTemperature);
  PARAMINSERT("Temperature2M", kFmiTemperature2M);
  PARAMINSERT("TemperatureAnomaly", kFmiTemperatureAnomaly);
  PARAMINSERT("TemperatureAnomalyMinus1", kFmiTemperatureAnomalyMinus1);
  PARAMINSERT("TemperatureAnomalyMinus2", kFmiTemperatureAnomalyMinus2);
  PARAMINSERT("TemperatureAnomalyPlus0", kFmiTemperatureAnomalyPlus0);
  PARAMINSERT("TemperatureAnomalyPlus1", kFmiTemperatureAnomalyPlus1);
  PARAMINSERT("TemperatureAnomalyPlus2", kFmiTemperatureAnomalyPlus2);
  PARAMINSERT("TemperatureCode", kFmiTemperatureCode);
  PARAMINSERT("TemperatureDeviation", kFmiTemperatureDeviation);
  PARAMINSERT("TemperatureF0", kFmiTemperatureF0);
  PARAMINSERT("TemperatureF1", kFmiTemperatureF1);
  PARAMINSERT("TemperatureF2.5", kFmiTemperatureF2_5);
  PARAMINSERT("TemperatureF5", kFmiTemperatureF5);
  PARAMINSERT("TemperatureF10", kFmiTemperatureF10);
  PARAMINSERT("TemperatureF12.5", kFmiTemperatureF12_5);
  PARAMINSERT("TemperatureF100", kFmiTemperatureF100);
  PARAMINSERT("TemperatureF25", kFmiTemperatureF25);
  PARAMINSERT("TemperatureF50", kFmiTemperatureF50);
  PARAMINSERT("TemperatureF75", kFmiTemperatureF75);
  PARAMINSERT("TemperatureF87.5", kFmiTemperatureF87_5);
  PARAMINSERT("TemperatureF90", kFmiTemperatureF90);
  PARAMINSERT("TemperatureF95", kFmiTemperatureF95);
  PARAMINSERT("TemperatureF97.5", kFmiTemperatureF97_5);
  PARAMINSERT("TemperatureF99", kFmiTemperatureF99);
  PARAMINSERT("TemperatureHighCld", kFmiTemperatureHighCld);
  PARAMINSERT("TemperatureLMCld", kFmiTemperatureLMCld);
  PARAMINSERT("TemperatureSea", kFmiTemperatureSea);
  PARAMINSERT("TemperatureSeaF0", kFmiTemperatureSeaF0);
  PARAMINSERT("TemperatureSeaF1", kFmiTemperatureSeaF1);
  PARAMINSERT("TemperatureSeaF2.5", kFmiTemperatureSeaF2_5);
  PARAMINSERT("TemperatureSeaF5", kFmiTemperatureSeaF5);
  PARAMINSERT("TemperatureSeaF12.5", kFmiTemperatureSeaF12_5);
  PARAMINSERT("TemperatureSeaF50", kFmiTemperatureSeaF50);
  PARAMINSERT("TemperatureSeaF87.5", kFmiTemperatureSeaF87_5);
  PARAMINSERT("TemperatureSeaF95", kFmiTemperatureSeaF95);
  PARAMINSERT("TemperatureSeaF97.5", kFmiTemperatureSeaF97_5);
  PARAMINSERT("TemperatureSeaF99", kFmiTemperatureSeaF99);
  PARAMINSERT("TemperatureSeaF100", kFmiTemperatureSeaF100);
  PARAMINSERT("TemperatureSea2", kFmiTemperatureSea2);
  PARAMINSERT("TemperatureSeaSurface", kFmiTemperatureSeaSurface);
  PARAMINSERT("TemperatureVirtual", kFmiTemperatureVirtual);
  PARAMINSERT("TemperatureVoltage", kFmiTemperatureVoltage);
  PARAMINSERT("Thickness", kFmiThickness);
  PARAMINSERT("TimeOfUVMaximum", kFmiTimeOfUVMaximum);
  PARAMINSERT("TimeToNextMeasure", kFmiTimeToNextMeasure);
  PARAMINSERT("TimeToNextSalting", kFmiTimeToNextSalting);
  PARAMINSERT("TopLinkIndex2", kFmiTopLinkIndex2);
  PARAMINSERT("TopLinkIndex3", kFmiTopLinkIndex3);
  PARAMINSERT("TopOfTurbulence", kFmiTopOfTurbulence);
  PARAMINSERT("TopoAzimuth", kFmiTopoAzimuth);
  PARAMINSERT("TopoDirectionToLand", kFmiTopoDirectionToLand);
  PARAMINSERT("TopoDirectionToSea", kFmiTopoDirectionToSea);
  PARAMINSERT("TopoDistanceToLand", kFmiTopoDistanceToLand);
  PARAMINSERT("TopoDistanceToSea", kFmiTopoDistanceToSea);
  PARAMINSERT("TopoGraf", kFmiTopoGraf);
  PARAMINSERT("TopoRelativeHeight", kFmiTopoRelativeHeight);
  PARAMINSERT("TopoSlope", kFmiTopoSlope);
  PARAMINSERT("TotalCloudCover", kFmiTotalCloudCover);
  PARAMINSERT("TotalCloudCoverF0", kFmiTotalCloudCoverF0);
  PARAMINSERT("TotalCloudCoverF1", kFmiTotalCloudCoverF1);
  PARAMINSERT("TotalCloudCoverF2.5", kFmiTotalCloudCoverF2_5);
  PARAMINSERT("TotalCloudCoverF5", kFmiTotalCloudCoverF5);
  PARAMINSERT("TotalCloudCoverF10", kFmiTotalCloudCoverF10);
  PARAMINSERT("TotalCloudCoverF12.5", kFmiTotalCloudCoverF12_5);
  PARAMINSERT("TotalCloudCoverF100", kFmiTotalCloudCoverF100);
  PARAMINSERT("TotalCloudCoverF25", kFmiTotalCloudCoverF25);
  PARAMINSERT("TotalCloudCoverF50", kFmiTotalCloudCoverF50);
  PARAMINSERT("TotalCloudCoverF75", kFmiTotalCloudCoverF75);
  PARAMINSERT("TotalCloudCoverF87.5", kFmiTotalCloudCoverF87_5);
  PARAMINSERT("TotalCloudCoverF90", kFmiTotalCloudCoverF90);
  PARAMINSERT("TotalCloudCoverF95", kFmiTotalCloudCoverF95);
  PARAMINSERT("TotalCloudCoverF97.5", kFmiTotalCloudCoverF97_5);
  PARAMINSERT("TotalCloudCoverF99", kFmiTotalCloudCoverF99);
  PARAMINSERT("TotalColumnWater", kFmiTotalColumnWater);
  PARAMINSERT("TotalColumnWaterF0", kFmiTotalColumnWaterF0);
  PARAMINSERT("TotalColumnWaterF1", kFmiTotalColumnWaterF1);
  PARAMINSERT("TotalColumnWaterF2.5", kFmiTotalColumnWaterF2_5);
  PARAMINSERT("TotalColumnWaterF5", kFmiTotalColumnWaterF5);
  PARAMINSERT("TotalColumnWaterF12.5", kFmiTotalColumnWaterF12_5);
  PARAMINSERT("TotalColumnWaterF50", kFmiTotalColumnWaterF50);
  PARAMINSERT("TotalColumnWaterF87.5", kFmiTotalColumnWaterF87_5);
  PARAMINSERT("TotalColumnWaterF95", kFmiTotalColumnWaterF95);
  PARAMINSERT("TotalColumnWaterF97.5", kFmiTotalColumnWaterF97_5);
  PARAMINSERT("TotalColumnWaterF99", kFmiTotalColumnWaterF99);
  PARAMINSERT("TotalColumnWaterF100", kFmiTotalColumnWaterF100);
  PARAMINSERT("TotalOzone", kFmiTotalOzone);
  PARAMINSERT("TotalPrecipitationF0", kFmiTotalPrecipitationF0);
  PARAMINSERT("TotalPrecipitationF10", kFmiTotalPrecipitationF10);
  PARAMINSERT("TotalPrecipitationF100", kFmiTotalPrecipitationF100);
  PARAMINSERT("TotalPrecipitationF25", kFmiTotalPrecipitationF25);
  PARAMINSERT("TotalPrecipitationF50", kFmiTotalPrecipitationF50);
  PARAMINSERT("TotalPrecipitationF75", kFmiTotalPrecipitationF75);
  PARAMINSERT("TotalPrecipitationF90", kFmiTotalPrecipitationF90);
  PARAMINSERT("TotalSnowLoad", kFmiTotalSnowLoad);
  PARAMINSERT("TotalSnowLoadChange", kFmiTotalSnowLoadChange);
  PARAMINSERT("TotalWindMS", kFmiTotalWindMS);
  PARAMINSERT("TrafficIndex", kFmiTrafficIndex);
  PARAMINSERT("Turbulence", kFmiTurbulence);
  PARAMINSERT("TurbulentKineticEnergy", kFmiTurbulentKineticEnergy);
  PARAMINSERT("UMomentumFlux", kFmiUMomentumFlux);
  PARAMINSERT("UVCumulated", kFmiUVCumulated);
  PARAMINSERT("UVIndex", kFmiUVIndex);
  PARAMINSERT("UVIndexMax", kFmiUVIndexMax);
  PARAMINSERT("UVMaximum", kFmiUVMaximum);
  PARAMINSERT("UnidirectivityIndex", kFmiUnidirectivityIndex);
  PARAMINSERT("VIS", kFmiVIS);
  PARAMINSERT("VMomentumFlux", kFmiVMomentumFlux);
  PARAMINSERT("VapourPressure", kFmiVapourPressure);
  PARAMINSERT("Vegetation", kFmiVegetation);
  PARAMINSERT("VegetationType", kFmiVegetationType);
  PARAMINSERT("VelocityDivergenceHoriz", kFmiVelocityDivergenceHoriz);
  PARAMINSERT("VelocityPotential", kFmiVelocityPotential);
  PARAMINSERT("VelocityPotentialM2S", kFmiVelocityPotentialM2S);
  PARAMINSERT("VerVis", kFmiVerticalVisibility);  // SPELLING!
  PARAMINSERT("VerticalMeltingArea", kFmiVerticalMeltingArea);
  PARAMINSERT("VerticalSoundingSignificance", kFmiVerticalSoundingSignificance);
  PARAMINSERT("VerticalVelocityCB12H", kFmiVerticalVelocityCB12H);
  PARAMINSERT("VerticalVelocityCBS", kFmiVerticalVelocityCBS);
  PARAMINSERT("VerticalVelocityDPAS", kFmiVerticalVelocityDPAS);
  PARAMINSERT("VerticalVelocityHPAH", kFmiVerticalVelocityHPAH);
  PARAMINSERT("VerticalVelocityHPAS", kFmiVerticalVelocityHPAS);
  PARAMINSERT("VerticalVelocityMMS", kFmiVerticalVelocityMMS);
  PARAMINSERT("VerticalVelocityMS", kFmiVerticalVelocityMS);
  PARAMINSERT("Visibility", kFmiVisibility);
  PARAMINSERT("Visibility2", kFmiVisibility2);
  PARAMINSERT("VisibilityChange", kFmiVisibilityChange);
  PARAMINSERT("VorticityAbsolute", kFmiVorticityAbsolute);
  PARAMINSERT("VorticityAdvectionAbsolute", kFmiVorticityAdvectionAbsolute);
  PARAMINSERT("VorticityAdvectionGeostr", kFmiVorticityAdvectionGeostr);
  PARAMINSERT("VorticityAdvectionRelative", kFmiVorticityAdvectionRelative);
  PARAMINSERT("VorticityGeostrophic", kFmiVorticityGeostrophic);
  PARAMINSERT("VorticityRelative", kFmiVorticityRelative);
  PARAMINSERT("WV", kFmiWV);
  PARAMINSERT("WaterAccumulation", kFmiWaterAccumulation);
  PARAMINSERT("WaterContent", kFmiWaterContent);
  PARAMINSERT("WaterEquivalentOfSnow", kFmiWaterEquivalentOfSnow);
  PARAMINSERT("WaterStorage", kFmiWaterStorage);
  PARAMINSERT("WaterVapourBrightnessTemperature", kFmiWaterVapourBrightnessTemperature);
  PARAMINSERT("WaterVapourBrightnessTemperatureIncludingCloudCorrection",
              kFmiWaterVapourBrightnessTemperatureIncludingCloudCorrection);
  PARAMINSERT("WaveDirection", kFmiWaveDirection);
  PARAMINSERT("WaveDirectionBandB", kFmiWaveDirectionBandB);
  PARAMINSERT("WaveDirectionBandC", kFmiWaveDirectionBandC);
  PARAMINSERT("WaveDirectionSwell0", kFmiWaveDirectionSwell0);
  PARAMINSERT("WaveDirectionSwell1", kFmiWaveDirectionSwell1);
  PARAMINSERT("WaveDirectionSwell2", kFmiWaveDirectionSwell2);
  PARAMINSERT("WavePeriod", kFmiWavePeriod);
  PARAMINSERT("WaveSpread", kFmiWaveSpread);
  PARAMINSERT("WeatherAndCloudiness", kFmiWeatherAndCloudiness);
  PARAMINSERT("WeatherSymbol1", kFmiWeatherSymbol1);
  PARAMINSERT("WeatherSymbol2", kFmiWeatherSymbol2);
  PARAMINSERT("WeatherSymbol3", kFmiWeatherSymbol3);
  PARAMINSERT("WeatherSymbol", kFmiWeatherSymbol);
  PARAMINSERT("WeatherSymbolTaf", kFmiWeatherSymbolTaf);
  PARAMINSERT("WetSnowLoad", kFmiWetSnowLoad);
  PARAMINSERT("WetnessOnOff", kFmiWetnessOnOff);
  PARAMINSERT("WindAndReflectivity", kFmiWindAndReflectivity);
  PARAMINSERT("WindDirection", kFmiWindDirection);
  PARAMINSERT("WindDirection10M", kFmiWindDirection10M);
  PARAMINSERT("WindDirectionCode", kFmiWindDirectionCode);
  PARAMINSERT("WindDirectionDeviation", kFmiWindDirectionDeviation);
  PARAMINSERT("WindDirectionVoltage", kFmiWindDirectionVoltage);
  PARAMINSERT("WindGust", kFmiWindGust);
  PARAMINSERT("WindGust2", kFmiWindGust2);
  PARAMINSERT("WindGustF0", kFmiWindGustF0);
  PARAMINSERT("WindGustF1", kFmiWindGustF1);
  PARAMINSERT("WindGustF2.5", kFmiWindGustF2_5);
  PARAMINSERT("WindGustF5", kFmiWindGustF5);
  PARAMINSERT("WindGustF10", kFmiWindGustF10);
  PARAMINSERT("WindGustF12.5", kFmiWindGustF12_5);
  PARAMINSERT("WindGustF100", kFmiWindGustF100);
  PARAMINSERT("WindGustF25", kFmiWindGustF25);
  PARAMINSERT("WindGustF50", kFmiWindGustF50);
  PARAMINSERT("WindGustF75", kFmiWindGustF75);
  PARAMINSERT("WindGustF87.5", kFmiWindGustF87_5);
  PARAMINSERT("WindGustF90", kFmiWindGustF90);
  PARAMINSERT("WindGustF95", kFmiWindGustF95);
  PARAMINSERT("WindGustF97.5", kFmiWindGustF97_5);
  PARAMINSERT("WindGustF99", kFmiWindGustF99);
  PARAMINSERT("WindGustKT", kFmiWindGustKT);
  PARAMINSERT("WindGustU", kFmiWindGustU);
  PARAMINSERT("WindGustV", kFmiWindGustV);
  PARAMINSERT("WindShearKT", kFmiWindShearKT);
  PARAMINSERT("WindShearMS", kFmiWindShearMS);
  PARAMINSERT("WindShearMS2", kFmiWindShearMS2);
  PARAMINSERT("WindShearU", kFmiWindShearU);
  PARAMINSERT("WindShearV", kFmiWindShearV);
  PARAMINSERT("WindSpeed10M", kFmiWindSpeed10M);
  PARAMINSERT("WindSpeedDeviation", kFmiWindSpeedDeviation);
  PARAMINSERT("WindSpeedF0", kFmiWindSpeedF0);
  PARAMINSERT("WindSpeedF1", kFmiWindSpeedF1);
  PARAMINSERT("WindSpeedF2.5", kFmiWindSpeedF2_5);
  PARAMINSERT("WindSpeedF5", kFmiWindSpeedF5);
  PARAMINSERT("WindSpeedF10", kFmiWindSpeedF10);
  PARAMINSERT("WindSpeedF12.5", kFmiWindSpeedF12_5);
  PARAMINSERT("WindSpeedF100", kFmiWindSpeedF100);
  PARAMINSERT("WindSpeedF25", kFmiWindSpeedF25);
  PARAMINSERT("WindSpeedF50", kFmiWindSpeedF50);
  PARAMINSERT("WindSpeedF75", kFmiWindSpeedF75);
  PARAMINSERT("WindSpeedF87.5", kFmiWindSpeedF87_5);
  PARAMINSERT("WindSpeedF90", kFmiWindSpeedF90);
  PARAMINSERT("WindSpeedF95", kFmiWindSpeedF95);
  PARAMINSERT("WindSpeedF97.5", kFmiWindSpeedF97_5);
  PARAMINSERT("WindSpeedF99", kFmiWindSpeedF99);
  PARAMINSERT("WindSpeedGeostr", kFmiWindSpeedGeostr);
  PARAMINSERT("WindSpeedKT", kFmiWindSpeedKT);
  PARAMINSERT("WindSpeedMS", kFmiWindSpeedMS);
  PARAMINSERT("WindUMS", kFmiWindUMS);
  PARAMINSERT("WindUMS10M", kFmiWindUMS10M);
  PARAMINSERT("WindVMS", kFmiWindVMS);
  PARAMINSERT("WindVMS10M", kFmiWindVMS10M);
  PARAMINSERT("WindVectorKT", kFmiWindVectorKT);
  PARAMINSERT("WindVectorMS", kFmiWindVectorMS);
  PARAMINSERT("WindVerticalDeviation", kFmiWindVerticalDeviation);
  PARAMINSERT("WmoBlockNumber", kFmiWmoBlockNumber);
  PARAMINSERT("WmoStationNumber", kFmiWmoStationNumber);
  PARAMINSERT("Year", kFmiYear);
  PARAMINSERT("ZResolution", kFmiZResolution);
  PARAMINSERT("Runoff", kFmiRunoff);
  PARAMINSERT("WaterLevelIndex", kFmiWaterLevelIndex);
  PARAMINSERT("SeasonalWaterLevelIndex", kFmiSeasonalWaterLevelIndex);
  PARAMINSERT("FloodWarningIndex", kFmiFloodWarningIndex);
  PARAMINSERT("PrecipitationLevel", kFmiPrecipitationLevel);
  PARAMINSERT("Snow1h", kFmiSnow1h);
  PARAMINSERT("ClearAirTurbulence", kFmiClearAirTurbulence);
  PARAMINSERT("ClearAirTurbulence2", kFmiClearAirTurbulence2);
  PARAMINSERT("HorizontalExtentOfCumulonimbus", kFmiHorizontalExtentOfCumulonimbus);
  PARAMINSERT("InCloudTurbulence", kFmiInCloudTurbulence);
  PARAMINSERT("BirchPollenTemperatureSum", kFmiBirchPollenTemperatureSum);
  PARAMINSERT("BirchPollenConcentration", kFmiBirchPollenConcentration);
  PARAMINSERT("BirchPollenAvailable", kFmiBirchPollenAvailable);
  PARAMINSERT("BirchPollenRemainingRatio", kFmiBirchPollenRemainingRatio);
  PARAMINSERT("GrassPollenTemperatureSum", kFmiGrassPollenTemperatureSum);
  PARAMINSERT("GrassPollenConcentration", kFmiGrassPollenConcentration);
  PARAMINSERT("GrassPollenAvailable", kFmiGrassPollenAvailable);
  PARAMINSERT("GrassPollenRemainingRatio", kFmiGrassPollenRemainingRatio);
  PARAMINSERT("AlderPollenTemperatureSum", kFmiAlderPollenTemperatureSum);
  PARAMINSERT("AlderPollenConcentration", kFmiAlderPollenConcentration);
  PARAMINSERT("AlderPollenAvailable", kFmiAlderPollenAvailable);
  PARAMINSERT("AlderPollenRemainingRatio", kFmiAlderPollenRemainingRatio);
  PARAMINSERT("MugwortPollenTemperatureSum", kFmiMugwortPollenTemperatureSum);
  PARAMINSERT("MugwortPollenConcentration", kFmiMugwortPollenConcentration);
  PARAMINSERT("MugwortPollenAvailable", kFmiMugwortPollenAvailable);
  PARAMINSERT("MugwortPollenRemainingRatio", kFmiMugwortPollenRemainingRatio);
  PARAMINSERT("RagweedPollenTemperatureSum", kFmiRagweedPollenTemperatureSum);
  PARAMINSERT("RagweedPollenConcentration", kFmiRagweedPollenConcentration);
  PARAMINSERT("RagweedPollenAvailable", kFmiRagweedPollenAvailable);
  PARAMINSERT("RagweedPollenRemainingRatio", kFmiRagweedPollenRemainingRatio);
  PARAMINSERT("SeaDepth", kFmiSeaDepth);
  PARAMINSERT("MixedLayerThickness", kFmiMixedLayerThickness);
  PARAMINSERT("SeaSurfaceElevation", kFmiSeaSurfaceElevation);
  PARAMINSERT("HighVegetationCover", kFmiHighVegetationCover);
  PARAMINSERT("SkinReservoirContent", kFmiSkinReservoirContent);
  PARAMINSERT("SoilTemperatureLevel1", kFmiSoilTemperatureLevel1);
  PARAMINSERT("SoilTemperatureLevel2", kFmiSoilTemperatureLevel2);
  PARAMINSERT("SoilTemperatureLevel3", kFmiSoilTemperatureLevel3);
  PARAMINSERT("SoilTemperatureLevel4", kFmiSoilTemperatureLevel4);
  PARAMINSERT("VolumetricSoilWaterLayer1", kFmiVolumetricSoilWaterLayer1);
  PARAMINSERT("VolumetricSoilWaterLayer2", kFmiVolumetricSoilWaterLayer2);
  PARAMINSERT("VolumetricSoilWaterLayer3", kFmiVolumetricSoilWaterLayer3);
  PARAMINSERT("VolumetricSoilWaterLayer4", kFmiVolumetricSoilWaterLayer4);
  PARAMINSERT("HighVegetationType", kFmiHighVegetationType);
  PARAMINSERT("LowVegetationType", kFmiLowVegetationType);
  PARAMINSERT("COConcentration", kFmiCOConcentration);
  PARAMINSERT("NOConcentration", kFmiNOConcentration);
  PARAMINSERT("NO2Concentration", kFmiNO2Concentration);
  PARAMINSERT("O3Concentration", kFmiO3Concentration);
  PARAMINSERT("PM10Concentration", kFmiPM10Concentration);
  PARAMINSERT("PM25Concentration", kFmiPM25Concentration);
  PARAMINSERT("SO2Concentration", kFmiSO2Concentration);
}

// ----------------------------------------------------------------------
/*!
 * Add known parameter value-name pairs into the map.
 *
 * \return False if the table was already initialized, true otherwise.
 */
// ----------------------------------------------------------------------

bool NFmiEnumConverter::EnumTableInit()
{
  if (!itsData.empty()) return false;

  switch (itsEnumSpace)
  {
    case kNoneEnumSpace:
      itsBadEnum = 0;
      return false;
    case kParamNames:
      itsBadEnum = kFmiBadParameter;
      InitParamNames(itsData);
      break;
    case kRoadRegions:
      itsBadEnum = kTieAlueNone;
      InitRoadRegions(itsData);
      break;
    case kPressRegions:
      itsBadEnum = kPressMaskNone;
      InitPressRegions(itsData);
      break;
  }
  return true;
}

// ======================================================================
