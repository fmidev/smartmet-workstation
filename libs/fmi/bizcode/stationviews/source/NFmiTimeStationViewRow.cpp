//**********************************************************
// C++ Class Name : NFmiTimeStationViewRow
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiTimeStationViewRow.cpp
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
#include "NFmiTimeStationViewRow.h"
#include "NFmiArea.h"
#include "NFmiCtrlViewList.h"
#include "NFmiStationViewHandler.h"
#include "NFmiDrawParamList.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiToolBox.h"
#include "CtrlViewDocumentInterface.h"

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiTimeStationViewRow::NFmiTimeStationViewRow(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
											   ,NFmiToolBox * theToolBox
											   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
											   ,int theIndex)
:NFmiCtrlView(theMapViewDescTopIndex, theArea->XYArea()
			 ,theToolBox
			 ,theDrawParam)
,itsViewList(0)
,itsMapArea(theArea->Clone())
,itsViewCount(GetViewCountFromDocument())
,itsRowIndex(theIndex)
{
	CreateViewRow();
}

NFmiTimeStationViewRow::~NFmiTimeStationViewRow()
{
	delete itsViewList;
}
//--------------------------------------------------------
// Draw
//--------------------------------------------------------
void NFmiTimeStationViewRow::Draw(NFmiToolBox* theGTB)
{
	if(itsViewList)
		itsViewList->Draw(theGTB);
}

//--------------------------------------------------------
// Update
//--------------------------------------------------------
void NFmiTimeStationViewRow::Update(void)
{
	if(GetViewCountFromDocument() != itsViewCount)
	{
		itsViewCount = GetViewCountFromDocument();
		CreateViewRow();
	}
	else
		UpdateViewRowRects();

	if(itsViewList)
		for(itsViewList->Reset(); itsViewList->Next();)
			((NFmiStationViewHandler*)itsViewList->Current())->Update();
}

// Tämän pitää päivittää emo ja data osan alueet.
void NFmiTimeStationViewRow::Update(const NFmiRect & theRect, NFmiToolBox * theToolBox)
{
    NFmiCtrlView::Update(theRect, theToolBox);
	if(itsViewList)
        itsViewList->Update(theRect, theToolBox);
	UpdateViewRowRects();
}

//--------------------------------------------------------
// LeftButtonDown
//--------------------------------------------------------
bool NFmiTimeStationViewRow::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		if(itsViewList && itsViewList->IsIn(thePlace))
		{
			bool result = itsViewList->LeftButtonDown(thePlace, theKey);
			return result;
		}
	}
	return false;
}
//--------------------------------------------------------
// LeftButtonUp
//--------------------------------------------------------
bool NFmiTimeStationViewRow::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
    if(itsViewList)
	{
		if(itsViewList->IsMouseDraggingOn() || itsViewList->IsIn(thePlace))
		{
			bool result = itsViewList->LeftButtonUp(thePlace, theKey);
			return result;
		}
	}
	return false;
}

bool NFmiTimeStationViewRow::MiddleButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		if(itsViewList && itsViewList->IsIn(thePlace))
		{
			bool result = itsViewList->MiddleButtonDown(thePlace, theKey);
			return result;
		}
	}
	return false;
}

bool NFmiTimeStationViewRow::MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		if(itsViewList && itsViewList->IsIn(thePlace))
		{
			bool result = itsViewList->MiddleButtonUp(thePlace, theKey);
			return result;
		}
	}
	return false;
}

bool NFmiTimeStationViewRow::LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) // Marko lisäsi 14.12.2001
{
	if(IsIn(thePlace))
	{
		if(itsViewList && itsViewList->IsIn(thePlace))
		{
			bool result = itsViewList->LeftDoubleClick(thePlace, theKey);
			return result;
		}
	}
	return false;
}

bool NFmiTimeStationViewRow::RightDoubleClick(const NFmiPoint &thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		if(itsViewList && itsViewList->IsIn(thePlace))
		{
			bool result = itsViewList->RightDoubleClick(thePlace, theKey);
			return result;
		}
	}
	return false;
}

//--------------------------------------------------------
// RightButtonUp
//--------------------------------------------------------
bool NFmiTimeStationViewRow::RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		if(itsViewList && itsViewList->IsIn(thePlace))
			return itsViewList->RightButtonUp(thePlace, theKey);
	}
	return false;
}

bool NFmiTimeStationViewRow::MouseMove(const NFmiPoint& thePlace, unsigned long theKey)
{
    if(itsViewList)
    {
        // Jos jossain on menossa hiiren raahaus TAI ollaan näytön sisällä, 
        // tällöin tarkastellaan hiiren likkeitä täällä.
        if(itsViewList->IsMouseDraggingOn() || IsIn(thePlace))
        {
            return itsViewList->MouseMove(thePlace, theKey);
        }
    }
	return false;
}

