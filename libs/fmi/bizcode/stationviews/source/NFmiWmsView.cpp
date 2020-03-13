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

using namespace std;
using namespace Gdiplus;

NFmiWmsView::NFmiWmsView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox *theToolBox
    , NFmiDrawingEnvironment* theDrawingEnvi
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamId
    , int theRowIndex
    , int theColumnIndex)
: NFmiStationView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawingEnvi
        , theDrawParam
        , theParamId
        , NFmiPoint(0, 0)
        , NFmiPoint(1, 1)
        , theRowIndex
        , theColumnIndex)
    , itsRowIndex(theRowIndex)
    , itsColIndex(theColumnIndex)
    , itsScreenPixelSizeInMM(0)
{
}

void NFmiWmsView::Draw(NFmiToolBox *theGTB)
{
    if(!theGTB)
        return;
    itsToolBox = theGTB;

    auto dataIdent = itsDrawParam->Param();
    decltype(auto) wmsSupport = itsCtrlViewDocumentInterface->GetWmsSupport();

    if(!IsParamDrawn())
    {
        wmsSupport.unregisterDynamicLayer(itsRowIndex, itsColIndex, itsMapViewDescTopIndex, dataIdent);
        return;
    }

    try
    {
        InitializeGdiplus(itsToolBox, &GetFrame());
        auto bitmapSize = itsCtrlViewDocumentInterface->ActualMapBitmapSizeInPixels(itsMapViewDescTopIndex);

        auto editorTimeStepInMinutes = static_cast<int>(::round(itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex) * 60));

        itsLatestParam = dataIdent.GetParamIdent();
        itsLatestProducer = dataIdent.GetProducer()->GetIdent();
        auto holder = wmsSupport.getDynamicImage(itsLatestProducer, itsLatestParam, *itsArea, itsTime, int(bitmapSize.X()), int(bitmapSize.Y()), editorTimeStepInMinutes);

        if(holder)
        {
            NFmiPoint startPoint(CtrlViewUtils::ConvertPointFromRect1ToRect2(itsRect.TopLeft(), NFmiRect(0, 0, 1, 1), NFmiRect(NFmiPoint(0, 0), itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex))));
            Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(startPoint.X()), static_cast<Gdiplus::REAL>(startPoint.Y()), static_cast<Gdiplus::REAL>(bitmapSize.X()), static_cast<Gdiplus::REAL>(bitmapSize.Y()));
            NFmiRect sourceRect(0, 0, holder->mImage->GetWidth(), holder->mImage->GetHeight());
            Gdiplus::REAL alpha = itsDrawParam->Alpha() / 100.f; // 0 on täysin läpinäkyvä, 0.5 = semi transparent ja 1.0 = opaque
            CtrlView::DrawBitmapToDC(itsToolBox->GetDC(), *holder->mImage, sourceRect, destRect, alpha, alpha >= 1.f ? true : false);

            wmsSupport.registerDynamicLayer(itsRowIndex, itsColIndex, itsMapViewDescTopIndex, dataIdent);
        }
        else
        {
            wmsSupport.unregisterDynamicLayer(itsRowIndex, itsColIndex, itsMapViewDescTopIndex, dataIdent);
        }
    }
    catch(...)
    {
    }
    this->CleanGdiplus();
}

std::string NFmiWmsView::ComposeToolTipText(const NFmiPoint & theRelativePoint)
{
    if (itsLatestParam != -1 && itsLatestProducer != -1)
    {
        return  itsCtrlViewDocumentInterface->GetWmsSupport().getFullLayerName(itsLatestProducer, itsLatestParam);
    }
    return "";
}

#endif // DISABLE_CPPRESTSDK
