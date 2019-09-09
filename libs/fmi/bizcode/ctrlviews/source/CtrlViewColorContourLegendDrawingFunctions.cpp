#include "stdafx.h"
#include "CtrlViewColorContourLegendDrawingFunctions.h"
#include "NFmiColorContourLegendSettings.h"
#include "NFmiColorContourLegendValues.h"
#include "ToolMasterDrawingFunctions.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "GraphicalInfo.h"
#include <gdiplus.h>

namespace
{
    class LegendDrawingMeasures
    {
    public:
        NFmiPoint backgroundRectSizeInPixels = NFmiPoint(0, 0);
        NFmiPoint colorRectSizeInPixels = NFmiPoint(0, 0);
        double maxValueStringLengthInPixels = 0;
        double paddingLengthInPixels = 0;
        double usedFontSizeInPixels = 0;
        double usedFontSizeInMM = 0;
        Gdiplus::Rect backgroundRectInPixels;
    };

    static bool IsTransparentColor(const NFmiColor& color)
    {
        return color.Alpha() == 0;
    }

    LegendDrawingMeasures CalculateLegendDrawingMeasures(const NFmiColorContourLegendSettings& colorContourLegendSettings, const NFmiColorContourLegendValues& colorContourLegendValues, float sizeFactor, const CtrlViewUtils::GraphicalInfo& graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics)
    {
        auto pixelsPerMM = graphicalInfo.itsPixelsPerMM_y;
        LegendDrawingMeasures legendDrawingMeasures;
        // 1. Laske pohjalaatikon koko
        // 1.1. Laske tekstin korkeus, siit‰ riippuu muidenkin osien koot
        legendDrawingMeasures.usedFontSizeInMM = colorContourLegendSettings.fontSizeInMM() * sizeFactor;
        legendDrawingMeasures.usedFontSizeInPixels = legendDrawingMeasures.usedFontSizeInMM * pixelsPerMM;
        legendDrawingMeasures.colorRectSizeInPixels = NFmiPoint(legendDrawingMeasures.usedFontSizeInPixels, legendDrawingMeasures.usedFontSizeInPixels);
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
        legendDrawingMeasures.maxValueStringLengthInPixels = maxStringLengthInPixels;
        legendDrawingMeasures.paddingLengthInPixels = legendDrawingMeasures.usedFontSizeInPixels * 0.3;

        // 1.3. Koko laatikon korkeus = N kpl laatikoiden korkeus
        double backgroundRectHeigthInPixels = colorContourLegendValues.classColors().size() * legendDrawingMeasures.colorRectSizeInPixels.Y();
        // Lis‰t‰‰n korkeuteen viel‰ padding ala ja yl‰reunaan ja otsikkorivin korkeus
        backgroundRectHeigthInPixels += legendDrawingMeasures.colorRectSizeInPixels.Y() + 2 * legendDrawingMeasures.paddingLengthInPixels;
        // 1.4. Koko laatikon leveys = maksimi tekstin leveys + laatikon leveys
        double backgroundRectWidthInPixels = legendDrawingMeasures.maxValueStringLengthInPixels + legendDrawingMeasures.colorRectSizeInPixels.X();
        // Lis‰t‰‰n leveyteen viel‰ padding vasempaan ja oikeaan reunaan
        backgroundRectWidthInPixels += 2 * legendDrawingMeasures.paddingLengthInPixels;
        legendDrawingMeasures.backgroundRectSizeInPixels = NFmiPoint(backgroundRectWidthInPixels, backgroundRectHeigthInPixels);

        return legendDrawingMeasures;
    }

