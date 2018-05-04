#pragma once

#include "xmlliteutils/XMLite.h"
#include "NFmiMapViewDescTop.h"
#include "persist2.h"
#include <agx\agx.h>
#include "NFmiApplicationWinRegistry.h"
#include "NFmiToolBox.h"
#include "SmartMetDocumentInterface.h"
#include "catlog/catlog.h"

namespace CFmiWin32TemplateHelpers
{
	template<class Tview>
	void PrintMapViewWithFullResolution(Tview *theView, CDC* pDC, CPrintInfo* pInfo)
	{
        SmartMetDocumentInterface *smartMetDocumentInterface = theView->GetSmartMetDocumentInterface();
		NFmiToolBox *toolBox = theView->ToolBox();
		if(smartMetDocumentInterface == 0 || toolBox == 0)
			return ; // pitäisi heittää poikkeus

		// HUOM!! printtauksen yhteydessä kutsu ensin DC:n asetus ja sitten printinfon!!!
		toolBox->SetDC(pDC);
		toolBox->GetPrintInfo(pInfo); // ensin pitää laittaa CDC ja sitten printInfo!!!

        smartMetDocumentInterface->Printing(true);
		NFmiPoint oldSize = theView->PrintViewSizeInPixels();
		// lasketaan sovitus printtaus alueeseen, niin että kuva ei vääristy ja se sopii paperille
		// Lisäksi karttanäyttö piirretään aina ilman aikakontrolli-ikkunaa, joten lasketaan uusi alue pelkän karttanäyttöosion mukaan
		// kuitenkin niin että suhteet eivät muutu.
		CSize destSize = pInfo->m_rectDraw.Size();
		CSize screenArea = theView->GetPrintedAreaOnScreenSizeInPixels();
//		NFmiRect oldRelativeMapRect = smartMetDocumentInterface->MapViewDescTop(mapViewDescTopIndex)->RelativeMapRect();
		NFmiRect oldRelativeMapRect = theView->RelativePrintRect() ? *(theView->RelativePrintRect()) : NFmiRect(0,0,1,1);
		theView->RelativePrintRect(NFmiRect(0,0,1,1)); // annetaan printtaukseen koko suhteellinen alue käyttöön karttaosiolle

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
			if(widthFactor > heightFactor) // tämä koodi sovittaa kuvan niin että siitä tuleee mahd. iso ja mahtuu annetulle paperille muotoa muuttamatta
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
		pInfo->m_rectDraw = newDestRect; // toimiiko tämä uuden koon asetus?
		toolBox->GetPrintInfo(pInfo); // otetaan printinfosta rect uudestaan!!!

		theView->PrintViewSizeInPixels(NFmiPoint(pInfo->m_rectDraw.right  - pInfo->m_rectDraw.left, pInfo->m_rectDraw.bottom - pInfo->m_rectDraw.top));
		CFmiWin32Helpers::SetDescTopGraphicalInfo(theView->GetGraphicalInfo(), pDC, theView->PrintViewSizeInPixels(), smartMetDocumentInterface->DrawObjectScaleFactor(), true); // true pakottaa initialisoinnin
		theView->SetPrintCopyCDC(pDC);

		if(smartMetDocumentInterface->IsToolMasterAvailable())
		{
			//<STRONG><A NAME="onprint">Print function</A>
			XuWindowSize(pInfo->m_rectDraw.right  - pInfo->m_rectDraw.left,
						 pInfo->m_rectDraw.bottom - pInfo->m_rectDraw.top);
			XuWindowSelect(pDC->GetSafeHdc());
		}

		theView->MakePrintViewDirty(true, true);
		theView->OldWayPrintUpdate(); // tämä pitää tehdä että prionttauksen aikaiset mapAreat ja systeemit tulevat voimaan
		theView->DoDraw();
		theView->DrawOverBitmapThings(toolBox); //en tiedä voiko tämän sijoittaa DoDraw:in?

		theView->GetGraphicalInfo().fInitialized = false; // printtauksen jälkeen tämä pitää taas laittaa falseksi että piirto osaa initialisoida sen uudestaan
		theView->SetPrintCopyCDC(0);
		theView->PrintViewSizeInPixels(oldSize);
		theView->RelativePrintRect(oldRelativeMapRect);
		CatLog::logMessage("Map view printed", CatLog::Severity::Info, CatLog::Category::Visualization);
        smartMetDocumentInterface->Printing(false);
		theView->MakePrintViewDirty(true, true);
		theView->OldWayPrintUpdate(); // tämä pitää tehdä että prionttauksen aikaiset mapAreat ja systeemit tulevat voimaan
	}

