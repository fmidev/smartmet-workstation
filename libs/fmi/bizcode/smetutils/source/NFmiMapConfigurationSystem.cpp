// ======================================================================
/*!
 * \file NFmiMapConfigurationSystem.cpp
 * \brief This class handles met-editors mapsystem. It knows the files 
 * and areas that are configured to the system.
 */
// ======================================================================

#include "NFmiMapConfigurationSystem.h"
#include "NFmiPathUtils.h"

#include <iostream>

// ----------------------------------------------------------------------
/*!
 *  syö spacet pois streamista ja palauttaa true:n jos ei olla lopussa
 *
 * \param theInput The input stream
 * \return Undocumented
 */
// ----------------------------------------------------------------------
static bool EatWhiteSpaces(std::istream & theInput)
{
  char ch = '\0';
  do
	{
	  ch = theInput.get();
	}
  while(isspace(ch));
  if(theInput.fail())
	return false; // jos stremin lopussa, epäonnistuu
  else
	theInput.unget();
  return true;
}

// ************************************************************************
// ******************* NFmiMapConfiguration ***********************************
// ************************************************************************

NFmiMapConfiguration::NFmiMapConfiguration() = default;
NFmiMapConfiguration::~NFmiMapConfiguration() = default;

void NFmiMapConfiguration::ReadFileNamesAndDrawStyles(std::istream & file, std::vector<std::string> &theFileNames, std::vector<int> &theDrawingStyles)
{
	const int maxBufferSize = 512; // kuinka pitkä tiedoston nimi voi olla polkuineen maksimissaan
	int mapCount = 0;
	file >> mapCount;
	std::string buffer;
	buffer.resize(maxBufferSize+1);
	int i = 0;
	int drawStyle = 0;
	char ch = '\0';
	for(i=0; i<mapCount; i++)
	{
		::EatWhiteSpaces(file);
		file.getline(&buffer[0], maxBufferSize);
		theFileNames.push_back(buffer);
		file >> drawStyle;
		theDrawingStyles.push_back(drawStyle);
	}
}
void NFmiMapConfiguration::ReadProjectionFileName(std::istream & file, std::string &theFileName)
{
	const int maxBufferSize = 512; // kuinka pitkä tiedoston nimi voi olla polkuineen maksimissaan
	std::string buffer;
	buffer.resize(maxBufferSize+1);
	::EatWhiteSpaces(file);
	file.getline(&buffer[0], maxBufferSize);
	theFileName = buffer;
}

static void InitializeFileNameBasedGuiNameVector(const std::vector<std::string>& fileNames, std::vector<std::string>& guiNamesOut)
{
	guiNamesOut.clear();
	for(const auto& fileName : fileNames)
		guiNamesOut.push_back(PathUtils::getFilename(fileName));
}

void NFmiMapConfiguration::InitializeFileNameBasedGuiNameVectors()
{
	::InitializeFileNameBasedGuiNameVector(itsMapFileNames, itsBackgroundMapFileNameBasedGuiNames);
	::InitializeFileNameBasedGuiNameVector(itsOverMapDibFileNames, itsOverlayMapFileNameBasedGuiNames);
}

// ************************************************************************
// ******************* NFmiMapConfigurationSystem *****************************
// ************************************************************************

NFmiMapConfigurationSystem::NFmiMapConfigurationSystem(void)
:itsMapConfigurations()
{}

std::shared_ptr<NFmiMapConfiguration>& NFmiMapConfigurationSystem::GetMapConfiguration(int theIndex)
{
	if(!itsMapConfigurations.empty() && theIndex >= 0 && theIndex < static_cast<int>(itsMapConfigurations.size()))
		return itsMapConfigurations[theIndex];
	else
	{
		static std::shared_ptr<NFmiMapConfiguration> dummy = std::make_shared<NFmiMapConfiguration>();
		return dummy;
	}
}
