//**********************************************************
// C++ Class Name : NFmiTrajectoryView 
// ---------------------------------------------------------
// Filetype: (HEADER)
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : metedit nfmiviews 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : Class Diagram 
// ---------------------------------------------------  
//  Author         : pietarin 
// 
//  Description: 
//   Apuikkuna trajektori dialogissa. T‰m‰n avulla s‰‰det‰‰n
//   trajektorin aloitus korkeutta ja siit‰ n‰kee trajektorin vertikaali
//   liikkeet ajan suhteen.
// 
//  Change Log: 
// 
//**********************************************************

#pragma once

#include "NFmiCtrlView.h"

class NFmiTimeControlView;
class NFmiTrajectory;
class NFmiSingleTrajector;
class NFmiColor;

class NFmiTrajectoryView : public NFmiCtrlView
{
 public:
	NFmiTrajectoryView(const NFmiRect &theRect, NFmiToolBox * theToolBox);
	~NFmiTrajectoryView(void);

	void Draw (NFmiToolBox *theGTB) override;
	bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;

 private:
	void DrawTrajectoryLegend(const NFmiTrajectory &theTrajectory, int theIndex);
	void DrawTimeGridLines(void);
	void DrawLegend(void);
	void DrawTrajectories(void);
	void DrawTrajectory(const NFmiTrajectory &theTrajectory, const NFmiColor &theColor);
	void DrawSingleTrajector(const NFmiSingleTrajector &theSingleTrajector, NFmiDrawingEnvironment &theEnvi, int theTimeStepInMinutes, int theTimeMarkerPixelSize, int theTimeMarkerPixelPenSize, FmiDirection theDirection);
	void DrawSelectedPressureLevelMarker(void);
	double Time2X(const NFmiMetTime &theTime);
	double TimeStepWidth(double timeStepInMinutes);
	void CalcRects(void);
	NFmiRect CalcDataRect(void);
	NFmiRect CalcPressureScaleRect(void);
	NFmiRect CalcStartPressureLevelMarkerRect(void);
	void DrawBackground(void);
	NFmiRect CalcTimeControlViewRect(void);
	void DrawTimeControl(void);
	bool CreateTimeControlView(void);
	double p2y(double p);
	double y2p(double y);
	void DrawPressureScale(void);
	void ChangePAxisValue(FmiDirection theMouseButton, bool upperPart);

	NFmiRect itsDataRect; // t‰m‰ on pikseli laatikko, joka on datan piirtoa varten
	NFmiTimeControlView * itsTimeControlView;
	NFmiRect itsTimeControlViewRect;
	NFmiRect itsStartPressureLevelMarkerRect; // t‰ll‰ boxilla merkit‰‰n aloitus painepinta korkeus ikkunaan

	float itsMaxPressure; // pintaa l‰hempi arvo (suurempi arvoltaan)
	float itsMinPressure; // ylemp‰n‰ oleva paine arvo (pienempi arvoltaan)
	NFmiRect itsPressureScaleFrame; // t‰lle alueelle piirret‰‰n log(p) asteikko
	int itsPressureScaleFontSize;
};

