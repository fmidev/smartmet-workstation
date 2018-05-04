#pragma once

#include <string>

// tiet‰‰ synop-plottauksen yhteydess‰ k‰ytett‰v‰‰ asemien priorisoinnin
// Ne asemat joilla on priorisointi, yritet‰‰n piirt‰‰ ennen muita.
// Prioriteetti 1 on korkein ja 2 sitten jne.
class NFmiSynopStationPriority
{
public:
	NFmiSynopStationPriority(void)
	:itsPriority(1)
	,itsLocationIndex(-1)
	,itsStationId(0)
	,itsStationName()
	{
	}

	NFmiSynopStationPriority(int thePriority, int theStationId, const std::string &theStationName)
	:itsPriority(thePriority)
	,itsLocationIndex(-1)
	,itsStationId(theStationId)
	,itsStationName(theStationName)
	{
	}

	int itsPriority;
	int itsLocationIndex; // optimointia, kun kerran lˆydetty halutun aseman sijainti, talletetaan sen indeksi t‰h‰n
						  // ei talletetan ohjaustiedostoon
	int itsStationId; // aseman wmo numero
	std::string itsStationName; // t‰m‰ on l‰hinn‰ ohjaustiedoston kommentointia
};

