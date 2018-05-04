#include "NFmiDrawParam.h"

#include <fstream>
#include <iostream>
#include <algorithm>

#include <regression/tframe.h>
#include <newbase/NFmiDataIdent.h>
#include <newbase/NFmiLevel.h>
#include <newbase/NFmiDataMatrix.h>

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

//! Protection against conflicts with global functions
namespace NFmiDrawParamTest
{
const std::string className = "NFmiDrawParam";

void printError(const std::string& msg)
{
  std::cerr << "\n\t" << className << " - " << msg << " ";
}

void constructors()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 0);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;

  try
  {
    NFmiDrawParam drawParam;
  }
  catch (...)
  {
    TEST_FAILED("exception: NFmiDrawParam default constructor failed!");
  }

  try
  {
    NFmiDrawParam drawParam(dataIdent, level, priority, dataType);
  }
  catch (...)
  {
    TEST_FAILED("exception: NFmiDrawParam constructor with arguments failed!");
  }

  try
  {
    NFmiDrawParam other;
    NFmiDrawParam second(other);
  }
  catch (...)
  {
    TEST_FAILED("exception: NFmiDrawParam copy constructor failed!");
  }

  NFmiDrawParam dP;

  try
  {
    const NFmiDrawParam* other = new NFmiDrawParam(dataIdent, level, priority, dataType);

    dP.Init(other);

    delete other;
  }
  catch (...)
  {
    TEST_FAILED("exception: NFmiDrawParam Init failed!");
  }

  TEST_PASSED();
}

void readWrite()
{
  try
  {
    using SharedNFmiDrawParam = boost::shared_ptr<NFmiDrawParam>;

    const std::string inFilename = "./data/drawparam_1_in.dpa";
    const std::string outFilename = "./data/drawparam_1_out.dpa";

    // Read data from a file
    SharedNFmiDrawParam drawParam(new NFmiDrawParam);
    if (not drawParam->Init(inFilename))
      TEST_FAILED("Initialization of NFmiDrawParam failed!");

    // Write back to a temporary file
    std::ofstream outFStream(outFilename.c_str());
    if (not outFStream.is_open())
      TEST_FAILED("Cannot open a temporary file to write DrawParam data.");
    drawParam->Write(outFStream);
    outFStream.close();

    // Compare the files
    namespace io = boost::iostreams;
    io::mapped_file_source f1(inFilename.c_str());
    io::mapped_file_source f2(outFilename.c_str());
    if (not(f1.size() == f2.size() && std::equal(f1.data(), f1.data() + f1.size(), f2.data())))
    {
      std::ostringstream msg;
      msg << "Original data from '" << inFilename.c_str() << "' file was wrote to '"
          << outFilename.c_str() << "' file. The files are not equal.";
      TEST_FAILED(msg.str());
    }
    std::remove(outFilename.c_str());
  }
  catch (...)
  {
    printError("Init and Write");
    throw;
  }

  TEST_PASSED();
}

