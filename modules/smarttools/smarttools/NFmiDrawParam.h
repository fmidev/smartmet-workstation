//**********************************************************
// C++ Class Name : NFmiDrawParam
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiDrawParam.h
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 2
//  - GD View Type      : Class Diagram
//  - GD View Name      : Markon ehdotus
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Thur - Jan 28, 1999
//
//
//  Description:
//   En ole vielä varma tämän luokan tarkoituksesta/toiminnasta,
//   tämä on
//   Persan idea.
//
//  Change Log:
// Changed 1999.09.28/Marko	Lisäsin SecondaryIsoLineColor:in käytön (käytetään
//							vertailtaessa samantyyppistä dataa
// päällekkäin)
//
//**********************************************************
#pragma once

#include "NFmiColor.h"
#include "NFmiMetEditorTypes.h"

#include <newbase/NFmiParameterName.h>
#include <newbase/NFmiDataIdent.h>
#include <newbase/NFmiPoint.h>
#include <newbase/NFmiLevel.h>
#include <newbase/NFmiInfoData.h>
#include <newbase/NFmiDataMatrix.h>  // täältä tulee myös checkedVector
#include <newbase/NFmiMetTime.h>
#include <boost/shared_ptr.hpp>

class NFmiDrawingEnvironment;

class NFmiDrawParam
{
 public:
  NFmiDrawParam(void);
  NFmiDrawParam(const NFmiDataIdent& theParam,
                const NFmiLevel& theLevel,
                int thePriority,
                NFmiInfoData::Type theDataType =
                    NFmiInfoData::kNoDataType);  //, NFmiMetEditorCoordinatorMapOptions*
  // theMetEditorCoordinatorMapOptions=0);
  NFmiDrawParam(const NFmiDrawParam& other);
  virtual ~NFmiDrawParam(void);

  void Init(const NFmiDrawParam* theDrawParam, bool fInitOnlyDrawingOptions = false);
  void Init(const boost::shared_ptr<NFmiDrawParam>& theDrawParam,
            bool fInitOnlyDrawingOptions = false);
  void HideParam(bool newValue) { fHidden = newValue; };
  void EditParam(bool newValue) { fEditedParam = newValue; };
  bool IsParamHidden(void) const { return fHidden; };
  bool IsParamEdited(void) const { return fEditedParam; };
  const NFmiMetTime& ModelOriginTime(void) const { return itsModelOriginTime; }
  void ModelOriginTime(const NFmiMetTime& newValue) { itsModelOriginTime = newValue; }
  int ModelRunIndex(void) const { return itsModelRunIndex; }
  void ModelRunIndex(int newValue) { itsModelRunIndex = newValue; }
  bool UseArchiveModelData(void) const;
  bool IsModelRunDataType(void) const;
  static bool IsModelRunDataType(NFmiInfoData::Type theDataType);
  const NFmiMetTime& ModelOriginTimeCalculated(void) const { return itsModelOriginTimeCalculated; }
  void ModelOriginTimeCalculated(const NFmiMetTime& newValue)
  {
    itsModelOriginTimeCalculated = newValue;
  }
  int TimeSerialModelRunCount(void) const { return itsTimeSerialModelRunCount; }
  void TimeSerialModelRunCount(int newValue)
  {
    itsTimeSerialModelRunCount = newValue;
    if (itsTimeSerialModelRunCount < 0)
      itsTimeSerialModelRunCount = 0;
  }

  NFmiInfoData::Type DataType(void) const { return itsDataType; }
  // HUOM! tämä asettaa vain itsDataType-dataosan arvon, ei mahdollista itsInfon data tyyppiä!!!!!!
  void DataType(NFmiInfoData::Type newValue) { itsDataType = newValue; };
  bool Init(const std::string& theFilename);
  bool StoreData(const std::string& theFilename);

