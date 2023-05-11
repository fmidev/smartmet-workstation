#include "NFmiTimeValueEditManagerView.h"
#include "NFmiDrawParamList.h"
#include "NFmiDrawParam.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiTimeSerialView.h"
#include "NFmiTimeSerialPrecipitationFormView.h"
#include "NFmiTimeSerialPrecipitationTypeView.h"
#include "NFmiTimeSerialFogIntensityView.h"
#include "NFmiTimeSerialSymbolView.h"
#include "NFmiToolBox.h"
#include "NFmiCtrlViewList.h"
#include "NFmiTimeControlView.h"
#include "NFmiRectangle.h"
#include "NFmiStepTimeScale.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "SpecialDesctopIndex.h"


const double kFmiEmptySpaceBetweenSubViews = 100.;
const long kFmiSymbolViewHeigth = 70;
//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiTimeValueEditManagerView::NFmiTimeValueEditManagerView
					(const NFmiRect & theRect
					,NFmiToolBox * theToolBox
					,NFmiDrawingEnvironment * theDrawingEnvi)
:NFmiCtrlView(0, theRect
			 ,theToolBox
			 ,theDrawingEnvi)
,itsViewList(new NFmiCtrlViewList())
,itsViewCount(0)
,itsSymbolViewCount(0) //laura lis�si
,itsTimeAxis(0)
,itsUpperTimeView(0)
,fTimeScaleUpdated(false)
{
}

NFmiTimeValueEditManagerView::~NFmiTimeValueEditManagerView(void)
{
	delete itsTimeAxis;
	delete itsUpperTimeView;
	delete itsViewList;
}
//--------------------------------------------------------
// Draw
//--------------------------------------------------------
void NFmiTimeValueEditManagerView::Draw(NFmiToolBox* theToolBox)
{
   CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, std::string(__FUNCTION__) + ": time-serial-view drawing starts");
   DrawBackground();
   if(itsCtrlViewDocumentInterface->UseTimeSerialAxisAutoAdjust()) // jos auto asteikko s��t� optio on p��ll�, s��det��n ne t�ss� yhteisesti enen piirtoa
	   AutoAdjustValueScales(false);
   UpdateTimeSerialViews();

   DrawTimeAxisView();
   itsViewList->Draw(theToolBox);
}

void NFmiTimeValueEditManagerView::UpdateTimeSerialViews(void)
{
	int rowIndex = 1;
	for(itsViewList->Reset(); itsViewList->Next(); )
	{
		itsViewList->Current()->Update(CalcListViewRect(rowIndex),itsToolBox);
		itsViewList->Current()->ViewGridRowNumber(rowIndex);
		rowIndex++;
	}
}

//--------------------------------------------------------
// DrawData
//--------------------------------------------------------
void NFmiTimeValueEditManagerView::DrawData (void)
{
   return;
}
//--------------------------------------------------------
// DrawBase
//--------------------------------------------------------
void NFmiTimeValueEditManagerView::DrawBase (void)
{
   return;
}

//--------------------------------------------------------
// MaxStationShowed
//--------------------------------------------------------
// POISTA T�M�!!!!!!!!
void NFmiTimeValueEditManagerView::MaxStationShowed (const unsigned int & newCount)
{
	for(itsViewList->Reset(); itsViewList->Next(); )
	{
		((NFmiTimeSerialView*)itsViewList->Current())->MaxStationShowed(newCount);
	}
}

//--------------------------------------------------------
// LeftButtonUp
//--------------------------------------------------------
bool NFmiTimeValueEditManagerView::ChangeTimeSeriesValues(void)
{
	bool anyDataModified = false;
	unsigned long viewCount = itsViewList->NumberOfItems();
	for(unsigned long i = 1; i <= viewCount; i++)
	{
		if(itsViewList->Index(i))
		{
			NFmiTimeSerialView *view = dynamic_cast<NFmiTimeSerialView*>(itsViewList->Current());
			if(view && view->DrawParam()->IsParamEdited()) // operaatio tehd��n vain aktiiviselle n�yt�lle
			{
				anyDataModified = true;
				view->ChangeTimeSeriesValues();
			}
		}
	}
	return anyDataModified;
	// Alla oleva originaali koodi ei sovi ollenkaan multi-thread + ProgressCancel -editointi filosofiaan. Korjasin yll� 
	// olevan koodin niin ett� ei k�ytet� listan omia sis�isi� iteraattoreita, vaan ulkoista indeksi�.
/*
	for(itsViewList->Reset(); itsViewList->Next(); )
	{
		((NFmiTimeSerialView*)itsViewList->Current())->ChangeTimeSeriesValues(); // todellisuudessa operaatio tehd��n vain aktiiviselle n�yt�lle
	}
*/
}

