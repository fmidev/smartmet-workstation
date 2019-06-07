#include "stdafx.h"
#include <locale>
#include <codecvt>

// Tällä funktiolla on tarkoitus muuttaa annettu 8-bit stringi Win32 sopivaksi UNICODE stringiksi.
// Konversio tehdään yrittämällä ensiksi käyttää C++11 mukana tulleita utf8->wstring -konversiota.
// Tämä konversio kuitenkin epäonnistuu, jos käytössä oli ei-utf8 stringi, jossa onkin esim. ääkkösiä.
// Näissä tapauksissa konversio tehdään Win32:en CA2T makrolla.
// Tarve funktiolle tuli kun Kirgisiassa on tehty synop qdataa, jossa on asemien nimissä käytetty
// kyrillisiä kirjaimia ja ne on talletettu siis utf8:ina. Juttu juontaa myös siihen että Linuxissa
// normi 8-bit stringiä käsitellään utf8:ina, mutta Windowsissa asciina.
// HUOM! Pitää keksiä miten hanskataan tietyt stringit, mitkä auheuttavat poikkeuksen from_bytes -funktiossa,
// Olen huomannut että jos lähtö stringissä on merkkejä, jotka ovat yli 128, tällöin poikkeus lentää, esim.
// aste-merkki ja ääkköset.
std::wstring convertPossibleUtf8StringToWideString(const std::string& utf8str)
{
    if(utf8str.empty())
        return std::wstring();
    else
    {
        try
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
            return convert.from_bytes(utf8str);
        }
        catch(...)
        {
            return std::wstring(CA2T(utf8str.c_str()));
        }
    }
}
