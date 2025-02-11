#include "NFmiModelProducerIndexView.h"
#include "NFmiToolBox.h"
#include "NFmiRectangle.h"
#include "NFmiAxisViewWithMinFontSize.h"
#include "NFmiStepTimeScale.h"
#include "NFmiAdjustedTimeScaleView.h"
#include "NFmiAxis.h"
#include "NFmiFastQueryInfo.h"

#include "boost\math\special_functions\round.hpp"

NFmiModelProducerIndexView::NFmiModelProducerIndexView(NFmiToolBox * theToolBox
													 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
													 ,const NFmiRect& theRect)
:NFmiModelCombineStatusView(theToolBox, theDrawParam, theRect)
,itsQueryInfoVector()
,itsProducerIndexInTimeVector()
,itsLoadedDataTimeBag()
,fUseNonModifieableDataTimeBag(false)
,itsNonModifieableDataTimeBag()
,itsNonModifieableTimeRange()
,itsNonModifieableTimeRangeProducerIndex(0)
,itsActiveProducerIndex(0)
,itsSelectedProducerPriorityTable()
,itsProducerColorTable()
,itsSourceDataExistLimits()
,fUseExtraShortRangeDataTimeBag(false)
,itsExtraShortRangeDataTimeBag()
,itsExtraShortRangeTimeRange(-1, -1)
,itsTimeAxis(0)
,itsTimeView(0)
,itsHourValueView(0)
,itsHourValueAxis(0)
,itsProducerColorIndexViewRect()
,fProducerSelectionMouseCaptured(false)
{
}

NFmiModelProducerIndexView::~NFmiModelProducerIndexView()
{
	delete itsTimeAxis;
	delete itsTimeView;
	delete itsHourValueAxis;
	delete itsHourValueView;
}

void NFmiModelProducerIndexView::Update(void)
{
}

void NFmiModelProducerIndexView::Draw(NFmiToolBox * theGTB)
{
	DrawBackground();
	if(itsTimeView)
		itsToolBox->Draw(itsTimeView);
	DrawHourValueView();
	itsProducerColorIndexViewRect = CalcStatusGridViewSize();
	NFmiModelCombineStatusView::Draw(theGTB);
}

bool NFmiModelProducerIndexView::IsViewDrawed(void)
{
	return true;
}

NFmiTimeBag NFmiModelProducerIndexView::GetUsedTimeBag(void)
{
	return itsLoadedDataTimeBag;
}

void NFmiModelProducerIndexView::DrawBackground(void)
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

void NFmiModelProducerIndexView::DrawHourValueView(void)
{
	if(itsHourValueView)
		itsHourValueView->DrawAll();
}

bool NFmiModelProducerIndexView::LeftButtonDown(const NFmiPoint& thePlace, unsigned long /* theKey */)
{
	if(itsProducerColorIndexViewRect.IsInside(thePlace))
		fProducerSelectionMouseCaptured = true;
	return true;
}

bool NFmiModelProducerIndexView::LeftButtonUp(const NFmiPoint& thePlace, unsigned long /* theKey */)
{
	fProducerSelectionMouseCaptured = false;
	return SetProducerIndex(GetPlaceIndexInProducerColorIndexView(thePlace));
}

bool NFmiModelProducerIndexView::RightButtonUp(const NFmiPoint& /* thePlace */, unsigned long /* theKey */)
{
	return false; // pakko overridata emo
}

bool NFmiModelProducerIndexView::MouseMove(const NFmiPoint& thePlace, unsigned long /* theKey */)
{
	return SetProducerIndex(GetPlaceIndexInProducerColorIndexView(thePlace));
}

// asettaa aktiivisen tuottajan annetun indeksin kohtaan (jos sallittua)
bool NFmiModelProducerIndexView::SetProducerIndex(int theIndex)
{
	if(theIndex != -1)
	{
		if(!IndexInsideRange(itsNonModifieableTimeRange, theIndex))
			if(IndexInsideRange(itsSourceDataExistLimits[itsActiveProducerIndex], theIndex))
			{
				itsProducerIndexInTimeVector[theIndex] = itsActiveProducerIndex;
				return true;
			}
	}
	return false;
}

bool NFmiModelProducerIndexView::IndexInsideRange(const NFmiPoint& theRange, int theIndex)
{
	if(theRange.X() != -1) // jos range sis‰lt‰‰ negatiivisia arvoja, on se invalidi, eik‰ indeksi ole sis‰ll‰
		if(theRange.X() <= theIndex && theRange.Y() >= theIndex)
			return true;
	return false;
}

