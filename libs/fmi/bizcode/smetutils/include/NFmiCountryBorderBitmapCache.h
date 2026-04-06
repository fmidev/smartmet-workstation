#pragma once

#include "CombinedMapHandlerInterface.h"

#include <map>
#include <string>
#include <memory>

#ifndef UNIX
namespace Gdiplus
{
    class Bitmap;
}
#endif // UNIX

// Luokka pit�� sis�ll��n yhden karttan�yt�n mahdolliset erilliset rajaviivan piirto
// bitmapit, joita on tarkoitus k�ytt�� n�yt�n piirrossa 'leimasimina'.
// Eri n�ytt�riveill� voi olla erilaiset s��d�t, joten jokainen rivi saa potentiaalisesti
// oman bitmapin cacheen talteen.
// Cachen avain koostuu seuraavista asioista: 
// 1. Viivan paksuus pikseleiss� (0-3 ja 0 on t�ll�in ei piirtoa) esim. 1
// 2. Viivan v�ri esim. musta joka ilmaistaa seuraavasti RRGGBB (eli RGB arvot heksana 00 on 0 ja FF on 255) esim. musta olisi 000000 ja valkoinen olisi FFFFFF
// Avain kokonaisena esim. 1_000000 joka olisi viivan paksuus 1 ja v�ri musta.
// Koska jokaisella n�yt�ll� on oma cache, voidaan t�m� cache siivota kokonaisuudessaan, aina kun esim. zooaus muuttuu tai tehd��n jotain mik�
// muuttaa pohjakartan kokoa tms.
// Eri rivit voivat jakaa cache-kuvia, koska yhdess� n�yt�ss� on aina sama kartta-alue ja koko joka karttaruudulla.
#ifndef UNIX
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
#endif // UNIX
