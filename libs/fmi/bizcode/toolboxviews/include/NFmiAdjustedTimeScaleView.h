//© Ilmatieteenlaitos/Marko.
//Original 1.12.1999
// 
//Ver. 1.12.1999
//---------------------------------------------------------- NFmiAdjustedTimeScaleView.h

#pragma once

#include "NFmiTimeScaleView.h"
#include "NFmiDataMatrix.h"

class ToolboxViewsInterface;
//_________________________________________________________ NFmiAdjustedTimeScaleView
class NFmiAdjustedTimeScaleView : public NFmiTimeScaleView
{
 public:

	NFmiAdjustedTimeScaleView(int theMapViewDescTopIndex, const NFmiRect &theRect
							,NFmiToolBox *theToolBox
							,NFmiStepTimeScale *theBaseScale = 0
							,NFmiStepTimeScale *theSelectedScale = 0
							,NFmiView *theEnclosure = 0
							,NFmiDrawingEnvironment *theEnvironment = 0
							,unsigned long theIdent = 0
							,bool isUpsideDown = true
							,const NFmiPoint& theFontSize = NFmiPoint(8,12)
							,bool drawSelectedTimes = true
                            ,bool useActiveMapTime = false);
                                 
	virtual ~NFmiAdjustedTimeScaleView(void); 

	virtual NFmiVoidPtrList &CollectOwnDrawingItems(NFmiVoidPtrList& theDrawingList);
	virtual NFmiVoidPtrList &DrawScale(NFmiVoidPtrList &theDrawingList
										,NFmiStepTimeScale *theScale
										,const NFmiRect &theRect);
	const NFmiPoint& FontSize(void){return itsFontSize;}
	void FontSize(const NFmiPoint& newSize){itsFontSize = newSize;}
	double Time2Value(const NFmiMetTime& theTime) const;
	NFmiRect CalcSelectedTimeArea(const NFmiMetTime &theTime) const;
	NFmiRect CalcTotalTimeScaleArea(void) const;
	void DrawSelectedTimes(void);
 
 protected:
	void GetTimeSteps(NFmiDrawingEnvironment &theDrawingEnvi, int &theMajorStepInMinutesOut, int &theMinorStepInMinutesOut);
	int GetTimeControltimeStep(void) const;
	NFmiRect CalcTimeArea(const NFmiMetTime& startTime, const NFmiMetTime& endTime, double theMaxWidth = 0.) const;
	void CalcTickMarkHelperValues(const NFmiRect &theScaleRect, const NFmiPoint &theFontSize, double &y1, double &y2, double &y3, double &y4);
	int CalcSkipHourPrintValue(int theStepNumber);
	int CalcSkipTickMarkValue(int theStepNumber);
	int CalcSkipDaysValue(int theTotalDayCount);
	double GetNormalToCurrentViewPixelRatio(void);
	int GetFinalSkipValue(int theCurrentValue, checkedVector<int> &theValues, int theMaxValue);
	double CalcTimeScaleWidthInMM(void);
	void DrawWallClockRectangle(void);
	double RelativeXToMM(double theRelativeXLength);

	NFmiPoint itsFontSize;
    ToolboxViewsInterface* itsToolboxViewsInterface;
	int itsMapViewDescTopIndex; // ctrl-view:issä on tieto minkä desctopin alla se on. Tämä ei koske muita kuin karttanäytöllä olevia viewtä, muilla tämä indeksi on -1 joka on puuttuva arvo
	bool fDrawSelectedTimes; // piirretäänkö ne vihreät laatikot, jotka osoittavat aikoja kartalla
    bool fUseActiveMapTime; // piirretäänkö vihreät laatikot aktiivisesta kartasta, vaiko itsMapViewDescTopIndex:in mukaisesti
	std::vector<int> itsTickMarkStepsLimitsInMinutes; // tähän talletetaan kaikki mahdolliset tick-mark aikastepit minuuteissa
   
 private:
};

//Inlines

