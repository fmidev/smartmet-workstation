//© Ilmatieteenlaitos/Marko.
//Original 1.12.1999
// 
//Ver. 1.12.1999
//---------------------------------------------------------- NFmiAdjustedTimeScaleView.cpp

#include "NFmiAdjustedTimeScaleView.h"
#include "ToolboxViewsInterface.h"
#include "NFmiToolBox.h"
#include "NFmiRectangle.h"
#include "NFmiLine.h"
#include "NFmiStepTimeScale.h"
#include "NFmiTimeScale.h"
#include "NFmiValueString.h"
#include "NFmiText.h"
#include "NFmiDictionaryFunction.h"
#include "MapViewMode.h"

#include "boost\math\special_functions\round.hpp"

NFmiAdjustedTimeScaleView::NFmiAdjustedTimeScaleView(int theMapViewDescTopIndex, const NFmiRect &theRect
													,NFmiToolBox *theToolBox
													,NFmiStepTimeScale *theBaseScale
													,NFmiStepTimeScale *theSelectedScale
													,NFmiView *theEnclosure
													,NFmiDrawingEnvironment *theEnvironment
													,unsigned long theIdent
													,bool isUpsideDown
													,const NFmiPoint& theFontSize
													,bool drawSelectedTimes
                                                    ,bool useActiveMapTime)
:NFmiTimeScaleView(theRect
					,theToolBox
					,theBaseScale
					,theSelectedScale
					,theEnclosure
					,theEnvironment
					,theIdent
					,isUpsideDown)
					,itsFontSize(theFontSize)
,itsToolboxViewsInterface(0)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,fDrawSelectedTimes(drawSelectedTimes)
,fUseActiveMapTime(useActiveMapTime)
{
    if(ToolboxViewsInterface::GetToolboxViewsInterfaceCallBack)
        itsToolboxViewsInterface = ToolboxViewsInterface::GetToolboxViewsInterfaceCallBack();
    else
        throw std::runtime_error("NFmiAdjustedTimeScaleView::NFmiAdjustedTimeScaleView - ToolboxViewsInterface getter not set");

	itsTickMarkStepsLimitsInMinutes.push_back(5);
	itsTickMarkStepsLimitsInMinutes.push_back(15);
	itsTickMarkStepsLimitsInMinutes.push_back(30);
	itsTickMarkStepsLimitsInMinutes.push_back(1*60);	// 1 h
	itsTickMarkStepsLimitsInMinutes.push_back(3*60);	// 3 h
	itsTickMarkStepsLimitsInMinutes.push_back(6*60);	// 6 h
	itsTickMarkStepsLimitsInMinutes.push_back(12*60);	// 12 h
	itsTickMarkStepsLimitsInMinutes.push_back(24*60);	// 1 vrk
	itsTickMarkStepsLimitsInMinutes.push_back(48*60);	// 2 vrk
	itsTickMarkStepsLimitsInMinutes.push_back(168*60);	// 1 vk
	itsTickMarkStepsLimitsInMinutes.push_back(30*24*60); // 1 kk
}
                                 
NFmiAdjustedTimeScaleView::~NFmiAdjustedTimeScaleView(void)
{
}

int NFmiAdjustedTimeScaleView::GetTimeControltimeStep(void) const
{
	return static_cast<int>(::round(itsToolboxViewsInterface->TimeControlTimeStepInHours(itsMapViewDescTopIndex) * 60.));
}

NFmiRect NFmiAdjustedTimeScaleView::CalcSelectedTimeArea(const NFmiMetTime &theTime) const
{
	int resolution = GetTimeControltimeStep();
	if(resolution > 60)
		resolution = 60;

	NFmiMetTime startTime(theTime);
	NFmiMetTime endTime(theTime);
	startTime.ChangeByMinutes(-resolution/2);
	endTime.ChangeByMinutes(resolution/2);

	double maxWidthInMM = 7.5;
	int maxPixelWidth = static_cast<int>(maxWidthInMM * itsToolboxViewsInterface->ContextPixelsPerMM_x(itsMapViewDescTopIndex));

	double maxWidth = itsToolBox->SX(maxPixelWidth); 
	NFmiRect timeAreaRect = CalcTimeArea(startTime, endTime, maxWidth);
	return timeAreaRect;
}

