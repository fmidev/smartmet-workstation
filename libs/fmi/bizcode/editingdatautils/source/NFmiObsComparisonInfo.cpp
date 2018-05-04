//**********************************************************
// C++ Class Name : NFmiObsComparisonInfo
// ---------------------------------------------------------
// Filetype: (SOURCE)
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
//
//  Change Log:
//
//**********************************************************
#include "NFmiObsComparisonInfo.h"
#include "NFmiSettings.h"
#include "NFmiDataStoringHelpers.h"

#include <algorithm>

#ifdef OLDGCC
 #include <strstream>
#else
 #include <sstream>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

NFmiObsComparisonInfo::NFmiObsComparisonInfo(void)
:itsComparisonMode(0)
,fDrawBorders(false)
,itsSymbolSize()
,itsSymbolType(1)
,itsOkColor()
,itsUnderLowColor()
,itsOverHighColor()
,itsExtrapolationOkColor()
,itsExtrapolationUnderLowColor()
,itsExtrapolationOverHighColor()
,itsParams()
{
}

void NFmiObsComparisonInfo::Init(void)
{
	itsComparisonMode = NFmiSettings::Require<int>("MetEditor::ObservationComparisonInfo::Mode");
	fDrawBorders = NFmiSettings::Require<bool>("MetEditor::ObservationComparisonInfo::DrawBorders");
	itsSymbolSize.X(NFmiSettings::Require<double>("MetEditor::ObservationComparisonInfo::SymbolSizeX"));
	itsSymbolSize.Y(NFmiSettings::Require<double>("MetEditor::ObservationComparisonInfo::SymbolSizeY"));
	itsSymbolType = NFmiSettings::Require<int>("MetEditor::ObservationComparisonInfo::SymbolType");

	ReadColorFromSettings(itsOkColor, "MetEditor::ObservationComparisonInfo::OkColor");
	ReadColorFromSettings(itsUnderLowColor, "MetEditor::ObservationComparisonInfo::UnderLowColor");
	ReadColorFromSettings(itsOverHighColor, "MetEditor::ObservationComparisonInfo::OverHighColor");

	ReadColorFromSettings(itsExtrapolationOkColor, "MetEditor::ObservationComparisonInfo::ExtrapolationOkColor");
	ReadColorFromSettings(itsExtrapolationUnderLowColor, "MetEditor::ObservationComparisonInfo::ExtrapolationUnderLowColor");
	ReadColorFromSettings(itsExtrapolationOverHighColor, "MetEditor::ObservationComparisonInfo::ExtrapolationOverHighColor");

	ReadColorFromSettings(itsFrameColor, "MetEditor::ObservationComparisonInfo::FrameColor");

	ReadParamsSettingsFromInfo();
}

void NFmiObsComparisonInfo::ReadParamsSettingsFromInfo(void)
{
	itsParams.clear();
	NFmiObsComparisonInfo::Param tmp = ReadParamSettingsFromInfo("T");
	itsParams.push_back(tmp);
	tmp = ReadParamSettingsFromInfo("P");
	itsParams.push_back(tmp);
	tmp = ReadParamSettingsFromInfo("DP");
	itsParams.push_back(tmp);
	tmp = ReadParamSettingsFromInfo("RH");
	itsParams.push_back(tmp);
	tmp = ReadParamSettingsFromInfo("WS");
	itsParams.push_back(tmp);
	tmp = ReadParamSettingsFromInfo("WD");
	itsParams.push_back(tmp);
	tmp = ReadParamSettingsFromInfo("N");
	itsParams.push_back(tmp);
	tmp = ReadParamSettingsFromInfo("RR");
	itsParams.push_back(tmp);
}

NFmiObsComparisonInfo::Param NFmiObsComparisonInfo::ReadParamSettingsFromInfo(const std::string &theParamStr)
{
	std::string baseString("MetEditor::ObservationComparisonInfo::Params::");
	baseString += theParamStr;
	baseString += "::";
	std::string idString(baseString);
	idString += "Id";
	std::string lowLimitString(baseString);
	lowLimitString += "LowLimit";
	std::string highLimitString(baseString);
	highLimitString += "HighLimit";
	std::string decimalCountString(baseString);
	decimalCountString += "DecimalCount";
	std::string conversionFactorString(baseString);
	conversionFactorString += "ConversionFactor";
	NFmiObsComparisonInfo::Param tmp;

	// n‰ille pit‰‰ olla arvot asetuksissa
	tmp.itsId = NFmiSettings::Require<int>(idString.c_str());
	tmp.itsLowLimit = NFmiSettings::Require<float>(lowLimitString.c_str());
	tmp.itsHighLimit = NFmiSettings::Require<float>(highLimitString.c_str());

	// n‰ille ei tarvitse olla arvoja
	tmp.itsDecimalCount = NFmiSettings::Optional<int>(decimalCountString.c_str(), 1);
	tmp.itsConversionFactor = NFmiSettings::Optional<float>(conversionFactorString.c_str(), 1.f);

	return tmp;
}

