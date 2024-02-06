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
#include "CombinedMapHandlerInterface.h"
#include "NFmiFastInfoUtils.h"
#include "NFmiFileString.h"
#include "WmsSupportInterface.h"
#include "wmssupport\ChangedLayers.h"

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
NFmiViewParamsView::NFmiViewParamsView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, NFmiDrawingEnvironment * theDrawingEnvi, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex, bool hasMapLayer)
:NFmiParamCommandView(theMapViewDescTopIndex, theRect, theToolBox, theDrawingEnvi, theDrawParam, theRowIndex, theColumnIndex, hasMapLayer)
,itsButtonSizeInMM_x(3)
,itsButtonSizeInMM_y(3)
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

NFmiRect NFmiViewParamsView::CalcParameterDragRect(int theParamLineIndex, int leftMargin, int topMargin, int rightMargin, int bottomMargin)
{
	// Jos ollaan n‰ytˆss‰, jossa fHasMapLayer = false (esim. poikkileikkausn‰yttˆ),
	// Pit‰‰ indeksi‰ s‰‰t‰‰, jotta raahaus visualisoinnit menev‰t oikein.
	if(!fHasMapLayer)
		theParamLineIndex--;

	NFmiRect parameterRowRect = CalcParameterRowRect(theParamLineIndex);
    NFmiPoint topLeftPoint = parameterRowRect.TopLeft();
    double left = topLeftPoint.X() + leftMargin * itsPixelSize.X();
    double top = topLeftPoint.Y() + topMargin * itsPixelSize.Y();
    double bottom = topLeftPoint.Y() + itsLineHeight - bottomMargin * itsPixelSize.Y();
    double right = GetFrame().Right() - rightMargin * itsPixelSize.X();
    return NFmiRect(left, top, right, bottom);
}

// Parametrin aktivaation merkiksi piirret‰‰n sen pohja vaaleammalla s‰vyll‰
void NFmiViewParamsView::DrawActiveParamMarkers(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theParamLineIndex)
{
    if(theDrawParam->IsActive())
    {
        NFmiRect paramRect = CalcParameterRowRect(theParamLineIndex);
		// Koko parametri laatikko menee reunoilla reunaviivojen p‰‰lle, joten lis‰t‰‰n t‰h‰n korostuslaatikkoon marginaalia molempiin reunoihin.
		auto horizontalMargin = ConvertMilliMeterToRelative(0.5, true);
		paramRect.Inflate(-horizontalMargin, 0);

        NFmiDrawingEnvironment drawingEnvi;
        float baseGrayIntensity = 217.f;
        drawingEnvi.SetFillColor(NFmiColor(baseGrayIntensity / 255.f, baseGrayIntensity / 255.f, baseGrayIntensity / 255.f));
        drawingEnvi.EnableFill();
        drawingEnvi.DisableFrame();
        DrawFrame(&drawingEnvi, paramRect);
    }
}

