#include "NFmiTimeControlView.h"
#include "NFmiDrawParam.h"
#include "NFmiAdjustedTimeScaleView.h"
#include "NFmiDrawParamList.h"
#include "NFmiStepTimeScale.h"
#include "NFmiToolBox.h"
#include "NFmiRectangle.h"
#include "NFmiLine.h"
#include "NFmiText.h"
#include "NFmiValueString.h"
#include "NFmiTimeScale.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiTrajectorySystem.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewDocumentInterface.h"
#include "CtrlViewFunctions.h"
#include "FmiSmartMetEditingMode.h"
#include "GraphicalInfo.h"
#include "NFmiAnimationData.h"
#include "NFmiTimeDescriptor.h"
#include "catlog/catlog.h"
#include "SmartMetViewId.h"
#include "ApplicationInterface.h"
#include "CtrlViewWin32Functions.h"
#include "NFmiColorSpaces.h"
#include "NFmiVirtualTimeData.h"


#include <gdiplus.h>
#include "boost\math\special_functions\round.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

AnimationButtonImageHolder NFmiTimeControlView::statAnimationButtonImages;
SmartMetViewId g_EditedDataTimeRangeChangedUpdateViewIdMask = SmartMetViewId::AllMapViews | SmartMetViewId::DataFilterToolDlg;

//_________________________________________________________ AnimationButtonImageHolder

// initialisoinnissa luetaan bitmapit tiedostoista, kutsu vasta kun itsBitmapFolder-dataosa on asetettu
// HUOM! heitt‰‰ poikkeuksia ep‰onnistuessaan
void AnimationButtonImageHolder::Initialize(void)
{
	fInitialized = true;
	itsPlayButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "control_play.png");
	itsPauseButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "control_pause.png");
	itsDelayButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "time.png");
	itsCloseButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "cross.png");
	itsRepeatButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "control_repeat_blue.png");
	itsAnimationLockButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "time_go.png");
	itsAnimationNoLockButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "time_delete.png");
	itsLastFrameDelayButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "time_add.png");
	itsVerticalTimeControlOnImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "vertical_time_control.png");
    itsVerticalTimeControlOffImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "vertical_time_control_delete.png");
    itsSetFullTimeRangeImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "full_time_range.png");
}




//_________________________________________________________ NFmiTimeControlView

NFmiTimeControlView::NFmiTimeControlView(int theMapViewDescTopIndex, const NFmiRect& theRect
										,NFmiToolBox *theToolBox
										,boost::shared_ptr<NFmiDrawParam> &theDrawParam
										,bool theShowResolutionChangerBox
										,bool theDrawViewInMouseMove
										,bool theShowEditTimeHandles
										,bool theShowSelectedTimes
										,bool theDrawTimeFilterTimesInNormalDraw
										,double theAxisShrinkFactor)

:NFmiCtrlView(theMapViewDescTopIndex, theRect
			 ,theToolBox
			 ,theDrawParam)
,itsTimeView(0)
,itsTimeAxis(0)
,fHasData(false)
,itsTimeBag(0)
,fShowResolutionChangerBox(theShowResolutionChangerBox)
,itsResolutionChangerBox()
,fMouseCaptured(false)
,fMouseCapturedMoveTimeLine(false)
,fMouseCapturedMoveSelectedTime(false)
,fMouseMovedWhenCaptured(false)
,itsLastMousePosition()
,fAnimationCloseButtonPressed(false)
,fAnimationPlayButtonPressed(false)
,fAnimationDelayButtonPressed(false)
,fAnimationRunModeButtonPressed(false)
,fAnimationLastFrameDelayButtonPressed(false)
,fAnimationLockModeButtonPressed(false)
,fAnimationVerticalControlButtonPressed(false)
,itsLeftTimeHandle()
,itsRightTimeHandle()
,fDrawViewInMouseMove(theDrawViewInMouseMove)
,fShowEditTimeHandles(theShowEditTimeHandles)
,itsAxisShrinkFactor(theAxisShrinkFactor)
,fDrawTimeFilterTimesInNormalDraw(theDrawTimeFilterTimesInNormalDraw)
,itsButtonSizeInMM_x(4.9)
,itsButtonSizeInMM_y(4.9)
,itsButtonOffSetFromEdgeFactor(0.25)
{
}

void NFmiTimeControlView::Clear(void)
{
	if(itsTimeAxis)
	{
		delete itsTimeAxis;
		itsTimeAxis = 0;
	}
	if(itsTimeView)
	{
		delete itsTimeView;
		itsTimeView = 0;
	}
	if(itsTimeBag)
	{
		delete itsTimeBag;
		itsTimeBag = 0;
	}
}

// yritys korjata konstruktorissa olevaa virtuaalisuus ongelmaa
// Konstruktori ei n‰emm‰ kutsu virtuaali funktioita niin kuin pit‰isi,
// kokeilen ett‰ joska erillinen funktio fixaisi ongelman.
void NFmiTimeControlView::Initialize(bool theShowSelectedTimes, bool theUseActiveMapTime) 
{
	if(statAnimationButtonImages.fInitialized == false)
	{
		try
		{
			statAnimationButtonImages.itsBitmapFolder = itsCtrlViewDocumentInterface->HelpDataPath() + "\\res";
			statAnimationButtonImages.Initialize();
		}
		catch(std::exception &e)
		{
			std::string errStr("Error in NFmiTimeControlView::Initialize, while trying to read animation button bitmaps from files: \n");
			errStr += e.what();
            itsCtrlViewDocumentInterface->LogAndWarnUser(errStr, "Error while trying to read animation button bitmaps", CatLog::Severity::Error, CatLog::Category::Configuration, false);
		}
	}

	Clear();

	if(itsCtrlViewDocumentInterface->DefaultEditedDrawParam())
	{
		if(fShowResolutionChangerBox)
		{
			NFmiRect frame(GetFrame());
			itsResolutionChangerBox = CalcResolutionChangerBoxRect();
		}

		fHasData = true;
		itsTimeBag = new NFmiTimeBag(GetUsedTimeBag());
		itsTimeAxis = new NFmiStepTimeScale(*itsTimeBag);

		NFmiDrawingEnvironment theEnvironment; // t‰st‰ envist‰ en oikein tied‰, pit‰isikˆ k‰ytt‰‰ yleist‰ vai ei??
		NFmiColor color(0,1,0);
		theEnvironment.SetFillColor(color);
		theEnvironment.EnableFill();
		NFmiStepTimeScale theSelectedScale(MakeEmptySelectedTimeScale());

        NFmiPoint fontSize(CtrlViewUtils::CalcTimeScaleFontSizeInPixels(GetGraphicalInfo().itsPixelsPerMM_x));

		itsTimeView = new NFmiAdjustedTimeScaleView(itsMapViewDescTopIndex, CalcTimeAxisRect()
										,itsToolBox
										,itsTimeAxis
										,&theSelectedScale
										,0
										,&theEnvironment
										,0
										,true
										,fontSize
										,theShowSelectedTimes
                                        ,theUseActiveMapTime);
		int bull = 0;
		itsTimeView->SetBorderWidth(bull);
	}
}

NFmiTimeControlView::~NFmiTimeControlView(void)
{
	Clear();
}

void NFmiTimeControlView::Draw(NFmiToolBox * theGTB)
{
	itsToolBox = theGTB;
	try
	{
		InitializeGdiplus(itsToolBox, &GetFrame());

		itsToolBox->SetTextAlignment(kLeft);
		DrawBackground();
		if(fHasData)
		{
			DrawTimeAxis();
			DrawAnimationBox();
			if(fDrawTimeFilterTimesInNormalDraw)
				DrawTimeFilterTimes(); // t‰m‰ piirret‰‰n DrawOverBitmapThings-metodsissa kaiken p‰‰lle esim. karttan‰ytˆn yhteydess‰

			((NFmiAdjustedTimeScaleView*)itsTimeView)->DrawSelectedTimes();
			DrawResolutionChangerBox();
			DrawVirtualTimeData();
		}
		else
			DrawNoDataAvailable();
	}
	catch(...)
	{
	}
	CleanGdiplus(); // t‰t‰ pit‰‰ kutsua piirron lopuksi InitializeGdiplus -metodin vastin parina.
}

NFmiPoint NFmiTimeControlView::GetViewSizeInPixels(void)
{
	return itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex);
}

static void DrawCenteredText(Gdiplus::Graphics *theGdiPlusGraphics, const NFmiColor &theColor, double theFontSizeInMM, const std::string &theStr, const NFmiPoint &thePlace, double pixelsPerMM, NFmiToolBox *theToolbox)
{
	Gdiplus::Color usedColor(CtrlView::NFmiColor2GdiplusColor(theColor));
	Gdiplus::SolidBrush aBrush(usedColor);
	Gdiplus::StringFormat stringFormat;
    CtrlView::SetGdiplusAlignment(kCenter, stringFormat);
	float usedFontSizeInPixels = static_cast<float>(theFontSizeInMM * pixelsPerMM);
	std::wstring fontNameStr(L"Arial");
	Gdiplus::Font aFont(fontNameStr.c_str(), usedFontSizeInPixels, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

	std::wstring wideStr = CtrlView::StringToWString(theStr);
	Gdiplus::PointF aPlace = CtrlView::Relative2GdiplusPoint(theToolbox, thePlace);
	theGdiPlusGraphics->DrawString(wideStr.c_str(), static_cast<INT>(wideStr.size()), &aFont, aPlace, &stringFormat, &aBrush);
}

bool NFmiTimeControlView::IsAnimationViewVisible() const
{
    return AllowAnimationBoxShown() && itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex).ShowTimesOnTimeControl();
}

// Annetaan varmuuden vuoksi lokaalisti luotu gdiplus -piirto otus parametrina (theUsedGdiPlusGraphics).
// Aikas‰‰timen full-range -nappula piirret‰‰n vain karttan‰yttˆjen aikakontrolli-ikkunoihin, mink‰ AllowAnimationBoxShown -metodi kertoo.
// Kun animaatio ikkuna on n‰kyviss‰, ei piirret‰ t‰t‰ nappulaa selkeyden vuoksi (p‰‰llekk‰isyyksi‰ piirrossa ja hit-boksissa).
void NFmiTimeControlView::DrawFullTimeRangeButton(Gdiplus::Graphics *theUsedGdiPlusGraphics)
{
    if(!IsAnimationViewVisible())
    {
        if(!itsCtrlViewDocumentInterface->Printing())
        {
            CtrlView::DrawAnimationButton(CalcFullTimeRangeButtonRect(), statAnimationButtonImages.itsSetFullTimeRangeImage, theUsedGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.3f);
        }
    }
}

