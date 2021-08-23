#include "NFmiSymbolBulkDrawData.h"

NFmiSymbolBulkDrawData::NFmiSymbolBulkDrawData()
{
    // Optimoidaan piirron datan ker‰yksen aika teht‰v‰‰ arvojen push_back operaatioita
    // laittamalla tietyn kokoinen alkuarvaus vector:eiden tilantarpeelle. T‰ll‰ yritet‰‰n
    // v‰ltt‰‰ varsinkin pienten vector:ien pikkuisia uudelleen allakointeja, joita MSVC 
    // k‰‰nt‰j‰ jostain syyst‰ suosii (kasvu ei tapahdu 2:n potensseissa, kuten esim. gcc:ss‰).
    const size_t reservedVectorSize = 256;
    values_.reserve(reservedVectorSize);
    drawnTexts_.reserve(reservedVectorSize);
    colors_.reserve(reservedVectorSize);
    symbolSizes_.reserve(reservedVectorSize);
    stationPointLatlons_.reserve(reservedVectorSize);
    relativeStationPointPositions_.reserve(reservedVectorSize);
}

void NFmiSymbolBulkDrawData::clear()
{
    *this = NFmiSymbolBulkDrawData();
}

bool NFmiSymbolBulkDrawData::hasAnyData() const
{
    return !values_.empty();
}

bool NFmiSymbolBulkDrawData::isDataOk() const
{
    if(values_.empty())
    {
        // Saa olla kokonaan puuttuva data (alueelle ei osu yht‰‰n asemaa tms.), 
        // voidaan lopettaa tarkastelut heti
        return true;
    }
    else
    {
        if(drawType_ != NFmiSymbolBulkDrawType::NoType)
        {
            if(!fontName_.empty())
            {
                auto valuesSize = values_.size();
                if(colors_.size() == valuesSize || colors_.size() == 1)
                {
                    if(symbolSizes_.size() == valuesSize || symbolSizes_.size() == 1)
                    {
                        return true;
                    }
                    else
                        errorMessage_ = "error with symbol size values";
                }
                else
                    errorMessage_ = "error with symbol color values";
            }
            else
                errorMessage_ = "font name was missing";
        }
        else
            errorMessage_ = "draw type was missing";
    }

    return false;
}

void NFmiSymbolBulkDrawData::addValue(float value)
{
    values_.push_back(value);
}

void NFmiSymbolBulkDrawData::addValues(const std::vector<float>& values)
{
    values_.insert(std::end(values_), std::begin(values), std::end(values));
}

void NFmiSymbolBulkDrawData::addDrawnText(const std::string& text)
{
    drawnTexts_.push_back(text);
}

void NFmiSymbolBulkDrawData::addDrawnTexts(const std::vector<std::string>& texts)
{
    drawnTexts_.insert(std::end(drawnTexts_), std::begin(texts), std::end(texts));
}

void NFmiSymbolBulkDrawData::addColor(const NFmiColor& color)
{
    colors_.push_back(color);
}

// Jos piirrett‰v‰ data on koostettu monesta erillisest‰ queryDatasta, pit‰‰
// kokonaisdata mahdollisesti rakentaa osista (ellei k‰ytet‰ yht‰ v‰ri‰).
void NFmiSymbolBulkDrawData::addColors(const std::vector<NFmiColor>& colors)
{
    colors_.insert(std::end(colors_), std::begin(colors), std::end(colors));
}

// Asetetaan vain 1 ja ainoa v‰ri
void NFmiSymbolBulkDrawData::setColor(const NFmiColor& color)
{
    colors_ = std::vector<NFmiColor>{color};
}

void NFmiSymbolBulkDrawData::addSymbolSize(const NFmiPoint& symbolSize)
{
    symbolSizes_.push_back(symbolSize);
}

// Jos piirrett‰v‰ data on koostettu monesta erillisest‰ queryDatasta, pit‰‰
// kokonaisdata mahdollisesti rakentaa osista (ellei k‰ytet‰ vakio kokoa).
void NFmiSymbolBulkDrawData::addSymbolSizes(const std::vector<NFmiPoint>& symbolSizes)
{
    symbolSizes_.insert(std::end(symbolSizes_), std::begin(symbolSizes), std::end(symbolSizes));
}

// Asetetaan vain 1 ja ainoa symboolikoko
void NFmiSymbolBulkDrawData::setSymbolSize(const NFmiPoint& symbolSize)
{
    symbolSizes_ = std::vector<NFmiPoint>{ symbolSize };
}

void NFmiSymbolBulkDrawData::addStationPointLatlons(const NFmiPoint& latlon)
{
    stationPointLatlons_.push_back(latlon);
}

void NFmiSymbolBulkDrawData::addRelativeStationPointPosition(const NFmiPoint& position)
{
    relativeStationPointPositions_.push_back(position);
}

void NFmiSymbolBulkDrawData::addRelativeStationPointPositions(const std::vector<NFmiPoint>& positions)
{
    relativeStationPointPositions_.insert(std::end(relativeStationPointPositions_), std::begin(positions), std::end(positions));
}

bool NFmiSymbolBulkDrawData::isChangingColorsAndSizesUsed() const
{
    return isChangingColorsUsed() && isChangingSizesUsed();
}

bool NFmiSymbolBulkDrawData::isChangingColorsUsed() const
{
    return colors_.size() > 1;
}

bool NFmiSymbolBulkDrawData::isChangingSizesUsed() const
{
    return symbolSizes_.size() > 1;
}
