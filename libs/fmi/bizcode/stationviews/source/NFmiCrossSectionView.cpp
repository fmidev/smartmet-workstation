#include "NFmiCrossSectionView.h"
#include "NFmiToolBox.h"
#include "NFmiLine.h"
#include "NFmiIsoLineData.h"
#include "NFmiGrid.h"
#include "NFmiDrawParam.h"
#include "NFmiDataHints.h"
#include "NFmiText.h"
#include "NFmiInfoOrganizer.h"
#include "ToolMasterDrawingFunctions.h"
#include "NFmiCrossSectionSystem.h"
#include "NFmiValueString.h"
#include "NFmiRectangle.h"
#include "NFmiStringTools.h"
#include "NFmiDrawParamList.h"
#include "NFmiWindBarb.h"
#include "NFmiPolyLine.h"
#include "NFmiTrajectorySystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiSmartToolModifier.h"
#include "NFmiSmartToolInfo.h"
#include "NFmiProducerSystem.h"
#include "NFmiSoundingData.h"
#include "NFmiMetMath.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiParamHandlerView.h"
#include "NFmiSoundingFunctions.h"
#include "NFmiExtraMacroParamData.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiTrajectory.h"
#include "CtrlViewFunctions.h"
#include "CtrlViewKeyboardFunctions.h"
#include "MapHandlerInterface.h"
#include "NFmiMacroParam.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "BetaProductParamBoxFunctions.h"
#include "NFmiFastInfoUtils.h"
#include "EditedInfoMaskHandler.h"
#include "ToolBoxStateRestorer.h"
#include "CtrlViewColorContourLegendDrawingFunctions.h"
#include "NFmiColorContourLegendSettings.h"
#include "NFmiColorContourLegendValues.h"
#include "catlog/catlog.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiPathUtils.h"
#include "TimeSerialModification.h"
#include "ParamHandlerViewFunctions.h"
#include "ColorStringFunctions.h"

#include <stdexcept>
#include "boost/math/special_functions/round.hpp"
#include "boost/make_shared.hpp"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// t‰m‰ viritetty viivan piirto pit‰isi korvata GDI+ piirroilla!!!!!
static void DrawLineWithToolBox(const NFmiPoint& theStartingPoint, const NFmiPoint& theEndingPoint, NFmiDrawingEnvironment *theEnvi, NFmiToolBox *theToolBox, bool moveXDir, bool moveYDir)
{
	FmiPattern pattern = theEnvi->GetFillPattern();
	int penSize = FmiRound(theEnvi->GetPenSize().X());
	if(pattern == FMI_SOLID || pattern == FMI_RELATIVE_FILL || penSize <= 1)
	{
		NFmiLine line(theStartingPoint, theEndingPoint, 0, theEnvi);
		theToolBox->Convert(&line);
	}
	else
	{ // tehd‰‰n tarvittaessa 1 pikseli‰ paksumpaa dash-viivaa omalla virityksell‰
		NFmiPoint oldPenSize = theEnvi->GetPenSize();
		theEnvi->SetPenSize(NFmiPoint(1,1));
		double pixelShiftX = theToolBox->SX(1);
		double pixelShiftY = theToolBox->SY(1);
		double lineX1 = theStartingPoint.X();
		double lineY1 = theStartingPoint.Y();
		double lineX2 = theEndingPoint.X();
		double lineY2 = theEndingPoint.Y();
		int forLoopSize = penSize/2;
		// piierret‰‰n ensin viivan 'vasemmalle' puolelle yksitt‰isi‰ viivoja (joista paksu viiva koostuu)
		for(int i = 0; i < forLoopSize; i++)
		{
			if(moveXDir)
			{
				lineX1 -= pixelShiftX;
				lineX2 -= pixelShiftX;
			}
			if(moveYDir)
			{
				lineY1 -= pixelShiftY;
				lineY2 -= pixelShiftY;
			}
			NFmiLine line(NFmiPoint(lineX1, lineY1), NFmiPoint(lineX2, lineY2), 0, theEnvi);
			theToolBox->Convert(&line);
		}

		lineX1 = theStartingPoint.X();
		lineY1 = theStartingPoint.Y();
		lineX2 = theEndingPoint.X();
		lineY2 = theEndingPoint.Y();
		for(int i = 0; i < forLoopSize; i++)
		{
			if(moveXDir)
			{
				lineX1 += pixelShiftX;
				lineX2 += pixelShiftX;
			}
			if(moveYDir)
			{
				lineY1 += pixelShiftY;
				lineY2 += pixelShiftY;
			}
			NFmiLine line(NFmiPoint(lineX1, lineY1), NFmiPoint(lineX2, lineY2), 0, theEnvi);
			theToolBox->Convert(&line);
		}

		theEnvi->SetPenSize(oldPenSize);
	}
}

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------

NFmiCrossSectionView::NFmiCrossSectionView(NFmiToolBox * theToolBox
                                 ,int viewGridRowNumber
                                 ,int viewGridColumnNumber)
:NFmiIsoLineView(CtrlViewUtils::kFmiCrossSectionView
				 ,boost::shared_ptr<NFmiArea>()
				,theToolBox
				,boost::shared_ptr<NFmiDrawParam>()
				,kFmiTemperature
				,NFmiPoint()
				,NFmiPoint()
				,viewGridRowNumber
                ,viewGridColumnNumber)
,itsLowerEndOfPressureAxis(1015)
,itsUpperEndOfPressureAxis(150)
,itsDataViewFrame(0.1,0.1,0.9,0.9)
,itsPressureScaleFrame()
,itsPressures()
,itsModelGroundPressures()
,itsGroundHeights()
,itsRoutePointsDistToEndPoint()
,itsPressureScaleFontSize(16, 16)
,itsParamHandlerView()
,itsParamHandlerViewRect()
,itsDrawSizeFactorX(1)
,itsDrawSizeFactorY(1)
,fDoCrossSectionDifferenceData(false)
{
	NFmiCrossSectionSystem::ExtraRowInfo &defaultAxisValues = itsCtrlViewDocumentInterface->CrossSectionSystem()->AxisValuesDefault();
	itsLowerEndOfPressureAxis = defaultAxisValues.itsLowerEndOfPressureAxis;
	itsUpperEndOfPressureAxis = defaultAxisValues.itsUpperEndOfPressureAxis;
	CalculateViewRects();
	StorePressureScaleLimits();

	InitParamHandlerView();
}

void NFmiCrossSectionView::InitParamHandlerView(void)
{
	CtrlView::GeneralInitParamHandlerView(this, itsParamHandlerView, false, false);
}

void NFmiCrossSectionView::UpdateParamHandlerView(void)
{
	CtrlView::GeneralUpdateParamHandlerView(this, itsParamHandlerView);
}

NFmiRect NFmiCrossSectionView::CalcParamHandlerViewRect(void)
{
	return CtrlView::GeneralCalcParamHandlerViewRect(this, itsDataViewFrame);
}

void NFmiCrossSectionView::SetParamHandlerViewRect(const NFmiRect& newRect)
{
	itsParamHandlerViewRect = newRect;
}

bool NFmiCrossSectionView::ShowParamHandlerView(void)
{
	if(itsParamHandlerView)
	{
		if(itsCtrlViewDocumentInterface->IsParamWindowViewVisible(itsMapViewDescTopIndex))
			return true;
	}
	return false;
}

void NFmiCrossSectionView::DrawParamView(NFmiToolBox * theGTB)
{
    if(itsCtrlViewDocumentInterface->BetaProductGenerationRunning())
    {
        // HUOM! Gdiplus piirtoja k‰ytet‰‰n poikkileikkaus piirrossa vain t‰ss‰ erikoistapauksessa. 
        // Jos se otetaan muuallakin k‰yttˆˆn, alustus ja siivous pit‰‰ siirt‰‰ NFmiCrossSectionView::Draw metodiin.
        InitializeGdiplus(itsToolBox, &GetFrame());
        StationViews::DrawBetaProductParamBox(this, true, nullptr);
        CleanGdiplus();
    }
    else
    {
        if(ShowParamHandlerView())
        {
            UpdateParamHandlerView();
            itsParamHandlerView->Draw(theGTB);
        }
    }
}

NFmiCrossSectionView::~NFmiCrossSectionView(void)
{
}

void NFmiCrossSectionView::StorePressureScaleLimits(void)
{
	NFmiCrossSectionSystem::ExtraRowInfo &extraRowInfo = itsCtrlViewDocumentInterface->CrossSectionSystem()->GetRowInfo(itsViewGridRowNumber -1); // itsViewGridRowNumber alkaa 1:st‰, mutta GetRowInfo:n indeksit alkavat 0:sta.
	extraRowInfo.itsLowerEndOfPressureAxis = itsLowerEndOfPressureAxis;
	extraRowInfo.itsUpperEndOfPressureAxis = itsUpperEndOfPressureAxis;
}

double NFmiCrossSectionView::y2p(double y) const
{
	const double errLimit = 0.001; // onko y framen sis‰ll‰ pit‰‰ sallia pieni virhe raja
	double p = kFloatMissing;
	if(y <= itsDataViewFrame.Bottom() + errLimit && y >= itsDataViewFrame.Top() - errLimit)
	{
		double boxh = itsDataViewFrame.Height();
		double by = itsDataViewFrame.Bottom();
		double dp = ::log(itsLowerEndOfPressureAxis) - ::log(itsUpperEndOfPressureAxis);
		p = itsLowerEndOfPressureAxis * ::exp((y-by)/(boxh/dp));
	}
	return p;
}

double NFmiCrossSectionView::p2y(double p) const
{
	return p2y(itsDataViewFrame, p);
}

double NFmiCrossSectionView::p2y(const NFmiRect& usedDataRect, double p) const
{
	if(p == kFloatMissing)
		return kFloatMissing;
	double boxh = usedDataRect.Height();
	double by = usedDataRect.Bottom();
	double dp = ::log(itsLowerEndOfPressureAxis) - ::log(itsUpperEndOfPressureAxis);
	return by - (::log(itsLowerEndOfPressureAxis) - ::log(p)) * (boxh / dp);
}

//--------------------------------------------------------
// Draw
//--------------------------------------------------------
void NFmiCrossSectionView::Draw(NFmiToolBox *theGTB)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, std::string(__FUNCTION__) + ": starting to draw cross-section row");
    if(!theGTB)
		return;
	itsToolBox = theGTB; // v‰h‰n arvelluttaa t‰m‰, koska toolboxien pit‰isi olla jo samoja

	if(!itsCtrlViewDocumentInterface->CrossSectionSystem()->IsViewVisible(itsViewGridRowNumber))
		return ;
    InitializeGdiplus(itsToolBox, &itsRect);
	// Tyhjennet‰‰n aina piirron aluksi
	itsExistingLabels.clear(); //EL
	itsOptimizedGridPtr.reset();

	CalculateViewRects();
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(NFmiColor(1.f,1.f,1.0f));
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	itsDrawingEnvironment.SetPenSize(NFmiPoint(1, 1));
	DrawFrame(itsDrawingEnvironment);

	//************** QUICKFIX!!!!! *******************
	// itsArea:n recti‰ tarvitaan NFmiIsoLineView::DrawSimpleIsoLinesWithImagine-metodissa
	// kun tehd‰‰n clippausta ja t‰m‰ pit‰‰ korvata jotenkin virtuaali jutulla
	itsArea = GetZoomedArea();

	//************** QUICKFIX!!!!! *******************
	boost::shared_ptr<NFmiDrawParam> oldDrawParam = itsDrawParam;
	PreCalculateTrajectoryLatlonPoints();
    itsCtrlViewDocumentInterface->CrossSectionSystem()->CalcMinorPoints(itsArea); // p‰ivitet‰‰n poikkileikkaus pisteet
    itsCtrlViewDocumentInterface->CrossSectionSystem()->CalcRouteTimes(); // t‰m‰kin pit‰‰ varmistaa
	CalcRouteDistances();
	NFmiDrawParamList *dpList = itsCtrlViewDocumentInterface->CrossSectionViewDrawParamList(itsViewGridRowNumber);
	if(dpList)
	{
		int i = 1;
		itsCrossSectionIsoLineDrawIndex = -1;

		for(dpList->Reset(); dpList->Next(); i++)
		{
			itsExistingLabels.clear(); //EL
			itsDrawParam = dpList->Current(); // asetetaan todella k‰ytetty drawParam k‰yttˆˆn
			itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, true, true, fGetCurrentDataFromQ2Server);
            NFmiStationView::SetupPossibleWindMetaParamData();
            if(itsInfo == 0 || itsDrawParam == 0)
			{ // voi menn‰ t‰h‰n esim. jos on tiputtanut ensin editoriin level dataa ja katsoo
			  // editoituna datana vaikka l‰mpˆtilaa. Sitten laitetaan editori operatiiviseen
			  // tilaan painamalla Lataa-nappia, jolloin editoitava data on pinta dataa (nyt
			  // se katsottu l‰mpˆtila drawParam pit‰‰ poistaa)
			  // itsCrossSectionDrawParam voi olla 0-pointteri, jos kyseess‰ viallinen MacroParam

				//dpList->Remove(true); // Ei poisteta en‰‰ jos dataa ei lˆydy
				continue;
			}
			DoTimeInterpolationSettingChecks(itsInfo);
			UpdateCachedParameterName();
			if(!itsDrawParam->IsParamHidden())
			{
				PrepareForTransparentDraw(); // jos piirto-ominaisuudessa on transparenssia, pit‰‰ tehd‰ kikka vitonen
				try
				{
					DrawCrossSection();
				}
				catch(std::exception &e)
				{
					std::string errorMessage = "Error in ";
					errorMessage += __FUNCTION__;
					errorMessage += " with parameter ";
					errorMessage += itsDrawParam->ParameterAbbreviation();
					errorMessage += ": ";
					errorMessage += e.what();
					CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
				}
				catch(...)
				{
					std::string errorMessage = "Unknown error in ";
					errorMessage += __FUNCTION__;
					errorMessage += " with parameter ";
					errorMessage += itsDrawParam->ParameterAbbreviation();
					CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
				}
				EndTransparentDraw(); // jos piirrossa oli l‰pin‰kyvyytt‰, pit‰‰ viel‰ tehd‰ pari kikkaa ja siivota j‰ljet
			}
		}

        DrawLegends();
		DrawTrajectory(itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(itsViewGridRowNumber - 1), itsCtrlViewDocumentInterface->GeneralColor(itsViewGridRowNumber - 1));
		DrawObsForModeTimeLine();
		DrawHelperTimeLines();
		DrawFlightLevelScale();
		DrawHeightScale();
		DrawGround();
		DrawHybridLevels();
		// piirret‰‰n viel‰ dataframe
		itsDrawingEnvironment.DisableFill();
		NFmiRectangle rect(itsDataViewFrame.TopLeft()
						,itsDataViewFrame.BottomRight()
						,0
						,&itsDrawingEnvironment);
		itsToolBox->Convert(&rect);
		DrawPressureScale();

		DrawParamView(theGTB); // piirrett‰v‰ viimeiseksi kartan p‰‰lle!!!
	}
	//************** QUICKFIX!!!!! *******************
	itsDrawParam = oldDrawParam; // laitetaan mik‰ oli sitten ennen piirtoa drawParam takaisin
	itsInfo = boost::shared_ptr<NFmiFastQueryInfo>();
	//************** QUICKFIX!!!!! *******************
    CleanGdiplus();
}

void NFmiCrossSectionView::DrawLegends()
{
    auto drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, itsViewGridRowNumber);
    if(drawParamList && drawParamList->NumberOfItems() >= 1)
    {
        auto& colorContourLegendSettings = itsCtrlViewDocumentInterface->ColorContourLegendSettings();
		auto* crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
        auto& graphicalInfo = crossSectionSystem->GetGraphicalInfo();
		auto sizeFactor = NFmiStationView::CalcUsedLegendSizeFactor(itsCtrlViewDocumentInterface->SingleMapViewHeightInMilliMeters(itsMapViewDescTopIndex), crossSectionSystem->RowCount());
        auto lastLegendRelativeBottomRightCorner = CtrlView::CalcProjectedPointInRectsXyArea(itsDataViewFrame, itsCtrlViewDocumentInterface->ColorContourLegendSettings().relativeStartPosition());

        for(const auto& drawParam : *drawParamList)
        {
            auto drawParamPtr = boost::make_shared<NFmiDrawParam>(*drawParam);
            auto fastInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(drawParamPtr, false, true);
            NFmiColorContourLegendValues colorContourLegendValues(drawParamPtr, fastInfo);
            if(colorContourLegendValues.useLegend())
            {
                CtrlView::DrawNormalColorContourLegend(colorContourLegendSettings, colorContourLegendValues, lastLegendRelativeBottomRightCorner, itsToolBox, graphicalInfo, *itsGdiPlusGraphics, sizeFactor, itsDataViewFrame);
            }
        }
    }
}

void NFmiCrossSectionView::PreCalculateTrajectoryLatlonPoints(void)
{
	itsTrajectoryLatlonPoints.clear();
    auto trajectorySystem = itsCtrlViewDocumentInterface->TrajectorySystem();
	if(trajectorySystem->ShowTrajectoriesInCrossSectionView())
	{
		int ssize = static_cast<int>(trajectorySystem->Trajectories().size());
		if(ssize > itsViewGridRowNumber - 1)
		{
			const_cast<NFmiTrajectory &>(trajectorySystem->Trajectory(itsViewGridRowNumber - 1)).CalculateCrossSectionTrajectoryHelpData();
			const NFmiTrajectory &trajectory = trajectorySystem->Trajectory(itsViewGridRowNumber - 1);
			itsTrajectoryLatlonPoints = trajectory.CrossSectionTrajectoryPoints();
		}
	}
}

std::string NFmiCrossSectionView::ComposeTrajectoryToolTipText()
{
	std::string str;
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
		const auto &trajectory = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(itsViewGridRowNumber - 1);
		str += "<hr color=red><br>";
		str += "<b><font color=";
		str += ColorString::Color2HtmlColorStr(itsCtrlViewDocumentInterface->GeneralColor(itsViewGridRowNumber - 1));
		str += ">";

		str += "Trajectory (";
		str += std::to_string(itsViewGridRowNumber);
		str += "):";
		if(trajectory.Direction() == kForward)
			str += " -> ";
		else
			str += " <- ";
		NFmiLevel level((trajectory.DataType() == 2) ? kFmiHybridLevel : kFmiPressureLevel, 1);
		if(trajectory.DataType() == 0)
			level.SetIdent(kFmiHeight);
		str += itsCtrlViewDocumentInterface->ProducerSystem().GetProducerAndLevelTypeString(trajectory.Producer(), level, trajectory.OriginTime(), false);
		str += " ";
		str += trajectory.Time().ToStr(::GetDictionaryString("CrossSectionDlgTimeStr"), itsCtrlViewDocumentInterface->Language());
		if(trajectory.DataType() != 0)
		{
			str += " P=";
			str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(trajectory.PressureLevel(), 1);
			if(trajectory.Isentropic())
			{
				str += " Is. ";
				str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(trajectory.IsentropicTpotValue(), 1);
			}
		}
	}
	return str;
}

// v‰ri otetaan v‰liaikaisesti ulkoa luotaus systeemist‰
void NFmiCrossSectionView::DrawTrajectory(const NFmiTrajectory &theTrajectory, const NFmiColor &theColor)
{
	if(!itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
		return ;

    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsDataViewFrame);

	NFmiDrawingEnvironment envi;
	// piirret‰‰n sitten p‰‰-trajektori
	envi.SetFrameColor(theColor);
	envi.SetPenSize(NFmiPoint(3 * itsDrawSizeFactorX, 3 * itsDrawSizeFactorY));
	DrawSingleTrajector(theTrajectory.MainTrajector(), envi, theTrajectory.TimeStepInMinutes(), FmiRound(7 * itsDrawSizeFactorX), FmiRound(2 * itsDrawSizeFactorX), theTrajectory.Direction());
}

static double Time2X(const NFmiMetTime &theTime, const NFmiMetTime &theStartTime, double theTotalDiffInMinutes, const NFmiRect &theRect)
{
	double currentDiff = theTime.DifferenceInMinutes(theStartTime);
	double x = theRect.Left() + currentDiff/theTotalDiffInMinutes * theRect.Width();
	return x;
}

