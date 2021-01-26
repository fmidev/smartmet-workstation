#pragma once

// T‰m‰ luokka tekee haluttujen parametrien interpolaatio muutokset SmartMetin lukemiin
// datoihin. Jos doForcedParameterInterpolationChanges_ on p‰‰ll‰, tehd‰‰n muutokset ja
// lokitetaan muuttuneet parametrit. Jos em. optio ei ole p‰‰ll‰, lokitetaan vain varoitusviesti.
// Lis‰ksi jos datassa on kFmiWindVectorMS aliparametri kFmiTotalWindMS yhdistelm‰ parametrissa,
// tehd‰‰n siihen aina automaattisesti lineaarisuus korjaus. T‰st‰ ei tehd‰ mit‰‰n lokitusta.

#include "NFmiParam.h"
#include "NFmiCachedRegistryValue.h"
#include <vector>

class NFmiQueryData;

class NFmiParameterInterpolationFixer
{
	std::string originalCheckedParametersConfigurationValue_;
	std::vector<NFmiParam> checkedParameters_;
    boost::shared_ptr<CachedRegBool> doForcedParameterInterpolationChanges_;
	// Ei sallita tupla initialisointia
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
