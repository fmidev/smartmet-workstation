#include "NFmiViewParamsView.h"
#include "NFmiToolBox.h"
#include "NFmiDrawParamList.h"
#include "NFmiDrawParam.h"
#include "NFmiText.h"
#include "NFmiLine.h"
#include "NFmiRectangle.h"
#include "NFmiMenuItem.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiFastQueryInfo.h"
#include "CtrlViewFunctions.h"
#include "NFmiDictionaryFunction.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiQueryDataUtil.h"
#include "SpecialDesctopIndex.h"
#include "GraphicalInfo.h"
#include "MapHandlerInterface.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "catlog/catlog.h"
#include "ToolBoxStateRestorer.h"
#include "NFmiMacroParamDataCache.h"

#include <gdiplus.h>
#include "boost\math\special_functions\round.hpp"

NFmiViewParamsView::ModelSelectorButtonImageHolder NFmiViewParamsView::statModelSelectorButtonImages;

// initialisoinnissa luetaan bitmapit tiedostoista, kutsu vasta kun itsBitmapFolder-dataosa on asetettu
// HUOM! heitt‰‰ poikkeuksia ep‰onnistuessaan
void NFmiViewParamsView::ModelSelectorButtonImageHolder::Initialize(void)
{
	fInitialized = true;
	itsNextModelButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "control_small_play.png");
	itsPreviousModelButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "control_small_reverse_play.png");
	itsFindNearestModelButtonImage = CtrlView::CreateBitmapFromFile(itsBitmapFolder, "control_small_pause.png"); // pause saa olla etsi l‰hin sopiva malli ajo-nappula
}

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiViewParamsView::NFmiViewParamsView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, NFmiDrawingEnvironment * theDrawingEnvi, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex)
:NFmiParamCommandView(theMapViewDescTopIndex, theRect, theToolBox, theDrawingEnvi, theDrawParam, theRowIndex, theColumnIndex)
,itsButtonSizeInMM_x(3)
,itsButtonSizeInMM_y(3)
,itsButtonOffSetFromEdgeFactor(0.05)
,fMouseCaptured(false)
,itsCapturedParamRowIndex(-1)
,itsCurrentDragRowIndex(-1)
,fMouseDraggingAction(false)
{
	if(statModelSelectorButtonImages.fInitialized == false)
	{
		try
		{
			statModelSelectorButtonImages.itsBitmapFolder = itsCtrlViewDocumentInterface->HelpDataPath() + "\\res";
			statModelSelectorButtonImages.Initialize();
		}
		catch(std::exception &e)
		{
			std::string errStr("Error in NFmiViewParamsView::Initialize, while trying to read animation button bitmaps from files: \n");
			errStr += e.what();
			itsCtrlViewDocumentInterface->LogAndWarnUser(errStr, "Error while trying to read animation button bitmaps", CatLog::Severity::Error, CatLog::Category::Visualization, false);
		}
	}
}

NFmiRect NFmiViewParamsView::CalcParamRect(int theParamLineIndex, bool fExcludeCheckBox, int leftMargin, int topMargin, int rightMargin, int bottomMargin)
{
    NFmiPoint topLeftPoint = LineTextPlace(theParamLineIndex, fExcludeCheckBox);
    double left = topLeftPoint.X() + leftMargin * itsPixelSize.X();
    double top = topLeftPoint.Y() + topMargin * itsPixelSize.Y();
    double bottom = topLeftPoint.Y() + itsLineHeight - bottomMargin * itsPixelSize.Y();
    double right = GetFrame().Right() - rightMargin * itsPixelSize.X();
    return NFmiRect(left, top, right, bottom);
}

void NFmiViewParamsView::DrawParamCheckBox(int lineIndex, boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
	static NFmiDrawingEnvironment envi;
	static bool enviInitialized = false;
	if(enviInitialized == false)
	{
		enviInitialized = true;
		envi.SetFrameColor(NFmiColor(0,0,0));
		envi.SetFillColor(NFmiColor(1,1,1));
		envi.EnableFill();
	}
	NFmiRect rect(CheckBoxRect(lineIndex, true));
	DrawCheckBox(rect, envi, !theDrawParam->IsParamHidden());
}

