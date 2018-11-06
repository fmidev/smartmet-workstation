#include "NFmiMacroParamDataCache.h"
#include "NFmiDrawParamList.h"
#include "catlog/catlog.h"

static void logMacroParamTotalPathWasInCorrectWarning(const std::string &functionName, const std::string &path1, const std::string &path2)
{
    std::string message = functionName;
    message += " had wrong macro-param total path: ";
    message += path1;
    message += " vs ";
    message += path2;
    CatLog::logMessage(message, CatLog::Severity::Warning, CatLog::Category::Data);
}

NFmiMacroParamDataCacheLayer::NFmiMacroParamDataCacheLayer(const std::string &macroParamTotalPath)
:layerCache_()
,macroParamTotalPath_(macroParamTotalPath)
{
}

void NFmiMacroParamDataCacheLayer::setCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData)
{
    if(macroParamTotalPath == macroParamTotalPath_)
    {
        auto iter = layerCache_.find(time);
        if(iter != layerCache_.end())
        {
            // Päivitetään cache dataa jostain syystä, en tiedä mikä tapaus menisi tänne
            iter->second = cacheData;
        }
        else
        {
            auto insertIter = layerCache_.insert(std::make_pair(time, cacheData));
            if(!insertIter.second)
            {
                std::string errorString = __FUNCTION__;
                errorString += " cannot insert new macroParam layer-time-cache with time ";
                errorString += time.ToStr(kYYYYMMDDHHMM);
                errorString += " with path ";
                errorString += macroParamTotalPath_;
                CatLog::logMessage(errorString, CatLog::Severity::Warning, CatLog::Category::Data);
            }
        }
    }
    else
        ::logMacroParamTotalPathWasInCorrectWarning(__FUNCTION__, macroParamTotalPath, macroParamTotalPath_);
}

bool NFmiMacroParamDataCacheLayer::getCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut)
{
    if(macroParamTotalPath == macroParamTotalPath_)
    {
        auto iter = layerCache_.find(time);
        if(iter != layerCache_.end())
        {
            cacheDataOut.swap(iter->second);
            return true;
        }
    }
    else
        ::logMacroParamTotalPathWasInCorrectWarning(__FUNCTION__, macroParamTotalPath, macroParamTotalPath_);

    return false;
}


// ***********************************************************************************************

void NFmiMacroParamDataCacheRow::setCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData)
{
    auto iter = layersCache_.find(layerIndex);
    if(iter != layersCache_.end())
    {
        iter->second.setCache(time, macroParamTotalPath, cacheData);
    }
    else
    {
        // Pitää luoda uusi layer-cache, koska haettua layeriä ei löytynyt
        auto insertedIter = layersCache_.insert(std::make_pair(layerIndex, NFmiMacroParamDataCacheLayer(macroParamTotalPath)));
        if(insertedIter.second)
            insertedIter.first->second.setCache(time, macroParamTotalPath, cacheData);
        else
        {
            std::string errorString = __FUNCTION__;
            errorString += " cannot insert new macroParam layer-cache with layer-index ";
            errorString += std::to_string(layerIndex);
            CatLog::logMessage(errorString, CatLog::Severity::Warning, CatLog::Category::Data);
        }
    }
}

bool NFmiMacroParamDataCacheRow::getCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut)
{
    auto iter = layersCache_.find(layerIndex);
    if(iter != layersCache_.end())
    {
        return iter->second.getCache(time, macroParamTotalPath, cacheDataOut);
    }
    return false;
}

// Tehdään cachen näyttöriville päivitys. Jos esim. jotain on poistettu tai lisätty riviltä/riville.
// Tässä siis tarvittaessa otetaan cachen data yhdeltä layer-indeksiltä talteen, sitten
// luodaan uusi map olio ja siirretään cache-data tälle uudelle map-oliolle ja lopuksi originaali 
// vielä poistetaan mapista.
// Siirtyneiden macroParamien tunnistus tehdään käyttämällä macroParamTotalPath:ia.
// Mitä jos samalla rivillä on kahdessa eri layerissa sama macroParam???? 
bool NFmiMacroParamDataCacheRow::update(NFmiDrawParamList &drawParamList)
{
    LayersCacheType newLayerCache;
    unsigned long layerIndex = 1;
    for(auto &drawParam : drawParamList)
    {
        if(drawParam->IsMacroParamCase(true))
        {
            if(!tryToMoveExistingLayerCache(layerIndex, *drawParam, newLayerCache))
            {
                // Jos ei löytynyt listasta valmista layerCachea, niin luodaan sitten uusi sellainen
                auto insertedIter = newLayerCache.insert(std::make_pair(layerIndex, NFmiMacroParamDataCacheLayer(drawParam->InitFileName())));
            }
        }
        layerIndex++;
    }

    // Laitetaan lopuksi uusi luotu cache originaalin tilalle
    layersCache_.swap(newLayerCache);
    return true;
}

void NFmiMacroParamDataCacheRow::swapCacheDataFromOriginalToNew(unsigned long layerIndex, LayersCacheType &originalLayersCache, LayersCacheType::iterator &iterToOriginal, LayersCacheType &newLayersCache)
{
    std::swap(newLayersCache[layerIndex], iterToOriginal->second);
    originalLayersCache.erase(iterToOriginal);
}

