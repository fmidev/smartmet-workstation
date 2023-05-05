#include "NFmiMacroParamDataCache.h"
#include "NFmiDrawParamList.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiFastQueryInfo.h"
#include "catlog/catlog.h"

// Kiinnostava map:in tyhjennys predikaatin avulla -funktio
template< typename ContainerT, typename PredicateT >
void erase_if_from_map(ContainerT& items, const PredicateT& predicate) 
{
    for(auto it = items.begin(); it != items.end(); ) 
    {
        if(predicate(*it)) 
            it = items.erase(it);
        else 
            ++it;
    }
};

static void logMacroParamTotalPathWasInCorrectWarning(const std::string &functionName, const std::string &path1, const std::string &path2)
{
    std::string message = functionName;
    message += " had wrong macro-param total path: ";
    message += path1;
    message += " vs ";
    message += path2;
    CatLog::logMessage(message, CatLog::Severity::Warning, CatLog::Category::Data);
}

NFmiMacroParamLayerCacheDataType::NFmiMacroParamLayerCacheDataType() = default;

NFmiMacroParamLayerCacheDataType::NFmiMacroParamLayerCacheDataType(const NFmiMacroParamLayerCacheDataType& other)
:dataMatrix_(other.dataMatrix_)
,useCalculationPoints_(other.useCalculationPoints_)
,useAlReadySpacedOutData_(other.useAlReadySpacedOutData_)
,dataAreaPtr_(other.dataAreaPtr_ ? other.dataAreaPtr_->Clone() : nullptr)
{
}

NFmiMacroParamLayerCacheDataType& NFmiMacroParamLayerCacheDataType::operator=(const NFmiMacroParamLayerCacheDataType& other)
{
    if(this != &other)
    {
        dataMatrix_ = other.dataMatrix_;
        useCalculationPoints_ = other.useCalculationPoints_;
        useAlReadySpacedOutData_ = other.useAlReadySpacedOutData_;
        dataAreaPtr_.reset(other.dataAreaPtr_ ? other.dataAreaPtr_->Clone() : nullptr);
    }
    return *this;
}

void NFmiMacroParamLayerCacheDataType::setCacheValues(const NFmiDataMatrix<float> &dataMatrix, bool useCalculationPoints, bool useAlReadySpacedOutData, const NFmiArea* dataArea)
{
    dataMatrix_ = dataMatrix;
    useCalculationPoints_ = useCalculationPoints;
    useAlReadySpacedOutData_ = useAlReadySpacedOutData;
    dataAreaPtr_.reset(dataArea ? dataArea->Clone() : nullptr);
}

static bool IsGridSizeSame(NFmiDataMatrix<float> &matrix, boost::shared_ptr<NFmiFastQueryInfo> &info)
{
    if(info)
    {
        if(matrix.NX() == info->GridXNumber() && matrix.NY() == info->GridYNumber())
            return true;
    }
    return false;
}

void NFmiMacroParamLayerCacheDataType::getCacheValues(NFmiDataMatrix<float> &dataMatrixOut, bool &useCalculationPointsOut, bool &useAlReadySpacedOutDataOut, boost::shared_ptr<NFmiFastQueryInfo> &usedInfoInOut)
{
    dataMatrixOut = dataMatrix_;
    useCalculationPointsOut = useCalculationPoints_;
    useAlReadySpacedOutDataOut = useAlReadySpacedOutData_;
    if(useAlReadySpacedOutDataOut || !::IsGridSizeSame(dataMatrixOut, usedInfoInOut))
    {
        // Jos cacheen laskettu data oli jo harvennettua, pit‰‰ myˆs nyt k‰ytetty info s‰‰t‰‰ takaisin kyseiseen oikeaan hilakokoon.
        // Lis‰ksi pit‰‰ varmistaa ett‰ infossa on oikea datan alue.
        usedInfoInOut = NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(static_cast<int>(dataMatrixOut.NX()), static_cast<int>(dataMatrixOut.NY()), NFmiInfoData::kMacroParam, dataAreaPtr_.get());
    }
}

