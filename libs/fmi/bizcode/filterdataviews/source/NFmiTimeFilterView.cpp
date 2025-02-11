// NFmiTimeFilterView.cpp: implementation for the NFmiTimeFilterView class.
//
//////////////////////////////////////////////////////////////////////

#include "NFmiTimeFilterView.h"
#include "NFmiToolBox.h"
#include "NFmiLine.h"
#include "NFmiRectangle.h"
#include "NFmiValueString.h"
#include "NFmiText.h"
#include "NFmiFastQueryInfo.h"
#include "CtrlViewDocumentInterface.h"

NFmiTimeFilterView::NFmiTimeFilterView(NFmiToolBox * theToolBox
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , const NFmiRect& theRect
    , int theIndex)
    :NFmiZoomView(0, theToolBox, theDrawParam, theRect)
    , itsFilterRelativeTimeRect()
    , itsGridXSize(1)
    , itsGridXOrigo(0)
    , itsIndex(theIndex)
    , itsExtraHourTimeLineCount(0)
{
    const NFmiPoint& ranges = itsCtrlViewDocumentInterface->TimeFilterLimits();
    itsGridXSize = int(ranges.Y() - ranges.X());
    itsGridXOrigo = int(ranges.X());
}

NFmiTimeFilterView::~NFmiTimeFilterView()
{
}

void NFmiTimeFilterView::Update(void)
{
    itsExtraHourTimeLineCount = 0; // 'resetoidaan' ensin
    boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsCtrlViewDocumentInterface->EditedSmartInfo();
    if(editedInfo)
    {
        editedInfo->FirstTime(); // Varmistetaan että ollaan 1. aika-askeleessa, jotta aikaresoluutio on aina sama samalle datalle (eikä satunnainen, jos vaihtuva resoluutioinen data ja aika-indeksi on joku muu kuin 0)
        if(editedInfo->TimeResolution() > 60) // piirretään apu-tunti-viivat jos resoluutio on yli tunnin
            itsExtraHourTimeLineCount = int(editedInfo->TimeResolution() / 60);
    }
}

void NFmiTimeFilterView::Draw(NFmiToolBox * theGTB)
{
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	itsDrawingEnvironment.SetPenSize(NFmiPoint(1,1));
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(NFmiColor(0.9f,0.9f,0.9f));
	DrawFrame(itsDrawingEnvironment);
	NFmiRect viewRect(GetFrame());

	double timeGridHeight = 0.6;
	for(int i=0; i<itsGridXSize; i++)
	{
		NFmiPoint startingPoint(i * (viewRect.Width()/itsGridXSize), timeGridHeight);
		NFmiPoint endingPoint(i * (viewRect.Width()/itsGridXSize), viewRect.Bottom());
		NFmiLine line(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&line);
	}
	NFmiPoint point1(viewRect.Left(), timeGridHeight);
	NFmiPoint point2(viewRect.Right(), timeGridHeight);
	NFmiLine line(point1, point2, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&line); // piirtää keskelle vaaka viivan

	// piirtää pienen väkäsen 0-kohdalle
	NFmiPoint point3(0.5, timeGridHeight - 0.2); 
	NFmiPoint point4(0.5, timeGridHeight - 0.02);
	NFmiLine line2(point3, point4, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&line2);

	NFmiRect middleGridPointRect(0, 0, viewRect.Width()/15, viewRect.Height()/3);
	middleGridPointRect.Center(NFmiPoint(0.5, 0.8));
	itsDrawingEnvironment.SetFillColor(NFmiColor(1,0,0));
	NFmiRectangle middlePoint(middleGridPointRect.TopLeft(), middleGridPointRect.BottomRight(), 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&middlePoint);

	NFmiPoint timeRanges(itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex));
	NFmiRect timeFilterRect(timeRanges.X(), timeGridHeight, timeRanges.Y(), viewRect.Bottom());
	if(timeRanges.X() == timeRanges.Y())
		timeFilterRect.Inflate(0.2, 0);
	double x1 = (timeFilterRect.Left() - itsGridXOrigo) * (viewRect.Width()/itsGridXSize);
	double x2 = (timeFilterRect.Right() - itsGridXOrigo) * (viewRect.Width()/itsGridXSize);
	double y1 = (timeFilterRect.Top()) * (viewRect.Height());
	double y2 = (timeFilterRect.Bottom()) * (viewRect.Height());

	NFmiPoint startingPoint = NFmiPoint(x1,y1);
	NFmiPoint endingPoint = NFmiPoint(x2,y2);
	itsFilterRelativeTimeRect = NFmiRect(startingPoint, endingPoint); // laitetaan relative recti talteen hiiri-tarkasteluja varten
	itsDrawingEnvironment.DisableFill();
	itsDrawingEnvironment.SetPenSize(NFmiPoint(2,2));
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,1));
	NFmiRectangle filterRectangle(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&filterRectangle);
	DrawHourLines();
	DrawTimeTexts();
}

