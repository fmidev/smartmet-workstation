#include "NFmiTrajectoryView.h"
#include "NFmiToolBox.h"
#include "NFmiLine.h"
#include "NFmiText.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiTrajectorySystem.h"
#include "NFmiTrajectory.h"
#include "NFmiTimeControlView.h"
#include "NFmiPolyline.h"
#include "NFmiValueString.h"
#include "NFmiProducerSystem.h"
#include "CtrlViewDocumentInterface.h"
#include "CtrlViewFunctions.h"
#include "CtrlViewTimeConsumptionReporter.h"

#include "boost\math\special_functions\round.hpp"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------

NFmiTrajectoryView::NFmiTrajectoryView(const NFmiRect &theRect, NFmiToolBox * theToolBox)
:NFmiCtrlView(0, theRect
				,theToolBox)
,itsDataRect()
,itsTimeControlView(0)
,itsTimeControlViewRect()
,itsStartPressureLevelMarkerRect()
,itsMaxPressure(1050)
,itsMinPressure(450)
,itsPressureScaleFrame()
,itsPressureScaleFontSize(16)
{
}
 NFmiTrajectoryView::~NFmiTrajectoryView(void)
{
	delete itsTimeControlView;
}

//--------------------------------------------------------
// Draw
//--------------------------------------------------------
void NFmiTrajectoryView::Draw(NFmiToolBox *theGTB)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    if(!theGTB)
		return;
	itsToolBox = theGTB; // vähän arvelluttaa tämä, koska toolboxien pitäisi olla jo samoja

	CalcRects();
	DrawBackground();
	DrawTimeControl();
	DrawPressureScale();
	DrawSelectedPressureLevelMarker();
	DrawTrajectories();
	DrawLegend();
	itsDrawingEnvironment.DisableFill();
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	DrawFrame(itsDrawingEnvironment, itsDataRect);
}

void NFmiTrajectoryView::DrawLegend(void)
{
	itsToolBox->RelativeClipRect(itsDataRect, true);

	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	NFmiPoint fontSize(16,16);
	itsDrawingEnvironment.SetFontSize(fontSize);
	std::string str("P:");
	str +=NFmiStringTools::Convert<int>(static_cast<int>(itsCtrlViewDocumentInterface->TrajectorySystem()->SelectedPressureLevel()));
	double xShift = itsToolBox->SX(3);
	NFmiPoint p1(itsDataRect.TopLeft());
	p1.X(p1.X() + xShift);
	NFmiText txt(p1, str, false, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&txt);


	int index = 0;
	const std::vector<boost::shared_ptr<NFmiTrajectory> >& trajectories = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectories();
	std::vector<boost::shared_ptr<NFmiTrajectory> >::const_iterator it = trajectories.begin();
	for( ; it != trajectories.end() ; ++it)
	{
		DrawTrajectoryLegend(*(*it).get(), index);
		index++;
	}
	itsToolBox->UseClipping(false);
}

static std::string GetModelLocationString(NFmiProducerSystem &theProdSystem, NFmiLocation &theLocation, const NFmiProducer &theProducer, const NFmiMetTime &theOriginTime)
{
	std::string name;
	// etsi mallin nimi
	unsigned int modelIndex = theProdSystem.FindProducerInfo(theProducer);
	if(modelIndex > 0)
		name += theProdSystem.Producer(modelIndex).UltraShortName();
	else
		name += "X?";
	name += theOriginTime.ToStr("HH ");

	name += CtrlViewUtils::GetLatitudeMinuteStr(theLocation.GetLatitude(), 0).c_str();
	name += ",";
	name += CtrlViewUtils::GetLongitudeMinuteStr(theLocation.GetLongitude(), 0).c_str();
	return name;
}

static std::string GetDataTypeString(int theDataType)
{
	std::string name;
	if(theDataType == 0)
		name += "pi";
	else if(theDataType == 1)
		name += "pp";
	else if(theDataType == 2)
		name += "mp";

	return name;
}

static std::string GetDirectionString(FmiDirection theDirection)
{
	std::string name;
	if(theDirection == kForward)
		name += "->";
	else if(theDirection == kBackward)
		name += "<-";

	return name;
}

