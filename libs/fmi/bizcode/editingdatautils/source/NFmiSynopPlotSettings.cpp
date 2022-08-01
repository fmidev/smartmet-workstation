#include "NFmiSynopPlotSettings.h"
#include "NFmiSettings.h"
#include "NFmiDataStoringHelpers.h"

NFmiSynopPlotSettings::NFmiSynopPlotSettings(void)
:fShowT(true)
,fShowTd(true)
,fShowV(true)
,fShowWw(true)
,fShowCh(true)
,fShowCm(true)
,fShowCl(true)
,fShowN(true)
,fShowNh(true)
,fShowDdff(true)
,fShowPPPP(true)
,fShowPpp(true)
,fShowA(true)
,fShowW1(true)
,fShowW2(true)
,fShowH(true)
,fShowRr(true)
,fShowTw(true)
,fUseSingleColor(false)
,itsSingleColor(0, 0, 0)
,itsFontSize(2.5)
,itsPlotSpacing(1)
,itsMetarPlotSettings()
{
}

// initialisoidaan NFmiSettings-luokan avulla, asetukset ovat editor.conf -tiedostossa
void NFmiSynopPlotSettings::Init(void)
{
	try
	{
		fShowT = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowT");
		fShowTd = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowTd");
		fShowV = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowV");
		fShowWw = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowWw");
		fShowCh = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowCh");
		fShowCm = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowCm");
		fShowCl = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowCl");
		fShowN = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowN");
		fShowNh = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowNh");
		fShowDdff = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowDdff");
		fShowPPPP = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowPPPP");
		fShowPpp = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowPpp");
		fShowA = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowA");
		fShowW1 = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowW1");
		fShowW2 = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowW2");
		fShowH = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowH");
		fShowRr = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowRr");
		fShowTw = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::ShowTw");
		fUseSingleColor = NFmiSettings::Require<bool>("MetEditor::SynopPlotSettings::UseSingleColor");

		// värin luku vaatii hieman kikkailua stringstreamin kanssa
		std::string tmp;
		tmp = NFmiSettings::Require<std::string>("MetEditor::SynopPlotSettings::SingleColor");
#ifdef OLDGCC
		std::istrstream colorStream(tmp);
#else
		std::stringstream colorStream(tmp);
#endif
		colorStream >> itsSingleColor;

		itsFontSize = NFmiSettings::Require<double>("MetEditor::SynopPlotSettings::FontSize");
		itsPlotSpacing = NFmiSettings::Require<double>("MetEditor::SynopPlotSettings::PlotSpacing");

		itsMetarPlotSettings.Init();
	}
	catch(...)
	{
		throw; // laitetaan poikkeus eteenpäin vain
	}
}

// talletetaan NFmiSettings-luokan avulla asetukset editor.conf -tiedostoon
void NFmiSynopPlotSettings::Store(void)
{
	try
	{
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowT", fShowT ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowTd", fShowTd ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowV", fShowV ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowWw", fShowWw ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowCh", fShowCh ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowCm", fShowCm ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowCl", fShowCl ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowN", fShowN ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowNh", fShowNh ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowDdff", fShowDdff ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowPPPP", fShowPPPP ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowPpp", fShowPpp ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowA", fShowA ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowW1", fShowW1 ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowW2", fShowW2 ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowH", fShowH ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowRr", fShowRr ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::ShowTw", fShowTw ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::UseSingleColor", fUseSingleColor ? "1" : "0", true);

#ifdef OLDGCC
		std::ostrstream colorStream;
#else
		std::stringstream colorStream;
#endif
		colorStream << itsSingleColor;

		NFmiSettings::Set("MetEditor::SynopPlotSettings::SingleColor", colorStream.str(), true);

		NFmiSettings::Set("MetEditor::SynopPlotSettings::FontSize", NFmiStringTools::Convert(itsFontSize), true);
		NFmiSettings::Set("MetEditor::SynopPlotSettings::PlotSpacing", NFmiStringTools::Convert(itsPlotSpacing), true);

		itsMetarPlotSettings.Store();
	}
	catch(...)
	{
		throw; // laitetaan poikkeus eteenpäin vain
	}
}