// Parametrin aktivaation merkiksi piirret‰‰n sen pohja vaaleammalla s‰vyll‰
void NFmiViewParamsView::DrawActiveParamMarkers(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theParamLineIndex)
{
    if(theDrawParam->IsActive())
    {
        NFmiRect paramRect = CalcParamRect(theParamLineIndex, true, 0, 1, 1, -1);

        NFmiDrawingEnvironment drawingEnvi;
        float baseGrayIntensity = 217.f;
        drawingEnvi.SetFillColor(NFmiColor(baseGrayIntensity / 255.f, baseGrayIntensity / 255.f, baseGrayIntensity / 255.f));
        drawingEnvi.EnableFill();
        drawingEnvi.DisableFrame();
        DrawFrame(&drawingEnvi, paramRect);
    }
}

void NFmiViewParamsView::DrawData(void)
{
	try
	{
		InitializeGdiplus(itsToolBox, &GetFrame());

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &GetFrame());
        DrawMouseDraggingBackground();
		bool crossSectionView = itsMapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView;
		NFmiDrawParamList* paramList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));
		if(paramList)
		{
			itsDrawingEnvironment->SetFontSize(itsFontSize);
			itsDrawingEnvironment->SetFontType(kArial);
			int counter = 1;
			for(paramList->Reset(); paramList->Next(); counter++)
			{
				boost::shared_ptr<NFmiDrawParam> drawParam = paramList->Current();
				if(drawParam)
				{
                    DrawActiveParamMarkers(drawParam, counter);
					itsDrawingEnvironment->SetFrameColor(CtrlViewUtils::GetParamTextColor(drawParam, itsCtrlViewDocumentInterface));

					NFmiString paramNameStr(CtrlViewUtils::GetParamNameString(drawParam, itsCtrlViewDocumentInterface, ::GetDictionaryString("MapViewToolTipOrigTimeNormal"), ::GetDictionaryString("MapViewToolTipOrigTimeMinute"), crossSectionView, false, false));
					NFmiText text(LineTextPlace(counter, true), paramNameStr, 0, itsDrawingEnvironment);
					itsToolBox->Convert(&text);
					DrawParamCheckBox(counter, drawParam);
					DrawModelSelectorButtons(drawParam, counter);
				}
			}
            DrawMouseDraggingAction();
		}
	}
	catch(...)
	{
	}
	CleanGdiplus(); // t‰t‰ pit‰‰ kutsua piirron lopuksi InitializeGdiplus -metodin vastin parina.
}

// Pit‰‰ piirt‰‰ paramboxin tausta uusiksi, jos ollaan hiirell‰ raahaamassa parametria,
// koska t‰llˆin pit‰‰ piirt‰‰ erilaisia markereit‰ n‰ytˆlle ja 
// normaalisti taustan piirt‰‰ NFmiParamHandlerView -luokka, jonka sis‰ll‰ t‰m‰ NFmiViewParamsView -luokan ilmentym‰ on.
void NFmiViewParamsView::DrawMouseDraggingBackground()
{
    if(fMouseDraggingAction)
    {
        NFmiDrawingEnvironment drawEnvi;
        drawEnvi.SetFillColor(NFmiCtrlView::gGreyColorDefault);
        drawEnvi.DisableFrame();
        drawEnvi.EnableFill();
        DrawFrame(&drawEnvi);
    }
}

void NFmiViewParamsView::DrawMouseDraggingAction()
{
    if(fMouseDraggingAction)
    {
        // 1. Piirret‰‰n laatikko raahattavan parametrin ymp‰rille
        NFmiRect draggedParamRect = CalcParamRect(itsCapturedParamRowIndex, true, 0, 1, 0, -1);
        NFmiDrawingEnvironment drawingEnvi;
        float baseGrayIntensity = 255.f;
        drawingEnvi.SetFrameColor(NFmiColor(baseGrayIntensity / 255.f, baseGrayIntensity / 255.f, baseGrayIntensity / 255.f)); // from-drop-positio piirret‰‰n vaalealla
        DrawFrame(&drawingEnvi, draggedParamRect);

        // 2. Piirret‰‰n viiva siihen v‰liin mihin parametri nyt tiputettaisiin, 
        // jos indeksi ei ole 0 (= ei kunnollista indeksi‰)
        // ja parametria ei olla tiputtamassa itsens‰ paikalle
        if(itsCurrentDragRowIndex && itsCurrentDragRowIndex != itsCapturedParamRowIndex)
        {
            bool useTopLine = itsCurrentDragRowIndex < itsCapturedParamRowIndex; // jos tiputus on l‰htˆpaikkaa ylemp‰n‰, k‰ytet‰‰n paramRectin yl‰viivaa
            NFmiRect dropZoneParamRect = CalcParamRect(itsCurrentDragRowIndex, true, 3, 0, 16, -2);
            if(useTopLine)
            {
                double newBottom = dropZoneParamRect.Top() + itsToolBox->SY(2);
                dropZoneParamRect.Bottom(newBottom); // tehd‰‰n laatikosta yhden pikselin korkuinen alkaen sen alkuper‰isest‰ yl‰osasta
            }
            else
            {
                double newTop = dropZoneParamRect.Bottom() - itsToolBox->SY(2);
                dropZoneParamRect.Top(newTop); // tehd‰‰n laatikosta yhden pikselin korkuinen alkaen sen alkuper‰isest‰ alaosasta
            }
            drawingEnvi.SetFrameColor(NFmiColor(0, 0, 0)); // to-drop-positio piirret‰‰n mustalla
            DrawFrame(&drawingEnvi, dropZoneParamRect);
        }
    }
}

