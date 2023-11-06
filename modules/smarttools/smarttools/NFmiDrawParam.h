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

#include <boost/shared_ptr.hpp>
#include <newbase/NFmiDataIdent.h>
#include <newbase/NFmiInfoData.h>
#include <newbase/NFmiLevel.h>
#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiParameterName.h>
#include <newbase/NFmiPoint.h>

class NFmiDrawingEnvironment;

const int DefaultFixedTextSymbolDrawLength = 0;
const double DefaultSymbolDrawDensity = 1.;
const double DrawParamMinSymbolDrawDensity = 0.4;
const double DrawParamMaxSymbolDrawDensity = 2.0;

class NFmiDrawParam
{
 public:
  NFmiDrawParam();
  NFmiDrawParam(const NFmiDataIdent& theParam,
                const NFmiLevel& theLevel,
                int thePriority,
                NFmiInfoData::Type theDataType =
                    NFmiInfoData::kNoDataType);  //, NFmiMetEditorCoordinatorMapOptions*
  // theMetEditorCoordinatorMapOptions=0);
  NFmiDrawParam(const NFmiDrawParam& other);
  virtual ~NFmiDrawParam();

  void Init(const NFmiDrawParam* theDrawParam, bool fInitOnlyDrawingOptions = false);
  void Init(const boost::shared_ptr<NFmiDrawParam>& theDrawParam,
            bool fInitOnlyDrawingOptions = false);
  void HideParam(bool newValue) { fHidden = newValue; };
  void EditParam(bool newValue) { fEditedParam = newValue; };
  bool IsParamHidden() const { return fHidden; };
  bool IsParamEdited() const { return fEditedParam; };
  const NFmiMetTime& ModelOriginTime() const { return itsModelOriginTime; }
  void ModelOriginTime(const NFmiMetTime& newValue) { itsModelOriginTime = newValue; }
  int ModelRunIndex() const { return itsModelRunIndex; }
  void ModelRunIndex(int newValue) { itsModelRunIndex = newValue; }
  bool UseArchiveModelData() const;
  bool IsModelRunDataType() const;
  static bool IsModelRunDataType(NFmiInfoData::Type theDataType);
  const NFmiMetTime& ModelOriginTimeCalculated() const { return itsModelOriginTimeCalculated; }
  void ModelOriginTimeCalculated(const NFmiMetTime& newValue)
  {
    itsModelOriginTimeCalculated = newValue;
  }
  int TimeSerialModelRunCount() const { return itsTimeSerialModelRunCount; }
  void TimeSerialModelRunCount(int newValue)
  {
    itsTimeSerialModelRunCount = newValue;
    if (itsTimeSerialModelRunCount < 0)
      itsTimeSerialModelRunCount = 0;
  }

  NFmiInfoData::Type DataType() const { return itsDataType; }
  // HUOM! tämä asettaa vain itsDataType-dataosan arvon, ei mahdollista itsInfon data tyyppiä!!!!!!
  void DataType(NFmiInfoData::Type newValue) { itsDataType = newValue; };
  bool Init(const std::string& theFilename);
  bool StoreData(const std::string& theFilename);