void NFmiTrajectoryView::DrawTrajectoryLegend(const NFmiTrajectory &theTrajectory, int theIndex)
{
	NFmiPoint fontSize(16,16);
	itsDrawingEnvironment.SetFontSize(fontSize);
	itsDrawingEnvironment.SetFrameColor(itsCtrlViewDocumentInterface->GeneralColor(theIndex));

	string str(NFmiStringTools::Convert<int>(theIndex+1));
	str += ". ";
	str += ::GetModelLocationString(itsCtrlViewDocumentInterface->ProducerSystem(), NFmiLocation(theTrajectory.LatLon()), theTrajectory.Producer(), theTrajectory.OriginTime());
	str += theTrajectory.Time().ToStr(" MM.DD HH:mm");
	str += " (";
	str += ::GetDirectionString(theTrajectory.Direction());
	str += " ";
	str += ::GetDataTypeString(theTrajectory.DataType());
	str += " ";
	if(theTrajectory.Is3DTrajectory())
	{
		str += "P=";
		str += NFmiStringTools::Convert<int>(static_cast<int>(theTrajectory.PressureLevel()));
		str += " ";
	}
	str += NFmiStringTools::Convert<int>(theTrajectory.TimeLengthInHours());
	str += "-";
	str += NFmiStringTools::Convert<int>(theTrajectory.TimeStepInMinutes());
	if(theTrajectory.PlumesUsed())
	{
		str += " ";
		str += NFmiStringTools::Convert<int>(theTrajectory.PlumeParticleCount());
		str += "x";
		str += NFmiStringTools::Convert<int>(static_cast<int>(theTrajectory.PlumeProbFactor()));
		str += "%";
	}
	if(theTrajectory.Isentropic())
	{
		str += " Is:";
		str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(theTrajectory.IsentropicTpotValue(), 1);
	}
	str += ")";

	NFmiString str2(str);
	double rowHeight = itsToolBox->SY(static_cast<long>(fontSize.Y()));
	double letterWidth = itsToolBox->SX(static_cast<long>(fontSize.X()));
	NFmiPoint strPoint(itsDataRect.TopLeft());
	strPoint.X(strPoint.X() + letterWidth * 4);
	strPoint.Y(strPoint.Y() + (rowHeight*0.1) + (theIndex * rowHeight * 0.7));
	NFmiText txt(strPoint, str2, false, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&txt);
}

void NFmiTrajectoryView::DrawSelectedPressureLevelMarker(void)
{
	itsDrawingEnvironment.SetFillColor(NFmiColor(0.8f,0.3f,0.3f));
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	DrawFrame(itsDrawingEnvironment, itsStartPressureLevelMarkerRect);
}

void NFmiTrajectoryView::ChangePAxisValue(FmiDirection theMouseButton, bool upperPart)
{
	float change = 50;
	float step = 50;
	if(upperPart)
	{
		if(itsMinPressure <= 10)
			change = 1;
		else if(itsMinPressure <= 50)
			change = 10;
	}
	step = change;
	if(theMouseButton == kRight)
		change = -change;
	if(upperPart)
	{
		float newValue = itsMinPressure + change;
		newValue = static_cast<int>(newValue / step) * step;
		if(itsMaxPressure - newValue >= step && newValue >= 1)
			itsMinPressure = newValue;
	}
	else
	{
		// **** MSVC 7.1 BUGI ************
		// Ilman tuota stringi viritystä ei Trajektori-näytön paineasteikon
		// alapään säätö vasemmalla hiiren klikkauksella ei toimi release-versiona.
		// Debug-versio toimii ilmankin.
		std::string bs;
		bs += "bs";
		// **** MSVC 7.1 BUGI ************

		float newValue = itsMaxPressure + change;
		newValue = static_cast<int>(newValue / step) * step;
		if(newValue - itsMinPressure >= step)
			itsMaxPressure = newValue;
	}
}

static double CalcSelectedPressure(double pressure, unsigned long theKey)
{
	double step = 25;
	if(theKey & kCtrlKey) // tehdään yhden mb:n tarkkuudella pyöristys
		step = 1;
	double newValue = round((pressure / step)) * step;
	if(newValue < 1)
		newValue = 1;
	return newValue;
}

bool NFmiTrajectoryView::LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(itsDataRect.IsInside(thePlace))
	{
		double pressure = y2p(thePlace.Y());
		itsCtrlViewDocumentInterface->TrajectorySystem()->SelectedPressureLevel(CalcSelectedPressure(pressure, theKey));
		return true;
	}
	else if(itsDataRect.Left() > thePlace.X() && itsDataRect.Bottom() > thePlace.Y())
	{ // nyt on klikattu paine asteikkoa
		if(itsDataRect.Center().Y() > thePlace.Y())
			ChangePAxisValue(kLeft, true);
		else
			ChangePAxisValue(kLeft, false);
		return true;
	}

	return false;
}