  // --------------- "set" ja "get" metodit -----------------
  const std::string& ParameterAbbreviation(void) const;
  void ParameterAbbreviation(const std::string &theParameterAbbreviation)
  {
    itsParameterAbbreviation = theParameterAbbreviation;
  }
  NFmiDataIdent& Param(void) { return itsParameter; };
  void Param(const NFmiDataIdent& theParameter) { itsParameter = theParameter; };
  NFmiLevel& Level(void) { return itsLevel; }
  void Level(const NFmiLevel& theLevel) { itsLevel = theLevel; }
  void Priority(int thePriority) { itsPriority = thePriority; };
  int Priority(void) const { return itsPriority; };
  void ViewType(const NFmiMetEditorTypes::View& theViewType) { itsViewType = theViewType; };
  NFmiMetEditorTypes::View ViewType(void) const { return itsViewType; };
  void FrameColor(const NFmiColor& theFrameColor) { itsFrameColor = theFrameColor; };
  const NFmiColor& FrameColor(void) const { return itsFrameColor; };
  void FillColor(const NFmiColor& theFillColor) { itsFillColor = theFillColor; };
  const NFmiColor& FillColor(void) const { return itsFillColor; };
  void IsolineLabelBoxFillColor(const NFmiColor& theColor)
  {
    itsIsolineLabelBoxFillColor = theColor;
    itsContourLabelBoxFillColor =
        theColor;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  };
  const NFmiColor& IsolineLabelBoxFillColor(void) const { return itsIsolineLabelBoxFillColor; };
  void ContourLabelBoxFillColor(const NFmiColor& theColor)
  {
    itsContourLabelBoxFillColor = theColor;
  };
  const NFmiColor& ContourLabelBoxFillColor(void) const { return itsContourLabelBoxFillColor; };
  void OnlyOneSymbolRelativeSize(const NFmiPoint& theOnlyOneSymbolRelativeSize)
  {
    itsOnlyOneSymbolRelativeSize = theOnlyOneSymbolRelativeSize;
  };
  const NFmiPoint& OnlyOneSymbolRelativeSize(void) const { return itsOnlyOneSymbolRelativeSize; };
  void OnlyOneSymbolRelativePositionOffset(const NFmiPoint& theOnlyOneSymbolRelativePositionOffset)
  {
    itsOnlyOneSymbolRelativePositionOffset = theOnlyOneSymbolRelativePositionOffset;
  };
  const NFmiPoint& OnlyOneSymbolRelativePositionOffset(void) const
  {
    return itsOnlyOneSymbolRelativePositionOffset;
  };
  void UseIsoLineGabWithCustomContours(const bool newState)
  {
    fUseIsoLineGabWithCustomContours = newState;
    fUseContourGabWithCustomContours =
        newState;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  };
  bool UseIsoLineGabWithCustomContours(void) const { return fUseIsoLineGabWithCustomContours; };
  void UseContourGabWithCustomContours(const bool newState)
  {
    fUseContourGabWithCustomContours = newState;
  };
  bool UseContourGabWithCustomContours(void) const { return fUseContourGabWithCustomContours; };
  void IsoLineGab(const double theIsoLineGab)
  {
    itsIsoLineGab = theIsoLineGab;
    if (itsIsoLineGab == 0)
      itsIsoLineGab = 1;  // gappi ei voi olla 0
  }
  double IsoLineGab(void) const { return itsIsoLineGab; };
  void ContourGab(const double theContourGab)
  {
    itsContourGab = theContourGab;
  }
  double ContourGab(void) const { return itsContourGab; };
  void ModifyingStep(const double theModifyingStep) { itsModifyingStep = theModifyingStep; };
  double ModifyingStep(void) const { return itsModifyingStep; };
  //	void				 ModifyingUnit (bool theModifyingUnit) { fModifyingUnit =
  // theModifyingUnit; }
  //	bool			 ModifyingUnit (void) const { return fModifyingUnit; }
  const NFmiMetEditorTypes::View* PossibleViewTypeList(void) const
  {
    return itsPossibleViewTypeList;
  }
  int PossibleViewTypeCount(void) const { return itsPossibleViewTypeCount; };
  const std::string& InitFileName(void) const { return itsInitFileName; }
  void InitFileName(std::string theFileName) { itsInitFileName = theFileName; }
  double AbsoluteMinValue(void) const { return itsAbsoluteMinValue; }
  void AbsoluteMinValue(double theAbsoluteMinValue) { itsAbsoluteMinValue = theAbsoluteMinValue; }
  double AbsoluteMaxValue(void) const { return itsAbsoluteMaxValue; }
  void AbsoluteMaxValue(double theAbsoluteMaxValue) { itsAbsoluteMaxValue = theAbsoluteMaxValue; }
  double TimeSeriesScaleMin(void) const { return itsTimeSeriesScaleMin; };
  double TimeSeriesScaleMax(void) const { return itsTimeSeriesScaleMax; };
  void TimeSeriesScaleMin(double theValue) { itsTimeSeriesScaleMin = theValue; };
  void TimeSeriesScaleMax(double theValue) { itsTimeSeriesScaleMax = theValue; };
  const NFmiColor& IsolineColor(void) const
  {
    return itsIsolineColor;
  };
  void IsolineColor(const NFmiColor& newColor)
  {
    itsIsolineColor = newColor;
    itsContourColor = newColor;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  const NFmiColor& ContourColor(void) const { return itsContourColor; };
  void ContourColor(const NFmiColor& newColor) { itsContourColor = newColor; };
  void IsolineTextColor(const NFmiColor& newColor)
  {
    itsIsolineTextColor = newColor;
    // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
    itsContourTextColor = newColor;  
  }
  const NFmiColor& IsolineTextColor(void) const
  {
    return itsIsolineTextColor;
  };

  void ContourTextColor(const NFmiColor& newColor) { itsContourTextColor = newColor; };
  const NFmiColor& ContourTextColor(void) const { return itsContourTextColor; };
  //	double TimeSerialModifyingLimit(void) const {return fModifyingUnit ?
  // itsTimeSerialModifyingLimit : 100;};
  double TimeSerialModifyingLimit(void) const { return itsTimeSerialModifyingLimit; };
  NFmiMetEditorTypes::View StationDataViewType(void) const { return itsStationDataViewType; };
  void TimeSerialModifyingLimit(double newValue) { itsTimeSerialModifyingLimit = newValue; };
  void StationDataViewType(NFmiMetEditorTypes::View newValue)
  {
    itsStationDataViewType = newValue;
  };

  void FileVersionNumber(const float theFileVersionNumber)
  {
    itsFileVersionNumber = theFileVersionNumber;
  };
  float FileVersionNumber(void) const { return itsFileVersionNumber; };
  void Unit(const std::string& theUnit) { itsUnit = theUnit; };
  const std::string& Unit(void) const { return itsUnit; };
  bool ShowNumbers(void) const { return fShowNumbers; }
  void ShowNumbers(bool theValue) { fShowNumbers = theValue; }
  bool ShowColors(void) const { return fShowColors; }
  void ShowColors(bool theValue) { fShowColors = theValue; }
  bool ShowColoredNumbers(void) const { return fShowColoredNumbers; }
  void ShowColoredNumbers(bool theValue) { fShowColoredNumbers = theValue; }
  bool ZeroColorMean(void) const { return fZeroColorMean; }
  void ZeroColorMean(bool theValue) { fZeroColorMean = theValue; }
  bool IsActive(void) const { return fActive; };
  void Activate(bool newState) { fActive = newState; };
  bool ShowDifferenceToOriginalData(void) const { return fShowDifferenceToOriginalData; }
  void ShowDifferenceToOriginalData(bool newValue) { fShowDifferenceToOriginalData = newValue; }
  //**************************************************************
  //********** 'versio 2' parametrien asetusfunktiot *************
  //**************************************************************
  float StationSymbolColorShadeLowValue(void) const { return itsStationSymbolColorShadeLowValue; }
  void StationSymbolColorShadeLowValue(float newValue)
  {
    itsStationSymbolColorShadeLowValue = newValue;
  }
  float StationSymbolColorShadeMidValue(void) const { return itsStationSymbolColorShadeMidValue; }
  void StationSymbolColorShadeMidValue(float newValue)
  {
    itsStationSymbolColorShadeMidValue = newValue;
  }
  float StationSymbolColorShadeHighValue(void) const { return itsStationSymbolColorShadeHighValue; }
  void StationSymbolColorShadeHighValue(float newValue)
  {
    itsStationSymbolColorShadeHighValue = newValue;
  }
  const NFmiColor& StationSymbolColorShadeLowValueColor(void) const
  {
    return itsStationSymbolColorShadeLowValueColor;
  }
  void StationSymbolColorShadeLowValueColor(const NFmiColor& newValue)
  {
    itsStationSymbolColorShadeLowValueColor = newValue;
  }
  const NFmiColor& StationSymbolColorShadeMidValueColor(void) const
  {
    return itsStationSymbolColorShadeMidValueColor;
  }
  void StationSymbolColorShadeMidValueColor(const NFmiColor& newValue)
  {
    itsStationSymbolColorShadeMidValueColor = newValue;
  }
  const NFmiColor& StationSymbolColorShadeHighValueColor(void) const
  {
    return itsStationSymbolColorShadeHighValueColor;
  }
  void StationSymbolColorShadeHighValueColor(const NFmiColor& newValue)
  {
    itsStationSymbolColorShadeHighValueColor = newValue;
  }
  int StationSymbolColorShadeClassCount(void) const { return itsStationSymbolColorShadeClassCount; }
  void StationSymbolColorShadeClassCount(int newValue)
  {
    itsStationSymbolColorShadeClassCount = newValue;
  }
  bool UseSymbolsInTextMode(void) const { return fUseSymbolsInTextMode; }
  void UseSymbolsInTextMode(bool newValue) { fUseSymbolsInTextMode = newValue; }
  int UsedSymbolListIndex(void) const { return itsUsedSymbolListIndex; }
  void UsedSymbolListIndex(int newValue) { itsUsedSymbolListIndex = newValue; }
  int SymbolIndexingMapListIndex(void) const { return itsSymbolIndexingMapListIndex; }
  void SymbolIndexingMapListIndex(int newValue) { itsSymbolIndexingMapListIndex = newValue; }
  NFmiMetEditorTypes::View GridDataPresentationStyle(void) const { return itsGridDataPresentationStyle; }
  void GridDataPresentationStyle(NFmiMetEditorTypes::View newValue) { itsGridDataPresentationStyle = newValue; }
  bool UseIsoLineFeathering(void) const { return fUseIsoLineFeathering; }
  void UseIsoLineFeathering(bool newValue)
  {
    fUseIsoLineFeathering = newValue;
    fUseContourFeathering =
        newValue;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  bool UseContourFeathering(void) const { return fUseContourFeathering; }
  void UseContourFeathering(bool newValue) { fUseContourFeathering = newValue; }
  bool IsoLineLabelsOverLapping(void) const { return fIsoLineLabelsOverLapping; }
  void IsoLineLabelsOverLapping(bool newValue) { fIsoLineLabelsOverLapping = newValue; }
  bool ShowColorLegend(void) const { return fShowColorLegend; }
  void ShowColorLegend(bool newValue) { fShowColorLegend = newValue; }
  bool UseSimpleIsoLineDefinitions(void) const { return fUseSimpleIsoLineDefinitions; }
  void UseSimpleIsoLineDefinitions(bool newValue)
  {
    fUseSimpleIsoLineDefinitions = newValue;
    fUseSimpleContourDefinitions =
        newValue;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  bool UseSimpleContourDefinitions(void) const { return fUseSimpleContourDefinitions; }
  void UseSimpleContourDefinitions(bool newValue) { fUseSimpleContourDefinitions = newValue; }
  bool UseSeparatorLinesBetweenColorContourClasses(void) const
  {
    return fUseSeparatorLinesBetweenColorContourClasses;
  }
  void UseSeparatorLinesBetweenColorContourClasses(bool newValue)
  {
    fUseSeparatorLinesBetweenColorContourClasses = newValue;
  }
  float SimpleIsoLineGap(void) const { return itsSimpleIsoLineGap; }
  void SimpleIsoLineGap(float newValue) { itsSimpleIsoLineGap = newValue; }
  float SimpleIsoLineZeroValue(void) const { return itsSimpleIsoLineZeroValue; }
  void SimpleIsoLineZeroValue(float newValue)
  {
    itsSimpleIsoLineZeroValue = newValue;
    itsSimpleContourZeroValue =
        newValue;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  float SimpleContourZeroValue(void) const { return itsSimpleContourZeroValue; }
  void SimpleContourZeroValue(float newValue) { itsSimpleContourZeroValue = newValue; }
  float SimpleIsoLineLabelHeight(void) const { return itsSimpleIsoLineLabelHeight; }
  void SimpleIsoLineLabelHeight(float newValue)
  {
    itsSimpleIsoLineLabelHeight = newValue;
    itsSimpleContourLabelHeight =
        newValue;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  float SimpleContourLabelHeight(void) const { return itsSimpleContourLabelHeight; }
  void SimpleContourLabelHeight(float newValue) { itsSimpleContourLabelHeight = newValue; }
  bool ShowSimpleIsoLineLabelBox(void) const { return fShowSimpleIsoLineLabelBox; }
  void ShowSimpleIsoLineLabelBox(bool newValue)
  {
    fShowSimpleIsoLineLabelBox = newValue;
    fShowSimpleContourLabelBox = newValue;
  }

  bool ShowSimpleContourLabelBox(void) const { return fShowSimpleContourLabelBox; }
  void ShowSimpleContourLabelBox(bool newValue) { fShowSimpleContourLabelBox = newValue; }
  float SimpleIsoLineWidth(void) const { return itsSimpleIsoLineWidth; }
  void SimpleIsoLineWidth(float newValue)
  {
    itsSimpleIsoLineWidth = newValue;
    itsSimpleContourWidth = newValue;
  }

  float SimpleContourWidth(void) const { return itsSimpleContourWidth; }
  void SimpleContourWidth(float newValue) { itsSimpleContourWidth = newValue; }
  int SimpleIsoLineLineStyle(void) const { return itsSimpleIsoLineLineStyle; }
  void SimpleIsoLineLineStyle(int newValue)
  {
    itsSimpleIsoLineLineStyle = newValue;
    itsSimpleContourLineStyle = newValue;
  }

  int SimpleContourLineStyle(void) const { return itsSimpleContourLineStyle; }
  void SimpleContourLineStyle(int newValue) { itsSimpleContourLineStyle = newValue; }
  float IsoLineSplineSmoothingFactor(void) const { return itsIsoLineSplineSmoothingFactor; }
  void IsoLineSplineSmoothingFactor(float newValue) { itsIsoLineSplineSmoothingFactor = newValue; }
  bool UseSingleColorsWithSimpleIsoLines(void) const { return fUseSingleColorsWithSimpleIsoLines; }
  void UseSingleColorsWithSimpleIsoLines(bool newValue)
  {
    fUseSingleColorsWithSimpleIsoLines = newValue;
  }
  float SimpleIsoLineColorShadeLowValue(void) const { return itsSimpleIsoLineColorShadeLowValue; }
  void SimpleIsoLineColorShadeLowValue(float newValue)
  {
    itsSimpleIsoLineColorShadeLowValue = newValue;
  }
  float SimpleIsoLineColorShadeMidValue(void) const { return itsSimpleIsoLineColorShadeMidValue; }
  void SimpleIsoLineColorShadeMidValue(float newValue)
  {
    itsSimpleIsoLineColorShadeMidValue = newValue;
  }
  float SimpleIsoLineColorShadeHighValue(void) const { return itsSimpleIsoLineColorShadeHighValue; }
  void SimpleIsoLineColorShadeHighValue(float newValue)
  {
    itsSimpleIsoLineColorShadeHighValue = newValue;
    itsSimpleIsoLineColorShadeHigh2Value = newValue;
  }

  float SimpleIsoLineColorShadeHigh2Value(void) const
  {
    return itsSimpleIsoLineColorShadeHigh2Value;
  }
  void SimpleIsoLineColorShadeHigh2Value(float newValue)
  {
    itsSimpleIsoLineColorShadeHigh2Value = newValue;
  }

  const NFmiColor& SimpleIsoLineColorShadeLowValueColor(void) const
  {
    return itsSimpleIsoLineColorShadeLowValueColor;
  }
  void SimpleIsoLineColorShadeLowValueColor(const NFmiColor& newValue)
  {
    itsSimpleIsoLineColorShadeLowValueColor = newValue;
  }
  const NFmiColor& SimpleIsoLineColorShadeMidValueColor(void) const
  {
    return itsSimpleIsoLineColorShadeMidValueColor;
  }
  void SimpleIsoLineColorShadeMidValueColor(const NFmiColor& newValue)
  {
    itsSimpleIsoLineColorShadeMidValueColor = newValue;
  }
  const NFmiColor& SimpleIsoLineColorShadeHighValueColor(void) const
  {
    return itsSimpleIsoLineColorShadeHighValueColor;
  }
  void SimpleIsoLineColorShadeHighValueColor(const NFmiColor& newValue)
  {
    itsSimpleIsoLineColorShadeHighValueColor = newValue;
    itsSimpleIsoLineColorShadeHigh2ValueColor = newValue;
  }

  const NFmiColor& SimpleIsoLineColorShadeHigh2ValueColor(void) const
  {
    return itsSimpleIsoLineColorShadeHigh2ValueColor;
  }
  void SimpleIsoLineColorShadeHigh2ValueColor(const NFmiColor& newValue)
  {
    itsSimpleIsoLineColorShadeHigh2ValueColor = newValue;
  }

  int SimpleIsoLineColorShadeClassCount(void) const { return itsSimpleIsoLineColorShadeClassCount; }
  void SimpleIsoLineColorShadeClassCount(int newValue)
  {
    itsSimpleIsoLineColorShadeClassCount = newValue;
  }
  const checkedVector<float>& SpecialIsoLineValues(void) const { return itsSpecialIsoLineValues; }
  void SetSpecialIsoLineValues(const checkedVector<float>& newValue)
  {
    itsSpecialIsoLineValues = newValue;
    itsSpecialContourValues = newValue;
  }

  const checkedVector<float>& SpecialContourValues(void) const { return itsSpecialContourValues; }
  void SetSpecialContourValues(const checkedVector<float>& newValue)
  {
    itsSpecialContourValues = newValue;
  }

  const checkedVector<float>& SpecialIsoLineLabelHeight(void) const
  {
    return itsSpecialIsoLineLabelHeight;
  }
  void SetSpecialIsoLineLabelHeight(const checkedVector<float>& newValue)
  {
    itsSpecialIsoLineLabelHeight = newValue;
    itsSpecialContourLabelHeight = newValue;
  }

  const checkedVector<float>& SpecialContourLabelHeight(void) const
  {
    return itsSpecialContourLabelHeight;
  }
  void SetSpecialContourLabelHeight(const checkedVector<float>& newValue)
  {
    itsSpecialContourLabelHeight = newValue;
  }

  const checkedVector<float>& SpecialIsoLineWidth(void) const { return itsSpecialIsoLineWidth; }
  void SetSpecialIsoLineWidth(const checkedVector<float>& newValue)
  {
    itsSpecialIsoLineWidth = newValue;
    itsSpecialContourWidth = newValue;
  }

  const checkedVector<float>& SpecialcontourWidth(void) const { return itsSpecialContourWidth; }
  void SetSpecialcontourWidth(const checkedVector<float>& newValue)
  {
    itsSpecialContourWidth = newValue;
  }

  const checkedVector<int>& SpecialIsoLineStyle(void) const { return itsSpecialIsoLineStyle; }
  void SetSpecialIsoLineStyle(const checkedVector<int>& newValue)
  {
    itsSpecialIsoLineStyle = newValue;
    itsSpecialContourStyle = newValue;
  }

  const checkedVector<int>& SpecialContourStyle(void) const { return itsSpecialContourStyle; }
  void SetSpecialContourStyle(checkedVector<int>& newValue) { itsSpecialContourStyle = newValue; }
  const checkedVector<int>& SpecialIsoLineColorIndexies(void) const
  {
    return itsSpecialIsoLineColorIndexies;
  }
  void SetSpecialIsoLineColorIndexies(const checkedVector<int>& newValue)
  {
    itsSpecialIsoLineColorIndexies = newValue;
    itsSpecialContourColorIndexies = newValue;
  }

  const checkedVector<int>& SpecialContourColorIndexies(void) const
  {
    return itsSpecialContourColorIndexies;
  }
  void SetSpecialContourColorIndexies(const checkedVector<int>& newValue)
  {
    itsSpecialContourColorIndexies = newValue;
  }

  const checkedVector<bool>& SpecialIsoLineShowLabelBox(void) const
  {
    return itsSpecialIsoLineShowLabelBox;
  }
  void SpecialIsoLineShowLabelBox(checkedVector<bool>& newValue)
  {
    itsSpecialIsoLineShowLabelBox = newValue;
  }
  bool DrawOnlyOverMask(void) const { return fDrawOnlyOverMask; }
  void DrawOnlyOverMask(bool newValue) { fDrawOnlyOverMask = newValue; }

  float ColorContouringColorShadeLowValue(void) const
  {
    return itsColorContouringColorShadeLowValue;
  }
  void ColorContouringColorShadeLowValue(float newValue)
  {
    itsColorContouringColorShadeLowValue = newValue;
  }
  float ColorContouringColorShadeMidValue(void) const
  {
    return itsColorContouringColorShadeMidValue;
  }
  void ColorContouringColorShadeMidValue(float newValue)
  {
    itsColorContouringColorShadeMidValue = newValue;
  }
  float ColorContouringColorShadeHighValue(void) const
  {
    return itsColorContouringColorShadeHighValue;
  }
  void ColorContouringColorShadeHighValue(float newValue)
  {
    itsColorContouringColorShadeHighValue = newValue;
  }
  float ColorContouringColorShadeHigh2Value(void) const
  {
    return itsColorContouringColorShadeHigh2Value;
  }
  void ColorContouringColorShadeHigh2Value(float newValue)
  {
    itsColorContouringColorShadeHigh2Value = newValue;
  }
  const NFmiColor& ColorContouringColorShadeLowValueColor(void) const
  {
    return itsColorContouringColorShadeLowValueColor;
  }
  void ColorContouringColorShadeLowValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeLowValueColor = newValue;
  }
  const NFmiColor& ColorContouringColorShadeMidValueColor(void) const
  {
    return itsColorContouringColorShadeMidValueColor;
  }
  void ColorContouringColorShadeMidValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeMidValueColor = newValue;
  }
  const NFmiColor& ColorContouringColorShadeHighValueColor(void) const
  {
    return itsColorContouringColorShadeHighValueColor;
  }
  void ColorContouringColorShadeHighValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeHighValueColor = newValue;
  }
  const NFmiColor& ColorContouringColorShadeHigh2ValueColor(void) const
  {
    return itsColorContouringColorShadeHigh2ValueColor;
  }
  void ColorContouringColorShadeHigh2ValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeHigh2ValueColor = newValue;
  }
  const NFmiColor& ColorContouringColorShadeHigh3ValueColor(void) const
  {
    return itsColorContouringColorShadeHigh3ValueColor;
  }
  void ColorContouringColorShadeHigh3ValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeHigh3ValueColor = newValue;
  }
  bool SimpleColorContourTransparentColor1() const { return fSimpleColorContourTransparentColor1; }
  void SimpleColorContourTransparentColor1(bool newValue) { fSimpleColorContourTransparentColor1 = newValue; }
  bool SimpleColorContourTransparentColor2() const { return fSimpleColorContourTransparentColor2; }
  void SimpleColorContourTransparentColor2(bool newValue) { fSimpleColorContourTransparentColor2 = newValue; }
  bool SimpleColorContourTransparentColor3() const { return fSimpleColorContourTransparentColor3; }
  void SimpleColorContourTransparentColor3(bool newValue) { fSimpleColorContourTransparentColor3 = newValue; }
  bool SimpleColorContourTransparentColor4() const { return fSimpleColorContourTransparentColor4; }
  void SimpleColorContourTransparentColor4(bool newValue) { fSimpleColorContourTransparentColor4 = newValue; }
  bool SimpleColorContourTransparentColor5() const { return fSimpleColorContourTransparentColor5; }
  void SimpleColorContourTransparentColor5(bool newValue) { fSimpleColorContourTransparentColor5 = newValue; }
  bool UseWithIsoLineHatch1(void) const { return fUseWithIsoLineHatch1; }
  void UseWithIsoLineHatch1(bool newValue) { fUseWithIsoLineHatch1 = newValue; }
  bool DrawIsoLineHatchWithBorders1(void) const { return fDrawIsoLineHatchWithBorders1; }
  void DrawIsoLineHatchWithBorders1(bool newValue) { fDrawIsoLineHatchWithBorders1 = newValue; }
  float IsoLineHatchLowValue1(void) const { return itsIsoLineHatchLowValue1; }
  void IsoLineHatchLowValue1(float newValue) { itsIsoLineHatchLowValue1 = newValue; }
  float IsoLineHatchHighValue1(void) const { return itsIsoLineHatchHighValue1; }
  void IsoLineHatchHighValue1(float newValue) { itsIsoLineHatchHighValue1 = newValue; }
  int IsoLineHatchType1(void) const { return itsIsoLineHatchType1; }
  void IsoLineHatchType1(int newValue) { itsIsoLineHatchType1 = newValue; }
  const NFmiColor& IsoLineHatchColor1(void) const { return itsIsoLineHatchColor1; }
  void IsoLineHatchColor1(const NFmiColor& newValue) { itsIsoLineHatchColor1 = newValue; }
  const NFmiColor& IsoLineHatchBorderColor1(void) const { return itsIsoLineHatchBorderColor1; }
  void IsoLineHatchBorderColor1(const NFmiColor& newValue)
  {
    itsIsoLineHatchBorderColor1 = newValue;
  }