NFmiPoint NFmiViewParamsView::GetViewSizeInPixels(void)
{
	if(itsMapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
		return itsCtrlViewDocumentInterface->CrossSectionViewSizeInPixels();
	else
		return itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex);
}

void NFmiViewParamsView::DrawModelSelectorButtons(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex)
{
	if(theDrawParam->IsModelRunDataType()) // piirret‰‰n tuottajan vaihto napit vain niille datoille, joille lˆytyy arkisto dataa
	{ // toistaiseksi vain pinta/painepinta/mallipinta datat arkistoissa
		itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, GetFrame()));
		// piirr‰ next-model-button
        CtrlView::DrawAnimationButton(CalcModelSelectorButtonRect(theRowIndex, 2), statModelSelectorButtonImages.itsPreviousModelButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f);
        CtrlView::DrawAnimationButton(CalcModelSelectorButtonRect(theRowIndex, 1), statModelSelectorButtonImages.itsFindNearestModelButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f);
        CtrlView::DrawAnimationButton(CalcModelSelectorButtonRect(theRowIndex, 0), statModelSelectorButtonImages.itsNextModelButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f);
	}
}

// Laskee vasempaan alariviin nappuloiden paikkoja indeksin perusteella.
// theIndex alkaa 1:st‰ ja indeksin kasvaessa nappuloita sijoitetaan aina enemm‰n oikealle.
NFmiRect NFmiViewParamsView::CalcModelSelectorButtonRect(int theRowIndex, int theButtonIndex)
{
	NFmiPoint rowPlace = LineTextPlace(theRowIndex, true);
	NFmiPoint buttonRelativeSize = CalcModelSelectorButtonRelativeSize(statModelSelectorButtonImages.itsNextModelButtonImage);
	NFmiPoint buttonRelativeEdgeOffset = CalcModelSelectorButtonRelativeEdgeOffset(buttonRelativeSize);

	// Sijoitetaan n‰m‰ nappulat parametri rivilleen hieman irti reunoista.
	// nappulat sijoitetaan oikeasta alareunasta kohti vasenta reunaa...
	double rightside = GetFrame().Right() - buttonRelativeEdgeOffset.X() - (theButtonIndex * buttonRelativeSize.X());
	double leftSide = rightside - buttonRelativeSize.X();
	double topSide = rowPlace.Y() + itsLineHeight - buttonRelativeSize.Y() - buttonRelativeEdgeOffset.Y();
	double bottomSide = topSide + buttonRelativeSize.Y();

	NFmiRect buttonRect(leftSide, topSide, rightside, bottomSide);
	return buttonRect;
}

NFmiPoint NFmiViewParamsView::CalcModelSelectorButtonRelativeEdgeOffset(const NFmiPoint &theButtonRelaviteSize)
{
	double offsetFromEdgeX = theButtonRelaviteSize.X() * itsButtonOffSetFromEdgeFactor;
	double offsetFromEdgeY = theButtonRelaviteSize.Y() * itsButtonOffSetFromEdgeFactor;
	return NFmiPoint(offsetFromEdgeX, offsetFromEdgeY);
}