bool NFmiTrajectoryView::RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(itsDataRect.Left() > thePlace.X() && itsDataRect.Bottom() > thePlace.Y())
	{ // nyt on klikattu paine asteikkoa
		if(itsDataRect.Center().Y() > thePlace.Y())
			ChangePAxisValue(kRight, true);
		else
			ChangePAxisValue(kRight, false);
		return true;
	}

	return false;
}

void NFmiTrajectoryView::DrawBackground(void)
{
	itsDrawingEnvironment.SetPenSize(NFmiPoint(1,1));
	itsDrawingEnvironment.EnableFrame();
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(NFmiColor(0.94f,0.92f,0.87f));
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	DrawFrame(itsDrawingEnvironment);

	itsDrawingEnvironment.SetFillColor(NFmiColor(0.99f,0.98f,0.92f));
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	DrawFrame(itsDrawingEnvironment, itsDataRect);
}

void NFmiTrajectoryView::CalcRects(void)
{
	itsPressureScaleFrame = CalcPressureScaleRect();
	itsDataRect = CalcDataRect(); // laske data recti
	itsTimeControlViewRect = CalcTimeControlViewRect();
	if(itsTimeControlView)
	{
		itsTimeControlView->SetFrame(itsTimeControlViewRect);
		itsTimeControlView->Initialize(true, true);
	}
	itsStartPressureLevelMarkerRect = CalcStartPressureLevelMarkerRect();
}

NFmiRect NFmiTrajectoryView::CalcStartPressureLevelMarkerRect(void)
{
	double xSize = itsToolBox->SX(14);
	double ySize = itsToolBox->SX(7);
	double y = p2y(itsCtrlViewDocumentInterface->TrajectorySystem()->SelectedPressureLevel());
	double x = itsDataRect.Left() + xSize/4.;
	NFmiRect rec(0, 0, xSize, ySize);
	rec.Center(NFmiPoint(x, y));
	return rec;
}

NFmiRect NFmiTrajectoryView::CalcPressureScaleRect(void)
{
	NFmiRect axisRect(GetFrame());
	double emptySpaceTop = itsToolBox->SY(3); // vähän tilaa jätetään ylös
	double emptySpaceBottom = itsToolBox->SY(54); // pitää olla tilaa aika ikkunalle
	axisRect.Top(axisRect.Top() + emptySpaceTop); // at the top there is also the time axis (in the future not!)
	axisRect.Bottom(axisRect.Bottom() - emptySpaceBottom);
	axisRect.Right(axisRect.Left() + itsToolBox->SX(static_cast<long>(itsPressureScaleFontSize * 4 * 0.62))); // 4 on neli merkkinen paine arvo esim 1000
	return axisRect;
}

NFmiRect NFmiTrajectoryView::CalcDataRect(void)
{
	double rightMargin = itsToolBox->SX(10);
	NFmiRect rec(itsPressureScaleFrame.Right(), itsPressureScaleFrame.Top(), itsRect.Right() - rightMargin, itsPressureScaleFrame.Bottom());

	return rec;
}

// suoran kaksi pistettä on annettu ja x:n arvo, laske y:n arvo
static double LaskeYSuoralla(double x, double x1, double x2, double y1, double y2)
{
	double k = (y2-y1)/(x2-x1);
	double b = (x1*y2 - y1*x2)/(x1-x2);
	double y = k*x + b;
	return y;
}

static int CalcStretchTimeControlViewByPixels(const NFmiRect &theDataRect, NFmiToolBox *theTB)
{
	int widthInPixels = theTB->HX(theDataRect.Width());
	double value = ::LaskeYSuoralla(widthInPixels, 383, 1161, 7, 24);
	return boost::math::iround(value);
}

// Aikakontrolli-ikkunan varaama alue.
// Pitää laskea vasta kun itsDataRect on laskettu!!
NFmiRect NFmiTrajectoryView::CalcTimeControlViewRect(void)
{
	int usedStretchFactor = CalcStretchTimeControlViewByPixels(itsDataRect, itsToolBox);
	double spaceX = itsToolBox->SX(usedStretchFactor); // ikkunaa pitää leventää hieman datarectiin nähden, koska itse aika-akseli ei men reunoille asti
	double spaceY = itsToolBox->SY(4); // pieni väli datalaatikon ja aikaikkunan välillä
	NFmiRect frame(GetFrame());
	NFmiRect resultRect(itsDataRect.Left() - spaceX, itsDataRect.Bottom() + spaceY, itsDataRect.Right() + spaceX, frame.Bottom());
	return resultRect;
}

