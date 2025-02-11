#include "NFmiModelProducerIndexViewWCTR.h"
#include "NFmiToolBox.h"
#include "NFmiRectangle.h"
#include "NFmiAxisViewWithMinFontSize.h"
#include "NFmiStepTimeScale.h"
#include "NFmiLine.h"
#include "NFmiAdjustedTimeScaleView.h"
#include "NFmiAxis.h"
#include "NFmiDataLoadingInfo.h"
#include "NFmiTimeScale.h"
#include "NFmiModelDataBlender.h"
#include "NFmiQueryInfo.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiText.h"

NFmiModelProducerIndexViewWCTR::NFmiModelProducerIndexViewWCTR(NFmiToolBox * theToolBox
													 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
													 ,const NFmiRect& theRect)
:NFmiModelCombineStatusView(theToolBox, theDrawParam, theRect)
,itsQueryInfoVector()
,itsProducerIndexInTimeVector()
,itsLoadedDataTimeDescriptor()
,itsActiveProducerIndex(0)
,itsSelectedProducerPriorityTable()
,itsProducerColorTable()
,itsSourceDataExistLimits()
,itsTimeAxis(0)
,itsTimeView(0)
,itsHourValueView(0)
,itsHourValueAxis(0)
,itsProducerColorIndexViewRect()
,fProducerSelectionMouseCaptured(false)
{
}

NFmiModelProducerIndexViewWCTR::~NFmiModelProducerIndexViewWCTR()
{
	delete itsTimeAxis;
	delete itsTimeView;
	delete itsHourValueAxis;
	delete itsHourValueView;
}

void NFmiModelProducerIndexViewWCTR::Update(void)
{
}

void NFmiModelProducerIndexViewWCTR::Draw(NFmiToolBox * theGTB)
{
	DrawBackground();

	DrawTimeView();
	DrawStatusBoxView();
	DrawHourValueView();
	DrawBlendingArea();
	DrawStartAndEndTimes();
}

void NFmiModelProducerIndexViewWCTR::DrawStatusBoxView()
{
	itsProducerColorIndexViewRect = CalcStatusGridViewSize();
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0, 0, 0));
	itsDrawingEnvironment.SetPenSize(NFmiPoint(1, 1));
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(NFmiColor(0.9f, 0.9f, 0.9f));
	NFmiColor color(1, 0, 0);
	NFmiTimeBag tmpTimeBag(MaximalCoverageTimeBag());

	itsLoadedDataTimeDescriptor.Reset();
	itsLoadedDataTimeDescriptor.Next();
	NFmiMetTime time1(itsLoadedDataTimeDescriptor.Time());
	NFmiMetTime time2(time1);
	double bottom = itsProducerColorIndexViewRect.Bottom();
	double top = itsProducerColorIndexViewRect.Top();
	for(; itsLoadedDataTimeDescriptor.Next(); )
	{
		time2 = itsLoadedDataTimeDescriptor.Time();
		color = CheckStatusBoxColor(itsLoadedDataTimeDescriptor.Index());
		itsDrawingEnvironment.SetFillColor(color);

		NFmiPoint startingPoint(Time2Value(time1), top);
		NFmiPoint endingPoint(Time2Value(time2) + 0.002, bottom);
		NFmiRectangle box(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&box);
		time1 = time2;
	}
}

void NFmiModelProducerIndexViewWCTR::DrawTimeView()
{
	if(itsTimeView)
		itsToolBox->Draw(itsTimeView);
}