bool NFmiTimeStationViewRow::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(IsIn(thePlace))
	{
		if(itsViewList->MouseWheel(thePlace, theKey, theDelta))
			return true;
	}
	return false;
}

bool NFmiTimeStationViewRow::IsMouseDraggingOn(void)
{
    if(itsViewList)
        return itsViewList->IsMouseDraggingOn();
    return false;
}

int NFmiTimeStationViewRow::GetViewCountFromDocument()
{
    return static_cast<int>(GetCtrlViewDocumentInterface()->ViewGridSize(itsMapViewDescTopIndex).X());
}

NFmiMetTime NFmiTimeStationViewRow::GetUsedTimeForViewTile(const NFmiMetTime& theStartTime, long theTimeStepInMinutes, int theRowIndex, int theColumnIndex)
{
    auto mapvieDisplayMode = itsCtrlViewDocumentInterface->MapViewDisplayMode(itsMapViewDescTopIndex);
	if(mapvieDisplayMode == CtrlViewUtils::MapViewMode::kOneTime)
		return theStartTime; // tässä moodissa jokaisessa ruudussa on sama aika
	else if(mapvieDisplayMode == CtrlViewUtils::MapViewMode::kRunningTime)
	{ // jokainen näyttöruutu on omaa aikaa, jokaisella ruudulla on samat parametrit. Ruudukon juoksutus menee
		// vasemmalta oikealle ja ylhäältä alas. Esim. Näyttö on 4x3 ruudukossa.
		// itsRowIndex = 2 eli 2. rivi (indeksit alkavat 1:stä).
		// itsIndex on 3 eli 3. sarake (indeksit 1:stä)
		// ((2-1) * 4) + 3 = 7    (4 on siis ruudukon x-koko)

		// eli lasketaan monesko ruutu on kyseessä ja siirretään aikaa sen mukaan
		int totalViewTileIndex = ((theRowIndex-1) * GetViewCountFromDocument()) + theColumnIndex;
		long moveTimeInMinutes = theTimeStepInMinutes * (totalViewTileIndex - 1);
		NFmiMetTime aTime(theStartTime);
		aTime.ChangeByMinutes(moveTimeInMinutes);
		return aTime;
	}
	else
	{	// normaali moodissa pitää siirtää alkuaikaa niin paljon kuin sarakkeita on (sarakkeista pitää vähentää 1, koska 1. sarakkeella on alkuaika)
		long moveTimeInMinutes = theTimeStepInMinutes * (theColumnIndex - 1);
		NFmiMetTime aTime(theStartTime);
		aTime.ChangeByMinutes(moveTimeInMinutes);
		return aTime;
	}
}

//--------------------------------------------------------
// Time
//--------------------------------------------------------
void NFmiTimeStationViewRow::Time(const NFmiMetTime& theTime)
{
	long usedTimeStep = boost::math::iround(itsCtrlViewDocumentInterface->TimeControlTimeStep(itsMapViewDescTopIndex)*60); // * 60 tunnit -> minuuteiksi
	NFmiMetTime tmpTime(theTime);
	if(usedTimeStep < tmpTime.GetTimeStep())
		tmpTime.SetTimeStep(static_cast<short>(usedTimeStep)); // pitää laittaa tämän näytön time steppi päälle, koska theTime:ssa voi olla isompi kuin tänne on asetettu
	NFmiCtrlView::Time(tmpTime);
	if(itsViewList)
	{
		int i=1;
		for(itsViewList->Reset(); itsViewList->Next(); i++)
			itsViewList->Current()->Time(GetUsedTimeForViewTile(tmpTime, usedTimeStep, itsRowIndex, i));
	}
}

//--------------------------------------------------------
// CreateViewRow
//--------------------------------------------------------
// luo itsViewList:in
void NFmiTimeStationViewRow::CreateViewRow(void)
{
	if(itsViewList)
		delete itsViewList;
	itsViewList = new NFmiCtrlViewList(itsMapViewDescTopIndex, GetFrame()
									  ,itsToolBox
									  ,itsDrawParam
									  ,true);

	for(int i = 1; i <= itsViewCount; i++)
		itsViewList->Add(CreateMapView(i));

}

void NFmiTimeStationViewRow::UpdateViewRowRects(void)
{
	if(itsViewList)
	{
		itsViewList->Reset();
		itsViewList->Next();
		NFmiRect rect;
		for(int i = 1; i <= itsViewCount; i++)
		{
			rect = CalcViewRect(i);
			itsViewList->Current()->Update(rect, itsToolBox);
			itsViewList->Next();
		}
	}
}

