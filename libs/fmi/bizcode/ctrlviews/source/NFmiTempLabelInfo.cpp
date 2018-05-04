#include "NFmiTempLabelInfo.h"
#include "NFmiDataStoringHelpers.h"

void NFmiTempLabelInfo::Write(std::ostream& os) const
{
    os << "// NFmiMTATempSystem::LabelInfo::Write..." << std::endl;

    os << "// StartPointPixelOffSet" << std::endl;
    os << itsStartPointPixelOffSet;
    os << "// TextAlignment + FontSize + DrawLabelText + ClipWithDataRect" << std::endl;
    os << itsTextAlignment << " " << itsFontSize << " " << fDrawLabelText << " " << fClipWithDataRect << std::endl;

    NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel� mahdollinen extra data
                                                            // Kun tulee uusia muuttujia, tee t�h�n extradatan t�ytt��, jotta se saadaan talteen tiedopstoon siten ett�
                                                            // edelliset versiot eiv�t mene solmuun vaikka on tullut uutta dataa.
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
    NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel� mahdollinen extra data
    is >> extraData;
    // T�ss� sitten otetaaan extradatasta talteen uudet muuttujat, mit� on mahdollisesti tullut
    // eli jos uusia muutujia tai arvoja, k�sittele t�ss�.

    if(is.fail())
        throw std::runtime_error("NFmiMTATempSystem::LabelInfo::Read failed");
}
