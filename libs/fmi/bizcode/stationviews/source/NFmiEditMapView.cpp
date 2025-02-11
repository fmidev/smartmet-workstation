// NFmiEditMapView.cpp: implementation of the NFmiEditMapView class.
//
// Changed 30.8.1999/Marko Muutin alueiden aktivointia.
//
//////////////////////////////////////////////////////////////////////

#ifndef UNIX
#pragma warning( disable : 4786 4996) // disable warning 4385. (liian pitkä luokan nimi)
#endif

#include "NFmiEditMapView.h"
#include "NFmiStringTools.h"
#include "NFmiTime.h"
#include "NFmiTimeControlView.h"
#include "NFmiDrawParam.h"
#include "NFmiDrawParamList.h"
#include "NFmiToolBox.h"
#include "NFmiTimeStationViewRowList.h"
#include "NFmiArea.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiRectangle.h"
#include "NFmiText.h"
#include "NFmiValueString.h"
#include "NFmiSynopPlotView.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiCrossSectionSystem.h"
#include "CtrlViewDocumentInterface.h"
#include "MapHandlerInterface.h"
#include "GraphicalInfo.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiAnimationData.h"
#include "catlog/catlog.h"
#include "CtrlViewTimeConsumptionReporter.h"

#include <gdiplus.h>

#include <vector>
#include <ctime>	// tzset
#include "boost\math\special_functions\round.hpp"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NFmiEditMapView::NFmiEditMapView(int theMapViewDescTopIndex
								,NFmiToolBox *theToolBox
								,boost::shared_ptr<NFmiDrawParam> &theDrawParam)
:NFmiCtrlView(theMapViewDescTopIndex, GetCtrlViewDocumentInterface()->GetMapHandlerInterface(theMapViewDescTopIndex)->Area()->XYArea()
			 ,theToolBox
			 ,theDrawParam)
,itsTimeControlView(0)
,itsMapArea()
,itsMapDrawParam(new NFmiDrawParam)
,itsViewGrid(0)
,fMouseMovedInCapture(false)
,itsVerticalAnimationInfo()
,itsLastSendTimeTextRect()
{
	NFmiRect mapRect = itsCtrlViewDocumentInterface->RelativeMapRect(itsMapViewDescTopIndex);
	SetMapAreaAndRect(GetCtrlViewDocumentInterface()->GetMapHandlerInterface(theMapViewDescTopIndex)->Area(), mapRect);
	itsViewGrid = new NFmiTimeStationViewRowList(itsMapViewDescTopIndex, itsMapArea
												  ,itsToolBox
												  ,itsMapDrawParam);

	itsTimeControlViewRect = NFmiRect(0, mapRect.Height(), 1., 1);
	CreateTimeControlView();
}

NFmiEditMapView::~NFmiEditMapView()
{
	delete itsTimeControlView;
	delete itsViewGrid;
}

bool NFmiEditMapView::CreateTimeControlView(void)
{
	NFmiTimeControlView::MouseStatusInfo mouseStatusInfo;
	if(itsTimeControlView)
		mouseStatusInfo = itsTimeControlView->GetMouseStatusInfo();
	delete itsTimeControlView;
	itsTimeControlView = 0;
    auto defaultEditedDrawParam = itsCtrlViewDocumentInterface->DefaultEditedDrawParam();
	if(defaultEditedDrawParam)
	{
		itsTimeControlView = new NFmiTimeControlView(itsMapViewDescTopIndex, itsTimeControlViewRect
													,itsToolBox
													, defaultEditedDrawParam
													,true
													,false
													,true
													,true
													,true);
		itsTimeControlView->Initialize(true, false);
		itsTimeControlView->SetMouseStatusInfo(mouseStatusInfo);
		return true;
	}
	return false;
}

void NFmiEditMapView::Draw(NFmiToolBox * theGTB)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, std::string(__FUNCTION__) + ": starting to draw map-view-desktop");
    try
    {
        // HUOM! jos GDI tulee laajempaan käyttöön, poista init+clean tästä ja siirrä ne Draw-metodiin!!!!!!!
        InitializeGdiplus(itsToolBox, &GetFrame());
        itsGdiPlusGraphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias); // Huom. antialiasointi saattaa hidastaa yllättävän paljon piirtoa

        NFmiMilliSecondTimer timer;
        timer.StartTimer();

        SetViewTimes();
        if(itsViewGrid)
        {
            itsViewGrid->Draw(theGTB);
            DrawVerticalAnimationControlInit();
        }
        if(itsTimeControlView)
        {
            if(itsTimeControlViewRect.Height() > 0.)
                itsTimeControlView->Draw(theGTB);
        }
        else
        {
            if(itsTimeControlViewRect.Height() > 0.)
                PrintNoDataMessageOnTimeControlView();
        }

        DrawLastEditedDataSendTime();

        LogWarningForTooLongMapViewDrawTime(timer);

		//std::string spaceoutMessage = "view-grid factors: ";
		//for(float viewCount = 1; viewCount <= 50; viewCount++)
		//{
		//	spaceoutMessage += "\n" + std::to_string(int(viewCount));
		//	auto factor = NFmiVisualizationSpaceoutSettings::calcViewSubGridFactor(viewCount);
		//	spaceoutMessage += " -> " + std::to_string(factor);
		//}
		//itsCtrlViewDocumentInterface->LogAndWarnUser(spaceoutMessage, "", CatLog::Severity::Info, CatLog::Category::Visualization, true);

	}
    catch(...)
    {
    }
    CleanGdiplus(); // tätä pitää kutsua piirron lopuksi InitializeGdiplus -metodin vastin parina.
}

