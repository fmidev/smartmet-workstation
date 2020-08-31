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
// Changed 1999.09.14/Marko Lis�sin itsRowIndex-attribuutin, jonka avulla voidaan
//							luoda oikeita popup-menuja. N�m� n�yt�t sidotaan n�ytt�ruudukon riveihin.
// 
//**********************************************************

#pragma once

#include "NFmiCtrlView.h"

class NFmiParamCommandView : public NFmiCtrlView
{

 public:
   NFmiParamCommandView (int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, NFmiDrawingEnvironment * theDrawingEnvi, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex, bool hasMapLayer);
   void Draw(NFmiToolBox* theGTB);
   virtual NFmiRect CalcSize(void); // koko saattaa muuttua, ja uutta kokoa pit�� voida kysy� oliolta
   void UpdateTextData(const NFmiPoint& theFontSize, double theLineHeight, const NFmiPoint &theCheckBoxSize, const NFmiPoint &thePixelSize);
   bool ShowView(void){return fShowView;};
   void ShowView(bool newStatus){fShowView = newStatus;};
   bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);

 protected:
   virtual void DrawBackground(void);
   virtual void DrawData(void){};
   NFmiRect CheckBoxRect(const NFmiRect& parameterRowRect);
   NFmiPoint LineTextPlace(int zeroBasedRowIndex, const NFmiRect& parameterRowRect, bool checkBoxMove);
   void DrawCheckBox(const NFmiRect &theRect, NFmiDrawingEnvironment &theEnvi, bool fDrawCheck);
   NFmiRect CalcParameterRowRect(int zeroBasedRowIndex) const;
   int CalcParameterRowIndex(const NFmiPoint& pointedPlace, double* indexRealValueOut = nullptr) const;
   void DrawCheckBox(const NFmiRect& parameterRowRect, bool isChecked);
   double ConvertMilliMeterToRelative(double lengthInMilliMeter, bool isDirectionX) const;

   void CalcTextData(void); // t�m� tehd��n aina kun ruudun kokoa muutetaan (Update) NFmiParamhandler:issa joka jakaa tiedon listassaan oleville view:lle
   void CalcFontSize(void);
   NFmiPoint itsFontSize;
   NFmiPoint itsCheckBoxSize;
   NFmiPoint itsPixelSize; // pikselin koko relatiivisessa yksik�ss�
   double itsLineHeight; // itsFontSize.Y()+jokin v�li
   bool fShowView; // 1999.12.10/Marko
   bool fHasMapLayer = false;
   // Jokaiselle riville lis�t��n t�ll�inen tuomaan hieman v�ljyytt� tekstiriveille
   static double itsParameterRowVerticalMarginInMM;
   // K�ytet��n jotain v�li� reunojen ja eri piirto olioiden v�lill� (reuna - checkbox - teksti)
   static double itsParameterRowHorizontalMarginInMM;
};

