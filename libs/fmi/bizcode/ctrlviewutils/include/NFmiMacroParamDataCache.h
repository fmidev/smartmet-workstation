#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"
#include "NFmiPtrList.h"
#include <map>

class NFmiDrawParam;
class NFmiDrawParamList;

// Cacheen dataksi laitetaan itse laskettu data-matriisi, sek� tieto siit�,
// onko smarttool laskuissa k�ytetty ns. CalculationPoint:eja.
class NFmiMacroParamLayerCacheDataType
{
public:
    NFmiMacroParamLayerCacheDataType() = default;
    NFmiMacroParamLayerCacheDataType(const NFmiDataMatrix<float> &dataMatrix, bool useCalculationPoints)
    :dataMatrix_(dataMatrix)
    ,useCalculationPoints_(useCalculationPoints)
    {}

    NFmiDataMatrix<float> dataMatrix_;
    bool useCalculationPoints_ = false;
};


// Yhteen piirtolayeriin liittyv�t yhden macroParamin datat (kaikki ajat)
class NFmiMacroParamDataCacheLayer
{
    using LayerCacheType = std::map<NFmiMetTime, NFmiMacroParamLayerCacheDataType>;
    LayerCacheType layerCache_;
    std::string macroParamTotalPath_;
public:
    NFmiMacroParamDataCacheLayer() = default;
    NFmiMacroParamDataCacheLayer(const std::string &macroParamTotalPath);
    void setCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiMacroParamLayerCacheDataType &cacheData);
    bool getCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiMacroParamLayerCacheDataType &cacheDataOut);
    const std::string& macroParamTotalPath() const { return macroParamTotalPath_; }
    void clearLayerCache();
};

// Yhden n�ytt�rivin kaikki macroParameihin liittyv�t piirtolayerit
class NFmiMacroParamDataCacheRow
{
    using LayersCacheType = std::map<unsigned long, NFmiMacroParamDataCacheLayer>;
    // Eri layerit on eroteltu piirtolayerin numerolla (layerin indeksit alkavat 1:st�)
    LayersCacheType layersCache_;
public:
    NFmiMacroParamDataCacheRow() = default;
    void clearMacroParamCache(const std::string &macroParamTotalPath);
    void clearMacroParamCache(const std::vector<std::string> &macroParamTotalPathList);
    void setCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiMacroParamLayerCacheDataType &cacheData);
    bool getCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiMacroParamLayerCacheDataType &cacheDataOut);
    bool update(NFmiDrawParamList &drawParamList);
private:
    bool tryToMoveExistingLayerCache(unsigned long layerIndex, const NFmiDrawParam &drawParam, LayersCacheType &newLayersCacheInOut);
    void swapCacheDataFromOriginalToNew(unsigned long layerIndex, LayersCacheType &originalLayersCache, LayersCacheType::iterator &iterToOriginal, LayersCacheType &newLayersCache);
};

// Yhden n�yt�n kaikki macroParameihin liittyv�t data cachet (kaikki sen rivit ja niiden piirtolayerit)
class NFmiMacroParamDataCacheForView
{
    // N�yt�n eri rivit on eroteltu rivin absoluuttisella indeksill� (rivien indeksit alkavat 1:st�)
    std::map<unsigned long, NFmiMacroParamDataCacheRow> rowsCache_;
public:
    NFmiMacroParamDataCacheForView() = default;
    void clearAllLayers();
    void clearMacroParamCache(unsigned long rowIndex, const std::string &macroParamTotalPath);
    void clearMacroParamCache(const std::vector<std::string> &macroParamTotalPathList);
    void setCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiMacroParamLayerCacheDataType &cacheData);
    bool getCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiMacroParamLayerCacheDataType &cacheDataOut);
    bool update(unsigned long rowIndex, NFmiDrawParamList &drawParamList);
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
    void clearMacroParamCache(unsigned long viewIndex, unsigned long rowIndex, const std::string &macroParamTotalPath);
    void clearMacroParamCache(const std::vector<std::string> &macroParamTotalPathList);
    void setCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiMacroParamLayerCacheDataType &cacheData);
    bool getCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiMacroParamLayerCacheDataType &cacheDataOut);
    bool update(unsigned long viewIndex, unsigned long rowIndex, NFmiDrawParamList &drawParamList);
    bool update(unsigned long viewIndex, NFmiPtrList<NFmiDrawParamList>* drawParamListVector);
};
