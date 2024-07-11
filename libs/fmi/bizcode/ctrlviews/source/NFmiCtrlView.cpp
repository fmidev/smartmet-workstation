//© Ilmatieteenlaitos/Marko
//  Original 23.09.1998
// 
//                                  
//Ver. xx.xx.xxxx/Marko
//
//-------------------------------------------------------------------- nctrview.cpp

#include "NFmiCtrlView.h"
#include "NFmiDrawParam.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiRectangle.h"
#include "NFmiToolBox.h"
#include "NFmiFastQueryInfo.h"
#include "SpecialDesctopIndex.h"
#include "MapHandlerInterface.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewFunctions.h"
#include "ColorStringFunctions.h"

#include <gdiplus.h>

const float NFmiCtrlView::gGreyColorBaseComponent = 192.f;
const NFmiColor NFmiCtrlView::gGreyColorDefault = NFmiColor(NFmiCtrlView::gGreyColorBaseComponent / 255.f, NFmiCtrlView::gGreyColorBaseComponent / 255.f, NFmiCtrlView::gGreyColorBaseComponent / 255.f);

//--------------------------------------------------------
// NFmiCtrlView constructor
//--------------------------------------------------------

NFmiCtrlView::NFmiCtrlView(void)
:itsRect()
,itsToolBox(0)
,itsDrawingEnvironment(0)
,itsDrawParam()
,itsInfo()
,itsCtrlViewDocumentInterface(nullptr)
,itsMapViewDescTopIndex(-1)
,itsViewGridRowNumber(-1)
,itsViewGridColumnNumber(-1)
,itsTime() // aika pit‰‰ asettaa erikseen Time()-metodilla, jos haluaa siihen jotain j‰rkev‰mp‰‰ kuin nykyhetken
,itsGdiPlusGraphics(0)
{ // itsDataParam ei voi toimia t‰m‰n j‰lkeen!!!!!
    SetupCtrlViewDocumentInterface(__FUNCTION__);
}

NFmiCtrlView::NFmiCtrlView (int theMapViewDescTopIndex, const NFmiRect & theRect
						   ,NFmiToolBox * theToolBox
						   ,NFmiDrawingEnvironment * theDrawingEnvi
                           , int viewGridRowNumber
                           , int viewGridColumnNumber)
:itsRect(theRect)
,itsToolBox(theToolBox)
,itsDrawingEnvironment(theDrawingEnvi)
,itsDrawParam()
,itsInfo()
,itsCtrlViewDocumentInterface(nullptr)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,itsViewGridRowNumber(viewGridRowNumber)
,itsViewGridColumnNumber(viewGridColumnNumber)
,itsTime() // aika pit‰‰ asettaa erikseen Time()-metodilla, jos haluaa siihen jotain j‰rkev‰mp‰‰ kuin nykyhetken
,itsGdiPlusGraphics(0)
{
    SetupCtrlViewDocumentInterface(__FUNCTION__);
}

NFmiCtrlView::NFmiCtrlView (int theMapViewDescTopIndex, const NFmiRect & theRect
						   ,NFmiToolBox * theToolBox
						   ,NFmiDrawingEnvironment * theDrawingEnvi
						   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
                           ,int viewGridRowNumber
                           ,int viewGridColumnNumber)
:itsRect(theRect)
,itsToolBox(theToolBox)
,itsDrawingEnvironment(theDrawingEnvi)
,itsDrawParam(theDrawParam)
,itsInfo()
,itsCtrlViewDocumentInterface(nullptr)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,itsViewGridRowNumber(viewGridRowNumber)
,itsViewGridColumnNumber(viewGridColumnNumber)
,itsTime() // aika pit‰‰ asettaa erikseen Time()-metodilla, jos haluaa siihen jotain j‰rkev‰mp‰‰ kuin nykyhetken
,itsGdiPlusGraphics(0)
{
    SetupCtrlViewDocumentInterface(__FUNCTION__);
}

