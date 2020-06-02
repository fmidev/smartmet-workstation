#pragma once

#include "source/newbaseUtil/AreaUtil.h"
#include "source/interpolation/Interpolation.h"
#include "source/interpolation/InterpolationOptimalDimensions.h"
#include "source/cache/RootDataCache.h"
#include "source/error/Error.h"
#include "source/cache/CacheUtil.h"
#include "CoordinateMatrix.h"
#include "NFmiGrid.h"
#include "NFmiArea.h"
#include "NFmiPoint.h"

#include <memory>

namespace SmartMetDataUtilities
{
    namespace
    {
        void populateMatrixFromGrid(std::shared_ptr<Fmi::CoordinateMatrix> matrixToPopulate, std::shared_ptr<NFmiGrid> interpolatedGrid)
        {
            interpolatedGrid->First();
            for(size_t j = 0; j != interpolatedGrid->YNumber(); ++j)
            {
                for(size_t i = 0; i != interpolatedGrid->XNumber(); ++i)
                {
                    matrixToPopulate->set(i, j, interpolatedGrid->LatLon());
                    interpolatedGrid->Next();
                }
            }
        }
    }

    class NewInterpolation : public Interpolation
    {
        NFmiGrid& grid_;
        const NFmiArea& mapArea_;
        RootDataCache& rootCache_;
        MapViewId keys_;
        std::shared_ptr<NFmiArea> croppedArea_ = nullptr;
        std::shared_ptr<Fmi::CoordinateMatrix> interpolatedMatrix_ = nullptr;
    public:
        NewInterpolation(NFmiGrid& grid, const NFmiArea& mapArea, RootDataCache& rootCache, const MapViewId &keys)
            :grid_(grid)
            , mapArea_(mapArea)
            , rootCache_(rootCache)
            , keys_(keys)
        {
        }

        virtual void calculate() override
        {
            croppedArea_ = AreaUtil::calculateCroppedArea(grid_, mapArea_);
            interpolatedMatrix_ = calculateInterpolatedMatrix();
        }

        virtual std::shared_ptr<NFmiArea> getArea() override
        {
            return croppedArea_;
        }
        virtual std::shared_ptr<Fmi::CoordinateMatrix> getMatrix() override
        {
            return interpolatedMatrix_;
        }
    private:
        std::shared_ptr<Fmi::CoordinateMatrix> calculateInterpolatedMatrix()
        {
            InterpolationOptimalDimensions optimalDimensions(grid_, *croppedArea_);
            auto optimalWidth = optimalDimensions.getWidth();
            auto optimalHeight = optimalDimensions.getHeight();
            auto matrixToPopulate = std::make_shared<Fmi::CoordinateMatrix>(optimalWidth, optimalHeight);
            auto interpolatedGrid = std::make_shared<NFmiGrid>(croppedArea_.get(), optimalWidth, optimalHeight);
            populateMatrixFromGrid(matrixToPopulate, interpolatedGrid);
            replaceOldRootDataIfNewDataSpansLargerArea(matrixToPopulate, interpolatedGrid);
            return matrixToPopulate;
        }

        void replaceOldRootDataIfNewDataSpansLargerArea(std::shared_ptr<Fmi::CoordinateMatrix> matrix, std::shared_ptr<NFmiGrid> grid)
        {
            auto keys = CacheUtil::createKeys<RootDataCache>(keys_, *croppedArea_, grid_);
            auto originalMapArea = std::shared_ptr<NFmiArea>(AreaUtil::createCopyOf(mapArea_));
            try
            {
                decltype(auto) rootData = rootCache_.get(keys);
                if(AreaUtil::areaInSquareMetersFor(*croppedArea_) >= AreaUtil::areaInSquareMetersFor(*rootData.getGrid()->Area()))
                {
                    rootCache_.put(keys, RootData(matrix, grid, originalMapArea));
                }
            }
            catch(const DataNotFoundException &)
            {
                rootCache_.put(keys, RootData(matrix, grid, originalMapArea));
            }
        }
    };
}
