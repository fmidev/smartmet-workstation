#include "NFmiTempLineInfo.h"

void NFmiTempLineInfo::Write(std::ostream& os) const
{
	os << "// NFmiMTATempSystem::LineInfo::Write..." << std::endl;

	os << "// Line Color" << std::endl;
	os << itsColor << std::endl;
	os << "// Line thickness + type + draw-it" << std::endl;
	os << itsThickness << " " << itsLineType << " " << fDrawLine << std::endl;

    NFmiDataStoringHelpers::NFmiExtraDataStorage extraData = FillExtraData(); // lopuksi viel� mahdollinen extra data
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiMTATempSystem::LineInfo::Write failed");
}

// T�m�n avulla kirjoitetaan streamiin vain data, ilman rivinvaihtoja ja kommentteja, paitsi extradatan kohdalla on endl
void NFmiTempLineInfo::WriteBasicData(std::ostream& os) const
{
    os << itsColor << " " << itsThickness << " " << itsLineType << " " << fDrawLine << std::endl;

    NFmiDataStoringHelpers::NFmiExtraDataStorage extraData = FillExtraData(); // lopuksi viel� mahdollinen extra data
    os << extraData;

    if(os.fail())
        throw std::runtime_error("NFmiMTATempSystem::LineInfo::WriteBasicData failed");
}

NFmiDataStoringHelpers::NFmiExtraDataStorage NFmiTempLineInfo::FillExtraData() const
{
    NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel� mahdollinen extra data
    // Kun tulee uusia muuttujia, tee t�h�n extradatan t�ytt��, jotta se saadaan talteen tiedopstoon siten ett�
    // edelliset versiot eiv�t mene solmuun vaikka on tullut uutta dataa.

    return extraData;
}

void NFmiTempLineInfo::Read(std::istream& is)
{ // toivottavasti olet poistanut kommentit luettavasta streamista!!
	is >> itsColor;
	is >> itsThickness;
	int tmpValue = 0;
	is >> tmpValue;
	itsLineType = static_cast<FmiPattern>(tmpValue);
	is >> fDrawLine;

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::LineInfo::Read failed");
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel� mahdollinen extra data
	is >> extraData;
	// T�ss� sitten otetaaan extradatasta talteen uudet muuttujat, mit� on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k�sittele t�ss�.

	if(is.fail())
		throw std::runtime_error("NFmiMTATempSystem::LineInfo::Read failed");
}