NFmiPoint NFmiViewParamsView::CalcModelSelectorButtonRelativeSize(Gdiplus::Bitmap *theImage)
{
	double relativeWidth = itsToolBox->SX(boost::math::iround(itsButtonSizeInMM_x * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x)); 
    double relativeHeight = itsToolBox->SY(boost::math::iround(itsButtonSizeInMM_y * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y));
	if(itsCtrlViewDocumentInterface->Printing() == false)
	{
		long bitmapSizeX = 16;
		long bitmapSizeY = 16;
		if(theImage)
		{
			bitmapSizeX = theImage->GetWidth();
			bitmapSizeY = theImage->GetHeight();
		}
		relativeWidth = itsToolBox->SX(bitmapSizeX);
		relativeHeight = itsToolBox->SY(bitmapSizeY);
	}
	return NFmiPoint(relativeWidth, relativeHeight);
}


//--------------------------------------------------------
// RightButtonDown
//--------------------------------------------------------
bool NFmiViewParamsView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		int index = CalcIndex(thePlace);
		return itsCtrlViewDocumentInterface->CreateViewParamsPopup(itsMapViewDescTopIndex, GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber), index) == true;
	}
	return false;
}

//--------------------------------------------------------
// CalcSize
//--------------------------------------------------------
NFmiRect NFmiViewParamsView::CalcSize(void)
{
	NFmiRect returnRect(GetFrame());
	int lineCount = 1; // minimi
    NFmiDrawParamList* drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));
    if(drawParamList && drawParamList->NumberOfItems())
        lineCount = drawParamList->NumberOfItems();

// ruudun korkeus on rivien m‰‰r‰*rivinkorkeus + viidesosa rivin korkeudesta (v‰h‰n tilaa pohjalle)
	double heigth = lineCount * itsLineHeight + 0.5 * itsLineHeight;
	returnRect.Bottom(returnRect.Top() + heigth);
	return returnRect;
}

// Marko lis‰si 3.4.2002
bool NFmiViewParamsView::LeftDoubleClick(const NFmiPoint &thePlace, unsigned long /* theKey */)
{
	if(IsIn(thePlace))
	{
		NFmiDrawParamList* paramList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));
		if(paramList)
		{
			int index = CalcIndex(thePlace);
			if(paramList->Index(index))
			{
				boost::shared_ptr<NFmiDrawParam> drawParam = paramList->Current();
				if(drawParam)
				{
					if(drawParam->IsActive())
					{
						if(itsMapViewDescTopIndex == 0) // en saa t‰t‰ toimimaan oikein kuin p‰‰karttan‰ytˆll‰ (joten est‰n drawParam muokkauksen muissa ikkunoissa), muissa parametri menee piiloon, enk‰ saa ikkunoita p‰ivitt‰m‰‰n ruutua oikeaan tilaan
						{
							NFmiDataIdent param = drawParam->Param();
                            bool macroParamInCase = (NFmiDrawParam::IsMacroParamCase(drawParam->DataType()));
							if(macroParamInCase) // macroParamin yhteydess‰ parametrin nimeksi pit‰‰ laittaa sen lyhenne, koska se on macroParamin tunniste!!
								param.GetParam()->SetName(drawParam->InitFileName());

							NFmiMenuItem menuItem(itsMapViewDescTopIndex, "xxx", param, kFmiModifyDrawParam, 
								NFmiMetEditorTypes::kFmiParamsDefaultView, &drawParam->Level(),
								drawParam->DataType(), index, drawParam->ViewMacroDrawParam());
							drawParam->HideParam(!drawParam->IsParamHidden()); // t‰m‰ on ruma fixi, mutta tupla klikki tekee t‰m‰n piilotus asetuksen jostain syyst‰ ja minun pit‰‰ laittaa se t‰ss‰ pois
							itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false); // ik‰v‰ kyll‰ yksˆis klikin j‰lkeen (ennen t‰t‰ toista klikki‰, josta syntyy tupla klikkaus) 
																				// on tehty ruudun p‰ivitys, joka nyt tuplaklikin kohdalla pit‰‰ kumota
							itsCtrlViewDocumentInterface->RefreshApplicationViewsAndDialogs("ViewParamsView::LeftDoubleClick: Double click has been pressed over parameter to open Draw-param dialog, this update fixes (UGLY) the first left-click's param show/hide action");
							return itsCtrlViewDocumentInterface->ExecuteCommand(menuItem, GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber), 1); // 1=viewtype ei m‰‰r‰tty viel‰
						}
					}
					else
					{
                        return ActivateParam(drawParam, index);
					}
				}
			}
		}
	}
	return false;
}

