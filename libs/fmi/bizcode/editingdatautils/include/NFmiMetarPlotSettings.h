#pragma once

#include "NFmiColor.h"

class NFmiMetarPlotSettings
{
public:
	NFmiMetarPlotSettings();

	void Init();
	void Store();

	void ShowAllParams(bool newValue);

	bool Show_SkyInfo() const { return fShow_SkyInfo; }
	void Show_SkyInfo(bool newValue) { fShow_SkyInfo = newValue; }
	bool Show_TT(void) const { return fShow_TT; }
	void Show_TT(bool newValue) { fShow_TT = newValue; }
	bool Show_TdTd() const { return fShow_TdTd; }
	void Show_TdTd(bool newValue) { fShow_TdTd = newValue; }
	bool Show_PhPhPhPh() const { return fShow_PhPhPhPh; }
	void Show_PhPhPhPh(bool newValue) { fShow_PhPhPhPh = newValue; }
	bool Show_dddff() const { return fShow_dddff; }
	void Show_dddff(bool newValue) { fShow_dddff = newValue; }
	bool Show_Gff() const { return fShow_Gff; }
	void Show_Gff(bool newValue) { fShow_Gff = newValue; }
	bool Show_VVVV() const { return fShow_VVVV; }
	void Show_VVVV(bool newValue) { fShow_VVVV = newValue; }
	bool Show_ww() const { return fShow_ww; }
	void Show_ww(bool newValue) { fShow_ww = newValue; }
	bool Show_Status() const { return fShow_Status; }
	void Show_Status(bool newValue) { fShow_Status = newValue; }

	bool UseSingleColor(void) const { return fUseSingleColor; }
	void UseSingleColor(bool newValue) { fUseSingleColor = newValue; }
	const NFmiColor& SingleColor(void) const { return itsSingleColor; }
	void SingleColor(const NFmiColor& newValue) { itsSingleColor = newValue; }
	double FontSize(void) const { return itsFontSize; }
	void FontSize(double newValue) { itsFontSize = newValue; }
	double PlotSpacing(void) const { return itsPlotSpacing; }
	void PlotSpacing(double newValue) { itsPlotSpacing = newValue; }

	std::string MakeViewMacroString() const;
	void InitFromViewMacroString(std::string& viewMacroStr);

private:
	// N�ytet��nk� taivaan tilasta tietoja vai ei (CAVOK/SKC/VV001/SCT003 (4x))
	bool fShow_SkyInfo = true;
	// N�ytet��nk� T'T' (l�mp�tila) plotissa vai ei
	bool fShow_TT = true;
	// N�ytet��nk� TdTd (kastepiste) plotissa vai ei
	bool fShow_TdTd = true;
	// N�ytet��nk� PhPhPhPh (paine) plotissa vai ei
	bool fShow_PhPhPhPh = true;
	// N�ytet��nk� dddff (tuulivektori) plotissa vai ei
	bool fShow_dddff = true;
	// N�ytet��nk� Gff (puuskan nopeus) plotissa vai ei
	bool fShow_Gff = true;
	// N�ytet��nk� VVVV (horisontaali n�kyvyys) plotissa vai ei
	bool fShow_VVVV = true;
	// N�ytet��nk� w'w' (vallitseva s��) plotissa vai ei
	bool fShow_ww = true;
	// N�ytet��nk� Status (v�rillinen laatikko keskell�) plotissa vai ei
	bool fShow_Status = true;

	// Eli k�ytet��nk� yksiv�rist� plottausta vai defaultti v�ri koodattua
	bool fUseSingleColor = false;
	// Jos piirret��n plottaus yksi v�risen�, t�ss� on se v�ri
	NFmiColor itsSingleColor;
	// fontin koko [mm]
	double itsFontSize = 2.5;
	// PlotSpacing, jos luku on 0, piirret��n kaikki asemat n�kyviin (sotkuinen),
	// jos luku 0 - 1, piirret��n synoppeja jonkin verran p��llekk�in (tihe��)
	// jos luku on 1, piirret��n synopit juuri omille alueilleen
	// jos luku on 1 - 2, piirret��n synoppeja isommill� v�leill� (harvaa)
	double itsPlotSpacing = 1;
};
