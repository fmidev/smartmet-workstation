// NFmiViewPosRegistryInfo-luokka pit‰‰ sis‰ll‰‰n tiettyj‰ tietoja mit‰ tarvitaan
// kun n‰ytˆn koko ja sijainti laitetaan rekisteriin. T‰m‰ ei siis hallitse itse 
// rekisteri‰, mutta tiet‰‰ avaimen ja ikkunan default aseman.
#pragma once

#include <string>

class NFmiViewPosRegistryInfo
{
public:
    NFmiViewPosRegistryInfo(const CRect &rectDefault, const std::string &winRegistryKeyStr);
    const CRect& DefaultWindowRect(void) const {return mRectDefault;}
    const std::string& WinRegistryKeyStr(void) const {return mWinRegistryKeyStr;}
private:
    CRect mRectDefault; // oletus koko jos rekisteriss‰ ei ole arvoa
    std::string mWinRegistryKeyStr;
};
