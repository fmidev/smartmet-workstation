//**********************************************************
// C++ Class Name : NFmiTimeStationViewRowList 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiTimeStationViewRowList.cpp 
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
//   T�m� luokka pit�� sis�ll��n n�yt�ruudukon. 
//   Ruudukko koostuu listasta n�ytt�rivej� (NFmiTimeStationViewRow).
//   
// 
//  Change Log: 
// 
//**********************************************************
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k��nt�j�n varoitusta (liian pitk� nimi >255 merkki� joka johtuu 'puretuista' STL-template nimist�)
#endif

#include "NFmiTimeStationViewRowList.h"
#include "NFmiArea.h"
#include "NFmiCtrlViewList.h"
#include "NFmiTimeStationViewRow.h"
#include "NFmiDrawParamList.h"
#include "NFmiDrawParam.h"
#include "CtrlViewDocumentInterface.h"
#include "MapHandlerInterface.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiTimeStationViewRowList::NFmiTimeStationViewRowList(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
													   ,NFmiToolBox * theToolBox
													   ,NFmiDrawingEnvironment * theDrawingEnvi
													   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam)
//:NFmiCtrlView(*theArea
:NFmiCtrlView(theMapViewDescTopIndex, theArea->XYArea()
			 ,theToolBox
			 ,theDrawingEnvi
			 ,theDrawParam)
,itsViewList(0)
,itsRowCount(GetViewRowCountFromDocument())
,itsMapArea(theArea->Clone())
{
	CreateRows();
}
NFmiTimeStationViewRowList::~NFmiTimeStationViewRowList(void)
{
	delete itsViewList;
}
//--------------------------------------------------------
// Draw 
//--------------------------------------------------------
void NFmiTimeStationViewRowList::Draw(NFmiToolBox* theGTB)
{
	if(itsViewList)
		itsViewList->Draw(theGTB);
}

int NFmiTimeStationViewRowList::GetViewRowCountFromDocument()
{
    return static_cast<int>(GetCtrlViewDocumentInterface()->ViewGridSize(itsMapViewDescTopIndex).Y());
}

//--------------------------------------------------------
// Update 
//--------------------------------------------------------
void NFmiTimeStationViewRowList::Update(void)
{
	if(GetViewRowCountFromDocument() != itsRowCount)
	{
		itsRowCount = GetViewRowCountFromDocument();
		CreateRows();
	}
	else
		UpdateRowRects();

    auto updateMapViewDrawingLayers = GetMapHandlerInterface()->UpdateMapViewDrawingLayers();
    CtrlViewUtils::MapViewMode displayMode = itsCtrlViewDocumentInterface->MapViewDisplayMode(itsMapViewDescTopIndex);
	if(displayMode == CtrlViewUtils::MapViewMode::kOneTime)
	{
		int columnCount = static_cast<int>(itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).X());
		for(int i = 1; i <= itsRowCount; i++)
		{
			for(int j = 1; j <= columnCount; j++)
			{
				bool rowUpdated = false;
				NFmiDrawParamList* drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex(i, j));
				if(drawParamList)
				{
					bool paramListDirty = drawParamList->IsDirty();
					if(updateMapViewDrawingLayers || paramListDirty)
					{
						if(rowUpdated == false && itsViewList->Index(i))
						{
							rowUpdated = true;
							((NFmiTimeStationViewRow*)itsViewList->Current())->Update();
						}
					}
					drawParamList->Dirty(false);
				}
			}
		}
	}
	else if(displayMode == CtrlViewUtils::MapViewMode::kRunningTime)
	{ // juokseva aika moodissa jokaisella rivill� on sama paramlista
		NFmiDrawParamList* drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex(1, 1));
		if(drawParamList)
		{
			bool paramListDirty = drawParamList->IsDirty();
			for(int i = 1; i <= itsRowCount; i++)
			{
				if(updateMapViewDrawingLayers || paramListDirty)
				{
					if(itsViewList->Index(i))
						((NFmiTimeStationViewRow*)itsViewList->Current())->Update();
				}
			}
			drawParamList->Dirty(false);
		}
	}
	else
	{ // normaali moodissa rivit menev�t simppelisti
		for(int i = 1; i <= itsRowCount; i++)
		{
			NFmiDrawParamList* drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, i);
			if(drawParamList && (updateMapViewDrawingLayers || drawParamList->IsDirty()))
			{
				if(itsViewList->Index(i))
					((NFmiTimeStationViewRow*)itsViewList->Current())->Update();
				drawParamList->Dirty(false);
			}
		}
	}
}

