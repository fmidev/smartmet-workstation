// ======================================================================
/*!
 * \file NFmiMetEditorTypes.h
 * \brief NFmiInfoData namespace declarations
 *
 */
// ======================================================================

#pragma once

namespace NFmiMetEditorTypes
{
typedef enum {
  kFmiTextView = 1,
  kFmiIsoLineView = 2,
  kFmiColorContourView = 3,
  kFmiColorContourIsoLineView = 4,
  kFmiQuickColorContourView = 5,
  kFmiSymbolView = 6,
  kFmiIndexedTextView = 7,
  kFmiFontTextView = 8,
  kFmiBitmapView = 9,
  kFmiParamsDefaultView = 10,
  kFmiPrecipFormSymbolView = 11,
  kFmiSynopWeatherSymbolView = 12,
  kFmiRawMirriFontSymbolView = 13,
  kFmiBetterWeatherSymbolView = 14,
  kFmiSmartSymbolView = 15
} View;

typedef enum {
  kFmiNoMask = 1,
  kFmiSelectionMask = 4,
  kFmiDisplayedMask = 16,
} Mask;

enum FmiUsedSmartMetTool
{
  kFmiNoToolSelected = 0,          // default tyhjä arvo
  kFmiDataModificationTool = 1,    // muokkausdialogi
  kFmiBrush = 2,                   // pensseli
  kFmiTimeSerialModification = 3,  // aikasarja editointi
  kFmiSmarttool = 4,               // smarttool kieli
  kFmiDataLoading = 5              // datan lataus
};

}  // end of namespace NFmiMetEditorTypes
