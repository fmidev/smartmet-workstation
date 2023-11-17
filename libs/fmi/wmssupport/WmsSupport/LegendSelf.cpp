#include "wmssupport/LegendSelf.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
    LegendSelf::Row::Row(unsigned int maxSelfLength)
        : remaining_{ maxSelfLength }
    {
    }


    void LegendSelf::Row::addLegend(const LegendBuffer &legend)
    {
        auto legendH = legend.legendHeightRelativeToOrientation();
        auto legendW = legend.legendWidthRelativeToOrientation();
        remaining_ = (std::max)((int)remaining_ - (int)legendW, 0);
        height_ = (std::max)(height_, legendH);
        legends.push_back(legend);
    }

    unsigned int LegendSelf::Row::getRemainingRoom() const
    {
        return remaining_;
    }

    unsigned int LegendSelf::Row::getHeight() const
    {
        return height_;
    }

    void LegendSelf::Row::updateHeight()
    {
        decltype(auto) maxLegend = std::max_element(legends.cbegin(), legends.cend(), [](const auto& el1, const auto& el2)
            {
                return el1.legendHeightRelativeToOrientation() < el2.legendHeightRelativeToOrientation();
            });
        if(maxLegend != legends.cend())
            height_ = maxLegend->height;
        else
            height_ = 0;
    }


    LegendSelf::LegendSelf(unsigned int maxSelfLength_, unsigned int maxSelfHeight_, Orientation orientation_)
        : maxSelfLength{ maxSelfLength_ }
        , maxSelfHeight{ maxSelfHeight_ }
        , orientation{ orientation_ }
    {
        addEmptySelf();
    }

    void LegendSelf::insert(std::vector<LegendBuffer>& legends)
    {
        for(auto& legend : legends)
        {
            legend.orientation = orientation;
            scaleLegendDownIfTooBigForEmptySelf(legend);

            Row* bestSelf = lookForBestSelfToPutLegend(legend);
            if(!bestSelf)
            {
                addEmptySelf();
                bestSelf = &selves.back();
            }

            bestSelf->addLegend(legend);
        }
        updateTotalHeight();
    }

    bool LegendSelf::shouldCompress()
    {
        return totalHeight > maxSelfHeight;
    }

    void LegendSelf::compress()
    {
        while(totalHeight > maxSelfHeight)
        {
            compress(0.9);
            auto legends = getLegends();
            clear();
            insert(legends);
        }
    }

    void LegendSelf::updateTotalHeight()
    {
        for(const auto& self : selves)
        {
            totalHeight += self.getHeight();
        }
    }

    LegendSelf::Row* LegendSelf::lookForBestSelfToPutLegend(const LegendBuffer& legend)
    {
        auto legendW = legend.legendWidthRelativeToOrientation();
        Row* bestSelf = nullptr;
        auto remainingAfterLegend = (std::numeric_limits<unsigned int>::max)();
        for(auto& self : selves)
        {
            if(self.getRemainingRoom() > legendW)
            {
                if(self.getRemainingRoom() - legendW < remainingAfterLegend)
                {
                    bestSelf = &self;
                    remainingAfterLegend = self.getRemainingRoom() - legendW;
                }
            }
        }
        return bestSelf;
    }

    void LegendSelf::addEmptySelf()
    {
        auto self = Row{ maxSelfLength };
        selves.push_back(self);
    }

    void LegendSelf::scaleLegendDownIfTooBigForEmptySelf(LegendBuffer& legend)
    {
        auto legendW = legend.legendWidthRelativeToOrientation();
        if(legendW > maxSelfLength)
        {
            auto factor = ((double)maxSelfLength) / (double)legendW;
            legend.height = static_cast<int>(std::floor(factor * legend.height));
            legend.width = static_cast<int>(std::floor(factor * legend.width));
        }
    }

    void LegendSelf::clear()
    {
        selves.clear();
        totalHeight = 0;
        addEmptySelf();
    }

    std::vector<LegendBuffer> LegendSelf::getLegends()
    {
        auto legends = std::vector<LegendBuffer>{};

        for(auto& self : selves)
        {
            std::copy(self.legends.begin(), self.legends.end(), std::back_inserter(legends));
        }
        return legends;
    }

    void LegendSelf::compress(double factor)
    {
        if(totalHeight <= maxSelfHeight)
        {
            return;
        }
        for(auto& self : selves)
        {
            auto heightLimit = static_cast<unsigned int>(std::floor(self.getHeight() * factor));
            for(auto& legend : self.legends)
            {
                if(legend.legendHeightRelativeToOrientation() > heightLimit)
                {
                    legend.height = static_cast<unsigned int>(std::floor(factor * legend.height));
                    legend.width = static_cast<unsigned int>(std::floor(factor * legend.width));
                }
            }
            self.updateHeight();
        }
    }
}