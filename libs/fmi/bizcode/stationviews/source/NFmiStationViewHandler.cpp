//**********************************************************
// C++ Class Name : NFmiStationViewHandler
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiStationViewHandler.cpp
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 3
//  - GD View Type      : Class Diagram
//  - GD View Name      : ruudukkon‰yttˆ
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Fri - Sep 10, 1999
//
//
//  Description:
//   T‰m‰ luokka pit‰‰ sis‰ll‰‰n kartan piirron
//   ja stationviewlistin joita se piirt‰‰.
//   T‰t‰ luokkaa on tarkoitus k‰ytt‰‰ editorin
//   'ruudukko'-n‰ytˆn palasena. N‰ist‰ ruudunpalasista
//   kootaan 'rivit' NFmiTimeStationViwRow-luokkaan ja
//   ruudukon muodostaa NFmiTimeStationViwRowList.
//   T‰t‰ viimeista pit‰‰
//   taas editorin n‰yttˆluokka omanaan.
//
//  Change Log:
//
//**********************************************************
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k‰‰nt‰j‰n varoitusta
#endif

#include "NFmiStationViewHandler.h"
#include "NFmiDrawParam.h"
#include "NFmiCtrlViewList.h"
#include "NFmiArea.h"
#include "NFmiToolBox.h"
#include "NFmiDrawParamList.h"
#include "NFmiStationView.h"
#include "NFmiIsoLineView.h"
#include "NFmiStationTextView.h"
#include "NFmiStationSimpleWeatherView.h"
#include "NFmiStationWindBarbView.h"
#include "NFmiStationIndexTextView.h"
#include "NFmiEditMapDataListHandler.h"
#include "NFmiAreaMaskList.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiLocationSelectionTool.h"
#include "NFmiText.h"
#include "NFmiRectangle.h"
#include "NFmiLine.h"
#include "NFmiProjectionCurvatureInfo.h"
#include "NFmiSatelView.h"
#include "NFmiPath.h"
#include "NFmiPolyline.h"
#include "NFmiGeoShape.h"
#include "NFmiFlashDataView.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiCrossSectionSystem.h"
#include "NFmiSynopPlotView.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiMTATempSystem.h"
#include "NFmiProducerName.h"
#include "NFmiStringTools.h"
#include "NFmiGrid.h"
#include "NFmiTrajectorySystem.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiStationArrowView.h"
#include "NFmiSoundingData.h"
#include "NFmiParamHandlerView.h"
#include "NFmiConceptualDataView.h"
#include "NFmiWindTableSystem.h"
#include "NFmiAutoComplete.h"
#include "NFmiSmartInfo.h"
#include "NFmiStreamLineView.h"
#include "NFmiBetaProductSystem.h"
#include "HakeMessage/HakeSystemConfigurations.h"
#include "HakeMessage/HakeMsg.h"
#include "CtrlViewDocumentInterface.h"
#include "MapHandlerInterface.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "GdiPlusLineInfo.h"
#include "NFmiTrajectory.h"
#include "CtrlViewFunctions.h"
#include "NFmiMapViewTimeLabelInfo.h"
#include "NFmiMapViewCache.h"
#include "NFmiFastInfoUtils.h"
#include "NFmiCapView.h"
#include "NFmiWmsView.h"
#include "NFmiDataParamModifierAreaConeChange.h"
#include "MapDrawFunctions.h"
#include "NFmiApplicationWinRegistry.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "catlog/catlog.h"
#include "NFmiVoidPtrList.h"
#include "EditedInfoMaskHandler.h"
#include "ToolBoxStateRestorer.h"
#include "NFmiColorContourLegendSettings.h"
#include "NFmiColorContourLegendValues.h"
#include "ToolMasterDrawingFunctions.h"
#include "CtrlViewColorContourLegendDrawingFunctions.h"
#include "CombinedMapHandlerInterface.h"
#include "ParamHandlerViewFunctions.h"
#include "MathHelper.h"
#include "ColorStringFunctions.h"
#include "NFmiValueString.h"

#ifndef DISABLE_CPPRESTSDK
#include "wmssupport/WmsSupport.h"
#include "wmssupport/LegendBuffer.h"
#include "wmssupport/LegendSelf.h"
#endif // DISABLE_CPPRESTSDK

#include "ApplicationInterface.h"
#include "CtrlViewWin32Functions.h"
#include "BetaProductParamBoxFunctions.h"
#include "NFmiCountryBorderDrawUtils.h"
#include "NFmiMapViewDescTop.h"

#include <list>
#include <regex>
#include "boost\math\special_functions\round.hpp"

using namespace std;


//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiStationViewHandler::NFmiStationViewHandler(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
											   ,NFmiToolBox * theToolBox
											   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
											   ,int theRowIndex
											   ,int theColumnIndex)
:NFmiCtrlView(theMapViewDescTopIndex, theArea->XYArea()
			 ,theToolBox
			 ,theDrawParam
             ,theRowIndex
             ,theColumnIndex)
,itsMapArea()
,itsViewList(0)
,itsMapDrawParam(new NFmiDrawParam)
,itsMapRect()
,itsZoomDragDownPoint()
,itsZoomDragUpPoint()
,itsOldZoomRect()
,itsParamHandlerView()
,itsParamHandlerViewRect()
,fWholeCrossSectionReallyMoved(false)
{
	NFmiRect mapRect = GetFrame();
	SetMapAreaAndRect(itsCtrlViewDocumentInterface->GetMapHandlerInterface(itsMapViewDescTopIndex)->Area(), mapRect);
	itsViewList = new NFmiCtrlViewList(itsMapViewDescTopIndex, mapRect
									  ,itsToolBox
									  ,itsMapDrawParam);
	if(itsViewGridRowNumber == 1 && itsViewGridColumnNumber == 1) // tehd‰‰n t‰m‰ alustus vain 1. rivin ensimm‰iseen ruudun luonnin yhteydess‰
		itsCtrlViewDocumentInterface->CrossSectionSystem()->CalcMinorPoints(itsMapArea); // t‰m‰ pit‰‰ tehd‰ ainakin kerran

	InitParamHandlerView();
}

NFmiStationViewHandler::~NFmiStationViewHandler()
{
	delete itsViewList;
}

static void DoBitBlt(CDC *sourceDc, CDC *destinationDc, CBitmap *usedBitmap, const CSize &sourceOffset)
{
    BITMAP bm;
    usedBitmap->GetBitmap(&bm);
    CBitmap *oldBitmap = destinationDc->SelectObject(usedBitmap);

    BOOL status = destinationDc->BitBlt(0
        , 0
        , bm.bmWidth
        , bm.bmHeight
        , sourceDc
        , sourceOffset.cx
        , sourceOffset.cy
        , SRCCOPY);
    destinationDc->SelectObject(oldBitmap);
}

static CSize CalcBitmapSizeFromFrame(NFmiToolBox * theGTB, const NFmiRect &mapFrame)
{
    int bitmapWidth = theGTB->HX(mapFrame.Width());
    int bitmapHeight = theGTB->HY(mapFrame.Height());
    return CSize(bitmapWidth, bitmapHeight);
}

static CSize CalcSourceOffsetFromFrame(NFmiToolBox * theGTB, const NFmiRect &mapFrame)
{
    int sourceX = theGTB->HX(mapFrame.Left());
    int sourceY = theGTB->HY(mapFrame.Top());
    return CSize(sourceX, sourceY);
}

void NFmiStationViewHandler::DoBasicDrawing(NFmiToolBox * theGTB, const NFmiRect &theMapFrame)
{
    DrawMap(theGTB, theMapFrame); // synop-plot printtausta varten pit‰‰ tehd‰ oma muuttuja, jolloin ei karttaa piirret‰
    DrawData(theGTB);
    DrawOverMap(theGTB, theMapFrame); // piirret‰‰n haluttaessa kartan ja datan p‰‰lle l‰pin‰kyv‰ esim. kaupunkien nimet/tiestˆ kartta
    DrawSilamStationMarkers();
    DrawTrajectories();
    DrawHALYMessageMarkers();
    DrawWindTableAreas();
}

void NFmiStationViewHandler::DoCacheDrawing(NFmiToolBox * theGTB, const NFmiRect &theMapFrame, CDC *destinationDc, int cacheRowIndex)
{
    CDC* sourceDC = theGTB->GetDC();

    // Talletetaan piirretty ruutu cachea varten, jos cache kaytossa.
    CSize bitmapSize = ::CalcBitmapSizeFromFrame(theGTB, theMapFrame);
    CSize sourceOffset = ::CalcSourceOffsetFromFrame(theGTB, theMapFrame);
    CBitmap *viewCache = new CBitmap;
    BOOL status = viewCache->CreateCompatibleBitmap(sourceDC, bitmapSize.cx, bitmapSize.cy); // HUOM! pit‰‰ olla sourceDC, eik‰ destDC!
    if(status)
    {
        ::DoBitBlt(sourceDC, destinationDc, viewCache, sourceOffset);
        itsCtrlViewDocumentInterface->MapViewCache(itsMapViewDescTopIndex).MapImage(itsTime, cacheRowIndex, viewCache);
    }
    else
    {
        static bool firstTime = true;
        viewCache->DeleteObject();  // en tied‰ pit‰‰kˆ ep‰onnistuneen luonnin j‰lkeen tuhota objectia
        delete viewCache;
        if(firstTime)
        {
            firstTime = false;
            std::string errorString = __FUNCTION__;
            errorString += " - cache bitmap (for map view grid section) creation failed 1. time. Size in pixels: ";
            errorString += std::to_string(bitmapSize.cx);
            errorString += "x";
            errorString += std::to_string(bitmapSize.cy);
            itsCtrlViewDocumentInterface->LogAndWarnUser(errorString, "", CatLog::Severity::Error, CatLog::Category::Visualization, true);
        }
    }
}

// Lasketaan eri karttan‰yttˆmoodeissa t‰m‰n olion k‰ytt‰m‰ cache rivi.
// cacheRowIndex luvut alkavat 0:sta ja menev‰t 49:‰‰n.
// itsViewGridRowNumber kertoo kuinka monennella n‰yttˆruudukossa n‰kyv‰ll‰ kartta rivill‰ 
// ollaan (ylin n‰kyv‰ on 1 jne., max 5 rivi‰ kerrallaan eli luvut 1-5).
// mapRowStartingIndex on karttaruudukon ylimm‰n rivin oikea rivinumero, joka alkaa 1:st‰ ja menee 50:een.
int NFmiStationViewHandler::CalcUsedCacheRowIndex() const
{
    CtrlViewUtils::MapViewMode displayMode = itsCtrlViewDocumentInterface->MapViewDisplayMode(itsMapViewDescTopIndex);
    int mapRowStartingIndex = itsCtrlViewDocumentInterface->MapRowStartingIndex(itsMapViewDescTopIndex);
    int cacheRowIndex = 0;
    if(displayMode == CtrlViewUtils::MapViewMode::kNormal)
        cacheRowIndex = mapRowStartingIndex + itsViewGridRowNumber - 2;
    else if(displayMode == CtrlViewUtils::MapViewMode::kOneTime)
    {
        int nthFrameInViewGrid = (itsViewGridRowNumber - 1) * boost::math::iround(itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).X()) + itsViewGridColumnNumber;
        cacheRowIndex = (mapRowStartingIndex - 1) + nthFrameInViewGrid - 1;
    }
    else if(displayMode == CtrlViewUtils::MapViewMode::kRunningTime)
        cacheRowIndex = mapRowStartingIndex - 1;

    return cacheRowIndex;
}

void NFmiStationViewHandler::TraceLogValidTimeAndAbsoluteRow()
{
    if(CatLog::doTraceLevelLogging())
    {
        auto absoluteRowIndex = CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
        std::string message = CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeCurrentUpdateIdString() + " NFmiStationViewHandler::Draw: validTime = ";
        message += itsTime.ToStr("YYYY.MM.DD HH:mm UTC", kEnglish);
        message += ", absolute row index = ";
        message += std::to_string(absoluteRowIndex);
        CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Visualization, true);
    }
}

//--------------------------------------------------------
// Draw
//--------------------------------------------------------
void NFmiStationViewHandler::Draw(NFmiToolBox * theGTB)
{
	static int counter = 0;
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    TraceLogValidTimeAndAbsoluteRow();
	counter++;
	itsToolBox = theGTB;
	// cachessa rivi-indeksit alkavat 0:sta
    int cacheRowIndex = CalcUsedCacheRowIndex();
	CDC* destDC = itsCtrlViewDocumentInterface->CopyCDC(itsMapViewDescTopIndex);
	NFmiRect frame(GetFrame());
	CBitmap *viewCache = 0;
    bool useDrawingCache = UseDrawingCache();
    if(useDrawingCache)
        viewCache = itsCtrlViewDocumentInterface->MapViewCache(itsMapViewDescTopIndex).MapImage(itsTime, cacheRowIndex);

	if(destDC)
	{
		if(!viewCache)
		{ // T‰t‰ ruutua ei lˆydy cachesta, pit‰‰ piirt‰‰.
			try
			{
                CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + ": have to draw all data", this);
                InitializeGdiplus(itsToolBox, &GetFrame());
                DoBasicDrawing(theGTB, frame);

                if(useDrawingCache)
                {
                    DoCacheDrawing(theGTB, frame, destDC, cacheRowIndex);
                }
            }
            catch(std::exception &e)
            {
                itsCtrlViewDocumentInterface->LogAndWarnUser(std::string("Error in NFmiStationViewHandler::Draw: ") + e.what(), "", CatLog::Severity::Error, CatLog::Category::Visualization, true);
            }
            catch(...)
			{
			}
			CleanGdiplus(); // t‰t‰ pit‰‰ kutsua piirron lopuksi InitializeGdiplus -metodin vastin parina.
		}
		else
		{ // Piirret‰‰n karttaruutu suoraan cachesta, nopeaa.
            CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + ": using cache image to draw (faster)", this);
            CBitmap* oldBitmap = destDC->SelectObject(viewCache);
			theGTB->DrawDC(destDC, frame);
			destDC->SelectObject(oldBitmap);
        }
	}
}

// Piirto cachea ei k‰ytet‰:
// 1. Kun ollaan printtaamassa
// 2. Kun ollaan piirt‰m‰ss‰ Beta product moodissa
bool NFmiStationViewHandler::UseDrawingCache()
{
    if(itsCtrlViewDocumentInterface->Printing() || itsCtrlViewDocumentInterface->BetaProductGenerationRunning())
        return false;
    else
        return itsCtrlViewDocumentInterface->MapViewCache(itsMapViewDescTopIndex).IsCacheUsed();
}

NFmiRect NFmiStationViewHandler::CalcCPCropAreasRelativeRect(const boost::shared_ptr<NFmiArea> &theArea)
{
	NFmiRect relativeRect;
	if(theArea)
	{
		NFmiPoint bottomLeftRelativePoint = LatLonToViewPoint(theArea->BottomLeftLatLon());
		NFmiPoint topRightRelativePoint = LatLonToViewPoint(theArea->TopRightLatLon());
		relativeRect = NFmiRect(bottomLeftRelativePoint, topRightRelativePoint);
	}
	return relativeRect;
}

void NFmiStationViewHandler::DrawCPCropArea(void)
{
	if(itsCtrlViewDocumentInterface->IsCPGridCropInAction())
	{
		if(itsViewGridRowNumber == 1 && itsViewGridColumnNumber == itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).X())
		{
			NFmiColor frameColor(0.9f, 0, 0.8f);
			NFmiColor fillColor;
			// piirret‰‰n uloimpi alue
			NFmiRect relativeRect = CalcCPCropAreasRelativeRect(itsCtrlViewDocumentInterface->CPGridCropLatlonArea());
			Gdiplus::Rect rectInPixels = CtrlView::Relative2GdiplusRect(itsToolBox, relativeRect);
            CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, frameColor, fillColor, false, true, 2, Gdiplus::DashStyleDash);
			if(itsCtrlViewDocumentInterface->IsCPGridCropNotPlausible())
			{ // CP croppaus on k‰ytˆss‰, mutta ei mahdollista, piirr‰ varoittava punainen rasti kartalle
				NFmiColor crossColor(1, 0, 0);
                CtrlView::DrawLine(*itsGdiPlusGraphics, rectInPixels.X, rectInPixels.Y, rectInPixels.X + rectInPixels.Width, rectInPixels.Y + rectInPixels.Height, crossColor, 2, Gdiplus::DashStyleDash);
                CtrlView::DrawLine(*itsGdiPlusGraphics, rectInPixels.X, rectInPixels.Y + rectInPixels.Height, rectInPixels.X + rectInPixels.Width, rectInPixels.Y, crossColor, 2, Gdiplus::DashStyleDash);
			}
			else
			{
				// piirret‰‰n sisempi alue
				relativeRect = CalcCPCropAreasRelativeRect(itsCtrlViewDocumentInterface->CPGridCropInnerLatlonArea());
				rectInPixels = CtrlView::Relative2GdiplusRect(itsToolBox, relativeRect);
                CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, frameColor, fillColor, false, true, 2, Gdiplus::DashStyleDash);
			}
		}
	}
}

void NFmiStationViewHandler::DrawAreaMask(Gdiplus::Graphics &theGdiPlusGraphics, NFmiWindTableSystem::AreaMaskData &theAreaMaskData)
{
	NFmiSvgPath &aPath = theAreaMaskData.SvgPath();
	float lineWidthInPixels = 1;
	int lineStyle = 0; // 0=yhten‰inen viiva
	GdiPlusLineInfo lineInfo(lineWidthInPixels, NFmiColor(0, 0, 0), lineStyle);
	int fillHatchStyle = -1; // -1 tarkoittaa ett‰ fillauksen yhteydess‰ ei k‰ytet‰ hatchi‰
	lineInfo.SetSmoothingMode(Gdiplus::SmoothingModeNone);

	NFmiSvgPath::storage_type &pathElements = aPath.GetData();
	std::vector<Gdiplus::PointF> xyPoints;

	bool firstPointFound = false;
	NFmiPoint firstLatLon;
	for(size_t i = 0; i < pathElements.size(); i++)
	{
		if(pathElements[i].itsType == NFmiSvgPath::kElementLineto)
		{
			if(firstPointFound == false)
			{
				firstLatLon = NFmiPoint(pathElements[i].itsX, pathElements[i].itsY);
				firstPointFound = true;
			}
			xyPoints.push_back(CtrlView::ConvertLatlonToGdiPlusPoint(this, NFmiPoint(pathElements[i].itsX, pathElements[i].itsY)));
		}
		else if(pathElements[i].itsType == NFmiSvgPath::kElementMoveto)
		{
            CtrlView::DrawGdiplusCurve(theGdiPlusGraphics, xyPoints, lineInfo, false, fillHatchStyle, itsCtrlViewDocumentInterface->Printing());
			xyPoints.clear();
		}
		else if(pathElements[i].itsType == NFmiSvgPath::kElementClosePath)
		{
			if(xyPoints.size() > 0)
				xyPoints.push_back(xyPoints[0]);
            CtrlView::DrawGdiplusCurve(theGdiPlusGraphics, xyPoints, lineInfo, false, fillHatchStyle, itsCtrlViewDocumentInterface->Printing());
			xyPoints.clear();
		}
	}

	// piirret‰‰n maski alueen nimi viel‰ 1. pisteen kohdalle
	if(firstPointFound)
	{
		double pixelsPerMM = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y;
        CtrlView::DrawTextToRelativeLocation(theGdiPlusGraphics, NFmiColor(0,0,1), 4.5, theAreaMaskData.Name(), LatLonToViewPoint(firstLatLon), pixelsPerMM, itsToolBox, L"arial", kCenter);
	}

}

void NFmiStationViewHandler::DrawWindTableAreas(void)
{
    auto &windTableSystem = itsCtrlViewDocumentInterface->WindTableSystem();
	if(windTableSystem.ViewVisible())
	{
		auto  &areaMaskDataList = windTableSystem.AreaMaskDataList();
		for(size_t i = 0; i < areaMaskDataList.size(); i++)
		{
			DrawAreaMask(*itsGdiPlusGraphics, areaMaskDataList[i]);
		}
	}
}

bool NFmiStationViewHandler::IsCrossSectionViewStuffShownOnThisMapView() const
{
	auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
	return crossSectionSystem->CrossSectionViewOn() && crossSectionSystem->CrossSectionSystemActive() && itsCtrlViewDocumentInterface->ShowCrossSectionMarkersOnMap(itsMapViewDescTopIndex);
}

// piirret‰‰n kartalle (jos ollaan oikeassa moodissa) poikkileikkauksen
// 'reitti' pallukat
void NFmiStationViewHandler::DrawCrossSectionPoints(void)
{
	if(IsCrossSectionViewStuffShownOnThisMapView())
	{
	    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
		if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
			return ; // ei piirret‰ n‰it‰ kun ollaan poikkileikkaus trajektori moodissa

		if(itsViewGridRowNumber == 1 && itsViewGridColumnNumber == 1) // vain 1. rivin ensimm‰iseen ruutuun piirto
		{
			bool drawWholeLine = (crossSectionSystem->GetCrossMode() != NFmiCrossSectionSystem::kTime) && (crossSectionSystem->GetCrossMode() != NFmiCrossSectionSystem::kObsAndFor);
			double smallCircleSizeInMM = 1.2;
            auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
			int pixelSizeX = static_cast<int>(smallCircleSizeInMM * graphicalInfo.itsPixelsPerMM_x);
			int pixelSizeY = static_cast<int>(smallCircleSizeInMM * graphicalInfo.itsPixelsPerMM_y);
			double xWidth = itsToolBox->SX(pixelSizeX);
			double yWidth = itsToolBox->SY(pixelSizeY);
			NFmiRect littleCircleRect(0, 0, xWidth, yWidth);
			if(drawWholeLine)
			{
				itsDrawingEnvironment.SetFillColor(NFmiColor(0.9f,0.9f,0.9f));
				itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
                crossSectionSystem->CalcMinorPoints(itsMapArea);
				const auto &points = crossSectionSystem->MinorPoints();
				for(unsigned int i=0 ; i < points.size(); i++)
				{
					littleCircleRect.Center(itsMapArea->ToXY(points[i]));
					itsToolBox->DrawEllipse(littleCircleRect, &itsDrawingEnvironment);
				}
			}
			NFmiPoint startPointXY = itsMapArea->ToXY(crossSectionSystem->StartPoint());
			NFmiPoint middlePointXY = itsMapArea->ToXY(crossSectionSystem->MiddlePoint());
			NFmiPoint endPointXY = itsMapArea->ToXY(crossSectionSystem->EndPoint());
			itsDrawingEnvironment.SetFillColor(crossSectionSystem->StartPointFillColor());
			itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,1.f));
			itsDrawingEnvironment.EnableFill();

			double bigCircleSizeInMM = 2.8;
            int pixelSizeX2 = static_cast<int>(bigCircleSizeInMM * graphicalInfo.itsPixelsPerMM_x);
			int pixelSizeY2 = static_cast<int>(bigCircleSizeInMM * graphicalInfo.itsPixelsPerMM_y);
			double xWidth2 = itsToolBox->SX(pixelSizeX2);
			double yWidth2 = itsToolBox->SY(pixelSizeY2);
			NFmiRect circleRect(0, 0, xWidth2, yWidth2);
			circleRect.Center(startPointXY);
			itsToolBox->DrawEllipse(circleRect, &itsDrawingEnvironment); // piirret‰‰n alkupisteen v‰ripallo
			NFmiPoint fontSize(20, 20);
			itsDrawingEnvironment.SetFontSize(fontSize);
			NFmiPoint textPoint1(startPointXY);
			textPoint1 += NFmiPoint(textPoint1.X() > 0.95 ? -0.03 : 0.02, textPoint1.Y() > 0.95 ? -0.03 : 0.02);
			itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
			NFmiText text1(textPoint1, NFmiString("1."), false, 0, &itsDrawingEnvironment);
			itsToolBox->Convert(&text1);

			if(drawWholeLine)
			{
				circleRect.Center(endPointXY);
				itsDrawingEnvironment.SetFillColor(crossSectionSystem->EndPointFillColor());
				itsToolBox->DrawEllipse(circleRect, &itsDrawingEnvironment); // piirret‰‰n loppupisteen v‰ripallo
				NFmiPoint textPoint2(endPointXY);
				textPoint2 += NFmiPoint(textPoint2.X() > 0.95 ? -0.03 : 0.02, textPoint2.Y() > 0.95 ? -0.03 : 0.02);
				itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
				NFmiText text2(textPoint2, NFmiString("2."), false, 0, &itsDrawingEnvironment);
				itsToolBox->Convert(&text2);
				if(crossSectionSystem->CrossSectionMode() == NFmiCrossSectionSystem::k3Point)
				{ // piirret‰‰ viel‰ keski piste v‰ri pallolla
					circleRect.Center(middlePointXY);
					itsDrawingEnvironment.SetFillColor(crossSectionSystem->MiddlePointFillColor());
					itsToolBox->DrawEllipse(circleRect, &itsDrawingEnvironment); // piirret‰‰n loppupisteen v‰ripallo
					NFmiPoint textPoint3(middlePointXY);
					textPoint2 += NFmiPoint(textPoint2.X() > 0.95 ? -0.03 : 0.02, textPoint2.Y() > 0.95 ? -0.03 : 0.02);
					itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
					NFmiText text3(textPoint3, NFmiString("M."), false, 0, &itsDrawingEnvironment);
					itsToolBox->Convert(&text3);
				}

				// piirret‰‰n lopuksi viel‰ mahd. aktivoitu minorpoint
				if(crossSectionSystem->IsMinorPointActivated())
				{
					itsDrawingEnvironment.SetFillColor(NFmiColor(0.f,0.f,0.f));
					littleCircleRect.Center(itsMapArea->ToXY(crossSectionSystem->ActivatedMinorPoint()));
					itsToolBox->DrawEllipse(littleCircleRect, &itsDrawingEnvironment);
				}
			}
		}
	}
}

void NFmiStationViewHandler::DrawTrajectories(void)
{
    auto trajectorySystem = itsCtrlViewDocumentInterface->TrajectorySystem();
	if(trajectorySystem->TrajectoryViewOn() && itsCtrlViewDocumentInterface->ShowTrajectorsOnMap(itsMapViewDescTopIndex))
	{
		// piirret‰‰n trajektorit kartalle
		int index = 0;
		const std::vector<boost::shared_ptr<NFmiTrajectory> >& trajectories = trajectorySystem->Trajectories();
		std::vector<boost::shared_ptr<NFmiTrajectory> >::const_iterator it = trajectories.begin();
		for( ; it != trajectories.end() ; ++it)
		{
			DrawTrajectory(*(*it).get(), itsCtrlViewDocumentInterface->GeneralColor(index)); // t‰ss‰ vaiheessa otetaan viel‰ luotauksista v‰ritykset
			index++;
		}

		// Piirr‰ hiirell‰ valittu piste kartalle
		NFmiDrawingEnvironment envi;
        auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
        long pixelSize1 = boost::math::iround(graphicalInfo.itsPixelsPerMM_x * 3.0);
		double x = itsToolBox->SX(pixelSize1);
		double y = itsToolBox->SY(pixelSize1);
		NFmiRect rec(0,0,x,y);
		rec.Center(LatLonToViewPoint(trajectorySystem->SelectedLatLon()));
		itsToolBox->DrawEllipse(rec, &envi);
        long pixelSize2 = boost::math::iround(graphicalInfo.itsPixelsPerMM_x * 0.8);
		double ticX = itsToolBox->SX(pixelSize2);
		double ticY = itsToolBox->SY(pixelSize2);
		NFmiPoint center(rec.Center());
		NFmiLine line1(NFmiPoint(rec.Left(), center.Y()), NFmiPoint(rec.Left()+ticX, center.Y()), 0, &envi);
		itsToolBox->Convert(&line1);
		NFmiLine line2(NFmiPoint(rec.Right(), center.Y()), NFmiPoint(rec.Right()-ticX, center.Y()), 0, &envi);
		itsToolBox->Convert(&line2);
		NFmiLine line3(NFmiPoint(center.X(), rec.Top()), NFmiPoint(center.X(), rec.Top()+ticY), 0, &envi);
		itsToolBox->Convert(&line3);
		NFmiLine line4(NFmiPoint(center.X(), rec.Bottom()), NFmiPoint(center.X(), rec.Bottom()-ticY), 0, &envi);
		itsToolBox->Convert(&line4);
	}
}

// v‰ri otetaan v‰liaikaisesti ulkoa luotaus systeemist‰
void NFmiStationViewHandler::DrawTrajectory(const NFmiTrajectory &theTrajectory, const NFmiColor &theColor)
{
	NFmiDrawingEnvironment envi;

    auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
    long pixelSize1 = boost::math::iround(graphicalInfo.itsPixelsPerMM_x * 0.25);
	// piirret‰‰n ensin mahdollinen pluumi
	if(theTrajectory.PlumesUsed())
	{
		NFmiColor grayColor(0.9f, 0.9f, 0.9f);
		NFmiColor fadeColor(theColor);
		fadeColor.Mix(grayColor, 0.62f);  // haalennetaan v‰ri‰ hiukan
		envi.SetFrameColor(fadeColor);
		envi.SetPenSize(NFmiPoint(pixelSize1, pixelSize1));

		const std::vector<boost::shared_ptr<NFmiSingleTrajector> >& plumes = theTrajectory.PlumeTrajectories();
		std::vector<boost::shared_ptr<NFmiSingleTrajector> >::const_iterator it = plumes.begin();
		for( ; it != plumes.end(); ++it)
			DrawSingleTrajector(*(*it).get(), envi, theTrajectory.TimeStepInMinutes(), pixelSize1 * 5, pixelSize1, theTrajectory.Direction());
	}


	// piirret‰‰n sitten p‰‰-trajektori
	envi.SetFrameColor(theColor);
    long pixelSize2 = boost::math::iround(graphicalInfo.itsPixelsPerMM_x * 0.8);
	envi.SetPenSize(NFmiPoint(pixelSize2, pixelSize2));
	DrawSingleTrajector(theTrajectory.MainTrajector(), envi, theTrajectory.TimeStepInMinutes(), pixelSize1 * 8, pixelSize1 * 2, theTrajectory.Direction());
}

