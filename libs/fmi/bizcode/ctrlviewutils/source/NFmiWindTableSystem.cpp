//© Ilmatieteenlaitos/Marko.
//Original 3.3.2009
//
// Luokka huolehtii Tuulitaulukon asetuksista ja datasta.
//---------------------------------------------------------- NFmiWindTableSystem.cpp

#include "NFmiWindTableSystem.h"
#include "NFmiSettings.h"
#include "NFmiDataStoringHelpers.h"
#include "NFmiMultiPolygonAreaMask.h"
#include "NFmiIndexMaskTools.h"
#include "NFmiFastQueryInfo.h"
#include "catlog/catlog.h"
#include "NFmiPathUtils.h"

#include <fstream>

void NFmiWindTableSystem::AreaMaskData::InitMask(const NFmiGrid &theGrid)
{
	itsCachedMask = NFmiIndexMaskTools::MaskInside(theGrid, itsSvgPath);
}

bool NFmiWindTableSystem::AreaMaskData::Init(const std::string &theInitFileName, const std::string &theName)
{
	try
	{
		itsName = theName;
		itsInitFileName = theInitFileName;

		if(itsInitFileName.empty() == false)
		{
			std::ifstream in(itsInitFileName.c_str(), std::ios::binary);
			if(in)
			{
				in >> itsSvgPath;
				return true;
			}
		}
	}
	catch(...)
	{
	}
	return false;
}


// ************** NFmiWindTableSystem ************************************


NFmiWindTableSystem::NFmiWindTableSystem(void)
:itsAreaMaskDataList()
,itsAreaMaskDataList2()
,itsSelectedAreaMaskList(0)
,itsControlDirectory()
,fViewVisible(false)
,fMustaUpdateTable(true)
,fUseMapTime(false)
,fDoInitializeAreaMasks(true)
,itsStartHourOffset(-1)
,itsEndHourOffset(1)
,itsSettingsKeyBaseStr()
{
}

NFmiWindTableSystem::~NFmiWindTableSystem(void)
{
}

void NFmiWindTableSystem::InitializeFromSettings(const std::string &theSettingsKeyBaseStr)
{
	itsSettingsKeyBaseStr = theSettingsKeyBaseStr;

	itsStartHourOffset = NFmiSettings::Require<int>(theSettingsKeyBaseStr + "StartHourOffset");
	itsEndHourOffset = NFmiSettings::Require<int>(theSettingsKeyBaseStr + "EndHourOffset");
}

// t‰ss‰ asetellaan l‰hinn‰ piirto-ominasuuksia
void NFmiWindTableSystem::Init(const NFmiWindTableSystem &theData)
{
	fViewVisible = theData.fViewVisible;
}

void NFmiWindTableSystem::StoreSettings(void)
{
	NFmiSettings::Set(itsSettingsKeyBaseStr + "StartHourOffset", NFmiStringTools::Convert(itsStartHourOffset), true);
	NFmiSettings::Set(itsSettingsKeyBaseStr + "EndHourOffset", NFmiStringTools::Convert(itsEndHourOffset), true);
}

void NFmiWindTableSystem::Write(std::ostream& os) const
{
//	os << itsSymbolID << " " << itsMessageCategory << " " << itsMinLevelToBeShown << " " << fUse << std::endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiWindTableSystem::Write failed");
}

void NFmiWindTableSystem::Read(std::istream& is)
{
//	int tmpValue = 0;
//	is >> tmpValue >> itsMessageCategory >> itsMinLevelToBeShown >> fUse;

	if(is.fail())
		throw std::runtime_error("NFmiWindTableSystem::Read failed");

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw std::runtime_error("NFmiWindTableSystem::Read failed");
}

void NFmiWindTableSystem::AddAreaMask(std::vector<AreaMaskData> &theAreaMaskDataList, const std::string &theFileName, const std::string &theName)
{
	AreaMaskData tmpMaskData;
    if(tmpMaskData.Init(theFileName, theName))
        theAreaMaskDataList.push_back(tmpMaskData);
    else
        CatLog::logMessage(std::string(__FUNCTION__) + ": unable to read mask file '" + theFileName + "'", CatLog::Severity::Error, CatLog::Category::Configuration);
}