bool NFmiTimeValueEditManagerView::MouseMove(const NFmiPoint &thePlace, unsigned long theKey)
{
	bool status = false;
	if(itsUpperTimeView)
	{
		status = itsUpperTimeView->MouseMove(thePlace, theKey);
		NFmiTimeControlView::MouseStatusInfo mouseStatus = itsUpperTimeView->GetMouseStatusInfo();
		if(status || (mouseStatus.NeedsUpdate()))
		{
			status = true; // NFmiTimeControlView::MouseMove -metodi palauttaa joka tapauksessa false:n, joten pieni kikka t�h�n ett� tied�mme, pik��k� staus muuttaa true:ksi
			itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
		}
	}
	
	if(!status)
	{
		status = itsViewList->MouseMove(thePlace, theKey);
	}
   return status;
}

//--------------------------------------------------------
// LeftButtonUp
//--------------------------------------------------------
bool NFmiTimeValueEditManagerView::LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey)
{
	fTimeScaleUpdated = false;
	NFmiTimeControlView::MouseStatusInfo mouseStatusWas;
	if(itsUpperTimeView)
		mouseStatusWas = itsUpperTimeView->GetMouseStatusInfo();

	bool status = itsViewList->LeftButtonUp(thePlace, theKey);
	if(!status && itsUpperTimeView)
		status = itsUpperTimeView->LeftButtonUp(thePlace, theKey);
	if(!status)
	{
		auto rowIndex = CalcRowIndex(thePlace);
		if(rowIndex == 0)
			rowIndex = 1;
		itsCtrlViewDocumentInterface->SetLastActiveDescTopAndViewRow(CtrlViewUtils::kFmiTimeSerialView, rowIndex);
	}
	else
        itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
	if(mouseStatusWas.fMouseCapturedMoveTimeLine || mouseStatusWas.fMouseCapturedMoveSelectedTime) // jos oltiin siirt�m�ss� aikajanaa hiirell� raahaamalla, pit�� karttan�ytt� p�ivitt�� lopuksi
		fTimeScaleUpdated = true;

   return status;
}

bool NFmiTimeValueEditManagerView::LeftDoubleClick(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(itsUpperTimeView && itsUpperTimeView->IsIn(thePlace))
		return itsUpperTimeView->LeftDoubleClick(thePlace, theKey);
	if(IsIn(thePlace))
		return itsViewList->LeftDoubleClick(thePlace, theKey);

	return false;
}

bool NFmiTimeValueEditManagerView::MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(itsUpperTimeView && itsUpperTimeView->MiddleButtonUp(thePlace, theKey))
	{
        itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
		return true;
	}
	return false;
}

//--------------------------------------------------------
// RightButtonUp
//--------------------------------------------------------
bool NFmiTimeValueEditManagerView::RightButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	bool status = itsViewList->RightButtonDown(thePlace, theKey);
	if(!status)
	{
		// TEE IKKUNAN OMAT JUTUT T�SS�
	}
   return status;
}
//--------------------------------------------------------
// LeftButtonDown
//--------------------------------------------------------
bool NFmiTimeValueEditManagerView::LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey)
{
	bool status = itsViewList->LeftButtonDown(thePlace, theKey);
	if(!status && itsUpperTimeView)
		status = itsUpperTimeView->LeftButtonDown(thePlace, theKey);
	if(!status)
	{
		// TEE IKKUNAN OMAT JUTUT T�SS�
	}
   return status;
}

// Jos ollaan aikakontrolliikkunassa, tai ollaan alueen ulkopuolella, palautetaan 0.
// Jos ollaan 1. rivin kohdalla tai sen yl�puolella, palautetaan 1.
// Jos ollaan 2. rivin kohdalla tai sen yl�puolella, palautetaan 2, jne.
// Jos ollaan viimeisin rivin alapuolella, palautetaan (rivien_lkm + 1).
int NFmiTimeValueEditManagerView::CalcRowIndex(const NFmiPoint& thePlace)
{
	if(itsUpperTimeView && itsUpperTimeView->IsIn(thePlace))
		return 0;

	if(IsIn(thePlace))
	{
		NFmiRect rect;
		for(int rowIndex = 1; rowIndex <= itsViewCount; rowIndex++)
		{
			itsViewList->Index(rowIndex);
			rect = CalcListViewRect(rowIndex);
			if(rect.Bottom() >= thePlace.Y())
			{
				return rowIndex;
			}
		}
		return itsViewCount + 1;
	}

	return 0;
}

