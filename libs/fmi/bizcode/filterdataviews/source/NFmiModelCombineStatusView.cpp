// NFmiModelCombineStatusView.cpp: implementation for the NFmiModelCombineStatusView class.
//
//////////////////////////////////////////////////////////////////////

#include "NFmiModelCombineStatusView.h"
#include "NFmiToolBox.h"
#include "NFmiRectangle.h"
#include "NFmiCombinedParam.h"
#include "NFmiFastQueryInfo.h"
#include "CtrlViewDocumentInterface.h"

NFmiModelCombineStatusView::NFmiModelCombineStatusView(NFmiToolBox * theToolBox
													 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
													 ,const NFmiRect& theRect)
:NFmiZoomView(0, theToolBox, theDrawParam, theRect)
,itsGridXSize(10)
,itsCheckedInfo()
{
}

NFmiModelCombineStatusView::~NFmiModelCombineStatusView()
{
}

void NFmiModelCombineStatusView::Update(void)
{
    boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsCtrlViewDocumentInterface->EditedSmartInfo();
    if(editedInfo)
        itsGridXSize = editedInfo->SizeTimes();
}

bool NFmiModelCombineStatusView::IsViewDrawed(void)
{
	return false;
}

NFmiTimeBag NFmiModelCombineStatusView::GetUsedTimeBag(void)
{
	if(itsCheckedInfo)
		return itsCheckedInfo->ValidTimes();

	return NFmiTimeBag();
}

void NFmiModelCombineStatusView::Draw(NFmiToolBox * theGTB)
{
	if(IsViewDrawed())
	{
		NFmiTimeBag timebag(GetUsedTimeBag());
		itsGridXSize = timebag.GetSize();

		itsDrawingEnvironment.SetFrameColor(NFmiColor(0,0,0));
		itsDrawingEnvironment.SetPenSize(NFmiPoint(1,1));
		itsDrawingEnvironment.EnableFill();
		itsDrawingEnvironment.SetFillColor(NFmiColor(0.9f,0.9f,0.9f));
		NFmiRect viewRect(CalcStatusGridViewSize());
		DrawFrame(itsDrawingEnvironment, viewRect);
		NFmiColor color(1,0,0);
		double bottom = viewRect.Bottom();
		double top = viewRect.Top();
		double boxWidth = viewRect.Width()/itsGridXSize;
		double start = viewRect.Left();
		for(int i=0; i<itsGridXSize; i++)
		{
			color = CheckStatusBoxColor(i);
			itsDrawingEnvironment.SetFillColor(color);

			NFmiPoint startingPoint(start + (i * boxWidth), top);
			NFmiPoint endingPoint(start + ((i+1) * boxWidth), bottom);
			NFmiRectangle box(startingPoint, endingPoint, 0, &itsDrawingEnvironment);
			itsToolBox->Convert(&box);
		}
	}
}

NFmiRect NFmiModelCombineStatusView::CalcStatusGridViewSize(void)
{
	return GetFrame();
}

bool NFmiModelCombineStatusView::LeftButtonDown(const NFmiPoint& /* thePlace */, unsigned long /* theKey */)
{
	return false; // pakko overridata emo
}

bool NFmiModelCombineStatusView::LeftButtonUp(const NFmiPoint& /* thePlace */, unsigned long /* theKey */)
{
	return false; // pakko overridata emo
}

bool NFmiModelCombineStatusView::RightButtonUp(const NFmiPoint& /* thePlace */, unsigned long /* theKey */)
{
	return false; // pakko overridata emo
}

bool NFmiModelCombineStatusView::MouseMove(const NFmiPoint& /* thePlace */, unsigned long /* theKey */)
{
	return false; // pakko overridata emo
}

// Käy läpi annetun info ja 'laskee' sen mukaan millä värillä kunkin ajan laatikko
// piirretään.
// Tee tämä: käy läpi kaikki parametrit ja katso, onko 1. paikalla jokin muu arvo kuin
// puuttuva.
// Värit:
// punainen, jos kaikilla parametreilla puuttuvaa
// oranssi, jos osalla parametreista puuttuvaa
// vihreä, jos kaikilla parametreilla puuttuvasta poikkeavaa

NFmiColor NFmiModelCombineStatusView::CheckStatusBoxColor(int theTimeIndex)
{
	NFmiColor color(1,0,0); // kaikki puuttuu, oletus arvo (punainen)
	if(itsCheckedInfo && itsCheckedInfo->TimeIndex(theTimeIndex))
	{
		bool anyMissing = false;
		bool allDataMissing = true;
		for(itsCheckedInfo->ResetParam(); itsCheckedInfo->NextParam(); )
		{
			float value = itsCheckedInfo->FloatValue();
			if(value == kFloatMissing || value == kTCombinedWeatherFloatMissing)
				anyMissing = true;
			else
				allDataMissing = false;
		}
		if(allDataMissing)
			;//color = NFmiColor(1,0,0);
		else if(anyMissing)
			color = NFmiColor(1,0.5f,0);
		else
			color = NFmiColor(0,1,0);
	}
	return color;
}