// Piirret‰‰n k‰ytt‰j‰lle editoidun datan alku ja loppu ajat 
// selvyyden vuoksi ikkunan yl‰reunaan.
void NFmiModelProducerIndexViewWCTR::DrawStartAndEndTimes()
{
	NFmiString timeFormat = "Www MM.DD HH:mm [utc]";
	NFmiDrawingEnvironment envi;
	envi.SetFontSize(NFmiPoint(18, 18));
	envi.SetFontType(kArial);
	auto oldTextAlingment = itsToolBox->GetTextAlignment();
	itsToolBox->SetTextAlignment(kTopLeft);
	const auto& startTime = itsLoadedDataTimeDescriptor.FirstTime();
	auto startTimeStr = NFmiString("Start time: ") + startTime.ToStr(timeFormat, kEnglish);
	NFmiText startTimeText(itsRect.TopLeft(), startTimeStr, false, nullptr, &envi);
	itsToolBox->Convert(&startTimeText);
	itsToolBox->SetTextAlignment(kTopRight);
	const auto& endTime = itsLoadedDataTimeDescriptor.LastTime();
	auto endTimeStr = NFmiString("End time: ") + endTime.ToStr(timeFormat, kEnglish);
	NFmiText endTimeText(itsRect.TopRight(), endTimeStr, false, nullptr, &envi);
	itsToolBox->Convert(&endTimeText);

	itsToolBox->SetTextAlignment(oldTextAlingment);
}

// Piierret‰‰n modelIndex v‰ri tauluun alue, mill‰ aikav‰lill‰
// malleja blendataan.
// Piirr‰n oranssin laatikon alaosaan ja sen poikki menee nouseva viiva
// kuvaamaan blendi‰.
void NFmiModelProducerIndexViewWCTR::DrawBlendingArea(void)
{
	NFmiModelDataBlender &modelDataBlender = itsCtrlViewDocumentInterface->ModelDataBlender();
	if(modelDataBlender.Use())
	{
		NFmiRect blendRect(itsProducerColorIndexViewRect);
		blendRect.Left(Time2Value(modelDataBlender.GetStartTime()));
		blendRect.Right(Time2Value(modelDataBlender.GetEndTime()));
		blendRect.Inflate(0, -0.06);
		blendRect.Bottom(itsProducerColorIndexViewRect.Bottom());

		NFmiDrawingEnvironment envi;
		envi.EnableFill();
		envi.SetFillColor(NFmiColor(1.f, 0.5f, 0.f));
		NFmiRectangle box(blendRect, 0, &envi);
		itsToolBox->Convert(&box);

		blendRect.Bottom(blendRect.Bottom() - itsToolBox->SY(1)); // jostain syyst‰ vinoviivan alkukorkeutta pit‰‰ s‰‰t‰‰ yhden pikselin verran ylˆs
		NFmiLine line(blendRect.BottomLeft(), blendRect.TopRight(), 0, &envi);
		itsToolBox->Convert(&line);
	}
}

bool NFmiModelProducerIndexViewWCTR::IsViewDrawed(void)
{
	return true;
}

void NFmiModelProducerIndexViewWCTR::DrawBackground(void)
{
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.EnableFrame();
	envi.SetFillColor(NFmiColor(1.f,1.f,0.97f));
	NFmiRectangle rect(itsRect.TopLeft()
					  ,itsRect.BottomRight()
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rect);
}

void NFmiModelProducerIndexViewWCTR::DrawHourValueView(void)
{
	if(itsHourValueView)
		itsHourValueView->DrawAll();
}

bool NFmiModelProducerIndexViewWCTR::LeftButtonDown(const NFmiPoint& thePlace, unsigned long /* theKey */)
{
	if(itsProducerColorIndexViewRect.IsInside(thePlace))
		fProducerSelectionMouseCaptured = true;
	return true;
}

bool NFmiModelProducerIndexViewWCTR::LeftButtonUp(const NFmiPoint& thePlace, unsigned long /* theKey */)
{
	fProducerSelectionMouseCaptured = false;
	return SetProducerIndex(GetPlaceIndexInProducerColorIndexView(thePlace));
}

bool NFmiModelProducerIndexViewWCTR::RightButtonUp(const NFmiPoint& /* thePlace */, unsigned long /* theKey */)
{
	return false; // pakko overridata emo
}

bool NFmiModelProducerIndexViewWCTR::MouseMove(const NFmiPoint& thePlace, unsigned long /* theKey */)
{
	return SetProducerIndex(GetPlaceIndexInProducerColorIndexView(thePlace));
}

// asettaa aktiivisen tuottajan annetun indeksin kohtaan (jos sallittua)
bool NFmiModelProducerIndexViewWCTR::SetProducerIndex(int theIndex)
{
	return SetProducerIndex(theIndex, itsActiveProducerIndex);
}

