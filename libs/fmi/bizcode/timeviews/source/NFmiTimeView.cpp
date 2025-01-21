#include "NFmiTimeView.h"
#include "NFmiTimeScale.h"
#include "NFmiTimeScaleView.h"
#include "NFmiStepTimeScale.h"
#include "NFmiToolBox.h"
#include "NFmiRectangle.h"
#include "NFmiLine.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiFastQueryInfo.h"


//______________________________________________________________ NFmiTimeView

NFmiTimeView::NFmiTimeView(int theMapViewDescTopIndex, const NFmiRect &theRect
						  ,NFmiToolBox *theToolBox
  						  ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
						  ,const NFmiTimeDescriptor &theTimeDescriptor
                          ,int theRowIndex)
:NFmiCtrlView(theMapViewDescTopIndex, theRect
			 ,theToolBox
			 ,theDrawParam
             ,theRowIndex)
,itsEditedDataTimeDescriptor()
,itsEditedDataTimeBag()
,itsZoomedTimeDescriptor()
,itsZoomedTimeBag()
,itsTimeAxis(0)
,itsTimeView(0)
{
	UpdateTimeSystem();
}

void NFmiTimeView::UpdateTimeSystem(void)
{
	if(itsCtrlViewDocumentInterface->EditedSmartInfo())
	{
		itsEditedDataTimeDescriptor = itsCtrlViewDocumentInterface->EditedSmartInfo()->TimeDescriptor();
		itsZoomedTimeDescriptor = NFmiTimeDescriptor(NFmiMetTime(60), itsCtrlViewDocumentInterface->TimeSerialViewTimeBag());
		if(itsEditedDataTimeDescriptor.UseTimeList())
		{	// luodaan 'feikki' timebagi tunnin resoluutiolla
			itsEditedDataTimeBag = NFmiTimeBag(itsEditedDataTimeDescriptor.FirstTime(), itsEditedDataTimeDescriptor.LastTime(), 60);
		}
		else
			itsEditedDataTimeBag = *itsEditedDataTimeDescriptor.ValidTimeBag();// ? *itsEditedDataTimeDescriptor.ValidTimeBag() : *itsEditedDataTimeDescriptor.OriginTimeBag();

		if(itsZoomedTimeDescriptor.UseTimeList())
		{	// luodaan 'feikki' timebagi tunnin resoluutiolla
			itsZoomedTimeBag = NFmiTimeBag(itsZoomedTimeDescriptor.FirstTime(), itsZoomedTimeDescriptor.LastTime(), 60);
		}
		else
			itsZoomedTimeBag = *itsZoomedTimeDescriptor.ValidTimeBag();// ? *itsZoomedTimeDescriptor.ValidTimeBag() : *itsZoomedTimeDescriptor.OriginTimeBag();

		UpdateTimeScale();
	}
}

//----------------------------------------------------------------------

NFmiTimeView::NFmiTimeView(NFmiTimeView &theTimeView)
:NFmiCtrlView(theTimeView)
{
}
//----------------------------------------------------------------------
NFmiTimeView::~NFmiTimeView(void)
{
	delete itsTimeAxis;
	delete itsTimeView;
}
//----------------------------------------------------------------------
void NFmiTimeView::Draw(NFmiToolBox * theToolBox)
{
	NFmiTimeView::DrawData(); // muuten kutsuu poikasen metodia!!!
}
//----------------------------------------------------------------------
NFmiRect NFmiTimeView::CalcTimeAxisRect(double theLeftSideGab)
{
	NFmiRect axisRect(GetFrame());
	axisRect.Left(axisRect.Left() + axisRect.Width() * theLeftSideGab);
	axisRect.Right(axisRect.Right() - axisRect.Width()/20.); // tähän pitäisi saada hard pixel siirto
	axisRect.Bottom(axisRect.Top() + axisRect.Height()/15.);
	return axisRect;
}