//--------------------------------------------------------
// RightButtonDown
//--------------------------------------------------------
bool NFmiTimeValueEditManagerView::RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	bool status = itsViewList->RightButtonUp(thePlace, theKey);
	if(!status && itsUpperTimeView && itsUpperTimeView->IsIn(thePlace))
		status = itsUpperTimeView->RightButtonUp(thePlace, theKey);
	if(!status)
	{
		if(IsIn(thePlace))
		{
			NFmiRect rect;
			for(int i=1; i<=itsViewCount; i++)
			{
				itsViewList->Index(i); //laura lis�si 011099
				rect = CalcListViewRect(i);
				if(rect.TopLeft().Y() >= thePlace.Y())
				{
                    itsCtrlViewDocumentInterface->CreateTimeSerialDialogPopup(i);
					return true;
				}
			}
            itsCtrlViewDocumentInterface->CreateTimeSerialDialogPopup(itsViewCount+1);
			return true;
		}
	}
	else
	{
		fTimeScaleUpdated = true;
        itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
	}
   return status;
}
//--------------------------------------------------------
// DrawBackground
//--------------------------------------------------------
void NFmiTimeValueEditManagerView::DrawBackground (void)
{
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.EnableFrame();
	envi.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	envi.SetFillColor(NFmiCtrlView::gGreyColorDefault);
	NFmiRectangle rect(itsRect.TopLeft()
					  ,itsRect.BottomRight()
					  ,0
					  ,&envi);
	itsToolBox->Convert(&rect);
}
//--------------------------------------------------------
// Update
//--------------------------------------------------------
void NFmiTimeValueEditManagerView::Update(const NFmiRect & theRect, NFmiToolBox * theToolBox)
{ // t�t� kutsutaan kun loogisen alueen kokoa tai paikkaa halutaan muuttaa
    // T�t� kutsutaan mm. CTimeEditValuesView::OnSize -metodista. T�ll�in pit�� p�ivitt�� 
    // aikakontrolli-ikkunan koko, enk� tied� siihen t�ll� hetkell� muuta keinoa kuin luoda se uudestaan.
	if(itsCtrlViewDocumentInterface->DefaultEditedDrawParam())
		CreateTimeAxisView(itsCtrlViewDocumentInterface->TimeSerialViewTimeBag());
}

void NFmiTimeValueEditManagerView::Update(void)
{
    // t�t� kutsutaan kun halutaan paivitt�� esim. uusia parametrin�ytt�j� n�ytt�listaan
    if(itsCtrlViewDocumentInterface->TimeSerialViewDirty())
    {
        for(itsViewList->Reset(); itsViewList->Next(); )
        {
            ((NFmiTimeSerialView*)itsViewList->Current())->UpdateTimeSystem();
        }
    }

    if(itsCtrlViewDocumentInterface->DefaultEditedDrawParam())
        CreateTimeAxisView(itsCtrlViewDocumentInterface->TimeSerialViewTimeBag());
    NFmiDrawParamList* drawParamList = itsCtrlViewDocumentInterface->TimeSerialViewDrawParamList();
    if(drawParamList && drawParamList->IsDirty())
    {
        bool fWasAnyParamSetToBeEdited = false;
        NFmiCtrlViewList* oldViewList = itsViewList;
        itsViewList = new NFmiCtrlViewList();

        int index = 0;
        itsViewCount = drawParamList->NumberOfItems();

        itsSymbolViewCount = 0;
        for(drawParamList->Reset(); drawParamList->Next(); )
        {
            index++;
            boost::shared_ptr<NFmiDrawParam> drawParam = drawParamList->Current();

            //asetetaan prioriteetti 0:ksi, ett� saadaan oikea j�rjestys
            //seuraavaa t�ytyy muuttaa, jos pikkuikkunoita halutaan muistakin parametreista
            if(drawParam->Param().GetParam()->GetIdent() == kFmiWeatherSymbol3)//t�h�n mahd muut my�s
            {
                drawParamList->Current()->Priority(0);
                itsSymbolViewCount++; //kokeillaan
            }

            if(drawParam->IsParamEdited())
                fWasAnyParamSetToBeEdited = true;
            itsViewList->Add(CreateTimeSerialView(drawParam, index), false); // false = ei sorttaa n�ytt�j�
        }
        itsViewList->Sort();

        if(!fWasAnyParamSetToBeEdited && index >= 1)
        { // jos mik��n n�yt�n parametreista ei ollut editoitavana, asetetaan 1. param editoitavaksi
            drawParamList->Reset();
            drawParamList->Next();
            drawParamList->Current()->EditParam(true);
        }
        drawParamList->Dirty(false);
        delete oldViewList;
    }
}

