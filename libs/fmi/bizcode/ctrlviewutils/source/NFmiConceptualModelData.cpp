//© Ilmatieteenlaitos/Marko.
// Original 19.2.2008
// 
// Luokka hanskaa käsitemallit eli rintamat sun muut. Tähän on
// koottu niin asetukset, että datan luku tietokannasta ja niiden tulkinta.
//---------------------------------------------------------- NFmiConceptualModelData.cpp


#include "NFmiConceptualModelData.h"
#include "NFmiSettings.h"
#include "SettingsFunctions.h"

NFmiConceptualModelData::NFmiConceptualModelData(void)
:fUse(false)
,itsDefaultUserName()
,itsDataBaseUrl()
,fUseFileData(false)
,fUseWOML(false)
,itsFileFilter()
{}

NFmiConceptualModelData::~NFmiConceptualModelData(void)
{
}

void NFmiConceptualModelData::InitFromSettings(const std::string &theInitNameSpace)
{
	fUse = NFmiSettings::Optional<bool>(std::string(theInitNameSpace + "::Use"), false);
	itsDefaultUserName = NFmiSettings::Optional<std::string>(theInitNameSpace + "::DefaultUserName", "");
	itsDataBaseUrl = SettingsFunctions::GetUrlFromSettings(theInitNameSpace + "::DataBaseUrl", true, "");

	fUseFileData = NFmiSettings::Optional<bool>(std::string(theInitNameSpace + "::UseFileData"), false);
	fUseWOML = NFmiSettings::Optional<bool>(std::string(theInitNameSpace + "::UseWOML"), false);
	itsFileFilter = NFmiSettings::Optional<std::string>(theInitNameSpace + "::FileFilter", "");
}