static double TimeStepWidth(double theTotalDiffInMinutes, double timeStepInMinutes, const NFmiRect &theRect)
{
	double offset = timeStepInMinutes/theTotalDiffInMinutes;
	double width = offset * theRect.Width();
	return width;
}

const NFmiMetTime& NFmiCrossSectionView::CurrentTime(void)
{
    return itsCtrlViewDocumentInterface->ActiveMapTime(); // haetaan aktiivisen karttan‰ytˆn current time
}

void NFmiCrossSectionView::DrawSingleTrajector(const NFmiSingleTrajector &theSingleTrajector, NFmiDrawingEnvironment &theEnvi, int theTimeStepInMinutes, int theTimeMarkerPixelSize, int theTimeMarkerPixelPenSize, FmiDirection theDirection)
{
	const NFmiTimeBag times(GetUsedTimeBagForDataCalculations());

	NFmiMetTime startTime(times.FirstTime());
	double timeDiffInMinutes = times.LastTime().DifferenceInMinutes(startTime);
	bool forwardDir = (theDirection == kForward);
	NFmiMetTime mapTime(CurrentTime());
	NFmiMetTime time1(theSingleTrajector.StartTime());
	double startX = ::Time2X(time1, startTime, timeDiffInMinutes, itsDataViewFrame);
	double x = startX;
	double stepX = ::TimeStepWidth(timeDiffInMinutes, theTimeStepInMinutes, itsDataViewFrame);
	if(!forwardDir)
		stepX = -stepX;
	NFmiMetTime time2;
	NFmiPoint mapTimeP(kFloatMissing, kFloatMissing); // haetaan t‰h‰n se piste, miss‰ trajektori oli menossa kun t‰m‰n kartan aika oli
	NFmiMetTime currentTime(theSingleTrajector.StartTime()); // pidet‰‰n laskua currentin pisteen ajasta
	const std::vector<float> &pressures = theSingleTrajector.Pressures();
	std::vector<float>::const_iterator it = pressures.begin();
	bool showTrajectoryArrows = itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoryArrows();
	bool showTrajectoryAnimationMarkers = itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoryAnimationMarkers();
	if(showTrajectoryArrows == false && showTrajectoryAnimationMarkers == false)
		return ;
	if(it != pressures.end())
	{
		double pressure1 = *it;
		double y = p2y(pressure1);
		double pressure2 = kFloatMissing;
		NFmiPoint p1(x, y);
		NFmiPoint p2;
		NFmiPolyline trajectorPolyLine(itsRect, 0, &theEnvi);
		trajectorPolyLine.AddPoint(p1);
		std::vector<float>::const_iterator endIt = pressures.end();
		++it; // pit‰‰ juoksuttaa yhden pyk‰l‰n verran eteenp‰in
		for( ; it != endIt; ++it )
		{
			x += stepX;
			pressure2 = *it;
			if(pressure2 == kFloatMissing)
				break; // lopetetaan heti kun on ei ok paine arvo
			y = p2y(pressure2);
			p2 = NFmiPoint(x, y);

			if(currentTime == mapTime)
				mapTimeP = p1;

			currentTime.ChangeByMinutes(forwardDir ? theTimeStepInMinutes : -theTimeStepInMinutes);

			if(showTrajectoryArrows) // HUOM! looppia pit‰‰ kuitenkin k‰yd‰ l‰pi vaikka ei piirret‰, jos markerit piirret‰‰n
				trajectorPolyLine.AddPoint(p2);
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
			if(pressures.size() >= 2)
			{
				long arrowHeadSize = theTimeMarkerPixelSize;
				double markWidth = itsToolBox->SX(arrowHeadSize);
				double markHeight = itsToolBox->SY(arrowHeadSize);
				NFmiPoint scale(markWidth, markHeight);

				double y2 = p2y(pressures[pressures.size()-2]);
				double deltaY = y - y2;
				double vdirRad = ::atan2(deltaY, stepX);
				double vdir1 = vdirRad * 360 / (2. * kPii); // ja siit‰ suunta viimeiseen paikkaan
				vdir1 += 90;
				if(forwardDir)
					vdir1 = ::fmod(vdir1, 360);
				else
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

NFmiMetTime NFmiCrossSectionView::GetCrossSectionTime(const NFmiPoint &theRelativePlace)
{
    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
	bool obsForMode = crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kObsAndFor;
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
		const std::vector<NFmiMetTime> &times = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryTimes();
		int column = GetNearestCrossSectionColumn(theRelativePlace);
		return times[column];
	}
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kTime || obsForMode)
	{
		const NFmiTimeBag usedtimeBag = GetUsedTimeBagForDataCalculations();
		int timeCount = usedtimeBag.GetSize();
		double pos = (theRelativePlace.X() - itsDataViewFrame.Left())/(itsDataViewFrame.Width()) * (timeCount - 1);
		int timeIndex = boost::math::iround(pos); // haetaan l‰hin aika k‰ytetyst‰ timebagista
		NFmiTimeBag times(usedtimeBag); // pit‰‰ tehd‰ kopio
		if(times.SetTime(static_cast<unsigned long>(timeIndex)))
			return times.CurrentTime();
		else
			return times.FirstTime(); // JOKIN MENI OIKEASTI PIELEEN; TƒMƒ ON HƒTƒ PASKA
	}
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kRoute)
	{
		int column = GetNearestCrossSectionColumn(theRelativePlace);
		return crossSectionSystem->RouteTimes().operator [](column);
	}
	else
		return CurrentTime();
}

void NFmiCrossSectionView::DrawOverBitmapThings(NFmiToolBox * /* theGTB */ , const NFmiPoint & /* thePlace */ )
{
}

static float CalcDiffValue(float value1, float value2)
{
	if(value1 == kFloatMissing || value2 == kFloatMissing)
		return kFloatMissing;
	else
		return value1 - value2;
}

std::string NFmiCrossSectionView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	std::string str;
    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
    auto crossMode = crossSectionSystem->GetCrossMode();
	// tooltippi‰ ei piirret‰, jos tooltip mode on pois p‰‰lt‰ (HUOM!, tein funktion nimest‰ huolimatta universaalin on/off s‰‰timen tooltipeille)
    if(crossSectionSystem->ShowTooltipOnCrossSectionView() == false)
		return str;
	else if(itsParamHandlerView && itsParamHandlerView->IsIn(theRelativePoint))
	{
		return itsParamHandlerView->ComposeToolTipText(theRelativePoint);
	}

	try
	{
		NFmiDrawParamList *dpList = itsCtrlViewDocumentInterface->CrossSectionViewDrawParamList(itsViewGridRowNumber);
		if(dpList)
		{
			if(dpList->NumberOfItems() > 0)
			{
				NFmiPoint latlon = GetCrossSectionLatlonPoint(theRelativePoint);
				str += "Lat: ";
				str += CtrlViewUtils::GetLatitudeMinuteStr(latlon.Y(), 1);
				str += " Lon: ";
				str += CtrlViewUtils::GetLongitudeMinuteStr(latlon.X(), 1);
				str += "\n";
				NFmiMetTime aTime = GetCrossSectionTime(theRelativePoint);
				str += aTime.ToStr(::GetDictionaryString("TempViewLegendTimeFormat"), itsCtrlViewDocumentInterface->Language());
				str += "\n";

				float p = static_cast<float>(y2p(theRelativePoint.Y()));
				str += "p: ";
				str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(p, 0);
				str += " mb";

				if(crossMode != NFmiCrossSectionSystem::kTime && crossMode != NFmiCrossSectionSystem::kObsAndFor)
				{ // lis‰t‰‰n et‰isyys [km] alku ja loppu pisteisiin
					str += "\n";
					str += "1. &lt;- "; // < -merkki pit‰‰ escapeta (&lt; :ll‰)
					int column = GetNearestCrossSectionColumn(theRelativePoint);
					if(itsRoutePointsDistToEndPoint[column] != kFloatMissing)
						str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(itsRoutePointsDistToEndPoint[column]/1000., 0);
					else
						str += "??";
					str += "km, ";
					if(itsRoutePointsDistToEndPoint[itsRoutePointsDistToEndPoint.size()-1] != kFloatMissing)
						str += NFmiValueString::GetStringWithMaxDecimalsSmartWay((itsRoutePointsDistToEndPoint[itsRoutePointsDistToEndPoint.size()-1] - itsRoutePointsDistToEndPoint[column])/1000., 0);
					else
						str += "??";
					str += "km -&gt; 2.";  // < -merkki pit‰‰ escapeta (&gt; :ll‰)
				}

				str += "<br><hr color=red><br>";

				str += MakePossibleVirtualTimeTooltipText();

				for(dpList->Reset(); dpList->Next(); )
				{
					itsDrawParam = dpList->Current(); // asetetaan todella k‰ytetty drawParam k‰yttˆˆn
					boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, true, true, fGetCurrentDataFromQ2Server);
					if(info == 0)
						continue; // t‰m‰ on virhe tilanne oikeasti!!!!
					DoTimeInterpolationSettingChecks(info);
					bool showExtraInfo = CtrlView::IsKeyboardKeyDown(VK_CONTROL); // jos CTRL-n‰pp‰in on pohjassa, laitetaan lis‰‰ infoa n‰kyville
					auto paramNameString = CtrlViewUtils::GetParamNameString(itsDrawParam, true, showExtraInfo, true, 0, false, true, true, nullptr);

					float value = GetLevelValue(info, p, latlon, aTime); // tee log(p) interpoloinnit qinfoon ja k‰yt‰ t‰ss‰!!!!
					NFmiExtraMacroParamData extraMacroParamData;

                    if(info->DataType() == NFmiInfoData::kCrossSectionMacroParam)
                    {
                        NFmiIsoLineData isoLineData;
                        CrossSectionTooltipData tooltipData;
                        tooltipData.latlons[0] = latlon;
                        tooltipData.times[0] = aTime;
                        tooltipData.pressures[0] = p;
                        FillCrossSectionMacroParamData(itsIsolineValues, isoLineData, itsPressures, &tooltipData, &extraMacroParamData);
						if(!tooltipData.macroParamErrorMessage.empty())
						{
							str += paramNameString;
							str += ": ";
							str += MakeMacroParamErrorTooltipText(tooltipData.macroParamErrorMessage);
							str += "\n";
							continue;
						}
						else
	                        value = tooltipData.values[0][0];
                    }
					else if(crossMode == NFmiCrossSectionSystem::kObsAndFor)
					{
                        // HUOM! t‰m‰ ei toimi, jos 
                        // 1. On useita parametreja k‰ytˆss‰ (koska vain viimeisen piirretyn layerin arvot j‰‰v‰t k‰ytettyyn matriisiin)
                        // 2. Jos parametri on piilotettu (t‰llˆin matriisiin ei lasketa arvoja ollenkaan)
						NFmiPoint projectedPoint(itsDataViewFrame.Project(theRelativePoint));
						value = itsIsolineValues.InterpolatedValue(projectedPoint, itsParamId);
					}

					if(itsDrawParam->IsParamHidden())
						str += "("; // jos parametri on piilotettu, laita teksti sulkuihin
					paramNameString = DoBoldingParameterNameTooltipText(paramNameString);
					auto fontColor = CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData());
					paramNameString = AddColorTagsToString(paramNameString, fontColor, true);
					str += paramNameString;
					if(itsDrawParam->IsParamHidden())
						str += ")"; // jos parametri on piilotettu, laita teksti sulkuihin
					str += ":	";
					str += "<b><font color=blue>";
					std::string valueStr = (value == kFloatMissing) ? "-" : NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, ((value > 1) ? 1 : 2));
					str += valueStr;
					str += "</font></b>";
					str += GetPossibleMacroParamSymbolText(value, valueStr, extraMacroParamData);
					str += MakeMacroParamDescriptionTooltipText(extraMacroParamData);
					str += CtrlViewUtils::GetArchiveOrigTimeString(itsDrawParam, itsCtrlViewDocumentInterface, info, fGetCurrentDataFromQ2Server, "TempViewLegendTimeFormat");
					str += "\n";
				}
			}
		}
		str += ComposeTrajectoryToolTipText();
	}
	catch(exception & /* e */ )
	{
		// ei onnistunut voi, voi, ei tehd‰ mit‰‰n
//		::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), e.what(), "Ongelma poikkileikkaus piirrossa!", MB_OK);
	}
	return str;
}

// HUOM! T‰m‰ ei tue tuulen meta parametreja, eli mm. tooltippiin ei saada oikeita arvoja niiss‰ tapauksissa.
float NFmiCrossSectionView::GetLevelValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, float P, const NFmiPoint &theLatlon, const NFmiMetTime &theTime, bool doMetaParamCheck)
{
    if(theInfo)
    {
        if(doMetaParamCheck && metaWindParamUsage.ParamNeedsMetaCalculations(itsDrawParam->Param().GetParamIdent()))
            return GetLevelValueForMetaParam(theInfo, P, theLatlon, theTime);
        else
        {
            if(theInfo->PressureDataAvailable() == false && theInfo->HeightDataAvailable())
            {
                float heightValue = static_cast<float>(::CalcHeightAtPressure(P) * 1000.);
                return theInfo->HeightValue(heightValue, theLatlon, theTime);
            }
            else if(theInfo->PressureDataAvailable())
                return theInfo->PressureLevelValue(P, theLatlon, theTime);
        }
    }
    return kFloatMissing;
}

float NFmiCrossSectionView::GetLevelValueForMetaParam(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, float P, const NFmiPoint &theLatlon, const NFmiMetTime &theTime)
{
    NFmiFastInfoUtils::QueryInfoParamStateRestorer fastInfoParamStateRestorer(*theInfo);
    auto paramId = itsDrawParam->Param().GetParamIdent();
    if(metaWindParamUsage.HasWsAndWd())
    {
        theInfo->Param(kFmiWindSpeedMS);
        float WS = GetLevelValue(theInfo, P, theLatlon, theTime, false);
        theInfo->Param(kFmiWindDirection);
        float WD = GetLevelValue(theInfo, P, theLatlon, theTime, false);
        switch(paramId)
        {
        case kFmiWindVectorMS:
            return NFmiFastInfoUtils::CalcWindVectorFromSpeedAndDirection(WS, WD);
        case kFmiWindUMS:
            return NFmiFastInfoUtils::CalcU(WS, WD);
        case kFmiWindVMS:
            return NFmiFastInfoUtils::CalcV(WS, WD);
        default:
            return kFloatMissing;
        }
    }
    else if(metaWindParamUsage.HasWindComponents())
    {
        theInfo->Param(kFmiWindUMS);
        float u = GetLevelValue(theInfo, P, theLatlon, theTime, false);
        theInfo->Param(kFmiWindVMS);
        float v = GetLevelValue(theInfo, P, theLatlon, theTime, false);
        switch(paramId)
        {
        case kFmiWindVectorMS:
            return NFmiFastInfoUtils::CalcWindVectorFromWindComponents(u, v);
        case kFmiWindDirection:
            return NFmiFastInfoUtils::CalcWD(u, v);
        case kFmiWindSpeedMS:
            return NFmiFastInfoUtils::CalcWS(u, v);
        default:
            return kFloatMissing;
        }
    }

    return kFloatMissing;
}

// laskee (interpoloi) kahden l‰himm‰n minor-pisteen avulla hiiren 'osoittaman'
// latlon-pisteen ja palauttaa sen
NFmiPoint NFmiCrossSectionView::GetCrossSectionLatlonPoint(const NFmiPoint &theRelativePlace)
{
	if(itsDataViewFrame.IsInside(theRelativePlace))
	{
        auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
        auto crossSectionMode = itsCtrlViewDocumentInterface->CrossSectionSystem()->GetCrossMode();
		if(crossSectionMode == NFmiCrossSectionSystem::kTime || crossSectionMode == NFmiCrossSectionSystem::kObsAndFor)
		{ // jos ollaan aika-leikkaus moodissa, piste on selv‰, palautetaan 1. main-pointti
			return crossSectionSystem->StartPoint();
		}
		else
		{
			const std::vector<NFmiPoint> &points = GetMinorPoints();
			int mpcount = static_cast<int>(points.size());
			double startX = crossSectionSystem->StartXYPoint().X();
			double width = crossSectionSystem->EndXYPoint().X() - startX;
			double pos = (theRelativePlace.X() - startX)/(width) * (mpcount - 1);
			double factor = pos - static_cast<int>(pos);
			int firstPos = static_cast<int>(pos);
			int secondPos = firstPos + 1;
			if(firstPos >= 0 && secondPos < mpcount)
			{
				NFmiPoint latlon1(points[firstPos]);
				NFmiPoint latlon2(points[secondPos]);
				if(latlon1.X() == kFloatMissing) // ei interpoloida jos puuttuva piste
					return latlon1;
				if(latlon2.X() == kFloatMissing)
					return latlon2;
				// t‰ss‰ joudutaan tekem‰‰n v‰h‰n ik‰v‰ latlon pisteiden interpolointi
				double lon = latlon1.X() * (1. - factor) + latlon2.X() * factor;
				double lat = latlon1.Y() * (1. - factor) + latlon2.Y() * factor;
				NFmiPoint wantedLatlon(lon, lat);
				return wantedLatlon;
			}
			else
				throw runtime_error("NFmiCrossSectionView::GetCrossSectionLatlonPoint-ei onnistunut indeksien lasku.");
		}
	}
	else
		throw runtime_error("NFmiCrossSectionView::GetCrossSectionLatlonPoint-Annettu piste oli rajojen ulkopuolella.");
}

int NFmiCrossSectionView::CalcHorizontalPointCount(void)
{
	// parempi pyyt‰‰ todelliset pisteet ja katsoa kuinka monta niit‰ oli, kuin haluttujen lukum‰‰r‰‰
	int mpcount = static_cast<int>(GetMinorPoints().size());
    auto crossSectionMode = itsCtrlViewDocumentInterface->CrossSectionSystem()->GetCrossMode();
    if(crossSectionMode == NFmiCrossSectionSystem::kTime || crossSectionMode == NFmiCrossSectionSystem::kObsAndFor)
	{ // jos ollaan aika-leikkaus moodissa, pit‰‰ katsoa kuinka monta aikaa leikkauksessa on
		boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, true, true);
		if(info)
		{
			NFmiTimeBag times = GetUsedTimeBagForDataCalculations();
			mpcount = 1 + times.LastTime().DifferenceInMinutes(times.FirstTime()) / 60;
		}
	}
	return mpcount;
}

// jos ep‰onnistuu, palautetaan -1, muuten annetun xy-paikan l‰hin vastaava
// poikkileikkaus pisteiden indeksi alkaen 0:sta.
int NFmiCrossSectionView::GetNearestCrossSectionColumn(const NFmiPoint &thePlace)
{
	if(itsDataViewFrame.IsInside(thePlace))
	{
		int mpcount = CalcHorizontalPointCount();
		double pos = (thePlace.X() - itsDataViewFrame.Left())/(itsDataViewFrame.Width()) * (mpcount - 1);
		return boost::math::iround(pos);
	}
	else
		throw runtime_error("NFmiCrossSectionView::GetNearestCrossSectionColumn-Annettu piste oli rajojen ulkopuolella (x-aks.).");
}

double NFmiCrossSectionView::Column2x(int theColumn)
{
	int mpcount = CalcHorizontalPointCount();
	double factor = static_cast<double>(theColumn) / (mpcount - 1.);
	double pos = itsDataViewFrame.Left() + factor * itsDataViewFrame.Width();
	return pos;
}

// kun uutta dataa on ladattu ja crossSectiondrawParam pit‰‰ p‰ivitt‰‰,
// kutsutaan t‰t‰ funktiota.
void NFmiCrossSectionView::Update(void)
{
	// jos on esim. ladattu viewMacro k‰yttˆˆn, pit‰‰ paine asteikon rajat p‰ivitt‰‰
	NFmiCrossSectionSystem::ExtraRowInfo &extraRowInfo = itsCtrlViewDocumentInterface->CrossSectionSystem()->GetRowInfo(itsViewGridRowNumber - 1);  // itsViewGridRowNumber alkaa 1:st‰, mutta GetRowInfo:n indeksit alkavat 0:sta.
	if(extraRowInfo.itsLowerEndOfPressureAxis != extraRowInfo.itsUpperEndOfPressureAxis) // ei vedet‰ vahingossakaan paineakselia lyttyyn
	{
		if(extraRowInfo.itsLowerEndOfPressureAxis != kFloatMissing)
			itsLowerEndOfPressureAxis = extraRowInfo.itsLowerEndOfPressureAxis;
		if(extraRowInfo.itsUpperEndOfPressureAxis != kFloatMissing)
			itsUpperEndOfPressureAxis = extraRowInfo.itsUpperEndOfPressureAxis;
	}

    itsCtrlViewDocumentInterface->CrossSectionSystem()->CrossSectionViewNeedsUpdate(false); // nollataan flagi kun p‰ivitys suoritettu
}

