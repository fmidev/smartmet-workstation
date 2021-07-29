//© Ilmatieteenlaitos/Marko.
//Original 3.3.2009
//
// Luokka huolehtii Tuulitaulukon asetuksista ja datasta.
//---------------------------------------------------------- NFmiWindTableSystem.h

#pragma once

#include "NFmiSvgPath.h"
#include "NFmiIndexMask.h"
#include "boost/shared_ptr.hpp"

class NFmiGrid;
class NFmiFastQueryInfo;

class NFmiWindTableSystem
{
public:
	class AreaMaskData
	{
	public:
		AreaMaskData(void)
		:itsName()
		,itsInitFileName()
		,itsSvgPath()
		,itsCachedMask()
		{}
		~AreaMaskData(void)
		{};

		bool Init(const std::string &theInitFileName, const std::string &theName);
		void InitMask(const NFmiGrid &theGrid);
		const NFmiIndexMask& GetMask(void) const {return itsCachedMask;}

		const std::string Name(void) const {return itsName;}
		const std::string InitFileName(void) const {return itsInitFileName;}
		NFmiSvgPath& SvgPath(void) {return itsSvgPath;}

	private:
		std::string itsName;
		std::string itsInitFileName;
		NFmiSvgPath itsSvgPath;
		NFmiIndexMask itsCachedMask;
	};


	NFmiWindTableSystem(void);
	~NFmiWindTableSystem(void);
	void InitializeFromSettings(const std::string &theSettingsKeyBaseStr);
	void Init(const NFmiWindTableSystem &theData); // tässä asetellaan lähinnä piirto-ominasuuksia
	void StoreSettings(void);
	bool ViewVisible(void) const {return fViewVisible;}
	void ViewVisible(bool newState) {fViewVisible = newState;}
	bool MustaUpdateTable(void) {return fMustaUpdateTable;}
	void MustaUpdateTable(bool newValue) {fMustaUpdateTable = newValue;}
	const std::string& ControlDirectory(void) const {return itsControlDirectory;}
	void ControlDirectory(const std::string &theNewValue) {itsControlDirectory = theNewValue;}
	void InitAreaMaskDatas(void);
	std::vector<AreaMaskData>& AreaMaskDataList(void);
	void InitializeAllAreaMasks(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	bool UseMapTime(void) const {return fUseMapTime;}
	void UseMapTime(bool newValue) {fUseMapTime = newValue;}
	int SelectedAreaMaskList(void) const {return itsSelectedAreaMaskList;}
	void SelectedAreaMaskList(int newValue);
	bool DoInitializeAreaMasks(void) const {return fDoInitializeAreaMasks;}
	void DoInitializeAreaMasks(bool newValue) {fDoInitializeAreaMasks = newValue;}
	int StartHourOffset(void) const {return itsStartHourOffset;}
	void StartHourOffset(int newValue) {itsStartHourOffset = newValue;}
	int EndHourOffset(void) const {return itsEndHourOffset;}
	void EndHourOffset(int newValue) {itsEndHourOffset = newValue;}

	// HUOM!! Tämä laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin tämä luokka luetaan sisään tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan. Eli älä käytä suoraan tätä metodia, vaan Init(filename)-metodia!!!!
	void Read(std::istream& is);
private:
	void AddAreaMask(std::vector<AreaMaskData> &theAreaMaskDataList, const std::string &theFileName, const std::string &theName);

	std::vector<AreaMaskData> itsAreaMaskDataList;
	std::vector<AreaMaskData> itsAreaMaskDataList2; // tässä vaihtoehtoinen area-maski lista
	int itsSelectedAreaMaskList;
	std::string itsControlDirectory;
	bool fViewVisible;
	bool fMustaUpdateTable; // tämä on optimointi, jos tämä on päällä, pitää taulukko näyttö päivittää
	bool fUseMapTime; // optio että käytä pääkarttanäytön 1. aikaa laskelmien lähtökohtana, muuten seinäkelloaikaa lähimmästä aikajaksosta
	bool fDoInitializeAreaMasks; // jos tämä on true, päivittää maskit halutulle datalle (tämä on optimointia)
	int itsStartHourOffset; // kun tuulitaulukossa lasketaan integrointeja aikojen yli, tämä on alku ajan hetken siirto taaksepäin
	int itsEndHourOffset; // tämä on vastaava siirto eteenpäin

	std::string itsSettingsKeyBaseStr;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiWindTableSystem& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiWindTableSystem& item){item.Read(is); return is;}