// T‰m‰ tarkistaa, onko dataMatrix_ tyhj‰ vai ei.
bool NFmiMacroParamLayerCacheDataType::isEmpty() const
{
    if(dataMatrix_.NX() && dataMatrix_.NY())
        return false;
    else
        return true;
}

// ***********************************************************************************************


NFmiMacroParamDataCacheLayer::NFmiMacroParamDataCacheLayer(const std::string &macroParamTotalPath)
:layerCache_()
,macroParamTotalPath_(macroParamTotalPath)
{
}

void NFmiMacroParamDataCacheLayer::setCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiMacroParamLayerCacheDataType &cacheData)
{
    if(macroParamTotalPath == macroParamTotalPath_)
    {
        auto iter = layerCache_.find(time);
        if(iter != layerCache_.end())
        {
            // P‰ivitet‰‰n cache dataa jostain syyst‰, en tied‰ mik‰ tapaus menisi t‰nne
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

bool NFmiMacroParamDataCacheLayer::getCache(const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiMacroParamLayerCacheDataType &cacheDataOut)
{
    if(macroParamTotalPath == macroParamTotalPath_)
    {
        auto iter = layerCache_.find(time);
        if(iter != layerCache_.end())
        {
            cacheDataOut = iter->second;
            return true;
        }
    }
    else
        ::logMacroParamTotalPathWasInCorrectWarning(__FUNCTION__, macroParamTotalPath, macroParamTotalPath_);

    return false;
}

void NFmiMacroParamDataCacheLayer::clearLayerCache()
{
    layerCache_.clear();
}


// ***********************************************************************************************

// Tyhjennet‰‰n vain kaikki layerit, joissa on kyseinen macroParamPath
void NFmiMacroParamDataCacheRow::clearMacroParamCache(const std::string &macroParamTotalPath)
{
    for(auto &layer : layersCache_) 
    {
        if(layer.second.macroParamTotalPath() == macroParamTotalPath)
            layer.second.clearLayerCache();
    }
}

void NFmiMacroParamDataCacheRow::clearMacroParamCache(const std::vector<std::string> &macroParamTotalPathList)
{
    for(const auto &macroParamPathName : macroParamTotalPathList)
    {
        clearMacroParamCache(macroParamPathName);
    }
}

void NFmiMacroParamDataCacheRow::setCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiMacroParamLayerCacheDataType &cacheData)
{
    auto iter = layersCache_.find(layerIndex);
    if(iter != layersCache_.end())
    {
        iter->second.setCache(time, macroParamTotalPath, cacheData);
    }
    else
    {
        // Pit‰‰ luoda uusi layer-cache, koska haettua layeri‰ ei lˆytynyt
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

bool NFmiMacroParamDataCacheRow::getCache(unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiMacroParamLayerCacheDataType &cacheDataOut)
{
    auto iter = layersCache_.find(layerIndex);
    if(iter != layersCache_.end())
    {
        return iter->second.getCache(time, macroParamTotalPath, cacheDataOut);
    }
    return false;
}

bool NFmiMacroParamDataCacheRow::getTotalCache(unsigned long layerIndex, const std::string& macroParamTotalPath, NFmiMacroParamDataCacheLayer& cacheDataOut)
{
    auto iter = layersCache_.find(layerIndex);
    if(iter != layersCache_.end())
    {
        cacheDataOut = iter->second;
        return true;
    }
    return false;
}

// Tehd‰‰n cachen n‰yttˆriville p‰ivitys. Jos esim. jotain on poistettu tai lis‰tty rivilt‰/riville.
// T‰ss‰ siis tarvittaessa otetaan cachen data yhdelt‰ layer-indeksilt‰ talteen, sitten
// luodaan uusi map olio ja siirret‰‰n cache-data t‰lle uudelle map-oliolle ja lopuksi originaali 
// viel‰ poistetaan mapista.
// Siirtyneiden macroParamien tunnistus tehd‰‰n k‰ytt‰m‰ll‰ macroParamTotalPath:ia.
// Mit‰ jos samalla rivill‰ on kahdessa eri layerissa sama macroParam???? 
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
                // Jos ei lˆytynyt listasta valmista layerCachea, niin luodaan sitten uusi sellainen
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
        // Katsotaan lˆytyikˆ sama macroParam samasta kohtaa layer-listasta
        if(layerIter->second.macroParamTotalPath() == drawParamTotalPath)
        {
            // Jos lˆytyi, siirret‰‰n cache originaalista uuteen cacheen ja poistetaan se originaalista
            swapCacheDataFromOriginalToNew(layerIndex, layersCache_, layerIter, newLayersCacheInOut);
            return true;
        }
    }

    // Etsit‰‰n 1. originaali listasta lˆytynyt oikealla macroParam tiedostopolulla oleva otus ja 
    // siirret‰‰n se uuteen listaan.
    auto samePathItemIter = std::find_if(layersCache_.begin(), layersCache_.end(), [&drawParamTotalPath](const auto &layerCache) {return layerCache.second.macroParamTotalPath() == drawParamTotalPath; });
    if(samePathItemIter != layersCache_.end())
    {
        swapCacheDataFromOriginalToNew(layerIndex, layersCache_, samePathItemIter, newLayersCacheInOut);
        return true;
    }

    return false;
}

// ***********************************************************************************************

static void logRowIndexNotFoundWarning(const std::string &functionName, unsigned long rowIndex)
{
    std::string errorString = functionName;
    errorString += " cannot find macroParam row-cache with rowIndex ";
    errorString += std::to_string(rowIndex);
    CatLog::logMessage(errorString, CatLog::Severity::Warning, CatLog::Category::Data);
}

void NFmiMacroParamDataCacheForView::clearAllLayers()
{
    // Ei menn‰ n‰yttˆtasoa pidemm‰lle kun tehd‰‰n t‰ysi tyhjennys
    rowsCache_.clear();
}

void NFmiMacroParamDataCacheForView::clearMacroParamCacheRow(unsigned long rowIndex)
{
    auto removedCount = rowsCache_.erase(rowIndex);
    if(removedCount == 0)
    {
        ::logRowIndexNotFoundWarning(__FUNCTION__, rowIndex);
    }
}

void NFmiMacroParamDataCacheForView::clearMacroParamCache(unsigned long rowIndex, const std::string &macroParamTotalPath)
{
    if(!rowsCache_.empty())
    {
        auto iter = rowsCache_.find(rowIndex);
        if(iter != rowsCache_.end())
        {
            iter->second.clearMacroParamCache(macroParamTotalPath);
        }
        else
        {
            ::logRowIndexNotFoundWarning(__FUNCTION__, rowIndex);
        }
    }
}

void NFmiMacroParamDataCacheForView::clearMacroParamCache(const std::vector<std::string> &macroParamTotalPathList)
{
    for(auto &rowCache : rowsCache_)
    {
        rowCache.second.clearMacroParamCache(macroParamTotalPathList);
    }
}

void NFmiMacroParamDataCacheForView::setCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiMacroParamLayerCacheDataType &cacheData)
{
    auto iter = rowsCache_.find(rowIndex);
    if(iter != rowsCache_.end())
    {
        return iter->second.setCache(layerIndex, time, macroParamTotalPath, cacheData);
    }
    else
    {
        // Pit‰‰ luoda uusi row-cache, koska haettua rivi‰ ei lˆytynyt
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

bool NFmiMacroParamDataCacheForView::getCache(unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiMacroParamLayerCacheDataType &cacheDataOut)
{
    auto iter = rowsCache_.find(rowIndex);
    if(iter != rowsCache_.end())
    {
        return iter->second.getCache(layerIndex, time, macroParamTotalPath, cacheDataOut);
    }
    return false;
}

bool NFmiMacroParamDataCacheForView::getTotalCache(unsigned long rowIndex, unsigned long layerIndex, const std::string& macroParamTotalPath, NFmiMacroParamDataCacheLayer& cacheDataOut)
{
    auto iter = rowsCache_.find(rowIndex);
    if(iter != rowsCache_.end())
    {
        return iter->second.getTotalCache(layerIndex, macroParamTotalPath, cacheDataOut);
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

void NFmiMacroParamDataCacheForView::swapRows(unsigned long rowIndex1, unsigned long rowIndex2)
{
    auto iter1 = rowsCache_.find(rowIndex1);
    auto iter2 = rowsCache_.find(rowIndex2);
    if(iter1 != rowsCache_.end() && iter2 != rowsCache_.end())
    {
        std::swap(iter1->second, iter2->second);
    }
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

// T‰t‰ kutsutaan kun mm. ladataan n‰yttˆmakroa ja kaikki macroParam cachet pit‰‰ ensin poistaa systeemist‰
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
    // N‰yttˆ-cacheja ei tyhjennet‰ ikin‰, siksi halutun n‰ytˆn pit‰‰ aina lˆyty‰!
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

void NFmiMacroParamDataCache::clearMacroParamCacheRow(unsigned long viewIndex, unsigned long rowIndex)
{
    // N‰yttˆ-cacheja ei tyhjennet‰ ikin‰, siksi halutun n‰ytˆn pit‰‰ aina lˆyty‰!
    auto iter = viewsCache_.find(viewIndex);
    if(iter != viewsCache_.end())
    {
        iter->second.clearMacroParamCacheRow(rowIndex);
    }
    else
    {
        ::logViewIndexNotFoundWarning(__FUNCTION__, viewIndex);
    }
}

void NFmiMacroParamDataCache::clearMacroParamCache(unsigned long viewIndex, unsigned long rowIndex, const std::string &macroParamTotalPath)
{
    // N‰yttˆ-cacheja ei tyhjennet‰ ikin‰, siksi halutun n‰ytˆn pit‰‰ aina lˆyty‰!
    auto iter = viewsCache_.find(viewIndex);
    if(iter != viewsCache_.end())
    {
        iter->second.clearMacroParamCache(rowIndex, macroParamTotalPath);
    }
    else
    {
        ::logViewIndexNotFoundWarning(__FUNCTION__, viewIndex);
    }
}

void NFmiMacroParamDataCache::clearMacroParamCache(const std::vector<std::string> &macroParamTotalPathList)
{
    for(auto &viewCache : viewsCache_)
    {
        viewCache.second.clearMacroParamCache(macroParamTotalPathList);
    }
}

void NFmiMacroParamDataCache::setCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, const NFmiMacroParamLayerCacheDataType &cacheData)
{
    if(cacheData.isEmpty())
        return; // Ei vied‰ tyhj‰‰ dataa cacheen, siit‰ tulisi ongelmia

    // N‰yttˆ-cacheja ei tyhjennet‰ ikin‰, siksi halutun n‰ytˆn pit‰‰ aina lˆyty‰!
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

bool NFmiMacroParamDataCache::getCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const NFmiMetTime &time, const std::string &macroParamTotalPath, NFmiMacroParamLayerCacheDataType &cacheDataOut)
{
    auto iter = viewsCache_.find(viewIndex);
    if(iter != viewsCache_.end())
    {
        return iter->second.getCache(rowIndex, layerIndex, time, macroParamTotalPath, cacheDataOut);
    }
    return false;
}

bool NFmiMacroParamDataCache::getTotalCache(unsigned long viewIndex, unsigned long rowIndex, unsigned long layerIndex, const std::string& macroParamTotalPath, NFmiMacroParamDataCacheLayer& cacheDataOut)
{
    auto iter = viewsCache_.find(viewIndex);
    if(iter != viewsCache_.end())
    {
        return iter->second.getTotalCache(rowIndex, layerIndex, macroParamTotalPath, cacheDataOut);
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

void NFmiMacroParamDataCache::swapMacroParamCacheRows(unsigned long viewIndex, unsigned long rowIndex1, unsigned long rowIndex2)
{
    if(rowIndex1 != rowIndex2)
    {
        auto iter = viewsCache_.find(viewIndex);
        if(iter != viewsCache_.end())
        {
            iter->second.swapRows(rowIndex1, rowIndex2);
        }
    }
}