void NFmiTimeControlView::DrawAnimationBox(void)
{
    if(IsAnimationViewVisible())
	{
		// sininen animaatio alue pit‰‰ klipata aikaskaala asteikkoon
		NFmiRect timeScaleRect = ((NFmiAdjustedTimeScaleView*)itsTimeView)->CalcTotalTimeScaleArea();
		itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, timeScaleRect));

		// Piirret‰‰n animaatioaikav‰li vaalean sinisell‰ l‰pin‰kyv‰ll‰ laatikolla
		NFmiRect animRect(CalcAnimationBoxRect());
		NFmiColor animBoxColor(0.f, 0.498f, 1.0f, 0.608f);
		Gdiplus::SolidBrush aBrushBox(CtrlView::NFmiColor2GdiplusColor(animBoxColor));
		Gdiplus::GraphicsPath aPath;
        CtrlView::MakePathFromRect(aPath, itsToolBox, animRect);
		itsGdiPlusGraphics->FillPath(&aBrushBox, &aPath);

		// kaikki nappulat ja muut pit‰‰ clipata animaatio boksin sis‰‰n
		NFmiRect buttonClipRect = timeScaleRect;
		buttonClipRect = buttonClipRect.Intersection(animRect);
		itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, buttonClipRect));
		// piirr‰ close-button
        CtrlView::DrawAnimationButton(CalcAnimationCloseButtonRect(), statAnimationButtonImages.itsCloseButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f, true);

        NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);

		// piirr‰ play/pause-button
		Gdiplus::Bitmap *usedPlayButtonBitmap = statAnimationButtonImages.itsPlayButtonImage;
		if(animData.AnimationOn() == true)
			usedPlayButtonBitmap = statAnimationButtonImages.itsPauseButtonImage;
        CtrlView::DrawAnimationButton(CalcAnimationPlayButtonRect(), usedPlayButtonBitmap, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f, true);

		// piirr‰ vertical control -button
		Gdiplus::Bitmap *usedVerticalControlButtonBitmap = statAnimationButtonImages.itsVerticalTimeControlOnImage;
		if(animData.ShowVerticalControl() == true)
			usedVerticalControlButtonBitmap = statAnimationButtonImages.itsVerticalTimeControlOffImage;
        CtrlView::DrawAnimationButton(CalcAnimationVerticalControlButtonRect(), usedVerticalControlButtonBitmap, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.9f, true);

		{
			// piirr‰ (time)delay-button
			NFmiRect delayRect = CalcAnimationDelayButtonRect();
            CtrlView::DrawAnimationButton(delayRect, statAnimationButtonImages.itsDelayButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f, true);
			// piirret‰‰n napin yl‰ puolelle k‰ytetty delay aika sadasosa sekunneilla
			NFmiColor textColor(0.f, 0.f, 0.0f);
			std::string delayStr = NFmiValueString::GetStringWithMaxDecimalsSmartWay(animData.FrameDelayInMS()/10., 0);
			NFmiPoint delayStrPlace = delayRect.Center();
			delayStrPlace.Y(delayStrPlace.Y() - delayRect.Height() * 0.7);
			::DrawCenteredText(itsGdiPlusGraphics, textColor, itsButtonSizeInMM_y * 0.7, delayStr, delayStrPlace, GetGraphicalInfo().itsPixelsPerMM_y, itsToolBox);
		}

		// piirr‰ runmode-button
        CtrlView::DrawAnimationButton(CalcAnimationRunModeButtonRect(), statAnimationButtonImages.itsRepeatButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f, true);

		// piirr‰ lockmode-button
		NFmiRect lockModeRect(CalcAnimationLockModeButtonRect());
		Gdiplus::Bitmap *usedLockButtonBitmap = statAnimationButtonImages.itsAnimationLockButtonImage;
		if(animData.LockMode() == NFmiAnimationData::kFollowEarliestLastObservation)
			usedLockButtonBitmap = statAnimationButtonImages.itsAnimationNoLockButtonImage;
        CtrlView::DrawAnimationButton(lockModeRect, usedLockButtonBitmap, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f, true);

		{
			// piirr‰ last frame delay factor-button
			NFmiRect lastFrameDelayRect = CalcLastFrameDelayFactorButtonRect();
            CtrlView::DrawAnimationButton(lastFrameDelayRect, statAnimationButtonImages.itsLastFrameDelayButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f, true);
			// piirret‰‰n napin yl‰ puolelle k‰ytetty delay kerroin
			NFmiColor textColor(0.f, 0.f, 0.0f);
			std::string lasframeDelayStr = NFmiStringTools::Convert<double>(animData.LastFrameDelayFactor());
			lasframeDelayStr += "x";
			NFmiPoint lastFrameDelayStrPlace = lastFrameDelayRect.Center();
			lastFrameDelayStrPlace.Y(lastFrameDelayStrPlace.Y() - lastFrameDelayRect.Height() * 0.7);
			::DrawCenteredText(itsGdiPlusGraphics, textColor, itsButtonSizeInMM_y * 0.7, lasframeDelayStr, lastFrameDelayStrPlace, GetGraphicalInfo().itsPixelsPerMM_y, itsToolBox);
		}
		itsGdiPlusGraphics->ResetClip();
	}
}

// Jos ollaan virtual-time moodissa piirret‰‰n seuraavaa (purpppura v‰rill‰):
// 1. Vasempaan alakulmaaan laatikko, jossa tekstit: VT Www<br>MM.DD HH:mm
// 2. Alaraunaa horisontaali palkki, jota voi klikata.
// 3. Jos virtual-time n‰kyy aika-asteikolla, piirret‰‰n sen kohtaan pysty palkki tai merkki ja VT kirjaimet
void NFmiTimeControlView::DrawVirtualTimeData()
{
	if(itsCtrlViewDocumentInterface->VirtualTimeUsed())
	{
		DrawVirtualTimeSlider();
		DrawVirtualTimeDataBox();
		// Close buttonin piirto pit‰‰ olla DrawVirtualTimeDataBox metodin kutsun per‰ss‰!
		CtrlView::DrawAnimationButton(CalcVirtualTimeCloseButtonRect(), statAnimationButtonImages.itsCloseButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f);
	}
}

void NFmiTimeControlView::DrawVirtualTimeDataBox()
{
	auto& graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);

	Gdiplus::StringFormat stringFormat;
	stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
	stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);

	const auto& virtualTime = itsCtrlViewDocumentInterface->VirtualTime();
	Gdiplus::REAL fontSize = 17;
	Gdiplus::Font aFont(L"Arial", fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	auto str1 = virtualTime.ToStr("VT HH:mm", kEnglish);
	std::wstring wString1 = CtrlView::StringToWString(std::string(str1));
	auto str2 = virtualTime.ToStr("YYYY.MM.DD", kEnglish);
	std::wstring wString2 = CtrlView::StringToWString(std::string(str2));

	Gdiplus::RectF boundingBox1;
	itsGdiPlusGraphics->MeasureString(wString1.c_str(), INT(wString1.size()), &aFont, Gdiplus::PointF(0, 0), &stringFormat, &boundingBox1);
	Gdiplus::RectF boundingBox2;
	itsGdiPlusGraphics->MeasureString(wString2.c_str(), INT(wString2.size()), &aFont, Gdiplus::PointF(0, 0), &stringFormat, &boundingBox2);

	float penThicknessInMM = 0.5f;
	auto [relativePenThickness, penThicknessInPixels] = ConvertMilliMetersToRelativeAndPixels(penThicknessInMM, false);

	// HUOM! virtualTimeBox on pikseli maailmassa
	NFmiRect virtualTimeBox;
	virtualTimeBox.Size(NFmiPoint(FmiMax(boundingBox1.Width, boundingBox2.Width) * 1.1, (boundingBox1.Height + boundingBox2.Height) * 1.f));
	auto usedRelativeBaseRect = itsTimeView->GetRelativeRect();
	auto newPlace = usedRelativeBaseRect.Place();
	newPlace.Y(newPlace.Y() - itsVirtualTimeSliderRect.Height() - relativePenThickness);
	usedRelativeBaseRect.Place(newPlace);
	CtrlView::PlaceBoxIntoFrame(virtualTimeBox, usedRelativeBaseRect, itsToolBox, kBottomRight);

	const auto& baseColor = NFmiVirtualTimeData::virtualTimeBaseColor;
	NFmiColor fillColor = NFmiColorSpaces::GetBrighterColor(baseColor, 50.);
	fillColor.Alpha(0.25f);
	Gdiplus::SolidBrush aBrushBox(CtrlView::NFmiColor2GdiplusColor(fillColor));
	Gdiplus::GraphicsPath aPath;
	Gdiplus::Rect gdiRect(static_cast<INT>(virtualTimeBox.Left()), static_cast<INT>(virtualTimeBox.Top()), static_cast<INT>(virtualTimeBox.Width()), static_cast<INT>(virtualTimeBox.Height()));
	// Otetaan t‰ss‰ vaiheessa talteen VT-laatikon suhteellinen rect
	itsVirtualTimeBoxRect = CtrlView::GdiplusRect2Relative(itsToolBox, gdiRect);
	aPath.AddRectangle(gdiRect);
	aPath.CloseFigure();
	itsGdiPlusGraphics->FillPath(&aBrushBox, &aPath);

	Gdiplus::Pen penBox(CtrlView::NFmiColor2GdiplusColor(NFmiColor()), static_cast<Gdiplus::REAL>(penThicknessInPixels));
	itsGdiPlusGraphics->DrawPath(&penBox, &aPath);

	NFmiPoint center = virtualTimeBox.Center();
	NFmiPoint topleft = virtualTimeBox.TopLeft();

	Gdiplus::PointF timeString1OffSet(static_cast<Gdiplus::REAL>(center.X()), static_cast<Gdiplus::REAL>(topleft.Y())); // t‰m‰ offset on suhteellinen laskettuun aika-string boxiin
	Gdiplus::PointF timeString2OffSet(timeString1OffSet);
	timeString2OffSet.Y += fontSize;

	Gdiplus::SolidBrush aBrushText1(CtrlView::NFmiColor2GdiplusColor(baseColor));
	itsGdiPlusGraphics->DrawString(wString1.c_str(), static_cast<INT>(wString1.size()), &aFont, timeString1OffSet, &stringFormat, &aBrushText1);

	Gdiplus::SolidBrush aBrushText2(CtrlView::NFmiColor2GdiplusColor(baseColor));
	itsGdiPlusGraphics->DrawString(wString2.c_str(), static_cast<INT>(wString2.size()), &aFont, timeString2OffSet, &stringFormat, &aBrushText2);
}

std::pair<double, int> NFmiTimeControlView::ConvertMilliMetersToRelativeAndPixels(double valueInMM, bool doDirectionX)
{
	auto& graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
	auto valueInPixels = boost::math::iround(valueInMM * (doDirectionX ? graphicalInfo.itsPixelsPerMM_x : graphicalInfo.itsPixelsPerMM_y));
	auto relativeValue = doDirectionX ? itsToolBox->SX(valueInPixels) : itsToolBox->SY(valueInPixels);
	return std::make_pair(relativeValue, valueInPixels);
}

void NFmiTimeControlView::DrawVirtualTimeSlider()
{
	auto& graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
	float sliderRectHeightInMM = 2.2f;
	auto [relativeHeight, heigthInPixels] = ConvertMilliMetersToRelativeAndPixels(sliderRectHeightInMM, false);

	itsVirtualTimeSliderRect = itsTimeView->GetRelativeRect();
	float penThicknessInMM = 0.6f;
	auto [relativePenThickness, penThicknessInPixels] = ConvertMilliMetersToRelativeAndPixels(penThicknessInMM, false);
	itsVirtualTimeSliderRect.Top(itsVirtualTimeSliderRect.Bottom() - relativeHeight - (relativePenThickness / 2.));
	itsVirtualTimeSliderRect.Height(relativeHeight);
	auto rectInPixels = CtrlView::Relative2GdiplusRect(itsToolBox, itsVirtualTimeSliderRect);

	const auto& baseColor = NFmiVirtualTimeData::virtualTimeBaseColor;
	NFmiColor fillColor = NFmiColorSpaces::GetBrighterColor(baseColor, 45.);
	fillColor.Alpha(0.35f);
	CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, baseColor, fillColor, true, true, static_cast<float>(penThicknessInPixels));
	DrawVirtualTimeMarker();
}

// Piirret‰‰n k‰rjell‰‰n seisova kolmio virtual-timen kohdalle,
// slider-laatikon p‰‰lle.
void NFmiTimeControlView::DrawVirtualTimeMarker()
{
	auto& graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
	float markerHeightInMM = 6.f;
	auto [relativeHeight, heigthInPixels] = ConvertMilliMetersToRelativeAndPixels(markerHeightInMM, false);

	const auto& baseColor = NFmiVirtualTimeData::virtualTimeBaseColor;
	Gdiplus::SolidBrush aBrushBox(CtrlView::NFmiColor2GdiplusColor(baseColor));
	auto timePointX = Time2Value(itsCtrlViewDocumentInterface->VirtualTime());
	// p1 on k‰rki kolmion piste, joka on sliderin yl‰osassa
	NFmiPoint p1(timePointX, itsVirtualTimeSliderRect.Top());
	NFmiPoint p2(timePointX - (relativeHeight / 3.), itsVirtualTimeSliderRect.Top() - relativeHeight);
	NFmiPoint p3(timePointX + (relativeHeight / 3.), itsVirtualTimeSliderRect.Top() - relativeHeight);
	auto gdiplusP1 = CtrlView::Relative2GdiplusPoint(itsToolBox, p1);
	auto gdiplusP2 = CtrlView::Relative2GdiplusPoint(itsToolBox, p2);
	auto gdiplusP3 = CtrlView::Relative2GdiplusPoint(itsToolBox, p3);
	Gdiplus::GraphicsPath aPath;
	aPath.AddLine(gdiplusP1, gdiplusP2);
	aPath.AddLine(gdiplusP2, gdiplusP3);
	aPath.CloseFigure();
	itsGdiPlusGraphics->FillPath(&aBrushBox, &aPath);

	float penThicknessInMM = 0.3f;
	auto [relativePenThickness, penThicknessInPixels] = ConvertMilliMetersToRelativeAndPixels(penThicknessInMM, false);
	Gdiplus::Pen penBox(CtrlView::NFmiColor2GdiplusColor(NFmiColor()), static_cast<Gdiplus::REAL>(penThicknessInPixels));
	itsGdiPlusGraphics->DrawPath(&penBox, &aPath);
}