// HUOM!! T‰m‰ laittaa kommentteja mukaan!
void NFmiObsComparisonInfo::Param::Write(std::ostream& os) const
{
	os << "// NFmiObsComparisonInfo::Param::Write..." << std::endl;

	os << "// ParId + LowLimit + HighLimit + DecimalCount + ConversionFactor" << std::endl;
	os << itsId << " " << itsLowLimit << " " << itsHighLimit << " " << itsDecimalCount <<  " " << itsConversionFactor << std::endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Param::Write failed");
}

// HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
// sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
void NFmiObsComparisonInfo::Param::Read(std::istream& is)
{
	is >> itsId >> itsLowLimit >> itsHighLimit >> itsDecimalCount >> itsConversionFactor;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Param::Read failed");

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Param::Read failed");
}

void NFmiObsComparisonInfo::ReadColorFromSettings(NFmiColor &theColor, const std::string &theKeyString)
{
	// v‰rin luku vaatii hieman kikkailua stringstreamin kanssa
	std::string tmp;
	tmp = NFmiSettings::Require<std::string>(theKeyString.c_str());
#ifdef OLDGCC
	std::istrstream colorStream(tmp);
#else
	std::stringstream colorStream(tmp);
#endif
	colorStream >> theColor;
}

void NFmiObsComparisonInfo::OkColor(const NFmiColor& newValue)
{
	itsOkColor = newValue;
	WriteColorToSettings(itsOkColor, "MetEditor::ObservationComparisonInfo::OkColor");
}
void NFmiObsComparisonInfo::UnderLowColor(const NFmiColor& newValue)
{
	itsUnderLowColor = newValue;
	WriteColorToSettings(itsUnderLowColor, "MetEditor::ObservationComparisonInfo::UnderLowColor");
}
void NFmiObsComparisonInfo::OverHighColor(const NFmiColor& newValue)
{
	itsOverHighColor = newValue;
	WriteColorToSettings(itsOverHighColor, "MetEditor::ObservationComparisonInfo::OverHighColor");
}
void NFmiObsComparisonInfo::ExtrapolationOkColor(const NFmiColor& newValue)
{
	itsExtrapolationOkColor = newValue;
	WriteColorToSettings(itsExtrapolationOkColor, "MetEditor::ObservationComparisonInfo::ExtrapolationOkColor");
}
void NFmiObsComparisonInfo::ExtrapolationUnderLowColor(const NFmiColor& newValue)
{
	itsExtrapolationUnderLowColor = newValue;
	WriteColorToSettings(itsExtrapolationUnderLowColor, "MetEditor::ObservationComparisonInfo::ExtrapolationUnderLowColor");
}
void NFmiObsComparisonInfo::ExtrapolationOverHighColor(const NFmiColor& newValue)
{
	itsExtrapolationOverHighColor = newValue;
	WriteColorToSettings(itsExtrapolationOverHighColor, "MetEditor::ObservationComparisonInfo::ExtrapolationOverHighColor");
}
void NFmiObsComparisonInfo::FrameColor(const NFmiColor& newValue)
{
	itsFrameColor = newValue;
	WriteColorToSettings(itsFrameColor, "MetEditor::ObservationComparisonInfo::FrameColor");
}

void NFmiObsComparisonInfo::WriteColorToSettings(NFmiColor &theColor, const std::string &theKeyString)
{
#ifdef OLDGCC
	std::ostrstream colorStream;
#else
	std::stringstream colorStream;
#endif
	colorStream << theColor;

	NFmiSettings::Set(theKeyString.c_str(), colorStream.str(), true);
}

void NFmiObsComparisonInfo::ComparisonMode(int newState)
{
	itsComparisonMode = newState;
#ifdef OLDGCC
	std::ostrstream oStream;
#else
	std::stringstream oStream;
#endif
	oStream << itsComparisonMode;
	NFmiSettings::Set("MetEditor::ObservationComparisonInfo::Mode", oStream.str(), true);
}

void NFmiObsComparisonInfo::DrawBorders(bool newState)
{
	fDrawBorders = newState;
	NFmiSettings::Set("MetEditor::ObservationComparisonInfo::DrawBorders", fDrawBorders ? "1" : "0", true);
}

