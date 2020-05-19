#pragma once
#include "source/common/Borders.h"
#include "NFmiPoint.h"
#include <algorithm>
#include "NFmiArea.h"

namespace SmartMetDataUtilities
{
    void Borders::updateBorderValuesWith(const NFmiPoint &point)
    {
        auto x = point.X();
        auto y = point.Y();
        leftBorder = std::min(leftBorder, x);
        rightBorder = std::max(rightBorder, x);
        bottomBorder = std::min(bottomBorder, y);
        topBorder = std::max(topBorder, y);
    }

    NFmiPoint Borders::topLeft() const
    {
        return NFmiPoint(leftBorder, topBorder);
    }

    NFmiPoint Borders::topRight() const
    {
        return NFmiPoint(rightBorder, topBorder);
    }

    NFmiPoint Borders::bottomLeft() const
    {
        return NFmiPoint(leftBorder, bottomBorder);
    }

    NFmiPoint Borders::bottomRight() const
    {
        return NFmiPoint(rightBorder, bottomBorder);
    }

    bool Borders::isUnmodified() const
    {
        return leftBorder == std::numeric_limits<double>::max()
            && rightBorder == -std::numeric_limits<double>::max()
            && bottomBorder == std::numeric_limits<double>::max()
            && topBorder == -std::numeric_limits<double>::max();
    }

    bool Borders::isPacific(const Borders& latLonBorders)
    {
        return NFmiArea::IsPacificView_legacy(latLonBorders.bottomLeft(), latLonBorders.topRight());
    }

    void Borders::convertToPacific()
    {
        if(leftBorder < 0)
        {
            leftBorder += 360;
        }
        if(rightBorder < 0)
        {
            rightBorder += 360;
        }
        if(leftBorder > rightBorder)
        {
            auto tmp = leftBorder;
            leftBorder = rightBorder;
            rightBorder = tmp;
        }
    }

    void Borders::convertToAtlantic()
    {
        if(leftBorder > 180)
        {
            leftBorder -= 360;
        }
        if(rightBorder > 180)
        {
            rightBorder -= 360;
        }
        if(leftBorder > rightBorder)
        {
            auto tmp = leftBorder;
            leftBorder = rightBorder;
            rightBorder = tmp;
        }
    }
}
