#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"
#include <map>

// Yhteen piirtolayeriin liittyv�t yhden macroParamin datat (kaikki ajat)
class NFmiMacroParamDataCacheLayer
{
    std::map<NFmiMetTime, NFmiDataMatrix<float>> layerCache_;
    std::string macroParamTotalPath_;
public:
    NFmiMacroParamDataCacheLayer(const std::string &macroParamTotalPath);
    void setCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);
};

// Yhden n�ytt�rivin kaikki macroParameihin liittyv�t piirtolayerit
class NFmiMacroParamDataCacheRow
{
    // Eri layerit on eroteltu piirtolayerin numerolla (layerin indeksit alkavat 1:st�)
    std::map<unsigned long, NFmiMacroParamDataCacheLayer> layersCache_;
public:
    NFmiMacroParamDataCacheRow() = default;
    void setCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);

};

// Yhden n�yt�n kaikki macroParameihin liittyv�t data cachet (kaikki sen rivit ja niiden piirtolayerit)
class NFmiMacroParamDataCacheForView
{
    // N�yt�n eri rivit on eroteltu rivin absoluuttisella indeksill� (rivien indeksit alkavat 1:st�)
    std::map<unsigned long, NFmiMacroParamDataCacheRow> rowsCache_;
public:
    NFmiMacroParamDataCacheForView() = default;
    void clearAllLayers();
    void setCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);
};

// Kokonais cache macroParamiin liittyviin datoihin SmartMetissa (k�sitt�� kaikki kartta- ja poikkileikkausn�yt�t)
class NFmiMacroParamDataCache
{
    // Eri n�yt�t on eroteltu n�ytt��n liittyv�ll� indeksill� (karttan�yt�t ovat nyt 0-2 ja poikkileikkausn�ytt� on 98 kFmiCrossSectionView)
    std::map<unsigned long, NFmiMacroParamDataCacheForView> viewsCache_;
public:
    NFmiMacroParamDataCache() = default;
    bool init(const std::initializer_list<unsigned long> &viewIndexList);
    void clearAllLayers();
    void clearView(unsigned long viewIndex);
    void setCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);
};
