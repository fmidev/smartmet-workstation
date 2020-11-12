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

	// Tätä pitää kutsua, kun NFmiMapConfiguration olio on muuten alustettu!
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
	// Optionaalinen nimi karttalayereille ja overlay kuville, jota on tarkoitus käyttää ainakin käyttöliittymien kanssa, 
	// kun käyttäjä esim. vaihtaa karttapohjaa suoraan (nimet menee popup valikkoon tai tooltippiin).
	std::vector<std::string> itsBackgroundMapDescriptiveNames;
	std::vector<std::string> itsOverlayMapDescriptiveNames;
	// Optionaalinen makro referenssi nimi karttalayereille ja overlay kuville, jota käytetään viittäämään
	// käytettyyn karttakuvaan näyttömakrojen yhteydessä. Tällöin näyttömakron säätö on joustavampaa kuin suoraan kuvan 
	// indeksiin (vektorissa) viittaaminen, jos esim. vaihdetaan kuvien järjestystä tai lisätään/poistetaan kuvia listoilta.
	std::vector<std::string> itsBackgroundMapMacroReferenceNames;
	std::vector<std::string> itsOverlayMapMacroReferenceNames;
	// Tiedostonimi pohjaiset karttapohjien Gui nimet (3. prioriteetti listalla)
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

