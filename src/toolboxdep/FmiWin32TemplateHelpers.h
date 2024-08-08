#pragma once

#include "NFmiMapViewDescTop.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiToolBox.h"
#include "SmartMetDocumentInterface.h"
#include "catlog/catlog.h"
#include "CtrlViewWin32Functions.h"
#include "UnicodeStringConversions.h"
#include "CtrlViewDocumentInterface.h"
#include "MapDrawFunctions.h"
#include "FmiWin32Helpers.h"

namespace CFmiWin32TemplateHelpers
{
    template<class Tview>
    class GraphicalInfoRestorer
    {
        Tview& view_;
        TrueMapViewSizeInfo originalTrueMapViewSizeInfo_;
        CtrlViewUtils::GraphicalInfo originalGraphicalInfo_;
    public:
        GraphicalInfoRestorer(Tview& theView)
            :view_(theView)
            ,originalTrueMapViewSizeInfo_(theView.GetTrueMapViewSizeInfo())
            ,originalGraphicalInfo_(theView.GetGraphicalInfo())
        {
        }

        ~GraphicalInfoRestorer()
        {
            view_.GetTrueMapViewSizeInfo() = originalTrueMapViewSizeInfo_;
            originalGraphicalInfo_.fInitialized = false;
            view_.GetGraphicalInfo() = originalGraphicalInfo_;
        }
    };


	template<class Tview>
	void PrintMapViewWithFullResolution(Tview *theView, CDC* pDC, CPrintInfo* pInfo)
	{
        SmartMetDocumentInterface *smartMetDocumentInterface = theView->GetSmartMetDocumentInterface();
		NFmiToolBox *toolBox = theView->ToolBox();
		if(smartMetDocumentInterface == nullptr || toolBox == nullptr)
			return ; // pit‰isi heitt‰‰ poikkeus

        GraphicalInfoRestorer<Tview> graphicalInfoRestorer(*theView);
		// HUOM!! printtauksen yhteydess‰ kutsu ensin DC:n asetus ja sitten printinfon!!!
		toolBox->SetDC(pDC);
		toolBox->GetPrintInfo(pInfo); // ensin pit‰‰ laittaa CDC ja sitten printInfo!!!

        NFmiPoint oldSize = theView->PrintViewSizeInPixels();
        auto mapViewDescTopIndex = theView->MapViewDescTopIndex();
        smartMetDocumentInterface->Printing(mapViewDescTopIndex, true);
        try
        {
            // lasketaan sovitus printtaus alueeseen, niin ett‰ kuva ei v‰‰risty ja se sopii paperille
            // Lis‰ksi karttan‰yttˆ piirret‰‰n aina ilman aikakontrolli-ikkunaa, joten lasketaan uusi alue pelk‰n karttan‰yttˆosion mukaan
            // kuitenkin niin ett‰ suhteet eiv‰t muutu.
            CSize destSize = pInfo->m_rectDraw.Size();
            CSize screenArea = theView->GetPrintedAreaOnScreenSizeInPixels();

            double widthFactor = destSize.cx / static_cast<double>(screenArea.cx);
            double heightFactor = destSize.cy / static_cast<double>(screenArea.cy);
            int usedDestWidth = screenArea.cx;
            int usedDestHeight = screenArea.cy;
            if(smartMetDocumentInterface->ApplicationWinRegistry().FitToPagePrint())
            {
                usedDestWidth = destSize.cx;
                usedDestHeight = destSize.cy;
            }
            else
            {
                if(widthFactor > heightFactor) // t‰m‰ koodi sovittaa kuvan niin ett‰ siit‰ tuleee mahd. iso ja mahtuu annetulle paperille muotoa muuttamatta
                {
                    usedDestWidth = static_cast<int>(::round(usedDestWidth * heightFactor));
                    usedDestHeight = static_cast<int>(::round(usedDestHeight * heightFactor));
                }
                else
                {
                    usedDestWidth = static_cast<int>(::round(usedDestWidth * widthFactor));
                    usedDestHeight = static_cast<int>(::round(usedDestHeight * widthFactor));
                }
            }

            CSize newDestSize = CSize(usedDestWidth, usedDestHeight);
            CRect newDestRect(pInfo->m_rectDraw.TopLeft(), newDestSize);
            pInfo->m_rectDraw = newDestRect; // toimiiko t‰m‰ uuden koon asetus?
            toolBox->GetPrintInfo(pInfo); // otetaan printinfosta rect uudestaan!!!

            NFmiPoint viewSizeInPixels(pInfo->m_rectDraw.right - pInfo->m_rectDraw.left, pInfo->m_rectDraw.bottom - pInfo->m_rectDraw.top);
            theView->PrintViewSizeInPixels(viewSizeInPixels);
            if(!smartMetDocumentInterface->DoMapViewOnSize(mapViewDescTopIndex, viewSizeInPixels, pDC))
            {
                CFmiWin32Helpers::SetDescTopGraphicalInfo(theView->IsMapView(), theView->GetGraphicalInfo(), pDC, theView->PrintViewSizeInPixels(), smartMetDocumentInterface->DrawObjectScaleFactor(), true); // true pakottaa initialisoinnin
            }
            theView->SetPrintCopyCDC(pDC);

            if(smartMetDocumentInterface->IsToolMasterAvailable())
            {
                CtrlView::SetToolMastersDC(pDC, pInfo->m_rectDraw);
            }

            theView->MakePrintViewDirty(true, true);
            theView->OldWayPrintUpdate(); // t‰m‰ pit‰‰ tehd‰ ett‰ prionttauksen aikaiset mapAreat ja systeemit tulevat voimaan
            theView->DoDraw();
            theView->DrawOverBitmapThings(toolBox); //en tied‰ voiko t‰m‰n sijoittaa DoDraw:in?
        }
        catch(...)
        { }
        // Try-catch varmistaa ett‰ lopun takaisinasetukset tehd‰‰n
		theView->GetGraphicalInfo().fInitialized = false; // printtauksen j‰lkeen t‰m‰ pit‰‰ taas laittaa falseksi ett‰ piirto osaa initialisoida sen uudestaan
        smartMetDocumentInterface->Printing(mapViewDescTopIndex, false);
        theView->SetPrintCopyCDC(nullptr);
		theView->PrintViewSizeInPixels(oldSize);
		CatLog::logMessage("Map view printed", CatLog::Severity::Info, CatLog::Category::Visualization);
		theView->MakePrintViewDirty(true, true);
		theView->OldWayPrintUpdate(); // t‰m‰ pit‰‰ tehd‰ ett‰ prionttauksen aikaiset mapAreat ja systeemit tulevat voimaan
        theView->Invalidate(FALSE);
	}