static bool IsPointOk(const NFmiPoint &thePoint)
{
	if(thePoint.X() == kFloatMissing || thePoint.Y() == kFloatMissing)
		return false;
	return true;
}

void NFmiStationViewHandler::DrawSingleTrajector(const NFmiSingleTrajector &theSingleTrajector, NFmiDrawingEnvironment &theEnvi, int theTimeStepInMinutes, int theTimeMarkerPixelSize, int theTimeMarkerPixelPenSize, FmiDirection theDirection)
{
	NFmiPoint latlon1(theSingleTrajector.StartLatLon());
	if(IsPointOk(latlon1))
	{
		bool forwardDir = (theDirection == kForward);
		NFmiPoint latlon2;
		NFmiPoint p1(LatLonToViewPoint(latlon1));
		NFmiPoint p2;
		NFmiPoint mapTimeP(kFloatMissing, kFloatMissing); // haetaan t‰h‰n se piste, miss‰ trajektori oli menossa kun t‰m‰n kartan aika oli
		NFmiMetTime currentTime(theSingleTrajector.StartTime()); // pidet‰‰n laskua currentin pisteen ajasta
		const std::vector<NFmiPoint> &points = theSingleTrajector.Points();
		std::vector<NFmiPoint>::const_iterator it = points.begin();
        auto trajectorySystem = itsCtrlViewDocumentInterface->TrajectorySystem();
        bool showTrajectoryArrows = trajectorySystem->ShowTrajectoryArrows();
		bool showTrajectoryAnimationMarkers = trajectorySystem->ShowTrajectoryAnimationMarkers();
		if(showTrajectoryArrows == false && showTrajectoryAnimationMarkers == false)
			return ;
		if(it != points.end())
		{
			NFmiPolyline trajectorPolyLine(itsRect, 0, &theEnvi);
			trajectorPolyLine.AddPoint(p1);
			std::vector<NFmiPoint>::const_iterator endIt = points.end();
			++it; // pit‰‰ juoksuttaa yhden pyk‰l‰n verran eteenp‰in
			for( ; it != endIt; ++it )
			{
				latlon2 = *it;
				if(!IsPointOk(latlon2))
					break; // lopetetaan heti kun on ei ok piste
				p2 = LatLonToViewPoint(latlon2);

				if(currentTime == itsTime)
					mapTimeP = p1;

				currentTime.ChangeByMinutes(forwardDir ? theTimeStepInMinutes : -theTimeStepInMinutes);

				if(showTrajectoryArrows)
				{
					trajectorPolyLine.AddPoint(p2);
				}

				p1 = p2;
			}
			if(showTrajectoryArrows)
			{
				NFmiPoint scale(1, 1);
				NFmiPoint offset(0, 0);
				itsToolBox->DrawPolyline(&trajectorPolyLine, offset, scale);
			}

			if(showTrajectoryArrows)
			{
				if(points.size() >= 2)
				{
					long arrowHeadSize = theTimeMarkerPixelSize;
					double markWidth = itsToolBox->SX(arrowHeadSize);
					double markHeight = itsToolBox->SY(arrowHeadSize);
					NFmiPoint scale(markWidth, markHeight);

					NFmiLocation loc1(points[points.size()-2]); // 2. viimeinen paikka
					NFmiPoint lastPoint(points[points.size()-1]);
					double vdir1 = loc1.Direction(lastPoint); // ja siit‰ suunta viimeiseen paikkaan
					// tehd‰‰n pohjois korjaus nuolen k‰rjen piirtoon
					NFmiAngle ang(itsMapArea->TrueNorthAzimuth(lastPoint));
					vdir1 += static_cast<float>(ang.Value());

					if(!forwardDir)
						vdir1 = ::fmod(vdir1+180, 360); // k‰‰nnet‰‰n nuolen suunta 180 astetta jos takaperin trajektori

					// piirr‰ etenemis nuolen k‰rki trajektorille
					NFmiPolyline arrowPolyLine(itsRect, 0, &theEnvi);
					arrowPolyLine.AddPoint(::RotatePoint(NFmiPoint(-0.7, 2), vdir1));
					arrowPolyLine.AddPoint(::RotatePoint(NFmiPoint(0, 0), vdir1));
					arrowPolyLine.AddPoint(::RotatePoint(NFmiPoint(0.7, 2), vdir1));
					itsToolBox->DrawPolyline(&arrowPolyLine, p2, scale);
				}
			}

			if(showTrajectoryAnimationMarkers)
			{
				if(mapTimeP.X() != kFloatMissing)
				{
					NFmiDrawingEnvironment envi;
					envi.SetFrameColor(NFmiColor(0,0,0)); // mustalla ajan merkkaus
					envi.SetPenSize(NFmiPoint(theTimeMarkerPixelPenSize, theTimeMarkerPixelPenSize));
					double w = itsToolBox->SX(theTimeMarkerPixelSize);
					double h = itsToolBox->SY(theTimeMarkerPixelSize);
					NFmiRect rec(0,0,w,h);
					rec.Center(mapTimeP);
					itsToolBox->DrawEllipse(rec, &envi);
				}
			}
		}
	}
}

// Piirt‰‰ ns. MTA moodissa valittujen luotausten paikat omilla v‰reill‰‰n.
// N‰m‰ piirret‰‰n hieman isommalla kolmioilla luotaus asemien alle.
// N‰m‰ pirret‰‰n kaikille karttariveille, koska n‰it‰ symboleja on niin v‰h‰n
// ja valitun luotauksen sijainti kiinnostaa k‰ytt‰ji‰ huomattavasti enemm‰n.
void NFmiStationViewHandler::DrawSelectedMTAModeSoundingPlaces(void)
{
    if(!IsSoundingMarkersDrawnOnThisMap(true))
        return ;

    NFmiMTATempSystem &mtaSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    bool soundingViewTimeLock = mtaSystem.GetSoundingViewSettingsFromWindowsRegisty().SoundingTimeLockWithMapView();
	const NFmiMTATempSystem::Container &temps = mtaSystem.GetTemps();
	if(temps.size() > 0)
	{
		NFmiDrawingEnvironment envi;
		// piirret‰‰n oranssi kolmia osoittamaan asemien paikkaa
		envi.EnableFill();
		envi.SetFillColor(NFmiColor(1.f, 0.35f, 0.f));
		envi.EnableFrame();
		envi.SetFrameColor(NFmiColor(0.f, 0.f, 0.f));

		NFmiPolyline markerPolyLine(itsRect, 0, &envi);
			// n‰m‰ pisteet on otettu Raimon salama2.eps tiedostosta
		markerPolyLine.AddPoint(NFmiPoint(-1, 1));
		markerPolyLine.AddPoint(NFmiPoint(0, -1));
		markerPolyLine.AddPoint(NFmiPoint(1, 1));

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsRect);

        auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
        long pixelSizeX = boost::math::iround(graphicalInfo.itsPixelsPerMM_x * 2.0);
        long pixelSizeY = boost::math::iround(graphicalInfo.itsPixelsPerMM_y * 2.0);
		double scaleValueX = itsToolBox->SX(pixelSizeX);
		double scaleValueY = itsToolBox->SY(pixelSizeY);
		NFmiPoint scale(scaleValueX, scaleValueY);
		NFmiMTATempSystem::Container::const_iterator it = temps.begin();
		int index = 0;
		for( ; it != temps.end(); ++it)
		{
            // jos luotaus-aikalukitus p‰‰ll‰ ja aika oli sama kuin aktiivisen karttan‰ytˆn aika, TAI jos ollaan normaali moodissa, oliko luotaus olion aika ja kartan aika samat
            if((soundingViewTimeLock && itsTime == itsCtrlViewDocumentInterface->ActiveMapTime()) || (!soundingViewTimeLock && itsTime == (*it).Time()))
			{
				NFmiPoint viewPoint(LatLonToViewPoint((*it).Latlon())); // t‰m‰ on offset
				markerPolyLine.GetEnvironment()->SetFillColor(mtaSystem.SoundingColor(index));
				itsToolBox->DrawPolyline(&markerPolyLine, viewPoint, scale);

			}
			index++;
		}
	}
}

bool NFmiStationViewHandler::IsSoundingMarkersDrawnOnThisMap(bool fDrawThisOnEveryRow)
{
    auto &mtaSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(mtaSystem.TempViewOn() && mtaSystem.ShowMapMarkers() && itsCtrlViewDocumentInterface->ShowSoundingMarkersOnMap(itsMapViewDescTopIndex))
    {
        if(fDrawThisOnEveryRow)
            return true; // K‰ytt‰j‰n valitsemat luotaukset piirret‰‰n joka riville

        CtrlViewUtils::MapViewMode mapViewDisplayMode = itsCtrlViewDocumentInterface->MapViewDisplayMode(itsMapViewDescTopIndex);
        // Riippuen karttan‰ytˆss‰ k‰ytetyst‰ aikamoodista (mapViewDisplayMode):
        // 0. Normaali moodi -> piirret‰‰n vain alimmalle n‰yttˆruudukon riville (jokaisessa sarakkeessa sama aika)
        // 1. One-time moodi -> piirret‰‰n vain alimman n‰yttˆruudukon rivin oikeaan kulmaan
        // 2. Running-time moodi -> piirret‰‰n kaikille n‰yttˆruudukon ruuduille (kaikilla eri aika)
        auto &viewGridSize = itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex);
        if(mapViewDisplayMode == CtrlViewUtils::MapViewMode::kNormal)
        {
	        if(itsViewGridRowNumber == viewGridSize.Y())
	            return true;
        }
        else if(mapViewDisplayMode == CtrlViewUtils::MapViewMode::kOneTime)
        {
            if(itsViewGridRowNumber == viewGridSize.Y() && itsViewGridColumnNumber == viewGridSize.X())
	            return true;
        }
        else if(mapViewDisplayMode == CtrlViewUtils::MapViewMode::kRunningTime)
        {
	        return true;
        }
    }

    return false;
}

// piirt‰‰ havaittujen luotausten paikat kartalle, jos on havainto aika
// ja halutaan n‰ytt‰‰ kyseisi‰ luotauksia luotaus n‰ytˆss‰ (ruksi p‰‰ll‰).
void NFmiStationViewHandler::DrawSoundingPlaces(void)
{
    if(!IsSoundingMarkersDrawnOnThisMap(false))
        return ;

	// Piirret‰‰n ensin kaikki extra sounding data paikat
	std::vector<NFmiProducer> &extraSoundingProds = itsCtrlViewDocumentInterface->ExtraSoundingProducerList();
	for(size_t i = 0; i<extraSoundingProds.size(); i++)
	{
        const NFmiProducer &prod = extraSoundingProds[i];
		boost::shared_ptr<NFmiFastQueryInfo> soundingInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->FindSoundingInfo(prod);
        if(prod.GetIdent() == kFmiMAST)
    		DrawSoundingSymbols(soundingInfo, 4, 1.3); // 4=piirr‰ kolmio v‰‰rinp‰in mastodatalle
        else
    		DrawSoundingSymbols(soundingInfo, 3, 0.9); // 3=piirr‰ neliˆ
	}

	// Sitten piirret‰‰n TEMP purusta saadut mahdolliset luotaus merkit kartalle
	NFmiProducer TEMPProd(kFmiRAWTEMP); // TEMP purusta saadut luotaukset ovat t‰ll‰ tuottajalla
	boost::shared_ptr<NFmiFastQueryInfo> TEMPInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kTEMPCodeSoundingData, TEMPProd, false);
	DrawSoundingSymbols(TEMPInfo, 2, 1.7); // 2=piirr‰ salmiakki

	// lopuksi piirret‰‰ muiden p‰‰lle 'oikeat' luotaukset
    DrawSoundingSymbols(itsCtrlViewDocumentInterface->InfoOrganizer()->GetPrioritizedSoundingInfo(NFmiInfoOrganizer::ParamCheckFlags(true)), 1, 1.4); // 1=piirr‰ kolmio
}

static void SetSoundingSymbolEnvi(NFmiDrawingEnvironment &envi)
{
	// piirret‰‰n vihre‰ kolmia osoittamaan asemien paikkaa
	envi.EnableFill();
	envi.SetFillColor(NFmiColor(0.551f, 0.9f, 0.21f));
	envi.EnableFrame();
	envi.SetFrameColor(NFmiColor(0.f, 0.f, 0.f));
}

static void MakesoundingSymbolMarker(NFmiPolyline &markerPolyLine, int theUsedSymbol)
{
	if(theUsedSymbol == 1)
	{ // piirr‰ kolmio
		markerPolyLine.AddPoint(NFmiPoint(-1, 1));
		markerPolyLine.AddPoint(NFmiPoint(0, -1));
		markerPolyLine.AddPoint(NFmiPoint(1, 1));
	}
	else if(theUsedSymbol == 2)
	{ // piirr‰ salmiakki
		markerPolyLine.AddPoint(NFmiPoint(0, 1));
		markerPolyLine.AddPoint(NFmiPoint(-0.7, 0));
		markerPolyLine.AddPoint(NFmiPoint(0, -1));
		markerPolyLine.AddPoint(NFmiPoint(0.7, 0));
	}
	else if(theUsedSymbol == 3)
	{ // piirr‰ neliˆ
		markerPolyLine.AddPoint(NFmiPoint(-1, -1));
		markerPolyLine.AddPoint(NFmiPoint(1, -1));
		markerPolyLine.AddPoint(NFmiPoint(1, 1));
		markerPolyLine.AddPoint(NFmiPoint(-1, 1));
	}
	else
	{ // piirr‰ ter‰v‰ kolmio
		markerPolyLine.AddPoint(NFmiPoint(-0.6, 1));
		markerPolyLine.AddPoint(NFmiPoint(0, -1));
		markerPolyLine.AddPoint(NFmiPoint(0.6, 1));
	}
}

NFmiPoint NFmiStationViewHandler::MakeSoundingMarkerScale(NFmiToolBox *theToolBox, double theSymbolSizeInMM)
{
    auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
    long pixelSizeX = boost::math::iround(graphicalInfo.itsPixelsPerMM_x * theSymbolSizeInMM);
    long pixelSizeY = boost::math::iround(graphicalInfo.itsPixelsPerMM_y * theSymbolSizeInMM);
	double scaleValueX = theToolBox->SX(pixelSizeX);
	double scaleValueY = theToolBox->SY(pixelSizeY);
	return NFmiPoint(scaleValueX, scaleValueY);
}

void NFmiStationViewHandler::DrawMovingSoundingSymbols(boost::shared_ptr<NFmiFastQueryInfo> &theSoundingInfo, int theUsedSymbol, double theSymbolSizeInMM)
{
	// merkit‰‰n kartalle kaikki liikkuvien luotauksien +- 30 minuutin sis‰ll‰ olevat 'luotaukset'
	theSoundingInfo->FirstLocation();  // Liikkuvissa luotauksissa vain yksi dummy paikka, laitetaan se p‰‰lle
	NFmiMetTime timeStart(itsTime);
	timeStart.ChangeByMinutes(-30);
	unsigned long timeIndex1 = 0;
	unsigned long timeIndex2 = 0;
	if(NFmiFastInfoUtils::FindTimeIndicesForGivenTimeRange(theSoundingInfo, timeStart, 60, timeIndex1, timeIndex2) == false)
		return ;

	float lat = 0;
	float lon = 0;
	float phase = 0;
	theSoundingInfo->Param(kFmiLatitude);
	unsigned long latIndex = theSoundingInfo->ParamIndex();
	theSoundingInfo->Param(kFmiLongitude);
	unsigned long lonIndex = theSoundingInfo->ParamIndex();
	theSoundingInfo->Param(kFmiPhaseOfFlight);
	unsigned long phaseIndex = theSoundingInfo->ParamIndex();

    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsRect);
    
	NFmiDrawingEnvironment enviNousu;
	::SetSoundingSymbolEnvi(enviNousu);
	enviNousu.SetFillColor(NFmiColor(0.85f,0.85f,0.f));
	NFmiPolyline markerPolyLineNousu(itsRect, 0, &enviNousu);
	::MakesoundingSymbolMarker(markerPolyLineNousu, theUsedSymbol);

	NFmiDrawingEnvironment enviLasku;
	::SetSoundingSymbolEnvi(enviLasku);
	enviLasku.SetFillColor(NFmiColor(0.5f,0.5f,0.99f));
	NFmiPolyline markerPolyLineLasku(itsRect, 0, &enviLasku);
	::MakesoundingSymbolMarker(markerPolyLineLasku, theUsedSymbol);

	NFmiDrawingEnvironment enviLento;
	::SetSoundingSymbolEnvi(enviLento);
	enviLento.SetFillColor(NFmiColor(0,0.5f,0.5f));
	NFmiPolyline markerPolyLineLento(itsRect, 0, &enviLento);
	::MakesoundingSymbolMarker(markerPolyLineLento, theUsedSymbol);

	NFmiPoint scale(MakeSoundingMarkerScale(itsToolBox, theSymbolSizeInMM));
	scale *= NFmiPoint(1.5, 1.5);
	NFmiRect circleRect;
	NFmiPoint circleScale(scale);
	circleScale *= NFmiPoint(2.3, 2.3);
	circleRect.Size(circleScale);

	for(unsigned long i=timeIndex1; i<=timeIndex2; i++)
	{
		theSoundingInfo->TimeIndex(i);

		NFmiLocation lastLoc;
		for(theSoundingInfo->ResetLevel() ; theSoundingInfo->NextLevel(); )
		{
			theSoundingInfo->ParamIndex(latIndex);
			lat = theSoundingInfo->FloatValue();
			theSoundingInfo->ParamIndex(lonIndex);
			lon = theSoundingInfo->FloatValue();
			theSoundingInfo->ParamIndex(phaseIndex);
			phase = theSoundingInfo->FloatValue();

			if(lat != kFloatMissing && lon != kFloatMissing)
			{
				NFmiPoint latlon(lon, lat);
				NFmiLocation loc(latlon);
				if(this->itsMapArea->IsInside(latlon))
				{
					bool startPoint = (theSoundingInfo->LevelIndex() == 0);
					double flightDir = lastLoc.Direction(loc);
					NFmiPoint viewPoint(LatLonToViewPoint(NFmiPoint(lon, lat)));
					circleRect.Center(viewPoint);
					if(phase == 6)
						startPoint ? itsToolBox->DrawEllipse(circleRect, &enviNousu) : itsToolBox->DrawPolyline(&markerPolyLineNousu, viewPoint, scale, flightDir);
					else if(phase == 5)
						startPoint ? itsToolBox->DrawEllipse(circleRect, &enviLasku) : itsToolBox->DrawPolyline(&markerPolyLineLasku, viewPoint, scale, flightDir);
					else
						startPoint ? itsToolBox->DrawEllipse(circleRect, &enviLento) : itsToolBox->DrawPolyline(&markerPolyLineLento, viewPoint, scale, flightDir);
					lastLoc = loc;
				}
			}
		}
	}
}

void NFmiStationViewHandler::DrawSoundingSymbols(boost::shared_ptr<NFmiFastQueryInfo> &theSoundingInfo, int theUsedSymbol, double theSymbolSizeInMM)
{
	if(theSoundingInfo == 0)
		return ;
    if(theSoundingInfo->IsGrid())
        return;
    theSoundingInfo->FirstLevel();
	if(NFmiFastInfoUtils::IsMovingSoundingData(theSoundingInfo))
	{ // Liikkuvissa luotauksissa on niin speciaali jutut ett‰ niiden piirtoon tarvitaan ihan oma funktio
		DrawMovingSoundingSymbols(theSoundingInfo, 4, theSymbolSizeInMM);
		return ;
	}

	if(theSoundingInfo->Time(itsTime))
	{ // jos luotaus info lˆytyi ja lˆytyi viel‰ aikakin, niin aletaan asemien piirto
		NFmiDrawingEnvironment envi;
		// piirret‰‰n oranssi kolmia osoittamaan asemien paikkaa
		envi.EnableFill();
		envi.SetFillColor(NFmiColor(1.f, 0.35f, 0.f));
		envi.EnableFrame();
		envi.SetFrameColor(NFmiColor(0.f, 0.f, 0.f));

		NFmiPolyline markerPolyLine(itsRect, 0, &envi);
		if(theUsedSymbol == 1)
		{ // piirr‰ kolmio
			markerPolyLine.AddPoint(NFmiPoint(-1, 1));
			markerPolyLine.AddPoint(NFmiPoint(0, -1));
			markerPolyLine.AddPoint(NFmiPoint(1, 1));
		}
		else if(theUsedSymbol == 2)
		{ // piirr‰ salmiakki
			markerPolyLine.AddPoint(NFmiPoint(0, 1));
			markerPolyLine.AddPoint(NFmiPoint(-0.7, 0));
			markerPolyLine.AddPoint(NFmiPoint(0, -1));
			markerPolyLine.AddPoint(NFmiPoint(0.7, 0));
		}
		else if(theUsedSymbol == 3)
		{ // piirr‰ neliˆ
			markerPolyLine.AddPoint(NFmiPoint(-1, -1));
			markerPolyLine.AddPoint(NFmiPoint(1, -1));
			markerPolyLine.AddPoint(NFmiPoint(1, 1));
			markerPolyLine.AddPoint(NFmiPoint(-1, 1));
		}
		else if(theUsedSymbol == 4)
		{ // piirr‰ korkea pylv‰s (mastoille)
            const double w = 0.5;
            const double h = 1.5;
			markerPolyLine.AddPoint(NFmiPoint(-w, -h));
			markerPolyLine.AddPoint(NFmiPoint(w, -h));
			markerPolyLine.AddPoint(NFmiPoint(w, h));
			markerPolyLine.AddPoint(NFmiPoint(-w, h));
		}
		else
		{ // piirr‰ kolmio ylˆsalaisin
			markerPolyLine.AddPoint(NFmiPoint(-1, -1));
			markerPolyLine.AddPoint(NFmiPoint(0, 1));
			markerPolyLine.AddPoint(NFmiPoint(1, -1));
		}

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsRect);

        auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
        long pixelSizeX = boost::math::iround(graphicalInfo.itsPixelsPerMM_x * theSymbolSizeInMM);
        long pixelSizeY = boost::math::iround(graphicalInfo.itsPixelsPerMM_y * theSymbolSizeInMM);
		double scaleValueX = itsToolBox->SX(pixelSizeX);
		double scaleValueY = itsToolBox->SY(pixelSizeY);
		NFmiPoint scale(scaleValueX, scaleValueY);
		for(theSoundingInfo->ResetLocation() ; theSoundingInfo->NextLocation(); )
		{
			NFmiPoint viewPoint(LatLonToViewPoint(theSoundingInfo->LatLon())); // t‰m‰ on offset
			if(NFmiSoundingData::HasRealSoundingData(theSoundingInfo))
				markerPolyLine.GetEnvironment()->SetFillColor(NFmiColor(0.551f, 0.9f, 0.21f)); // lˆytyy dataa, vihre‰ kolmio
			else
				markerPolyLine.GetEnvironment()->SetFillColor(NFmiColor(1.f, 0.35f, 0.f)); // ei dataa, oranssi kolmio
			itsToolBox->DrawPolyline(&markerPolyLine, viewPoint, scale);
		}
	}
}

bool NFmiStationViewHandler::ShowWarningMessages(void)
{
#ifndef DISABLE_CPPRESTSDK
    if(itsCtrlViewDocumentInterface->ShowWarningMarkersOnMap(itsMapViewDescTopIndex))
	{
		if(itsCtrlViewDocumentInterface->WarningCenterSystem().getLegacyData().WarningCenterViewOn()) // jos HALY dialogi on p‰‰ll‰
		{
			if(itsViewGridRowNumber == itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).Y()) // piirret‰‰n HALY symbolit vain viimeiselle riville
				return true;
		}
	}
#endif // DISABLE_CPPRESTSDK
	return false;
}

void NFmiStationViewHandler::DrawSilamStationMarkers(NFmiSilamStationList &theStationList, NFmiDrawingEnvironment &theEnvi, const NFmiString &theSynopStr, double symbolXShift, double symbolYShift, NFmiRect &thePlaceRect)
{
	auto &locations = theStationList.Locations();
	for(size_t i = 0; i< locations.size(); i++)
	{
		const NFmiPoint &latlon = locations[i].itsLatlon;
		if(itsMapArea->IsInside(latlon))
		{
			NFmiPoint p = LatLonToViewPoint(latlon);
			NFmiPoint p2 = LatLonToViewPoint(latlon);
			// pelkk‰ toolbox-alignmentti center (eik‰ mik‰‰n muukaan) vie teksti‰ keskelle y-suunnassa, joten t‰m‰ siirros siirt‰‰ tekstin ihan keskelle
			p2.Y(p2.Y() + symbolYShift);
			p2.X(p2.X() - symbolXShift);
			NFmiText txt(p2, theSynopStr, false, 0, &theEnvi);
			itsToolBox->Convert(&txt);
		}
	}
}

void NFmiStationViewHandler::DrawSilamStationMarkers(void)
{
	FmiDirection oldAlingment = itsToolBox->GetTextAlignment();
	try
	{
        auto trajectorySystem = itsCtrlViewDocumentInterface->TrajectorySystem();
		if(trajectorySystem->TrajectoryViewOn())
		{
			// piirret‰‰n vain alimm‰lle n‰yttˆriville
			if(itsViewGridColumnNumber == 1 && itsViewGridRowNumber == itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).Y())
			{
				double fontSizeInMM_x = 3.0;
				double fontSizeInMM_y = 3.0;
                auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
                int fontSizeX = static_cast<int>(fontSizeInMM_x * graphicalInfo.itsPixelsPerMM_x * 1.88);
				int fontSizeY = static_cast<int>(fontSizeInMM_y * graphicalInfo.itsPixelsPerMM_y * 1.88);

				NFmiDrawingEnvironment envi;
				envi.BoldFont(true);
				envi.SetFontType(kSynop);
				envi.SetFontSize(NFmiPoint(fontSizeX, fontSizeY));
				itsToolBox->SetTextAlignment(kBottomLeft);

				// asema markeria varten lasketaan pienen laatikon koko
				double dx = itsToolBox->SX(3);
				double dy = itsToolBox->SY(3);
				NFmiRect placeRect(0, 0, dx, dy);

				double fontShiftY =	itsToolBox->SY(static_cast<long>(envi.GetFontSize().Y()))/4.;
				double fontShiftX =	itsToolBox->SX(static_cast<long>(envi.GetFontSize().Y()))/4.4;

				int code = 161;
				std::string str;
				str += static_cast<unsigned char>(code);
				NFmiString synopFontStr(str);

				envi.SetFrameColor(NFmiColor(1,0,0)); // ydin voimalat punasella
				DrawSilamStationMarkers(trajectorySystem->NuclearPlants(), envi, synopFontStr, fontShiftX, fontShiftY, placeRect);
				envi.SetFrameColor(NFmiColor(0,0,1)); // muut paikat sinisell‰
				DrawSilamStationMarkers(trajectorySystem->OtherPlaces(), envi, synopFontStr, fontShiftX, fontShiftY, placeRect);
			}
		}
	}
	catch(...)
	{
	}
	itsToolBox->SetTextAlignment(oldAlingment);
}

namespace
{
	// T‰h‰n luokkaan luetaan kyseiset kuva-imaget kerran ja k‰ytet‰‰n kaikille NFmiStationViewHandler-instansseille yhteisesti.
	// N‰iden kuvien avulla piirret‰‰n sovittuja HAKE-varoitus merkkej‰.
	// Gdiplus::Bitmap-olioita ei tarvitse tuhota, Gdiplus huolehtii siit‰ automaattisesti.
	class AnimationButtonImageHolder
	{
	public:
		AnimationButtonImageHolder(void)
		:itsBitmapFolder()
		,itsLargeBuildingImage(nullptr)
		,itsFireWideImage(nullptr)
		,itsFireExtinguisher(nullptr)
		,itsFireExtinguisherLarge(nullptr)
		,itsHazardousMaterials(nullptr)
		,itsFireEngine(nullptr)
		,itsShipWreck(nullptr)
		,itsShipWreckOther(nullptr)
		,itsLifeBuoy(nullptr)
		,itsCarCrashSign(nullptr)
		,itsCarCrashPoliceSign(nullptr)
		,itsTrainCrashSign(nullptr)
		,itsTrainCrashPoliceSign(nullptr)
		,itsShipCrashSign(nullptr)
		,itsAirplaneCrashSign(nullptr)
		,itsSmokeObservation(nullptr)
        ,itsUnknownHakeMessageType(nullptr)
        ,itsKahaNoRain(nullptr)
        ,itsKahaHail(nullptr)
        ,itsKahaRain(nullptr)
        ,itsKahaTornado(nullptr)
        ,itsKahaFlood(nullptr)
        ,itsKahaWindDamage(nullptr)
        ,itsKahaThunder(nullptr)
        ,itsKahaIceThickness(nullptr)
        ,itsKahaIcingRain(nullptr)
        ,itsKahaPedestrianSlippery1(nullptr)
        ,itsKahaSnowDepth(nullptr)
        ,itsKahaSnow1(nullptr)
        ,itsKahaSnow2(nullptr)
        ,itsKahaSnow3(nullptr)
        ,itsKahaHails(nullptr)
        ,itsKahaSleet1(nullptr)
        ,itsKahaSleet2(nullptr)
        ,itsKahaSleet3(nullptr)
        ,itsKahaRain1(nullptr)
        ,itsKahaRain2(nullptr)
        ,itsKahaRain3(nullptr)
        ,itsKahaRoadSlippery(nullptr)
        ,itsKahaFog(nullptr)
        ,itsKahaWaterTemperature(nullptr)
        ,itsUnknownKaHaMessageType(nullptr)
        ,itsIconSizeInMM_x(9.8)
		,itsIconSizeInMM_y(9.8)
        ,fInitialized(false)
		{
		}