  // --------------- "set" ja "get" metodit -----------------
  const std::string& ParameterAbbreviation() const;
  void ParameterAbbreviation(const std::string& theParameterAbbreviation)
  {
    itsParameterAbbreviation = theParameterAbbreviation;
  }
  NFmiDataIdent& Param() { return itsParameter; };
  void Param(const NFmiDataIdent& theParameter) { itsParameter = theParameter; };
  NFmiLevel& Level() { return itsLevel; }
  void Level(const NFmiLevel& theLevel) { itsLevel = theLevel; }
  void Priority(int thePriority) { itsPriority = thePriority; };
  int Priority() const { return itsPriority; };
  void ViewType(const NFmiMetEditorTypes::View& theViewType) { itsViewType = theViewType; };
  NFmiMetEditorTypes::View ViewType() const { return itsViewType; };
  void FrameColor(const NFmiColor& theFrameColor) { itsFrameColor = theFrameColor; };
  const NFmiColor& FrameColor() const { return itsFrameColor; };
  void FillColor(const NFmiColor& theFillColor) { itsFillColor = theFillColor; };
  const NFmiColor& FillColor() const { return itsFillColor; };
  void IsolineLabelBoxFillColor(const NFmiColor& theColor)
  {
    itsIsolineLabelBoxFillColor = theColor;
    itsContourLabelBoxFillColor =
        theColor;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  };
  const NFmiColor& IsolineLabelBoxFillColor() const { return itsIsolineLabelBoxFillColor; };
  void ContourLabelBoxFillColor(const NFmiColor& theColor)
  {
    itsContourLabelBoxFillColor = theColor;
  };
  const NFmiColor& ContourLabelBoxFillColor() const { return itsContourLabelBoxFillColor; };
  void OnlyOneSymbolRelativeSize(const NFmiPoint& theOnlyOneSymbolRelativeSize)
  {
    itsOnlyOneSymbolRelativeSize = theOnlyOneSymbolRelativeSize;
  };
  const NFmiPoint& OnlyOneSymbolRelativeSize() const { return itsOnlyOneSymbolRelativeSize; };
  void OnlyOneSymbolRelativePositionOffset(const NFmiPoint& theOnlyOneSymbolRelativePositionOffset)
  {
    itsOnlyOneSymbolRelativePositionOffset = theOnlyOneSymbolRelativePositionOffset;
  };
  const NFmiPoint& OnlyOneSymbolRelativePositionOffset() const
  {
    return itsOnlyOneSymbolRelativePositionOffset;
  };
  void UseIsoLineGabWithCustomContours(const bool newState)
  {
    fUseIsoLineGabWithCustomContours = newState;
    fUseContourGabWithCustomContours =
        newState;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  };
  bool UseIsoLineGabWithCustomContours() const { return fUseIsoLineGabWithCustomContours; };
  void UseContourGabWithCustomContours(const bool newState)
  {
    fUseContourGabWithCustomContours = newState;
  };
  bool UseContourGabWithCustomContours() const { return fUseContourGabWithCustomContours; };
  void IsoLineGab(const double theIsoLineGab)
  {
    itsIsoLineGab = theIsoLineGab;
    if (itsIsoLineGab == 0)
      itsIsoLineGab = 1;  // gappi ei voi olla 0
  }
  double IsoLineGab() const { return itsIsoLineGab; };
  void ContourGab(const double theContourGab)
  {
    itsContourGab = theContourGab;
    if (itsContourGab == 0)
      itsContourGab = 1;
  }
  double ContourGab() const { return itsContourGab; };
  void ModifyingStep(const double theModifyingStep) { itsModifyingStep = theModifyingStep; };
  double ModifyingStep() const { return itsModifyingStep; };
  //	void				 ModifyingUnit (bool theModifyingUnit) { fModifyingUnit =
  // theModifyingUnit; }
  //	bool			 ModifyingUnit () const { return fModifyingUnit; }
  const NFmiMetEditorTypes::View* PossibleViewTypeList() const { return itsPossibleViewTypeList; }
  int PossibleViewTypeCount() const { return itsPossibleViewTypeCount; };
  const std::string& InitFileName() const { return itsInitFileName; }
  void InitFileName(std::string theFileName) { itsInitFileName = theFileName; }
  double AbsoluteMinValue() const { return itsAbsoluteMinValue; }
  void AbsoluteMinValue(double theAbsoluteMinValue) { itsAbsoluteMinValue = theAbsoluteMinValue; }
  double AbsoluteMaxValue() const { return itsAbsoluteMaxValue; }
  void AbsoluteMaxValue(double theAbsoluteMaxValue) { itsAbsoluteMaxValue = theAbsoluteMaxValue; }
  double TimeSeriesScaleMin() const { return itsTimeSeriesScaleMin; };
  double TimeSeriesScaleMax() const { return itsTimeSeriesScaleMax; };
  void TimeSeriesScaleMin(double theValue) { itsTimeSeriesScaleMin = theValue; };
  void TimeSeriesScaleMax(double theValue) { itsTimeSeriesScaleMax = theValue; };
  const NFmiColor& IsolineColor() const { return itsIsolineColor; };
  void IsolineColor(const NFmiColor& newColor)
  {
    itsIsolineColor = newColor;
    itsContourColor = newColor;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  const NFmiColor& ContourColor() const { return itsContourColor; };
  void ContourColor(const NFmiColor& newColor) { itsContourColor = newColor; };
  void IsolineTextColor(const NFmiColor& newColor)
  {
    itsIsolineTextColor = newColor;
    // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
    itsContourTextColor = newColor;
  }
  const NFmiColor& IsolineTextColor() const { return itsIsolineTextColor; };

  void ContourTextColor(const NFmiColor& newColor) { itsContourTextColor = newColor; };
  const NFmiColor& ContourTextColor() const { return itsContourTextColor; };
  //	double TimeSerialModifyingLimit() const {return fModifyingUnit ?
  // itsTimeSerialModifyingLimit : 100;};
  double TimeSerialModifyingLimit() const { return itsTimeSerialModifyingLimit; };
  NFmiMetEditorTypes::View StationDataViewType() const { return itsStationDataViewType; };
  void TimeSerialModifyingLimit(double newValue) { itsTimeSerialModifyingLimit = newValue; };
  void StationDataViewType(NFmiMetEditorTypes::View newValue)
  {
    itsStationDataViewType = newValue;
  };

  void FileVersionNumber(const float theFileVersionNumber)
  {
    itsFileVersionNumber = theFileVersionNumber;
  };
  float FileVersionNumber() const { return itsFileVersionNumber; };
  void Unit(const std::string& theUnit) { itsUnit = theUnit; };
  const std::string& Unit() const { return itsUnit; };
  bool ShowNumbers() const { return fShowNumbers; }
  void ShowNumbers(bool theValue) { fShowNumbers = theValue; }
  bool ShowColors() const { return fShowColors; }
  void ShowColors(bool theValue) { fShowColors = theValue; }
  bool ShowColoredNumbers() const { return fShowColoredNumbers; }
  void ShowColoredNumbers(bool theValue) { fShowColoredNumbers = theValue; }
  bool ZeroColorMean() const { return fZeroColorMean; }
  void ZeroColorMean(bool theValue) { fZeroColorMean = theValue; }
  bool IsActive() const { return fActive; };
  void Activate(bool newState) { fActive = newState; };
  bool ShowDifferenceToOriginalData() const { return fShowDifferenceToOriginalData; }
  void ShowDifferenceToOriginalData(bool newValue) { fShowDifferenceToOriginalData = newValue; }
  //**************************************************************
  //********** 'versio 2' parametrien asetusfunktiot *************
  //**************************************************************
  float StationSymbolColorShadeLowValue() const { return itsStationSymbolColorShadeLowValue; }
  void StationSymbolColorShadeLowValue(float newValue)
  {
    itsStationSymbolColorShadeLowValue = newValue;
  }
  float StationSymbolColorShadeMidValue() const { return itsStationSymbolColorShadeMidValue; }
  void StationSymbolColorShadeMidValue(float newValue)
  {
    itsStationSymbolColorShadeMidValue = newValue;
  }
  float StationSymbolColorShadeHighValue() const { return itsStationSymbolColorShadeHighValue; }
  void StationSymbolColorShadeHighValue(float newValue)
  {
    itsStationSymbolColorShadeHighValue = newValue;
  }
  const NFmiColor& StationSymbolColorShadeLowValueColor() const
  {
    return itsStationSymbolColorShadeLowValueColor;
  }
  void StationSymbolColorShadeLowValueColor(const NFmiColor& newValue)
  {
    itsStationSymbolColorShadeLowValueColor = newValue;
  }
  const NFmiColor& StationSymbolColorShadeMidValueColor() const
  {
    return itsStationSymbolColorShadeMidValueColor;
  }
  void StationSymbolColorShadeMidValueColor(const NFmiColor& newValue)
  {
    itsStationSymbolColorShadeMidValueColor = newValue;
  }
  const NFmiColor& StationSymbolColorShadeHighValueColor() const
  {
    return itsStationSymbolColorShadeHighValueColor;
  }
  void StationSymbolColorShadeHighValueColor(const NFmiColor& newValue)
  {
    itsStationSymbolColorShadeHighValueColor = newValue;
  }
  int StationSymbolColorShadeClassCount() const { return itsStationSymbolColorShadeClassCount; }
  void StationSymbolColorShadeClassCount(int newValue)
  {
    itsStationSymbolColorShadeClassCount = newValue;
  }
  bool UseSymbolsInTextMode() const { return fUseSymbolsInTextMode; }
  void UseSymbolsInTextMode(bool newValue) { fUseSymbolsInTextMode = newValue; }
  int UsedSymbolListIndex() const { return itsUsedSymbolListIndex; }
  void UsedSymbolListIndex(int newValue) { itsUsedSymbolListIndex = newValue; }
  int SymbolIndexingMapListIndex() const { return itsSymbolIndexingMapListIndex; }
  void SymbolIndexingMapListIndex(int newValue) { itsSymbolIndexingMapListIndex = newValue; }
  NFmiMetEditorTypes::View GridDataPresentationStyle() const
  {
    return itsGridDataPresentationStyle;
  }
  void GridDataPresentationStyle(NFmiMetEditorTypes::View newValue)
  {
    itsGridDataPresentationStyle = newValue;
  }
  bool UseIsoLineFeathering() const { return fUseIsoLineFeathering; }
  void UseIsoLineFeathering(bool newValue)
  {
    fUseIsoLineFeathering = newValue;
    fUseContourFeathering =
        newValue;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  bool UseContourFeathering() const { return fUseContourFeathering; }
  void UseContourFeathering(bool newValue) { fUseContourFeathering = newValue; }
  bool IsoLineLabelsOverLapping() const { return fIsoLineLabelsOverLapping; }
  void IsoLineLabelsOverLapping(bool newValue) { fIsoLineLabelsOverLapping = newValue; }
  bool ShowColorLegend() const { return fShowColorLegend; }
  void ShowColorLegend(bool newValue) { fShowColorLegend = newValue; }
  bool UseSimpleIsoLineDefinitions() const { return fUseSimpleIsoLineDefinitions; }
  void UseSimpleIsoLineDefinitions(bool newValue)
  {
    fUseSimpleIsoLineDefinitions = newValue;
    fUseSimpleContourDefinitions =
        newValue;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  bool UseSimpleContourDefinitions() const { return fUseSimpleContourDefinitions; }
  void UseSimpleContourDefinitions(bool newValue) { fUseSimpleContourDefinitions = newValue; }
  bool UseSeparatorLinesBetweenColorContourClasses() const
  {
    return fUseSeparatorLinesBetweenColorContourClasses;
  }
  void UseSeparatorLinesBetweenColorContourClasses(bool newValue)
  {
    fUseSeparatorLinesBetweenColorContourClasses = newValue;
  }
  float SimpleIsoLineGap() const { return itsSimpleIsoLineGap; }
  void SimpleIsoLineGap(float newValue) { itsSimpleIsoLineGap = newValue; }
  float SimpleIsoLineZeroValue() const { return itsSimpleIsoLineZeroValue; }
  void SimpleIsoLineZeroValue(float newValue)
  {
    itsSimpleIsoLineZeroValue = newValue;
    itsSimpleContourZeroValue =
        newValue;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  float SimpleContourZeroValue() const { return itsSimpleContourZeroValue; }
  void SimpleContourZeroValue(float newValue) { itsSimpleContourZeroValue = newValue; }
  float SimpleIsoLineLabelHeight() const { return itsSimpleIsoLineLabelHeight; }
  void SimpleIsoLineLabelHeight(float newValue)
  {
    itsSimpleIsoLineLabelHeight = newValue;
    itsSimpleContourLabelHeight =
        newValue;  // **** Versio 3 parametri asetetaan toistaiseksi myös näin ****
  }

  float SimpleContourLabelHeight() const { return itsSimpleContourLabelHeight; }
  void SimpleContourLabelHeight(float newValue) { itsSimpleContourLabelHeight = newValue; }
  bool ShowSimpleIsoLineLabelBox() const { return fShowSimpleIsoLineLabelBox; }
  void ShowSimpleIsoLineLabelBox(bool newValue)
  {
    fShowSimpleIsoLineLabelBox = newValue;
    fShowSimpleContourLabelBox = newValue;
  }

  bool ShowSimpleContourLabelBox() const { return fShowSimpleContourLabelBox; }
  void ShowSimpleContourLabelBox(bool newValue) { fShowSimpleContourLabelBox = newValue; }
  float SimpleIsoLineWidth() const { return itsSimpleIsoLineWidth; }
  void SimpleIsoLineWidth(float newValue)
  {
    itsSimpleIsoLineWidth = newValue;
    itsSimpleContourWidth = newValue;
  }

  float SimpleContourWidth() const { return itsSimpleContourWidth; }
  void SimpleContourWidth(float newValue) { itsSimpleContourWidth = newValue; }
  int SimpleIsoLineLineStyle() const { return itsSimpleIsoLineLineStyle; }
  void SimpleIsoLineLineStyle(int newValue)
  {
    itsSimpleIsoLineLineStyle = newValue;
    itsSimpleContourLineStyle = newValue;
  }

  int SimpleContourLineStyle() const { return itsSimpleContourLineStyle; }
  void SimpleContourLineStyle(int newValue) { itsSimpleContourLineStyle = newValue; }
  float IsoLineSplineSmoothingFactor() const { return itsIsoLineSplineSmoothingFactor; }
  void IsoLineSplineSmoothingFactor(float newValue) { itsIsoLineSplineSmoothingFactor = newValue; }
  bool UseSingleColorsWithSimpleIsoLines() const { return fUseSingleColorsWithSimpleIsoLines; }
  void UseSingleColorsWithSimpleIsoLines(bool newValue)
  {
    fUseSingleColorsWithSimpleIsoLines = newValue;
  }
  float SimpleIsoLineColorShadeLowValue() const { return itsSimpleIsoLineColorShadeLowValue; }
  void SimpleIsoLineColorShadeLowValue(float newValue)
  {
    itsSimpleIsoLineColorShadeLowValue = newValue;
  }
  float SimpleIsoLineColorShadeMidValue() const { return itsSimpleIsoLineColorShadeMidValue; }
  void SimpleIsoLineColorShadeMidValue(float newValue)
  {
    itsSimpleIsoLineColorShadeMidValue = newValue;
  }
  float SimpleIsoLineColorShadeHighValue() const { return itsSimpleIsoLineColorShadeHighValue; }
  void SimpleIsoLineColorShadeHighValue(float newValue)
  {
    itsSimpleIsoLineColorShadeHighValue = newValue;
  }

  float SimpleIsoLineColorShadeHigh2Value() const { return itsSimpleIsoLineColorShadeHigh2Value; }
  void SimpleIsoLineColorShadeHigh2Value(float newValue)
  {
    itsSimpleIsoLineColorShadeHigh2Value = newValue;
  }

  const NFmiColor& SimpleIsoLineColorShadeLowValueColor() const
  {
    return itsSimpleIsoLineColorShadeLowValueColor;
  }
  void SimpleIsoLineColorShadeLowValueColor(const NFmiColor& newValue)
  {
    itsSimpleIsoLineColorShadeLowValueColor = newValue;
  }
  const NFmiColor& SimpleIsoLineColorShadeMidValueColor() const
  {
    return itsSimpleIsoLineColorShadeMidValueColor;
  }
  void SimpleIsoLineColorShadeMidValueColor(const NFmiColor& newValue)
  {
    itsSimpleIsoLineColorShadeMidValueColor = newValue;
  }
  const NFmiColor& SimpleIsoLineColorShadeHighValueColor() const
  {
    return itsSimpleIsoLineColorShadeHighValueColor;
  }
  void SimpleIsoLineColorShadeHighValueColor(const NFmiColor& newValue)
  {
    itsSimpleIsoLineColorShadeHighValueColor = newValue;
  }

  const NFmiColor& SimpleIsoLineColorShadeHigh2ValueColor() const
  {
    return itsSimpleIsoLineColorShadeHigh2ValueColor;
  }
  void SimpleIsoLineColorShadeHigh2ValueColor(const NFmiColor& newValue)
  {
    itsSimpleIsoLineColorShadeHigh2ValueColor = newValue;
  }

  const NFmiColor& SimpleIsoLineColorShadeHigh3ValueColor() const
  {
    return itsSimpleIsoLineColorShadeHigh3ValueColor;
  }

  void SimpleIsoLineColorShadeHigh3ValueColor(const NFmiColor& newValue)
  {
    itsSimpleIsoLineColorShadeHigh3ValueColor = newValue;
  }

  int SimpleIsoLineColorShadeClassCount() const { return itsSimpleIsoLineColorShadeClassCount; }
  void SimpleIsoLineColorShadeClassCount(int newValue)
  {
    itsSimpleIsoLineColorShadeClassCount = newValue;
  }
  const std::vector<float>& SpecialIsoLineValues() const { return itsSpecialIsoLineValues; }
  void SetSpecialIsoLineValues(const std::vector<float>& newValue)
  {
    itsSpecialIsoLineValues = newValue;
    itsSpecialContourValues = newValue;
  }

  const std::vector<float>& SpecialContourValues() const { return itsSpecialContourValues; }
  void SetSpecialContourValues(const std::vector<float>& newValue)
  {
    itsSpecialContourValues = newValue;
  }

  const std::vector<float>& SpecialIsoLineLabelHeight() const
  {
    return itsSpecialIsoLineLabelHeight;
  }
  void SetSpecialIsoLineLabelHeight(const std::vector<float>& newValue)
  {
    itsSpecialIsoLineLabelHeight = newValue;
    itsSpecialContourLabelHeight = newValue;
  }

  const std::vector<float>& SpecialContourLabelHeight() const
  {
    return itsSpecialContourLabelHeight;
  }
  void SetSpecialContourLabelHeight(const std::vector<float>& newValue)
  {
    itsSpecialContourLabelHeight = newValue;
  }

  const std::vector<float>& SpecialIsoLineWidth() const { return itsSpecialIsoLineWidth; }
  void SetSpecialIsoLineWidth(const std::vector<float>& newValue)
  {
    itsSpecialIsoLineWidth = newValue;
    itsSpecialContourWidth = newValue;
  }

  const std::vector<float>& SpecialcontourWidth() const { return itsSpecialContourWidth; }
  void SetSpecialcontourWidth(const std::vector<float>& newValue)
  {
    itsSpecialContourWidth = newValue;
  }

  const std::vector<int>& SpecialIsoLineStyle() const { return itsSpecialIsoLineStyle; }
  void SetSpecialIsoLineStyle(const std::vector<int>& newValue)
  {
    itsSpecialIsoLineStyle = newValue;
    itsSpecialContourStyle = newValue;
  }

  const std::vector<int>& SpecialContourStyle() const { return itsSpecialContourStyle; }
  void SetSpecialContourStyle(std::vector<int>& newValue) { itsSpecialContourStyle = newValue; }
  const std::vector<int>& SpecialIsoLineColorIndexies() const
  {
    return itsSpecialIsoLineColorIndexies;
  }
  void SetSpecialIsoLineColorIndexies(const std::vector<int>& newValue)
  {
    itsSpecialIsoLineColorIndexies = newValue;
    itsSpecialContourColorIndexies = newValue;
  }

  const std::vector<int>& SpecialContourColorIndexies() const
  {
    return itsSpecialContourColorIndexies;
  }
  void SetSpecialContourColorIndexies(const std::vector<int>& newValue)
  {
    itsSpecialContourColorIndexies = newValue;
  }

  const std::vector<bool>& SpecialIsoLineShowLabelBox() const
  {
    return itsSpecialIsoLineShowLabelBox;
  }
  void SpecialIsoLineShowLabelBox(std::vector<bool>& newValue)
  {
    itsSpecialIsoLineShowLabelBox = newValue;
  }
  bool DrawOnlyOverMask() const { return fDrawOnlyOverMask; }
  void DrawOnlyOverMask(bool newValue) { fDrawOnlyOverMask = newValue; }

  float ColorContouringColorShadeLowValue() const { return itsColorContouringColorShadeLowValue; }
  void ColorContouringColorShadeLowValue(float newValue)
  {
    itsColorContouringColorShadeLowValue = newValue;
  }
  float ColorContouringColorShadeMidValue() const { return itsColorContouringColorShadeMidValue; }
  void ColorContouringColorShadeMidValue(float newValue)
  {
    itsColorContouringColorShadeMidValue = newValue;
  }
  float ColorContouringColorShadeHighValue() const { return itsColorContouringColorShadeHighValue; }
  void ColorContouringColorShadeHighValue(float newValue)
  {
    itsColorContouringColorShadeHighValue = newValue;
  }
  float ColorContouringColorShadeHigh2Value() const
  {
    return itsColorContouringColorShadeHigh2Value;
  }
  void ColorContouringColorShadeHigh2Value(float newValue)
  {
    itsColorContouringColorShadeHigh2Value = newValue;
  }
  const NFmiColor& ColorContouringColorShadeLowValueColor() const
  {
    return itsColorContouringColorShadeLowValueColor;
  }
  void ColorContouringColorShadeLowValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeLowValueColor = newValue;
  }
  const NFmiColor& ColorContouringColorShadeMidValueColor() const
  {
    return itsColorContouringColorShadeMidValueColor;
  }
  void ColorContouringColorShadeMidValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeMidValueColor = newValue;
  }
  const NFmiColor& ColorContouringColorShadeHighValueColor() const
  {
    return itsColorContouringColorShadeHighValueColor;
  }
  void ColorContouringColorShadeHighValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeHighValueColor = newValue;
  }
  const NFmiColor& ColorContouringColorShadeHigh2ValueColor() const
  {
    return itsColorContouringColorShadeHigh2ValueColor;
  }
  void ColorContouringColorShadeHigh2ValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeHigh2ValueColor = newValue;
  }
  const NFmiColor& ColorContouringColorShadeHigh3ValueColor() const
  {
    return itsColorContouringColorShadeHigh3ValueColor;
  }
  void ColorContouringColorShadeHigh3ValueColor(const NFmiColor& newValue)
  {
    itsColorContouringColorShadeHigh3ValueColor = newValue;
  }
  bool SimpleColorContourTransparentColor1() const { return fSimpleColorContourTransparentColor1; }
  void SimpleColorContourTransparentColor1(bool newValue)
  {
    fSimpleColorContourTransparentColor1 = newValue;
  }
  bool SimpleColorContourTransparentColor2() const { return fSimpleColorContourTransparentColor2; }
  void SimpleColorContourTransparentColor2(bool newValue)
  {
    fSimpleColorContourTransparentColor2 = newValue;
  }
  bool SimpleColorContourTransparentColor3() const { return fSimpleColorContourTransparentColor3; }
  void SimpleColorContourTransparentColor3(bool newValue)
  {
    fSimpleColorContourTransparentColor3 = newValue;
  }
  bool SimpleColorContourTransparentColor4() const { return fSimpleColorContourTransparentColor4; }
  void SimpleColorContourTransparentColor4(bool newValue)
  {
    fSimpleColorContourTransparentColor4 = newValue;
  }
  bool SimpleColorContourTransparentColor5() const { return fSimpleColorContourTransparentColor5; }
  void SimpleColorContourTransparentColor5(bool newValue)
  {
    fSimpleColorContourTransparentColor5 = newValue;
  }
  bool UseWithIsoLineHatch1() const { return fUseWithIsoLineHatch1; }
  void UseWithIsoLineHatch1(bool newValue) { fUseWithIsoLineHatch1 = newValue; }
  bool DrawIsoLineHatchWithBorders1() const { return fDrawIsoLineHatchWithBorders1; }
  void DrawIsoLineHatchWithBorders1(bool newValue) { fDrawIsoLineHatchWithBorders1 = newValue; }
  float IsoLineHatchLowValue1() const { return itsIsoLineHatchLowValue1; }
  void IsoLineHatchLowValue1(float newValue) { itsIsoLineHatchLowValue1 = newValue; }
  float IsoLineHatchHighValue1() const { return itsIsoLineHatchHighValue1; }
  void IsoLineHatchHighValue1(float newValue) { itsIsoLineHatchHighValue1 = newValue; }
  int IsoLineHatchType1() const { return itsIsoLineHatchType1; }
  void IsoLineHatchType1(int newValue) { itsIsoLineHatchType1 = newValue; }
  const NFmiColor& IsoLineHatchColor1() const { return itsIsoLineHatchColor1; }
  void IsoLineHatchColor1(const NFmiColor& newValue) { itsIsoLineHatchColor1 = newValue; }
  const NFmiColor& IsoLineHatchBorderColor1() const { return itsIsoLineHatchBorderColor1; }
  void IsoLineHatchBorderColor1(const NFmiColor& newValue)
  {
    itsIsoLineHatchBorderColor1 = newValue;
  }