bool NFmiViewParamsView::ActivateParam(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theParamIndex)
{
    NFmiMenuItem menuItem(itsMapViewDescTopIndex, "xxx", theDrawParam->Param(), kFmiActivateView,
        NFmiMetEditorTypes::kFmiParamsDefaultView, &theDrawParam->Level(),
        theDrawParam->DataType(), theParamIndex, theDrawParam->ViewMacroDrawParam());
    return itsCtrlViewDocumentInterface->ExecuteCommand(menuItem, GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber), 1); // 1=viewtype ei m‰‰r‰tty viel‰
}

// CalcIndex-metodi ei mielest‰ni oikein toimi kunnolla, siksi virittelin t‰h‰n
// uuden rivin lasku funktion.
static int CalcBetterIndex(const NFmiPoint &thePlace, const NFmiRect &theFrame, int theParamCount)
{
	if(theParamCount == 0)
		theParamCount = 1;
	double lineHeight = theFrame.Height()/theParamCount;

	double linePos = (thePlace.Y() - theFrame.Top()) / lineHeight;
	return static_cast<int>(linePos)+1; // t‰m‰ sama kuin ceil-funktio eli 'katto'
}

int NFmiViewParamsView::GetParamCount(void)
{
	NFmiDrawParamList* paramList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));
	if(paramList)
	{
		return static_cast<int>(paramList->NumberOfItems());
	}
	else
		return 0;
}

bool NFmiViewParamsView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(IsIn(thePlace))
	{
		int realRowIndex = CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber); // itsDoc->MapViewDescTop(itsMapViewDescTopIndex)->MapRowStartingIndex() + itsRowIndex - 1;
		bool useCrossSectionParams = itsMapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView;
        int paramIndex = ::CalcBetterIndex(thePlace, GetFrame(), GetParamCount());
        if(theKey & kShiftKey)
        {
            return itsCtrlViewDocumentInterface->ChangeParamSettingsToNextFixedDrawParam(itsMapViewDescTopIndex, realRowIndex, paramIndex, theDelta > 0 ? true : false, useCrossSectionParams);
        }
        else if(theKey & kCtrlKey)
		{
			return itsCtrlViewDocumentInterface->ChangeActiveMapViewParam(itsMapViewDescTopIndex, realRowIndex, paramIndex, theDelta > 0 ? true : false, useCrossSectionParams);
		}
		else
			return itsCtrlViewDocumentInterface->MoveActiveMapViewParamInDrawingOrderList(itsMapViewDescTopIndex, realRowIndex, theDelta > 0 ? true : false, useCrossSectionParams);
	}
	return false;
}

bool NFmiViewParamsView::DoAfterParamModeModifications(NFmiDrawParamList *theParamList)
{
    theParamList->Dirty(true);
    itsCtrlViewDocumentInterface->CheckAnimationLockedModeTimeBags(itsMapViewDescTopIndex, false); // kun parametrin n‰kyvyytt‰ vaihdetaan, pit‰‰ tehd‰ mahdollisesti animaatio moodin datan tarkistus
    auto rowIndex = GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
    itsCtrlViewDocumentInterface->MacroParamDataCache().update(itsMapViewDescTopIndex, static_cast<unsigned long>(rowIndex), *theParamList);
    itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, true);
    return true;
}

bool NFmiViewParamsView::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
    fMouseCaptured = false;
    itsCurrentDragRowIndex = -1;
    fMouseDraggingAction = false;
    if(IsIn(thePlace))
    {
        NFmiDrawParamList* paramList = GetDrawParamList();
        if(paramList && paramList->NumberOfItems() >= 2)
        { // pit‰‰ olla v‰h. kaksi parametria listassa ennen kuin kannattaa mietti‰ raahauksia
            itsCapturedParamRowIndex = CalcIndex(thePlace);
            if(itsCapturedParamRowIndex)
            {
                fMouseCaptured = true;
                itsCtrlViewDocumentInterface->MouseCaptured(true);
                return true;
            }
        }
    }
    return false;
}

bool NFmiViewParamsView::MouseMove(const NFmiPoint& thePlace, unsigned long theKey)
{
    // HUOM!! ei pid‰ tutkia onko hiiri ikkunan sis‰ll‰ vaan onko hiiri kaapattu!
    if(!IsIn(thePlace))
        int x = 0;

    if(itsCtrlViewDocumentInterface->MouseCaptured() && fMouseCaptured)
    { // t‰m‰ blokki liittyy parametrin raahaukseen (piirtoj‰rjestyksen muuttoa varten)
        itsCurrentDragRowIndex = CalcIndex(thePlace); // 0 indeksi on puuttuva eli ollaan parambox laatikon yl‰/ala puolella
        if(itsCurrentDragRowIndex && itsCurrentDragRowIndex != itsCapturedParamRowIndex)
        { // Nyt hiirt‰ on raahattu originaali parametrin kohdalta jonkun toisen parametrin p‰‰lle
            fMouseDraggingAction = true; // T‰st‰ eteenp‰in raahaus on piirrett‰v‰ tavalla tai toisella, kunnes hiirest‰ p‰‰stet‰‰n irti
        }
    }

    if(fMouseDraggingAction)
        Draw(itsToolBox);

    return false;
}

