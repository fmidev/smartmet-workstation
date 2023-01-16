#pragma once

#include <string> 

class NFmiHelpDataInfo;

struct NFmiCachedDataFileInfo
{
	NFmiCachedDataFileInfo();
	static bool IsDataCached(const NFmiHelpDataInfo& theDataInfo);

	std::string itsTotalServerFileName; // t‰ss‰ on serverill‰ olevan tiedoston polku
	bool fFilePacked = false; // tieto onko serverill‰ oleva tiedosto pakattu vai ei
	std::string itsTotalCacheFileName; // t‰m‰ on datatiedoston lopullinen polku lokaali cachessa
	std::string itsTotalCacheTmpFileName; // t‰m‰ on tiedoston lokaali tmp hakemiston polku, mist‰ se rename:lla siirret‰‰n lopulliseen paikkaan nimeen
	std::string itsTotalCacheTmpPackedFileName; // jos tiedosto oli pakattu, t‰m‰ on lokaali tmp hakemiston nimi pakatulle tiedostolle, joka sitten puretaan itsTotalCacheTmpFileName:ksi
	double itsFileSizeInMB = 0; // tiedoston koko levyll‰ serverill‰ [MB] (joko pakattu tai ei pakattu koko)
};