  bool UseWithIsoLineHatch2() const { return fUseWithIsoLineHatch2; }
  void UseWithIsoLineHatch2(bool newValue) { fUseWithIsoLineHatch2 = newValue; }
  bool DrawIsoLineHatchWithBorders2() const { return fDrawIsoLineHatchWithBorders2; }
  void DrawIsoLineHatchWithBorders2(bool newValue) { fDrawIsoLineHatchWithBorders2 = newValue; }
  float IsoLineHatchLowValue2() const { return itsIsoLineHatchLowValue2; }
  void IsoLineHatchLowValue2(float newValue) { itsIsoLineHatchLowValue2 = newValue; }
  float IsoLineHatchHighValue2() const { return itsIsoLineHatchHighValue2; }
  void IsoLineHatchHighValue2(float newValue) { itsIsoLineHatchHighValue2 = newValue; }
  int IsoLineHatchType2() const { return itsIsoLineHatchType2; }
  void IsoLineHatchType2(int newValue) { itsIsoLineHatchType2 = newValue; }
  const NFmiColor& IsoLineHatchColor2() const { return itsIsoLineHatchColor2; }
  void IsoLineHatchColor2(const NFmiColor& newValue) { itsIsoLineHatchColor2 = newValue; }
  int IsoLineLabelDigitCount() const { return itsIsoLineLabelDigitCount; }
  void IsoLineLabelDigitCount(int newValue)
  {
    itsIsoLineLabelDigitCount = newValue;
    if (itsIsoLineLabelDigitCount > 10)
      itsIsoLineLabelDigitCount = 10;
    itsContourLabelDigitCount = newValue;
    if (itsContourLabelDigitCount > 10)
      itsContourLabelDigitCount = 10;
  }