  bool UseWithIsoLineHatch2(void) const { return fUseWithIsoLineHatch2; }
  void UseWithIsoLineHatch2(bool newValue) { fUseWithIsoLineHatch2 = newValue; }
  bool DrawIsoLineHatchWithBorders2(void) const { return fDrawIsoLineHatchWithBorders2; }
  void DrawIsoLineHatchWithBorders2(bool newValue) { fDrawIsoLineHatchWithBorders2 = newValue; }
  float IsoLineHatchLowValue2(void) const { return itsIsoLineHatchLowValue2; }
  void IsoLineHatchLowValue2(float newValue) { itsIsoLineHatchLowValue2 = newValue; }
  float IsoLineHatchHighValue2(void) const { return itsIsoLineHatchHighValue2; }
  void IsoLineHatchHighValue2(float newValue) { itsIsoLineHatchHighValue2 = newValue; }
  int IsoLineHatchType2(void) const { return itsIsoLineHatchType2; }
  void IsoLineHatchType2(int newValue) { itsIsoLineHatchType2 = newValue; }
  const NFmiColor& IsoLineHatchColor2(void) const { return itsIsoLineHatchColor2; }
  void IsoLineHatchColor2(const NFmiColor& newValue) { itsIsoLineHatchColor2 = newValue; }
  int IsoLineLabelDigitCount(void) const { return itsIsoLineLabelDigitCount; }
  void IsoLineLabelDigitCount(int newValue)
  {
    itsIsoLineLabelDigitCount = newValue;
    if (itsIsoLineLabelDigitCount > 10)
      itsIsoLineLabelDigitCount = 10;
    itsContourLabelDigitCount = newValue;
    if (itsContourLabelDigitCount > 10)
      itsContourLabelDigitCount = 10;
  }