    void DrawNormalColorContourLegendBackground(const NFmiColorContourLegendSettings& colorContourLegendSettings, LegendDrawingMeasures& legendDrawingMeasures, const Gdiplus::PointF& lastLegendBottomRightCornerInPixels, Gdiplus::Graphics& gdiPlusGraphics)
    {
        // Laatikko pit‰‰ viel‰ 'k‰‰nt‰‰' y-akselin suhteen
        auto yLocationInPixels = boost::math::iround(lastLegendBottomRightCornerInPixels.Y - legendDrawingMeasures.backgroundRectSizeInPixels.Y());
        Gdiplus::Point locationInPixels(boost::math::iround(lastLegendBottomRightCornerInPixels.X), yLocationInPixels);
        Gdiplus::Size sizeInPixels(boost::math::iround(legendDrawingMeasures.backgroundRectSizeInPixels.X()), boost::math::iround(legendDrawingMeasures.backgroundRectSizeInPixels.Y()));
        legendDrawingMeasures.backgroundRectInPixels = Gdiplus::Rect(locationInPixels, sizeInPixels);

        CtrlView::DrawRect(gdiPlusGraphics,
            legendDrawingMeasures.backgroundRectInPixels,
            colorContourLegendSettings.backgroundRectSettings().frameLineColor(),
            colorContourLegendSettings.backgroundRectSettings().fillColor(),
            true, true, true,
            static_cast<float>(colorContourLegendSettings.backgroundRectSettings().frameLineWidthInMM()),
            static_cast<Gdiplus::DashStyle>(colorContourLegendSettings.backgroundRectSettings().frameLineType()));
    }

    void DrawNormalColorContourLegendClassValueTexts(const NFmiColorContourLegendSettings& colorContourLegendSettings, const LegendDrawingMeasures& legendDrawingMeasures, const NFmiColorContourLegendValues& colorContourLegendValues, const Gdiplus::PointF& lastLegendBottomRightCornerInPixels, NFmiToolBox* toolbox, const CtrlViewUtils::GraphicalInfo& graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics)
    {
        const auto& textColor = colorContourLegendSettings.backgroundRectSettings().frameLineColor();
        auto pixelsPerMM = graphicalInfo.itsPixelsPerMM_y;
        const auto& fontName = CtrlView::StringToWString(colorContourLegendSettings.fontName());
        auto paddingInPixels = boost::math::iround(legendDrawingMeasures.paddingLengthInPixels);
        auto heigthInPixels = boost::math::iround(legendDrawingMeasures.colorRectSizeInPixels.Y());
        int currentTextPosXInPixels = boost::math::iround(lastLegendBottomRightCornerInPixels.X + paddingInPixels + legendDrawingMeasures.maxValueStringLengthInPixels);
        int currentTextPosYInPixels = boost::math::iround(lastLegendBottomRightCornerInPixels.Y - paddingInPixels + heigthInPixels * 0.15);
        for(const auto& classLimitText : colorContourLegendValues.classLimitTexts())
        {
            currentTextPosYInPixels -= heigthInPixels;
            auto textLocationInPixels = Gdiplus::Point(currentTextPosXInPixels, currentTextPosYInPixels);
            auto relativeTextPosition = CtrlView::GdiplusPoint2Relative(toolbox, textLocationInPixels);
            CtrlView::DrawTextToRelativeLocation(gdiPlusGraphics, textColor, legendDrawingMeasures.usedFontSizeInMM, classLimitText, relativeTextPosition, pixelsPerMM, toolbox, fontName, kRight, Gdiplus::FontStyleRegular);
        }

        // Piirret‰‰n viel‰ otsikko
        currentTextPosXInPixels = boost::math::iround(lastLegendBottomRightCornerInPixels.X + paddingInPixels);
        currentTextPosYInPixels -= boost::math::iround(heigthInPixels * 1.7);
        auto textLocationInPixels = Gdiplus::Point(currentTextPosXInPixels, currentTextPosYInPixels);
        auto relativeTextPosition = CtrlView::GdiplusPoint2Relative(toolbox, textLocationInPixels);
        gdiPlusGraphics.SetClip(legendDrawingMeasures.backgroundRectInPixels);
        CtrlView::DrawTextToRelativeLocation(gdiPlusGraphics, textColor, legendDrawingMeasures.usedFontSizeInMM, colorContourLegendValues.name(), relativeTextPosition, pixelsPerMM, toolbox, fontName, kLeft, Gdiplus::FontStyleRegular);
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
            useFill, true, false,
            static_cast<float>(boost::math::iround(drawSettings.frameLineWidthInMM() * pixelsPerMM)),
            static_cast<Gdiplus::DashStyle>(drawSettings.frameLineType()));
    }

    void DrawNormalColorContourLegendClassColors(const NFmiColorContourLegendSettings& colorContourLegendSettings, const LegendDrawingMeasures& legendDrawingMeasures, const NFmiColorContourLegendValues& colorContourLegendValues, const Gdiplus::PointF& lastLegendBottomRightCornerInPixels, const CtrlViewUtils::GraphicalInfo& graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics)
    {
        const auto& classColors = colorContourLegendValues.classColors();
        auto paddingInPixels = boost::math::iround(legendDrawingMeasures.paddingLengthInPixels);
        int currentRectTopInPixels = boost::math::iround(lastLegendBottomRightCornerInPixels.Y - paddingInPixels);
        int rectLeftInPixels = boost::math::iround(lastLegendBottomRightCornerInPixels.X + paddingInPixels + legendDrawingMeasures.maxValueStringLengthInPixels);
        auto widthInPixels = boost::math::iround(legendDrawingMeasures.colorRectSizeInPixels.X());
        auto heigthInPixels = boost::math::iround(legendDrawingMeasures.colorRectSizeInPixels.Y());
        for(size_t index = 0; index < classColors.size(); index++)
        {
            currentRectTopInPixels -= heigthInPixels;
            auto colorRectInPixels = Gdiplus::Rect(rectLeftInPixels, currentRectTopInPixels, widthInPixels, heigthInPixels);
            const auto& color = classColors[index];

            if(::IsTransparentColor(color))
            {
                DrawNormalColorContourLegendClassColorRect(colorContourLegendSettings.invsibleColorRectSettings(), colorRectInPixels, color, true, graphicalInfo, gdiPlusGraphics);
            }
            else
            {
                DrawNormalColorContourLegendClassColorRect(colorContourLegendSettings.backgroundRectSettings(), colorRectInPixels, color, false, graphicalInfo, gdiPlusGraphics);
            }
        }
    }
}

