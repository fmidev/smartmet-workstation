#pragma once

#include "ColorContouringData.h"
#include "NFmiColor.h"
#include "NFmiDataIdent.h"
#include "NFmiInfoData.h"
#include "NFmiLevel.h"
#include "NFmiMetEditorTypes.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class NFmiDrawParam;

class IsolineVizualizationData
{
    // Perus isoviiva asetuksia
    std::vector<float> originalClassLimits_;
    std::vector<int> originalColorIndexies_;
    std::vector<NFmiColor> originalColors_;
	bool stationData_ = false;
    NFmiMetEditorTypes::View usedViewType_ = NFmiMetEditorTypes::View::kFmiIsoLineView;
    float usedStep_ = 0;
    float usedZeroLimitValue_ = 0;
    bool customCase_ = false;
    bool useSimpleDefinitions_ = true;
    bool useSingleColor_ = false;
    bool useColorBlending_ = false;
    float dataMin_ = kFloatMissing;
    float dataMax_ = kFloatMissing;
    float splineSmoothingFactor_ = 0;
    std::vector<float> lineWidths_;
    std::vector<int> lineStyles_;
    std::vector<float> labelHeights_;
    int labelBoxFillColorIndex_ = 0;
    int labelTextColorIndex_ = 1;
    int labelDecimalsCount_ = 0;
    bool useFeathering_ = false;
    bool useLabelBox_ = false;

    // Lasketut tai johdetut asetukset alkavat tästä
    float startLimitValue_ = 0;
    int usedIsolineCount_ = 0;
    std::vector<float> finalClassLimits_;
    std::vector<float> finalToolmasterFixedClassLimits_;
    std::vector<int> finalColorIndexies_;
    std::vector<NFmiColor> finalColors_;
    bool isCorrectlyInitialized_ = false;
    std::string initializationErrorMessage_;
    ContouringJobData isolineJobData_;
public:
    IsolineVizualizationData();

    bool initialize(const ContouringJobData& isolineJobData, const boost::shared_ptr<NFmiDrawParam>& drawParam, float dataMin, float dataMax, bool stationData);
    bool isCorrectlyInitialized() const { return isCorrectlyInitialized_; }
    const std::string& initializationErrorMessage() const { return initializationErrorMessage_; }
    const std::vector<float>& finalClassLimits() const { return finalClassLimits_; }
    std::vector<float>& finalClassLimits() { return finalClassLimits_; }
    const std::vector<float>& finalToolmasterFixedClassLimits() const { return finalToolmasterFixedClassLimits_; }
    std::vector<float>& finalToolmasterFixedClassLimits() { return finalToolmasterFixedClassLimits_; }
    const std::vector<int>& finalColorIndexies() const { return finalColorIndexies_; }
    std::vector<int>& finalColorIndexies() { return finalColorIndexies_; }
    const std::vector<NFmiColor>& finalColors() const { return finalColors_; }
    int usedIsolineCount() const { return usedIsolineCount_; }
    float usedStep() const { return usedStep_; }
    bool useSingleColor() const { return useSingleColor_; }
    float startLimitValue() const { return startLimitValue_; }
    float splineSmoothingFactor() const { return splineSmoothingFactor_; }
    const std::vector<float>& lineWidths() const { return  lineWidths_; }
    std::vector<float>& lineWidths() { return  lineWidths_; }
    const std::vector<int>& lineStyles() const { return  lineStyles_; }
    std::vector<int>& lineStyles() { return  lineStyles_; }
    const std::vector<float>& labelHeights() const { return  labelHeights_; }
    std::vector<float>& labelHeights() { return  labelHeights_; }
    int labelBoxFillColorIndex() const { return  labelBoxFillColorIndex_; }
    int labelTextColorIndex() const { return  labelTextColorIndex_; }
    int labelDecimalsCount() const { return  labelDecimalsCount_; }
    bool useFeathering() const { return  useFeathering_; }
    bool useLabelBox() const { return useLabelBox_; }
    std::pair<int, NFmiColor> getMultiColorValues(float limitValue);

private:
    bool initialize(const std::vector<float>& classLimits, const std::vector<NFmiColor>& colors);
    bool initialize(const std::vector<float>& classLimits, const std::vector<int>& colorIndexies);
    bool doSimpleIsolineInitialization();
    bool doCustomIsolineInitialization();
    bool doFinalLimitsAndColorsChecks();
    void missingLimitCleanUp();
    bool calculateIsolineStartAndCount();
    void makePossibleErrorLogging() const;
    void justCopyFinalValues();
    void calculateNonBlendingColors();
    void calculateBlendedColors();
    std::pair<int, NFmiColor> getBlendedColorValues(float limitValue);
    void initFromDrawParam(const boost::shared_ptr<NFmiDrawParam>& drawParam);
    std::pair<std::vector<float>, std::vector<NFmiColor>> getSimpleColors(const boost::shared_ptr<NFmiDrawParam>& drawParam);
    std::pair<std::vector<float>, std::vector<int>> getCustomColors(const boost::shared_ptr<NFmiDrawParam>& drawParam);
    void padCustomAttributes();
    void calcFixedToolmasterContourLimits();
};
