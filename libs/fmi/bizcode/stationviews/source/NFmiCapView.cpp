#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255 merkkiä joka johtuu 'puretuista' STL-template nimistä)
#define _USE_MATH_DEFINES
#endif

#include "NFmiCapView.h"
#include "NFmiToolBox.h"
#include "NFmiDrawParam.h"
#include "NFmiArea.h"
#include "NFmiColorSpaces.h"
#include "NFmiSettings.h"
#include "NFmiConceptualDataView.h"
#include "CtrlViewDocumentInterface.h"
#include "GraphicalInfo.h"
#include "MapHandlerInterface.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "GdiPlusLineInfo.h"
#include "CtrlViewFunctions.h"
#include "NFmiFileSystem.h"
#include "NFmiYKJArea.h"
#include "CapDataSystem.h"
#include "CapData.h"
#include "WarningMember.h"
#include "catlog/catlog.h"
#include <cmath>
#include <math.h>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Gdiplus;
using namespace NFmiStringTools;
using namespace Warnings;

NFmiImageMap NFmiCapView::itsCapSymbolMap;

NFmiCapView::NFmiCapView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
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
    , itsRowIndex(theRowIndex)
    , itsScreenPixelSizeInMM(0)
{
    
}

//This has to be called in GeneralDataCoc before use!
void NFmiCapView::InitCapSymbolMap(const std::string &theWomlDirectory)
{
    if(!NFmiCapView::itsCapSymbolMap.Initialized())
    {
        // Get baseFolder from NFmiSettings-class, do abs.-relative path tricks
        std::string baseFolder = NFmiSettings::Optional<std::string>("SmartMet::Warnings::BaseSymbolFolder", "warning_symbols");
        baseFolder = NFmiFileSystem::MakeAbsolutePath(baseFolder, theWomlDirectory);
        // Get initFile from NFmiSettings-class, do abs.-relative path tricks
        std::string initFile = NFmiSettings::Optional<std::string>("SmartMet::Warnings::SymbolMapFile", "warningsymbolmap.txt");
        initFile = NFmiFileSystem::MakeAbsolutePath(initFile, theWomlDirectory);
        NFmiCapView::itsCapSymbolMap.Initialize(baseFolder, initFile);
    }
}

void NFmiCapView::Draw(NFmiToolBox *theGTB)
{
    if(!theGTB)
        return;
    itsToolBox = theGTB;
    if(!IsParamDrawn())
        return;

    if(itsToolBox->GetDC()->IsPrinting() == FALSE)
        itsScreenPixelSizeInMM = 1. / itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x;

    try
    {
        InitializeGdiplus(itsToolBox, &GetFrame());
        itsGdiPlusGraphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias); // NOTE. anti-aliasing might slow down drawing considerably

        //Get warning data from CapDataSystem
        auto &capDataSystem = GetCtrlViewDocumentInterface()->GetCapDataSystem();
        capData_ = capDataSystem.capdata(1, 1);
        std::vector<std::shared_ptr<WarningMember>> warnings_ = capData_.warnings();
        std::vector<std::shared_ptr<WarningMember>> validWarnings;

        //Define size for the symbols and move icons that would overlap
        float wantedSymbolSizeInMM = 10;

        //Loop through all WarningMembers
        for(std::vector<shared_ptr<WarningMember>>::iterator it = warnings_.begin(); it != warnings_.end(); ++it)
        {
            //Draw only warnings that are valid for viewed time
            if(((*it)->getEffectiveFrom() <= itsTime) && ((*it)->getEffectiveUntil() >= itsTime))
            {
                NFmiColor color = getWarningColor(*it);
                NFmiPoint warningSymbolLocation = LatLonToViewPoint((*it)->getCenter());

                std::vector<vector<NFmiPoint>> inputWarningAreas = (*it)->getWarningAreaPolygons();
                for(std::vector<vector<NFmiPoint>>::iterator i = inputWarningAreas.begin(); i != inputWarningAreas.end(); ++i)
                {
                    vector<NFmiPoint> coordinateVector = *i;
                    std::vector<Gdiplus::PointF> gdiPoints = CtrlView::ConvertLatLon2GdiPoints(this, coordinateVector);

                    //Draw warning area
                    GdiPlusLineInfo fillInfo(0.2f, color, 0);
                    CtrlView::DrawGdiplusCurve(*itsGdiPlusGraphics, gdiPoints, fillInfo, true, -1, itsCtrlViewDocumentInterface->Printing());
                    GdiPlusLineInfo lineInfo(0.2f, NFmiColor(0, 0, 0), 0);
                    CtrlView::DrawGdiplusCurve(*itsGdiPlusGraphics, gdiPoints, lineInfo, false, -1, itsCtrlViewDocumentInterface->Printing());
                }

                //Pick symbols and draw them after warning polygons
                validWarnings.push_back(*it);
            }
        }
        //Draw warning symbols
        for(std::vector<shared_ptr<WarningMember>>::iterator it = validWarnings.begin(); it != validWarnings.end(); ++it)
        {
            drawSymbol(*it, wantedSymbolSizeInMM);
        }

    }
    catch(...)
    {
        CatLog::logMessage("Unable to draw warnings (NFmiCapView::Draw).", CatLog::Severity::Error, CatLog::Category::Visualization);
        throw std::runtime_error(std::string("NFmiCapView::Draw - failed \n"));
    }
    CleanGdiplus(); // needs to be run in the end as a counter method for InitializeGdiplus.
}

