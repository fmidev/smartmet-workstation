#pragma once

// This class makes the interpolation changes for the wanted parameters to the data that
// SmartMet reads. If doForcedParameterInterpolationChanges_ is on, the changes are made and
// the changed parameters are logged. If that option is not on, only a warning message is logged.
// In addition, if the data has the kFmiWindVectorMS subparameter within the kFmiTotalWindMS combined parameter,
// a linearity correction is always applied to it automatically. Nothing is logged about this.

#include "NFmiParam.h"
#include "NFmiCachedRegistryValue.h"
#include <vector>

class NFmiQueryData;

class NFmiParameterInterpolationFixer
{
	std::string originalCheckedParametersConfigurationValue_;
	std::vector<NFmiParam> checkedParameters_;
    boost::shared_ptr<CachedRegBool> doForcedParameterInterpolationChanges_;
	// Double initialization is not allowed
	bool initialized_ = false;
public:
	NFmiParameterInterpolationFixer();
	~NFmiParameterInterpolationFixer();
	void init();

	bool doForcedParameterInterpolationChanges();
	void doForcedParameterInterpolationChanges(bool newValue);

	void fixCheckedParametersInterpolation(NFmiQueryData* data, const std::string& dataFileName);
private:
	const std::string& makeConfigurationKey() const;
	void doFinalChecksForCheckedParameters() const;
	std::vector<NFmiParam> makeCheckedParametersFromConfigurations(const std::string& configurationKey);
};
