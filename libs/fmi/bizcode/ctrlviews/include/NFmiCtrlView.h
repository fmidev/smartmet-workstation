//**********************************************************
// C++ Class Name : NFmiCtrlView
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiCtrlView.h
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 2
//  - GD View Type      : Class Diagram
//  - GD View Name      : Markon ehdotus
// ---------------------------------------------------
//  Author         : Persa
//  Creation Date  : 30.04.1997
//
//
//  Description:
//
//  Change Log:
//Ver. 2.05.1997/Persa
//Ver. 7.05.1997/Persa
//Ver. 11.10.1997/Persa LeftButton1Click changed to return updateStatus
//Ver. 09.03.1998/Marko RightButton1Click changed to return updateStatus
//Ver. 23.09.1998/Marko Added new button control functions.
//Ver. 23.09.1998/Marko Added the cpp file.
//Ver  16.04.1999/Persa Added fDirty member and Dirty methods
// Changed 1999.09.10/Marko Tästä lähtien haluttu näytettävä aika annetaan ulkoa päin.
// Changed 1999.09.28/Marko Muutin DrawFrame()-metodin public:iksi.
// Changed 1999.09.28/Marko Lisäsin uuden Draw-metodin, mikä piirtää vain annetun rect:in alueen.
//
//**********************************************************

#pragma once

#include "NFmiRect.h"
#include "NFmiMetTime.h"
#include "NFmiColor.h"
#include "boost\shared_ptr.hpp"

class NFmiToolBox;
class NFmiDrawingEnvironment;
class NFmiDrawParam;
class NFmiDataParam;
class CtrlViewDocumentInterface;
class NFmiDataIdent;
class NFmiLevel;
class NFmiFastQueryInfo;
class MapHandlerInterface;
class NFmiArea;

namespace Gdiplus
{
	class Graphics;
}

class NFmiCtrlView
{
 public:
	enum ViewType
	{
		kAreaFilterView = 1,
		kTimeFilterView = 2,
		kGridViewSelectorView = 3,
		kTimeControlView = 4, 
		kModelCombineStatusView = 5,
		kMovingCPLocationView = 6, 
		kModelProducerIndexViewWCTR = 9
	};

   NFmiCtrlView (void); // aiheuttaa ongelmia tiettyjen operaatioiden kanssa, pitäisi poistaa?? Esim vertailuoperaattorit kaatuvat varmasti !!
   NFmiCtrlView (int theMapViewDescTopIndex, const NFmiRect & theRect
				,NFmiToolBox * theToolBox
				,NFmiDrawingEnvironment * theDrawingEnvi
				,boost::shared_ptr<NFmiDrawParam> &theDrawParam
                ,int viewGridRowNumber = -1
                ,int viewGridColumnNumber = -1);
   NFmiCtrlView (int theMapViewDescTopIndex, const NFmiRect & theRect
			    ,NFmiToolBox * theToolBox
				,NFmiDrawingEnvironment * theDrawingEnvi
                ,int viewGridRowNumber = -1
                ,int viewGridColumnNumber = -1);
   NFmiCtrlView (const NFmiCtrlView& theView);
   inline virtual  ~NFmiCtrlView (void){};
   inline const NFmiRect& GetFrame(void) const { return itsRect; };
   inline virtual const NFmiRect& GetFrameForParamBox(void) const { return itsRect; } ;
   inline void SetFrame (const NFmiRect &theRect) {itsRect = theRect; } ;
   virtual void Update (const NFmiRect & theRect, NFmiToolBox * theToolBox = 0);
   virtual void Update (void){};
   inline virtual void Draw (NFmiToolBox *){ } ;
   inline bool IsIn (const NFmiPoint thePoint) const{ return itsRect.IsInside(thePoint) == true; } ;
   inline virtual bool LeftButtonDown(const NFmiPoint &, unsigned long){ return false; } ;
   inline virtual bool LeftButtonUp(const NFmiPoint &, unsigned long){ return false; } ;
   inline virtual bool MiddleButtonDown(const NFmiPoint &, unsigned long){ return false; } ;
   inline virtual bool MiddleButtonUp(const NFmiPoint &, unsigned long){ return false; } ;
   inline virtual bool RightButtonDown(const NFmiPoint &, unsigned long){ return false; } ;
   inline virtual bool RightButtonUp(const NFmiPoint &, unsigned long){ return false; } ;
   inline virtual bool MouseMove(const NFmiPoint &, unsigned long){ return false; } ;
   inline virtual bool MouseWheel(const NFmiPoint &, unsigned long, short ){ return false; } ;
   inline virtual bool LeftDoubleClick(const NFmiPoint &, unsigned long){ return false; } ; // Marko lisäsi 14.12.2001
   inline virtual bool RightDoubleClick(const NFmiPoint &, unsigned long){ return false; };
   inline virtual bool IsMouseDraggingOn(void){ return false; };
   boost::shared_ptr<NFmiDrawParam> DrawParam(void){return itsDrawParam;};
   virtual boost::shared_ptr<NFmiArea> GetArea() const { return nullptr; };
   virtual void SetArea(const boost::shared_ptr<NFmiArea>& theArea) { /* Ei tehdä emoluokassa mitään */ };

   bool operator==(const NFmiCtrlView& theCtrlView) const;
   bool operator< (const NFmiCtrlView& theCtrlView) const;