bool NFmiTimeControlView::IsTimeFiltersDrawn(void)
{
	if(fShowEditTimeHandles)
		if(itsCtrlViewDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal)  // jos ns. edit-moodi p‰‰ll‰, piiret‰‰n aikarajoitin viivat
			return true;
	return false;
}

void NFmiTimeControlView::DrawNoDataAvailable(void)
{
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.EnableFrame();
	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	envi.SetFillColor(NFmiColor(1.f,1.f,1.f));
	NFmiRectangle rect(GetFrame()
						,0
						,&envi);
	itsToolBox->Convert(&rect);

	if(itsCtrlViewDocumentInterface->EditedDataNeedsToBeLoaded())
	{
		NFmiString str("Loading data, please wait. Elapsed time: ");
		str += NFmiMilliSecondTimer::EasyTimeDiffStr(itsCtrlViewDocumentInterface->EditedDataNeedsToBeLoadedTimer().CurrentTimeDiffInMSeconds(), true);
		NFmiPoint place = GetFrame().TopLeft();
		place.X(place.X() + GetFrame().Width()/100.);
		envi.SetFontSize(NFmiPoint(22,22));
		NFmiText text(place, str, false, 0, &envi);
		itsToolBox->Convert(&text);
	}
	else
	{
		NFmiString str(::GetDictionaryString("MapViewNoData"));
		NFmiPoint place = GetFrame().TopLeft();
		place.X(place.X() + GetFrame().Width()/100.);
		envi.SetFontSize(NFmiPoint(18,18));
		NFmiText text(place, str, false, 0, &envi);
		itsToolBox->Convert(&text);
	}
}

const NFmiTimeBag& NFmiTimeControlView::GetUsedTimeBag(void)
{
	return *(itsCtrlViewDocumentInterface->TimeControlViewTimes(itsMapViewDescTopIndex).ValidTimeBag());
}

void NFmiTimeControlView::SetUsedTimeBag(const NFmiTimeBag &newTimeBag)
{
	NFmiTimeDescriptor timeDesc(itsCtrlViewDocumentInterface->TimeControlViewTimes(itsMapViewDescTopIndex).OriginTime(), newTimeBag);
	itsCtrlViewDocumentInterface->TimeControlViewTimes(itsMapViewDescTopIndex, timeDesc);
}

const NFmiMetTime& NFmiTimeControlView::StartTime(void) const
{
	return itsCtrlViewDocumentInterface->TimeFilterStartTime();
}

const NFmiMetTime& NFmiTimeControlView::EndTime(void) const
{
	return itsCtrlViewDocumentInterface->TimeFilterEndTime();
}

void NFmiTimeControlView::StartTime(const NFmiMetTime &newValue)
{
    itsCtrlViewDocumentInterface->SetTimeFilterStartTime(newValue);
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(g_EditedDataTimeRangeChangedUpdateViewIdMask);
}

void NFmiTimeControlView::EndTime(const NFmiMetTime &newValue)
{
    itsCtrlViewDocumentInterface->SetTimeFilterEndTime(newValue);
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(g_EditedDataTimeRangeChangedUpdateViewIdMask);
}

// lasketaan pikseleissa annetusta fonttikoosta sellainen fontti koko, mik‰ j‰‰ annetun
// fontin minimi ja maksimi kokojen v‰liin
static double FixFontSize(double fontSize, double pixelSizeInMM, double minFontSizeInMM, double maxFontSizeInMM)
{
	double fontSizeInMM = fontSize * pixelSizeInMM;
	fontSizeInMM = FmiMin(fontSizeInMM, maxFontSizeInMM);
	fontSizeInMM = FmiMax(fontSizeInMM, minFontSizeInMM);
	return fontSizeInMM / pixelSizeInMM;
}

// funtio piirt‰‰ resoluutio stringin sille varattuun laatikkoon.
// jos tilaa on, tehd‰‰n kirjaimista korkeintaan 5 mm korkeita.
// Funktio laskee kuinka pitk‰ tekstist‰ tulisi ja jos tilaa on liian 
// v‰h‰n, fontin kokoa pienennet‰‰n vastaavasti.
void NFmiTimeControlView::DrawResolutionChangerBox(void)
{
	if(fShowResolutionChangerBox)
	{
		NFmiDrawingEnvironment envi;
		envi.EnableFrame();
		envi.SetFontType(kArial);
		//envi.BoldFont(true);
		envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));

		itsToolBox->SetTextAlignment(kCenter);

		NFmiString resolutionStr = GetResolutionText();
		unsigned long strLen = resolutionStr.GetLen();
		double fontSizeInMM = 5.0;
		double fontSizefactor = 1.0; // t‰t‰ k‰ytet‰‰n normaali kokoiselle resoluutio tekstille eli 2 merkki pitk‰ (esim. 1h)
		int fontSize = static_cast<int>(fontSizeInMM * fontSizefactor * GetGraphicalInfo().itsPixelsPerMM_y * 1.88);

		envi.SetFontSize(NFmiPoint(fontSize, fontSize));
		NFmiText dummyText(NFmiPoint(-11111, -11111), "", false, 0, &envi); // printataan tyhj‰ kerran lasketulla fontilla, ett‰ voidaan laskea tekstin vaatima tila
		itsToolBox->Convert(&dummyText);
		double measure = itsToolBox->MeasureText(resolutionStr) * 2.2;
		double fontSizeFixFactor = 1;
		if(measure > itsResolutionChangerBox.Width())
			fontSizeFixFactor = itsResolutionChangerBox.Width() / measure;
        fontSize = boost::math::iround(fontSize*fontSizeFixFactor);
		envi.SetFontSize(NFmiPoint(fontSize, fontSize)); // asetetaan korjattu fontti koko voimaan

		NFmiPoint strPlace(itsResolutionChangerBox.Center());
		NFmiText text(strPlace, resolutionStr, false, 0, &envi);
		itsToolBox->Convert(&text);
	}
}

NFmiString NFmiTimeControlView::GetResolutionText(void)
{
	int usedTimeResolutionInMinutes = GetUsedTimeResolutionInMinutes();
	if(usedTimeResolutionInMinutes >= 60)
	{
		NFmiString str(NFmiValueString::GetStringWithMaxDecimalsSmartWay(usedTimeResolutionInMinutes/60., 2)); // max 2 desimaalia
		str += "h";
		return str;
	}
	else
	{
		NFmiString str(NFmiValueString::GetStringWithMaxDecimalsSmartWay(usedTimeResolutionInMinutes, 0));
		str += "min";
		return str;
	}
}

static bool IsMainMapViewDesctopIndex(int mapViewDescTopIndex)
{
    return mapViewDescTopIndex == 0;
}

namespace
{
	std::vector<int> gTimeResolutionLimitsInMinutes{ 1,5,10,15,30,60,120,180,360,720,1440, 2880 };
	std::vector<int> gTimeResolutionExtendedLimitsInMinutes{ 1,5,10,15,30,60,120,180,240,300,360,420,480,540,600,660,720,780,840,900,960,1020,1080,1140,1200,1260,1320,1380,1440,1500,1560,1620,1680,1740,1800,1860,1920,1980,2040,2100,2160,2220,2280,2340,2400,2460,2520,2580,2640,2700,2760,2820,2880 };

	int seekNextLowerLmit(int currentResolution, const std::vector<int>& timeResolutionLimitsInMinutes)
	{
		if(currentResolution <= timeResolutionLimitsInMinutes.front())
			return timeResolutionLimitsInMinutes.front();
		else if(currentResolution > timeResolutionLimitsInMinutes.back())
			return timeResolutionLimitsInMinutes.back();

		// lower_bound etsii joko etsityn arvon tai ensimm‰isen sit‰ suuremman arvon paikan iteraattorin
		auto iter = std::lower_bound(timeResolutionLimitsInMinutes.begin(), timeResolutionLimitsInMinutes.end(), currentResolution);
		if(iter == timeResolutionLimitsInMinutes.end())
		{
			// T‰h‰n ei pit‰isi menn‰, palautetaan vaikka pienin arvo
			return timeResolutionLimitsInMinutes.front();
		}

		// Ei tarvitse tarkistaa ett‰ ollaanko jo alarajalla, koska se on jo otettu huomioon heti alussa.
		// Otetaan yksi pyk‰l‰ alas ja palautetaan se
		--iter;
		return *iter;
	}

	int seekNextUpperLmit(int currentResolution, const std::vector<int>& timeResolutionLimitsInMinutes)
	{
		if(currentResolution < timeResolutionLimitsInMinutes.front())
			return timeResolutionLimitsInMinutes.front();
		else if(currentResolution >= timeResolutionLimitsInMinutes.back())
			return timeResolutionLimitsInMinutes.back();

		// upper_bound etsii ensimm‰isen haettua arvoa suuremman arvon paikan iteraattorin
		auto iter = std::upper_bound(timeResolutionLimitsInMinutes.begin(), timeResolutionLimitsInMinutes.end(), currentResolution);
		if(iter == timeResolutionLimitsInMinutes.end())
		{
			// T‰h‰n ei pit‰isi menn‰, palautetaan vaikka suurin arvo
			return timeResolutionLimitsInMinutes.back();
		}

		return *iter;
	}
}

void NFmiTimeControlView::ChangeResolution(bool fLeftClicked, bool ctrlKeyDown)
{
	int usedTimeResolutionInMinutes = GetUsedTimeResolutionInMinutes();
	const auto& usedTimeResolutionLimits = ctrlKeyDown ? gTimeResolutionExtendedLimitsInMinutes : gTimeResolutionLimitsInMinutes;
	if(fLeftClicked) // pienennet‰‰n resoluutiota
	{
		usedTimeResolutionInMinutes = ::seekNextLowerLmit(usedTimeResolutionInMinutes, usedTimeResolutionLimits);
	}
	else
	{
		usedTimeResolutionInMinutes = ::seekNextUpperLmit(usedTimeResolutionInMinutes, usedTimeResolutionLimits);
	}
	itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex, usedTimeResolutionInMinutes/60.f);
	itsCtrlViewDocumentInterface->CheckAnimationLockedModeTimeBags(itsMapViewDescTopIndex, false);
	itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
    if(::IsMainMapViewDesctopIndex(itsMapViewDescTopIndex))
    {
        // P‰‰karttan‰ytˆn aika-stepin vaihto vaikuttaa myˆs warning ja sea-icing ikkunoiden tiloihin
        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::WarningCenterDlg);
    }
}

// t‰ll‰ piirret‰‰n tavara, joka tulee myˆs bitmapin p‰‰lle
void NFmiTimeControlView::DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4)
{
	itsToolBox = theGTB;
	if(fHasData)
		DrawTimeFilterTimes();
}

static const double sTimeFilterMarkerHeightInMM = 3.5f;
static const double sTimeFilterDragAreaWidthInMM = 3.5f;

NFmiRect NFmiTimeControlView::CalcTimeFilterBoxRect(void)
{
	int pixelSizeY = static_cast<int>(sTimeFilterMarkerHeightInMM * GetGraphicalInfo().itsPixelsPerMM_y);
	double relYSize = itsToolBox->SY(pixelSizeY);

	double startPos = Time2Value(StartTime());
	double endPos = Time2Value(EndTime());

	if(::fabs(startPos - endPos) < 0.001)
	{ // jos alku ja loppu aika ovat samoja tai tarpeeksi l‰hell‰ toisiaan suht. maailmassa, venyt‰ sit‰ x- ja y-suunnassa
		double relXSize = itsToolBox->SY(2);
		return NFmiRect(startPos - relXSize, GetFrame().Top() + itsToolBox->SY(2), endPos + relXSize, GetFrame().Top() + relYSize * 3);
	}
	else
		return NFmiRect(startPos, GetFrame().Top() + itsToolBox->SY(2), endPos, GetFrame().Top() + relYSize);
}

static const double sResolutionChangerBoxWidthInMM = 12.;
NFmiRect NFmiTimeControlView::CalcResolutionChangerBoxRect(void)
{
	int pixelSizeX = static_cast<int>(sResolutionChangerBoxWidthInMM * GetGraphicalInfo().itsPixelsPerMM_x);
	double relXSize = itsToolBox->SX(pixelSizeX);

	NFmiRect rect(GetFrame());
	rect.Left(rect.Right() - relXSize);
	return rect;
}

