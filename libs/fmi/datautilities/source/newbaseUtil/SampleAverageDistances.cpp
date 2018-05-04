#include "source/newbaseUtil/SampleAverageDistances.h"
#include "source/common/CommonAliases.h"
#include "source/newbaseUtil/PointsUtil.h"
#include "source/newbaseUtil/GridUtil.h"

#include "NFmiPoint.h"
#include "NFmiGrid.h"

#include <vector>
#include <numeric>

using namespace std;

namespace SmartMetDataUtilities
{
    namespace
    {
        double averageDistanceToGivenNeighbours(const NFmiPoint &point, const PointPair&& neighbours, const NFmiGrid& grid)
        {
            auto distance1 = GridUtil::surfaceDistanceOfPoints(grid, point, neighbours.first);
            auto distance2 = GridUtil::surfaceDistanceOfPoints(grid, point, neighbours.second);
            return (distance1 + distance2) / 2;
        }

        void setSampleToDefaultPattern(const NFmiGrid& grid, SamplePoints& sample)
        {
            auto gridWidth = grid.XNumber();
            auto gridHeight = grid.YNumber();

            auto outerBoxLeft = gridWidth / 6;
            auto outerBoxRight = gridWidth - outerBoxLeft;
            auto outerBoxBottom = gridHeight / 6;
            auto outerBoxTop = gridHeight - outerBoxBottom;

            auto innerBoxLeft = 2 * gridWidth / 6;
            auto innerBoxRight = gridWidth - innerBoxLeft;
            auto innerBoxBottom = 2 * gridHeight / 6;
            auto innerBoxTop = gridHeight - innerBoxBottom;

            auto middleWidth = gridWidth / 2;
            auto middleHeight = gridHeight / 2;

            auto outerBoxLeftMiddle = NFmiPoint(outerBoxLeft, middleHeight);
            auto outerBoxRightMiddle = NFmiPoint(outerBoxRight, middleHeight);
            auto middleOuterBoxBottom = NFmiPoint(middleWidth, outerBoxBottom);
            auto middleOuterBoxTop = NFmiPoint(middleWidth, outerBoxTop);

            auto innerBoxBottomLeft = NFmiPoint(innerBoxLeft, innerBoxBottom);
            auto innerBoxBottomRight = NFmiPoint(innerBoxRight, innerBoxBottom);
            auto innerBoxTopLeft = NFmiPoint(innerBoxLeft, innerBoxTop);
            auto innerBoxTopRight = NFmiPoint(innerBoxRight, innerBoxTop);

            auto middle = NFmiPoint(middleWidth, middleHeight);

            sample.clear();
            sample.push_back(outerBoxLeftMiddle);
            sample.push_back(outerBoxRightMiddle);
            sample.push_back(middleOuterBoxBottom);
            sample.push_back(middleOuterBoxTop);
            sample.push_back(innerBoxBottomLeft);
            sample.push_back(innerBoxBottomRight);
            sample.push_back(innerBoxTopLeft);
            sample.push_back(innerBoxTopRight);
            sample.push_back(middle);
        }

        template<typename T>
        inline double averageOf(const std::vector<T> &vec) {
            if(vec.empty())
            {
                return 0.0;
            }
            return std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
        };
    }

    SampleAverageDistances::SampleAverageDistances(NFmiGrid& grid)
        :grid_(grid)
    {
    }

    double SampleAverageDistances::getHorizontalAverage()
    {
        if(horizontalDistances_.empty())
        {
            calculateAverageDistancesFor();
        }
        return averageOf(horizontalDistances_);
    };

    double SampleAverageDistances::getVerticalAverage()
    {
        if(verticalDistances_.empty())
        {
            calculateAverageDistancesFor();
        }
        return averageOf(verticalDistances_);
    };

    void SampleAverageDistances::calculateAverageDistancesFor()
    {
        setSampleToDefaultPattern(grid_, sample_);
        for(const auto& point : sample_)
        {
            auto averageToHorizNeighbours = averageDistanceToGivenNeighbours(point, PointsUtil::horizontalNeighbours(point), grid_);
            auto averageToVertNeighbours = averageDistanceToGivenNeighbours(point, PointsUtil::verticalNeighbours(point), grid_);
            horizontalDistances_.push_back(averageToHorizNeighbours);
            verticalDistances_.push_back(averageToVertNeighbours);
        }
    }
}