NFmiRect NFmiAdjustedTimeScaleView::CalcTotalTimeScaleArea(void) const
{
	NFmiRect totalArea = GetFrame();
	NFmiRect scaleArea = CalcScaleArea();
	totalArea.Left(scaleArea.Left());
	totalArea.Right(scaleArea.Right());

	return totalArea;
}

void NFmiAdjustedTimeScaleView::DrawSelectedTimes(void)
{
	if(fDrawSelectedTimes)
	{
		NFmiRect timeScaleRect = CalcTotalTimeScaleArea();
		GetToolBox()->RelativeClipRect(timeScaleRect, true);
		NFmiDrawingEnvironment envi;
		envi.EnableFrame();
		envi.SetFontType(kArial);
		envi.SetFrameColor(NFmiColor(.95f,0,0));
		envi.SetFillColor(NFmiColor(0,1,0));
		envi.EnableFill();

		int selectedTimeResolution = GetTimeControltimeStep();
		int viewCount = int(itsToolboxViewsInterface->MapViewGridSize(itsMapViewDescTopIndex).X());
        CtrlViewUtils::MapViewMode displayMode = itsToolboxViewsInterface->MapViewDisplayMode(itsMapViewDescTopIndex);
		if(displayMode == CtrlViewUtils::MapViewMode::kOneTime)
			viewCount = 1;
		else if(displayMode == CtrlViewUtils::MapViewMode::kRunningTime)
			viewCount = static_cast<int>(itsToolboxViewsInterface->MapViewGridSize(itsMapViewDescTopIndex).X() * itsToolboxViewsInterface->MapViewGridSize(itsMapViewDescTopIndex).Y());

        NFmiMetTime tmpTime = fUseActiveMapTime ? itsToolboxViewsInterface->ActiveMapTime() : itsToolboxViewsInterface->CurrentMapTime(itsMapViewDescTopIndex);
		for(int i = 0; i < viewCount; i++)
		{
			NFmiRect subScaleRect = CalcSelectedTimeArea(tmpTime);

			NFmiRectangle tmp(subScaleRect.TopLeft(),
					  subScaleRect.BottomRight(),
					  0,
					  &envi);
			GetToolBox()->Convert(&tmp);

			tmpTime.ChangeByMinutes(selectedTimeResolution);
		}
		GetToolBox()->UseClipping(false);
	}
}


NFmiVoidPtrList& NFmiAdjustedTimeScaleView::CollectOwnDrawingItems(NFmiVoidPtrList& theDrawingList)
{
	if(!itsBaseScale)
		return theDrawingList;

	NFmiRect timeScaleRect = CalcTotalTimeScaleArea();
	NFmiDrawingEnvironment envi;
	envi.EnableFrame();
	envi.SetFrameColor(NFmiColor(0, 0, 0));
	NFmiRectangle scaleRec(timeScaleRect, 0, &envi);
	GetToolBox()->Convert(&scaleRec);
	GetToolBox()->RelativeClipRect(timeScaleRect, true);

	NFmiVoidPtrList& list = DrawScale(theDrawingList, itsBaseScale, CalcScaleArea());
	GetToolBox()->UseClipping(false);

	return list;
}

NFmiRect NFmiAdjustedTimeScaleView::CalcTimeArea(const NFmiMetTime& startTime, const NFmiMetTime& endTime, double theMaxWidth) const
{    
  NFmiRect baseScaleRect(CalcScaleArea());
  baseScaleRect.Height(baseScaleRect.Height() + theMaxWidth/20.); // korotetaan hieman SmartMEtin vihreän laatikon korkeutta

  double startX = ((NFmiTimeScale*)itsBaseScale->Scale())->RelTimeLocation(startTime) * baseScaleRect.Width();
  double endX = ((NFmiTimeScale*)itsBaseScale->Scale())->RelTimeLocation(endTime) * baseScaleRect.Width();

  NFmiRect subScaleRect(baseScaleRect.TopLeft() + NFmiPoint(startX, 0)
                       ,baseScaleRect.BottomLeft() + NFmiPoint(endX, 0));

  subScaleRect.Inflate(-subScaleRect.Width()*0.07, 0);
  subScaleRect += NFmiPoint(0,subScaleRect.Height()*0.25);
  if(theMaxWidth)
  {
	  if(subScaleRect.Width() > theMaxWidth)
	  {
		  NFmiPoint center(subScaleRect.Center());
		  NFmiPoint newSize(theMaxWidth, subScaleRect.Height());
		  subScaleRect.Size(newSize);
		  subScaleRect.Center(center);
	  }
  }
// lasketaan laatikolle vielä minimi koko, ettei katoa ihan kokonaan näytöltä
	NFmiPoint sizeOfRect(subScaleRect.Size());
	NFmiPoint centerOfRect(subScaleRect.Center());
	int xPix = itsToolBox->HX(sizeOfRect.X());
	int yPix = itsToolBox->HY(sizeOfRect.Y());
	if(xPix < 8)
		sizeOfRect.X(itsToolBox->SX(8));
	if(yPix < 8)
		sizeOfRect.Y(itsToolBox->SY(8));
	subScaleRect.Size(sizeOfRect);
	subScaleRect.Center(centerOfRect);

  return subScaleRect;
}