namespace CtrlView
{
    void DrawNormalColorContourLegend(const NFmiColorContourLegendSettings & colorContourLegendSettings, const NFmiColorContourLegendValues& colorContourLegendValues, NFmiPoint& lastLegendRelativeBottomRightCornerInOut, NFmiToolBox *toolbox, const CtrlViewUtils::GraphicalInfo &graphicalInfo, Gdiplus::Graphics& gdiPlusGraphics)
    {
        float sizeFactor = ::CalcMMSizeFactor(static_cast<float>(graphicalInfo.itsViewHeightInMM), 1.1f);
        if(sizeFactor < 1)
            sizeFactor = std::pow(sizeFactor, 1.4f);
        auto legendDrawingMeasures = CalculateLegendDrawingMeasures(colorContourLegendSettings, colorContourLegendValues, sizeFactor, graphicalInfo, gdiPlusGraphics);
        Gdiplus::PointF lastLegendBottomRightCornerInPixels = CtrlView::Relative2GdiplusPoint(toolbox, lastLegendRelativeBottomRightCornerInOut);
        lastLegendBottomRightCornerInPixels.X += static_cast<float>(legendDrawingMeasures.paddingLengthInPixels);
        // 2. Piirr‰ pohja laatikko
        DrawNormalColorContourLegendBackground(colorContourLegendSettings, legendDrawingMeasures, lastLegendBottomRightCornerInPixels, gdiPlusGraphics);
        // 3. Piirr‰ value tekstit loopissa
        DrawNormalColorContourLegendClassValueTexts(colorContourLegendSettings, legendDrawingMeasures, colorContourLegendValues, lastLegendBottomRightCornerInPixels, toolbox, graphicalInfo, gdiPlusGraphics);
        // 4. Piirr‰ v‰ri laatikot loopissa
        // 5. Piirr‰ otsikko teksti
        DrawNormalColorContourLegendClassColors(colorContourLegendSettings, legendDrawingMeasures, colorContourLegendValues, lastLegendBottomRightCornerInPixels, graphicalInfo, gdiPlusGraphics);
        // 6. Move last legend BR corner
        auto relativeLegendRect = CtrlView::GdiplusRect2Relative(toolbox, legendDrawingMeasures.backgroundRectInPixels);
        lastLegendRelativeBottomRightCornerInOut = relativeLegendRect.BottomRight();
    }

    // xyArea sis‰lt‰‰ suhteellisen kartan alueen, joka voi olla 0,0 - 1,1 (kartta peitt‰‰ koko n‰ytˆn ilman
    // aikakontrolli-ikkunaa) tai esim. 0, 0 - 0.5, 0.46 (karttan‰ytˆll‰ 2x2 ruudukko ja aikakontrolli-ikkuna vie
    // n‰ytˆn alaosan).
    // xyPoint on sijainti 0,0 - 1,1 maailmassa (voi olla myˆs sen ulkona).
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