//--------------------------------------------------------
// FindView
//--------------------------------------------------------
// funktion tarkoitus on kertoa, l�ytyyk� annetusta viewlistasta sellaista viewt�, jolla on
// annettu drawparam-pointteri
bool NFmiTimeValueEditManagerView::FindView(NFmiCtrlViewList* theViewList, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	for(theViewList->Reset(); theViewList->Next(); )
		if(theDrawParam == theViewList->Current()->DrawParam()) // HUOM! pointteri vertailu
			return true;
	return false;
}

//--------------------------------------------------------
// CreateTimeSerialView
//--------------------------------------------------------
NFmiTimeSerialView* NFmiTimeValueEditManagerView::CreateTimeSerialView(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int index)
{
	NFmiTimeSerialView* view = 0;
	if(theDrawParam)
	{
		NFmiRect rect(CalcListViewRect(index));
		FmiParameterName parName = (FmiParameterName) theDrawParam->Param().GetParam()->GetIdent();

		{
			switch (parName)
			{
            case kFmiPrecipitationForm:
            case kFmiPotentialPrecipitationForm:
				view = new NFmiTimeSerialPrecipitationFormView(rect
															   ,itsToolBox
															   ,itsDrawingEnvironment
															   ,theDrawParam
															   ,index);
				break;
			case kFmiPrecipitationType:
				view = new NFmiTimeSerialPrecipitationTypeView(rect
															   ,itsToolBox
															   ,itsDrawingEnvironment
															   ,theDrawParam
															   ,index);
				break;
			case kFmiFogIntensity:
				view = new NFmiTimeSerialFogIntensityView(rect
														   ,itsToolBox
														   ,itsDrawingEnvironment
														   ,theDrawParam
														   ,index);
				break;
			case kFmiWeatherSymbol3:
				view = new NFmiTimeSerialSymbolView(rect
												   ,itsToolBox
												   ,itsDrawingEnvironment
												   ,theDrawParam
												   ,index);
				break;
			default:
				view = new NFmiTimeSerialView(rect
											 ,itsToolBox
											 ,itsDrawingEnvironment
											 ,theDrawParam
											 ,index);
				break;
			}
		}
	}
	return view;
}
//--------------------------------------------------------
// CalcListViewRect
//--------------------------------------------------------
NFmiRect NFmiTimeValueEditManagerView::CalcListViewRect(int theIndex)
{
	NFmiRect rect;
	NFmiRect frame(GetFrame());
	double emptySpace = frame.Height()/kFmiEmptySpaceBetweenSubViews; // tila n�yt�n yl�- ja alaosassa ja alin�ytt�jen v�liss�
	double frameHeight = frame.Height();
	double timeAxisViewHeight = CalcTimeAxisHeight();

	if(itsSymbolViewCount > 0 && theIndex <= itsSymbolViewCount)
	{
		double subFrameHeight = itsToolBox->SY(kFmiSymbolViewHeigth);
		double subFrameTop = frame.Top() + (theIndex * emptySpace) + ((theIndex - 1) * subFrameHeight) + timeAxisViewHeight;
		rect = NFmiRect(frame.Left(), subFrameTop, frame.Right(), subFrameTop + subFrameHeight);
	}
	else
	{
		double subFrameHeight = (frameHeight - timeAxisViewHeight - (itsSymbolViewCount*itsToolBox->SY(kFmiSymbolViewHeigth)) - ((itsViewCount + 1) * emptySpace)) / (itsViewCount-itsSymbolViewCount);
		double subFrameTop = frame.Top() + (theIndex * emptySpace) + ((theIndex - itsSymbolViewCount - 1) * subFrameHeight) + timeAxisViewHeight + (itsSymbolViewCount*itsToolBox->SY(kFmiSymbolViewHeigth));
		rect = NFmiRect(frame.Left(), subFrameTop, frame.Right(), subFrameTop + subFrameHeight);
	}
	return rect;
}