void NFmiSynopPlotSettings::ShowAllParams(bool newValue)
{
	fShowT = newValue;
	fShowTd = newValue;
	fShowV = newValue;
	fShowWw = newValue;
	fShowCh = newValue;
	fShowCm = newValue;
	fShowCl = newValue;
	fShowN = newValue;
	fShowNh = newValue;
	fShowDdff = newValue;
	fShowPPPP = newValue;
	fShowPpp = newValue;
	fShowA = newValue;
	fShowW1 = newValue;
	fShowW2 = newValue;
	fShowH = newValue;
	fShowRr = newValue;
	fShowTw = newValue;
}

// HUOM!! Tämä laittaa kommentteja mukaan!
void NFmiSynopPlotSettings::Write(std::ostream& os) const
{
	os << "// NFmiSynopPlotSettings::Write..." << std::endl;

	os << "// ShowT + ShowTd + ShowV + ShowWw" << std::endl;
	os << fShowT << " " << fShowTd << " " << fShowV << " " << fShowWw << std::endl;

	os << "// ShowCh + ShowCm + ShowCl + ShowN" << std::endl;
	os << fShowCh << " " << fShowCm << " " << fShowCl << " " << fShowN << std::endl;

	os << "// ShowNh + ShowDdff + ShowPPPP + ShowPpp" << std::endl;
	os << fShowNh << " " << fShowDdff << " " << fShowPPPP << " " << fShowPpp << std::endl;

	os << "// ShowA + ShowW1 + ShowW2 + ShowH" << std::endl;
	os << fShowA << " " << fShowW1 << " " << fShowW2 << " " << fShowH << std::endl;

	os << "// ShowRr + ShowTw + UseSingleColor" << std::endl;
	os << fShowRr << " " << fShowTw << " " << fUseSingleColor << std::endl;

	os << "// SingleColor" << std::endl;
	os << itsSingleColor << std::endl;

	os << "// itsFontSize + itsPlotSpacing" << std::endl;
	os << itsFontSize << " " << itsPlotSpacing << std::endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	// Kun tulee uusia muuttujia, tee tähän extradatan täyttöä, jotta se saadaan talteen tiedopstoon siten että
	// edelliset versiot eivät mene solmuun vaikka on tullut uutta dataa.

	// 1. uusi string data on metar-plot asetukset
	extraData.Add(itsMetarPlotSettings.MakeViewMacroString());

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiSynopPlotSettings::Write failed");
}

// HUOM!! ennen kuin tämä luokka luetaan sisään tiedostosta, poista kommentit
// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
// sekaan. Eli älä käytä suoraan tätä metodia, vaan Init(filename)-metodia!!!!
void NFmiSynopPlotSettings::Read(std::istream& is)
{
	is >> fShowT >> fShowTd >> fShowV >> fShowWw;

	is >> fShowCh >> fShowCm >> fShowCl >> fShowN;

	is >> fShowNh >> fShowDdff >> fShowPPPP >> fShowPpp;

	is >> fShowA >> fShowW1 >> fShowW2 >> fShowH;

	is >> fShowRr >> fShowTw >> fUseSingleColor;

	is >> itsSingleColor;

	is >> itsFontSize >> itsPlotSpacing;

	if(is.fail())
		throw std::runtime_error("NFmiSynopPlotSettings::Read failed");

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	is >> extraData;
	// Tässä sitten otetaaan extradatasta talteen uudet muuttujat, mitä on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, käsittele tässä.

	// 1. uusi string data on metar-plot asetukset
	if(extraData.itsStringValues.size() >= 1)
	{
		itsMetarPlotSettings.InitFromViewMacroString(extraData.itsStringValues[0], *this);
	}
	else
	{
		// Jos viewMacro talletus oli tehty vanhalla versiolla, pitää antaa tyhjä string tässä, 
		// jotta saadaan halutu asetukset päälle, eli kaikki parametrit näkyviksi ja tietyt synop-plot optiot käyttöön
		itsMetarPlotSettings.InitFromViewMacroString(std::string(), *this);
	}

	if(is.fail())
		throw std::runtime_error("NFmiSynopPlotSettings::Read failed");
}
