//**********************************************************
// C++ Class Name : NFmiViewParamsView
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiViewParamsView.h
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
//  Author         : pietarin
//  Creation Date  : Thur - Jan 28, 1999
//
//
//  Description:
//
//  Change Log:
//
//**********************************************************

#pragma once

#include "NFmiParamCommandView.h"

class NFmiDrawParamList;

namespace Gdiplus
{
	class Bitmap;
}

class NFmiViewParamsView : public NFmiParamCommandView
{
 public:
	// Tähän luokkaan luetaan ksyeiset kuva-imaget kerran ja käytetään kaikille NFmiTimeControlView-instansseille yhteisesti.
	// Gdiplus::Bitmap-olioita ei tarvitse tuhota, Gdiplus huolehtii siitä automaattisesti.
	class ModelSelectorButtonImageHolder
	{
	public:
		ModelSelectorButtonImageHolder(void)
		:itsBitmapFolder()
		,itsNextModelButtonImage(0)
		,itsPreviousModelButtonImage(0)
		,itsFindNearestModelButtonImage(0)
		,fInitialized(false)
		{
		}

		// initialisoinnissa luetaan bitmapit tiedostoista, kutsu vasta kun itsBitmapFolder-dataosa on asetettu
		void Initialize(void);  // HUOM! heittää poikkeuksia epäonnistuessaan

		std::string itsBitmapFolder;
		Gdiplus::Bitmap *itsNextModelButtonImage;
		Gdiplus::Bitmap *itsPreviousModelButtonImage;
		Gdiplus::Bitmap *itsFindNearestModelButtonImage;
		bool fInitialized;
	};


	NFmiViewParamsView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, NFmiDrawingEnvironment * theDrawingEnvi, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex, bool hasMapLayer);
    bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
    bool LeftButtonUp(const NFmiPoint &, unsigned long);
	bool RightButtonUp(const NFmiPoint &, unsigned long);
    bool MouseMove(const NFmiPoint& thePlace, unsigned long theKey);
    bool LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey); // Marko lisäsi 3.4.2002
    bool IsMouseDraggingOn(void);
	NFmiRect CalcSize(void); // koko saattaa muuttua, ja uutta kokoa pitää voida kysyä oliolta
	bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta);
    bool IsMouseCaptured(void) { return fMouseCaptured; }

 protected:
    int GetParamCount(void);
	bool LeftClickOnModelSelectionButtons(const NFmiPoint &thePlace, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex);
	static ModelSelectorButtonImageHolder statModelSelectorButtonImages; // tämä on staattinen dataosa, koska näitä timekontrol-instansseja luodaan lennossa jatkuvasti uudelleen ja uudelleen eli bitmapit luetaan vain kerran kaikkien käyttöön
	void DrawData(void);
	void DrawParamCheckBox(int lineIndex, boost::shared_ptr<NFmiDrawParam> &theDrawParam);
	void DrawModelSelectorButtons(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex);
	NFmiPoint CalcModelSelectorButtonRelativeSize(Gdiplus::Bitmap *theImage);
	NFmiRect CalcModelSelectorButtonRect(int theRowIndex, int theButtonIndex);
	NFmiPoint GetViewSizeInPixels(void);
	NFmiPoint CalcModelSelectorButtonRelativeEdgeOffset(const NFmiPoint &theButtonRelaviteSize);
    bool DoAfterParamModeModifications(NFmiDrawParamList *theParamList);
    bool ActivateParam(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theParamIndex);
    void DrawActiveParamMarkers(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theParamLineIndex);
    NFmiRect CalcParamRect(int theParamLineIndex, bool fExcludeCheckBox, int leftMargin, int topMargin, int rightMargin, int bottomMargin);
    NFmiDrawParamList* GetDrawParamList();
    void DrawMouseDraggingAction();
    void DrawMouseDraggingBackground();
	void DrawBackgroundMapLayerText();
private:
   double itsButtonSizeInMM_x; // paino nappuloiden koko millimetreissä x-suunnassa
   double itsButtonSizeInMM_y; // paino nappuloiden koko millimetreissä y-suunnassa
   double itsButtonOffSetFromEdgeFactor; // kuinka kaukana paino nappula on reunoista irti suhteessa napulan kokoon

   // Hiirellä raahaamis piirtojärjestyksen vaihtoon liittyvät muuttujat
   bool fMouseCaptured; // Tämä asetetaan päälle heti LeftButtonDown:issa
   int itsCapturedParamRowIndex; // Mikä parametri rivi on kaapattu raahattavaksi (LeftButtonDown), -1 on reset tila
   int itsCurrentDragRowIndex; // Mihin riville ollaan kaapattua parametria raahaamassa tällä hetkellä (MouseMove:ssa), -1 on reset tila
   // Jos hiirtä on siirretty vasen nappi pohjassa tarpeeksi, mennään dragging moodiin, jolloin LeftButtonUp osaa toimia erilailla kuin normaalisti,
   // Mutta jos vasen painetaan pohjaan ja se liikkuu hieman ja päästetään taas ylös, tällöin halutaan normaali toiminta kuten esim. show-param on/off tms. toimintoja.
   // Tämä myös pakottaa näytön piirron MouseMove:ssa, jos ollaan menty drag-moodiin.
   bool fMouseDraggingAction; 
};

