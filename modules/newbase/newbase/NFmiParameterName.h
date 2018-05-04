// ======================================================================
/*!
 * \file NFmiParameterName.h
 * \brief Definition of enum FmiParameterName
 */
// ======================================================================
/*!
 * \enum FmiParameterName
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include <iostream>

// **********************************************************************
// Muista muuttaa myös NFmiEnumConverter.cpp, jos lisäät tai poistat
// jonkin parametrin.
// **********************************************************************

enum FmiParameterName
{
  kFmiBadParameter = 0,
  kFmiPressure = 1,
  kFmiGeopHeight,
  kFmiGeomHeight,
  kFmiTemperature,
  kFmiMaximumTemperature,
  kFmiMinimumTemperature,
  kFmiTemperatureAnomaly,
  kFmiPotentialTemperature,                /*theta*/
  kFmiPseudoAdiabaticPotentialTemperature, /*thetaw*/
  kFmiDewPoint = 10,
  kFmiDewPointDeficit,
  kFmiSpecificHumidity,
  kFmiHumidity,
  kFmiMixingRatio,
  kFmiStabilityIndex,
  kFmiSaturationDeficit = 16,
  kFmiTopoGraf = 17,

  kFmiTotalWindMS = 19,
  kFmiWindDirection = 20,
  kFmiWindSpeedMS,
  kFmiWindVectorMS,
  kFmiWindUMS,
  kFmiWindVMS,
  kFmiWindGustU = 25,
  kFmiWindGustV = 26,

  kFmiHeightMinus20C = 28,
  kFmiStreamFunction = 29,
  kFmiVorticityRelative = 30,
  kFmiVorticityAbsolute,
  kFmiVorticityAdvectionRelative,
  kFmiVorticityAdvectionAbsolute,
  kFmiVelocityDivergenceHoriz,
  kFmiMoistureDivergenceHoriz,
  kFmiVorticityGeostrophic,
  kFmiVorticityAdvectionGeostr = 37,

  kFmiVelocityPotential = 39,
  kFmiVerticalVelocityCBS = 40,
  kFmiVerticalVelocityCB12H,
  kFmiVerticalVelocityHPAH,
  kFmiVerticalVelocityMMS,
  kFmiWindShearMS,
  kFmiWindShearKT,
  kFmiLapseRate,
  kFmiPrecipitableWater,
  kFmiPrecipitationConv, /*RRCON*/
  kFmiPrecipitationRate, /*HHSADE*/
  kFmiPrecipitationAmount = 50,
  kFmiSnowDepth,
  kFmiRadiationOutLW,
  kFmiRadiationOutSW,
  kFmiRadiationInSW,
  kFmiPrecipitationLarge = 55, /*RRLAR*/
  kFmiPrecipitationType = 56,
  kFmiPrecipitationForm = 57,  //(hessaan osaparametrejä)
  kFmiGolfIndex = 58,          // TV4 golf-index arvo 0-100 (100 on huippu golfkeli)
  kFmiCAPE = 59,
  kFmiSeaLevel = 60,
  kFmiTemperatureSea,
  kFmiSalinity,
  kFmiDensity = 63,
  kFmiSeaLevelMinimum = 64,
  kFmiSeaLevelMaximum = 65,
  kFmiCIN = 66,
  kFmiHelicity = 67,
  kFmiLCL = 68,
  kFmiLNB = 69,

  kFmiTotalCloudCover = 79,
  kFmiKIndex = 80,
  kFmiRoadT1Derivative = 81,
  kFmiRoadT2Derivative = 82,
  kFmiFreezingPoint1 = 83,
  kFmiFreezingPoint2 = 84,
  kFmiConductivity1 = 85,
  kFmiConductivity2 = 86,
  kFmiSensorFault = 87,
  kFmiSaltSpread1 = 88,
  kFmiSaltSpread2 = 89,
  kFmiSaltConcentration1 = 90,
  kFmiSaltConcentration2 = 91,
  kFmiRoadStatus = 92,
  kFmiFriction3 = 93,
  kFmiWaterContent = 94,
  kFmiSnowContent = 95,
  kFmiIceContent = 96,

  kFmiThickness = 101,

  kFmiRadarBorder = 108,  // is the point at the border of two radar measurements
  kFmiRadarProducer = 109,
  kFmiRadarParameter = 110,
  kFmiRadarLevelType = 111,
  kFmiElevationAngle = 112,
  kFmiCAPPIHeight = 113,
  kFmiRadarRadius = 114,
  kFmiRadarLayer = 115,
  kFmiAzimuthalLayer = 116,
  kFmiCrossSectionDirection = 117,
  kFmiAccumulationTime = 118,
  kFmiMindBZ = 119,
  kFmiMinRange = 120,
  kFmiMaxRange = 121,
  kFmiColumnNumber = 122,
  kFmiRowNumber = 123,
  kFmiHorizontalResolution = 124,
  kFmiZResolution = 125,
  kFmiCorrectedReflectivity = 126,
  kFmiEchoTop = 127,
  kFmiRainfallDepth = 128,
  kFmiRawRadarData = 129,
  kFmiRadialWind = 130,
  kFmiWindAndReflectivity = 131,
  kFmiMaxLayerHeight = 132,
  kFmiMaxElevation = 133,
  kFmiProjectionID = 134,
  kFmiGridOrientation = 135,

  kFmiVelocityPotentialM2S = 139,
  kFmiVerticalVelocityHPAS = 140,
  kFmiVerticalVelocityDPAS,

  kFmiVerticalVelocityMS = 143,
  kFmiWindShearMS2,
  kFmiLogNatOfPressure = 145,
  kFmiLapseRate2 = 146,
  kFmiPrecipitableWater2,

  kFmiPrecipitationRate2,
  kFmiPrecipitationAmount2 = 150,
  kFmiSnowDepth2,
  kFmiRadiationOutLW2,
  kFmiRadiationOutSW2,
  kFmiRadiationInSW2 = 154,
  kFmiBatteryVoltage = 155,

  kFmiSeaLevelAnomaly = 160,
  kFmiTemperatureSea2, /* kFmiCLDTop ??*/
  kFmiTemperatureSeaSurface = 162,
  kFmiCurrentSpeed = 163,
  kFmiCurrentDirection = 164,
  kFmiSigWaveHeight = 165,
  kFmiSigWavePeriod = 166,
  kFmiWaveDirection = 167,
  kFmiWaveSpread = 168,
  kFmiMaxWaveHeight = 169,
  kFmiMaxWavePeriod = 170,
  kFmiWavePeriod = 171,
  kFmiSigWaveHeightBandB = 172,
  kFmiSigWavePeriodBandB = 173,
  kFmiWaveDirectionBandB = 174,
  kFmiSigWaveHeightBandC = 175,
  kFmiSigWavePeriodBandC = 176,
  kFmiWaveDirectionBandC = 177,
  kFmiUnidirectivityIndex = 178,
  kFmiSigWaveLength = 179,
  kFmiMixedLayerDepth = 180,
  kFmiSigWaveHeightSwell0 = 181,
  kFmiSigWaveHeightSwell1 = 182,
  kFmiSigWaveHeightSwell2 = 183,
  kFmiSigWavePeriodSwell0 = 184,
  kFmiSigWavePeriodSwell1 = 185,
  kFmiSigWavePeriodSwell2 = 186,
  kFmiWaveDirectionSwell0 = 187,
  kFmiWaveDirectionSwell1 = 188,
  kFmiWaveDirectionSwell2 = 189,
  kFmiSeaSprayIcing = 190,
  kFmiNO2Contents = 191,
  kFmiCOContents = 192,
  kFmiSO2Contents = 193,
  kFmiAQIndex = 194,
  kFmiSmogIndex = 195,
  kFmiUVIndexMax = 196,
  kFmiUVIndex = 197,
  kFmiOzoneAnomaly = 198,

  kFmiPrecipitationRateLarge = 200,
  kFmiPrecipitationRateConv,
  kFmiPressureReduced = 202,
  kFmiPressureTendency,
  kFmiNOContents = 204,
  kFmiO3Contents = 205,
  kFmiPM10Contents = 206,
  kFmiPM25Contents = 207,

  kFmiTemperatureVirtual = 212,

  kFmiSnowWarning = 220,  // arvot: 0 = ei varoitusta, 1 = lumivaroitus, 2 = lumihälytys

  kFmiPressureAnomaly = 226,
  kFmiGeopotentialHeightAnomaly,

  kFmiSkinTemperature = 235,

  kFmiOpenLandSnowDepth = 239,

  kFmiTOL = 240,
  kFmiQOL = 241,
  kFmiDivergenceAbsolute = 242,

  kFmiDivergenceRelative = 244,
  kFmiWindShearU,
  kFmiWindShearV = 246,

  kFmiVapourPressure = 255,
  kFmiDragCoefficient = 256,
  kFmiEvaporation = 257,

  kFmiPoP = 259,  // probability of precipitation
  kFmiProbabilityThunderstorm = 260,
  kFmiFlashAccuracy = 261,
  kFmiFlashMultiplicity = 262,
  kFmiFlashStrength = 263,
  kFmiSnowfallRate = 264,
  kFmiSnowAccumulation,
  kFmiWaterEquivalentOfSnow = 266,
  kFmiWaterAccumulation,  // kFmiWaterEquivalentOfGraupel = 267,
  kFmiSnowWaterRatio,

  kFmiFreezingLevel = 270,
  kFmiMiddleAndLowCloudCover = 271,
  kFmiConvectiveCloudCover = 272,
  kFmiLowCloudCover,
  kFmiMediumCloudCover,
  kFmiHighCloudCover,
  kFmiCloudWater = 276,

  kFmiConvectiveSnow = 278,
  kFmiLargeScaleSnow = 279,

  kFmiLandSeaMask = 281,
  kFmiForestFraction = 282,
  kFmiSurfaceRoughness = 283,
  kFmiAlbedo,
  kFmiSoilTemperature,
  kFmiSoilMoistureContent,
  kFmiVegetation,
  kFmiGroundTemperature = 288,
  kFmiSurfaceSoilWetness = 289,
  kFmiLocationId = 290,
  kFmiYear = 291,
  kFmiMonth = 292,
  kFmiDay = 293,
  kFmiHour = 294,
  kFmiMinute = 295,
  kFmiSecond = 296,
  kFmiForecastPeriod = 297,
  kFmiModelLevel = 298,
  kFmiCloudCondensate = 299,

  kFmiTurbulentKineticEnergy = 301,
