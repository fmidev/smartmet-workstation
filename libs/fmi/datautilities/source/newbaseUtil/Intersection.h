#pragma once
#include "source/common/CommonAliases.h"
#include "source/common/Borders.h"

#include "NFmiArea.h"
#include "NFmiGrid.h"

#include <functional>

namespace SmartMetDataUtilities
{
    class Intersection
    {
        Borders intersectionBorders_;
        const NFmiArea &area_;
        NFmiGrid &grid_;
        bool limitsInUse_;
        const StepFunction right_;
        const StepFunction left_;
        const StepFunction up_;
        const StepFunction down_;
    public:
        Intersection(const NFmiArea &area, NFmiGrid &grid);
        Borders getBorders();

        static bool gridAndMapAreasIntersect(NFmiGrid& grid, const NFmiArea& mapArea);
    private:
        void calculate();
        void loopGridWithLimits();
        void loopGrid();
        void loopGridWithGivenStepper(const StepperWithPossibilityToDoChecking& predicate);
    };
}