	template<class Tview>
	void OnPrintMapView(Tview *theView, CDC* pDC, CPrintInfo* pInfo)
	{
        SmartMetDocumentInterface *smartMetDocumentInterface = theView->GetSmartMetDocumentInterface();
		if(pInfo->m_nCurPage > 1) // jos ollaan sivuilla 2-n, pitää aina askeltaa desctopin aikaa eteenpäin. Muista että pitää ottaa huomioon vielä eri aikamoodit ja kartta ruudukon koko
		{
			NFmiMetTime aTime = theView->PrintingStartTime();
			aTime.ChangeByMinutes((pInfo->m_nCurPage-1) * theView->PrintingPageShiftInMinutes());
			theView->CurrentPrintTime(aTime);
		}

		bool doAllwaysLowMemoryPrint = theView->DoAllwaysLowMemoryPrint(); // Toistaiseksi apukarttanäytöt printataan käyttäen valmista cache kuvaa, koska täysresoluutio printtaus on ilmeisesti
																// aina toiminut epämääräisesti. Ainakin versiosta 5.3.x lähtien apukarttanäyttöjen printtaus on ontunut.
																// Ongelma liittyy jotenkin siihen että itsToolBox:in CDC vaihtuu jossain tilanteissa kesken piirtoa, enkä tiedä mikä sen aiheuttaa.

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
//			CFmiWin32TemplateHelpers::PrintMapViewWithFullResolutionUsingBitmap(theView, pDC, pInfo); // TÄMÄ FUNKTIO AIHEUTTAA BLUE-SCREENIN!!!

		// jos ollaan viimeisellä printti sivulla ja ollaan printattu monisivu printtiä, pitää originaali aika vielä asettaa takaisin
		if(pInfo->m_nCurPage > 1 && pInfo->GetToPage() == pInfo->m_nCurPage)
			theView->CurrentPrintTime(theView->PrintingStartTime());
	}


	template<class Tview>
	BOOL OnPreparePrintingMapView(Tview *theView, CPrintInfo* pInfo, bool disableRangePrint)
	{
		theView->PrintingStartTime(theView->CalcPrintingStartTime()); // otetaan tämä nyt talteen, että saadaan se asetettua taas takaisin, kun lopetetaan moni sivu printtausta
		theView->PrintingPageShiftInMinutes(theView->CalcPrintingPageShiftInMinutes());
        auto &applicationWinRegistry = theView->GetSmartMetDocumentInterface()->ApplicationWinRegistry();
		int usedMaxPages = applicationWinRegistry.MaxRangeInPrint();
		if(disableRangePrint || applicationWinRegistry.LowMemoryPrint())
			usedMaxPages = 1; // koska low-memory printtaus perustuu muistissa olevaan kuvaan, ei toistaiseksi voi tulostaa kuin nykyisen hetken eli yhden kuvan vain
		pInfo->SetMaxPage(usedMaxPages);

		BOOL status = theView->DoPreparePrinting(pInfo);

		if(pInfo->m_pPD->PrintAll())
			pInfo->SetMaxPage(1); // jos oli PrintAll valittuna, mikä on default valinta, pitää maxPageksi laittaa taas yksi, muuten systeemi printtaa 1:sta maxPaageen asti sivuja

		return status;
	}

