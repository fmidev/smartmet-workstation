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
// Changed 1999.09.14/Marko	Poistin käytöstä edit-näytöt
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
   bool LeftDoubleClick(const NFmiPoint& thePlace, unsigned long theKey) override; // Marko lisäsi 17.12.2001
   bool IsMouseDraggingOn(void) override;
   bool Init(void);
   bool IsMouseCaptured(void) { return fMouseCaptured; }
   std::string ComposeToolTipText(const NFmiPoint& thePlace) override;

 protected:
   void UpdateTextData(void);
   void DrawBackground(void);
   void AdjustFinalViewPositions(const NFmiRect& theOriginalViewPositionFromParent, FmiDirection theViewPlacement);

//  Nämä Näyttöluokat ovat sekä NFmiCtrlViewList:assa, että erillisinä pareina koska
//  jossain tapauksissa näitä luokkia pitää käsitellä pareina, esim. Update()-metodissa,
//	kun lasketaan näyttöjen kokoja, jolloin jonkun näytön koko on riippuvainen parinsa 
//  koosta. Mutta joskus käytetään NFmiCtrlViewList:in metodeita tekemään koodista 
//  siistiä esim. hiirenkäsittelyt jne.
   NFmiCtrlViewList* itsViewList;
	// 1. pari
   NFmiParamCommandView* itsViewParamCommandView;	// 1. vasemmalla (kuudesta)	"Näytä"
   NFmiParamCommandView* itsViewParamsView;			// 1. oikealta (kuudesta)	"T--"
   NFmiParamCommandView* itsMaskParamCommandView;	// 3. vasemmalla (kuudesta)	"Maski"
   NFmiParamCommandView* itsMaskParamsView;			// 3. oikealta (kuudesta)	"T > 3"

 private:
   bool fMouseCaptured;
   bool fShowMaskSection;
   bool fViewParamsViewHasMapLayer;
};

