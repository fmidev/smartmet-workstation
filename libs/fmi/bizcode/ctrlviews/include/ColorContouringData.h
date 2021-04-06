#pragma once

#include "NFmiColor.h"
#include "NFmiDataIdent.h"
#include "NFmiInfoData.h"
#include "NFmiLevel.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class NFmiDrawParam;

class ContouringJobData
{
public:
    NFmiDataIdent dataIdent_;
    NFmiLevel level_;
    std::string nameAbbreviation_;
    NFmiInfoData::Type dataType_ = NFmiInfoData::kNoDataType;
    int viewIndex_ = 0;
    int rowIndex_ = 0;
    int layerIndex_ = 0;
};

class ColorContouringData
{
    std::vector<float> originalClassLimits_;
    std::vector<int> originalColorIndexies_;
    std::vector<NFmiColor> originalColors_;
    // Oletusarvo 0 tarkoittaa että steppiä ei ole asetettu eikä sitä käytetä
    float originalBlendingStep_ = 0; 
    bool useColorBlending_ = false;
    std::vector<float> finalClassLimits_;
    std::vector<int> finalColorIndexies_;
    std::vector<NFmiColor> finalColors_;
    float finalBlendingStep_ = 0;
    bool useDefaultColorTable_ = true;
    bool isCorrectlyInitialized_ = false;
    std::string initializationErrorMessage_;
    std::vector<float> neededStepsBetweenLimits_;
    std::vector<int> colorStepsBetweenLimitColors_;
    int separationLineStyle_ = 0;
    ContouringJobData contouringJobData_;
    // On turha tehdä liikaa blendattuja steppejä, tähän laitetaan limiitti sille.
    int maximumAllowedLimitCount_ = 715;
public:
    ColorContouringData();

    bool initialize(const ContouringJobData& contouringJobData, const boost::shared_ptr<NFmiDrawParam>& drawParam);
    bool initializeForLegendCalculations(const boost::shared_ptr<NFmiDrawParam>& drawParam, int maxAllowedLimits);
    bool isCorrectlyInitialized() const { return isCorrectlyInitialized_; }
    const std::string& initializationErrorMessage() const { return initializationErrorMessage_; }
    const std::vector<float>& finalClassLimits() const { return finalClassLimits_; }
    std::vector<float>& finalClassLimits() { return finalClassLimits_; }
    const std::vector<int>& finalColorIndexies() const { return finalColorIndexies_; }
    std::vector<int>& finalColorIndexies() { return finalColorIndexies_; }
    const std::vector<NFmiColor>& finalColors() const { return finalColors_; }
    float finalBlendingStep() const { return finalBlendingStep_; }
    bool useDefaultColorTable() const { return useDefaultColorTable_ ; }
    bool createNewToolMasterColorTable(int colorTableIndex);
    int separationLineStyle() const { return separationLineStyle_; }
    bool useColorBlending() const { return useColorBlending_; }

private:
    bool initialize(const std::vector<float>& classLimits, const std::vector<NFmiColor>& colors, float step, int separationLineStyle);
    bool initialize(const std::vector<float>& classLimits, const std::vector<int>& colorIndexies, float step);
    bool doSimpleContourInitialization();
    bool doCustomContourInitialization();
    bool checkIfColorBlendingIsUsed() const;
    bool doFinalLimitsAndColorsChecks();
    void missingLimitCleanUp();
    bool doSimpleContourNonBlendingSetups();
    bool doSimpleContourBlendingSetups();
    std::vector<float> calcNeededStepsBetweenLimits();
    std::vector<int> calcColorStepsBetweenLimitColors();
    bool canBlendingBeDoneWithinColorCubeColors();
    bool doBlendingWithinColorCubeColors();
    bool blendingValuesCalculatedCorrectly() const;
    bool doBlendingWithNewColorTable();
    std::string makeJobDataString() const;
    void makePossibleErrorLogging() const;
};
