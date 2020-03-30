#pragma once

#include "CombinedMapHandlerInterface.h"

#include <map>
#include <string>
#include <memory>

namespace Gdiplus
{
    class Bitmap;
}

// Luokka pitää sisällään yhden karttanäytön mahdolliset erilliset rajaviivan piirto
// bitmapit, joita on tarkoitus käyttää näytön piirrossa 'leimasimina'.
// Eri näyttöriveillä voi olla erilaiset säädöt, joten jokainen rivi saa potentiaalisesti
// oman bitmapin cacheen talteen.
// Cachen avain koostuu seuraavista asioista: 
// 1. Viivan paksuus pikseleissä (0-3 ja 0 on tällöin ei piirtoa) esim. 1
// 2. Viivan väri esim. musta joka ilmaistaa seuraavasti RRGGBB (eli RGB arvot heksana 00 on 0 ja FF on 255) esim. musta olisi 000000 ja valkoinen olisi FFFFFF
// Avain kokonaisena esim. 1_000000 joka olisi viivan paksuus 1 ja väri musta.
// Koska jokaisella näytöllä on oma cache, voidaan tämä cache siivota kokonaisuudessaan, aina kun esim. zooaus muuttuu tai tehdään jotain mikä
// muuttaa pohjakartan kokoa tms.
// Eri rivit voivat jakaa cache-kuvia, koska yhdessä näytössä on aina sama kartta-alue ja koko joka karttaruudulla.
class NFmiCountryBorderBitmapCache
{
    std::map<std::string, std::unique_ptr<Gdiplus::Bitmap>> bitmapCacheMap_;
public:
    NFmiCountryBorderBitmapCache();
    NFmiCountryBorderBitmapCache(const NFmiCountryBorderBitmapCache& other);
    NFmiCountryBorderBitmapCache& operator=(const NFmiCountryBorderBitmapCache& other);
    ~NFmiCountryBorderBitmapCache();

    Gdiplus::Bitmap* getCacheBitmap(const std::string& keyString) const;
    void insertCacheBitmap(const std::string& keyString, std::unique_ptr<Gdiplus::Bitmap> &&cacheBitmap);
    void clearCache();
    void setBorderDrawDirtyState(CountryBorderDrawDirtyState newState, const std::string& keyString = "");
};
