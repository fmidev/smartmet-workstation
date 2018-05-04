#include "NFmiTempLabelInfo.h"
#include "NFmiDataStoringHelpers.h"

void NFmiTempLabelInfo::Write(std::ostream& os) const
{
    os << "// NFmiMTATempSystem::LabelInfo::Write..." << std::endl;

    os << "// StartPointPixelOffSet" << std::endl;
    os << itsStartPointPixelOffSet;
    os << "// TextAlignment + FontSize + DrawLabelText + ClipWithDataRect" << std::endl;
    os << itsTextAlignment << " " << itsFontSize << " " << fDrawLabelText << " " << fClipWithDataRect << std::endl;

    NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
                                                            // Kun tulee uusia muuttujia, tee tähän extradatan täyttöä, jotta se saadaan talteen tiedopstoon siten että
                                                            // edelliset versiot eivät mene solmuun vaikka on tullut uutta dataa.
    os << "// possible extra data" << std::endl;
    os << extraData;

    if(os.fail())
        throw std::runtime_error("NFmiMTATempSystem::LabelInfo::Write failed");
}

void NFmiTempLabelInfo::Read(std::istream& is)
{ // toivottavasti olet poistanut kommentit luettavasta streamista!!
    is >> itsStartPointPixelOffSet;
    int tmpValue = 0;
    is >> tmpValue;
    itsTextAlignment = static_cast<FmiDirection>(tmpValue);
    is >> itsFontSize >> fDrawLabelText >> fClipWithDataRect;

    if(is.fail())
        throw std::runtime_error("NFmiMTATempSystem::LabelInfo::Read failed");
    NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
    is >> extraData;
    // Tässä sitten otetaaan extradatasta talteen uudet muuttujat, mitä on mahdollisesti tullut
    // eli jos uusia muutujia tai arvoja, käsittele tässä.

    if(is.fail())
        throw std::runtime_error("NFmiMTATempSystem::LabelInfo::Read failed");
}
