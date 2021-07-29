#include "NFmiFastDrawParamList.h"
#include "NFmiDrawParam.h"
#include "NFmiStringTools.h"

NFmiFastDrawParamList::NFmiFastDrawParamList() = default;
NFmiFastDrawParamList::~NFmiFastDrawParamList() = default;

bool NFmiFastDrawParamList::Find(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fGroundData)
{
	itsCurrentKey = "";
    FastContainer::key_type tmpKey = CalcKey(theDrawParam, fGroundData);
	FastContainer::iterator it = itsContainer.find(tmpKey);
	if(it != itsContainer.end())
	{
		itsCurrentKey = tmpKey;
		return true;
	}
	else
		return false;
}

bool NFmiFastDrawParamList::Find(const NFmiDataIdent& theParam, const NFmiLevel* theLevel, NFmiInfoData::Type theDataType, const std::string &theMacroParamInitFile, bool fUseOnlyParamId)
{
	itsCurrentKey = "";
	FastContainer::key_type tmpKey = CalcKey(theParam, theLevel, theDataType, theMacroParamInitFile, fUseOnlyParamId);
	FastContainer::iterator it = itsContainer.find(tmpKey);
	if(it != itsContainer.end())
	{
		itsCurrentKey = tmpKey;
		return true;
	}
	else
		return false;
}

void NFmiFastDrawParamList::Add(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fGroundData)
{
	itsContainer.insert(FastContainer::value_type(CalcKey(theDrawParam, fGroundData), boost::shared_ptr<NFmiDrawParam>(theDrawParam)));
}

boost::shared_ptr<NFmiDrawParam> NFmiFastDrawParamList::Current()
{
	boost::shared_ptr<NFmiDrawParam> tmpDrawParam;
	try
	{
		tmpDrawParam = itsContainer.at(itsCurrentKey);
	}
	catch(...)
	{
	}
	return tmpDrawParam;
}

size_t NFmiFastDrawParamList::Size()
{
	return itsContainer.size();
}

NFmiFastDrawParamList::FastContainer::iterator NFmiFastDrawParamList::Begin()
{
	return itsContainer.begin();
}

NFmiFastDrawParamList::FastContainer::iterator NFmiFastDrawParamList::End()
{
	return itsContainer.end();
}

// Laskee string avaimen, miss‰ stringi laskettu seuraavasti:
// 'p' + parid + '_' + 't' + prodid + '_' + 'l' + leveltype + '_' + levelvalue + '_' + 'd' + datatype
// Jos fUseOnlyParamId on true, j‰‰ pois kohta ['t' + prodid + '_']
// Jos theLevel on 0-pointteri, j‰‰ pois kohta ['l' + leveltype + '_']
// Jos kyse on macroParamista, tulee avaimen per‰‰n viel‰ '_'+theMacroParamInitFile
std::string NFmiFastDrawParamList::CalcKey(const NFmiDataIdent& theParam, const NFmiLevel* theLevel, NFmiInfoData::Type theDataType, const std::string &theMacroParamInitFile, bool fUseOnlyParamId)
{
	std::string aKey;
	aKey += "p";
	aKey += NFmiStringTools::Convert<unsigned long>(theParam.GetParamIdent());
	aKey += "_";
	if(fUseOnlyParamId == false)
	{
		aKey += "t";
		aKey += NFmiStringTools::Convert<unsigned long>(theParam.GetProducer()->GetIdent());
		aKey += "_";
	}
	if(theLevel)
	{
		aKey += "l";
		aKey += NFmiStringTools::Convert<FmiLevelType>(theLevel->LevelType());
		aKey += "_";
		aKey += NFmiStringTools::Convert<float>(theLevel->LevelValue());
		aKey += "_";
	}
	aKey += "d";
	aKey += NFmiStringTools::Convert<NFmiInfoData::Type>(theDataType);
    if(NFmiDrawParam::IsMacroParamCase(theDataType))
	{
		aKey += "_";
		aKey += theMacroParamInitFile;
	}
	return aKey;
}

// Tekee syhteellisen macroParamin suhteellisen polun alkuosion ilman tiedoston p‰‰tett‰ (.dpa) esim. 
// "macroParm1" tai "MinunMakrot\macroParm1"
std::string NFmiFastDrawParamList::MakeMacroParamRelativeFilePathStart(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    std::string relativeFilePath;
    if(!theDrawParam->MacroParamRelativePath().empty())
    {
        relativeFilePath += theDrawParam->MacroParamRelativePath();
        relativeFilePath += kFmiDirectorySeparator;
    }
    relativeFilePath += theDrawParam->ParameterAbbreviation();
    return relativeFilePath;
}

std::string NFmiFastDrawParamList::CalcKey(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fGroundData)
{
	bool useParIdOnly = (theDrawParam->DataType() == NFmiInfoData::kEditable) || (theDrawParam->DataType() == NFmiInfoData::kCopyOfEdited);
	return CalcKey(theDrawParam->Param(), fGroundData ? 0 : &theDrawParam->Level(), theDrawParam->DataType(), MakeMacroParamRelativeFilePathStart(theDrawParam), useParIdOnly);
}