std::string getSymbolCode(const std::shared_ptr<WarningMember>& warning)
{
    std::string warning_name;
    //Sea-water-height shallow/high has to be read from causes
    if(warning->getWarningContext() == "sea-water-height")
    {
        std::string cause = warning->getCauses();
        warning_name = warning->getWarningContext() + "-" + cause;
    }
    else
    {
        warning_name = warning->getWarningContext();
    }
    return warning_name;
}


//Draws mapped symbol in desired size
void NFmiCapView::drawSymbol(const std::shared_ptr<WarningMember>& warning, double wantedSymbolSizeInMM)
{
    bool printing = itsCtrlViewDocumentInterface->Printing();
    wantedSymbolSizeInMM *= NFmiConceptualDataView::CalcScreenSizeFactor(*itsCtrlViewDocumentInterface, itsMapViewDescTopIndex);
    auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
    double symbolSizeInPixels = graphicalInfo.itsPixelsPerMM_y * wantedSymbolSizeInMM;
    Gdiplus::Bitmap *symbolBitmap = NFmiCapView::itsCapSymbolMap.GetRightSizeImage(symbolSizeInPixels, printing, getSymbolCode(warning));
    
    //Draw wind icons dynamically
    if(warning->getWarningContext() == "sea-wind" || warning->getWarningContext() == "wind")
    {
        drawArrow(warning, wantedSymbolSizeInMM);
    }
    else
    {
        NFmiRect symbolRect(CalcSymbolRelativeRect(warning->getCenter(), wantedSymbolSizeInMM));
        CtrlView::DrawAnimationButton(symbolRect, symbolBitmap, itsGdiPlusGraphics, *itsToolBox, printing, itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex), 1.f, true);
    }
  
}

std::string NFmiCapView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
    std::string str = "Cap-data: ";
    auto fontColor = CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData());
    str = AddColorTagsToString(str, fontColor, true);
    auto capDataStrBaseLength = str.size();
    if(capData_.warnings().size())
    {
        //Find correct warning and display its info
        NFmiPoint latlon = ViewPointToLatLon(theRelativePoint);

        for(size_t i = 0; i < capData_.warnings().size(); i++)
        {
            WarningMember &tmpWarning = *capData_.warnings()[i];
            //vector<NFmiPoint> &latlonVec = tmpWarning.getLatLonPoints();
            std::vector<std::vector<NFmiPoint>> &latlonVectors = tmpWarning.getWarningAreaPolygons();
            for(auto area : latlonVectors) {
                if(insideArea(area, latlon) && (tmpWarning.getEffectiveFrom() <= itsTime && tmpWarning.getEffectiveUntil() >= itsTime))
                {
                    str += tmpWarning.getEffectiveFrom().LocalTime().ToStr("DD.MM. HH:mm")
                        + string(" - ") + tmpWarning.getEffectiveUntil().LocalTime().ToStr("DD.MM. HH:mm") + string(" (local time) ");
                    str += tmpWarning.getInfo() + "\n\n";
                }
            }
        }
    }
    if(str.length() < (capDataStrBaseLength + 2))
    {
        str += "No warning info";
    }

    return str;
}

