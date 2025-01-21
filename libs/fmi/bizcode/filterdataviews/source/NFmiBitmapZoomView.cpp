// NFmiBitmapZoomView.cpp: implementation for the NFmiBitmapZoomView class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255 merkkiä joka johtuu 'puretuista' STL-template nimistä)
#endif

#include "NFmiBitmapZoomView.h"
#include "NFmiToolBox.h"
#include "CtrlViewDocumentInterface.h"
#include "MapHandlerInterface.h"
#include "CtrlViewGdiPlusFunctions.h"

void NFmiBitmapZoomView::Draw(NFmiToolBox * theGTB)
{
	if(theGTB)
	{
        auto mapHandlerInterface = GetMapHandlerInterface();
        Gdiplus::Bitmap* aBitmap = mapHandlerInterface->GetBitmap();
		if(aBitmap)
		{
			NFmiRect bitmapRect = mapHandlerInterface->TotalAbsolutRect();
			CRect mfcRect;
			itsToolBox->UpdateClientRect(); // poista tämä kun voit
			itsToolBox->ConvertRect(GetFrame(), mfcRect);
			Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(mfcRect.left), static_cast<Gdiplus::REAL>(mfcRect.top), static_cast<Gdiplus::REAL>(mfcRect.right), static_cast<Gdiplus::REAL>(mfcRect.bottom));
			CtrlView::DrawBitmapToDC_4(theGTB->GetDC(), *aBitmap, bitmapRect, destRect, true);
		}
	}
	DrawConstAreaRects();
	DrawZoomedAreaRect();
}

NFmiBitmapZoomView::NFmiBitmapZoomView(NFmiToolBox * theToolBox
						 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam)
:NFmiZoomView(0, theToolBox, theDrawParam)
{
}

NFmiBitmapZoomView::~NFmiBitmapZoomView()
{
}