// Lis‰t‰‰n loppuun kenoviiva, jos siell‰ ei jo sellaista ole.
// T‰m‰ on kopsattu bizcode\autocomplete\include\NFmiBetaProductHelperFunctions.h tiedostosta, 
// koska en halunnut autocomplete riippuvuutta t‰nne. Enk‰ halua lis‰t‰ t‰t‰ funktiota newbase tai smarttools kirjastoihin
// jotka ovat editingdatautils kirjaston ainoat riippuvuudet.
static void AddDirectorySeparatorAtEnd(std::string &thePathInOut)
{
    if(thePathInOut.size() && thePathInOut[thePathInOut.size() - 1] != '\\' && thePathInOut[thePathInOut.size() - 1] != '/')
        thePathInOut += kFmiDirectorySeparator;
}

void NFmiWindTableSystem::InitAreaMaskDatas(void)
{
	itsAreaMaskDataList.clear();
    auto usedBaseDirectory = NFmiSettings::Optional<std::string>(itsSettingsKeyBaseStr + "SeaAreaBaseDirectory", "");
    if(usedBaseDirectory.empty())
        usedBaseDirectory = itsControlDirectory + "\\maps\\merialueet\\";
    else
        usedBaseDirectory = PathUtils::getAbsoluteFilePath(usedBaseDirectory, itsControlDirectory);
    ::AddDirectorySeparatorAtEnd(usedBaseDirectory);

	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B1N.svg", "B1N");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B1S.svg", "B1S");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B2.svg", "B2");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B3N.svg", "B3N");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B3S.svg", "B3S");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B4E.svg", "B4E");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B4W.svg", "B4W");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B7W.svg", "B7W");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B7E.svg", "B7E");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B5W.svg", "B5W");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B5E.svg", "B5E");
/*
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B6.svg", "B6");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B8E.svg", "B8E");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B8W.svg", "B8W");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B9.svg", "B9");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B10N.svg", "B10N");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B10S.svg", "B10S");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B11.svg", "B11");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B12.svg", "B12");
	AddAreaMask(itsAreaMaskDataList, usedBaseDirectory + "B13.svg", "B13");
*/
	// t‰ss‰ initialisoidaan lista kaksi, jossa leikattuja merialueita
	itsAreaMaskDataList2.clear();
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B1N_limited.svg", "B1N");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B1S_limited.svg", "B1S");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B2_limited.svg", "B2");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B3N_limited.svg", "B3N");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B3S_limited.svg", "B3S");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B4E_limited.svg", "B4E");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B4W_limited.svg", "B4W");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B7W_limited.svg", "B7W");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B7E_limited.svg", "B7E");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B5W_limited.svg", "B5W");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B5E_limited.svg", "B5E");
/*
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B6.svg", "B6");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B8E.svg", "B8E");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B8W.svg", "B8W");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B9.svg", "B9");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B10N.svg", "B10N");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B10S.svg", "B10S");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B11.svg", "B11");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B12.svg", "B12");
	AddAreaMask(itsAreaMaskDataList2, usedBaseDirectory + "B13.svg", "B13");
*/
}

void NFmiWindTableSystem::InitializeAllAreaMasks(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	if(theInfo && theInfo->Grid())
	{
		std::vector<AreaMaskData> &areaMaskDataList = AreaMaskDataList();
		for(size_t k = 0; k < areaMaskDataList.size(); k++)
			areaMaskDataList[k].InitMask(*(theInfo->Grid()));
	}
	fDoInitializeAreaMasks = false;
}

std::vector<NFmiWindTableSystem::AreaMaskData>& NFmiWindTableSystem::AreaMaskDataList(void) 
{
	if(itsSelectedAreaMaskList == 0)
		return itsAreaMaskDataList;
	else
		return itsAreaMaskDataList2;
}

void NFmiWindTableSystem::SelectedAreaMaskList(int newValue)
{
	if(itsSelectedAreaMaskList != newValue)
	{
		fMustaUpdateTable;
		fDoInitializeAreaMasks = true;
	}
	itsSelectedAreaMaskList = newValue;
}