  int ContourLabelDigitCount() const { return itsContourLabelDigitCount; }
  void ContourLabelDigitCount(int newValue)
  {
    itsContourLabelDigitCount = newValue;
    if (itsContourLabelDigitCount > 10)
      itsContourLabelDigitCount = 10;
  }

  //**************************************************************
  //********** 'versio 2' parametrien asetusfunktiot *************
  //**************************************************************
  float Alpha() const { return itsAlpha; }
  void Alpha(float newValue)
  {
    itsAlpha = newValue;
    if (itsAlpha < NFmiDrawParam::itsMinAlpha)
      itsAlpha = NFmiDrawParam::itsMinAlpha;
    if (itsAlpha > 100.f)
      itsAlpha = 100.f;
  }

  bool ViewMacroDrawParam() const { return fViewMacroDrawParam; }
  void ViewMacroDrawParam(bool newState) { fViewMacroDrawParam = newState; }
  const std::string& MacroParamRelativePath() const { return itsMacroParamRelativePath; }
  void MacroParamRelativePath(const std::string& newValue) { itsMacroParamRelativePath = newValue; }
  bool BorrowedParam() const { return fBorrowedParam; }
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
  void UseViewMacrosSettingsForMacroParam(bool newValue)
  {
    fUseViewMacrosSettingsForMacroParam = newValue;
  }
  bool DoSparseSymbolVisualization() const { return fDoSparseSymbolVisualization; }
  void DoSparseSymbolVisualization(bool newValue) { fDoSparseSymbolVisualization = newValue; }
  NFmiMetEditorTypes::View GetViewType(bool isStationData) const;
  static bool IsColorContourType(NFmiMetEditorTypes::View viewType);
  static bool IsIsolineType(NFmiMetEditorTypes::View viewType);
  bool DoIsoLineColorBlend() const { return fDoIsoLineColorBlend; }
  void DoIsoLineColorBlend(bool newValue) { fDoIsoLineColorBlend = newValue; }
  bool ShowContourLegendPotentially() const;
  bool TreatWmsLayerAsObservation() const { return fTreatWmsLayerAsObservation; }
  void TreatWmsLayerAsObservation(bool newValue) { fTreatWmsLayerAsObservation = newValue; }
  int FixedTextSymbolDrawLength() const { return itsFixedTextSymbolDrawLength; }
  void FixedTextSymbolDrawLength(int newValue);
  bool IsFixedTextSymbolDrawLengthUsed() const;
  double SymbolDrawDensityX() const { return itsSymbolDrawDensityX; }
  void SymbolDrawDensityX(double newValue);
  double SymbolDrawDensityY() const { return itsSymbolDrawDensityY; }
  void SymbolDrawDensityY(double newValue);
  const std::string& PossibleColorValueParameter() const { return itsPossibleColorValueParameter; }
  void PossibleColorValueParameter(const std::string& newValue);
  bool IsPossibleColorValueParameterValid() const;
  bool FlipArrowSymbol() const { return fFlipArrowSymbol; }
  void FlipArrowSymbol(bool newState) { fFlipArrowSymbol = newState; }

