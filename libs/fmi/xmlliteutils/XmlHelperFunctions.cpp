#include "XmlHelperFunctions.h"

namespace XmlHelper
{

    LPXNode GetChildNode(LPXNode theRootNode, int childIndex)
    {
        if(theRootNode && theRootNode->GetChildCount() >= childIndex)
            return theRootNode->GetChild(childIndex);
        else
            return nullptr;
    }

    std::string GetChildNodeText(LPXNode theRootNode, const std::string &theChildName)
    {
        LPXNode childNode = theRootNode->Find(CA2T(theChildName.c_str()));
        if(childNode)
        {
            std::string tmp = CT2A(childNode->GetText());
            return tmp;
        }

        return "";
    }

    std::string GetChildsChildNodeText(LPXNode theRootNode, const std::string &theChildName, const std::string &theChildsChildName)
    {
        LPXNode childNode = theRootNode->Find(CA2T(theChildName.c_str()));
        if(childNode)
        {
            LPXNode childsChildNode = childNode->Find(CA2T(theChildsChildName.c_str()));
            if(childsChildNode)
            {
                std::string tmp = CT2A(childsChildNode->GetText());
                return tmp;
            }
        }

        return "";
    }

    std::string AttributeValue(LPXNode theNode, const std::string &theAttributeName)
    {
        LPCTSTR attribStrPtr = theNode->GetAttrValue(CA2T(theAttributeName.c_str()));
        if(attribStrPtr)
            return std::string(CT2A(attribStrPtr));
        else
            return "";
    }

    std::string ChildNodeValue(LPXNode theNode, const std::string &theChildNodeName)
    {
        return GetChildNodeText(theNode, theChildNodeName);
    }

    std::string ChildsChildNodeValue(LPXNode theNode, const std::string &theChildNodeName, const std::string &theChildsChildNodeName)
    {
        return GetChildsChildNodeText(theNode, theChildNodeName, theChildsChildNodeName);
    }
}
