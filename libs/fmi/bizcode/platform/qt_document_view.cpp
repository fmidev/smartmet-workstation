#ifdef UNIX
#ifdef HAVE_QT6

#include "qt_document_view.h"

#include "CtrlViewDocumentInterface.h"
#include "MapHandlerInterface.h"
#include "NFmiDrawParam.h"
#include "NFmiEditMapView.h"
#include "NFmiToolBox.h"
#include "catlog/catlog.h"
#include "gdiplus_stub.h"

#include <QPainter>

#include <string>

DocumentMapView::DocumentMapView() = default;

DocumentMapView::~DocumentMapView()
{
    destroy();
}

void DocumentMapView::destroy()
{
    // The view holds the toolbox pointer, so it has to go first.
    itsEditMapView.reset();
    itsToolBox.reset();
}

bool DocumentMapView::create(int theMapViewDescTopIndex)
{
    destroy();
    itsMapViewDescTopIndex = theMapViewDescTopIndex;
    itsLastError.clear();

    // NFmiEditMapView's constructor dereferences the map handler straight away:
    //   NFmiCtrlView(index, GetCtrlViewDocumentInterface()->GetMapHandlerInterface(index)
    //                           ->Area()->XYArea(), ...)
    // On Linux CtrlViewDocumentInterfaceForGeneralDataDoc::GetMapHandlerInterface still
    // returns nullptr, because the map handler it wraps (NFmiGdiPlusImageMapHandler) is
    // one of the GdiPlus* sources excluded from the Linux build. Check before building
    // the view, otherwise this is a segfault rather than a diagnosable failure.
    auto* documentInterface = CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation
                                  ? CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()
                                  : nullptr;
    if(!documentInterface)
    {
        itsLastError = "no CtrlViewDocumentInterface - the document is not initialized";
    }
    else if(!documentInterface->GetMapHandlerInterface(theMapViewDescTopIndex))
    {
        itsLastError = "no map handler on Linux yet (NFmiGdiPlusImageMapHandler is not ported), "
                       "so the real map view cannot be created";
    }
    if(!itsLastError.empty())
    {
        CatLog::logMessage("Real map view unavailable: " + itsLastError,
                           CatLog::Severity::Warning, CatLog::Category::Visualization, true);
        return false;
    }

    try
    {
        const int width = itsWidth > 0 ? itsWidth : 800;
        const int height = itsHeight > 0 ? itsHeight : 600;

        itsToolBox = std::make_unique<NFmiToolBox>(width, height);
        // CSmartMetView uses a plain default draw param for the map view itself
        itsDrawParam = std::make_shared<NFmiDrawParam>();
        itsEditMapView = std::make_unique<NFmiEditMapView>(
            itsMapViewDescTopIndex, itsToolBox.get(), itsDrawParam);

        CatLog::logMessage("Real map view created for desc top " +
                               std::to_string(itsMapViewDescTopIndex),
                           CatLog::Severity::Info, CatLog::Category::Visualization, true);
        return true;
    }
    catch(std::exception& e)
    {
        itsLastError = e.what();
    }
    catch(...)
    {
        itsLastError = "unknown exception";
    }

    CatLog::logMessage("Could not create the real map view: " + itsLastError,
                       CatLog::Severity::Error, CatLog::Category::Visualization, true);
    destroy();
    return false;
}

void DocumentMapView::setViewSize(int theWidthInPixels, int theHeightInPixels)
{
    itsWidth = theWidthInPixels;
    itsHeight = theHeightInPixels;
    if(itsToolBox)
        itsToolBox->SetClientRect(NFmiRect(0, 0, itsWidth, itsHeight));
}

bool DocumentMapView::draw(QPainter& thePainter)
{
    if(!itsEditMapView || !itsToolBox)
        return false;

    try
    {
        // NFmiToolBox draws through the Gdiplus::Graphics stub, which is just a
        // QPainter holder on Linux.
        Gdiplus::Graphics graphics(&thePainter);
        itsToolBox->SetGraphics(&graphics);
        itsEditMapView->Draw(itsToolBox.get());
        itsToolBox->SetGraphics(nullptr);
        return true;
    }
    catch(std::exception& e)
    {
        itsLastError = e.what();
    }
    catch(...)
    {
        itsLastError = "unknown exception";
    }

    CatLog::logMessage("Real map view drawing failed, falling back to the standalone "
                       "renderer: " + itsLastError,
                       CatLog::Severity::Error, CatLog::Category::Visualization, true);
    destroy();
    return false;
}

#endif // HAVE_QT6
#endif // UNIX