bool NFmiCrossSectionView::IsToolMasterAvailable(void)
{
	return itsCtrlViewDocumentInterface->IsToolMasterAvailable();
}

// Jostain syyst‰ karttan‰ytˆille transparency piirtoon liittyv‰ bitmap pit‰‰ deletoida (muuten vuotaa),
// mutta poikkileikkausn‰ytˆss‰ sit‰ ei saa tehd‰, muuten kaatuu (ei vuoda, vaikka ei deletoida).
bool NFmiCrossSectionView::DeleteTransparencyBitmap()
{
    return false;
}

bool NFmiCrossSectionView::IsMapViewCase()
{
	return false;
}

void NFmiCrossSectionView::DrawCrossSection(void)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, std::string(__FUNCTION__) + ": starting to draw layer");
	if(fGetCurrentDataFromQ2Server)
		return ; // ei viel‰ q2server tukea poikkileikkausn‰ytˆss‰!!!!

	itsIsolineValues = kFloatMissing; // tyhjennet‰‰n aina ensin data-setti ennen niiden laskua
	fDoCrossSectionDifferenceData = false;
	NFmiIsoLineData isoLineData;
	isoLineData.itsInfo = this->itsInfo;
	isoLineData.itsParam = itsDrawParam->Param();
	isoLineData.itsTime = CurrentTime();
    isoLineData.itsIsolineMinLengthFactor = itsCtrlViewDocumentInterface->ApplicationWinRegistry().IsolineMinLengthFactor();

    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
	int oldVerticalPointCount = crossSectionSystem->VerticalPointCount();
	int usedVerticalPointCount = oldVerticalPointCount;
	if(isoLineData.itsParam.GetParamIdent() == kFmiWindVectorMS)
	{ // tuuli viiri piirtoa varten vertikaali tiheytt‰ pit‰‰ harventaa
		usedVerticalPointCount = (oldVerticalPointCount-1) / 3;
		// tuuli viirit piirret‰‰n harvemmassa, joten haluan alkaa piirt‰‰ niit‰ alempaa, muuten alas j‰‰ niin paljon tyhj‰‰
	}

	itsPressures = MakePressureVector(usedVerticalPointCount, oldVerticalPointCount);

    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, NFmiMetEditorTypes::kFmiNoMask); // k‰yd‰‰n kaikki pisteet l‰pi
	// Kun k‰ytet‰‰n imagine piirtoa,ei dataa talleteta isolinedata-rakenteisiin
	// kuten toolmaster-piirrossa, koska imagine k‰ytt‰‰ erilaista data rakennetta (matriisia)
	if(itsInfo->DataType() == NFmiInfoData::kCrossSectionMacroParam && crossSectionSystem->GetCrossMode() != NFmiCrossSectionSystem::kObsAndFor)
		FillCrossSectionMacroParamData(itsIsolineValues, isoLineData, itsPressures);
	else if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
		FillTrajectoryCrossSectionData(itsIsolineValues, isoLineData, itsPressures);
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kObsAndFor)
		FillObsAndForCrossSectionData(itsIsolineValues, isoLineData, itsPressures);
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kRoute)
		FillRouteCrossSectionData(itsIsolineValues, isoLineData, itsPressures);
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kTime)
		FillTimeCrossSectionData(itsIsolineValues, isoLineData, itsPressures);
	else
        FillCrossSectionData(itsIsolineValues, isoLineData, itsPressures);

	if(!isoLineData.InitIsoLineData(itsIsolineValues))
		return; // Jos data hila (isoLineData.Init() ==> false) oli tyhj‰, pit‰‰ lopettaa

	// HUOM! TƒMƒ ON VIRITYS!!!! -alkaa
	// Tuulensuunta-parametri on saatettu laittaa suuntanuoli asetukseen ja se toimiikiin hyvin karttan‰ytˆll‰. 
	// Mutta minusta olisi h‰m‰‰v‰‰ jos tuuulen suunta piirret‰‰n nuolella poikkileikkausn‰ytˆss‰ ,koska se ei osoittaisi tuulen suuntaa
	// n‰ytˆll‰ vaan maantieteellisesti. WindBarb on eri asia ja se piirret‰‰n kuten kartalla, mutta siihen ollaan totuttu...
	// Lis‰ksi nyt kun valitsin tuulen suunnan n‰ytˆlle, ei tapahdu mit‰‰n, koska piirto ei tue suunta-nuolen piirtoa.
	// ELI JOS paramtri on tuulen suunta ja hilaesitys on nuoli, muuta se isoviiva esitykseksi.
	if(isoLineData.itsParam.GetParamIdent() == kFmiWindDirection && itsDrawParam->GridDataPresentationStyle() == NFmiMetEditorTypes::View::kFmiArrowView)
		itsDrawParam->GridDataPresentationStyle(NFmiMetEditorTypes::View::kFmiIsoLineView);
	// HUOM! TƒMƒ ON VIRITYS!!!! - loppuu

	if(fDoCrossSectionDifferenceData)
		SetUpDifferenceDrawing(itsDrawParam);

    {
		auto doWindVectorDraw = isoLineData.itsParam.GetParamIdent() == kFmiWindVectorMS;
        // Making ToolMaster setup and drawing in one protected zone
        std::lock_guard<std::mutex> toolMasterLock(NFmiIsoLineView::sToolMasterOperationMutex);
        if(doWindVectorDraw || FillIsoLineVisualizationInfo(itsDrawParam, &isoLineData, IsToolMasterAvailable(), false))
        {
            NFmiDataMatrix<NFmiPoint> koordinaatit;
            FillXYMatrix(isoLineData, koordinaatit, itsPressures);
            FillMainPointXYInfo(koordinaatit);

            if(doWindVectorDraw)
                DrawCrosssectionWindVectors(isoLineData, koordinaatit);
            else
            {
				if(!IsToolMasterAvailable())
				{
					Imagine::NFmiDataHints helper(itsIsolineValues);
                    DrawCrosssectionWithImagine(isoLineData, itsIsolineValues, helper, koordinaatit);
				}
                else
                    DrawCrosssectionWithToolMaster(isoLineData);
            }
			if(fDoCrossSectionDifferenceData)
				RestoreUpDifferenceDrawing(itsDrawParam);

             // n‰m‰ pit‰isi siirt‰‰ ulos t‰‰lt‰, ett‰ ne voitaisiin piirt‰‰ koko roskan p‰‰lle yhden kerran
             // HUOM! ei piirret‰ jos windVector ja monta parametria ruudulla, koska windvectorit piirret‰‰n hieman eri paikkoihin ja tulee sekamelska
            if(isoLineData.itsParam.GetParamIdent() != kFmiWindVectorMS || itsCtrlViewDocumentInterface->CrossSectionViewDrawParamList(itsViewGridRowNumber)->NumberOfItems() == 1)
                DrawGridPoints(koordinaatit);
            DrawActivatedMinorPointLine();
        }
    }
}

void NFmiCrossSectionView::FillMainPointXYInfo(NFmiDataMatrix<NFmiPoint> &theCoordinates)
{
	int xCount = static_cast<int>(theCoordinates.NX());
	int yCount = static_cast<int>(theCoordinates.NY());
	if(xCount > 1 && yCount > 0)
	{
        auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
        crossSectionSystem->StartXYPoint(theCoordinates[0][0]);
        crossSectionSystem->MiddleXYPoint(theCoordinates[xCount/2][0]);
        crossSectionSystem->EndXYPoint(theCoordinates[xCount-1][0]);
	}
}

void NFmiCrossSectionView::DrawCrosssectionWithToolMaster(NFmiIsoLineData& theIsoLineData)
{
	NFmiRect relRect(itsDataViewFrame);
	NFmiRect zoomedAreaRect(0,0,1,1);
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
		relRect = itsTrajectoryDataRect;

	if(theIsoLineData.fUseIsoLines)
		itsCrossSectionIsoLineDrawIndex++;
	NFmiPoint grid2PixelRatio(0, 0); // t‰t‰ ei k‰ytet‰ viel‰ toistaiseksi poikkileikkaus n‰ytˆss‰, siksi alustetaan 0:ksi.
	::ToolMasterDraw(itsToolBox->GetDC(), &theIsoLineData, relRect, zoomedAreaRect, grid2PixelRatio, itsCrossSectionIsoLineDrawIndex, GetVisualizationSettings());
}

void NFmiCrossSectionView::DrawCrosssectionWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, Imagine::NFmiDataHints &theHelper, NFmiDataMatrix<NFmiPoint> &theCoordinates)
{
	const NFmiPoint dummyOffSet; // pit‰‰ tehd‰ tyhj‰ offset, koska emoluokassa k‰ytet‰‰n offsetteja, muuta ei t‰‰ll‰
    auto &graphicalInfo = itsCtrlViewDocumentInterface->CrossSectionSystem()->GetGraphicalInfo();

	if(theIsoLineData.itsHatch1.fUseHatch)
		DrawHatchesWithImagine(theIsoLineData, theIsoLineData.itsHatch1, theValues, theCoordinates, theHelper, dummyOffSet);
	if(theIsoLineData.itsHatch2.fUseHatch)
		DrawHatchesWithImagine(theIsoLineData, theIsoLineData.itsHatch2, theValues, theCoordinates, theHelper, dummyOffSet);

	if(theIsoLineData.fUseIsoLines)
	{
		if(!theIsoLineData.fUseCustomIsoLineClasses) // piirret‰‰n tasa v‰liset isoviivat
			DrawSimpleIsoLinesWithImagine(theIsoLineData, theValues, theCoordinates, theHelper, dummyOffSet);
		else
			DrawCustomIsoLinesWithImagine(theIsoLineData, theValues, theCoordinates, theHelper, dummyOffSet);
	}
	else
	{
		if(theIsoLineData.fUseCustomColorContoursClasses) // piirret‰‰n tasa v‰liset isoviivat
			DrawCustomColorContourWithImagine(theIsoLineData, theValues, theCoordinates, theHelper, dummyOffSet);
		else
			DrawSimpleColorContourWithImagine(theIsoLineData, theValues, theCoordinates, theHelper, dummyOffSet);
	}
}

const std::vector<NFmiPoint>& NFmiCrossSectionView::GetMinorPoints(void)
{
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
		return itsTrajectoryLatlonPoints;
	else
		return itsCtrlViewDocumentInterface->CrossSectionSystem()->MinorPoints();
}

void NFmiCrossSectionView::DrawActivatedMinorPointLine(void)
{
    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
	if(crossSectionSystem->GetCrossMode() != NFmiCrossSectionSystem::kTime && crossSectionSystem->IsMinorPointActivated())
	{
		NFmiDrawingEnvironment envi;
		envi.SetFrameColor(NFmiColor(0,0,0)); // laitetaan viiva mustaksi
		envi.SetPenSize(NFmiPoint(1 * itsDrawSizeFactorX, 1 * itsDrawSizeFactorY));
		int index = crossSectionSystem->ActivatedMinorPointIndex();
		int pointCount = static_cast<int>(GetMinorPoints().size());
		double xCoordinate = itsDataViewFrame.Left() + index * itsDataViewFrame.Width() / (pointCount-1);
		NFmiLine line(NFmiPoint(xCoordinate, itsDataViewFrame.Top()), NFmiPoint(xCoordinate, itsDataViewFrame.Bottom()), 0, &envi);
		itsToolBox->Convert(&line);
	}
}

// piirret‰‰n laskettu hilapisteikko n‰kyviin kaiken p‰‰lle
void NFmiCrossSectionView::DrawGridPoints(NFmiDataMatrix<NFmiPoint> &theCoordinates)
{
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.SetFillColor(NFmiColor(1.f,0.f,0.f));
	double xwidth = itsToolBox->SX(FmiRound(3 * itsDrawSizeFactorX));
	double ywidth = itsToolBox->SY(FmiRound(3 * itsDrawSizeFactorY));
	NFmiRect rect(0, 0, xwidth, ywidth);
	for(unsigned int j=0 ; j<theCoordinates.NY() ; j++)
	{
		for(unsigned int i=0 ; i<theCoordinates.NX() ; i++)
		{
			if(j == 0 || i == 0)
			{
				rect.Center(theCoordinates[i][j]);
				NFmiRectangle re(rect, 0, &envi);
				itsToolBox->Convert(&re);
			}
		}
	}
}

// tee vektori, jossa on halutut paineet vektorissa. T‰m‰n avulla lasketaan
// se matriisi, jossa on haluttuihin pisteisiin ja haluttuihin korkeuksiin lasketut
// eri parametrien arvot.
// tehd‰‰n niin ett‰ jaetaan data-laatikko halutun moneen osaan, lasketaan y pisteet
// tasav‰lein ala ja yl‰ reinojen v‰liin, sitten lasketaan y arvot log(p) konversion avulla paine arvoiksi
// usedCount ja normalCount lukujen avulla voidaan laskea tuuliviirien tai symbolien
// harvennettu jaotus ja aloitus muualta kuin alimman normaali rivin kohdalta (koska pinnassa puuttuvaa
// dataa ja harvennuksen takia symbolit alkaisivat vasta korkeammalta kuin voisivat). Siis jos aloitus korkeus
// <= 1000 mb, laita symboli aloitus korkeudeksi hieman isompi, jos aloitus korkeus on korkeammalla, voi
// aloittaa symboli datan jaotuksen kun normaalin datankin jaotuksen.
std::vector<float> NFmiCrossSectionView::MakePressureVector(int usedCount, int normalCount)
{
	float startY = static_cast<float>(itsDataViewFrame.Bottom());
	float endY = static_cast<float>(itsDataViewFrame.Top());
	float normalStep = (startY-endY)/(normalCount-1);
	if(usedCount != normalCount)
	{
		startY -= normalStep;
		endY = endY + (2 * normalStep);
	}

	float step = (startY-endY)/(usedCount-1);
	float y = startY;
	std::vector<float> pressures(usedCount);
	for(int i=0; i<usedCount; i++)
	{
		float p = static_cast<float>(y2p(y));
		pressures[i] = p;
		y -= step; // menn‰‰n pinnasta pienempi‰ paineita kohti (=korkeammalle)
	}
	return pressures;
}

// T‰ytet‰‰n isoviivapiirtoa varten xy-piste matriisi, jossa pisteet sijaitsevat n‰ytˆn suhteellisessa avaruudessa.
// Pisteiden sijainnit lasketaan annetun korkeus vektorin mukaan (y-aks.) ja jaetaan tasav‰leihin
// poikkileikkaus pisteiden muodostaman pistejoukon l‰pi (x-aks.)
void NFmiCrossSectionView::FillXYMatrix(NFmiIsoLineData &theIsoLineData, NFmiDataMatrix<NFmiPoint> &theCoordinates, std::vector<float> &thePressures)
{
	NFmiPoint tmpPoint;
	theCoordinates.Resize(theIsoLineData.itsXNumber, theIsoLineData.itsYNumber);
	if(!itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
		for(int j=0 ; j < theIsoLineData.itsYNumber; j++)
		{
			for(int i=0 ; i < theIsoLineData.itsXNumber; i++)
			{
				double factor = static_cast<double>(i)/(theIsoLineData.itsXNumber-1);
				tmpPoint.X(itsDataViewFrame.Left() + itsDataViewFrame.Width() * factor);
				tmpPoint.Y(p2y(thePressures[j]));
				theCoordinates[i][j] = tmpPoint;
			}
		}
	}
	else // trajektorien kanssa pit‰‰ hieman viilailla, koska trajektorit piirret‰‰n vain siihen kohtaa
		// aikan‰yttˆ‰, mik‰ on valittu. Osa j‰‰ tyhj‰ksi tai ei tule n‰kyviin. Dataa haetaan vain
		// trajektorin reitille ja xy koordinaatit pit‰‰ laskea oikein niille
	{
		const NFmiTimeBag totalTimes = GetUsedTimeBagForDataCalculations();
		NFmiMetTime startTime(totalTimes.FirstTime());
		double timeDiffInMinutes = totalTimes.LastTime().DifferenceInMinutes(startTime);
		const std::vector<NFmiMetTime> &trajectoryTimes = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryTimes();
		if(trajectoryTimes.size() > 1)
		{
			// trajektorin aika-steppi lasketaan kahden 1. ajan perusteella. T‰m‰ siksi ett‰ muodostettu aika-vektorin
			// aika-steppi ei ole sama kuin laskuissa k‰ytetty aika-steppi tai sama kuin aikakontrolli-ikkunan steppi
			int trajektorytimeStepInMinutes = trajectoryTimes[1].DifferenceInMinutes(trajectoryTimes[0]);
			double startX = ::Time2X(trajectoryTimes[0], totalTimes.FirstTime(), timeDiffInMinutes, itsDataViewFrame);
			double stepX = ::TimeStepWidth(timeDiffInMinutes, trajektorytimeStepInMinutes, itsDataViewFrame);

			for(int j=0 ; j < theIsoLineData.itsYNumber; j++)
			{
				for(int i=0 ; i < theIsoLineData.itsXNumber; i++)
				{
					tmpPoint.X(startX + i * stepX);
					tmpPoint.Y(p2y(thePressures[j]));
					theCoordinates[i][j] = tmpPoint;
				}
			}
			itsTrajectoryDataRect = NFmiRect(startX, p2y(thePressures[thePressures.size()-1]), startX + (theIsoLineData.itsXNumber-1) * stepX, p2y(thePressures[0]));
		}
	}
}

boost::shared_ptr<NFmiArea> NFmiCrossSectionView::GetZoomedArea(void)
{
	return itsCtrlViewDocumentInterface->GetMapHandlerInterface(0)->Area(); // tassa haetaan vain p‰‰karttan‰ytˆn area
}

void NFmiCrossSectionView::FillTrajectoryCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures)
{
	const std::vector<NFmiPoint> &points = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryPoints();
	const std::vector<NFmiMetTime> &times = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryTimes();
	FillRouteCrossSectionData(theValues, theIsoLineData, thePressures, points, times);
}

void NFmiCrossSectionView::FillCrossSectionMacroParamData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures, CrossSectionTooltipData *possibleTooltipData, NFmiExtraMacroParamData* possibleExtraMacroParamData)
{
    if(!possibleTooltipData)
    {
        if(theIsoLineData.itsInfo == 0)
            return;

        theIsoLineData.itsInfo->First(); // asetetaan varmuuden vuoksi First:iin
    // laitetaan myˆs t‰m‰ matriisi aluksi puuttuvaksi, ett‰ sit‰ ei virhetilanteissa tarvitse erikseen s‰‰dell‰
        NFmiExtraMacroParamData::AdjustValueMatrixToMissing(theIsoLineData.itsInfo, theValues);
        theIsoLineData.itsInfo->SetValues(theValues); // nollataan infossa ollut data missing-arvoilla, ett‰ saadaan puhdas kentt‰ laskuihin
    }

	auto macroParamSystemPtr = itsCtrlViewDocumentInterface->MacroParamSystem();
	NFmiSmartToolModifier smartToolModifier(itsCtrlViewDocumentInterface->InfoOrganizer());
    try // ensin tulkitaan macro
    {
        smartToolModifier.IncludeDirectory(itsCtrlViewDocumentInterface->SmartToolInfo()->LoadDirectory());

        auto macroParamPtr = macroParamSystemPtr->GetWantedMacro(itsDrawParam->InitFileName());
        if(macroParamPtr)
        {
            smartToolModifier.InitSmartTool(macroParamPtr->MacroText(), true);
        }
        else
            throw runtime_error(string("NFmiCrossSectionView::FillCrossSectionMacroParamData: Error, couldn't find macroParam:") + itsDrawParam->ParameterAbbreviation());
    }
	catch(exception &e)
	{
		std::string errorText = CtrlViewUtils::MakeMacroParamRelatedFinalErrorMessage("Error: Macro Parameter intepretion failed", &e, itsDrawParam, macroParamSystemPtr->RootPath());
		CtrlViewUtils::SetMacroParamErrorMessage(errorText, *itsCtrlViewDocumentInterface, possibleTooltipData ? &possibleTooltipData->macroParamErrorMessage : nullptr);
	}

	try // suoritetaan macro sitten
	{
        if(possibleTooltipData)
        { 
            smartToolModifier.CalcCrossSectionSmartToolValues(possibleTooltipData->values, possibleTooltipData->pressures, possibleTooltipData->latlons, possibleTooltipData->times);
        }
        else
        {
            std::vector<NFmiPoint> latlons = MakeLatlonVector();
            std::vector<NFmiMetTime> times = MakeTimeVector();
            smartToolModifier.CalcCrossSectionSmartToolValues(theValues, thePressures, latlons, times);
        }

		if(possibleExtraMacroParamData)
		{
			*possibleExtraMacroParamData = smartToolModifier.ExtraMacroParamData();
		}
	}
	catch(exception &e)
	{
		std::string errorText = CtrlViewUtils::MakeMacroParamRelatedFinalErrorMessage("Error: MacroParam calculation failed", &e, itsDrawParam, macroParamSystemPtr->RootPath());
		CtrlViewUtils::SetMacroParamErrorMessage(errorText, *itsCtrlViewDocumentInterface, possibleTooltipData ? &possibleTooltipData->macroParamErrorMessage : nullptr);
	}
}