		// initialisoinnissa luetaan bitmapit tiedostoista, kutsu vasta kun itsBitmapFolder-dataosa on asetettu
		void Initialize(void)  // HUOM! heitt‰‰ poikkeuksia ep‰onnistuessaan
		{
			fInitialized = true;
			itsLargeBuildingImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "building-large-on-fire.png");
			itsFireWideImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "fire-wider-flames.png");
			itsFireExtinguisher = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "fire-extinguisher.png");
			itsFireExtinguisherLarge = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "fire-extinguisher-large.png");
			itsHazardousMaterials = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "bio_hazard.png");
			itsFireEngine = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "fire_engine_v1.png");
			itsShipWreck = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "shipwreck.png");
			itsShipWreckOther = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "shipcrashpolice.png");
			itsLifeBuoy = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "lifebuoy.png");
			itsCarCrashSign = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "carcrashwarningsign.png");
			itsCarCrashPoliceSign = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "carcrashpolice.png");
			itsTrainCrashSign = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "railwaywarningsign.png");
			itsTrainCrashPoliceSign = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "traincrashpolice.png");
			itsShipCrashSign = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "boatcrashwarningsign.png");
			itsAirplaneCrashSign = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "airplanewarningsign.png");
			itsSmokeObservation = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "smoke_obs.png");
            itsUnknownHakeMessageType = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "question_mark_hake_32x32.png");

            itsKahaNoRain = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-ei-sadetta.png");
            itsKahaHail = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-rakeet.png");
            itsKahaRain = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-sadetta.png");
            itsKahaTornado = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-trombi.png");
            itsKahaFlood = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-tulva.png");
            itsKahaWindDamage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-tuulivahinko.png");
            itsKahaThunder = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-ukkoshavainto.png");
            itsKahaIceThickness = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-jaan-paksuus.png");
            itsKahaIcingRain = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-jaatavasade.png");
            itsKahaPedestrianSlippery1 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-jalankulkuliukkaus-1.png");
            itsKahaSnowDepth = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-lumensyvyys.png");
            itsKahaSnow1 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-lumisade-1.png");
            itsKahaSnow2 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-lumisade-2.png");
            itsKahaSnow3 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-lumisade-3.png");
            itsKahaHails = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-rakeet.png");
            itsKahaSleet1 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-rantasade-1.png");
            itsKahaSleet2 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-rantasade-2.png");
            itsKahaSleet3 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-rantasade-3.png");
            itsKahaRain1 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-vesisade-1.png");
            itsKahaRain2 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-vesisade-2.png");
            itsKahaRain3 = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-vesisade-3.png");
            itsKahaRoadSlippery = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-tieliikennehairio.png");
            itsKahaFog = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-sumu.png");
            itsKahaWaterTemperature = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/icon-vedenlampotila.png");
            itsUnknownKaHaMessageType = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "KaHa-icons/question_mark_kaha_32x32.png");
        }

		std::string itsBitmapFolder;
        // Normal Hake message icons
		Gdiplus::Bitmap *itsLargeBuildingImage;
		Gdiplus::Bitmap *itsFireWideImage;
		Gdiplus::Bitmap *itsFireExtinguisher;
		Gdiplus::Bitmap *itsFireExtinguisherLarge;
		Gdiplus::Bitmap *itsHazardousMaterials;
		Gdiplus::Bitmap *itsFireEngine;
		Gdiplus::Bitmap *itsShipWreck;
		Gdiplus::Bitmap *itsShipWreckOther;
		Gdiplus::Bitmap *itsLifeBuoy; // pelastusrengas
		Gdiplus::Bitmap *itsCarCrashSign; // liikenne onnettomuus varoitus kyltti
		Gdiplus::Bitmap *itsCarCrashPoliceSign; // liikenne onnettomuus varoitus kyltti poliisi
		Gdiplus::Bitmap *itsTrainCrashSign; // liikenne onnettomuus raiteilla varoitus kyltti
		Gdiplus::Bitmap *itsTrainCrashPoliceSign; // liikenne onnettomuus raiteilla varoitus kyltti poliisi
		Gdiplus::Bitmap *itsShipCrashSign; // liikenne onnettomuus vesill‰ varoitus kyltti
        Gdiplus::Bitmap *itsAirplaneCrashSign; // lento onnettomuus varoitus kyltti
		Gdiplus::Bitmap* itsSmokeObservation; // savuhavainto symboli
        Gdiplus::Bitmap *itsUnknownHakeMessageType; // tuntematon sanoma id, jolloin merkiksi punainen kysymerkki pallo

        // New Kaha icons (citizen observations)
        Gdiplus::Bitmap *itsKahaNoRain;
        Gdiplus::Bitmap *itsKahaHail;
        Gdiplus::Bitmap *itsKahaRain;
        Gdiplus::Bitmap *itsKahaTornado;
        Gdiplus::Bitmap *itsKahaFlood;
        Gdiplus::Bitmap *itsKahaWindDamage;
        Gdiplus::Bitmap *itsKahaThunder;

        Gdiplus::Bitmap *itsKahaIceThickness;
        Gdiplus::Bitmap *itsKahaIcingRain;
        Gdiplus::Bitmap *itsKahaPedestrianSlippery1;
        Gdiplus::Bitmap *itsKahaSnowDepth;
        Gdiplus::Bitmap *itsKahaSnow1;
        Gdiplus::Bitmap *itsKahaSnow2;
        Gdiplus::Bitmap *itsKahaSnow3;
        Gdiplus::Bitmap *itsKahaHails;
        Gdiplus::Bitmap *itsKahaSleet1;
        Gdiplus::Bitmap *itsKahaSleet2;
        Gdiplus::Bitmap *itsKahaSleet3;
        Gdiplus::Bitmap *itsKahaRain1;
        Gdiplus::Bitmap *itsKahaRain2;
        Gdiplus::Bitmap *itsKahaRain3;
        Gdiplus::Bitmap *itsKahaRoadSlippery;
        Gdiplus::Bitmap *itsKahaFog;
        Gdiplus::Bitmap *itsKahaWaterTemperature;

        Gdiplus::Bitmap *itsUnknownKaHaMessageType; // tuntematon sanoma id, jolloin merkiksi sininen kysymysmerkki

        double itsIconSizeInMM_x; // t‰m‰ on haluttujen 16 x 16 pix ikonien koko printatessa
		double itsIconSizeInMM_y;

		bool fInitialized;
	};

	AnimationButtonImageHolder gAnimationButtonImageHolder; // yksi instannsi luodaan nimettˆm‰‰n namespaceen
}

void NFmiStationViewHandler::InitializeWarningSymbolFiles(void)
{
	std::string confName = "SmartMet::WomlDirectoryPath";
	if (NFmiSettings::IsSet(confName))
	{
		std::string name = NFmiSettings::Require<std::string>(confName);
		gAnimationButtonImageHolder.itsBitmapFolder = itsCtrlViewDocumentInterface->ControlPath() + name + "\\warning-icons";
		gAnimationButtonImageHolder.Initialize();
	}
	else
	{
		gAnimationButtonImageHolder.itsBitmapFolder = itsCtrlViewDocumentInterface->HelpDataPath() + "\\warning-icons";
		gAnimationButtonImageHolder.Initialize();
	}
}

void NFmiStationViewHandler::InitializeWarningSymbols(void)
{
	if(gAnimationButtonImageHolder.fInitialized == false)
	{
		try
		{
			InitializeWarningSymbolFiles();
		}
		catch(std::exception &e)
		{
			std::string errStr("Error in NFmiStationViewHandler::InitializeWarningSymbols, while trying to read animation button bitmaps from files: \n");
			errStr += e.what();
			itsCtrlViewDocumentInterface->LogAndWarnUser(errStr, "Error while trying to read animation button bitmaps", CatLog::Severity::Error, CatLog::Category::Configuration, true);
		}
	}
}

void NFmiStationViewHandler::GetShownMessages()
{
#ifndef DISABLE_CPPRESTSDK
    itsShownHakeMessages.clear();
    itsShownKaHaMessages.clear();

    boost::shared_ptr<NFmiArea> zoomedArea = itsCtrlViewDocumentInterface->GetMapHandlerInterface(itsMapViewDescTopIndex)->Area();
    int timeStepForMessagesInMinutes = itsCtrlViewDocumentInterface->GetTimeRangeForWarningMessagesOnMapViewInMinutes();
    NFmiMetTime startTime = HakeLegacySupport::HakeSystemConfigurations::MakeStartTimeForHakeMessages(itsTime, timeStepForMessagesInMinutes);

    if(itsCtrlViewDocumentInterface->ApplicationWinRegistry().ShowHakeMessages())
        itsShownHakeMessages = itsCtrlViewDocumentInterface->WarningCenterSystem().getHakeMessages(startTime, itsTime, *zoomedArea);
    if(itsCtrlViewDocumentInterface->ApplicationWinRegistry().ShowKaHaMessages())
        itsShownKaHaMessages = itsCtrlViewDocumentInterface->WarningCenterSystem().getKahaMessages(startTime, itsTime, *zoomedArea);
#endif // DISABLE_CPPRESTSDK
}

const NFmiColor gDefaultHakeFillColor(1, 0, 0);
const NFmiColor gDefaultKaHaFillColor(0, 0, 1);

void NFmiStationViewHandler::DrawHALYMessageMarkers(void)
{
#ifndef DISABLE_CPPRESTSDK
    if(ShowWarningMessages())
    {
        InitializeWarningSymbols();
        GetShownMessages();

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsRect);
        ShowWarningMessages(itsShownHakeMessages, true);
        ShowWarningMessages(itsShownKaHaMessages, false);
    }
#endif // DISABLE_CPPRESTSDK
}

#ifndef DISABLE_CPPRESTSDK
void NFmiStationViewHandler::ShowWarningMessages(const std::vector<HakeMessage::HakeMsg> &messages, bool isHakeMessage)
{
    for(const auto &message : messages)
    {
        DrawWantedWarningIcon(message, isHakeMessage);
    }
}

void NFmiStationViewHandler::DrawHakeMessageIcon(const HakeMessage::HakeMsg &theWarningMessage, const NFmiPoint &latlon)
{
    switch(theWarningMessage.Category())
    {
    case 201:
    { // piirr‰ tieliikenneonnettomuus poliisi
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsCarCrashPoliceSign, 0.85f, 0.85);
        break;
    }
    case 202:
    { // piirr‰ tieliikenneonnettomuus pelastus pieni
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsCarCrashSign, 0.85f, 0.7);
        break;
    }
    case 203:
    { // piirr‰ tieliikenneonnettomuus pelastus keskisuuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsCarCrashSign, 0.85f, 0.85);
        break;
    }
    case 204:
    { // piirr‰ tieliikenneonnettomuus pelastus suuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsCarCrashSign, 0.85f, 1.);
        break;
    }

    case 211:
    { // piirr‰ raideliikenneonnettomuus poliisi keskisuuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsTrainCrashPoliceSign, 0.85f, 0.85);
        break;
    }
    case 212:
    { // piirr‰ raideliikenneonnettomuus pelastus pieni
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsTrainCrashSign, 0.85f, 0.7);
        break;
    }
    case 213:
    { // piirr‰ raideliikenneonnettomuus pelastus keskisuuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsTrainCrashSign, 0.85f, 0.85);
        break;
    }
    case 214:
    { // piirr‰ raideliikenneonnettomuus pelastus suuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsTrainCrashSign, 0.85f, 1.);
        break;
    }

    case 220:
    { // piirr‰ vesiliikenne onnettomuus muu poliisi
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsShipWreckOther, 0.85f, 0.8);
        break;
    }
    case 221:
    { // piirr‰ vesiliikenne onnettomuus pieni
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsShipWreck, 0.85f, 0.75);
        break;
    }
    case 222:
    { // piirr‰ vesiliikenne onnettomuus keskisuuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsShipWreck, 0.85f, 0.9);
        break;
    }
    case 223:
    { // piirr‰ vesiliikenne onnettomuus suuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsShipWreck, 0.85f, 1.1);
        break;
    }

    case 231:
    { // piirr‰ ilmaliikenneonnettomuus pelastus pieni
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsAirplaneCrashSign, 0.85f, 0.7);
        break;
    }
    case 232:
    { // piirr‰ ilmaliikenneonnettomuus pelastus keskisuuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsAirplaneCrashSign, 0.85f, 0.85);
        break;
    }
    case 233:
    { // piirr‰ ilmaliikenneonnettomuus pelastus suuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsAirplaneCrashSign, 0.85f, 1.);
        break;
    }
    case 234:
    { // piirr‰ ilmaliikenneonnettomuus vaara pieni
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsAirplaneCrashSign, 0.85f, 0.7);
        break;
    }
    case 235:
    { // piirr‰ ilmaliikenneonnettomuus vaara keskisuuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsAirplaneCrashSign, 0.85f, 0.85);
        break;
    }
    case 236:
    { // piirr‰ ilmaliikenneonnettomuus vaara suuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsAirplaneCrashSign, 0.85f, 1.);
        break;
    }

	case 420:
	{ // piirr‰ savuhavainto
		DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsSmokeObservation, 0.85f, 0.8);
		break;
	}

    case 421:
    { // piirr‰ maastopalo pieni
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsFireWideImage, 0.85f, 0.6);
        break;
    }

    case 422:
    { // piirr‰ maastopalo keskisuuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsFireWideImage, 0.85f, 0.8);
        break;
    }

    case 423:
    { // piirr‰ maastopalo suuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsFireWideImage, 0.85f);
        break;
    }
    case 461:
    { // piirr‰ vahingontorjunta pieni
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsFireExtinguisher, 0.85f);
        break;
    }
    case 462:
    { // piirr‰ vahingontorjunta keskisuuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsFireExtinguisherLarge, 0.75f);
        break;
    }
    case 463:
    { // piirr‰ vahingontorjunta suuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsFireEngine, 0.85f);
        break;
    }
    case 403:
    { // piirr‰ rakennuspalo suuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsLargeBuildingImage, 0.9f);
        break;
    }
    case 451:
    { // piirr‰ vahingollista ainetta pieni
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsHazardousMaterials, 0.9f, 0.6);
        break;
    }
    case 452:
    { // piirr‰ vahingollista ainetta keskisuuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsHazardousMaterials, 0.9f, 0.8);
        break;
    }
    case 453:
    { // piirr‰ vahingollista ainetta suuri
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsHazardousMaterials, 0.9f);
        break;
    }
    case 483:
    { // piirr‰ Ihmisen pelastaminen vedest‰
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsLifeBuoy, 0.85f, 1.);
        break;
    }
    default:
    {
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsUnknownHakeMessageType, 0.9f);
        break;
    }
    }
}

void NFmiStationViewHandler::DrawKaHaMessageIcon(const HakeMessage::HakeMsg &theWarningMessage, const NFmiPoint &latlon)
{
    switch(theWarningMessage.Category())
    {
    case 1:
    { // tuuli vahinko
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaWindDamage, 0.85f, 0.85);
        break;
    }
    case 2:
    { // tulva
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaFlood, 0.85f, 0.85);
        break;
    }
    case 3:
    { // rakeita
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaHail, 0.85f, 0.85);
        break;
    }
    case 4:
    { // trombi
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaTornado, 0.85f, 0.85);
        break;
    }
    case 5:
    { // sadetta/ei sadetta. P‰‰tett‰v‰ kumpi symboli n‰ytet‰‰n.
        std::size_t found = theWarningMessage.TotalMessageStr().find("Ei sadetta");
        if(found != std::string::npos)
        {
            DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaNoRain, 0.85f, 0.85);
        }
        else
        {
            DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaRain, 0.85f, 0.85);
        }
        break;
    }
    case 6:
    { // ukkosta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaThunder, 0.85f, 0.85);
        break;
    }
    case 7:
    { // J‰‰npaksuus
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaIceThickness, 0.85f, 0.85);
        break;
    }
    case 8:
    { // Jalankulkuliukkaus
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaPedestrianSlippery1, 0.85f, 0.85);
        break;
    }
    case 9:
    { // S‰‰n aiheuttama tieliikenneh‰iriˆ
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaRoadSlippery, 0.85f, 0.85);
        break;
    }
    case 10:
    { // Lumensyvyys
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaSnowDepth, 0.85f, 0.85);
        break;
    }
    case 12:
    { // Sumu
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaFog, 0.85f, 0.85);
        break;
    }
    case 13:
    { // Veden pintal‰mpˆtila
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaWaterTemperature, 0.85f, 0.85);
        break;
    }
    case 1100:
    { // ei sadetta.
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaNoRain, 0.85f, 0.85);
        break;
    }
    // vesisateet 1121-1123
    case 1121:
    { // heikkoa vesi sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaRain1, 0.85f, 0.85);
        break;
    }
    case 1122:
    { // kohtalaista vesi sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaRain2, 0.85f, 0.85);
        break;
    }
    case 1123:
    { // voimakasta vesi sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaRain3, 0.85f, 0.85);
        break;
    }
    // r‰nt‰sateet 1131-1133
    case 1131:
    { // heikkoa r‰nt‰ sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaSleet1, 0.85f, 0.85);
        break;
    }
    case 1132:
    { // kohtalaista r‰nt‰ sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaSleet2, 0.85f, 0.85);
        break;
    }
    case 1133:
    { // voimakasta r‰nt‰ sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaSleet3, 0.85f, 0.85);
        break;
    }
    // lumisateet 1141-1143
    case 1141:
    { // heikkoa lumi sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaSnow1, 0.85f, 0.85);
        break;
    }
    case 1142:
    { // kohtalaista lumi sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaSnow2, 0.85f, 0.85);
        break;
    }
    case 1143:
    { // voimakasta lumi sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaSnow3, 0.85f, 0.85);
        break;
    }
    // j‰‰t‰v‰t sateet 1151-1153
    case 1151:
    case 1152:
    case 1153:
    { // j‰‰t‰v‰‰ sadetta
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsKahaIcingRain, 0.85f, 0.85);
        break;
    }
    default:
    {
        DrawWarningIcon(latlon, gAnimationButtonImageHolder.itsUnknownKaHaMessageType, 0.85f, 0.85);
        break;
    }
    }
}

// piirret‰‰n varoitukseen liittyv‰ symboli halutulla tavalla, jos kyseiseen varoitukseen liittyy joku m‰‰r‰tty symboli.
// jos mit‰‰n ei piirret‰, palauttaa false merkiksi ett‰ piirto on teht‰v‰ vanhalla tavalla.
void NFmiStationViewHandler::DrawWantedWarningIcon(const HakeMessage::HakeMsg &theWarningMessage, bool isHakeMessage)
{
	NFmiPoint latlon = theWarningMessage.LatlonPoint();
    if(isHakeMessage)
    {
        DrawHakeMessageIcon(theWarningMessage, latlon);
    }
    else
    {
        DrawKaHaMessageIcon(theWarningMessage, latlon);
    }
}
#endif // DISABLE_CPPRESTSDK

void NFmiStationViewHandler::DrawWarningIcon(const NFmiPoint &theLatlon, Gdiplus::Bitmap *theImage, float theAlpha, double theSizeFactor)
{
	NFmiRect symbolrect = CalcSymbolRelativeRect(theLatlon, theImage, theSizeFactor);
    CtrlView::DrawAnimationButton(symbolrect, theImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), theAlpha, true);
}

NFmiRect NFmiStationViewHandler::CalcSymbolRelativeRect(const NFmiPoint &theLatlon, Gdiplus::Bitmap *theImage, double theSizeFactor)
{
	NFmiPoint viewPoint(LatLonToViewPoint(theLatlon));
	NFmiPoint relativeSize = CalcRelativeWarningIconSize(theImage);
	relativeSize.X(relativeSize.X()*theSizeFactor);
	relativeSize.Y(relativeSize.Y()*theSizeFactor);
	NFmiRect symbolRect(0, 0, relativeSize.X(), relativeSize.Y());
	symbolRect.Center(viewPoint);
	return symbolRect;
}

NFmiPoint NFmiStationViewHandler::GetViewSizeInPixels(void)
{
	return itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex);
}

NFmiPoint NFmiStationViewHandler::CalcRelativeWarningIconSize(Gdiplus::Bitmap *theImage)
{
    auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
    double relativeWidth = itsToolBox->SX(boost::math::iround(gAnimationButtonImageHolder.itsIconSizeInMM_x * graphicalInfo.itsPixelsPerMM_x));
    double relativeHeight = itsToolBox->SY(boost::math::iround(gAnimationButtonImageHolder.itsIconSizeInMM_y * graphicalInfo.itsPixelsPerMM_y));
	if(itsCtrlViewDocumentInterface->Printing() == false)
	{
		long bitmapSizeX = 32;
		long bitmapSizeY = 32;
		if(theImage)
		{
			bitmapSizeX = theImage->GetWidth();
			bitmapSizeY = theImage->GetHeight();
		}
		relativeWidth = itsToolBox->SX(bitmapSizeX);
		relativeHeight = itsToolBox->SY(bitmapSizeY);
	}
	return NFmiPoint(relativeWidth, relativeHeight);
}

NFmiPoint NFmiStationViewHandler::LatLonToViewPoint(const NFmiPoint& theLatLon) const
{
	return itsMapArea->ToXY(theLatLon);
}

NFmiPoint NFmiStationViewHandler::ViewPointToLatLon(const NFmiPoint& theViewPoint) const
{
	return itsMapArea->ToLatLon(theViewPoint);
}

void NFmiStationViewHandler::DrawLegends(NFmiToolBox* theGTB)
{
	if(DrawContourLegendOnThisMapRow())
	{
		// Let's draw first possible Wms legends on right side of view
		DrawWmsLegends(theGTB);

		// Then normal queryData based legends are drawn on the left side of view
		itsToolBox = theGTB;
		auto drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex());
		if(drawParamList && drawParamList->NumberOfItems() >= 1)
		{
			auto& colorContourLegendSettings = itsCtrlViewDocumentInterface->ColorContourLegendSettings();
			auto& graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
			const auto& viewGridSize = itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex);
			auto sizeFactor = NFmiStationView::CalcUsedLegendSizeFactor(itsCtrlViewDocumentInterface->SingleMapViewHeightInMilliMeters(itsMapViewDescTopIndex), static_cast<int>(viewGridSize.Y()));
			auto lastLegendRelativeBottomRightCorner = CtrlView::CalcProjectedPointInRectsXyArea(itsMapArea->XYArea(), itsCtrlViewDocumentInterface->ColorContourLegendSettings().relativeStartPosition());

			for(const auto& drawParam : *drawParamList)
			{
				if(!drawParam->IsParamHidden() && drawParam->ShowContourLegendPotentially())
				{
					auto drawParamPtr = boost::make_shared<NFmiDrawParam>(*drawParam);
					auto fastInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(drawParamPtr, false, true);
					NFmiColorContourLegendValues colorContourLegendValues(drawParamPtr, fastInfo);
					if(colorContourLegendValues.useLegend())
					{
						CtrlView::DrawNormalColorContourLegend(colorContourLegendSettings, colorContourLegendValues, lastLegendRelativeBottomRightCorner, itsToolBox, graphicalInfo, *itsGdiPlusGraphics, sizeFactor, GetFrame());
					}
				}
			}
		}
	}
}

bool NFmiStationViewHandler::DrawContourLegendOnThisMapRow()
{
    bool drawLegend = false;
    CtrlViewUtils::MapViewMode displayMode = itsCtrlViewDocumentInterface->MapViewDisplayMode(itsMapViewDescTopIndex);
    if(displayMode == CtrlViewUtils::MapViewMode::kOneTime)
    { 
        // 1-time -moodissa piirret‰‰n legenda jokaiseen ruutuun
        drawLegend = true;
    }
    else if(displayMode == CtrlViewUtils::MapViewMode::kRunningTime)
    {
        // running-time -moodissa piirret‰‰n legenda vain alimman rivin oikean puoleiseen ruutuun
        if((itsViewGridRowNumber == itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).Y()) && (itsViewGridColumnNumber == itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).X()))
            drawLegend = true;
    }
    else
    {
        // normaali -moodissa piirret‰‰n legenda vain oikean puoleiseen sarakkeeseen
        if((itsViewGridColumnNumber == itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).X()))
            drawLegend = true;
    }
    return drawLegend;
}

void NFmiStationViewHandler::DrawWmsLegends(NFmiToolBox* theGTB)
{
#ifndef DISABLE_CPPRESTSDK
    if(!theGTB || (itsDrawParam && itsDrawParam->IsParamHidden()))
    {
        return;
    }
    itsToolBox = theGTB;
	auto realRowIndex = CalcRealRowIndex();
	auto wmsSupportPtr = itsCtrlViewDocumentInterface->GetWmsSupport();
    auto registeredLayers = wmsSupportPtr->getRegisteredLayers(realRowIndex, itsViewGridColumnNumber, itsMapViewDescTopIndex);
    auto drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, realRowIndex);
	if(drawParamList)
	{
		for(const auto& registered : registeredLayers)
		{
			if(!drawParamList->Find(registered, nullptr, NFmiInfoData::kWmsData))
			{
				wmsSupportPtr->unregisterDynamicLayer(realRowIndex, itsViewGridColumnNumber, itsMapViewDescTopIndex, registered);
			}
			else
			{
				// Viel‰ jos piirto-optioissa ei ole legendan piirto p‰‰ll‰, poistetaan rekisterˆidyist‰ (en ymm‰rr‰ logiikkaa, miten sen saa taas p‰‰lle)
				if(!drawParamList->Current()->ShowColorLegend())
					wmsSupportPtr->unregisterDynamicLayer(realRowIndex, itsViewGridColumnNumber, itsMapViewDescTopIndex, registered);
			}
		}

		auto legends = wmsSupportPtr->getLegends(realRowIndex, itsViewGridColumnNumber, itsMapViewDescTopIndex);
		if(legends.empty())
		{
			return;
		}

		auto bitmapSize = NFmiPoint(itsCtrlViewDocumentInterface->ActualMapBitmapSizeInPixels(itsMapViewDescTopIndex));
		// calculate how many columns there are
		auto wholeDesktopSize = itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex);
		auto nCols = static_cast<int>(wholeDesktopSize.X() / bitmapSize.X());

		// draw legends only if this is the last column
		if(itsViewGridColumnNumber != nCols)
		{
			return;
		}
		try
		{
			InitializeGdiplus(itsToolBox, &GetFrame());
			auto sizeToFitHorizontal = NFmiPoint{ bitmapSize.X(), bitmapSize.Y() / 5 };
			auto sizeToFitVertical = NFmiPoint{ bitmapSize.X() / 5, bitmapSize.Y() - (bitmapSize.Y() / 5) };

			// partition legends into higher than wide and vice versa

			auto posH = std::partition(legends.begin(), legends.end(), [](const auto& legend)
				{
					return legend->mImage->GetWidth() > legend->mImage->GetHeight();
				});

			auto vLegends = std::vector<Wms::LegendBuffer>{};
			std::copy(posH, legends.end(), std::back_inserter(vLegends));

			auto hLegends = std::vector<Wms::LegendBuffer>{};
			std::copy(legends.begin(), posH, std::back_inserter(hLegends));

			// sort

			std::sort(hLegends.begin(), hLegends.end(), [](const auto& legend1, const auto& legend2)
				{
					return legend1.width > legend2.width;
				});

			std::sort(vLegends.begin(), vLegends.end(), [](const auto& legend1, const auto& legend2)
				{
					return legend1.height > legend2.height;
				});

			// pack

			// horizontal

			auto hSelves = Wms::LegendSelf(static_cast<unsigned int>(sizeToFitHorizontal.X()), static_cast<unsigned int>(sizeToFitHorizontal.Y()), Wms::Orientation::Horizontal);
			hSelves.insert(hLegends);

			// vertical

			auto vSelves = Wms::LegendSelf(static_cast<unsigned int>(sizeToFitVertical.Y()), static_cast<unsigned int>(sizeToFitVertical.X()), Wms::Orientation::Vertical);
			vSelves.insert(vLegends);

			// check if compressing is needed

			// horizontal

			if(hSelves.shouldCompress())
			{
				hSelves.compress();
			}

			// vertical
			if(vSelves.shouldCompress())
			{
				vSelves.compress();
			}

			// Draw

			NFmiPoint startPoint(CtrlViewUtils::ConvertPointFromRect1ToRect2(itsRect.TopLeft(), NFmiRect(0, 0, 1, 1), NFmiRect(NFmiPoint(0, 0), itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex))));

			// horizontal
			for(auto& self : hSelves.selves)
			{
				// we draw from left to right starting from the bottom left corner
				for(auto& legend : self.legends)
				{
					auto legendW = legend.width;
					auto legendH = legend.height;
					Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(startPoint.X() + bitmapSize.X() - hSelves.horizontalShift - legendW), static_cast<Gdiplus::REAL>(startPoint.Y() + bitmapSize.Y() - hSelves.verticalShift - legendH), static_cast<Gdiplus::REAL>(legendW), static_cast<Gdiplus::REAL>(legendH));
					NFmiRect sourceRect(0, 0, legendW, legendH);
					Gdiplus::REAL alpha = itsDrawParam->Alpha() / 100.f; // 0 on t‰ysin l‰pin‰kyv‰, 0.5 = semi transparent ja 1.0 = opaque
					bool doNearestInterpolation = alpha >= 1.f ? true : false;
					CtrlView::DrawBitmapToDC_4(itsToolBox->GetDC(), *legend.get()->mImage, sourceRect, destRect, doNearestInterpolation, NFmiImageAttributes(alpha), itsGdiPlusGraphics);

					// shift
					hSelves.horizontalShift += legendW;
				}

				// shift
				hSelves.horizontalShift = 0;
				hSelves.verticalShift += self.getHeight();
			}

			// vertical
			for(auto& self : vSelves.selves)
			{
				// we draw from top to bottom starting from the top right corner
				for(auto& legend : self.legends)
				{
					auto legendW = legend.width;
					auto legendH = legend.height;
					Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(startPoint.X() + bitmapSize.X() - vSelves.horizontalShift - legendW), static_cast<Gdiplus::REAL>(startPoint.Y() + vSelves.verticalShift), static_cast<Gdiplus::REAL>(legendW), static_cast<Gdiplus::REAL>(legendH));
					NFmiRect sourceRect(0, 0, legendW, legendH);
					Gdiplus::REAL alpha = itsDrawParam->Alpha() / 100.f; // 0 on t‰ysin l‰pin‰kyv‰, 0.5 = semi transparent ja 1.0 = opaque
					bool doNearestInterpolation = alpha >= 1.f ? true : false;
					CtrlView::DrawBitmapToDC_4(itsToolBox->GetDC(), *legend.get()->mImage, sourceRect, destRect, doNearestInterpolation, NFmiImageAttributes(alpha), itsGdiPlusGraphics);

					// shift
					vSelves.verticalShift += legendH;
				}

				// shift
				vSelves.horizontalShift += self.getHeight();
				vSelves.verticalShift = 0;
			}
		}
		catch(...)
		{
		}
	}
