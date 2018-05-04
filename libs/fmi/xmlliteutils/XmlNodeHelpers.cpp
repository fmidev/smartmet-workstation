#include "xmlliteutils/XmlNodeHelpers.h"

namespace 
{
    std::string GetChildNodeText(LPXNode theRootNode, const std::string &theChildName, bool fDoReferenceConversion)
    {
        LPXNode childNode = theRootNode->Find(CA2T(theChildName.c_str()));
        if(childNode)
        {
            DISP_OPT options;
            options.reference_value = fDoReferenceConversion;
            std::string tmp = CT2A(childNode->GetText(&options));
            return tmp;
        }
        else
            return std::string();
    }
}

namespace XmlLiteUtils
{

    const std::string AttributeValue(LPXNode theNode, const std::string &theAttributeName)
    {
        return std::string(CT2A(theNode->GetAttrValue(CA2T(theAttributeName.c_str()))));
    }

    const std::string ChildNodeValue(LPXNode theNode, const std::string &theChildNodeName, bool fDoReferenceConversion)
    {
        return ::GetChildNodeText(theNode, theChildNodeName, fDoReferenceConversion);
    }

    const std::string MakeNodeStr(const std::string &theNodeName, bool fStartNode)
    {
        std::string xmlStr;
        xmlStr += "<";
        if(fStartNode == false)
            xmlStr += "/";
        xmlStr += theNodeName;
        xmlStr += ">";
        return xmlStr;
    }

}