// T‰t‰ k‰ytet‰‰n vain NFmiCrossSectionView::FillCrossSectionMacroParamData -metodissa.
std::vector<NFmiPoint> NFmiCrossSectionView::MakeLatlonVector(void)
{
	std::vector<NFmiPoint> latlons;

    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
		latlons = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryPoints();
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kRoute)
		latlons = crossSectionSystem->MinorPoints();
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kTime)
	{
		NFmiPoint point = crossSectionSystem->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
        size_t wantedPoints = MakeMacroParamTimeModeTimeVector().size();
		for(size_t i=0; i<wantedPoints; i++)
			latlons.push_back(point);
	}
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kObsAndFor)
	{
		NFmiPoint point = crossSectionSystem->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
		int wantedPoints = static_cast<int>(GetUsedTimeBagForDataCalculations().GetSize());
		for(int i=0; i<wantedPoints; i++)
			latlons.push_back(point);
	}
	else
		latlons = crossSectionSystem->MinorPoints();

	return latlons;
}

// T‰t‰ k‰ytet‰‰n vain NFmiCrossSectionView::FillCrossSectionMacroParamData -metodissa.
std::vector<NFmiMetTime> NFmiCrossSectionView::MakeTimeVector(void)
{
    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
    bool obsForMode = crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kObsAndFor;
	std::vector<NFmiMetTime> times;

	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
		times = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryTimes();
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kRoute)
		times = crossSectionSystem->RouteTimes();
	else if(crossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kTime || obsForMode)
        return MakeMacroParamTimeModeTimeVector();
	else
	{
		int wantedTimes = crossSectionSystem->WantedMinorPointCount();
		for(int i=0; i<wantedTimes; i++)
			times.push_back(CurrentTime()); // itsTime ei k‰y, vaan pit‰‰ ottaa aika dokumentista
	}
	return times;
}

#ifdef min
#undef min
#endif
// Lasketaan macroParam laskuja varten erillinen timeVector, jossa on sopiva m‰‰r‰ 
// tasav‰lein olevia aikoja. Koska aika-macroParam laskut ainakin viel‰ ovat niin hitaita,
// yritet‰‰n t‰ss‰ laskea jokin sopiva aika-askel, mill‰ saadaan sopivan v‰h‰n aika-askelia.
std::vector<NFmiMetTime> NFmiCrossSectionView::MakeMacroParamTimeModeTimeVector(void)
{
    const long suitableTimeCount = 35; // pyrimme l‰himm‰ksi t‰t‰ lukua el ilaskemme kuinka monta aika-askelta millekin stepille tulisi ja l‰himm‰ksi t‰t‰ p‰‰ssyt valitaan
    std::vector<long> suitableTimeStepsInMinutes = { 60, 120, 180, 360, 720, 1440 };
    std::vector<long> timeCounts;
    std::vector<long> absDiffsInMinutes;
    NFmiTimeBag actualTimeRange(GetUsedTimeBagForDataCalculations());
    double timeRangeInMinutes = actualTimeRange.LastTime().DifferenceInMinutes(actualTimeRange.FirstTime());

    for(auto step : suitableTimeStepsInMinutes)
    {
        long timeCount = boost::math::iround(timeRangeInMinutes / step);
        timeCounts.push_back(timeCount);
        auto absDiff = std::labs(suitableTimeCount - timeCount);
        absDiffsInMinutes.push_back(absDiff);
    }

    auto minDiffIter = std::min_element(absDiffsInMinutes.begin(), absDiffsInMinutes.end());
    size_t minIndex = minDiffIter - absDiffsInMinutes.begin();
    long usedTimeStep = suitableTimeStepsInMinutes[minIndex];
    long usedTimeCount = timeCounts[minIndex];

    std::vector<NFmiMetTime> times;
    NFmiMetTime aTime = actualTimeRange.FirstTime();
    aTime.SetTimeStep(1);
    for(long i = 0; i < usedTimeCount; i++)
    {
        times.push_back(aTime);
        aTime.ChangeByMinutes(usedTimeStep);
    }

    return times;
}

static bool FindNextTimePosition(vector<int> &theDataFoundTimes, int startPos, int &foundPos)
{
	for(size_t i = startPos+1; i < theDataFoundTimes.size(); i++)
	{
		if(theDataFoundTimes[i] != 0)
		{
			foundPos = static_cast<int>(i);
			return true;
		}
	}
	return false;
}

static void InterpolateSoundingDatasInMatrix(NFmiDataMatrix<float> &theValues, int pos1, int pos2, FmiParameterName parId)
{
	if(pos1 == pos2)
		throw runtime_error("Error in NFmiCrossSectionView - InterpolateSoundingDatasInMatrix : pos1 and pos2 were the same.");
	if(pos2 - pos1 == 1)
		return; // per‰kk‰iset ajat, ei tarvitse interpoloida
	for(double i = pos1+1; i < pos2; i++)
	{
		double factor = (i-pos1)/(pos2 - pos1);
		for(size_t j = 0; j < theValues.NY(); j++)
		{
			if(parId == kFmiWindVectorMS)
				theValues[static_cast<size_t>(i)][j] = static_cast<float>(NFmiInterpolation::WindVector(factor, theValues[pos1][j], theValues[pos2][j]));
			else
				theValues[static_cast<size_t>(i)][j] = static_cast<float>(NFmiInterpolation::Linear(factor, theValues[pos1][j], theValues[pos2][j]));
		}
	}
}

static void InterpolateSoundingDatasInMatrixWithBeforeTimeData(NFmiDataMatrix<float> &theValues, int theCurrentPos, FmiParameterName parId, int theBeforeStartIndex, NFmiDataMatrix<float> &theBeforeStartValues)
{
	for(size_t i = 0; i < static_cast<size_t>(theCurrentPos); i++)
	{
		double factor = static_cast<double>(i-theBeforeStartIndex)/(theCurrentPos - theBeforeStartIndex);
		for(size_t j = 0; j < theValues.NY(); j++)
		{
			if(parId == kFmiWindVectorMS)
				theValues[i][j] = static_cast<float>(NFmiInterpolation::WindVector(factor, theBeforeStartValues[0][j], theValues[theCurrentPos][j]));
			else
				theValues[i][j] = static_cast<float>(NFmiInterpolation::Linear(factor, theBeforeStartValues[0][j], theValues[theCurrentPos][j]));
		}
	}
}

static void InterpolateSoundingDatasInMatrix(NFmiDataMatrix<float> &theValues, vector<int> &theDataFoundTimes, FmiParameterName parId, int theBeforeStartIndex, NFmiDataMatrix<float> &theBeforeStartValues)
{
	// etsit‰‰n sellaiset ajanhetket, miss‰ on dataa. Kutsutaan v‰liin interpolointi t‰ll‰isiin hetkiin
	int currentPos = -1;
	int nextPos = -1;
	if(::FindNextTimePosition(theDataFoundTimes, currentPos, currentPos))
	{
		if(currentPos > 0 && theBeforeStartIndex < 0)
			::InterpolateSoundingDatasInMatrixWithBeforeTimeData(theValues, currentPos, parId, theBeforeStartIndex, theBeforeStartValues);

		for( ; ::FindNextTimePosition(theDataFoundTimes, currentPos, nextPos); )
		{
			::InterpolateSoundingDatasInMatrix(theValues, currentPos, nextPos, parId);
			currentPos = nextPos; // lopuksi positiot p‰ivitet‰‰n
		}
	}
	else
		throw runtime_error("Error in NFmiCrossSectionView - InterpolateSoundingDatasInMatrix : coudn't find any data times and that shouldn't be possible.");
}

static void FillCrossSectionMatrixWithSoundingData(NFmiDataMatrix<float> &theValues, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, std::vector<float> &thePressures, int theTimeIndex)
{
	FmiParameterName parId = static_cast<FmiParameterName>(theDrawParam->Param().GetParamIdent());
	NFmiSoundingData soundingData;
	if(soundingData.FillSoundingData(theInfo, theInfo->Time(), theInfo->OriginTime(), *theInfo->Location()))
	{
		for(size_t i=0; i < thePressures.size(); i++)
			theValues[theTimeIndex][i] = soundingData.GetValueAtPressure(parId, thePressures[i]);
	}
}

static void FillBeforeStartTimeWithCrossSectionValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiTimeBag &theTimes, boost::shared_ptr<NFmiDrawParam> &theDrawParam, std::vector<float> &thePressures, int &theBeforeStartIndex, NFmiDataMatrix<float> &theBeforeStartValues)
{
	if(theInfo->FindNearestTime(theTimes.FirstTime(), kBackward, 360))
	{
		do
		{
			if(NFmiSoundingData::HasRealSoundingData(theInfo))
			{ // dataa lˆytyi asemalta johonkin aikaan, t‰ytet‰‰n vektori silt‰ kohdalta, ja lasketaan aika-indeksi talteen
				theBeforeStartIndex = theInfo->Time().DifferenceInMinutes(theTimes.FirstTime())/theTimes.Resolution();
				::FillCrossSectionMatrixWithSoundingData(theBeforeStartValues, theInfo, theDrawParam, thePressures, 0);
				break;
			}
		} while(theInfo->PreviousTime());
	}
}

static void FillCrossSectionMatrixWithObservedSoundings(NFmiDataMatrix<float> &theValues, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiTimeBag &theTimes, std::vector<float> &thePressures, int &theFirstForecastTimeIndex, std::vector<NFmiMetTime> &theObsForModeFoundObsTimes, const NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage)
{
	bool foundDataYet = false;
	int firstDataIndex = -1;
	theObsForModeFoundObsTimes.clear();
	vector<int> dataFoundTimes(theTimes.GetSize(), 0);
	for(theTimes.Reset(); theTimes.Next(); )
	{
		if(theInfo->Time(theTimes.CurrentTime()))
		{
			if(NFmiSoundingData::HasRealSoundingData(theInfo))
			{ // dataa lˆytyi asemalta johonkin aikaan, t‰ytet‰‰n matriisi silt‰ kohdalta, ja merkit‰‰n viimeksi lˆytynyt aika-indeksi talteen
				FmiParameterName parId = static_cast<FmiParameterName>(theDrawParam->Param().GetParamIdent());
				if(theInfo->Param(parId) || parId == kFmiHumidity || metaWindParamUsage.ParamNeedsMetaCalculations(theDrawParam->Param().GetParamIdent())) // humidity lasketaan vaikka sit‰ ei lˆydy infosta
				{
					theFirstForecastTimeIndex = theTimes.CurrentIndex();
					if(foundDataYet == false)
					{
						firstDataIndex = theFirstForecastTimeIndex;
						foundDataYet = true;
					}
					theObsForModeFoundObsTimes.push_back(theTimes.CurrentTime());
					dataFoundTimes[theFirstForecastTimeIndex] = 1; // asetetaan myˆs taulukkoon ett‰ t‰lle ajalle on lˆytynyt dataa
					::FillCrossSectionMatrixWithSoundingData(theValues, theInfo, theDrawParam, thePressures, theFirstForecastTimeIndex);
				}
			}
		}
	}
	int beforeStartIndex = 0;
	NFmiDataMatrix<float> beforeStartValues(1, thePressures.size(), kFloatMissing);
	if(foundDataYet && firstDataIndex > 0)
	{ // on lˆytynyt luotauksia, mutta 1. timebagin aikaan (= indeksi 0) ei ollut mit‰‰n luotausta.
	  // Nyt pit‰‰ etsi‰ mahd. lˆytyv‰ edellinen luotaus aikaikkunan ulkopuolelta, ett‰
	  // aikaikkunan alkukin saadaan t‰ytetty‰ interpoloimalla.
		::FillBeforeStartTimeWithCrossSectionValues(theInfo, theTimes, theDrawParam, thePressures, beforeStartIndex, beforeStartValues);
	}

	if(theFirstForecastTimeIndex != -1)
	{
		theFirstForecastTimeIndex = theFirstForecastTimeIndex + 1; // ennuste dataa aletaan t‰ytt‰m‰‰n viimeisen lˆydetyn havainto data ajan j‰lkeen
		::InterpolateSoundingDatasInMatrix(theValues, dataFoundTimes, static_cast<FmiParameterName>(theDrawParam->Param().GetParamIdent()), beforeStartIndex, beforeStartValues);
	}
	else
		theFirstForecastTimeIndex = 0;// (theTimes.GetSize() / 2) + 1;  // jos ei lˆytynyt kuitenkaan havainto dataa, ennusteita aletaan t‰ytt‰m‰‰n vasta puolesta v‰list‰, ett‰ k‰ytt‰j‰ huomaa  puutteen
}

int NFmiCrossSectionView::FillObsPartOfTimeCrossSectionData(NFmiDataMatrix<float>& theValues, NFmiIsoLineData& theIsoLineData, std::vector<float>& thePressures)
{
	// ker‰‰ dataa matriisiin siten, ett‰ alhaalla (pinnalla) olevat datat ovat
	// matriisin y-akselin alap‰‰ss‰.
	// x-akseli t‰ytet‰‰n timebagist‰ tulevilla ajoilla
	// 1. Katsotaan siis mik‰ on l‰hinn‰ StartPoint:ia oleva luotaus asema.
	// 2. T‰ytet‰‰n se havainto datalla niin hyvin kuin voidaan.
	// 3. Jatketaan siit‰ mihin havainnot loppuivat ja t‰ytet‰‰n valitulla ennusteella.
	NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
	auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
	NFmiPoint point = crossSectionSystem->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
	theValues.Resize(times.GetSize(), thePressures.size(), kFloatMissing);

	int firstForecastTimeIndex = -1;
	boost::shared_ptr<NFmiFastQueryInfo> soundingInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->GetPrioritizedSoundingInfo(NFmiInfoOrganizer::ParamCheckFlags(true));
	if(soundingInfo && soundingInfo->NearestLocation(NFmiLocation(point), 500 * 1000))
	{
		crossSectionSystem->ObsForModeLocation(*soundingInfo->Location());
		try
		{
			::FillCrossSectionMatrixWithObservedSoundings(theValues, soundingInfo, itsDrawParam, times, thePressures, firstForecastTimeIndex, itsObsForModeFoundObsTimes, metaWindParamUsage);
		}
		catch(...)
		{ // luultavasti haluttua parametria ei lˆytynyt
			firstForecastTimeIndex = 0; //(times.GetSize() / 2) + 1;  // jos ei lˆytynyt havainto dataa, parametria tai asemaa tarpeeksi l‰helt‰, ennusteita aletaan t‰ytt‰m‰‰n vasta puolesta v‰list‰, ett‰ k‰ytt‰j‰ huomaa  puutteen
		}
	}
	else
	{
		crossSectionSystem->ObsForModeLocation(NFmiLocation()); // jos asemaa ei lˆytynyt, laitetaan ns. default location, miss‰ asema id on 0
		firstForecastTimeIndex = 0; //(times.GetSize() / 2) + 1;  // jos ei lˆytynyt havainto dataa, parametria tai asemaa tarpeeksi l‰helt‰, ennusteita aletaan t‰ytt‰m‰‰n vasta puolesta v‰list‰, ett‰ k‰ytt‰j‰ huomaa  puutteen
	}

	return firstForecastTimeIndex;
}

void NFmiCrossSectionView::FillObsAndForCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures)
{ 
	itsFirstForecastTimeIndex = FillObsPartOfTimeCrossSectionData(theValues, theIsoLineData, thePressures);

	// lopuksi t‰ytet‰‰n loppu osa matriisista halutulla ennuste datalla
    auto wantedParamId = itsDrawParam->Param().GetParamIdent();
    if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
        FillTimeCrossSectionDataForMetaWindParam(theValues, theIsoLineData, thePressures, itsFirstForecastTimeIndex, wantedParamId, false);
    else
    {
		NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
		NFmiPoint point = itsCtrlViewDocumentInterface->CrossSectionSystem()->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
		theIsoLineData.itsInfo->TimeCrossSectionValuesLogP(theValues, thePressures, point, times, itsFirstForecastTimeIndex);
    }
}

template<typename GetDataFunction>
bool CalcCrossSectionMetaWindParamMatrix(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiDataMatrix<float> &theValues, std::vector<float> &thePressures, unsigned long wantedParamId, const NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage, GetDataFunction getDataFunction)
{
    if(metaWindParamUsage.HasWsAndWd())
    {
        NFmiDataMatrix<float> wsValues;
        info->Param(kFmiWindSpeedMS);
        getDataFunction(wsValues);
        NFmiDataMatrix<float> wdValues;
        info->Param(kFmiWindDirection);
        getDataFunction(wdValues);
        if(wantedParamId == kFmiWindVectorMS)
        {
            NFmiFastInfoUtils::CalcMatrixWindVectorFromSpeedAndDirection(wsValues, wdValues, theValues);
            return true;
        }
        else if(wantedParamId == kFmiWindUMS)
        {
            NFmiDataMatrix<float> dummyValuesV;
            NFmiFastInfoUtils::CalcMatrixWindComponentsFromSpeedAndDirection(wsValues, wdValues, theValues, dummyValuesV);
            return true;
        }
        else if(wantedParamId == kFmiWindVMS)
        {
            NFmiDataMatrix<float> dummyValuesU;
            NFmiFastInfoUtils::CalcMatrixWindComponentsFromSpeedAndDirection(wsValues, wdValues, dummyValuesU, theValues);
            return true;
        }
    }
    else if(metaWindParamUsage.HasWindComponents())
    {
        NFmiDataMatrix<float> uValues;
        info->Param(kFmiWindUMS);
        getDataFunction(uValues);
        NFmiDataMatrix<float> vValues;
        info->Param(kFmiWindVMS);
        getDataFunction(vValues);
        if(wantedParamId == kFmiWindVectorMS)
        {
            NFmiFastInfoUtils::CalcMatrixWindVectorFromWindComponents(uValues, vValues, theValues);
            return true;
        }
        else if(wantedParamId == kFmiWindSpeedMS)
        {
            NFmiFastInfoUtils::CalcMatrixWsFromWindComponents(uValues, vValues, theValues);
            return true;
        }
        else if(wantedParamId == kFmiWindDirection)
        {
            NFmiFastInfoUtils::CalcMatrixWdFromWindComponents(uValues, vValues, theValues);
            return true;
        }
    }

    std::string warningString = "CrossSection meta wind param calculation logic error with param: ";
    warningString += std::to_string(wantedParamId);
    CatLog::logMessage(warningString, CatLog::Severity::Warning, CatLog::Category::Visualization);
    return false;
}

bool NFmiCrossSectionView::FillRouteCrossSectionDataForMetaWindParam(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures, unsigned long wantedParamId, bool doUserDrawData,
	const std::vector<NFmiPoint>& theLatlonPoints,
	const std::vector<NFmiMetTime>& thePointTimes)
{
    const auto &info = theIsoLineData.itsInfo;
	if(doUserDrawData)
		return ::CalcCrossSectionMetaWindParamMatrix(info, theValues, thePressures, wantedParamId, metaWindParamUsage, [&](NFmiDataMatrix<float>& valuesOut) { valuesOut = NFmiFastQueryInfo::CalcRouteCrossSectionLeveldata(*info, theLatlonPoints, thePointTimes); });
	else
		return ::CalcCrossSectionMetaWindParamMatrix(info, theValues, thePressures, wantedParamId, metaWindParamUsage, [&](NFmiDataMatrix<float> &valuesOut) {info->RouteCrossSectionValuesLogP(valuesOut, thePressures, theLatlonPoints, thePointTimes); });
}