void NFmiTrajectoryView::DrawTimeControl(void)
{
	if(itsTimeControlView == 0 || itsCtrlViewDocumentInterface->TrajectorySystem()->TrajectoryViewTimeBagDirty())
		CreateTimeControlView();
	if(itsTimeControlView && itsTimeControlViewRect.Height() > 0.)
		itsTimeControlView->Draw(itsToolBox);

	DrawTimeGridLines();
}


static int CalcUsedTimeStepWithGridLinesInMinutes(const NFmiRect &theDataRect, NFmiToolBox *theTB, const NFmiTimeBag &theTimes)
{
	static bool firstTime = true;
	static std::vector<int> availableTimeStepsInMinutes;
	if(firstTime)
	{
		firstTime = false;
		availableTimeStepsInMinutes.push_back(30);
		availableTimeStepsInMinutes.push_back(60);
		availableTimeStepsInMinutes.push_back(3*60);
		availableTimeStepsInMinutes.push_back(6*60);
		availableTimeStepsInMinutes.push_back(12*60);
		availableTimeStepsInMinutes.push_back(24*60);
		availableTimeStepsInMinutes.push_back(48*60);
	}

	int widthInPixels = theTB->HX(theDataRect.Width());
	double lenghtInMinutes = theTimes.LastTime().DifferenceInMinutes(theTimes.FirstTime());
	std::vector<int>::iterator it = availableTimeStepsInMinutes.begin();
	for(; it != availableTimeStepsInMinutes.end(); ++it)
	{
		double pixelsBetweenHelpLines = widthInPixels/(lenghtInMinutes/(*it));
		if(pixelsBetweenHelpLines > 40.)
			return *it;
	}
	return 72*60; // muuten palautetaan pisin aika
}

void NFmiTrajectoryView::DrawTimeGridLines(void)
{
	NFmiDrawingEnvironment envi;
	envi.SetFillPattern(FMI_DASH);
	envi.SetFrameColor(NFmiColor(0.7f, 0.7f, 0.7f));

	const NFmiTimeBag &times = itsCtrlViewDocumentInterface->TrajectorySystem()->TrajectoryViewTimeBag();
	int usedTimeStep = CalcUsedTimeStepWithGridLinesInMinutes(itsDataRect, itsToolBox, times);

	NFmiMetTime time1(times.FirstTime());
	time1.SetTimeStep(usedTimeStep);
	double startX = Time2X(time1);
	double x = startX;
	double stepX = TimeStepWidth(usedTimeStep);
	for(; time1 < times.LastTime(); time1.NextMetTime())
	{
		NFmiPoint p1(x, itsDataRect.Top());
		NFmiPoint p2(x, itsDataRect.Bottom());
		NFmiLine line1(p1, p2, 0, &envi);
		itsToolBox->Convert(&line1);
		x += stepX;
	}
}

bool NFmiTrajectoryView::CreateTimeControlView(void)
{
	delete itsTimeControlView;
	itsTimeControlView = 0;
	if(itsCtrlViewDocumentInterface->DefaultEditedDrawParam())
	{
		itsTimeControlView = new NFmiTrajectoryTimeControlView(itsMapViewDescTopIndex, itsTimeControlViewRect
													,itsToolBox
													, itsCtrlViewDocumentInterface->DefaultEditedDrawParam());
		itsTimeControlView->Initialize(true, true);
		return true;
	}
	return false;
}

double NFmiTrajectoryView::Time2X(const NFmiMetTime &theTime)
{
	const NFmiTimeBag &times = itsCtrlViewDocumentInterface->TrajectorySystem()->TrajectoryViewTimeBag();
	double totalMinutes = times.LastTime().DifferenceInMinutes(times.FirstTime());
	// lasketaan halutun ajan offsett
	double wantedMinutes = theTime.DifferenceInMinutes(times.FirstTime());
	double offset = wantedMinutes/totalMinutes;
	double x = itsDataRect.Left() + offset * itsDataRect.Width();
	return x;
}

