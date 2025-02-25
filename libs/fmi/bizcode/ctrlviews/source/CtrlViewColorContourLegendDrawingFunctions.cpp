#include "stdafx.h"
#include "CtrlViewColorContourLegendDrawingFunctions.h"
#include "NFmiColorContourLegendSettings.h"
#include "NFmiColorContourLegendValues.h"
#include "ToolMasterDrawingFunctions.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "GraphicalInfo.h"
#include "ToolMasterColorCube.h"
#include <gdiplus.h>

namespace
{
    class LegendDrawingMeasures
    {
    public:
        Gdiplus::Point backgroundRectSizeInPixels = Gdiplus::Point(0, 0);
        Gdiplus::Point colorRectSizeInPixels = Gdiplus::Point(0, 0);
        int maxValueStringLengthInPixels = 0;
        int paddingLengthInPixels = 0;
        int usedFontSizeInPixels = 0;
        int titleTextAreaHeightInPixels = 0;
        double usedFontSizeInPixelsReal = 0;
        double usedFontSizeInMM = 0;
        Gdiplus::Rect backgroundRectInPixels;
    };

    LegendDrawingMeasures CalculateLegendDrawingMeasures(const NFmiColorContourLegendSettings& colorContourLegendSettings, const NFmiColorContourLegendValues& colorContourLegendValues, float sizeFactor, const CtrlViewUtils::GraphicalInfo& graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics)
    {
        auto pixelsPerMM = graphicalInfo.itsPixelsPerMM_y;
        LegendDrawingMeasures legendDrawingMeasures;
        // 1. Laske pohjalaatikon koko
        // 1.1. Laske tekstin korkeus, siit� riippuu muidenkin osien koot
        legendDrawingMeasures.usedFontSizeInMM = colorContourLegendSettings.fontSizeInMM() * sizeFactor;
        legendDrawingMeasures.usedFontSizeInPixelsReal = legendDrawingMeasures.usedFontSizeInMM * pixelsPerMM;
        legendDrawingMeasures.usedFontSizeInPixels = boost::math::iround(legendDrawingMeasures.usedFontSizeInPixelsReal);
        legendDrawingMeasures.colorRectSizeInPixels = Gdiplus::Point(legendDrawingMeasures.usedFontSizeInPixels, legendDrawingMeasures.usedFontSizeInPixels);
        // 1.2. Laske maksimi tekstin leveys
        double maxStringLengthInPixels = 0;
        auto fontNameWide = CtrlView::StringToWString(colorContourLegendSettings.fontName());
        auto usedFont = CtrlView::CreateFontPtr(legendDrawingMeasures.usedFontSizeInMM, pixelsPerMM, fontNameWide, Gdiplus::FontStyleRegular);
        Gdiplus::PointF oringinInPixels(0, 0);
        const auto& classLimitTexts = colorContourLegendValues.classLimitTexts();
        for(const auto& classLimitText : classLimitTexts)
        {
            auto stringBoundingRectInPixels = CtrlView::GetStringBoundingBox(gdiPlusGraphics, classLimitText, oringinInPixels, *usedFont);
            if(maxStringLengthInPixels < stringBoundingRectInPixels.Width)
                maxStringLengthInPixels = stringBoundingRectInPixels.Width;
        }
        legendDrawingMeasures.maxValueStringLengthInPixels = static_cast<int>(std::ceil(maxStringLengthInPixels));
        legendDrawingMeasures.paddingLengthInPixels = boost::math::iround(legendDrawingMeasures.usedFontSizeInPixelsReal * 0.3);

        // 1.3. Koko laatikon korkeus = N kpl laatikoiden korkeus
        int backgroundRectHeigthInPixels = static_cast<int>(colorContourLegendValues.classColors().size()) * legendDrawingMeasures.colorRectSizeInPixels.Y;
        // Lasketaan otsikko alueen korkeus (tilaa 1.6 riville teksti�, 1.6, koska jos pit�� menn� 2-3 riviseksi, t�ll�in fontin kokoa pienennet��n)
        // factor x rivi korkeus + yl�osan padding.
        legendDrawingMeasures.titleTextAreaHeightInPixels = boost::math::iround(1.6 * legendDrawingMeasures.colorRectSizeInPixels.Y + legendDrawingMeasures.paddingLengthInPixels);
        // Lis�t��n lopulliseen korkeuteen viel� otsikkoalueen korkeus ja padding alareunaan.
        backgroundRectHeigthInPixels += legendDrawingMeasures.titleTextAreaHeightInPixels + legendDrawingMeasures.paddingLengthInPixels;
        // 1.4. Koko laatikon leveys = maksimi tekstin leveys + laatikon leveys
        int backgroundRectWidthInPixels = legendDrawingMeasures.maxValueStringLengthInPixels + legendDrawingMeasures.colorRectSizeInPixels.X;
        // Lis�t��n leveyteen viel� padding vasempaan ja oikeaan reunaan
        backgroundRectWidthInPixels += (2 * legendDrawingMeasures.paddingLengthInPixels);
        legendDrawingMeasures.backgroundRectSizeInPixels = Gdiplus::Point(backgroundRectWidthInPixels, backgroundRectHeigthInPixels);

        return legendDrawingMeasures;
    }