   void SetToolBox(NFmiToolBox* theToolBox) { itsToolBox = theToolBox; };
   NFmiToolBox* GetToolBox() {return itsToolBox;}
   virtual const NFmiMetTime& Time(void){return itsTime;};
   virtual void Time(const NFmiMetTime& theTime){itsTime = theTime;};
   void DrawFrame(NFmiDrawingEnvironment * theEnvi);
   void DrawFrame(NFmiDrawingEnvironment * theEnvi, const NFmiRect& theFrame);
   NFmiDrawingEnvironment* GetDrawingEnvironment() { return itsDrawingEnvironment; }

   virtual std::string ComposeToolTipText(const NFmiPoint& /* theRelativePoint */ ){return std::string();};
   int MapViewDescTopIndex(void) const {return itsMapViewDescTopIndex;}
   void MapViewDescTopIndex(int newValue) {itsMapViewDescTopIndex = newValue;}
   int ViewGridRowNumber() const { return itsViewGridRowNumber; }
   void ViewGridRowNumber(int rowNumber) { itsViewGridRowNumber = rowNumber; }
   int ViewGridColumnNumber() const { return itsViewGridColumnNumber; }
   void ViewGridColumnNumber(int columnNumber) { itsViewGridColumnNumber = columnNumber; }
   int ViewRowLayerNumber() const { return itsViewRowLayerNumber; }
   void ViewRowLayerNumber(int viewRowLayerNumber) { itsViewRowLayerNumber = viewRowLayerNumber; }
   int GetUsedParamRowIndex(int theRowIndex, int theColumnIndex) const;
   int GetUsedParamRowIndex() const;
   bool IsPrintedMapViewDesctop();
   bool IsPrinting() const;

   virtual NFmiPoint LatLonToViewPoint(const NFmiPoint & /* theLatLon */ ){return NFmiPoint(kFloatMissing, kFloatMissing);}
   virtual NFmiPoint ViewPointToLatLon(const NFmiPoint & /* theViewPoint */ ){return NFmiPoint(kFloatMissing, kFloatMissing);}
   inline virtual bool IsSingleSymbolView(void) { return true; }; // Tätä käytetään, kun arvioidaan kartalle piirrettävän tekstin pituutta (liittyy hiladatan piirron harvennukseen). Eli jos true, 'tekstin' pituus on aina 1, muuten se lasketaan lennossa
   inline virtual bool IsTimeSerialView(void) { return false; };
   static const float gGreyColorBaseComponent; // millä kertoimella rakennetaan gGreyColorDefault
   static const NFmiColor gGreyColorDefault; // Mikä on Windowssin perus harmaa värisävy
   CtrlViewDocumentInterface* GetCtrlViewDocumentInterface();
   Gdiplus::Graphics* GdiPlusGraphics() { return itsGdiPlusGraphics; }
   // Tätä käyttää mm. InitializeGdiplus, mutta piti tiettyjä tapauksia varten tehdä irrallinen funktio, jolla luonti voidaan tehdä mista tahansa toolbox oliosta.
   static Gdiplus::Graphics* CreateGdiplusGraphics(NFmiToolBox *theToolBox, const NFmiRect *theRelativeClipRect);
   static NFmiRect CalcWantedDirectionalPosition(const NFmiRect& positionalRect, const NFmiRect& movedRect, FmiDirection wantedPosition);

	// suoran kaksi pistettä on annettu ja x:n arvo, laske y:n arvo
	static double InterpolateWithTwoPoints(double x, double x1, double x2, double y1, double y2, double minY = kFloatMissing, double maxY = kFloatMissing)
	{
		double k = (y2-y1)/(x2-x1);
		double b = (x1*y2 - y1*x2)/(x1-x2);
		double y = k*x + b;
		if(minY != kFloatMissing)
			y = FmiMax(y, minY);
		if(maxY != kFloatMissing)
			y = FmiMin(y, maxY);
		return y;
	}

 protected:
	NFmiRect CalcMaskRectSize(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	void InitializeGdiplus(NFmiToolBox *theToolBox, const NFmiRect *theRelativeClipRect);
	void CleanGdiplus(void);
	int CalcRealRowIndex(int theRowIndex, int theColumnIndex);
    std::unique_ptr<MapHandlerInterface> GetMapHandlerInterface();

	NFmiRect itsRect;
	NFmiToolBox * itsToolBox;
	NFmiDrawingEnvironment * itsDrawingEnvironment;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
	boost::shared_ptr<NFmiFastQueryInfo> itsInfo;
    // ei omista, ei tuhoa
    CtrlViewDocumentInterface* itsCtrlViewDocumentInterface;
    // ctrl-view:issä on tieto minkä desctopin alla se on. Tämä ei koske muita kuin karttanäytöllä olevia viewtä, muilla tämä indeksi on -1 joka on puuttuva arvo
	int itsMapViewDescTopIndex; 
    // E.g. in map view there might be sub view grid with 3 rows and 4 columns
    // This is view grid's row number (starts from 1)
    int itsViewGridRowNumber; 
    // This is view grid's column number (starts from 1)
    int itsViewGridColumnNumber;
    // If there is list of ctrl-views drawn over each others (like on map view theere is different parameters on same row), this might hold that layer order number (starts from 1)
    int itsViewRowLayerNumber = -1;

	// aikaa tarvitaan joissain näyttöluokissa, joten laitoin ajan asetuksen mukaan
	NFmiMetTime itsTime;	// 1999.09.10/Marko Tämän ajan mukaan asetetaan info (siis jos tarvitsee).

	Gdiplus::Graphics *itsGdiPlusGraphics; // tehdään GDI+ piirto tällä
private:
    void SetupCtrlViewDocumentInterface(const std::string &callerFunctionName);
};

