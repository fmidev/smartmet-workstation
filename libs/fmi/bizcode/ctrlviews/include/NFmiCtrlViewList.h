//**********************************************************
// C++ Class Name : NFmiCtrlViewList 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiCtrlViewList.h 
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
//    Sis�lt�� monta ctrl-view:t� listassa.
// 
//  Change Log: 
// Changed 1999.08.30/Marko	Lis�sin NumberOfItems()-metodin.
// 
//**********************************************************

#pragma once

#include "NFmiCtrlView.h"
#include "NFmiSortedPtrList.h"

#ifdef _MSC_VER
#pragma warning(disable : 4512) // poistaa VC++ k��nt�j�n C4512 "assignment operator could not be generated" varoituksen
#endif

class NFmiCtrlViewList : public NFmiCtrlView
{

public:
	
  NFmiCtrlViewList(void);
  NFmiCtrlViewList(int theMapViewDescTopIndex, const NFmiRect & theRect,
				   NFmiToolBox * theToolBox,
				   NFmiDrawingEnvironment * theDrawingEnvi,
				   boost::shared_ptr<NFmiDrawParam> &theDrawParam,
				   bool fResponseToMouse = false);
  inline NFmiCtrlViewList(NFmiCtrlViewList& theList); 
  virtual ~NFmiCtrlViewList(void); 
  using NFmiCtrlView::Update;
  void Update(const NFmiRect& theRect, NFmiToolBox* theToolBox = 0);
  void Draw(NFmiToolBox* theToolBox);
  inline bool IsIn(const NFmiPoint thePoint) const
  { return itsRect.IsInside (thePoint); }

  bool LeftButtonDown(const NFmiPoint& thePoint, unsigned long aNumber); 
  bool LeftButtonUp(const NFmiPoint& thePoint, unsigned long aNumber); 
  bool MiddleButtonDown(const NFmiPoint &thePoint, unsigned long aNumber); 
  bool MiddleButtonUp(const NFmiPoint &thePoint, unsigned long aNumber); 
  bool RightButtonDown(const NFmiPoint& thePoint, unsigned long aNumber); 
  bool RightButtonUp(const NFmiPoint& thePoint, unsigned long aNumber); 
  bool MouseMove(const NFmiPoint& thePoint, unsigned long aNumber); 
  bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta); 
  bool LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey); // Marko lis�si 14.12.2001
  bool RightDoubleClick(const NFmiPoint &thePlace, unsigned long theKey);
  bool IsMouseDraggingOn(void);
  bool Add(NFmiCtrlView* theView, bool fSortViewOrder = true);
  bool Add (NFmiCtrlView* theView, unsigned long theIndex);
  bool Reset (void);
  bool Next (void);
  bool Previous (void);
  NFmiCtrlView * Current(void);
  void Clear(bool fDeleteData = false);
  bool Swap(unsigned long index1, unsigned long index2); // k�ytetty listojen testauksessa
  bool Sort(bool fAscendingOrder = true);
  bool Find(NFmiCtrlView* item);
//  bool Find(NFmiDrawParam* theDrawParam); // etsii vain pointterin perusteella
  bool Index(unsigned long index);
  bool Remove(bool fDeleteView = false);
  unsigned long NumberOfItems(void)
  { return itsList.NumberOfItems(); }
  using NFmiCtrlView::Time;
  void Time(const NFmiMetTime& theTime);

 private:
  NFmiSortedPtrList<NFmiCtrlView>     itsList;
  NFmiPtrList<NFmiCtrlView>::Iterator itsIter;

  // aikasarja n�yt�ss� ei vastata hiiren klikkauksiin muuten kuin
  // jos klikkaus osuu ikkunaan, mutta karttan�ytt� vastaa aina
  bool fResponseToMouseClicksAllways;
};