	template<class Tview>
	void OnPrintMapView(Tview *theView, CDC* pDC, CPrintInfo* pInfo)
	{
        CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->SetPrintedDescTopIndex(theView->MapViewDescTopIndex());
        SmartMetDocumentInterface *smartMetDocumentInterface = theView->GetSmartMetDocumentInterface();
		if(pInfo->m_nCurPage > 1) // jos ollaan sivuilla 2-n, pit‰‰ aina askeltaa desctopin aikaa eteenp‰in. Muista ett‰ pit‰‰ ottaa huomioon viel‰ eri aikamoodit ja kartta ruudukon koko
		{
			NFmiMetTime aTime = theView->PrintingStartTime();
			aTime.ChangeByMinutes((pInfo->m_nCurPage-1) * theView->PrintingPageShiftInMinutes());
			theView->CurrentPrintTime(aTime);
		}

		bool doAllwaysLowMemoryPrint = theView->DoAllwaysLowMemoryPrint(); // Toistaiseksi apukarttan‰ytˆt printataan k‰ytt‰en valmista cache kuvaa, koska t‰ysresoluutio printtaus on ilmeisesti
																// aina toiminut ep‰m‰‰r‰isesti. Ainakin versiosta 5.3.x l‰htien apukarttan‰yttˆjen printtaus on ontunut.
																// Ongelma liittyy jotenkin siihen ett‰ itsToolBox:in CDC vaihtuu jossain tilanteissa kesken piirtoa, enk‰ tied‰ mik‰ sen aiheuttaa.

        if(doAllwaysLowMemoryPrint || smartMetDocumentInterface->ApplicationWinRegistry().LowMemoryPrint())
		{
			// Printataan vain memorybitmap koska oikea printtaus vaatii paljon mm. symboli koko  jne logiikkaa
			if(theView->MemoryBitmap())
			{
				bool status = CFmiWin32Helpers::BitmapToPrinter(pDC,
																theView->MemoryBitmap(),
																pInfo,
																theView->RelativePrintRect(), 
                                                                smartMetDocumentInterface->ApplicationWinRegistry().FitToPagePrint());
				if(status == false)
                    smartMetDocumentInterface->LogAndWarnUser("Cannot print the map view because BitmapToPrinter failed.", "Printing error", CatLog::Severity::Error, CatLog::Category::Visualization, false);
			}
			else
                smartMetDocumentInterface->LogAndWarnUser("Error when printing using low memory print-out, no memory bitmap available.", "Printing error", CatLog::Severity::Error, CatLog::Category::Visualization, false);
		}
		else
			CFmiWin32TemplateHelpers::PrintMapViewWithFullResolution(theView, pDC, pInfo);
//			CFmiWin32TemplateHelpers::PrintMapViewWithFullResolutionUsingBitmap(theView, pDC, pInfo); // TƒMƒ FUNKTIO AIHEUTTAA BLUE-SCREENIN!!!

		// jos ollaan viimeisell‰ printti sivulla ja ollaan printattu monisivu printti‰, pit‰‰ originaali aika viel‰ asettaa takaisin
		if(pInfo->m_nCurPage > 1 && pInfo->GetToPage() == pInfo->m_nCurPage)
			theView->CurrentPrintTime(theView->PrintingStartTime());
        CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->ResetPrintedDescTopIndex();
    }