// kFmiTemperatureChgByParamn, Ei enää tule Hirlamista
// kFmiHumidityChgByParamn,
// kFmiCloudWaterChgByParamn = 304,
#define NO_kFmixxxChgByParamn
  // tells 'NFmiEnumConverter.cpp' that we don't have them

  kFmiFlashLength = 305,
  kFmiFlashWidth,
  kFmiFlashDirection,
  kFmiFlashDuration = 308,

  kFmiRadiationNetSurfaceSW = 311,
  kFmiRadiationNetSurfaceLW,
  kFmiRadiationNetTopAtmSW,
  kFmiRadiationNetTopAtmLW,
  kFmiRadiationLW,
  kFmiRadiationSW,
  kFmiRadiationGlobal,
  kFmiRadiationReflected = 318,

  kFmiLatentHeatFlux = 321,
  kFmiSensibleHeatFlux,
  kFmiBoundaryLayerDissipation,
  kFmiUMomentumFlux = 324,
  kFmiVMomentumFlux = 325,
  kFmiWeatherAndCloudiness = 326,
  kFmiFogIntensity = 327,
  kFmiCloudSymbol = 328,
  kFmiLowCloudSymbol,
  kFmiMediumCloudSymbol,
  kFmiHighCloudSymbol,
  kFmiConvCloudSymbol,
  kFmiFrontSymbol,
  kFmiFogSymbol,
  kFmiSmogSymbol,
  kFmiWeatherSymbol1,       /*HSADE1*/
  kFmiWeatherSymbol2,       /*HSADE2*/
  kFmiWeatherSymbol3 = 338, /*HESSAA*/
  kFmiSimpleWeather = 339,
  kFmiWindSpeedGeostr = 340,
  kFmiWindSpeed10M,
  kFmiWindDirection10M,
  kFmiTemperature2M,
  kFmiDewPoint2M = 344,
  kFmiHumidity2M = 345,
  kFmiWindUMS10M = 346,
  kFmiWindVMS10M = 347,
  kFmiShipDirection = 348,
  kFmiShipSpeed = 349,
  kFmiWeatherSymbol = 350,
  kFmiChillFactor = 351,
  kFmiDegreeDays,
  kFmiPrecipitation1h = 353,
  kFmiPrecipitation3h,
  kFmiPrecipitation6h = 355, /*SADE_6*/
  kFmiPrecipitation12h,      /*SADE12*/
  kFmiPrecipitation24h = 357,
  kFmiMaximumTemperature24h = 358,
  kFmiMinimumTemperature24h = 359,
  kFmiMaximumTemperature2m = 360,
  kFmiMinimumTemperature2m = 361,
  kFmiMinimumWind = 362,
  kFmiPrecipitation4h = 363,
  kFmiCloudTopBrightnessTemperature = 364,
  kFmiWaterVapourBrightnessTemperature = 365,
  kFmiWaterVapourBrightnessTemperatureIncludingCloudCorrection = 366,
  kFmiMaximumIcingIndex = 367,
  kFmiCloudWaterReflectivity = 368,

  kFmiSatoKierto = 370,

  kFmiFrostProbability = 372,
  kFmiSevereFrostProbability,

  kFmiConvectiveSnowFallRate = 378,
  kFmiLargeScaleSnowFallRate = 379,
  kFmiConvectiveAvailablePotentialEnergy = 380,
  kFmiRoughness = 381,  // See also: SurfaceRoughness 283
  kFmiStormRelativeHelicity = 382,

  kFmiSurfaceRoughnessAtSea = 383,
  kFmiIceConcentration = 384,
  kFmiIceThickness = 385,  // ollut joskus kFmiWindInstantaneous
  kFmiIceMinThickness = 386,
  kFmiIceMaxThickness = 387,
  kFmiIceDegreeOfRidging = 388,
  kFmiIceSpeed = 389,
  kFmiIceDirection = 390,

  kFmiCanopyWater = 392,
  kFmiGroundFrost = 393,
  kFmiSurfaceTypeFraction = 394,
  kFmiSoilType = 395,

  kFmiVegetationType = 399,
  kFmiForecasterCode = 400,
  kFmiWmoBlockNumber = 401,
  kFmiWmoStationNumber,
  kFmiStationType,
  kFmiLatitude,
  kFmiLongitude,
  kFmiPressureChange,
  kFmiVisibility,
  kFmiPresentWeather = 408,
  kFmiPastWeather1,
  kFmiPastWeather2 = 410,
  kFmiLowCloudType,
  kFmiMiddleCloudType,
  kFmiHighCloudType,
  kFmiCloudHeight,
  kFmiPrecipitationPeriod,
  kFmiMaximumWind,
  kFmiWindGust,
  kFmiOldVisibility,
  kFmiVisibilityChange,
  kFmiRainOnOff = 420,
  kFmiSunOnOff,
  kFmiWetnessOnOff,
  kFmiRainMinutes,
  kFmiStateOfGround = 424,
  kFmiSnowDepth06 = 425,
  kFmiSnowDepth18 = 426,
  kFmiDailyMeanTemperature,
  kFmiMinimumTemperature06,
  kFmiMaximumTemperature06,
  kFmiMinimumGroundTemperature06,
  kFmiMinimumTemperature18,
  kFmiMaximumTemperature18,
  kFmiMonthlyMeanTemperature = 433,
  kFmiMonthlyPrecipitation = 434,
  kFmiSunHours = 435,
  kFmiDailyGlobalRadiation,
  kFmiDailyDiffuseRadiation,
  kFmiDailyReflectedRadiation,
  kFmiDailyNetRadiation,
  kFmiPrecipitationReliability,
  kFmiTotalOzone = 441,
  kFmiUVCumulated = 442,
  kFmiUVMaximum = 443,
  kFmiTimeOfUVMaximum = 444,
  kFmiDailyPrecipitationCode,
  kFmiHourlySunShine = 446,
  kFmiHourlyGlobalRadiation,
  kFmiHourlyDiffuseRadiation,
  kFmiHourlyReflectedRadiation,
  kFmiHourlyNetRadiation = 450,
  kFmiMonthlyMeanPrecipitation3160,
  kFmiMonthlyMeanTemperature3160,
  kFmiMonthlyMeanCloudiness3160,
  kFmiMonthly15SnowDepth3160 = 454,
  kFmiMonthlyMeanPrecipitation6190,
  kFmiMonthlyMeanTemperature6190,
  kFmiMonthlyMeanCloudiness6190,
  kFmiMonthly15SnowDepth6190 = 458,
  kFmiHourlyPressure = 459,
  kFmiHourlyTemperature = 460,
  kFmiHourlyMinimumTemperature,
  kFmiHourlyMaximumTemperature,
  kFmiHourlyRelativeHumidity,
  kFmiHourlyWindSpeed = 464,
  kFmiHourlyMinimumWindSpeed = 465,
  kFmiHourlyMaximumWindSpeed = 466,
  kFmiHourlyMaximumGust = 467,
  kFmiHourlyWindDirection = 468,
  kFmiHourlyMaxRainIntensity = 469,
  kFmiRainIntensityWeather = 470,
  kFmiHourlyWaterTemperature = 471,
  kFmiPressureAtStationLevel = 472,
  kFmiPrecipitation06 = 473,
  kFmiPrecipitation18 = 474,
  kFmiFlAltitude = 475,
  kFmiPhaseOfFlight = 476,
  kFmiTurbulence = 477,
  kFmiBaseOfTurbulence = 478,
  kFmiTopOfTurbulence = 479,
  kFmiIcing = 480,
  kFmiStdDevTotalCloud = 481,
  kFmiTemperatureHighCld = 482,
  kFmiStdDevHighCld = 483,
  kFmiTemperatureLMCld = 484,
  kFmiStdDevLMCld = 485,
  kFmiFogPercentage = 486,
  kFmiLandPercentage = 487,
  kFmiSeaPercentage = 488,
  kFmiSnowPercentage = 489,
  kFmiSunGlintPercentage = 490,
  kFmiIR = 491,
  kFmiVIS = 492,
  kFmiWV = 493,
  kFmiCH112 = 494,
  kFmiPrecipitation7Days = 495,
  kFmiPrecipitation10Days = 496,
  kFmiPrecipitation30Days = 497,
  kFmiSunSeconds = 498,
  kFmiHaKeTaskCode = 499,
  kFmiCloudBase = 500,
  kFmi1CloudCover = 501,
  kFmi1CloudBase,
  kFmi2CloudCover,
  kFmi2CloudBase,
  kFmi3CloudCover,
  kFmi3CloudBase,
  kFmi4CloudCover,
  kFmi4CloudBase,
  kFmiCbCloudCover,
  kFmiCbCloudBase = 510,
  kFmiSolarElevation = 511,   // 28.3.2000/Lasse
  kFmiCloudCeilingHFT = 513,  // Tero 22.10.2010 Cloud Ceiling is the height of the lowest cloud in
                              // the sky covering more than half the sky (BKN, OVC)

  kFmiWindGustKT = 514,
  kFmiAviationVisibility = 515,
  kFmiVerticalVisibility,
  kFmiWindSpeedKT,
  kFmiHourlyMaximumGustKT,
  kFmiWindVectorKT,
  kFmiWeatherSymbolTaf = 520,
  kFmiAviationWeather1,
  kFmiAviationWeather2,
  kFmiAviationWeather3,
  kFmiPackedAviationCloud1,
  kFmiPackedAviationCloud2,
  kFmiPackedAviationCloud3,
  kFmiPackedAviationCloud4,
  kFmiLandCover = 530,

  kFmiIceCover = 532,
  kFmiDeepSoilTemperature = 533,
  kFmiDeepSoilMoistureContent = 534,
  kFmiForestGroundHumidity = 535,
  kFmiFieldGroundHumidity = 536,
  kFmiPrecipitation5d = 537,
  kFmiForestFireWarning = 538,

  kFmiGroundHumidityForest = 540,
  kFmiClusterTemperature1 = 541,
  kFmiClusterTemperature2 = 542,
  kFmiClusterTemperature3 = 543,
  kFmiClusterTemperature4 = 544,
  kFmiClusterTemperature5 = 545,
  kFmiClusterTemperature6 = 546,
  kFmiClusterGeopHeight1 = 547,
  kFmiClusterGeopHeight2 = 548,
  kFmiClusterGeopHeight3 = 549,
  kFmiClusterGeopHeight4 = 550,
  kFmiClusterGeopHeight5 = 551,
  kFmiClusterGeopHeight6 = 552,

  kFmiEFIWindSpeed = 556,
  kFmiEFITemperature = 557,
  kFmiEFIWindGust = 558,
  kFmiEFIWindPrecipitation = 559,
  kFmiProbabilityOfTemp = 560,
  kFmiProbabilityOfTempLimit1 = 561,  // EPS: tod.n. T-poikkeama alle -8 astetta / T < -15
  kFmiProbabilityOfTempLimit2 = 562,  // EPS: tod.n. T-poikkeama alle -4 astetta / T < -20
  kFmiProbabilityOfTempLimit3 = 563,  // EPS: tod.n. T-poikkeama yli +4 astetta / T < -25
  kFmiProbabilityOfTempLimit4 = 564,  // EPS: tod.n. T-poikkeama yli +8 astetta / T < -30

  kFmiProbabilityOfPrec = 570,
  kFmiProbabilityOfPrecLimit1 = 571,  // EPS: tod.n. rr24h yli 1mm / 6 h > 1 mm
  kFmiProbabilityOfPrecLimit2 = 572,  // EPS: tod.n. rr24h yli 5mm / 6 h > 5 mm
  kFmiProbabilityOfPrecLimit3 = 573,  // EPS: tod.n. rr24h yli 10mm / 6 h > 10 mm
  kFmiProbabilityOfPrecLimit4 = 574,  // EPS: tod.n. rr24h yli 20mm
  kFmiProbabilityOfPrecLimit5 = 575,
  kFmiProbabilityOfPrecLimit6 = 576,
  kFmiProbabilityOfPrecLimit7 = 577,
  kFmiProbabilityOfPrecLimit8 = 578,
  kFmiProbabilityOfPrecLimit9 = 579,

  kFmiProbabilityOfWind = 580,
  kFmiProbabilityOfWindLimit1 = 581,  // EPS: tod.n. keskituuli yli 10
  kFmiProbabilityOfWindLimit2 = 582,  // EPS: tod.n. keskituuli yli 20
  kFmiProbabilityOfGustLimit1 = 583,  // EPS: tod.n. puuska yli 15
  kFmiProbabilityOfGustLimit2 = 584,  // EPS: tod.n. puuska yli 20
  kFmiProbabilityOfGustLimit3 = 585,  // EPS: tod.n. puuska yli 25
  kFmiProbabilityOfGustLimit4 = 586,  // EPS: tod.n. puuska yli 30
  kFmiProbabilityOfGustLimit5 = 587,  // EPS: tod.n. puuska yli 35

  kFmiProbabilityOfPrecLimit10 = 588,  // NOTE: CONTINUED FROM ABOVE!!

  kFmiTopoSlope = 590,
  kFmiTopoAzimuth,
  kFmiTopoRelativeHeight,
  kFmiTopoDistanceToSea,
  kFmiTopoDirectionToSea,
  kFmiTopoDistanceToLand,
  kFmiTopoDirectionToLand = 596,

  kFmiSensorOrdinal = 600,
  kFmiDeltaTime,

  // Pollen models
  // birch = koivu, grass = ruoho, alder = leppä, mugwort = pujo, ragweed = heinä
  // Remaining ratio becomes available and then becomes concentration when released

  kFmiBirchPollenTemperatureSum = 602,  // accumulated temperature sum for birch flowering
  kFmiBirchPollenConcentration = 603,   // amount of pollen released
  kFmiBirchPollenAvailable = 604,       // amount ready to be released when weather permits
  kFmiBirchPollenRemainingRatio = 605,  // ratio of flowering remaining to go

  kFmiGrassPollenTemperatureSum = 606,
  kFmiGrassPollenConcentration = 607,
  kFmiGrassPollenAvailable = 608,
  kFmiGrassPollenRemainingRatio = 609,

  kFmiAlderPollenTemperatureSum = 610,
  kFmiAlderPollenConcentration = 611,
  kFmiAlderPollenAvailable = 612,
  kFmiAlderPollenRemainingRatio = 613,

  kFmiMugwortPollenTemperatureSum = 614,
  kFmiMugwortPollenConcentration = 615,
  kFmiMugwortPollenAvailable = 616,
  kFmiMugwortPollenRemainingRatio = 617,

  kFmiRagweedPollenTemperatureSum = 618,
  kFmiRagweedPollenConcentration = 619,
  kFmiRagweedPollenAvailable = 620,
  kFmiRagweedPollenRemainingRatio = 621,

  // Tiesäämallin ja aurausmallin parametreja
  kFmiFrostSum = 634,  // kFmiGrowthPeriodPrecipitationAnomaly
  kFmiRoadFrostCover = 635,
  kFmiRoadSnowCover = 636,
  kFmiRoadMaintenanceMeasure = 637,
  kFmiTimeToNextMeasure = 638,
  kFmiWaterStorage = 639,
  kFmiTimeToNextSalting = 640,
  kFmiRoadCondition = 641,
  kFmiProbabilityDryRoad = 642,
  kFmiProbabilityWetRoad = 643,
  kFmiProbabilityMoistRoad = 644,
  kFmiProbabilitySnowyRoad = 645,
  kFmiProbabilityFrostyRoad = 646,
  kFmiProbabilityIcyRoad = 647,
  kFmiRoadTemperature = 648,
  kFmiGrowthPeriodDeviationInDays = 649,
  kFmiGrowthPeriodDeviationInPrcnt = 650,
  kFmiGrowthPeriodPrecipitation = 651,
  kFmiEffectiveTemperatureSum = 652,
  kFmiRoadConditionSeverity = 653,
  kFmiRoadTemperature2 = 654,
  kFmiDeepRoadTemperature = 655,
  kFmiDeepRoadTemperature2 = 656,
  kFmiRoadConditionAlternative = 657,
  kFmiGrowthPeriodOnOff = 658,
  kFmiRoadNotification = 659,
  kFmiRoadWarning = 660,
  kFmiPedestrianIndex = 661,
  kFmiRoadReasoning = 662,
  kFmiTrafficIndex = 663,
  kFmiFriction = 664,
  kFmiRoadIceCover = 665,               // ice on road
  kFmiRoadIceCoverOnVehiclePath = 666,  // ice on actual vehicle path (more worn out)

  kFmiWindDirectionCode = 670,
  kFmiWindDirectionVoltage,
  kFmiAverageWindDirection,
  kFmiMaximumWindDirection,
  kFmiMinimumWindDirection,
  kFmiWindDirectionDeviation,
  kFmiWindSpeedDeviation,
  kFmiWindVerticalDeviation = 677,

  kFmiTemperatureCode = 680,
  kFmiTemperatureVoltage,
  kFmiAverageTemperature,
  kFmiTemperatureDeviation = 683,

  kFmiHumidityCode = 690,
  kFmiHumidityVoltage,
  kFmiAverageHumidity,
  kFmiMaximumHumidity,
  kFmiMinimumHumidity,
  kFmiHumidityDeviation = 695,

  kFmiPressureCode = 700,
  kFmiPressureVoltage,
  kFmiAveragePressure,
  kFmiMaximumPressure,
  kFmiMinimumPressure,
  kFmiPressureDeviation,
  kFmiSurfaceWaterPhase = 706,
  kFmiProbabilityOfSnow = 707,

  kFmiVerticalSoundingSignificance = 708,

  kFmiSatelCh1 = 710,
  kFmiSatelCh2,
  kFmiSatelCh3,
  kFmiSatelCh4,
  kFmiSatelCh5,
  kFmiSatelCh6,
  kFmiSatelCh112,
  kFmiSatelCh124,
  kFmiSatelCh345,
  kFmiSatelCh7,
  kFmiSatelCh8,
  kFmiSatelCh9,
  kFmiSatelCh10,
  kFmiSatelCh11,
  kFmiSatelCh12,
  kFmiSatelCh129 = 730,  // meteosat8/noaa yhdistelmä kanavia, ch 1,2,9 (jätin tilaa tavallisille
                         // kanaville tuohon väliin)
  kFmiSatelCh239,
  kFmiSatelCh321,
  kFmiSatelCh4910,  // ch 4,9,10
  kFmiSatelCh164,

  kFmi1CloudType = 745,
  kFmi2CloudType,
  kFmi3CloudType,
  kFmi4CloudType,  // = 748
                   // 12.12.02/EL ----------------------------->

  kFmiMist = 749,
  kFmi_FL_1_Base = 750,
  kFmi_FL_1_Top,
  kFmi_FL_1_Cover,
  kFmi_FL_1_CloudType,

  kFmi_FL_2_Base = 754,
  kFmi_FL_2_Top,
  kFmi_FL_2_Cover,
  kFmi_FL_2_CloudType,

  kFmi_FL_3_Base = 758,
  kFmi_FL_3_Top,
  kFmi_FL_3_Cover,
  kFmi_FL_3_CloudType,

  kFmi_FL_4_Base = 762,
  kFmi_FL_4_Top,
  kFmi_FL_4_Cover,
  kFmi_FL_4_CloudType,

  kFmi_FL_5_Base = 766,
  kFmi_FL_5_Top,
  kFmi_FL_5_Cover,
  kFmi_FL_5_CloudType,

  kFmi_FL_6_Base = 770,
  kFmi_FL_6_Top,
  kFmi_FL_6_Cover,
  kFmi_FL_6_CloudType,

  kFmi_FL_7_Base = 774,
  kFmi_FL_7_Top,
  kFmi_FL_7_Cover,
  kFmi_FL_7_CloudType,

  kFmi_FL_8_Base = 778,
  kFmi_FL_8_Top,
  kFmi_FL_8_Cover,
  kFmi_FL_8_CloudType = 781,

  kFmi_FL_Cb_Base,
  kFmi_FL_Cb_Cover,

  kFmi_FL_Min_Base,
  kFmi_FL_Max_Base = 785,

  // 12.12.02/EL <-----------------------------

  // 30.5.03 Lasse ----------------------->
  kFmiPollenIndexNut = 786,
  kFmiPollenIndexAlder = 787,
  kFmiPollenIndexBirch = 788,
  kFmiPollenIndexHay = 790,
  kFmiPollenIndexMugwort = 789,
  kFmiPollenIndexMould = 791,
  kFmiPollenIndexSum = 792,

  kFmiNormalGroundMinTemperature = 793,
  kFmiNormalGroundMinTemperatureF02 = 794,
  kFmiNormalGroundMinTemperatureF12 = 795,
  kFmiNormalGroundMinTemperatureF37 = 796,
  kFmiNormalGroundMinTemperatureF50 = 797,
  kFmiNormalGroundMinTemperatureF63 = 798,
  kFmiNormalGroundMinTemperatureF88 = 799,
  kFmiNormalGroundMinTemperatureF98 = 800,

  kFmiNormalMeanTemperature = 801,
  kFmiNormalMeanTemperatureF02 = 802,
  kFmiNormalMeanTemperatureF12 = 803,
  kFmiNormalMeanTemperatureF37 = 804,
  kFmiNormalMeanTemperatureF50 = 805,
  kFmiNormalMeanTemperatureF63 = 806,
  kFmiNormalMeanTemperatureF88 = 807,
  kFmiNormalMeanTemperatureF98 = 808,
  kFmiNormalMaxTemperature = 811,
  kFmiNormalMaxTemperatureF02 = 812,
  kFmiNormalMaxTemperatureF12 = 813,
  kFmiNormalMaxTemperatureF37 = 814,
  kFmiNormalMaxTemperatureF50 = 815,
  kFmiNormalMaxTemperatureF63 = 816,
  kFmiNormalMaxTemperatureF88 = 817,
  kFmiNormalMaxTemperatureF98 = 818,
  kFmiNormalMinTemperature = 821,
  kFmiNormalMinTemperatureF02 = 822,
  kFmiNormalMinTemperatureF12 = 823,
  kFmiNormalMinTemperatureF37 = 824,
  kFmiNormalMinTemperatureF50 = 825,
  kFmiNormalMinTemperatureF63 = 826,
  kFmiNormalMinTemperatureF88 = 827,
  kFmiNormalMinTemperatureF98 = 828,
  // 30.5.03 Lasse <------------------------

  kFmiPoP24h = 829,
  kFmiPoP18 = 830,
  kFmiPoP06 = 831,
  kFmiEvaporation18 = 832,

  kFmiMetarObservationType = 840,

  // Antti Westerberg lisäsi tiesäävaroituksien Kriging-analyysia varten
  kFmiRoadWarning1 = 841,
  kFmiRoadWarning2 = 842,
  kFmiRoadWarning3 = 843,
  kFmiRoadWarning4 = 844,
  kFmiRoadWarning5 = 845,
  kFmiRoadWarning6 = 846,

  // Reijo Solantien lumikuormamallit

  kFmiSnowLoad = 850,       // lumikuorma (=853+854+855)
  kFmiHoarFrostLoad = 851,  // huurrekuorma
  kFmiTotalSnowLoad = 852,  // kokonaiskuorma (lumi + huurre)

  kFmiDrySnowLoad = 853,     // kuiva lumikuorma
  kFmiWetSnowLoad = 854,     // märkä lumikuorma
  kFmiFrozenSnowLoad = 855,  // jäätynyt lumikuorma
  kFmiSnowLoadType = 856,    // kokonaislumikuorman dominoiva tyyppi

  // Sateen koostumus

  kFmiPrecipitationWater = 857,  // vesisateen määrä
  kFmiPrecipitationSnow = 858,   // lumisateen määrä
  kFmiPrecipitationSleet = 859,  // räntäsateen määrä

  // Lumikuormamalli jatkuu

  kFmiTotalSnowLoadChange = 860,  // kokonaiskuorman muutos

  // ECMWF:n viikottaiset kuukausiennusteet

  kFmiTemperatureAnomalyMinus2 = 861,  // 2TALM2
  kFmiTemperatureAnomalyMinus1 = 862,  // 2TALM1
  kFmiTemperatureAnomalyPlus0 = 863,   // 2TAG0
  kFmiTemperatureAnomalyPlus1 = 864,   // 2TAG1
  kFmiTemperatureAnomalyPlus2 = 865,   // 2TAG2
  kFmiPrecipitationAnomaly10 = 866,    // TPAG10
  kFmiPrecipitationAnomaly20 = 867,    // TPAG20

  // Tutkadatan laatu 0-100
  kFmiDetectability = 868,

  kFmiVerticalMeltingArea = 870,

  // EC EPS lämpötilafraktiilit

  kFmiTemperatureF100 = 871,
  kFmiTemperatureF90 = 872,
  kFmiTemperatureF75 = 873,
  kFmiTemperatureF50 = 874,
  kFmiTemperatureF25 = 875,
  kFmiTemperatureF10 = 876,
  kFmiTemperatureF0 = 877,

  kFmiTotalCloudCoverF100 = 881,
  kFmiTotalCloudCoverF90 = 882,
  kFmiTotalCloudCoverF75 = 883,
  kFmiTotalCloudCoverF50 = 884,
  kFmiTotalCloudCoverF25 = 885,
  kFmiTotalCloudCoverF10 = 886,
  kFmiTotalCloudCoverF0 = 887,

  kFmiTotalPrecipitationF100 = 891,
  kFmiTotalPrecipitationF90 = 892,
  kFmiTotalPrecipitationF75 = 893,
  kFmiTotalPrecipitationF50 = 894,
  kFmiTotalPrecipitationF25 = 895,
  kFmiTotalPrecipitationF10 = 896,
  kFmiTotalPrecipitationF0 = 897,

  // Added 10.11.2008 - Antti Westerberg
  kFmiExceptionalPrecipitation = 910,
  kFmi1CloudTop = 911,
  kFmi2CloudTop,
  kFmi3CloudTop,
  kFmi4CloudTop,
  kFmiCbCloudTop,  //= 915

  // Tuliset ensemble parameter: mm amount for given fractile

  kFmiPrecipitation3hF0 = 920,
  kFmiPrecipitation3hF1 = 921,
  kFmiPrecipitation3hF2 = 922,
  kFmiPrecipitation3hF5 = 923,
  kFmiPrecipitation3hF6 = 924,
  kFmiPrecipitation3hF7 = 925,
  kFmiPrecipitation3hF8 = 926,
  kFmiPrecipitation3hF9 = 927,
  kFmiPrecipitation3hF10 = 928,
  kFmiPrecipitation3hF12 = 929,
  kFmiPrecipitation3hF20 = 930,
  kFmiPrecipitation3hF25 = 931,
  kFmiPrecipitation3hF30 = 932,
  kFmiPrecipitation3hF37 = 933,
  kFmiPrecipitation3hF40 = 934,
  kFmiPrecipitation3hF50 = 935,
  kFmiPrecipitation3hF60 = 936,
  kFmiPrecipitation3hF63 = 937,
  kFmiPrecipitation3hF70 = 938,
  kFmiPrecipitation3hF75 = 939,
  kFmiPrecipitation3hF80 = 940,
  kFmiPrecipitation3hF88 = 941,
  kFmiPrecipitation3hF90 = 942,
  kFmiPrecipitation3hF95 = 943,
  kFmiPrecipitation3hF98 = 944,
  kFmiPrecipitation3hF99 = 945,
  kFmiPrecipitation3hF100 = 946,

  // Tuliset ensemble parameter: probability that precipitation
  // exceeds given mm amount

  kFmiProbabilityOfPrecipitation3h0mm = 947,   // > 0 mm
  kFmiProbabilityOfPrecipitation3h01mm = 948,  // > 0.1 mm
  kFmiProbabilityOfPrecipitation3h05mm = 949,  // > 0.5 mm
  kFmiProbabilityOfPrecipitation3h1mm = 950,   // > 1 mm
  kFmiProbabilityOfPrecipitation3h2mm = 951,
  kFmiProbabilityOfPrecipitation3h3mm = 952,
  kFmiProbabilityOfPrecipitation3h4mm = 953,
  kFmiProbabilityOfPrecipitation3h5mm = 954,
  kFmiProbabilityOfPrecipitation3h6mm = 955,
  kFmiProbabilityOfPrecipitation3h7mm = 956,
  kFmiProbabilityOfPrecipitation3h8mm = 957,
  kFmiProbabilityOfPrecipitation3h9mm = 958,
  kFmiProbabilityOfPrecipitation3h10mm = 959,
  kFmiProbabilityOfPrecipitation3h12mm = 960,
  kFmiProbabilityOfPrecipitation3h14mm = 961,
  kFmiProbabilityOfPrecipitation3h16mm = 962,
  kFmiProbabilityOfPrecipitation3h18mm = 963,
  kFmiProbabilityOfPrecipitation3h20mm = 964,
  kFmiProbabilityOfPrecipitation3h25mm = 965,
  kFmiProbabilityOfPrecipitation3h30mm = 966,
  kFmiProbabilityOfPrecipitation3h35mm = 967,
  kFmiProbabilityOfPrecipitation3h40mm = 968,
  kFmiProbabilityOfPrecipitation3h45mm = 969,
  kFmiProbabilityOfPrecipitation3h50mm = 970,
  kFmiProbabilityOfPrecipitation3h60mm = 971,
  kFmiProbabilityOfPrecipitation3h70mm = 972,
  kFmiProbabilityOfPrecipitation3h80mm = 973,
  kFmiProbabilityOfPrecipitation3h90mm = 974,
  kFmiProbabilityOfPrecipitation3h100mm = 975,
  kFmiProbabilityOfPrecipitation3h150mm = 976,
  kFmiProbabilityOfPrecipitation3h200mm = 977,
  kFmiProbabilityOfPrecipitation3h500mm = 978,

  kFmiFlagDate = 999,

  kFmiProbabilityOfWindLimit3 = 1023,  // EPS: tod.n. keskituuli yli 17
  kFmiProbabilityOfWindLimit4 = 1024,  // EPS: tod.n. keskituuli yli 21
  kFmiProbabilityOfWindLimit5 = 1025,  // EPS: tod.n. keskituuli yli 25
  kFmiProbabilityOfWindLimit6 = 1026,  // EPS: tod.n. keskituuli yli 30

  kFmiProbabilityOfColdLimit1 = 1031,  // EPS: tod.n. T < -15
  kFmiProbabilityOfColdLimit2 = 1032,  // EPS: tod.n. T < -20
  kFmiProbabilityOfColdLimit3 = 1033,  // EPS: tod.n. T < -25
  kFmiProbabilityOfColdLimit4 = 1034,  // EPS: tod.n. T < -30

  kFmiProbabilityOfHeatLimit1 = 1041,  // EPS: tod.n. T > 25
  kFmiProbabilityOfHeatLimit2 = 1042,  // EPS: tod.n. T > 27
  kFmiProbabilityOfHeatLimit3 = 1043,  // EPS: tod.n. T > 30

  kFmiProbabilityOfCAPE1 = 1051,  // EPS: tod.n. CAPE > 50
  kFmiProbabilityOfCAPE2 = 1052,  // EPS: tod.n. CAPE > 500
  kFmiProbabilityOfCAPE3 = 1053,  // EPS: tod.n. CAPE > 1000
  kFmiProbabilityOfCAPE4 = 1054,  // EPS: tod.n. CAPE > 1500

  kFmiProbabilityOfPr6Limit1 = 1061,  // EPS: tod.n. rr6h > 1mm
  kFmiProbabilityOfPr6Limit2 = 1062,  // EPS: tod.n. rr6h > 5mm
  kFmiProbabilityOfPr6Limit3 = 1063,  // EPS: tod.n. rr6h > 10mm
  kFmiProbabilityOfPr6Limit4 = 1064,  // EPS: tod.n. rr6h > 15mm
  kFmiProbabilityOfPr6Limit5 = 1065,  // EPS: tod.n. rr6h > 30 mm
  kFmiProbabilityOfPr6Limit6 = 1066,  // EPS: tod.n. rr6h > 50 mm

  kFmiProbabilityOfPr24Limit1 = 1071,  // EPS: tod.n. rr24h > 10 mm
  kFmiProbabilityOfPr24Limit2 = 1072,  // EPS: tod.n. rr24h > 30 mm
  kFmiProbabilityOfPr24Limit3 = 1073,  // EPS: tod.n. rr24h > 50 mm
  kFmiProbabilityOfPr24Limit4 = 1074,  // EPS: tod.n. rr24h > 70 mm
  kFmiProbabilityOfPr24Limit5 = 1075,  // EPS: tod.n. rr24h > 120 mm

  kFmiProbabilityOfSn6Limit1 = 1081,  // EPS: tod.n. SN 6h > 1mm
  kFmiProbabilityOfSn6Limit2 = 1082,  // EPS: tod.n. SN 6h > 5mm
  kFmiProbabilityOfSn6Limit3 = 1083,  // EPS: tod.n. SN 6h > 10mm
  kFmiProbabilityOfSn6Limit4 = 1084,  // EPS: tod.n. SN 6h > 15mm
  kFmiProbabilityOfSn6Limit5 = 1085,  // EPS: tod.n. SN 6h > 30 mm
  kFmiProbabilityOfSn6Limit6 = 1086,  // EPS: tod.n. SN 6h > 50 mm

  kFmiProbabilityOfSn24Limit1 = 1091,  // EPS: tod.n. SN 24h > 1mm
  kFmiProbabilityOfSn24Limit2 = 1092,  // EPS: tod.n. SN 24h > 5mm
  kFmiProbabilityOfSn24Limit3 = 1093,  // EPS: tod.n. SN 24h > 10mm
  kFmiProbabilityOfSn24Limit4 = 1094,  // EPS: tod.n. SN 24h > 15mm
  kFmiProbabilityOfSn24Limit5 = 1095,  // EPS: tod.n. SN 24h > 30 mm
  kFmiProbabilityOfSn24Limit6 = 1096,  // EPS: tod.n. SN 24h > 50 mm

  kFmiBioMassSum = 1097,  // Aquamare NetCDF parameters
  kFmiBioMassSumChange = 1098,
  kFmiBioMassSumChangeP = 1099,
  kFmiCyanosSum = 1100,
  kFmiCyanosSumChange = 1101,
  kFmiCyanosSumChangeP = 1102,

  // Added to be able to convert Latvian radar data

  kFmiReflectivity = 1103,    // Uncorrected reflectivity of radar data
  kFmiRadialVelocity = 1104,  // Radial velocity of precipitation
  kFmiSpectralWidth = 1105,   // Spectral width of radial velocity

  kFmiAshConcentration = 1106,  // MetOffice ash concentration data

  kFmiGrowthPeriodStarted = 1107,
  kFmiGrowthPeriodEnded = 1108,

  kFmiAshOnOff = 1109,  // MetOffice ash concentration data: 0=none, 1=ash

  // More fractile numbers KN 04072012

  kFmiWindSpeedF100 = 1110,
  kFmiWindSpeedF90 = 1111,
  kFmiWindSpeedF75 = 1112,
  kFmiWindSpeedF50 = 1113,
  kFmiWindSpeedF25 = 1114,
  kFmiWindSpeedF10 = 1115,
  kFmiWindSpeedF0 = 1116,

  kFmiWindGustF100 = 1117,
  kFmiWindGustF90 = 1118,
  kFmiWindGustF75 = 1119,
  kFmiWindGustF50 = 1120,
  kFmiWindGustF25 = 1121,
  kFmiWindGustF10 = 1122,
  kFmiWindGustF0 = 1123,

  // New growth season parameters for Ilmanet

  kFmiGrowthPeriodPentadRelativeHumidityMean = 1124,
  kFmiGrowthPeriodPentadRelativeHumidityDeviation = 1125,
  kFmiGrowthPeriodPentadRelativeHumidityMax = 1126,
  kFmiGrowthPeriodPentadRelativeHumidityMin = 1127,
  kFmiGrowthPeriodPentadEvaporation = 1128,
  kFmiGrowthPeriodEvaporationSum = 1129,
  // GrowthPeriodStarted				DEFINED EARLIER
  // GrowthPeriodEnded				DEFINED EARLIER
  // EffectiveTemperatureSum			DEFINED EARLIER
  kFmiEffectiveTemperatureSumDeviation = 1130,
  // GrowthPeriodDeviationInDays		DEFINED EARLIER
  // GrowthPeriodDeviationInPrcnt		DEFINED EARLIER
  kFmiGrowthPeriodPentadTemperatureMean = 1131,
  kFmiGrowthPeriodPentadTemperatureMeanDeviation = 1132,
  kFmiGrowthPeriodPentadTemperatureMax = 1133,
  kFmiGrowthPeriodPentadTemperatureMaxDeviation = 1134,
  kFmiGrowthPeriodPentadTemperatureMin = 1135,
  kFmiGrowthPeriodPentadTemperatureMinDeviation = 1136,
  kFmiGrowthPeriodPentadSurfaceTemperatureMin = 1137,
  kFmiGrowthPeriodPentadFrostNights = 1138,
  kFmiGrowthPeriodPentadPrecipitation = 1139,
  kFmiGrowthPeriodPentadPrecipitationDeviation = 1140,
  kFmiGrowthPeriodPentadPrecipitationDays01mm = 1141,
  kFmiGrowthPeriodPentadPrecipitationDays1mm = 1142,
  kFmiGrowthPeriodPrecipitationSum = 1143,
  kFmiGrowthPeriodPrecipitationSumDeviationInPrcnt = 1144,
  kFmiGrowthPeriodPentadSurfaceTemperatureMin025Perc = 1145,

  // Water velocity parameters (HBM)

  kFmiSeaWaterVelocityU = 1146,
  kFmiSeaWaterVelocityV = 1147,
  kFmiSeaWaterVelocityW = 1148,

  // Ice parameters from Helmi ice model

  kFmiIceVelocityU = 1149,
  kFmiIceVelocityV = 1150,
  kFmiRaftIceConcentration = 1151,
  kFmiRaftIceThickness = 1152,
  kFmiRidgeIceConcentration = 1153,
  kFmiMeanIceThickness = 1154,
  kFmiRunoff = 1155,
  kFmiWaterLevelIndex = 1156,

  // Corrected reflectivity adjusted to water/sleet/snow strength scale 0-N (rdbz animation)

  kFmiPrecipitationLevel = 1157,
  kFmiSnow1h = 1158,

  // Accumulated radiation parameters for open data

  kFmiRadiationGlobalAccumulation = 1159,
  kFmiRadiationLWAccumulation = 1160,
  kFmiRadiationNetSurfaceLWAccumulation = 1161,
  kFmiRadiationNetSurfaceSWAccumulation = 1162,
  kFmiRadiationDiffuseAccumulation = 1163,

  // SADIS extra parameters needed for Latvia

  kFmiClearAirTurbulence = 1164,              // grib: 260165
  kFmiHorizontalExtentOfCumulonimbus = 1165,  // grib: 260120
  kFmiInCloudTurbulence = 1166,               // grib: 260164

  // New precipitation parameters from Harmonie

  kFmiPrecipitationGraupel = 1167,      // accumulated graupel
  kFmiPrecipitationRateGraupel = 1168,  // graupel rate
  kFmiPrecipitationSolid = 1169,        // accumulated solid eli lumi+graupel
  kFmiPrecipitationRateSolid = 1170,    // solid rate eli lumi+graupel

  kFmiPrecipitationInstantWater = 1171,    // "hetkellinen" vesisade
  kFmiPrecipitationInstantSnow = 1172,     // "hetkellinen" lumisade
  kFmiPrecipitationInstantGraupel = 1173,  // "hetkellinen" graupel

  kFmiCloudTop = 1174,  // Onhan naita cloud toppeja aika paljon mutta kaikki aiemmat liittyy
                        // metareihin (?), tama on ennustettu
  kFmiFloodWarningIndex = 1175,

  kFmiHakeMessageType = 1176,

  // Dry and wet deposition 6h cumulative
  kFmiSurface6hAccumulatedNOxDryDeposition = 1180,
  kFmiSurface6hAccumulatedNOxWetDeposition,
  kFmiSurface6hAccumulatedNHxDryDeposition,
  kFmiSurface6hAccumulatedNHxWetDeposition,
  kFmiSurface6hAccumulatedSDryDeposition,
  kFmiSurface6hAccumulatedSWetDeposition,
  // Dry and wet deposition 24h cumulative
  kFmiSurface24hAccumulatedNOxDryDeposition = 1186,
  kFmiSurface24hAccumulatedNOxWetDeposition,
  kFmiSurface24hAccumulatedNHxDryDeposition,
  kFmiSurface24hAccumulatedNHxWetDeposition,
  kFmiSurface24hAccumulatedSDryDeposition,
  kFmiSurface24hAccumulatedSWetDeposition,

  kFmiAbsoluteHumidity = 1192,
  kFmiPrecipitationInstantSolid = 1193,

  // MyOcean NEMO parameters
  kFmiSeaDepth = 1194,                 // NetCDF: model_depth = sea_floor_depth_below_sea_level
  kFmiMixedLayerThickness = 1195,      // NetCDF: mlp = ocean_mixed_layer_thickness
  kFmiSeaSurfaceElevation = 1196,      // NetCDF: ssh = sea_surface_elevation
  kFmiSeasonalWaterLevelIndex = 1197,  // SYKE seasonal water level

  // Seuraavat 5 parametria Harmoniesta
  kFmiIcingWarningIndex = 1198,
  kFmiCloudIce = 1199,
  kFmiSnowMixingRatio = 1200,       // satavan lumen sekoitussuhde
  kFmiRainWaterMixingRatio = 1201,  // satavan veden sekoitussuhde
  kFmiGraupelMixingRatio = 1202,    // satavan lumirakeen sekoitussuhde

  kFmiOceanIcing = 1203,  // valtamerien parskejaataminen

  kFmiMoninObukhovLength = 1204,
  kFmiPrecipitationForm2 = 1205,
  kFmiPrecipitationForm3 = 1206,

  kFmiQNH,  // QNH = MSL paine (hPa) laskettuna ICAO:n maaritteleman standardi-ilmakehan avulla

  kFmiPrecipitationInstantTotal = 1208,  // "hetkellinen" kokonaissade, eli vesi+lumi+graupel
  kFmiClearAirTurbulence2,
  kFmiTotalColumnWater,  // Vertically integrated total water (vapour + cloud water + cloud ice) kg
                         // m-2

  // De-icing project
  kFmiDeIcingClass0Probability,  // no de-icing needed
  kFmiDeIcingClass1Probability,  // frost on plane, T=-3...+1 and T-Td<3.0
  kFmiDeIcingClass2Probability,  // weak/moderate snow/rain, vis>2km
  kFmiDeIcingClass3Probability,  // heavy snow/rain, freezing rain, vis<2km
  kFmiDeIcingClass4Probability,  // exceptionally string snow/rain, no deicing possible

  // TopLink -lentosääprojektin indekseja
  kFmiTopLinkIndex2 = 1216,  // Lumisateen ja jäätävän sateen intensiteetin sekä sateesta johtuvan
                             // näkyvyyden yhdistelmä
  kFmiTopLinkIndex3,         // Lumi/Räntäsateen ja näkyvyyden yhdistelmä

  // Ukkosen todennaikoisyys, toinen parametri koska 260 on aliparametrina ja sen tarkkuus on
  // rajattu
  kFmiProbabilityThunderstorm2 = 1218,

  // Sateen olomuodon todennakoisyyskia
  kFmiProbabilityOfPrecFormDrizzle = 1219,
  kFmiProbabilityOfPrecFormRain,
  kFmiProbabilityOfPrecFormSleet,
  kFmiProbabilityOfPrecFormSnow,
  kFmiProbabilityOfPrecFormFreezingDrizzle,
  kFmiProbabilityOfPrecFormFreezingRain,

  kFmiBoundaryLayerTurbulence =
      1225,  // https://wiki.fmi.fi/display/PROJEKTIT/Rajakerroksen+turbulenssi

  kFmiPotentialPrecipitationForm =
      1226,                // MAHDOLLINEN sateen olomuoto, lasketaan vaikka sadetta (RR) ei olisi
  kFmiSnowDensity = 1227,  // Snow density kg/m3

  // SILAM parameters

  kFmiAreaOfRisk = 1228,

  kFmiCloudBase2,        // Second parameter for cloud base height (post processed with himan)
  kFmiFrostLayerTop,     // Frost layer top measured from ground
  kFmiFrostLayerBottom,  // Frost layer bottom

  kFmiVisibility2 = 1232,
  kFmiCloudTop2,  // post-processed cloud top
  kFmiAverageWindSpeedMS,
  kFmiPotentialPrecipitationType = 1235,
  kFmiHumidityRelativeToIce,
  kFmiFrostPoint,
  kFmiAvailableEnsembleMemberCount,
  kFmiWindGust2,  // post-processed wind gust speed
  kFmiProbabilityOfSigWaveHeightLimit1,
  kFmiProbabilityOfSigWaveHeightLimit2,
  kFmiProbabilityOfSigWaveHeightLimit3,
  kFmiProbabilityOfSigWaveHeightLimit4,
  kFmiProbabilityOfConvectivePrecipitationLimit1,
  kFmiProbabilityOfConvectivePrecipitationLimit2,
  kFmiProbabilityOfConvectivePrecipitationLimit3,
  kFmiProbabilityOfConvectivePrecipitationLimit4,
  kFmiProbabilityOfConvectivePrecipitationLimit5,
  kFmiProbabilityOfConvectivePrecipitationLimit6,
  kFmiProbabilityOfPrecipitation3h03mm,  // > 0.3 mm
  kFmiProbabilityOfColdLimit5,           // T < -35

  kFmiDifferentialReflectivity = 1300,   // Opera ODIM ZDR = Logged Differential Reflectivity
  kFmiSpecificDifferentialPhase = 1301,  // Opera ODIM KDP = Specific Differential Phase
  kFmiDifferentialPhase = 1302,          // Opera ODIM PHIDP = Differential Phase
  kFmiSignalQualityIndex = 1303,         // Opera ODIM SQI = Signal Quality Index
  kFmiReflectivityCorrelation = 1304,    // Opera ODIM RHORV = Correlation between Zh and Zv

  // Add a bunch of extreme fractile parameters (on the high and low end of the distribution)

  kFmiTemperatureF99 = 4500,
  kFmiTemperatureF97_5,
  kFmiTemperatureF95,
  kFmiTemperatureF87_5,
  kFmiTemperatureF12_5,
  kFmiTemperatureF5,
  kFmiTemperatureF2_5,
  kFmiTemperatureF1,

  kFmiTotalCloudCoverF99 = 4508,
  kFmiTotalCloudCoverF97_5,
  kFmiTotalCloudCoverF95,
  kFmiTotalCloudCoverF87_5,
  kFmiTotalCloudCoverF12_5,
  kFmiTotalCloudCoverF5,
  kFmiTotalCloudCoverF2_5,
  kFmiTotalCloudCoverF1,

  kFmiPrecipitation3hF97_5 = 4516,
  kFmiPrecipitation3hF87_5,
  kFmiPrecipitation3hF12_5,
  kFmiPrecipitation3hF2_5,

  kFmiWindSpeedF99 = 4520,
  kFmiWindSpeedF97_5,
  kFmiWindSpeedF95,
  kFmiWindSpeedF87_5,
  kFmiWindSpeedF12_5,
  kFmiWindSpeedF5,
  kFmiWindSpeedF2_5,
  kFmiWindSpeedF1,

  kFmiWindGustF99 = 4528,
  kFmiWindGustF97_5,
  kFmiWindGustF95,
  kFmiWindGustF87_5,
  kFmiWindGustF12_5,
  kFmiWindGustF5,
  kFmiWindGustF2_5,
  kFmiWindGustF1,

  kFmiDewPointF100 = 4536,
  kFmiDewPointF99,
  kFmiDewPointF97_5,
  kFmiDewPointF95,
  kFmiDewPointF87_5,
  kFmiDewPointF50,
  kFmiDewPointF12_5,
  kFmiDewPointF5,
  kFmiDewPointF2_5,
  kFmiDewPointF1,
  kFmiDewPointF0,

  kFmiCAPEF100 = 4547,
  kFmiCAPEF99,
  kFmiCAPEF97_5,
  kFmiCAPEF95,
  kFmiCAPEF87_5,
  kFmiCAPEF50,
  kFmiCAPEF12_5,
  kFmiCAPEF5,
  kFmiCAPEF2_5,
  kFmiCAPEF1,
  kFmiCAPEF0,

  kFmiIceCoverF100 = 4558,
  kFmiIceCoverF99,
  kFmiIceCoverF97_5,
  kFmiIceCoverF95,
  kFmiIceCoverF87_5,
  kFmiIceCoverF50,
  kFmiIceCoverF12_5,
  kFmiIceCoverF5,
  kFmiIceCoverF2_5,
  kFmiIceCoverF1,
  kFmiIceCoverF0,

  kFmiSnowDepthF100 = 4569,
  kFmiSnowDepthF99,
  kFmiSnowDepthF97_5,
  kFmiSnowDepthF95,
  kFmiSnowDepthF87_5,
  kFmiSnowDepthF50,
  kFmiSnowDepthF12_5,
  kFmiSnowDepthF5,
  kFmiSnowDepthF2_5,
  kFmiSnowDepthF1,
  kFmiSnowDepthF0,

  kFmiMaximumTemperatureF100 = 4580,
  kFmiMaximumTemperatureF99,
  kFmiMaximumTemperatureF97_5,
  kFmiMaximumTemperatureF95,
  kFmiMaximumTemperatureF87_5,
  kFmiMaximumTemperatureF50,
  kFmiMaximumTemperatureF12_5,
  kFmiMaximumTemperatureF5,
  kFmiMaximumTemperatureF2_5,
  kFmiMaximumTemperatureF1,
  kFmiMaximumTemperatureF0,

  kFmiMinimumTemperatureF100 = 4591,
  kFmiMinimumTemperatureF99,
  kFmiMinimumTemperatureF97_5,
  kFmiMinimumTemperatureF95,
  kFmiMinimumTemperatureF87_5,
  kFmiMinimumTemperatureF50,
  kFmiMinimumTemperatureF12_5,
  kFmiMinimumTemperatureF5,
  kFmiMinimumTemperatureF2_5,
  kFmiMinimumTemperatureF1,
  kFmiMinimumTemperatureF0,

  kFmiTemperatureSeaF100 = 4602,
  kFmiTemperatureSeaF99,
  kFmiTemperatureSeaF97_5,
  kFmiTemperatureSeaF95,
  kFmiTemperatureSeaF87_5,
  kFmiTemperatureSeaF50,
  kFmiTemperatureSeaF12_5,
  kFmiTemperatureSeaF5,
  kFmiTemperatureSeaF2_5,
  kFmiTemperatureSeaF1,
  kFmiTemperatureSeaF0,

  kFmiPressureF100 = 4613,
  kFmiPressureF99,
  kFmiPressureF97_5,
  kFmiPressureF95,
  kFmiPressureF87_5,
  kFmiPressureF50,
  kFmiPressureF12_5,
  kFmiPressureF5,
  kFmiPressureF2_5,
  kFmiPressureF1,
  kFmiPressureF0,

  kFmiTotalColumnWaterF100 = 4624,
  kFmiTotalColumnWaterF99,
  kFmiTotalColumnWaterF97_5,
  kFmiTotalColumnWaterF95,
  kFmiTotalColumnWaterF87_5,
  kFmiTotalColumnWaterF50,
  kFmiTotalColumnWaterF12_5,
  kFmiTotalColumnWaterF5,
  kFmiTotalColumnWaterF2_5,
  kFmiTotalColumnWaterF1,
  kFmiTotalColumnWaterF0,

  kFmiSnowAccumulationF100 = 4635,
  kFmiSnowAccumulationF99,
  kFmiSnowAccumulationF97_5,
  kFmiSnowAccumulationF95,
  kFmiSnowAccumulationF87_5,
  kFmiSnowAccumulationF50,
  kFmiSnowAccumulationF12_5,
  kFmiSnowAccumulationF5,
  kFmiSnowAccumulationF2_5,
  kFmiSnowAccumulationF1,
  kFmiSnowAccumulationF0,

  kFmiGeopHeightF100 = 4646,
  kFmiGeopHeightF99,
  kFmiGeopHeightF97_5,
  kFmiGeopHeightF95,
  kFmiGeopHeightF87_5,
  kFmiGeopHeightF50,
  kFmiGeopHeightF12_5,
  kFmiGeopHeightF5,
  kFmiGeopHeightF2_5,
  kFmiGeopHeightF1,
  kFmiGeopHeightF0,

  kFmiSpecificHumidityF100 = 4657,
  kFmiSpecificHumidityF99,
  kFmiSpecificHumidityF97_5,
  kFmiSpecificHumidityF95,
  kFmiSpecificHumidityF87_5,
  kFmiSpecificHumidityF50,
  kFmiSpecificHumidityF12_5,
  kFmiSpecificHumidityF5,
  kFmiSpecificHumidityF2_5,
  kFmiSpecificHumidityF1,
  kFmiSpecificHumidityF0,

  kFmiPressureF90 = 4668,
  kFmiPressureF75,
  kFmiPressureF25,
  kFmiPressureF10,

  /* START SOUNDING INDEX PARAMETERS */

  /*
   *
   * The following parameters are widely used with sounding index data but were not originally
   * included in this file.
   *
   * To make the data more easily manipulated with qd-tools, the parameters are added with
   * their original numbers.
   */

  // aluksi surface (sur) arvojen avulla lasketut parametrit
  kFmiSoundingLCLSur = 4720,
  kFmiSoundingLFCSur,
  kFmiSoundingELSur,
  kFmiSoundingCAPESur,
  kFmiSoundingCAPE0_3kmSur,
  kFmiSoundingCINSur,
  kFmiSoundingLCLHeightSur,
  kFmiSoundingLFCHeightSur,
  kFmiSoundingELHeightSur,
  kFmiSoundingCAPE_TT_Sur,  // cape -10 ja -40 asteen kerroksen lapi
  // sitten 500m arvojen avulla lasketut parametrit
  kFmiSoundingLCL500m = 4730,
  kFmiSoundingLFC500m,
  kFmiSoundingEL500m,
  kFmiSoundingCAPE500m,
  kFmiSoundingCAPE0_3km500m,
  kFmiSoundingCIN500m,
  kFmiSoundingLCLHeight500m,
  kFmiSoundingLFCHeight500m,
  kFmiSoundingELHeight500m,
  kFmiSoundingCAPE_TT_500m,  // cape -10 ja -40 asteen kerroksen lapi
  // sitten mostunstable (MostUn) arvojen avulla lasketut parametrit
  kFmiSoundingLCLMostUn = 4740,
  kFmiSoundingLFCMostUn,
  kFmiSoundingELMostUn,
  kFmiSoundingCAPEMostUn,
  kFmiSoundingCAPE0_3kmMostUn,
  kFmiSoundingCINMostUn,
  kFmiSoundingLCLHeightMostUn,
  kFmiSoundingLFCHeightMostUn,
  kFmiSoundingELHeightMostUn,
  kFmiSoundingCAPE_TT_MostUn,  // cape -10 ja -40 asteen kerroksen lapi
  // sitten normaaleja yleis indeksejä
  kFmiSoundingSHOW = 4750,
  kFmiSoundingLIFT,
  kFmiSoundingKINX,
  kFmiSoundingCTOT,
  kFmiSoundingVTOT,
  kFmiSoundingTOTL,
  // sitten vielä 'speciaali' paramereja
  kFmiSoundingBS0_6km = 4770,  // BS=bulkshear
  kFmiSoundingBS0_1km,
  kFmiSoundingSRH0_3km,  // SRH=storm related helicity
  kFmiSoundingSRH0_1km,
  kFmiSoundingWS1500m,
  kFmiSoundingThetaE0_3km,

  /* STOP SOUNDING INDEX PARAMETERS */

  /* Clim4Energy parameters. Names close matches to grib_api names, apart from the last two not to
     deviate from old names  */

  kFmiHighVegetationCover = 4800,
  kFmiSkinReservoirContent,
  kFmiSoilTemperatureLevel1,
  kFmiSoilTemperatureLevel2,
  kFmiSoilTemperatureLevel3,
  kFmiSoilTemperatureLevel4,
  kFmiVolumetricSoilWaterLayer1,
  kFmiVolumetricSoilWaterLayer2,
  kFmiVolumetricSoilWaterLayer3,
  kFmiVolumetricSoilWaterLayer4,
  kFmiHighVegetationType,  // grib_api: TypeOfHighVegetation
  kFmiLowVegetationType,   // grib_api: TypeOfLowVegetation

  // WindMill icing forecasts (Karoliina Hämäläinen, SOL-4545)
  kFmiIcingRate = 4820,
  kFmiIceMass = 4821,

  kFmiCOConcentration = 4900,
  kFmiNOConcentration,
  kFmiNO2Concentration,
  kFmiO3Concentration,
  kFmiPM10Concentration,
  kFmiPM25Concentration,
  kFmiSO2Concentration,

  // Some value larger than any other enum value
  kFmiLastParameter = 6000

};

// ----------------------------------------------------------------------
/*
 * g++ may complain about choosing unsigned int over int if there is
 * no explicit cast for enumerated values. Defining the output operator
 * makes the problem go away.
 */
// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& out, FmiParameterName p)
{
  out << static_cast<unsigned long>(p);
  return out;
}

// ======================================================================