double NFmiTrajectoryView::TimeStepWidth(double timeStepInMinutes)
{
	const NFmiTimeBag &times = itsCtrlViewDocumentInterface->TrajectorySystem()->TrajectoryViewTimeBag();
	double totalMinutes = times.LastTime().DifferenceInMinutes(times.FirstTime());

	double offset = timeStepInMinutes/totalMinutes;
	double width = offset * itsDataRect.Width();
	return width;
}

double NFmiTrajectoryView::y2p(double y)
{
	const double errLimit = 0.001; // onko y framen sisällä pitää sallia pieni virhe raja
	double p = kFloatMissing;
	if(y <= itsDataRect.Bottom() + errLimit && y >= itsDataRect.Top() - errLimit)
	{
		double boxh = itsDataRect.Height();
		double by = itsDataRect.Bottom();
		double dp = ::log(itsMaxPressure) - ::log(itsMinPressure);
		p = itsMaxPressure * ::exp((y-by)/(boxh/dp));
	}
	return p;
}

double NFmiTrajectoryView::p2y(double p)
{
	if(p == kFloatMissing)
		return kFloatMissing;
	double boxh = itsDataRect.Height();
	double by = itsDataRect.Bottom();
	double dp = ::log(itsMaxPressure) - ::log(itsMinPressure);
	return by - (::log(itsMaxPressure) - ::log(p)) * (boxh / dp);
}

void NFmiTrajectoryView::DrawPressureScale(void)
{
	NFmiDrawingEnvironment envi;
	envi.SetFontSize(NFmiPoint(itsPressureScaleFontSize, itsPressureScaleFontSize));
	envi.SetFillPattern(FMI_DASHDOT);
	FmiDirection oldAlignment = itsToolBox->GetTextAlignment();
	itsToolBox->SetTextAlignment(kRight);

	std::vector<double> pValues{ 1000, 925, 850, 700, 500, 300, 150, 100, 50, 20, 10, 5, 2, 1 };
	auto endIt = pValues.end();
	double y = 0;
	double tickMarkLength = itsToolBox->SX(4);
	double moveLabelY = itsToolBox->SY(static_cast<long>(itsPressureScaleFontSize/2));
	for (auto it = pValues.begin(); it != endIt;  ++it)
	{
		double pressure = *it;
		if(pressure <= itsMaxPressure && pressure >= itsMinPressure)
		{
			// tick markin piirto (koko ruudun levyinen apu viiva)
			y = p2y(pressure);
			bool pressureOnBorder = false;
			if(::fabs(y - itsDataRect.Top()) < 0.001 || ::fabs(y - itsDataRect.Bottom()) < 0.001)
				pressureOnBorder = true;
			NFmiPoint p1(itsPressureScaleFrame.Right(), y);
			p1.X(p1.X() - tickMarkLength);
			NFmiPoint p2(pressureOnBorder ? itsDataRect.Left() : itsDataRect.Right(), y); // jos viiva on datalaatikon ala/ylä rajalla, piirretään vain tick mark, muuten koko ruudun ylittävä apuviiva
			NFmiLine line1(p1, p2, 0, &envi);
			itsToolBox->Convert(&line1);

			// label tekstin piirto
			NFmiPoint p3(p1);
			p3.Y(p3.Y() - moveLabelY);
			NFmiString str1(NFmiStringTools::Convert<double>(pressure));
			NFmiText txt1(p3, str1, false, 0, &envi);
			itsToolBox->Convert(&txt1);
		}
	}
	itsToolBox->SetTextAlignment(oldAlignment);
}

void NFmiTrajectoryView::DrawTrajectories(void)
{
	itsToolBox->RelativeClipRect(itsDataRect, true);
	// piirretään trajektorit kartalle
	int index = 0;
    itsCtrlViewDocumentInterface->TrajectorySystem()->MakeSureThatTrajectoriesAreCalculated(); // optimointi koodia viewmakrojen takia....
	const std::vector<boost::shared_ptr<NFmiTrajectory> >& trajectories = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectories();
	std::vector<boost::shared_ptr<NFmiTrajectory> >::const_iterator it = trajectories.begin();
	for( ; it != trajectories.end() ; ++it)
	{
		DrawTrajectory(*(*it).get(), itsCtrlViewDocumentInterface->GeneralColor(index)); // tässä vaiheessa otetaan vielä luotauksista väritykset
		index++;
	}
	itsToolBox->UseClipping(false);
}

