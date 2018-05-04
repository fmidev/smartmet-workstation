#pragma once

#include "NFmiSynopStationPriority.h"
#include "NFmiDataMatrix.h" // checkedVector

typedef checkedVector<NFmiSynopStationPriority> SynopStationPriorityList;

class NFmiSynopStationPrioritySystem;
std::ostream& operator<<(std::ostream& os, const NFmiSynopStationPrioritySystem& item);
std::istream& operator>>(std::istream& is, NFmiSynopStationPrioritySystem& item);

// Luokka joka pit‰‰ synop-plottiin liittyvien asemien prioriteeteist‰ kirjaa.
// Luokalla on nyt nelj‰ prioriteetti listaa, joihin vodaan laittaa asemia.
// Prioriteetti 1:ll‰ olevat asemat piirret‰‰n ensin, jos mahtuvat kartalle,
// sitten 2:lla olevat jne..
// Synoppeja ei laiteta p‰‰llekk‰in (ainakaan liikaa), joten siksi pit‰‰
// pysty‰ priorisoimaan t‰rkeimm‰t asemat, ett‰ ne n‰kyisiv‰t melkein aina.
class NFmiSynopStationPrioritySystem
{
public:
	NFmiSynopStationPrioritySystem(void)
	:itsInitFileName()
	,itsPriorityList1()
	,itsPriorityList2()
	,itsPriorityList3()
	,itsPriorityList4()
	{}

	void Clear(void);

	SynopStationPriorityList& PriorityList(int theWantedPriority);
	const SynopStationPriorityList& PriorityList(int theWantedPriority) const;
	bool Init(const std::string &theInitFileName);

	const std::string& InitFileName(void) const {return itsInitFileName;}

	// HUOM!! T‰m‰ laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
	void Read(std::istream& is);

	void InsertStationInfo(const NFmiSynopStationPriority &theInfo, bool fSearchForDuplicate);
	static int MaxPriorityLevel(void){return 4;}
private:
	std::string itsInitFileName;
	SynopStationPriorityList itsPriorityList1; // t‰nne laitetaan prioriteetti 1. tason asemat
	SynopStationPriorityList itsPriorityList2; // t‰nne laitetaan prioriteetti 2. tason asemat
	SynopStationPriorityList itsPriorityList3; // t‰nne laitetaan prioriteetti 3. tason asemat
	SynopStationPriorityList itsPriorityList4; // t‰nne laitetaan prioriteetti 4. tason asemat
};
inline std::ostream& operator<<(std::ostream& os, const NFmiSynopStationPrioritySystem& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiSynopStationPrioritySystem& item){item.Read(is); return is;}

