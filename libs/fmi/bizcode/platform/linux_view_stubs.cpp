// Linux definitions for the parts of the view stack that are not ported yet.
//
// SmartMet's real map view (NFmiEditMapView and the NFmiStationViewHandler grid under
// it) compiles on Linux, but a handful of its drawing helpers have their bodies inside
// "#ifndef UNIX" blocks, and one editing class is excluded from the Linux build
// altogether. Nothing linked those view classes into an executable before, so the gaps
// only surfaced once the Qt window started hosting the real view.
//
// Everything here is a placeholder that keeps the link complete so the rest of the map
// view draws. Each one logs the first time it is hit, so the terminal shows exactly
// which unported pieces a given map view actually needs.
//
// To finish a piece: implement it in its own source file (the Windows body is right
// there inside the "#ifndef UNIX" block) and delete the stub from here.

#ifdef UNIX

#include "BetaProductParamBoxFunctions.h"
#include "NFmiBetaProductSystem.h"   // NFmiBetaProduct
#include "NFmiDataParamModifier.h"
#include "NFmiStreamLineView.h"
#include "NFmiTimeControlView.h"
#include "NFmiToolBox.h"
#include "catlog/catlog.h"

#include <set>
#include <string>

namespace
{
    void logUnported(const char* theFunctionName)
    {
        static std::set<std::string> alreadyLogged;
        if(alreadyLogged.insert(theFunctionName).second)
        {
            CatLog::logMessage(std::string("Not ported to Linux yet: ") + theFunctionName,
                               CatLog::Severity::Warning, CatLog::Category::Visualization, true);
        }
    }
}

// ---- time control view: animation buttons and time filter markers -------------------
// The Windows bodies draw GDI+ button bitmaps, so these need the Qt drawing work first.

NFmiPoint NFmiTimeControlView::CalcAnimationButtonRelativeSize(double theSizeFactorX,
                                                               double theSizeFactorY)
{
    logUnported("NFmiTimeControlView::CalcAnimationButtonRelativeSize");
    // The Windows version sizes the button from its bitmap and falls back to 16 pixels
    // when no bitmap is loaded. Without the bitmaps that fallback is all there is.
    if(!itsToolBox)
        return NFmiPoint(0, 0);
    return NFmiPoint(itsToolBox->SX(static_cast<long>(16 * theSizeFactorX)),
                     itsToolBox->SY(static_cast<long>(16 * theSizeFactorY)));
}

void NFmiTimeControlView::DrawAnimationBox(void)
{
    logUnported("NFmiTimeControlView::DrawAnimationBox");
}

void NFmiTimeControlView::DrawTimeFilterTimes(void)
{
    logUnported("NFmiTimeControlView::DrawTimeFilterTimes");
}

void NFmiTimeControlView::DrawVirtualTimeData()
{
    logUnported("NFmiTimeControlView::DrawVirtualTimeData");
}

// ---- streamline rendering -----------------------------------------------------------

void NFmiStreamLineView::DrawStreamLinePaths(const StreamlineCalculationParameters& /* theCalcParams */,
                                             const std::vector<NFmiStreamlineData>& /* theLatlonPaths */)
{
    logUnported("NFmiStreamLineView::DrawStreamLinePaths");
}

// ---- beta product parameter box -----------------------------------------------------
// Beta products are a printing/export feature that the Qt port does not offer yet.

namespace StationViews
{
    void DrawBetaProductParamBox(NFmiCtrlView* /* view */,
                                 bool /* fCrossSectionInfoWanted */,
                                 const NFmiBetaProduct* /* optionalBetaProduct */)
    {
        logUnported("StationViews::DrawBetaProductParamBox");
    }

    const NFmiBetaProduct& GetPrintingBetaProductForParamBoxDraw(
        int /* viewDesctopIndex */, CtrlViewDocumentInterface& /* ctrlViewDocumentInterface */)
    {
        logUnported("StationViews::GetPrintingBetaProductForParamBoxDraw");
        static const NFmiBetaProduct emptyBetaProduct;
        return emptyBetaProduct;
    }
}

// ---- edited data brush --------------------------------------------------------------
// NFmiDataParamModifier.cpp is excluded from the Linux build: it needs the Windows-only
// libs/fmi/multiprocesstools, the Uniras ToolMaster gridding, and
// NFmiDataMatrix::InterpolatedValue, which exists only in the workstation's forked
// modules/newbase and not in mainline newbase. Only the editing brush uses it, and the
// Qt port has no editing UI, so the map view just needs these to link.

NFmiDataParamModifier::NFmiDataParamModifier(std::shared_ptr<NFmiFastQueryInfo>& theInfo,
                                             std::shared_ptr<NFmiDrawParam>& theDrawParam,
                                             std::shared_ptr<NFmiAreaMaskList>& theMaskList,
                                             unsigned long theAreaMask)
    : itsInfo(theInfo)
    , itsDrawParam(theDrawParam)
    , itsParamMaskList(theMaskList)
    , itsMaskType(theAreaMask)
{
    logUnported("NFmiDataParamModifier (edited data brush)");
}

bool NFmiDataParamModifier::ModifyData(void)
{
    logUnported("NFmiDataParamModifier::ModifyData");
    return false;
}

bool NFmiDataParamModifier::ModifyData2(void)
{
    logUnported("NFmiDataParamModifier::ModifyData2");
    return false;
}

float NFmiDataParamModifier::Calculate(const float& theValue)
{
    return theValue;
}

#endif // UNIX
