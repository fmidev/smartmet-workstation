#pragma once
#include "source/rootData/RootData.h"

#include "NFmiArea.h"
namespace SmartMetDataUtilities
{
    class CellsToCopy
    {
        const NFmiArea& mapArea_;
        const RootData& rootData_;
        int firstColumnToCopy_;
        int lastColumnToCopy_;
        int firstRowToCopy_;
        int lastRowToCopy_;
    public:
        CellsToCopy(const NFmiArea& mapArea, const RootData& rootData)
            : mapArea_(mapArea)
            , rootData_(rootData)
        {
        }
        ~CellsToCopy() = default;

        void calculate()
        {
            calculateRowsAndColumsToCopy();
            growRowsAndColumnsByOne();
            fixRowsAndColumnsSoThatTheyAreInsideRootMatrix();
        }

        int getFirstColumn() const
        {
            return firstColumnToCopy_;
        }

        int getLastColumn() const
        {
            return lastColumnToCopy_;
        }

        int getFirstRow() const
        {
            return firstRowToCopy_;
        }

        int getLastRow() const
        {
            return lastRowToCopy_;
        }

        unsigned int width() const
        {
            return lastColumnToCopy_ - firstColumnToCopy_ + 1;
        }

        unsigned int height() const
        {
            return lastRowToCopy_ - firstRowToCopy_ + 1;
        }

    private:
        void calculateRowsAndColumsToCopy()
        {
            auto& rootGrid = rootData_.getGrid();
            auto gridBottomLeft = rootGrid->LatLonToGrid(mapArea_.BottomLeftLatLon());
            auto gridTopRight = rootGrid->LatLonToGrid(mapArea_.TopRightLatLon());
            lastColumnToCopy_ = static_cast<int>(ceil(gridTopRight.X()));
            firstColumnToCopy_ = static_cast<int>(floor(gridBottomLeft.X()));
            firstRowToCopy_ = static_cast<int>(floor(gridBottomLeft.Y()));
            lastRowToCopy_ = static_cast<int>(ceil(gridTopRight.Y()));
        }
        void growRowsAndColumnsByOne()
        {
            lastColumnToCopy_ = lastColumnToCopy_ + 1;
            firstColumnToCopy_ = firstColumnToCopy_ - 1;
            firstRowToCopy_ = firstRowToCopy_ - 1;
            lastRowToCopy_ = lastRowToCopy_ + 1;
        }

        void fixRowsAndColumnsSoThatTheyAreInsideRootMatrix()
        {
            auto lastColumn = rootData_.getMatrix()->NX() - 1;
            auto lastRow = rootData_.getMatrix()->NY() - 1;
            firstColumnToCopy_ = (firstColumnToCopy_ < 0) ? 0 : firstColumnToCopy_;
            lastColumnToCopy_ = (int)((lastColumnToCopy_ > lastColumn) ? lastColumn : lastColumnToCopy_);
            firstRowToCopy_ = (firstRowToCopy_ < 0) ? 0 : firstRowToCopy_;
            lastRowToCopy_ = (int)((lastRowToCopy_ > lastRow) ? lastRow : lastRowToCopy_);
        }

    };
}