// jos datan resoluutio on yli tunnin, piirretään lisä viivat jokaisen tunnin kohdalle
void NFmiTimeFilterView::DrawHourLines(void)
{
	if(itsExtraHourTimeLineCount >= 2)
	{
		NFmiRect viewRect(GetFrame());
		double timeStepLength = viewRect.Width()/itsGridXSize;
		double hourStepLength = viewRect.Width()/itsGridXSize/itsExtraHourTimeLineCount;
		double timeGridHeight = 0.9;
		itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
		itsDrawingEnvironment.SetPenSize(NFmiPoint(1,1));
		for(int i=0; i<itsGridXSize; i++)
		{
			NFmiPoint startingPoint(i * timeStepLength, timeGridHeight);
			NFmiPoint endingPoint(i * timeStepLength, viewRect.Bottom());
			for(int j = 0; j < itsExtraHourTimeLineCount; j++)
			{
				startingPoint.X(startingPoint.X() + hourStepLength);
				endingPoint.X(endingPoint.X() + hourStepLength);
				NFmiLine line(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
				itsToolBox->Convert(&line);
			}
		}
	}
}

bool NFmiTimeFilterView::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	double left = itsFilterRelativeTimeRect.Left();
	double xDiff = left-thePlace.X();
	if(xDiff >= 0. && xDiff < 0.05)
	{
		itsDragTarget = kLeft;
		return true;
	}
	double right = itsFilterRelativeTimeRect.Right();
	double xDiff2 = fabs(right-thePlace.X());
	if(xDiff2 < 0.02)
	{
		itsDragTarget = kRight;
		return true;
	}
	if(xDiff < 0. && xDiff > -0.05)
	{
		itsDragTarget = kLeft;
		return true;
	}
	if(itsFilterRelativeTimeRect.IsInside(thePlace))
	{
		itsDragTarget = kCenter;
		return true;
	}
	itsDragTarget = kBase; // tarkoittaa, että kursori oli sinisen laatikon ulkopuolella
	return true;
}

bool NFmiTimeFilterView::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(theKey & kCtrlKey) // select constant area
	{
		NFmiPoint timeRanges(itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex));
		if(timeRanges.Y() - timeRanges.X() >= 1)
			timeRanges.X(timeRanges.X() + 1);
		if(timeRanges.Y() - timeRanges.X() >= 1)
			timeRanges.Y(timeRanges.Y() - 1);
        itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex, timeRanges, true);
	}
	else
	{
		if(itsDragTarget == kLeft)
			itsDragTarget = kBase; // 1999.08.30/Marko
		else if(itsDragTarget == kRight)
			ChangeTimeFilterRect(thePlace, theKey);
		if(itsDragTarget == kBase)
		{
			MoveTimeFilterRect(thePlace, theKey);
			return true;
		}
	}
	return true;
}

bool NFmiTimeFilterView::RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey)
{
	if(theKey & kCtrlKey) // select constant area
	{
		NFmiPoint timeRanges(itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex));
		timeRanges.X(timeRanges.X() - 1);
		timeRanges.Y(timeRanges.Y() + 1);
        itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex, timeRanges, true);
		return true;
	}
	return false;
}

bool NFmiTimeFilterView::MouseMove(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(itsDragTarget == kRight || itsDragTarget == kLeft)
	{
		ChangeTimeFilterRect(thePlace, theKey);
	}
	else //if(itsDragTarget == kBase)
	{
		ChangeTimeFilterRect(thePlace, theKey);
	}
	Draw(itsToolBox);
	return true;
}