// palauttaa hiirell‰ osoitetun paikan indeksin (producer v‰ri laatikossa)
int NFmiModelProducerIndexView::GetPlaceIndexInProducerColorIndexView(const NFmiPoint& thePlace)
{
	if(itsProducerColorIndexViewRect.IsInside(thePlace))
	{
		int size = itsLoadedDataTimeBag.GetSize();
		double start = itsProducerColorIndexViewRect.Left();
		double width = itsProducerColorIndexViewRect.Width();
		int index = boost::math::iround((thePlace.X() - start) / width * (size - 1));
		return index;
	}
	return -1;
}

// tuli hieman mega-luokan interface
void NFmiModelProducerIndexView::SetDataFromDialog(	 std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theQueryInfoVector
													,const NFmiTimeBag& theLoadedDataTimeBag
													,const NFmiTimeBag& theNonModifieableDataTimeBag
													,const std::vector<int>& theSelectedProducerPriorityTable
													,const std::vector<NFmiColor>& theProducerColorTable
													,int theNonModifieableTimeRangeProducerIndex
													,int theActiveProducerIndex)
{
	itsQueryInfoVector = theQueryInfoVector; // onnistuuko pointteri datan kopio???
	itsLoadedDataTimeBag = theLoadedDataTimeBag;
	fUseNonModifieableDataTimeBag = theNonModifieableDataTimeBag.GetSize() > 1;
	itsNonModifieableDataTimeBag = theNonModifieableDataTimeBag;
	itsNonModifieableTimeRangeProducerIndex = theNonModifieableTimeRangeProducerIndex;
	itsActiveProducerIndex = theActiveProducerIndex;
	itsSelectedProducerPriorityTable = theSelectedProducerPriorityTable;
	itsProducerColorTable = theProducerColorTable;
	UpdateHelperData();
	CreateTimeAxis();
	CreateHourValueAxis();
}

void NFmiModelProducerIndexView::UpdateHelperData(void)
{
	InitProducerIndexInTimeVector(); // tekee vain vektorin koon
	CalcNonModifieableTimeRange();
	CalcSourceDataExistLimits();
	FillProducerIndexInTimeVector(); // t‰ytt‰‰ vektorin indekseill‰
}

// vain luo vectorin
void NFmiModelProducerIndexView::InitProducerIndexInTimeVector(void)
{
	itsProducerIndexInTimeVector.clear();
	itsProducerIndexInTimeVector.resize(itsLoadedDataTimeBag.GetSize(), -1); // alustetaan vector -1:ll‰
}

void NFmiModelProducerIndexView::CalcNonModifieableTimeRange(void)
{
	if(fUseNonModifieableDataTimeBag)
	{
		itsNonModifieableTimeRange = CalcTimeIndexRange(itsLoadedDataTimeBag, itsNonModifieableDataTimeBag, false);
	}
	else
		itsNonModifieableTimeRange = NFmiPoint(-1, -1);
}

void NFmiModelProducerIndexView::CalcSourceDataExistLimits(void)
{
	size_t size = itsQueryInfoVector.size();
	itsSourceDataExistLimits.resize(size, NFmiPoint(-1, -1));
	NFmiTimeBag timebag;
	for(size_t i=0; i< size; i++)
	{
		if(itsQueryInfoVector[i])
		{
			timebag = itsQueryInfoVector[i]->ValidTimes();
			itsSourceDataExistLimits[i] = CalcTimeIndexRange(itsLoadedDataTimeBag, timebag, true);
		}
		else
			itsSourceDataExistLimits[i] = NFmiPoint(-1, -1);
	}
}

// t‰ytt‰‰ taulukon prioriteetti listan mukaisesti
void NFmiModelProducerIndexView::FillProducerIndexInTimeVector(void)
{
	if(fUseExtraShortRangeDataTimeBag)
		FillNonMarkedRange(itsProducerIndexInTimeVector, itsExtraShortRangeTimeRange, itsNonModifieableTimeRangeProducerIndex);
	else if(fUseNonModifieableDataTimeBag)
		FillNonMarkedRange(itsProducerIndexInTimeVector, itsNonModifieableTimeRange, itsNonModifieableTimeRangeProducerIndex);

    size_t size = itsSelectedProducerPriorityTable.size();
	for(size_t i=0; i<size; i++)
		FillNonMarkedRange(itsProducerIndexInTimeVector, itsSourceDataExistLimits[itsSelectedProducerPriorityTable[i]], itsSelectedProducerPriorityTable[i]);
}

void NFmiModelProducerIndexView::FillNonMarkedRange(std::vector<int>& theIndexVector, const NFmiPoint& theRange, int theIndex)
{
  if(theRange.X() >= 0)
    for(int i= static_cast<int>(theRange.X()); i <= theRange.Y(); i++)
		if(theIndexVector[i] ==  -1)
			theIndexVector[i] = theIndex;
}