void NFmiAdjustedTimeScaleView::CalcTickMarkHelperValues(const NFmiRect &theScaleRect, const NFmiPoint &theFontSize, double &y1, double &y2, double &y3, double &y4)
{
	if(fUpsideDown)
	{
		y1 = theScaleRect.Top();
		y2 = theScaleRect.Bottom();
		y3 = y2 + theFontSize.Y() * 0.1;
		y4 = y3 + theFontSize.Y() * 0.74;
	}
	else
	{
		y1 = theScaleRect.Bottom();
		y2 = theScaleRect.Top();
		y3 = y2;
		y4 = y2-theFontSize.Y();
	}
}


// Laskee 'normaali' aika-akselinäytön leveyden suhteessa currenttiin tilanteeseen.
// Käytetään harventamaan liian kaposten näyttöjen lukemia ja tickmarkkeja.
// Normaali näytön leveys tarkoittaa leveyttä, millä akselit näyttävät muuten
// ilamn tätä harvennusta ihan ok:lta.
double NFmiAdjustedTimeScaleView::GetNormalToCurrentViewPixelRatio(void)
{
	// tämä luku kertoo aika-akselin suht noormaalin leveyden (pikseleissä), 
	// jolloin akselin luvut ja tickmarkit ovat suht. selkeitä. tämän
	// luvun avulla on tarkoitus alkaa harventaa lukuja ja tickmarkkeja (lisää)
	// jos aika-akseli näytön leveys on riittävän verran kapeampi kuin tämä.
	static double gNormalScaleWidthInPixels = 500;

	int longWayPixelCount = itsToolBox->HX(GetFrame().Width());
	double ratio = gNormalScaleWidthInPixels / longWayPixelCount;
	return ratio;
}

// palauttaa luvun, kuinka monella tasolla skippi arvoja on tarkoitus korottaa
// riippuen ratiosta.
static int GetSkipAdvance(double theRatio)
{
	if(theRatio > 7.)
		return 5;
	else if(theRatio > 4.4)
		return 4;
	else if(theRatio > 2.7)
		return 3;
	else if(theRatio > 1.8)
		return 2;
	else if(theRatio > 1.3)
		return 1;
	return 0;
}

static int GetFinalSkipValue2(int theCurrentValue, int theAdvanceValue, std::vector<int> &theValues, int theMaxValue)
{
	std::vector<int>::iterator it = std::find(theValues.begin(), theValues.end(), theCurrentValue);
	int index = static_cast<int>(it - theValues.begin());
	index += theAdvanceValue;
	if(index  < static_cast<int>(theValues.size()))
		return theValues[index];
	else
		return theMaxValue;
}

// paluttaa lopullisen skippi arvon
int NFmiAdjustedTimeScaleView::GetFinalSkipValue(int theCurrentValue, std::vector<int> &theValues, int theMaxValue)
{
	double ratio = GetNormalToCurrentViewPixelRatio();
	if(theCurrentValue < theMaxValue && ratio > 1.)
	{
		int advanceValue = GetSkipAdvance(ratio);
		if(advanceValue)
			return GetFinalSkipValue2(theCurrentValue, advanceValue, theValues, theMaxValue);
	}
	return theCurrentValue;
}