static std::string MakeLastTimeText(const NFmiMetTime &theTime)
{
    if(theTime == NFmiMetTime::gMissingTime)
        return "--:--";
    else
        return std::string(theTime.ToStr("HH:mm"));
}

static NFmiColor GetTimeTextcolor(const NFmiMetTime &theTime, bool fIsLastEditedDataSendHasComeBack)
{
    if(theTime == NFmiMetTime::gMissingTime)
        return NFmiColor(0.4f, 0.4f, 0.4f); // harmaa, jos aika oli puuttuvaa
    else if(fIsLastEditedDataSendHasComeBack)
        return NFmiColor(0.129f, 0.568f, 0.005f); // vihreä, jos data on jo tullut takaisin
    else
        return NFmiColor(0.99f, 0.64f, 0.f); // keltainen, jos data ei ole vielä tullut takaisin
//    else
//        return NFmiColor(0.75f, 0.f, 0.f); // punainen, jos on tullut korruptoitunut operatiivinen data takaisin viimeksi
}

void NFmiEditMapView::DrawLastEditedDataSendTime()
{
    itsLastSendTimeTextRect = NFmiRect(); // nollataan varmuuden vuoksi tämä laatikko aluksi
    bool isMainMapView = itsMapViewDescTopIndex == 0;
    bool notPrinting = !itsCtrlViewDocumentInterface->Printing();
    bool notBetaGenerationRunning = !itsCtrlViewDocumentInterface->BetaProductGenerationRunning();
    bool showLastTimeOnView = itsCtrlViewDocumentInterface->Registry_ShowLastSendTimeOnMapView();
    if(isMainMapView && notPrinting && notBetaGenerationRunning && showLastTimeOnView)
    {
        CtrlViewUtils::GraphicalInfo &graphInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
        auto &lastEditedDataSendTime = itsCtrlViewDocumentInterface->LastEditedDataSendTime();
        std::string timeText = ::MakeLastTimeText(lastEditedDataSendTime);
        NFmiColor usedTextColor = ::GetTimeTextcolor(lastEditedDataSendTime, itsCtrlViewDocumentInterface->IsLastEditedDataSendHasComeBack());
        double fontSizeInMM = 5;
        float fontSizeInPixels = static_cast<float>(fontSizeInMM * graphInfo.itsPixelsPerMM_y);
        NFmiColor frameColor(0.f, 0.f, 0.f);
        NFmiColor fillColor(1.f, 1.f, 1.f, 0.5f);
        FmiDirection textAlingment = kTopRight;
        std::wstring fontName = L"Courier New";
        NFmiPoint textOriginPoint = itsViewGrid->GetFrame().TopRight(); // Tekstien piirto alkaa hilanäytön oikeasta yläkulmasta
        Gdiplus::PointF textOriginPointInPixel = CtrlView::Relative2GdiplusPoint(itsToolBox, textOriginPoint);

        Gdiplus::RectF textBoundingBox = CtrlView::GetStringBoundingBox(*itsGdiPlusGraphics, timeText, fontSizeInPixels, textOriginPointInPixel, fontName);
        textBoundingBox.Inflate(1.05f, 1.f);
        Gdiplus::Rect rectInPixels(
            static_cast<int>(textOriginPointInPixel.X - textBoundingBox.Width), // laatikkoa siirretään vasemmalle oikeasta reunasta tässä
            static_cast<int>(textOriginPointInPixel.Y),
            static_cast<int>(textBoundingBox.Width),
            static_cast<int>(textBoundingBox.Height)
            );

        itsLastSendTimeTextRect = CtrlView::GdiplusRect2Relative(itsToolBox, rectInPixels); // otetaan teksti laatikko talteen tooltip tarkasteluja varten
        CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, frameColor, fillColor, true, true, 1.f);
        CtrlView::DrawTextToRelativeLocation(*itsGdiPlusGraphics, usedTextColor, fontSizeInMM, timeText, textOriginPoint, graphInfo.itsPixelsPerMM_x, itsToolBox, fontName, textAlingment, Gdiplus::FontStyleBold);
    }
}

void NFmiEditMapView::LogWarningForTooLongMapViewDrawTime(NFmiMilliSecondTimer &theTimer)
{
    theTimer.StopTimer();
    if(theTimer.TimeDiffInMSeconds() > 30 * 1000)
    { // Jos karttanäytön piirto kestää yli määrätyn ajan, laitetaan lokiin varoitus
        std::string warningText("Map view #");
        warningText += NFmiStringTools::Convert(itsMapViewDescTopIndex + 1);
        warningText += " drawing lasted long time: ";
        warningText += theTimer.EasyTimeDiffStr();
        itsCtrlViewDocumentInterface->LogAndWarnUser(warningText, "", CatLog::Severity::Warning, CatLog::Category::Visualization, true);
    }
}