    void DrawNormalColorContourLegendBackground(const NFmiColorContourLegendSettings& colorContourLegendSettings, LegendDrawingMeasures& legendDrawingMeasures, const Gdiplus::Point& lastLegendBottomRightCornerInPixels, Gdiplus::Graphics& gdiPlusGraphics, const Gdiplus::Rect& clipRect)
    {
        // Laatikko pit�� viel� 'k��nt��' y-akselin suhteen
        auto yLocationInPixels = lastLegendBottomRightCornerInPixels.Y - legendDrawingMeasures.backgroundRectSizeInPixels.Y;
        Gdiplus::Point locationInPixels(lastLegendBottomRightCornerInPixels.X, yLocationInPixels);
        Gdiplus::Size sizeInPixels(legendDrawingMeasures.backgroundRectSizeInPixels.X, legendDrawingMeasures.backgroundRectSizeInPixels.Y);
        legendDrawingMeasures.backgroundRectInPixels = Gdiplus::Rect(locationInPixels, sizeInPixels);

        gdiPlusGraphics.SetClip(clipRect);
        CtrlView::DrawRect(gdiPlusGraphics,
            legendDrawingMeasures.backgroundRectInPixels,
            colorContourLegendSettings.backgroundRectSettings().frameLineColor(),
            colorContourLegendSettings.backgroundRectSettings().fillColor(),
            true, true,
            static_cast<float>(colorContourLegendSettings.backgroundRectSettings().frameLineWidthInMM()),
            static_cast<Gdiplus::DashStyle>(colorContourLegendSettings.backgroundRectSettings().frameLineType()));
        gdiPlusGraphics.ResetClip();
    }

    void DrawNormalColorContourLegendClassValueTexts(const NFmiColorContourLegendSettings& colorContourLegendSettings, const LegendDrawingMeasures& legendDrawingMeasures, const NFmiColorContourLegendValues& colorContourLegendValues, const Gdiplus::Point& lastLegendBottomRightCornerInPixels, NFmiToolBox* toolbox, const CtrlViewUtils::GraphicalInfo& graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics, const Gdiplus::Rect & finalClipRectInPixels)
    {
        gdiPlusGraphics.SetClip(finalClipRectInPixels);
        const auto& textColor = colorContourLegendSettings.backgroundRectSettings().frameLineColor();
        auto pixelsPerMM = graphicalInfo.itsPixelsPerMM_y;
        const auto& fontName = CtrlView::StringToWString(colorContourLegendSettings.fontName());
        auto paddingInPixels = legendDrawingMeasures.paddingLengthInPixels;
        auto heigthInPixels = legendDrawingMeasures.colorRectSizeInPixels.Y;
        int currentTextPosXInPixels = lastLegendBottomRightCornerInPixels.X + paddingInPixels + legendDrawingMeasures.maxValueStringLengthInPixels;
        int currentTextPosYInPixels = lastLegendBottomRightCornerInPixels.Y - paddingInPixels;
        float startPositionAdjustFactor = 0.15f;
        currentTextPosYInPixels += boost::math::iround(heigthInPixels * startPositionAdjustFactor);
        for(const auto& classLimitText : colorContourLegendValues.classLimitTexts())
        {
            currentTextPosYInPixels -= heigthInPixels;
            auto textLocationInPixels = Gdiplus::Point(currentTextPosXInPixels, currentTextPosYInPixels);
            auto relativeTextPosition = CtrlView::GdiplusPoint2Relative(toolbox, textLocationInPixels);
            CtrlView::DrawTextToRelativeLocation(gdiPlusGraphics, textColor, legendDrawingMeasures.usedFontSizeInMM, classLimitText, relativeTextPosition, pixelsPerMM, toolbox, fontName, kRight, Gdiplus::FontStyleRegular);
        }

        // Piirret��n viel� otsikko alkaen vasemmasta yl�reunasta, lasketaan hieman ekstra tilaa
        // pystysuunnassa, koska uusi "parametrin nimi"/title piirtokoodi pyrkii saamaan pidemm�tkin tekstit 
        // n�kyville pienent�m�ll� fonttia ja k�ytt�m�ll� wrap optiota ja useampia riveja tarvittaessa.
        const auto& bgRect = legendDrawingMeasures.backgroundRectInPixels;
        Gdiplus::PointF topLeftCornerInPixels((float)bgRect.GetLeft(), (float)bgRect.GetTop());
        Gdiplus::SizeF layoutRectSizeInPixels((float)(bgRect.GetRight() - bgRect.GetLeft()), (float)legendDrawingMeasures.titleTextAreaHeightInPixels);
        Gdiplus::RectF layoutRectInPixels(topLeftCornerInPixels, layoutRectSizeInPixels);
        CtrlView::DrawWrappedTextToRelativeLocation(gdiPlusGraphics, textColor, legendDrawingMeasures.usedFontSizeInMM, colorContourLegendValues.name(), pixelsPerMM, toolbox, fontName, kTopLeft, layoutRectInPixels, Gdiplus::FontStyleRegular);
        gdiPlusGraphics.ResetClip();
    }