// lasketaan, millä jaolliset tunnit piirretään aikaikkunaan
// Tarkoitus on harventaa printattavia lukuja, riippuen
// aika-akselin koosta/timestepista eli theStepNumber:ista
// ja näytön koosta
int NFmiAdjustedTimeScaleView::CalcSkipHourPrintValue(int theStepNumber) // theStepNumber eli oikeasti kuinka monta tuntia on koko aika-kontrollin alueella (=hoursInTimeScale)
{
	static const int SSIZE = 4;
	static int values[SSIZE] = {1,3,6,12};
	static std::vector<int> vvalues(values, values+SSIZE);
	static int maxValue = 24; 

	int skipHourPrintUnlessDivident = 1;
	if(theStepNumber < 10)
		skipHourPrintUnlessDivident = 1;
	else if(theStepNumber < 30)
		skipHourPrintUnlessDivident = 3;
	else if(theStepNumber < 50)
		skipHourPrintUnlessDivident = 6;
	else if(theStepNumber < 120)
		skipHourPrintUnlessDivident = 12;
	else
		skipHourPrintUnlessDivident = maxValue;
	return GetFinalSkipValue(skipHourPrintUnlessDivident, vvalues, maxValue);
}

// lasketaan, millä jaolliset tunteihin piirretään aikaikkunaan merkillä
// Tarkoitus on harventaa piirrettäviä tickmarkkeja, riippuen
// aika-akselin koosta/timestepista eli theStepNumber:ista
// ja näytön koosta
int NFmiAdjustedTimeScaleView::CalcSkipTickMarkValue(int theStepNumber) // theStepNumber eli oikeasti kuinka monta tuntia on koko aika-kontrollin alueella (=hoursInTimeScale)
{
	static const int SSIZE = 4;
	static int values[SSIZE] = {1,3,6,12};
	static std::vector<int> vvalues(values, values+SSIZE);
	static int maxValue = 24; 

	int skipTickMarkUnlessDivident = 1; 
	if(theStepNumber < 50)
		skipTickMarkUnlessDivident = 1;
	else if(theStepNumber < 100)
		skipTickMarkUnlessDivident = 2;
	else if(theStepNumber < 150)
		skipTickMarkUnlessDivident = 3;
	else if(theStepNumber < 255)
		skipTickMarkUnlessDivident = 6;
	else if(theStepNumber < 395)
		skipTickMarkUnlessDivident = 12;
	else// if(theStepNumber < 335)
		skipTickMarkUnlessDivident = 24;
	return GetFinalSkipValue(skipTickMarkUnlessDivident, vvalues, maxValue);
}

int NFmiAdjustedTimeScaleView::CalcSkipDaysValue(int theTotalDayCount)
{
	int skipDays = 0;
	if(theTotalDayCount >= 28)
	{
		if(theTotalDayCount < 65)
			skipDays = 2;
		else if(theTotalDayCount < 160)
			skipDays = 5;
		else if(theTotalDayCount < 310)
			skipDays = 10;
		else if(theTotalDayCount < 510)
			skipDays = 30;
		else
			skipDays = 60;
	}
	return skipDays;
}

double NFmiAdjustedTimeScaleView::RelativeXToMM(double theRelativeXLength)
{
	double widthInPixels = itsToolBox->HX(theRelativeXLength);
	double widthInMM = widthInPixels / itsToolboxViewsInterface->ContextPixelsPerMM_x(itsMapViewDescTopIndex);
	return widthInMM;
}

double NFmiAdjustedTimeScaleView::CalcTimeScaleWidthInMM(void)
{
	return RelativeXToMM(CalcScaleArea().Width());
}