void NFmiTimeControlView::DrawTimeFilterTimes()
{
	if(IsTimeFiltersDrawn())
	{
		// HUOM!!!!!
		// Jouduin k‰ytt‰m‰‰n t‰ss‰ kohtaa lokaali Gdiplus::Graphics -oliota, koska luokan oman instanssin 
		// itsGdiPlusGraphics-instanssin k‰yttˆ kaatoi omituisesti ohjelman. T‰m‰ on varoittava esimerkki
		// siit‰, ett‰ ehk‰ kannattaa k‰ytt‰‰ lokaaleja Graphics-olioita. En tied‰ miten raskas operaatio
		// on luoda aina joka k‰yttˆˆn oma instanssi piirto hommia varten.
		// Luulen ett‰ ongelma on siin‰ ett‰ SmartMet luo jatkuvasti n‰yttˆluokkia uudelleen ja uudelleen
		// t‰st‰ ehk‰ seuraa se ett‰ joku piirto metodi saattaa aiheuttaa luokan uudelleen luonnin kesken 
		// kaiken, jolloin myˆs GDI+ otus pit‰isi luoda uudestaan. Korjaaminen voisi ehk‰ tapahtua siten 
		// ett‰ luokkien luonti ja muu toiminta pit‰‰ laittaa uusiksi (= iso ja vaivaloinen tyˆ).
		Gdiplus::Graphics *gdiPlusGraphics = Gdiplus::Graphics::FromHDC(itsToolBox->GetDC()->GetSafeHdc());
		try
		{
			if(gdiPlusGraphics == 0)
				throw std::runtime_error("Error in NFmiTimeControlView::DrawTimeFilterTimes -method, unable to initialize Gdiplus::Graphics, application error.");

			if(itsToolBox->GetDC()->IsPrinting())
				gdiPlusGraphics->SetPageUnit(Gdiplus::UnitPixel); // t‰h‰n asti on pelattu printatessa aina pikseli maailmassa, joten gdiplus:in pit‰‰ laittaa siihen

            DrawFullTimeRangeButton(gdiPlusGraphics);

			// vaalean punainen time filtter jana pit‰‰ clipata aikaskaalaasteikkoon
			NFmiRect timeScaleRect = ((NFmiAdjustedTimeScaleView*)itsTimeView)->CalcTotalTimeScaleArea();
			gdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, timeScaleRect));

			// Piirret‰‰n animaatioaikav‰li vaalean sinisell‰ l‰pin‰kyv‰ll‰ laatikolla
			NFmiRect timeFilterRect(CalcTimeFilterBoxRect());
			NFmiColor timeFilterColor(1.f, 0.498f, 0.f, 0.608f);
			Gdiplus::SolidBrush aBrushBox(CtrlView::NFmiColor2GdiplusColor(timeFilterColor));
			Gdiplus::GraphicsPath aPath;
            CtrlView::MakePathFromRect(aPath, itsToolBox, timeFilterRect);
			gdiPlusGraphics->FillPath(&aBrushBox, &aPath);

			int pixelSizeX = static_cast<int>(sTimeFilterDragAreaWidthInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x);
			double relXSize = itsToolBox->SX(pixelSizeX);
			itsLeftTimeHandle = timeFilterRect;
			itsLeftTimeHandle.Right(itsLeftTimeHandle.Left() + relXSize);
			itsRightTimeHandle = timeFilterRect;
			itsRightTimeHandle.Left(itsRightTimeHandle.Right() - relXSize);

		}
		catch(...)
		{
		}
		if(gdiPlusGraphics)
		{
			gdiPlusGraphics->ResetClip();
			delete gdiPlusGraphics;
			gdiPlusGraphics = 0;
		}
	}
}

void NFmiTimeControlView::ClearAllMouseCaptureFlags(void)
{
	fMouseCaptured = false;
	fMouseCapturedMoveTimeLine = false;
	fMouseMovedWhenCaptured = false;
	fMouseCapturedMoveSelectedTime = false;
	fMouseCapturedAnimationBox = false;
	itsAnimationDragDirection = kCenter;
	itsTimeScaleDragDirection = kCenter;

	fAnimationCloseButtonPressed = false;
	fAnimationPlayButtonPressed = false;
	fAnimationDelayButtonPressed = false;
	fAnimationRunModeButtonPressed = false;
	fAnimationLastFrameDelayButtonPressed = false;
	fAnimationLockModeButtonPressed = false;
	fAnimationVerticalControlButtonPressed = false;
}

// theSizeFactorX -parametri on pika viritys, jotta saisin FullTimeRange-nappulan toimimaan, ja se on 2x leve‰mpi kuin normaalit nappulat (32 x 16 pikseli‰)
// theSizeFactorY -parametri on pika viritys, jotta saisin tehty‰ nappuloista tuplakokoisia 
NFmiPoint NFmiTimeControlView::CalcAnimationButtonRelativeSize(double theSizeFactorX, double theSizeFactorY)
{
    double relativeWidth = itsToolBox->SX(boost::math::iround(theSizeFactorX * itsButtonSizeInMM_x * GetGraphicalInfo().itsPixelsPerMM_x));
    double relativeHeight = itsToolBox->SY(boost::math::iround(theSizeFactorY * itsButtonSizeInMM_y * GetGraphicalInfo().itsPixelsPerMM_y));
	if(itsCtrlViewDocumentInterface->Printing() == false)
	{
        long bitmapSizeX = boost::math::iround(16 * theSizeFactorX);
		long bitmapSizeY = boost::math::iround(16 * theSizeFactorY);
		if(statAnimationButtonImages.itsPlayButtonImage)
		{
            bitmapSizeX = boost::math::iround(statAnimationButtonImages.itsPlayButtonImage->GetWidth() * theSizeFactorX);
			bitmapSizeY = boost::math::iround(statAnimationButtonImages.itsPlayButtonImage->GetHeight() * theSizeFactorY);
		}
		relativeWidth = itsToolBox->SX(bitmapSizeX);
		relativeHeight = itsToolBox->SY(bitmapSizeY);
	}
	return NFmiPoint(relativeWidth, relativeHeight);
}

NFmiPoint NFmiTimeControlView::CalcAnimationButtonRelativeEdgeOffset(const NFmiPoint &theButtonRelaviteSize)
{
	double offsetFromEdgeX = theButtonRelaviteSize.X() * itsButtonOffSetFromEdgeFactor;
	double offsetFromEdgeY = theButtonRelaviteSize.Y() * itsButtonOffSetFromEdgeFactor;
	return NFmiPoint(offsetFromEdgeX, offsetFromEdgeY);
}

NFmiRect NFmiTimeControlView::CalcFullTimeRangeButtonRect(void)
{
    if(IsAnimationViewVisible())
        return NFmiRect();
        
    NFmiPoint buttonRelativeSize = CalcAnimationButtonRelativeSize(2.);

    // Sijoitetaan FullTimeRange-nappi vasempaan alakulmaan kiinni reunoihin
    double leftSide = GetFrame().Left();
    double rightside = leftSide + buttonRelativeSize.X();
    double topSide = GetFrame().Bottom() - buttonRelativeSize.Y();
    double bottomSide = GetFrame().Bottom();
    NFmiRect closeRect(leftSide, topSide, rightside, bottomSide);
    return closeRect;
}

NFmiRect NFmiTimeControlView::CalcAnimationCloseButtonRect()
{
	return CalcAnimationButtonTopLeftRect(CalcAnimationBoxRect());
}

NFmiRect NFmiTimeControlView::CalcVirtualTimeCloseButtonRect()
{
	return CalcAnimationButtonTopLeftRect(itsVirtualTimeBoxRect);
}

NFmiRect NFmiTimeControlView::CalcAnimationButtonTopLeftRect(const NFmiRect &baseRect)
{
	NFmiPoint buttonRelativeSize = CalcAnimationButtonRelativeSize(2, 2);
	NFmiPoint buttonRelativeEdgeOffset = CalcAnimationButtonRelativeEdgeOffset(buttonRelativeSize);
	// Sijoitetaan close-nappi oikeaa yl‰ kulmaan hieman irti reunoista
	double leftSide = baseRect.Right() - buttonRelativeSize.X() - buttonRelativeEdgeOffset.X();
	double rightside = baseRect.Right() - buttonRelativeEdgeOffset.X();
	double topSide = baseRect.Top() + buttonRelativeEdgeOffset.Y();
	double bottomSide = baseRect.Top() + buttonRelativeSize.Y() + buttonRelativeEdgeOffset.Y();
	NFmiRect closeRect(leftSide, topSide, rightside, bottomSide);
	return closeRect;
}

NFmiRect NFmiTimeControlView::CalcAnimationVerticalControlButtonRect(void)
{
	// sijoitetaan t‰m‰ nappi close -napin alle
	NFmiRect rect = CalcAnimationCloseButtonRect();
	NFmiPoint buttonRelativeSize = CalcAnimationButtonRelativeSize(2, 2);
	NFmiPoint buttonRelativeEdgeOffset = CalcAnimationButtonRelativeEdgeOffset(buttonRelativeSize);
	NFmiPoint center = rect.Center();
	center.Y(center.Y() + rect.Height() + buttonRelativeEdgeOffset.Y());
	rect.Center(center);
	return rect;
}

// Laskee vasempaan alariviin nappuloiden paikkoja indeksin perusteella.
// theIndex alkaa 1:st‰ ja indeksin kasvaessa nappuloita sijoitetaan aina enemm‰n oikealle.
NFmiRect NFmiTimeControlView::CalcAnimationButtonRect(int theIndex)
{
	NFmiRect animRect = CalcAnimationBoxRect();
	NFmiPoint buttonRelativeSize = CalcAnimationButtonRelativeSize(2, 2);
	NFmiPoint buttonRelativeEdgeOffset = CalcAnimationButtonRelativeEdgeOffset(buttonRelativeSize);

	// Sijoitetaan n‰m‰ nappulat vasemapaan ala kulmaan hieman irti reunoista.
	double leftSide = animRect.Left() + ((theIndex-1)*buttonRelativeSize.X()) + (theIndex*buttonRelativeEdgeOffset.X());
	double rightside = leftSide + buttonRelativeSize.X();
	double topSide = animRect.Bottom() - buttonRelativeSize.Y() - buttonRelativeEdgeOffset.Y();
	double bottomSide = animRect.Bottom() - buttonRelativeEdgeOffset.Y();
	NFmiRect buttonRect(leftSide, topSide, rightside, bottomSide);
	return buttonRect;
}

NFmiRect NFmiTimeControlView::CalcAnimationPlayButtonRect(void)
{
	NFmiRect closeRect = CalcAnimationButtonRect(1);
	return closeRect;
}

NFmiRect NFmiTimeControlView::CalcAnimationDelayButtonRect(void)
{
	NFmiRect delayRect = CalcAnimationButtonRect(2);
	return delayRect;
}

NFmiRect NFmiTimeControlView::CalcAnimationRunModeButtonRect(void)
{
	NFmiRect closeRect = CalcAnimationButtonRect(3);
	return closeRect;
}

NFmiRect NFmiTimeControlView::CalcAnimationLockModeButtonRect(void)
{
	NFmiRect closeRect = CalcAnimationButtonRect(4);
	return closeRect;
}

NFmiRect NFmiTimeControlView::CalcLastFrameDelayFactorButtonRect(void)
{
	NFmiRect rect = CalcAnimationButtonRect(5);
	return rect;
}


bool NFmiTimeControlView::IsInsideAnyAnimationButton(const NFmiPoint &thePlace)
{
	if(CalcAnimationBoxRect().IsInside(thePlace))
	{ // pit‰‰ olla animaatio boxin sis‰ll‰, ennen kuin tarkastellaan erillisi‰ nappeja!!
		if(CalcAnimationCloseButtonRect().IsInside(thePlace))
			return true;
		if(CalcAnimationPlayButtonRect().IsInside(thePlace))
			return true;
		if(CalcAnimationDelayButtonRect().IsInside(thePlace))
			return true;
		if(CalcAnimationRunModeButtonRect().IsInside(thePlace))
			return true;
		if(CalcAnimationLockModeButtonRect().IsInside(thePlace))
			return true;
		if(CalcLastFrameDelayFactorButtonRect().IsInside(thePlace))
			return true;
		if(CalcAnimationVerticalControlButtonRect().IsInside(thePlace))
			return true;
	}
	return false;
}