bool NFmiModelProducerIndexViewWCTR::SetProducerIndex(int theIndex, int theProducerIndex)
{
	if(theIndex != -1)
	{
		if(IndexInsideRange(itsSourceDataExistLimits[theProducerIndex], theIndex))
		{
			itsProducerIndexInTimeVector[theIndex] = theProducerIndex;
			return true;
		}
	}
	return false;
}


// T‰ll‰ metodilla s‰‰det‰‰n ett‰ jos malli blenderi on k‰ytˆss‰ ja on kaksi eri mallia valittuna
// 1- ja 2-prioriteeteissa, 1. mallia t‰ytet‰‰n annettuun aikaan asti (jos mahd.)
// ja 2. mallia t‰st‰ alkaen eteenp‰in (jos mahd.).
// Jos palauttaa true, pit‰‰ n‰yttˆj‰ p‰ivitt‰‰ ja jos false, ei tarvitse p‰ivitt‰‰.
bool NFmiModelProducerIndexViewWCTR::SelectSecondProducerFromThisTimeOn(int theTimeIndex)
{
	if(itsCtrlViewDocumentInterface->ModelDataBlender().Use())
	{
		if(itsSelectedProducerPriorityTable.size() >= 2 && (itsSelectedProducerPriorityTable[0] != itsSelectedProducerPriorityTable[1]))
		{
			// asetetaan ensin timeIndex - 1 :een asti prod 1:ll‰ ja siit‰ eteenp‰in prod 2:lla
			for(int i=0; i <= theTimeIndex; i++)
				SetProducerIndex(i, itsSelectedProducerPriorityTable[0]);
			for(size_t i=theTimeIndex+1; i < itsProducerIndexInTimeVector.size(); i++)
				SetProducerIndex(static_cast<int>(i), itsSelectedProducerPriorityTable[1]);
			return true;
		}
	}
	return false;
}

bool NFmiModelProducerIndexViewWCTR::IndexInsideRange(const NFmiPoint& theRange, int theIndex)
{
	if(theRange.X() != -1) // jos range sis‰lt‰‰ negatiivisia arvoja, on se invalidi, eik‰ indeksi ole sis‰ll‰
		if(theRange.X() <= theIndex && theRange.Y() >= theIndex)
			return true;
	return false;
}

// palauttaa hiirell‰ osoitetun paikan indeksin (producer v‰ri laatikossa)
int NFmiModelProducerIndexViewWCTR::GetPlaceIndexInProducerColorIndexView(const NFmiPoint& thePlace)
{
	if(itsProducerColorIndexViewRect.IsInside(thePlace))
	{
		NFmiMetTime time1(Value2Time(thePlace));
		if(itsLoadedDataTimeDescriptor.TimeToNearestStep(time1, kCenter, 12*60))
			return itsLoadedDataTimeDescriptor.Index();
	}
	return -1;
}

// tuli hieman mega-luokan interface
void NFmiModelProducerIndexViewWCTR::SetDataFromDialog( std::vector<boost::shared_ptr<NFmiQueryInfo> > &theQueryInfoVector
														,const NFmiTimeDescriptor& theLoadedDataTimeDescriptor
														,const std::vector<int>& theSelectedProducerPriorityTable
														,const std::vector<NFmiColor>& theProducerColorTable
														,int theActiveProducerIndex)
{
	itsQueryInfoVector = theQueryInfoVector; // onnistuuko pointteri datan kopio???
	itsLoadedDataTimeDescriptor = theLoadedDataTimeDescriptor;
	itsActiveProducerIndex = theActiveProducerIndex;
	itsSelectedProducerPriorityTable = theSelectedProducerPriorityTable;
	itsProducerColorTable = theProducerColorTable;
	UpdateHelperData();
	CreateTimeAxis();
	itsProducerColorIndexViewRect = CalcStatusGridViewSize();
	CreateHourValueAxis();
}

