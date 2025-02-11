//**********************************************************
// C++ Class Name : NFmiParamHandlerView 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiParamHandlerView.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 2 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : Markon ehdotus 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thur - Jan 28, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// Changed 1999.09.14/Marko	Poistin k�yt�st� edit-n�yt�t
// 
//**********************************************************

#pragma once

#include "NFmiParamCommandView.h"

class NFmiDrawParam;
class NFmiCtrlViewList;

class NFmiParamHandlerView : public NFmiParamCommandView
{

 public:
   NFmiParamHandlerView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex, bool theShowMaskSection, bool viewParamsViewHasMapLayer);
   inline virtual ~NFmiParamHandlerView (void); 
   using NFmiParamCommandView::Update;
   void Update (const NFmiRect & theRect, NFmiToolBox * theToolBox, FmiDirection theViewPlacement, bool doIterativeFinalCalculations = false);
   void Draw (NFmiToolBox * theGTB) override;
   bool LeftButtonDown (const NFmiPoint& thePlace, unsigned long theKey) override;
   bool LeftButtonUp (const NFmiPoint& thePlace, unsigned long theKey) override;
   bool RightButtonDown (const NFmiPoint& /* thePlace */ , unsigned long /* theKey */ ) override { return false; };
   bool RightButtonUp (const NFmiPoint& thePlace, unsigned long theKey) override;
   bool MouseMove (const NFmiPoint& thePlace, unsigned long theKey) override;
   bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
   bool LeftDoubleClick(const NFmiPoint& thePlace, unsigned long theKey) override; // Marko lis�si 17.12.2001
   bool IsMouseDraggingOn(void) override;
   bool Init(void);
   bool IsMouseCaptured(void) { return fMouseCaptured; }
   std::string ComposeToolTipText(const NFmiPoint& thePlace) override;

 protected:
   void UpdateTextData(void);
   void DrawBackground(void);
   void AdjustFinalViewPositions(const NFmiRect& theOriginalViewPositionFromParent, FmiDirection theViewPlacement);

//  N�m� N�ytt�luokat ovat sek� NFmiCtrlViewList:assa, ett� erillisin� pareina koska
//  jossain tapauksissa n�it� luokkia pit�� k�sitell� pareina, esim. Update()-metodissa,
//	kun lasketaan n�ytt�jen kokoja, jolloin jonkun n�yt�n koko on riippuvainen parinsa 
//  koosta. Mutta joskus k�ytet��n NFmiCtrlViewList:in metodeita tekem��n koodista 
//  siisti� esim. hiirenk�sittelyt jne.
   NFmiCtrlViewList* itsViewList;
	// 1. pari
   NFmiParamCommandView* itsViewParamCommandView;	// 1. vasemmalla (kuudesta)	"N�yt�"
   NFmiParamCommandView* itsViewParamsView;			// 1. oikealta (kuudesta)	"T--"
   NFmiParamCommandView* itsMaskParamCommandView;	// 3. vasemmalla (kuudesta)	"Maski"
   NFmiParamCommandView* itsMaskParamsView;			// 3. oikealta (kuudesta)	"T > 3"

 private:
   bool fMouseCaptured;
   bool fShowMaskSection;
   bool fViewParamsViewHasMapLayer;
};

