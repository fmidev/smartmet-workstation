#include "SmartMetOpenUrlAction.h"
#include <string>
#include <map>

class NFmiPoint;

class NFmiMouseClickUrlActionData
{
    // Konffissa urlit annetaan tyyliin MyUrl_1 = https://develop.weatherproof.fi/...
    // Urlit laitetaan j�rjestykseen (url1:n laukaisee SHIFT+1 + left-click) sanan url per�ss� olevan numeron perusteella.
    // map:issa on siis se int numero, mist� tulee j�rjestys
    // map:in pair:issa on urlin kuvaava nimi string eli se alkuosa MyUrl_1:st� eli MyUrl, jos calme-case, niin sanat erotellaan (My Url)
    // Ja loppuun itse url string:iin.
    // Eli mapissa int=j�rjestysnumero, string=kuvaava nimi, string=url
    std::map<int, std::pair<std::string, std::string>> itsMouseActionUrls;
    std::string itsBaseNameSpace;
    static std::map<unsigned int, SmartMetOpenUrlAction> itsOpenUrlActionKeyMappings;
    float itsAreaRadiusInKm;
public:
    NFmiMouseClickUrlActionData();

    bool InitFromSettings(const std::string& baseNameSpace);
    const std::string& GetMouseActionBaseUrl(int index) const;
    std::string GetMouseActionUrl(SmartMetOpenUrlAction urlAction, const NFmiPoint &latlon) const;
    // Urlissa voi olla placeholder, mihin on tarkoitus sijoittaa halutun alueen latlonkp:s�de tiedot (25.0,60.2:150)
    static const std::map<unsigned int, SmartMetOpenUrlAction>& OpenUrlActionKeyMappings();
};
