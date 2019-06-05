#pragma once

#include "xmlliteutils/XMLite.h"
#include <string>

namespace XmlHelper 
{
    LPXNode GetChildNode(LPXNode theRootNode, int childIndex);
    std::string GetChildNodeText(LPXNode theRootNode, const std::string &theChildName);
    std::string GetChildsChildNodeText(LPXNode theRootNode, const std::string &theChildName, const std::string &theChildsChildName);
    std::string AttributeValue(LPXNode theNode, const std::string &theAttributeName);
    std::string ChildNodeValue(LPXNode theNode, const std::string &theChildNodeName);
    std::string ChildsChildNodeValue(LPXNode theNode, const std::string &theChildNodeName, const std::string &theChildsChildNodeName);
}