    // Halutaan ett‰ kun mist‰ tahansa n‰ytˆst‰ k‰ynnistet‰‰n t‰m‰ prnttaus prosessi, 
    // niin printtaavan ikkunan pit‰‰ olla Printtaus-dialogin emoikkuna, jotta dialogi 
    // keskittyy printtaavan ikkunan keskelle, eik‰ aina p‰‰ikkunan keskelle.
    inline void SetNewParentToPrintDialog(CWnd *newParent, CPrintInfo* pInfo)
    {
        if(newParent && pInfo)
        {
            // 1. Luodaan new:lla uusi CPrintDialog, jonka parent on haluttu ikkuna, default arvot CPrintInfo::CPrintInfo() -metodista
            DWORD flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOSELECTION;
            auto newPrintDlg = new CPrintDialog(FALSE, flags, newParent);
            // 2. Otetaan tietyt asetukset (jotka asetettu CPrintInfo::CPrintInfo() -metodissa) viel‰ vanhasta dialogista
            newPrintDlg->m_pd.nMinPage = pInfo->m_pPD->m_pd.nMinPage;
            newPrintDlg->m_pd.nMaxPage = pInfo->m_pPD->m_pd.nMaxPage;
            // 3. deletoidaan vanha dialogi pois alta
            delete pInfo->m_pPD;
            // 4. Asetetaan uusi tilalle
            pInfo->m_pPD = newPrintDlg;
            // 5. Samat loppu tarkistukset kuin CPrintInfo::CPrintInfo() -metodissa
            ASSERT(pInfo->m_pPD->m_pd.hDC == NULL);
        }
    }

	template<class Tview>
	BOOL OnPreparePrintingMapView(Tview *theView, CPrintInfo* pInfo, bool disableRangePrint)
	{
        SetNewParentToPrintDialog(theView, pInfo);
		theView->PrintingStartTime(theView->CalcPrintingStartTime()); // otetaan t‰m‰ nyt talteen, ett‰ saadaan se asetettua taas takaisin, kun lopetetaan moni sivu printtausta
		theView->PrintingPageShiftInMinutes(theView->CalcPrintingPageShiftInMinutes());
        auto &applicationWinRegistry = theView->GetSmartMetDocumentInterface()->ApplicationWinRegistry();
		int usedMaxPages = applicationWinRegistry.MaxRangeInPrint();
		if(disableRangePrint || applicationWinRegistry.LowMemoryPrint())
			usedMaxPages = 1; // koska low-memory printtaus perustuu muistissa olevaan kuvaan, ei toistaiseksi voi tulostaa kuin nykyisen hetken eli yhden kuvan vain
		pInfo->SetMaxPage(usedMaxPages);

		BOOL status = theView->DoPreparePrinting(pInfo);

		if(pInfo->m_pPD->PrintAll())
			pInfo->SetMaxPage(1); // jos oli PrintAll valittuna, mik‰ on default valinta, pit‰‰ maxPageksi laittaa taas yksi, muuten systeemi printtaa 1:sta maxPaageen asti sivuja

		return status;
	}