// tällä piirretään tavara, joka tulee myös bitmapin päälle
void NFmiEditMapView::DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4)
{
    // Ei jatketa, jos on käynnissä jonkun muun näytön printtaus,printatessa muiden ruutujen päivitys voi aiheuttaa ongelmia...
	if(itsCtrlViewDocumentInterface->Printing() && !IsPrintedMapViewDesctop())
		return ;
	if(itsCtrlViewDocumentInterface->DataModificationInProgress()) // Uusien datan muokkaus rutiinien kanssa ruutujen päivitys voi aiheuttaa ongelmia 
												// Multi-thread ja/tai progress/cancel (-> erillinen muokkaus threadi) systeemit menevät jotenkin sekaisin
		return ;

	if(itsViewGrid)
		itsViewGrid->DrawOverBitmapThings(theGTB, dummy, dummy2, dummy3, dummy4);

	if(itsTimeControlView)
	{
		if(itsTimeControlViewRect.Height() > 0.)
			itsTimeControlView->DrawOverBitmapThings(theGTB, dummy, dummy2, dummy3, dummy4);
	}
}

static bool DoesLabelsFitOnScreen(double theMapAreaHeightInMM, size_t theLabelCount, double theMinLabelHeightInMM, double theMinLabelGapInMM)
{
	return ((theMapAreaHeightInMM/theLabelCount) > (theMinLabelHeightInMM + theMinLabelGapInMM));
}

static long GetNextLongerTimeStep(long theTimeStep)
{
	if(theTimeStep < 5)
		return 5;
	else if(theTimeStep < 10)
		return 10;
	else if(theTimeStep < 15)
		return 15;
	else if(theTimeStep < 30)
		return 30;
	else if(theTimeStep < 60)
		return 60;
	else if(theTimeStep < 120)
		return 120;
	else if(theTimeStep < 180)
		return 180;
	else if(theTimeStep < 360)
		return 360;
	else if(theTimeStep < 720)
		return 720;
	else if(theTimeStep < 1440)
		return 1440;
	else
		return 2880;
}

static bool DoesLabelsFitOnMapVertically(double theMapAreaHeightInMMIn, size_t theLabelCountIn, double labelHeight, double labelGab)
{
	if(theLabelCountIn > 1)
		return (theMapAreaHeightInMMIn / theLabelCountIn) > (labelHeight + labelGab - (labelGab/theLabelCountIn)); // huom1 labelGabeja on yksi vähemmän kuin labeleita
	else
		return (theMapAreaHeightInMMIn / theLabelCountIn) > (labelHeight + labelGab);
}

static const double gMinLabelHeightInMM = 3;
static const double gMaxLabelHeightInMM = 5;
static const double gMinLabelGapInMM = 0;
static const double gMaxLabelGapInMM = 2.5;

static void CalcLabelDimensions(double theMapAreaHeightInMMIn, size_t theLabelCountIn, const NFmiString &theTimeStampFormat, 
								double &theLabelWidthInMMOut, double &theLabelHeightInMMOut, double &theLabelGabInMMOut)
{
	double labelHeight = gMaxLabelHeightInMM;
	double labelGab = gMaxLabelGapInMM;
	bool doesLabelsFit = ::DoesLabelsFitOnMapVertically(theMapAreaHeightInMMIn, theLabelCountIn, labelHeight, labelGab);
	if(doesLabelsFit == false)
	{
		labelGab -= 0.5;
		for( ; ::DoesLabelsFitOnMapVertically(theMapAreaHeightInMMIn, theLabelCountIn, labelHeight, labelGab) == false; )
		{
			labelGab -= 0.5;
			labelHeight -= 0.5;
			if(labelGab < gMinLabelGapInMM || labelHeight < gMinLabelHeightInMM)
				break;  // tämä on virhe, tähän ei pitäisi mennä
		}
	}
	theLabelWidthInMMOut = 20.;
	theLabelHeightInMMOut = labelHeight;
	theLabelGabInMMOut = labelGab;

}

void NFmiEditMapView::MakeLabelPath(Gdiplus::GraphicsPath &theLabelPathOut, NFmiRect &theBaseRectInOut)
{
	// GDI+ GraphicsPath-Outline -metodin vian takia joudun rakentamaan label-pathin kahdesta kaaresta ja kahdesta viivasta
	double ellipseWidth = theBaseRectInOut.Height() * 1.;
	theBaseRectInOut.Left(theBaseRectInOut.Left() + ellipseWidth/2.);
	theBaseRectInOut.Right(theBaseRectInOut.Right() - ellipseWidth/2.);
	theLabelPathOut.AddLine(CtrlView::Relative2GdiplusPoint(itsToolBox, theBaseRectInOut.TopLeft()), CtrlView::Relative2GdiplusPoint(itsToolBox, theBaseRectInOut.TopRight()));
	NFmiRect rightEllipseRect(theBaseRectInOut.Right() - ellipseWidth/2., theBaseRectInOut.Top(), theBaseRectInOut.Right() + ellipseWidth/2., theBaseRectInOut.Bottom());
	theLabelPathOut.AddArc(CtrlView::Relative2GdiplusRect(itsToolBox, rightEllipseRect), 270, 180);
	theLabelPathOut.AddLine(CtrlView::Relative2GdiplusPoint(itsToolBox, theBaseRectInOut.BottomRight()), CtrlView::Relative2GdiplusPoint(itsToolBox, theBaseRectInOut.BottomLeft()));
	NFmiRect leftEllipseRect(theBaseRectInOut.Left() - ellipseWidth/2., theBaseRectInOut.Top(), theBaseRectInOut.Left() + ellipseWidth/2., theBaseRectInOut.Bottom());
	theLabelPathOut.AddArc(CtrlView::Relative2GdiplusRect(itsToolBox, leftEllipseRect), 90, 180);
}

