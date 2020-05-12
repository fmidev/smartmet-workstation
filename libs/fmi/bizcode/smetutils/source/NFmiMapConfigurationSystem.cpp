// ======================================================================
/*!
 * \file NFmiMapConfigurationSystem.cpp
 * \brief This class handles met-editors mapsystem. It knows the files 
 * and areas that are configured to the system.
 */
// ======================================================================

#include "NFmiMapConfigurationSystem.h"

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

NFmiMapConfiguration::NFmiMapConfiguration(void)
:itsMapFileNames()
,itsMapDrawingStyles()
,itsOverMapDibFileNames()
,itsOverMapDibDrawingStyles()
,itsProjectionFileName()
{}

void NFmiMapConfiguration::Clear(void)
{
	itsMapFileNames.clear();
	itsMapDrawingStyles.clear();
	itsOverMapDibFileNames.clear();
	itsOverMapDibDrawingStyles.clear();
	itsProjectionFileName = "";
}

std::istream & NFmiMapConfiguration::Read(std::istream & file)
{
	Clear();
	ReadFileNamesAndDrawStyles(file, itsMapFileNames, itsMapDrawingStyles);
	ReadFileNamesAndDrawStyles(file, itsOverMapDibFileNames, itsOverMapDibDrawingStyles);
	ReadProjectionFileName(file, itsProjectionFileName);
	return file;
}

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


// ************************************************************************
// ******************* NFmiMapConfigurationSystem *****************************
// ************************************************************************

NFmiMapConfigurationSystem::NFmiMapConfigurationSystem(void)
:itsMapConfigurations()
{}

const NFmiMapConfiguration& NFmiMapConfigurationSystem::GetMapConfiguration(int theIndex)
{
	static NFmiMapConfiguration dummy;
	if(!itsMapConfigurations.empty() && theIndex >= 0 && theIndex < static_cast<int>(itsMapConfigurations.size()))
		return itsMapConfigurations[theIndex];
	return dummy;
}

std::istream & NFmiMapConfigurationSystem::Read(std::istream & file)
{
	itsMapConfigurations.clear();
	int configurationCount = 0;
	file >> configurationCount;
	NFmiMapConfiguration confi;
	for(int i=0; i<configurationCount; i++)
	{
		file >> confi;
		itsMapConfigurations.push_back(confi);
	}
	return file;
}