	template<class Tview>
	void OnPrepareDCMapView(Tview *theView, CDC* pDC, CPrintInfo* pInfo)
	{
		if(pDC->IsPrinting())
		{
            SmartMetDocumentInterface *smartMetDocumentInterface = theView->GetSmartMetDocumentInterface();
			short usedPaperSize = pInfo->m_pPD->GetDevMode()->dmPaperSize; // 'Smart'-orientation saadaan tehd‰ vain standardi paperi koille. Jos k‰ytt‰j‰ 
																			// on s‰‰t‰nyt erikois paperikoon, tulee kooksi DMPAPER_USER, ja silloin ei orientaatiota saa muuttaa mitenk‰‰n.

			if(smartMetDocumentInterface->ApplicationWinRegistry().SmartOrientationPrint() && usedPaperSize > 0 && usedPaperSize <= DMPAPER_LAST) // DMPAPER_USER ei toimi suoraan vaikka speksien mukaan pit‰isi
			{
				CSize screenArea = theView->GetPrintedAreaOnScreenSizeInPixels();
				bool needToChangeOrientation = false;
				// tarkistetaan tarvitaanko orientaation vaihtoa
				short usedOrientation = pInfo->m_pPD->GetDevMode()->dmOrientation;
				short newOrientation = usedOrientation;
				if(usedOrientation == DMORIENT_PORTRAIT && (screenArea.cy < screenArea.cx))
				{
					newOrientation = DMORIENT_LANDSCAPE;
					needToChangeOrientation = true;
				}
				else if(usedOrientation == DMORIENT_LANDSCAPE && (screenArea.cy > screenArea.cx))
				{
					newOrientation = DMORIENT_PORTRAIT;
					needToChangeOrientation = true;
				}
				if(needToChangeOrientation)
				{
					CFmiWin32Helpers::SetPrinterMode(pDC, newOrientation);
				}
			}
		}
	}

	template<class Tview>
    void DoWindowSizeSettings(Tview *theView, bool justMoveWindow, std::string theBaseErrorStr)
    {
        try
        {
            Persist2::ReadWindowRectFromSettings(theView->SettingKeyBaseStr(), theView, justMoveWindow);
            return;
        }
        catch(std::exception &e)
        {
            std::string errStr;
            errStr += theBaseErrorStr;
            errStr += "\n";
            errStr += e.what();
            CatLog::logMessage(errStr, CatLog::Severity::Error, CatLog::Category::Configuration);
        }
        catch(...)
        {
            std::string errStr("Unknown ");
            errStr += theBaseErrorStr;
            CatLog::logMessage(errStr, CatLog::Severity::Error, CatLog::Category::Configuration);
        }
        theView->SetDefaultValues(); // asetetaan virhe tilanteessa ikkuna defaultti paikkaan ja kokoon
    }

	template<class Tview>
    void DoWindowSizeSettingsFromWinRegistry(NFmiApplicationWinRegistry &theApplicationWinRegistry, Tview *theView, bool justMoveWindow, std::string theBaseErrorStr, unsigned int theMapViewDescTopIndex)
    {
        try
        {
            Persist2::ReadWindowRectFromWinRegistry(theApplicationWinRegistry, theView->MakeUsedWinRegistryKeyStr(theMapViewDescTopIndex), theView, justMoveWindow);
            return;
        }
        catch(std::exception &e)
        {
            std::string errStr;
            errStr += theBaseErrorStr;
            errStr += "\n";
            errStr += e.what();
            CatLog::logMessage(errStr, CatLog::Severity::Error, CatLog::Category::Configuration);
        }
        catch(...)
        {
            std::string errStr("Unknown ");
            errStr += theBaseErrorStr;
            CatLog::logMessage(errStr, CatLog::Severity::Error, CatLog::Category::Configuration);
        }
        theView->SetDefaultValues(); // asetetaan virhe tilanteessa ikkuna defaultti paikkaan ja kokoon
    }

	template<class Tview>
    void ArrowKeyMapPan(Tview *theView, SmartMetDocumentInterface *smartMetDocumentInterface, const NFmiPoint &thePanDragMoveFactor)
    {
        // haetaan 1. rivin 1. mapview, jotta silt‰ voidaan pyyt‰‰ kartan pannaukseen liittyvi‰ tietoja
        NFmiStationViewHandler *mapView = theView->GetMapViewHandler(1, 1);
        if(mapView)
        {
            boost::shared_ptr<NFmiArea> mapArea = mapView->GetArea();
            NFmiPoint mapViewCenter = mapArea->XYArea().Center();
            NFmiPoint movePoint(mapArea->XYArea().Width() * thePanDragMoveFactor.X(), mapArea->XYArea().Height() * thePanDragMoveFactor.Y());
            NFmiPoint zoomDragUpPoint(mapViewCenter + movePoint);
            smartMetDocumentInterface->PanMap(theView->MapViewDescTopIndex(), mapArea, mapViewCenter, zoomDragUpPoint);
            smartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Pan map with arrow keys");
        }
    }

