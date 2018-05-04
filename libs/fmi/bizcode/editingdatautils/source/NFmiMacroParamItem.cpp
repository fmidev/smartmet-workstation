#include "NFmiMacroParamItem.h"
#include "NFmiMacroParam.h"

bool operator==(const NFmiMacroParamItem &obj1, const NFmiMacroParamItem &obj2)
{
	if(&obj1 ==  &obj2)
		return true;
	if(obj1.itsMacroParam && obj2.itsMacroParam)
	{
		if(*(obj1.itsMacroParam.get()) == *(obj2.itsMacroParam.get()))
		{
			if(obj1.itsDirectoryItems == obj2.itsDirectoryItems)
				return true;
		}
	}
	return false;
}
