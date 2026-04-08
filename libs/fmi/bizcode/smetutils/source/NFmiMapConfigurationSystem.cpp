// ======================================================================
/*!
 * \file NFmiMapConfigurationSystem.cpp
 * \brief This class handles met-editors mapsystem. It knows the files 
 * and areas that are configured to the system.
 */
// ======================================================================

#include "NFmiMapConfigurationSystem.h"
#include "NFmiPathUtils.h"

#include <filesystem>
#include <iostream>

// ----------------------------------------------------------------------
/*!
 *  sy� spacet pois streamista ja palauttaa true:n jos ei olla lopussa
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
	return false; // jos stremin lopussa, ep�onnistuu
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
	const int maxBufferSize = 512; // kuinka pitk� tiedoston nimi voi olla polkuineen maksimissaan
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
	const int maxBufferSize = 512; // kuinka pitk� tiedoston nimi voi olla polkuineen maksimissaan
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
		guiNamesOut.push_back(std::filesystem::path(fileName).stem().string());
}

void NFmiMapConfiguration::InitializeFileNameBasedGuiNameVectors()
{
	::InitializeFileNameBasedGuiNameVector(itsMapFileNames, itsBackgroundMapFileNameBasedGuiNames);
	::InitializeFileNameBasedGuiNameVector(itsOverMapDibFileNames, itsOverlayMapFileNameBasedGuiNames);
}

size_t NFmiMapConfiguration::MapLayersCount() const
{
	return itsMapFileNames.size();
}

size_t NFmiMapConfiguration::MapOverlaysCount() const
{
	return itsOverMapDibFileNames.size();
}

static const std::string& GetLayerTextFromVector(size_t layerIndex, const std::vector<std::string>& layerNames)
{
	if(layerIndex < layerNames.size())
		return layerNames[layerIndex];
	else
	{
		static const std::string emptyString;
		return emptyString;
	}
}

// Priorisointi kun tehd��n map-layer nimej� Gui:lle:
// 1. Descriptive name
// 2. Macro-reference name
// 3. V��nnet��n sopiva nimi bitmapin tiedosto nimest�
std::string NFmiMapConfiguration::GetBestGuiUsedMapLayerName(size_t layerIndex, bool backgroundMapCase) const
{
	// 1. Jos l�ytyy ei-puuttuva descriptiveName, k�ytet��n sit�.
	std::string bestGuiUsedname = ::GetLayerTextFromVector(layerIndex, backgroundMapCase ? itsBackgroundMapDescriptiveNames : itsOverlayMapDescriptiveNames);
	if(bestGuiUsedname.empty())
	{
		// 2. Jos l�ytyy ei-puuttuva macroReferenceName, k�ytet��n sit�.
		bestGuiUsedname = ::GetLayerTextFromVector(layerIndex, backgroundMapCase ? itsBackgroundMapMacroReferenceNames : itsOverlayMapMacroReferenceNames);
		if(bestGuiUsedname.empty())
		{
			// 3. Muutoin tehd��n nimi kuvan tiedostonimest�
			bestGuiUsedname = ::GetLayerTextFromVector(layerIndex, backgroundMapCase ? itsBackgroundMapFileNameBasedGuiNames : itsOverlayMapFileNameBasedGuiNames);
		}
	}
	return bestGuiUsedname;
}

const std::string& NFmiMapConfiguration::GetMacroReferenceName(size_t layerIndex, bool backgroundMapCase) const
{
	return ::GetLayerTextFromVector(layerIndex, backgroundMapCase ? itsBackgroundMapMacroReferenceNames : itsOverlayMapMacroReferenceNames);
}


// ************************************************************************
// ******************* NFmiMapConfigurationSystem *****************************
// ************************************************************************

NFmiMapConfigurationSystem::NFmiMapConfigurationSystem(void)
:itsMapConfigurations()
{}

std::shared_ptr<NFmiMapConfiguration>& NFmiMapConfigurationSystem::GetMapConfiguration(size_t theIndex)
{
	if(!itsMapConfigurations.empty() && theIndex < itsMapConfigurations.size())
		return itsMapConfigurations[theIndex];
	else
	{
		static std::shared_ptr<NFmiMapConfiguration> dummy = std::make_shared<NFmiMapConfiguration>();
		return dummy;
	}
}