#endif // DISABLE_CPPRESTSDK
}

// Piirt‰‰ datan ruutuun, eli data-viewlistan ja mm. projektioviivat jos niin on asetukset
void NFmiStationViewHandler::DrawData(NFmiToolBox* theGTB)
{
    if(itsViewList)
    {
        if(itsViewGridRowNumber == 1) // piirret‰‰n vain ylimm‰iseen riviin maskit
        {
            boost::shared_ptr<NFmiAreaMaskList> maskList = itsCtrlViewDocumentInterface->ParamMaskListMT();
            if(maskList)
                maskList->SyncronizeMaskTime(itsTime); // en ole varma mihin t‰m‰ pit‰‰ laittaa
            DrawMasksOnMap(theGTB); // piirret‰‰n maski kartan p‰‰lle, mutta varsinaisen datan alle
        }
        itsViewList->Draw(theGTB);
    }

    // piirret‰‰n viel‰ datan p‰‰lle projektio viivat, jos asetukset sallivat
    if(itsCtrlViewDocumentInterface->ProjectionCurvatureInfo()->GetDrawingMode() == NFmiProjectionCurvatureInfo::kOverEverything)
        DrawProjetionLines(theGTB);

	if(!HasSeparateCountryBorderLayer())
	{
		// Rajaviivat piirret‰‰n kartalle t‰‰ll‰ vain jos niit‰ ei piirret‰ erillisess‰ kerroksessa.
		// nullptr -parametri tarkoittaa ett‰ k‰ytet‰‰n yleist‰ rajaviiva piirtoa.
		NFmiCountryBorderDrawUtils::drawCountryBordersToMapView(this, theGTB, nullptr);
	}
}

bool NFmiStationViewHandler::DrawTimeTextInThisMapViewTile(void)
{
	bool drawText = false;
    CtrlViewUtils::MapViewMode displayMode = itsCtrlViewDocumentInterface->MapViewDisplayMode(itsMapViewDescTopIndex);
	if(displayMode == CtrlViewUtils::MapViewMode::kOneTime)
	{ // 1-moodissa piirret‰‰nvain alimman rivin 1. ruutuun teksti
		drawText = (itsViewGridColumnNumber == 1) && (itsViewGridRowNumber == itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).Y());
	}
	else if(displayMode == CtrlViewUtils::MapViewMode::kRunningTime)
		drawText = true; // 2-moodissa piirret‰‰n aika joka ruutuun
	else // normaali/muussa moodissa piirret‰‰n tekstit vain viimeiselle riville
		drawText = (itsViewGridRowNumber == itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).Y());
	return drawText;
}

// Jos ollaan normaali moodissa, palautetaan vain annettu Utc-aika.
// Jos ollaan Beta-product moodissa ja kyseinen beta-product haluaa ett‰ k‰ytet‰‰n lokaali aikoja, muunnetaan annettu utc-aika lokaaliksi.
static NFmiMetTime CalcUsedTimeboxTime(const NFmiMetTime &theOriginalUtcTime, const NFmiBetaProduct *theUsedBetaProduct)
{
    if(theUsedBetaProduct && !theUsedBetaProduct->UseUtcTimesInTimeBox())
        return NFmiMetTime(theOriginalUtcTime.CorrectLocalTime(), 1);
    else
        return theOriginalUtcTime;
}

// Jos ollaan normaali piirto moodissa, palautetaan normaali time formaatti teksti sellaisenaan.
// Jos ollaan Beta-product moodissa ja kyseinen beta-product haluaa ett‰ k‰ytet‰‰n lokaali aikoja, yritet‰‰n tekstist‰ poistaa utc -sanat (koska smartmetin konffeissa tulee defaulttina UTC -sana mukaan).
static std::string MakeUsedTimeboxTimeFormatString(const std::string &theOriginalTimeFormatString, const NFmiBetaProduct *theUsedBetaProduct)
{
    if(theUsedBetaProduct && !theUsedBetaProduct->UseUtcTimesInTimeBox())
    {
        std::string finalFormatString = theOriginalTimeFormatString;
        boost::algorithm::ireplace_first(finalFormatString, "utc", ""); // Poistetaan vain 1. lˆytyv‰, jos utc on useita kertoja niin ei voi mit‰‰n...
        return finalFormatString;
    }
    else
        return theOriginalTimeFormatString;
}

void NFmiStationViewHandler::DrawTimeText(void)
{
	if(itsCtrlViewDocumentInterface->EditedSmartInfo())
	{
		if(itsCtrlViewDocumentInterface->ShowTimeString(itsMapViewDescTopIndex))
		{
			if(DrawTimeTextInThisMapViewTile())
			{
				auto& graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
				NFmiMapViewTimeLabelInfo& timeLabelInfo = itsCtrlViewDocumentInterface->MapViewTimeLabelInfo();

				const NFmiBetaProduct* currentBetaProduct = itsCtrlViewDocumentInterface->GetCurrentGeneratedBetaProduct();
				NFmiMetTime usedTimeboxTime = ::CalcUsedTimeboxTime(itsTime, currentBetaProduct);
				// Aikatekstien varsinainen piirtokoodi alkaa
				NFmiString formatStr1 = ::MakeUsedTimeboxTimeFormatString(timeLabelInfo.TimeStringInfo1().itsTimeFormat, currentBetaProduct);
				NFmiString timeStr1(usedTimeboxTime.ToStr(formatStr1, itsCtrlViewDocumentInterface->Language()));
				NFmiString formatStr2 = ::MakeUsedTimeboxTimeFormatString(timeLabelInfo.TimeStringInfo2().itsTimeFormat, currentBetaProduct);
				NFmiString timeStr2(usedTimeboxTime.ToStr(formatStr2, itsCtrlViewDocumentInterface->Language()));
				double singleViewGridHeightInMM = graphicalInfo.itsViewHeightInMM;
				// lasketaan piirto alueen (yhden kartta ikkunan) mm koon mukainen koko kerroin, niin ett‰ aikateksteist‰ ei tule aina joka tapauksessa saman kokoisia
				double sizeFactor = MathHelper::InterpolateWithTwoPoints(singleViewGridHeightInMM,
					timeLabelInfo.ViewSize1(),
					timeLabelInfo.ViewSize2(),
					timeLabelInfo.SizeFactor1(),
					timeLabelInfo.SizeFactor2(),
					timeLabelInfo.MinFactor(),
					timeLabelInfo.MaxFactor()
				);
				auto* mapViewDescTop = itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface().getMapViewDescTop(itsMapViewDescTopIndex);
				auto timeBoxTextSizeFactor = mapViewDescTop->TimeBoxTextSizeFactor();
				double font1SizeInMM = timeLabelInfo.TimeStringInfo1().itsFontSizeInMM * sizeFactor * timeBoxTextSizeFactor;
				font1SizeInMM = FmiMax(timeLabelInfo.AbsoluteMinFontSizeInMM(), FmiMin(font1SizeInMM, timeLabelInfo.AbsoluteMaxFontSizeInMM()));
				int font1Size = static_cast<int>(font1SizeInMM * graphicalInfo.itsPixelsPerMM_y * 1.88);
				double font2SizeInMM = timeLabelInfo.TimeStringInfo2().itsFontSizeInMM * sizeFactor * timeBoxTextSizeFactor;
				font2SizeInMM = FmiMax(timeLabelInfo.AbsoluteMinFontSizeInMM(), FmiMin(font2SizeInMM, timeLabelInfo.AbsoluteMaxFontSizeInMM()));
				int font2Size = static_cast<int>(font2SizeInMM * graphicalInfo.itsPixelsPerMM_y * 1.88);

				Gdiplus::StringFormat stringFormat;
				stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
				stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
				std::wstring fontNameStr1 = CtrlView::StringToWString(timeLabelInfo.TimeStringInfo1().itsFontName);
				Gdiplus::Font aFont1(fontNameStr1.c_str(), static_cast<Gdiplus::REAL>(font1Size), timeLabelInfo.TimeStringInfo1().fBold ? Gdiplus::FontStyleBold : Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
				std::wstring wString1 = CtrlView::StringToWString(std::string(static_cast<char*>(timeStr1)));

				std::wstring fontNameStr2 = CtrlView::StringToWString(timeLabelInfo.TimeStringInfo2().itsFontName);
				Gdiplus::Font aFont2(fontNameStr2.c_str(), static_cast<Gdiplus::REAL>(font2Size), timeLabelInfo.TimeStringInfo2().fBold ? Gdiplus::FontStyleBold : Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
				std::wstring wString2 = CtrlView::StringToWString(std::string(static_cast<char*>(timeStr2)));

				// lasketaan pikseli maailmassa aika boxin koko ja sijainti
				Gdiplus::RectF boundingBox1;
				itsGdiPlusGraphics->MeasureString(wString1.c_str(), INT(wString1.size()), &aFont1, Gdiplus::PointF(0, 0), &stringFormat, &boundingBox1);
				Gdiplus::RectF boundingBox2;
				itsGdiPlusGraphics->MeasureString(wString2.c_str(), INT(wString2.size()), &aFont2, Gdiplus::PointF(0, 0), &stringFormat, &boundingBox2);
				NFmiRect timeBox;
				timeBox.Size(NFmiPoint(FmiMax(boundingBox1.Width, boundingBox2.Width) * 1.1, (font1Size + font2Size) * 1.15));
				FmiDirection timeBoxLocation = mapViewDescTop->TimeBoxLocation();
				CtrlView::PlaceBoxIntoFrame(timeBox, GetFrame(), itsToolBox, timeBoxLocation);

				Gdiplus::SolidBrush aBrushBox(CtrlView::NFmiColor2GdiplusColor(mapViewDescTop->TimeBoxFillColor()));
				Gdiplus::GraphicsPath aPath;
				Gdiplus::Rect gdiRect(static_cast<INT>(timeBox.Left()), static_cast<INT>(timeBox.Top()), static_cast<INT>(timeBox.Width()), static_cast<INT>(timeBox.Height())); // = CFmiGdiPlusHelpers::Relative2GdiplusRect(itsToolBox, timeBox);
				itsTimeBoxRelativeRect = CtrlView::GdiplusRect2Relative(itsToolBox, gdiRect);
				aPath.AddRectangle(gdiRect);
				aPath.CloseFigure();
				itsGdiPlusGraphics->FillPath(&aBrushBox, &aPath);

				Gdiplus::REAL penThickness = static_cast<Gdiplus::REAL>(timeLabelInfo.BoxFrameThicknessInMM() * graphicalInfo.itsPixelsPerMM_y);
				Gdiplus::Pen penBox(CtrlView::NFmiColor2GdiplusColor(timeLabelInfo.BoxFrameColor()), penThickness);
				itsGdiPlusGraphics->DrawPath(&penBox, &aPath);

				NFmiPoint center = timeBox.Center();
				NFmiPoint topleft = timeBox.TopLeft();

				Gdiplus::PointF timeString1OffSet(static_cast<Gdiplus::REAL>(center.X()), static_cast<Gdiplus::REAL>(topleft.Y())); // t‰m‰ offset on suhteellinen laskettuun aika-string boxiin
				Gdiplus::PointF timeString2OffSet(timeString1OffSet);
				timeString2OffSet.Y += font1Size;

				Gdiplus::SolidBrush aBrushText1(CtrlView::NFmiColor2GdiplusColor(timeLabelInfo.TimeStringInfo1().itsColor));
				itsGdiPlusGraphics->DrawString(wString1.c_str(), static_cast<INT>(wString1.size()), &aFont1, timeString1OffSet, &stringFormat, &aBrushText1);

				Gdiplus::SolidBrush aBrushText2(CtrlView::NFmiColor2GdiplusColor(timeLabelInfo.TimeStringInfo2().itsColor));
				itsGdiPlusGraphics->DrawString(wString2.c_str(), static_cast<INT>(wString2.size()), &aFont2, timeString2OffSet, &stringFormat, &aBrushText2);
				return;
			}
		}
	}

	// Jos ei aikalaatikkoa piirretty syyst‰ tai toisesta, laitetaan tyhj‰ laatikko sen merkiksi
	itsTimeBoxRelativeRect = NFmiRect();
}

bool NFmiStationViewHandler::IsThisActiveViewRow() const
{
	return itsCtrlViewDocumentInterface->AbsoluteActiveViewRow(itsMapViewDescTopIndex) == CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
}

// piirt‰‰ reunat sopivalla v‰rill‰ ja mahdollisesti levitystyˆkalun jutut
// 1. normaali ei aktiivinen v‰ri musta, ohut reuna
// 2. Aktiivinen n‰yttˆ, rivi ja aika punaisella paksulla reunalla
// 3. aktiivinen n‰yttˆ ja rivi ohuella haaleammalla punaisella
void NFmiStationViewHandler::DrawCurrentFrame(NFmiToolBox* theGTB)
{
    if(itsCtrlViewDocumentInterface->BetaProductGenerationRunning())
        return;  // jos Beta product piirto k‰ynniss‰, ei piirret‰ reunoja

    if(itsCtrlViewDocumentInterface->ShowWaitCursorWhileDrawingView() == false)
		return;  // jos jossain ohjelmassa on animaatio menossa, ‰l‰ piirr‰ t‰t‰ framea, koska se v‰lkkyy

    bool activeMapView = true;
    bool activeRow = IsThisActiveViewRow();
	bool activeTime = itsCtrlViewDocumentInterface->ActiveViewTime() == itsTime;
	NFmiPoint penSize(1,1);
	NFmiColor frameColor(0,0,0);
    if(activeMapView && activeRow)
    {
        if(activeTime)
        {
            frameColor = NFmiColor(1, 0, 0);
            penSize = NFmiPoint(2, 2);
        }
        else
            frameColor = NFmiColor(0.8f, 0.2f, 0.2f);
    }

	NFmiDrawingEnvironment envi;
	envi.SetFrameColor(frameColor);
	envi.SetPenSize(penSize);
	NFmiRect rect(GetFrame());
	NFmiRectangle rectangle(rect.TopLeft()
							,rect.BottomRight()
							,0
							,&envi);
	theGTB->Convert(&rectangle);
	DrawFrame(envi);
}

//--------------------------------------------------------
// Update
//--------------------------------------------------------
void NFmiStationViewHandler::Update(void)
{
	if(itsViewList)
	{
        auto mapHandler = itsCtrlViewDocumentInterface->GetMapHandlerInterface(itsMapViewDescTopIndex);
		if(mapHandler->MakeNewBackgroundBitmap())
		{
			itsMapRect = itsRect;
			SetMapAreaAndRect(mapHandler->Area(), itsMapRect);
		}

        auto doGeneralViewUpdates = mapHandler->UpdateMapViewDrawingLayers() || mapHandler->MakeNewBackgroundBitmap();
        NFmiDrawParamList* drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex());
		if(drawParamList && (doGeneralViewUpdates || drawParamList->IsDirty()))
		{
            CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + ": forced to recreate layers", this);
			itsViewList->Clear(true);
			for(drawParamList->Reset(); drawParamList->Next();)
			{
                auto stationView = CreateStationView(drawParamList->Current());
                if(stationView)
				    itsViewList->Add(stationView);
			}
			itsViewList->Time(itsTime); // pit‰‰ asettaa myˆs aika oikein, koska joskus n‰yttˆjen synkkaus menee pieleen ja luultavasti currentti aika joka on j‰‰nyt t‰ss‰ updatessa, joutuu mouseMove:ssa taas luotauksen ajaksi jos k‰ytt‰j‰ v‰‰nt‰‰ hiiri pohjassa karttan‰ytˆss‰
			SetViewListArea();
		}
	}
}

bool NFmiStationViewHandler::IsMouseCursorOverParameterBox(const NFmiPoint & theMouseCursorPlace)
{
    if(ShowParamHandlerView() && itsParamHandlerView->IsIn(theMouseCursorPlace))
        return true;
    else
        return false;
}

//--------------------------------------------------------
// LeftButtonDown
//--------------------------------------------------------
bool NFmiStationViewHandler::LeftButtonDown(const NFmiPoint& thePlace, unsigned long theKey)
{
	fWholeCrossSectionReallyMoved = false;
	itsLastMouseDownRelPlace = thePlace;
    itsCtrlViewDocumentInterface->LeftMouseButtonDown(true);
    itsCtrlViewDocumentInterface->RightMouseButtonDown(false);
    itsCtrlViewDocumentInterface->MouseCaptured(true);
	if(itsViewList && GetFrame().IsInside(thePlace))
	{
		SetThisAsActiveViewRow();
		if(IsMouseCursorOverParameterBox(thePlace)) // param-n‰ytˆn on napattava ensimm‰iseksi hiiren toiminnot!!!!!!!!
		{
            return MakeParamHandlerViewActions([&]() {return itsParamHandlerView->LeftButtonDown(thePlace, theKey); });
		}
		else if(IsRangeMeterModeOn(false))
		{
			if(!(theKey & kCtrlKey))
			{
				// Raahaus aloitetaan vain jos ei ole CTRL nappula pohjassa
				SetRangeMeterDragStart(thePlace);
			}
		}
		else if(itsCtrlViewDocumentInterface->ModifyToolMode() == CtrlViewUtils::kFmiEditorModifyToolModeBrush)
		{
			NFmiRect updateRect(itsCtrlViewDocumentInterface->UpdateRect());
			updateRect.Center(thePlace);
            itsCtrlViewDocumentInterface->UpdateRect(updateRect);
		}
		else if(IsControlPointModeOn())
		{
		}
		else if(AllowCrossSectionPointManipulations())
		{
            LeftButtonDownCrossSectionActions(thePlace, theKey);
		}
	}
	return false;
}

bool NFmiStationViewHandler::IsRangeMeterModeOn(bool checkAlsoIfMouseDragIsOn) const
{
	auto& mapViewRangeMeter = itsCtrlViewDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	auto status1 = mapViewRangeMeter.ModeOn();
	// Oletuksena status2 on tosi, jolloin vain status1 arvolla on merkitys && tarkastelussa
	auto status2 = true;
	if(checkAlsoIfMouseDragIsOn)
	{
		status2 = mapViewRangeMeter.MouseDragOn();
	}
	auto status3 = !mapViewRangeMeter.LockModeOn();

	return status1 && status2 && status3;
}

void NFmiStationViewHandler::SetRangeMeterDragStart(const NFmiPoint& thePlace)
{
	auto& mapViewRangeMeter = itsCtrlViewDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	mapViewRangeMeter.MouseDragOn(true);
	mapViewRangeMeter.DragStartLatlonPoint(ViewPointToLatLon(thePlace));
}

void NFmiStationViewHandler::SetRangeMeterDragEnd(const NFmiPoint& thePlace, bool mouseDragging)
{
	auto& mapViewRangeMeter = itsCtrlViewDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	// Metodissa MouseDragin voi vain laittaa off tilaan
	if(!mouseDragging)
	{
		mapViewRangeMeter.MouseDragOn(false);
	}
	mapViewRangeMeter.DragEndLatlonPoint(ViewPointToLatLon(thePlace));
}

void NFmiStationViewHandler::MoveRangeMeterStart(const NFmiPoint& thePlace)
{
	auto& mapViewRangeMeter = itsCtrlViewDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	// Laitetaan MouseDrag varmuuden vuoksi off tilaan
	mapViewRangeMeter.MouseDragOn(false);
	mapViewRangeMeter.MoveStartLatlonPoint(ViewPointToLatLon(thePlace));
}

void NFmiStationViewHandler::LeftButtonDownCrossSectionActions(const NFmiPoint& thePlace, unsigned long )
{
    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
    crossSectionSystem->DragWholeCrossSection(false); // varmistus asetus
    NFmiCrossSectionSystem::CrossMode mode = crossSectionSystem->GetCrossMode();
    if(mode == NFmiCrossSectionSystem::kNormal || mode == NFmiCrossSectionSystem::kRoute)
    {
        double maxRelDist = 0.03;
        NFmiPoint relPoint(LatLonToViewPoint(crossSectionSystem->StartPoint()));
        if(!(::fabs(relPoint.X() - thePlace.X()) < maxRelDist && ::fabs(relPoint.Y() - thePlace.Y()) < maxRelDist))
        { // jos klikattu piste oli tarpeeksi kaukana aloituspisteest‰
            crossSectionSystem->DragWholeCrossSection(true);
            crossSectionSystem->LastMousePosition(thePlace);
        }
    }
}


void NFmiStationViewHandler::SelectLocations(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint& theLatLon
									 ,int theSelectionCombineFunction
									 ,unsigned long theMask
									 ,bool fMakeMTAModeAdd // vain tietyist‰ paikoista kun t‰t‰ metodia kutsutaan, saa luotauksen lis‰t‰ (left buttom up karttan‰ytˆll‰ l‰hinn‰)
									 ,bool fDoOnlyMTAModeAdd)
{
	itsCtrlViewDocumentInterface->SelectLocations(itsMapViewDescTopIndex, theInfo, itsMapArea, theLatLon, itsTime, theSelectionCombineFunction, theMask, fMakeMTAModeAdd, fDoOnlyMTAModeAdd);
}

void NFmiStationViewHandler::DoTotalLocationSelection(const NFmiPoint & thePlace, const NFmiPoint &theLatlon, unsigned long theKey, bool fDrawSelectionOnThisView)
{
	try
	{
		if(itsViewList->NumberOfItems() > 0) // jos yksikin n‰yttˆ listassa, hoidetaan hiiren klikkaus siell‰
		{
			// haluamme piirt‰‰ vain t‰ss‰ haarassa tietyiss‰ tilanteissa valitut pisteet pyyhk‰isy ikkunaan, ei muulloin
			// eli 1. Kun ollaan mouse movessa ja 2. kun ruudulla on jotain dataa, mutta EI kun ollaan oikeassa yl‰kulmassa, koska siihen piirret‰‰n jo muutenkin automaattisesti
			if(itsViewGridRowNumber == 1 && itsViewGridColumnNumber == itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).X())
                itsCtrlViewDocumentInterface->DrawSelectionOnThisView(false);
			else
                itsCtrlViewDocumentInterface->DrawSelectionOnThisView(fDrawSelectionOnThisView);
			itsViewList->LeftButtonUp(thePlace, theKey);
		}
		else
		{ // lis‰sin koodin NFmiStationView-luokasta, ett‰ paikan valinnat onnistuisivat vaikka mit‰‰n dataa ei ole
		  // valittuna karttan‰ytˆlle. Nyt voidaan tyhj‰‰ karttaa klikkailla ja valita pisteit‰ esim. luotaus-n‰ytˆlle.
		  // HUOM! t‰‰lt‰ ei saa valittuja pisteit‰ piirretty‰ karttan‰yttˆˆn.
			boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->EditedSmartInfo();
			if(info)
			{
				if(itsCtrlViewDocumentInterface->ModifyToolMode() != CtrlViewUtils::kFmiEditorModifyToolModeBrush && !IsControlPointModeOn()) // siveltimen kanssa ei voi valita asemia???
				{
					dynamic_cast<NFmiSmartInfo*>(info.get())->LocationSelectionSnapShot();

					if(theKey & kCtrlKey)
						SelectLocations(info, theLatlon, kFmiSelectionCombineAdd, NFmiMetEditorTypes::kFmiSelectionMask, false);
					else if(theKey & kShiftKey)
						SelectLocations(info, theLatlon, kFmiSelectionCombineRemove, NFmiMetEditorTypes::kFmiSelectionMask, false);
					else
						SelectLocations(info, theLatlon, kFmiSelectionCombineClearFirst, NFmiMetEditorTypes::kFmiSelectionMask, true);
				}
			}
		}
    }
	catch(...)
	{ // haluan vain varmistaa ett‰ asetus menee lopuksi pois p‰‰lt‰
        itsCtrlViewDocumentInterface->DrawSelectionOnThisView(false);
		throw;
	}
    itsCtrlViewDocumentInterface->DrawSelectionOnThisView(false);
}

//--------------------------------------------------------
// LeftButtonUp
//--------------------------------------------------------
bool NFmiStationViewHandler::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
    // mouse captured pit‰‰ hanskata, vaikka hiiri olisi itsParamHandlerView -ikkunan ulkona
    if(ShowParamHandlerView() && itsParamHandlerView->IsMouseCaptured())
    {
        return MakeParamHandlerViewActions([&]() {return itsParamHandlerView->LeftButtonUp(thePlace, theKey); });
    }

    if(itsViewList && GetFrame().IsInside(thePlace))
	{
		SetThisAsActiveViewRow();

        itsCtrlViewDocumentInterface->ActiveViewTime(itsTime);
		bool ctrlKeyDown = (theKey & kCtrlKey);
		if(IsMouseCursorOverParameterBox(thePlace)) // napattava ensimm‰iseksi hiiren toiminnot!!!!!!!!
		{
            return MakeParamHandlerViewActions([&]() {return itsParamHandlerView->LeftButtonUp(thePlace, theKey); });
		}
		else if(IsRangeMeterModeOn(!ctrlKeyDown))
		{
			if(!ctrlKeyDown)
			{
				// Raahaus lopetetaan vain jos ei ole CTRL nappula pohjassa
				SetRangeMeterDragEnd(thePlace, false);
				return true;
			}
			else
			{
				MoveRangeMeterStart(thePlace);
				return true;
			}
		}
		else if(IsBrushToolUsed())
		{
            LeftButtonUpBrushToolActions();
		}
		else if(IsControlPointModeOn())
		{
            LeftButtonUpControlPointModeActions(thePlace, theKey);
		}
		else 
		{
            if(AllowCrossSectionPointManipulations())
            {
                if(LeftButtonUpCrossSectionActions(thePlace, theKey))
                    return true;
            }
            NFmiPoint latlon = itsMapArea->ToLatLon(thePlace);
            DoTotalLocationSelection(thePlace, latlon, theKey, false);
        }
		// 8.2.2000/Marko Muutin t‰m‰n palauttamaan aina true:n jos piste on ollut sis‰ll‰
		// toivon, ett‰ outo takkuisuus loppuu t‰ll‰ tavalla, kun klikkaa ruutua, miss‰ ei ole parametreja
		return true;
	}
	return false;
}

// Jos palauttaa true, ei saa jatkaa kutsuvassa metodissa hilapisteiden valintaan.
bool NFmiStationViewHandler::LeftButtonUpCrossSectionActions(const NFmiPoint& thePlace, unsigned long theKey)
{
    NFmiPoint latlon = itsMapArea->ToLatLon(thePlace);
    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView);
	if(theKey & kCtrlKey)
    { // CTRL-pohjassa aktivoidaan l‰hin minor piste
        crossSectionSystem->ActivateNearestMinorPoint(latlon);
        itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
        return true; // ei menn‰ hilapisteen valintaan
    }
    else
    {
        if(fWholeCrossSectionReallyMoved && crossSectionSystem->DragWholeCrossSection())
        {
            crossSectionSystem->DragWholeCrossSection(false);
            itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
        }
        else
        {
            crossSectionSystem->StartPoint(latlon);
            // t‰ytyy myˆs mahdollistaa pelk‰n luotaus paikan valinta kun ollaan poikkileikkaus moodissa
            if(itsCtrlViewDocumentInterface->GetMTATempSystem().TempViewOn())
                SelectLocations(boost::shared_ptr<NFmiFastQueryInfo>(), latlon, kFmiSelectionCombineClearFirst, NFmiMetEditorTypes::kFmiSelectionMask, true, true);

            itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
        }
    }
    return false; // Jatketaan hilapisteen valintaan
}