    void DrawNormalColorContourLegendClassColorRect(const NFmiColorRectSettings& drawSettings, const Gdiplus::Rect& rectInPixels, const NFmiColor& color, bool transparentColor, const CtrlViewUtils::GraphicalInfo& graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics)
    {
        auto pixelsPerMM = graphicalInfo.itsPixelsPerMM_y;
        auto useFill = !transparentColor;
        CtrlView::DrawRect(gdiPlusGraphics,
            rectInPixels,
            drawSettings.frameLineColor(),
            color,
            useFill, true,
            static_cast<float>(boost::math::iround(drawSettings.frameLineWidthInMM() * pixelsPerMM)),
            static_cast<Gdiplus::DashStyle>(drawSettings.frameLineType()));
    }

    void DrawNormalColorContourLegendClassColors(const NFmiColorContourLegendSettings& colorContourLegendSettings, const LegendDrawingMeasures& legendDrawingMeasures, const NFmiColorContourLegendValues& colorContourLegendValues, const Gdiplus::Point& lastLegendBottomRightCornerInPixels, const CtrlViewUtils::GraphicalInfo& graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics, const Gdiplus::Rect& finalClipRectInPixels)
    {
        gdiPlusGraphics.SetClip(finalClipRectInPixels);
        const auto& classColors = colorContourLegendValues.classColors();
        auto paddingInPixels = legendDrawingMeasures.paddingLengthInPixels;
        int currentRectTopInPixels = lastLegendBottomRightCornerInPixels.Y - paddingInPixels;
        int rectLeftInPixels = lastLegendBottomRightCornerInPixels.X + paddingInPixels + legendDrawingMeasures.maxValueStringLengthInPixels;
        auto widthInPixels = legendDrawingMeasures.colorRectSizeInPixels.X;
        auto heigthInPixels = legendDrawingMeasures.colorRectSizeInPixels.Y;
        for(size_t index = 0; index < classColors.size(); index++)
        {
            currentRectTopInPixels -= heigthInPixels;
            auto colorRectInPixels = Gdiplus::Rect(rectLeftInPixels, currentRectTopInPixels, widthInPixels, heigthInPixels);
            const auto& color = classColors[index];

            if(ToolMasterColorCube::IsColorFullyTransparent(color))
            {
                if(colorContourLegendSettings.drawTransparentRects())
                    DrawNormalColorContourLegendClassColorRect(colorContourLegendSettings.invsibleColorRectSettings(), colorRectInPixels, color, true, graphicalInfo, gdiPlusGraphics);
            }
            else
            {
                DrawNormalColorContourLegendClassColorRect(colorContourLegendSettings.backgroundRectSettings(), colorRectInPixels, color, false, graphicalInfo, gdiPlusGraphics);
            }
        }
        gdiPlusGraphics.ResetClip();
    }