double NFmiTimeValueEditManagerView::CalcTimeAxisHeight(void)
{
	double height = itsToolBox->SY(52);
	return height;
}

// koolla ei vaikutusta, timescale view on aina saman kokoinen korkeussuunnassa!!!!!
NFmiRect NFmiTimeValueEditManagerView::CalcUpperTimeAxisRect(void)
{ // metodi otettu nFmiTimeView:st�, rectin leveys on t�rke�, ja sen pit�� olla sama kuin kaikilla aikasarjaikkunoilla
  // yhten�isyys t�m�n luokan (manager) ja aikasarjaikkunoiden v�lille pit�isi saada selvemm�ksi!!!!
	NFmiRect axisRect(GetFrame());
	double height = CalcTimeAxisHeight();
	axisRect.Left(axisRect.Left() + axisRect.Width()/32.); // t�h�n pit�isi saada hard pixel siirto
	axisRect.Right(axisRect.Right() - axisRect.Width()/35.); // t�h�n pit�isi saada hard pixel siirto
	axisRect.Bottom(axisRect.Top() + height);
	return axisRect;
}

void NFmiTimeValueEditManagerView::CreateTimeAxisView(const NFmiTimeBag& theTimeBag)
{
	NFmiTimeControlView::MouseStatusInfo mouseStatus;
	if(itsUpperTimeView)
		mouseStatus = itsUpperTimeView->GetMouseStatusInfo();
	delete itsUpperTimeView;
	itsUpperTimeView = new NFmiTimeSerialTimeControlView(itsMapViewDescTopIndex, CalcUpperTimeAxisRect()
												,itsToolBox
												,itsDrawingEnvironment
												, itsCtrlViewDocumentInterface->DefaultEditedDrawParam()
												,false
												,false
												,false
												,true);
	itsUpperTimeView->Initialize(true, true); // t�m� on konstruktori virtuaali funktio ongelma fixi yritys

	itsUpperTimeView->SetMouseStatusInfo(mouseStatus);
}

void NFmiTimeValueEditManagerView::DrawTimeAxisView(void)
{
	if(itsUpperTimeView)
		itsUpperTimeView->Draw(itsToolBox);
}

void NFmiTimeValueEditManagerView::ResetAllModifyFactorValues(void)
{
	for(itsViewList->Reset(); itsViewList->Next(); )
	{
		((NFmiTimeSerialView*)itsViewList->Current())->ResetModifyFactorValues();
	}
}

NFmiCtrlView* NFmiTimeValueEditManagerView::ActiveView(void)
{
	for(itsViewList->Reset(); itsViewList->Next(); )
	{
		if(((NFmiTimeSerialView*)itsViewList->Current())->IsActivated())
			return itsViewList->Current();
	}
	return 0;
}

bool NFmiTimeValueEditManagerView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	bool status = itsViewList->MouseWheel(thePlace, theKey, theDelta);
	if(!status)
	{
		if(itsUpperTimeView)
		{
			status = itsUpperTimeView->MouseWheel(thePlace, theKey, theDelta);
			if(status)
				fTimeScaleUpdated = true;
		}
	}
	if(status)
        itsCtrlViewDocumentInterface->TimeSerialViewDirty(true);
	return status;
}

std::string NFmiTimeValueEditManagerView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	if(itsUpperTimeView && itsUpperTimeView->IsIn(theRelativePoint))
		return itsUpperTimeView->ComposeToolTipText(theRelativePoint);
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next();)
		{
			if(itsViewList->Current()->IsIn(theRelativePoint))
				return ((NFmiTimeSerialView*)itsViewList->Current())->ComposeToolTipText(theRelativePoint);
		}
	}
	return std::string();
}

bool NFmiTimeValueEditManagerView::AutoAdjustValueScales(bool fJustActive)
{
	bool status = false;
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next();)
		{
			if(fJustActive == false || (fJustActive && ((NFmiTimeSerialView*)itsViewList->Current())->IsActivated()))
			{
				status |= ((NFmiTimeSerialView*)itsViewList->Current())->AutoAdjustValueScale();
				if(fJustActive)
					break;
			}
		}
	}
	return status;
}

std::string NFmiTimeValueEditManagerView::MakeCsvDataString()
{
	std::string csvDataString;
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next();)
		{
			csvDataString += itsViewList->Current()->MakeCsvDataString();
		}
	}
	return csvDataString;
}