bool NFmiViewParamsView::IsMouseDraggingOn(void)
{
    return fMouseDraggingAction;
}

NFmiDrawParamList* NFmiViewParamsView::GetDrawParamList()
{
    return itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber));
}

bool NFmiViewParamsView::LeftButtonUp(const NFmiPoint &thePlace, unsigned long theKey)
{
    // HUOM!! ei pid‰ tutkia onko hiiri ikkunan sis‰ll‰ vaan ollaanko raahaus moodissa!
    if(fMouseDraggingAction)
    { // t‰m‰ blokki liittyy parametrin raahaukseen (piirtoj‰rjestyksen muuttoa varten)
        fMouseDraggingAction = false;
        NFmiDrawParamList* paramList = GetDrawParamList();
        if(paramList && paramList->MoveParam(itsCapturedParamRowIndex, itsCurrentDragRowIndex))
            return DoAfterParamModeModifications(paramList);
    }
    else if(IsIn(thePlace))
	{
        NFmiDrawParamList* paramList = GetDrawParamList();
		if(paramList)
		{
			int index = CalcIndex(thePlace);
			if(paramList->Index(index))
			{
				boost::shared_ptr<NFmiDrawParam> drawParam = paramList->Current();
				if(drawParam)
				{
					if(theKey & kCtrlKey)
					{ // jos ctrl pohjassa aktivoidaan kyseinen parametri
                        return ActivateParam(drawParam, index);
					}
					else if(theKey & kShiftKey)
					{ // jos shift pohjassa poistetaan parametri n‰ytˆlt‰
						paramList->Remove();
                        return DoAfterParamModeModifications(paramList);
                    }
					else
					{
						if(LeftClickOnModelSelectionButtons(thePlace, drawParam, index))
							return true;
						else
						{ // toglataan parametrin n‰yt‰/piilota moodia
							drawParam->HideParam(!drawParam->IsParamHidden());
                            return DoAfterParamModeModifications(paramList);
                        }
					}
				}
			}
		}
	}
	return false;
}

// HUOM! theRowIndex on parametrin rivi numero t‰ll‰ karttan‰yttˆ rivill‰. Sill‰ lasketaan painonappien paikat.
// Dokumentin funktioille pit‰‰ antaa taas itsRowNumber, joka on taas t‰m‰n n‰ytˆn sijainti riveiss‰ karttan‰yttˆ ruudukossa,
// t‰t‰ tietoa tarvitaan kun pit‰‰ puhdistaa muutoksen takia karttarivi cachesta.
bool NFmiViewParamsView::LeftClickOnModelSelectionButtons(const NFmiPoint &thePlace, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex)
{
    auto usedRowIndex = GetUsedParamRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
    if(CalcModelSelectorButtonRect(theRowIndex, 2).IsInside(thePlace)) // t‰ss‰ tutkitaan osuiko hiiren klikkaus previous-nappiin (2)
	{
		itsCtrlViewDocumentInterface->SetModelRunOffset(theDrawParam, -1, itsMapViewDescTopIndex, usedRowIndex);
		return true;
	}
	else if(CalcModelSelectorButtonRect(theRowIndex, 0).IsInside(thePlace)) // t‰ss‰ tutkitaan osuiko hiiren klikkaus next-nappiin (0)
	{
		itsCtrlViewDocumentInterface->SetModelRunOffset(theDrawParam, 1, itsMapViewDescTopIndex, usedRowIndex);
		return true;
	}
	else if(CalcModelSelectorButtonRect(theRowIndex, 1).IsInside(thePlace)) // t‰ss‰ tutkitaan osuiko hiiren klikkaus nearest-model-nappiin (1)
	{
		itsCtrlViewDocumentInterface->SetNearestBeforeModelOrigTimeRunoff(theDrawParam, itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex), itsMapViewDescTopIndex, usedRowIndex);
		return true;
	}
	return false;
}
