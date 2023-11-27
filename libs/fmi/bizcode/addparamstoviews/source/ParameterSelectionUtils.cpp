#include "ParameterSelectionUtils.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace
{
    auto findDataRowItemIterator(const std::string& uniqueRowItemId, const std::vector<AddParams::SingleRowItem>& dialogRowData)
    {
        return std::find_if(dialogRowData.begin(), dialogRowData.end(),
            [&uniqueRowItemId]
        (const auto& singleRowItem)
            {
                return uniqueRowItemId == singleRowItem.uniqueDataId();
            });
    }
}

namespace AddParams
{
    const SingleRowItem* findDataRowItem(const std::string &uniqueRowItemId, const std::vector<SingleRowItem> &dialogRowData)
    {
        auto iter = ::findDataRowItemIterator(uniqueRowItemId, dialogRowData);

        if(iter != dialogRowData.end())
            return &(*iter);
        else
            return nullptr;
    }

    // Etsit‰‰n tietyn datan tietyn parametrin row-item
    const SingleRowItem* findParameterDataRowItem(const std::string& uniqueRowItemId, unsigned long parameterId, const std::vector<SingleRowItem>& dialogRowData)
    {
        auto iter = ::findDataRowItemIterator(uniqueRowItemId, dialogRowData);

        if(iter != dialogRowData.end())
        {
            // Liikutetaan iteraattoria datan 1. parametrin rivin kohdalle
            ++iter;
            // Kun tietty data on lˆytynyt, etsit‰‰n siit‰ mahdollisesti lˆytyv‰ parametri ennen kuin seuraava data tai loppu tulee kohdolle
            for( ; iter != dialogRowData.end(); ++iter)
            {
                if(!iter->uniqueDataId().empty())
                    break; // 1. rivi, jolla on jokin uniikki id stringi, on mennyt l‰pi nykyisen data jutut
                if(iter->itemId() == parameterId)
                    return &(*iter);
            }
        }

        return nullptr;
    }

}
