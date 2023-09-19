#pragma once
#include "NFmiColor.h"
#include "NFmiPoint.h"
#include "NFmiRect.h"
#include <vector>
#include <string>

enum class NFmiSymbolBulkDrawType
{
    NoType,
    Text,
    Arrow,
    HessaaSymbol,
    WindBarb,
    BitmapSymbol1 = 101,
    BitmapSymbol2 = 102,
    BitmapSymbol3 = 103
};

// Koska symbolin v‰rit vaihtelevat arvon mukaan?
enum class NFmiSymbolColorChangingType
{
    // Default, jos DrawParamin ShowColoredNumbers asetus niin sanoo.
    DrawParamSet,
    // Normaalisti muuttavat erikois v‰ritykset, jotka ohitetaan,
    // jos DrawParamin ShowColoredNumbers asetus on p‰‰ll‰.
    Mixed,
    // V‰ri‰ voi vaihtaa vain toisen parametrin avustuksella
    OnlyWithOtherParameterValues,
    // V‰ri‰ ei vaihdella arvon mukaan miss‰‰n tilanteessa.
    Never
};

class NFmiSymbolBulkDrawData
{
    NFmiSymbolBulkDrawType drawType_ = NFmiSymbolBulkDrawType::NoType;
    // Normaali luvun tekstipiirroissa pit‰‰ olla tasav‰linen fontti => Courier New on siksi default.
    std::wstring fontName_ = L"Arial";
    bool useBoldFont_ = false;
    // Jokaisen piirretyn pisteen arvo
    std::vector<float> values_;
    // Jokaisen tektstin‰ piirretyn pisteen sis‰ltˆ
    std::vector<std::string> drawnTexts_;
    // Millainen relatiivinen offset on asemapisteen ja piirretyn symbolin v‰lill‰.
    // Lopullinen piirtopisteen keskipiste saadaan, kun relativeStationPointPositions_ 
    // pisteeseen lis‰t‰‰n t‰m‰ offset arvo.
    NFmiPoint relativePositionOffset_;
    // Joillekin piirto tyyleill‰ on piirto-objekteilla xy suunnassa oleva vakio koko.
    // Esim. Wind-barb, bitmap piirto, jne. T‰m‰ on eri asia kuin symbolSizes_ arvo.
    // T‰m‰n ja relativePositionOffset_:in avulla voidaan rakentaa symbolin piirrossa 
    // tarvittava suhteellinen rect.
    NFmiPoint relativeDrawObjectSize_;
    bool isChangingSymbolColorsUsed_ = false;
    // Jos v‰rej‰ on vector:issa 1, piirret‰‰n kaikki sill‰ v‰rill‰, muuten jokaiselle arvolle pit‰‰ lˆyty‰ erillinen v‰ri
    std::vector<NFmiColor> colors_;
    // Kaikki symbolit piirret‰‰n joko yhden kokoisiksi (vector:issa 1 luku), tai sitten jokaiselle annetaan oma kokonsa
    std::vector<NFmiPoint> symbolSizes_;
    // Asemapisteiden piirto tulee myˆs mahdollisesti mukaan asetuksien mukaan. 
    // Asemapiste piirret‰‰n sitten aina symbolin p‰‰lle.
    bool drawStationPoint_ = false;
    // Mink‰ kokoinen laatikko piirret‰‰n, t‰t‰ laatikkoa vain siirrell‰‰n asemien pisteiden kohdalle
    NFmiRect baseStationPointRect_;
    // Mill‰ v‰rill‰ asemapiste piirret‰‰n
    NFmiColor stationPointColor_;
    // Jokaisen piirretyn asemapisteen latlon piste
    std::vector<NFmiPoint> stationPointLatlons_;
    // Jokaisen piirretyn asemapisteen sijainti toolboxin suhteellisessa 0,0 - 1,1 maailmassa
    std::vector<NFmiPoint> relativeStationPointPositions_;
    // Tietyt symbolit piirret‰‰n viivoilla, jonka kyn‰ on t‰m‰n paksuinen
    int penSize_ = 1;
    // Onko visualisointi k‰ynniss‰ printtaus prosessin yhteydess‰ vai ei.
    bool printing_ = false;
    NFmiPoint mapViewSizeInPixels_;
    // isOk metodi tekee tarvittaessa virheraportin, joka talletetaan t‰h‰n.
    // On mutable, koska sen arvoa muokataan isOk const metodissa (varsinainen data ei muutu).
    mutable std::string errorMessage_;
public:
    NFmiSymbolBulkDrawData();

