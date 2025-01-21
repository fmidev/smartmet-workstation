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
	NFmiTimeSerialSymbolView(int theMapViewDescTopIndex, const NFmiRect & theRect
						    ,NFmiToolBox * theToolBox
						    ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
						    ,int theIndex);
	virtual ~NFmiTimeSerialSymbolView();

	bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override {return false;}
	bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override {return false;}
	void DrawLocationInTime(const NFmiPoint &theLatLonPoint, NFmiDrawingEnvironment& theCurrentDataLineStyle, NFmiDrawingEnvironment& theModifiedDataLineStyle, bool drawModificationLines) override;

protected:
	virtual void DrawSymbol(const NFmiPoint &theLatLonPoint, NFmiMetTime& time, NFmiRect& rect, NFmiTimePerioid& step);
	virtual void DrawSimpleSymbol(const NFmiPoint &theLatLonPoint, NFmiMetTime& time, NFmiRect& rect);
	NFmiRect CalcSymbolRect(NFmiMetTime& time);
	void DrawValueAxis(void) override;
	virtual void DrawValueGrids(NFmiDrawingEnvironment & envi, double minPos, double maxPos){return;}
	virtual void DrawGrids (NFmiDrawingEnvironment & envi){return;}
	void CreateValueScale(void) override;
	void DrawModifyFactorAxis(void) override;
	void DrawModifyingUnit(void) override;
	void DrawModifyFactorPoints(void) override {return;}
	bool AutoAdjustValueScale(void){return false;};

};

#endif // !defined(AFX_NFMITIMESERIALSYMBOLVIEW_H__3F914351_7568_11D3_9B33_00105AF41F22__INCLUDED_)
