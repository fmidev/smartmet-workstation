//**********************************************************
// C++ Class Name : NFmiCtrlViewList 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiCtrlViewList.cpp 
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
//    Sis‰lt‰‰ monta ctrl-view:t‰ listassa.
// 
//  Change Log: 
// 
//**********************************************************
#include "NFmiCtrlViewList.h"
#include "NFmiDrawParam.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "catlog/catlog.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiCtrlViewList::NFmiCtrlViewList (void)
:itsList()
,itsIter(itsList.Start())
,fResponseToMouseClicksAllways(false)
{
}
//--------------------------------------------------------
NFmiCtrlViewList::NFmiCtrlViewList(int theMapViewDescTopIndex, const NFmiRect& theRect
								  ,NFmiToolBox* theToolBox
								  ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								  ,bool fResponseToMouse)
:NFmiCtrlView(theMapViewDescTopIndex, theRect
			 ,theToolBox
			 ,theDrawParam)
,itsList()
,itsIter(itsList.Start())
,fResponseToMouseClicksAllways(fResponseToMouse)
{
}
//-------------------------------------------------------- 
NFmiCtrlViewList::NFmiCtrlViewList (NFmiCtrlViewList& theList)
  : NFmiCtrlView()
  , itsList()
  , itsIter(itsList.Start())
  , fResponseToMouseClicksAllways(theList.fResponseToMouseClicksAllways)
{
	theList.Reset();
	while(theList.Next())
	{
		Add(theList.Current());
	}
	Reset();
}
//-------------------------------------------------------- 
NFmiCtrlViewList::~NFmiCtrlViewList (void)
{
	Clear(true);
}
//--------------------------------------------------------
// Update 
//--------------------------------------------------------
void NFmiCtrlViewList::Update (const NFmiRect& theRect, NFmiToolBox* theToolBox)
{
    NFmiCtrlView::Update(theRect, theToolBox);
	for(Reset();Next();)
		Current()->Update(theRect, theToolBox);
}

static void DoCtrlViewDraw(NFmiCtrlView* ctrlView, NFmiToolBox* toolBox)
{
	if(ctrlView)
	{
		try
		{
			ctrlView->Draw(toolBox);
		}
		catch(std::exception& e)
		{
			std::string warningMessage = CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeCtrlViewIdentifier(ctrlView);
			warningMessage += " drawing failed with following error message: \"";
			warningMessage += e.what();
			warningMessage += "\"";
			CatLog::logMessage(warningMessage, CatLog::Severity::Warning, CatLog::Category::Visualization, true);
		}
	}
}

//--------------------------------------------------------
// Draw 
//--------------------------------------------------------
void NFmiCtrlViewList::Draw (NFmiToolBox* theToolBox)
{
    if(itsMapViewDescTopIndex >= 0 && itsMapViewDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
    {
        // If this is map-view drawing list, let's set the layer index before drawing each layer
        int layerIndex = 1;
        for(Reset(); Next();)
        {
            auto ctrlView = Current();
            ctrlView->ViewRowLayerNumber(layerIndex++);
			::DoCtrlViewDraw(ctrlView, theToolBox);
        }
    }
    else
    {
		for(Reset(); Next();)
		{
			::DoCtrlViewDraw(Current(), theToolBox);
		}
    }
}
//--------------------------------------------------------
// LeftButtonDown 
//--------------------------------------------------------
bool NFmiCtrlViewList::LeftButtonDown (const NFmiPoint& thePoint, unsigned long aNumber)
{
	for(Reset();Next();)
	{
		if( Current()->LeftButtonDown(thePoint, aNumber))
			return true;
	}
	return false;
}
//--------------------------------------------------------
// LeftButtonUp 
//--------------------------------------------------------
bool NFmiCtrlViewList::LeftButtonUp (const NFmiPoint& thePoint, unsigned long aNumber)
{
	for(Reset();Next();)
	{
		if( Current()->LeftButtonUp(thePoint, aNumber))
			return true;
	}
	return fResponseToMouseClicksAllways == true;
}

bool NFmiCtrlViewList::MiddleButtonDown(const NFmiPoint &thePoint, unsigned long aNumber)
{
	for(Reset();Next();)
	{
		if( Current()->MiddleButtonDown(thePoint, aNumber))
			return true;
	}
	return false; // yrit‰n optimoida koodia, enk‰ halua ett‰ middlemouse down vastaa aina true, joka johtaa eri n‰yttˆjen p‰ivitykseen
}

bool NFmiCtrlViewList::MiddleButtonUp(const NFmiPoint &thePoint, unsigned long aNumber)
{
	for(Reset();Next();)
	{
		if( Current()->MiddleButtonUp(thePoint, aNumber))
			return true;
	}
	return fResponseToMouseClicksAllways;
}


bool NFmiCtrlViewList::LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) // Marko lis‰si 14.12.2001
{
	for(Reset();Next();)
	{
		if( Current()->LeftDoubleClick(thePlace, theKey))
			return true;
	}
	return false; // tupla-clickiin ei vastata aina!
}

