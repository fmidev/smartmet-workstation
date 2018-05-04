// CloneBitmap.cpp: implementation of the CCloneBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CloneBitmap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCloneBitmap::CCloneBitmap()
{
	m_hObject=NULL;
	m_lpData=NULL;
}

CCloneBitmap::~CCloneBitmap()
{
}

int CCloneBitmap::Clone(HBITMAP hSrc)
{
	CBitmap bmpSrc;
	BITMAP bmp;
	if(!bmpSrc.Attach(hSrc))
		return 0;
	if(!bmpSrc.GetBitmap(&bmp))
		return 0;
	HGDIOBJ hOldSrc, hOldDest;
	if(m_hObject)
		DeleteObject();
	BITMAPINFOHEADER bmi;
	CDC dcSrc, dcDest, dcScr;
	HDC hdc;
	hdc=::GetDC(NULL);
	dcScr.Attach(hdc);
	dcDest.CreateCompatibleDC(&dcScr);
	dcSrc.CreateCompatibleDC(&dcScr);
	bmi.biBitCount=32;
	bmi.biClrImportant=0;
	bmi.biClrUsed=0;
	bmi.biCompression=BI_RGB;
	bmi.biHeight=bmp.bmHeight;
	bmi.biPlanes=1;
	bmi.biSize=sizeof(bmi);
	bmi.biSizeImage=0;
	bmi.biWidth=bmp.bmWidth;
	bmi.biXPelsPerMeter=0;
	bmi.biYPelsPerMeter=0;
	m_hObject=CreateDIBSection(dcScr, (LPBITMAPINFO)&bmi, 0, (void**)&m_lpData, NULL, DIB_RGB_COLORS);
	hOldDest=dcDest.SelectObject(*this);
	hOldSrc=dcSrc.SelectObject(bmpSrc);
	if(!dcDest.BitBlt(0, 0, bmp.bmWidth, bmp.bmHeight, &dcSrc, 0, 0, SRCCOPY))
		return 0;
	dcSrc.SelectObject(hOldSrc);
	dcDest.SelectObject(hOldDest);
	dcSrc.DeleteDC();
	dcDest.DeleteDC();
	dcScr.Detach();
	ReleaseDC(NULL, hdc);
	bmpSrc.Detach();
	return 1;
}

int CCloneBitmap::ChangeColor(COLORREF clrOld, COLORREF clrNew)
{
	if(m_lpData==NULL)
		return 0;
	BITMAP bmp;
	if(!GetBitmap(&bmp))
		return 0;
	if(bmp.bmHeight==0 || bmp.bmWidth==0)
		return 0;
	DWORD dwSize;
	dwSize=bmp.bmHeight*bmp.bmWidth;

	COLORREF *pData;
	pData=m_lpData;

	for(DWORD d=0; d<dwSize; d++)
	{
		if(*pData==clrOld)
			*pData=clrNew;
		pData++;
	}
	
	return 1;
}

HICON CCloneBitmap::MakeIcon(COLORREF clrTransparent)
{
	HICON hIcon;
	BITMAP bmp;
	DWORD dwSize;
	COLORREF *lpData;

	if(m_lpData==NULL)
		return NULL;
	if(!GetBitmap(&bmp))
		return NULL;
	if(bmp.bmHeight==0 || bmp.bmWidth==0)
		return NULL;

	HBITMAP hMask, hColor;
	LPCOLORREF lpColorData, lpMaskData;

	BITMAPINFOHEADER bmi;
	bmi.biBitCount=32;
	bmi.biClrImportant=0;
	bmi.biClrUsed=0;
	bmi.biCompression=BI_RGB;
	bmi.biHeight=32;
	bmi.biPlanes=1;
	bmi.biSize=sizeof(bmi);
	bmi.biSizeImage=0;
	bmi.biWidth=32;
	bmi.biXPelsPerMeter=0;
	bmi.biYPelsPerMeter=0;
	hColor=CreateDIBSection(NULL, (LPBITMAPINFO)&bmi, 0, (void**)&lpColorData, NULL, DIB_RGB_COLORS);
	if(hColor==NULL)
		return NULL;
	hMask=CreateDIBSection(NULL, (LPBITMAPINFO)&bmi, 0, (void**)&lpMaskData, NULL, DIB_RGB_COLORS);
	if(hMask==NULL)
	{
		::DeleteObject(hColor);
		return NULL;
	}

	dwSize=32*32;

	lpData=m_lpData;

	for(DWORD d=0; d<dwSize; d++)
	{
		if(*lpData==clrTransparent)
		{
			lpMaskData[d]=0xFFFFFF;
			lpColorData[d]=0;
		}
		else
		{
			lpMaskData[d]=0;
			lpColorData[d]=*lpData;
		}
		lpData++;
	}


	ICONINFO ii;
	ii.fIcon=1;
	ii.hbmColor=hColor;
	ii.hbmMask=hMask;
	ii.xHotspot=0;
	ii.yHotspot=0;
	hIcon=CreateIconIndirect(&ii);

	::DeleteObject(hMask);
	::DeleteObject(hColor);

	return hIcon;
}

HICON CCloneBitmap::BitmapToIcon(UINT bitmapId, const ColorPOD &transParentColor)
{
	HICON hIcon;
	HBITMAP hBmp = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(bitmapId));
	if(hBmp!=NULL)
	{
		CCloneBitmap bmpClone;
		bmpClone.Clone(hBmp);
		::DeleteObject(hBmp);
		hIcon = bmpClone.MakeIcon(IRGB(transParentColor.r, transParentColor.g, transParentColor.b)); // make icon
	}
	return hIcon;
}
