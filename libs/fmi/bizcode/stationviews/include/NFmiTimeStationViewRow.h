//**********************************************************
// C++ Class Name : NFmiTimeStationViewRow 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiTimeStationViewRow.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 3 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : ruudukkon�ytt� 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Fri - Sep 10, 1999 
// 
// 
//  Description: 
//   T�m� luokka pit�� sis�ll��n listan karttan�ytt�j� 
//   (NFmiStationViewHandler)
//   T�t� luokkaa on tarkoitus k�ytt�� editorin 
//   'ruudukko'-n�yt�n 'rivin�'.
//   N�ist� riveist� ruudukon muodostaa NFmiTimeStationViwRowList. 
//   
//   T�t� viimeista pit�� taas editorin n�ytt�luokka 
//   omanaan.
// 
//  Change Log: 
// 
//**********************************************************

#pragma once

#include "NFmiCtrlView.h"

class NFmiDrawParamList;
class NFmiStationViewHandler;
class NFmiCtrlViewList;
class NFmiArea;
class NFmiParamHandlerView;
class NFmiDataIdent;

class NFmiTimeStationViewRow : public NFmiCtrlView 
{

 public:
   NFmiTimeStationViewRow(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
					   ,NFmiToolBox * theToolBox
					   ,NFmiDrawingEnvironment * theDrawingEnvi
					   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					   ,int theIndex);
   virtual  ~NFmiTimeStationViewRow();
   void Draw(NFmiToolBox* theGTB);
   void Update(void);
   void Update (const NFmiRect & theRect, NFmiToolBox * theToolBox = 0);
   bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
   bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
   bool MiddleButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
   bool MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
   bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
   bool MouseMove (const NFmiPoint& thePlace, unsigned long theKey);
   bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta); 
   bool LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey); // Marko lis�si 14.12.2001
   bool RightDoubleClick(const NFmiPoint &thePlace, unsigned long theKey);
   bool IsMouseDraggingOn(void);
   void Time(const NFmiMetTime& theTime);
   NFmiStationViewHandler* CreateMapView(int theIndex);
   NFmiStationViewHandler* GetMapView(int theIndex);

   void DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4);// t�ll� piirret��n tavara, joka tulee my�s bitmapin p��lle
   bool StoreToolTipDataInDoc(const NFmiPoint& theRelativePlace);
   bool HasActiveRowView();
   std::string ComposeToolTipText(const NFmiPoint& theRelativePoint);
   NFmiCtrlView* GetViewWithRealRowIndex(int theRealRowIndex, const NFmiMetTime &theTime, const NFmiDataIdent &theDataIdent, bool fUseParamIdOnly);

 private:
   NFmiMetTime GetUsedTimeForViewTile(const NFmiMetTime& theStartTime, long theTimeStepInMinutes, int theRowIndex, int theColumnIndex);
   void CreateViewRow (void);
   void UpdateViewRowRects(void);
   NFmiRect CalcViewRect (int theIndex);
   int GetViewCountFromDocument();

//   T�h�n laitetaan NFmiStationViewHandler:it.
   NFmiCtrlViewList* itsViewList;
   boost::shared_ptr<NFmiArea> itsMapArea; // k�ytet��n vain NFmiStationViewHandler:in konstruktorissa?!?! 

//    V�liaikainen ratkaisu.
   int itsViewCount;
   int itsRowIndex; // monesko 'rivi' t�m� on kokonais n�ytt�ruudukossa (k�ytet��n oikean drawparamlistin pyyt�miseen)

};

