#include "source/rootData/RootDataCropper.h"

#include "NFmiDataMatrix.h"
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

    unique_ptr<NFmiDataMatrix<NFmiPoint>> RootDataCropper::getCroppedRootMatrix()
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

    unique_ptr<NFmiDataMatrix<NFmiPoint>> RootDataCropper::cropRootMatrix() const
    {
        auto rootMatrix = rootData_.getMatrix();
        auto matrixToPopulate = make_unique<NFmiDataMatrix<NFmiPoint>>(cellsToCopy_.width(), cellsToCopy_.height());

        for(size_t j = cellsToCopy_.getFirstRow(), j1 = 0; j <= cellsToCopy_.getLastRow(); ++j, ++j1)
        {
            for(size_t i = cellsToCopy_.getFirstColumn(), i1 = 0; i <= cellsToCopy_.getLastColumn(); ++i, ++i1)
            {
                (*matrixToPopulate)[i1][j1] = (*rootMatrix)[i][j];
            }
        }
        return matrixToPopulate;
    }

    unique_ptr<NFmiArea> RootDataCropper::cropRootArea() const
    {
        auto lastRow = croppedRootData_->NY() - 1;
        auto lastColumn = croppedRootData_->NX() - 1;
        auto bottomLeftLatLon = (*croppedRootData_)[0][0];
        auto topRightLatLon = (*croppedRootData_)[lastColumn][lastRow];
        return  unique_ptr<NFmiArea>(mapArea_.CreateNewArea(bottomLeftLatLon, topRightLatLon));
    }
}
