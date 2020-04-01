#include "WmsSupportDefines.h"

namespace Wms
{
    bool CustomIdentCompare::operator() (const NFmiDataIdent& el1, const NFmiDataIdent& el2) const
    {
        auto producerId1 = el1.GetProducer()->GetIdent();
        auto producerId2 = el2.GetProducer()->GetIdent();

        auto paramId1 = el1.GetParamIdent();
        auto paramId2 = el2.GetParamIdent();

        if(producerId1 == producerId2)
        {
            return paramId1 < paramId2;
        }
        return producerId1 < producerId2;
    }
}