// Tarkastellaan onko joku animaatio nappi painettu alas.
// Jos on, tarkastetaan onko hiiri viel‰ napin p‰‰ll‰ kun se on p‰‰stetty irti, jolloin voidaan tehd‰ napin toiminta.
bool NFmiTimeControlView::AnimationButtonReleased(const NFmiPoint & thePlace,unsigned long theKey)
{
	if(IsAnimationButtonPressed() || IsInsideAnyAnimationButton(thePlace))
	{
		NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);

		if(fAnimationCloseButtonPressed && CalcAnimationCloseButtonRect().IsInside(thePlace))
		{
			animData.ShowTimesOnTimeControl(false);
			animData.AnimationOn(false); // suljetaan myˆs animaatio samalla (ett‰ jos animointi ikkuna avataan myˆhemmin, ei animaatio mene heti p‰‰lle)
		}
		else if(fAnimationPlayButtonPressed || CalcAnimationPlayButtonRect().IsInside(thePlace))
			animData.AnimationOn(!animData.AnimationOn());
		else if(fAnimationDelayButtonPressed || CalcAnimationDelayButtonRect().IsInside(thePlace))
		{
			int currentDelayInMS = animData.FrameDelayInMS();
			currentDelayInMS -= 100;
			if(currentDelayInMS < 0)
				currentDelayInMS = 0;
            currentDelayInMS = boost::math::iround(currentDelayInMS / 100.) * 100; // fiksataan viel‰ l‰himp‰‰ 100 jaolliseen lukuun
			animData.FrameDelayInMS(currentDelayInMS);
		}
		else if(fAnimationRunModeButtonPressed || CalcAnimationRunModeButtonRect().IsInside(thePlace))
			animData.ToggleRunMode();
		else if(fAnimationLastFrameDelayButtonPressed || CalcLastFrameDelayFactorButtonRect().IsInside(thePlace))
		{
			double currentLastFrameDelay = animData.LastFrameDelayFactor();
			currentLastFrameDelay -= 0.5;
			if(currentLastFrameDelay < 1)
				currentLastFrameDelay = 1;
			currentLastFrameDelay = ::round(currentLastFrameDelay/0.5)*0.5; // fiksataan viel‰ l‰himp‰‰ 0.5 jaolliseen lukuun
			animData.LastFrameDelayFactor(currentLastFrameDelay);
		}
		else if(fAnimationLockModeButtonPressed || CalcAnimationLockModeButtonRect().IsInside(thePlace))
		{
			animData.ToggleLockMode();
			if(animData.LockMode() != NFmiAnimationData::kNoLock)
				itsCtrlViewDocumentInterface->CheckAnimationLockedModeTimeBags(itsMapViewDescTopIndex, false); // t‰m‰ p‰ivitt‰‰ timebagit kohdalleen ja likaa mahd. n‰yttˆ ruutuja cachesta
		}
		else if(fAnimationVerticalControlButtonPressed || CalcAnimationVerticalControlButtonRect().IsInside(thePlace))
		{
			animData.ShowVerticalControl(!animData.ShowVerticalControl());
		}

        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
        return true;
	}
	else
		return false;
}

bool NFmiTimeControlView::HandlePossibleVirtualTimeSet(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(!itsCtrlViewDocumentInterface->VirtualTimeUsed() || !itsVirtualTimeSliderRect.IsInside(thePlace))
	{
		return false;
	}

	auto pointedTime = itsTimeView->GetTime(thePlace);
	itsCtrlViewDocumentInterface->VirtualTime(pointedTime);
	return true;
}

bool NFmiTimeControlView::HandlePossibleVirtualTimeBoxCloseButtonClick(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(!itsCtrlViewDocumentInterface->VirtualTimeUsed() || !CalcVirtualTimeCloseButtonRect().IsInside(thePlace))
	{
		return false;
	}

	itsCtrlViewDocumentInterface->ToggleVirtualTimeMode("Virtual-Time mode closed by pressing Close button inside Virtual-Time-box in time-control-view");
	return true;
}

// Tiettyj‰ hiiren klikkauksia halutaan tehd‰ monessa kohtaa (4) NFmiTimeControlView::LeftButtonUp 
// metodissa, siksi ne on niputettu t‰h‰n yhteen kutsuun.
bool NFmiTimeControlView::DoPrimaryLeftButtonUpChecks(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(HandlePossibleVirtualTimeSet(thePlace, theKey))
	{
		return true;
	}

	if(HandlePossibleVirtualTimeBoxCloseButtonClick(thePlace, theKey))
	{
		return true;
	}

	if(itsCtrlViewDocumentInterface->SetDataToPreviousTime(itsMapViewDescTopIndex))
	{
		itsTimeBag->SetCurrent(itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex));
		return true;
	}
	
	return false;
}

bool NFmiTimeControlView::LeftButtonUp(const NFmiPoint & thePlace,unsigned long theKey)
{
	bool status = false;
	bool ctrlKeyDown = (theKey & kCtrlKey);
	// Huomasin ett‰ Winkkarin hiiren k‰sittely rutiinissa on joskus 'vuotoja'.
	// Eli LeftButtonDown-metodissa on tarkoitus merkit‰, onko hiiri painettu alas aikakontrolli-ikkunassa.
	// Mutta jos klikkailee hiiren vasenta nappia nopeasti per‰kk‰in, ei LeftButtonDown-metodia aina kutsutakaan.
	// T‰st‰ seurasi, ett‰ jos ei ole merkint‰‰ onko hiiri 'kaapattu', mutta hiiri on aikakontrolli-ikkunan p‰‰ll‰
	// tehd‰‰n kuitenkin hyppy edelliseen aikaan. Muuten taaksep‰in ajassa klikkailu ei toimi kuten
	// ennen n‰it‰ Mirwasta otettuja aikakontrolli hiirenk‰sittely uudistuksia.
	auto isMouseCaptured = IsMouseCaptured();
	auto isInsideFullTimeRangeButtonRect = CalcFullTimeRangeButtonRect().IsInside(thePlace);
    if(!isMouseCaptured && !isInsideFullTimeRangeButtonRect)
	{
        if(IsIn(thePlace))
        {	
			// eli jos hiiren napista on p‰‰stetty irti (vaikka sit‰ ei oltu otettu kiinni) ja kursori on aikaikkunan p‰‰ll‰, siirret‰‰n aikaa taaksep‰in
            if((ctrlKeyDown) && (theKey & kShiftKey))
            {
                if(itsTimeBag->FindNearestTime(itsTimeView->GetTime(thePlace), kCenter, itsTimeBag->Resolution()))
                {
                    StartTime(itsTimeBag->CurrentTime());
                    return true;
                }
            }
            else if(AnimationButtonReleased(thePlace, theKey))
                status = true;
            else if(fShowResolutionChangerBox && itsResolutionChangerBox.IsInside(thePlace))
            {
                ChangeResolution(true, ctrlKeyDown);
                status = true;
            }
			else if(DoPrimaryLeftButtonUpChecks(thePlace, theKey))
			{
				status = true;
			}
		}
        else
            status = false; // jos hiirt‰ ei oltu klikattu pohjaan aikaikkunassa, eik‰ olla aikakontrolliikkunassa, ei tehd‰ mit‰‰n
	}
	else
	{
        itsCtrlViewDocumentInterface->FilterDialogUpdateStatus(1); // 1 = filterdialogin aikakontrolli-ikkuna pit‰‰ p‰ivitt‰‰

		if(fMouseMovedWhenCaptured)
		{
			if(fMouseCaptured)
			{ // lopetetaan vain editointi rajoitin aikajanan raahaus
				fMouseCaptured = false;
				status = true;
			}
			else if(fMouseCapturedMoveTimeLine)
			{ // lopetetaan aikakontrollin raahaus ja s‰‰det‰‰n sen alku- ja loppup‰‰t sopiviksi

				// jos raahaus on ollut mit‰tˆn, siirret‰‰n valittua aikaa, eik‰ tehd‰ raahausta
				double xDiff = thePlace.X() - itsLeftButtonDownMousePosition.X();
				if(::fabs(xDiff) < 0.001)
				{
					if(DoPrimaryLeftButtonUpChecks(thePlace, theKey))
					{
						status = true;
					}
				}
				else
				{
					int timeStepInMinutes = GetUsedTimeResolutionInMinutes();
					const NFmiTimeBag & oldTimeBag = GetUsedTimeBag();
					NFmiTimeBag newTimeBag = CtrlViewUtils::GetAdjustedTimeBag(oldTimeBag.FirstTime(), oldTimeBag.LastTime(), timeStepInMinutes);
					SetUsedTimeBag(newTimeBag);
				}

				fMouseCapturedMoveTimeLine = false;
				status = true;
			}
			else if(fMouseCapturedMoveSelectedTime)
				status = (SetTime(thePlace, true) == true);
			else if(fMouseCapturedAnimationBox)
			{
				// jos raahaus on ollut mit‰tˆn, siirret‰‰n valittua aikaa, eik‰ tehd‰ raahausta
				double xDiff = thePlace.X() - itsLeftButtonDownMousePosition.X();
				if(::fabs(xDiff) < 0.001)
				{
					DoPrimaryLeftButtonUpChecks(thePlace, theKey);
				}
				else
				{
					NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
					animData.ShowTimebag(animData.Times()); // asetetaan lopuksi oikeat animaatio ajat myˆs show-times ajoiksi (t‰m‰ siis 'loksauttaa' animaatio ajat timestepin mukaisesti)
				}
				status = true;
			}
        }
		if(status == false)
		{
			// jos hiirt‰ ei oltu raahattu, tehd‰‰n vain jotain seuraavista
            if(isInsideFullTimeRangeButtonRect)
            {
                itsCtrlViewDocumentInterface->ResetTimeFilterTimes();
                ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(g_EditedDataTimeRangeChangedUpdateViewIdMask);
                status = true;
            }
			else if(ctrlKeyDown)
			{
				status = (SetTime(thePlace) == true);
			}
			else if(theKey & kShiftKey)
			{ // t‰m‰kin pit‰‰ hanskata, ett‰ hiiri ei liikkunut kun on klikattu hiirell‰ shift-nappi pohjassa
				NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
				animData.ShowTimebag(animData.Times()); // asetetaan lopuksi oikeat animaatio ajat myˆs show-times ajoiksi (t‰m‰ siis 'loksauttaa' animaatio ajat timestepin mukaisesti)
				status = true;
			}
			else
			{

				if(fMouseCapturedAnimationBox && fMouseMovedWhenCaptured)
				{ // jos klikkaus alas tapahtui animaatio boxissa, ja sit‰ on raahattu, eik‰ oltu painettu mit‰‰n nappulaa
					status = true;
				}
				else if(DoPrimaryLeftButtonUpChecks(thePlace, theKey))
				{
					status = true;
				}
			}
		}
	}

    if(status)
    {
        // Jos status on laitettu true:ksi, varmistetaan ett‰ tehd‰‰n ainakin kyseisen karttan‰ytˆn p‰ivitys. 
        // Optimointia, ettei kaikkia n‰yttˆj‰ p‰ivitet‰. Jos halutaan ett‰ tehd‰‰n laajempia n‰yttˆjen p‰ivityksi‰, pit‰‰ eri kohdissa laittaa enemm‰n n‰yttˆjen p‰ivityslippuja p‰‰lle
        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
    }

    ClearAllMouseCaptureFlags();
	itsCtrlViewDocumentInterface->MouseCaptured(false);
	return status;
}

NFmiRect NFmiTimeControlView::CalcSelectedTimeArea(const NFmiMetTime &theTime) const
{
	if(itsTimeView)
		return ((NFmiAdjustedTimeScaleView*) itsTimeView)->CalcSelectedTimeArea(theTime);
	else
		return NFmiRect();
}

bool NFmiTimeControlView::AnimationLeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	NFmiRect animationTimesBox(CalcAnimationBoxRect());
	if((theKey & kShiftKey) && animationTimesBox.IsInside(thePlace) == false)
	{ // shift-nappi pohjassa synnytet‰‰n uusi animaatio-aika-boksi, jos ollaan nykyisen animaatio boksin ulkopuolella (tai sit‰ ei esim. ole n‰kyviss‰)
		fMouseCapturedAnimationBox = true;
		itsAnimationDragDirection = kBase;
		NFmiMetTime dragPointTime = GetTimeFromPosition(thePlace, 1);
		NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
		animData.MouseDragStartTime(dragPointTime);
		animData.ShowTimesOnTimeControl(true);
		animData.ShowTimebag(NFmiTimeBag(dragPointTime, dragPointTime, 1));
		return true;
	}
	else if(animationTimesBox.IsInside(thePlace))
	{ // oltiin animaatio boksin sis‰ll‰, tarrataan siihen kiinni tai painetaan jotain paino-napeista
		if(CalcAnimationCloseButtonRect().IsInside(thePlace))
		{
			fAnimationCloseButtonPressed = true;
			return true;
		}
		else if(CalcAnimationPlayButtonRect().IsInside(thePlace))
		{
			fAnimationPlayButtonPressed = true;
			return true;
		}
		else if(CalcAnimationDelayButtonRect().IsInside(thePlace))
		{
			fAnimationDelayButtonPressed = true;
			return true;
		}
		else if(CalcAnimationRunModeButtonRect().IsInside(thePlace))
		{
			fAnimationRunModeButtonPressed = true;
			return true;
		}
		else if(CalcAnimationLockModeButtonRect().IsInside(thePlace))
		{
			fAnimationLockModeButtonPressed = true;
			return true;
		}
		else if(CalcLastFrameDelayFactorButtonRect().IsInside(thePlace))
		{
			fAnimationLastFrameDelayButtonPressed = true;
			return true;
		}
		else if(CalcAnimationVerticalControlButtonRect().IsInside(thePlace))
		{
			fAnimationVerticalControlButtonPressed = true;
			return true;
		}
		else
		{
			fMouseCapturedAnimationBox = true;
			itsAnimationDragDirection = kCenter;
			double hotSpotWidthInMM = 4.; // animaatio-boxin p‰‰dyt ovat hot-spotteja, joihin tartuttaessa venytet‰‰n vasenta tai oikeaa reunaa
            double hotSpotWidth = itsToolBox->SX(boost::math::iround(hotSpotWidthInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x));
			if(animationTimesBox.Left() + hotSpotWidth > thePlace.X())
				itsAnimationDragDirection = kLeft;
			else if(animationTimesBox.Right() - hotSpotWidth < thePlace.X())
				itsAnimationDragDirection = kRight;
			return true;
		}
	}
	return false;
}

