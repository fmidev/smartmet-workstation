#pragma once

#include "wmssupport/LegendBuffer.h"

#include <vector>

namespace Wms
{
    class LegendSelf
    {
        class Row
        {
            unsigned int remaining_;
            unsigned int height_ = 0;
        public:
            Row(unsigned int maxSelfLength);

            std::vector<LegendBuffer> legends;

            void addLegend(LegendBuffer legend);

            unsigned int getRemainingRoom() const;
            unsigned int getHeight() const;
            void updateHeight();
        };

        unsigned int maxSelfLength;
        unsigned int maxSelfHeight;
        unsigned int totalHeight = 0;
        Orientation orientation;
    public:
        std::vector<Row> selves;
        double horizontalShift = 0;
        double verticalShift = 0;

        LegendSelf(unsigned int maxSelfLength_, unsigned int maxSelfHeight_, Orientation orientation_);

        void insert(std::vector<LegendBuffer>& legends);
        bool shouldCompress();
        void compress();

    private:
        void updateTotalHeight();
        Row* lookForBestSelfToPutLegend(const LegendBuffer& legend);
        void addEmptySelf();
        void scaleLegendDownIfTooBigForEmptySelf(LegendBuffer& legend);
        void clear();
        std::vector<LegendBuffer> getLegends();
        void compress(double factor);
    };
}