// väri otetaan väliaikaisesti ulkoa luotaus systeemistä
void NFmiTrajectoryView::DrawTrajectory(const NFmiTrajectory &theTrajectory, const NFmiColor &theColor)
{
	NFmiDrawingEnvironment envi;

	// piirretään ensin mahdollinen pluumi
	if(theTrajectory.PlumesUsed())
	{
		NFmiColor grayColor(0.9f, 0.9f, 0.9f);
		NFmiColor fadeColor(theColor);
		fadeColor.Mix(grayColor, 0.62f);  // haalennetaan väriä hiukan
		envi.SetFrameColor(fadeColor);
		envi.SetPenSize(NFmiPoint(1, 1));

		const std::vector<boost::shared_ptr<NFmiSingleTrajector> >& plumes = theTrajectory.PlumeTrajectories();
		std::vector<boost::shared_ptr<NFmiSingleTrajector> >::const_iterator it = plumes.begin();
		for( ; it != plumes.end(); ++it)
			DrawSingleTrajector(*(*it).get(), envi, theTrajectory.TimeStepInMinutes(), 5, 1, theTrajectory.Direction());
	}


	// piirretään sitten pää-trajektori
	envi.SetFrameColor(theColor);
	envi.SetPenSize(NFmiPoint(3,3));
	DrawSingleTrajector(theTrajectory.MainTrajector(), envi, theTrajectory.TimeStepInMinutes(), 7, 2, theTrajectory.Direction());
}

static bool IsPointOk(const NFmiPoint &thePoint)
{
	if(thePoint.X() == kFloatMissing || thePoint.Y() == kFloatMissing)
		return false;
	return true;
}

void NFmiTrajectoryView::DrawSingleTrajector(const NFmiSingleTrajector &theSingleTrajector, NFmiDrawingEnvironment &theEnvi, int theTimeStepInMinutes, int theTimeMarkerPixelSize, int theTimeMarkerPixelPenSize, FmiDirection theDirection)
{
	bool forwardDir = (theDirection == kForward);
    NFmiMetTime mapTime(itsCtrlViewDocumentInterface->ActiveMapTime());
	NFmiMetTime time1(theSingleTrajector.StartTime());
	double startX = Time2X(time1);
	double x = startX;
	double stepX = TimeStepWidth(theTimeStepInMinutes);
	if(!forwardDir)
		stepX = -stepX;
	NFmiMetTime time2;
	double pressure1 = theSingleTrajector.StartPressureLevel();
	double pressure2 = kFloatMissing;
	NFmiPoint p2;
	NFmiPoint mapTimeP(kFloatMissing, kFloatMissing); // haetaan tähän se piste, missä trajektori oli menossa kun tämän kartan aika oli
	NFmiMetTime currentTime(theSingleTrajector.StartTime()); // pidetään laskua currentin pisteen ajasta
	const std::vector<float> &pressures = theSingleTrajector.Pressures();
	std::vector<float>::const_iterator it = pressures.begin();
	bool showTrajectoryArrows = itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoryArrows();
	bool showTrajectoryAnimationMarkers = itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoryAnimationMarkers();
	if(showTrajectoryArrows == false && showTrajectoryAnimationMarkers == false)
		return ;
	if(it != pressures.end())
	{
		NFmiPolyline trajectorPolyLine(itsRect, 0, &theEnvi);
		double y = p2y(*it);
		NFmiPoint p1(x, y);
		trajectorPolyLine.AddPoint(p1);
		std::vector<float>::const_iterator endIt = pressures.end();
		++it; // pitää juoksuttaa yhden pykälän verran eteenpäin
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

			if(showTrajectoryArrows) // HUOM! looppia pitää kuitenkin käydä läpi vaikka ei piirretä, jos markerit piirretään
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
			if(pressures.size() >= 2)
			{
				long arrowHeadSize = theTimeMarkerPixelSize;
				double markWidth = itsToolBox->SX(arrowHeadSize);
				double markHeight = itsToolBox->SY(arrowHeadSize);
				NFmiPoint scale(markWidth, markHeight);

				double y2 = p2y(pressures[pressures.size()-2]);
				double deltaY = y - y2;
				double vdirRad = ::atan2(deltaY, stepX);
				double vdir1 = vdirRad * 360 / (2. * kPii); // ja siitä suunta viimeiseen paikkaan
				vdir1 += 90;
				if(forwardDir)
					vdir1 = ::fmod(vdir1, 360);
				else
					vdir1 = ::fmod(vdir1+180, 360); // käännetään nuolen suunta 180 astetta jos takaperin trajektori
				// piirrä etenemis nuolen kärki trajektorille
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