NFmiCtrlView::NFmiCtrlView(const NFmiCtrlView& theView)
:itsRect(theView.itsRect)
,itsToolBox(theView.itsToolBox)
,itsDrawingEnvironment(theView.itsDrawingEnvironment)
,itsDrawParam(theView.itsDrawParam)
,itsInfo(theView.itsInfo)
,itsCtrlViewDocumentInterface(theView.itsCtrlViewDocumentInterface)
,itsMapViewDescTopIndex(theView.itsMapViewDescTopIndex)
,itsViewGridRowNumber(theView.itsViewGridRowNumber)
,itsViewGridColumnNumber(theView.itsViewGridColumnNumber)
,itsTime() // aika pit‰‰ asettaa erikseen Time()-metodilla, jos haluaa siihen jotain j‰rkev‰mp‰‰ kuin nykyhetken
,itsGdiPlusGraphics(0)
{
}

CtrlViewDocumentInterface* NFmiCtrlView::GetCtrlViewDocumentInterface()
{
    return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation();
}

void NFmiCtrlView::SetupCtrlViewDocumentInterface(const std::string &callerFunctionName)
{
    if(CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation)
        itsCtrlViewDocumentInterface = CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation();
    else
        throw std::runtime_error(callerFunctionName + " - CtrlViewDocumentInterface was not set");
}

//--------------------------------------------------------
// Update
//--------------------------------------------------------
void NFmiCtrlView::Update(const NFmiRect &theRect, NFmiToolBox *theToolBox)
{
	itsRect=theRect;
	itsToolBox = theToolBox;
}
//--------------------------------------------------------
// operator==
//--------------------------------------------------------
bool NFmiCtrlView::operator==(const NFmiCtrlView& theCtrlView) const
{
	return (*itsDrawParam == *theCtrlView.itsDrawParam) == true;
}

//--------------------------------------------------------
// operator <
//--------------------------------------------------------
bool NFmiCtrlView::operator< (const NFmiCtrlView& theCtrlView) const
{
	return (*itsDrawParam < *theCtrlView.itsDrawParam) == true;
}

void NFmiCtrlView::DrawFrame(NFmiDrawingEnvironment * theEnvi)
{
	NFmiRectangle rect(itsRect.TopLeft()
					  ,itsRect.BottomRight()
					  ,0
					  ,theEnvi);
	itsToolBox->Convert(&rect);
}

void NFmiCtrlView::DrawFrame(NFmiDrawingEnvironment * theEnvi, const NFmiRect& theFrame)
{
	NFmiRectangle rect(theFrame.TopLeft()
					  ,theFrame.BottomRight()
					  ,0
					  ,theEnvi);
	itsToolBox->Convert(&rect);
}

int NFmiCtrlView::GetUsedParamRowIndex(int theRowIndex, int theColumnIndex) const
{
	if(itsMapViewDescTopIndex > CtrlViewUtils::kFmiMaxMapDescTopIndex)
		return theRowIndex; // t‰m‰ on muut kuin karttan‰ytˆt

    CtrlViewUtils::MapViewMode mapViewMode = itsCtrlViewDocumentInterface->MapViewDisplayMode(itsMapViewDescTopIndex);

	if(mapViewMode == CtrlViewUtils::MapViewMode::kOneTime)
	{ // jokainen n‰yttˆruutu on samaa aikaa, jokaisella ruudulla on omat parametrit. Ruudukon juoksutus menee 
		// vasemmalta oikealle ja ylh‰‰lt‰ alas. Esim. N‰yttˆ on 4x3 ruudukossa. 
		// theRowIndex = 2 eli 2. rivi (indeksit alkavat 1:st‰).
		// theColumnIndex on 3 eli 3. sarake (indeksit 1:st‰)
		// ((2-1) * 4) + 3 = 7    (4 on siis ruudukon x-koko)
		int totalViewTileIndex = ((theRowIndex-1) * static_cast<int>(itsCtrlViewDocumentInterface->ViewGridSize(itsMapViewDescTopIndex).X())) + theColumnIndex;
		return totalViewTileIndex;
	}
	else if(mapViewMode == CtrlViewUtils::MapViewMode::kRunningTime)
		return 1; // juokseva aika moodissa palautetaan aina 1. param rivi, koska joka ruudussa on eri aika ja samat 1. rivin
					// parametrit (muista ett‰ kyseess‰ on virtuaali 1. rivi eli n‰yttˆ luokat el‰v‰t 1:st‰ alkavassa maailmassa)
	else
		return theRowIndex; // normaali moodissa (tai muissa tapauksissa) palautetaan normaali rivi indeksi
}