// Jotta hiirell‰ voidaan manipuloida poikkileikkausn‰ytˆn pisteit‰, pit‰‰ seuraavat ehdot pit‰‰ paikkaansa:
// 1. Poikkileikkausn‰ytˆn pit‰‰ olla p‰‰ll‰.
// 2. Siin‰ ei saa olla trajektori -moodi p‰‰ll‰, koska silloin ei voi s‰‰t‰‰ aloituspisteit‰.
// 3. Karttan‰ytˆn pit‰‰ olla moodissa miss‰ pisteet n‰ytet‰‰n kartalla.
// 4. TƒMƒ EI ENƒƒ KƒYT÷SSƒ: Karttaruudun pit‰‰ olla vasemmalla ylh‰‰ll‰ (1. ruutu), koska vain siihen piirret‰‰n kyseiset pisteet.
bool NFmiStationViewHandler::AllowCrossSectionPointManipulations()
{
    if(IsCrossSectionViewStuffShownOnThisMapView() && !itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
    {
        if(itsCtrlViewDocumentInterface->ShowCrossSectionMarkersOnMap(itsMapViewDescTopIndex))
        {
//            if(itsViewGridRowNumber == 1 && itsViewGridColumnNumber == 1) // vain 1. rivin ensimm‰iseen ruutuun sallitaan crosssection klikkaukset
            {
                return true;
            }
        }
    }
    return false;
}

void NFmiStationViewHandler::LeftButtonUpControlPointModeActions(const NFmiPoint& thePlace, unsigned long theKey)
{
    auto cpManager = itsCtrlViewDocumentInterface->CPManager();
    if(!cpManager->MouseCaptured())
    {
        NFmiPoint latlon(itsMapArea->ToLatLon(thePlace));
        cpManager->ActivateCP(latlon, true);
        if((theKey & kShiftKey) && (theKey & kCtrlKey))
            cpManager->AddCP(latlon);
        else if((theKey & kShiftKey))
            cpManager->EnableCP(!cpManager->IsEnabledCP());
    }
    cpManager->MouseCaptured(false);
    // P‰ivityst‰ vaatii vain t‰m‰ karttan‰yttˆ, p‰‰karttan‰yttˆ ja aikasarjaikkuna
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex) | SmartMetViewId::MainMapView | SmartMetViewId::TimeSerialView);
}

bool NFmiStationViewHandler::IsControlPointModeOn()
{
    return itsCtrlViewDocumentInterface->MetEditorOptionsData().ControlPointMode() && itsCtrlViewDocumentInterface->ShowControlPointsOnMap(itsMapViewDescTopIndex);
}

void NFmiStationViewHandler::LeftButtonUpBrushToolActions()
{
    boost::shared_ptr<NFmiDrawParam> drawParam = itsCtrlViewDocumentInterface->ActiveDrawParamFromActiveRow(itsMapViewDescTopIndex);
    if(drawParam)
    {
        itsCtrlViewDocumentInterface->CheckAndValidateAfterModifications(NFmiMetEditorTypes::kFmiBrush, false, NFmiMetEditorTypes::kFmiNoMask, FmiParameterName(drawParam->Param().GetParam()->GetIdent()));
        itsCtrlViewDocumentInterface->MapViewCache(itsMapViewDescTopIndex).MakeTimeDirty(itsTime);
        // Kutsutaan MapViewDirty funktiota, jotta voidaan liata macroParam datat, jotka ovat riippuvaisia editoidusta datasta
        itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, false, true, false, true, false);
        std::string paramName = "[" + drawParam->ParameterAbbreviation() + "]";
        CatLog::logMessage(paramName + " - modified with Brush tool.", CatLog::Severity::Info, CatLog::Category::Editing, true);
        // P‰ivitet‰‰n pensseli vedon lopuksi vain karttan‰yttˆj‰ (voisi p‰ivitt‰‰ viel‰ aikasarja ja taulukkon‰ytˆn, mutta optimoidaan)
        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    }
}

bool NFmiStationViewHandler::IsBrushToolUsed()
{
    if(itsCtrlViewDocumentInterface->ModifyToolMode() == CtrlViewUtils::kFmiEditorModifyToolModeBrush && itsCtrlViewDocumentInterface->ViewBrushed())
        return true;
    else
        return false;
}

bool NFmiStationViewHandler::LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
        if(IsMouseCursorOverParameterBox(thePlace)) // napattava ensimm‰iseksi hiiren toiminnot!!!!!!!!
        {
            return MakeParamHandlerViewActions([&]() {return itsParamHandlerView->LeftDoubleClick(thePlace, theKey); });
        }

		if(itsViewList && itsViewList->IsIn(thePlace))
		{
			bool result = itsViewList->LeftDoubleClick(thePlace, theKey);
            if(result)
            {
    			return result;
            }
            else
            {
                itsCtrlViewDocumentInterface->ZoomMapInOrOut(itsMapViewDescTopIndex, itsMapArea, thePlace, 0.85);
			    return true;
            }
		}
	}
	return false;
}

bool NFmiStationViewHandler::RightDoubleClick(const NFmiPoint &thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		if(itsViewList && itsViewList->IsIn(thePlace))
		{
			bool result = itsViewList->RightDoubleClick(thePlace, theKey);
            if(result)
            {
    			return result;
            }
            else
            {
                itsCtrlViewDocumentInterface->ZoomMapInOrOut(itsMapViewDescTopIndex, itsMapArea, thePlace, 1.15);
			    return true;
            }
		}
	}
	return false;
}

bool NFmiStationViewHandler::IsCrossSectionSystemDisableingNormalMiddleMouseButtonUse() const
{
	if(IsCrossSectionViewStuffShownOnThisMapView())
	{
	    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
		if(crossSectionSystem->CrossSectionMode() == NFmiCrossSectionSystem::k3Point)
		{
			NFmiCrossSectionSystem::CrossMode crossMode = crossSectionSystem->GetCrossMode();
			if(crossMode == NFmiCrossSectionSystem::kNormal || crossMode == NFmiCrossSectionSystem::kRoute)
				return true; // t‰llˆin keski hiiren nappi pit‰‰ j‰tt‰‰ poikkileikkausn‰ytˆn k‰yttˆˆn!!
		}
	}
	return false;
}

bool NFmiStationViewHandler::MiddleButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(itsViewList && GetFrame().IsInside(thePlace))
	{
		if(!IsCrossSectionSystemDisableingNormalMiddleMouseButtonUse())
		{ 
			// poikkileikkaus moodi ei saa olla p‰‰ll‰ kun l‰hdet‰‰n tekem‰‰n zoomia
			itsCtrlViewDocumentInterface->MiddleMouseButtonDown(true);
			itsZoomDragDownPoint = thePlace;
			itsZoomDragUpPoint = thePlace;
			itsOldZoomRect = NFmiRect(itsZoomDragDownPoint, itsZoomDragUpPoint);
			SetThisAsActiveViewRow();
			if(theKey & kCtrlKey)
				itsCtrlViewDocumentInterface->MapMouseDragPanMode(true);
			else
                itsCtrlViewDocumentInterface->MapMouseDragPanMode(false);
		}
	}
	return false;
}

bool NFmiStationViewHandler::MiddleButtonUp(const NFmiPoint & thePlace, unsigned long /* theKey */ )
{
	if(itsViewList && GetFrame().IsInside(thePlace))
	{
        itsCtrlViewDocumentInterface->MapMouseDragPanMode(false);
        if(AllowCrossSectionPointManipulations() && IsCrossSectionSystemDisableingNormalMiddleMouseButtonUse()) // eli k‰ytet‰‰n sitten keski nappia poikkileikkausn‰ytˆlle
        {
            itsCtrlViewDocumentInterface->CrossSectionSystem()->MiddlePoint(itsMapArea->ToLatLon(thePlace));
            itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
            return true; // ei menn‰ hilapisteen valintaan
        }
		else if(itsCtrlViewDocumentInterface->MiddleMouseButtonDown() && itsCtrlViewDocumentInterface->MouseCaptured())
		{ // tehd‰‰ varsinainen zoomaus sitten
			if(itsCtrlViewDocumentInterface->MapMouseDragPanMode()) // en tied‰ miksi t‰ll‰inen ehto on, koska juuri aiemmin on t‰m‰ laitettu false:ksi
			{
                itsCtrlViewDocumentInterface->MiddleMouseButtonDown(false);
			}
			else
			{
                itsCtrlViewDocumentInterface->MiddleMouseButtonDown(false);
				itsDrawingEnvironment.EnableInvert();
				NFmiRectangle rec1(itsOldZoomRect, 0, &itsDrawingEnvironment);
				itsToolBox->Convert(&rec1);
				itsDrawingEnvironment.DisableInvert();
				double minWidthPix = itsToolBox->SX(30); // zoomi laatikon pit‰‰ olla v‰hint‰‰n tietyn pikseli m‰‰r‰n kokoinen
				double minHeightPix = itsToolBox->SY(30);
				if(itsOldZoomRect.Width() > minWidthPix && itsOldZoomRect.Height() > minHeightPix)
				{
                    itsCtrlViewDocumentInterface->ZoomMapWithRelativeRect(itsMapViewDescTopIndex, itsMapArea, itsOldZoomRect);
				}
			}
            UpdateOnlyThisMapViewAtNextGeneralViewUpdate();
			return true;
		}
	}
	return false;
}

void NFmiStationViewHandler::UpdateOnlyThisMapViewAtNextGeneralViewUpdate()
{
    itsCtrlViewDocumentInterface->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(itsMapViewDescTopIndex); // optimointia
}

bool NFmiStationViewHandler::HandleTimeBoxMouseWheel(const NFmiPoint& thePlace, unsigned long theKey, short theDelta)
{
	auto *mapViewDescTop = itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface().getMapViewDescTop(itsMapViewDescTopIndex);
	if(mapViewDescTop)
	{
		if((theKey & kCtrlKey) && (theKey & kShiftKey))
		{
			// CTRL + SHIFT pohjassa tehd‰‰n taustav‰rin alpha kanavan s‰‰tˆ‰
			auto origAlpha = mapViewDescTop->GetTimeBoxFillColorAlpha();
			auto newAlphaValue = origAlpha + (theDelta > 0 ? 0.05f : -0.05f);
			// Arvon asetus pit‰‰ tehd‰ CombinedMapHandler:in kautta, jotta muutos menisi myˆs WinRegistry:n ja sitten n‰yttˆmakroihin
			itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface().onSetTimeBoxFillColorAlpha(itsMapViewDescTopIndex, newAlphaValue);
			// Palautetaan true, jos kerroin on oikeasti muuttunut
			return origAlpha != mapViewDescTop->GetTimeBoxFillColorAlpha();
		}
		if((theKey & kCtrlKey))
		{
			// CTRL pohjassa tehd‰‰n kokokertoimen s‰‰tˆ‰
			auto origTextSizeFactor = mapViewDescTop->TimeBoxTextSizeFactor();
			auto newValue = origTextSizeFactor + (theDelta > 0 ? 0.1f : -0.1f);
			// Arvon asetus pit‰‰ tehd‰ CombinedMapHandler:in kautta, jotta muutos menisi myˆs WinRegistry:n ja sitten n‰yttˆmakroihin
			itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface().onSetTimeBoxTextSizeFactor(itsMapViewDescTopIndex, newValue);
			// Palautetaan true, jos kerroin on oikeasti muuttunut
			return origTextSizeFactor != mapViewDescTop->TimeBoxTextSizeFactor();
		}
	}
	return false;
}

bool NFmiStationViewHandler::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(itsViewList && GetFrame().IsInside(thePlace))
	{
		if(itsTimeBoxRelativeRect.IsInside(thePlace))
		{
			return MakeParamHandlerViewActions([&]() {return HandleTimeBoxMouseWheel(thePlace, theKey, theDelta); });
		}
		if(IsMouseCursorOverParameterBox(thePlace))
		{
			return MakeParamHandlerViewActions([&]() {return itsParamHandlerView->MouseWheel(thePlace, theKey, theDelta); });
		}
		if((theKey & kCtrlKey) && (theKey & kShiftKey))
		{
            // S‰‰det‰‰n kaikkia ruudulla olevia level/satel kanavia jne. kerrallaan
            bool status = false;
            for(itsViewList->Reset(); itsViewList->Next(); ) // hybrid data muutos ensin
            {
                status |= ChangeHybridDataLevel((NFmiStationView*)itsViewList->Current(), theDelta); // muuttaa mallipinta ja painepintaa
                status |= ChangeSatelDataChannel((NFmiStationView*)itsViewList->Current(), theDelta); // sitten mahd. satel image kanava muutos
            }
            MakeParamLevelChangeDirtyOperations(status); // jos yksikin level vaihtui, p‰ivitet‰‰n ikkunat
            return status;
        }
		else if(theKey & kCtrlKey) // jos ctrl-nappi pohjassa zoomataan karttaa
		{
            itsCtrlViewDocumentInterface->ZoomMapInOrOut(itsMapViewDescTopIndex, itsMapArea, thePlace, (theDelta > 0) ? 0.95 : 1.05);
			return true;
		}
		else if(theKey & kShiftKey) // jos shift-nappi pohjassa muutetaan aktiivisen piirto-layerin hybrid-datojen leveli‰ ylˆs/alas tai satel datan kanavaa
		{
			bool status = false;
			for(itsViewList->Reset(); itsViewList->Next(); ) // hybrid data muutos ensin
			{
				if(itsViewList->Current()->DrawParam()->IsActive())
				{
					status |= ChangeHybridDataLevel((NFmiStationView*)itsViewList->Current(), theDelta); // muuttaa mallipinta ja painepintaa
					status |= ChangeSatelDataChannel((NFmiStationView*)itsViewList->Current(), theDelta); // sitten mahd. satel image kanava muutos
					break;
				}
			}
            MakeParamLevelChangeDirtyOperations(status); // jos yksikin level vaihtui, p‰ivitet‰‰n ikkunat
			return status; 
		}
		else
		{
			if(theDelta < 0)
				return itsCtrlViewDocumentInterface->ScrollViewRow(itsMapViewDescTopIndex, 1);
			else
				return itsCtrlViewDocumentInterface->ScrollViewRow(itsMapViewDescTopIndex, -1);
		}
	}
	return false;
}

void NFmiStationViewHandler::MakeParamLevelChangeDirtyOperations(bool changesHappened)
{
    if(changesHappened)
    {
        itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, true);
        auto cacheRowIndex = CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber) - 1;
        itsCtrlViewDocumentInterface->MapViewCache(itsMapViewDescTopIndex).MakeRowDirty(cacheRowIndex);
    }
}

void NFmiStationViewHandler::InitParamHandlerView(void)
{
	CtrlView::GeneralInitParamHandlerView(this, itsParamHandlerView, true, true);
}

void NFmiStationViewHandler::UpdateParamHandlerView(void)
{
	CtrlView::GeneralUpdateParamHandlerView(this, itsParamHandlerView);
}

NFmiRect NFmiStationViewHandler::CalcParamHandlerViewRect()
{
	return CtrlView::GeneralCalcParamHandlerViewRect(this, GetFrame());
}

void NFmiStationViewHandler::SetParamHandlerViewRect(const NFmiRect& newRect)
{
	itsParamHandlerViewRect = newRect;
}

bool NFmiStationViewHandler::ShowParamHandlerView(void)
{
	if(itsParamHandlerView)
	{
		if(itsCtrlViewDocumentInterface->IsParamWindowViewVisible(itsMapViewDescTopIndex))
		{
            CtrlViewUtils::MapViewMode displayMode = itsCtrlViewDocumentInterface->MapViewDisplayMode(itsMapViewDescTopIndex);
			if(displayMode == CtrlViewUtils::MapViewMode::kOneTime)
				return true; // OneTime-moodissa jokaisessa ruudussa on paramview
			else if(displayMode == CtrlViewUtils::MapViewMode::kRunningTime)
			{
				if(itsViewGridRowNumber == 1 && itsViewGridColumnNumber == 1)
					return true; // RunningTime-moodissa vain 1. rivin 1. ruutu saa paramviewin
			}
			else
			{
				if(itsViewGridColumnNumber == 1)
					return true; // normaali tai muissa moodeissa vain 1. sarakkeen ruudut saavat paramviewin
			}
		}
	}
	return false;
}

void NFmiStationViewHandler::DrawParamView(NFmiToolBox* theGTB)
{
	if(ShowParamHandlerView())
	{
		bool doBetaProductParameterBox = itsCtrlViewDocumentInterface->BetaProductGenerationRunning();
		bool doBetaParamBoxDuePrinting = (itsCtrlViewDocumentInterface->Printing() && IsPrintedMapViewDesctop());
		if(doBetaProductParameterBox || doBetaParamBoxDuePrinting)
		{
			const NFmiBetaProduct* optionalPrintingBetaProduct = doBetaParamBoxDuePrinting ? &StationViews::GetPrintingBetaProductForParamBoxDraw(itsMapViewDescTopIndex, *itsCtrlViewDocumentInterface) : nullptr;

			StationViews::DrawBetaProductParamBox(this, false, optionalPrintingBetaProduct);
		}
		else
		{
			UpdateParamHandlerView();
			itsParamHandlerView->Draw(theGTB);
		}
	}
}

bool NFmiStationViewHandler::ChangeSatelDataChannel(NFmiStationView* theView, short theDelta)
{
	if(theView)
	{
		boost::shared_ptr<NFmiDrawParam> drawParam = theView->DrawParam();
		if(drawParam && drawParam->DataType() == NFmiInfoData::kSatelData)
		{
			if(theDelta < 0)
				drawParam->Param(itsCtrlViewDocumentInterface->HelpDataInfoSystem()->GetNextSatelChannel(drawParam->Param(), kForward));
			else
				drawParam->Param(itsCtrlViewDocumentInterface->HelpDataInfoSystem()->GetNextSatelChannel(drawParam->Param(), kBackward));
			drawParam->ParameterAbbreviation(static_cast<char*>(drawParam->Param().GetParamName()));
			return true;
		}
	}
	return false;
}