    Gdiplus::Point ConvertRealPointToIntPoint(const Gdiplus::PointF& realPoint)
    {
        return Gdiplus::Point(boost::math::iround(realPoint.X), boost::math::iround(realPoint.Y));
    }
}

namespace CtrlView
{
    void DrawNormalColorContourLegend(const NFmiColorContourLegendSettings & colorContourLegendSettings, const NFmiColorContourLegendValues& colorContourLegendValues, NFmiPoint& lastLegendRelativeBottomRightCornerInOut, NFmiToolBox *toolbox, const CtrlViewUtils::GraphicalInfo &graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics, float sizeFactor, const NFmiRect& relativeDataRect)
    {
        auto legendDrawingMeasures = CalculateLegendDrawingMeasures(colorContourLegendSettings, colorContourLegendValues, sizeFactor, graphicalInfo, gdiPlusGraphics);
        auto lastLegendBottomRightCornerInPixels = ConvertRealPointToIntPoint(CtrlView::Relative2GdiplusPoint(toolbox, lastLegendRelativeBottomRightCornerInOut));
        lastLegendBottomRightCornerInPixels.X += legendDrawingMeasures.paddingLengthInPixels;
        // 2. Piirr� pohja laatikko
        DrawNormalColorContourLegendBackground(colorContourLegendSettings, legendDrawingMeasures, lastLegendBottomRightCornerInPixels, gdiPlusGraphics, CtrlView::Relative2GdiplusRect(toolbox, relativeDataRect));
        auto relativeBackgroundRect = CtrlView::GdiplusRect2Relative(toolbox, legendDrawingMeasures.backgroundRectInPixels);
        // Lopullinen clippaus laatikko on data-laatikon ja background-rectin leikkaus
        auto finalRelativeClipRect = relativeDataRect.Intersection(relativeBackgroundRect);
        auto finalClipRectInPixels = CtrlView::Relative2GdiplusRect(toolbox, finalRelativeClipRect);
        // 3. Piirr� value tekstit loopissa
        DrawNormalColorContourLegendClassValueTexts(colorContourLegendSettings, legendDrawingMeasures, colorContourLegendValues, lastLegendBottomRightCornerInPixels, toolbox, graphicalInfo, gdiPlusGraphics, finalClipRectInPixels);
        // 4. Piirr� v�ri laatikot loopissa
        // 5. Piirr� otsikko teksti
        DrawNormalColorContourLegendClassColors(colorContourLegendSettings, legendDrawingMeasures, colorContourLegendValues, lastLegendBottomRightCornerInPixels, graphicalInfo, gdiPlusGraphics, finalClipRectInPixels);
        // 6. Move last legend BR corner
        auto relativeLegendRect = CtrlView::GdiplusRect2Relative(toolbox, legendDrawingMeasures.backgroundRectInPixels);
        lastLegendRelativeBottomRightCornerInOut = relativeLegendRect.BottomRight();
    }

    // xyArea sis�lt�� suhteellisen kartan alueen, joka voi olla 0,0 - 1,1 (kartta peitt�� koko n�yt�n ilman
    // aikakontrolli-ikkunaa) tai esim. 0, 0 - 0.5, 0.46 (karttan�yt�ll� 2x2 ruudukko ja aikakontrolli-ikkuna vie
    // n�yt�n alaosan).
    // xyPoint on sijainti 0,0 - 1,1 maailmassa (voi olla my�s sen ulkona).
    // Lasketaan piste, joka on xyArea:n maailmassa, mutta sijoitettuna niin kuin xyPoint
    // olisi sijoitettu 0,0 - 1,1 laatikkoon. Esim.
    // xyArea = 0, 0 - 0.5, 0.46
    // xyPoint = 0.3, 0.4
    // => 0.15, 0.184
    NFmiPoint CalcProjectedPointInRectsXyArea(const NFmiRect& xyArea, const NFmiPoint& xyPoint)
    {
        auto x = xyArea.Left() + xyArea.Width() * xyPoint.X();
        auto y = xyArea.Top() + xyArea.Height() * xyPoint.Y();
        return NFmiPoint(x, y);
    }
}