NFmiPoint NFmiModelProducerIndexView::CalcTimeIndexRange(NFmiTimeBag& thePrimeTimeBag, NFmiTimeBag& theCheckedTimeBag, bool fCalcNormalRange)
{
	NFmiPoint range;
	
	thePrimeTimeBag.FindNearestTime(theCheckedTimeBag.FirstTime(), kForward);
	int startIndex = thePrimeTimeBag.CurrentIndex();
	thePrimeTimeBag.FindNearestTime(theCheckedTimeBag.LastTime(), kBackward);
	int endIndex = thePrimeTimeBag.CurrentIndex();

	if(startIndex < 0 || endIndex < 0)
		range = NFmiPoint(-1, -1);
	else
	{
		range = NFmiPoint(startIndex, endIndex);
		if(fCalcNormalRange && fUseNonModifieableDataTimeBag)
			CutWithNonEditableRange(&range);
	}

	return range;
}

void NFmiModelProducerIndexView::CutWithNonEditableRange(NFmiPoint* theRange)
{ // Yksinkertaistus: ei editoitava alue voi olla vain alussa (a la Kepa)
	if(theRange->X() <= itsNonModifieableTimeRange.Y())
		theRange->X(itsNonModifieableTimeRange.Y() + 1);
	if(theRange->Y() <= itsNonModifieableTimeRange.Y())
		theRange->Y(itsNonModifieableTimeRange.Y() + 1);
	if(theRange->X() == theRange->Y()) // jos X ja Y samoja, alkuper‰inen alue on ollut ei editoitavalla alueella
		*theRange = NFmiPoint(-1, -1);
}

NFmiRect NFmiModelProducerIndexView::CalcTimeAxisRect(void)
{
	NFmiRect rect(GetFrame());
	double bottom = rect.Bottom() - rect.Height()/25.;
	double top = rect.Bottom() - rect.Height()/2.5;
	double left = rect.Left() + rect.Width()/50.;
	double right = rect.Right() - rect.Width()/50.;
	rect = NFmiRect(left, top, right, bottom);
	return rect;
}

NFmiRect NFmiModelProducerIndexView::CalcHourValueAxisRect(void)
{
	NFmiRect rect(GetFrame());
	double top = rect.Top() + rect.Height()/25.;
	double bottom = rect.Top() + rect.Height()/4.;
	double left = rect.Left() + rect.Width()/40.;
	double right = rect.Right() - rect.Width()/40.;
	rect = NFmiRect(left, top, right, bottom);
	return rect;
}

NFmiRect NFmiModelProducerIndexView::CalcStatusGridViewSize(void)
{
	NFmiRect rect(GetFrame());
	double top = rect.Top() + rect.Height()/3.2;
	double bottom = rect.Bottom() - rect.Height()/2.05;
	double left = rect.Left() + rect.Width()/80.;
	double right = rect.Right() - rect.Width()/80.;
	rect = NFmiRect(left, top, right, bottom);
	return rect;
}

NFmiColor NFmiModelProducerIndexView::CheckStatusBoxColor(int theTimeIndex)
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

void NFmiModelProducerIndexView::CreateTimeAxis(void)
{
	static NFmiDrawingEnvironment envi;
	delete itsTimeAxis;
	delete itsTimeView;
	itsLoadedDataTimeBag.Reset();
	itsTimeAxis = new NFmiStepTimeScale(itsLoadedDataTimeBag);
	NFmiStepTimeScale theSelectedScale(itsLoadedDataTimeBag);
	itsTimeAxisRect = CalcTimeAxisRect();

	NFmiPoint fontSize(18, 18);

	itsTimeView = new NFmiAdjustedTimeScaleView(-1, CalcTimeAxisRect()
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

void NFmiModelProducerIndexView::CreateHourValueAxis(void)
{
	delete itsHourValueAxis;
	delete itsHourValueView;
	double lowLimit = 0
		  ,highLimit = itsLoadedDataTimeBag.LastTime().DifferenceInHours(itsLoadedDataTimeBag.FirstTime()); 

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

bool NFmiModelProducerIndexView::IsProducerIndexInTimeVectorFilled(void)
{
	return std::find(itsProducerIndexInTimeVector.begin(), itsProducerIndexInTimeVector.end(), -1) == itsProducerIndexInTimeVector.end();
}

void NFmiModelProducerIndexView::SetExtraShortRangeData(bool useExtraData, const NFmiTimeBag& theExtraShortRangeTimeBag, NFmiTimeBag& theLoadedDataTimeBag)
{
	fUseExtraShortRangeDataTimeBag = useExtraData;
	itsExtraShortRangeDataTimeBag = theExtraShortRangeTimeBag;
	itsExtraShortRangeTimeRange = CalcTimeIndexRange(theLoadedDataTimeBag, itsExtraShortRangeDataTimeBag, false);
}
