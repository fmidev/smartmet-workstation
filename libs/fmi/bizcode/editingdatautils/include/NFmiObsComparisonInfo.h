//**********************************************************
// C++ Class Name : NFmiObsComparisonInfo
// ---------------------------------------------------------
// Filetype: (HEADER)
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : metedit nfmiviews
//  - GD View Type      : Class Diagram
//  - GD View Name      : Class Diagram
// ---------------------------------------------------
//  Author         : pietarin
//
//  Description:
//   Pit�� sis�ll��n tietoa mit� tarvitaan kun ennusteita verrataan
//   havaintoihin karttan�yt�ll�. Eli mink� kokoinen symboli, symboli muoto,
//   onko vertailu moodi p��ll� vai ei, eri parametrien rajoja, symboleihin
//   liittyv�t v�ritys tiedot jne.
//   T�m� liittyy siis kun halutaan katsoa havaintojen ja ennusteiden eroja
//   karttan�yt�ll� ja piirret��n v�ri l�pysk�it� ennusteen p��lle sen mukaan
// miten hyvin ennusteet osuvat havaintojen kanssa.
//
//  Change Log:
//
//**********************************************************

#pragma once

#include "NFmiPoint.h"
#include "NFmiColor.h"
#include "NFmiDataMatrix.h"

class NFmiObsComparisonInfo
{
public:

	class Param
	{
	public:
		// HUOM! erotus tehd��n aina ennuste - havainto, eli jos luku on positiivinen on ennuste suurempi kuin havainto
		// ja highlimit ja overhigh v�ri tulevat t�ss� tilanteessa (siis jos ero menee yli high-rajan).

		Param(void)
		:itsId(0)
		,itsLowLimit(-10)
		,itsHighLimit(10)
		,itsDecimalCount(1)
		,itsConversionFactor(1)
		{
		}

		Param(int theParId, float theLowLimit, float theHighLimit, int theDecimalCount = 1, float theConversionFactor = 1.f)
		:itsId(theParId)
		,itsLowLimit(theLowLimit)
		,itsHighLimit(theHighLimit)
		,itsDecimalCount(theDecimalCount)
		,itsConversionFactor(theConversionFactor)
		{
		}

		// HUOM!! T�m� laittaa kommentteja mukaan!
		void Write(std::ostream& os) const;
		// HUOM!! ennen kuin t�m� luokka luetaan sis��n tiedostosta, poista kommentit
		// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
		// sekaan. Eli �l� k�yt� suoraan t�t� metodia, vaan Init(filename)-metodia!!!!
		void Read(std::istream& is);

		int itsId; // param id eli esim. 4 jos l�mp�tila
		float itsLowLimit; // kuinka paljon t�lle parametrille saa tulla eroa alasp�in ennen kuin laitetaan 'underlow' v�rinen l�tk� ruutuun
		float itsHighLimit; // kuinka paljon t�lle parametrille saa tulla eroa yl�sp�in ennen kuin laitetaan 'overhigh' v�rinen l�tk� ruutuun
		int itsDecimalCount; // kuinka monta desimaalia laitetaan tooltippiin (default 1)
		float itsConversionFactor; // mill� luvulla kerrotaan havainto ennenkuin sit� verrataan ennusteen kanssa (default 1)
								   // t�m� sen takia ett� kokoniaspilvisyys on ennusteissa 0-100 % mutta havainnoissa 0-8 oktaa
	};

	NFmiObsComparisonInfo(void);
	void Init(void);
	void NextSymbolSize(void);
	void NextSymbolType(void);
	const NFmiObsComparisonInfo::Param& GetParam(int theParId);

	void NextComparisonMode(void);
	int ComparisonMode(void) const {return itsComparisonMode;}
	void ComparisonMode(int newState);
	bool DrawBorders(void) const {return fDrawBorders;}
	void DrawBorders(bool newState);
	const NFmiPoint& SymbolSize(void) const {return itsSymbolSize;}
	void SymbolSize(const NFmiPoint &newValue);
	int SymbolType(void) const {return itsSymbolType;}
	void SymbolType(int newValue);
	const NFmiColor& OkColor(void) const {return itsOkColor;}
	const NFmiColor& UnderLowColor(void) const {return itsUnderLowColor;}
	const NFmiColor& OverHighColor(void) const {return itsOverHighColor;}
	void OkColor(const NFmiColor& newValue);
	void UnderLowColor(const NFmiColor& newValue);
	void OverHighColor(const NFmiColor& newValue);
	const NFmiColor& ExtrapolationOkColor(void) const {return itsExtrapolationOkColor;}
	const NFmiColor& ExtrapolationUnderLowColor(void) const {return itsExtrapolationUnderLowColor;}
	const NFmiColor& ExtrapolationOverHighColor(void) const {return itsExtrapolationOverHighColor;}
	void ExtrapolationOkColor(const NFmiColor& newValue);
	void ExtrapolationUnderLowColor(const NFmiColor& newValue);
	void ExtrapolationOverHighColor(const NFmiColor& newValue);
	const NFmiColor& FrameColor(void) const {return itsFrameColor;}
	void FrameColor(const NFmiColor& newValue);

	// HUOM!! T�m� laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin t�m� luokka luetaan sis��n tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan. Eli �l� k�yt� suoraan t�t� metodia, vaan Init(filename)-metodia!!!!
	void Read(std::istream& is);
private:
	void SaveSymbolSizeToSettings(void);
	void ReadColorFromSettings(NFmiColor &theColor, const std::string &theKeyString);
	void WriteColorToSettings(NFmiColor &theColor, const std::string &theKeyString);
	void ReadParamsSettingsFromInfo(void);
	NFmiObsComparisonInfo::Param ReadParamSettingsFromInfo(const std::string &theParamStr);

	int itsComparisonMode; // 0 == off, 1= obs-only, 2=obs+approx
	bool fDrawBorders; // piirret��nk� ympyr�/neli� reunojen kanssa (haalean keltainen)
	NFmiPoint itsSymbolSize; // varoitus symbolin koko pikseleiss�
	int itsSymbolType; // 1 = neli�, 2 = ympyr�
	// n�it� v�rej� k�ytet��n normaalissa havainto vs. ennuste vertailussa
	NFmiColor itsOkColor;
	NFmiColor itsUnderLowColor;
	NFmiColor itsOverHighColor;
	// n�it� v�rej� k�ytet��n extrapoloiduissa havainnoissa (eli 'ennustetuissa' havainnoissa)
	NFmiColor itsExtrapolationOkColor;
	NFmiColor itsExtrapolationUnderLowColor;
	NFmiColor itsExtrapolationOverHighColor;

	NFmiColor itsFrameColor;

	std::vector<NFmiObsComparisonInfo::Param> itsParams;
};

inline std::ostream& operator<<(std::ostream& os, const NFmiObsComparisonInfo& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiObsComparisonInfo& item){item.Read(is); return is;}

inline std::ostream& operator<<(std::ostream& os, const NFmiObsComparisonInfo::Param& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiObsComparisonInfo::Param& item){item.Read(is); return is;}

