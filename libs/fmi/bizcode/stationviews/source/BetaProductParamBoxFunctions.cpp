#include "BetaProductParamBoxFunctions.h"
#include "NFmiCtrlView.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiBetaProductSystem.h"
#include "NFmiDrawParamList.h"
#include "CtrlViewFunctions.h"
#include "NFmiDictionaryFunction.h"
#include "GraphicalInfo.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiToolBox.h"
#include "CtrlViewWin32Functions.h"
#include <gdiplus.h>

namespace
{
    // laskee kertoimen, joka vaihtelee 0 ja 2:n v‰lill‰. Kerroin on 1 jos n‰yttˆruudun
    // koko on n. puolet koko n‰ytˆn korkeudesta.
    double CalcViewSizeFactor(CtrlViewDocumentInterface *ctrlViewDocumentInterface, int mapViewDescTopIndex)
    {
        const double optimalViewHeightFraction = 700. / 1440.;
        auto& graphicalInfo = ctrlViewDocumentInterface->GetGraphicalInfo(mapViewDescTopIndex);
        double comparisonHeightInPixels = graphicalInfo.itsScreenHeightInPixels * optimalViewHeightFraction;
        // pikaviritys, koska yll‰ oleva koodi ei suostunut toimimaan ollenkaan!!!!
        NFmiPoint subViewSizeInPixels(ctrlViewDocumentInterface->ActualMapBitmapSizeInPixels(mapViewDescTopIndex));
        return (0.7 * (subViewSizeInPixels.Y() - comparisonHeightInPixels) / comparisonHeightInPixels) + 1.;
    }

    FmiDirection GetBetaProductParamBoxLocation(const NFmiBetaProduct *currentBetaProduct)
    {
        return currentBetaProduct->ParamBoxLocation();
    }

    std::vector<std::string> MakeBetaProductParamBoxTexts(CtrlViewDocumentInterface *ctrlViewDocumentInterface, NFmiDrawParamList *drawParamList, bool fCrossSectionInfoWanted, bool fShowModelOriginTime)
    {
        std::vector<std::string> paramBoxTexts;
        if(drawParamList == nullptr || drawParamList->NumberOfItems() == 0)
            paramBoxTexts.push_back("---"); // Ei parametreja merkki
        else
        {
            for(drawParamList->Reset(); drawParamList->Next(); )
            {
                auto drawParamPtr = drawParamList->Current();
                if(!drawParamPtr->IsParamHidden())
                    paramBoxTexts.push_back(CtrlViewUtils::GetParamNameString(drawParamPtr, fCrossSectionInfoWanted, false, false, 10, false, false, fShowModelOriginTime, nullptr));
            }
        }

        return paramBoxTexts;
    }

    std::vector<NFmiColor> MakeBetaProductParamBoxTextColors(CtrlViewDocumentInterface *ctrlViewDocumentInterface, NFmiDrawParamList *drawParamList)
    {
        std::vector<NFmiColor> paramBoxTextColors;
        if(drawParamList == nullptr || drawParamList->NumberOfItems() == 0)
            paramBoxTextColors.push_back(NFmiColor(0, 0, 0)); // Ei parametreja v‰ri on musta
        else
        {
            for(drawParamList->Reset(); drawParamList->Next();)
            {
                auto drawParamPtr = drawParamList->Current();
                if(!drawParamPtr->IsParamHidden())
                    paramBoxTextColors.push_back(CtrlViewUtils::GetParamTextColor(drawParamPtr->DataType(), drawParamPtr->UseArchiveModelData()));
            }
        }

        return paramBoxTextColors;
    }

