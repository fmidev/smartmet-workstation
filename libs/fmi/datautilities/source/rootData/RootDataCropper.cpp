#include "source/rootData/RootDataCropper.h"

#include "CoordinateMatrix.h"
#include "NFmiPoint.h"
#include "NFmiArea.h"

using namespace std;

namespace SmartMetDataUtilities
{
    RootDataCropper::RootDataCropper(const NFmiArea& mapArea, const RootData& rootData)
        : mapArea_(mapArea)
        , rootData_(rootData)
        , cellsToCopy_(mapArea, rootData)
    {
    }

    unique_ptr<Fmi::CoordinateMatrix> RootDataCropper::getCroppedRootMatrix()
    {
        if(!croppedRootData_)
        {
            calculate();
        }
        return move(croppedRootData_);
    }

    unique_ptr<NFmiArea> RootDataCropper::getCroppedRootArea()
    {
        if(!croppedRootArea_)
        {
            calculate();
        }
        return move(croppedRootArea_);
    }

    void RootDataCropper::calculate()
    {
        cellsToCopy_.calculate();
        croppedRootData_ = cropRootMatrix();
        croppedRootArea_ = cropRootArea();
    }

    unique_ptr<Fmi::CoordinateMatrix> RootDataCropper::cropRootMatrix() const
    {
        auto rootMatrix = rootData_.getMatrix();
        auto matrixToPopulate = make_unique<Fmi::CoordinateMatrix>(cellsToCopy_.width(), cellsToCopy_.height());

        for(size_t j = cellsToCopy_.getFirstRow(), j1 = 0; j <= cellsToCopy_.getLastRow(); ++j, ++j1)
        {
            for(size_t i = cellsToCopy_.getFirstColumn(), i1 = 0; i <= cellsToCopy_.getLastColumn(); ++i, ++i1)
            {
                matrixToPopulate->set(i1, j1, (*rootMatrix)(i1, j1));
            }
        }
        return matrixToPopulate;
    }

    unique_ptr<NFmiArea> RootDataCropper::cropRootArea() const
    {
        auto lastRow = croppedRootData_->height() - 1;
        auto lastColumn = croppedRootData_->width() - 1;
        auto bottomLeftLatLon = (*croppedRootData_)(0, 0);
        auto topRightLatLon = (*croppedRootData_)(lastColumn, lastRow);
        return  unique_ptr<NFmiArea>(mapArea_.CreateNewArea(bottomLeftLatLon, topRightLatLon));
    }
}
