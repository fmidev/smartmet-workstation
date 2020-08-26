//**********************************************************
// C++ Class Name : NFmiParamHandlerView 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiParamHandlerView.cpp 
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
// 
//**********************************************************
#include "NFmiParamHandlerView.h"
#include "NFmiToolBox.h"
#include "NFmiCtrlViewList.h"
#include "NFmiViewParamCommandView.h"
#include "NFmiViewParamsView.h"
#include "NFmiMaskParamCommandView.h"
#include "NFmiMaskParamsView.h"
#include "NFmiRectangle.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiParamHandlerView::NFmiParamHandlerView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, NFmiDrawingEnvironment * theDrawingEnvi, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex, bool theShowMaskSection, bool viewParamsViewHasMapLayer)
:NFmiParamCommandView(theMapViewDescTopIndex, theRect, theToolBox, theDrawingEnvi, theDrawParam, theRowIndex, theColumnIndex, false)
,itsViewList(new NFmiCtrlViewList)
,itsViewParamCommandView(0)
,itsViewParamsView(0)
,itsMaskParamCommandView(0)
,itsMaskParamsView(0)
,fMouseCaptured(false)
,fShowMaskSection(theShowMaskSection)
,fViewParamsViewHasMapLayer(viewParamsViewHasMapLayer)
{
}

NFmiParamHandlerView::~NFmiParamHandlerView(void)
{
	delete itsViewList;
}
//--------------------------------------------------------
// Update 
//--------------------------------------------------------
void NFmiParamHandlerView::Update(const NFmiRect & theRect , NFmiToolBox * theToolBox)
{
	const double frameDivader = 7.; // miss� suhteess� 'n�yt�' ja param ikkuna jaetaan leveys suunnassa
	itsToolBox = theToolBox;
	// aluksi pit�� s��t�� annettu leveys, korkeus lasketaan t��ll�
	itsRect.Width(theRect.Width());
	itsRect.Place(theRect.TopLeft()); // pidet��n topleft-kulma siin�, miss� on annettussa thRect-parametrissa

	UpdateTextData();
	if(itsViewParamCommandView && itsViewParamsView)
	{
		NFmiRect rect11 = itsViewParamCommandView->CalcSize();
		rect11.Width(GetFrame().Width()/frameDivader);
		rect11.Place(theRect.TopLeft()); // pidet��n topleft-kulma siin�, miss� on annettussa thRect-parametrissa
		NFmiRect rect12 = itsViewParamsView->CalcSize();
		rect12.Left(rect11.Right());
		rect12.Width(GetFrame().Width() - GetFrame().Width()/frameDivader); // annetaan 'loput' leveydest� rinnakkais ikkunalle
		double higherValue = FmiMax(rect11.Height(), rect12.Height());
		double newTop = theRect.Top();
		rect11.Top(newTop);
		rect12.Top(newTop);
		rect11.Bottom(newTop + higherValue);
		rect12.Bottom(newTop + higherValue);
		itsViewParamCommandView->Update(rect11, itsToolBox);
		itsViewParamsView->Update(rect12, itsToolBox);
	}

	// maski-n�ytt� laitetaan vain 1. rivin 1. sarakkeeseen
	if((fShowMaskSection == false || (itsViewGridRowNumber != 1 || itsViewGridColumnNumber != 1)))
	{
		if(itsViewParamCommandView) 
		{
			NFmiRect rect(GetFrame());
			rect.Top(itsViewParamCommandView->GetFrame().Top());
			rect.Bottom(itsViewParamCommandView->GetFrame().Bottom());
			NFmiParamCommandView::Update(rect, itsToolBox);
		}
		if(itsMaskParamCommandView)
			itsMaskParamCommandView->ShowView(false);
		if(itsMaskParamsView)
			itsMaskParamsView->ShowView(false);
	}
	else
	{
		if(itsMaskParamCommandView && itsMaskParamsView && itsViewParamCommandView)
		{
			NFmiRect rect31 = itsMaskParamCommandView->CalcSize();
			rect31.Width(GetFrame().Width()/frameDivader);
			NFmiRect rect32 = itsMaskParamsView->CalcSize();
			rect32.Left(rect31.Right());
			rect32.Width(GetFrame().Width() - GetFrame().Width()/frameDivader); // annetaan 'loput' leveydest� rinnakkais ikkunalle
			double higherValue = FmiMax(rect31.Height(), rect32.Height());
			double newTop = itsViewParamCommandView->GetFrame().Bottom();
			rect31.Top(newTop);
			rect32.Top(newTop);
			rect31.Bottom(newTop + higherValue);
			rect32.Bottom(newTop + higherValue);
			itsMaskParamCommandView->Update(rect31, itsToolBox);
			itsMaskParamsView->Update(rect32, itsToolBox);
			itsMaskParamCommandView->ShowView(true);
			itsMaskParamsView->ShowView(true);
		}

		if(itsViewParamCommandView && itsMaskParamCommandView) 
		{
			NFmiRect rect(GetFrame());
			rect.Top(itsViewParamCommandView->GetFrame().Top());
			rect.Bottom(itsMaskParamCommandView->GetFrame().Bottom());
			NFmiParamCommandView::Update(rect, itsToolBox);
		}
	}
}
//--------------------------------------------------------
// Init 
//--------------------------------------------------------
bool NFmiParamHandlerView::Init()
{
	if(itsViewParamCommandView)
		delete itsViewParamCommandView;
	NFmiRect rect(GetFrame());
	NFmiRect viewParamCommandViewRect(rect);
	viewParamCommandViewRect.Bottom(rect.Top()+rect.Height()/3.);
	viewParamCommandViewRect.Right(rect.Left()+rect.Width()/2.5);
	itsViewParamCommandView = new NFmiViewParamCommandView(itsMapViewDescTopIndex, viewParamCommandViewRect, itsToolBox, itsDrawingEnvironment, itsDrawParam, itsViewGridRowNumber, itsViewGridColumnNumber);
	itsViewList->Add(itsViewParamCommandView, false);

	NFmiRect viewParamsViewRect(rect);
	viewParamsViewRect.Bottom(rect.Top()+rect.Height()/3.);
	viewParamsViewRect.Left(viewParamCommandViewRect.Right());
	itsViewParamsView = new NFmiViewParamsView(itsMapViewDescTopIndex, viewParamsViewRect, itsToolBox, itsDrawingEnvironment, itsDrawParam, itsViewGridRowNumber, itsViewGridColumnNumber, fViewParamsViewHasMapLayer);
	itsViewList->Add(itsViewParamsView, false);

	NFmiRect maskParamCommandViewRect(rect);
	maskParamCommandViewRect.Top(viewParamCommandViewRect.Bottom());
	maskParamCommandViewRect.Bottom(maskParamCommandViewRect.Top()+rect.Height()/3.);
	maskParamCommandViewRect.Right(viewParamCommandViewRect.Right());
	itsMaskParamCommandView = new NFmiMaskParamCommandView(itsMapViewDescTopIndex, maskParamCommandViewRect, itsToolBox, itsDrawingEnvironment, itsDrawParam, itsViewGridRowNumber, itsViewGridColumnNumber);
	itsViewList->Add(itsMaskParamCommandView, false);

	NFmiRect maskParamsViewRect(rect);
	maskParamsViewRect.Top(maskParamCommandViewRect.Top());
	maskParamsViewRect.Bottom(maskParamCommandViewRect.Bottom());
	maskParamsViewRect.Left(viewParamCommandViewRect.Right());
	itsMaskParamsView = new NFmiMaskParamsView(itsMapViewDescTopIndex, maskParamsViewRect, itsToolBox, itsDrawingEnvironment, itsDrawParam, itsViewGridRowNumber, itsViewGridColumnNumber);
	itsViewList->Add(itsMaskParamsView, false);

	return true;
}
//--------------------------------------------------------
// Draw 
//--------------------------------------------------------
void NFmiParamHandlerView::Draw(NFmiToolBox * theToolBox)
{
    UpdateTextData();
    DrawBackground();
    DrawData();
    if(itsViewList)
        itsViewList->Draw(theToolBox);
}