//Checks whether given location is inside polygon
boolean NFmiCapView::insideArea(const std::vector<NFmiPoint> &latlonVec, const NFmiPoint &theLatlon)
{
    double minX = 180.0, maxX = -180.0, minY = 90.0, maxY = -90.0;
    size_t i, j;
    boolean isInside = false;
    for(auto loc : latlonVec)
    {
        minX = (loc.X() < minX) ? loc.X() : minX;
        maxX = (loc.X() > maxX) ? loc.X() : maxX;
        minY = (loc.Y() < minY) ? loc.Y() : minY;
        maxY = (loc.Y() > maxY) ? loc.Y() : maxY;
    }
    if(theLatlon.X() < minX || theLatlon.X() > maxX || theLatlon.Y() < minY || theLatlon.Y() > maxY) {
        return false;
    }

    for(i = 0, j = latlonVec.size() - 1; i < latlonVec.size(); j = i++)
    {
        if(((latlonVec[i].Y() > theLatlon.Y()) != (latlonVec[j].Y() > theLatlon.Y())) && (theLatlon.X() < (latlonVec[j].X() - latlonVec[i].X()) * (theLatlon.Y() - latlonVec[i].Y()) / (latlonVec[j].Y() - latlonVec[i].Y()) + latlonVec[i].X()))
            isInside = !isInside;
    }
    return isInside;
}

double NFmiCapView::distance(NFmiPoint& a, NFmiPoint& b)
{
    return sqrt(pow(a.X() - b.X(), 2.0) + pow(a.Y() - b.Y(), 2.0));
}

//Pick correct color for warning, green is default
NFmiColor NFmiCapView::getWarningColor(const std::shared_ptr<WarningMember>& warning)
{
    std::string warningLevel = warning->getSeverity();
    int level = std::stoi(warningLevel.substr(warningLevel.find("-") + 1));

    NFmiColor color;
    const float alpha = 0.2f;
    switch(level) {
    case 2: //Yellow
        color = NFmiColor(1.0f, 1.0f, 0.f, alpha);
        break;
    case 3: //Orange
        color = NFmiColor(1.0f, 0.64f, 0.f, alpha);
        break;
    case 4: //Red
        color = NFmiColor(1.0f, 0.f, 0.f, alpha);
        break;
    default: //Green
        color = NFmiColor(0.f, 1.0f, 0.f, alpha);
        break;
    }

    //Level 1 sea-wind warnings are drawn with yellow color
    std::string context = warning->getWarningContext();
    if(context == "sea-wind" && level == 1) {
        color = NFmiColor(1.0f, 1.0f, 0.f, alpha);
    }

    return color;
}