// Katsoo onko kyseisen karttan‰ytˆn data hybrid ja nyt myˆs painepinta dataa.
// Jos on, yritt‰‰ vaihtaa leveli‰ hiiren rullauksen mukaan ylˆs/alas.
// Palauttaa true, jos level vaihtui.
bool NFmiStationViewHandler::ChangeHybridDataLevel(NFmiStationView* theView, short theDelta)
{
	if(theView)
	{
		boost::shared_ptr<NFmiDrawParam> drawParam = theView->DrawParam();
		if(drawParam)
		{
			boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(drawParam, false, true);
			if(info)
			{
				if(info->SizeLevels() > 1)
				{
					FmiLevelType levType = drawParam->Level().LevelType();
					if(levType != kFmiSoundingLevel)
					{
						if(theDelta < 0)
						{
							if(info->NextLevel())
							{
								drawParam->Level(*info->Level());
								return true;
							}
							else // pyˆr‰ytet‰‰n levelit ymp‰ri jos ollaan tultu loppuun
							{
								info->FirstLevel();
								drawParam->Level(*info->Level());
								return true;
							}
						}
						else
						{
							if(info->PreviousLevel())
							{
								drawParam->Level(*info->Level());
								return true;
							}
							else // pyˆr‰ytet‰‰n levelit ymp‰ri jos ollaan tultu alkuun
							{
								info->LastLevel();
								drawParam->Level(*info->Level());
								return true;
							}
						}
					}
					else
					{ // luotaus datalla on omat level s‰‰dˆt
						NFmiVPlaceDescriptor soundingPlotLevels(itsCtrlViewDocumentInterface->SoundingPlotLevels());
						soundingPlotLevels.Level(drawParam->Level());
						if(theDelta < 0)
						{
							if(soundingPlotLevels.Next())
							{
								drawParam->Level(*soundingPlotLevels.Level());
								return true;
							}
							else // pyˆr‰ytet‰‰n levelit ymp‰ri jos ollaan tultu loppuun
							{
								soundingPlotLevels.Reset();
								soundingPlotLevels.Next();
								drawParam->Level(*soundingPlotLevels.Level());
								return true;
							}
						}
						else
						{
							if(soundingPlotLevels.Previous())
							{
								drawParam->Level(*soundingPlotLevels.Level());
								return true;
							}
							else // pyˆr‰ytet‰‰n levelit ymp‰ri jos ollaan tultu alkuun
							{
								soundingPlotLevels.Index(soundingPlotLevels.Size() - 1); // ei ollut last-matodia
								drawParam->Level(*soundingPlotLevels.Level());
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

void NFmiStationViewHandler::SetThisAsActiveViewRow() const
{
	itsCtrlViewDocumentInterface->AbsoluteActiveViewRow(itsMapViewDescTopIndex, CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));
	// Uuden ParameterSelection-dialogin tarvitsemia asetuksia laitetaan t‰ll‰ uudella funktiolla.
	itsCtrlViewDocumentInterface->SetLastActiveDescTopAndViewRow(itsMapViewDescTopIndex, GetUsedParamRowIndex());
}

//--------------------------------------------------------
// RightButtonUp
//--------------------------------------------------------
bool NFmiStationViewHandler::RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(itsViewList && GetFrame().IsInside(thePlace))
	{
		// Ensin timebox hanskaus jos hiiren oikea klikattu
		if(itsTimeBoxRelativeRect.IsInside(thePlace))
		{
			return itsCtrlViewDocumentInterface->CreateMapViewTimeBoxPopup(itsMapViewDescTopIndex);
		}
		// Sitten pit‰‰ handlata parametrin lis‰ys param boxista 
        if(IsMouseCursorOverParameterBox(thePlace))
        {
            return MakeParamHandlerViewActions([&]() {return itsParamHandlerView->RightButtonUp(thePlace, theKey); });
        }

		SetThisAsActiveViewRow();
		NFmiPoint latlon = itsMapArea->ToLatLon(thePlace);
        itsCtrlViewDocumentInterface->ActiveViewTime(itsTime);
		if(IsControlPointModeOn())
		{
			return itsCtrlViewDocumentInterface->CreateCPPopup();
		}
		if(itsCtrlViewDocumentInterface->ModifyToolMode() == CtrlViewUtils::kFmiEditorModifyToolModeBrush && itsCtrlViewDocumentInterface->ViewBrushed())
		{
			boost::shared_ptr<NFmiDrawParam> drawParam = itsCtrlViewDocumentInterface->ActiveDrawParamFromActiveRow(itsMapViewDescTopIndex);
			if(drawParam)
			{
                itsCtrlViewDocumentInterface->CheckAndValidateAfterModifications(NFmiMetEditorTypes::kFmiBrush, false, NFmiMetEditorTypes::kFmiNoMask, FmiParameterName(drawParam->Param().GetParam()->GetIdent()));
                itsCtrlViewDocumentInterface->MapViewCache(itsMapViewDescTopIndex).MakeTimeDirty(itsTime);
			}
            std::string paramName = "[" + drawParam->ParameterAbbreviation() + "]";
            CatLog::logMessage(paramName + " - modified with Brush tool.", CatLog::Severity::Info, CatLog::Category::Editing, true);
		}
        else if(AllowCrossSectionPointManipulations())
        {
            itsCtrlViewDocumentInterface->CrossSectionSystem()->EndPoint(latlon);
            // t‰ytyy myˆs mahdollistaa pelk‰n luotaus paikan valinta kun ollaan poikkileikkaus moodissa
            if(itsCtrlViewDocumentInterface->GetMTATempSystem().TempViewOn())
                SelectLocations(boost::shared_ptr<NFmiFastQueryInfo>(), latlon, kFmiSelectionCombineClearFirst, NFmiMetEditorTypes::kFmiDisplayedMask, true, true);
			ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::CrossSectionView);
			itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
            return true; // ei menn‰ hilapisteen valintaan
        }
		if(itsViewList->NumberOfItems() > 0) // jos yksikin n‰yttˆ listassa, hoidetaan hiiren klikkaus siell‰
			return itsViewList->RightButtonUp(thePlace, theKey);
		else
		{ // lis‰sin koodin NFmiStationView-luokasta (vastaava metodi), ett‰ paikan valinnat onnistuisivat vaikka mit‰‰n dataa ei ole
		  // valittuna karttan‰ytˆlle. Nyt voidaan tyhj‰‰ karttaa klikkailla ja valita pisteit‰ esim. luotaus-n‰ytˆlle.
		  // HUOM! t‰‰lt‰ ei saa valittuja pisteit‰ piirretty‰ karttan‰yttˆˆn.
			boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->EditedSmartInfo();
			if(info)
			{
				if((theKey & kCtrlKey) && (theKey & kShiftKey))
				{
					// ctrl+shift+right-mouseclick:in avulla voidaan valita aktiivinen n‰yttˆrivi ja aika ilman, ett‰ valinnat muuttuvat
				}
				else if(itsCtrlViewDocumentInterface->ModifyToolMode() != CtrlViewUtils::kFmiEditorModifyToolModeBrush) // siveltimen kanssa ei voi valita asemia???
				{
					dynamic_cast<NFmiSmartInfo*>(info.get())->LocationSelectionSnapShot();

					if(theKey & kCtrlKey)
						SelectLocations(info, latlon, kFmiSelectionCombineAdd, NFmiMetEditorTypes::kFmiDisplayedMask, false);
					else if(theKey & kShiftKey)
						SelectLocations(info, latlon, kFmiSelectionCombineRemove, NFmiMetEditorTypes::kFmiDisplayedMask, false);
					else
						SelectLocations(info, latlon, kFmiSelectionCombineClearFirst, NFmiMetEditorTypes::kFmiDisplayedMask, true);
				}
			}
		}
	}
	return false;
}

bool NFmiStationViewHandler::RightButtonDown(const NFmiPoint & thePlace, unsigned long /* theKey */ )
{
	itsLastMouseDownRelPlace = thePlace;
	return false;
}

bool NFmiStationViewHandler::IsMouseDraggingOn(void)
{
    if(itsParamHandlerView && itsParamHandlerView->IsMouseDraggingOn())
        return true;
    if(itsViewList && itsViewList->IsMouseDraggingOn())
        return true;
    return false;
}

bool NFmiStationViewHandler::MouseDragZooming(const NFmiPoint &thePlace)
{
	if(itsCtrlViewDocumentInterface->MapMouseDragPanMode())
	{ // T‰ss‰ on kartan 'pannaus' hoidettu
        itsCtrlViewDocumentInterface->PanMap(itsMapViewDescTopIndex, itsMapArea, thePlace, itsZoomDragUpPoint);
		itsZoomDragUpPoint = thePlace;
	}
	else
	{ // t‰ss‰ tehd‰‰n kartan p‰‰lle vedetty‰ zoomi laatikkoa
		itsZoomDragUpPoint = thePlace;
		itsDrawingEnvironment.EnableInvert();
		itsDrawingEnvironment.DisableFill();
		NFmiRectangle rec1(itsOldZoomRect, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&rec1);
		NFmiRect zoomRect(itsZoomDragDownPoint, itsZoomDragUpPoint);
		NFmiRectangle rec2(zoomRect, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&rec2);
		itsDrawingEnvironment.DisableInvert();
		itsOldZoomRect = zoomRect;
	}
	return true;
}

bool NFmiStationViewHandler::MouseMoveBrushAction(const NFmiPoint &thePlace)
{
	if(itsCtrlViewDocumentInterface->MouseCaptured() && IsIn(thePlace)) // turha, on jo kysytty?
	{
		boost::shared_ptr<NFmiDrawParam> drawParam = itsCtrlViewDocumentInterface->ActiveDrawParamFromActiveRow(itsMapViewDescTopIndex);
		if(drawParam && drawParam->DataType() == NFmiInfoData::kEditable && (!drawParam->IsParamHidden())) // parametri ei saa myˆs olla piilossa jos sit‰ meinaa alkaa sutimaan!!
		{
			if(drawParam->Param().Type() == kSymbolicParam)
				return false;
			boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(drawParam, false, false);
			if(info && info->Time(itsTime)) // else brushtool
			{
				if(IsThisActiveViewRow() && itsCtrlViewDocumentInterface->ActiveViewTime() == itsTime)
				{
//					if(itsDoc->EditorModeData()->FullControlEditDataTimeBag().IsInside(itsTime)) // tee vastaava funktio dokumenttiin
					{
						try
						{
							DoBrushingUndoRituals(drawParam);
						}
						catch(...)
						{
							// heitetty poikkeus eli halutaan lopettaa toiminto
                            itsCtrlViewDocumentInterface->MouseCaptured(false);
							return false;
						}
                        itsCtrlViewDocumentInterface->ViewBrushed(true);
                        itsCtrlViewDocumentInterface->ActiveViewRect(GetFrame());
						double modifySizeX = (itsCtrlViewDocumentInterface->BrushSize().X()/100.)/ itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).X();
						NFmiPoint latlon(itsMapArea->ToLatLon(thePlace));
						boost::shared_ptr<NFmiAreaMaskList> emptyMaskList(new NFmiAreaMaskList());
						boost::shared_ptr<NFmiAreaMaskList> maskList = itsCtrlViewDocumentInterface->ParamMaskListMT();
						if(drawParam->Param().Type() != kContinuousParam)
						{
							NFmiDataParamModifierAreaCircleSetValue modifier(info, drawParam
											, itsCtrlViewDocumentInterface->UseMaskWithBrush() ? maskList : emptyMaskList
																		,itsMapArea
																		,latlon
																		,modifySizeX / 2.
																		, itsCtrlViewDocumentInterface->BrushSpecialParamValue()
																		,NFmiMetEditorTypes::kFmiNoMask);
							modifier.ModifyData();
						}
						else if(itsCtrlViewDocumentInterface->BrushToolLimitSetting() == 3) // 3 == arvon abs. asetus
						{
							NFmiDataParamModifierAreaCircleSetValue modifier(info, drawParam
											, itsCtrlViewDocumentInterface->UseMaskWithBrush() ? maskList : emptyMaskList
																		,itsMapArea
																		,latlon
																		,modifySizeX / 2.
																		, itsCtrlViewDocumentInterface->BrushToolLimitSettingValue()//BrushValue()
																		,NFmiMetEditorTypes::kFmiNoMask);
							modifier.ModifyData();
						}
						else//(drawParam->Param().Type() == kContinuousParam)
						{
                            boost::shared_ptr<NFmiArea> infoArea(info->Area()->Clone());
							NFmiDataParamModifierAreaConeChange modifier(info, drawParam
											,itsCtrlViewDocumentInterface->UseMaskWithBrush() ? maskList : emptyMaskList
																		,itsMapArea
																		,infoArea
																		,latlon
																		,modifySizeX / 2.
																		,itsCtrlViewDocumentInterface->LeftMouseButtonDown() ? -itsCtrlViewDocumentInterface->BrushValue() : itsCtrlViewDocumentInterface->BrushValue()
																		,NFmiMetEditorTypes::kFmiNoMask // pit‰isi kai olla aktivaatio maski, mutta sit‰ ei k‰ytet‰ kuitenkaan nyt
																		, itsCtrlViewDocumentInterface->BrushToolLimitSetting()
																		, itsCtrlViewDocumentInterface->BrushToolLimitSettingValue());
							modifier.ModifyData2();
						}

						itsCtrlViewDocumentInterface->WindTableSystem().MustaUpdateTable(true);
						NFmiRect rect(0, 0, modifySizeX, modifySizeX);
						rect.Center(thePlace);
						NFmiRect combinedRect(rect.SmallestEnclosing(itsCtrlViewDocumentInterface->UpdateRect()));
						DrawMapInMouseMove(itsToolBox, combinedRect);
						if(itsViewList)
							itsViewList->Draw(itsToolBox);

						NFmiDrawingEnvironment envi;
						envi.DisableFill();
						itsToolBox->DrawEllipse(rect, &envi);

                        itsCtrlViewDocumentInterface->UpdateRect(rect);
					}
				}
			}
		}
	}
	return false;
}

bool NFmiStationViewHandler::MouseMoveControlPointAction(const NFmiPoint &thePlace)
{
	NFmiPoint latlon(itsMapArea->ToLatLon(thePlace));
	boost::shared_ptr<NFmiEditorControlPointManager> CPMan = itsCtrlViewDocumentInterface->CPManager();
	if(CPMan && CPMan->IsNearestPointActivateCP(latlon))
	{
		if(!CPMan->MouseCaptured())
			CPMan->MouseCaptured(true);
		else
		{
			CPMan->MoveCP(latlon);
			DrawControlPoints();
		}
	}
	return false;
}

// HUOM!!! siisti t‰m‰ funktio ja poista turhia juttuja ja siirr‰ osia esim. dociin
bool NFmiStationViewHandler::MouseMove(const NFmiPoint &thePlace, unsigned long theKey)
{
    // mouse captured pit‰‰ hanskata, vaikka hiiri olisi itsParamHandlerView -ikkunan ulkona
    if(ShowParamHandlerView() && itsParamHandlerView->IsMouseCaptured())
    {
        return MakeParamHandlerViewActions([&]() {return itsParamHandlerView->MouseMove(thePlace, theKey); });
    }

    if(!GetFrame().IsInside(thePlace))
        return false;

    if(IsMouseCursorOverParameterBox(thePlace))
        return false; // ei move handlausta, jos ollaa parametri boxin sis‰ll‰, koska muuten ei voi oikea klikill‰ lis‰t‰ parametreja, jos hiiri liikahtaa


    try
    {
        InitializeGdiplus(itsToolBox, &GetFrame());

        if(itsCtrlViewDocumentInterface->MiddleMouseButtonDown() && itsCtrlViewDocumentInterface->MouseCaptured())
            return MouseDragZooming(thePlace); // tehd‰‰n zoomi laatikon piirtoa suoraan karttan‰ytˆlle
		else if(IsRangeMeterModeOn(false))
		{
			if(!(theKey & kCtrlKey))
			{
				// Raahausta tehd‰‰n vain jos ei ole CTRL nappula pohjassa
				SetRangeMeterDragEnd(thePlace, true);
				return true;
			}
			else
				return false;
		}
		else if(itsCtrlViewDocumentInterface->ModifyToolMode() == CtrlViewUtils::kFmiEditorModifyToolModeBrush)
            return MouseMoveBrushAction(thePlace);
        else if(itsCtrlViewDocumentInterface->MouseCaptured() && IsControlPointModeOn())
            return MouseMoveControlPointAction(thePlace);

        if(AllowCrossSectionPointManipulations())
        {
            MouseMoveCrossSectionActions(thePlace, theKey);
        }
        if(itsCtrlViewDocumentInterface->MouseCaptured() && itsCtrlViewDocumentInterface->GetMTATempSystem().TempViewOn())
        {
            NFmiPoint latlon(itsMapArea->ToLatLon(thePlace));
            NFmiMTATempSystem::Container &temps = const_cast<NFmiMTATempSystem::Container &>(itsCtrlViewDocumentInterface->GetMTATempSystem().GetTemps());
            if(temps.size())
            { // siirret‰‰n 1. luotaus hiiren osoittamaan paikkaan
                temps[0].Latlon(latlon);
                if(itsLastMouseDownRelPlace != thePlace)
                    itsCtrlViewDocumentInterface->MustDrawTempView(true);
            }
        }
        NFmiPoint latlon(itsMapArea->ToLatLon(thePlace));
        DoTotalLocationSelection(thePlace, latlon, theKey, true);
        if(itsCtrlViewDocumentInterface->MouseCaptured() && itsCtrlViewDocumentInterface->TimeSerialDataViewOn())
        {
            itsCtrlViewDocumentInterface->MustDrawTimeSerialView(true);
            itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
        }
    }
    catch(std::exception &e)
    {
        itsCtrlViewDocumentInterface->LogAndWarnUser(std::string("Error in NFmiStationViewHandler::Draw: ") + e.what(), "", CatLog::Severity::Error, CatLog::Category::Visualization, true);
    }
    catch(...)
    {
    }
    CleanGdiplus(); // t‰t‰ pit‰‰ kutsua piirron lopuksi InitializeGdiplus -metodin vastin parina.
    return false;
}

void NFmiStationViewHandler::MouseMoveCrossSectionActions(const NFmiPoint& thePlace, unsigned long theKey)
{
    NFmiPoint latlon(itsMapArea->ToLatLon(thePlace));
    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
    if(itsCtrlViewDocumentInterface->LeftMouseButtonDown())
    {
        if(crossSectionSystem->DragWholeCrossSection())
        {
            NFmiPoint moveBy(thePlace - crossSectionSystem->LastMousePosition());
            NFmiPoint relStartPoint(LatLonToViewPoint(crossSectionSystem->StartPoint()));
            NFmiPoint relMiddlePoint(LatLonToViewPoint(crossSectionSystem->MiddlePoint()));
            NFmiPoint relEndPoint(LatLonToViewPoint(crossSectionSystem->EndPoint()));
            relStartPoint += moveBy;
            relMiddlePoint += moveBy;
            relEndPoint += moveBy;
            NFmiPoint startLatlon(itsMapArea->ToLatLon(relStartPoint));
            NFmiPoint middleLatlon(itsMapArea->ToLatLon(relMiddlePoint));
            NFmiPoint endLatlon(itsMapArea->ToLatLon(relEndPoint));
            crossSectionSystem->StartPoint(startLatlon);
            crossSectionSystem->MiddlePoint(middleLatlon);
            crossSectionSystem->EndPoint(endLatlon);
            crossSectionSystem->LastMousePosition(thePlace);
            itsCtrlViewDocumentInterface->MustDrawCrossSectionView(true);
            fWholeCrossSectionReallyMoved = true;
        }
        else
        {
            crossSectionSystem->StartPoint(latlon);
            itsCtrlViewDocumentInterface->MustDrawCrossSectionView(true);
        }
    }
    else if(itsCtrlViewDocumentInterface->RightMouseButtonDown())
    {
        crossSectionSystem->EndPoint(latlon);
        itsCtrlViewDocumentInterface->MustDrawCrossSectionView(true);
    }
}

//--------------------------------------------------------
// Time
//--------------------------------------------------------
void NFmiStationViewHandler::Time(const NFmiMetTime& theTime)
{
	NFmiCtrlView::Time(theTime);
	if(itsViewList)
		itsViewList->Time(theTime);
}
//--------------------------------------------------------
// DrawSelectedLocations
//--------------------------------------------------------
void NFmiStationViewHandler::DrawSelectedLocations(void)
{
	if(itsCtrlViewDocumentInterface->ShowSelectedPointsOnMap(itsMapViewDescTopIndex) == false)
		return ;

    if(itsViewList->NumberOfItems())
    {   // Jos yksikin parametri valittuna n‰ytˆlle, piirret‰‰n sen avulla valitut pisteet
        for(itsViewList->Reset(); itsViewList->Next();)
        {
            ((NFmiStationView*)itsViewList->Current())->DrawAllSelectedStationsWithInvertStationRect(NFmiMetEditorTypes::kFmiSelectionMask);
            if(((NFmiStationView*)itsViewList->Current())->DrawAllSelectedStationsWithInvertStationRect(NFmiMetEditorTypes::kFmiDisplayedMask))
                break;
        }
    }
    else
    {   // Tarvittaessa luodaan yksi n‰yttˆ, jonka avulla piirret‰‰n valitut pisteet
        NFmiStationView stationView(itsMapViewDescTopIndex, GetArea(), itsToolBox, itsDrawParam, kFmiTemperature, NFmiPoint(0, 0), NFmiPoint(1, 1), itsViewGridRowNumber, itsViewGridColumnNumber);
        stationView.Time(itsTime);
        stationView.DrawAllSelectedStationsWithInvertStationRect(NFmiMetEditorTypes::kFmiSelectionMask);
        stationView.DrawAllSelectedStationsWithInvertStationRect(NFmiMetEditorTypes::kFmiDisplayedMask);
    }
}

// t‰m‰ pit‰‰ yhdist‰‰ NFmiStationView:n CP piirtoon, eik‰ pidet‰ erill‰‰n!!!!
void NFmiStationViewHandler::DrawControlPoints(void)
{
	if(itsCtrlViewDocumentInterface->ShowControlPointsOnMap(itsMapViewDescTopIndex) == false)
		return ;

	boost::shared_ptr<NFmiEditorControlPointManager> CPMan = itsCtrlViewDocumentInterface->CPManager();
	if(CPMan)
	{
		int pixels = 1 + 2*(itsToolBox->HY(sqrt(itsMapArea->Width() * itsMapArea->Height()))/55);
		pixels = min(11, pixels);
		pixels = max(4, pixels);
		float height = static_cast<float>(itsToolBox->SY(pixels));
		float width = static_cast<float>(itsToolBox->SX(pixels));
		NFmiRect CPRect(NFmiPoint(0, 0), NFmiPoint(width, height));
		itsDrawingEnvironment.SetFontType(kArial);
		NFmiPoint oldPenSize = itsDrawingEnvironment.GetPenSize();
		NFmiPoint normalRectPenSize(2,2);
		NFmiPoint activeRectPenSize(3,3);
		boost::shared_ptr<NFmiArea> zoomedArea = itsCtrlViewDocumentInterface->GetMapHandlerInterface(itsMapViewDescTopIndex)->Area();

		for(CPMan->ResetCP(); CPMan->NextCP();)
		{
			NFmiPoint latLonPoint(CPMan->LatLon());
			if(!zoomedArea->IsInside(latLonPoint))
				continue;
			NFmiPoint xy(itsMapArea->ToXY(latLonPoint));
			CPRect.Center(xy);

			itsDrawingEnvironment.EnableFill();
			if(CPMan->IsActivateCP())
			{
				itsDrawingEnvironment.SetFrameColor(NFmiColor(1.f,0.f,0.f));
				itsDrawingEnvironment.SetFillColor(NFmiColor(0.f,0.f,1.f));
				itsDrawingEnvironment.SetPenSize(activeRectPenSize);
			}
			else
			{
				itsDrawingEnvironment.SetFrameColor(NFmiColor(0.3f,0.7f,0.1f));
				itsDrawingEnvironment.SetFillColor(NFmiColor(0.2f,0.1f,0.7f));
				itsDrawingEnvironment.SetPenSize(normalRectPenSize);
			}
			if(!CPMan->IsEnabledCP())
			{
				itsDrawingEnvironment.DisableFill();
			}
			NFmiRectangle rectangle(CPRect, 0, &itsDrawingEnvironment);
			itsToolBox->Convert(&rectangle);
			if(CPMan->IsCPMovingInTime())
			{
				NFmiRect ellipseRect(NFmiPoint(0,0), NFmiPoint(width*0.8, height*0.8));
				ellipseRect.Center(xy);
				itsDrawingEnvironment.SetFrameColor(NFmiColor(0.98f,0.987f,0.0115f));
				itsToolBox->DrawEllipse(ellipseRect, &itsDrawingEnvironment);
			}
		}
		itsDrawingEnvironment.SetPenSize(oldPenSize);
		DrawControlPointData();
        DrawCPCropArea();
    }
}

void NFmiStationViewHandler::DrawControlPointData(void)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
			((NFmiStationView*)itsViewList->Current())->DrawControlPointData();
	}

}

static bool CheckIsTotalCloudinessUnitInProcents(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = theCtrlViewDocumentInterface->InfoOrganizer()->Info(theDrawParam, false, true);
	if(info)
	{
		int nonMissingValues = 0;
		for(info->ResetTime(); info->NextTime(); )
		{
			for(info->ResetLocation(); info->NextLocation(); )
			{
				float value = info->FloatValue();
				if(value != kFloatMissing)
				{
					nonMissingValues++;
					if(value >= 10.f)
						return true;
				}
			}
			if(nonMissingValues > 200)
				return false; // jos ei ole ollut 200 ei-puuttuvaan arvoon yht‰‰n yli 10 arvoa, oletetaan ett‰ kok.pilv. on octina
		}
		return false;
	}
	return true;
}

// T‰m‰ funktio toimii vain wind-arrow ja wind-vector tyyppien kanssa.
static bool IsWantedWindViewType(boost::shared_ptr<NFmiDrawParam>& drawParam, boost::shared_ptr<NFmiFastQueryInfo>& info, NFmiMetEditorTypes::View wantedViewType)
{
	// Jos GridDataPresentationStyle on haluttua tyyppia, se riitt‰‰, koska wind-vec ja arrow tyypit ovat erikoisuus
	// ja se asetus on k‰ytetty vaikka kyse olisi asemadatasta.
	auto gridDataIsWantedType = drawParam->GridDataPresentationStyle() == wantedViewType;
	if(gridDataIsWantedType)
		return true;
	auto stationDataIsWantedType = (info && info->IsGrid() == false) && (drawParam->StationDataViewType() == wantedViewType);
	if(stationDataIsWantedType)
		return true;
	// Jos hiladata halutaan visualisoida halutulla wind piirtotyylill‰, nyt sekin onnistuu
	auto dataIsVisualizedWithWantedSymbols = drawParam->GridDataPresentationStyle() == NFmiMetEditorTypes::View::kFmiTextView && (drawParam->StationDataViewType() == wantedViewType);
	return dataIsVisualizedWithWantedSymbols;
}

//--------------------------------------------------------
// CreateStationView
//--------------------------------------------------------
NFmiStationView * NFmiStationViewHandler::CreateStationView(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	NFmiStationView *stationView = 0;
	if(theDrawParam)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(theDrawParam, false, true);
		NFmiPoint dataOffSet(0,0);
		NFmiPoint dataSize(20,20);
		FmiParameterName param = FmiParameterName(theDrawParam->Param().GetParam()->GetIdent());
        bool isGridData = info ? (info->IsGrid() == true) : false;
        bool useTextView = theDrawParam->GridDataPresentationStyle() == NFmiMetEditorTypes::View::kFmiTextView;
		auto dataType = theDrawParam->DataType();

        if(param == NFmiInfoData::kFmiSpSynoPlot || param == NFmiInfoData::kFmiSpSoundingPlot || param == NFmiInfoData::kFmiSpMinMaxPlot || param == NFmiInfoData::kFmiSpMetarPlot) // jos synop plottia halutaan tai sounding-plottia tai min/max plottia (9996)
		{
			stationView = new NFmiSynopPlotView(itsMapViewDescTopIndex, itsMapArea
											 ,itsToolBox
											 ,theDrawParam
											 ,param
											 ,itsViewGridRowNumber
                                             ,itsViewGridColumnNumber);
		}
		else if(dataType == NFmiInfoData::kSatelData) // param >= kFmiSatelCh1 && param <= kFmiSatelCh345)
		{
			stationView = new NFmiSatelView(itsMapViewDescTopIndex, itsMapArea
											 ,itsToolBox
											 ,theDrawParam
											 ,param
                                             ,itsViewGridRowNumber
                                             ,itsViewGridColumnNumber);
        }
		else if(dataType == NFmiInfoData::kConceptualModelData) // rintaman piirto
		{
			stationView = new NFmiConceptualDataView(itsMapViewDescTopIndex, itsMapArea
													,itsToolBox
													,theDrawParam
													,param
                                                    , itsViewGridRowNumber
                                                    , itsViewGridColumnNumber);
        }
        else if(dataType == NFmiInfoData::kCapData) // Capin piirto
        {
            stationView = new NFmiCapView(itsMapViewDescTopIndex, itsMapArea
                , itsToolBox
                , theDrawParam
                , param
                , itsViewGridRowNumber
                , itsViewGridColumnNumber);
        }
        else if(dataType == NFmiInfoData::kWmsData) // WMS:n piirto
        {
#ifndef DISABLE_CPPRESTSDK
            stationView = new NFmiWmsView(itsMapViewDescTopIndex, itsMapArea
                , itsToolBox
                , theDrawParam
                , param
                , itsViewGridRowNumber
                , itsViewGridColumnNumber);
#endif // DISABLE_CPPRESTSDK
        }
		else if(dataType == NFmiInfoData::kMapLayer) // country border viivojen piirto tavallisessa NFmiStationView -oliossa
		{
			stationView = new NFmiStationView(itsMapViewDescTopIndex, itsMapArea
				, itsToolBox
				, theDrawParam
				, param
				, dataOffSet
				, dataSize
				, itsViewGridRowNumber
				, itsViewGridColumnNumber);
		}
		else if(dataType == NFmiInfoData::kFlashData || (theDrawParam->Param().GetProducer()->GetIdent() == kFmiFlashObs && theDrawParam->Param().GetParamIdent() == kFmiFlashStrength ))
		{
			stationView = new NFmiFlashDataView(itsMapViewDescTopIndex, itsMapArea
											 ,itsToolBox
											 ,theDrawParam
											 ,param
											 ,dataOffSet
											 ,dataSize
                                            , itsViewGridRowNumber
                                            , itsViewGridColumnNumber);

		}
		else if(::IsWantedWindViewType(theDrawParam, info, NFmiMetEditorTypes::View::kFmiArrowView))
		{
			stationView = new NFmiStationArrowView(itsMapViewDescTopIndex, itsMapArea
											 ,itsToolBox
											 ,theDrawParam
											 ,param
											 ,dataOffSet
											 ,dataSize
                                            , itsViewGridRowNumber
                                            , itsViewGridColumnNumber);
        }
        else if(param == NFmiInfoData::kFmiSpStreamline)
        {
			stationView = new NFmiStreamLineView(itsMapViewDescTopIndex, itsMapArea
																	,itsToolBox
																	,theDrawParam
																	,param
                                                                    , itsViewGridRowNumber
                                                                    , itsViewGridColumnNumber);
        }
        else if(((isGridData && useTextView) || (!isGridData)) && theDrawParam->StationDataViewType() == NFmiMetEditorTypes::View::kFmiPrecipFormSymbolView)
        { // Jos hila-piirto on symboli piirto ja asema-piirtona on valittu sateenolomuoto, piirret‰‰n data sateen olomuoto n‰ytˆll‰, vaikka kyse olisi hiladatasta.
			stationView = new NFmiPrecipitationFormSymbolTextView(itsMapViewDescTopIndex, itsMapArea
																	,itsToolBox
																	,theDrawParam
																	,param
																	,itsCtrlViewDocumentInterface->DataLists()->PrecipitationFormSymbolList()
																	,dataOffSet
																	,dataSize
                                                                    , itsViewGridRowNumber
                                                                    , itsViewGridColumnNumber);
        }
        else if(((isGridData && useTextView) || (!isGridData)) && theDrawParam->StationDataViewType() == NFmiMetEditorTypes::View::kFmiRawMirriFontSymbolView)
        { // Jos hila-piirto on symboli piirto ja asema-piirtona on valittu mirri-font symbol, piirret‰‰n data mirri font symboleilla niiden suorilla raaka arvoilla.
            stationView = new NFmiRawMirriFontSymbolTextView(itsMapViewDescTopIndex, itsMapArea
                , itsToolBox
                , theDrawParam
                , param
                , itsCtrlViewDocumentInterface->DataLists()->PrecipitationFormSymbolList()
                , dataOffSet
                , dataSize
                , itsViewGridRowNumber
                , itsViewGridColumnNumber);
        }
        else if(((isGridData && useTextView) || (!isGridData)) && theDrawParam->StationDataViewType() == NFmiMetEditorTypes::View::kFmiBetterWeatherSymbolView)
        { // Jos hila-piirto on symboli piirto ja asema-piirtona on valittu mirri-font symbol, piirret‰‰n data mirri font symboleilla niiden suorilla raaka arvoilla.
            stationView = new NFmiBetterWeatherSymbolView(itsMapViewDescTopIndex, itsMapArea
                , itsToolBox
                , theDrawParam
                , param
                , itsCtrlViewDocumentInterface->DataLists()->PrecipitationFormSymbolList()
                , dataOffSet
                , dataSize
                , itsViewGridRowNumber
                , itsViewGridColumnNumber);
        }
        else if(((isGridData && useTextView) || (!isGridData)) && theDrawParam->StationDataViewType() == NFmiMetEditorTypes::View::kFmiSmartSymbolView)
        {
            stationView = new NFmiSmartSymbolView(itsMapViewDescTopIndex, itsMapArea
                , itsToolBox
                , theDrawParam
                , param
                , itsCtrlViewDocumentInterface->DataLists()->PrecipitationFormSymbolList()
                , dataOffSet
                , dataSize
                , itsViewGridRowNumber
                , itsViewGridColumnNumber);
        }
        else if(((isGridData && useTextView) || (!isGridData)) && theDrawParam->StationDataViewType() == NFmiMetEditorTypes::View::kFmiCustomSymbolView)
        {
            stationView = new NFmiCustomSymbolView(itsMapViewDescTopIndex, itsMapArea
                , itsToolBox
                , theDrawParam
                , param
                , itsCtrlViewDocumentInterface->DataLists()->PrecipitationFormSymbolList()
                , dataOffSet
                , dataSize
                , itsViewGridRowNumber
                , itsViewGridColumnNumber);
        }
        else if(((isGridData && useTextView) || (!isGridData)) && theDrawParam->StationDataViewType() == NFmiMetEditorTypes::View::kFmiSynopWeatherSymbolView)
        { // Jos hila-piirto on symboli piirto ja asema-piirtona on valittu sateenolomuoto, piirret‰‰n data synop-s‰‰ symboleilla, vaikka kyse olisi hiladatasta.
			stationView = new NFmiStationIndexTextView(itsMapViewDescTopIndex, itsMapArea
													  ,itsToolBox
													  ,theDrawParam
													  ,param
													  , itsCtrlViewDocumentInterface->DataLists()->ParamPictureList(param)
													  ,dataOffSet
													  ,dataSize
                                                    , itsViewGridRowNumber
                                                    , itsViewGridColumnNumber);
        }
		else if(::IsWantedWindViewType(theDrawParam, info, NFmiMetEditorTypes::View::kFmiWindVectorView))
		{
			stationView = new NFmiStationWindBarbView(itsMapViewDescTopIndex, itsMapArea
													 ,itsToolBox
													 ,theDrawParam
													 ,param
													 ,dataOffSet
													 ,dataSize
                                                    , itsViewGridRowNumber
                                                    , itsViewGridColumnNumber);
        }
		else if((!useTextView) && info && info->IsGrid())
		{
			stationView = new NFmiIsoLineView(itsMapViewDescTopIndex, itsMapArea
											 ,itsToolBox
											 ,theDrawParam
											 ,param
											 ,dataOffSet
											 ,dataSize
											 ,itsViewGridRowNumber
                                             ,itsViewGridColumnNumber);
		}
		else if(theDrawParam->StationDataViewType() != NFmiMetEditorTypes::View::kFmiTextView && info && info->IsGrid() == false)
		{ // eli tarvittaessa myˆs asema data voidaan haluta piirt‰‰ isoviivoina tai contoureina
			stationView = new NFmiIsoLineView(itsMapViewDescTopIndex, itsMapArea
											 ,itsToolBox
											 ,theDrawParam
											 ,param
											 ,dataOffSet
											 ,dataSize
											 , itsViewGridRowNumber
                                             , itsViewGridColumnNumber);
        }
		else
		{
			switch(param)
			{
			case kFmiWindVectorMS:
			stationView = new NFmiStationWindBarbView(itsMapViewDescTopIndex, itsMapArea
													 ,itsToolBox
													 ,theDrawParam
													 ,param
													 ,dataOffSet
													 ,dataSize
                                                    , itsViewGridRowNumber
                                                    , itsViewGridColumnNumber);
            break;

			case kFmiWeatherSymbol3:
			stationView = new NFmiStationSimpleWeatherView(itsMapViewDescTopIndex, itsMapArea
														  ,itsToolBox
														  ,theDrawParam
														  ,param
														  ,dataOffSet
														  ,dataSize
                                                          , itsViewGridRowNumber
                                                          , itsViewGridColumnNumber);
            break;

			case kFmiWeatherSymbol1:
			case kFmiWeatherSymbol2:
			case kFmiPresentWeather:
			case kFmiAviationWeather1:
			case kFmiAviationWeather2:
			case kFmiAviationWeather3:
			stationView = new NFmiStationIndexTextView(itsMapViewDescTopIndex, itsMapArea
													  ,itsToolBox
													  ,theDrawParam
													  ,param
													  , itsCtrlViewDocumentInterface->DataLists()->ParamPictureList(param)
													  ,dataOffSet
													  ,dataSize
                                                    , itsViewGridRowNumber
                                                    , itsViewGridColumnNumber);
            break;

			case kFmiFogIntensity:
			stationView = new NFmiStationFogTextView(itsMapViewDescTopIndex, itsMapArea
													  ,itsToolBox
													  ,theDrawParam
													  ,param
													  , itsCtrlViewDocumentInterface->DataLists()->ParamPictureList(kFmiWeatherSymbol1) // k‰ytet‰‰n HSADE1 listaa, mist‰ symboli haetaan
													  ,dataOffSet
													  ,dataSize
                                                    , itsViewGridRowNumber
                                                    , itsViewGridColumnNumber);
            break;

            case kFmiPrecipitationForm:
            case kFmiPotentialPrecipitationForm:
				stationView = new NFmiPrecipitationFormSymbolTextView(itsMapViewDescTopIndex, itsMapArea
																	  ,itsToolBox
																	  ,theDrawParam
																	  ,param
																	  , itsCtrlViewDocumentInterface->DataLists()->PrecipitationFormSymbolList() //
																	  ,dataOffSet
																	  ,dataSize
                                                                    , itsViewGridRowNumber
                                                                    , itsViewGridColumnNumber);
                break;

			case kFmiCloudSymbol:
				stationView = new NFmiCloudSymbolTextView(itsMapViewDescTopIndex, itsMapArea
													  ,itsToolBox
													  ,theDrawParam
													  ,param
													  , itsCtrlViewDocumentInterface->DataLists()->PrecipitationFormSymbolList() // t‰ll‰ ei ole v‰li‰, siell‰ lasketaan symbolit toista kautta
													  ,dataOffSet
													  ,dataSize
                                                      , itsViewGridRowNumber
                                                      , itsViewGridColumnNumber);
                break;

			case kFmiPastWeather1:
			case kFmiPastWeather2:
				stationView = new NFmiTotalCloudinessSymbolTextView(itsMapViewDescTopIndex, itsMapArea
													  ,itsToolBox
													  ,theDrawParam
													  ,param
													  , itsCtrlViewDocumentInterface->DataLists()->PastWeatherSymbolList() //
													  ,dataOffSet
													  ,dataSize
                                                    , itsViewGridRowNumber
                                                    , itsViewGridColumnNumber);
                break;

			default:
				{
				// kokonais pilvisyys piti laittaa viel‰ speciaali tapaukseksi, koska synop-datassa pilvisyys on oktina ja hila datassa prosentteina
					unsigned long prodId = theDrawParam->Param().GetProducer()->GetIdent();
					bool synopProducer = (prodId == kFmiSYNOP || prodId == kFmiSHIP || prodId == kFmiMETAR);
					bool totalCloudinessParam = (param == kFmiTotalCloudCover || param == kFmi1CloudCover || param == kFmi2CloudCover || param == kFmi3CloudCover || param == kFmi4CloudCover);
					bool totCloudUnitInProcents = (synopProducer && totalCloudinessParam) ? ::CheckIsTotalCloudinessUnitInProcents(theDrawParam, itsCtrlViewDocumentInterface) : true;
					if(synopProducer && totalCloudinessParam && totCloudUnitInProcents == false)
						stationView = new NFmiTotalCloudinessSymbolTextView(itsMapViewDescTopIndex, itsMapArea
														,itsToolBox
														,theDrawParam
														,param
														, itsCtrlViewDocumentInterface->DataLists()->TotalCloudinessSymbolList() // k‰ytet‰‰n HSADE1 listaa, mist‰ symboli haetaan
														,dataOffSet
														,dataSize
                                                        , itsViewGridRowNumber
                                                        , itsViewGridColumnNumber);
                    else if(synopProducer && (param == kFmiLowCloudType || param == kFmiMiddleCloudType || param == kFmiHighCloudType))
						stationView = new NFmiClCmChSymbolTextView(itsMapViewDescTopIndex, itsMapArea
														,itsToolBox
														,theDrawParam
														,param
														,dataOffSet
														,dataSize
                                                        , itsViewGridRowNumber
                                                        , itsViewGridColumnNumber);
                    else
						stationView = new NFmiStationTextView(itsMapViewDescTopIndex, itsMapArea
															,itsToolBox
															,theDrawParam
															,param
															,dataOffSet
															,dataSize
                                                            , itsViewGridRowNumber
                                                            , itsViewGridColumnNumber);
                    break;
				}
			}
		}
	}
	return stationView;
}
//--------------------------------------------------------
// SetMapAreaAndRect
//--------------------------------------------------------
void NFmiStationViewHandler::SetMapAreaAndRect(const boost::shared_ptr<NFmiArea> &theArea, const NFmiRect& theRect)
{
	if(theArea)
	{
        itsMapArea = boost::shared_ptr<NFmiArea>(theArea->Clone());
		itsMapRect = theRect;
		itsRect = theRect; // t‰m‰ on emon dataa!!!!
		itsMapArea->SetXYArea(itsMapRect); // siirt‰‰ suhteellist‰ aluetta
	}
}

//--------------------------------------------------------
// DrawMap
//--------------------------------------------------------
void NFmiStationViewHandler::DrawMap(NFmiToolBox * theGTB, const NFmiRect& theRect)
{
	if(theGTB)
	{
		auto* mapDc = itsCtrlViewDocumentInterface->MapBlitDC(itsMapViewDescTopIndex);
		if(!itsCtrlViewDocumentInterface->Printing() && mapDc)
		{
			theGTB->DrawDC(mapDc, theRect);
		}
		else 
		{
			// Else on hitaampi tapa , jota k‰ytet‰‰n ainakin printtauksen yhteydess‰
            auto mapHandler = itsCtrlViewDocumentInterface->GetMapHandlerInterface(itsMapViewDescTopIndex);
			Gdiplus::Bitmap *aBitmap = mapHandler->GetBitmap();
			if(aBitmap)
			{
				NFmiRect bitmapRect = mapHandler->ZoomedAbsolutRect();
				CRect mfcRect;
				theGTB->ConvertRect(itsMapRect, mfcRect);
				Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(mfcRect.left), static_cast<Gdiplus::REAL>(mfcRect.top), static_cast<Gdiplus::REAL>(mfcRect.Width()), static_cast<Gdiplus::REAL>(mfcRect.Height()));
                CtrlView::DrawBitmapToDC_4(theGTB->GetDC(), *aBitmap, bitmapRect, destRect, true, NFmiImageAttributes(), itsGdiPlusGraphics);
			}
		}
		if(itsCtrlViewDocumentInterface->ProjectionCurvatureInfo()->GetDrawingMode() == NFmiProjectionCurvatureInfo::kOverMap)
			DrawProjetionLines(theGTB);
	}
}

