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
 *  Eats the spaces out of the stream and returns true if we are not at the end
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
	// If at the end of the stream, it fails
	return false;
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
	// The maximum length a file name including its path can be
	const int maxBufferSize = 512;
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
	// The maximum length a file name including its path can be
	const int maxBufferSize = 512;
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
		guiNamesOut.push_back(PathUtils::getFilename(fileName, false));
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

// Prioritization when making map-layer names for the GUI:
// 1. Descriptive name
// 2. Macro-reference name
// 3. Derive a suitable name from the bitmap's file name
std::string NFmiMapConfiguration::GetBestGuiUsedMapLayerName(size_t layerIndex, bool backgroundMapCase) const
{
	// 1. If a non-missing descriptiveName is found, use it.
	std::string bestGuiUsedname = ::GetLayerTextFromVector(layerIndex, backgroundMapCase ? itsBackgroundMapDescriptiveNames : itsOverlayMapDescriptiveNames);
	if(bestGuiUsedname.empty())
	{
		// 2. If a non-missing macroReferenceName is found, use it.
		bestGuiUsedname = ::GetLayerTextFromVector(layerIndex, backgroundMapCase ? itsBackgroundMapMacroReferenceNames : itsOverlayMapMacroReferenceNames);
		if(bestGuiUsedname.empty())
		{
			// 3. Otherwise make a name from the image's file name
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