  int ContourLabelDigitCount(void) const { return itsContourLabelDigitCount; }
  void ContourLabelDigitCount(int newValue)
  {
    itsContourLabelDigitCount = newValue;
    if (itsContourLabelDigitCount > 10)
      itsContourLabelDigitCount = 10;
  }

  //**************************************************************
  //********** 'versio 2' parametrien asetusfunktiot *************
  //**************************************************************
  float Alpha(void) const { return itsAlpha; }
  void Alpha(float newValue)
  {
    itsAlpha = newValue;
    if (itsAlpha < NFmiDrawParam::itsMinAlpha)
      itsAlpha = NFmiDrawParam::itsMinAlpha;
    if (itsAlpha > 100.f)
      itsAlpha = 100.f;
  }

  bool ViewMacroDrawParam(void) const { return fViewMacroDrawParam; }
  void ViewMacroDrawParam(bool newState) { fViewMacroDrawParam = newState; }
  const std::string& MacroParamRelativePath(void) const { return itsMacroParamRelativePath; }
  void MacroParamRelativePath(const std::string& newValue) { itsMacroParamRelativePath = newValue; }
  bool BorrowedParam(void) const { return fBorrowedParam; }
  void BorrowedParam(bool newValue) { fBorrowedParam = newValue; }
  // ---------------------- operators ------------------------
  bool operator==(const NFmiDrawParam& theDrawParam) const;
  bool operator<(const NFmiDrawParam& theDrawParam) const;
  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  const static float itsMinAlpha;