void NFmiTimeFilterView::ChangeTimeFilterRect(const NFmiPoint & thePlace, unsigned long theKey)
{
	NFmiPoint timeFilterRect(itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex));
	NFmiPoint gridPoint(Relative2TimeFilterPoint(thePlace));
	double width = timeFilterRect.Y() - timeFilterRect.X();
	switch (itsDragTarget)
	{
	case kRight:
		timeFilterRect.Y(gridPoint.X());
		break;
	case kLeft:
		timeFilterRect.X(gridPoint.X());
		break;
	default:
		if(width)
		{
			timeFilterRect.X(round(gridPoint.X() - (width/2.)));
			timeFilterRect.Y(round(timeFilterRect.X() + width));
		}
		else
		{
			timeFilterRect.X(gridPoint.X());
			timeFilterRect.Y(gridPoint.X());
		}
		break;
	}
	if(theKey & kShiftKey)
        itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex, timeFilterRect, true); // true=pyöristä lähimpää tasatuntiin
	else
        itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex, timeFilterRect, false); // false=älä pyöristä lähimpää tasatuntiin
}

void NFmiTimeFilterView::MoveTimeFilterRect(const NFmiPoint & thePlace, unsigned long theKey)
{
	NFmiPoint timeRanges(itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex));
	int diff = int(timeRanges.Y() - timeRanges.X());
	NFmiPoint gridPoint(Relative2TimeFilterPoint(thePlace));
	timeRanges.X(gridPoint.X());
	timeRanges.Y(gridPoint.X() + diff);
	if(theKey & kShiftKey)
        itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex, timeRanges, true); // true=pyöristä lähimpää tasatuntiin
	else
        itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex, timeRanges, false); // false=älä pyöristä lähimpää tasatuntiin
}

NFmiPoint NFmiTimeFilterView::Relative2TimeFilterPoint(const NFmiPoint& theRelativePoint)
{
	NFmiRect relativeRect(GetFrame());
	NFmiPoint returnPoint(theRelativePoint.X() * itsGridXSize / relativeRect.Width(), 0);
	returnPoint.X(returnPoint.X() + itsGridXOrigo);
	return returnPoint;
}

// päivittää aikasiirtoihin/tasoituksiin liittyviä selvitystekstejä
// tekstit kertovat kuinka paljon aikaa on tarkoitus siirtää
void NFmiTimeFilterView::DrawTimeTexts(void)
{
	NFmiString timeStr(" ");
	double startValue = itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex).X();
	double endValue = itsCtrlViewDocumentInterface->TimeFilterRange(itsIndex).Y();
	boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->EditedSmartInfo();
	if(info) // tiedetään resoluutio, ja voidaan ilmoittaa aika tunneissa ja minuuteissa
	{
        info->FirstTime(); // asetetaan 1. aikaan, koska jos editoitu data on muuttuva aika-askelinen, tällöin on sattumanvaraista, minkä aika-askeleen resoluutio tulee tässä käyttöön
		int resolutionInMinutes = info->TimeResolution();
		startValue *= resolutionInMinutes;
		int startHours = int(fabs(startValue)/60);
		int startMinutes = int(fabs(startValue)) % 60;
		NFmiValueString startHoursStr(startHours, "%d");
		NFmiValueString startMinutesStr(startMinutes, "%d");
		if(startValue < 0)
			timeStr += "-";
		else
			timeStr += "+";
		timeStr += startHoursStr;
		timeStr += "h:";
		timeStr += startMinutesStr;
		timeStr += "min";

		endValue *= resolutionInMinutes;
		if(startValue != endValue)
		{
			timeStr += " - ";
			int endHours = int(fabs(endValue)/60);
			int endMinutes = int(fabs(endValue)) % 60;
			NFmiValueString endHoursStr(endHours, "%d");
			NFmiValueString endMinutesStr(endMinutes, "%d");
			if(endValue < 0)
				timeStr += "-";
			else
				timeStr += "+";
			timeStr += endHoursStr;
			timeStr += "h:";
			timeStr += endMinutesStr;
			timeStr += "min";
		}
	}
	else // ei tiedetä resoluutiota, ilmoitetaan vain indeksinä
	{
		NFmiValueString start(fabs(startValue), "%.2f");
		if(startValue < 0)
			timeStr += "-";
		else
			timeStr += "+";
		timeStr += start;
		if(startValue != endValue)
		{
			timeStr += " - ";
			if(endValue < 0)
				timeStr += "-";
			else
				timeStr += "+";
			NFmiValueString end(fabs(endValue), "%.2f");
			timeStr += end;
		}
	}

	itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
	itsDrawingEnvironment.SetFontSize(NFmiPoint(16,16));
	NFmiPoint place(GetFrame().TopLeft());
	NFmiText text(place, timeStr, false, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&text);
}
