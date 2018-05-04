#ifndef TREE_H
#define TREE_H

#include <vector>
#include <memory>

namespace cppext
{
    template<typename ValueT>
    class Tree
    {
    public:
        ValueT value;
    private:
        virtual void type() const = 0;
    };

    template <typename ValueT>
    class Node : public Tree<ValueT>
    {
    public:
        Node() = default;
        Node(ValueT val, std::vector<std::unique_ptr<Tree>> childs)
        {
            Tree<ValueT>::value = std::move(val);
            children = std::move(childs);
        }

        std::vector<std::unique_ptr<Tree>> children;
    private:
        void type() const final {}
    };

    template <typename ValueT>
    class Leaf : public Tree<ValueT>
    {
    public:
        Leaf() = default;
        Leaf(ValueT val)
        {
            Tree<ValueT>::value = std::move(val);
        }
    private:
        void type() const final {}
    };


}
#endif
