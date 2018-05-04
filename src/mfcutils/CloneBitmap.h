#pragma once

// CloneBitmap.h: interface for the CCloneBitmap class.
//
//////////////////////////////////////////////////////////////////////

class ColorPOD
{
public:
	ColorPOD(BYTE ar = 255, BYTE ag = 255, BYTE ab = 255)
		:r(ar)
		,g(ag)
		,b(ab)
	{}

	BYTE r;
	BYTE g;
	BYTE b;
};

#define IRGB(r,g,b)			((COLORREF)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))
#define INVERSECOLOR(x)		((COLORREF)(x&0xFF00)|((x&0xFF)<<16)|((x&0xFF0000)>>16))

class CCloneBitmap : public CBitmap  
{
public:
	COLORREF *m_lpData;
	HICON MakeIcon(COLORREF clrTransparent);
	int ChangeColor(COLORREF clrOld, COLORREF clrNew);
	int Clone(HBITMAP hSrc);
	CCloneBitmap();
	virtual ~CCloneBitmap();
	static HICON BitmapToIcon(UINT bitmapId, const ColorPOD &transParentColor = ColorPOD(255, 255, 255));
};