  static bool IsMacroParamCase(NFmiInfoData::Type theDataType);
  bool IsMacroParamCase(bool justCheckDataType);
  bool UseTransparentFillColor() const { return fUseTransparentFillColor; }
  void UseTransparentFillColor(bool newValue) { fUseTransparentFillColor = newValue; }
  bool UseViewMacrosSettingsForMacroParam() const { return fUseViewMacrosSettingsForMacroParam; }
  void UseViewMacrosSettingsForMacroParam(bool newValue) { fUseViewMacrosSettingsForMacroParam = newValue; }
  bool DoSparseSymbolVisualization() const { return fDoSparseSymbolVisualization; }
  void DoSparseSymbolVisualization(bool newValue) { fDoSparseSymbolVisualization = newValue; }
  NFmiMetEditorTypes::View GetViewType(bool isStationData) const;
  static bool IsColorContourType(NFmiMetEditorTypes::View viewType);
  static bool IsIsolineType(NFmiMetEditorTypes::View viewType);
  bool DoIsoLineColorBlend() const { return fDoIsoLineColorBlend; }
  void DoIsoLineColorBlend(bool newValue) { fDoIsoLineColorBlend = newValue; }
  bool ShowContourLegendPotentially() const;

 protected:
  double SimpleColorContourTransparentColors2Double() const;
  void Double2SimpleColorContourTransparentColors(double theValue);