	template<class Tview>
	void OnPrepareDCMapView(Tview *theView, CDC* pDC, CPrintInfo* pInfo)
	{
		if(pDC->IsPrinting())
		{
            SmartMetDocumentInterface *smartMetDocumentInterface = theView->GetSmartMetDocumentInterface();
			short usedPaperSize = pInfo->m_pPD->GetDevMode()->dmPaperSize; // 'Smart'-orientation saadaan tehdä vain standardi paperi koille. Jos käyttäjä 
																			// on säätänyt erikois paperikoon, tulee kooksi DMPAPER_USER, ja silloin ei orientaatiota saa muuttaa mitenkään.

			if(smartMetDocumentInterface->ApplicationWinRegistry().SmartOrientationPrint() && usedPaperSize > 0 && usedPaperSize <= DMPAPER_LAST) // DMPAPER_USER ei toimi suoraan vaikka speksien mukaan pitäisi
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
        // haetaan 1. rivin 1. mapview, jotta siltä voidaan pyytää kartan pannaukseen liittyviä tietoja
        NFmiStationViewHandler *mapView = theView->GetMapViewHandler(1, 1);
        if(mapView)
        {
            boost::shared_ptr<NFmiArea> mapArea = mapView->MapArea();
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
        // haetaan 1. rivin 1. mapview, jotta siltä voidaan pyytää zoomaukseen liittyviä tietoja
        NFmiStationViewHandler *mapView = theView->GetMapViewHandler(1, 1);
        if(mapView)
        {
            NFmiPoint mapViewCenter = mapView->MapArea()->XYArea().Center();
            smartMetDocumentInterface->ZoomMapInOrOut(theView->MapViewDescTopIndex(), mapView->MapArea(), mapViewCenter, theZoomFactor);
            smartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Zoom map with arrow keys");
        }
    }

    template<class Tview>
    void DoOffScreenDraw(Tview *theView, SmartMetDocumentInterface *smartMetDocumentInterface, CBitmap &theDrawedScreenBitmapOut)
    {
        if(theView == nullptr)
            return;

        unsigned int mapViewDesktopIndex = theView->MapViewDescTopIndex();

        CClientDC dc(theView);
        CDC dcMem;
        dcMem.CreateCompatibleDC(&dc);

        CRect clientArea;
        theView->GetClientRect(&clientArea);
        if(clientArea.Height() < 4)
        {
            dcMem.DeleteDC();
            return; // kun ruudun korkeus on tarpeeksi pieni, ohjelma voi kaatua ja turha näin pientä ruutua on edes piirtää
        }

        CDC dcMemCopy; // välimuistin apuna käytetty dc
        dcMemCopy.CreateCompatibleDC(&dc);
        smartMetDocumentInterface->MapViewDescTop(mapViewDesktopIndex)->CopyCDC(&dcMemCopy);

        theDrawedScreenBitmapOut.DeleteObject();
        theDrawedScreenBitmapOut.CreateCompatibleBitmap(&dc, clientArea.Width(), clientArea.Height());

        CBitmap *oldBitmap = dcMem.SelectObject(&theDrawedScreenBitmapOut);
        theView->ToolBox()->SetDC(&dcMem);

        // *** Tässä tehdään background kartta ***
        CDC dcMem2;
        dcMem2.CreateCompatibleDC(&dc);
        CBitmap *oldBitmap2 = 0;
        theView->GenerateMapBitmap(theView->MemoryBitmap(), &dcMem2, &dc, oldBitmap2);
        smartMetDocumentInterface->MapViewDescTop(mapViewDesktopIndex)->MapBlitDC(&dcMem2);
        // *** Tässä tehdään background kartta ***

        {
            theView->SetToolMastersDC(&dcMem);
            theView->DoDraw();
        }

        // *** Tässä background kartan jälkihoito ***
        smartMetDocumentInterface->MapViewDescTop(mapViewDesktopIndex)->MapBlitDC(0);
        /* itsMapBitmap = */ dcMem2.SelectObject(oldBitmap2);
        dcMem2.DeleteDC();
        // *** Tässä background kartan jälkihoito ***
        smartMetDocumentInterface->MapViewDescTop(mapViewDesktopIndex)->CopyCDC(0);
        dcMemCopy.DeleteDC();

        theView->DrawOverBitmapThings(theView->ToolBox());
        dcMem.SelectObject(oldBitmap);
        dcMem.DeleteDC();
    }

    template<class Tview>
    void DoOffScreenDrawForNonMapView(Tview *theView, SmartMetDocumentInterface *smartMetDocumentInterface, CBitmap &theDrawedScreenBitmapOut)
    {
        if(theView == nullptr)
            return;

        CClientDC dc(theView);
        CDC dcMem;
        dcMem.CreateCompatibleDC(&dc);

        CRect clientArea;
        theView->GetClientRect(&clientArea);
        if(clientArea.Height() < 4)
        {
            dcMem.DeleteDC();
            return; // kun ruudun korkeus on tarpeeksi pieni, ohjelma voi kaatua ja turha näin pientä ruutua on edes piirtää
        }

        theDrawedScreenBitmapOut.DeleteObject();
        theDrawedScreenBitmapOut.CreateCompatibleBitmap(&dc, clientArea.Width(), clientArea.Height());

        CBitmap *oldBitmap = dcMem.SelectObject(&theDrawedScreenBitmapOut);
        theView->ToolBox()->SetDC(&dcMem);

        {
            theView->SetToolMastersDC(&dcMem);
            theView->DoDraw();
        }

        theView->DrawOverBitmapThings(theView->ToolBox());
        dcMem.SelectObject(oldBitmap);
        dcMem.DeleteDC();
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
        // pakko tehdä tälläinen viritys, koska view:sta luotu dc on eri kokoinen kuin käytetty bitmapin koko
        view->ToolBox()->SetClientRect(CRect(0, 0, pBitMap.bmWidth, pBitMap.bmHeight));

        // pitää tyhjentää bitmappi piirtämällä valkoinen rect fillillä
        NFmiDrawingEnvironment envi;
        envi.EnableFill();
        envi.EnableFrame();
        envi.SetFillColor(NFmiColor(1, 1, 1));
        envi.SetFrameColor(NFmiColor(0.8f, 0.8f, 0.8f)); // piirretään vaalean harmaalla reunus
        NFmiRect bitmapRect(0, 0, 1, 1);
        NFmiRectangle drawRect(bitmapRect, 0, &envi);
        view->ToolBox()->Convert(&drawRect);

        NFmiRect drawingrect = bitmapRect;

        drawingrect.Inflate(-0.23); // en tiedä miksi, mutta tällälailla pitää piirto laatikkoa pienentää, muuten synop-plotti leviää piirrettäessä bitmappiin
        view->EditMapView()->DrawSynopPlotOnToolTip(view->ToolBox(), drawingrect, fDrawSoundingPlot, fDrawMinMaxPlot, fDrawMetarPlot);
        view->SynopPlotBitmap(dcMem.SelectObject(oldBitmap));
        dcMem.DeleteDC();
    }

    template<class Tview>
    void NotifyDisplayTooltip(Tview *view, NMHDR * pNMHDR, LRESULT * result, bool printingOn_DontSetDcs, int tooltipId)
    {
        if(printingOn_DontSetDcs)
            return; // pitää estää erikseen apukarttanäytöissä kaikenlaiset CDC-asetuksen kesken printtauksen!
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
                strU_ = CA2T(view->EditMapView()->ComposeToolTipText(relativePoint).c_str());
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
            { // jos löytyi, laitetaan imagelist tagit kuntoon stringiin
                std::string imgListStr = "<ilst idres=0 mask>";
                tmpStr.replace(pos, foundPlotStr.size(), imgListStr);
                strU_ = CA2T(tmpStr.c_str());
                DrawSynopPlotImage(view, drawSoundingPlot, drawMinMaxPlot, drawMetarPlot); // tehdään haluttu synop-plotti bitmappiin ja laiteaan se tooltip-systeemiin
                BITMAP pBitMap;
                view->SynopPlotBitmap()->GetBitmap(&pBitMap);
                view->ToolTipControl()->SetImageList(view->SynopPlotBitmapHandle(), pBitMap.bmWidth, pBitMap.bmHeight, 1);
            }
            pNotify->ti->sTooltip = strU_;
        }
    }

};


