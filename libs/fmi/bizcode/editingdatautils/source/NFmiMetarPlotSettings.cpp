#include "NFmiMetarPlotSettings.h"
#include "NFmiSettings.h"
#include "NFmiDataStoringHelpers.h"

NFmiMetarPlotSettings::NFmiMetarPlotSettings() = default;

// initialisoidaan NFmiSettings-luokan avulla, asetukset ovat editor.conf -tiedostossa
void NFmiMetarPlotSettings::Init()
{
	try
	{
		fShow_SkyInfo = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::Show_SkyInfo", true);
		fShow_TT = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::Show_TT", true);
		fShow_TdTd = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::Show_TdTd", true);
		fShow_PhPhPhPh = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::Show_PhPhPhPh", true);
		fShow_dddff = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::Show_dddff", true);
		fShow_Gff = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::Show_Gff", true);
		fShow_VVVV = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::Show_VVVV", true);
		fShow_ww = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::Show_ww", true);
		fShow_Status = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::Show_Status", true);
		fUseSingleColor = NFmiSettings::Optional<bool>("MetEditor::MetarPlotSettings::UseSingleColor", false);

		// värin luku vaatii hieman kikkailua stringstreamin kanssa
		std::string tmp;
		tmp = NFmiSettings::Optional<std::string>("MetEditor::MetarPlotSettings::SingleColor", std::string("0 0 0 0"));
		std::stringstream colorStream(tmp);
		colorStream >> itsSingleColor;

		itsFontSize = NFmiSettings::Optional<double>("MetEditor::MetarPlotSettings::FontSize", 2.5);
		itsPlotSpacing = NFmiSettings::Optional<double>("MetEditor::MetarPlotSettings::PlotSpacing", 1);
	}
	catch(...)
	{
		throw; // laitetaan poikkeus eteenpäin vain
	}
}

// talletetaan NFmiSettings-luokan avulla asetukset editor.conf -tiedostoon
void NFmiMetarPlotSettings::Store()
{
	try
	{
		NFmiSettings::Set("MetEditor::MetarPlotSettings::Show_SkyInfo", fShow_SkyInfo ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::Show_TT", fShow_TT ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::Show_TdTd", fShow_TdTd ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::Show_PhPhPhPh", fShow_PhPhPhPh ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::Show_dddff", fShow_dddff ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::Show_Gff", fShow_Gff ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::Show_VVVV", fShow_VVVV ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::Show_ww", fShow_ww ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::Show_Status", fShow_Status ? "1" : "0", true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::UseSingleColor", fUseSingleColor ? "1" : "0", true);

		std::stringstream colorStream;
		colorStream << itsSingleColor;
		NFmiSettings::Set("MetEditor::MetarPlotSettings::SingleColor", colorStream.str(), true);

		NFmiSettings::Set("MetEditor::MetarPlotSettings::FontSize", NFmiStringTools::Convert(itsFontSize), true);
		NFmiSettings::Set("MetEditor::MetarPlotSettings::PlotSpacing", NFmiStringTools::Convert(itsPlotSpacing), true);
	}
	catch(...)
	{
		throw; // laitetaan poikkeus eteenpäin vain
	}
}

void NFmiMetarPlotSettings::ShowAllParams(bool newValue)
{
	fShow_SkyInfo = newValue;
	fShow_TT = newValue;
	fShow_TdTd = newValue;
	fShow_PhPhPhPh = newValue;
	fShow_dddff = newValue;
	fShow_Gff = newValue;
	fShow_VVVV = newValue;
	fShow_ww = newValue;
	fShow_Status = newValue;
}

std::string NFmiMetarPlotSettings::MakeViewMacroString() const
{
	std::stringstream out;
	out << fShow_SkyInfo << " " << fShow_TT << " " << fShow_TdTd << " " << fShow_PhPhPhPh;
	out << " " << fShow_dddff << " " << fShow_Gff << " " << fShow_VVVV << " " << fShow_ww << " " << fShow_Status;
	out << " " << fUseSingleColor << " " << itsSingleColor;
	out << " " << itsFontSize << " " << itsPlotSpacing;

    // Lopuksi vielä mahdollinen extra data. Kun tulee uusia muuttujia, tee tähän extradatan täyttöä, jotta se 
	// saadaan talteen tiedopstoon siten että edelliset versiot eivät mene solmuun vaikka on tullut uutta dataa.
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;

	out << extraData;

	return out.str();
}

void NFmiMetarPlotSettings::InitFromViewMacroString(std::string& viewMacroStr)
{
	if(viewMacroStr.empty())
		return;

	std::stringstream in(viewMacroStr);
	in >> fShow_SkyInfo >> fShow_TT >> fShow_TdTd >> fShow_PhPhPhPh;
	in >> fShow_dddff >> fShow_Gff >> fShow_VVVV >> fShow_ww >> fShow_Status;
	in >> fUseSingleColor >> itsSingleColor;
	in >> itsFontSize >> itsPlotSpacing;

	// Lopuksi vielä mahdollinen extra data ja niiden käsittely.
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;

	in >> extraData;

	if(in.fail())
		throw std::runtime_error("NFmiMetarPlotSettings::InitFromViewMacroString failed");
}

