#include "NFmiCategoryHeaderInitData.h"


NFmiCategoryHeaderInitData::NFmiCategoryHeaderInitData() = default;

NFmiCategoryHeaderInitData::NFmiCategoryHeaderInitData(const std::string name, NFmiInfoData::Type type, NFmiCaseStudyDataCategory category)
    :uniqueName(name)
    ,dataType(type)
    ,dataCategory(category)
{}