void NFmiCrossSectionView::FillRouteCrossSectionData(NFmiDataMatrix<float>& theValues, NFmiIsoLineData& theIsoLineData, std::vector<float>& thePressures)
{
	const auto& points = itsCtrlViewDocumentInterface->CrossSectionSystem()->MinorPoints();
	const auto& times = itsCtrlViewDocumentInterface->CrossSectionSystem()->RouteTimes();
	FillRouteCrossSectionData(theValues, theIsoLineData, thePressures, points, times);
}

void NFmiCrossSectionView::FillRouteCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures,
	const std::vector<NFmiPoint>& theLatlonPoints,
	const std::vector<NFmiMetTime>& thePointTimes)
{
    auto wantedParamId = itsDrawParam->Param().GetParamIdent();
    if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
        FillRouteCrossSectionDataForMetaWindParam(theValues, theIsoLineData, thePressures, wantedParamId, false, theLatlonPoints, thePointTimes);
    else
    {
        theIsoLineData.itsInfo->RouteCrossSectionValuesLogP(theValues, thePressures, theLatlonPoints, thePointTimes);
    }

	FillRouteCrossSectionUserDrawData(theIsoLineData, theLatlonPoints, thePointTimes);
}

bool NFmiCrossSectionView::FillTimeCrossSectionDataForMetaWindParam(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures, unsigned int theStartTimeIndex, unsigned long wantedParamId, bool doUserDrawData)
{
    auto point = itsCtrlViewDocumentInterface->CrossSectionSystem()->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
    const auto &info = theIsoLineData.itsInfo;
    NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
	if(doUserDrawData)
		return ::CalcCrossSectionMetaWindParamMatrix(info, theValues, thePressures, wantedParamId, metaWindParamUsage, [&](NFmiDataMatrix<float>& valuesOut) { valuesOut = NFmiFastQueryInfo::CalcTimeCrossSectionLeveldata(*info, point, times); });
	else
		return ::CalcCrossSectionMetaWindParamMatrix(info, theValues, thePressures, wantedParamId, metaWindParamUsage, [&](NFmiDataMatrix<float> &valuesOut) {info->TimeCrossSectionValuesLogP(valuesOut, thePressures, point, times, theStartTimeIndex); });
}

void NFmiCrossSectionView::FillTimeCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures)
{ // ker‰‰ dataa matriisiin siten, ett‰ alhaalla (pinnalla) olevat datat ovat
  // matriisin y-akselin alap‰‰ss‰.
    auto wantedParamId = itsDrawParam->Param().GetParamIdent();
    if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
        FillTimeCrossSectionDataForMetaWindParam(theValues, theIsoLineData, thePressures, 0, wantedParamId, false);
    else
    {
        // x-akseli t‰ytet‰‰n timebagist‰ tulevilla ajoilla
        NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
        NFmiPoint point = itsCtrlViewDocumentInterface->CrossSectionSystem()->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
        theIsoLineData.itsInfo->TimeCrossSectionValuesLogP(theValues, thePressures, point, times);
    }

	FillTimeCrossSectionUserDrawData(theIsoLineData);
}

bool NFmiCrossSectionView::FillCrossSectionDataForMetaWindParam(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures, unsigned long wantedParamId, bool doUserDrawData)
{
    const auto &points = itsCtrlViewDocumentInterface->CrossSectionSystem()->MinorPoints();
    const auto &info = theIsoLineData.itsInfo;
	if(doUserDrawData)
		return ::CalcCrossSectionMetaWindParamMatrix(info, theValues, thePressures, wantedParamId, metaWindParamUsage, [&](NFmiDataMatrix<float>& valuesOut) { valuesOut = NFmiFastQueryInfo::CalcCrossSectionLeveldata(*info, points, theIsoLineData.itsTime); });
	else
		return ::CalcCrossSectionMetaWindParamMatrix(info, theValues, thePressures, wantedParamId, metaWindParamUsage, [&](NFmiDataMatrix<float> &valuesOut) {info->CrossSectionValuesLogP(valuesOut, theIsoLineData.itsTime, thePressures, points); });
}

static bool IsContourDrawUsed(const boost::shared_ptr<NFmiDrawParam> &drawParam)
{
	auto drawStyle = drawParam->GridDataPresentationStyle();
	return (drawStyle == NFmiMetEditorTypes::View::kFmiColorContourView) || (drawStyle == NFmiMetEditorTypes::View::kFmiColorContourIsoLineView) || (drawStyle == NFmiMetEditorTypes::View::kFmiQuickColorContourView);
}

void NFmiCrossSectionView::FillCrossSectionData(NFmiDataMatrix<float> &theValues, NFmiIsoLineData &theIsoLineData, std::vector<float> &thePressures)
{ 
	// ker‰‰ dataa matriisiin siten, ett‰ alhaalla (pinnalla) olevat datat ovat
    // matriisin y-akselin alap‰‰ss‰
    auto wantedParamId = itsDrawParam->Param().GetParamIdent();
    if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
        FillCrossSectionDataForMetaWindParam(theValues, theIsoLineData, thePressures, wantedParamId, false);
    else
    {
        const std::vector<NFmiPoint> &points = itsCtrlViewDocumentInterface->CrossSectionSystem()->MinorPoints();
        theIsoLineData.itsInfo->CrossSectionValuesLogP(theValues, theIsoLineData.itsTime, thePressures, points);
    }

    FillCrossSectionUserDrawData(theIsoLineData);
}

// T‰m‰ funktio laskee koordinaatit annetun paine-matriisin arvoista (y-akseli), ja matriisin sarakkeiden indekseist‰ x-koordinaatin.
// Koordinaatit sijoitetaan 0,0 - 1,1 maailmaan. Eli x-coordinaatit menev‰t aina 0:sta (1. sarake eli 0-indeksi) 1:een (viimeinen sarake).
// Y-koordinaatit lasketaan niin, ett‰ itsDataViewFrame:n katsojan kulmasta oleva alareuna on 0 ja yl‰reuna on 1. Koska
// paineet on otettu koko datasta eli kaikki levelit on mukana, saattaa y-koordinaattien arvot menn‰ alle 0:n ja yli 1:en, mutta kyseiset 
// pisteet j‰‰v‰t katsojalta sitten piiloon (mutta niit‰ ei voi j‰tt‰‰ pois, koska isoviivat/contourit jatkuvat piirtoreunojen yli).
NFmiDataMatrix<NFmiPoint> NFmiCrossSectionView::CalcRelativeCoordinatesFromPressureMatrix(const NFmiDataMatrix<float>& pressureValues) const
{
	// N‰iss‰ koordinaatti laskuissa k‰ytet‰‰n 0,0 - 1,1 aluetta (koska itse Toolmaster piirrossakin k‰ytet‰‰n sit‰)
	NFmiRect usedDataViewFrame(0,0,1,1);
	// K‰‰nnet‰‰n y-akseli, jotta koordinaatit menee toolmaster systeemien kanssa oikein p‰in (ei voi tehd‰ suoraan constructor:in kanssa, koska siell‰ pirun min/max tarkasteluja)
	usedDataViewFrame.Top(1);
	usedDataViewFrame.Bottom(0);
	NFmiDataMatrix<NFmiPoint> coordinates(pressureValues.NX(), pressureValues.NY());
	auto matrixSizeX = static_cast<double>(pressureValues.NX());
	for(size_t yIndex = 0; yIndex < pressureValues.NY(); yIndex++)
	{
		for(size_t xIndex = 0; xIndex < matrixSizeX; xIndex++)
		{
			auto xCoordinate = xIndex / (matrixSizeX - 1.);
			auto yCoordinate = p2y(usedDataViewFrame, pressureValues[xIndex][yIndex]);
			coordinates[xIndex][yIndex] = NFmiPoint(xCoordinate, yCoordinate);
		}
	}
	return coordinates;
}

static NFmiDataMatrix<float> MakePressureLevelBasedPressureMatrix(size_t xSize, NFmiFastQueryInfo &usedInfo)
{
	NFmiDataMatrix<float> pressureValues(xSize, usedInfo.SizeLevels(), kFloatMissing);
	const auto& pressureLevelValues = usedInfo.PressureLevelDataPressures();
	std::vector<float> pressureLevelValuesBS(pressureLevelValues.begin(), pressureLevelValues.end());
	for(size_t pressureColumnIndex = 0; pressureColumnIndex < pressureValues.NX(); pressureColumnIndex++)
	{
		auto& pressureColumn = pressureValues[pressureColumnIndex];
		pressureColumn = pressureLevelValuesBS;
	}

	if((usedInfo.HeightDataAvailable() && !usedInfo.HeightParamIsRising()) ||
		((usedInfo.PressureDataAvailable()) && usedInfo.PressureParamIsRising()))
	{
		// paineet pit‰‰ k‰‰nt‰‰, koska ne k‰‰nnet‰‰n myˆs datan haussa
		for(auto& pressureVector : pressureValues)
		{
			std::reverse(pressureVector.begin(), pressureVector.end());
		}
	}
	return pressureValues;
}

static std::vector<float> MakeHeightLevelBasedVector(NFmiFastQueryInfo& usedInfo)
{
	std::vector<float> heights;
	for(usedInfo.ResetLevel(); usedInfo.NextLevel(); )
	{
		heights.push_back(usedInfo.Level()->LevelValue());
	}
	if(!usedInfo.HeightParamIsRising())
		std::reverse(heights.begin(), heights.end());
	return heights;
}

static NFmiDataMatrix<float> ConvertMetricHeightsToPressureValues(const NFmiDataMatrix<float>& heightValues)
{
	NFmiDataMatrix<float> pressureValues = heightValues;
	for(size_t yIndex = 0; yIndex < pressureValues.NY(); yIndex++)
	{
		for(size_t xIndex = 0; xIndex < pressureValues.NX(); xIndex++)
		{
			// Annetut metri arvot pit‰‰ muuttaa kilometreiksi ennen konversiota
			pressureValues[xIndex][yIndex] = static_cast<float>(CalcPressureAtHeight(heightValues[xIndex][yIndex] / 1000.));
		}
	}
	return pressureValues;
}

static NFmiDataMatrix<float> MakeHeightLevelBasedPressureMatrix(size_t xSize, NFmiFastQueryInfo& usedInfo)
{
	NFmiDataMatrix<float> heightValues(xSize, usedInfo.SizeLevels(), kFloatMissing);
	auto heightLevelValues = ::MakeHeightLevelBasedVector(usedInfo);
	for(size_t heightColumnIndex = 0; heightColumnIndex < heightValues.NX(); heightColumnIndex++)
	{
		auto& heightColumn = heightValues[heightColumnIndex];
		heightColumn = heightLevelValues;
	}
	return ::ConvertMetricHeightsToPressureValues(heightValues);
}

static NFmiDataMatrix<float> MakeCrossSectionUserDrawPressureData(NFmiIsoLineData& theIsoLineData, const std::vector<NFmiPoint> & latlonPoints)
{
	auto& usedInfo = *theIsoLineData.itsInfo;
	NFmiFastInfoUtils::QueryInfoParamStateRestorer queryInfoParamStateRestorer(usedInfo);
	if(usedInfo.Param(kFmiPressure))
	{
		return NFmiFastQueryInfo::CalcCrossSectionLeveldata(usedInfo, latlonPoints, theIsoLineData.itsTime);
	}
	else if(usedInfo.PressureLevelDataAvailable())
	{
		return ::MakePressureLevelBasedPressureMatrix(latlonPoints.size(), usedInfo);
	}
	else if(usedInfo.Param(kFmiGeopHeight) || usedInfo.Param(kFmiGeomHeight))
	{
		return ::ConvertMetricHeightsToPressureValues(NFmiFastQueryInfo::CalcCrossSectionLeveldata(usedInfo, latlonPoints, theIsoLineData.itsTime));
	}
	else if(usedInfo.HeightLevelDataAvailable())
	{
		return ::MakeHeightLevelBasedPressureMatrix(latlonPoints.size(), usedInfo);
	}
	else
		return NFmiDataMatrix<float>();
}

static NFmiDataMatrix<float> MakeTimeCrossSectionUserDrawPressureData(NFmiIsoLineData& theIsoLineData, const NFmiPoint& latlon, NFmiTimeBag &times)
{
	auto& usedInfo = *theIsoLineData.itsInfo;
	NFmiFastInfoUtils::QueryInfoParamStateRestorer queryInfoParamStateRestorer(usedInfo);
	if(usedInfo.Param(kFmiPressure))
	{
		return NFmiFastQueryInfo::CalcTimeCrossSectionLeveldata(usedInfo, latlon, times);
	}
	else if(usedInfo.PressureLevelDataAvailable())
	{
		return ::MakePressureLevelBasedPressureMatrix(times.GetSize(), usedInfo);
	}
	else if(usedInfo.Param(kFmiGeopHeight) || usedInfo.Param(kFmiGeomHeight))
	{
		return ::ConvertMetricHeightsToPressureValues(NFmiFastQueryInfo::CalcTimeCrossSectionLeveldata(usedInfo, latlon, times));
	}
	else if(usedInfo.HeightLevelDataAvailable())
	{
		return ::MakeHeightLevelBasedPressureMatrix(times.GetSize(), usedInfo);
	}
	else
		return NFmiDataMatrix<float>();
}

static NFmiDataMatrix<float> MakeRouteCrossSectionUserDrawPressureData(NFmiIsoLineData& theIsoLineData, const std::vector<NFmiPoint> &latlons, const std::vector<NFmiMetTime> &times)
{
	auto& usedInfo = *theIsoLineData.itsInfo;
	NFmiFastInfoUtils::QueryInfoParamStateRestorer queryInfoParamStateRestorer(usedInfo);
	if(usedInfo.Param(kFmiPressure))
	{
		return NFmiFastQueryInfo::CalcRouteCrossSectionLeveldata(usedInfo, latlons, times);
	}
	else if(usedInfo.PressureLevelDataAvailable())
	{
		return ::MakePressureLevelBasedPressureMatrix(times.size(), usedInfo);
	}
	else if(usedInfo.Param(kFmiGeopHeight) || usedInfo.Param(kFmiGeomHeight))
	{
		return ::ConvertMetricHeightsToPressureValues(NFmiFastQueryInfo::CalcRouteCrossSectionLeveldata(usedInfo, latlons, times));
	}
	else if(usedInfo.HeightLevelDataAvailable())
	{
		return ::MakeHeightLevelBasedPressureMatrix(times.size(), usedInfo);
	}
	else
		return NFmiDataMatrix<float>();
}

bool NFmiCrossSectionView::IsUserDrawDataNeeded(NFmiFastQueryInfo& usedInfo)
{
	return ::IsContourDrawUsed(itsDrawParam) && (usedInfo.PressureDataAvailable() || usedInfo.HeightDataAvailable());
}

NFmiDataMatrix<float> NFmiCrossSectionView::MakeCrossSectionUserDrawValueData(NFmiIsoLineData& theIsoLineData)
{
	NFmiDataMatrix<float> values;
	auto wantedParamId = itsDrawParam->Param().GetParamIdent();
	if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
	{
		std::vector<float> fakePressures;
		FillCrossSectionDataForMetaWindParam(values, theIsoLineData, fakePressures, wantedParamId, true);
	}
	else
	{
		const auto& latlonPoints = itsCtrlViewDocumentInterface->CrossSectionSystem()->MinorPoints();
		values = NFmiFastQueryInfo::CalcCrossSectionLeveldata(*theIsoLineData.itsInfo, latlonPoints, theIsoLineData.itsTime);
	}
	return values;
}

NFmiDataMatrix<float> NFmiCrossSectionView::MakeTimeCrossSectionUserDrawValueData(NFmiIsoLineData& theIsoLineData)
{
	NFmiDataMatrix<float> values;
	auto wantedParamId = itsDrawParam->Param().GetParamIdent();
	if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
	{
		std::vector<float> fakePressures;
		FillTimeCrossSectionDataForMetaWindParam(values, theIsoLineData, fakePressures, 0, wantedParamId, true);
	}
	else
	{
		auto point = itsCtrlViewDocumentInterface->CrossSectionSystem()->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
		NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
		values = NFmiFastQueryInfo::CalcTimeCrossSectionLeveldata(*theIsoLineData.itsInfo, point, times);
	}
	return values;
}

NFmiDataMatrix<float> NFmiCrossSectionView::MakeRouteCrossSectionUserDrawValueData(NFmiIsoLineData& theIsoLineData,
	const std::vector<NFmiPoint>& theLatlonPoints,
	const std::vector<NFmiMetTime>& thePointTimes)
{
	NFmiDataMatrix<float> values;
	auto wantedParamId = itsDrawParam->Param().GetParamIdent();
	if(metaWindParamUsage.ParamNeedsMetaCalculations(wantedParamId))
	{
		std::vector<float> fakePressures;
		FillRouteCrossSectionDataForMetaWindParam(values, theIsoLineData, fakePressures, wantedParamId, true, theLatlonPoints, thePointTimes);
	}
	else
	{
		values = NFmiFastQueryInfo::CalcRouteCrossSectionLeveldata(*theIsoLineData.itsInfo, theLatlonPoints, thePointTimes);
	}
	return values;
}

void NFmiCrossSectionView::FillCrossSectionUserDrawData(NFmiIsoLineData& theIsoLineData)
{
	if(IsUserDrawDataNeeded(*theIsoLineData.itsInfo))
	{
		auto values = MakeCrossSectionUserDrawValueData(theIsoLineData);
		const auto& latlonPoints = itsCtrlViewDocumentInterface->CrossSectionSystem()->MinorPoints();
		auto pressureValues = ::MakeCrossSectionUserDrawPressureData(theIsoLineData, latlonPoints);
		auto coordinates = CalcRelativeCoordinatesFromPressureMatrix(pressureValues);
		theIsoLineData.InitContourUserDrawData(values, coordinates);
	}
}

void NFmiCrossSectionView::FillTimeCrossSectionUserDrawData(NFmiIsoLineData& theIsoLineData)
{
	if(IsUserDrawDataNeeded(*theIsoLineData.itsInfo))
	{
		auto values = MakeTimeCrossSectionUserDrawValueData(theIsoLineData);
		auto point = itsCtrlViewDocumentInterface->CrossSectionSystem()->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
		NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
		auto pressureValues = ::MakeTimeCrossSectionUserDrawPressureData(theIsoLineData, point, times);
		auto coordinates = CalcRelativeCoordinatesFromPressureMatrix(pressureValues);
		theIsoLineData.InitContourUserDrawData(values, coordinates);
	}
}

void NFmiCrossSectionView::FillRouteCrossSectionUserDrawData(NFmiIsoLineData& theIsoLineData,
	const std::vector<NFmiPoint>& theLatlonPoints,
	const std::vector<NFmiMetTime>& thePointTimes)
{
	if(IsUserDrawDataNeeded(*theIsoLineData.itsInfo))
	{
		auto values = MakeRouteCrossSectionUserDrawValueData(theIsoLineData, theLatlonPoints, thePointTimes);
		auto pressureValues = ::MakeRouteCrossSectionUserDrawPressureData(theIsoLineData, theLatlonPoints, thePointTimes);
		auto coordinates = CalcRelativeCoordinatesFromPressureMatrix(pressureValues);
		theIsoLineData.InitContourUserDrawData(values, coordinates);
	}
}

static bool DoTimeBagSpacingOut(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	if(theDrawParam && theDrawParam->Param().GetParamIdent() == kFmiWindVectorMS)
	{
        auto crossMode = theCtrlViewDocumentInterface->CrossSectionSystem()->CrossSectionMode();
		if(crossMode == NFmiCrossSectionSystem::kObsAndFor || crossMode == NFmiCrossSectionSystem::kTime)
			return true;
	}

	return false;
}

