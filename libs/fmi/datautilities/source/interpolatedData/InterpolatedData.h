#pragma once
#include "NFmiGrid.h"
#include "CoordinateMatrix.h"

#include <memory>

namespace SmartMetDataUtilities
{
    class InterpolatedData
    {
        std::shared_ptr<NFmiArea> area_;
        std::shared_ptr<Fmi::CoordinateMatrix> matrix_;
    public:
        InterpolatedData(std::shared_ptr<Fmi::CoordinateMatrix> matrix, std::shared_ptr<NFmiArea> area)
            :area_(area)
            , matrix_(matrix) {}

        InterpolatedData() = default;
        ~InterpolatedData() = default;

        std::shared_ptr<NFmiArea> getArea() const
        {
            return area_;
        }

        std::shared_ptr<Fmi::CoordinateMatrix> getMatrix() const
        {
            return matrix_;
        }

        static bool matrixIsNotEmpty(const Fmi::CoordinateMatrix& matrix)
        {
            return matrix.width() > 0
                && matrix.height() > 0;
        }
    };
}