	template<class Tview>
    void ArrowKeyMapZoom(Tview *theView, SmartMetDocumentInterface *smartMetDocumentInterface, double theZoomFactor)
    {
        // haetaan 1. rivin 1. mapview, jotta silt‰ voidaan pyyt‰‰ zoomaukseen liittyvi‰ tietoja
        NFmiStationViewHandler *mapView = theView->GetMapViewHandler(1, 1);
        if(mapView)
        {
            boost::shared_ptr<NFmiArea> mapArea = mapView->GetArea();
            NFmiPoint mapViewCenter = mapArea->XYArea().Center();
            smartMetDocumentInterface->ZoomMapInOrOut(theView->MapViewDescTopIndex(), mapArea, mapViewCenter, theZoomFactor);
            smartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Zoom map with arrow keys");
        }
    }

    template<class Tview>
    class NFmiMapBlitDCHandler
    {
        Tview& mapView_;
        CFmiWin32Helpers::DeviceContextHelper& usedDcHelper_;
        CDC& compatibilityDc_;
        NFmiMapViewDescTop& mapViewDesctop_;
    public:
        NFmiMapBlitDCHandler(Tview& mapView, CFmiWin32Helpers::DeviceContextHelper& usedDcHelper, CDC& compatibilityDc, NFmiMapViewDescTop& mapViewDesctop)
            :mapView_(mapView)
            , usedDcHelper_(usedDcHelper)
            , compatibilityDc_(compatibilityDc)
            , mapViewDesctop_(mapViewDesctop)
        {
            mapView_.GenerateMapBitmap(mapView_.MapBitmap(), &usedDcHelper_.getDc(), &compatibilityDc_);
            mapViewDesctop_.MapBlitDC(&usedDcHelper_.getDc());
        }

        ~NFmiMapBlitDCHandler()
        {
            mapViewDesctop_.MapBlitDC(nullptr);
        }
    };

    class NFmiCopyCDCHandler
    {
        CFmiWin32Helpers::DeviceContextHelper usedDcHelper_;
        NFmiMapViewDescTop& mapViewDesctop_;
    public:
        NFmiCopyCDCHandler(CDC* usedDc, NFmiMapViewDescTop& mapViewDesctop)
            :usedDcHelper_(usedDc)
            , mapViewDesctop_(mapViewDesctop)
        {
            mapViewDesctop_.CopyCDC(&usedDcHelper_.getDc());
        }

        ~NFmiCopyCDCHandler()
        {
            mapViewDesctop_.CopyCDC(nullptr);
        }
    };

    template<class Tview>
    void SetupDrawnBitmap(Tview* theView, CDC *compatibleDC, CBitmap& theDrawnScreenBitmapOut, CFmiWin32Helpers::DeviceContextHelper &usedDC)
    {
        CRect clientArea;
        theView->GetClientRect(&clientArea);
        theDrawnScreenBitmapOut.DeleteObject();
        theDrawnScreenBitmapOut.CreateCompatibleBitmap(compatibleDC, clientArea.Width(), clientArea.Height());
        usedDC.SelectBitmap(&theDrawnScreenBitmapOut);
        theView->ToolBox()->SetDC(&usedDC.getDc());
    }

    template<class Tview>
    void DoOffScreenDraw(Tview *theView, SmartMetDocumentInterface *smartMetDocumentInterface, CBitmap &theDrawnScreenBitmapOut)
    {
        if(theView == nullptr)
            return;

        unsigned int mapViewDesktopIndex = theView->MapViewDescTopIndex();
        if(MapDraw::stopDrawingTooSmallMapview(theView, mapViewDesktopIndex))
            return;

        CClientDC dc(theView);
        CFmiWin32Helpers::DeviceContextHelper dcMem(&dc);

        auto* mapViewDesctop = smartMetDocumentInterface->MapViewDescTop(mapViewDesktopIndex);
        // v‰limuistin apuna k‰ytetty dc
        NFmiCopyCDCHandler copyCDCHandler(&dc, *mapViewDesctop);

        SetupDrawnBitmap(theView, &dc, theDrawnScreenBitmapOut, dcMem);

        // *** T‰ss‰ tehd‰‰n background kartta ***
        CFmiWin32Helpers::DeviceContextHelper dcMem2(&dc);
        NFmiMapBlitDCHandler<Tview> mapBlitDC(*theView, dcMem2, dc, *mapViewDesctop);
        // *** T‰ss‰ tehd‰‰n background kartta ***

        theView->SetToolsDCs(&dcMem.getDc());
        theView->DoDraw();
        theView->DrawOverBitmapThings(theView->ToolBox());
    }