// t‰ll‰ haetaan k‰ytett‰v‰‰ timabagi‰ kun lasketaan eri moodeissa dataa.
// T‰m‰ on tehty sit‰ varten ett‰ kun hiirell‰ raahataan aikakontrolli ikkunaa, tehd‰‰n 
// aikaikkunan liu'utuksen takia timebagi, jossa resoluutio 5 minuuttia.
// T‰st‰ seuraa ett‰ muuten laskettaisiin ja piirrett‰isiin data t‰ll‰ 5 minuutin askeleella.
// Eli jos ollaan aika/obsAndFor -moodissa ja ollaan raahaamassa aikakontrolli-ikkunaa, tehd‰‰n
// t‰llˆin timebagi, jossa onkin yhden tunnin aika-askel (=> v‰hemm‰n turhaa tyˆt‰ ja nopeampi ruudun p‰ivitys)
NFmiTimeBag NFmiCrossSectionView::GetUsedTimeBagForDataCalculations(void)
{
    auto crossMode = itsCtrlViewDocumentInterface->CrossSectionSystem()->CrossSectionMode();
    NFmiTimeBag usedTimeBag = itsCtrlViewDocumentInterface->CrossSectionSystem()->CrossSectionTimeControlTimeBag();
	if(crossMode == NFmiCrossSectionSystem::kObsAndFor || crossMode == NFmiCrossSectionSystem::kTime)
	{
		// jos p‰‰karttan‰ytˆn (sielt‰ otetaan perus aika-askel) aika-askel on suurempi kuin nykyisen k‰ytˆss‰ ollen timebagin,
		// oletetaan ett‰ ollaan raahaamassa poikkileikkausn‰ytˆn aika-ikkunaa
		if(::round(itsCtrlViewDocumentInterface->TimeControlTimeStep(0)*60.) > usedTimeBag.Resolution())
			usedTimeBag = CtrlViewUtils::GetAdjustedTimeBag(usedTimeBag.FirstTime(), usedTimeBag.LastTime(), 60);
	}

	if(itsDrawParam && itsDrawParam->Param().GetParamIdent() == kFmiWindVectorMS)
	{ // jos on tuuliviiri piirto, lasketaan sopiva aikaharvennus datalle
		usedTimeBag = CalcHelperLineTimeBag();
	}

	return usedTimeBag;
}


bool NFmiCrossSectionView::LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
    // mouse captured pit‰‰ hanskata, vaikka hiiri olisi itsParamHandlerView -ikkunan ulkona
    if(ShowParamHandlerView() && itsParamHandlerView->IsMouseCaptured())
        return itsParamHandlerView->LeftButtonUp(thePlace, theKey);

	if(!itsRect.IsInside(thePlace))
		return false;

	itsCtrlViewDocumentInterface->SetLastActiveDescTopAndViewRow(itsMapViewDescTopIndex, 
		GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));

	// ensin pit‰‰ handlata parametrin lis‰ys param boxista jos hiiren oikea klikattu
	if(ShowParamHandlerView() && itsParamHandlerView->IsIn(thePlace))
		return itsParamHandlerView->LeftButtonUp(thePlace, theKey);

	auto ctrlKeyDown = theKey & kCtrlKey;
	if(itsPressureScaleFrame.IsInside(thePlace))
	{
		if(itsPressureScaleFrame.Center().Y() > thePlace.Y())
			return ChangePressureScale(kDown, true, ctrlKeyDown); // true liikuttaa asteikon yl‰p‰‰t‰ ja false alap‰‰t‰
		else
			return ChangePressureScale(kDown, false, ctrlKeyDown); // true liikuttaa asteikon yl‰p‰‰t‰ ja false alap‰‰t‰
	}
	if(ctrlKeyDown)
	{
		return true;
	}
	return true;
}

// theDir joko arvo kUp siirt‰‰ asteikkoa halutulla m‰‰r‰ll‰ ylˆsp‰in muut arvot alasp‰in
// fChangeUpperAxis jos true, siirt‰‰ asteikon yl‰p‰‰t‰, jos false, muuttaa asteikon alap‰‰t‰.
bool NFmiCrossSectionView::ChangePressureScale(FmiDirection theDir, bool fChangeUpperAxis, bool ctrlKeyDown)
{
	double minPressure = 10;
    double startValue = itsLowerEndOfPressureAxis;
    double endValue = itsUpperEndOfPressureAxis;
    double absChangeValue = ctrlKeyDown ? 10 : 50;
	if(startValue - endValue <= 2.f * absChangeValue)
	{
		// kun asteikko on vedetty tarpeeksi lyhyeksi, muutetaan muutos arvoa pienemm‰ksi, ett‰ voidaan tehd‰ tarkempia s‰‰tˆj‰
		absChangeValue = ctrlKeyDown ? 1 : 5;
	}
    double changeValue = theDir == kUp ? -absChangeValue : absChangeValue;
	if(fChangeUpperAxis)
		endValue += changeValue;
	else
		startValue += changeValue;
	if(startValue - endValue <= absChangeValue)
		return false; // ei tehd‰ mit‰‰n, jos akselista tulisi liian kapea tai jopa negatiivinen
	if(endValue < minPressure)
		endValue = minPressure; // ei voida menn‰ 0 tasolle paineessa, asetetaan maksi paineeksi vaikka 10 mb taso.
	if(endValue > absChangeValue && fmod(endValue, absChangeValue) != 0) // t‰m‰ palauttaa loppu arvon taas tasa jakov‰leihin, mik‰ saattaa menn‰ sekaisin kun ollaan oltu yl‰ rajoilla
		endValue = round(endValue / absChangeValue) * absChangeValue;
	itsLowerEndOfPressureAxis = startValue;
	itsUpperEndOfPressureAxis = endValue;
	StorePressureScaleLimits();
	return true;
}

bool NFmiCrossSectionView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(!itsRect.IsInside(thePlace))
		return false;

	itsCtrlViewDocumentInterface->SetLastActiveDescTopAndViewRow(itsMapViewDescTopIndex,
		GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));

	// ensin pit‰‰ handlata parametrin lis‰ys param boxista jos hiiren oikea klikattu
	if(ShowParamHandlerView() && itsParamHandlerView->IsIn(thePlace))
		return itsParamHandlerView->RightButtonUp(thePlace, theKey);

	if(itsPressureScaleFrame.IsInside(thePlace))
	{
		auto ctrlKeyDown = theKey & kCtrlKey;
		if(itsPressureScaleFrame.Center().Y() > thePlace.Y())
			return ChangePressureScale(kUp, true, ctrlKeyDown); // true liikuttaa asteikon yl‰p‰‰t‰ ja false alap‰‰t‰
		else
			return ChangePressureScale(kUp, false, ctrlKeyDown); // true liikuttaa asteikon yl‰p‰‰t‰ ja false alap‰‰t‰
	}

	itsCtrlViewDocumentInterface->CreateCrossSectionViewPopup(itsViewGridRowNumber);
	return true;
}

bool NFmiCrossSectionView::LeftDoubleClick(const NFmiPoint& thePlace, unsigned long theKey)
{
	// ensin pit‰‰ handlata parametrin lis‰ys param boxista jos hiiren oikea klikattu
	if(ShowParamHandlerView() && itsParamHandlerView->IsIn(thePlace))
		return itsParamHandlerView->LeftDoubleClick(thePlace, theKey);
	return false;
}

// Funktio, joka palauttaa annetusta drawparamista stringin ,joka kertoo:
// Param: id (name) Prod: id (name) Level: id (name, value)
std::string NFmiCrossSectionView::GetSelectedParamInfoString(NFmiFastQueryInfo *theInfo)
{
	const NFmiDataIdent &dataIdent = theInfo->Param();

	string str("Param:");
	str += NFmiStringTools::Convert<unsigned long>(dataIdent.GetParamIdent());
	str += " ('";
	str += dataIdent.GetParamName();
	str += "')";

	str += ", Producer: ";
	str += NFmiStringTools::Convert<unsigned long>(dataIdent.GetProducer()->GetIdent());
	str += " ('";
	str += dataIdent.GetProducer()->GetName();
	str += "')";
	if(theInfo->Level()->LevelType() == kFmiHybridLevel)
		str += " (hybr.)";
	else
		str += " (press.)";
	return str;
}

//--------------------------------------------------------
// CreateHeightView
//--------------------------------------------------------
void NFmiCrossSectionView::CalculateViewRects(void)
{
	itsPressureScaleFrame = CalcPressureScaleRect();
	itsDataViewFrame = CalcDataViewRect();
}

NFmiRect NFmiCrossSectionView::CalcPressureScaleRect(void)
{
	NFmiRect axisRect(GetFrame());
	double emptySpace = axisRect.Height() / 70.;
	axisRect.Top(axisRect.Top() + emptySpace);
	axisRect.Bottom(axisRect.Bottom() - emptySpace * 0.5);
	axisRect.Right(axisRect.Left() + itsToolBox->SX(FmiRound(itsPressureScaleFontSize.X() * itsDrawSizeFactorX)) * 4 * 0.62); // 4 on neli merkkinen paine arvo esim 1000
	return axisRect;
}

// Lasketaan itse data n‰ytˆn koko korkeus akselin avulla.
// Oletus: itsPressureScaleFrame on jo laskettu.
NFmiRect NFmiCrossSectionView::CalcDataViewRect(void)
{
	NFmiRect frame(GetFrame());
	NFmiRect resultRect(itsPressureScaleFrame.Right(), itsPressureScaleFrame.Top(), frame.Right() - frame.Width() / 50., itsPressureScaleFrame.Bottom());
	return resultRect;
}

double NFmiCrossSectionView::CalcDataRectPressureScaleRatio(void)
{
	NFmiRect pRect = CalcPressureScaleRect();
	NFmiRect dRect = CalcDataViewRect();
	return dRect.Width() / pRect.Width();
}

//--------------------------------------------------------
// DrawHeightView
//--------------------------------------------------------
void NFmiCrossSectionView::DrawPressureScale(void)
{
	// t‰h‰n tulee keinotekoinen datan piirto clippaus, koska en saa
	// trajektori datan toolmaster piirtoa nyt clippaamaan oikein
	// eli piirr‰n t‰h‰n reuna palkit ennen kuin piirr‰n paine asteikon
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.DisableFrame();
	itsDrawingEnvironment.SetFillColor(NFmiColor(1.f,1.f,1.f));
	NFmiRect marginRectLeft(itsRect.Left(), itsDataViewFrame.Top(), itsDataViewFrame.Left(), itsDataViewFrame.Bottom());
	NFmiRectangle rec1(marginRectLeft, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&rec1);

	itsDrawingEnvironment.EnableFrame();

	NFmiDrawingEnvironment envi;
	NFmiPoint usedFontSize(itsPressureScaleFontSize.X() * itsDrawSizeFactorX, itsPressureScaleFontSize.Y() * itsDrawSizeFactorY);
	envi.SetFontSize(usedFontSize);
	envi.SetPenSize(NFmiPoint(1 * itsDrawSizeFactorX, 1 * itsDrawSizeFactorY));
	FmiDirection oldAlignment = itsToolBox->GetTextAlignment();
	itsToolBox->SetTextAlignment(kRight);

	std::vector<double> pValues;
	pValues.push_back(1000);
	pValues.push_back(925);
	pValues.push_back(850);
	pValues.push_back(700);
	pValues.push_back(500);
	pValues.push_back(300);
	pValues.push_back(150);
	pValues.push_back(100);
	pValues.push_back(50);
	pValues.push_back(20);
	pValues.push_back(10);
	std::vector<double>::const_iterator endIt = pValues.end();
	double y = 0;
	double tickMarkLength = itsToolBox->SX(FmiRound(4 * itsDrawSizeFactorX));
	double moveLabelY = itsToolBox->SY(static_cast<long>(usedFontSize.Y()/2));
	for (std::vector<double>::const_iterator it = pValues.begin(); it != endIt;  ++it)
	{
		double pressure = *it;
		if(pressure <= itsLowerEndOfPressureAxis && pressure >= itsUpperEndOfPressureAxis)
		{
			// tick markin piirto
			envi.SetFillPattern(FMI_SOLID);
			y = p2y(pressure);
			NFmiPoint p1(itsPressureScaleFrame.Right(), y);
			NFmiPoint p2(p1);
			p2.X(p2.X() - tickMarkLength);
			NFmiLine line1(p1, p2, 0, &envi);
			itsToolBox->Convert(&line1);

			// label tekstin piirto
			NFmiPoint p3(p2);
			p3.Y(p3.Y() - moveLabelY);
			NFmiString str1(NFmiStringTools::Convert<double>(pressure));
			NFmiText txt1(p3, str1, false, 0, &envi);
			itsToolBox->Convert(&txt1);

			// apuviivaston piirto ruudun poikki
			envi.SetFillPattern(FMI_DASHDOT);
			NFmiPoint p4(itsDataViewFrame.Left(), y);
			NFmiPoint p5(itsDataViewFrame.Right(), y);
			::DrawLineWithToolBox(p4, p5, &envi, itsToolBox, false, true);
		}
	}
	itsToolBox->SetTextAlignment(oldAlignment);
}

static int CalcSkipFactorX(int xSize, double theViewWidthInMM)
{
	double widthPerSymbolFactor = theViewWidthInMM / xSize;
	double skipFactor = 30. / widthPerSymbolFactor;
    return FmiMax(1, boost::math::iround(skipFactor));
}

static int CalcUsedWindVectorXSize(const NFmiTimeBag &doTimeBagSpacing, bool fDoTimeBagSpacing, int theSkipFactor, int theOrigXSize)
{
	if(fDoTimeBagSpacing)
		return doTimeBagSpacing.GetSize();
	else
	{
        return boost::math::iround(theOrigXSize / static_cast<double>(theSkipFactor)) + 1;
	}
}

static double Time2XLocation(const NFmiMetTime &theTime, const NFmiMetTime &theStartTime, long theEnd2StartDiffInMinutes, const NFmiRect &theFrame)
{
	double timeDiff = theTime.DifferenceInMinutes(theStartTime);
	double ratio = timeDiff/theEnd2StartDiffInMinutes;
	double xPos = theFrame.Left() + theFrame.Width() * ratio;
	return xPos;
}

void NFmiCrossSectionView::DrawCrosssectionWindVectors(NFmiIsoLineData& theIsoLineData, const NFmiDataMatrix<NFmiPoint> &theXYCoordinates)
{
	// Tuulivektoreita pit‰‰ harventaa. T‰ss‰ on kaksi eri tapaa tehd‰ se.
	// 1. Jos ollaan normaali poikkileikkauksessa, lasketaan t‰ss‰ sopiva harvennus, esim. piirr‰ joka toinen tai joka kolmas vektori.
	// 2. Jos ollaan aika tai vastaavassa tilassa, piirret‰‰n sopivan aikaharvennus timebagin avulla piirrett‰vien vektorien x-paikat.
	itsDrawingEnvironment.SetFrameColor(itsDrawParam->FrameColor());
	itsDrawingEnvironment.SetFillColor(itsDrawParam->FillColor());
	int xSize = static_cast<int>(theIsoLineData.itsIsolineData.NX());
	int skipFactor = ::CalcSkipFactorX(xSize, itsCtrlViewDocumentInterface->CrossSectionSystem()->GetGraphicalInfo().itsViewWidthInMM);
	bool doTimeBagSpacing = ::DoTimeBagSpacingOut(itsCtrlViewDocumentInterface, itsDrawParam);
	if(doTimeBagSpacing)
		skipFactor = 1;
	NFmiTimeBag usedTimeBag = CalcHelperLineTimeBag();
	NFmiMetTime startTime(NFmiMetTime::gMissingTime);
	NFmiMetTime endTime(NFmiMetTime::gMissingTime);
	GetStartAndEndTimes(startTime, endTime);
	long diffInMinutes = endTime.DifferenceInMinutes(startTime);

	int usedXSize = ::CalcUsedWindVectorXSize(usedTimeBag, doTimeBagSpacing, skipFactor, xSize);
	int ySize = static_cast<int>(theIsoLineData.itsIsolineData.NY());

	// lasketaan symboli kokoa
	double symbSizeX = 0.3 * itsDataViewFrame.Width()/::pow(static_cast<double>(usedXSize), 0.6);
	double symbSizeY = 0.3 * itsDataViewFrame.Height()/::pow(static_cast<double>(ySize),0.6);
	if(symbSizeX > symbSizeY) // tasoitetaan jos x/y koot poikkeaa paljon toisistaan
		symbSizeX = (symbSizeY + symbSizeX)/2.;
	else
		symbSizeY = (symbSizeY + symbSizeX)/2.;
	double usedSize = ::pow(symbSizeX*symbSizeX + symbSizeY*symbSizeY, 0.55);
	NFmiRect symbolRect(0, 0, usedSize, usedSize);

	for(int j = 0; j < ySize; j++)
	{
		usedTimeBag.Reset();
		for(int i = 0; i < xSize; i++)
		{
			if(i % skipFactor == 0)
			{
				if(doTimeBagSpacing)
				{
					usedTimeBag.Next();
					double x = ::Time2XLocation(usedTimeBag.CurrentTime(), startTime, diffInMinutes, itsDataViewFrame);
					symbolRect.Center(NFmiPoint(x, theXYCoordinates[i][j].Y()));
				}
				else
					symbolRect.Center(theXYCoordinates[i][j]);

				DrawWindVector(theIsoLineData.itsIsolineData[i][j], symbolRect);
			}
		}
	}
}

void NFmiCrossSectionView::DrawWindVector(float theValue, const NFmiRect &theSymbolRect)
{
	if(theValue != kFloatMissing)
	{
		float windSpeed = theValue / 100;
		float windDir = float( ((int)theValue % 100) * 10);
        
		NFmiPoint symbolSize(itsDrawParam->OnlyOneSymbolRelativeSize());
		long pixelWidth = FmiRound(itsCtrlViewDocumentInterface->CrossSectionSystem()->GetGraphicalInfo().itsPixelsPerMM_x * itsDrawParam->SimpleIsoLineWidth() * itsDrawSizeFactorX);
		itsDrawingEnvironment.SetPenSize(NFmiPoint(pixelWidth, pixelWidth));
		NFmiWindBarb(windSpeed
					,windDir
					,theSymbolRect
					,itsToolBox
                    ,false
					,symbolSize.X() * 0.7
					,symbolSize.Y() * 0.5
					,0
					,&itsDrawingEnvironment).Build();
	}
}

// Kaksi paikkaa mihin label voidaan sijoittaa:
// 1. Heti viimeisen labelin yl‰ puolelle jos on tilaa.
// 2. Heti viimeisen labeli ala puolelle, jos ylh‰‰ll‰ ei ole tilaa.
static double CalcHelpScaleUnitStringYPos(const NFmiRect &theRect, double lastLabelHeight, double relFontHeight, double relFontYmove)
{
	double unitStringY = 0;
	if(lastLabelHeight - relFontHeight + relFontYmove > theRect.Top())
		unitStringY = lastLabelHeight - relFontHeight + relFontYmove;
	else
		unitStringY = lastLabelHeight + relFontHeight + relFontYmove;
	return unitStringY;
}

void NFmiCrossSectionView::DrawFlightLevelScale(void)
{
    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, kRight, true, &itsDataViewFrame);
	NFmiDrawingEnvironment envi;
	int fontSize = FmiRound(16 * itsDrawSizeFactorY);
	envi.SetPenSize(NFmiPoint(2 * itsDrawSizeFactorX, 2 * itsDrawSizeFactorY));
	envi.SetFontSize(NFmiPoint(fontSize, fontSize));
	NFmiPoint moveLabelRelatively(-itsToolBox->SX(FmiRound(2 * itsDrawSizeFactorX)), -itsToolBox->SY(fontSize)/2.);
	long extraOffsetInPixels = 0;
