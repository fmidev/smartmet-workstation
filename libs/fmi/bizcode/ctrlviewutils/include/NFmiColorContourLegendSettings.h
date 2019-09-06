//© Ilmatieteenlaitos/Marko.
// Original 4.9.2019
// 
// Luokka hanskaa color contourien yhteyteen piirrettävien värilegendojen 
// piirto-ominaisuuksia. Ominaisuudet luetaan SmartMetin konfiguraatioista.
//---------------------------------------------------------- NFmiColorContourLegendSettings.h

#pragma once

#include "NFmiColor.h"
#include "NFmiPoint.h"
#include <string>

class NFmiColorRectSettings
{
    double frameLineWidthInMM_ = 0.1;
    NFmiColor frameLineColor_ = NFmiColor(0, 0, 0, 1);
    int frameLineType_ = 0; // 0=Solid,1=Dash,2=Dot,3=DashDot,4=DashDotDot,5=Custom
    NFmiColor fillColor_ = NFmiColor(1, 1, 1, 1);
public:
    NFmiColorRectSettings() = default;
    void initFromSettings(const std::string &initialNameSpace);

    double frameLineWidthInMM() const { return frameLineWidthInMM_; }
    void frameLineWidthInMM(double newValue) { frameLineWidthInMM_ = newValue; }
    const NFmiColor& frameLineColor() const { return frameLineColor_; }
    void frameLineColor(const NFmiColor &newColor) { frameLineColor_ = newColor; }
    int frameLineType() const { return frameLineType_; }
    void frameLineType(int newValue) { frameLineType_ = newValue; }
    const NFmiColor& fillColor() const { return fillColor_; }
    void fillColor(const NFmiColor &newColor) { fillColor_ = newColor; }
};

class NFmiColorContourLegendSettings
{
    NFmiColorRectSettings backgroundRectSettings_;
    NFmiColorRectSettings invsibleColorRectSettings_;
    double fontSizeInMM_ = 3;
    NFmiColor fontColor_ = NFmiColor(0, 0, 0, 1);
    std::string fontName_ = "Arial";
    // Tämän verran laitetaan karttaruudun vasemman reuna ja 1. legenda laatikon väliin (skaalataan karttaruudun koon mukaan)
    double spaceBetweenLegendsInMM_ = 0;
    // Legenda laatikoiden piirto aloitetaan (1. piirretyn color-contour layerin legenda kenttä) bottom-left 
    // paikka 0,0 - 1,1 laatikossa (Huom! vasen yläkulma on 0,0 ja oikea alakulma on 1,1)
    NFmiPoint relativeStartPosition_ = NFmiPoint(0, 0.85);

    bool initialized_ = false;
    std::string initialNameSpace_;
public:

    NFmiColorContourLegendSettings(void) = default;
    void initFromSettings(const std::string &initialNameSpace);

    const NFmiColorRectSettings& backgroundRectSettings() const { return  backgroundRectSettings_; }
    void backgroundRectSettings(const NFmiColorRectSettings &newValue) { backgroundRectSettings_ = newValue; }
    const NFmiColorRectSettings& invsibleColorRectSettings() const { return  invsibleColorRectSettings_; }
    void invsibleColorRectSettings(const NFmiColorRectSettings &newValue) { invsibleColorRectSettings_ = newValue; }
    double fontSizeInMM() const { return  fontSizeInMM_; }
    void fontSizeInMM(double newValue) { fontSizeInMM_ = newValue; }
    const NFmiColor& fontColor() const { return  fontColor_; }
    void fontColor(const NFmiColor &newColor) { fontColor_ = newColor; }
    const std::string& fontName() const { return fontName_; }
    void fontName(const std::string& newValue) { fontName_ = newValue; }
    double spaceBetweenLegendsInMM() const { return spaceBetweenLegendsInMM_; }
    void spaceBetweenLegendsInMM(double newValue) { spaceBetweenLegendsInMM_ = newValue; }
    const NFmiPoint& relativeStartPosition() const { return  relativeStartPosition_; }
    void relativeStartPosition(const NFmiPoint &newValue) { relativeStartPosition_ = newValue; }

private:
    void initializeInvisibleColorRectSettings();
};