void NFmiViewParamsView::DrawBackgroundMapLayerText(int& zeroBasedRowIndexInOut, const NFmiRect& parameterRowRect)
{
	if(fHasMapLayer)
	{
		itsDrawingEnvironment->SetFrameColor(CtrlViewUtils::GetParamTextColor(NFmiInfoData::kMapLayer, false));
		NFmiString mapLayerText = itsCtrlViewDocumentInterface->GetCombinedMapHandlerInterface().getCurrentMapLayerGuiText(itsMapViewDescTopIndex, true);
		// map-layer rivin indeksi on 0 ja se annetaan LineTextPlace -metodille.
		NFmiText text(LineTextPlace(zeroBasedRowIndexInOut, parameterRowRect, false), mapLayerText, false, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&text);
		zeroBasedRowIndexInOut++;
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
		NFmiDrawParamList* paramList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex());
		if(paramList)
		{
			itsDrawingEnvironment->SetFontSize(itsFontSize);
			itsDrawingEnvironment->SetFontType(kArial);
			int zeroBasedRowIndex = 0;
			NFmiRect parameterRowRect = CalcParameterRowRect(zeroBasedRowIndex);
			DrawBackgroundMapLayerText(zeroBasedRowIndex, parameterRowRect);
			for(paramList->Reset(); paramList->Next(); zeroBasedRowIndex++)
			{
				parameterRowRect = CalcParameterRowRect(zeroBasedRowIndex);
				boost::shared_ptr<NFmiDrawParam> drawParam = paramList->Current();
				if(drawParam)
				{
                    DrawActiveParamMarkers(drawParam, zeroBasedRowIndex);
					itsDrawingEnvironment->SetFrameColor(CtrlViewUtils::GetParamTextColor(drawParam->DataType(), drawParam->UseArchiveModelData()));

					auto paramNameStr(CtrlViewUtils::GetParamNameString(drawParam, crossSectionView, false, false, 0, false, true, true, nullptr));
					if(IsNewDataParameterName(paramNameStr))
					{
						itsDrawingEnvironment->BoldFont(true);
					}

					NFmiText text(LineTextPlace(zeroBasedRowIndex, parameterRowRect, true), paramNameStr, true, 0, itsDrawingEnvironment);
					itsToolBox->Convert(&text);
					itsDrawingEnvironment->BoldFont(false);
					DrawCheckBox(parameterRowRect, !drawParam->IsParamHidden());
					DrawModelSelectorButtons(drawParam, parameterRowRect);
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
        NFmiRect draggedParamRect = CalcParameterDragRect(itsCapturedParamRowIndex, 1, 1, 0, -1);
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
            NFmiRect dropZoneParamRect = CalcParameterDragRect(itsCurrentDragRowIndex, 3, 0, 16, -2);
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

void NFmiViewParamsView::DrawModelSelectorButtons(boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiRect& parameterRowRect)
{
	if(theDrawParam->IsModelRunDataType()) // piirret‰‰n tuottajan vaihto napit vain niille datoille, joille lˆytyy arkisto dataa
	{ // toistaiseksi vain pinta/painepinta/mallipinta datat arkistoissa
		itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, GetFrame()));
		// piirr‰ next-model-button
        CtrlView::DrawAnimationButton(CalcModelSelectorButtonRect(parameterRowRect, 2), statModelSelectorButtonImages.itsPreviousModelButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f);
        CtrlView::DrawAnimationButton(CalcModelSelectorButtonRect(parameterRowRect, 1), statModelSelectorButtonImages.itsFindNearestModelButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f);
        CtrlView::DrawAnimationButton(CalcModelSelectorButtonRect(parameterRowRect, 0), statModelSelectorButtonImages.itsNextModelButtonImage, itsGdiPlusGraphics, *itsToolBox, itsCtrlViewDocumentInterface->Printing(), GetViewSizeInPixels(), 0.7f);
	}
}

// Laskee vasempaan alariviin nappuloiden paikkoja indeksin perusteella.
// theIndex alkaa 1:st‰ ja indeksin kasvaessa nappuloita sijoitetaan aina enemm‰n oikealle.
NFmiRect NFmiViewParamsView::CalcModelSelectorButtonRect(const NFmiRect& parameterRowRect, int theButtonIndex)
{
	NFmiPoint buttonRelativeSize = CalcModelSelectorButtonRelativeSize(statModelSelectorButtonImages.itsNextModelButtonImage);

	// Pieni marginaali oikean reuna ja nappulan oikean reunan v‰liin
	auto rightSideMargin = ConvertMilliMeterToRelative(0.5, true);
	auto bottomSideMargin = ConvertMilliMeterToRelative(0.2, false);

	// Sijoitetaan n‰m‰ nappulat parametri rivilleen hieman irti reunoista.
	// nappulat sijoitetaan oikeasta reunasta kohti vasenta reunaa...
	double rightside = parameterRowRect.Right() - rightSideMargin - (theButtonIndex * buttonRelativeSize.X());
	double leftSide = rightside - buttonRelativeSize.X();
	double topSide = parameterRowRect.Bottom() - bottomSideMargin - buttonRelativeSize.Y();
	double bottomSide = topSide + buttonRelativeSize.Y();

	NFmiRect buttonRect(leftSide, topSide, rightside, bottomSide);
	return buttonRect;
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
		double layerIndexRealValue = 0;
		int layerIndex = CalcParameterRowIndex(thePlace, &layerIndexRealValue);
		return itsCtrlViewDocumentInterface->CreateViewParamsPopup(itsMapViewDescTopIndex, GetUsedParamRowIndex(), layerIndex, layerIndexRealValue) == true;
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
    NFmiDrawParamList* drawParamList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex());
	if(drawParamList && drawParamList->NumberOfItems())
	{
		if(fHasMapLayer)
	        lineCount = drawParamList->NumberOfItems() + 1; // +1 tulee map-layerist‰
		else
			lineCount = drawParamList->NumberOfItems();
	}

	// ruudun korkeus on rivien m‰‰r‰*rivinkorkeus (rivin korkeudessa on jo mukana oma marginaali)
	double heigth = lineCount * itsLineHeight;
	// Lis‰t‰‰n vertikaali marginaali alkuun ja loppuun eli 2x marginaali
	heigth += ConvertMilliMeterToRelative(itsParameterRowVerticalMarginInMM * 2, false);

	returnRect.Bottom(returnRect.Top() + heigth);
	return returnRect;
}

// Marko lis‰si 3.4.2002
bool NFmiViewParamsView::LeftDoubleClick(const NFmiPoint &thePlace, unsigned long /* theKey */)
{
	if(IsIn(thePlace))
	{
		NFmiDrawParamList* paramList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex());
		if(paramList)
		{
			int index = CalcParameterRowIndex(thePlace);
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
								NFmiMetEditorTypes::View::kFmiParamsDefaultView, &drawParam->Level(),
								drawParam->DataType(), index, drawParam->ViewMacroDrawParam());
                            // T‰m‰ on ruma fixi, mutta tupla klikki tekee t‰m‰n piilotus asetuksen jostain syyst‰ ja minun pit‰‰ laittaa se t‰ss‰ pois
                            drawParam->HideParam(!drawParam->IsParamHidden());
                            // Ik‰v‰ kyll‰ yksˆis klikin j‰lkeen (ennen t‰t‰ toista klikki‰, josta syntyy tupla klikkaus) 
                            // on tehty ruudun p‰ivitys, joka nyt tuplaklikin kohdalla pit‰‰ kumota
							itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false); 
							itsCtrlViewDocumentInterface->RefreshApplicationViewsAndDialogs("ViewParamsView::LeftDoubleClick: Double click has been pressed over parameter to open Draw-param dialog, this update fixes (UGLY) the first left-click's param show/hide action", GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
							return itsCtrlViewDocumentInterface->ExecuteCommand(menuItem, GetUsedParamRowIndex(), 1); // 1=viewtype ei m‰‰r‰tty viel‰
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
        NFmiMetEditorTypes::View::kFmiParamsDefaultView, &theDrawParam->Level(),
        theDrawParam->DataType(), theParamIndex, theDrawParam->ViewMacroDrawParam());
    return itsCtrlViewDocumentInterface->ExecuteCommand(menuItem, GetUsedParamRowIndex(), 1); // 1=viewtype ei m‰‰r‰tty viel‰
}

