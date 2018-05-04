#pragma once

#include "NFmiColor.h"

class NFmiSynopPlotSettings
{
public:
	NFmiSynopPlotSettings(void);

	void Init(void); // initialisoidaan NFmiSettings-luokan avulla, asetukset ovat editor.conf -tiedostossa
	void Store(void); // talletetaan NFmiSettings-luokan avulla asetukset editor.conf -tiedostoon

	void ShowAllParams(bool newValue);
	bool ShowT(void) const {return fShowT;}
	void ShowT(bool newValue) {fShowT = newValue;}
	bool ShowTd(void) const {return fShowTd;}
	void ShowTd(bool newValue) {fShowTd = newValue;}
	bool ShowV(void) const {return fShowV;}
	void ShowV(bool newValue) {fShowV = newValue;}
	bool ShowWw(void) const {return fShowWw;}
	void ShowWw(bool newValue) {fShowWw = newValue;}
	bool ShowCh(void) const {return fShowCh;}
	void ShowCh(bool newValue) {fShowCh = newValue;}
	bool ShowCm(void) const {return fShowCm;}
	void ShowCm(bool newValue) {fShowCm = newValue;}
	bool ShowCl(void) const {return fShowCl;}
	void ShowCl(bool newValue) {fShowCl = newValue;}
	bool ShowN(void) const {return fShowN;}
	void ShowN(bool newValue) {fShowN = newValue;}
	bool ShowNh(void) const {return fShowNh;}
	void ShowNh(bool newValue) {fShowNh = newValue;}
	bool ShowDdff(void) const {return fShowDdff;}
	void ShowDdff(bool newValue) {fShowDdff = newValue;}
	bool ShowPPPP(void) const {return fShowPPPP;}
	void ShowPPPP(bool newValue) {fShowPPPP = newValue;}
	bool ShowPpp(void) const {return fShowPpp;}
	void ShowPpp(bool newValue) {fShowPpp = newValue;}
	bool ShowA(void) const {return fShowA;}
	void ShowA(bool newValue) {fShowA = newValue;}
	bool ShowW1(void) const {return fShowW1;}
	void ShowW1(bool newValue) {fShowW1 = newValue;}
	bool ShowW2(void) const {return fShowW2;}
	void ShowW2(bool newValue) {fShowW2 = newValue;}
	bool ShowH(void) const {return fShowH;}
	void ShowH(bool newValue) {fShowH = newValue;}
	bool ShowRr(void) const {return fShowRr;}
	void ShowRr(bool newValue) {fShowRr = newValue;}
	bool ShowTw(void) const {return fShowTw;}
	void ShowTw(bool newValue) {fShowTw = newValue;}
	bool UseSingleColor(void) const {return fUseSingleColor;}
	void UseSingleColor(bool newValue) {fUseSingleColor = newValue;}
	const NFmiColor& SingleColor(void) const {return itsSingleColor;}
	void SingleColor(const NFmiColor& newValue) {itsSingleColor = newValue;}
	double FontSize(void) const {return itsFontSize;}
	void FontSize(double newValue) {itsFontSize = newValue;}
	double PlotSpacing(void) const {return itsPlotSpacing;}
	void PlotSpacing(double newValue) {itsPlotSpacing = newValue;}

	// HUOM!! T‰m‰ laittaa kommentteja mukaan!
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
	void Read(std::istream& is);

private:
	bool fShowT; // n‰ytet‰‰nkˆ T synop-plotissa vai ei
	bool fShowTd;
	bool fShowV;
	bool fShowWw;
	bool fShowCh;
	bool fShowCm;
	bool fShowCl;
	bool fShowN;
	bool fShowNh;
	bool fShowDdff;
	bool fShowPPPP;	  // PPPP eli pressure
	bool fShowPpp; // ppp eli pressure change
	bool fShowA; // a eli paineen muutoksen luonne
	bool fShowW1;
	bool fShowW2;
	bool fShowH; // h eli pilven korkeus
	bool fShowRr;
	bool fShowTw; // eli veden l‰mpˆtila
	bool fUseSingleColor; // eli k‰ytet‰‰nkˆ yksiv‰rist‰ synop plottausta vai defaultti v‰ri koodattua
	NFmiColor itsSingleColor; // jos piirret‰‰n plottaus yksi v‰risen‰, t‰ss‰ on se v‰ri
	double itsFontSize; // [mm]
	double itsPlotSpacing; // jos luku on 0, piirret‰‰n kaikki asemat n‰kyviin (sotkuinen),
						   // jos luku 0 - 1, piirret‰‰n synoppeja jonkin verran p‰‰llekk‰in (tihe‰‰)
						   // jos luku on 1, piirret‰‰n synopit juuri omille alueilleen
						   // jos luku on 1 - 2, piirret‰‰n synoppeja isommill‰ v‰leill‰ (harvaa)
};

inline std::ostream& operator<<(std::ostream& os, const NFmiSynopPlotSettings& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiSynopPlotSettings& item){item.Read(is); return is;}

