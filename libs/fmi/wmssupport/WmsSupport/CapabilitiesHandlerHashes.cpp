#include "wmssupport/CapabilitiesHandlerHashes.h"

namespace Wms
{
CapabilitiesHandlerHashes::CapabilitiesHandlerHashes() = default;

CapabilitiesHandlerHashes::CapabilitiesHandlerHashes(const CapabilitiesHandlerHashes &other) = default;

// Pitää palauttaa kopio LayerInfo:sta, koska tämä hashes rakenne saattaa vaihtaa
// sisältö-oliota minä hetkenä hyvänsä, jolloin palautettu LayerInfo pointteri/referenssi
// osoittaisi deletoituun olioon.
LayerInfo CapabilitiesHandlerHashes::getLayerInfo(const NFmiDataIdent& dataIdent) const
{
    auto producerId = dataIdent.GetProducer()->GetIdent();
    auto paramId = dataIdent.GetParamIdent();
    try
    {
        return hashes_.at(producerId).at(paramId);
    }
    catch(...)
    {
        // Logging warning for missing layerInfo
        std::string warningMessage = "Unable to find layerInfo for Producer: ";
        warningMessage += dataIdent.GetProducer()->GetName();
        warningMessage += " (prod-id: ";
        warningMessage += std::to_string(producerId);
        warningMessage += "), Parameter: ";
        warningMessage += dataIdent.GetParamName();
        warningMessage += " (par-id: ";
        warningMessage += std::to_string(paramId);
        warningMessage += ")";
        throw std::runtime_error(warningMessage);
    }
}

}
