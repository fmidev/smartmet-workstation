//� Ilmatieteenlaitos/Marko.
// Original 4.9.2019
// 
// Luokka hanskaa color contourien yhteyteen piirrett�vien v�rilegendojen 
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
    void doColorAlphaFixes(bool invertColorAlphaValues);
};

class NFmiColorContourLegendSettings
{
    NFmiColorRectSettings backgroundRectSettings_;
    NFmiColorRectSettings invsibleColorRectSettings_;
    double fontSizeInMM_ = 5;
    NFmiColor fontColor_ = NFmiColor(0, 0, 0, 1);
    std::string fontName_ = "Arial";
    // T�m�n verran laitetaan karttaruudun vasemman reuna ja 1. legenda laatikon v�liin (skaalataan karttaruudun koon mukaan)
    double spaceBetweenLegendsInMM_ = 0;
    // Legenda laatikoiden piirto aloitetaan (1. piirretyn color-contour layerin legenda kentt�) bottom-left 
    // paikka 0,0 - 1,1 laatikossa (Huom! vasen yl�kulma on 0,0 ja oikea alakulma on 1,1)
    NFmiPoint relativeStartPosition_ = NFmiPoint(0, 0.85);
    bool drawTransparentRects_ = false;
    // invertColorAlphaValues_:in pit�� olla true, kunnes SmartMet versio 5.13.15.0 on vanhin 
    // k�yt�ss� olevista versioista ja silloin t�m� voidaan muuttaa false:ksi ja muuttaa 
    // color_contour_legend_settings.conf tiedoston v�riasetukset niin ett� full opacity 
    // on 0.f ja full transparency on 1.f.
    bool invertColorAlphaValues_ = true;

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
    bool drawTransparentRects() const { return drawTransparentRects_; }
    void drawTransparentRects(bool newValue) { drawTransparentRects_ = newValue; }

private:
    void initializeInvisibleColorRectSettings();
    void doColorAlphaFixes();
};