// T�m�n pit�� p�ivitt�� emo ja data osan alueet.
void NFmiTimeStationViewRowList::Update(const NFmiRect & theRect, NFmiToolBox * theToolBox)
{
    NFmiCtrlView::Update(theRect, theToolBox);
	if(itsViewList)
        itsViewList->Update(theRect, theToolBox);
    UpdateRowRects();
}

//--------------------------------------------------------
// LeftButtonDown 
//--------------------------------------------------------
bool NFmiTimeStationViewRowList::LeftButtonDown(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(itsViewList && itsViewList->IsIn(thePlace))
		return itsViewList->LeftButtonDown(thePlace, theKey);
	return false;
}
//--------------------------------------------------------
// LeftButtonUp 
//--------------------------------------------------------
bool NFmiTimeStationViewRowList::LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
    // leftButtonUp k�sitell��n, jos jossain on mouse-draggays p��ll�, tai ollaan n�yt�n sis�ll�
    if(itsViewList && (itsViewList->IsMouseDraggingOn() || itsViewList->IsIn(thePlace)))
		return itsViewList->LeftButtonUp(thePlace, theKey);
	return false;
}

bool NFmiTimeStationViewRowList::MiddleButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(itsViewList && itsViewList->IsIn(thePlace))
		return itsViewList->MiddleButtonDown(thePlace, theKey);
	return false;
}

bool NFmiTimeStationViewRowList::MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(itsViewList && itsViewList->IsIn(thePlace))
		return itsViewList->MiddleButtonUp(thePlace, theKey);
	return false;
}

bool NFmiTimeStationViewRowList::LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) // Marko lis�si 14.12.2001
{
	if(itsViewList && itsViewList->IsIn(thePlace))
		return itsViewList->LeftDoubleClick(thePlace, theKey);
	return false;
}

bool NFmiTimeStationViewRowList::RightDoubleClick(const NFmiPoint &thePlace, unsigned long theKey)
{
	if(itsViewList && itsViewList->IsIn(thePlace))
		return itsViewList->RightDoubleClick(thePlace, theKey);
	return false;
}

//--------------------------------------------------------
// RightButtonUp 
//--------------------------------------------------------
bool NFmiTimeStationViewRowList::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(itsViewList && itsViewList->IsIn(thePlace))
		return itsViewList->RightButtonUp(thePlace, theKey);
	return false;
}

bool NFmiTimeStationViewRowList::MouseMove(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(itsViewList)
	{
        if(itsViewList->IsMouseDraggingOn())
            return itsViewList->MouseMove(thePlace, theKey);
        else if(itsViewList->Index(itsCtrlViewDocumentInterface->ActiveViewRow(itsMapViewDescTopIndex)))
			return itsViewList->Current()->MouseMove(thePlace, theKey);
	}
	return false;
}

bool NFmiTimeStationViewRowList::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(itsViewList)
	{
		return itsViewList->MouseWheel(thePlace, theKey, theDelta);
	}
	return false;
}

bool NFmiTimeStationViewRowList::IsMouseDraggingOn(void)
{
    if(itsViewList)
        return itsViewList->IsMouseDraggingOn();
    return false;
}

//--------------------------------------------------------
// Time 
//--------------------------------------------------------
void NFmiTimeStationViewRowList::Time(const NFmiMetTime& theTime)
{
	NFmiCtrlView::Time(theTime);
	if(itsViewList)
		itsViewList->Time(theTime);
}