void NFmiEditMapView::DrawVerticalAnimationControl(void)
{

    CtrlViewUtils::GraphicalInfo &graphInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
    NFmiColor frameColor(0.3f, 0.3f, 0.3f);
    NFmiColor fillColor(1, 1, 1, 0.6f);
    NFmiColor selectedFillColor(0.5f, 0.9f, 0.65f, 0.6f);
    NFmiColor strColor(0.f, 0.f, 0.f);

    NFmiMetTime selectedTime = itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex);
    double fontSizeInMM = itsVerticalAnimationInfo.itsLabelHeightInMM * 0.8;
    std::wstring fontNameStr(L"Arial");
    NFmiRect singleLabelRect(itsVerticalAnimationInfo.itsControlRect.TopLeft(), NFmiPoint(itsVerticalAnimationInfo.itsControlRect.Right(), itsVerticalAnimationInfo.itsControlRect.Top() + itsVerticalAnimationInfo.itsRelLabelHeight));
    double yMove = itsVerticalAnimationInfo.itsRelLabelGab + itsVerticalAnimationInfo.itsRelLabelHeight;
    float yMoveInPixels = static_cast<float>(itsToolBox->HYs(yMove));
    itsVerticalAnimationInfo.itsUsedTimes.First();

    Gdiplus::GraphicsPath labelPath;
    MakeLabelPath(labelPath, singleLabelRect);
    Gdiplus::Matrix moveMatrix;
    moveMatrix.Translate(0, yMoveInPixels);
    NFmiColor usedFillColor;

    for(int i = 0; i < itsVerticalAnimationInfo.itsUsedLabelCount - 1; i++)
    {
        const NFmiMetTime &currentTime = itsVerticalAnimationInfo.itsUsedTimes.CurrentTime();
        usedFillColor = fillColor;
        if(currentTime == selectedTime)
            usedFillColor = selectedFillColor;
        CtrlView::DrawPath(*itsGdiPlusGraphics, labelPath, frameColor, usedFillColor, true, true, 1);
        std::string labelStr = currentTime.ToStr(itsVerticalAnimationInfo.itsTimeStampFormat, itsCtrlViewDocumentInterface->Language()).CharPtr();
        CtrlView::DrawTextToRelativeLocation(*itsGdiPlusGraphics, strColor, fontSizeInMM, labelStr, singleLabelRect.Center(), graphInfo.itsPixelsPerMM_x, itsToolBox, fontNameStr, kCenter, Gdiplus::FontStyleBold);
        singleLabelRect += NFmiPoint(0, yMove);
        labelPath.Transform(&moveMatrix);
        itsVerticalAnimationInfo.itsUsedTimes.Next();
    }

    // piirretään lopuksi vielä Play-label
    usedFillColor = fillColor;
    if(itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex).AnimationOn())
        usedFillColor = selectedFillColor;
    CtrlView::DrawPath(*itsGdiPlusGraphics, labelPath, frameColor, usedFillColor, true, true, 1); // pitää kysyä Play statusta, että filli color saadaan oikein
    std::string labelStr = "Play";
    CtrlView::DrawTextToRelativeLocation(*itsGdiPlusGraphics, strColor, fontSizeInMM, labelStr, singleLabelRect.Center(), graphInfo.itsPixelsPerMM_x, itsToolBox, fontNameStr, kCenter);

}

static double MMtoRelativeLength(double lengthInMM, NFmiToolBox &theToolBox, CtrlViewUtils::GraphicalInfo &theGraphicalInfo, bool doX)
{
	if(doX)
	{
		double lengthInPixels = theGraphicalInfo.itsPixelsPerMM_x * lengthInMM;
		double relLength = theToolBox.SXs(lengthInPixels);
		return relLength;
	}
	else // tehdään sitten y sunnassa laskut
	{
		double lengthInPixels = theGraphicalInfo.itsPixelsPerMM_y * lengthInMM;
		double relLength = theToolBox.SYs(lengthInPixels);
		return relLength;
	}
}

void NFmiEditMapView::CalcVerticalAnimationControlRectAndStuff(NFmiVerticalAnimationInfo &theVerticalAnimationInfo)
{
	double rightMarginInMM = 3.; // pieni marginaali oikeaan reunaan
	double controlHeightInMM = (theVerticalAnimationInfo.itsLabelGabInMM + theVerticalAnimationInfo.itsLabelHeightInMM) * theVerticalAnimationInfo.itsUsedLabelCount;
	double controlWidthInMM = theVerticalAnimationInfo.itsLabelWidthInMM;
    auto &graphInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
	double relMarginWidth = ::MMtoRelativeLength(rightMarginInMM, *itsToolBox, graphInfo, true);
	double relControlHeight = ::MMtoRelativeLength(controlHeightInMM, *itsToolBox, graphInfo, false);
	double relControlWidth = ::MMtoRelativeLength(controlWidthInMM, *itsToolBox, graphInfo, true);
	NFmiRect verticalAnimFrame = itsMapRect;
	verticalAnimFrame.Height(relControlHeight);
	verticalAnimFrame.Width(relControlWidth);
	NFmiPoint center = itsMapRect.Center();
	center.X(itsMapRect.Right() - relMarginWidth - relControlWidth/2.);
	verticalAnimFrame.Center(center);
	theVerticalAnimationInfo.itsControlRect = verticalAnimFrame;
	theVerticalAnimationInfo.itsControlRectOnMouseMove = theVerticalAnimationInfo.itsControlRect;

	theVerticalAnimationInfo.itsRelLabelGab = ::MMtoRelativeLength(theVerticalAnimationInfo.itsLabelGabInMM, *itsToolBox, graphInfo, false);
	theVerticalAnimationInfo.itsRelLabelHeight = ::MMtoRelativeLength(theVerticalAnimationInfo.itsLabelHeightInMM, *itsToolBox, graphInfo, false);
}