bool NFmiCtrlViewList::RightDoubleClick(const NFmiPoint &thePlace, unsigned long theKey)
{
	for(Reset();Next();)
	{
		if( Current()->RightDoubleClick(thePlace, theKey))
			return true;
	}
	return false; // tupla-clickiin ei vastata aina!
}

bool NFmiCtrlViewList::IsMouseDraggingOn(void)
{
    for(Reset(); Next();)
    {
        if(Current()->IsMouseDraggingOn())
            return true;
    }
    return false;
}

//--------------------------------------------------------
// RightButtonDown 
//--------------------------------------------------------
bool NFmiCtrlViewList::RightButtonDown (const NFmiPoint& thePoint, unsigned long aNumber)
{
	for(Reset();Next();)
	{
		if( Current()->RightButtonDown(thePoint, aNumber))
			return true;
	}
	return false;
}
//--------------------------------------------------------
// RightButtonUp 
//--------------------------------------------------------
bool NFmiCtrlViewList::RightButtonUp (const NFmiPoint& thePoint, unsigned long aNumber)
{
	for(Reset();Next();)
	{
		if( Current()->RightButtonUp(thePoint, aNumber))
			return true;
	}
	return fResponseToMouseClicksAllways == true;
}
//--------------------------------------------------------
// RightButtonUp 
//--------------------------------------------------------
bool NFmiCtrlViewList::MouseMove (const NFmiPoint& thePoint, unsigned long aNumber)
{
	for(Reset();Next();)
	{
		if( Current()->MouseMove(thePoint, aNumber))
			return true;
	}
	return false;
}

bool NFmiCtrlViewList::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	for(Reset();Next();)
	{
		if( Current()->MouseWheel(thePlace, theKey, theDelta))
			return true;
	}
	return false;
}

//--------------------------------------------------------
// Add 
//--------------------------------------------------------
bool NFmiCtrlViewList::Add (NFmiCtrlView* theView, bool fSortViewOrder)
{
	if(theView)
	{
		if(fSortViewOrder)
			return itsList.InsertionSort(theView);
		else
			return itsList.AddEnd(theView);
	}
	return false;
}
//--------------------------------------------------------
// Add 
//--------------------------------------------------------
bool NFmiCtrlViewList::Add(NFmiCtrlView* theView, unsigned long theIndex)
{
	if(theView)
	{
		return itsList.Add(theView, theIndex);
	}
	return false;
}
//--------------------------------------------------------
// Reset 
//--------------------------------------------------------
//  Asettaa iteraattorin osoittaamaan listan alkua, jolla 
//  tarkoitetaan ennen ensimm‰ist‰ item:‰ olevaa node:a.
bool NFmiCtrlViewList::Reset (void)
{
   itsIter = itsList.Start();
   return true;
}
//--------------------------------------------------------
// Current 
//--------------------------------------------------------
NFmiCtrlView* NFmiCtrlViewList::Current (void)
{return itsIter.CurrentPtr();}
//--------------------------------------------------------
// Next 
//--------------------------------------------------------
bool NFmiCtrlViewList::Next (void)
{return itsIter.Next();}
//--------------------------------------------------------
// Previous 
//--------------------------------------------------------
bool NFmiCtrlViewList::Previous (void)
{return itsIter.Previous();}
//--------------------------------------------------------
// Clear 
//--------------------------------------------------------
void NFmiCtrlViewList::Clear(bool fDeleteData)
{
	itsList.Clear(fDeleteData);
}
//--------------------------------------------------------
// Swap 
//--------------------------------------------------------
bool NFmiCtrlViewList::Swap(unsigned long index1, unsigned long index2)
{
	return itsList.Swap(index1, index2);
}
//--------------------------------------------------------
// Sort 
//--------------------------------------------------------
bool NFmiCtrlViewList::Sort(bool fAscendingOrder)
{
	return itsList.Sort(fAscendingOrder);
}
//--------------------------------------------------------
// Find, compare with NFmiCtrlView::operator ==
//--------------------------------------------------------
bool NFmiCtrlViewList::Find(NFmiCtrlView* item)
{
	if(item)
	{
		itsIter = itsList.Find(item);
		if(Current())
			return true;
		Reset();
	}
	return false;
}

// etsii vain drawparamin pointterin perusteella
/*
bool NFmiCtrlViewList::Find(NFmiDrawParam* theDrawParam)
{
	if(theDrawParam)
	{
		itsIter = itsList.Start();
		for(; itsIter.Next(); )
		{
			if(itsIter.CurrentPtr()->DrawParam() == theDrawParam)
				return true;
		}
	}
	return false;
}
*/
//--------------------------------------------------------
// Index
//--------------------------------------------------------
bool NFmiCtrlViewList::Index(unsigned long index)
{
	itsIter = itsList.Index(index);
	if(Current())
		return true;
	return false;
}

bool NFmiCtrlViewList::Remove(bool fDeleteView)
{
	return itsIter.Remove(fDeleteView);
}

void NFmiCtrlViewList::Time(const NFmiMetTime& theTime)
{
	itsTime = theTime;
	for(Reset();Next();)
	{
		Current()->Time(theTime);
	}
}
