#include "NFmiSynopStationPrioritySystem.h"
#include "NFmiFileSystem.h"
#include "NFmiCommentStripper.h"

SynopStationPriorityList& NFmiSynopStationPrioritySystem::PriorityList(int theWantedPriority)
{
	switch(theWantedPriority)
	{
	case 1:
		return itsPriorityList1;
	case 2:
		return itsPriorityList2;
	case 3:
		return itsPriorityList3;
	default:
		return itsPriorityList4;
	}
}

const SynopStationPriorityList& NFmiSynopStationPrioritySystem::PriorityList(int theWantedPriority) const
{
	switch(theWantedPriority)
	{
	case 1:
		return itsPriorityList1;
	case 2:
		return itsPriorityList2;
	case 3:
		return itsPriorityList3;
	default:
		return itsPriorityList4;
	}
}

void NFmiSynopStationPrioritySystem::Clear(void)
{
	itsPriorityList1.clear();
	itsPriorityList2.clear();
	itsPriorityList3.clear();
	itsPriorityList4.clear();
}

bool NFmiSynopStationPrioritySystem::Init(const std::string &theInitFileName)
{
	Clear();
	itsInitFileName = theInitFileName;
	if(itsInitFileName.empty())
		return false;
    if(!NFmiFileSystem::FileExists(itsInitFileName))
        return false;

	NFmiCommentStripper stripComments;
	if(stripComments.ReadAndStripFile(itsInitFileName))
	{
		std::stringstream strippedControlFile(stripComments.GetString());
		strippedControlFile >> *this;

		return true; // ei voi tarkistaa strippedControlFile.good():illa, koska tiedostoa luetaan niin kauan kun tavaraa löytyy, joten se on aina lopuksi fail
	}
	return false;
}

// HUOM!! Tämä laittaa kommentteja mukaan!
void NFmiSynopStationPrioritySystem::Write(std::ostream& os) const
{
	os << "// This is a comment. To this file is stored synop-plot's prioritized stations. Priorisations goes from 1 to 4" << std::endl;
	os << "// With same priority numbers stations are ordered by their order in this list, e.g. 1. is drawn first and then second etc" << std::endl;
	os << "// Station list is in following format:" << std::endl;
	os << "// priority-number wmoID station-name (name is more like a comment, wmoID is decisive)" << std::endl;
	// tulostetaan prioriteetti listat prioriteetti järjestyksessä
	int listCount = MaxPriorityLevel();
	for(int i=0 ; i<listCount; i++)
	{
		const SynopStationPriorityList& priorityList = PriorityList(i);
		for(size_t j=0 ; j<priorityList.size(); j++)
		{
			os << priorityList[j].itsPriority << " " << priorityList[j].itsStationId << " " << priorityList[j].itsStationName << std::endl;
		}
	}
}

// HUOM!! ennen kuin tämä luokka luetaan sisään tiedostosta, poista kommentit
// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
// sekaan. Eli älä käytä suoraan tätä metodia, vaan Init(filename)-metodia!!!!
void NFmiSynopStationPrioritySystem::Read(std::istream& is)
{
	NFmiSynopStationPriority tmpPriority;
	std::string name;
	char buffer[512]="";
	int priority = 0;
	int wmoId = 0;
	bool keepReading = true;
	while(keepReading)
	{
		is >> priority;
		if(is.fail())
			break;
		if(priority > MaxPriorityLevel())
			priority = MaxPriorityLevel();
		is >> wmoId;
		if(is.fail())
			break;
		is.getline(buffer, 511);
		if(is.fail())
			break;
		name = buffer;
		InsertStationInfo(NFmiSynopStationPriority(priority, wmoId, name), false); // false=ei etsitä luku vaiheessa duplikaatteja, vaan luotetaan että niitä ei ole
	}
}

void NFmiSynopStationPrioritySystem::InsertStationInfo(const NFmiSynopStationPriority &theInfo, bool fSearchForDuplicate)
{
	if(fSearchForDuplicate)
	{ // search and destroy existing one
	}
	PriorityList(theInfo.itsPriority).push_back(theInfo);
}