  static std::string MetTime2String(const NFmiMetTime& theTime);
  static NFmiMetTime String2MetTime(const std::string& theStr);
  static std::string Color2String(const NFmiColor& theColor);
  static NFmiColor String2Color(const std::string& theColorString);

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

  NFmiMetEditorTypes::View
      itsGridDataPresentationStyle;  // isoviiva, color contouring, contour+isoviiva, quick color
                                     // contouring, etc.
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
  // Lisätty 5. väri, jotta simple-isoline no-color-blending piirrot voidaan tehdä
  // oikein (vaatii 4 rajaa ja 5 väriä), kun color-blend on käytössä käytetään 4 rajaa ja 4 väriä
  NFmiColor itsSimpleIsoLineColorShadeHigh3ValueColor;
  int itsSimpleIsoLineColorShadeClassCount;  // kuinka monta väri luokkaa tehdään skaalaukseen
  // speciaali isoviiva asetukset (otetaan käyttöön, jos fUseSimpleIsoLineDefinitions=false)
  std::vector<float>
      itsSpecialIsoLineValues;  // tähän laitetaan kaikki arvot, johon halutaan isoviiva
  std::vector<float> itsSpecialContourValues;       // **** Versio 3 parametri ****
  std::vector<float> itsSpecialIsoLineLabelHeight;  // isoviivalabeleiden korkeudet (0=ei labelia)
  std::vector<float> itsSpecialContourLabelHeight;  // **** Versio 3 parametri ****
  std::vector<float> itsSpecialIsoLineWidth;        // viivan paksuudet
  std::vector<float> itsSpecialContourWidth;        // **** Versio 3 parametri ****
  std::vector<int> itsSpecialIsoLineStyle;          // viiva tyylit
  std::vector<int> itsSpecialContourStyle;          // **** Versio 3 parametri ****
  std::vector<int> itsSpecialIsoLineColorIndexies;  // eri viivojen väri indeksit (pitää tehdä
                                                    // näyttö taulukko käyttäjälle)
  std::vector<int> itsSpecialContourColorIndexies;  // **** Versio 3 parametri ****
  std::vector<bool> itsSpecialIsoLineShowLabelBox;  // eri viivojen väri indeksit (pitää tehdä
                                                    // näyttö taulukko käyttäjälle)
  // colorcontouring ja quick contouring asetukset
  bool fDrawOnlyOverMask;                      // jos true, data piirretään vain maskin päälle
  float itsColorContouringColorShadeLowValue;  // väri skaalaus alkaa tästä arvosta
  float itsColorContouringColorShadeMidValue;  // väri skaalauksen keskiarvo
  float itsColorContouringColorShadeHighValue;
  float itsColorContouringColorShadeHigh2Value;  // väri skaalaus loppuu tähän arvoon
  NFmiColor itsColorContouringColorShadeLowValueColor;
  NFmiColor itsColorContouringColorShadeMidValueColor;
  NFmiColor itsColorContouringColorShadeHighValueColor;
  NFmiColor itsColorContouringColorShadeHigh2ValueColor;
  // Lisätty 5. väri, jotta simple-contour piirrot voidaan tehdä samaan tyyliin kuin
  // custom-contourit
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
  // Arkistodatan käyttöön liittyviä asetuksia (historialliseen dataan voidaan viitata kahdella eri
  // tavalla) Joko suoraan fixatulla itsModelOriginTime:lla tai itsModelRunIndex:illä, jolla
  // kerrotaan monesko ajo kiinnostaa viimeiseen nähden. Prioriteetti järjestys on, että ensin
  // tarkistetaan löytyykö fiksattu aika (itsModelOriginTime) ja sitten vasta onko suhteellinen.
  bool fBorrowedParam;