void NFmiEditMapView::DrawVerticalAnimationControlInit(void)
{
	NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
	if(animData.ShowTimesOnTimeControl() && animData.ShowVerticalControl())
	{
		NFmiTimeBag times = animData.Times();
		size_t usedLabelCount = times.GetSize() + 1; // +1 tulee Play-labelista, joka on alimmaisena
        auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
        double screenPixelSizeXInMM = 1./ graphicalInfo.itsPixelsPerMM_x;
		double screenPixelSizeYInMM = 1./ graphicalInfo.itsPixelsPerMM_y;
		double screenHeightInPixels = itsToolBox->HYs(itsMapRect.Height());
		double mapAreaHeightInMM = screenPixelSizeYInMM * screenHeightInPixels;
		double minLabelHeightInMM = 3;
		double maxLabelHeightInMM = 5;
		double minLabelGapInMM = 0;
		double maxLabelGapInMM = 2.5;
		bool doesLabelsFitOnScreen = ::DoesLabelsFitOnScreen(mapAreaHeightInMM, usedLabelCount, minLabelHeightInMM, minLabelGapInMM);
		if(doesLabelsFitOnScreen == false)
		{ // pitää harventaa labeleiden lukumäärää, niin että ne pienimmillään voisivat mahtua karttaosioon
			long usedStepInMinutes = times.Resolution();
			long newStepInMinutes = 0;
			size_t newLabelCount = 0;
			long timeStretch = times.LastTime().DifferenceInMinutes(times.FirstTime());
			do
			{
				newStepInMinutes = ::GetNextLongerTimeStep(usedStepInMinutes);
				if(newStepInMinutes == usedStepInMinutes) // jos samat, ollaan jo päästy suurimpaan aika-askeleeseen
					return ; // ei toimi, aikoja on liikaa piirrettäväksi jopa suurimmalla aika askeleella (2 vrk), lopetetaan, luultavasti virhetilanne
                newLabelCount = boost::math::iround(static_cast<double>(timeStretch) / newStepInMinutes) + 1;
				usedStepInMinutes = newStepInMinutes;
			}while(::DoesLabelsFitOnScreen(mapAreaHeightInMM, newLabelCount, minLabelHeightInMM, minLabelGapInMM) == false);
			NFmiMetTime lastTime(times.FirstTime()); // uusi viimeinen aika pitää laskea
			lastTime.ChangeByMinutes(static_cast<long>(newStepInMinutes*(newLabelCount-2)));
			times = NFmiTimeBag(times.FirstTime(), lastTime, newStepInMinutes);
		}
		usedLabelCount = times.GetSize() + 1;
		NFmiString timeStampFormat("HH:mm ww");
		if(itsCtrlViewDocumentInterface->Language() != kFinnish)
			timeStampFormat = "HH:mm www"; // UK

		double usedLabelWidthInMM = 0;
		double usedLabelHeightInMM = 0;
		double usedLabelGabInMM = 0;
		::CalcLabelDimensions(mapAreaHeightInMM, usedLabelCount, timeStampFormat, usedLabelWidthInMM, usedLabelHeightInMM, usedLabelGabInMM);

		itsVerticalAnimationInfo = NFmiVerticalAnimationInfo(); // nollataan rakenne
		itsVerticalAnimationInfo.itsLabelGabInMM = usedLabelGabInMM;
		itsVerticalAnimationInfo.itsLabelHeightInMM = usedLabelHeightInMM;
		itsVerticalAnimationInfo.itsLabelWidthInMM = usedLabelWidthInMM;
		itsVerticalAnimationInfo.itsMapAreaHeightInMM = mapAreaHeightInMM;
		itsVerticalAnimationInfo.itsUsedTimes = times;
		itsVerticalAnimationInfo.itsTimeStampFormat = timeStampFormat;
		itsVerticalAnimationInfo.itsUsedLabelCount = static_cast<int>(usedLabelCount);
		CalcVerticalAnimationControlRectAndStuff(itsVerticalAnimationInfo);
		DrawVerticalAnimationControl();
	}
}

// Jos tehdään operaatioita hiirellä karttanäytöllä ja
// hiiri on tämän EditMapView:in alueella, aktivoidaan tämän 
// karttanäytön indeksi ns. aktiiviseksi karttanäyttö aikaindeksiksi.
void NFmiEditMapView::ActivateMapView(const NFmiPoint & thePlace)
{
    if(IsIn(thePlace))
        itsCtrlViewDocumentInterface->ActiveMapDescTopIndex(itsMapViewDescTopIndex);
}

bool NFmiEditMapView::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
    ActivateMapView(thePlace);
	itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
    bool status = false;
    if(itsCtrlViewDocumentInterface->MouseCaptured())
    { // Hiiren kaappaus pitää tarkastella ensin
        if(itsTimeControlView && itsTimeControlView->IsMouseCaptured())
            status = itsTimeControlView->LeftButtonUp(thePlace, theKey);
        else if(itsViewGrid->IsMouseDraggingOn() || itsViewGrid->IsIn(thePlace))
            status = itsViewGrid->LeftButtonUp(thePlace, theKey);
    }

    if(!status)
    {
        // muuten tehdään normaali tarkastelut 
        status = (itsTimeControlView == 0) ? false : itsTimeControlView->LeftButtonUp(thePlace, theKey);
        if(status == false)
            status = itsViewGrid->LeftButtonUp(thePlace, theKey);
    }

    itsCtrlViewDocumentInterface->MouseCaptured(false);
    itsCtrlViewDocumentInterface->LeftMouseButtonDown(false);

	return status;
}

