#pragma once
#include "NFmiGrid.h"
#include "NFmiDataMatrix.h"

#include <memory>

namespace SmartMetDataUtilities
{
    class RootData
    {
        std::shared_ptr<NFmiGrid> grid_;
        std::shared_ptr<NFmiDataMatrix<NFmiPoint>> matrix_;
        std::shared_ptr<NFmiArea> originalMapArea_;
    public:
        RootData(std::shared_ptr<NFmiDataMatrix<NFmiPoint>> matrix, std::shared_ptr<NFmiGrid> grid, std::shared_ptr<NFmiArea> originalMapArea)
            :grid_(grid)
            , matrix_(matrix)
        ,originalMapArea_(originalMapArea) {}

        RootData() = default;
        ~RootData() = default;

        std::shared_ptr<NFmiGrid> getGrid() const
        {
            return grid_;
        }

        std::shared_ptr<NFmiDataMatrix<NFmiPoint>> getMatrix() const
        {
            return matrix_;
        }

        std::shared_ptr<NFmiArea> getOriginalMapArea() const
        {
            return originalMapArea_;
        }
    };
}