  // Jos tässä arvo, etsitään arkistosta (SmartMetin (tulevaisuudessa) oma tai Q2Server)
  // suoraan kyseinen aika. Jos arvo on NFmiDrawParam::gMissingOriginTime, tämä ei ole käytössä.
  NFmiMetTime itsModelOriginTime;

  // Jos tässä on negatiivinen arvo (-1 on edellinen malliajo, -2 on sitä edellinen jne.), haetaan
  // arkistosta dataa. Tämä luku on siis verrattuna kyseisen mallin viimeisimpään data osaan, joka
  // löytyy. Tarkoittaen että jos Hirlamista on pinta ja painepinta datasta tullut jo 06:n ajo ja
  // mallipintadatasta vasta 00, silloin viimeisin ajo on 06 ja -1 viittaa tällöin 00-ajoon.
  // Jos tämä on 0 tai positiivinen, tämä ei ole käytössä.
  int itsModelRunIndex;

  // tähän lasketaan relatiivisen malliajon mukainen
  // origin aika, jotä käytetään sitten mm. tooltipeissä ja muualla
  NFmiMetTime itsModelOriginTimeCalculated;

  // tähän määrätään kuinka monta viimeista ajoa näytetään mallille
  // kerrallaa aikasarjassa. Jos arvo on 0 (default), ei näytetä kuin viimeinen ajo normaalisti.
  int itsTimeSerialModelRunCount;

