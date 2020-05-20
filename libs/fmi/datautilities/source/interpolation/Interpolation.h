#pragma once

#include <memory>

class NFmiArea;
class NFmiPoint;
namespace Fmi
{
    class CoordinateMatrix;
}

namespace SmartMetDataUtilities
{
    class Interpolation
    {
    public:
        virtual void calculate() = 0;
        virtual std::shared_ptr<NFmiArea> getArea() = 0;
        virtual std::shared_ptr<Fmi::CoordinateMatrix> getMatrix() = 0;
    };
}