//**********************************************************
// C++ Class Name : NFmiCrossSectionManagerView 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: NFmiCrossSectionManagerView.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 3 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : uusi aikasarja viritys 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thur - Mar 25, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// 
//**********************************************************

#pragma once

#include "NFmiCtrlView.h"

class NFmiCtrlViewList;
class NFmiToolBox;
class NFmiDrawingEnvironment;
class NFmiTimeBag;
class NFmiTimeControlView;
class NFmiCrossSectionView;
class NFmiCrossSectionSystem;


class NFmiCrossSectionManagerView : public NFmiCtrlView
{

 public:
	NFmiCrossSectionManagerView(const NFmiRect & theRect
								,NFmiToolBox * theToolBox);
	~NFmiCrossSectionManagerView(void);
	void Update(void) override;
	void Draw(NFmiToolBox* theToolBox) override;
	bool LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey) override;
	bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
    bool MouseMove(const NFmiPoint& thePlace, unsigned long theKey) override;
    bool MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
	bool LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) override;
    bool IsMouseDraggingOn(void) override;
    void DrawOverBitmapThings(NFmiToolBox *theGTB, const NFmiPoint &thePlace); // tooltip feikki viritys
    void UpdateListViewFrames(void);
	void UpdateSize(void);
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
    void CalcDrawSizeFactors(void);

 private:
	void Destroy(void);
	void DestroyViewList(void);
    void CreateViewList(void);
	void DrawData(void);
	void DrawBase(void);
	void DrawBackground(void);
	double CalcDataRectPressureScaleRatio(void);
	NFmiRect CalcPressureScaleRect(void);

	NFmiRect CalcFooterRect(void); // Laskettaessa eri näyttöalueita, tätä pitää kutsua ensin
	NFmiRect CalcHeaderRect(void);
	NFmiRect CalcViewListRect(void); // tätä toiseksi
	NFmiRect CalcListViewRect(int theIndex); // näitä voi kutsua sitten 
	NFmiRect CalcTimeControlViewRect(void); // näitä voi kutsua sitten 
	void DrawActivatedMinorPoint(void);
	NFmiPoint GetStartLatLonPoint(void);
	NFmiPoint GetEndLatLonPoint(void);
	NFmiPoint GetMiddleLatLonPoint(void);

	bool CreateTimeControlView(void);
	void DrawFooter(void);
	void DrawHeader(void);
	void ActivateCrossSectionView(const NFmiPoint& thePlace);

	NFmiRect itsHeaderRect; // tälle alueelle piirretään poikkileikkaus datasta mm. aika, mahd. paikka, v- ja h-resoluutiot
	NFmiRect itsFooterRect; // tälle alueelle piirretään poikkileikkaus datasta mm. mahd. aikakontrolli-näyttö tai pääpisteiden koordinaatit
	NFmiRect itsViewListRect;
	NFmiCtrlViewList *itsViewList;

	NFmiTimeControlView * itsTimeControlView; // jos aikapoikkileikkaus moodi päällä, piirretään tämä ruudun ala laitaan
	NFmiRect itsTimeControlViewRect;

	NFmiCrossSectionSystem *itsCrossSectionSystem; // ei omista, ei tuhoa!

	// Teen seuraavanlaisen systeemin että printtauksessa tulee sopivan kokoisia fontteja ja viiva paksuuksia:
	// Otan talteen viimeisen ruudulla piirrettyjen pikselien koot mm.
	// Kun ollaan printtaamassa kuvaa, lasketaan kerroin, millä ruutu piirrossa olevat pikseli määrät on kerrottava
	// että saadaan saman kokoisia piirto-olioita myös paperille.
	double itsDrawSizeFactorX; // ruutu piirrossa 1, lasketaan printatessa tälle erillinen arvo
	double itsDrawSizeFactorY;
	double itsLastScreenDrawPixelSizeInMM_x;
	double itsLastScreenDrawPixelSizeInMM_y;
	double itsLastScreenDataRectPressureScaleRatio; // tämän avulla yritetään vielä korjata koko laskuja, koska näytönohjaimet eivät anna aina 
												// oikeita millimetri kokoja näytöille. Jos 0, ei ole tietoa suhteesta, eikä korjausta voi tehdä.
};

