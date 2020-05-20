#pragma once
#include "NFmiGrid.h"
#include "CoordinateMatrix.h"

#include <memory>

namespace SmartMetDataUtilities
{
    class RootData
    {
        std::shared_ptr<NFmiGrid> grid_;
        std::shared_ptr<Fmi::CoordinateMatrix> matrix_;
        std::shared_ptr<NFmiArea> originalMapArea_;
    public:
        RootData(std::shared_ptr<Fmi::CoordinateMatrix> matrix, std::shared_ptr<NFmiGrid> grid, std::shared_ptr<NFmiArea> originalMapArea)
            :grid_(grid)
            , matrix_(matrix)
        ,originalMapArea_(originalMapArea) {}

        RootData() = default;
        ~RootData() = default;

        std::shared_ptr<NFmiGrid> getGrid() const
        {
            return grid_;
        }

        std::shared_ptr<Fmi::CoordinateMatrix> getMatrix() const
        {
            return matrix_;
        }

        std::shared_ptr<NFmiArea> getOriginalMapArea() const
        {
            return originalMapArea_;
        }
    };
}