int NFmiCtrlView::GetUsedParamRowIndex() const
{
    return GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
}

bool NFmiCtrlView::IsPrintedMapViewDesctop()
{
    if(itsCtrlViewDocumentInterface)
        return itsCtrlViewDocumentInterface->GetPrintedDescTopIndex() == itsMapViewDescTopIndex;
    return false;
}

bool NFmiCtrlView::IsPrinting() const
{
    if(itsToolBox)
    {
        return itsToolBox->GetDC()->IsPrinting() == TRUE;
    }
    else
        return false;
}

// N‰ytetty rivi numero t‰‰ll‰ (theRowIndex) ei ole aina sama kuin dokumentin
// rivi, koska nyky‰‰n n‰yttˆrivej‰ voi selata myˆs pystysuunnassa.
int NFmiCtrlView::CalcRealRowIndex(int theRowIndex, int theColumnIndex) const
{
	int realRowIndex = GetUsedParamRowIndex(theRowIndex, theColumnIndex); 
	if(itsMapViewDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
		realRowIndex += itsCtrlViewDocumentInterface->MapRowStartingIndex(itsMapViewDescTopIndex) - 1;
	return realRowIndex;
}

int NFmiCtrlView::CalcRealRowIndex() const
{
	return CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
}

std::unique_ptr<MapHandlerInterface> NFmiCtrlView::GetMapHandlerInterface()
{
    return itsCtrlViewDocumentInterface->GetMapHandlerInterface(itsMapViewDescTopIndex);
}

// Luodaan GDI++ piirto olio annetun toolboxin avulla.
// Jos piirtoaluetta halutaan rajata, annetaan alue theRelativeClipRect -parametrissa.
Gdiplus::Graphics* NFmiCtrlView::CreateGdiplusGraphics(NFmiToolBox *theToolBox, const NFmiRect *theRelativeClipRect)
{
    if(theToolBox == 0)
        throw std::runtime_error(std::string("Error in ") + __FUNCTION__ + " -method, given toolbox was 0-pointer, unable to initialize Gdiplus::Graphics, application error.");

    Gdiplus::Graphics *gdiPlusGraphics = Gdiplus::Graphics::FromHDC(theToolBox->GetDC()->GetSafeHdc());
    if(gdiPlusGraphics == 0)
        throw std::runtime_error(std::string("Error in ") + __FUNCTION__ + " -method, unable to initialize Gdiplus::Graphics, application error.");

    if(theToolBox->GetDC()->IsPrinting())
        gdiPlusGraphics->SetPageUnit(Gdiplus::UnitPixel); // t‰h‰n asti on pelattu printatessa aina pikseli maailmassa, joten gdiplus:in pit‰‰ laittaa siihen
    if(theRelativeClipRect)
        gdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(theToolBox, *theRelativeClipRect));
    return gdiPlusGraphics;
}

void NFmiCtrlView::InitializeGdiplus(NFmiToolBox *theToolBox, const NFmiRect *theRelativeClipRect)
{
    itsGdiPlusGraphics = NFmiCtrlView::CreateGdiplusGraphics(theToolBox, theRelativeClipRect);
}

// T‰t‰ pit‰‰ kutsua, kun lopetetaan n‰yttˆluokan piirto, joka on alustettu InitializeGdiplus -metodi kutsulla.
void NFmiCtrlView::CleanGdiplus(void)
{
	if(itsGdiPlusGraphics)
	{
		delete itsGdiPlusGraphics;
		itsGdiPlusGraphics = nullptr;
	}
}

NFmiRect NFmiCtrlView::CalcMaskRectSize(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	theInfo->FirstLocation(); // laitetaan 1. hilapiste eli vasen alanurkka kohdalle
	NFmiPoint latlon1(theInfo->LatLon());
	NFmiPoint latlon2(theInfo->PeekLocationLatLon(1, 0));
	NFmiPoint latlon3(theInfo->PeekLocationLatLon(0, 1));
	NFmiPoint p1(LatLonToViewPoint(latlon1));
	NFmiPoint p2(LatLonToViewPoint(latlon2));
	NFmiPoint p3(LatLonToViewPoint(latlon3));
	double factor = 1.2; // pit‰‰ v‰h‰n suurentaa laatikoiden kokoa, jos hila ja karttapohja eiv‰t ole samaa projektiota, t‰llˆin voi j‰‰d‰ pieni‰ aukkoja maskiin
	double distX = ::fabs(p1.X() - p2.X()) * factor;
	// 4. Calc relative dist of two vertical neighbor grid point in y dir
	double distY = ::fabs(p1.Y() - p3.Y()) * factor;

	NFmiRect rect(0, 0, distX, distY);
	return rect;
}