  // Default is true (like old behaviour). If set false, label boxes will be filled with
  // itsIsolineLabelBoxFillColor.
  bool fUseTransparentFillColor;
  // This is FIX for complicated SmartMet code in case where we wan't to prevent loading
  // macroParameter's drawing settings normal way in NFmiStationView's SetupUsedDrawParam method.
  // AND we want to prevent that some viewMacro loadings are not considered as viewMacros at all
  // (F12 and CTRL + F12 functions in SmartMet). Don't use this for anything else. Right way to
  // correct the problem is huge task (re-write certain parts of SmartMet's code).
  bool fUseViewMacrosSettingsForMacroParam;
  // Sparse symboli piirto tarkoittaa että suurin osa hilan arvoista puuttuu (ainakin pitäisi)
  // ja kaikki ei-puuttuvat arvot piirretään symboleilla oli SmartMetin symboliharvennus asetus
  // käytössä tai ei.
  bool fDoSparseSymbolVisualization;
  // Tästä eteenpäin isoviivojen värejä blendaillaan, vain jos tämän arvo on true.
  // Blendaus välinä tällöin käytetään itsIsolineGab:in arvoa ja blendejä tehdään kahden isoviiva
  // limitin väleihin.
  bool fDoIsoLineColorBlend;
  // SmartMetin animaatioissa ei ole tietoa onko joku Wms layer data havainto vai ei. Wms
  // protokollat eivät mahdollista sellaista tietoa. Siksi käyttäjän pitää kertoa onko jossain Wms
  // layerissä kyse havainnosta. Tällä on merkitystä ainakin kun piirretään animaatioita moodissa,
  // joka seuraa viimeisimpiä havaintodatoja.
  bool fTreatWmsLayerAsObservation = false;
  // Käyttäjä voi säätää vakiopituuden tekstisymboli piirroille.
  // Tämä vaikuttaa symbolien harvennuksen laskuissa, mutta ei pakota itse tekstin pituuteen mitään.
  // Oletus arvo on 0, jolloin asetus ei ole käytössä, arvo jos < 1, ei tätä käytetä ollenkaan.
  int itsFixedTextSymbolDrawLength = DefaultFixedTextSymbolDrawLength;
  // Käyttäjä voi säätää symbolipiirroissa tiheämpää ja harvempaa piirtoa.
  // Oletus arvo on 1, jolloin harvennus ei muutu mitenkään.
  // Arvoavaruus on [0.5 , 1.5].
  double itsSymbolDrawDensityX = DefaultSymbolDrawDensity;
  double itsSymbolDrawDensityY = DefaultSymbolDrawDensity;
  // Käyttäjä voi halutessaan värjatä jonkun datan jonkun parametrin symbolit jollain toisen
  // parametrin arvojen avulla. Seuraavia arvoja voi antaa tekstinä:
  // 1) T tai par4, jolloin väritys haetaan saman tuottajan ja saman levelin lämpötila parametrista
  // 2) (NOT IMPLEMENTED YET) T_925 tai par4_925, jolloin väritys haetaan saman tuottajan halutulta
  // 925 hPa leveliltä lämpötila parametrista 3) (NOT IMPLEMENTED YET) T_ec tai T_ec_925, jolloin
  // väritys haetaan Ecmwf datan pinnasta tai 925 hPa tasosta
  std::string itsPossibleColorValueParameter;
  // Joskus pitää saada nuoli symboli käännettyä 180 astetta.
  bool fFlipArrowSymbol = false;
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