void NFmiModelProducerIndexViewWCTR::UpdateHelperData(void)
{
	InitProducerIndexInTimeVector(); // tekee vain vektorin koon
	CalcSourceDataExistLimits();
	FillProducerIndexInTimeVector(); // t‰ytt‰‰ vektorin indekseill‰
}

// vain luo vectorin
void NFmiModelProducerIndexViewWCTR::InitProducerIndexInTimeVector(void)
{
	itsProducerIndexInTimeVector.clear();
	itsProducerIndexInTimeVector.resize(itsLoadedDataTimeDescriptor.Size(), -1); // alustetaan vector -1:ll‰
}

void NFmiModelProducerIndexViewWCTR::CalcSourceDataExistLimits(void)
{
	size_t size = itsQueryInfoVector.size();
	itsSourceDataExistLimits.resize(size, NFmiPoint(-1, -1));
	NFmiTimeDescriptor timeDescriptor;
	for(size_t i=0; i< size; i++)
	{
		if(itsQueryInfoVector[i])
		{
			timeDescriptor = itsQueryInfoVector[i]->TimeDescriptor();
			itsSourceDataExistLimits[i] = CalcTimeIndexRange(itsLoadedDataTimeDescriptor, timeDescriptor);
		}
		else
			itsSourceDataExistLimits[i] = NFmiPoint(-1, -1);
	}
}

// t‰ytt‰‰ taulukon prioriteetti listan mukaisesti
void NFmiModelProducerIndexViewWCTR::FillProducerIndexInTimeVector(void)
{
	int size = static_cast<int>(itsSelectedProducerPriorityTable.size());
	for(int i=0; i<size; i++)
		FillNonMarkedRange(itsProducerIndexInTimeVector, itsSourceDataExistLimits[itsSelectedProducerPriorityTable[i]], itsSelectedProducerPriorityTable[i]);
}

void NFmiModelProducerIndexViewWCTR::FillNonMarkedRange(std::vector<int>& theIndexVector, const NFmiPoint& theRange, int theIndex)
{
  if(theRange.X() >= 0)
    for(int i= static_cast<int>(theRange.X()); i <= theRange.Y(); i++)
		if(theIndexVector[i] ==  -1)
			theIndexVector[i] = theIndex;
}

NFmiPoint NFmiModelProducerIndexViewWCTR::CalcTimeIndexRange(NFmiTimeDescriptor& thePrimeTimeDescriptor, NFmiTimeDescriptor& theCheckedTimeDescriptor)
{
	NFmiPoint range;

	thePrimeTimeDescriptor.TimeToNearestStep(theCheckedTimeDescriptor.FirstTime(), kForward);
	int startIndex = thePrimeTimeDescriptor.Index();
	thePrimeTimeDescriptor.TimeToNearestStep(theCheckedTimeDescriptor.LastTime(), kBackward);
	int endIndex = thePrimeTimeDescriptor.Index();

	if(startIndex < 0 || endIndex < 0)
		range = NFmiPoint(-1, -1);
	else
		range = NFmiPoint(startIndex, endIndex);
	return range;
}


NFmiRect NFmiModelProducerIndexViewWCTR::CalcTimeAxisRect(void)
{
	NFmiRect rect(GetFrame());
	double bottom = rect.Bottom() - (rect.Height() / 90.);
	double top = rect.Bottom() - rect.Height()/2.1;
	double left = rect.Left() + rect.Width()/50.;
	double right = rect.Right() - rect.Width()/50.;
	rect = NFmiRect(left, top, right, bottom);
	return rect;
}

NFmiRect NFmiModelProducerIndexViewWCTR::CalcHourValueAxisRect(void)
{
	NFmiRect rect(GetFrame());
	double bottom = itsProducerColorIndexViewRect.Top() - (rect.Height() / 25.);
	double top = bottom - rect.Height()/6.;
	double left = rect.Left() + rect.Width()/40.;
	double right = rect.Right() - rect.Width()/40.;
	rect = NFmiRect(left, top, right, bottom);
	return rect;
}

