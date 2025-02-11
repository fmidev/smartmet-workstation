//© Ilmatieteenlaitos/Marko
//  Original 3.6.1998
// 
//                                  
//Ver. 3.6.1998 / Marko
//----------------------------------------------------- NFmiTimeView.h

#pragma once

#include "NFmiCtrlView.h"
#include "NFmiTimeDescriptor.h"

class NFmiAxis;
class NFmiAxisView;
class NFmiStepTimeScale;
class NFmiTimeScaleView;
//_____________________________________________________ NFmiTimeView

class NFmiTimeView: public NFmiCtrlView 
{
 public:

	NFmiTimeView(int theMapViewDescTopIndex, const NFmiRect &theRect
				,NFmiToolBox *theToolBox
				,boost::shared_ptr<NFmiDrawParam> &theDrawParam
				,const NFmiTimeDescriptor &theTimeDescriptor
                ,int theRowIndex);
	NFmiTimeView( NFmiTimeView &theTimeView);
	~NFmiTimeView(void);

	void Draw(NFmiToolBox * theToolBox) override;
	NFmiTimeDescriptor& EditedDataTimeDescriptor(void);
	NFmiTimeBag& EditedDataTimeBag(void);
	NFmiTimeDescriptor& ZoomedTimeDescriptor(void);
	NFmiTimeBag& ZoomedTimeBag(void);
	void UpdateTimeSystem(void);
	
 protected:

	void UpdateTimeScale(void);
	virtual void DrawData(void);
	virtual void DrawBackground(void);
	virtual void DrawTimeLine(const NFmiMetTime& theTime);
	void DrawTimeGrids(NFmiDrawingEnvironment& envi,double minPos,double maxPos);

	double Time2Value(const NFmiMetTime& theTime); // time to axis position
	NFmiMetTime Value2Time(const NFmiPoint& thePoint, bool fExact = false); // k‰ytt‰‰ vain pisteen x arvoa
	virtual bool FindTimeIndex(double theRelPos, double theMaxDiff, int& theIndex);

	virtual NFmiRect CalcTimeAxisRect(double theLeftSideGab = 0.05); // theLeftSideGab = kuinka paljon j‰tet‰‰n tilaa aika-akselin vasemmalle puolelle
																	 // normaalisti j‰tet‰‰n 5% (0.05), mutta aluetoimisto versiossa halutaan enemm‰n tilaa havainnoille, joten siell‰ luku on isompi
	virtual NFmiMetTime CalcCurrentTime(void)const{return NFmiMetTime();};

 protected:

	 NFmiTimeDescriptor itsEditedDataTimeDescriptor;
	 NFmiTimeBag itsEditedDataTimeBag; // t‰h‰n lasketaan joku bagi jos timeDescriptorissa on timelist
	 NFmiTimeDescriptor itsZoomedTimeDescriptor;
	 NFmiTimeBag itsZoomedTimeBag; // t‰h‰n lasketaan joku bagi jos timeDescriptorissa on timelist
	 NFmiStepTimeScale *itsTimeAxis;
	 NFmiTimeScaleView *itsTimeView;
};

