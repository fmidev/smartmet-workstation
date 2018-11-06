#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"
#include "NFmiPtrList.h"
#include <map>

class NFmiDrawParam;
class NFmiDrawParamList;

// Yhteen piirtolayeriin liittyvät yhden macroParamin datat (kaikki ajat)
class NFmiMacroParamDataCacheLayer
{
    using LayerCacheType = std::map<NFmiMetTime, NFmiDataMatrix<float>>;
    LayerCacheType layerCache_;
    std::string macroParamTotalPath_;
public:
    NFmiMacroParamDataCacheLayer() = default;
    NFmiMacroParamDataCacheLayer(const std::string &macroParamTotalPath);
    void setCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);
    const std::string& macroParamTotalPath() const { return macroParamTotalPath_; }
};

// Yhden näyttörivin kaikki macroParameihin liittyvät piirtolayerit
class NFmiMacroParamDataCacheRow
{
    using LayersCacheType = std::map<unsigned long, NFmiMacroParamDataCacheLayer>;
    // Eri layerit on eroteltu piirtolayerin numerolla (layerin indeksit alkavat 1:stä)
    LayersCacheType layersCache_;
public:
    NFmiMacroParamDataCacheRow() = default;
    void setCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData);
    bool getCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut);
    bool update(NFmiDrawParamList &drawParamList);
private:
    bool tryToMoveExistingLayerCache(unsigned long layerIndex, const NFmiDrawParam &drawParam, LayersCacheType &newLayersCacheInOut);
    void swapCacheDataFromOriginalToNew(unsigned long layerIndex, LayersCacheType &originalLayersCache, LayersCacheType::iterator &iterToOriginal, LayersCacheType &newLayersCache);
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
    bool update(unsigned long rowIndex, NFmiDrawParamList &drawParamList);
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
    bool update(unsigned long viewIndex, unsigned long rowIndex, NFmiDrawParamList &drawParamList);
    bool update(unsigned long viewIndex, NFmiPtrList<NFmiDrawParamList>* drawParamListVector);
};