FmiDirection NFmiTimeControlView::TimeScaleOperationPosition(const NFmiPoint & thePlace, double theHotSpotWidthInMM)
{
    double hotSpotWidth = itsToolBox->SX(boost::math::iround(theHotSpotWidthInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x));

	FmiDirection dragDirection = kCenter; // raahataan koko asteikkoa
	NFmiRect timeScaleArea(itsTimeView->CalcScaleArea()); // t‰m‰ on siis itse asteikon alue, ei koko time kontrollin alue
	if(timeScaleArea.Left() + hotSpotWidth > thePlace.X())
		dragDirection = kLeft; // raahataan vain vasenta p‰‰ty‰
	else if(timeScaleArea.Right() - hotSpotWidth < thePlace.X())
		dragDirection = kRight; // raahataan vain oikean puoleista p‰‰ty‰
	return dragDirection;
}

bool NFmiTimeControlView::TimeScaleLeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	// eli jos ollaan aika-kontrollissa, mutta ei osuttu alku/loppu aika s‰‰tˆ kahvoihin tai resoluutio boxiin
	// tai animaatio boxiin, siirret‰‰n tarvittaessa koko aikajanaa tai sen reunaa
	fMouseCapturedMoveTimeLine = true;

	double hotSpotWidthInMM = 10.; // animaatio-boxin p‰‰dyt ovat hot-spotteja, joihin tartuttaessa venytet‰‰n vasenta tai oikeaa reunaa
	itsTimeScaleDragDirection = TimeScaleOperationPosition(thePlace, hotSpotWidthInMM);

	return true;
}

bool NFmiTimeControlView::LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey)
{
	itsLeftButtonDownMousePosition = thePlace;

	ClearAllMouseCaptureFlags();
	if(IsIn(thePlace))
	{
		if(fShowResolutionChangerBox && itsResolutionChangerBox.IsInside(thePlace))
			return false;
		else
		{
			itsLastMousePosition = thePlace;
            if(CalcFullTimeRangeButtonRect().IsInside(thePlace))
                return true; // Jos painettiin fulltimeRange -napin kohdalla, pit‰‰ loput tarkastelut ohittaa
			
			if(CalcSelectedTimeArea(itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex)).IsInside(thePlace))
			{ // aletaan raahaamaan p‰‰ karttan‰ytˆn ensimm‰isen kartan aika-laatikkoa (se vihre‰ laatikko aikakontrolli ikkunassa)
				fMouseCapturedMoveSelectedTime = true;
                itsCtrlViewDocumentInterface->MouseCaptured(true);
				return true;
			}
			
			if(IsTimeFiltersDrawn())
			{
				if(itsLeftTimeHandle.IsInside(thePlace) || itsRightTimeHandle.IsInside(thePlace))
				{
					fMouseCaptured = true;
                    itsCtrlViewDocumentInterface->MouseCaptured(true);
					return true;
				}
				if((theKey & kCtrlKey) && (theKey & kShiftKey))
				{ // kontrol + shift vasen s‰‰t‰‰ aikafiltterin alkuaikaa
                    itsCtrlViewDocumentInterface->MouseCaptured(true);
					return true;
				}
			}

			if(AnimationLeftButtonDown(thePlace, theKey))
			{
                itsCtrlViewDocumentInterface->MouseCaptured(true);
				return true;
			}
			else if(TimeScaleLeftButtonDown(thePlace, theKey))
			{
                itsCtrlViewDocumentInterface->MouseCaptured(true);
				return true;
			}
		}
	}
	return false;
}

bool NFmiTimeControlView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	bool ctrlKeyDown = (theKey & kCtrlKey);
	itsCtrlViewDocumentInterface->FilterDialogUpdateStatus(1); // 1 = filterdialogin aikakontrolli-ikkuna pit‰‰ p‰ivitt‰‰
	if(itsResolutionChangerBox.IsInside(thePlace))
	{
		ChangeResolution(false, ctrlKeyDown);
		return true;
	}
	else if((ctrlKeyDown) && (theKey & kShiftKey))
	{
		if(itsTimeBag->FindNearestTime(itsTimeView->GetTime(thePlace), kCenter, itsTimeBag->Resolution()))
		{
			EndTime(itsTimeBag->CurrentTime());
            return true;
		}
	}
	else if(ctrlKeyDown)
	{
		return SetTime(thePlace) == true;
	}
	else
	{
		if(CalcAnimationBoxRect().IsInside(thePlace) && CalcAnimationDelayButtonRect().IsInside(thePlace))
		{
			NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
			int currentDelayInMS = animData.FrameDelayInMS();
			currentDelayInMS += 100;
			if(currentDelayInMS > 2000)
				currentDelayInMS = 2000;
            currentDelayInMS = boost::math::iround(currentDelayInMS / 100.) * 100; // fiksataan viel‰ l‰himp‰‰ 10 jaolliseen lukuun
			animData.FrameDelayInMS(currentDelayInMS);
            ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
            return true;
		}
		else if(CalcAnimationBoxRect().IsInside(thePlace) && CalcLastFrameDelayFactorButtonRect().IsInside(thePlace))
		{
			NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
			double currentLastFrameDelay = animData.LastFrameDelayFactor();
			currentLastFrameDelay += 0.5;
			if(currentLastFrameDelay > 5)
				currentLastFrameDelay = 5;
			currentLastFrameDelay = ::round(currentLastFrameDelay/0.5)*0.5; // fiksataan viel‰ l‰himp‰‰ 0.5 jaolliseen lukuun
			animData.LastFrameDelayFactor(currentLastFrameDelay);
            ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
            return true;
		}
		else if(itsCtrlViewDocumentInterface->SetDataToNextTime(itsMapViewDescTopIndex))
		{
			itsTimeBag->SetCurrent(itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex));
			return true;
		}
	}
	return false;
}

bool NFmiTimeControlView::MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	return SetTime(thePlace);
}

bool NFmiTimeControlView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	bool ctrlKeyDown = (theKey & kCtrlKey);
	if(IsIn(thePlace))
	{ 
		if(fShowResolutionChangerBox && itsResolutionChangerBox.IsInside(thePlace))
		{
			ChangeResolution((theDelta > 0) ? false : true, ctrlKeyDown);
		}
		else if(itsTimeView->GetFrame().IsInside(thePlace) && (ctrlKeyDown))
		{ // kontrolli pohjassa hiiren rullalla venytet‰‰n tai kutistet‰‰n aikajanaa, siten ett‰
			// aika ei liiku kursorin kohdalla. Suhteellinen ajan p‰iden liikutus nopeus on aikaresoluutio.
			// Lis‰ksi jos ollaan ihan vasemmassa reunassa, liikutetaan vain vasenta reunaa ja jos
			// ollaan aivan oikeassa reunassa, liikutetaan vain oikeaa reunaa.
			// 1. Laske hiiren kursorin x-paikan suhde aikajanan alku ja loppu pisteisiin
			FmiDirection mousePlaceInScale = TimeScaleOperationPosition(thePlace, 10);
			double xFac = (thePlace.X() - itsTimeView->GetFrame().Left())/itsTimeView->GetFrame().Width(); // laske aikajanan alkupisteen ja hiiren kursorin suhde aikajanan leveyteen
			int timeStepInMinutes = GetUsedTimeResolutionInMinutes();
			const NFmiTimeBag tmpTimeBag = GetUsedTimeBag();
            int usedTimeChangeInMinutes = boost::math::iround(tmpTimeBag.LastTime().DifferenceInMinutes(tmpTimeBag.FirstTime()) / 20.);
			if(usedTimeChangeInMinutes < timeStepInMinutes*2)
				usedTimeChangeInMinutes  = timeStepInMinutes*2;

			// n‰m‰ on kCenter:iss‰ laskettuja arvoja
			double changeStartTimeInMinutes = xFac * usedTimeChangeInMinutes * (theDelta < 0 ? -1 : 1);
			double changeEndTimeInMinutes = (1.-xFac) * usedTimeChangeInMinutes * (theDelta > 0 ? -1 : 1);

			if(mousePlaceInScale == kLeft)
			{
				changeStartTimeInMinutes = 1.0 * timeStepInMinutes * (theDelta < 0 ? -1 : 1);
				changeEndTimeInMinutes = 0.0;
			}
			else if(mousePlaceInScale == kRight)
			{
				changeStartTimeInMinutes = 0.0;
				changeEndTimeInMinutes = 1. * timeStepInMinutes * (theDelta > 0 ? -1 : 1);
			}

			NFmiMetTime newStartTime = tmpTimeBag.FirstTime();
			newStartTime.ChangeByMinutes(static_cast<long>(changeStartTimeInMinutes));
			NFmiMetTime newEndTime = tmpTimeBag.LastTime();
			newEndTime.ChangeByMinutes(static_cast<long>(changeEndTimeInMinutes));

			NFmiTimeBag newTimeBag = CtrlViewUtils::GetAdjustedTimeBag(newStartTime, newEndTime, timeStepInMinutes);
			SetUsedTimeBag(newTimeBag);
		}
		else if(itsTimeView->GetFrame().IsInside(thePlace) && (theKey & kShiftKey))
		{
			// t‰h‰n aikajanan siirtokoodi
			NFmiTimeBag tmpTimeBag = GetUsedTimeBag();
			int timeStepInMinutes = GetUsedTimeResolutionInMinutes();
			if(timeStepInMinutes < 60)
				timeStepInMinutes  =  120;
			else if(timeStepInMinutes <= 120)
				timeStepInMinutes  =  timeStepInMinutes*3;
			else
				timeStepInMinutes  =  timeStepInMinutes*4;
            timeStepInMinutes = boost::math::iround(timeStepInMinutes / 60.f) * 60;

			tmpTimeBag.MoveByMinutes((theDelta > 0) ? timeStepInMinutes : -timeStepInMinutes);
			NFmiTimeBag fixedTimeBag = CtrlViewUtils::GetAdjustedTimeBag(tmpTimeBag.FirstTime(), tmpTimeBag.LastTime(), timeStepInMinutes); // fixataan aikabagi ainakin l‰himp‰‰n tuntiin
			SetUsedTimeBag(fixedTimeBag);
		}
		else if(CalcAnimationBoxRect().IsInside(thePlace) && CalcAnimationDelayButtonRect().IsInside(thePlace))
		{
			NFmiAnimationData& animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
			int currentDelayInMS = animData.FrameDelayInMS();
			int usedDeltaValue = 10;
			if(theDelta > 0)
			{
				if(currentDelayInMS >= 1000)
					usedDeltaValue = 100;
				else if(currentDelayInMS >= 500)
					usedDeltaValue = 50;
				else if(currentDelayInMS >= 100)
					usedDeltaValue = 20;
			}
			else
			{
				if(currentDelayInMS > 1000)
					usedDeltaValue = 100;
				else if(currentDelayInMS > 500)
					usedDeltaValue = 50;
				else if(currentDelayInMS > 100)
					usedDeltaValue = 20;
			}
			currentDelayInMS += (theDelta > 0) ? usedDeltaValue : -usedDeltaValue;
			if(currentDelayInMS < 0)
				currentDelayInMS = 0;
			else if(currentDelayInMS > 2000)
				currentDelayInMS = 2000;
			animData.FrameDelayInMS(currentDelayInMS);
			ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
			return true;
		}
		else if(CalcAnimationBoxRect().IsInside(thePlace) && CalcLastFrameDelayFactorButtonRect().IsInside(thePlace))
		{
			NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
			double currentLastFrameDelay = animData.LastFrameDelayFactor();
			currentLastFrameDelay += (theDelta > 0) ? 0.1 : -0.1;
			if(currentLastFrameDelay < 1)
				currentLastFrameDelay = 1;
			else if(currentLastFrameDelay > 5)
				currentLastFrameDelay = 5;
			animData.LastFrameDelayFactor(currentLastFrameDelay);
            ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
            return true;
		}
		else
		{
			// mousewheel up vie aikaa taakse- ja down eteenp‰in
			if(theDelta > 0)
                itsCtrlViewDocumentInterface->SetDataToNextTime(itsMapViewDescTopIndex, true);
			else
                itsCtrlViewDocumentInterface->SetDataToPreviousTime(itsMapViewDescTopIndex, true);
		}

        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
        return true;
	}
	return false;
}

