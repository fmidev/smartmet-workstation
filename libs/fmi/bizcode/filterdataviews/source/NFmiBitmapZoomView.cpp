// NFmiBitmapZoomView.cpp: implementation for the NFmiBitmapZoomView class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k��nt�j�n varoitusta (liian pitk� nimi >255 merkki� joka johtuu 'puretuista' STL-template nimist�)
#endif

#include "NFmiBitmapZoomView.h"
#include "NFmiToolBox.h"
#include "CtrlViewDocumentInterface.h"
#include "MapHandlerInterface.h"
#include "CtrlViewGdiPlusFunctions.h"

void NFmiBitmapZoomView::Draw(NFmiToolBox * theGTB)
{
#ifndef UNIX
	if(theGTB)
	{
        auto mapHandlerInterface = GetMapHandlerInterface();
        Gdiplus::Bitmap* aBitmap = mapHandlerInterface->GetBitmap();
		if(aBitmap)
		{
			NFmiRect bitmapRect = mapHandlerInterface->TotalAbsolutRect();
			CRect mfcRect;
			itsToolBox->UpdateClientRect(); // poista t�m� kun voit
			itsToolBox->ConvertRect(GetFrame(), mfcRect);
			Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(mfcRect.left), static_cast<Gdiplus::REAL>(mfcRect.top), static_cast<Gdiplus::REAL>(mfcRect.right), static_cast<Gdiplus::REAL>(mfcRect.bottom));
			CtrlView::DrawBitmapToDC_4(theGTB->GetDC(), *aBitmap, bitmapRect, destRect, true);
		}
	}
#endif
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

