// ======================================================================
/*!
 * \file NFmiMapConfigurationSystem.h
 * \brief This class handles met-editors mapsystem. It knows the files 
 * and areas that are configured to the system.
 */
// ======================================================================

#pragma once

#include <vector>
#include <string>
#include <memory>

class NFmiMapConfiguration
{
public:
	NFmiMapConfiguration();
	~NFmiMapConfiguration();
	const std::vector<std::string>& MapFileNames(void) const {return itsMapFileNames;}
	const std::vector<int>& MapDrawingStyles(void) const {return itsMapDrawingStyles;}
	const std::vector<std::string>& OverMapDibFileNames(void) const {return itsOverMapDibFileNames;}
	const std::vector<int>& OverMapDibDrawingStyles(void) const {return itsOverMapDibDrawingStyles;}
	const std::string& ProjectionFileName(void) const {return itsProjectionFileName;}
	const std::string& Projection(void) const{ return itsProjection; }
	void Projection(const std::string& theProjection)	{		itsProjection = theProjection;	}
	void ProjectionFileName(const std::string& theProjectionFileName)	{		itsProjectionFileName = theProjectionFileName;	}
	const std::vector<std::string>& BackgroundMapDescriptiveNames() const { return itsBackgroundMapDescriptiveNames; }
	const std::vector<std::string>& OverlayMapDescriptiveNames() const { return itsOverlayMapDescriptiveNames; }
	const std::vector<std::string>& BackgroundMapMacroReferenceNames() const { return  itsBackgroundMapMacroReferenceNames; }
	const std::vector<std::string>& OverlayMapMacroReferenceNames() const { return  itsOverlayMapMacroReferenceNames; }
	const std::vector<std::string>& BackgroundMapFileNameBasedGuiNames() const { return itsBackgroundMapFileNameBasedGuiNames; }
	const std::vector<std::string>& OverlayMapFileNameBasedGuiNames() const { return itsOverlayMapFileNameBasedGuiNames; }

	void AddMap(const std::string& theFilename, int theDrawingStyle)
	{
		itsMapFileNames.push_back(theFilename);
		itsMapDrawingStyles.push_back(theDrawingStyle);
	}

	void AddOverMapDib(const std::string& theFilename, int theDrawingStyle)
	{
		itsOverMapDibFileNames.push_back(theFilename);
		itsOverMapDibDrawingStyles.push_back(theDrawingStyle);
	}

	void AddBackgroundMapDescriptiveName(const std::string& name) { itsBackgroundMapDescriptiveNames.push_back(name); }
	void AddOverlayMapDescriptiveNames(const std::string& name) { itsOverlayMapDescriptiveNames.push_back(name); }
	void AddBackgroundMapMacroReferenceNames(const std::string& name) { itsBackgroundMapMacroReferenceNames.push_back(name); }
	void AddOverlayMapMacroReferenceNames(const std::string& name) { itsOverlayMapMacroReferenceNames.push_back(name); }
	size_t MapLayersCount() const;
	size_t MapOverlaysCount() const;
	std::string GetBestGuiUsedMapLayerName(size_t layerIndex, bool backgroundMapCase) const;
	const std::string& GetMacroReferenceName(size_t layerIndex, bool backgroundMapCase) const;

	// This must be called once the NFmiMapConfiguration object is otherwise initialized!
	void InitializeFileNameBasedGuiNameVectors();

private:
	void ReadFileNamesAndDrawStyles(std::istream & file, std::vector<std::string> &theFileNames, std::vector<int> &theDrawingStyles);
	void ReadProjectionFileName(std::istream & file, std::string &theFileName);

	std::vector<std::string> itsMapFileNames;
	std::vector<int> itsMapDrawingStyles;
	std::vector<std::string> itsOverMapDibFileNames;
	std::vector<int> itsOverMapDibDrawingStyles;
	std::string itsProjectionFileName;
	std::string itsProjection;
	// Optional name for map layers and overlay images, meant to be used at least with the user interfaces,
	// when the user e.g. switches the base map directly (the names go into a popup menu or tooltip).
	std::vector<std::string> itsBackgroundMapDescriptiveNames;
	std::vector<std::string> itsOverlayMapDescriptiveNames;
	// Optional macro reference name for map layers and overlay images, used to refer to
	// the used map image in connection with view macros. This makes view macro configuration more flexible than referring directly to the image
	// index (in the vector), e.g. if the image order is changed or images are added/removed from the lists.
	std::vector<std::string> itsBackgroundMapMacroReferenceNames;
	std::vector<std::string> itsOverlayMapMacroReferenceNames;
	// File-name based GUI names for the base maps (3rd priority in the list)
	std::vector<std::string> itsBackgroundMapFileNameBasedGuiNames;
	std::vector<std::string> itsOverlayMapFileNameBasedGuiNames;
};

class NFmiMapConfigurationSystem
{
public:
	NFmiMapConfigurationSystem(void);

	std::shared_ptr<NFmiMapConfiguration>& GetMapConfiguration(size_t theIndex);

	std::vector<NFmiMapConfiguration>::size_type Size()
	{
		return itsMapConfigurations.size();
	}
	/**
	 * Add a new map configuration to the map configuration system.
	 *
	 * @param theConf map configuration
	 */
	const void AddMapConfiguration(const std::shared_ptr<NFmiMapConfiguration> & theConf)
	{
		itsMapConfigurations.push_back(theConf);
	}

private:
	std::vector<std::shared_ptr<NFmiMapConfiguration>> itsMapConfigurations;
};