bool NFmiTimeControlView::IsAnimationButtonPressed(void)
{
	return (fAnimationCloseButtonPressed || fAnimationPlayButtonPressed || fAnimationDelayButtonPressed || fAnimationRunModeButtonPressed || fAnimationLockModeButtonPressed || fAnimationLastFrameDelayButtonPressed || fAnimationVerticalControlButtonPressed);
}

bool NFmiTimeControlView::AnimationMouseMove(const NFmiPoint& thePlace, unsigned long theKey, bool &fixLastPosition)
{
	if(fMouseCapturedAnimationBox)
	{
		if(IsAnimationButtonPressed())
			return true; // jos joku animaatio nappula oli painettu pohjaan, ‰l‰ tee mit‰‰n, mutta palauta true

		NFmiAnimationData &animData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
		NFmiTimeBag currentShowTimes = animData.ShowTimebag();
		NFmiTimeBag newShowTimes = currentShowTimes;
		NFmiMetTime mousePointerTime = GetTimeFromPosition(thePlace, 1);
		if(itsAnimationDragDirection == kBase)
		{ // ollaan 'synnytt‰m‰ss‰' (shift-pohjassa) uutta animaatioaikav‰li‰
			if(mousePointerTime > animData.MouseDragStartTime())
				newShowTimes = NFmiTimeBag(animData.MouseDragStartTime(), mousePointerTime, 1);
			else
				newShowTimes = NFmiTimeBag(mousePointerTime, animData.MouseDragStartTime(), 1);
		}
		else if(itsAnimationDragDirection == kLeft)
		{ // venytet‰‰n animaatioaikav‰lin vasenta reunaa
			newShowTimes = NFmiTimeBag(mousePointerTime, currentShowTimes.LastTime(), 1);
		}
		else if(itsAnimationDragDirection == kRight)
		{ // venytet‰‰n animaatioaikav‰lin oikeaa reunaa
			newShowTimes = NFmiTimeBag(currentShowTimes.FirstTime(), mousePointerTime, 1);
		}
		else if(itsAnimationDragDirection == kCenter)
		{ // raahataan koko animaatioaikav‰li‰
			double xDiff = thePlace.X() - itsLastMousePosition.X();
			long diffInMinutes = RelativeLength2Minutes(xDiff);
			NFmiMetTime time1(newShowTimes.FirstTime());
			time1.SetTimeStep(1);
			time1.ChangeByMinutes(diffInMinutes);
			NFmiMetTime time2(newShowTimes.LastTime());
			time2.SetTimeStep(1);
			time2.ChangeByMinutes(diffInMinutes);
			// Pit‰‰ luodan uusi timebagi jossa jokainen komponentti on minuutti resoluutiolla ,timebagin MoveByMinutes-metodi ei toimi kunnolla t‰ss‰
			newShowTimes = NFmiTimeBag(time1, time2, 1);
			if(currentShowTimes == newShowTimes)
				fixLastPosition = false; // ei oteta talteen viime hiiren kursorin positiota, koska raahaus oli niin pieni ett‰ muutosta ei tapahtunut

		}
		animData.ShowTimebag(newShowTimes);
		return true;
	}
	return false;
}

bool NFmiTimeControlView::TimeScaleMouseMove(const NFmiPoint& thePlace, unsigned long theKey, bool &fixLastPosition)
{
	if(fMouseCapturedMoveTimeLine)
	{
		// t‰h‰n aikajanan siirtokoodi
		double xDiff = thePlace.X() - itsLastMousePosition.X();
		NFmiTimeBag tmpTimeBag = GetUsedTimeBag();
		NFmiTimeBag oldTimeBag = tmpTimeBag; // t‰h‰n verrataan onko muutoksia tapahtunut
		long diffInMinutes = RelativeLength2Minutes(xDiff);

		if(itsTimeScaleDragDirection == kCenter)
		{
			tmpTimeBag.MoveByMinutes(-diffInMinutes); // huom pit‰‰ laittaa - merkki eteen, muuten raahataan v‰‰r‰‰n suuntaan
		}
		else if(itsTimeScaleDragDirection == kLeft)
		{ // siirret‰‰n vasenta reunaa eli alkuaikaa
			NFmiMetTime startTime = tmpTimeBag.FirstTime();
			startTime.ChangeByMinutes(-diffInMinutes); // huom pit‰‰ laittaa - merkki eteen, muuten raahataan v‰‰r‰‰n suuntaan
			tmpTimeBag = NFmiTimeBag(startTime, tmpTimeBag.LastTime(), tmpTimeBag.Resolution());
		}
		else if(itsTimeScaleDragDirection == kRight)
		{ // siirret‰‰n oikeaa reunaa eli loppuaikaa
			NFmiMetTime endTime = tmpTimeBag.LastTime();
			endTime.ChangeByMinutes(-diffInMinutes); // huom pit‰‰ laittaa - merkki eteen, muuten raahataan v‰‰r‰‰n suuntaan
			tmpTimeBag = NFmiTimeBag(tmpTimeBag.FirstTime(), endTime, tmpTimeBag.Resolution());
		}

		NFmiTimeBag fixedTimeBag = CtrlViewUtils::GetAdjustedTimeBag(tmpTimeBag.FirstTime(), tmpTimeBag.LastTime(), 5); // fixataan aika bagi raahauksen yhteydess‰ l‰himp‰‰n 5 minuuttiin
		if((oldTimeBag == fixedTimeBag) == false)
		{
			SetUsedTimeBag(fixedTimeBag);
		}
		else
			fixLastPosition = false; // ei oteta talteen viime hiiren kursorin positiota, koska raahaus oli niin pieni ett‰ muutosta ei tapahtunut

		return true;
	}
	return false;
}

int NFmiTimeControlView::RelativeLength2Minutes(double theLength)
{
	NFmiTimeBag tmpTimeBag = GetUsedTimeBag();
	long timeScaleLengthInMinutes = tmpTimeBag.LastTime().DifferenceInMinutes(tmpTimeBag.FirstTime());
	double timeViewRelativeWidth = itsTimeView->GetFrame().Width();
	long diffInMinutes = static_cast<long>(timeScaleLengthInMinutes * theLength/ timeViewRelativeWidth);
	return diffInMinutes;
}

bool NFmiTimeControlView::MouseMove(const NFmiPoint& thePlace, unsigned long theKey)
{ // HUOM!! ei pid‰ tutkia onko hiiri ikkunan sis‰ll‰ vaan onko hiiri kaapattu!
    bool fixLastPosition = true;
    if(itsCtrlViewDocumentInterface->MouseCaptured())
    {
        if(theKey & kCtrlKey)
        {
            SetTime(thePlace); // kontrolli pohjassa siirret‰‰n aika hiiren kursorin kohdalle
        }
        else if(fMouseCaptured)// && mouseMoveIsOkNow)
        { // t‰m‰ blokki on editoinntiin liittyvien alku/loppu aikajanan s‰‰tˆ‰
            fMouseMovedWhenCaptured = true;
            NFmiMetTime timeStart = StartTime();
            NFmiMetTime timeEnd = EndTime();
            NFmiMetTime aTime = GetTimeFromPosition(thePlace, 1);
            if(abs(timeStart.DifferenceInMinutes(aTime)) <= abs(timeEnd.DifferenceInMinutes(aTime)))
            {
                StartTime(aTime);
            }
            else
            {
                EndTime(aTime);
            }
        }
        else if(TimeScaleMouseMove(thePlace, theKey, fixLastPosition))
        {
            fMouseMovedWhenCaptured = true;
        }
        else if(fMouseCapturedMoveSelectedTime)
        {
            fMouseMovedWhenCaptured = true;
            SetTime(thePlace, true); // kontrolli pohjassa siirret‰‰n aika hiiren kursorin kohdalle
        }
        else if(AnimationMouseMove(thePlace, theKey, fixLastPosition))
        {
            if(fixLastPosition == true)
                fMouseMovedWhenCaptured = true; // asetetaan vain trueksi, jos hiiri on jossain vaiheessa liikkunut animaatio boksin raahauksen takia.
                                                // n‰in vasen hiiren klikkaus pystyy tekem‰‰n aika-askel taaksep‰in toiminnon, jos hiiri ei merkitt‰v‰sti liiku
        }

        if(fixLastPosition)
            itsLastMousePosition = thePlace;
        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
        return true;
    }

    return false;
}

void NFmiTimeControlView::DrawTimeAxis()
{
	if(itsTimeView)
	{
		((NFmiAdjustedTimeScaleView*)itsTimeView)->FontSize(CtrlViewUtils::CalcTimeScaleFontSizeInPixels(GetGraphicalInfo().itsPixelsPerMM_x));
		NFmiStepTimeScale theSelectedScale(MakeEmptySelectedTimeScale());
		itsTimeView->SetSelectedScale(&theSelectedScale);
		itsToolBox->Draw(itsTimeView);
	}
}

double NFmiTimeControlView::GetAxisWidthShrinkFactor(void)
{
	return itsAxisShrinkFactor;
}

NFmiRect NFmiTimeControlView::CalcTimeAxisRect()
{
	NFmiRect axisRect(GetFrame());
	axisRect.Inflate(-axisRect.Width()*GetAxisWidthShrinkFactor(), 0);
	if(fShowResolutionChangerBox)
		axisRect.Right(itsResolutionChangerBox.Left());
	return axisRect;
}

NFmiRect NFmiTimeControlView::CalcAnimationBoxRect(void)
{
	NFmiRect animBoxRect; // luodaan tyhj‰ boksi defaulttina
    if(IsAnimationViewVisible())
	{ // jos boksi n‰ytet‰‰n aikakontrolli-ikkunassa, lasketaan sen alue
		animBoxRect = GetFrame();
		// HUOM! t‰ss‰ pit‰‰ pyyt‰‰ ns. showTimebag, joka saattaa poiketa k‰ytetyst‰ timebagist‰ esim. anim-boxin raahauksen tai venytyksen aikana
		const NFmiTimeBag &animShowTimes = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex).ShowTimebag();
		animBoxRect.Left(Time2Value(animShowTimes.FirstTime()));
		animBoxRect.Right(Time2Value(animShowTimes.LastTime()));
	}
	return animBoxRect;
}

void NFmiTimeControlView::DrawBackground()
{
	// piirret‰‰n pohja harmaalla
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	itsDrawingEnvironment.SetFillColor(NFmiColor(1.f,1.f,1.f));
	itsDrawingEnvironment.EnableFill();
	NFmiRect frame = GetFrame();
	NFmiRectangle tmp(frame.TopLeft(),
			  frame.BottomRight(),
			  0,
			  &itsDrawingEnvironment);
	itsToolBox->Convert(&tmp);
}

int NFmiTimeControlView::GetUsedTimeResolutionInMinutes(void)
{
	return static_cast<int>(::round((itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex) * 60)));
}

NFmiStepTimeScale NFmiTimeControlView::MakeEmptySelectedTimeScale(void)
{
	NFmiMetTime firstTime(itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex));
	NFmiTimeBag timeDescriptor2(firstTime,firstTime,1);
	return NFmiStepTimeScale(timeDescriptor2);
}

static NFmiMetTime GetTimeFromPosition(const NFmiTimeBag &theTimes, const NFmiPoint & thePlace, const NFmiRect & theTimeScaleRect, int theResolutionInMinutes)
{
	double relativePos = (thePlace.X() - theTimeScaleRect.Left()) / theTimeScaleRect.Width();
	long timeRangeInMinutes = theTimes.LastTime().DifferenceInMinutes(theTimes.FirstTime());
	long timeFromStartInMinutes = static_cast<long>(timeRangeInMinutes * relativePos);

	NFmiMetTime aTime(theTimes.FirstTime());
	aTime.ChangeByMinutes(timeFromStartInMinutes);
	aTime.SetTimeStep(theResolutionInMinutes);
	return aTime;
}

NFmiMetTime NFmiTimeControlView::GetTimeFromPosition(const NFmiPoint & thePlace, int theUsedResolution)
{
	return ::GetTimeFromPosition(*itsTimeBag, thePlace, itsTimeView->GetFrame(), theUsedResolution);
}

