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
// Changed 1999.09.10/Marko T‰st‰ l‰htien haluttu n‰ytett‰v‰ aika annetaan ulkoa p‰in.
// Changed 1999.09.28/Marko Muutin DrawFrame()-metodin public:iksi.
// Changed 1999.09.28/Marko Lis‰sin uuden Draw-metodin, mik‰ piirt‰‰ vain annetun rect:in alueen.
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

   NFmiCtrlView (void); // aiheuttaa ongelmia tiettyjen operaatioiden kanssa, pit‰isi poistaa?? Esim vertailuoperaattorit kaatuvat varmasti !!
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
   inline virtual bool LeftDoubleClick(const NFmiPoint &, unsigned long){ return false; } ; // Marko lis‰si 14.12.2001
   inline virtual bool RightDoubleClick(const NFmiPoint &, unsigned long){ return false; };
   inline virtual bool IsMouseDraggingOn(void){ return false; };
   boost::shared_ptr<NFmiDrawParam> DrawParam(void){return itsDrawParam;};
   virtual boost::shared_ptr<NFmiArea> GetArea() const { return nullptr; };
   virtual void SetArea(const boost::shared_ptr<NFmiArea>& theArea) { /* Ei tehd‰ emoluokassa mit‰‰n */ };

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
   int CalcRealRowIndex() const;
   const std::string& CachedParameterName(bool tooltipVersion) const;
   void CachedParameterName(const std::string& newName, bool tooltipVersion);
   virtual void UpdateCachedParameterName();
   std::string MakePossibleVirtualTimeTooltipText();

   virtual NFmiPoint LatLonToViewPoint(const NFmiPoint & /* theLatLon */ ) const {return NFmiPoint(kFloatMissing, kFloatMissing);}
   virtual NFmiPoint ViewPointToLatLon(const NFmiPoint & /* theViewPoint */ ) const {return NFmiPoint(kFloatMissing, kFloatMissing);}
   inline virtual bool IsSingleSymbolView(void) { return true; }; // T‰t‰ k‰ytet‰‰n, kun arvioidaan kartalle piirrett‰v‰n tekstin pituutta (liittyy hiladatan piirron harvennukseen). Eli jos true, 'tekstin' pituus on aina 1, muuten se lasketaan lennossa
   inline virtual bool IsTimeSerialView(void) { return false; };
   static const float gGreyColorBaseComponent; // mill‰ kertoimella rakennetaan gGreyColorDefault
   static const NFmiColor gGreyColorDefault; // Mik‰ on Windowssin perus harmaa v‰ris‰vy
   CtrlViewDocumentInterface* GetCtrlViewDocumentInterface();
   Gdiplus::Graphics* GdiPlusGraphics() { return itsGdiPlusGraphics; }
   // T‰t‰ k‰ytt‰‰ mm. InitializeGdiplus, mutta piti tiettyj‰ tapauksia varten tehd‰ irrallinen funktio, jolla luonti voidaan tehd‰ mista tahansa toolbox oliosta.
   static Gdiplus::Graphics* CreateGdiplusGraphics(NFmiToolBox *theToolBox, const NFmiRect *theRelativeClipRect);
   static NFmiRect CalcWantedDirectionalPosition(const NFmiRect& positionalRect, const NFmiRect& movedRect, FmiDirection wantedPosition);
   static std::string DoBoldingParameterNameTooltipText(std::string parameterStr);
   static bool IsNewDataParameterName(const std::string &parameterStr);
   static std::string AddColorTagsToString(const std::string& str, const NFmiColor& color, bool addBoldTags);
   virtual std::string MakeCsvDataString();

 protected:
	NFmiRect CalcMaskRectSize(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	void InitializeGdiplus(NFmiToolBox *theToolBox, const NFmiRect *theRelativeClipRect);
	void CleanGdiplus(void);
	int CalcRealRowIndex(int theRowIndex, int theColumnIndex) const;
    std::unique_ptr<MapHandlerInterface> GetMapHandlerInterface();

	NFmiRect itsRect;
	NFmiToolBox * itsToolBox;
	NFmiDrawingEnvironment * itsDrawingEnvironment;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
	boost::shared_ptr<NFmiFastQueryInfo> itsInfo;
    // ei omista, ei tuhoa
    CtrlViewDocumentInterface* itsCtrlViewDocumentInterface;
    // ctrl-view:iss‰ on tieto mink‰ desctopin alla se on. T‰m‰ ei koske muita kuin karttan‰ytˆll‰ olevia viewt‰, muilla t‰m‰ indeksi on -1 joka on puuttuva arvo
	int itsMapViewDescTopIndex; 
    // E.g. in map view there might be sub view grid with 3 rows and 4 columns
    // This is view grid's row number (starts from 1)
    int itsViewGridRowNumber; 
    // This is view grid's column number (starts from 1)
    int itsViewGridColumnNumber;
    // If there is list of ctrl-views drawn over each others (like on map view theere is different parameters on same row), this might hold that layer order number (starts from 1)
    int itsViewRowLayerNumber = -1;

	// aikaa tarvitaan joissain n‰yttˆluokissa, joten laitoin ajan asetuksen mukaan
	NFmiMetTime itsTime;	// 1999.09.10/Marko T‰m‰n ajan mukaan asetetaan info (siis jos tarvitsee).
	// Parametrin nimi stringin teko on yll‰tt‰v‰n raskasta ja se kannattaa laskea kullakin piirtokerralla kerran muistiin.
	// Parametrin nimi riippu paljon erilaisista datatyypeista ja tilanteista, se on mm. seuraavaa 'normi' muotoa: E00/06.09 CAPE
	std::string itsCachedParameterName;
	// Tehd‰‰n parametrin nimi pareina, toinen n‰ytˆn piirtoa varten ja toinen tooltippej‰ varten. Se miss‰ on tooltip versio, hanskaa
	// erikoismerkit kuten '>' jne. edelleen, joilla on erikoismerkitys html kieless‰.
	std::string itsCachedParameterNameForTooltip;

	Gdiplus::Graphics *itsGdiPlusGraphics; // tehd‰‰n GDI+ piirto t‰ll‰
private:
    void SetupCtrlViewDocumentInterface(const std::string &callerFunctionName);
};

