#pragma once

#include "NFmiInfoData.h"
#include <string>

enum class NFmiCaseStudyDataCategory
{
	Error = 0,
	Model,
	Observation,
	Analyze,
	Edited,
	SatelImage,
	CustomFolder
};

class NFmiCategoryHeaderInitData
{
public:
	std::string uniqueName;
	NFmiInfoData::Type dataType = NFmiInfoData::kNoDataType;
	NFmiCaseStudyDataCategory dataCategory = NFmiCaseStudyDataCategory::Error;

	NFmiCategoryHeaderInitData();
	NFmiCategoryHeaderInitData(const std::string name, NFmiInfoData::Type type, NFmiCaseStudyDataCategory category);
};
