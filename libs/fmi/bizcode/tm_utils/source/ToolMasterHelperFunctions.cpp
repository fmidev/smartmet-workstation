#include "stdafx.h"
#include "ToolMasterHelperFunctions.h"
#include "NFmiGriddingHelperInterface.h"
#include "agX/agx.h"

namespace
{
    // k‰ytet‰‰n avuksi least-squeare-metodin yhteydess‰
    struct MultiplyFunctor
    {
        MultiplyFunctor(double theFactor) :itsFactor(theFactor) {}
        double operator()(double theValue)
        {
            return theValue*itsFactor;
        }
        double itsFactor;
    };
}

namespace Toolmaster
{
    void SetToolMastersDC(CDC* theDC, const CRect &theClientRect)
    {
        XuWindowSize(theClientRect.Width(), theClientRect.Height());
        XuWindowSelect(theDC->GetSafeHdc());
    }

    void DoToolMasterGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, const NFmiRect &theRelativeRect, int theGriddingFunction, float theObservationRadiusRelative, const NFmiDataMatrix<float> &gridData, std::vector<float> &toolMasterGridValuesOut)
    {
        // Toolmaster griddaus funktioiden k‰yttˆ
        XuViewWorldLimits(theRelativeRect.Left(), theRelativeRect.Right(), theRelativeRect.Top(), theRelativeRect.Bottom(), 0, 0);
        XuUndefined(kFloatMissing, 2);

        if(theObservationRadiusRelative != kFloatMissing)
            XuGriddingLocalFitRadius(theObservationRadiusRelative);
        else
            XuGriddingLocalFitRadius(1.41); // T‰m‰ on laskentaruudun (~ 0,0 - 1,1) kulmapisteiden diagonaalinen et‰isyys, mik‰ on t‰m‰n radiuksen oletusarvo (= kaikkia pisteit‰ k‰ytet‰‰n aina laskuissa)

        switch(theGriddingFunction)
        {
        case kFmiXuGriddingThinPlateSplineCalc:
            XuGriddingThinPlateSplineCalc(&xValues[0], &yValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()));
            break;

        case kFmiXuGriddingFastLocalFitCalc:
            XuGriddingFastLocalFitCalc(&xValues[0], &yValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()));
            break;
        case kFmiXuGriddingLocalFitCalc:
            XuGriddingLocalFitCalc(&xValues[0], &yValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()));
            break;
        case kFmiXuGriddingTriangulationCalc: // t‰m‰ ei oikein toimi (joskus teki rumaa mutta j‰rkev‰‰ tulosta, nyt rikki)
        {
            // triangulaatio algoritmi vaatii paria tyˆ taulukkoa
            std::vector<int> int_array(31 * arraySize + (gridData.NY() * gridData.NX()));
            std::vector<float> float_array(5 * arraySize);
            XuGriddingTriangulationCalc(&xValues[0], &yValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()), &int_array[0], &float_array[0]);
            break;
        }
        case kFmiXuGriddingLeastSquaresCalc: // en saa tekem‰‰n j‰rkev‰‰ tulosta
        {
            const double factor = 100.;
            MultiplyFunctor mul(factor);
            XuViewWorldLimits(0, factor, 0, factor, 0, 0);
            int side_length = static_cast<int>(factor / 2); // t‰m‰ menee luultavasti pieleen, pit‰‰ olla int, mutta asetin maailman 0,0 - 1,1:n kokoiseksi
            int num_y_subgrid = 2;//itsGridYSize/10; // subgridin pit‰isi kai olla jaollinen originaali gridin koosta
            int num_x_subgrid = 2;//itsGridXSize/10;
            std::vector<float> tmpXValues(xValues.begin(), xValues.end());
            std::transform(tmpXValues.begin(), tmpXValues.end(), tmpXValues.begin(), mul);
            std::vector<float> tmpYValues(yValues.begin(), yValues.end());
            std::transform(tmpYValues.begin(), tmpYValues.end(), tmpYValues.begin(), mul);
            XuGriddingLeastSquaresCalc(&tmpXValues[0], &tmpYValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()), side_length, num_y_subgrid, num_x_subgrid);
            break;
        }
        default:
            break;
        }
    }

}