bool NFmiEditMapView::MiddleButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
    ActivateMapView(thePlace);
	if(itsViewGrid && itsViewGrid->IsIn(thePlace))
		return itsViewGrid->MiddleButtonDown(thePlace, theKey);
	return false;
}

bool NFmiEditMapView::MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
    bool status = false;
    ActivateMapView(thePlace);
	if(itsViewGrid && itsViewGrid->IsIn(thePlace))
        status = itsViewGrid->MiddleButtonUp(thePlace, theKey);
	else if(itsTimeControlView && itsTimeControlView->IsIn(thePlace))
        status = itsTimeControlView->MiddleButtonUp(thePlace, theKey);

    itsCtrlViewDocumentInterface->MouseCaptured(false);
    itsCtrlViewDocumentInterface->MiddleMouseButtonDown(false);

    return status;
}

bool NFmiEditMapView::LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) // Marko lisäsi 14.12.2001
{
    ActivateMapView(thePlace);
	if(itsViewGrid)
		return itsViewGrid->LeftDoubleClick(thePlace, theKey);
	return false;
}

bool NFmiEditMapView::RightDoubleClick(const NFmiPoint &thePlace, unsigned long theKey)
{
    ActivateMapView(thePlace);
	if(itsViewGrid)
		return itsViewGrid->RightDoubleClick(thePlace, theKey);
	return false;
}

bool NFmiEditMapView::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
    ActivateMapView(thePlace);
	if(IsIn(thePlace))
	{
		fMouseMovedInCapture = false;
		if(itsTimeControlView && itsTimeControlView->LeftButtonDown(thePlace, theKey))
			return false; // lopetetaan jos oli sisällä ja palautuu false (eli ei tarvetta päivittää ruutua jos hiiri on vasta painettu alas)
		else
			return itsViewGrid->LeftButtonDown(thePlace, theKey);
	}
	return false;
}

bool NFmiEditMapView::RightButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
    ActivateMapView(thePlace);
	if(IsIn(thePlace))
	{
		if(itsCtrlViewDocumentInterface->ModifyToolMode() == CtrlViewUtils::kFmiEditorModifyToolModeBrush)
		{
            itsCtrlViewDocumentInterface->RightMouseButtonDown(true);
            itsCtrlViewDocumentInterface->LeftMouseButtonDown(false);
            itsCtrlViewDocumentInterface->MouseCaptured(true);
			NFmiRect updateRect(itsCtrlViewDocumentInterface->UpdateRect());
			updateRect.Center(thePlace);
            itsCtrlViewDocumentInterface->UpdateRect(updateRect);
		}
		else if(itsCtrlViewDocumentInterface->CrossSectionSystem()->CrossSectionSystemActive())
		{
            itsCtrlViewDocumentInterface->MouseCaptured(true);
            itsCtrlViewDocumentInterface->RightMouseButtonDown(true);
		}
	}
	return false;
}

bool NFmiEditMapView::MouseMove(const NFmiPoint& thePlace, unsigned long theKey)
{ // HUOM!! ei pidä tutkia onko hiiri ikkunan sisällä vaan onko hiiri kaapattu!
    bool status = false;
    if(itsCtrlViewDocumentInterface->MouseCaptured())
    {
        if(itsTimeControlView && itsTimeControlView->IsMouseCaptured())
        {
            itsTimeControlView->MouseMove(thePlace, theKey);
            fMouseMovedInCapture = true;
            return true; // tämä pakottaa ruudun päivityksen, kun aikaikkunaa raahataan
        }
        else if(itsViewGrid->IsMouseDraggingOn() || itsViewGrid->IsIn(thePlace))
            status = itsViewGrid->MouseMove(thePlace, theKey); // otetaan status talteen, sitä käytetään myöhemmin
        if(status)
            fMouseMovedInCapture = true;
    }

    if(itsViewGrid->IsIn(thePlace))
    {
        NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
        bool doVerticalAnimationControl = animData.ShowTimesOnTimeControl() && animData.ShowVerticalControl();
        itsViewGrid->StoreToolTipDataInDoc(thePlace);
        if(itsCtrlViewDocumentInterface->MouseCaptured())
        {
            if(status && itsCtrlViewDocumentInterface->MiddleMouseButtonDown() && itsCtrlViewDocumentInterface->MapMouseDragPanMode())
                return true; // jos ollaan "mouse drag pan" -moodissa, pitää kartta näyttö päivittää
        }
        else if(doVerticalAnimationControl && itsVerticalAnimationInfo.itsControlRectOnMouseMove.IsInside(thePlace))
        { // tehdään ajan säädöt
            double placeYInRect = thePlace.Y() - itsVerticalAnimationInfo.itsControlRect.Top();
            int index = static_cast<int>(::floor((placeYInRect / itsVerticalAnimationInfo.itsControlRect.Height())* itsVerticalAnimationInfo.itsUsedLabelCount));
            if(index == itsVerticalAnimationInfo.itsUsedLabelCount - 1)
            {
                animData.AnimationOn(true);
                itsCtrlViewDocumentInterface->RefreshApplicationViewsAndDialogs("Map view: Mouse move hovering over Animation time labels 'animate' label", GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
                return false; // Palauta false, koska edellä tehtiin jo ruutujen päivitys oikeilla loki teksteillä
            }
            else if(itsVerticalAnimationInfo.itsUsedTimes.SetTime(index))
            {
                animData.AnimationOn(false);
                NFmiMetTime newTime = itsVerticalAnimationInfo.itsUsedTimes.CurrentTime();
                NFmiMetTime oldTime = itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex);
                if(newTime != oldTime)
                {
                    // Tämä laittaa optimoidun näyttöjen päivityksen päälle, joten seuraavan rivin RefreshApplicationViewsAndDialogs voi olla ilman näyttö maskia
                    itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex, newTime); 
                    itsCtrlViewDocumentInterface->RefreshApplicationViewsAndDialogs("Map view: Mouse move hovering over Animation time labels sigle valid time label");
                    return false; // Palauta false, koska edellä tehtiin jo ruutujen päivitys oikeilla loki teksteillä
                }
            }
        }
    }

    return false;
}