NFmiRect NFmiModelProducerIndexViewWCTR::CalcStatusGridViewSize(void)
{
	NFmiRect rect(GetFrame());
	double bottom = itsTimeAxisRect.Top() - (rect.Height() / 45.);
	double top = bottom - rect.Height()/5.5;
	double left = rect.Left() + rect.Width()/80.;
	double right = rect.Right() - rect.Width()/80.;
	rect = NFmiRect(left, top, right, bottom);
	return rect;
}

NFmiColor NFmiModelProducerIndexViewWCTR::CheckStatusBoxColor(int theTimeIndex)
{
	NFmiColor color(0.8f, 0.8f, 0.8f); // kaikki puuttuu, oletus arvo (punainen)
	if(theTimeIndex >= 0 && theTimeIndex < static_cast<int>(itsProducerIndexInTimeVector.size()))
	{
		int prodIndex = itsProducerIndexInTimeVector[theTimeIndex];
		if(prodIndex >= 0 && prodIndex < static_cast<int>(itsProducerColorTable.size()))
			color = itsProducerColorTable[prodIndex];
	}
	return color;
}

const NFmiTimeBag& NFmiModelProducerIndexViewWCTR::MaximalCoverageTimeBag()
{
    return itsCtrlViewDocumentInterface->GetUsedDataLoadingInfo().MaximalCoverageTimeBag();
}

void NFmiModelProducerIndexViewWCTR::CreateTimeAxis(void)
{
	static NFmiDrawingEnvironment envi;
	delete itsTimeAxis;
	delete itsTimeView;
	NFmiTimeBag tmpTimeBag(MaximalCoverageTimeBag());
	tmpTimeBag.Reset();
	itsTimeAxis = new NFmiStepTimeScale(tmpTimeBag);
	NFmiStepTimeScale theSelectedScale(tmpTimeBag);
	itsTimeAxisRect = CalcTimeAxisRect();

	NFmiPoint fontSize(18, 18);

	itsTimeView = new NFmiAdjustedTimeScaleView(0, itsTimeAxisRect
									,itsToolBox
									,itsTimeAxis
									,&theSelectedScale
									,0
									,&envi
									,0
									,true
									,fontSize
									,false); // false = ei piirret‰ niit‰ vihreit‰ laatikoita
	int bull = 0;
	itsTimeView->SetBorderWidth(bull);
}

void NFmiModelProducerIndexViewWCTR::CreateHourValueAxis(void)
{
	delete itsHourValueAxis;
	delete itsHourValueView;
	NFmiTimeBag tmpTimeBag(MaximalCoverageTimeBag());
	double lowLimit = 0
		  ,highLimit = tmpTimeBag.LastTime().DifferenceInHours(tmpTimeBag.FirstTime()); 

	NFmiStepScale scale((float)lowLimit
					  ,(float)highLimit
					  ,highLimit > 60. ? 12.f : 6.f	// step value
					  ,0				// start gap
					  ,kForward			// scale direction
					  ,false);			// steps on borders (onko akselin reunat aina stepin kohdalla)
	itsHourValueAxis = new NFmiAxis(scale, NFmiTitle("Y"));
	itsHourValueAxisRect = CalcHourValueAxisRect();
	itsHourValueView = new NFmiAxisViewWithMinFontSize(itsHourValueAxisRect
													,itsToolBox
													,itsHourValueAxis
													,kRight
													,kLeft
													,true
													,true
													,false
													,0.1f);
}

bool NFmiModelProducerIndexViewWCTR::IsProducerIndexInTimeVectorFilled(void)
{
	return std::find(itsProducerIndexInTimeVector.begin(), itsProducerIndexInTimeVector.end(), -1) == itsProducerIndexInTimeVector.end();
}

//-----------------------------------------------------------------
double NFmiModelProducerIndexViewWCTR::Time2Value(const NFmiMetTime& theTime)
{
	double value = ((NFmiTimeScale*)itsTimeAxis->Scale())->RelTimeLocation(theTime);
	double finalValue = itsTimeAxisRect.Left() + value * itsTimeAxisRect.Width();
	return finalValue;
}

NFmiMetTime NFmiModelProducerIndexViewWCTR::Value2Time(const NFmiPoint& thePoint)
{
	return itsTimeView->GetTime(thePoint);
}
