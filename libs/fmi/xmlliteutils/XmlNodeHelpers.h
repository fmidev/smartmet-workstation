#include "xmlliteutils/XMLite.h"
#include <string>

namespace XmlLiteUtils
{
    // XML viestin purku helppereit�
    const std::string AttributeValue(LPXNode theNode, const std::string &theAttributeName);
    const std::string ChildNodeValue(LPXNode theNode, const std::string &theChildNodeName, bool fDoReferenceConversion = true); // fDoReferenceConversion=true tarkoittaa ett� esim. merkki: '<' muutetaan ".lt":ksi
    const std::string MakeNodeStr(const std::string &theNodeName, bool fStartNode);
}