    void clear();

    NFmiSymbolBulkDrawType drawType() const { return drawType_; }
    void drawType(NFmiSymbolBulkDrawType drawType) { drawType_ = drawType; }
    const std::wstring& fontName() const { return fontName_; }
    void fontName(const std::wstring& newValue)  { fontName_ = newValue; }
    bool useBoldFont() const { return useBoldFont_; }
    void useBoldFont(bool newValue) { useBoldFont_ = newValue; }
    const std::vector<float>& values() const { return values_; }
    void addValue(float value);
    void addValues(const std::vector<float>& values);
    const std::vector<std::string>& drawnTexts() const { return drawnTexts_; }
    void addDrawnText(const std::string &text);
    void addDrawnTexts(const std::vector<std::string>& texts);
    const NFmiPoint& relativePositionOffset() const { return relativePositionOffset_; }
    void relativePositionOffset(const NFmiPoint& offset) { relativePositionOffset_ = offset; }
    const NFmiPoint& relativeDrawObjectSize() const { return relativeDrawObjectSize_; }
    void relativeDrawObjectSize(const NFmiPoint& objectSize) { relativeDrawObjectSize_ = objectSize; }
    bool isChangingSymbolColorsUsed() const { return isChangingSymbolColorsUsed_; }
    void isChangingSymbolColorsUsed(bool newValue) { isChangingSymbolColorsUsed_ = newValue; }
    const std::vector<NFmiColor>& colors() const { return colors_; }
    void addColor(const NFmiColor& color);
    void addColors(const std::vector<NFmiColor>& colors);
    void setColor(const NFmiColor& color);
    const std::vector<NFmiPoint>& symbolSizes() const { return symbolSizes_; }
    void addSymbolSize(const NFmiPoint& symbolSize);
    void addSymbolSizes(const std::vector<NFmiPoint>& symbolSizes);
    void setSymbolSize(const NFmiPoint& symbolSize);

    bool drawStationPoint() const { return  drawStationPoint_; }
    void drawStationPoint(bool newValue) { drawStationPoint_ = newValue; }
    const NFmiRect& baseStationPointRect() const { return  baseStationPointRect_; }
    void baseStationPointRect(const NFmiRect& newValue) { baseStationPointRect_ = newValue; }
    const NFmiColor& stationPointColor() const { return  stationPointColor_; }
    void stationPointColor(const NFmiColor& newValue) { stationPointColor_ = newValue; }
    const std::vector<NFmiPoint>& stationPointLatlons() const { return stationPointLatlons_; }
    void addStationPointLatlons(const NFmiPoint& latlon);
    const std::vector<NFmiPoint>& relativeStationPointPositions() const { return relativeStationPointPositions_; }
    void addRelativeStationPointPosition(const NFmiPoint& position);
    void addRelativeStationPointPositions(const std::vector<NFmiPoint>& positions);
    int penSize() const { return penSize_; }
    void penSize(int newValue) { penSize_ = newValue; }
    bool printing() const { return printing_; }
    void printing(bool newValue) { printing_ = newValue; }
    const NFmiPoint& mapViewSizeInPixels() const { return mapViewSizeInPixels_; }
    void mapViewSizeInPixels(const NFmiPoint& newValue) { mapViewSizeInPixels_ = newValue; }

    bool isDataOk() const;
    bool hasAnyData() const;
    const std::string& errorMessage() const { return errorMessage_; }
    bool isChangingColorsAndSizesUsed() const;
    bool isChangingColorsUsed() const;
    bool isChangingSizesUsed() const;
private:
};