  NFmiDataIdent itsParameter;
  NFmiLevel itsLevel;
  std::string itsParameterAbbreviation;
  int itsPriority;

  //   Tähän talletetaan tiedoston nimi, mistä luetaan/mihin
  //   kirjoitetaan
  //   kyseisen luokan tiedot.
  std::string itsInitFileName;
  std::string itsMacroParamRelativePath;  // tätä tarvitaan kun viewMacrojen yhteydessä on
                                          // macroParametreja ja
  // ne ovat alihakemistossa. Eli Kun viewMacro talletetaan tiedostoon, lisätään tämä
  // tieto itsParameterAbbreviation-tiedosn yhteyteen ja se myös puretaan luettaessa tähän.
  // Tämä avulla voidaan rakentaa oikea suhteellinen polku haluttuun macroParamiin
  // Suhteellinen polku on ilman kenoviivoja alussa ja lopussa (esim. "euroMakrot" tai
  // "euroMakrot\analyysi")

  //  Parametrin oletus näyttötyyppi (symboli,
  //  isoviiva, teksti...)
  NFmiMetEditorTypes::View itsViewType;
  NFmiMetEditorTypes::View itsStationDataViewType;  // jos viewtype on isoviiva, mutta data on asema
  // dataa, pitää olla varalla joku näyttötyyppi
  // että voidaan piirtää tällöin
  NFmiColor itsFrameColor;
  NFmiColor itsFillColor;

  NFmiColor itsIsolineLabelBoxFillColor;
  NFmiColor itsContourLabelBoxFillColor;  // **** Versio 3 parametri ****
  //   Minkä kokoinen näyttöön piirrettävä 'symbolidata'
  //   on suhteessa
  //   annettuun asemalle/hilalle varattuun 'datalaatikkoon'.
  NFmiPoint itsOnlyOneSymbolRelativeSize;
  NFmiPoint itsOnlyOneSymbolRelativePositionOffset;
  bool fUseIsoLineGabWithCustomContours;
  bool fUseContourGabWithCustomContours;  // **** Versio 3 parametri ****
  double itsIsoLineGab;
  double itsContourGab;  // **** Versio 3 parametri ****
  double itsModifyingStep;
  //	bool fModifyingUnit;	//(= 0, jos yksikkö on %, = 1, jos yksikkö on sama kuin itsUnit)
  double itsTimeSerialModifyingLimit;  // aikasarjanäytön muutos akselin minimi ja maksimi arvo
  NFmiColor itsIsolineColor;
  NFmiColor itsContourColor;  // **** Versio 3 parametri ****
  NFmiColor itsIsolineTextColor;
  NFmiColor itsContourTextColor;  // **** Versio 3 parametri ****

  double itsAbsoluteMinValue;
  double itsAbsoluteMaxValue;

  double itsTimeSeriesScaleMin;  // käytetään aikasarjaeditorissa
  double itsTimeSeriesScaleMax;  // käytetään aikasarjaeditorissa

  //   Lista mahdollisista näyttötyypeistä kyseiselle
  //   parametrille.
  NFmiMetEditorTypes::View itsPossibleViewTypeList[5];
  int itsPossibleViewTypeCount;

  // Tekstinäyttö:
  bool fShowNumbers;
  bool fShowColors;          // asema tekstiä varten
  bool fShowColoredNumbers;  // asema tekstiä varten
  bool fZeroColorMean;       // asema tekstiä varten

  //***********************************************
  //********** 'versio 2' parametreja *************
  //***********************************************
  float itsStationSymbolColorShadeLowValue;   // väri skaalaus alkaa tästä arvosta
  float itsStationSymbolColorShadeMidValue;   // väri skaalauksen keskiarvo
  float itsStationSymbolColorShadeHighValue;  // väri skaalaus loppuu tähän arvoon
  NFmiColor itsStationSymbolColorShadeLowValueColor;
  NFmiColor itsStationSymbolColorShadeMidValueColor;
  NFmiColor itsStationSymbolColorShadeHighValueColor;
  int itsStationSymbolColorShadeClassCount;  // kuinka monta väri luokkaa tehdään skaalaukseen
  bool fUseSymbolsInTextMode;  // käytetäänkö tekstiä vai mapataanko arvoja kohden jokin symboli
                               // ruudulle?
  int itsUsedSymbolListIndex;         // 0=ei mitään, 1=synopfont, 2=hessaa, ...
  int itsSymbolIndexingMapListIndex;  // indeksi johonkin symbolilistaan, jossa on mapattu arvot
                                      // haluttuihin symboleihin