void generalData()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 5);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;
  NFmiMetTime metTime(2017, 3, 8);

  NFmiDrawParam dP(dataIdent, level, priority, dataType);

  try
  {
    dP.Param(param);
    if (dP.Param().GetParamIdent() != dataIdent.GetParamIdent())
      TEST_FAILED("Value is not 10");
  }
  catch (...)
  {
    printError("Param - GetParamIdent");
    throw;
  }

  try
  {
    dP.Level(level);
    if (dP.Level().LevelValue() != 5)
      TEST_FAILED("Value is not 5");
  }
  catch (...)
  {
    printError("Level - LevelValue");
    throw;
  }

  try
  {
    dP.ParameterAbbreviation("FooBar");
    if (dP.ParameterAbbreviation() != "FooBar")
      TEST_FAILED("Value is not FooBar");
  }
  catch (...)
  {
    printError("ParameterAbbreviation");
  }

  try
  {
    dP.InitFileName("FooBar.name");
    if (dP.InitFileName() != "FooBar.name")
      TEST_FAILED("Value is not FooBar.name");
  }
  catch (...)
  {
    printError("InitFileName");
    throw;
  }

  try
  {
    dP.MacroParamRelativePath("/fOO/Bar");
    if (dP.MacroParamRelativePath() != "/fOO/Bar")
      TEST_FAILED("Value is not /fOO/Bar");
  }
  catch (...)
  {
    printError("MacroParamRelativePath");
    throw;
  }

  try
  {
    if (dP.FileVersionNumber() != 3.0)
      TEST_FAILED("Value is not 3.0");
  }
  catch (...)
  {
    printError("FileVersionNumber");
    throw;
  }

  try
  {
    dP.EditParam(true);
    if (not dP.IsParamEdited())
      TEST_FAILED("It is not edited");
  }
  catch (...)
  {
    printError("IsParamEdited");
    throw;
  }

  try
  {
    dP.Unit("ug/m2");
    if (dP.Unit() != "ug/m2")
      TEST_FAILED("Value is not ug/m2");
  }
  catch (...)
  {
    printError("Unit");
    throw;
  }

  try
  {
    dP.Activate(true);
    if (not dP.IsActive())
      TEST_FAILED("It is not active");
  }
  catch (...)
  {
    printError("Activate");
    throw;
  }

  try
  {
    dP.DataType(NFmiInfoData::Type::kViewable);
    if (dP.DataType() != NFmiInfoData::Type::kViewable)
      TEST_FAILED("Value is not NFmiInfoData::Type::kViewable");
  }
  catch (...)
  {
    printError("DataType");
    throw;
  }

  try
  {
    dP.ViewMacroDrawParam(true);
    if (not dP.ViewMacroDrawParam())
      TEST_FAILED("It is not ViewMacroDrawParam");
  }
  catch (...)
  {
    printError("ViewMacroDrawParam");
    throw;
  }

  try
  {
    dP.BorrowedParam(true);
    if (not dP.BorrowedParam())
      TEST_FAILED("It is not BorrowedParam");
  }
  catch (...)
  {
    printError("BorrowedParam");
    throw;
  }

  try
  {
    dP.ModelOriginTime(metTime);
    if (boost::posix_time::to_simple_string(dP.ModelOriginTime().PosixTime()) !=
        "2017-Mar-08 00:00:00")
      TEST_FAILED("Value is not 2017-Mar-08 00:00:00");
  }
  catch (...)
  {
    printError("ModelOriginTime");
    throw;
  }

  try
  {
    dP.ModelRunIndex(1);
    if (dP.ModelRunIndex() != 1)
      TEST_FAILED("Value is not 1");
  }
  catch (...)
  {
    printError("ModelRunIndex");
    throw;
  }

  try
  {
    dP.ModelOriginTimeCalculated(metTime);
    if (dP.ModelOriginTimeCalculated() != metTime)
      TEST_FAILED("Value is not correct");
  }
  catch (...)
  {
    printError("ModelOriginTimeCalculated");
    throw;
  }

  try
  {
    dP.TimeSerialModelRunCount(2);
    if (dP.TimeSerialModelRunCount() != 2)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("TimeSerialModelRunCount");
    throw;
  }

  try
  {
    dP.ModelRunDifferenceIndex(3);
    if (dP.ModelRunDifferenceIndex() != 3)
      TEST_FAILED("Value is not 3");
  }
  catch (...)
  {
    printError("ModelRunDifferenceIndex");
    throw;
  }

  try
  {
    dP.DataComparisonProdId(1);
    if (dP.DataComparisonProdId() != 1)
      TEST_FAILED("Value is not 1");
  }
  catch (...)
  {
    printError("DataComparisonProdId");
    throw;
  }

  try
  {
    dP.DataComparisonType(NFmiInfoData::Type::kStationary);
    if (dP.DataComparisonType() != NFmiInfoData::Type::kStationary)
      TEST_FAILED("Value is not NFmiInfoData::Type::kStationary");
  }
  catch (...)
  {
    printError("DataComparisonType");
    throw;
  }

  TEST_PASSED();
}

