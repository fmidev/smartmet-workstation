#pragma once

#include "NFmiInfoData.h"

#include "boost/shared_ptr.hpp"
#include <unordered_map>

class NFmiDrawParam;
class NFmiDataIdent;
class NFmiLevel;

// T‰m‰ luokka on tehty korvaamaan NFmiDrawParamList-luokka kun sit‰ k‰ytet‰‰n
// dokumnetti luokassa itsModifiedPropertiesDrawParamList toiminnassa.
// T‰st‰ on tullut liian hidas, koska listassa on tuhansia drawParameja. Etsint‰ on ollut
// aina lineaarinen ja listasta etsit‰‰ kokoajan tavaraa, kun tulee uutta dataa ja
// sit‰ tulee jatkuvasti.
// Toteutin luokan hash table:na eli hakuaika on periaatteessa aina vakio.
// K‰ytet‰‰n containerin toteutuksena boost:in unordered_map -luokkaa.
class NFmiFastDrawParamList
{
	using FastContainer = std::unordered_map<std::string, boost::shared_ptr<NFmiDrawParam>>;

	FastContainer itsContainer;
	FastContainer::key_type itsCurrentKey; // findilla etsityn drawParamin avain (iteraattoria ei saa ottaa talteen koska se voi invalidoitua!!!!)
public:

	NFmiFastDrawParamList();
	~NFmiFastDrawParamList();

	bool Find(boost::shared_ptr<NFmiDrawParam>& theDrawParam, bool fGroundData);
	bool Find(const NFmiDataIdent& theParam, const NFmiLevel* theLevel, NFmiInfoData::Type theDataType, const std::string& theMacroParamInitFile, bool fUseOnlyParamId);
	void Add(boost::shared_ptr<NFmiDrawParam>& theDrawParam, bool fGroundData);
	boost::shared_ptr<NFmiDrawParam> Current();
	size_t Size();
	FastContainer::iterator Begin();
	FastContainer::iterator End();

private:
	std::string CalcKey(const NFmiDataIdent& theParam, const NFmiLevel* theLevel, NFmiInfoData::Type theDataType, const std::string& theMacroParamInitFile, bool fUseOnlyParamId);
	std::string MakeMacroParamRelativeFilePathStart(boost::shared_ptr<NFmiDrawParam>& theDrawParam);
	std::string CalcKey(boost::shared_ptr<NFmiDrawParam>& theDrawParam, bool fGroundData);
};