bool NFmiTimeControlView::SetTime(const NFmiPoint & thePlace, bool fStayInsideAnimationTimes)
{
//	NFmiMetTime oldTime = itsTimeBag->CurrentTime();
	NFmiMetTime oldTime = itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex);
	int resolution = static_cast<int>(::round(itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex)*60));
	try
	{
		int usedResolution = FmiMin(resolution, 60);
		NFmiMetTime aTime = GetTimeFromPosition(thePlace, usedResolution);
		if(fStayInsideAnimationTimes || itsTimeBag->IsInside(aTime))
		{
			if(oldTime != aTime)
                itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex, aTime, fStayInsideAnimationTimes);
			else
				return false;
		}
		return true;
	}
	catch(...)
	{
		// virhetilanteessa ei tehd‰ mit‰‰, eli ei siirret‰ aikaakaan
		return false;
	}

}

double NFmiTimeControlView::Time2Value(const NFmiMetTime& theTime)
{
	double value = ((NFmiTimeScale*)itsTimeAxis->Scale())->RelTimeLocation(theTime);
	NFmiRect rect(CalcTimeAxisRect());
	double finalValue = rect.Left() + value * rect.Width();
	return finalValue;
}

CtrlViewUtils::GraphicalInfo& NFmiTimeControlView::GetGraphicalInfo(void)
{
	return itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
}

static std::string GetToolTipTimeString(const NFmiMetTime &theTime, FmiLanguage theLanguage)
{
	return static_cast<char*>(theTime.ToStr(::GetDictionaryString("StationDataTableViewTimeFormat"), theLanguage));
}

std::string NFmiTimeControlView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	try
	{
		std::string str;

		// Kaikissa tapauksissa jos Virtual-Time moodi p‰‰ll‰, laitetaan tietoa siit‰ heti ensimm‰iseksi
		str += MakePossibleVirtualTimeTooltipText();

        if(CalcFullTimeRangeButtonRect().IsInside(theRelativePoint))
        { // Animaatio ikkuna ei siis saa olla p‰‰ll‰ ja jos ollaan FullTimeRange -napin sis‰ll‰
            str += "Set full editing time range for edited data";
        }
		else if(IsInsideAnyAnimationButton(theRelativePoint))
		{
            NFmiAnimationData &animationData = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex);
			if(CalcAnimationPlayButtonRect().IsInside(theRelativePoint))
			{
				if(animationData.AnimationOn())
					str += "Pause button (Play mode on)";
				else
					str += "Play button (Pause mode on)";
			}
			else if(CalcAnimationDelayButtonRect().IsInside(theRelativePoint))
			{
				str += "Animation delay button\n";
				str += "Delay now ";
                str += NFmiStringTools::Convert<int>(boost::math::iround(animationData.FrameDelayInMS() / 10.));
				str += " [ms]";
			}
			else if(CalcLastFrameDelayFactorButtonRect().IsInside(theRelativePoint))
			{
				str += "Last frame delay factor\n";
				str += "Delay is now ";
				str += NFmiStringTools::Convert<double>(animationData.LastFrameDelayFactor());
				str += "x";
			}
			else if(CalcAnimationRunModeButtonRect().IsInside(theRelativePoint))
			{
				NFmiAnimationData::RunMode runMode = animationData.GetRunMode();
				if(runMode == NFmiAnimationData::kNormal)
					str += "Sweep button (Forward mode on)";
				else if(runMode == NFmiAnimationData::kSweep)
					str += "Backward button (Sweep mode on)";
				else
					str += "Forward button (Backward mode on)";
			}
			else if(CalcAnimationLockModeButtonRect().IsInside(theRelativePoint))
			{
                static const std::string noLockModeStr = "\"No-lock\" mode";
                static const std::string followLatestkModeStr = "\"Follow latest of any obs time\" mode";
                static const std::string followLatestkMutualModeStr = "\"Follow latest mutual obs time\" mode";
                static const std::string pressButtonForStr = "Press button for ";
                static const std::string isOnStr = " is on\n(";
				NFmiAnimationData::AnimationLockMode lockMode = animationData.LockMode();
				if(lockMode == NFmiAnimationData::kNoLock)
					str += noLockModeStr + isOnStr + pressButtonForStr + followLatestkModeStr + ")";
				else if(lockMode == NFmiAnimationData::kFollowLastObservation)
					str += followLatestkModeStr + isOnStr + pressButtonForStr + followLatestkMutualModeStr + ")";
				else
					str += followLatestkMutualModeStr + isOnStr + pressButtonForStr + noLockModeStr + ")";
			}
			else if(CalcAnimationCloseButtonRect().IsInside(theRelativePoint))
			{
				str += "Close animation window";
			}
			else if(CalcAnimationVerticalControlButtonRect().IsInside(theRelativePoint))
			{
				if(animationData.ShowVerticalControl())
					str += "Vertical control off (Now shown on map view)";
				else
					str += "Vertical control on (Now not shown on map view)";
			}
		}
		else if(CalcAnimationBoxRect().IsInside(theRelativePoint))
		{ // laitetaan animaatio alku-luppu ajat, resoluutio ja ollaanko anim moodissa vai ei
			NFmiTimeBag animTimes = itsCtrlViewDocumentInterface->AnimationData(itsMapViewDescTopIndex).Times();
			str += "Animation\nStart: ";
			str += ::GetToolTipTimeString(animTimes.FirstTime(), itsCtrlViewDocumentInterface->Language());
			str += "\nEnd: ";
			str += ::GetToolTipTimeString(animTimes.LastTime(), itsCtrlViewDocumentInterface->Language());
			str += "\nStep: ";
			str += GetResolutionText();
		}
		else
		{
			str += "<font color=blue>Cursor time:</font>\n";
			NFmiMetTime aTime1 = GetTimeFromPosition(theRelativePoint, 10);
			str += ::GetToolTipTimeString(aTime1, itsCtrlViewDocumentInterface->Language());

			str += "<br><hr color=red><br>";

			str += "<font color=green>Selected time:</font>\n";
			NFmiMetTime aTime2 = itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex);
			str += ::GetToolTipTimeString(aTime2, itsCtrlViewDocumentInterface->Language());
		}
		return str;
	}
	catch(...)
	{
		// ei tehd‰ mit‰‰n 
	}
	return std::string();
}

NFmiTimeControlView::MouseStatusInfo NFmiTimeControlView::GetMouseStatusInfo(void)
{
	MouseStatusInfo info;
	info.fMouseCaptured = fMouseCaptured;
	info.fMouseCapturedMoveTimeLine = fMouseCapturedMoveTimeLine;
	info.fMouseCapturedMoveSelectedTime = fMouseCapturedMoveSelectedTime;
	info.fMouseMovedWhenCaptured = fMouseMovedWhenCaptured;
	info.itsLastMousePosition = itsLastMousePosition;
	info.fMouseCapturedAnimationBox = fMouseCapturedAnimationBox;
	info.itsAnimationDragDirection = itsAnimationDragDirection;
	info.itsTimeScaleDragDirection = itsTimeScaleDragDirection;
	info.fAnimationCloseButtonPressed = fAnimationCloseButtonPressed;
	info.fAnimationPlayButtonPressed = fAnimationPlayButtonPressed;
	info.fAnimationDelayButtonPressed = fAnimationDelayButtonPressed;
	info.fAnimationRunModeButtonPressed = fAnimationRunModeButtonPressed;
	info.fAnimationLastFrameDelayButtonPressed = fAnimationLastFrameDelayButtonPressed;
	info.fAnimationLockModeButtonPressed = fAnimationLockModeButtonPressed;
	info.fAnimationVerticalControlButtonPressed = fAnimationVerticalControlButtonPressed;
	info.itsLeftButtonDownMousePosition = itsLeftButtonDownMousePosition;

	return info;
}

void NFmiTimeControlView::SetMouseStatusInfo(const MouseStatusInfo &theMouseStatusInfo)
{
	fMouseCaptured = theMouseStatusInfo.fMouseCaptured;
	fMouseCapturedMoveTimeLine = theMouseStatusInfo.fMouseCapturedMoveTimeLine;
	fMouseCapturedMoveSelectedTime = theMouseStatusInfo.fMouseCapturedMoveSelectedTime;
	fMouseMovedWhenCaptured = theMouseStatusInfo.fMouseMovedWhenCaptured;
	itsLastMousePosition = theMouseStatusInfo.itsLastMousePosition;
	fMouseCapturedAnimationBox = theMouseStatusInfo.fMouseCapturedAnimationBox;
	itsAnimationDragDirection = theMouseStatusInfo.itsAnimationDragDirection;
	itsTimeScaleDragDirection = theMouseStatusInfo.itsTimeScaleDragDirection;
	fAnimationCloseButtonPressed = theMouseStatusInfo.fAnimationCloseButtonPressed;
	fAnimationPlayButtonPressed = theMouseStatusInfo.fAnimationPlayButtonPressed;
	fAnimationDelayButtonPressed = theMouseStatusInfo.fAnimationDelayButtonPressed;
	fAnimationRunModeButtonPressed = theMouseStatusInfo.fAnimationRunModeButtonPressed;
	fAnimationLastFrameDelayButtonPressed = theMouseStatusInfo.fAnimationLastFrameDelayButtonPressed;
	fAnimationLockModeButtonPressed = theMouseStatusInfo.fAnimationLockModeButtonPressed;
	fAnimationVerticalControlButtonPressed = theMouseStatusInfo.fAnimationVerticalControlButtonPressed;
	itsLeftButtonDownMousePosition = theMouseStatusInfo.itsLeftButtonDownMousePosition;
}


// ---------------------------------------------------------------------------------------

NFmiTimeSerialTimeControlView::NFmiTimeSerialTimeControlView(int theMapViewDescTopIndex,
							 const NFmiRect & theRect
							,NFmiToolBox * theToolBox
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,bool theShowResolutionChangerBox
							,bool theDrawViewInMouseMove
							,bool theShowEditTimeHandles
							,bool theShowSelectedTimes)
:NFmiTimeControlView(theMapViewDescTopIndex,
					 theRect
					,theToolBox
					,theDrawParam
					,theShowResolutionChangerBox
					,theDrawViewInMouseMove
					,theShowEditTimeHandles
					,theShowSelectedTimes
					,true
					   )
{
}

NFmiTimeSerialTimeControlView::~NFmiTimeSerialTimeControlView(void)
{
}

bool NFmiTimeSerialTimeControlView::LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey)
{
	if(NFmiTimeControlView::LeftButtonDown(thePlace, theKey))
	{
        itsCtrlViewDocumentInterface->MouseCapturedInTimeWindow(true);
		return true;
	}
	return false;
}

bool NFmiTimeSerialTimeControlView::LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey)
{
	bool status = NFmiTimeControlView::LeftButtonUp(thePlace, theKey);
    itsCtrlViewDocumentInterface->MouseCapturedInTimeWindow(false);
	return status;
}

NFmiPoint NFmiTimeSerialTimeControlView::GetViewSizeInPixels(void)
{
	return itsCtrlViewDocumentInterface->TimeSerialViewSizeInPixels();
}

const NFmiTimeBag& NFmiTimeSerialTimeControlView::GetUsedTimeBag(void)
{
	return itsCtrlViewDocumentInterface->TimeSerialViewTimeBag();
}

void NFmiTimeSerialTimeControlView::SetUsedTimeBag(const NFmiTimeBag &newTimeBag)
{
    itsCtrlViewDocumentInterface->TimeSerialViewTimeBag(newTimeBag);
}

NFmiTrajectoryTimeControlView::NFmiTrajectoryTimeControlView(int theMapViewDescTopIndex, const NFmiRect & theRect
															,NFmiToolBox * theToolBox
															,boost::shared_ptr<NFmiDrawParam> &theDrawParam)
:NFmiTimeControlView(theMapViewDescTopIndex, theRect, theToolBox, theDrawParam, false, false, false, false, false)
{
}

NFmiTrajectoryTimeControlView::~NFmiTrajectoryTimeControlView(void)
{
}


const NFmiTimeBag& NFmiTrajectoryTimeControlView::GetUsedTimeBag(void)
{
	// t‰h‰n pit‰‰ pyyt‰‰ trajektori systeemilt‰ valittujen trajektorien 1. ja viimeisen ajan muodostama timebagi
	return itsCtrlViewDocumentInterface->TrajectorySystem()->TrajectoryViewTimeBag();
}

void NFmiTrajectoryTimeControlView::SetUsedTimeBag(const NFmiTimeBag &newTimeBag)
{
    itsCtrlViewDocumentInterface->TrajectorySystem()->TrajectoryViewTimeBag(newTimeBag);
}

