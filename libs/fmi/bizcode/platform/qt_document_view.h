#pragma once
// Hosts SmartMet's real map view (NFmiEditMapView) on a QPainter.
//
// This is the Linux counterpart of what CSmartMetView does on Windows:
//
//     itsToolBox     = new NFmiToolBox(this);                      // CDC backed
//     itsEditMapView = new NFmiEditMapView(descTop, toolBox, drawParam);
//     ...
//     mapView->SetToolsDCs(&dcMem.getDc());
//     itsEditMapView->Draw(itsToolBox);
//
// On Linux NFmiToolBox draws through the Gdiplus::Graphics stub, which wraps a
// QPainter, so the same view object can render straight onto a QImage.
//
// NFmiEditMapGeneralDataDoc::Init() must have run first: the views reach the document
// through CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation, which
// the document installs during Init.

#ifdef UNIX
#ifdef HAVE_QT6

#include <memory>
#include <string>

class NFmiDrawParam;
class NFmiEditMapView;
class NFmiToolBox;
class QPainter;

class DocumentMapView
{
public:
    DocumentMapView();
    ~DocumentMapView();

    // Builds the toolbox and the real map view. Returns false and stays unusable if
    // anything throws, so the caller can fall back to the standalone renderer.
    bool create(int theMapViewDescTopIndex);
    bool isReady() const { return itsEditMapView != nullptr; }

    void setViewSize(int theWidthInPixels, int theHeightInPixels);

    // Draws the document's map view onto the painter. Returns false if the view threw,
    // in which case it is torn down so the fallback renderer takes over.
    bool draw(QPainter& thePainter);

    // Reason the last create()/draw() failed, for reporting to the user.
    const std::string& lastError() const { return itsLastError; }

private:
    void destroy();

    std::unique_ptr<NFmiToolBox> itsToolBox;
    std::unique_ptr<NFmiEditMapView> itsEditMapView;
    std::shared_ptr<NFmiDrawParam> itsDrawParam;
    int itsMapViewDescTopIndex = 0;
    int itsWidth = 0;
    int itsHeight = 0;
    std::string itsLastError;
};

#endif // HAVE_QT6
#endif // UNIX