//--------------------------------------------------------
// UpdateFontSize 
//--------------------------------------------------------
void NFmiParamHandlerView::UpdateTextData(void)
{
	CalcTextData(); // t�m� tehd��n aina kun ruudun kokoa muutetaan (Update) NFmiParamhandler:issa joka jakaa tiedon listassaan oleville view:lle
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next(); )
			(static_cast<NFmiParamCommandView*>(itsViewList->Current())->UpdateTextData(itsFontSize, itsLineHeight, itsCheckBoxSize, itsPixelSize));
	}
}
//--------------------------------------------------------
// RightButtonDown 
//--------------------------------------------------------
bool NFmiParamHandlerView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(itsViewList)
		return itsViewList->RightButtonUp(thePlace, theKey);
	return false;
}

bool NFmiParamHandlerView::LeftButtonDown(const NFmiPoint& thePlace, unsigned long theKey)
{
	fMouseCaptured = true;
    if(itsViewList)
        return itsViewList->LeftButtonDown(thePlace, theKey);
    return false;
}

bool NFmiParamHandlerView::LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{ 
	fMouseCaptured = false;
	if(itsViewList)
		return itsViewList->LeftButtonUp(thePlace, theKey);
	return false;
}

bool NFmiParamHandlerView::LeftDoubleClick(const NFmiPoint& thePlace, unsigned long theKey) // Marko lis�si 17.12.2001
{
	if(itsViewList)
		return itsViewList->LeftDoubleClick(thePlace, theKey);
	return false;
}

bool NFmiParamHandlerView::MouseMove(const NFmiPoint& thePlace, unsigned long theKey)
{ 
	if(fMouseCaptured)
	{
        if(itsViewList)
            return itsViewList->MouseMove(thePlace, theKey);
    }
	return false;
}

bool NFmiParamHandlerView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(itsViewList)
		return itsViewList->MouseWheel(thePlace, theKey, theDelta);
	return false;
}

bool NFmiParamHandlerView::IsMouseDraggingOn(void)
{
    if(itsViewList)
        return itsViewList->IsMouseDraggingOn();
    return false;
}

void NFmiParamHandlerView::DrawBackground(void)
{
	itsDrawingEnvironment->SetFrameColor(NFmiColor(0,0,0));
	itsDrawingEnvironment->SetFillColor(NFmiCtrlView::gGreyColorDefault);
	itsDrawingEnvironment->EnableFill();
	NFmiRect frame = GetFrame();
	NFmiRectangle tmp(frame.TopLeft(),
			  frame.BottomRight(),
			  0,
			  itsDrawingEnvironment);
	itsToolBox->Convert(&tmp);
}