void generalVisualization()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 5);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;

  NFmiDrawParam dP(dataIdent, level, priority, dataType);
  try
  {
    dP.ViewType(NFmiMetEditorTypes::View::kFmiIsoLineView);
    if (dP.ViewType() != NFmiMetEditorTypes::View::kFmiIsoLineView)
      TEST_FAILED("Value is not NFmiMetEditorTypes::View::kFmiIsoLineView");
  }
  catch (...)
  {
    printError("ViewType");
    throw;
  }

  try
  {
    const int style = 2;
    dP.GridDataPresentationStyle(style);
    if (dP.GridDataPresentationStyle() != style)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("GridDataPresentationStyle");
    throw;
  }

  try
  {
    dP.UseIsoLineFeathering(true);
    if (not dP.UseIsoLineFeathering())
      TEST_FAILED("Is not enabled");
  }
  catch (...)
  {
    printError("UseIsoLineFeathering");
    throw;
  }

  try
  {
    dP.UseSimpleIsoLineDefinitions(true);
    if (not dP.UseSimpleIsoLineDefinitions())
      TEST_FAILED("Is not enabled");
  }
  catch (...)
  {
    printError("UseSimpleIsoLineDefinitions");
    throw;
  }

  try
  {
    dP.UseSimpleContourDefinitions(true);
    if (not dP.UseSimpleContourDefinitions())
      TEST_FAILED("Is not enabled");
  }
  catch (...)
  {
    printError("UseSimpleContourDefinitions");
    throw;
  }

  try
  {
    dP.UseSeparatorLinesBetweenColorContourClasses(true);
    if (not dP.UseSeparatorLinesBetweenColorContourClasses())
      TEST_FAILED("Is not enabled");
  }
  catch (...)
  {
    printError("UseSeparatorLinesBetweenColorContourClasses");
    throw;
  }

  try
  {
    dP.SimpleIsoLineZeroValue(-1);
    if (dP.SimpleIsoLineZeroValue() != -1)
      TEST_FAILED("Value is not -1");
  }
  catch (...)
  {
    printError("SimpleIsoLineZeroValue");
    throw;
  }

  try
  {
    dP.IsoLineSplineSmoothingFactor(2);
    if (dP.IsoLineSplineSmoothingFactor() != 2)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("IsoLineSplineSmoothingFactor");
    throw;
  }

  try
  {
    dP.DrawOnlyOverMask(true);
    if (not dP.DrawOnlyOverMask())
      TEST_FAILED("Is not enabled");
  }
  catch (...)
  {
    printError("DrawOnlyOverMask");
    throw;
  }

  try
  {
    dP.UseCustomColorContouring(true);
    if (not dP.UseCustomColorContouring())
      TEST_FAILED("Is not enabled");
  }
  catch (...)
  {
    printError("UseCustomColorContouring");
    throw;
  }

  try
  {
    dP.UseCustomIsoLineing(true);
    if (not dP.UseCustomIsoLineing())
      TEST_FAILED("Is not enabled");
  }
  catch (...)
  {
    printError("UseCustomIsoLineing");
    throw;
  }

  try
  {
    dP.Alpha(20.0);
    if (dP.Alpha() != 20.0)
      TEST_FAILED("Value is noe 20.0");
  }
  catch (...)
  {
    printError("Alpha");
    throw;
  }

  try
  {
    dP.HideParam(true);
    if (not dP.IsParamHidden())
      TEST_FAILED("Is not hidden");
    dP.HideParam(false);
  }
  catch (...)
  {
    printError("HideParam");
    throw;
  }

  try
  {
    dP.ShowDifference(true);
    if (not dP.ShowDifference())
      TEST_FAILED("Does not show difference");
    dP.ShowDifference(false);
  }
  catch (...)
  {
    printError("ShowDifference");
    throw;
  }

  try
  {
    dP.ShowDifferenceToOriginalData(true);
    if (not dP.ShowDifferenceToOriginalData())
      TEST_FAILED("Does not show difference");
    dP.ShowDifferenceToOriginalData(false);
  }
  catch (...)
  {
    printError("ShowDifferenceToOriginalData");
    throw;
  }

  TEST_PASSED();
}

