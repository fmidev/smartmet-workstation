// ======================================================================
/*!
 * \file NFmiMapConfigurationSystem.h
 * \brief This class handles met-editors mapsystem. It knows the files 
 * and areas that are configured to the system.
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"

class NFmiMapConfiguration
{
public:
	NFmiMapConfiguration(void);
	void Clear(void);
	//@{ \name Kirjoitus- ja luku-operaatiot
//	std::ostream & Write(std::ostream & file) const;
	std::istream & Read(std::istream & file);
	//@}
	const std::vector<std::string>& MapFileNames(void) const {return itsMapFileNames;}
	const std::vector<int>& MapDrawingStyles(void) const {return itsMapDrawingStyles;}
	const std::vector<std::string>& OverMapDibFileNames(void) const {return itsOverMapDibFileNames;}
	const std::vector<int>& OverMapDibDrawingStyles(void) const {return itsOverMapDibDrawingStyles;}
	const std::string& ProjectionFileName(void) const {return itsProjectionFileName;}
	const std::string& Projection(void) const{ return itsProjection; }
	/**
	 * Sets the projection.
	 */
	void Projection(const std::string& theProjection)
	{
		itsProjection = theProjection;
	}

	/**
	 * Sets the projection filename.
	 */
	void ProjectionFileName(const std::string& theProjectionFileName)
	{
		itsProjectionFileName = theProjectionFileName;
	}

	/**
	 * Adds a map.
	 */
	void AddMap(const std::string& theFilename, int theDrawingStyle)
	{
		itsMapFileNames.push_back(theFilename);
		itsMapDrawingStyles.push_back(theDrawingStyle);
	}
	/**
	 * Adds an "over map dib".
	 */
	void AddOverMapDib(const std::string& theFilename, int theDrawingStyle)
	{
		itsOverMapDibFileNames.push_back(theFilename);
		itsOverMapDibDrawingStyles.push_back(theDrawingStyle);
	}

private:
	void ReadFileNamesAndDrawStyles(std::istream & file, std::vector<std::string> &theFileNames, std::vector<int> &theDrawingStyles);
	void ReadProjectionFileName(std::istream & file, std::string &theFileName);

	std::vector<std::string> itsMapFileNames;
	std::vector<int> itsMapDrawingStyles;
	std::vector<std::string> itsOverMapDibFileNames;
	std::vector<int> itsOverMapDibDrawingStyles;
	std::string itsProjectionFileName;
	std::string itsProjection;
};

//inline std::ostream & operator<<(std::ostream & os, const NFmiMapConfiguration & item)
//{  return item.Write(os);}
inline std::istream & operator>>(std::istream & is, NFmiMapConfiguration & item)
{  return item.Read(is);}

class NFmiMapConfigurationSystem
{
public:
	NFmiMapConfigurationSystem(void);

	const NFmiMapConfiguration& GetMapConfiguration(int theIndex);

	std::vector<NFmiMapConfiguration>::size_type Size() 
	{
		return itsMapConfigurations.size();
	}
	/**
	 * Add a new map configuration to the map configuration system.
	 *
	 * @param theConf map configuration
	 */
	const void AddMapConfiguration(const NFmiMapConfiguration & theConf)
	{
		itsMapConfigurations.push_back(theConf);
	}

	//@{ \name Kirjoitus- ja luku-operaatiot
//	std::ostream & Write(std::ostream & file) const;
	std::istream & Read(std::istream & file);
	//@}
private:
	std::vector<NFmiMapConfiguration> itsMapConfigurations;
};

//inline std::ostream & operator<<(std::ostream & os, const NFmiMapConfigurationSystem & item)
//{  return item.Write(os);}
inline std::istream & operator>>(std::istream & is, NFmiMapConfigurationSystem & item)
{  return item.Read(is);}