//----------------------------------------------------------------------
void NFmiTimeView::DrawData(void)
{
	DrawBackground();
	DrawTimeLine(CalcCurrentTime());
	if(itsTimeView)
		itsToolBox->Draw(itsTimeView);
}
//----------------------------------------------------------------------
void NFmiTimeView::DrawBackground(void)
{
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.EnableFrame();
	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	envi.SetFillColor(NFmiColor(0.8f,0.9f,1.f));
	NFmiRectangle rect(itsRect.TopLeft()
					  ,itsRect.BottomRight()
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rect);
}
//----------------------------------------------------------------------
void NFmiTimeView::DrawTimeLine(const NFmiMetTime& theTime)
{
	NFmiDrawingEnvironment envi;
	envi.SetFrameColor(NFmiColor(1.f,0.f,0.f));
	double x = Time2Value(theTime);
	NFmiLine line(NFmiPoint(x,itsRect.Top())
				 ,NFmiPoint(x,itsRect.Bottom())
				 ,0
				 ,&envi);
	itsToolBox->Convert(&line);
}
//-----------------------------------------------------------------
double NFmiTimeView::Time2Value(const NFmiMetTime& theTime)
{
    if(itsTimeAxis)
    {
        auto timeScale = static_cast<NFmiTimeScale*>(itsTimeAxis->Scale());
        if(timeScale)
        {
            double value = timeScale->RelTimeLocation(theTime);
            NFmiRect rect(CalcTimeAxisRect());
            double finalValue = rect.Left() + value * rect.Width();
            return finalValue;
        }
    }

    return 0;
}

void NFmiTimeView::DrawTimeGrids(NFmiDrawingEnvironment& envi,double minPos,double maxPos)
{ // min- and maxpos are the other axis relative positions (e.g. value-axis)
	int steps = itsTimeAxis->StepNumber();
	int usedStepAdvance = 1;
	if(steps < 70)
		usedStepAdvance = 1;
	else if(steps < 200)
		usedStepAdvance = 3;
	else if(steps < 410)
		usedStepAdvance = 6;
	else if(steps < 820)
		usedStepAdvance = 12;
	else if(steps < 1620)
		usedStepAdvance = 24;
	else
		usedStepAdvance = 48;
	NFmiRect timeRect = CalcTimeAxisRect();
	double stepValue = timeRect.Width()/(steps-1);
	double value = timeRect.Left();
	for(int i = 0; i < steps; i += usedStepAdvance) // piirretään vaaka akselit
	{
		NFmiLine line(NFmiPoint(value, minPos) // HUOM!! toimii vain jos aikaasteikko-viivat ovat pystysuuntaan
					 ,NFmiPoint(value, maxPos)
					 ,0
					 ,&envi);
		itsToolBox->Convert(&line);
		value += stepValue*usedStepAdvance;
	}
}

bool NFmiTimeView::FindTimeIndex(double theRelPos, double theMaxDiff, int& theIndex)
{
	NFmiMetTime pointedTime(Value2Time(NFmiPoint(theRelPos, 0)));
	EditedDataTimeDescriptor().TimeToNearestStep(pointedTime);
	theIndex = EditedDataTimeDescriptor().Index();

	return true;
}

NFmiMetTime NFmiTimeView::Value2Time(const NFmiPoint& thePoint, bool fExact)
{
	if(itsTimeView)
        return itsTimeView->GetTime(thePoint, fExact);
	else
		return NFmiMetTime();
}

void NFmiTimeView::UpdateTimeScale(void)
{
	if(itsTimeAxis)
		delete itsTimeAxis;
	itsTimeAxis = new NFmiStepTimeScale(ZoomedTimeBag());
	NFmiStepTimeScale theSelectedScale(ZoomedTimeBag());

	if(itsTimeView)
		delete itsTimeView;
    itsTimeView = new NFmiTimeScaleView(CalcTimeAxisRect()
                                       ,itsToolBox
                                       ,itsTimeAxis
                                       ,&theSelectedScale);
}

NFmiTimeDescriptor& NFmiTimeView::EditedDataTimeDescriptor(void)
{
	return itsEditedDataTimeDescriptor;
}

NFmiTimeBag& NFmiTimeView::EditedDataTimeBag(void)
{
	return itsEditedDataTimeBag;
}

NFmiTimeDescriptor& NFmiTimeView::ZoomedTimeDescriptor(void)
{
	return itsZoomedTimeDescriptor;
}
NFmiTimeBag& NFmiTimeView::ZoomedTimeBag(void)
{
	return itsZoomedTimeBag;
}
