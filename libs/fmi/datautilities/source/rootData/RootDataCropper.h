#pragma once
#include "source/rootData/CellsToCopy.h"

namespace SmartMetDataUtilities
{
    class RootDataCropper
    {
        const NFmiArea& mapArea_;
        const RootData& rootData_;
        CellsToCopy cellsToCopy_;
        std::unique_ptr<NFmiArea> croppedRootArea_;
        std::unique_ptr<NFmiDataMatrix<NFmiPoint>> croppedRootData_;
    public:
        RootDataCropper(const NFmiArea& area, const RootData& rootData);

        std::unique_ptr<NFmiDataMatrix<NFmiPoint>> getCroppedRootMatrix();
        std::unique_ptr<NFmiArea> getCroppedRootArea();
    private:
        void calculate();
        std::unique_ptr<NFmiDataMatrix<NFmiPoint>> cropRootMatrix() const;
        std::unique_ptr<NFmiArea> cropRootArea() const;
    };
}