    template<class Tview>
    void DoPossibleAutoAdjustValueScalesSetups(Tview* theView, SmartMetDocumentInterface* smartMetDocumentInterface)
    {
        auto mapViewDescTopIndex = theView->MapViewDescTopIndex();
        if(mapViewDescTopIndex == CtrlViewUtils::kFmiTimeSerialView || mapViewDescTopIndex == CtrlViewUtils::kFmiSoundingView)
        {
            const auto *currentBetaProduct = smartMetDocumentInterface->GetCurrentGeneratedBetaProduct();
            if(currentBetaProduct && currentBetaProduct->EnsureCurveVisibility())
            {
                theView->AutoAdjustValueScales(false, false);
            }
        }
    }

    template<class Tview>
    void DoOffScreenDrawForNonMapView(Tview *theView, SmartMetDocumentInterface *smartMetDocumentInterface, CBitmap & theDrawnScreenBitmapOut)
    {
        if(theView == nullptr)
            return;

        // Mahdollinen asteikoiden s‰‰tˆ pit‰‰  tehd‰ ennen kuin view-luokkien device-context:eja aletaan asettelemaan!!!
        DoPossibleAutoAdjustValueScalesSetups(theView, smartMetDocumentInterface);

        CClientDC dc(theView);
        CFmiWin32Helpers::DeviceContextHelper dcMem(&dc);

        if(MapDraw::stopDrawingTooSmallMapview(theView, theView->MapViewDescTopIndex()))
            return;

        SetupDrawnBitmap(theView, &dc, theDrawnScreenBitmapOut, dcMem);
        theView->SetToolMastersDC(&dcMem.getDc());
        theView->DoDraw();
        theView->DrawOverBitmapThings(theView->ToolBox());
    }

    template<class Tview>
    void DrawSynopPlotImage(Tview *view, bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot)
    {
        CClientDC dc(view);
        CDC dcMem;
        dcMem.CreateCompatibleDC(&dc);
        CBitmap *oldBitmap = dcMem.SelectObject(view->SynopPlotBitmap());
        view->SetToolBoxsDC(&dcMem);
        BITMAP pBitMap;
        view->SynopPlotBitmap()->GetBitmap(&pBitMap);
        // pakko tehd‰ t‰ll‰inen viritys, koska view:sta luotu dc on eri kokoinen kuin k‰ytetty bitmapin koko
        view->ToolBox()->SetClientRect(CRect(0, 0, pBitMap.bmWidth, pBitMap.bmHeight));

        // pit‰‰ tyhjent‰‰ bitmappi piirt‰m‰ll‰ valkoinen rect fillill‰
        NFmiDrawingEnvironment envi;
        envi.EnableFill();
        envi.EnableFrame();
        envi.SetFillColor(NFmiColor(1, 1, 1));
        envi.SetFrameColor(NFmiColor(0.8f, 0.8f, 0.8f)); // piirret‰‰n vaalean harmaalla reunus
        NFmiRect bitmapRect(0, 0, 1, 1);
        NFmiRectangle drawRect(bitmapRect, 0, &envi);
        view->ToolBox()->Convert(&drawRect);

        NFmiRect drawingrect = bitmapRect;

        drawingrect.Inflate(-0.23); // en tied‰ miksi, mutta t‰ll‰lailla pit‰‰ piirto laatikkoa pienent‰‰, muuten synop-plotti levi‰‰ piirrett‰ess‰ bitmappiin
        view->EditMapView()->DrawSynopPlotOnToolTip(view->ToolBox(), drawingrect, fDrawSoundingPlot, fDrawMinMaxPlot, fDrawMetarPlot);
        dcMem.SelectObject(oldBitmap);
        dcMem.DeleteDC();
    }