  NFmiMetEditorTypes::View itsGridDataPresentationStyle;  // isoviiva, color contouring, contour+isoviiva, quick color contouring, etc.
  bool fUseIsoLineFeathering;        // isoviivojen harvennus optio
  bool fUseContourFeathering;        // **** Versio 3 parametri ****
  bool fIsoLineLabelsOverLapping;    // voivatko isoviiva labelit mennä päällekkäin vai ei?
  bool fShowColorLegend;  // piirretäänko rivin viimeiseen ruutuun color contouringin väri selitys
                          // laatikko?
  bool fUseSimpleIsoLineDefinitions;  // otetaanko isoviivamääritykset simppelillä tavalla, vai
                                      // kaikki arvot spesiaali tapauksina määritettyinä
  bool fUseSimpleContourDefinitions;  // **** Versio 3 parametri ****
  bool fUseSeparatorLinesBetweenColorContourClasses;  // piirrä viivat arvo/väri luokkien välille
  float itsSimpleIsoLineGap;
  float itsSimpleIsoLineZeroValue;    // tämän arvon kautta isoviivat joutuvat menemään
  float itsSimpleContourZeroValue;    // **** Versio 3 parametri ****
  float itsSimpleIsoLineLabelHeight;  // relatiivinen vai mm? (0=ei näytetä ollenkaan)
  float itsSimpleContourLabelHeight;  // **** Versio 3 parametri ****
  bool fShowSimpleIsoLineLabelBox;    // ei vielä muita attribuutteja isoviiva labelille (tämä
                                      // tarkoittaa lukua ympäroivää laatikkoa)
  bool fShowSimpleContourLabelBox;    // **** Versio 3 parametri ****
  float itsSimpleIsoLineWidth;        // relatiivinen vai mm?
  float itsSimpleContourWidth;        // **** Versio 3 parametri ****
  int itsSimpleIsoLineLineStyle;      // 1=yht. viiva, 2=pisteviiva jne.
  int itsSimpleContourLineStyle;      // **** Versio 3 parametri ****
  float itsIsoLineSplineSmoothingFactor;    // 0-10, 0=ei pehmennystä, 10=maksimi pyöritys
  bool fUseSingleColorsWithSimpleIsoLines;  // true=sama väri kaikille isoviivoille, false=tehdään
                                            // väriskaala
                                            // isoviivan väriskaalaus asetukset
  float itsSimpleIsoLineColorShadeLowValue;    // väri skaalaus alkaa tästä arvosta
  float itsSimpleIsoLineColorShadeMidValue;    // väri skaalauksen keskiarvo
  float itsSimpleIsoLineColorShadeHighValue;   // väri skaalaus loppuu tähän arvoon
  float itsSimpleIsoLineColorShadeHigh2Value;  // **** Versio 3 parametri ****
  NFmiColor itsSimpleIsoLineColorShadeLowValueColor;
  NFmiColor itsSimpleIsoLineColorShadeMidValueColor;
  NFmiColor itsSimpleIsoLineColorShadeHighValueColor;
  NFmiColor itsSimpleIsoLineColorShadeHigh2ValueColor;  // **** Versio 3 parametri ****
  int itsSimpleIsoLineColorShadeClassCount;  // kuinka monta väri luokkaa tehdään skaalaukseen
  // speciaali isoviiva asetukset (otetaan käyttöön, jos fUseSimpleIsoLineDefinitions=false)
  checkedVector<float>
      itsSpecialIsoLineValues;  // tähän laitetaan kaikki arvot, johon halutaan isoviiva
  checkedVector<float> itsSpecialContourValues;       // **** Versio 3 parametri ****
  checkedVector<float> itsSpecialIsoLineLabelHeight;  // isoviivalabeleiden korkeudet (0=ei labelia)
  checkedVector<float> itsSpecialContourLabelHeight;  // **** Versio 3 parametri ****
  checkedVector<float> itsSpecialIsoLineWidth;        // viivan paksuudet
  checkedVector<float> itsSpecialContourWidth;        // **** Versio 3 parametri ****
  checkedVector<int> itsSpecialIsoLineStyle;          // viiva tyylit
  checkedVector<int> itsSpecialContourStyle;          // **** Versio 3 parametri ****
  checkedVector<int> itsSpecialIsoLineColorIndexies;  // eri viivojen väri indeksit (pitää tehdä
                                                      // näyttö taulukko käyttäjälle)
  checkedVector<int> itsSpecialContourColorIndexies;  // **** Versio 3 parametri ****
  checkedVector<bool> itsSpecialIsoLineShowLabelBox;  // eri viivojen väri indeksit (pitää tehdä
                                                      // näyttö taulukko käyttäjälle)
  // colorcontouring ja quick contouring asetukset
  bool fDrawOnlyOverMask;          // jos true, data piirretään vain maskin päälle
  float itsColorContouringColorShadeLowValue;  // väri skaalaus alkaa tästä arvosta
  float itsColorContouringColorShadeMidValue;  // väri skaalauksen keskiarvo
  float itsColorContouringColorShadeHighValue;
  float itsColorContouringColorShadeHigh2Value;  // väri skaalaus loppuu tähän arvoon
  NFmiColor itsColorContouringColorShadeLowValueColor;
  NFmiColor itsColorContouringColorShadeMidValueColor;
  NFmiColor itsColorContouringColorShadeHighValueColor;
  NFmiColor itsColorContouringColorShadeHigh2ValueColor;
  // Lisätty 5. väri, jotta simple-contour piirrot voidaan tehdä samaan tyyliin kuin custom-contourit
  NFmiColor itsColorContouringColorShadeHigh3ValueColor; 
  // Kaikki Simple-Color-Contour värit voidaan laittaa
  // läpinäkyviksi, eli jos flagin arvo on true, on väri läpinäkyvä
  // ja jos false, se on normaali näkyvä väri.
  bool fSimpleColorContourTransparentColor1 = false;
  bool fSimpleColorContourTransparentColor2 = false;
  bool fSimpleColorContourTransparentColor3 = false;
  bool fSimpleColorContourTransparentColor4 = false;
  bool fSimpleColorContourTransparentColor5 = false;
  // isoviivojen kanssa voi käyttää myös hatchättyjä alueita (2 kpl)
  bool fUseWithIsoLineHatch1;
  bool fDrawIsoLineHatchWithBorders1;
  float itsIsoLineHatchLowValue1;   // hatch alueen ala-arvo
  float itsIsoLineHatchHighValue1;  // hatch alueen yläarvo
  int itsIsoLineHatchType1;  // hatch tyyppi 1=vinoviiva oikealle, 2=vinoviiva vasemmalle jne.
  NFmiColor itsIsoLineHatchColor1;
  NFmiColor itsIsoLineHatchBorderColor1;
  bool fUseWithIsoLineHatch2;
  bool fDrawIsoLineHatchWithBorders2;
  float itsIsoLineHatchLowValue2;   // hatch alueen ala-arvo
  float itsIsoLineHatchHighValue2;  // hatch alueen yläarvo
  int itsIsoLineHatchType2;  // hatch tyyppi 1=vinoviiva oikealle, 2=vinoviiva vasemmalle jne.
  NFmiColor itsIsoLineHatchColor2;
  int itsIsoLineLabelDigitCount;  // isoviiva labelin näytettävien digitaalien lukumäärä
  int itsContourLabelDigitCount;  // **** Versio 3 parametri ****
                                  //***********************************************
                                  //********** 'versio 2' parametreja *************
                                  //***********************************************
  float itsAlpha;  // läpinäkyvyys kerroin, 0 on täysin läpinäkyvä ja 100 täysin läpinäkymätön (0 on
                   // estetty ja itsMinAlpha on säädetty minimi raja)

