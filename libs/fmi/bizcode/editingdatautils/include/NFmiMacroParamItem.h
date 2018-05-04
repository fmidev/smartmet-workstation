#pragma once

#include <vector>
#include "boost/shared_ptr.hpp"

class NFmiMacroParam;

// t�m�n luokan avulla rakennetaan puu, miss� on kaikki macroparamit hierarkkisessa puu rakenteessa
// jotka l�ytyv�t juuri hakemiston alta.
class NFmiMacroParamItem
{
public:
	boost::shared_ptr<NFmiMacroParam> itsMacroParam;
	std::vector<NFmiMacroParamItem> itsDirectoryItems; // jos kyseinen macroparam olikin hakemisto, t��ll� on ne macroparamitemit, jotka ovat siin� hakemistossa
};
bool operator==(const NFmiMacroParamItem &obj1, const NFmiMacroParamItem &obj2);