void NFmiObsComparisonInfo::SymbolSize(const NFmiPoint &newValue)
{
	itsSymbolSize = newValue;
	SaveSymbolSizeToSettings();
}

void NFmiObsComparisonInfo::SaveSymbolSizeToSettings(void)
{
	NFmiSettings::Set("MetEditor::ObservationComparisonInfo::SymbolSizeX", NFmiStringTools::Convert<double>(itsSymbolSize.X()), true);
	NFmiSettings::Set("MetEditor::ObservationComparisonInfo::SymbolSizeY", NFmiStringTools::Convert<double>(itsSymbolSize.Y()), true);
}

void NFmiObsComparisonInfo::SymbolType(int newValue)
{
	itsSymbolType = newValue;
	NFmiSettings::Set("MetEditor::ObservationComparisonInfo::SymbolType", NFmiStringTools::Convert<int>(itsSymbolType), true);
}

void NFmiObsComparisonInfo::NextSymbolSize(void)
{
	itsSymbolSize += NFmiPoint(5, 5);
	if(itsSymbolSize.X() > 26)
	{
		itsSymbolSize.X(11);
		itsSymbolSize.Y(11);
	}
	SaveSymbolSizeToSettings();
}

void NFmiObsComparisonInfo::NextSymbolType(void)
{
	itsSymbolType++;
	if(itsSymbolType > 2)
		itsSymbolType = 1;
	NFmiSettings::Set("MetEditor::ObservationComparisonInfo::SymbolType", NFmiStringTools::Convert<int>(itsSymbolType), true);
}

struct FindParam
{
	FindParam(int theParId)
	:itsParId(theParId)
	{}

	bool operator()(const NFmiObsComparisonInfo::Param &theParam) const
	{
		return theParam.itsId == itsParId;
	}

	int itsParId;
};

const NFmiObsComparisonInfo::Param& NFmiObsComparisonInfo::GetParam(int theParId)
{
	static NFmiObsComparisonInfo::Param dummy;
	checkedVector<NFmiObsComparisonInfo::Param>::iterator it = std::find_if(itsParams.begin(), itsParams.end(), FindParam(theParId));
	if(it != itsParams.end())
		return *it;
	return dummy;
}

void NFmiObsComparisonInfo::NextComparisonMode(void)
{
	itsComparisonMode++;
	if(itsComparisonMode > 2)
		itsComparisonMode = 0;
	ComparisonMode(itsComparisonMode); // t‰m‰ huolehtii ett‰ asetuksiin talletus tapahtuu
}

// HUOM!! T‰m‰ laittaa kommentteja mukaan!
void NFmiObsComparisonInfo::Write(std::ostream& os) const
{
	os << "// NFmiObsComparisonInfo::Write..." << std::endl;

	os << "// ComparisonMode + DrawBorders + SymbolType" << std::endl;
	os << itsComparisonMode << " " << fDrawBorders << " " << itsSymbolType << std::endl;

	os << "// SymbolSize" << std::endl;
	os << itsSymbolSize << std::endl;

	os << "// itsOkColor" << std::endl;
	os << itsOkColor << std::endl;

	os << "// UnderLowColor" << std::endl;
	os << itsUnderLowColor << std::endl;

	os << "// OverHighColor" << std::endl;
	os << itsOverHighColor << std::endl;

	os << "// ExtrapolationOkColor" << std::endl;
	os << itsExtrapolationOkColor << std::endl;

	os << "// ExtrapolationUnderLowColor" << std::endl;
	os << itsExtrapolationUnderLowColor << std::endl;

	os << "// ExtrapolationOverHighColor" << std::endl;
	os << itsExtrapolationOverHighColor << std::endl;

	os << "// FrameColor" << std::endl;
	os << itsFrameColor << std::endl;

	os << "// Vector<NFmiObsComparisonInfo::Param>" << std::endl;
	NFmiDataStoringHelpers::WriteContainer(itsParams, os, "");

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Write failed");
}

// HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
// sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
void NFmiObsComparisonInfo::Read(std::istream& is)
{
	is >> itsComparisonMode >> fDrawBorders >> itsSymbolType;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
	is >> itsSymbolSize;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
	is >> itsOkColor;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
	is >> itsUnderLowColor;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
	is >> itsOverHighColor;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
	is >> itsExtrapolationOkColor;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
	is >> itsExtrapolationUnderLowColor;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
	is >> itsExtrapolationOverHighColor;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
	is >> itsFrameColor;

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
	NFmiDataStoringHelpers::ReadContainer(itsParams, is);

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw std::runtime_error("NFmiObsComparisonInfo::Read failed");
}
