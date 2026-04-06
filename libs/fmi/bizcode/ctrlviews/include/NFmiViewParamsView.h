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

#ifndef UNIX
namespace Gdiplus
{
	class Bitmap;
}
#endif

class NFmiViewParamsView : public NFmiParamCommandView
{
 public:
	// T�h�n luokkaan luetaan ksyeiset kuva-imaget kerran ja k�ytet��n kaikille NFmiTimeControlView-instansseille yhteisesti.
	// Gdiplus::Bitmap-olioita ei tarvitse tuhota, Gdiplus huolehtii siit� automaattisesti.
#ifndef UNIX
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
		void Initialize(void);  // HUOM! heitt�� poikkeuksia ep�onnistuessaan

		std::string itsBitmapFolder;
		Gdiplus::Bitmap *itsNextModelButtonImage;
		Gdiplus::Bitmap *itsPreviousModelButtonImage;
		Gdiplus::Bitmap *itsFindNearestModelButtonImage;
		bool fInitialized;
	};
#endif // UNIX


	NFmiViewParamsView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex, bool hasMapLayer);
    bool LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey) override;
    bool LeftButtonUp(const NFmiPoint &, unsigned long) override;
	bool RightButtonUp(const NFmiPoint &, unsigned long) override;
    bool MouseMove(const NFmiPoint& thePlace, unsigned long theKey) override;
    bool LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey) override; // Marko lis�si 3.4.2002
    bool IsMouseDraggingOn(void) override;
	NFmiRect CalcSize(void); // koko saattaa muuttua, ja uutta kokoa pit�� voida kysy� oliolta
	bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
    bool IsMouseCaptured(void) { return fMouseCaptured; }
	std::string ComposeToolTipText(const NFmiPoint& thePlace) override;

 protected:
    int GetParamCount(void);
	bool LeftClickOnModelSelectionButtons(const NFmiPoint &thePlace, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theParameterRowIndex);
#ifndef UNIX
	static ModelSelectorButtonImageHolder statModelSelectorButtonImages; // t�m� on staattinen dataosa, koska n�it� timekontrol-instansseja luodaan lennossa jatkuvasti uudelleen ja uudelleen eli bitmapit luetaan vain kerran kaikkien k�ytt��n
#endif // UNIX
	void DrawData(void) override;
	void DrawModelSelectorButtons(boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiRect& parameterRowRect);
#ifndef UNIX
	NFmiPoint CalcModelSelectorButtonRelativeSize(Gdiplus::Bitmap *theImage);
	NFmiRect CalcModelSelectorButtonRect(const NFmiRect& parameterRowRect, int theButtonIndex);
#endif // UNIX
	NFmiPoint GetViewSizeInPixels(void);
    bool DoAfterParamModeModifications(NFmiDrawParamList *theParamList);
    bool ActivateParam(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theParamIndex);
    void DrawActiveParamMarkers(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theParamLineIndex);
    NFmiRect CalcParameterDragRect(int theParamLineIndex, int leftMargin, int topMargin, int rightMargin, int bottomMargin);
    NFmiDrawParamList* GetDrawParamList();
    void DrawMouseDraggingAction();
    void DrawMouseDraggingBackground();
	void DrawBackgroundMapLayerText(int &zeroBasedRowIndexInOut, const NFmiRect &parameterRowRect);
	std::string MakeMacroParamTooltipText(const boost::shared_ptr<NFmiDrawParam>& drawParam, const std::string& paramStr);
	std::string MakeWmsTooltipText(const boost::shared_ptr<NFmiDrawParam>& drawParam, const std::string &paramStr);
private:
   double itsButtonSizeInMM_x; // paino nappuloiden koko millimetreiss� x-suunnassa
   double itsButtonSizeInMM_y; // paino nappuloiden koko millimetreiss� y-suunnassa

   // Hiirell� raahaamis piirtoj�rjestyksen vaihtoon liittyv�t muuttujat
   bool fMouseCaptured; // T�m� asetetaan p��lle heti LeftButtonDown:issa
   int itsCapturedParamRowIndex; // Mik� parametri rivi on kaapattu raahattavaksi (LeftButtonDown), -1 on reset tila
   int itsCurrentDragRowIndex; // Mihin riville ollaan kaapattua parametria raahaamassa t�ll� hetkell� (MouseMove:ssa), -1 on reset tila
   // Jos hiirt� on siirretty vasen nappi pohjassa tarpeeksi, menn��n dragging moodiin, jolloin LeftButtonUp osaa toimia erilailla kuin normaalisti,
   // Mutta jos vasen painetaan pohjaan ja se liikkuu hieman ja p��stet��n taas yl�s, t�ll�in halutaan normaali toiminta kuten esim. show-param on/off tms. toimintoja.
   // T�m� my�s pakottaa n�yt�n piirron MouseMove:ssa, jos ollaan menty drag-moodiin.
   bool fMouseDraggingAction; 
};