// Laskee aikaikkunan todellisen leveyden ja aikavälin avulla sopivat major- ja minor -tick markkien resoluutiot minuuteissa.
// Major-tick mark on se merkki johon tulee tunti lukema.
// Minor-tick on merkki johon tulee viiva (ellei sse ole myös major samalla)
// Kun Major-step on laskettu, minor on aina yksi pykälä siitä pienempi tai jos ollaan jo alarajalla, se on sitten sama.
void NFmiAdjustedTimeScaleView::GetTimeSteps(NFmiDrawingEnvironment &theDrawingEnvi, int &theMajorStepInMinutesOut, int &theMinorStepInMinutesOut)
{
	// 1. Calc width of time scale in [mm]
	double scaleWidthInMM = CalcTimeScaleWidthInMM();
	// 2. Calc width of string "00" and "00:00" in [mm]
	double shortLabelLenInMM = 2 * theDrawingEnvi.GetFontSize().X() / (itsToolboxViewsInterface->ContextPixelsPerMM_x(itsMapViewDescTopIndex) * 1.88);
	double longLabelLenInMM = 5 * theDrawingEnvi.GetFontSize().X() / (itsToolboxViewsInterface->ContextPixelsPerMM_x(itsMapViewDescTopIndex) * 1.88);
	// 3. Calc width of timeview in minutes
    int minutesInTimeScale = boost::math::iround(itsBaseScale->StepNumber() * itsBaseScale->StepValue());
    int scaleStepInMinutes = boost::math::iround(itsBaseScale->StepValue());
	size_t indexCounter = 0;
	for(indexCounter = 0; indexCounter < itsTickMarkStepsLimitsInMinutes.size(); indexCounter++)
	{
		int currentStepValue = itsTickMarkStepsLimitsInMinutes[indexCounter];
		bool useShortLabel = (currentStepValue >= 60);
	// 4. Calc number of labels with each timestep-values from itsTickMarkStepsLimitsInMinutes
		double labelCount = minutesInTimeScale / static_cast<double>(currentStepValue);
	// 5. Calc the combined width of labels
		double combinedLabelWidthInMM = labelCount * (useShortLabel ? shortLabelLenInMM : longLabelLenInMM);
	// 6. Calc ratio between combined labels and timescale-widht in mm
		double combinedLabelsToScaleRatio = combinedLabelWidthInMM / scaleWidthInMM;
	// 7. Use this ratio to deside if this is proper major step, if no, test next step
		if(combinedLabelsToScaleRatio < 0.7)
			break;
	}
	// 8. When proper major step is determined, the minor step will be one below that
	if(indexCounter >= itsTickMarkStepsLimitsInMinutes.size())
		indexCounter = itsTickMarkStepsLimitsInMinutes.size()-1; // jos ei löytynyt sopivaa steppiä, laitetaan sitten tuo viimeinen steppi päälle
	theMajorStepInMinutesOut = itsTickMarkStepsLimitsInMinutes[indexCounter];
	if(indexCounter > 0)
		theMinorStepInMinutesOut = itsTickMarkStepsLimitsInMinutes[indexCounter-1];
	else
		theMinorStepInMinutesOut = itsTickMarkStepsLimitsInMinutes[0]; // pakka laitta tähänkin ensimmäinen steppi vaihtoehto
}

double NFmiAdjustedTimeScaleView::Time2Value(const NFmiMetTime& theTime) const
{
	double value = ((NFmiTimeScale*)itsBaseScale->Scale())->RelTimeLocation(theTime);
	NFmiRect rect(GetFrame());
	double finalValue = rect.Left() + value * rect.Width();
	return finalValue;
}

void NFmiAdjustedTimeScaleView::DrawWallClockRectangle(void)
{
	NFmiDrawingEnvironment theEnvironment;
	// piirretään ensin 'seinäkello' siten että piirretään asteikon pohja vaalen vihreällä nykyhetkestä tulevaisuuteen.
	NFmiMetTime currentTime(1);
	NFmiRect wallClockRec = GetFrame();
	wallClockRec.Top(wallClockRec.Top() + itsToolBox->SY(1)); // siirretään yläreunaa yhden pikselin verran alaspäin
	wallClockRec.Bottom(wallClockRec.Bottom() - itsToolBox->SY(1)); // siirretään alareunaa yhden pikselin verran ylöspäin
	double wallTimeLocation = Time2Value(currentTime);
	if(wallClockRec.IsInside(NFmiPoint(wallTimeLocation, wallClockRec.Center().Y())))
		wallClockRec.Left(wallTimeLocation);
	if(wallTimeLocation < wallClockRec.Right())
	{ // piirretään seinäkello värjäys, koska 'tulevaisuus' aikaa tulee ainakin jonkin verran näkyviin
		theEnvironment.SetFillColor(NFmiColor(0.86f,0.92f,0.85f));
		theEnvironment.DisableFrame();
		theEnvironment.EnableFill();
		NFmiRect timeScaleRect = CalcTotalTimeScaleArea(); // rajoitetaan piirto aikaskaalaan alueelle ja siitä vielä pikselin verran sisään
		if(wallClockRec.Left() <= timeScaleRect.Left())
		{
			wallClockRec.Left(timeScaleRect.Left());
			wallClockRec.Left(wallClockRec.Left() + itsToolBox->SX(1));
		}

		if(wallClockRec.Right() >= timeScaleRect.Right())
		{
			wallClockRec.Right(timeScaleRect.Right());
			wallClockRec.Right(wallClockRec.Right() - itsToolBox->SX(1));
		}

		NFmiRectangle wallClockRectangle(wallClockRec, 0, &theEnvironment);
		GetToolBox()->Convert(&wallClockRectangle);
	}
}

