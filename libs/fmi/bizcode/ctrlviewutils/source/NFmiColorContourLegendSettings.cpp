#include "NFmiColorContourLegendSettings.h"
#include "NFmiSettings.h"
#include "SettingsFunctions.h"

void NFmiColorRectSettings::InitFromSettings(const std::string &initialNameSpace)
{
    frameLineWidthInMM_ = NFmiSettings::Optional<double>(initialNameSpace + "::frameLineWidthInMM", frameLineWidthInMM_);
    frameLineColor_ = SettingsFunctions::GetColorFromSettings(initialNameSpace + "::frameLineColor", &frameLineColor_);
    frameLineType_ = NFmiSettings::Optional<int>(initialNameSpace + "::frameLineType", frameLineType_);
    fillColor_ = SettingsFunctions::GetColorFromSettings(initialNameSpace + "::frameFillColor", &fillColor_);

}

void NFmiColorContourLegendSettings::InitFromSettings(const std::string &initialNameSpace)
{
    if(initialized_)
        throw std::runtime_error(std::string(__FUNCTION__) + ": all ready initialized");

    initialized_ = true;
    initialNameSpace_ = initialNameSpace;
    backgroundRectSettings_.InitFromSettings(initialNameSpace + "::backgroundRectSettings");
    initializeInvisibleColorRectSettings();

    fontSizeInMM_ = NFmiSettings::Optional<double>(initialNameSpace + "::fontSizeInMM", fontSizeInMM_);
    fontColor_ = SettingsFunctions::GetColorFromSettings(initialNameSpace + "::fontColor", &fontColor_);
    fontName_ = NFmiSettings::Optional<std::string>(initialNameSpace + "::fontName", fontName_);
    spaceBetweenLegendsInMM_ = NFmiSettings::Optional<double>(initialNameSpace + "::spaceBetweenLegendsInMM", spaceBetweenLegendsInMM_);
    try
    {
        // Yritet‰‰n hakea arvoa asetuksista, jos niit‰ ei lˆydy, lent‰‰ poikkeus ja oletusarvo s‰ilyy
        relativeStartPosition_ = SettingsFunctions::GetCommaSeaparatedPointFromSettings(initialNameSpace + "::relativeStartPosition");
    }
    catch(...)
    { 
        // Ei tarvitse tehd‰ mit‰‰n, t‰m‰ on normaali tilanne
    }
}

void NFmiColorContourLegendSettings::initializeInvisibleColorRectSettings()
{
    // Laitetaan viivan paksuus normaalia isommaksi
    invsibleColorRectSettings_.frameLineWidthInMM(0.4);
    // Frame color punaiseksi
    invsibleColorRectSettings_.frameLineColor(NFmiColor(1, 0, 0, 1));
    // Laitetaan frame line tyyliksi 1 = dash eli ---
    invsibleColorRectSettings_.frameLineType(1);
    // Asetetaan t‰m‰ fill-v‰ri tyhj‰ksi (alpha on 0) 
    invsibleColorRectSettings_.fillColor(NFmiColor(0,0,0,0));
}