    inline bool AllowTooltipDisplay(SmartMetDocumentInterface* smartMetDocumentInterface)
    {
        if(smartMetDocumentInterface->Printing())
            return false; // Printtauksen yhteydess‰ ei tehd‰ mit‰‰n tooltip virityksi‰!
        if(smartMetDocumentInterface->MouseCaptured())
            return false; // Jos hiirt‰ raahataan mitenk‰‰n, ei tehd‰ tooltippej‰!

        return true;
    }

    template<class Tview>
    void NotifyDisplayTooltip(Tview *view, NMHDR * pNMHDR, LRESULT * result, SmartMetDocumentInterface* smartMetDocumentInterface, int tooltipId)
    {
        if(!AllowTooltipDisplay(smartMetDocumentInterface))
            return;

        *result = 0;
        NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

        if(pNotify->ti->nIDTool == tooltipId)
        {
            CPoint pt = *pNotify->pt;
            view->ScreenToClient(&pt);

            CString strU_;

            try
            {
                CDC *theDC = view->GetDC();
                if(!theDC)
                    return;
                view->SetToolsDCs(theDC);
                NFmiPoint relativePoint(view->ToolBox()->ToViewPoint(pt.x, pt.y));
                view->ReleaseDC(theDC);
                strU_ = ::convertPossibleUtf8StringToWideString(view->EditMapView()->ComposeToolTipText(relativePoint)).c_str();
            }
            catch(std::exception &e)
            {
                strU_ = _TEXT("Error while making the tooltip string:\n");
                strU_ += CA2T(e.what());
            }
            catch(...)
            {
                strU_ = _TEXT("Error (unknown) while making the tooltip string");
            }

            if(strU_.IsEmpty())
            {
                if(view->GetSmartMetDocumentInterface()->EditedSmartInfo() == 0)
                    strU_ = _TEXT("No data in system");
            }

            bool drawSoundingPlot = false;
            bool drawMinMaxPlot = false;
            bool drawMetarPlot = false;
            std::string tmpStr = CT2A(strU_);
            std::string foundPlotStr;
            std::string::size_type pos = CFmiExtraMapView::FindPlotImagePosition(tmpStr, foundPlotStr, drawSoundingPlot, drawMinMaxPlot, drawMetarPlot);

            if(pos != std::string::npos)
            { // jos lˆytyi, laitetaan imagelist tagit kuntoon stringiin
                std::string imgListStr = "<ilst idres=0 mask>";
                tmpStr.replace(pos, foundPlotStr.size(), imgListStr);
                strU_ = CA2T(tmpStr.c_str());
                DrawSynopPlotImage(view, drawSoundingPlot, drawMinMaxPlot, drawMetarPlot); // tehd‰‰n haluttu synop-plotti bitmappiin ja laiteaan se tooltip-systeemiin
                BITMAP pBitMap;
                view->SynopPlotBitmap()->GetBitmap(&pBitMap);
                view->ToolTipControl()->SetImageList(view->SynopPlotBitmapHandle(), pBitMap.bmWidth, pBitMap.bmHeight, 1);
            }
            pNotify->ti->sTooltip = strU_;
        }
    }

    inline void DoBitBltOperation(CDC &destinyDc, CDC &sourceDc, const CRect &clientArea, DWORD drawOperation)
    {
        destinyDc.BitBlt(0
            , 0
            , clientArea.Width()
            , clientArea.Height()
            , &sourceDc
            , 0
            , 0
            , drawOperation);
    }

    template<class Tview>
    void DoGraphicalInfoSetup(Tview* mapView, CDC* pDC, SmartMetDocumentInterface* smartMetDocumentInterface)
    {
        auto mapViewDescTopIndex = mapView->MapViewDescTopIndex();
        auto& gInfo = smartMetDocumentInterface->GetGraphicalInfo(mapViewDescTopIndex);
        CFmiWin32Helpers::SetDescTopGraphicalInfo(mapView->IsMapView(), gInfo, pDC, mapView->PrintViewSizeInPixels(), smartMetDocumentInterface->DrawObjectScaleFactor());
        CFmiWin32Helpers::DoGraphReportOnDraw(gInfo, smartMetDocumentInterface->DrawObjectScaleFactor());
    }