//--------------------------------------------------------
// CalcViewRect
//--------------------------------------------------------

//   Laskee n:en ruudun suhteellisen rect:in.
//    Indeksi alkaa 1:stä.
NFmiRect NFmiTimeStationViewRow::CalcViewRect(int theIndex)
{
   NFmiRect returnVal = GetFrame();
   double width = returnVal.Width() / itsViewCount;
   NFmiPoint size(width, returnVal.Height());
   NFmiPoint place((theIndex - 1)* width, returnVal.TopLeft().Y());
   returnVal.Place(place);
   returnVal.Size(size);
   return returnVal;
}
//--------------------------------------------------------
// CreateMapView
//--------------------------------------------------------

//   Luo halutun kohdan näytön (eli esim. 3. ruudun).
//   Indeksi alkaa 1:stä.
NFmiStationViewHandler* NFmiTimeStationViewRow::CreateMapView(int theIndex)
{
	NFmiRect rect = CalcViewRect(theIndex);
	itsMapArea->SetXYArea(rect); // siirtää suhteellistä aluetta

	NFmiStationViewHandler* stationViewHandler = new NFmiStationViewHandler(itsMapViewDescTopIndex, itsMapArea
												  ,itsToolBox
												  ,itsDrawParam
												  ,itsRowIndex
												  ,theIndex);
   return stationViewHandler;
}

//   Indeksi alkaa 1:stä.
NFmiStationViewHandler* NFmiTimeStationViewRow::GetMapView(int theIndex)
{
	if(itsViewList)
        if(itsViewList->Index(theIndex))
            return dynamic_cast<NFmiStationViewHandler*>(itsViewList->Current());
    return 0;
}

// muuttaa annetun (esim. hiirellä osoitetun) suhteellisen (0,0 1,1) pisteen oikeaksi
// kartan latlon pisteeksi ja tallettaa latlon-pisteen ja ajan dokumenttiin
bool NFmiTimeStationViewRow::StoreToolTipDataInDoc(const NFmiPoint& theRelativePlace)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
		{
			if(itsViewList->Current()->IsIn(theRelativePlace))
			{
				((NFmiStationViewHandler*)itsViewList->Current())->StoreToolTipDataInDoc(theRelativePlace);
				return true;
			}
		}
	}
	return false;
}

// Sisältääkö tämä karttanäyttörivi sen hiirellä aktiiviseksi klikatun karttaruudun?
bool NFmiTimeStationViewRow::HasActiveRowView()
{
	if(itsViewList)
	{
		auto &combinedMapHandler = itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface();
		auto realActiveRowIndex = combinedMapHandler.absoluteActiveViewRow(itsMapViewDescTopIndex);
		for(itsViewList->Reset(); itsViewList->Next(); )
		{
			auto *currentView = itsViewList->Current();
			auto currentViewRealRowIndex = currentView->CalcRealRowIndex();
			if(realActiveRowIndex == currentViewRealRowIndex)
				return true;
		}
	}
	return false;
}

// tällä piirretään tavara, joka tulee myös bitmapin päälle
void NFmiTimeStationViewRow::DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
		{
			((NFmiStationViewHandler*)itsViewList->Current())->DrawOverBitmapThings(theGTB, dummy, itsViewCount, dummy3, dummy4); // itsViewCount kertoo viimeisen ruudun indeksin!
		}
	}
}

std::string NFmiTimeStationViewRow::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
		{
			if(itsViewList->Current()->GetFrame().IsInside(theRelativePoint))
			{
				return itsViewList->Current()->ComposeToolTipText(theRelativePoint); // tooltippi tehdään vain yhden rivin yhdestä ruudusta joten heti return kun löytyy oikea rivi/ruutu
			}
		}
	}
	return std::string();
}

NFmiCtrlView* NFmiTimeStationViewRow::GetViewWithRealRowIndex(int theRealRowIndex, const NFmiMetTime &theTime, const NFmiDataIdent &theDataIdent, bool fUseParamIdOnly)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
		{
			auto* ctrlView = itsViewList->Current();
			if(theRealRowIndex == ctrlView->CalcRealRowIndex())
			{
				if(itsViewList->Current()->Time() == theTime)
				{
					NFmiStationViewHandler* view = dynamic_cast<NFmiStationViewHandler*>(itsViewList->Current());
					if(view)
					{
						return view->GetView(theDataIdent, fUseParamIdOnly);
					}
				}
			}
		}
	}
	return 0;
}
