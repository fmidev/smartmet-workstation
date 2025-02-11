//**********************************************************
// C++ Class Name : NFmiTimeStationViewRowList 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiTimeStationViewRowList.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : ruudukkonaytto rev eng 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : Class Diagram 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Tues - Sep 14, 1999 
// 
// 
//  Description: 
//   Tämä luokka pitää sisällään näytöruudukon. 
//   Ruudukko koostuu listasta näyttörivejä (NFmiTimeStationViewRow).
//   
// 
//  Change Log: 
// 
//**********************************************************

#pragma once

#include "NFmiCtrlView.h"

class NFmiCtrlViewList;
class NFmiTimeStationViewRow;
class NFmiArea;
class NFmiDataIdent;
class NFmiStationViewHandler;

class NFmiTimeStationViewRowList : public NFmiCtrlView
{

 public:
   NFmiTimeStationViewRowList(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							   ,NFmiToolBox * theToolBox
							   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   ~NFmiTimeStationViewRowList(void);
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
   void Time(const NFmiMetTime & theTime) override;

   void DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4);// tällä piirretään tavara, joka tulee myös bitmapin päälle
   void StoreToolTipDataInDoc(const NFmiPoint& theRelativePlace);
   std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
   NFmiCtrlView* GetViewWithRealRowIndex(int theRealRowIndex, const NFmiMetTime &theTime, const NFmiDataIdent &theDataIdent, bool fUseParamIdOnly = false);
   NFmiStationViewHandler* GetMapView(int theRowIndex, int theIndex);

 private:
   NFmiTimeStationViewRow* CreateRowView(int theIndex);
   NFmiRect CalcViewRect(int theIndex);
   void CreateRows(void);
   void UpdateRowRects(void);
   int GetViewRowCountFromDocument();

//   Tähän talletetaan NFmiTimeStationViewRow-olioita.
   NFmiCtrlViewList* itsViewList;
   int itsRowCount;
   boost::shared_ptr<NFmiArea> itsMapArea; // käytetään vain NFmiTimeStationViewRow:in konstruktorissa?!?! 

};