bool NFmiEditMapView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
    ActivateMapView(thePlace);
	if(itsViewGrid && itsViewGrid->IsIn(thePlace))
	{
		return itsViewGrid->MouseWheel(thePlace, theKey, theDelta);
	}
	else if(itsTimeControlView && itsTimeControlView->MouseWheel(thePlace, theKey, theDelta))
		return true;

	return false;
}

bool NFmiEditMapView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
    ActivateMapView(thePlace);
    itsCtrlViewDocumentInterface->MouseCaptured(false);
    itsCtrlViewDocumentInterface->RightMouseButtonDown(false);
    itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
	if(itsViewGrid && itsViewGrid->IsIn(thePlace))
	{
		return itsViewGrid->RightButtonUp(thePlace, theKey);
	}
	else if(itsTimeControlView && itsTimeControlView->IsIn(thePlace))
	{	// tee GetFrame():a käyttäen
		return itsTimeControlView->RightButtonUp(thePlace, theKey);
	}
	return false;
}

void NFmiEditMapView::Update()
{
	if(!this)
		return;
	UpdateMap();
	if(itsViewGrid)
	{
		NFmiRect mapRect = itsCtrlViewDocumentInterface->RelativeMapRect(itsMapViewDescTopIndex);
		itsViewGrid->Update(mapRect, itsToolBox);
		itsViewGrid->Update();
		UpdateTimeControlView(); // drawparamlistin pitää olla 'likainen' ennen kuin updatetaan tätä!!!
	}
}

void NFmiEditMapView::UpdateTimeControlView(void)
{
	NFmiRect mapRect = itsCtrlViewDocumentInterface->RelativeMapRect(itsMapViewDescTopIndex);
	itsTimeControlViewRect = NFmiRect(0, mapRect.Height(), 1., 1);
	CreateTimeControlView();
}

void NFmiEditMapView::UpdateMap(void)
{
    auto mapHandler = itsCtrlViewDocumentInterface->GetMapHandlerInterface(itsMapViewDescTopIndex);
	if(mapHandler->MakeNewBackgroundBitmap())
	{
        itsCtrlViewDocumentInterface->DoAutoZoom(itsMapViewDescTopIndex);
		NFmiRect mapRect = itsCtrlViewDocumentInterface->RelativeMapRect(itsMapViewDescTopIndex);
		SetMapAreaAndRect(mapHandler->Area(), mapRect);
	}
}


// TÄMÄ SIIRTYY POIS TÄÄLTÄ????
void NFmiEditMapView::SetMapAreaAndRect(const boost::shared_ptr<NFmiArea> &theArea, const NFmiRect& theRect)
{
	if(theArea)
	{
		itsMapArea = theArea;
		itsMapRect = theRect;
		itsMapArea->SetXYArea(itsMapRect); // siirtää suhteellistä aluetta
	}
}

void NFmiEditMapView::PrintNoDataMessageOnTimeControlView(void)
{
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.EnableFrame();
	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	envi.SetFillColor(NFmiColor(1.f,1.f,1.f));
	NFmiRectangle rect(itsTimeControlViewRect.TopLeft()
					  ,itsTimeControlViewRect.BottomRight()
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rect);

	if(itsCtrlViewDocumentInterface->EditedDataNeedsToBeLoaded())
	{
		NFmiString str("Loading data, please wait.\nElapsed time: ");
		str += NFmiMilliSecondTimer::EasyTimeDiffStr(itsCtrlViewDocumentInterface->EditedDataNeedsToBeLoadedTimer().CurrentTimeDiffInMSeconds(), true);
		NFmiPoint place = itsTimeControlViewRect.TopLeft();
		place.X(place.X() + GetFrame().Width()/100.);
		envi.SetFontSize(NFmiPoint(22,22));
		NFmiText text(place, str, false, 0, &envi);
		itsToolBox->Convert(&text);
	}
	else
	{
		NFmiString str(::GetDictionaryString("MapViewNoData"));
		NFmiPoint place = itsTimeControlViewRect.TopLeft();
		place.X(place.X() + itsTimeControlViewRect.Width()/100.);
		envi.SetFontSize(NFmiPoint(18,18));
		NFmiText text(place, str, false, 0, &envi);
		itsToolBox->Convert(&text);
	}
}