    std::unique_ptr<Gdiplus::Font> MakeParamBoxFont(CtrlViewDocumentInterface *ctrlViewDocumentInterface, int mapViewDescTopIndex, double sizeFactor)
    {
        std::wstring fontNameStr(L"Arial");
        double fontSizeInMM = 1.6 * ::CalcViewSizeFactor(ctrlViewDocumentInterface, mapViewDescTopIndex) * sizeFactor;
        int font1Size = static_cast<int>(fontSizeInMM * ctrlViewDocumentInterface->GetGraphicalInfo(mapViewDescTopIndex).itsPixelsPerMM_y * 1.88);
        bool boldFont = true;
        return std::make_unique<Gdiplus::Font>(fontNameStr.c_str(), static_cast<Gdiplus::REAL>(font1Size), boldFont ? Gdiplus::FontStyleBold : Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    }

    Gdiplus::RectF CalcTextBoundingAbsoluteRect(const std::string &text, const Gdiplus::Font &theUsedFont, const Gdiplus::StringFormat &theStringFormat, const Gdiplus::Graphics &theGraphic)
    {
        std::wstring wideStr = CtrlView::StringToWString(text);
        Gdiplus::RectF boundingBox;
        theGraphic.MeasureString(wideStr.c_str(), INT(wideStr.size()), &theUsedFont, Gdiplus::PointF(0, 0), &theStringFormat, &boundingBox);
        return boundingBox;
    }

    double SetMinimumParamBoxWidth(const Gdiplus::Graphics &graphics, const Gdiplus::Font &theUsedFont, const Gdiplus::StringFormat &theStringFormat, double currentWidth)
    {
        // Parametri boksilla pit‰‰ olla jokin minimi leveys ja se lasketaan t‰ss‰
        Gdiplus::RectF minBoundingBox = ::CalcTextBoundingAbsoluteRect("1234567890", theUsedFont, theStringFormat, graphics);
        double minimumParamBoxWidth = minBoundingBox.Width;
        if(currentWidth < minimumParamBoxWidth)
            currentWidth = minimumParamBoxWidth;
        return currentWidth;
    }

    double SetMinimumParamBoxHeight(const Gdiplus::Graphics &graphics, const Gdiplus::Font &theUsedFont, const Gdiplus::StringFormat &theStringFormat, double currentHeigth)
    {
        Gdiplus::RectF minBoundingBox = ::CalcTextBoundingAbsoluteRect("1234567890", theUsedFont, theStringFormat, graphics);
        double minimumParamBoxHeight = minBoundingBox.Height * 1.2; // Minimiss‰‰n boxin korkeus on n. yhden rivin verran
        if(currentHeigth < minimumParamBoxHeight)
            currentHeigth = minimumParamBoxHeight;
        return currentHeigth;
    }

    double SetMaximumParamBoxWidth(const Gdiplus::Graphics &graphics, const Gdiplus::Font &theUsedFont, const Gdiplus::StringFormat &theStringFormat, double currentWidth)
    {
        // Parametri boksilla pit‰‰ olla myˆs jokin minimi leveys ja se lasketaan t‰ss‰
        Gdiplus::RectF maxBoundingBox = ::CalcTextBoundingAbsoluteRect("1234567890123456789012345678901234567890", theUsedFont, theStringFormat, graphics);
        double maximumParamBoxWidth = maxBoundingBox.Width;
        if(currentWidth > maximumParamBoxWidth)
            currentWidth = maximumParamBoxWidth;
        return currentWidth;
    }

    // Laskee l‰hinn‰ poksin koon (pikseleiss‰)
    NFmiRect CalcBetaProductParamBoxSize(Gdiplus::Graphics *gdiPlusGraphics, const std::vector<std::string> &theParamBoxTexts, const Gdiplus::Font &theUsedFont, const Gdiplus::StringFormat &theStringFormat)
    {
        double totalHeight = 0;
        double maxWidth = 0;
        Gdiplus::RectF boundingBox;
        for(const auto &paramStr : theParamBoxTexts)
        {
            boundingBox = ::CalcTextBoundingAbsoluteRect(paramStr, theUsedFont, theStringFormat, *gdiPlusGraphics);
            totalHeight += boundingBox.Height;
            if(maxWidth < boundingBox.Width)
                maxWidth = boundingBox.Width;
        }

        maxWidth = ::SetMinimumParamBoxWidth(*gdiPlusGraphics, theUsedFont, theStringFormat, maxWidth);
        totalHeight = ::SetMinimumParamBoxHeight(*gdiPlusGraphics, theUsedFont, theStringFormat, totalHeight);
        maxWidth = ::SetMaximumParamBoxWidth(*gdiPlusGraphics, theUsedFont, theStringFormat, maxWidth);

        return NFmiRect(0, 0, maxWidth, totalHeight);
    }

#ifdef max
#undef max
#endif

    // If runtime info is added to beta-product param box:
    // 1. calc new param box size
    // 2. runtime info will be on the left side of parameters (own column)
    // 3. return x-offset where parameters are drawn from the total rect's left edge
    double MakeRunTimeInfoAdjustments(const Gdiplus::Graphics &graphics, const NFmiBetaProduct *currentBetaProduct, const Gdiplus::Font &theUsedFont, const Gdiplus::StringFormat &theStringFormat, NFmiRect &paramBoxAbsoluteRect)
    {
        if(currentBetaProduct->DisplayRunTimeInfo())
        {
            Gdiplus::RectF titleBoundingBox = ::CalcTextBoundingAbsoluteRect(NFmiBetaProductionSystem::RunTimeTitleString(), theUsedFont, theStringFormat, graphics);
            Gdiplus::RectF timeInfoBoundingBox = ::CalcTextBoundingAbsoluteRect(NFmiBetaProductionSystem::RunTimeFormatString(), theUsedFont, theStringFormat, graphics);
            double minHeight = (titleBoundingBox.Height + timeInfoBoundingBox.Height) * 1.2;
            double minWidth = std::max(titleBoundingBox.Width, timeInfoBoundingBox.Width);

            if(paramBoxAbsoluteRect.Height() < minHeight)
                paramBoxAbsoluteRect.Height(minHeight);
            paramBoxAbsoluteRect.Width(paramBoxAbsoluteRect.Width() + minWidth);
            return minWidth;
        }

        return 0;
    }

    void DrawGdiplusBoxRect(CtrlViewDocumentInterface *ctrlViewDocumentInterface, int mapViewDescTopIndex, Gdiplus::Graphics *gdiPlusGraphics, const NFmiRect &theBoxAbsoluteRect, const NFmiColor &theBoxFillColor, const NFmiColor &theBoxFrameCcolor, float theFrameThicknessInMM)
    {
        Gdiplus::SolidBrush aBrushBox(CtrlView::NFmiColor2GdiplusColor(theBoxFillColor));
        Gdiplus::GraphicsPath aPath;
        Gdiplus::Rect gdiRect(static_cast<INT>(theBoxAbsoluteRect.Left()), static_cast<INT>(theBoxAbsoluteRect.Top()), static_cast<INT>(theBoxAbsoluteRect.Width()), static_cast<INT>(theBoxAbsoluteRect.Height()));
        aPath.AddRectangle(gdiRect);
        aPath.CloseFigure();
        gdiPlusGraphics->FillPath(&aBrushBox, &aPath);

        Gdiplus::REAL penThickness = static_cast<Gdiplus::REAL>(theFrameThicknessInMM * ctrlViewDocumentInterface->GetGraphicalInfo(mapViewDescTopIndex).itsPixelsPerMM_y);
        Gdiplus::Pen penBox(CtrlView::NFmiColor2GdiplusColor(theBoxFrameCcolor), penThickness);
        gdiPlusGraphics->DrawPath(&penBox, &aPath);
    }

    void DrawBetaProductPlainParamBox(CtrlViewDocumentInterface *ctrlViewDocumentInterface, int mapViewDescTopIndex, Gdiplus::Graphics *gdiPlusGraphics, const NFmiRect &theParamBoxAbsoluteRect)
    {
        const float grayShade = 1.0f;
        NFmiColor rectFillcolor(grayShade, grayShade, grayShade, 0.15f); // Pit‰‰ olla l‰pikuultava
        NFmiColor rectFrameColor(0, 0, 0);
        DrawGdiplusBoxRect(ctrlViewDocumentInterface, mapViewDescTopIndex, gdiPlusGraphics, theParamBoxAbsoluteRect, rectFillcolor, rectFrameColor, 0.1f);
    }

    Gdiplus::RectF CalcParamTextBaseBoundingBox(const Gdiplus::Graphics &graphics, const NFmiRect &theParamBoxAbsoluteRect, const Gdiplus::Font &theUsedFont, const Gdiplus::StringFormat &theStringFormat, double paramStartingPointX)
    {
        // Lasketaan perus bounding boxi, jossa kiinnostava osio on sen korkeus (otetaan korkaus isosta H kirjaimesta)
        Gdiplus::RectF boundingBox = ::CalcTextBoundingAbsoluteRect("H", theUsedFont, theStringFormat, graphics);
        boundingBox.X = static_cast<Gdiplus::REAL>(theParamBoxAbsoluteRect.Left() + paramStartingPointX);
        boundingBox.Y = static_cast<Gdiplus::REAL>(theParamBoxAbsoluteRect.Top());
        boundingBox.Width = static_cast<Gdiplus::REAL>(theParamBoxAbsoluteRect.Width() - paramStartingPointX);
        return boundingBox;
    }

    void DrawAllParamRows(Gdiplus::Graphics &graphics, Gdiplus::RectF &paramTextBoundingBoxInOut, const std::vector<std::string> &theParamBoxTexts, const Gdiplus::Font &theUsedFont, const Gdiplus::StringFormat &theStringFormat, const std::vector<NFmiColor> &theParamBoxTextColors)
    {
        for(size_t i = 0; i < theParamBoxTexts.size(); i++)
        {
            std::wstring wideStr = CtrlView::StringToWString(theParamBoxTexts[i]);
            Gdiplus::SolidBrush textBrush(CtrlView::NFmiColor2GdiplusColor(theParamBoxTextColors[i]));
            graphics.DrawString(wideStr.c_str(), static_cast<INT>(wideStr.size()), &theUsedFont, paramTextBoundingBoxInOut, &theStringFormat, &textBrush);
            paramTextBoundingBoxInOut.Y += paramTextBoundingBoxInOut.Height;
        }
    }

    void DrawBetaProductRuntimeInfo(CtrlViewDocumentInterface *ctrlViewDocumentInterface, Gdiplus::Graphics *gdiPlusGraphics, const NFmiRect &theParamBoxAbsoluteRect, const Gdiplus::Font &theUsedFont, const Gdiplus::StringFormat &theStringFormat, double paramStartingPointX, const NFmiBetaProduct *currentBetaProduct)
    {
        if(currentBetaProduct->DisplayRunTimeInfo())
        {
            Gdiplus::SolidBrush textBrush(CtrlView::NFmiColor2GdiplusColor(NFmiColor(0, 0, 0)));
            Gdiplus::RectF boundingBox = ::CalcTextBoundingAbsoluteRect("H", theUsedFont, theStringFormat, *gdiPlusGraphics);
            boundingBox.X = static_cast<Gdiplus::REAL>(theParamBoxAbsoluteRect.Left());
            boundingBox.Y = static_cast<Gdiplus::REAL>(theParamBoxAbsoluteRect.Top());
            boundingBox.Width = static_cast<Gdiplus::REAL>(theParamBoxAbsoluteRect.Left() + paramStartingPointX);
            std::wstring titleStr = CtrlView::StringToWString(NFmiBetaProductionSystem::RunTimeTitleString());
            gdiPlusGraphics->DrawString(titleStr.c_str(), static_cast<INT>(titleStr.size()), &theUsedFont, boundingBox, &theStringFormat, &textBrush);

            boundingBox.Y += boundingBox.Height;
            std::string runtimeStr = ctrlViewDocumentInterface->BetaProductionSystem().BetaProductRuntime().LocalTime().ToStr(NFmiBetaProductionSystem::RunTimeFormatString(), ctrlViewDocumentInterface->Language());
            std::wstring runtimeWideStr = CtrlView::StringToWString(runtimeStr);
            gdiPlusGraphics->DrawString(runtimeWideStr.c_str(), static_cast<INT>(runtimeWideStr.size()), &theUsedFont, boundingBox, &theStringFormat, &textBrush);
        }
    }

    void DrawBetaProductParamBox(CtrlViewDocumentInterface *ctrlViewDocumentInterface, int mapViewDescTopIndex, Gdiplus::Graphics *gdiPlusGraphics, const NFmiRect &theParamBoxAbsoluteRect, const std::vector<std::string> &theParamBoxTexts, const Gdiplus::Font &theUsedFont, const Gdiplus::Font &theUsedRuntimeFont, const Gdiplus::StringFormat &theStringFormat, const std::vector<NFmiColor> &theParamBoxTextColors, double paramStartingPointX, const NFmiBetaProduct *currentBetaProduct)
    {
        DrawBetaProductPlainParamBox(ctrlViewDocumentInterface, mapViewDescTopIndex, gdiPlusGraphics, theParamBoxAbsoluteRect);
        Gdiplus::RectF paramTextBoundingBox = ::CalcParamTextBaseBoundingBox(*gdiPlusGraphics, theParamBoxAbsoluteRect, theUsedFont, theStringFormat, paramStartingPointX);
        ::DrawAllParamRows(*gdiPlusGraphics, paramTextBoundingBox, theParamBoxTexts, theUsedFont, theStringFormat, theParamBoxTextColors);
        DrawBetaProductRuntimeInfo(ctrlViewDocumentInterface, gdiPlusGraphics, theParamBoxAbsoluteRect, theUsedRuntimeFont, theStringFormat, paramStartingPointX, currentBetaProduct);
    }

    NFmiBetaProduct g_BetaProductForParamboxDraw;
}

namespace StationViews
{
	void DrawBetaProductParamBox(NFmiCtrlView *view, bool fCrossSectionInfoWanted, const NFmiBetaProduct* optionalBetaProduct)
	{
        if(view)
        {
            // Jos diPlusGraphics on 0-pointteri, on t‰nne tultu DrawOverBitmapThings -systeemin kautta ja sit‰ ei haluta tehd‰ koska on turha piirt‰‰ sama laatikko kahdesti p‰‰llekk‰in
            auto gdiPlusGraphics = view->GdiPlusGraphics();
            auto ctrlViewDocumentInterface = view->GetCtrlViewDocumentInterface();
            if(gdiPlusGraphics && ctrlViewDocumentInterface)
            {
                const NFmiBetaProduct *currentBetaProduct = optionalBetaProduct ? optionalBetaProduct : ctrlViewDocumentInterface->GetCurrentGeneratedBetaProduct();
                if(currentBetaProduct)
                {
                    FmiDirection paramBoxLocation = ::GetBetaProductParamBoxLocation(currentBetaProduct);
                    if(paramBoxLocation != kNoDirection)
                    {
                        auto mapViewDescTopIndex = view->MapViewDescTopIndex();
                        NFmiDrawParamList *drawParamList = ctrlViewDocumentInterface->DrawParamList(mapViewDescTopIndex, view->GetUsedParamRowIndex(view->ViewGridRowNumber(), view->ViewGridColumnNumber()));
                        std::vector<std::string> paramBoxTexts = ::MakeBetaProductParamBoxTexts(ctrlViewDocumentInterface, drawParamList, fCrossSectionInfoWanted, currentBetaProduct->ShowModelOriginTime());
                        std::vector<NFmiColor> paramBoxTextColors = ::MakeBetaProductParamBoxTextColors(ctrlViewDocumentInterface, drawParamList);
                        std::unique_ptr<Gdiplus::Font> usedFont = ::MakeParamBoxFont(ctrlViewDocumentInterface, mapViewDescTopIndex, 1.0);
                        Gdiplus::StringFormat stringFormat;
                        stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
                        stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);

                        NFmiRect paramBoxAbsoluteRect = CalcBetaProductParamBoxSize(gdiPlusGraphics, paramBoxTexts, *usedFont, stringFormat);
                        std::unique_ptr<Gdiplus::Font> usedRuntimeFont = MakeParamBoxFont(ctrlViewDocumentInterface, mapViewDescTopIndex, 0.8);
                        double paramStartingPointX = ::MakeRunTimeInfoAdjustments(*gdiPlusGraphics, currentBetaProduct, *usedRuntimeFont, stringFormat, paramBoxAbsoluteRect);
                        CtrlView::PlaceBoxIntoFrame(paramBoxAbsoluteRect, view->GetFrameForParamBox(), view->GetToolBox(), paramBoxLocation);
                        ::DrawBetaProductParamBox(ctrlViewDocumentInterface, mapViewDescTopIndex, gdiPlusGraphics, paramBoxAbsoluteRect, paramBoxTexts, *usedFont, *usedRuntimeFont, stringFormat, paramBoxTextColors, paramStartingPointX, currentBetaProduct);
                    }
                }
            }
        }
	}

    const NFmiBetaProduct& GetPrintingBetaProductForParamBoxDraw(int viewDesctopIndex, CtrlViewDocumentInterface& ctrlViewDocumentInterface)
    {
        static bool betaProductInitialized = false;
        if(!betaProductInitialized)
        {
            betaProductInitialized = true;
            g_BetaProductForParamboxDraw.ShowModelOriginTime(true);
            g_BetaProductForParamboxDraw.ParamBoxLocation(kTopLeft);
        }

        g_BetaProductForParamboxDraw.ParamBoxLocation(ctrlViewDocumentInterface.ParamWindowViewPosition(viewDesctopIndex));

        return g_BetaProductForParamboxDraw;
    }

}
