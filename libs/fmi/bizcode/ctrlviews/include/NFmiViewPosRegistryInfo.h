// NFmiViewPosRegistryInfo-luokka pit�� sis�ll��n tiettyj� tietoja mit� tarvitaan
// kun n�yt�n koko ja sijainti laitetaan rekisteriin. T�m� ei siis hallitse itse 
// rekisteri�, mutta tiet�� avaimen ja ikkunan default aseman.
#pragma once

#include <string>

class NFmiViewPosRegistryInfo
{
public:
#ifndef UNIX
    NFmiViewPosRegistryInfo(const CRect &rectDefault, const std::string &winRegistryKeyStr);
    const CRect& DefaultWindowRect(void) const {return mRectDefault;}
#else
    NFmiViewPosRegistryInfo(const std::string &winRegistryKeyStr) : mWinRegistryKeyStr(winRegistryKeyStr) {}
    NFmiViewPosRegistryInfo() = default;
#endif
    const std::string& WinRegistryKeyStr(void) const {return mWinRegistryKeyStr;}
private:
#ifndef UNIX
    CRect mRectDefault; // oletus koko jos rekisteriss� ei ole arvoa
#endif
    std::string mWinRegistryKeyStr;
};
