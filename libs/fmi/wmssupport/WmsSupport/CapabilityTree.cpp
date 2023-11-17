#include "wmssupport/CapabilityTree.h"

#include "NFmiParameterName.h"

#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
    namespace
    {
        std::unique_ptr<CapabilityNode> createNewNode(const NFmiProducer& producer, const std::string& name)
        {
            auto subTree = std::make_unique<CapabilityNode>();
            subTree->value = Capability{ producer, kFmiLastParameter, name };
            return std::move(subTree);
        }

        CapabilityNode& findNodeWithName(std::vector<std::unique_ptr<CapabilityTree>>& children, const std::string& name)
        {
            for(const auto& child : children)
            {
                if(child->value.name == name)
                {
                    return dynamic_cast<CapabilityNode&>(*child);
                }
            }
            throw std::runtime_error("Could not find node with given name.");
        }
    }

    void insertLeaf(CapabilityNode& node, const CapabilityLeaf& leaf, std::list<std::string>& path)
    {
        if(path.empty())
        {
            node.children.push_back(std::make_unique<CapabilityLeaf>(leaf));
            return;
        }
        try
        {
            auto& subTree = findNodeWithName(node.children, path.front());
            path.pop_front();
            insertLeaf(subTree, leaf, path);
        }
        catch(const std::exception&)
        {
            auto subTree = createNewNode(leaf.value.producer, path.front());
            path.pop_front();
            insertLeaf(*subTree, leaf, path);
            node.children.push_back(std::move(subTree));
        }
    }

}