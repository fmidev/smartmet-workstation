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
//  - GD View Name      : ruudukkonäyttö 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Fri - Sep 10, 1999 
// 
// 
//  Description: 
//   Tämä luokka pitää sisällään listan karttanäyttöjä 
//   (NFmiStationViewHandler)
//   Tätä luokkaa on tarkoitus käyttää editorin 
//   'ruudukko'-näytön 'rivinä'.
//   Näistä riveistä ruudukon muodostaa NFmiTimeStationViwRowList. 
//   
//   Tätä viimeista pitää taas editorin näyttöluokka 
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
					   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					   ,int theIndex);
   ~NFmiTimeStationViewRow();
   void Draw(NFmiToolBox* theGTB) override;
   void Update(void) override;
   void Update (const NFmiRect & theRect, NFmiToolBox * theToolBox = 0) override;
   bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MiddleButtonDown(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MouseMove (const NFmiPoint& thePlace, unsigned long theKey) override;
   bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
   bool LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) override; // Marko lisäsi 14.12.2001
   bool RightDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) override;
   bool IsMouseDraggingOn(void) override;
   void Time(const NFmiMetTime& theTime) override;
   NFmiStationViewHandler* CreateMapView(int theIndex);
   NFmiStationViewHandler* GetMapView(int theIndex);

   void DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4);// tällä piirretään tavara, joka tulee myös bitmapin päälle
   bool StoreToolTipDataInDoc(const NFmiPoint& theRelativePlace);
   bool HasActiveRowView();
   std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
   NFmiCtrlView* GetViewWithRealRowIndex(int theRealRowIndex, const NFmiMetTime &theTime, const NFmiDataIdent &theDataIdent, bool fUseParamIdOnly);

 private:
   NFmiMetTime GetUsedTimeForViewTile(const NFmiMetTime& theStartTime, long theTimeStepInMinutes, int theRowIndex, int theColumnIndex);
   void CreateViewRow (void);
   void UpdateViewRowRects(void);
   NFmiRect CalcViewRect (int theIndex);
   int GetViewCountFromDocument();

//   Tähän laitetaan NFmiStationViewHandler:it.
   NFmiCtrlViewList* itsViewList;
   boost::shared_ptr<NFmiArea> itsMapArea; // käytetään vain NFmiStationViewHandler:in konstruktorissa?!?! 

//    Väliaikainen ratkaisu.
   int itsViewCount;
   int itsRowIndex; // monesko 'rivi' tämä on kokonais näyttöruudukossa (käytetään oikean drawparamlistin pyytämiseen)

};