bool NFmiMacroParamDataCacheRow::tryToMoveExistingLayerCache(unsigned long layerIndex, const NFmiDrawParam &drawParam, LayersCacheType &newLayersCacheInOut)
{
    const auto &drawParamTotalPath = drawParam.InitFileName();
    auto layerIter = layersCache_.find(layerIndex);
    if(layerIter != layersCache_.end())
    {
        // Katsotaan löytyikö sama macroParam samasta kohtaa layer-listasta
        if(layerIter->second.macroParamTotalPath() == drawParamTotalPath)
        {
            // Jos löytyi, siirretään cache originaalista uuteen cacheen ja poistetaan se originaalista
            swapCacheDataFromOriginalToNew(layerIndex, layersCache_, layerIter, newLayersCacheInOut);
            return true;
        }
    }

    // Etsitään 1. originaali listasta löytynyt oikealla macroParam tiedostopolulla oleva otus ja 
    // siirretään se uuteen listaan.
    auto samePathItemIter = std::find_if(layersCache_.begin(), layersCache_.end(), [&drawParamTotalPath](const auto &layerCache) {return layerCache.second.macroParamTotalPath() == drawParamTotalPath; });
    if(samePathItemIter != layersCache_.end())
    {
        swapCacheDataFromOriginalToNew(layerIndex, layersCache_, samePathItemIter, newLayersCacheInOut);
        return true;
    }

    return false;
}

// ***********************************************************************************************

void NFmiMacroParamDataCacheForView::clearAllLayers()
{
    // Ei mennä näyttötasoa pidemmälle kun tehdään täysi tyhjennys
    rowsCache_.clear();
}

void NFmiMacroParamDataCacheForView::setCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData)
{
    auto iter = rowsCache_.find(rowIndex);
    if(iter != rowsCache_.end())
    {
        return iter->second.setCache(layerIndex, time, macroParamTotalPath, cacheData);
    }
    else
    {
        // Pitää luoda uusi row-cache, koska haettua riviä ei löytynyt
        auto insertedIter = rowsCache_.insert(std::make_pair(rowIndex, NFmiMacroParamDataCacheRow()));
        if(insertedIter.second)
            insertedIter.first->second.setCache(layerIndex, time, macroParamTotalPath, cacheData);
        else
        {
            std::string errorString = __FUNCTION__;
            errorString += " cannot insert new macroParam row-cache with row-index ";
            errorString += std::to_string(rowIndex);
            CatLog::logMessage(errorString, CatLog::Severity::Warning, CatLog::Category::Data);
        }
    }
}

bool NFmiMacroParamDataCacheForView::getCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut)
{
    auto iter = rowsCache_.find(rowIndex);
    if(iter != rowsCache_.end())
    {
        return iter->second.getCache(layerIndex, time, macroParamTotalPath, cacheDataOut);
    }
    return false;
}

bool NFmiMacroParamDataCacheForView::update(unsigned long rowIndex, NFmiDrawParamList &drawParamList)
{
    auto iter = rowsCache_.find(rowIndex);
    if(iter != rowsCache_.end())
    {
        return iter->second.update(drawParamList);
    }
    return false;
}

// ***********************************************************************************************

bool NFmiMacroParamDataCache::init(const std::initializer_list<unsigned long> &viewIndexList)
{
    if(viewsCache_.empty())
    {
        for(auto viewIndex : viewIndexList)
        {
            viewsCache_.insert(std::make_pair(viewIndex, NFmiMacroParamDataCacheForView()));
        }
        return true;
    }

    // Cache oli jo alustettu
    return false;
}

// Tätä kutsutaan kun mm. ladataan näyttömakroa ja kaikki macroParam cachet pitää ensin poistaa systeemistä
void NFmiMacroParamDataCache::clearAllLayers()
{
    for(auto &viewCache : viewsCache_)
        viewCache.second.clearAllLayers();
}

static void logViewIndexNotFoundWarning(const std::string &functionName, unsigned long viewIndex)
{
    std::string errorString = functionName;
    errorString += " cannot find macroParam view-cache with viewIndex ";
    errorString += std::to_string(viewIndex);
    CatLog::logMessage(errorString, CatLog::Severity::Warning, CatLog::Category::Data);
}

void NFmiMacroParamDataCache::clearView(unsigned long viewIndex)
{
    auto iter = viewsCache_.find(viewIndex);
    if(iter != viewsCache_.end())
    {
        iter->second.clearAllLayers();
    }
    else
    {
        ::logViewIndexNotFoundWarning(__FUNCTION__, viewIndex);
    }
}

void NFmiMacroParamDataCache::setCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiDataMatrix<float> &cacheData)
{
    // Näyttö-cacheja ei tyhjennetä ikinä, siksi halutun näytön pitää aina löytyä!
    auto iter = viewsCache_.find(viewIndex);
    if(iter != viewsCache_.end())
    {
        iter->second.setCache(rowIndex, layerIndex, time, macroParamTotalPath, cacheData);
    }
    else
    {
        ::logViewIndexNotFoundWarning(__FUNCTION__, viewIndex);
    }
}

bool NFmiMacroParamDataCache::getCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiDataMatrix<float> &cacheDataOut)
{
    auto iter = viewsCache_.find(viewIndex);
    if(iter != viewsCache_.end())
    {
        return iter->second.getCache(rowIndex, layerIndex, time, macroParamTotalPath, cacheDataOut);
    }
    return false;
}

bool NFmiMacroParamDataCache::update(unsigned long viewIndex, unsigned long rowIndex, NFmiDrawParamList &drawParamList)
{
    auto iter = viewsCache_.find(viewIndex);
    if(iter != viewsCache_.end())
    {
        return iter->second.update(rowIndex, drawParamList);
    }
    return false;
}

bool NFmiMacroParamDataCache::update(unsigned long viewIndex, NFmiPtrList<NFmiDrawParamList>* drawParamListVector)
{
    if(drawParamListVector)
    {
        unsigned long rowIndex = 1;
        auto iter = drawParamListVector->Start();
        for(; iter.Next(); rowIndex++)
        {
            auto &drawParamList = iter.Current();
            update(viewIndex, rowIndex, drawParamList);
        }
    }
    return true;
}