//--------------------------------------------------------
// DrawOverMap Eli t‰m‰ kuva piirret‰‰n yli perus kartan ja datan
//--------------------------------------------------------
void NFmiStationViewHandler::DrawOverMap(NFmiToolBox * theGTB, const NFmiRect& theRect)
{
	if(theGTB)
	{
        auto mfcRect = MapDraw::getMfcRect(itsMapRect, itsToolBox);
        NFmiPoint bitmapSize(mfcRect.Width(), mfcRect.Height());
        auto destRect = MapDraw::getDestRect(mfcRect);

        int wantedDrawOverMapMode = 1; // means overlay is drawn after all the dynamic data is drawn
        MapDraw::drawOverlayMap(itsCtrlViewDocumentInterface, itsMapViewDescTopIndex, wantedDrawOverMapMode, theGTB->GetDC(), destRect, bitmapSize, itsGdiPlusGraphics);
	}
}

void NFmiStationViewHandler::DrawProjetionLines(NFmiToolBox * theGTB)
{
	if(theGTB)
	{
		theGTB->SetTextAlignment(kLeft);
		FmiFontType oldFont = itsDrawingEnvironment.GetFontType();
		itsDrawingEnvironment.SetFontType(kArial);
        try
        {
            auto projInfo = itsCtrlViewDocumentInterface->ProjectionCurvatureInfo();
            if(projInfo->GetDrawingMode() != NFmiProjectionCurvatureInfo::kNoDraw)
            {
                double usedFontSizeInMM = 2.0;
                NFmiPoint fontSize(CalcFontSize(usedFontSizeInMM));
                double usedFontSizeInRelativeUnit = itsMapArea->Width() * usedFontSizeInMM / itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsViewHeightInMM;
                projInfo->CalcProjectionLinesAndLabels(itsMapArea.get(), fontSize, usedFontSizeInRelativeUnit);
                theGTB->DrawValueLineList(&projInfo->GetProjectionLines(), &itsDrawingEnvironment, itsMapRect); // envi ja rect annetaan turhaan, koska funktio haluaa niit‰!!!
                std::list<std::shared_ptr<NFmiText>> &labels = projInfo->GetProjectionLineLabels();
                std::list<std::shared_ptr<NFmiText>>::iterator it = labels.begin();
                for(; it != labels.end(); ++it)
                    theGTB->Convert(&(*it->get()));
            }
        }
        catch(...)
        {
//            itsCtrlViewDocumentInterface->LogAndWarnUser("Unknown exception in NFmiStationViewHandler::DrawProjetionLines", "", NFmiLogger::kError, true);
        }
		itsDrawingEnvironment.SetFontType(oldFont);
	}
}

// muuttaa millimetrin pikselin kooksi
double NFmiStationViewHandler::MM2PixelSize(double theWantedMM)
{
	CDC* dc = itsToolBox->GetDC();
	int oldMappingMode = dc->SetMapMode(MM_HIMETRIC);

	CSize size(static_cast<int>(theWantedMM*100.), static_cast<int>(theWantedMM*100.));
	dc->HIMETRICtoDP(&size);

	dc->SetMapMode(oldMappingMode);
	return size.cy;
}

NFmiPoint NFmiStationViewHandler::CalcFontSize(double theWantedSizeInMM)
{
	double fontSizeInPixels = theWantedSizeInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x * 1.88;
	fontSizeInPixels = round(fontSizeInPixels);
	return NFmiPoint(fontSizeInPixels, fontSizeInPixels);
}

void NFmiStationViewHandler::DrawMapInMouseMove(NFmiToolBox * theGTB, const NFmiRect& theRect)
{
	if(theGTB)
	{
		auto* mapDc = itsCtrlViewDocumentInterface->MapBlitDC(itsMapViewDescTopIndex);
		if(mapDc)
		{
			NFmiPoint place(theRect.Place());
			place -= GetFrame().Place();
			NFmiPoint size(1,1); // koolla ei ole todellisuudessa merkityst‰
			NFmiRect sourceRect(place, size);
			theGTB->DrawDC(mapDc, theRect, sourceRect);
		}
		else
		{
			// t‰nne ei pit‰isi menn‰!!!!!
		}
	}
}

void NFmiStationViewHandler::SetViewListArea(void)
{
	if(itsViewList && itsMapArea)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
		{
			itsViewList->Current()->SetArea(itsMapArea);
		}
	}
}

// muuttaa annetun (esim. hiirell‰ osoitetun) suhteellisen (0,0 1,1) pisteen oikeaksi
// kartan latlon pisteeksi ja tallettaa latlon-pisteen ja ajan dokumenttiin
void NFmiStationViewHandler::StoreToolTipDataInDoc(const NFmiPoint& theRelativePlace)
{
    itsCtrlViewDocumentInterface->ToolTipLatLonPoint(itsMapArea->ToLatLon(theRelativePlace));
    itsCtrlViewDocumentInterface->ToolTipTime(itsTime);
    itsCtrlViewDocumentInterface->ToolTipRealRowIndex(CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));
    itsCtrlViewDocumentInterface->ToolTipColumnIndex(itsViewGridColumnNumber);
	itsCtrlViewDocumentInterface->ToolTipMapViewDescTopIndex(itsMapViewDescTopIndex);
}

void NFmiStationViewHandler::DoBrushingUndoRituals(boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
    if(itsCtrlViewDocumentInterface->HasActiveViewChanged())
    {
        if(theDrawParam)
        {
            boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(theDrawParam, false, false);
            if(info)
                itsCtrlViewDocumentInterface->SnapShotData(info, theDrawParam->Param(), "Brush tool modification", itsTime, itsTime);
        }
        itsCtrlViewDocumentInterface->LastBrushedViewTime(itsTime);
        itsCtrlViewDocumentInterface->LastBrushedViewRealRowIndex(CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));
    }
}

static NFmiRect ClipRect(const NFmiRect &theClipper, const NFmiRect &theRect)
{
	static NFmiRect empty;
	if(theRect.Left() > theClipper.Right())
		return empty;
	if(theRect.Top() > theClipper.Bottom())
		return empty;
	if(theRect.Right() < theClipper.Left())
		return empty;
	if(theRect.Bottom() < theClipper.Top())
		return empty;

	double left = theRect.Left() > theClipper.Left() ? theRect.Left() : theClipper.Left();
	double top = theRect.Top() > theClipper.Top() ? theRect.Top() : theClipper.Top();
	double right = theRect.Right() < theClipper.Right() ? theRect.Right() : theClipper.Right();
	double bottom = theRect.Bottom() < theClipper.Bottom() ? theRect.Bottom() : theClipper.Bottom();
	NFmiRect retRect(left, top, right, bottom);
	return retRect;
}

static NFmiColor CalcMaskColor(float theMaskValue)
{
	if(theMaskValue <= 0.1f)
		return NFmiColor(0.23f,0.23f,0.23f);
	else if(theMaskValue <= 0.2f)
		return NFmiColor(0.3f,0.3f,0.3f);
	else if(theMaskValue <= 0.3f)
		return NFmiColor(0.43f,0.43f,0.43f);
	else if(theMaskValue <= 0.4f)
		return NFmiColor(0.493f,0.493f,0.493f);
	else if(theMaskValue <= 0.5f)
		return NFmiColor(0.53f,0.53f,0.53f);
	else if(theMaskValue <= 0.6f)
		return NFmiColor(0.5943f,0.5943f,0.5943f);
	else if(theMaskValue <= 0.7f)
		return NFmiColor(0.643f,0.643f,0.643f);
	else if(theMaskValue <= 0.8f)
		return NFmiColor(0.6953f,0.6953f,0.6953f);
	else if(theMaskValue <= 0.9f)
		return NFmiColor(0.7243f,0.7243f,0.7243f);
	else if(theMaskValue < 1.f)
		return NFmiColor(0.763f,0.763f,0.763f);
	return NFmiColor(0.f,0.f,0.f);
}

void NFmiStationViewHandler::DrawMasksOnMap(NFmiToolBox* theGTB)
{
    if(itsCtrlViewDocumentInterface->Registry_ShowMasksOnMap(itsMapViewDescTopIndex))
	{
		if(!theGTB)
			return;
		boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->EditedSmartInfo();
		if(!info)
			return ;
		if(info->Grid() == 0) // editoitavalle asema datalle ei tehd‰ maskia
			return ;
		NFmiDrawingEnvironment envi;
		envi.EnableFill();
		envi.DisableFrame();
        EditedInfoMaskHandler editedInfoMaskHandler(info, NFmiMetEditorTypes::kFmiNoMask); // muutin activaatiomaskin k‰ytˆst‰ kaikkien piirtoon, koska muuten harvennetun datan piirto ei toimi oikein
		NFmiRect maskRect(CalcMaskRectSize(info));
		NFmiPoint xy;
		NFmiPoint latLon;
		boost::shared_ptr<NFmiAreaMaskList> maskList = itsCtrlViewDocumentInterface->ParamMaskListMT();
		if(maskList && maskList->CheckIfMaskUsed())
		{
			maskList->SyncronizeMaskTime(itsTime);
			for(info->ResetLocation(); info->NextLocation();)
			{
				latLon = info->LatLon();
				float maskValue = static_cast<float>(maskList->MaskValue(latLon));
				if(maskValue >= 1.f) // jos maski on 1, ei piirret‰ mit‰‰n
					continue;
				xy = LatLonToViewPoint(latLon);
				maskRect.Center(xy);
				NFmiRect clippedRect(ClipRect(itsMapArea->XYArea(), maskRect));
				if(clippedRect.Width() > 0 && clippedRect.Height() > 0)
				{
					NFmiColor col(CalcMaskColor(maskValue));
					envi.SetFillColor(col);
					NFmiRectangle rec(clippedRect, 0, &envi);
					theGTB->Convert(&rec);
				}
			}
		}
	}
}

// t‰ll‰ piirret‰‰n tavara, joka tulee myˆs bitmapin p‰‰lle
void NFmiStationViewHandler::DrawOverBitmapThings(NFmiToolBox * theGTB, bool /* dummy */, int theViewIndex, float /* dummy3 */, void* /* dummy4 */)
{
    itsToolBox = theGTB;
    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsRect);
    InitializeGdiplus(itsToolBox, &GetFrame());

    DrawLegends(theGTB);
    if(IsControlPointModeOn())
        DrawControlPoints();
    if((itsViewGridRowNumber == 1 && itsViewGridColumnNumber == theViewIndex) || itsCtrlViewDocumentInterface->IsPreciseTimeSerialLatlonPointUsed()) // vain 1. rivin viimeiseen ruutuun PAITSI jos ollaan tietyss‰ tarkkuus tilassa, milloin valittu piste piirret‰‰n jokaiseen karttaruutuun
        DrawSelectedLocations();

    DrawSelectedMTAModeSoundingPlaces();
    DrawSoundingPlaces();
    DrawCrossSectionPoints(); // n‰m‰ piirret‰‰n vain vasempaan yl‰ kulmaan karttaruudukossa

    DrawMouseCursorHelperCrossHair();
    DrawSelectedSynopFromGridView();
	DrawMapViewRangeMeterData();

	// Parametrilaatikko piirrett‰v‰ mahdollisimman lopussa kartan p‰‰lle!!!
    DrawParamView(theGTB); 
	// Aikalegenda laatikko piirrett‰v‰ parametri boksin p‰‰lle, koska se on l‰pin‰kyv‰ 
	// ja jos sijoitettu 'vahingossa' p‰‰llekk‰in, time-box on l‰pin‰kyv‰, parambox ei.
	DrawTimeText(); 
	DrawAutocompleteLocations(); // t‰m‰ ottaa huomioon, ettei piirr‰ parametri boxin p‰‰lle!
    DrawCurrentFrame(theGTB);
    CleanGdiplus();
}

#ifdef max
#undef max
#undef min
#endif
static int CalcPenWidthInPixels(float penSizeInMM, float pixelsPerMMOnScreen)
{
	return std::max(1, boost::math::iround(penSizeInMM * pixelsPerMMOnScreen));
}

static std::vector<std::vector<NFmiPoint>> SplitPossibleOverEdgeLongitudeJumps(const std::vector<NFmiPoint>& latlonPolyline)
{
	std::vector<std::vector<NFmiPoint>> splitPolylines;
	if(!latlonPolyline.empty())
	{
		size_t lastSplitIndex = 0;
		for(size_t index = 0; index < latlonPolyline.size() - 1; index++)
		{
			const auto& p1 = latlonPolyline[index];
			auto index2 = index + 1;
			const auto& p2 = latlonPolyline[index2];
			if(std::fabs(p1.X() - p2.X()) > 300.)
			{
				splitPolylines.push_back(std::vector<NFmiPoint>(latlonPolyline.begin() + lastSplitIndex, latlonPolyline.begin() + index2));
				lastSplitIndex = index2;
			}
		}

		if(splitPolylines.empty())
		{
			splitPolylines.push_back(latlonPolyline);
		}
		else
		{
			splitPolylines.push_back(std::vector<NFmiPoint>(latlonPolyline.begin() + lastSplitIndex, latlonPolyline.end()));
		}
	}
	return splitPolylines;
}

std::vector<NFmiPoint> NFmiStationViewHandler::ConvertLatlonToRelativePoints(const std::vector<NFmiPoint>& latlonPoints)
{
	std::vector<NFmiPoint> relativePoints;
	relativePoints.reserve(latlonPoints.size());
	for(const auto& latlon : latlonPoints)
	{
		relativePoints.push_back(LatLonToViewPoint(latlon));
	}
	return relativePoints;
}

void NFmiStationViewHandler::DrawMapViewRangeMeterData()
{
	auto &mapViewRangeMeter = itsCtrlViewDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	if(mapViewRangeMeter.ModeOn())
	{
		auto toolTipRealRowIndex = itsCtrlViewDocumentInterface->ToolTipRealRowIndex();
		auto toolTipColumnIndex = itsCtrlViewDocumentInterface->ToolTipColumnIndex();
		auto toolTipMapViewDescTopIndex = itsCtrlViewDocumentInterface->ToolTipMapViewDescTopIndex();
		if(toolTipRealRowIndex == CalcRealRowIndex() && toolTipColumnIndex == itsViewGridColumnNumber && toolTipMapViewDescTopIndex == itsMapViewDescTopIndex)
		{
			double rangeInMeters = mapViewRangeMeter.CalculateStartEndDistanceInMeters();
			const auto& color = mapViewRangeMeter.GetSelectedColor();
			const auto& startLatlon = mapViewRangeMeter.DragStartLatlonPoint();
			NFmiLocation startLocation(startLatlon);
			std::vector<NFmiPoint> latlonCirclePoints;
			double usedAngleStep = 2.;
			latlonCirclePoints.reserve(boost::math::iround(360. / usedAngleStep) + 1);
			for(auto currentAngle = 0.; currentAngle <= 360.; currentAngle += usedAngleStep)
			{
				auto currentLocation = startLocation.GetLocation(currentAngle, rangeInMeters, itsMapArea->PacificView());
				latlonCirclePoints.push_back(currentLocation.GetLocation());
			}

			auto pixelsPerMM = static_cast<float>(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y);
			float lineWidthInMM = 0.3f;
			float lineWidthInPixels = static_cast<float>(::CalcPenWidthInPixels(lineWidthInMM, pixelsPerMM));

			// Piirr‰ range-ympyr‰
			auto splitLatlonCirclePoints = ::SplitPossibleOverEdgeLongitudeJumps(latlonCirclePoints);
			for(const auto &latlonCirclePoints : splitLatlonCirclePoints)
			{
				auto relativeCirclePoints = ConvertLatlonToRelativePoints(latlonCirclePoints);
				auto gdiPoints = CtrlView::Relative2GdiplusPolyLine(itsToolBox, relativeCirclePoints, NFmiPoint());
				int lineStyle = 0; // 0=yhten‰inen viiva
				GdiPlusLineInfo lineInfo(lineWidthInPixels, color, lineStyle);
				CtrlView::DrawGdiplusCurve(*itsGdiPlusGraphics, gdiPoints, lineInfo, false, 0, itsCtrlViewDocumentInterface->Printing());
			}

			// Piirr‰ aloituspisteen kohdalle pikku risti
			auto startRelativePosition = LatLonToViewPoint(startLatlon);
			auto startGdiPosition = CtrlView::Relative2GdiplusPointInt(itsToolBox, startRelativePosition);
			float crossHairLengthInMM = 2.5f;
			float crossHairLengthInPixels = crossHairLengthInMM * pixelsPerMM;
			// vaaka viiva
			int line1_x1 = boost::math::iround(startGdiPosition.X - crossHairLengthInPixels);
			int line1_x2 = boost::math::iround(startGdiPosition.X + crossHairLengthInPixels);
			int line1_y1 = startGdiPosition.Y;
			int line1_y2 = startGdiPosition.Y;
			CtrlView::DrawLine(*itsGdiPlusGraphics, line1_x1, line1_y1, line1_x2, line1_y2, color, lineWidthInPixels);
			// pysty viiva
			int line2_x1 = startGdiPosition.X;
			int line2_x2 = startGdiPosition.X;
			int line2_y1 = boost::math::iround(startGdiPosition.Y - crossHairLengthInPixels);
			int line2_y2 = boost::math::iround(startGdiPosition.Y + crossHairLengthInPixels);
			CtrlView::DrawLine(*itsGdiPlusGraphics, line2_x1, line2_y1, line2_x2, line2_y2, color, lineWidthInPixels);

			// Piirr‰ aloitus ja lopetus pisteiden v‰liin katkoviiva
			const auto& endLatlon = mapViewRangeMeter.DragEndLatlonPoint();
			auto endRelativePosition = LatLonToViewPoint(endLatlon);
			auto endGdiPosition = CtrlView::Relative2GdiplusPointInt(itsToolBox, endRelativePosition);
			CtrlView::DrawLine(*itsGdiPlusGraphics, startGdiPosition.X, startGdiPosition.Y, endGdiPosition.X, endGdiPosition.Y, color, lineWidthInPixels, Gdiplus::DashStyleDot);

			// Piirret‰‰n avustavaa teksti‰ karttaruudun oikeaan alakulmaan
			auto textRelativeLocation = GetFrame().BottomRight();
			std::wstring fontName = L"arial";
			double fontSizeInMM = 6.f;
			double relativeTextLineHeight = itsToolBox->SY(boost::math::iround(fontSizeInMM * pixelsPerMM));

			textRelativeLocation.Y(textRelativeLocation.Y() - relativeTextLineHeight);
			std::string usedEndLatlonStr = "End location: ";
			usedEndLatlonStr += CtrlViewUtils::GetFixedLatlonStr(endLatlon);
			CtrlView::DrawTextToRelativeLocation(
				*itsGdiPlusGraphics,
				color,
				fontSizeInMM,
				usedEndLatlonStr,
				textRelativeLocation,
				pixelsPerMM,
				itsToolBox,
				fontName,
				kRight);

			textRelativeLocation.Y(textRelativeLocation.Y() - relativeTextLineHeight);
			std::string usedStartLatlonStr = "Start location: ";
			usedStartLatlonStr += CtrlViewUtils::GetFixedLatlonStr(startLatlon);
			CtrlView::DrawTextToRelativeLocation(
				*itsGdiPlusGraphics,
				color,
				fontSizeInMM,
				usedStartLatlonStr,
				textRelativeLocation,
				pixelsPerMM,
				itsToolBox,
				fontName,
				kRight);

			textRelativeLocation.Y(textRelativeLocation.Y() - relativeTextLineHeight);
			double rangeInKm = rangeInMeters / 1000.;
			std::string speedInfoStr = "Speed: ";
			// Nopeus laskuissa k‰ytet‰‰n mitatun et‰isyyden kanssa karttan‰ytˆss‰ olevaa aika-askelta matkaan k‰ytett‰v‰n‰ aikana
			auto usedTimeStepInHours = GetUsedTimeResolutionInHours();
			double speedInKm = rangeInKm / usedTimeStepInHours;
			speedInfoStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(speedInKm, 1);
			speedInfoStr += " (dur ";
			speedInfoStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(usedTimeStepInHours, 1);
			speedInfoStr += " h from time-step)";
			CtrlView::DrawTextToRelativeLocation(
				*itsGdiPlusGraphics,
				color,
				fontSizeInMM,
				speedInfoStr,
				textRelativeLocation,
				pixelsPerMM,
				itsToolBox,
				fontName,
				kRight);

			textRelativeLocation.Y(textRelativeLocation.Y() - relativeTextLineHeight);
			std::string rangeStr = "Start to end distance: ";
			auto rangeInKmStr = (rangeInMeters < 0) ? std::string("-.-") : NFmiValueString::GetStringWithMaxDecimalsSmartWay(rangeInKm, 1);
			rangeStr += rangeInKmStr;
			rangeStr += " km (";
			auto angleInDegrees = startLocation.Direction(endLatlon);
			if(angleInDegrees < 0)
			{
				// halutaan arvot 0-360, mutta NFmiLocation palauttaa arvov‰lin 180-360 arvot negatiivisina arvoina
				angleInDegrees = 180 + (180 + angleInDegrees);
			}
			rangeStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(angleInDegrees, 1);
			rangeStr += "∞"; // Astemerkki tms. ovat aina potentiaalinen ongelma, koska ei kuulu perus ascii merkkeihin
			rangeStr += ")";

			CtrlView::DrawTextToRelativeLocation(
				*itsGdiPlusGraphics,
				color,
				fontSizeInMM,
				rangeStr,
				textRelativeLocation,
				pixelsPerMM,
				itsToolBox,
				fontName,
				kRight);

			textRelativeLocation.Y(textRelativeLocation.Y() - relativeTextLineHeight);
			std::string lockModeStr = "Lock mode ";
			lockModeStr += mapViewRangeMeter.LockModeOn() ? "On" : "Off";
			lockModeStr += " (Toggle CTRL + Y)";
			CtrlView::DrawTextToRelativeLocation(
				*itsGdiPlusGraphics,
				color,
				fontSizeInMM,
				lockModeStr,
				textRelativeLocation,
				pixelsPerMM,
				itsToolBox,
				fontName,
				kRight);

			textRelativeLocation.Y(textRelativeLocation.Y() - relativeTextLineHeight);
			std::string moveStartStr = "Move start point (CTRL + left up)";
			CtrlView::DrawTextToRelativeLocation(
				*itsGdiPlusGraphics,
				color,
				fontSizeInMM,
				moveStartStr,
				textRelativeLocation,
				pixelsPerMM,
				itsToolBox,
				fontName,
				kRight);

			textRelativeLocation.Y(textRelativeLocation.Y() - relativeTextLineHeight);
			std::string measureStr = "Measure dist: left down - drag - left up";
			CtrlView::DrawTextToRelativeLocation(
				*itsGdiPlusGraphics,
				color,
				fontSizeInMM,
				measureStr,
				textRelativeLocation,
				pixelsPerMM,
				itsToolBox,
				fontName,
				kRight);

			textRelativeLocation.Y(textRelativeLocation.Y() - relativeTextLineHeight);
			std::string rangeTitleStr = "Distance measure (press Y on/off)";
			CtrlView::DrawTextToRelativeLocation(
				*itsGdiPlusGraphics,
				color,
				fontSizeInMM,
				rangeTitleStr,
				textRelativeLocation,
				pixelsPerMM,
				itsToolBox,
				fontName,
				kRight);
		}
	}
}

float NFmiStationViewHandler::GetUsedTimeResolutionInHours()
{
	return itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex);
}

static bool IsRectIntersecting(const NFmiRect &theRect, std::vector<NFmiRect> &theExistingRects)
{
	for(size_t i=0; i < theExistingRects.size(); i++)
	{
		if(theRect.Intersect(theExistingRects[i]))
			return true;
	}
	return false;
}

void NFmiStationViewHandler::DrawAutocompleteLocations(void)
{
    auto &autoComplete = itsCtrlViewDocumentInterface->AutoComplete();
    if(autoComplete.AutoCompleteDialogOn() == false)
        return;

    itsAutoCompletionRects.clear(); // aluksi aina nollataan laatikot

    // piirret‰‰n autocomplete juttuja vain 1. rivin 1. sarakkeeseen
    if(itsViewGridRowNumber == 1 && itsViewGridColumnNumber == 1)
    {
        std::vector<NFmiACLocationInfo> locInfos = autoComplete.AutoCompleteResults();
        if(locInfos.size() == 0)
            return;

        NFmiRect markerCircleBase = CalcBaseMarkerRect(autoComplete.MarkerSizeInMM());
        // Lis‰t‰‰n jo t‰ss‰ vaiheessa kaikki markerit rect-Listaan, ett‰ niiden p‰‰lle ei piirret‰ mit‰‰n
        for(size_t i = 0; i < locInfos.size(); i++)
        {
            markerCircleBase.Center(LatLonToViewPoint(locInfos[i].itsLatlon));
            itsAutoCompletionRects.push_back(markerCircleBase);
        }

        // varsinainen piirto rutiini alkaa t‰st‰
        for(size_t i = 0; i < locInfos.size(); i++)
            DrawAutocompleteLocation(itsGdiPlusGraphics, locInfos[i], markerCircleBase);
    }
}

