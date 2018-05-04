#pragma once

#include <memory>

template <typename A>
class NFmiDataMatrix;

class NFmiArea;
class NFmiPoint;

namespace SmartMetDataUtilities
{
    class Interpolation
    {
    public:
        virtual void calculate() = 0;
        virtual std::shared_ptr<NFmiArea> getArea() = 0;
        virtual std::shared_ptr<NFmiDataMatrix<NFmiPoint>> getMatrix() = 0;
    };
}