void NFmiCapView::drawArrow(const std::shared_ptr<WarningMember>& warning, double wantedSymbolSizeInMM) {
    
    Gdiplus::GraphicsPath arrowPath;
    float originalSymbolLengthInPixels;
    // Wind arrow cannot be constructed in separate functions, because Gdiplus::GraphicsPath-class'
    // copy constructor and assignment operators are protected. :(
    
    if(warning->getWarningContext() == "sea-wind") //draw sea-wind arrow
    {
        //draw circle and small arrow
        const float circleRadius = 1.1f;
        const float circleAndArrowHeadOverlap = - 0.05f; // negative overlap at the moment, looks better
        const float arrowHeadTipY = 1.0f + circleRadius - circleAndArrowHeadOverlap;
        const float arrowHeadTipX = 0.f;
        const float arrowHeadWingX = 0.6f;
        const float arrowHeadBaseY = circleRadius - circleAndArrowHeadOverlap;
        
        originalSymbolLengthInPixels = 2 * circleRadius + arrowHeadTipY - circleAndArrowHeadOverlap;

        Gdiplus::PointF tip(arrowHeadTipX, arrowHeadTipY);
        Gdiplus::PointF tipOfRightWing(arrowHeadWingX, arrowHeadBaseY);
        Gdiplus::PointF tipOfLeftWing(-arrowHeadWingX, arrowHeadBaseY);

        arrowPath.AddLine(tip, tipOfRightWing);
        arrowPath.AddLine(tipOfRightWing, tipOfLeftWing);
        arrowPath.AddLine(tipOfLeftWing, tip);
        arrowPath.AddEllipse(-circleRadius, -circleRadius, 2 * circleRadius, 2 * circleRadius);
    }
    else //draw normal arrow
    {
        const float arrowHeadTipY = 1.5f;
        const float arrowHeadTipX = 0.f;
        const float arrowHeadInnerBaseX = 0.5f;
        const float arrowHeadOuterBaseX = 0.9f;
        const float arrowHeadBaseY = 0.7f;
        const float arrowTailX = 0.5f;
        const float arrowTailY = -1.5f;

        originalSymbolLengthInPixels = arrowHeadTipY - arrowTailY;

        //Circulate points clockwise starting from the tip
        Gdiplus::PointF tip(arrowHeadTipX, arrowHeadTipY);
        Gdiplus::PointF tipOfRightWing(arrowHeadOuterBaseX, arrowHeadBaseY);
        Gdiplus::PointF baseOfRightWing(arrowHeadInnerBaseX, arrowHeadBaseY);
        Gdiplus::PointF tailRight(arrowTailX, arrowTailY);
        Gdiplus::PointF tailLeft(-arrowTailX, arrowTailY);
        Gdiplus::PointF baseOfLeftWing(-arrowHeadInnerBaseX, arrowHeadBaseY);
        Gdiplus::PointF tipOfLeftWing(-arrowHeadOuterBaseX, arrowHeadBaseY);

        arrowPath.AddLine(tip, tipOfRightWing);
        arrowPath.AddLine(tipOfRightWing, baseOfRightWing);
        arrowPath.AddLine(baseOfRightWing, tailRight);
        arrowPath.AddLine(tailRight, tailLeft);
        arrowPath.AddLine(tailLeft, baseOfLeftWing);
        arrowPath.AddLine(baseOfLeftWing, tipOfLeftWing);
        arrowPath.CloseFigure();
    }
    

    Gdiplus::PointF aPlace(CtrlView::ConvertLatlonToGdiPlusPoint(this, warning->getCenter())); //Gdiplus::PointF aPlace(CtrlView::ConvertLatlonToGdiPlusPoint(this, theData.itsSymbolLatlon));
    float angle = warning->getPhysicalDirection(); //float angle = theData.itsParameterValue2 + 180;
    NFmiAngle ang(itsArea->TrueNorthAzimuth(warning->getCenter()));
    angle += static_cast<float>(ang.Value()); // Fix north to map's north

    //Rotate symbol
    Gdiplus::Matrix rotateMatrix;
    rotateMatrix.Rotate(angle);
    arrowPath.Transform(&rotateMatrix);

    //Scale symbol
    float wantedPixelLength = static_cast<float>(wantedSymbolSizeInMM *1.1 * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x);
    float scaleFactor = wantedPixelLength / originalSymbolLengthInPixels;
    Gdiplus::Matrix scaleMatrix;
    scaleMatrix.Scale(scaleFactor, scaleFactor);
    arrowPath.Transform(&scaleMatrix);

    //Move symbol
    Gdiplus::Matrix moveMatrix;
    moveMatrix.Translate(aPlace.X, aPlace.Y);
    arrowPath.Transform(&moveMatrix);

    CtrlView::DrawPath(*itsGdiPlusGraphics, arrowPath, NFmiColor(0, 0, 0), NFmiColor(0, 0, 0), true, false, 0);
    if(warning->getPhysicalValue() != kFloatMissing)
    { // Draw speed on top of the arrow
        float usedFontSizeInPixels = NFmiConceptualDataView::CalcUsedFontSizeInPixels((float) wantedSymbolSizeInMM * 0.3f, *itsCtrlViewDocumentInterface, itsMapViewDescTopIndex, 1.f);
        Gdiplus::PointF aPlace(CtrlView::ConvertLatlonToGdiPlusPoint(this, warning->getCenter()));
        CtrlView::DrawSimpleText(*itsGdiPlusGraphics, NFmiColor(1, 1, 1), usedFontSizeInPixels, std::to_string((int)warning->getPhysicalValue()), NFmiPoint(aPlace.X, aPlace.Y), L"Arial", kCenter);
    }
}