void simpleIsoline()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 5);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;

  NFmiDrawParam dP(dataIdent, level, priority, dataType);
  try
  {
    NFmiColor color(1.0, 0.0, 0.0, 0.5);
    dP.IsolineLabelBoxFillColor(color);
    if (dP.IsolineLabelBoxFillColor() != color)
      TEST_FAILED("Color is not red");
  }
  catch (...)
  {
    printError("IsolineLabelBoxFillColor");
    throw;
  }

  try
  {
    NFmiColor color(1.0, 0.0, 0.0, 0.5);
    dP.ContourLabelBoxFillColor(color);
    if (dP.ContourLabelBoxFillColor() != color)
      TEST_FAILED("Color is not red");
  }
  catch (...)
  {
    printError("ContourLabelBoxFillColor");
    throw;
  }

  try
  {
    dP.IsoLineGab(0.1);
    if (dP.IsoLineGab() != 0.1)
      TEST_FAILED("Value is not 0.1");
  }
  catch (...)
  {
    printError("IsoLineGab");
    throw;
  }

  try
  {
    NFmiColor color(1.0, 0.0, 0.0, 0.5);
    dP.IsolineColor(color);
    if (dP.IsolineColor() != color)
      TEST_FAILED("Color is not red");
  }
  catch (...)
  {
    printError("IsoLineColor");
    throw;
  }

  try
  {
    NFmiColor color(1.0, 0.0, 0.0, 0.5);
    dP.IsolineTextColor(color);
    if (dP.IsolineTextColor() != color)
      TEST_FAILED("Color is not red");
  }
  catch (...)
  {
    printError("IsoLineTextColor");
    throw;
  }

  try
  {
    dP.SimpleIsoLineGap(0.1f);
    if (dP.SimpleIsoLineGap() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("SimpleIsoLineGab");
    throw;
  }

  try
  {
    dP.SimpleIsoLineLabelHeight(0.1f);
    if (dP.SimpleIsoLineLabelHeight() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("SimpleIsoLineLabelHeight");
    throw;
  }

  try
  {
    dP.ShowSimpleIsoLineLabelBox(true);
    if (not dP.ShowSimpleIsoLineLabelBox())
      TEST_FAILED("Is not enabled");
    dP.ShowSimpleIsoLineLabelBox(false);
  }
  catch (...)
  {
    printError("ShowSimpleIsoLineLabelBox");
    throw;
  }

  try
  {
    dP.SimpleIsoLineWidth(0.1f);
    if (dP.SimpleIsoLineWidth() != 0.1f)
      TEST_FAILED("Value is not 0.1");
  }
  catch (...)
  {
    printError("SimpleIsoLineWidth");
    throw;
  }

  try
  {
    dP.SimpleIsoLineLineStyle(2);
    if (dP.SimpleIsoLineLineStyle() != 2)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("SimpleIsoLineLineStyle");
    throw;
  }

  try
  {
    dP.UseSingleColorsWithSimpleIsoLines(true);
    if (not dP.UseSingleColorsWithSimpleIsoLines())
      TEST_FAILED("Value is not enebled");
    dP.UseSingleColorsWithSimpleIsoLines(false);
  }
  catch (...)
  {
    printError("UseSingleColorsWithSimpleIsoLines");
    throw;
  }

  try
  {
    dP.SimpleIsoLineColorShadeLowValue(0.1f);
    if (dP.SimpleIsoLineColorShadeLowValue() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("SimpleIsoLineColorShadeLowValue");
    throw;
  }

  try
  {
    dP.SimpleIsoLineColorShadeMidValue(0.2f);
    if (dP.SimpleIsoLineColorShadeMidValue() != 0.2f)
      TEST_FAILED("Value is not 0.2f");
  }
  catch (...)
  {
    printError("SimpleIsoLineColorShadeMidValue");
    throw;
  }

  try
  {
    dP.SimpleIsoLineColorShadeHighValue(0.3f);
    if (dP.SimpleIsoLineColorShadeHighValue() != 0.3f)
      TEST_FAILED("Value is not 0.3f");
  }
  catch (...)
  {
    printError("SimpleIsoLineColorShadeHighValue");
    throw;
  }

  try
  {
    dP.SimpleIsoLineColorShadeHigh2Value(0.2f);
    if (dP.SimpleIsoLineColorShadeHigh2Value() != 0.2f)
      TEST_FAILED("Value is not 0.2f");
  }
  catch (...)
  {
    printError("SimpleIsoLineColorShadeHigh2Value");
    throw;
  }

  try
  {
    NFmiColor color(1.0, 0.0, 0.0, 0.5);
    dP.SimpleIsoLineColorShadeLowValueColor(color);
    if (dP.SimpleIsoLineColorShadeLowValueColor() != color)
      TEST_FAILED("Color is not red");
  }
  catch (...)
  {
    printError("SimpleIsoLineColorShadeLowValueColor");
    throw;
  }

  try
  {
    NFmiColor color(1.0, 0.0, 0.0, 0.5);
    dP.SimpleIsoLineColorShadeMidValueColor(color);
    if (dP.SimpleIsoLineColorShadeMidValueColor() != color)
      TEST_FAILED("Color is not red");
  }
  catch (...)
  {
    printError("SimpleIsoLineColorShadeMidValueColor");
    throw;
  }

  try
  {
    NFmiColor color(1.0, 0.0, 0.0, 0.5);
    dP.SimpleIsoLineColorShadeHighValueColor(color);
    if (dP.SimpleIsoLineColorShadeHighValueColor() != color)
      TEST_FAILED("Color is not red");
  }
  catch (...)
  {
    printError("SimpleIsoLineColorShadeHighValueColor");
    throw;
  }

  try
  {
    NFmiColor color(1.0, 0.0, 0.0, 0.5);
    dP.SimpleIsoLineColorShadeHigh2ValueColor(color);
    if (dP.SimpleIsoLineColorShadeHigh2ValueColor() != color)
      TEST_FAILED("Color is not red");
  }
  catch (...)
  {
    printError("SimpleIsoLineColorShadeHigh2ValueColor");
    throw;
  }

  try
  {
    dP.SimpleIsoLineColorShadeClassCount(3);
    if (dP.SimpleIsoLineColorShadeClassCount() != 3)
      TEST_FAILED("Value is not 3");
  }
  catch (...)
  {
    printError("SimpleIsoLineColorShadeClassCount");
    throw;
  }

  TEST_PASSED();
}

void simpleColorContour()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 5);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;
  NFmiColor color(1.0, 0.0, 0.0, 0.5);

  NFmiDrawParam dP(dataIdent, level, priority, dataType);

  try
  {
    dP.ContourGab(0.1);
    if (dP.ContourGab() != 0.1)
      TEST_FAILED("Value is not 0.1");
  }
  catch (...)
  {
    printError("ContourGab");
    throw;
  }

  try
  {
    dP.ContourColor(color);
    if (dP.ContourColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("ContourColor");
    throw;
  }

  try
  {
    dP.SimpleContourWidth(0.1f);
    if (dP.SimpleContourWidth() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("SimpleContourWidth");
    throw;
  }

  try
  {
    dP.SimpleContourLineStyle(2);
    if (dP.SimpleContourLineStyle() != 2)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("SimpleContourLineStyle");
    throw;
  }

  try
  {
    dP.ColorContouringColorShadeLowValue(0.1f);
    if (dP.ColorContouringColorShadeLowValue() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("ColorContouringColorShadeLowValue");
    throw;
  }

  try
  {
    dP.ColorContouringColorShadeMidValue(0.1f);
    if (dP.ColorContouringColorShadeMidValue() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("ColorContouringColorShadeMidValue");
    throw;
  }

  try
  {
    dP.ColorContouringColorShadeHighValue(0.1f);
    if (dP.ColorContouringColorShadeHighValue() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("ColorContouringColorShadeHighValue");
    throw;
  }

  try
  {
    dP.ColorContouringColorShadeHigh2Value(0.1f);
    if (dP.ColorContouringColorShadeHigh2Value() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("ColorContouringColorShadeHigh2Value");
    throw;
  }

  try
  {
    dP.ColorContouringColorShadeLowValueColor(color);
    if (dP.ColorContouringColorShadeLowValueColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("ColorContouringColorShadeLowValueColor");
    throw;
  }

  try
  {
    dP.ColorContouringColorShadeMidValueColor(color);
    if (dP.ColorContouringColorShadeMidValueColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("ColorContouringColorShadeMidValueColor;");
    throw;
  }

  try
  {
    dP.ColorContouringColorShadeHighValueColor(color);
    if (dP.ColorContouringColorShadeHighValueColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("ColorContouringColorShadeHighValueColor");
    throw;
  }

  try
  {
    dP.ColorContouringColorShadeHigh2ValueColor(color);
    if (dP.ColorContouringColorShadeHigh2ValueColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("ColorContouringColorShadeHigh2ValueColor");
    throw;
  }

  TEST_PASSED();
}

void customIsoline()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 5);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;
  NFmiColor color(1.0, 0.0, 0.0, 0.5);
  checkedVector<float> floatVector;
  floatVector.push_back(-1.0f);
  floatVector.push_back(0.0f);
  floatVector.push_back(1.1f);

  NFmiDrawParam dP(dataIdent, level, priority, dataType);

  try
  {
    dP.SetSpecialIsoLineValues(floatVector);
    if (dP.SpecialIsoLineValues().at(2) != floatVector.at(2))
      TEST_FAILED("Value is not 1.1f");
  }
  catch (...)
  {
    printError("SpecialIsoLineValues");
    throw;
  }

  try
  {
    dP.SetSpecialContourValues(floatVector);
    if (dP.SpecialContourValues().at(2) != floatVector.at(2))
      TEST_FAILED("Value is not 1.1f");
  }
  catch (...)
  {
    printError(" SpecialContourValues");
    throw;
  }

  try
  {
    dP.SetSpecialIsoLineLabelHeight(floatVector);
    if (dP.SpecialIsoLineLabelHeight().at(2) != floatVector.at(2))
      TEST_FAILED("Value is not 1.1f");
  }
  catch (...)
  {
    printError("SpecialIsoLineLabelHeight");
    throw;
  }

  try
  {
    dP.SetSpecialContourLabelHeight(floatVector);
    if (dP.SpecialContourLabelHeight().at(2) != floatVector.at(2))
      TEST_FAILED("Value is not 1.1f");
  }
  catch (...)
  {
    printError("SpecialContourLabelHeight");
    throw;
  }

  try
  {
    dP.SetSpecialIsoLineWidth(floatVector);
    if (dP.SpecialIsoLineWidth().at(2) != floatVector.at(2))
      TEST_FAILED("Value is not 1.1f");
  }
  catch (...)
  {
    printError("SpecialIsoLineWidth");
    throw;
  }

  try
  {
    dP.SetSpecialcontourWidth(floatVector);
    if (dP.SpecialcontourWidth().at(2) != floatVector.at(2))
      TEST_FAILED("Value is not 1.1f");
  }
  catch (...)
  {
    printError("SpecialContourWidth");
    throw;
  }

  checkedVector<int> intVector;
  intVector.push_back(-1);
  intVector.push_back(0);
  intVector.push_back(1);

  try
  {
    dP.SetSpecialIsoLineStyle(intVector);
    if (dP.SpecialIsoLineStyle().at(2) != intVector.at(2))
      TEST_FAILED("Value is not 1");
  }
  catch (...)
  {
    printError("SpecialIsoLineStyle");
    throw;
  }

  try
  {
    dP.SetSpecialContourStyle(intVector);
    if (dP.SpecialContourStyle().at(2) != intVector.at(2))
      TEST_FAILED("Value is not 1");
  }
  catch (...)
  {
    printError("SpecialContourStyle");
    throw;
  }

  try
  {
    dP.SetSpecialIsoLineColorIndexies(intVector);
    if (dP.SpecialIsoLineColorIndexies().at(2) != intVector.at(2))
      TEST_FAILED("Value is not 1");
  }
  catch (...)
  {
    printError("SpecialIsoLineColorIndexies");
    throw;
  }

  try
  {
    dP.SetSpecialContourColorIndexies(intVector);
    if (dP.SpecialContourColorIndexies().at(2) != intVector.at(2))
      TEST_FAILED("Value is not 1");
  }
  catch (...)
  {
    printError("SpecialContourColorIndexies");
    throw;
  }

  checkedVector<bool> boolVector;
  boolVector.push_back(false);
  boolVector.push_back(true);
  boolVector.push_back(false);

  try
  {
    dP.SpecialIsoLineShowLabelBox(boolVector);
    if (dP.SpecialIsoLineShowLabelBox().at(2) != boolVector.at(2))
      TEST_FAILED("Value is not false");
  }
  catch (...)
  {
    printError("SpecialIsoLineShowLabelBox");
    throw;
  }

  TEST_PASSED();
}

void customColorContour()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 5);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;
  NFmiColor color(1.0, 0.0, 0.0, 0.5);

  NFmiDrawParam dP(dataIdent, level, priority, dataType);

  try
  {
    dP.UseIsoLineGabWithCustomContours(true);
    if (not dP.UseIsoLineGabWithCustomContours())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("UseIsoLineGabWithCustomContours");
    throw;
  }

  try
  {
    dP.UseContourGabWithCustomContours(true);
    if (not dP.UseContourGabWithCustomContours())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("UseContourGabWithCustomContours");
    throw;
  }

  checkedVector<float> floatVector;
  floatVector.push_back(-1.0f);
  floatVector.push_back(0.0f);
  floatVector.push_back(1.1f);

  try
  {
    dP.SetSpecialColorContouringValues(floatVector);
    if (dP.SpecialColorContouringValues().at(2) != 1.1f)
      TEST_FAILED("Value is not 1.1f");
  }
  catch (...)
  {
    printError("SpecialColorContouringValues");
    throw;
  }

  checkedVector<int> intVector;
  intVector.push_back(-1);
  intVector.push_back(0);
  intVector.push_back(1);

  try
  {
    dP.SpecialColorContouringColorIndexies(intVector);
    if (dP.SpecialColorContouringColorIndexies().at(2) != 1)
      TEST_FAILED("Value is not 1");
  }
  catch (...)
  {
    printError("SpecialColorContouringColorIndexies");
    throw;
  }

  TEST_PASSED();
}

void symbolSettings()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 5);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;
  NFmiColor color(1.0, 0.0, 0.0, 0.5);
  NFmiMetEditorTypes::View stationDataViewType = NFmiMetEditorTypes::View::kFmiSymbolView;
  NFmiDrawParam dP(dataIdent, level, priority, dataType);

  try
  {
    dP.StationDataViewType(stationDataViewType);
    if (dP.StationDataViewType() != stationDataViewType)
      TEST_FAILED("Value is not kFmiSymbolView");
  }
  catch (...)
  {
    printError("StationDataViewType");
    throw;
  }

  try
  {
    dP.FrameColor(color);
    if (dP.FrameColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("FrameColor");
    throw;
  }

  try
  {
    dP.FillColor(color);
    if (dP.FillColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("FillColor");
    throw;
  }

  try
  {
    NFmiPoint symbolWidthAndHeight(0.5, 0.6);
    dP.RelativeSize(symbolWidthAndHeight);
    if (dP.RelativeSize() != symbolWidthAndHeight)
      TEST_FAILED("Value is not same as the input");
  }
  catch (...)
  {
    printError("dP.RelativeSize");
    throw;
  }

  try
  {
    NFmiPoint relativePositionOffset(0.1, -0.1);
    dP.RelativePositionOffset(relativePositionOffset);
    if (dP.RelativePositionOffset() != relativePositionOffset)
      TEST_FAILED("Value is not same as the input");
  }
  catch (...)
  {
    printError("RelativePositionOffset");
    throw;
  }

  try
  {
    NFmiPoint relativeSize(0.1, 0.05);
    dP.OnlyOneSymbolRelativeSize(relativeSize);
    if (dP.OnlyOneSymbolRelativeSize() != relativeSize)
      TEST_FAILED("Value is not same as the input");
  }
  catch (...)
  {
    printError("OnlyOneSymbolRelativeSize");
    throw;
  }

  try
  {
    NFmiPoint relativePositionOffset(0.1, -0.1);
    dP.OnlyOneSymbolRelativePositionOffset(relativePositionOffset);
    if (dP.OnlyOneSymbolRelativePositionOffset() != relativePositionOffset)
      TEST_FAILED("Value is not same as the input");
  }
  catch (...)
  {
    printError("OnlyOneSymbolRelativePositionOffset");
    throw;
  }

  try
  {
    dP.ShowNumbers(true);
    if (not dP.ShowNumbers())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("ShowNumbers");
    throw;
  }

  try
  {
    dP.ShowMasks(true);
    if (not dP.ShowMasks())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("ShowMasks");
    throw;
  }

  try
  {
    dP.ShowColors(true);
    if (not dP.ShowColors())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("ShowColors");
    throw;
  }

  try
  {
    dP.ShowColoredNumbers(true);
    if (not dP.ShowColoredNumbers())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("ShowColoredNumbers");
    throw;
  }

  try
  {
    dP.ZeroColorMean(true);
    if (not dP.ZeroColorMean())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("ZeroColorMean");
    throw;
  }

  try
  {
    dP.StationSymbolColorShadeLowValue(0.1f);
    if (dP.StationSymbolColorShadeLowValue() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("StationSymbolColorShadeLowValue");
    throw;
  }

  try
  {
    dP.StationSymbolColorShadeMidValue(0.1f);
    if (dP.StationSymbolColorShadeMidValue() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("StationSymbolColorShadeMidValue");
    throw;
  }

  try
  {
    dP.StationSymbolColorShadeHighValue(0.1f);
    if (dP.StationSymbolColorShadeHighValue() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("StationSymbolColorShadeHighValue");
    throw;
  }

  try
  {
    dP.StationSymbolColorShadeLowValueColor(color);
    if (dP.StationSymbolColorShadeLowValueColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("StationSymbolColorShadeLowValueColor");
    throw;
  }

  try
  {
    dP.StationSymbolColorShadeMidValueColor(color);
    if (dP.StationSymbolColorShadeMidValueColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("StationSymbolColorShadeMidValueColor");
    throw;
  }

  try
  {
    dP.StationSymbolColorShadeHighValueColor(color);
    if (dP.StationSymbolColorShadeHighValueColor() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("StationSymbolColorShadeHighValueColor");
    throw;
  }

  try
  {
    dP.StationSymbolColorShadeClassCount(2);
    if (dP.StationSymbolColorShadeClassCount() != 2)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("StationSymbolColorShadeClassCount");
    throw;
  }

  try
  {
    dP.UseSymbolsInTextMode(true);
    if (not dP.UseSymbolsInTextMode())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("UseSymbolsInTextMode");
    throw;
  }

  try
  {
    dP.UsedSymbolListIndex(2);
    if (dP.UsedSymbolListIndex() != 2)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("UsedSymbolListIndex");
    throw;
  }

  try
  {
    dP.SymbolIndexingMapListIndex(2);
    if (dP.SymbolIndexingMapListIndex() != 2)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("SymbolIndexingMapListIndex");
    throw;
  }

  TEST_PASSED();
}

void dataEditing()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 5);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;
  NFmiDrawParam dP(dataIdent, level, priority, dataType);

  try
  {
    dP.ModifyingStep(2.1);
    if (dP.ModifyingStep() != 2.1)
      TEST_FAILED("Value is not 2.1");
  }
  catch (...)
  {
    printError("ModifyingStep");
    throw;
  }

  try
  {
    dP.TimeSerialModifyingLimit(2.1);
    if (dP.TimeSerialModifyingLimit() != 2.1)
      TEST_FAILED("Value is not 2.1");
  }
  catch (...)
  {
    printError("TimeSerialModifyingLimit");
    throw;
  }

  try
  {
    dP.AbsoluteMinValue(1.1);
    if (dP.AbsoluteMinValue() != 1.1)
      TEST_FAILED("Value is not 1.1");
  }
  catch (...)
  {
    printError("AbsoluteMinValue");
    throw;
  }

  try
  {
    dP.AbsoluteMaxValue(3.1);
    if (dP.AbsoluteMaxValue() != 3.1)
      TEST_FAILED("Value is not 3.1");
  }
  catch (...)
  {
    printError("AbsoluteMaxValue");
    throw;
  }

  try
  {
    dP.TimeSeriesScaleMin(0.1);
    if (dP.TimeSeriesScaleMin() != 0.1)
      TEST_FAILED("Value is not 0.1");
  }
  catch (...)
  {
    printError("TimeSeriesScaleMin");
    throw;
  }

  try
  {
    dP.TimeSeriesScaleMax(3.1);
    if (dP.TimeSeriesScaleMax() != 3.1)
      TEST_FAILED("Value is not 3.1");
  }
  catch (...)
  {
    printError("TimeSeriesScaleMax");
    throw;
  }

  TEST_PASSED();
}

void hatchSettings()
{
  NFmiParam param(10);
  NFmiDataIdent dataIdent(param);
  NFmiLevel level(kFmiAnyLevelType, 5);
  int priority = 2;
  NFmiInfoData::Type dataType = NFmiInfoData::kEditable;
  NFmiDrawParam dP(dataIdent, level, priority, dataType);

  NFmiColor color(1.0, 0.0, 0.0, 0.5);

  try
  {
    dP.UseWithIsoLineHatch1(true);
    if (not dP.UseWithIsoLineHatch1())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("UseWithIsoLineHatch1");
    throw;
  }

  try
  {
    dP.DrawIsoLineHatchWithBorders1(true);
    if (not dP.DrawIsoLineHatchWithBorders1())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("DrawIsoLineHatchWithBorders1");
    throw;
  }

  try
  {
    dP.IsoLineHatchLowValue1(0.1f);
    if (dP.IsoLineHatchLowValue1() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("IsoLineHatchLowValue1");
    throw;
  }

  try
  {
    dP.IsoLineHatchHighValue1(0.2f);
    if (dP.IsoLineHatchHighValue1() != 0.2f)
      TEST_FAILED("Value is not 0.2f");
  }
  catch (...)
  {
    printError("IsoLineHatchHighValue1");
    throw;
  }

  try
  {
    dP.IsoLineHatchType1(2);
    if (dP.IsoLineHatchType1() != 2)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("IsoLineHatchType1");
    throw;
  }

  try
  {
    dP.IsoLineHatchColor1(color);
    if (dP.IsoLineHatchColor1() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("IsoLineHatchColor1");
    throw;
  }

  try
  {
    dP.IsoLineHatchBorderColor1(color);
    if (dP.IsoLineHatchBorderColor1() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("IsoLineHatchBorderColor1");
    throw;
  }

  try
  {
    dP.UseWithIsoLineHatch2(true);
    if (not dP.UseWithIsoLineHatch2())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("UseWithIsoLineHatch2");
    throw;
  }

  try
  {
    dP.DrawIsoLineHatchWithBorders2(true);
    if (not dP.DrawIsoLineHatchWithBorders2())
      TEST_FAILED("Value is not true");
  }
  catch (...)
  {
    printError("DrawIsoLineHatchWithBorders2");
    throw;
  }

  try
  {
    dP.IsoLineHatchLowValue2(0.1f);
    if (dP.IsoLineHatchLowValue2() != 0.1f)
      TEST_FAILED("Value is not 0.1f");
  }
  catch (...)
  {
    printError("IsoLineHatchLowValue2");
    throw;
  }

  try
  {
    dP.IsoLineHatchHighValue2(0.2f);
    if (dP.IsoLineHatchHighValue2() != 0.2f)
      TEST_FAILED("Value is not 0.2f");
  }
  catch (...)
  {
    printError("IsoLineHatchHighValue2");
    throw;
  }

  try
  {
    dP.IsoLineHatchType2(2);
    if (dP.IsoLineHatchType2() != 2)
      TEST_FAILED("Value is not 2");
  }
  catch (...)
  {
    printError("IsoLineHatchType2");
    throw;
  }

  try
  {
    dP.IsoLineHatchColor2(color);
    if (dP.IsoLineHatchColor2() != color)
      TEST_FAILED("Value is not red");
  }
  catch (...)
  {
    printError("IsoLineHatchColor2");
    throw;
  }

  TEST_PASSED();
}

void init()
{
}
// ----------------------------------------------------------------------
/*!
 * The actual test suite
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  virtual const char* error_message_prefix() const { return "\n\t"; }
  void test(void)
  {
    TEST(NFmiDrawParamTest::constructors);
    TEST(NFmiDrawParamTest::generalData);
    TEST(NFmiDrawParamTest::readWrite);
    TEST(NFmiDrawParamTest::generalVisualization);
    TEST(NFmiDrawParamTest::simpleIsoline);
    TEST(NFmiDrawParamTest::simpleColorContour);
    TEST(NFmiDrawParamTest::customIsoline);
    TEST(NFmiDrawParamTest::customColorContour);
    TEST(NFmiDrawParamTest::symbolSettings);
    TEST(NFmiDrawParamTest::dataEditing);
    TEST(NFmiDrawParamTest::hatchSettings);
  }
};
}

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiDrawParamTest tester" << endl << "================" << endl;
  NFmiDrawParamTest::tests t;
  return t.run();
}

// ======================================================================
