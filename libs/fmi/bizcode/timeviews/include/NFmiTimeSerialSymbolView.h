// NFmiTimeSerialSymbolView.h: interface for the NFmiTimeSerialSymbolView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NFMITIMESERIALSYMBOLVIEW_H__3F914351_7568_11D3_9B33_00105AF41F22__INCLUDED_)
#define AFX_NFMITIMESERIALSYMBOLVIEW_H__3F914351_7568_11D3_9B33_00105AF41F22__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "NFmiTimeSerialView.h"

class NFmiTimeSerialSymbolView : public NFmiTimeSerialView  
{
public:
	NFmiTimeSerialSymbolView(const NFmiRect & theRect
						    ,NFmiToolBox * theToolBox
						    ,NFmiDrawingEnvironment * theDrawingEnvi
						    ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
						    ,int theIndex
						    ,double theManualModifierLength);
	virtual ~NFmiTimeSerialSymbolView();

	virtual bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey){return false;}
	virtual bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey){return false;}
	virtual void DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle);

protected:
	virtual void DrawSymbol(const NFmiPoint &theLatLonPoint, NFmiMetTime& time, NFmiRect& rect, NFmiTimePerioid& step);
	virtual void DrawSimpleSymbol(const NFmiPoint &theLatLonPoint, NFmiMetTime& time, NFmiRect& rect);
	NFmiRect CalcSymbolRect(NFmiMetTime& time);
	virtual void DrawValueAxis(void);
	virtual void DrawValueGrids(NFmiDrawingEnvironment & envi, double minPos, double maxPos){return;}
	virtual void DrawGrids (NFmiDrawingEnvironment & envi){return;}
	virtual void CreateValueScale(void);
	virtual void DrawModifyFactorAxis(void);
	virtual void DrawModifyingUnit(void);
	virtual void DrawModifyFactorPoints(void){return;}
	bool AutoAdjustValueScale(void){return false;};

};

#endif // !defined(AFX_NFMITIMESERIALSYMBOLVIEW_H__3F914351_7568_11D3_9B33_00105AF41F22__INCLUDED_)
