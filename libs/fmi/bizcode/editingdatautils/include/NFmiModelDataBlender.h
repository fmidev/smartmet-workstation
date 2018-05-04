// ======================================================================
/*!
 * \file NFmiModelDataBlender.h
 * \brief Class that handles information that involves the mixing of two
 *  model data in time.
 */
// ======================================================================

#pragma once

#include <vector>
#include "NFmiTimeDescriptor.h"

class NFmiFastQueryInfo;

class NFmiModelDataBlender
{
public:
	NFmiModelDataBlender(void);
	~NFmiModelDataBlender(void);

	void InitFromSettings(const std::string &theBaseNameSpace);
	void StoreToSettings(void);
	void InitWithNewTimes(const NFmiTimeDescriptor &theTimeDescriptor, int theStartHour = -1, int theEndHour = -1); // alustetaan itsBlendingFactors uudestaan, kuitenkin yritt�en s�ilytt�� edelliset arvot mahd. hyvin
	void MakeBlendLinear(void); // tekee olemassa olevan blendaus v�lin kertoimista lineaariset
	bool MoveStartTime(FmiDirection theDirection);
	bool MoveEndTime(FmiDirection theDirection);
	void MoveBlendTimes(FmiDirection theDirection);
	bool DoBlending(NFmiFastQueryInfo &theResult, NFmiFastQueryInfo &theSource1, NFmiFastQueryInfo &theSource2, bool fDoJustBlendedTimes);
	float GetBlendFactor(size_t theIndex);
	float SetBlendFactor(size_t theIndex, float newValue);
	int GetTimeStepInMinutes(size_t theIndex);
	int GetForecastHour(size_t theIndex);

	bool Use(void) const {return fUse;}
	void Use(bool newValue) {fUse = newValue;}
	int GetStartTimeHour(void) const {return itsStartTimeHour;}
	int GetEndTimeHour(void) const {return itsEndTimeHour;}
	size_t GetStarTimeIndex(void) const {return itsStarTimeIndex;}
	size_t GetEndTimeIndex(void) const {return itsEndTimeIndex;}
	const NFmiTimeDescriptor& GetTimeDescriptor(void) const {return itsTimeDescriptor;}
	const NFmiMetTime& GetStartTime(void);
	const NFmiMetTime& GetEndTime(void);

private:
	void UpdateForecastHours(void);
	float CalcLinearFactor(size_t index);

	bool fUse; // onko systeemi k�yt�ss� (datan lataus dialogissa)
	int itsStartTimeHour; // t�m� on blendauksen aloitus aika ns. forecast-hour yksik�ss� eli tuntina alkaen 0:sta
	int itsEndTimeHour; // t�m� on blendauksen lopetus aika ns. forecast-hour yksik�ss� eli tuntina alkaen 0:sta
	size_t itsStarTimeIndex; // blendaus kertoimia pidet��n koko rakennettavalle ajalle, t�m� kertoo itsBlendingFactors:ista blendauksen aloitus kohdan indeksin
	size_t itsEndTimeIndex;
	std::vector<float> itsBlendingFactors; // koko rakennettavan blendattavan datan kaikki aika-askeleet blendaus kertoimia varten, keroo 
											// mill� osuudella 2. kahdesta l�hdedatoista otetaan kullekin aika-askeleelle.
											// eli 0 arvolla otetaan data pelk�st��n 1. datasta ja 1:ll� pelk�st��n 2. datasta. 0.3:lla otetaan 70% 1. ja 30% 2. datasta.
	NFmiTimeDescriptor itsTimeDescriptor; // t�ss� on rakennettavan datan ajat

};