// etsii 1. mahd. synop-plot -näytön
NFmiSynopPlotView* NFmiEditMapView::GetSynopPlotViewFromToolTipPos(bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot)
{
	NFmiDrawParamList *drawParamList = itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface().getDrawParamListWithRealRowNumber(itsMapViewDescTopIndex, itsCtrlViewDocumentInterface->ToolTipRealRowIndex());
	if(drawParamList)
	{
		// ensin etsitään normaalia synop-plot paramia
		NFmiInfoData::SpecialParameters usedSpecialParam = NFmiInfoData::kFmiSpSynoPlot;
		if(fDrawSoundingPlot)
			usedSpecialParam = NFmiInfoData::kFmiSpSoundingPlot;
		else if(fDrawMinMaxPlot)
			usedSpecialParam = NFmiInfoData::kFmiSpMinMaxPlot;
		else if(fDrawMetarPlot)
			usedSpecialParam = NFmiInfoData::kFmiSpMetarPlot;

		NFmiParam param(usedSpecialParam);
		NFmiDataIdent dataIdent(param);
		bool ignoreLevelInfo = false;
		if(fDrawSoundingPlot)
			ignoreLevelInfo = true;
		if(drawParamList->Find(dataIdent, 0, NFmiInfoData::kAnyData, true, ignoreLevelInfo))
		{
			auto realRowIndex = itsCtrlViewDocumentInterface->ToolTipRealRowIndex();
			NFmiCtrlView *view = itsViewGrid->GetViewWithRealRowIndex(realRowIndex, itsCtrlViewDocumentInterface->ToolTipTime(), dataIdent, true);
			if(view)
			{
				return dynamic_cast<NFmiSynopPlotView*>(view);
			}
		}
	}
	return 0;
}

// uuden tooltip systeemin CPPTooltip piirto
void NFmiEditMapView::DrawSynopPlotOnToolTip(NFmiToolBox * theToolBox, const NFmiRect &theRect, bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot)
{
	NFmiSynopPlotView *synopView = GetSynopPlotViewFromToolTipPos(fDrawSoundingPlot, fDrawMinMaxPlot, fDrawMetarPlot);
	if(synopView)
	{
		synopView->DrawSynopPlot(theToolBox, NFmiLocation(itsCtrlViewDocumentInterface->ToolTipLatLonPoint()), theRect, itsCtrlViewDocumentInterface->ToolTipTime(), 3.0, fDrawSoundingPlot, fDrawMinMaxPlot, fDrawMetarPlot); // 2.0 mm fontit
	}
}

struct null_deleter
{
	void operator()(void const *) const
	{
	}
};

NFmiString NFmiEditMapView::GetToolTipText(void)
{
    return "";
}

NFmiString NFmiEditMapView::Value2String(float theValue, int theDigitCount, FmiInterpolationMethod theInterpolationMethod, FmiParamType theParamType)
{
	NFmiString str;
	if(theDigitCount == 0 && theInterpolationMethod != kLinearly && theParamType != kContinuousParam)
        str = NFmiValueString(boost::math::iround(theValue), "%d");
	else if(theDigitCount == 0) // lisätään linear + 0-tapauksessa kuitenkin yksi desimaali
		str = NFmiValueString(theValue, "%0.1f");
	else
	{
		NFmiString format("%0.");
		format += NFmiValueString(theDigitCount, "%d");
		format += "f";
		str = NFmiValueString(theValue, format);
	}
	return str;
}

// käy asettamassa kaikkiin mahdollisiin näyttöihin oikeat ajat
void NFmiEditMapView::SetViewTimes(void)
{
	if(itsViewGrid)
		itsViewGrid->Time(itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex));
}

std::string NFmiEditMapView::GetLastSendTimeTooltipText()
{
    std::string str;
    if(itsCtrlViewDocumentInterface->LastEditedDataSendTime() == NFmiMetTime::gMissingTime)
        str += ::GetDictionaryString("No edited data has been sent yet");
    else
    {
        str += ::GetDictionaryString("Last edited data was sent at");
        str += "\n";
        str += itsCtrlViewDocumentInterface->LastEditedDataSendTime().ToStr("HH:mm Utc (YYYY.MM.DD)", itsCtrlViewDocumentInterface->Language());
        str += "\n";
        if(itsCtrlViewDocumentInterface->IsLastEditedDataSendHasComeBack())
            str += ::GetDictionaryString("New operational data has been received after that");
        else
            str += ::GetDictionaryString("No new operational data has been received since");
    }
    return str;
}

std::string NFmiEditMapView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
    if(itsLastSendTimeTextRect.IsInside(theRelativePoint))
        return GetLastSendTimeTooltipText();

	std::string str = GetToolTipText(); // täältä tulee tiettyjen työkalujen tooltip tekstit
	if(str.empty() == false)
		return str;

	// jos noilta työkaluilta ei tullut mitään, kysytään muilta komponenteilta tekstiä
	if(itsViewGrid && itsViewGrid->IsIn(theRelativePoint))
		return itsViewGrid->ComposeToolTipText(theRelativePoint);
	else if(itsTimeControlView && itsTimeControlView->IsIn(theRelativePoint))
		return itsTimeControlView->ComposeToolTipText(theRelativePoint);
	if(itsCtrlViewDocumentInterface->EditedSmartInfo() == 0)
		return std::string();

	return std::string();
}

NFmiStationViewHandler* NFmiEditMapView::GetMapViewHandler(int theRowIndex, int theIndex)
{
	if(itsViewGrid)
        return itsViewGrid->GetMapView(theRowIndex, theIndex);
    return 0;
}
