//� Ilmatieteenlaitos/Marko.
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
	void Init(const NFmiWindTableSystem &theData); // t�ss� asetellaan l�hinn� piirto-ominasuuksia
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

	// HUOM!! T�m� laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin t�m� luokka luetaan sis��n tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan. Eli �l� k�yt� suoraan t�t� metodia, vaan Init(filename)-metodia!!!!
	void Read(std::istream& is);
private:
	void AddAreaMask(std::vector<AreaMaskData> &theAreaMaskDataList, const std::string &theFileName, const std::string &theName);

	std::vector<AreaMaskData> itsAreaMaskDataList;
	std::vector<AreaMaskData> itsAreaMaskDataList2; // t�ss� vaihtoehtoinen area-maski lista
	int itsSelectedAreaMaskList;
	std::string itsControlDirectory;
	bool fViewVisible;
	bool fMustaUpdateTable; // t�m� on optimointi, jos t�m� on p��ll�, pit�� taulukko n�ytt� p�ivitt��
	bool fUseMapTime; // optio ett� k�yt� p��karttan�yt�n 1. aikaa laskelmien l�ht�kohtana, muuten sein�kelloaikaa l�himm�st� aikajaksosta
	bool fDoInitializeAreaMasks; // jos t�m� on true, p�ivitt�� maskit halutulle datalle (t�m� on optimointia)
	int itsStartHourOffset; // kun tuulitaulukossa lasketaan integrointeja aikojen yli, t�m� on alku ajan hetken siirto taaksep�in
	int itsEndHourOffset; // t�m� on vastaava siirto eteenp�in

	std::string itsSettingsKeyBaseStr;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiWindTableSystem& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiWindTableSystem& item){item.Read(is); return is;}


