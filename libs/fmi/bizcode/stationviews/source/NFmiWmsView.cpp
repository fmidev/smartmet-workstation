#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255 merkkiä joka johtuu 'puretuista' STL-template nimistä)
#endif

#include "NFmiWmsView.h"
#ifndef DISABLE_CPPRESTSDK

#include "boost/shared_ptr.hpp"
#include "NFmiArea.h"
#include "NFmiDrawParam.h"
#include "NFmiText.h"
#include "NFmiToolBox.h"
#include "CtrlViewDocumentInterface.h"
#include "WmsSupport.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewFunctions.h"
#include "catlog/catlog.h"
#include "WmsSupport/ChangedLayers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
using namespace Gdiplus;

NFmiWmsView::NFmiWmsView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox *theToolBox
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamId
    , int theRowIndex
    , int theColumnIndex)
: NFmiStationView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawParam
        , theParamId
        , NFmiPoint(0, 0)
        , NFmiPoint(1, 1)
        , theRowIndex
        , theColumnIndex)
    , itsScreenPixelSizeInMM(0)
{
}

void NFmiWmsView::Draw(NFmiToolBox *theGTB)
{
    if(!theGTB)
        return;
    itsToolBox = theGTB;

    auto dataIdent = itsDrawParam->Param();
    decltype(auto) wmsSupportPtr = itsCtrlViewDocumentInterface->GetWmsSupport();

    if(!IsParamDrawn())
    {
        wmsSupportPtr->unregisterDynamicLayer(CalcRealRowIndex(), itsViewGridColumnNumber, itsMapViewDescTopIndex, dataIdent);
        return;
    }

    try
    {
        InitializeGdiplus(itsToolBox, &GetFrame());
        auto bitmapSize = itsCtrlViewDocumentInterface->ActualMapBitmapSizeInPixels(itsMapViewDescTopIndex);

        auto editorTimeStepInMinutes = static_cast<int>(::round(itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex) * 60));

        auto holder = wmsSupportPtr->getDynamicImage(dataIdent, *itsArea, itsTime, int(bitmapSize.X()), int(bitmapSize.Y()), editorTimeStepInMinutes);

        if(holder)
        {
            NFmiPoint startPoint(CtrlViewUtils::ConvertPointFromRect1ToRect2(itsRect.TopLeft(), NFmiRect(0, 0, 1, 1), NFmiRect(NFmiPoint(0, 0), itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex))));
            Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(startPoint.X()), static_cast<Gdiplus::REAL>(startPoint.Y()), static_cast<Gdiplus::REAL>(bitmapSize.X()), static_cast<Gdiplus::REAL>(bitmapSize.Y()));
            NFmiRect sourceRect(0, 0, holder->mImage->GetWidth(), holder->mImage->GetHeight());
            Gdiplus::REAL alpha = itsDrawParam->Alpha() / 100.f; // 0 on täysin läpinäkyvä, 0.5 = semi transparent ja 1.0 = opaque
            bool doNearestInterpolation = alpha >= 1.f ? true : false;
            CtrlView::DrawBitmapToDC_4(itsToolBox->GetDC(), *holder->mImage, sourceRect, destRect, doNearestInterpolation, NFmiImageAttributes(alpha), itsGdiPlusGraphics);

            wmsSupportPtr->registerDynamicLayer(CalcRealRowIndex(), itsViewGridColumnNumber, itsMapViewDescTopIndex, dataIdent);
        }
        else
        {
            wmsSupportPtr->unregisterDynamicLayer(CalcRealRowIndex(), itsViewGridColumnNumber, itsMapViewDescTopIndex, dataIdent);
        }
    }
    catch(std::exception &e)
    {
        CatLog::logMessage(e.what(), CatLog::Severity::Error, CatLog::Category::NetRequest, true);
    }
    this->CleanGdiplus();
}

std::string NFmiWmsView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
    try
    {
        std::string parameterStr;
        auto wmsSupportPtr = itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface().getWmsSupport();
        if(wmsSupportPtr)
        {
            const auto& dataIdent = itsDrawParam->Param();
            parameterStr = itsCtrlViewDocumentInterface->GetWmsSupport()->getFullLayerName(dataIdent);
            auto fontColor = CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData());
            parameterStr = AddColorTagsToString(parameterStr, fontColor, true);
            auto timeDimensionStr = wmsSupportPtr->makeWmsLayerTimeDimensionTooltipString(dataIdent, true);
            if(!timeDimensionStr.empty())
            {
                parameterStr += timeDimensionStr;
            }
        }
        return parameterStr;
    }
    catch(std::exception& e)
    {
        return std::string("Tooltip error:\n") + e.what();
    }
}

#endif // DISABLE_CPPRESTSDK