// Lyhyesti: movedRect halutaan siirt‰‰ paikkaan, jonka positionalRect:in jonkun kulmapisteen sijainti sanelee.
NFmiRect NFmiCtrlView::CalcWantedDirectionalPosition(const NFmiRect& positionalRect, const NFmiRect& movedRect, FmiDirection wantedPosition)
{
	NFmiRect finalRect(movedRect);
	auto originalPlace = movedRect.Place();
	auto placeX = originalPlace.X();
	auto placeY = originalPlace.Y();
	switch(wantedPosition)
	{
	case kTopLeft:
	{
		// top-left on helppo, koska ollaan jo halutussa Place:ssa
		break;
	}
	case kTopRight:
	{
		placeX = positionalRect.Right() - finalRect.Width();
		break;
	}
	case kBottomRight:
	{
		placeX = positionalRect.Right() - finalRect.Width();
		placeY = positionalRect.Bottom() - finalRect.Height();
		break;
	}
	case kBottomLeft:
	{
		placeY = positionalRect.Bottom() - finalRect.Height();
		break;
	}
	default:
		break;
	}

	finalRect.Place(NFmiPoint(placeX, placeY));
	return finalRect;
}

const std::string& NFmiCtrlView::CachedParameterName(bool tooltipVersion) const
{ 
	if(tooltipVersion)
		return itsCachedParameterNameForTooltip;
	else
		return itsCachedParameterName;
}

void NFmiCtrlView::CachedParameterName(const std::string& newName, bool tooltipVersion)
{ 
	if(tooltipVersion)
		itsCachedParameterNameForTooltip = newName;
	else
		itsCachedParameterName = newName;
}

void NFmiCtrlView::UpdateCachedParameterName()
{
	// T‰m‰ on normi karttan‰ytˆn alustus, josta tehtiin oletustoiminto (override jutut tehd‰‰n mm. aikasarjaan ja poikkileikkaus luokkiin).
	CachedParameterName(CtrlViewUtils::GetParamNameString(itsDrawParam, false, false, false, 0, false, true, true, itsInfo), false);
	CachedParameterName(CtrlViewUtils::GetParamNameString(itsDrawParam, false, false, true, 0, false, true, true, itsInfo), true);
}

std::string NFmiCtrlView::MakePossibleVirtualTimeTooltipText()
{
	if(itsCtrlViewDocumentInterface->VirtualTimeUsed())
	{
		return itsCtrlViewDocumentInterface->GetVirtualTimeTooltipText();
	}

	return "";
}

std::string NFmiCtrlView::DoBoldingParameterNameTooltipText(std::string parameterStr)
{
	// Jos annettu parameterStr alkaa highlight merkill‰, lis‰t‰‰n alkuun ja loppuun html bold tagit
	if(IsNewDataParameterName(parameterStr))
	{
		parameterStr = "<b>" + parameterStr + "</b>";
	}
	return parameterStr;
}

bool NFmiCtrlView::IsNewDataParameterName(const std::string& parameterStr)
{
	return (!parameterStr.empty() && parameterStr.front() == CtrlViewUtils::ParameterStringHighlightCharacter);
}

std::string NFmiCtrlView::AddColorTagsToString(const std::string &str, const NFmiColor &color, bool addBoldTags)
{
	std::string coloredStr;
	if(addBoldTags)
	{
		coloredStr += "<b>";
	}
	coloredStr += "<font color=";
	coloredStr += ColorString::Color2HtmlColorStr(color);
	coloredStr += ">";
	coloredStr += str;
	coloredStr += "</font>";
	if(addBoldTags)
	{
		coloredStr += "</b>";
	}
	return coloredStr;
}

std::string NFmiCtrlView::MakeCsvDataString()
{
	return std::string();
}
