#include "NFmiColorContourLegendSettings.h"
#include "NFmiSettings.h"
#include "SettingsFunctions.h"
#include "catlog/catlog.h"

void NFmiColorRectSettings::initFromSettings(const std::string &initialNameSpace)
{
    frameLineWidthInMM_ = NFmiSettings::Optional<double>(initialNameSpace + "::frameLineWidthInMM", frameLineWidthInMM_);
    frameLineColor_ = SettingsFunctions::GetColorFromSettings(initialNameSpace + "::frameLineColor", &frameLineColor_);
    frameLineType_ = NFmiSettings::Optional<int>(initialNameSpace + "::frameLineType", frameLineType_);
    fillColor_ = SettingsFunctions::GetColorFromSettings(initialNameSpace + "::frameFillColor", &fillColor_);
}

void NFmiColorRectSettings::doColorAlphaFixes(bool invertColorAlphaValues)
{
    if(invertColorAlphaValues)
    {
        frameLineColor_.InvertAlphaChannel();
        fillColor_.InvertAlphaChannel();
    }
}

void NFmiColorContourLegendSettings::initFromSettings(const std::string &initialNameSpace)
{
    if(initialized_)
        throw std::runtime_error(std::string(__FUNCTION__) + ": all ready initialized");

    initialized_ = true;
    initialNameSpace_ = initialNameSpace;
    backgroundRectSettings_.initFromSettings(initialNameSpace + "::backgroundRectSettings");
    initializeInvisibleColorRectSettings();

    fontSizeInMM_ = NFmiSettings::Optional<double>(initialNameSpace + "::fontSizeInMM", fontSizeInMM_);
    fontColor_ = SettingsFunctions::GetColorFromSettings(initialNameSpace + "::fontColor", &fontColor_);
    fontName_ = NFmiSettings::Optional<std::string>(initialNameSpace + "::fontName", fontName_);
    spaceBetweenLegendsInMM_ = NFmiSettings::Optional<double>(initialNameSpace + "::spaceBetweenLegendsInMM", spaceBetweenLegendsInMM_);
    try
    {
        relativeStartPosition_ = SettingsFunctions::GetCommaSeparatedPointFromSettings(initialNameSpace + "::relativeStartPosition", &relativeStartPosition_);
    }
    catch(std::exception &e)
    { 
        std::string logMessage = "Exception occured when reading color-contour legend setting 'relativeStartPosition': ";
        logMessage += e.what();
        logMessage += ". Will use default setting instead";
        CatLog::logMessage(logMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
    }
    drawTransparentRects_ = NFmiSettings::Optional<bool>(initialNameSpace + "::drawTransparentRects", drawTransparentRects_);
    invertColorAlphaValues_ = NFmiSettings::Optional<bool>(initialNameSpace + "::invertColorAlphaValues", invertColorAlphaValues_);
    doColorAlphaFixes();
}

void NFmiColorContourLegendSettings::initializeInvisibleColorRectSettings()
{
    // Laitetaan viivan paksuus normaalia isommaksi
    invsibleColorRectSettings_.frameLineWidthInMM(0.4);
    // Frame color punaiseksi
    invsibleColorRectSettings_.frameLineColor(NFmiColor(1, 0, 0, 1));
    // Laitetaan frame line tyyliksi 1 = dash eli ---
    invsibleColorRectSettings_.frameLineType(1);
    // Asetetaan tämä fill-väri tyhjäksi (alpha on 0) 
    invsibleColorRectSettings_.fillColor(NFmiColor(0,0,0,0));
}

void NFmiColorContourLegendSettings::doColorAlphaFixes()
{
    if(invertColorAlphaValues_)
    {
        backgroundRectSettings_.doColorAlphaFixes(invertColorAlphaValues_);
        invsibleColorRectSettings_.doColorAlphaFixes(invertColorAlphaValues_);
        fontColor_.InvertAlphaChannel();
    }
}