static int GetMinutesOfTheDay(const NFmiTime &theTime)
{
	int minutesOfDay = theTime.GetHour() * 60 + theTime.GetMin();
	return minutesOfDay;
}

enum TimeLabelPrintMode
{
	kFmiLabelModeNormal = 0,
	kFmiLabelModeFirstMainTime = 1, // ensimmäinen 00 tai 12
	kFmiLabelModeFirstSynopTime = 2, // ensimmäinen 00, 03, 06, 09, 12, 15, 18, 21
	kFmiLabelModeFirstFullHour = 3
};

NFmiVoidPtrList& NFmiAdjustedTimeScaleView::DrawScale(NFmiVoidPtrList &theDrawingList
													,NFmiStepTimeScale *theScale
													,const NFmiRect &theRect)
{
	DrawWallClockRectangle();
	NFmiDrawingEnvironment theEnvironment;
	theEnvironment.SetFrameColor(NFmiColor(0.4f,0.0f,0.0f));
	theEnvironment.EnableFrame();
	theEnvironment.SetFontSize(itsFontSize);
	theEnvironment.SetFontType(kArial);

	FmiDirection oldAlignment = GetToolBox()->GetTextAlignment();
	GetToolBox()->SetTextAlignment(kCenter);

	int majorStepInMinutes = 0;
	int minorStepInMinutes = 0;
	GetTimeSteps(theEnvironment, majorStepInMinutes, minorStepInMinutes);

	double y1, y2, y3, y4; //y4 laura 27041999
	NFmiPoint fontSize(GetToolBox()->ToViewPoint(long(itsFontSize.X()), long(itsFontSize.Y())));
	CalcTickMarkHelperValues(theRect, fontSize, y1, y2, y3, y4);

	NFmiLine tmpline(NFmiPoint(theRect.Left(), y1),
			 NFmiPoint(theRect.Right(), y1),
			 0,
			 &theEnvironment);
	GetToolBox()->Convert(&tmpline);

	if(theScale)
	{
		theScale->Reset();
        int minutesInTimeScale = boost::math::iround(theScale->StepNumber() * theScale->StepValue());
		TimeLabelPrintMode timeLabelPrintMode = kFmiLabelModeNormal;
		if(minutesInTimeScale < 3*60)
			timeLabelPrintMode = kFmiLabelModeFirstFullHour;
		else if(minutesInTimeScale < 12*60)
			timeLabelPrintMode = kFmiLabelModeFirstSynopTime;
		else if(minutesInTimeScale < 24*60)
			timeLabelPrintMode = kFmiLabelModeFirstMainTime;

		int daysTotalInData = static_cast<int>(minutesInTimeScale / (24.*60.));
		int skipDays = 0;
		if(majorStepInMinutes > 24*60)
            skipDays = boost::math::iround(majorStepInMinutes / (24 * 60.));

		NFmiMetTime lastDate(NFmiMetTime::gMissingTime);
		bool datePrinted = false;
		float cursor;
		float endOfCursor = theScale->EndValue();
		NFmiMetTime lastPrintedMinorTickMark(NFmiMetTime::gMissingTime);
		NFmiMetTime lastPrintedMajorTickMark(NFmiMetTime::gMissingTime);
		NFmiMetTime aTime(theScale->StartTime(), 1);

//		while(theScale->Next(cursor)) // theScale -> next -looppi antaa cursor:ille arvot: 0, stepInMinutes, 2*stepInMinutes, 3*stepInMinutes, jne.
		float usedStepInMinutes = FmiMin(theScale->StepValue(), static_cast<float>(minorStepInMinutes));
		for(cursor = theScale->StartValue(); cursor <= endOfCursor; cursor += usedStepInMinutes, aTime.ChangeByMinutes(static_cast<long>(usedStepInMinutes)))
		{
			if(cursor == 0 && theScale->StepValue() == 0)
				break; // tässä on timebagi ollut tyhjä, ja jää iki looppiin, ellei poikkaista
			double x = theRect.Left() + theScale->Scale()->RelLocation(cursor) * theRect.Width();

			if(aTime.GetYear() != lastDate.GetYear() || aTime.GetMonth() != lastDate.GetMonth() || aTime.GetDay() != lastDate.GetDay())
				datePrinted = false;

			// 1. määrää piirtoväri, joka toinen päivä punainen ja joka toinen sininen. Ota väri suoraan päivämäärän päivän numeron mukaan, vaikka siinä on tiettyjä huonoja puolia.
			int odd = aTime.GetDay()%2;
			theEnvironment.SetFrameColor(NFmiColor(odd*1.0f, 0.0f,(1-odd)*1.0f));

			// 2. Piirrä tick-mark (= minor aika merkki, joka voi olla myös major-merkin kohdalla)
			if(GetMinutesOfTheDay(aTime) % minorStepInMinutes == 0)
			{
				if(minorStepInMinutes <= 24*60 || aTime.DifferenceInMinutes(lastPrintedMinorTickMark) >= minorStepInMinutes)
				{
					NFmiLine tmpline(NFmiPoint(x, y1), NFmiPoint(x, y2), 0, &theEnvironment);
					GetToolBox()->Convert(&tmpline);
					lastPrintedMinorTickMark = aTime;
				}
			}

			// 3. Piirrä tunti/minuutti-lukema (= major aika merkki)
			if(GetMinutesOfTheDay(aTime) % majorStepInMinutes == 0)
			{
				if(majorStepInMinutes <= 24*60 || aTime.DifferenceInMinutes(lastPrintedMajorTickMark) >= majorStepInMinutes)
				{
					NFmiLine tmpline(NFmiPoint(x, y1), NFmiPoint(x, y2 + (y2 - y1) * 0.17), 0, &theEnvironment);
					GetToolBox()->Convert(&tmpline);

					NFmiString timeLabel = (majorStepInMinutes >= 60) ? aTime.ToStr("HH") : aTime.ToStr("HH:mm");
					NFmiText tmptext(NFmiPoint(x, y3), timeLabel, false, 0, &theEnvironment);
					GetToolBox()->Convert(&tmptext);
					lastPrintedMajorTickMark = aTime;

					// 4. Piirrä päiväys aikaleima ja viikopäivä Jos mahdollista laita päivämää klo 12 utc kohtaan, 
					// mutta jos majorStep on suurempi kuin 12h, laita leima 00-kohtaan.
					if(datePrinted)
						continue;
					bool printDateNow = false; // tässä hanskataan pri erikois tapausta
					if(timeLabelPrintMode != kFmiLabelModeNormal)
					{
						if(timeLabelPrintMode == kFmiLabelModeFirstFullHour && aTime.GetMin() == 0)
							printDateNow = true;
						if(timeLabelPrintMode == kFmiLabelModeFirstSynopTime && aTime.GetHour() % 3 == 0)
							printDateNow = true;
						if(timeLabelPrintMode == kFmiLabelModeFirstMainTime && aTime.GetHour() % 12 == 0)
							printDateNow = true;
					}

					if(printDateNow || (majorStepInMinutes <= 12*60 && aTime.GetHour() == 12) || (majorStepInMinutes > 12*60 && aTime.GetHour() == 0))
					{
						NFmiString dateString(aTime.ToStr(NFmiString(::GetDictionaryString("TimeScaleViewDateStr"))));
						NFmiText tmptext(NFmiPoint(x, y4), dateString, false, 0, &theEnvironment);
						GetToolBox()->Convert(&tmptext);

						// piirretään myös viikon päivä päiväyksen alle
						NFmiString weekDayStr("Www");
						if(itsToolboxViewsInterface->Language() == kFinnish)
							weekDayStr = "Ww";
						NFmiString weekDayString(aTime.ToStr(weekDayStr, itsToolboxViewsInterface->Language()));
						NFmiText tmptext2(NFmiPoint(x, y4 + (y4-y3)), weekDayString, false, 0, &theEnvironment);
						GetToolBox()->Convert(&tmptext2);
						datePrinted = true;
						lastDate = aTime;
					}
				}
			}
		}
	}
	GetToolBox()->SetTextAlignment(oldAlignment);
	return theDrawingList;
}