static double GetUsedRadius(int index, double theBoxHeight)
{
	if(index == 0)
		return theBoxHeight * 1.2;
	else if(index == 1)
		return theBoxHeight * 2.2;
	else
		return theBoxHeight * 3.7;
}

static const double trigFactor = 2 * kPii / 360;

NFmiPoint NFmiStationViewHandler::CalcNewCenterPoint(double relativeX, double relativeY, double relativeW, double radius, double angle)
{
	double radSin = radius * ::sin(angle * trigFactor);
	double radCos = radius * ::cos(angle * trigFactor);
	double newCenterX = relativeX + (relativeW / 2.) + radCos;
	if(radCos < 0) // jos kulma osoittaa relatiivisesta pisteest‰ vasemmalle, heitet‰‰n koko boksia vasemmalle
		newCenterX = relativeX - (relativeW / 2.) + radCos;
	double newCenterY = relativeY - radSin;
	return NFmiPoint(newCenterX, newCenterY);
}

static bool IsBoundinBoxInGoodPlace(const NFmiRect &theRelativeBoundingBox, std::vector<NFmiRect> &theAutoCompletionRects, const NFmiRect &theViewRect)
{
	if(theViewRect.IsInside(theRelativeBoundingBox))
	{
		if(::IsRectIntersecting(theRelativeBoundingBox, theAutoCompletionRects) == false)
			return true;
	}
	return false;
}

bool NFmiStationViewHandler::CheckBoundingBox(NFmiRect &theBoundBox, std::vector<NFmiRect> &theAutoCompletionRects, double relativeX, double relativeY, double relativeW, double radius, double angle, FmiDirection &theMarkerConnectingPlace)
{
	theBoundBox.Center(CalcNewCenterPoint(relativeX, relativeY, relativeW, radius, angle));
	if(::IsBoundinBoxInGoodPlace(theBoundBox, theAutoCompletionRects, GetFrame()))
	{ // jos ei mennyt p‰‰llekk‰in vanhojen laatikoiden kanssa, laitetaan teksti t‰h‰n
		if(::fabs(angle) <= 90)
			theMarkerConnectingPlace = kMiddleLeft;
		else
			theMarkerConnectingPlace = kMiddleRight;
		return true;
	}
	return false;
}

// etsi paikka theRelativePoint:in ymp‰rilt‰ (ei p‰‰lt‰), joka ei mene p‰‰llekk‰in theAutoCompletionRects-listassa olevien rectien kanssa. 
// Halutun rectin koko on theRelativeBoundingBox:in kokoinen.
// Etsit‰‰n uusi paikka puoliympyr‰n muotoiselta alueelta, kolmella eri s‰teell‰. S‰de m‰‰r‰ytyy theRelativeBoundingBox:in korkeuden mukaan. 
// 1. kierroksella 1/2 korkeutta, 2. kierroksella 1/3 korkeutta ja lopuksi 2/3 korkeutta.
NFmiRect NFmiStationViewHandler::SearchNameBoxLocation(const NFmiPoint &theRelativePoint, const NFmiRect &theRelativeBoundingBox, std::vector<NFmiRect> &theAutoCompletionRects, double theOneLineBoxHeight, FmiDirection &theMarkerConnectingPlace)
{
	NFmiRect newRelativeBoundingBox(theRelativeBoundingBox);
	double relWidth = theRelativeBoundingBox.Width();
	double relX = theRelativePoint.X();
	double relY = theRelativePoint.Y();
	double usedRadius = GetUsedRadius(0, theOneLineBoxHeight);

	for(int i=0; i<3; i++)
	{
		usedRadius = GetUsedRadius(i, theOneLineBoxHeight);
		for(double angle=0; angle < 90; angle += 15)
		{
			if(CheckBoundingBox(newRelativeBoundingBox, theAutoCompletionRects, relX, relY, relWidth, usedRadius, angle, theMarkerConnectingPlace))
				return newRelativeBoundingBox;
			// kokeillaan sitten vastaavaa negatiivisella kulmalla
			else if(CheckBoundingBox(newRelativeBoundingBox, theAutoCompletionRects, relX, relY, relWidth, usedRadius, -angle, theMarkerConnectingPlace))
				return newRelativeBoundingBox;
			// Kokeillaan sitten hakea vasemmalta puolelta ylh‰‰lt‰
			if(CheckBoundingBox(newRelativeBoundingBox, theAutoCompletionRects, relX, relY, relWidth, usedRadius, 180 - angle, theMarkerConnectingPlace))
				return newRelativeBoundingBox;
			// Kokeillaan lopuksi hakea vasemmalta puolelta alhaalta
			if(CheckBoundingBox(newRelativeBoundingBox, theAutoCompletionRects, relX, relY, relWidth, usedRadius, -(180 - angle), theMarkerConnectingPlace))
				return newRelativeBoundingBox;
		}
	}

	// Jos ei lˆytynyt paikkaa, laitetaan se siteen vain 1. sijoitus paikkaan
	usedRadius = GetUsedRadius(0, theOneLineBoxHeight);
	newRelativeBoundingBox.Center(CalcNewCenterPoint(relX, relY, relWidth, usedRadius, 0));
	theMarkerConnectingPlace = kCenter;
	return newRelativeBoundingBox;
}

void NFmiStationViewHandler::DrawAutocompleteLocation(Gdiplus::Graphics *theGdiPlusGraphics, const NFmiACLocationInfo &theLocInfo, NFmiRect &theMarkerCircleBase)
{
	if(theLocInfo.itsLatlon != NFmiACLocationInfo::gMissingLatlon)
	{
		double pixelsPerMM = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y;
        auto &autoComplete = itsCtrlViewDocumentInterface->AutoComplete();
		NFmiColor textColor = autoComplete.NameTextColor();
		auto nameStr = theLocInfo.itsName;
		if(theLocInfo.itsArea.empty() == false)
		{
			nameStr += L"\n";
			nameStr += theLocInfo.itsArea;
		}
		if(theLocInfo.itsCountry.empty() == false)
		{
			nameStr += L" (";
			nameStr += theLocInfo.itsCountry;
			nameStr += L")";
		}

		std::wstring fontName = CtrlView::StringToWString(autoComplete.FontName()); // L"arial";
		double fontSizeInMM = autoComplete.FontSizeInMM();
		std::string oneLineTestStr = "Pasalsmaksljjks"; // t‰m‰n stringin avulla halutaan tiet‰‰ vain yhden rivin 'laatikon' korkeus
		Gdiplus::RectF oneLineBoundinBox = CtrlView::GetStringBoundingBox(*theGdiPlusGraphics, oneLineTestStr, static_cast<float>(fontSizeInMM*pixelsPerMM), Gdiplus::PointF(), fontName);
		Gdiplus::RectF boundinBox = CtrlView::GetWStringBoundingBox(*theGdiPlusGraphics, nameStr, static_cast<float>(fontSizeInMM*pixelsPerMM), Gdiplus::PointF(), fontName);
		NFmiPoint relativePoint = LatLonToViewPoint(theLocInfo.itsLatlon);
		Gdiplus::PointF locationInPixels = CtrlView::Relative2GdiplusPoint(itsToolBox, relativePoint);
		boundinBox.Offset(locationInPixels);
		NFmiRect relativeOneLineBoundingBox = CtrlView::GdiplusRect2Relative(itsToolBox, oneLineBoundinBox);
		NFmiRect relativeBoundingBox = CtrlView::GdiplusRect2Relative(itsToolBox, boundinBox);

		// etsi laatikon paikka niin ett‰ se ei ole p‰‰llekk‰in muiden kanssa, eik‰ se ole paikan itsens‰ p‰‰ll‰
		FmiDirection markerConnectingPlace = kLeft;
		NFmiRect foundRect = SearchNameBoxLocation(relativePoint, relativeBoundingBox, itsAutoCompletionRects, relativeOneLineBoundingBox.Height(), markerConnectingPlace);
        itsAutoCompletionRects.push_back(foundRect);
		Gdiplus::Rect foundBoundingBox = CtrlView::Relative2GdiplusRect(itsToolBox, foundRect);


		NFmiColor frameColor = autoComplete.RectangleFrameColor(); //(0.f, 0.f, 0.0f);
		NFmiColor fillColor = autoComplete.RectangleFillColor(); //(1.f, 1.f, 0.0f, 0.6f);
		bool doFill = true;
		if(markerConnectingPlace == kCenter || markerConnectingPlace == kTopCenter)
			fillColor.Alpha(fillColor.Alpha()*0.8f); // jos boxille ei lˆytynyt paikkaa, joka ei ole p‰‰llekk‰in, laitetaan enemm‰n l‰pin‰kyvyytt‰
        CtrlView::DrawRect(*theGdiPlusGraphics, foundBoundingBox, frameColor, fillColor, doFill, true, FmiMax(1.f, static_cast<float>FmiRound(autoComplete.RectangleFramePenSizeInMM() * pixelsPerMM)));

        CtrlView::DrawTextToRelativeLocation(
									*theGdiPlusGraphics, 
									textColor, 
									fontSizeInMM, 
									nameStr,
									foundRect.TopLeft(),
									pixelsPerMM,
									itsToolBox,
									fontName,
									kTopLeft);

		// Piirr‰ viel‰ paikan markkeri (musta ympyr‰, jossa punainen keskus) ja siit‰ viiva laatikkoon!!!
		DrawMarkerPoint(relativePoint, foundRect, itsAutoCompletionRects, markerConnectingPlace, theMarkerCircleBase);
	}
}

NFmiRect NFmiStationViewHandler::CalcBaseMarkerRect(double theMarkerSizeInMM)
{
	double circleSizeInMM = theMarkerSizeInMM;
    auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
    int pixelSizeX = boost::math::iround(circleSizeInMM * graphicalInfo.itsPixelsPerMM_x);
    int pixelSizeY = boost::math::iround(circleSizeInMM * graphicalInfo.itsPixelsPerMM_y);
	double xWidth = itsToolBox->SX(pixelSizeX);
	double yWidth = itsToolBox->SY(pixelSizeY);
	return NFmiRect(0, 0, xWidth, yWidth);
}

void NFmiStationViewHandler::DrawMarkerPoint(const NFmiPoint &theRelativePlace, const NFmiRect &theTextRect, std::vector<NFmiRect> &theAutoCompletionRects, FmiDirection theMarkerConnectingPlace, NFmiRect &theMarkerCircleBase)
{
    auto &autoComplete = itsCtrlViewDocumentInterface->AutoComplete();
	double lineWidthInMM = autoComplete.ConnectingLinePenSizeInMM();
    int lineWidth = FmiMax(1, boost::math::iround(lineWidthInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y));
	theMarkerCircleBase.Center(theRelativePlace);
	NFmiDrawingEnvironment envi;
	envi.SetPenSize(NFmiPoint(lineWidth, lineWidth));
	envi.EnableFill();
	envi.SetFrameColor(autoComplete.MarkerFrameColor());
	envi.SetFillColor(autoComplete.MarkerFillColor());
	NFmiPoint lineEndingPlace;
	if(theMarkerConnectingPlace == kMiddleLeft || theMarkerConnectingPlace == kCenter)
		lineEndingPlace = NFmiPoint(theTextRect.Left(), theTextRect.Center().Y());
	else if(theMarkerConnectingPlace == kMiddleRight || theMarkerConnectingPlace == kTopCenter)
		lineEndingPlace = NFmiPoint(theTextRect.Right(), theTextRect.Center().Y());
	NFmiLine line(theRelativePlace, lineEndingPlace, 0, &envi);
	itsToolBox->Convert(&line);
	itsToolBox->DrawEllipse(theMarkerCircleBase, &envi);
}

static bool IsPacificViewData(boost::shared_ptr<NFmiFastQueryInfo> &theEditedInfo)
{
    if(theEditedInfo)
    {
        if(theEditedInfo->Grid()) // editoidun datan pit‰isi olla hiladata
            return theEditedInfo->Grid()->Area()->PacificView();
    }

    return false;
}

void NFmiStationViewHandler::DrawMouseCursorHelperCrossHair(void)
{ // t‰m‰n on tarkoitus piirt‰‰ apu hiiri kursorit muihin karttaruutuihin, ett‰ k‰ytt‰j‰
  // n‰kee kaikilla ruuduilla, miss‰ hiiren kursori menee
	if(!itsCtrlViewDocumentInterface->ShowMouseHelpCursorsOnMap())
		return ;

	static NFmiDrawingEnvironment envi;
	envi.EnableInvert();
	envi.EnableFill();
	double width1 = itsToolBox->SX(8);
	double height1 = itsToolBox->SY(8);
	double width2 = itsToolBox->SX(24);
	double height2 = itsToolBox->SY(24);
	double width3 = itsToolBox->SX(2);
	double height3 = itsToolBox->SY(2);
	NFmiRect aRect1(0, 0, width1, height1);
	NFmiRect aRect2(0, 0, width2, height2);
	NFmiPoint xyPoint(LatLonToViewPoint(itsCtrlViewDocumentInterface->ToolTipLatLonPoint()));
	aRect1.Center(xyPoint);
	aRect2.Center(xyPoint);
	// piirret‰‰n er‰‰nlainen t‰ht‰ys ristikko hiiren kursorin kohdalle
	NFmiRectangle aRec1(NFmiPoint(aRect1.Left(), xyPoint.Y() - height3/2.), NFmiPoint(aRect2.Left(), xyPoint.Y() + height3/2.), 0, &envi);
	itsToolBox->Convert(&aRec1);
	NFmiRectangle aRec2(NFmiPoint(aRect1.Right(), xyPoint.Y() - height3/2.), NFmiPoint(aRect2.Right(), xyPoint.Y() + height3/2.), 0, &envi);
	itsToolBox->Convert(&aRec2);
	NFmiRectangle aRec3(NFmiPoint(xyPoint.X() - width3/2., aRect1.Bottom()), NFmiPoint(xyPoint.X() + width3/2., aRect2.Bottom()), 0, &envi);
	itsToolBox->Convert(&aRec3);
	NFmiRectangle aRec4(NFmiPoint(xyPoint.X() - width3/2., aRect1.Top()), NFmiPoint(xyPoint.X() + width3/2., aRect2.Top()), 0, &envi);
	itsToolBox->Convert(&aRec4);
	NFmiRectangle aRec5(NFmiPoint(xyPoint.X() - width3/2., xyPoint.Y() - height3/2.), NFmiPoint(xyPoint.X() + width3/2., xyPoint.Y() + height3/2.), 0, &envi);
	itsToolBox->Convert(&aRec5);
}

void NFmiStationViewHandler::DrawSelectedSynopFromGridView(void)
{
	if(itsCtrlViewDocumentInterface->SynopDataGridViewOn())
		if(itsCtrlViewDocumentInterface->ShowSynopHighlight())
		{
			static NFmiDrawingEnvironment envi;
			envi.SetFrameColor(NFmiColor(1.f, 0.f, 0.f)); // punainen
			envi.SetPenSize(NFmiPoint(3,3));
			double width1 = itsToolBox->SX(8);
			double height1 = itsToolBox->SY(8);
			double width2 = itsToolBox->SX(18);
			double height2 = itsToolBox->SY(18);
			double width3 = itsToolBox->SX(2);
			double height3 = itsToolBox->SY(2);
			NFmiRect aRect1(0, 0, width1, height1);
			NFmiRect aRect2(0, 0, width2, height2);
			NFmiRect aRect3(0, 0, width3, height3);
			NFmiPoint xyPoint(this->LatLonToViewPoint(itsCtrlViewDocumentInterface->GetSynopHighlightLatlon()));
			aRect1.Center(xyPoint);
			aRect2.Center(xyPoint);
			aRect3.Center(xyPoint);
			// piirret‰‰n er‰‰nlainen t‰ht‰ys ristikko aseman kohdalle
			NFmiLine line1(aRect1.BottomLeft(), aRect2.BottomLeft(), 0, &envi);
			itsToolBox->Convert(&line1);
			NFmiLine line2(aRect1.TopLeft(), aRect2.TopLeft(), 0, &envi);
			itsToolBox->Convert(&line2);
			NFmiLine line3(aRect1.BottomRight(), aRect2.BottomRight(), 0, &envi);
			itsToolBox->Convert(&line3);
			NFmiLine line4(aRect1.TopRight(), aRect2.TopRight(), 0, &envi);
			itsToolBox->Convert(&line4);
			NFmiRectangle aRectangle(aRect3, 0, &envi);
			itsToolBox->Convert(&aRectangle);
		}
}

// 1. Jos beforeDataCase == true, tehd‰‰n aina background kartta-layerille teksti, 
// mutta tarkistetaan myˆs tehd‰‰nkˆ overlay-layerille teksti, jos se piirret‰‰n heti kartan p‰‰lle.
// 2. Jos beforeDataCase == false, ei tehd‰ ollenkaan background tapausta, vaan katsotaan ainoastaan tehd‰‰nkˆ 
// overlay-layerille teksti, jos se piirret‰‰n datan p‰‰lle.
std::string NFmiStationViewHandler::ComposeMapLayerToolTipText(bool beforeDataIsDrawnCase)
{
	auto& combinedMapHandlerInterface = itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface();
	auto addBackgroundText = beforeDataIsDrawnCase;
	auto overlayDrawMode = beforeDataIsDrawnCase ? 0 : 1;
	auto addOverlayText = combinedMapHandlerInterface.isOverlayMapDrawnForThisDescTop(itsMapViewDescTopIndex, overlayDrawMode);
	std::string str;
	if(addBackgroundText || addOverlayText)
	{
		if(!beforeDataIsDrawnCase)
			str += "\n<hr color=red>";
		str += "\n<b><font color=";
		str += ColorString::Color2HtmlColorStr(CtrlViewUtils::GetParamTextColor(NFmiInfoData::kMapLayer, false));
		str += ">";
		if(addBackgroundText)
			str += combinedMapHandlerInterface.getCurrentMapLayerGuiText(itsMapViewDescTopIndex, true);
		if(addOverlayText)
		{
			if(addBackgroundText)
				str += "\n";
			str += combinedMapHandlerInterface.getCurrentMapLayerGuiText(itsMapViewDescTopIndex, false);
		}
		str += "</font></b>\n";
	}
	return str;
}

std::string NFmiStationViewHandler::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
	std::string str;
	// Parametri laatikon p‰‰lt‰ ei tehd‰ normi tooltippi‰, koska jotkut tooltipien laskut ovat super hitaita ja saattavat h‰irit‰ parametrin valinta popupin avautumista.
	// Tehd‰‰n param-boxille vain tiettyjen tapauksien k‰sittely kuten mik‰ on kohdalla olevan macroParamin kaava, tms.
	if(IsMouseCursorOverParameterBox(theRelativePoint))
	{
		return itsParamHandlerView->ComposeToolTipText(theRelativePoint);
	}

	if(itsViewList)
	{
		NFmiString timeStr = itsTime.ToStr(::GetDictionaryString("TempViewLegendTimeFormat"), itsCtrlViewDocumentInterface->Language());
		timeStr += " [UTC]";
		str += static_cast<char*>(timeStr);
		str += "\n";
		NFmiPoint latlon(itsCtrlViewDocumentInterface->ToolTipLatLonPoint());
		str += "Lat: ";
		str += CtrlViewUtils::GetLatitudeMinuteStr(latlon.Y(), 1);
		str += " Lon: ";
		str += CtrlViewUtils::GetLongitudeMinuteStr(latlon.X(), 1);
		str += ComposeMapLayerToolTipText(true);
		str += "<hr color=red>";
		str += "\n";
		str += MakePossibleVirtualTimeTooltipText();

		auto viewGridSizePoint = itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex);
		auto viewGridSize = int(viewGridSizePoint.X() * viewGridSizePoint.Y());
		str += itsCtrlViewDocumentInterface->ApplicationWinRegistry().VisualizationSpaceoutSettings().composePossibleTooltipWarningText(*itsMapArea, viewGridSize);

		if(!itsViewList->NumberOfItems())
		{

		}
		else
		{
			for(itsViewList->Reset(); itsViewList->Next(); )
			{
				str += itsViewList->Current()->ComposeToolTipText(theRelativePoint);
				str += "\n";
			}
			// Otetaan viimeisin rivinvaihto pois, tulevat tekstinlis‰ys funktiot lis‰‰v‰t tavittaessa alkuun rivinvaihdon
			str.pop_back();
		}
		str += ComposeWarningMessageToolTipText();
		str += ComposeSilamLocationsToolTipText();
		// Lopuksi viel‰ mahdollinen kartta overlay layer teksti
		str += ComposeMapLayerToolTipText(false);
	}
	return str;
}

static void ConvertXML2PlainCode(std::string &theConvertedStr)
{
	// Pit‰‰ korvata XML tagien < ja > merkit, ett‰ tooltip n‰ytt‰‰ tekstin sellaisenaan
	NFmiStringTools::ReplaceAll(theConvertedStr, "<", "&lt;");
	NFmiStringTools::ReplaceAll(theConvertedStr, ">", "&gt;");
}

#ifndef DISABLE_CPPRESTSDK
void NFmiStationViewHandler::GetMinimumDistanceMessage(const std::vector<HakeMessage::HakeMsg> &messages, const NFmiLocation &wantedLocation, HakeMessage::HakeMsg &minimumDistanceMessage, double &minimumDistance)
{
    for(const auto &message : messages)
    {
        double currDist = wantedLocation.Distance(message.LatlonPoint());;
        if(currDist < minimumDistance)
        {
            minimumDistance = currDist;
            minimumDistanceMessage = message;
        }
    }
}

void AddLeadingZero(std::string& s)
{
    if(s.length() < 2)
    {
        s = "0" + s;
    }
}

std::string KahaUTCTime(const HakeMessage::HakeMsg &msg)
{
    std::string str;

    str += "UTC Aika: ";
    std::string hours = std::to_string(msg.StartTime().GetHour());
    AddLeadingZero(hours);
    std::string mins = std::to_string(msg.StartTime().GetMin());
    AddLeadingZero(mins);
    std::string secs = std::to_string(msg.StartTime().GetSec());
    AddLeadingZero(secs);
    str += hours + ":" + mins + ":" + secs;

    return str;
}

static std::string KahaToolTipText(const HakeMessage::HakeMsg &msg)
{
    std::string str;
    std::string tempStr;
    std::stringstream strStream;

    //Phenomenon
    str = msg.MessageStr() + "\n";

    // Category id
    str += "Kategoria id: " + std::to_string(msg.Category()) + "\n";

    //UTC time
    str += KahaUTCTime(msg) + "\n";

    return str;
}

// Xml pohjaisessa sanomassa pit‰‰ eri kokonaisviesti jakaa xml-node riveihin, 
// jotta siit‰ saa selv‰‰ (originaali viesti on yhdell‰ rivill‰).
static std::string GetFinalXmlBaseHakeToolTipText(const HakeMessage::HakeMsg &msg)
{
    std::string str;
    std::vector<std::string>messageNodes = NFmiStringTools::Split(msg.TotalMessageStr(), ">");
    for(unsigned int i = 0; i<messageNodes.size(); i++)
    {
        str += messageNodes[i];
        str += ">\n";
    }
    // Pit‰‰ korvata XML tagien < ja > merkit, ett‰ tooltip n‰ytt‰‰ tekstin sellaisenaan
    ::ConvertXML2PlainCode(str);

    return str;
}

static std::string GetFinalJsonBaseHakeToolTipText(const HakeMessage::HakeMsg &msg)
{
    std::string str = msg.MakeTooltipMessageStr();
    return str;
}

static std::string HakeToolTipText(const HakeMessage::HakeMsg &msg)
{
    if(msg.IsFromXmlFormat())
        return ::GetFinalXmlBaseHakeToolTipText(msg);
    else
        return ::GetFinalJsonBaseHakeToolTipText(msg);
}

static std::string MakeMessageHeader(std::string&& typeName)
{
    std::string message = "L‰hin ";
    message += typeName;
    message += " sanoma:\n";
    return message;
}

static std::string GetFinalWarningMessageForTooltip(double minDistHake, double minDistKaha, const HakeMessage::HakeMsg &hakeMessage, const HakeMessage::HakeMsg &kahaMessage)
{
    //Treat Hake and KaHa messages differently
    if(!(hakeMessage == HakeMessage::HakeMsg::unInitialized) || !(kahaMessage == HakeMessage::HakeMsg::unInitialized))
    {
        std::string message;
        message += "\n<hr color=red>\n";
        if(minDistHake < minDistKaha)
        {
            message += ::MakeMessageHeader("Hake");
            message += ::HakeToolTipText(hakeMessage);
        }
        else
        {
            message += ::MakeMessageHeader("KaHa");
            message += ::KahaToolTipText(kahaMessage);
        }
        return message;
    }
    return "";
}

#endif // DISABLE_CPPRESTSDK

std::string NFmiStationViewHandler::ComposeWarningMessageToolTipText(void)
{
#ifndef DISABLE_CPPRESTSDK
    if(ShowWarningMessages())
	{
        GetShownMessages();
        NFmiLocation wantedLocation(itsCtrlViewDocumentInterface->ToolTipLatLonPoint());
		double minDistHake = 9999999999;
        double minDistKaha = 9999999999;
        HakeMessage::HakeMsg hakeMessage = HakeMessage::HakeMsg::unInitialized;
        HakeMessage::HakeMsg kahaMessage = HakeMessage::HakeMsg::unInitialized;
        GetMinimumDistanceMessage(itsShownHakeMessages, wantedLocation, hakeMessage, minDistHake);
        GetMinimumDistanceMessage(itsShownKaHaMessages, wantedLocation, kahaMessage, minDistKaha);
        
        return ::GetFinalWarningMessageForTooltip(minDistHake, minDistKaha, hakeMessage, kahaMessage);
	}
#endif // DISABLE_CPPRESTSDK
	return "";
}

static NFmiSilamStationList::Station GetClosestSilamStation(NFmiSilamStationList &theLocations, const NFmiLocation &theSearchPlace)
{
	NFmiSilamStationList::Station closestLoc;
	closestLoc.itsLatlon = NFmiPoint::gMissingLatlon;
	auto &locList = theLocations.Locations();
	double minDist = 9999999999;
	for(size_t i=0; i < locList.size(); i++)
	{
		double currentDist = theSearchPlace.Distance(locList[i].itsLatlon);
		if(currentDist < minDist)
		{
			minDist = currentDist;
			closestLoc = locList[i];
		}
	}

	return closestLoc;
}

static std::string MakeSilamStationToolTipText(NFmiSilamStationList::Station &theStation, const std::string &theStationTypeStr)
{
	std::string str;
	str += "\n<hr color=red>\n";
	str += theStationTypeStr;
	str += "\nInfo: ";
	str += theStation.itsInfo;
	str += "\nCountry: ";
	str += theStation.itsCountry;
	str += "\nType: ";
	str += theStation.itsType;
	str	+= "\nLocation: ";
	str += CtrlViewUtils::GetLatitudeMinuteStr(theStation.itsLatlon.Y(), 1);
	str += " ";
	str += CtrlViewUtils::GetLongitudeMinuteStr(theStation.itsLatlon.X(), 1);
	return str;
}

std::string NFmiStationViewHandler::ComposeSilamLocationsToolTipText(void)
{
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->TrajectoryViewOn())
	{
		double maxAllowedDistanceInMeters = 200*1000;
		NFmiLocation wantedLoc(itsCtrlViewDocumentInterface->ToolTipLatLonPoint());
		NFmiSilamStationList::Station stat1 = ::GetClosestSilamStation(itsCtrlViewDocumentInterface->TrajectorySystem()->NuclearPlants(), wantedLoc);
		NFmiSilamStationList::Station stat2 = ::GetClosestSilamStation(itsCtrlViewDocumentInterface->TrajectorySystem()->OtherPlaces(), wantedLoc);
		double minDist1 = wantedLoc.Distance(stat1.itsLatlon);
		double minDist2 = wantedLoc.Distance(stat2.itsLatlon);
		if(minDist1 < minDist2)
		{
			if(minDist1 < maxAllowedDistanceInMeters)
				return ::MakeSilamStationToolTipText(stat1, "Nuclear power plant");
		}
		else
		{
			if(minDist2 < maxAllowedDistanceInMeters)
				return ::MakeSilamStationToolTipText(stat2, "Non-nuclear place");
		}
	}
	return "";
}

NFmiCtrlView* NFmiStationViewHandler::GetView(const NFmiDataIdent &theDataIdent, bool fUseParamIdOnly)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
		{
			if(fUseParamIdOnly)
			{
				if((itsViewList->Current()->DrawParam()->Param().GetParamIdent() == theDataIdent.GetParamIdent()))
					return itsViewList->Current();
			}
			else
				if((itsViewList->Current()->DrawParam()->Param() == theDataIdent))
					return itsViewList->Current();
		}
	}
	return 0;
}

boost::shared_ptr<NFmiArea> NFmiStationViewHandler::GetArea() const 
{ 
	return itsMapArea; 
}

void NFmiStationViewHandler::SetArea(const boost::shared_ptr<NFmiArea>& theArea)
{
	throw std::runtime_error(std::string("Don't use this ") + __FUNCTION__ + " -function before real implementation is made...");
}

bool NFmiStationViewHandler::HasSeparateCountryBorderLayer() const
{
	if(itsCtrlViewDocumentInterface)
	{
		NFmiDrawParamList* drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex());
		return CombinedMapHandlerInterface::hasSeparateBorderLayer(drawParamList);
	}
	return false;
}