    template<class Tview>
    void MapViewOnDraw(Tview *mapView, CDC* pDC, SmartMetDocumentInterface *smartMetDocumentInterface)
    {
        if(smartMetDocumentInterface->Printing())
            return; // tulee ongelmia, jos ruutua p‰ivitet‰‰n kun samalla printataan

        DoGraphicalInfoSetup(mapView, pDC, smartMetDocumentInterface);
        auto mapViewDescTopIndex = mapView->MapViewDescTopIndex();
        if(MapDraw::stopDrawingTooSmallMapview(mapView, mapViewDescTopIndex))
            return;

        CClientDC dc(mapView);
        CFmiWin32Helpers::DeviceContextHelper dcMem(&dc);

        auto memoryBitmap = mapView->MemoryBitmap();
        auto mapViewDesctop = smartMetDocumentInterface->MapViewDescTop(mapViewDescTopIndex);
        if(mapViewDesctop->RedrawMapView() || smartMetDocumentInterface->ViewBrushed())
        {
            // v‰limuistin apuna k‰ytetty dc
            NFmiCopyCDCHandler copyCDCHandler(&dc, *mapViewDesctop);

            WaitCursorHelper waitCursorHelper(smartMetDocumentInterface->ShowWaitCursorWhileDrawingView());
            if(!smartMetDocumentInterface->ViewBrushed())
            {
                CtrlView::MakeCombatibleBitmap(mapView, &memoryBitmap);
                // Aina kun memoryBitmap:ia s‰‰det‰‰n, pit‰‰ myˆs finalImageBitmap s‰‰t‰‰ samalla
                auto finalImageBitmap = mapView->FinalMapViewImageBitmap();
                CtrlView::MakeCombatibleBitmap(mapView, &finalImageBitmap);
            }
            dcMem.SelectBitmap(memoryBitmap);

            // *** T‰ss‰ tehd‰‰n background kartta ***
            CFmiWin32Helpers::DeviceContextHelper dcMem2(&dc);
            NFmiMapBlitDCHandler<Tview> mapBlitDC(*mapView, dcMem2, dc, *mapViewDesctop);
            // *** T‰ss‰ tehd‰‰n background kartta ***

            {
                // Varsinainen piirto t‰ss‰
                mapView->SetToolsDCs(&dcMem.getDc());
                mapView->DoDraw();
            }

            // *** T‰ss‰ background kartan j‰lkihoito ***
            mapViewDesctop->ClearRedrawMapView();
            smartMetDocumentInterface->ViewBrushed(false);
        }
        else
            dcMem.SelectBitmap(memoryBitmap);

        // Erilaisista visualisointi optimoinneista johtuen pit‰‰ kartta kuvasta piirt‰‰ ensin
        // pohjadata ja sitten sen p‰‰lle lopullinen kuva, mihin tulee kaikki mahdollinen.
        // Molemmat kuvat pit‰‰ ottaa erikseen talteen eri k‰yttˆtarkoituksiin.
        CFmiWin32Helpers::DeviceContextHelper finalImageDc(&dc);
        finalImageDc.SelectBitmap(mapView->FinalMapViewImageBitmap());

        CRect clientArea;
        mapView->GetClientRect(&clientArea);
        DoBitBltOperation(finalImageDc.getDc(), dcMem.getDc(), clientArea, SRCCOPY);

        auto toolbox = mapView->ToolBox();
        toolbox->SetDC(&finalImageDc.getDc());
        mapView->DrawOverBitmapThings(toolbox); // t‰t‰ voisi tutkia, mitk‰ voisi siirt‰‰ t‰‰lt‰ pois.
        mapViewDesctop->MapViewBitmapDirty(false);
        mapViewDesctop->MapHandler()->ClearUpdateMapViewDrawingLayers();

        // Lopuksi viimeinen kuva pit‰‰ piirt‰‰ originaali piirtopinnalle kerrallaan
        DoBitBltOperation(*pDC, finalImageDc.getDc(), clientArea, SRCCOPY);
    }

    template<typename DocumentInterface>
    void OnAcceleratorToggleVirtualTimeMode(DocumentInterface & documentInterface, const std::string &viewName)
    {
        std::string updateMessage = "Virtual-Time mode changed to ";
        updateMessage += documentInterface->VirtualTimeUsed() ? "OFF" : "ON";
        updateMessage += " from " + viewName + " by CRTL + K, reloading all model data and recalculating and redrawing everything";
        documentInterface->ToggleVirtualTimeMode(updateMessage);
        if(ApplicationInterface::GetApplicationInterfaceImplementation)
        {
            ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateAllViewsAndDialogs(updateMessage);
        }
    }

}


