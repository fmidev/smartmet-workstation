//© Ilmatieteenlaitos/Marko.
// Original 29.8.2006
// 
// Luokka pit‰‰ huolta helpEditorMoodin asioista
// eli usean editoijan systeemist‰, miss‰ yksi on p‰‰editoija
// ja muita metkuja toisilla editori-koneilla voi osallistua 
// apueditoijina.
//---------------------------------------------------------- NFmiHelpEditorSystem.cpp

#include "NFmiHelpEditorSystem.h"
#include "NFmiSettings.h"

NFmiHelpEditorSystem::NFmiHelpEditorSystem(void)
:fUse(false)
,fHelpEditor(false)
,itsDataPath()
,itsFileNameBase()
,itsInitNameSpace()
{
}

NFmiHelpEditorSystem::~NFmiHelpEditorSystem(void)
{
}

// Settings should be following format
// HelpEditorSystem
// {
//   Use = 1
//   HelpEditor = 0
//   DataPath = P:\data\helpdata
//   FileNameBase = help_pal_data.sqd
//   HelpColor = 0 255 0 // RGB arvot 0-255 skaalassa
// }
//
// and so on. System will try to find as many Producer# as it can. 
// You should put them in rising order in file to make it clearer.
void NFmiHelpEditorSystem::InitFromSettings(const std::string &theInitNameSpace)
{
	itsInitNameSpace = theInitNameSpace;
	try
	{
		fUse = NFmiSettings::Optional<bool>(std::string(theInitNameSpace + "::Use"), false);
		fHelpEditor = NFmiSettings::Optional<bool>(std::string(theInitNameSpace + "::HelpEditor"), false);
		itsDataPath = NFmiSettings::Optional<std::string>(std::string(theInitNameSpace + "::DataPath"), "");
		itsFileNameBase = NFmiSettings::Optional<std::string>(std::string(theInitNameSpace + "::FileNameBase"), "");

		std::string colorStr = NFmiSettings::Optional<std::string>(std::string(theInitNameSpace + "::HelpColor"), "");
		if(colorStr.empty())
			itsHelpColor = NFmiColor(223.f/255.f, 223.f/255.f, 0.f);
		else
		{
			std::vector<std::string> rgbParts = NFmiStringTools::Split(colorStr, " ");
			if(rgbParts.size() != 3)
				throw std::runtime_error(std::string("HelpColor RGB part was invalid, needs three values r,g,b:\n") + colorStr);

			int red = NFmiStringTools::Convert<int>(rgbParts[0]);
			int green = NFmiStringTools::Convert<int>(rgbParts[1]);
			int blue = NFmiStringTools::Convert<int>(rgbParts[2]);
			itsHelpColor = NFmiColor(red/255.f, green/255.f, blue/255.f);
		}
	
	}
	catch(std::exception &e)
	{
		throw std::runtime_error(std::string("Error in NFmiHelpEditorSystem::InitFromSettings:\n") + e.what());
	}
	catch(...)
	{
		throw std::runtime_error(std::string("Unknown Error in NFmiHelpEditorSystem::InitFromSettings"));
	}
}

// Huom! t‰m‰ asettaa kaikki talletettavat asiat settingseihin, mutta ei tee save:a.
void NFmiHelpEditorSystem::StoreSettings(bool fStoreHelpEditorStatus)
{
	NFmiSettings::Set(std::string(itsInitNameSpace + "::Use"), NFmiStringTools::Convert(fUse ? 1 : 0), true);
	if(fStoreHelpEditorStatus) // t‰t‰ ei ehk‰ haluta muistettavaksi asetukseksi
		NFmiSettings::Set(std::string(itsInitNameSpace + "::HelpEditor"), NFmiStringTools::Convert(fHelpEditor ? 1 : 0), true);
	NFmiSettings::Set(std::string(itsInitNameSpace + "::DataPath"), itsDataPath, true);
	NFmiSettings::Set(std::string(itsInitNameSpace + "::FileNameBase"), itsFileNameBase, true);
}