int NFmiViewParamsView::GetParamCount(void)
{
	NFmiDrawParamList* paramList = itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex());
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
		int realRowIndex = CalcRealRowIndex(itsViewGridRowNumber, itsViewGridColumnNumber);
		bool useCrossSectionParams = itsMapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView;
        int paramIndex = CalcParameterRowIndex(thePlace);
		if(paramIndex)
		{
			if(theKey & kShiftKey)
			{
				return itsCtrlViewDocumentInterface->ChangeParamSettingsToNextFixedDrawParam(itsMapViewDescTopIndex, realRowIndex, paramIndex, theDelta > 0 ? true : false);
			}
			else if(theKey & kCtrlKey)
			{
				return itsCtrlViewDocumentInterface->ChangeActiveMapViewParam(itsMapViewDescTopIndex, realRowIndex, paramIndex, theDelta > 0 ? true : false, useCrossSectionParams);
			}
			else
				return itsCtrlViewDocumentInterface->MoveActiveMapViewParamInDrawingOrderList(itsMapViewDescTopIndex, realRowIndex, theDelta > 0 ? true : false, useCrossSectionParams);
		}
	}
	return false;
}

bool NFmiViewParamsView::DoAfterParamModeModifications(NFmiDrawParamList *theParamList)
{
    theParamList->Dirty(true);
    itsCtrlViewDocumentInterface->CheckAnimationLockedModeTimeBags(itsMapViewDescTopIndex, false); // kun parametrin n‰kyvyytt‰ vaihdetaan, pit‰‰ tehd‰ mahdollisesti animaatio moodin datan tarkistus
    auto rowIndex = GetUsedParamRowIndex();
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
            itsCapturedParamRowIndex = CalcParameterRowIndex(thePlace);
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
        itsCurrentDragRowIndex = CalcParameterRowIndex(thePlace); // 0 indeksi on puuttuva eli ollaan parambox laatikon yl‰/ala puolella
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
    return itsCtrlViewDocumentInterface->DrawParamList(itsMapViewDescTopIndex, GetUsedParamRowIndex());
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
			int index = CalcParameterRowIndex(thePlace);
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

// Ainoa tapaus mik‰ kiinnostaa on macroParamit ja niiden kaavat
std::string NFmiViewParamsView::ComposeToolTipText(const NFmiPoint& thePlace)
{
	NFmiDrawParamList* paramList = GetDrawParamList();
	if(paramList)
	{
		int index = CalcParameterRowIndex(thePlace);
		if(paramList->Index(index))
		{
			boost::shared_ptr<NFmiDrawParam> drawParam = paramList->Current();
			if(drawParam)
			{
				auto dataType = drawParam->DataType();
				bool macroParamCase = (dataType == NFmiInfoData::kMacroParam || dataType == NFmiInfoData::kCrossSectionMacroParam || dataType == NFmiInfoData::kQ3MacroParam);
				bool wmsParamCase = (dataType == NFmiInfoData::kWmsData);
				bool crossSectionCase = itsMapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView;
				bool timeSerialCase = itsMapViewDescTopIndex == CtrlViewUtils::kFmiTimeSerialView;
				std::string paramStr = CtrlViewUtils::GetParamNameString(drawParam, crossSectionCase, !macroParamCase, true, 30, timeSerialCase, true, true, nullptr);
				paramStr = DoBoldingParameterNameTooltipText(paramStr);
				auto fontColor = CtrlViewUtils::GetParamTextColor(drawParam->DataType(), drawParam->UseArchiveModelData());
				paramStr = AddColorTagsToString(paramStr, fontColor, true);
				if(macroParamCase)
				{
					return MakeMacroParamTooltipText(drawParam, paramStr);
				}
				else if(wmsParamCase)
				{
					return MakeWmsTooltipText(drawParam, paramStr);
				}
				else
				{
					std::string str = paramStr;
					std::string tmpLatestObsStr = CtrlViewUtils::GetLatestObservationTimeString(drawParam, itsCtrlViewDocumentInterface, ::GetDictionaryString("YYYY.MM.DD HH:mm"), crossSectionCase);
					if(!tmpLatestObsStr.empty())
					{
						str += "\n";
						str += tmpLatestObsStr;
					}

					if(itsMapViewDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
					{
						std::vector<boost::shared_ptr<NFmiFastQueryInfo>> infoVector;
						auto mapViewArea = itsCtrlViewDocumentInterface->GetMapHandlerInterface(itsMapViewDescTopIndex)->Area();
						itsCtrlViewDocumentInterface->MakeDrawedInfoVectorForMapView(infoVector, drawParam, mapViewArea);
						if(!infoVector.empty())
						{
							for(const auto &info : infoVector)
							{
								str += "\n";
								str += NFmiFastInfoUtils::GetTotalDataFilePath(info);
							}
						}
					}
					else
					{
						auto info = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(drawParam, crossSectionCase, true);
						if(info)
						{
							str += "\n";
							str += NFmiFastInfoUtils::GetTotalDataFilePath(info);
						}
					}
					return str;
				}
			}
		}
	}
	return "";
}

// HUOM! theRowIndex on parametrin rivi numero t‰ll‰ karttan‰yttˆ rivill‰. Sill‰ lasketaan painonappien paikat.
// Dokumentin funktioille pit‰‰ antaa taas itsRowNumber, joka on taas t‰m‰n n‰ytˆn sijainti riveiss‰ karttan‰yttˆ ruudukossa,
// t‰t‰ tietoa tarvitaan kun pit‰‰ puhdistaa muutoksen takia karttarivi cachesta.
bool NFmiViewParamsView::LeftClickOnModelSelectionButtons(const NFmiPoint &thePlace, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theParameterRowIndex)
{
	// Jos ollaan n‰ytˆss‰, jossa fHasMapLayer = false (esim. poikkileikkausn‰yttˆ),
	// Pit‰‰ indeksi‰ s‰‰t‰‰, jotta raahaus visualisoinnit menev‰t oikein.
	if(!fHasMapLayer)
		theParameterRowIndex--;

	auto parameterRowRect = CalcParameterRowRect(theParameterRowIndex);
    auto usedRowIndex = GetUsedParamRowIndex();
    if(CalcModelSelectorButtonRect(parameterRowRect, 2).IsInside(thePlace)) // t‰ss‰ tutkitaan osuiko hiiren klikkaus previous-nappiin (2)
	{
		itsCtrlViewDocumentInterface->SetModelRunOffset(theDrawParam, -1, itsMapViewDescTopIndex, usedRowIndex);
		return true;
	}
	else if(CalcModelSelectorButtonRect(parameterRowRect, 0).IsInside(thePlace)) // t‰ss‰ tutkitaan osuiko hiiren klikkaus next-nappiin (0)
	{
		itsCtrlViewDocumentInterface->SetModelRunOffset(theDrawParam, 1, itsMapViewDescTopIndex, usedRowIndex);
		return true;
	}
	else if(CalcModelSelectorButtonRect(parameterRowRect, 1).IsInside(thePlace)) // t‰ss‰ tutkitaan osuiko hiiren klikkaus nearest-model-nappiin (1)
	{
		itsCtrlViewDocumentInterface->SetNearestBeforeModelOrigTimeRunoff(theDrawParam, itsCtrlViewDocumentInterface->CurrentTime(itsMapViewDescTopIndex), itsMapViewDescTopIndex, usedRowIndex);
		return true;
	}
	return false;
}

std::string NFmiViewParamsView::MakeMacroParamTooltipText(const boost::shared_ptr<NFmiDrawParam> &drawParam, const std::string& paramStr)
{
	try
	{
		std::string macroParamTooltip = paramStr;
		macroParamTooltip += "<br>File: ";
		NFmiFileString macroParamFilename = drawParam->InitFileName();
		macroParamFilename.Extension("st");
		macroParamTooltip += macroParamFilename;
		macroParamTooltip += "<br><hr color=red><br>"; // v‰liviiva
		macroParamTooltip += CtrlViewUtils::XmlEncode(CtrlViewUtils::GetMacroParamFormula(itsCtrlViewDocumentInterface->MacroParamSystem(), drawParam));
		return macroParamTooltip;
	}
	catch(...)
	{
	}
	return "";
}

std::string NFmiViewParamsView::MakeWmsTooltipText(const boost::shared_ptr<NFmiDrawParam>& drawParam, const std::string& paramStr)
{
	std::string str = paramStr;
	try
	{
		auto& wmsSupport = itsCtrlViewDocumentInterface->GetWmsSupport();
		const auto& dataIdent = drawParam->Param();
		auto layerInfo = wmsSupport.getHashedLayerInfo(dataIdent);
		auto fixedLayerInfoName = layerInfo.name;
		boost::replace_all(fixedLayerInfoName, ":", "/");
		str += "<br>" + layerInfo.style.legendDomain + " + " + fixedLayerInfoName;
	}
	catch(std::exception &e)
	{
		std::string errorMessage = "NFmiViewParamsView::MakeWmsTooltipText failed: ";
		errorMessage += e.what();
		CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Operational, true);
	}
	return str;
}
