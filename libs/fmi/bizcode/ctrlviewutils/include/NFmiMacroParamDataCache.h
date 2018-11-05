#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"
#include <map>

// Yhteen piirtolayeriin liittyvät yhden macroParamin datat (kaikki ajat)
class NFmiMacroParamDataCacheLayer
{
    std::map<NFmiMetTime, NFmiDataMatrix<float>> layerCache_;
    std::string macroParamTotalPath_;
public:
    NFmiMacroParamDataCacheLayer(const std::string &macroParamTotalPath);
    void setCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);
};

// Yhden näyttörivin kaikki macroParameihin liittyvät piirtolayerit
class NFmiMacroParamDataCacheRow
{
    // Eri layerit on eroteltu piirtolayerin numerolla (layerin indeksit alkavat 1:stä)
    std::map<unsigned long, NFmiMacroParamDataCacheLayer> layersCache_;
public:
    NFmiMacroParamDataCacheRow() = default;
    void setCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);

};

// Yhden näytön kaikki macroParameihin liittyvät data cachet (kaikki sen rivit ja niiden piirtolayerit)
class NFmiMacroParamDataCacheForView
{
    // Näytön eri rivit on eroteltu rivin absoluuttisella indeksillä (rivien indeksit alkavat 1:stä)
    std::map<unsigned long, NFmiMacroParamDataCacheRow> rowsCache_;
public:
    NFmiMacroParamDataCacheForView() = default;
    void clearAllLayers();
    void setCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);
};

// Kokonais cache macroParamiin liittyviin datoihin SmartMetissa (käsittää kaikki kartta- ja poikkileikkausnäytöt)
class NFmiMacroParamDataCache
{
    // Eri näytöt on eroteltu näyttöön liittyvällä indeksillä (karttanäytöt ovat nyt 0-2 ja poikkileikkausnäyttö on 98 kFmiCrossSectionView)
    std::map<unsigned long, NFmiMacroParamDataCacheForView> viewsCache_;
public:
    NFmiMacroParamDataCache() = default;
    bool init(const std::initializer_list<unsigned long> &viewIndexList);
    void clearAllLayers();
    void clearView(unsigned long viewIndex);
    void setCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);
};