 private:
  static float itsFileVersionNumber;
  float itsInitFileVersionNumber;

  bool fHidden;       // näyttö voidaan piiloittaa tämän mukaisesti
  bool fEditedParam;  // vain yksi parametreista voidaan editoida yhtä aikaa
  //	bool fEditableParam;	// onko parametri suoraan editoitavissa ollenkaan? (esim. HESSAA tai
  // tuulivektori eivät ole)

  std::string itsUnit;
  bool fActive;  // onko kyseinen parametri näytön aktiivinen parametri (jokaisella näyttörivillä
                 // aina yksi aktivoitunut parametri)
  bool fShowDifferenceToOriginalData;

  // lisäsin tämän, kun laitoin editoriin satelliitti kuvien
  // katselun mahdollisuuden (satel-datalla ei ole infoa)
  NFmiInfoData::Type itsDataType;  
  
  
  // is this DrawParam from viewmacro, if it is, then some things are handled
  // differently when modifying options, default value is false This is not stored in file!
  bool fViewMacroDrawParam;


  // onko parametri lainassa vai ei (A-J Punkan vilkutus juttu)
  // Arkistodatan käyttöön liittyviä asetuksia (historialliseen dataan voidaan viitata kahdella eri tavalla)
  // Joko suoraan fixatulla itsModelOriginTime:lla tai itsModelRunIndex:illä, jolla kerrotaan monesko ajo
  // kiinnostaa viimeiseen nähden.
  // Prioriteetti järjestys on, että ensin tarkistetaan löytyykö fiksattu aika (itsModelOriginTime)
  // ja sitten vasta onko suhteellinen.
  bool fBorrowedParam;       

  // Jos tässä arvo, etsitään arkistosta (SmartMetin (tulevaisuudessa) oma tai Q2Server)
  // suoraan kyseinen aika. Jos arvo on NFmiDrawParam::gMissingOriginTime, tämä ei ole käytössä.
  NFmiMetTime itsModelOriginTime;  


  // Jos tässä on negatiivinen arvo (-1 on edellinen malliajo, -2 on sitä edellinen jne.), haetaan
  // arkistosta dataa. Tämä luku on siis verrattuna kyseisen mallin viimeisimpään data osaan, joka löytyy.
  // Tarkoittaen että jos Hirlamista on pinta ja painepinta datasta tullut jo 06:n ajo ja
  // mallipintadatasta vasta 00, silloin viimeisin ajo on 06 ja -1 viittaa tällöin 00-ajoon.
  // Jos tämä on 0 tai positiivinen, tämä ei ole käytössä.
  int itsModelRunIndex;  

  // tähän lasketaan relatiivisen malliajon mukainen
  // origin aika, jotä käytetään sitten mm. tooltipeissä ja muualla
  NFmiMetTime itsModelOriginTimeCalculated;  

  // tähän määrätään kuinka monta viimeista ajoa näytetään mallille
  // kerrallaa aikasarjassa. Jos arvo on 0 (default), ei näytetä kuin viimeinen ajo normaalisti.
  int itsTimeSerialModelRunCount;  

  // Default is true (like old behaviour). If set false, label boxes will be filled with itsIsolineLabelBoxFillColor.
  bool fUseTransparentFillColor;
  // This is FIX for complicated SmartMet code in case where we wan't to prevent loading macroParameter's drawing settings
  // normal way in NFmiStationView's SetupUsedDrawParam method. AND we want to prevent that some viewMacro loadings
  // are not considered as viewMacros at all (F12 and CTRL + F12 functions in SmartMet).
  // Don't use this for anything else. Right way to correct the problem is huge task (re-write certain parts of SmartMet's code).
  bool fUseViewMacrosSettingsForMacroParam;
  // Sparse symboli piirto tarkoittaa että suurin osa hilan arvoista puuttuu (ainakin pitäisi) 
  // ja kaikki ei-puuttuvat arvot piirretään symboleilla oli SmartMetin symboliharvennus asetus käytössä tai ei.
  bool fDoSparseSymbolVisualization;
  // Tästä eteenpäin isoviivojen värejä blendaillaan, vain jos tämän arvo on true.
  // Blendaus välinä tällöin käytetään itsIsolineGab:in arvoa ja blendejä tehdään kahden isoviiva limitin väleihin.
  bool fDoIsoLineColorBlend;
};
//@{ \name Globaalit NFmiDrawParam-luokan uudelleenohjaus-operaatiot
inline std::ostream& operator<<(std::ostream& os, const NFmiDrawParam& item)
{
  return item.Write(os);
}
inline std::istream& operator>>(std::istream& is, NFmiDrawParam& item)
{
  return item.Read(is);
}
//@}

