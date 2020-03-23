//**********************************************************
// C++ Class Name : NFmiParamCommandView 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiParamCommandView.h 
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
// Changed 1999.09.14/Marko Lisäsin itsRowIndex-attribuutin, jonka avulla voidaan
//							luoda oikeita popup-menuja. Nämä näytöt sidotaan näyttöruudukon riveihin.
// 
//**********************************************************

#pragma once

#include "NFmiCtrlView.h"

class NFmiParamCommandView : public NFmiCtrlView
{

 public:
   NFmiParamCommandView (int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, NFmiDrawingEnvironment * theDrawingEnvi, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex, bool hasMapLayer);
   void Draw(NFmiToolBox* theGTB);
   virtual NFmiRect CalcSize(void); // koko saattaa muuttua, ja uutta kokoa pitää voida kysyä oliolta
   void UpdateTextData(const NFmiPoint& theFontSize, const NFmiPoint& theFirstLinePlace, double theLineHeight, const NFmiPoint &theCheckBoxSize, const NFmiPoint &thePixelSize);
   bool ShowView(void){return fShowView;};
   void ShowView(bool newStatus){fShowView = newStatus;};
   bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);

 protected:
   int CalcIndex(const NFmiPoint& thePlace, double *indexRealValueOut = nullptr);
   virtual void DrawBackground(void);
   virtual void DrawData(void){};
   NFmiRect CheckBoxRect(int lineIndex, bool drawedRect);
   NFmiPoint LineTextPlace(int lineIndex, bool checkBoxMove);
   void DrawCheckBox(const NFmiRect &theRect, NFmiDrawingEnvironment &theEnvi, bool fDrawCheck);

   void CalcTextData(void); // tämä tehdään aina kun ruudun kokoa muutetaan (Update) NFmiParamhandler:issa joka jakaa tiedon listassaan oleville view:lle
   void CalcFontSize(void);
   NFmiPoint itsFontSize;
   NFmiPoint itsCheckBoxSize;
   NFmiPoint itsPixelSize; // pikselin koko relatiivisessa yksikössä
   NFmiPoint itsFirstLinePlace;
   double itsLineHeight; // itsFontSize.Y()+jokin väli
   bool fShowView; // 1999.12.10/Marko
   bool fHasMapLayer;
};

