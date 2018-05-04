#pragma once

#include <vector>
#include "boost/shared_ptr.hpp"

class NFmiMacroParam;

// tämän luokan avulla rakennetaan puu, missä on kaikki macroparamit hierarkkisessa puu rakenteessa
// jotka löytyvät juuri hakemiston alta.
class NFmiMacroParamItem
{
public:
	boost::shared_ptr<NFmiMacroParam> itsMacroParam;
	std::vector<NFmiMacroParamItem> itsDirectoryItems; // jos kyseinen macroparam olikin hakemisto, täällä on ne macroparamitemit, jotka ovat siinä hakemistossa
};
bool operator==(const NFmiMacroParamItem &obj1, const NFmiMacroParamItem &obj2);