//	if(itsDoc->GetMTATempSystem().ShowKilometerScale()) // jos olisi s‰‰tˆ ett‰ piirret‰‰nkˆ km-asteikko vai ei, olisi ehto laitettava t‰h‰n
//		extraOffsetInPixels = static_cast<long>(fontSize*1.6); // siirret‰‰n asteikkoa, jos myˆs kilometri asteikko on n‰kyviss‰
	double extraOffset = itsToolBox->SX(extraOffsetInPixels);

	double tickMarkWidth = itsToolBox->SX(FmiRound(6 * itsDrawSizeFactorX));
	double unitStringYoffset = itsToolBox->SY(fontSize);

	double lastHeightInDataBox = 0;
	for (double flLevel = 0; flLevel <= 500;  flLevel += 10)
	{
		bool onlySmallTick = true;
		double shortTickChange = tickMarkWidth/3.;
		if(static_cast<int>(flLevel) % 50 == 0 || flLevel < 50)
		{
			onlySmallTick = false;
			shortTickChange = 0;
		}
		double P = ::CalcFlightLevelPressure(flLevel*100);
		double x = itsDataViewFrame.Right() - extraOffset;
		double y = p2y(P);
		NFmiPoint p1(x, y);
		NFmiPoint p2(x-tickMarkWidth + shortTickChange, y);
		NFmiLine l1(p1, p2, 0, &envi);
		itsToolBox->Convert(&l1);
		NFmiString str(NFmiStringTools::Convert<int>(static_cast<int>(flLevel)));
		NFmiPoint p3 = p2 + moveLabelRelatively;
		if(onlySmallTick == false)
		{
			NFmiText text1(p3, str, false, 0, &envi);
			itsToolBox->Convert(&text1);
			if(itsDataViewFrame.IsInside(p2))
				lastHeightInDataBox = y;
		}
	}
	if(lastHeightInDataBox != 0)
	{
		double unitStringY = CalcHelpScaleUnitStringYPos(itsDataViewFrame, lastHeightInDataBox, unitStringYoffset, moveLabelRelatively.Y());
		double unitStringX = itsDataViewFrame.Right();
		NFmiText txt1(NFmiPoint(unitStringX + moveLabelRelatively.X() - extraOffset, unitStringY), "FL", false, 0, &envi);
		itsToolBox->Convert(&txt1);
	}
}

void NFmiCrossSectionView::GetStartAndEndTimes(NFmiMetTime &theStartTimeOut, NFmiMetTime &theEndTimeOut)
{
    itsCtrlViewDocumentInterface->CrossSectionSystem()->GetStartAndEndTimes(theStartTimeOut, theEndTimeOut, CurrentTime(), itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView());
}

static short CalcTimeHelperLineStepInMinutes(long theDiffInMinutes, double theViewWidthInMM)
{
	// pyrit‰‰n ett‰ apuviiva on n. 2 cm v‰lein
	double widthPerHourFactor = theViewWidthInMM / (theDiffInMinutes/60.);
	if(widthPerHourFactor > 80)
		return 15;
	else if(widthPerHourFactor > 40)
		return 30;
	else if(widthPerHourFactor > 20)
		return 60;
	else if(widthPerHourFactor > 7)
		return 180;
	else if(widthPerHourFactor > 3.5)
		return 360;
	else if(widthPerHourFactor > 1.5)
		return 720;
	else if(widthPerHourFactor > 0.75)
		return 1440;
	else
		return 2880;
}

// Lasketaan sopivan harva timebagi, jolla piirret‰‰n mm. aikapoikkileikkauseen pysty-apupuviivoja t‰rkeiden aikojen kohdalle.
// T‰t‰ timebagia k‰ytet‰‰n myˆs tuuliviirien piirron harvennuksessa.
NFmiTimeBag NFmiCrossSectionView::CalcHelperLineTimeBag(void)
{
	NFmiMetTime startTime(NFmiMetTime::gMissingTime);
	NFmiMetTime endTime(NFmiMetTime::gMissingTime);
	GetStartAndEndTimes(startTime, endTime);
	long diffInMinutes = endTime.DifferenceInMinutes(startTime);
	short usedTimeStepInMinutes = ::CalcTimeHelperLineStepInMinutes(diffInMinutes, itsCtrlViewDocumentInterface->CrossSectionSystem()->GetGraphicalInfo().itsViewWidthInMM);
	NFmiMetTime helperLineTimeStart(startTime);
	helperLineTimeStart.SetTimeStep(usedTimeStepInMinutes);
	NFmiMetTime helperLineTimeEnd(endTime);
	helperLineTimeEnd.SetTimeStep(usedTimeStepInMinutes);
	NFmiTimeBag usedTimeBag(helperLineTimeStart, helperLineTimeEnd, usedTimeStepInMinutes);
	return usedTimeBag;
}

// piirt‰‰ aika/reitti/trajektori moodeissa aika apu viivat
void NFmiCrossSectionView::DrawHelperTimeLines(void)
{
	NFmiTimeBag usedHelperLineTimeBag = CalcHelperLineTimeBag();
	if(usedHelperLineTimeBag.Resolution() && itsCtrlViewDocumentInterface->CrossSectionSystem()->GetCrossMode() != NFmiCrossSectionSystem::kObsAndFor)
	{
		NFmiMetTime startTime(NFmiMetTime::gMissingTime);
		NFmiMetTime endTime(NFmiMetTime::gMissingTime);
		GetStartAndEndTimes(startTime, endTime);
		long diffInMinutes = endTime.DifferenceInMinutes(startTime);

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsDataViewFrame);
		NFmiDrawingEnvironment envi;
		envi.SetFrameColor(NFmiColor(0.65f, 0.65f, 0.65f));
		envi.SetFillPattern(FMI_DASH);
		envi.SetPenSize(NFmiPoint(1 * itsDrawSizeFactorX, 1 * itsDrawSizeFactorY));
		for(usedHelperLineTimeBag.Reset(); usedHelperLineTimeBag.Next(); )
		{
			double x = ::Time2XLocation(usedHelperLineTimeBag.CurrentTime(), startTime, diffInMinutes, itsDataViewFrame);
			double y1 = itsDataViewFrame.Bottom();
			double y2 = itsDataViewFrame.Top();

			NFmiPoint p1(x, y1);
			NFmiPoint p2(x, y2);
			::DrawLineWithToolBox(p1, p2, &envi, itsToolBox, true, false);
		}
	}
}

// piirt‰‰  ensimm‰isen ennuste ajan harmaalla viivalla
// piirt‰‰ myˆs kaikki ajanhetket, mist‰ on havaittu luotaus
void NFmiCrossSectionView::DrawObsForModeTimeLine(void)
{
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView() == false)
	{
		if(itsCtrlViewDocumentInterface->CrossSectionSystem()->GetCrossMode() == NFmiCrossSectionSystem::kObsAndFor && itsFirstForecastTimeIndex >= 0)
		{
            ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsDataViewFrame);
            NFmiDrawingEnvironment envi;
			envi.SetFrameColor(NFmiColor(0.1f, 0.5f, 0.1f));
			envi.SetPenSize(NFmiPoint(2 * itsDrawSizeFactorX, 2 * itsDrawSizeFactorY));

			double placeInTime = static_cast<double>(itsFirstForecastTimeIndex) / (GetUsedTimeBagForDataCalculations().GetSize() - 1);
			double x = itsDataViewFrame.Left() + itsDataViewFrame.Width() * placeInTime;
			double y1 = itsDataViewFrame.Bottom();
			double y2 = itsDataViewFrame.Top();
			// tick-mark ensin
			NFmiPoint p1(x, y1);
			NFmiPoint p2(x, y2);
			::DrawLineWithToolBox(p1, p2, &envi, itsToolBox, true, false);

			// piirr‰ myˆs viiva havaittujen luotauksien kohdalle
			if(itsObsForModeFoundObsTimes.size())
			{
				envi.SetPenSize(NFmiPoint(1 * itsDrawSizeFactorX, 1 * itsDrawSizeFactorY));
				envi.SetFrameColor(NFmiColor(0.9f, 0.66f, 0.66f));
				envi.SetFillPattern(FMI_DASHDOTDOT);
				for(size_t i = 0; i < itsObsForModeFoundObsTimes.size(); i++)
				{
					const NFmiTimeBag timeBag = GetUsedTimeBagForDataCalculations();
					double placeInTime2 = static_cast<double>(itsObsForModeFoundObsTimes[i].DifferenceInMinutes(timeBag.FirstTime()))/(timeBag.LastTime().DifferenceInMinutes(timeBag.FirstTime()));
					double x2 = itsDataViewFrame.Left() + itsDataViewFrame.Width() * placeInTime2;
					NFmiPoint obsp1(x2, y1);
					NFmiPoint obsp2(x2, y2);
					::DrawLineWithToolBox(obsp1, obsp2, &envi, itsToolBox, true, false);
				}
			}
		}
	}
}

static double GetUsedHeightStepInKm(double heightDiffKm)
{
	double usedHeightStepInKm = 1.;
	if(heightDiffKm < 0.1)
		usedHeightStepInKm = 0.01;
	else if(heightDiffKm < 0.2)
		usedHeightStepInKm = 0.02;
	else if(heightDiffKm < 0.5)
		usedHeightStepInKm = 0.05;
	else if(heightDiffKm < 1.)
		usedHeightStepInKm = 0.1;
	else if(heightDiffKm < 2)
		usedHeightStepInKm = 0.2;
	else if(heightDiffKm < 5)
		usedHeightStepInKm = 0.5;

	return usedHeightStepInKm;
}

static std::string ComposeHeightValueString(double theHeightKm, int theUsedDecimalCount, bool theUseKmAsUnit)
{
	double usedHeightValue = theHeightKm;
	if(theUseKmAsUnit == false)
	{
		usedHeightValue = theHeightKm * 1000.;
		usedHeightValue = FmiRound(usedHeightValue);
		theUsedDecimalCount = 0;
	}

	std::string str(NFmiStringTools::Convert<int>(static_cast<int>(usedHeightValue)));
	if(theUsedDecimalCount > 0 && CtrlViewUtils::IsEqualEnough(usedHeightValue, 0., 0.001) == false)
		str = NFmiValueString::GetStringWithMaxDecimalsSmartWay(usedHeightValue, theUsedDecimalCount);
	return str;
}

// Piirt‰‰ standardi ilmakeh‰n mukaiset kilometri palkit ja tekstit n‰yttˆruudun 
// oikeaan laitaan, data ruudun ulkopuolelle.
void NFmiCrossSectionView::DrawHeightScale(void)
{
//	if(itsDoc->GetMTATempSystem().ShowKilometerScale())
	{
		double minHeightKm = ::CalcHeightAtPressure(itsLowerEndOfPressureAxis);
		double maxHeightKm = ::CalcHeightAtPressure(itsUpperEndOfPressureAxis);
		double heightDiffKm = maxHeightKm - minHeightKm;
		double usedHeightStepKm = ::GetUsedHeightStepInKm(heightDiffKm);
		double usedStartHeightKm = std::floor(minHeightKm / usedHeightStepKm) * usedHeightStepKm;
		double usedEndHeightKm = std::ceil(maxHeightKm / usedHeightStepKm) * usedHeightStepKm;
		if(usedEndHeightKm > 15)
			usedEndHeightKm = 15;
		int usedDecimals = 0;
		if(usedHeightStepKm < 0.1)
			usedDecimals = 2;
		else if(usedHeightStepKm < 1.)
			usedDecimals = 1;

		const auto& usedRelativeFrame = GetFrame();

		// Laitetaanko korkeus arvot kilometrein‰ vai metrein‰. 
		// Samalla unit label on joko KM tai m.
		// Metreihin halutaan siirty‰ vain jos asteikko on l‰hell‰ 0 korkeutta ja kilometridesimaaleja olisi 
		// k‰ytˆss‰ ainakin 2, mik‰ tarkoittaa kymmenien metrien tarkkuutta
		bool useKmAsUnit = true;
		const double minMaxHeightInKmForMetresUnit = 0.5;
		if(usedDecimals >= 2 && ::fabs(minHeightKm) < minMaxHeightInKmForMetresUnit && ::fabs(maxHeightKm) < minMaxHeightInKmForMetresUnit)
			useKmAsUnit = false;

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, kRight, true, &usedRelativeFrame);
		NFmiDrawingEnvironment envi;
		int fontSize = FmiRound(16 * itsDrawSizeFactorY);
		envi.SetFontSize(NFmiPoint(fontSize, fontSize));
		NFmiPoint moveLabelRelatively(-itsToolBox->SX(FmiRound(2 * itsDrawSizeFactorX)), -itsToolBox->SY(fontSize)/2.);
		envi.SetPenSize(NFmiPoint(1 * itsDrawSizeFactorX, 1 * itsDrawSizeFactorY));

		double tickMarkWidth = itsToolBox->SX(FmiRound(6 * itsDrawSizeFactorX));
		double unitStringYoffset = itsToolBox->SY(fontSize);

		double lastHeightInDataBox = 0;
		for (double heightKM = usedStartHeightKm; heightKM <= usedEndHeightKm;  heightKM += usedHeightStepKm)
		{
			double P = ::CalcPressureAtHeight(heightKM);
			double x = usedRelativeFrame.Right();
			double y = p2y(P);
			// ensin label teksti reunaan
			NFmiPoint p1(x, y);
			NFmiString heightLabelStr(::ComposeHeightValueString(heightKM, usedDecimals, useKmAsUnit));
			NFmiPoint p2 = p1 + moveLabelRelatively;
			NFmiText text1(p2, heightLabelStr, false, 0, &envi);
			itsToolBox->Convert(&text1);
			// Sitten tick-mark alkamaan data-laatikosta oikealle
			NFmiPoint p3(itsDataViewFrame.Right(), y);
			NFmiPoint p4(itsDataViewFrame.Right() + tickMarkWidth, y);
			NFmiLine l1(p3, p4, 0, &envi);
			itsToolBox->Convert(&l1);
			if(usedRelativeFrame.IsInside(p3))
				lastHeightInDataBox = y;
		}
		if(lastHeightInDataBox != 0)
		{
			double unitStringY = CalcHelpScaleUnitStringYPos(usedRelativeFrame, lastHeightInDataBox, unitStringYoffset, moveLabelRelatively.Y());
			double unitStringX = usedRelativeFrame.Right();
			NFmiText txt1(NFmiPoint(unitStringX + moveLabelRelatively.X(), unitStringY), useKmAsUnit ? "KM" : "m", false, 0, &envi);
			itsToolBox->Convert(&txt1);
		}
	}
}

std::vector<float> NFmiCrossSectionView::CalcCurrentLevelPressures(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	std::vector<float> values;
	if(theInfo && theInfo->Param(kFmiPressure))
	{
        auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
        auto crossMode = crossSectionSystem->GetCrossMode();
		bool obsForMode = crossMode == NFmiCrossSectionSystem::kObsAndFor;
        auto trajectorySystem = itsCtrlViewDocumentInterface->TrajectorySystem();
		if(trajectorySystem->ShowTrajectoriesInCrossSectionView())
		{
			const std::vector<NFmiPoint> &points = trajectorySystem->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryPoints();
			const std::vector<NFmiMetTime> &times = trajectorySystem->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryTimes();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				values.push_back(theInfo->InterpolatedValue(points[i], times[i]));
		}
		else if(crossMode == NFmiCrossSectionSystem::kNormal)
		{
			NFmiMetTime aTime(CurrentTime());
			const std::vector<NFmiPoint> &points = crossSectionSystem->MinorPoints();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				values.push_back(theInfo->InterpolatedValue(points[i], aTime));
		}
		else if(crossMode == NFmiCrossSectionSystem::kRoute)
		{
			const std::vector<NFmiMetTime> &times = crossSectionSystem->RouteTimes();
			const std::vector<NFmiPoint> &points = crossSectionSystem->MinorPoints();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				values.push_back(theInfo->InterpolatedValue(points[i], times[i]));
		}
		else if(crossMode == NFmiCrossSectionSystem::kTime || obsForMode)
		{
			NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
			NFmiPoint point = crossSectionSystem->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
			for(times.Reset(); times.Next(); )
				values.push_back(theInfo->InterpolatedValue(point, times.CurrentTime()));
		}
	}
	return values;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiCrossSectionView::GetFirstHybridInfo(void)
{
	NFmiDrawParamList *dpList = itsCtrlViewDocumentInterface->CrossSectionViewDrawParamList(itsViewGridRowNumber);
	if(dpList)
	{
		for(dpList->Reset(); dpList->Next(); )
		{
			boost::shared_ptr<NFmiDrawParam> drawParam = dpList->Current();
			boost::shared_ptr<NFmiFastQueryInfo> hybridInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(drawParam, true, false);
			if(hybridInfo && hybridInfo->DataType() == NFmiInfoData::kHybridData)
				return hybridInfo;
		}
	}
	return boost::shared_ptr<NFmiFastQueryInfo>();
}

void NFmiCrossSectionView::DrawHybridLevels(void)
{
	boost::shared_ptr<NFmiFastQueryInfo> hybridInfo = GetFirstHybridInfo();
	if(hybridInfo) // vain 1. mallipinta datan levelit piirret‰‰n
	{ // piirret‰‰n hybridilevelit oikeille kohdilleen poikkileikkaukseen
		if(itsCtrlViewDocumentInterface->CrossSectionSystem()->ShowHybridLevels())
		{
            ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsDataViewFrame);
            int fontSize = FmiRound(15 * itsDrawSizeFactorY);
			double labelMoveX = itsToolBox->SX(FmiRound(2 * itsDrawSizeFactorX));
			double labelMoveY = itsToolBox->SY(fontSize)/2.;
			NFmiDrawingEnvironment envi;
			envi.SetFontSize(NFmiPoint(fontSize, fontSize));
			envi.SetFrameColor(NFmiColor(0.2f, 0.55f, 0.2f));
			envi.SetFillPattern(FMI_DOT);
			envi.SetPenSize(NFmiPoint(FmiRound(0.5 * itsDrawSizeFactorX), FmiRound(0.5 * itsDrawSizeFactorY)));

			for(hybridInfo->ResetLevel(); hybridInfo->NextLevel(); )
			{
				std::vector<float> levelPressures(CalcCurrentLevelPressures(hybridInfo));
				if(levelPressures.size() > 1)
				{
					double x = Column2x(0);
					double y = p2y(levelPressures[0]);
					NFmiPoint p1(x, y);
					NFmiPoint p2;
					bool firstFoundPoint = true;
					for(int i=1; i<static_cast<int>(levelPressures.size()); i++)
					{
						x = Column2x(i);
						y = p2y(levelPressures[i]);
						p2 = NFmiPoint(x, y);
						if(p1.X() != kFloatMissing && p1.Y() != kFloatMissing && p2.X() != kFloatMissing && p2.Y() != kFloatMissing)
						{
							::DrawLineWithToolBox(p1, p2, &envi, itsToolBox, false, true);

							if(firstFoundPoint) // eli 1. kerralla piirret‰‰n hybridille label (tulee siis vasempaan reunaan)
							{
								NFmiString labelStr(NFmiStringTools::Convert<float>(hybridInfo->Level()->LevelValue()));
								NFmiPoint labelPoint(p1.X() + labelMoveX, p1.Y() - labelMoveY);
								NFmiText txt1(labelPoint, labelStr, false, 0, &envi);
								itsToolBox->Convert(&txt1);
								firstFoundPoint = false;
							}
						}
						p1 = p2;
					}
				}
			}
		}
	}
}

// DrawGroundLevel-metodissa pit‰‰ varmistaa ett‰ CrossSectionSystemin
// Main XY-pisteet on initialisoitu, ett‰ pinta profiili saadaan piirretty‰.
static void QuickInitMainXYPoints(NFmiCrossSectionSystem &theCrossSectionSystem, const NFmiRect &theDataRect)
{
	theCrossSectionSystem.StartXYPoint(NFmiPoint(theDataRect.Left(), 0));
	theCrossSectionSystem.MiddleXYPoint(NFmiPoint(theDataRect.Center().X(), 0));
	theCrossSectionSystem.EndXYPoint(NFmiPoint(theDataRect.Right(), 0));
}

static bool DrawBatymetria(NFmiDrawParamList &theDrawParamList, NFmiInfoOrganizer &theInfoOrganizer)
{
	if(theDrawParamList.Index(1)) // etsit‰‰n 1. drawParam
	{
		boost::shared_ptr<NFmiFastQueryInfo> firstDataInfo = theInfoOrganizer.Info(theDrawParamList.Current(), true, true);
		if(firstDataInfo && firstDataInfo->SizeLevels() > 1) // datassa pit‰‰ olla useita leveleit‰
		{
			for(firstDataInfo->ResetLevel(); firstDataInfo->NextLevel(); )
			{
				if(firstDataInfo->Level()->LevelValue() < 0)
					return true; // jos datan yksikin level value on negatiivinen, t‰llˆin oletetaan ett‰ tarvitaan batymetria piirtoa
			}
		}
	}
	return false;
}