//--------------------------------------------------------
// CreateRowView 
//--------------------------------------------------------
NFmiTimeStationViewRow* NFmiTimeStationViewRowList::CreateRowView(int theIndex)
{
	NFmiRect rect = CalcViewRect(theIndex);
	itsMapArea->SetXYArea(rect); // siirt�� suhteellist� aluetta

    NFmiTimeStationViewRow* returnVal = new NFmiTimeStationViewRow(itsMapViewDescTopIndex, itsMapArea
																  ,itsToolBox
																  ,itsDrawingEnvironment
																  ,itsDrawParam
																  ,theIndex);
   return returnVal;
}
//--------------------------------------------------------
// CalcViewRect 
//--------------------------------------------------------
// Laskee indeksill� merkityn n�ytt�rivin alueen.
NFmiRect NFmiTimeStationViewRowList::CalcViewRect(int theIndex)
{
   NFmiRect returnVal = GetFrame();
   double height = returnVal.Height() / itsRowCount;
   NFmiPoint size(returnVal.Width(), height);
   NFmiPoint place(returnVal.TopLeft().X(), (theIndex - 1)* height);
   returnVal.Place(place);
   returnVal.Size(size);
   return returnVal;
}
//--------------------------------------------------------
// CreateRows 
//--------------------------------------------------------
// initialisoi itsViewList:in sopivilla NFmiTimeStationViewRow-otuksilla
void NFmiTimeStationViewRowList::CreateRows(void)
{
	if(itsViewList)
		delete itsViewList;
	itsViewList = new NFmiCtrlViewList(itsMapViewDescTopIndex, GetFrame()
									  ,itsToolBox
									  ,itsDrawingEnvironment
									  ,itsDrawParam);
	for(int i = 1; i <= itsRowCount; i++)
		itsViewList->Add(CreateRowView(i));
}

void NFmiTimeStationViewRowList::UpdateRowRects(void)
{
	if(itsViewList)
	{
		itsViewList->Reset();
		itsViewList->Next();
		NFmiRect rect;
		for(int i = 1; i <= itsRowCount; i++)
		{
			rect = CalcViewRect(i);
			itsViewList->Current()->Update(rect, itsToolBox);
			itsViewList->Next();
		}
	}
}

//--------------------------------------------------------
// StoreToolTipDataInDoc 
//--------------------------------------------------------
void NFmiTimeStationViewRowList::StoreToolTipDataInDoc(const NFmiPoint& theRelativePlace)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
		{
			if(((NFmiTimeStationViewRow*)itsViewList->Current())->StoreToolTipDataInDoc(theRelativePlace))
				return;
		}
	}
}

// t�ll� piirret��n tavara, joka tulee my�s bitmapin p��lle
void NFmiTimeStationViewRowList::DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
			((NFmiTimeStationViewRow*)itsViewList->Current())->DrawOverBitmapThings(theGTB, dummy, dummy2, dummy3, dummy4);
	}
}

std::string NFmiTimeStationViewRowList::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
		{
			if(itsViewList->Current()->GetFrame().IsInside(theRelativePoint))
				return itsViewList->Current()->ComposeToolTipText(theRelativePoint); // tooltippi tehd��n vain yhden rivin yhdest� ruudusta joten heti return kun l�ytyy oikea rivi
		}
	}
	return std::string();
}

NFmiCtrlView* NFmiTimeStationViewRowList::GetView(int theRowIndex, const NFmiMetTime &theTime, const NFmiDataIdent &theDataIdent, bool fUseParamIdOnly)
{
	if(itsViewList)
	{
		int counter = 1; // indeksit alkavat 1:st�
		for(itsViewList->Reset(); itsViewList->Next(); counter++)
		{
			if(counter == theRowIndex)
			{
				NFmiTimeStationViewRow *rowView = dynamic_cast<NFmiTimeStationViewRow *>(itsViewList->Current());
				if(rowView)
					return rowView->GetView(theTime, theDataIdent, fUseParamIdOnly);
			}
		}
	}
	return 0;
}

NFmiStationViewHandler* NFmiTimeStationViewRowList::GetMapView(int theRowIndex, int theIndex)
{
	if(itsViewList)
	{
		int counter = 1; // indeksit alkavat 1:st�
		for(itsViewList->Reset(); itsViewList->Next(); counter++)
		{
			if(counter == theRowIndex)
			{
				NFmiTimeStationViewRow *rowView = dynamic_cast<NFmiTimeStationViewRow *>(itsViewList->Current());
				if(rowView)
                    return rowView->GetMapView(theIndex);
			}
		}
	}
	return 0;
}
