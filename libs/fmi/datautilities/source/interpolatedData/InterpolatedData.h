#pragma once
#include "NFmiGrid.h"
#include "NFmiDataMatrix.h"

#include <memory>

namespace SmartMetDataUtilities
{
    class InterpolatedData
    {
        std::shared_ptr<NFmiArea> area_;
        std::shared_ptr<NFmiDataMatrix<NFmiPoint>> matrix_;
    public:
        InterpolatedData(std::shared_ptr<NFmiDataMatrix<NFmiPoint>> matrix, std::shared_ptr<NFmiArea> area)
            :area_(area)
            , matrix_(matrix) {}

        InterpolatedData() = default;
        ~InterpolatedData() = default;

        std::shared_ptr<NFmiArea> getArea() const
        {
            return area_;
        }

        std::shared_ptr<NFmiDataMatrix<NFmiPoint>> getMatrix() const
        {
            return matrix_;
        }

        static bool matrixIsNotEmpty(const NFmiDataMatrix<NFmiPoint>& matrix)
        {
            return matrix.NX() > 0
                && matrix.NY() > 0;
        }
    };
}