// Piirret‰‰n maanpinta seuraavin prioriteetti s‰‰nnˆin:
// 0. Jos 1. piirrett‰v‰ data vaatii batymetriaa (= level heigth sis‰lt‰‰ negatiivisia arvoja), piirret‰‰n maanpinta topo-datasta.
// 1. Jos n‰ytˆss‰ mallidataa, jolta lˆytyy pintadatasta kFmiPressureAtStationLevel -parametri, 
//    k‰ytet‰‰n sit‰ m‰‰ritt‰m‰‰n maanpintaa (paine-asteikolle)
// 2. Jos n‰ytˆss‰ mallipinta dataa, piirr‰ mallipintadatan alinkerros maanpintana (paine-asteikolle)
// 3. K‰y drawParam-Listaa l‰pi kunnes lˆytyy data, jolta lˆytyy pinnan piirto kohdasta 1 tai 2.
// 4. Muuten piirr‰ topografia datasta korkeus parametri (korkeus-asteikolle)
void NFmiCrossSectionView::DrawGround(void)
{ 
	// yleiset piirto optio maanpinta piirtoa varten
	NFmiDrawingEnvironment envi;
	envi.SetFrameColor(NFmiColor(0.5f, 0.25f, 0));
	double lineWidthInMM = 1.1;
	int pixelSize = boost::math::iround(lineWidthInMM * itsCtrlViewDocumentInterface->CrossSectionSystem()->GetGraphicalInfo().itsPixelsPerMM_y);
	envi.SetPenSize(NFmiPoint(pixelSize, pixelSize));

	NFmiDrawParamList *dpList = itsCtrlViewDocumentInterface->CrossSectionViewDrawParamList(itsViewGridRowNumber);
	if(dpList)
	{
		bool drawBatymetria = ::DrawBatymetria(*dpList, *itsCtrlViewDocumentInterface->InfoOrganizer());
		if(drawBatymetria == false)
		{
			for(dpList->Reset(); dpList->Next(); )
			{
				boost::shared_ptr<NFmiDrawParam> drawParam = dpList->Current();
				boost::shared_ptr<NFmiFastQueryInfo> groundInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kViewable, *(drawParam->Param().GetProducer()), true);
				if(groundInfo && groundInfo->Param(kFmiPressureAtStationLevel))
				{
					if(DrawModelGroundLevel(groundInfo, envi, false))
    					return ; // voidaan lopettaa
				}

				// Haetaan paras verkikaali data, mik‰ mallista lˆytyy (1. hybrid-data, 2. pressure-data), ja jos se oli hybrid, piirret‰‰n sen alin kerros
				boost::shared_ptr<NFmiFastQueryInfo> hybridInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->FindSoundingInfo(*(drawParam->Param().GetProducer()));
				if(hybridInfo && hybridInfo->DataType() == NFmiInfoData::kHybridData)
				{
					if(DrawModelGroundLevel(hybridInfo, envi, true))
    					return ; // voidaan lopettaa
				}
			}
		}
	}
	DrawGroundLevel(envi); // jos muu ei auta, piirret‰‰n maanpinta topografian avulla
}

// piirret‰‰n karkea maanpinta poikkileikkaus n‰yttˆˆn
void NFmiCrossSectionView::DrawGroundLevel(NFmiDrawingEnvironment &theEnvi)
{
    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
	::QuickInitMainXYPoints(*crossSectionSystem, itsDataViewFrame);
	CalcGroundHeights(); // ker‰‰ pinta profiili t‰st‰

    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsDataViewFrame);
    
	double startX = crossSectionSystem->StartXYPoint().X();
	double width = crossSectionSystem->EndXYPoint().X() - startX;
	NFmiPolyline groundPolyLine(itsDataViewFrame, 0, &theEnvi);
	for(int i = 0; i < static_cast<int>(itsGroundHeights.size()); i++)
	{
		if(itsGroundHeights[i] == kFloatMissing)
			itsGroundHeights[i] = 0;

		double x = startX + i * width / (itsGroundHeights.size()-1);
		groundPolyLine.AddPoint(NFmiPoint(x, p2y(CalcPressureAtHeight(itsGroundHeights[i]/1000.))));
	}
	itsToolBox->DrawPolyline(&groundPolyLine, NFmiPoint(0,0), NFmiPoint(1,1));

	// Siit‰ l‰htien kun batymetria otettiin mukaan topografia dataan, pit‰‰ piirt‰‰ meren pinta viiva 
	// sellaisiin kohtiin miss‰ topo arvot ovat negatiivisia.
	DrawSeaLevel(); 
}

using NFmiSoundingFunctions::MyPoint;

// T‰m‰ laskee kahden annetun pisteen ja meren viiva pisteiden v‰lisen leikkauksen x-kohdan.
static double CalcSeaLevelLinePlaceX(const NFmiPoint &theStartPoint, const NFmiPoint &theEndPoint)
{
	MyPoint p1(theStartPoint.X(), theStartPoint.Y());
	MyPoint p2(theEndPoint.X(), theEndPoint.Y());
	MyPoint p3(theStartPoint.X(), 0);
	MyPoint p4(theEndPoint.X(), 0);

	MyPoint intersectionPoint = NFmiSoundingFunctions::CalcTwoLineIntersectionPoint(p1, p2, p3, p4);
	return intersectionPoint.x;
}

static double CalcRelativeXplace(int theIndex, double theRelativeStartX, double thrRelativeWidth, double theRelativePlaceDividor, const std::vector<float> &theGroundHeights)
{
	if(theIndex == 0)
		return theRelativeStartX;
	else if(theIndex == theGroundHeights.size()) // t‰m‰ on erikoistapaus eli kun ollaan k‰yty ground-pisteet l‰pi ja ollaan viel‰ pinnan alla
	{
		double relativeX = theRelativeStartX + (theIndex * thrRelativeWidth / theRelativePlaceDividor);
		return relativeX;
	}
	else
	{
		NFmiPoint p1(theRelativeStartX + ((theIndex - 1) * thrRelativeWidth / theRelativePlaceDividor), theGroundHeights[theIndex - 1]);
		NFmiPoint p2(theRelativeStartX + (theIndex * thrRelativeWidth / theRelativePlaceDividor), theGroundHeights[theIndex]);
		double relativeX = ::CalcSeaLevelLinePlaceX(p1, p2);
		return relativeX;
	}
}

// Oletus DrawSeaLevel-funktiolle:
// T‰t‰ kutsutaan NFmiCrossSectionView::DrawGroundLevel-metodin lopussa ennen kuin
// kuin toolboxin clippaus laitetaan falseksi. T‰llˆin on kutsuttu QuickInitMainXYPoints
// -funktiota ja CalcGroundHeights-metodia, miss‰ on laskettu maanpinta/merenpohja pisteet.
// Lis‰ksi clippaus on p‰‰ll‰ eik‰ sit‰ tarvitse erikseen asetell‰ p‰‰lle ja pois t‰‰ll‰.
void NFmiCrossSectionView::DrawSeaLevel(void)
{
	double lineWidthInMM = 0.7;
    auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
	int pixelSize = static_cast<int>(lineWidthInMM * crossSectionSystem->GetGraphicalInfo().itsPixelsPerMM_y);
	pixelSize = FmiMax(pixelSize, 1); // pit‰‰ kuitenkin olla v‰h. 1 pikselin paksuinen viiva
	NFmiDrawingEnvironment seaLevelLineStyle;
	seaLevelLineStyle.SetPenSize(NFmiPoint(pixelSize, pixelSize));
	seaLevelLineStyle.SetFrameColor(NFmiColor(0.f, 0.f, 0.6f));
	double relativeStartX = crossSectionSystem->StartXYPoint().X();
	double relativeWidth = crossSectionSystem->EndXYPoint().X() - relativeStartX;
	double relativePlaceDividor = itsGroundHeights.size() - 1.;

	bool belowZero = false; // t‰m‰n muuttujan avulla tiedet‰‰n ollaanko jo menossa merenpinnan alla topo-datan kanssa.
	double zeroHeightRelative = p2y(CalcPressureAtHeight(0.));
	NFmiPoint startPoint(0, zeroHeightRelative);
	NFmiPoint endPoint(0, zeroHeightRelative);
	for(int i = 0; i < static_cast<int>(itsGroundHeights.size()); i++)
	{
		if(itsGroundHeights[i] <= 0)
		{
			if(belowZero)
			{
			}
			else
			{ // merkit‰‰n t‰m‰ merenpinnan viivan alkupisteeksi
				belowZero = true;
				startPoint.X(::CalcRelativeXplace(i, relativeStartX, relativeWidth, relativePlaceDividor, itsGroundHeights));
			}
		}
		else
		{
			if(belowZero)
			{ // jos oltiin pinnan alla, merkit‰‰n loppupisteeksi ja piirret‰‰n t‰m‰ osio merenpintaa
				endPoint.X(::CalcRelativeXplace(i, relativeStartX, relativeWidth, relativePlaceDividor, itsGroundHeights)); 

				NFmiLine line(startPoint, endPoint, 0, &seaLevelLineStyle);
				itsToolBox->Convert(&line);
			}
			belowZero = false;
		}
	}

	if(belowZero)
	{ // jos oltiin lopuksi viel‰ pinnan alla, annetaan indeksiksi itsGroundHeights.size(), jolloin loppupisteeksi saadaan loppureuna
		endPoint.X(::CalcRelativeXplace(static_cast<int>(itsGroundHeights.size()), relativeStartX, relativeWidth, relativePlaceDividor, itsGroundHeights)); 

		NFmiLine line(startPoint, endPoint, 0, &seaLevelLineStyle);
		itsToolBox->Convert(&line);
	}

}

// Jos annetusta vektorista lˆytyy yksikin ei missing arvo, palauta true, muuten false
static void ClearValueVectorIfOnlyMissingValues(std::vector<float>& values)
{
	auto nonMissingIter = std::find_if(values.begin(), values.end(), [](auto value) {return value != kFloatMissing; });
	if(nonMissingIter == values.end())
	{
		values.clear();
	}
}

// Piirret‰‰n mallista saatu karkea maanpinta profiili n‰yttˆˆn.
// Palauttaa true, jos piirretty jotain, muuten false.
bool NFmiCrossSectionView::DrawModelGroundLevel(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDrawingEnvironment &theEnvi, bool fUseHybridCalculations)
{
	if(theInfo)
	{
		if(fUseHybridCalculations)
			CalcModelGroundPressures(theInfo); // ker‰‰ pinta profiili mallipintadatasta
		else
			CalcModelPressuresAtStation(theInfo); // ker‰‰ pinta profiili pintadatan kFmiPressureAtStationLevel -parametrista

		::ClearValueVectorIfOnlyMissingValues(itsModelGroundPressures);

        // Jos datassa ei ollutkaan haluttua parametria, t‰llˆin itsModelGroundPressures on tyhj‰ ja ei voida piirt‰‰ mit‰‰n.
        if(itsModelGroundPressures.size())
        {
            ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsDataViewFrame);

            auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
            double startX = crossSectionSystem->StartXYPoint().X();
            double width = crossSectionSystem->EndXYPoint().X() - startX;
            NFmiPolyline groundPolyLine(itsDataViewFrame, 0, &theEnvi);
            for(int i = 0; i < static_cast<int>(itsModelGroundPressures.size()); i++)
            {
                double x = startX + i * width / (itsModelGroundPressures.size() - 1);
                groundPolyLine.AddPoint(NFmiPoint(x, p2y(itsModelGroundPressures[i])));
            }
            itsToolBox->DrawPolyline(&groundPolyLine, NFmiPoint(0, 0), NFmiPoint(1, 1));

            return true;
        }
	}
    return false;
}

void NFmiCrossSectionView::CalcRouteDistances(void)
{
	const std::vector<NFmiPoint> &points = GetMinorPoints();
	itsRoutePointsDistToEndPoint = std::vector<float>(points.size(), 0);
	if(points.size() > 1)
	{
		NFmiLocation loc1(points[0]);
		NFmiLocation loc2(points[0]);
		for(int i = 1; i < static_cast<int>(points.size()); i++)
		{
			loc2 = NFmiLocation(points[i]);
			if(loc1.GetLongitude() != kFloatMissing && loc2.GetLongitude() != kFloatMissing)
				itsRoutePointsDistToEndPoint[i] = static_cast<float>(loc2.Distance(loc1) + itsRoutePointsDistToEndPoint[i-1]); // poikkileikkauslinjaa pitkin laskettu kumulatiivinen et‰isyys alusta
			else
				itsRoutePointsDistToEndPoint[i] = kFloatMissing;
			loc1 = loc2;
		}
	}
}

void NFmiCrossSectionView::CalcGroundHeights(void) //NFmiSmartInfo *theInfo)
{
	// lasketaan topo-datasta maanpinta
	boost::shared_ptr<NFmiFastQueryInfo> topoInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kStationary);
	if(topoInfo && topoInfo->Param(kFmiTopoGraf))
	{
		topoInfo->FirstLevel();
		itsGroundHeights.clear();

        auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
        auto crossMode = crossSectionSystem->GetCrossMode();
        if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
		{
			const std::vector<NFmiPoint> &points = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryPoints();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				itsGroundHeights.push_back(topoInfo->InterpolatedValue(points[i]));
		}
		else if(crossMode == NFmiCrossSectionSystem::kNormal)
		{
			NFmiMetTime aTime(CurrentTime());
			const std::vector<NFmiPoint> &points = crossSectionSystem->MinorPoints();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				itsGroundHeights.push_back(topoInfo->InterpolatedValue(points[i]));
		}
		else if(crossMode == NFmiCrossSectionSystem::kRoute)
		{
			const std::vector<NFmiPoint> &points = crossSectionSystem->MinorPoints();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				itsGroundHeights.push_back(topoInfo->InterpolatedValue(points[i]));
		}
		else if((crossMode == NFmiCrossSectionSystem::kTime) || (crossMode == NFmiCrossSectionSystem::kObsAndFor))
		{
			NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
			NFmiPoint point = crossSectionSystem->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
			for(times.Reset(); times.Next(); )
				itsGroundHeights.push_back(topoInfo->InterpolatedValue(point));
		}
	}
}

void NFmiCrossSectionView::CalcModelGroundPressures(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	itsModelGroundPressures.clear();
	if(theInfo && theInfo->Param(kFmiPressure))
	{
		if(theInfo->PressureParamIsRising())
			theInfo->LastLevel();
		else
			theInfo->FirstLevel();

        auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
        auto crossMode = crossSectionSystem->GetCrossMode();
        auto trajectorySystem = itsCtrlViewDocumentInterface->TrajectorySystem();
        bool obsForMode = crossMode == NFmiCrossSectionSystem::kObsAndFor;
		if(trajectorySystem->ShowTrajectoriesInCrossSectionView())
		{
			const std::vector<NFmiPoint> &points = trajectorySystem->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryPoints();
			const std::vector<NFmiMetTime> &times = trajectorySystem->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryTimes();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				itsModelGroundPressures.push_back(theInfo->InterpolatedValue(points[i], times[i]));
		}
		else if(crossMode == NFmiCrossSectionSystem::kNormal)
		{
			NFmiMetTime aTime(CurrentTime());
			const std::vector<NFmiPoint> &points = crossSectionSystem->MinorPoints();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				itsModelGroundPressures.push_back(theInfo->InterpolatedValue(points[i], aTime));
		}
		else if(crossMode == NFmiCrossSectionSystem::kRoute)
		{
			const std::vector<NFmiMetTime> &times = crossSectionSystem->RouteTimes();
			const std::vector<NFmiPoint> &points = crossSectionSystem->MinorPoints();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				itsModelGroundPressures.push_back(theInfo->InterpolatedValue(points[i], times[i]));
		}
		else if(crossMode == NFmiCrossSectionSystem::kTime || obsForMode)
		{
			NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
			NFmiPoint point = crossSectionSystem->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
			for(times.Reset(); times.Next(); )
				itsModelGroundPressures.push_back(theInfo->InterpolatedValue(point, times.CurrentTime()));
		}
	}
}

void NFmiCrossSectionView::CalcModelPressuresAtStation(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	itsModelGroundPressures.clear();
	if(theInfo && theInfo->Param(kFmiPressureAtStationLevel))
	{
		theInfo->FirstLevel();
        auto crossSectionSystem = itsCtrlViewDocumentInterface->CrossSectionSystem();
        auto crossMode = crossSectionSystem->GetCrossMode();
        auto trajectorySystem = itsCtrlViewDocumentInterface->TrajectorySystem();

		bool obsForMode = crossMode == NFmiCrossSectionSystem::kObsAndFor;
		if(trajectorySystem->ShowTrajectoriesInCrossSectionView())
		{
			const std::vector<NFmiPoint> &points = trajectorySystem->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryPoints();
			const std::vector<NFmiMetTime> &times = trajectorySystem->Trajectory(itsViewGridRowNumber - 1).CrossSectionTrajectoryTimes();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				itsModelGroundPressures.push_back(theInfo->InterpolatedValue(points[i], times[i]));
		}
		else if(crossMode == NFmiCrossSectionSystem::kNormal)
		{
			NFmiMetTime aTime(CurrentTime());
			const std::vector<NFmiPoint> &points = crossSectionSystem->MinorPoints();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				itsModelGroundPressures.push_back(theInfo->InterpolatedValue(points[i], aTime));
		}
		else if(crossMode == NFmiCrossSectionSystem::kRoute)
		{
			const std::vector<NFmiMetTime> &times = crossSectionSystem->RouteTimes();
			const std::vector<NFmiPoint> &points = crossSectionSystem->MinorPoints();
			for(int i = 0; i < static_cast<int>(points.size()); i++)
				itsModelGroundPressures.push_back(theInfo->InterpolatedValue(points[i], times[i]));
		}
		else if(crossMode == NFmiCrossSectionSystem::kTime || obsForMode)
		{
			NFmiTimeBag times(GetUsedTimeBagForDataCalculations()); // pit‰‰ tehd‰ kopio
			NFmiPoint point = crossSectionSystem->StartPoint(); // otetaan 1. p‰‰piste aikapoikkileikkauksen kohteeksi
			for(times.Reset(); times.Next(); )
				itsModelGroundPressures.push_back(theInfo->InterpolatedValue(point, times.CurrentTime()));
		}
	}
}

bool NFmiCrossSectionView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(itsRect.IsInside(thePlace))
	{
		if(ShowParamHandlerView() && itsParamHandlerView->IsIn(thePlace))
		{
			return itsParamHandlerView->MouseWheel(thePlace, theKey, theDelta);
		}
		if(itsPressureScaleFrame.IsInside(thePlace))
		{
			auto ctrlKeyDown = theKey & kCtrlKey;
			if(itsPressureScaleFrame.Center().Y() > thePlace.Y())
				return ChangePressureScale(theDelta < 0 ? kDown : kUp, true, ctrlKeyDown); // true liikuttaa asteikon yl‰p‰‰t‰ ja false alap‰‰t‰
			else
				return ChangePressureScale(theDelta < 0 ? kDown : kUp, false, ctrlKeyDown); // true liikuttaa asteikon yl‰p‰‰t‰ ja false alap‰‰t‰
		}
		else
			return itsCtrlViewDocumentInterface->CrossSectionSystem()->MouseWheel(thePlace, theKey, theDelta);
	}
	return false;
}

bool NFmiCrossSectionView::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
    if(itsRect.IsInside(thePlace))
    {
		itsCtrlViewDocumentInterface->SetLastActiveDescTopAndViewRow(itsMapViewDescTopIndex,
			GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));
		if(ShowParamHandlerView() && itsParamHandlerView->IsIn(thePlace))
        {
            return itsParamHandlerView->LeftButtonDown(thePlace, theKey);
        }
    }
    return false;
}

bool NFmiCrossSectionView::MouseMove(const NFmiPoint& thePlace, unsigned long theKey)
{
    // mouse captured pit‰‰ hanskata, vaikka hiiri olisi itsParamHandlerView -ikkunan ulkona
    if(ShowParamHandlerView() && itsParamHandlerView->IsMouseCaptured())
        return itsParamHandlerView->MouseMove(thePlace, theKey);

    return false;
}

bool NFmiCrossSectionView::IsMouseDraggingOn(void)
{
    if(itsParamHandlerView && itsParamHandlerView->IsMouseDraggingOn())
        return true;
    else
        return false;
}

void NFmiCrossSectionView::UpdateCachedParameterName()
{
	CachedParameterName(CtrlViewUtils::GetParamNameString(itsDrawParam, true, false, false, 0, false, true, true, itsInfo), false);
	CachedParameterName(CtrlViewUtils::GetParamNameString(itsDrawParam, true, false, true, 0, false, true, true, itsInfo), true);
}
