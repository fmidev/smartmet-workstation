#pragma once

// Compatibility helper: NFmiDataMatrix<float>::InterpolatedValue was removed
// from the system smartmet-library-newbase. This free function provides the
// same bilinear / nearest-point / wind-direction interpolation over a matrix.

#include "NFmiDataMatrix.h"
#include "NFmiGlobals.h"
#include "NFmiInterpolation.h"
#include "NFmiPoint.h"
#include "NFmiRect.h"

#include <cmath>

namespace DataMatrixInterpolation
{

inline double FixIndexOnEdges(double index, std::size_t size)
{
    if(index < 0 && index > -0.5)
        return 0;
    if(index >= static_cast<double>(size) - 1.0 && index < static_cast<double>(size) - 0.5)
        return static_cast<double>(size) - 1.0001;
    return index;
}

// Full form with explicit coordinate rectangle.
inline float InterpolatedValue(const NFmiDataMatrix<float>& matrix,
                               const NFmiPoint& thePoint,
                               const NFmiRect& theRelativeCoords,
                               FmiParameterName theParamId,
                               bool fDontInvertY = false,
                               FmiInterpolationMethod interp = kLinearly)
{
    float value = kFloatMissing;
    auto NX = matrix.NX();
    auto NY = matrix.NY();
    if(NX == 0 || NY == 0)
        return value;

    double xInd =
        ((NX - 1) * (thePoint.X() - theRelativeCoords.Left())) / theRelativeCoords.Width();
    double yInd =
        fDontInvertY
            ? ((NY - 1) * (thePoint.Y() - theRelativeCoords.Top())) / theRelativeCoords.Height()
            : ((NY - 1) *
               (theRelativeCoords.Height() - (thePoint.Y() - theRelativeCoords.Top()))) /
                  theRelativeCoords.Height();
    xInd = FixIndexOnEdges(xInd, NX);
    yInd = FixIndexOnEdges(yInd, NY);

    int x1 = static_cast<int>(std::floor(xInd));
    int y1 = static_cast<int>(std::floor(yInd));
    int x2 = x1 + 1;
    int y2 = y1 + 1;
    if(x1 >= 0 && x2 < static_cast<int>(NX) && y1 >= 0 && y2 < static_cast<int>(NY))
    {
        double xFraction = xInd - x1;
        double yFraction = yInd - y1;
        if(interp == kNearestPoint)
            return matrix.At(static_cast<int>(std::round(xInd)),
                             static_cast<int>(std::round(yInd)),
                             kFloatMissing);
        else
        {
            if(theParamId == kFmiWindDirection || theParamId == kFmiWaveDirection)
                value = static_cast<float>(NFmiInterpolation::ModBiLinear(
                    xFraction, yFraction,
                    matrix.At(x1, y2, kFloatMissing), matrix.At(x2, y2, kFloatMissing),
                    matrix.At(x1, y1, kFloatMissing), matrix.At(x2, y1, kFloatMissing), 360));
            else if(theParamId == kFmiWindVectorMS)
                value = static_cast<float>(NFmiInterpolation::WindVector(
                    xFraction, yFraction,
                    matrix.At(x1, y2, kFloatMissing), matrix.At(x2, y2, kFloatMissing),
                    matrix.At(x1, y1, kFloatMissing), matrix.At(x2, y1, kFloatMissing)));
            else
                value = static_cast<float>(NFmiInterpolation::BiLinear(
                    xFraction, yFraction,
                    matrix.At(x1, y2, kFloatMissing), matrix.At(x2, y2, kFloatMissing),
                    matrix.At(x1, y1, kFloatMissing), matrix.At(x2, y1, kFloatMissing)));
        }
    }
    return value;
}

// Short form: assumes the default [0,0]-[1,1] coordinate rectangle.
inline float InterpolatedValue(const NFmiDataMatrix<float>& matrix,
                               const NFmiPoint& thePoint,
                               FmiParameterName theParamId,
                               bool fDontInvertY = false,
                               FmiInterpolationMethod interp = kLinearly)
{
    static const NFmiRect defaultCoords(0, 0, 1, 1);
    return InterpolatedValue(matrix, thePoint, defaultCoords, theParamId, fDontInvertY, interp);
}

} // namespace DataMatrixInterpolation
