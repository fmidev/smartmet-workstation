#ifndef UNIX
#include "stdafx.h"
#endif // UNIX
#include <locale>
#include <codecvt>
#include <string>

// T�ll� funktiolla on tarkoitus muuttaa annettu 8-bit stringi Win32 sopivaksi UNICODE stringiksi.
// Konversio tehd��n yritt�m�ll� ensiksi k�ytt�� C++11 mukana tulleita utf8->wstring -konversiota.
// T�m� konversio kuitenkin ep�onnistuu, jos k�yt�ss� oli ei-utf8 stringi, jossa onkin esim. ��kk�si�.
// N�iss� tapauksissa konversio tehd��n Win32:en CA2T makrolla.
// Tarve funktiolle tuli kun Kirgisiassa on tehty synop qdataa, jossa on asemien nimiss� k�ytetty
// kyrillisi� kirjaimia ja ne on talletettu siis utf8:ina. Juttu juontaa my�s siihen ett� Linuxissa
// normi 8-bit stringi� k�sitell��n utf8:ina, mutta Windowsissa asciina.
// HUOM! Pit�� keksi� miten hanskataan tietyt stringit, mitk� auheuttavat poikkeuksen from_bytes -funktiossa,
// Olen huomannut ett� jos l�ht� stringiss� on merkkej�, jotka ovat yli 128, t�ll�in poikkeus lent��, esim.
// aste-merkki ja ��kk�set.
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
#ifndef UNIX
            return std::wstring(CA2T(utf8str.c_str()));
#else
            // On Linux, strings are UTF-8; do a simple widening for the fallback
            return std::wstring(utf8str.begin(), utf8str.end());
#endif
        }
    }
}
