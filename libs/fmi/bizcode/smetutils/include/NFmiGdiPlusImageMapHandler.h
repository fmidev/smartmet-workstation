// NFmiGdiPlusImageMapHandler.h: interface for the
// NFmiGdiPlusImageMapHandler class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include <gdiplus.h>
#include <list>
#include <boost/shared_ptr.hpp>

#include "NFmiRect.h"
#include "NFmiDataMatrix.h"

class NFmiArea;
class NFmiPolyline;
namespace Gdiplus
{
	class Bitmap;
}

namespace Imagine
{
    class NFmiPath;
}

class NFmiGdiPlusImageMapHandler
{
public:

	NFmiGdiPlusImageMapHandler(void);
	virtual ~NFmiGdiPlusImageMapHandler(void);
	Gdiplus::Bitmap* GetBitmap(void);
	int GetDrawStyle(void);
	const std::string& GetBitmapFileName(void);
	const std::string GetBitmapAbsoluteFileName(void);
	Gdiplus::Bitmap* GetOverMapBitmap(void);
	int GetOverMapDrawStyle(void);
	const std::string& GetOverMapBitmapFileName(void);
	const std::string GetOverMapBitmapAbsoluteFileName(void);
	bool Init(const std::string& theAreaFileName, const checkedVector<std::string> &theMapFileNames, const checkedVector<int> &theMapDrawStyles, const checkedVector<std::string> &theOverMapBitmapFileNames, const checkedVector<int> &theOverMapBitmapDrawStyles);
	bool Init(const checkedVector<std::string> &theMapFileNames, const checkedVector<int> &theMapDrawStyles, const checkedVector<std::string> &theOverMapBitmapFileNames, const checkedVector<int> &theOverMapBitmapDrawStyles);
	void Clear(void);

	void OriginalArea(const std::string& theArea);
	void Area(const boost::shared_ptr<NFmiArea> &newArea);
	NFmiRect ZoomedAbsolutRect(void);
	NFmiRect ZoomedAbsolutRectOverMap(void);
	NFmiRect TotalAbsolutRect(void);
	NFmiRect TotalAbsolutRectOverMap(void);
	boost::shared_ptr<NFmiArea> TotalArea(void);
	boost::shared_ptr<NFmiArea> Area(void);
	bool SetMaxArea(void);
	bool SetHalfArea(void); // asettaa zoomin puoleksi koko alueesta ja keskelle
	// aspectratio muodossa x/y
	double BitmapAspectRatio(void);
	double BitmapAspectRatioOverMap(void);
	const NFmiRect& Position(void);
    bool MakeNewBackgroundBitmap() const;
    void SetMakeNewBackgroundBitmap(bool newState);
    void ClearMakeNewBackgroundBitmap();
    bool UpdateMapViewDrawingLayers() const;
    void SetUpdateMapViewDrawingLayers(bool newState);
    void ClearUpdateMapViewDrawingLayers();

	int UsedMapIndex(void){return itsUsedMapIndex;};
	int OverMapBitmapIndex(void) const {return itsUsedOverMapBitmapIndex;}
	void OverMapBitmapIndex(int newValue);
	void UsedMapIndex(int theIndex);
	bool ShowOverMap(void);
	void NextMap(void);
	void PreviousMap(void);
	void NextOverMap(void);
	void PreviousOverMap(void);
	const std::string& ControlPath(void) const {return itsControlPath;}
	void ControlPath(const std::string& newValue) {itsControlPath = newValue;}
	void MakeSwapBaseArea(void);
	void SwapArea(void);
	bool MapReallyChanged(void) const {return fMapReallyChanged;}
	void MapReallyChanged(bool newValue) {fMapReallyChanged = newValue;}
	std::list<NFmiPolyline*>& DrawBorderPolyLineList(void) {return itsDrawBorderPolyLineList;}
	void DrawBorderPolyLineList(std::list<NFmiPolyline*> &newValue);
    const std::list<std::vector<NFmiPoint>>& DrawBorderPolyLineListGdiplus() { return itsDrawBorderPolyLineListGdiplus; }
    void DrawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>> &newValue) { itsDrawBorderPolyLineListGdiplus = newValue; }
    void DrawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>> &&newValue) { itsDrawBorderPolyLineListGdiplus = newValue; }
    boost::shared_ptr<Imagine::NFmiPath> LandBorderPath(void) {return itsLandBorderPath;}
    void LandBorderPath(boost::shared_ptr<Imagine::NFmiPath> &thePath) {itsLandBorderPath = thePath;}

private:
	void EmptyBitmapVector(checkedVector<Gdiplus::Bitmap*>& theBitmaps); // deletoi ja laittaa 0-pointterit vektoriin
	void ClearBitmapVector(checkedVector<Gdiplus::Bitmap*>& theBitmaps);
	Gdiplus::Bitmap* CreateBitmapFromFile(const std::string &theFileName);
	boost::shared_ptr<NFmiArea> ReadArea(const std::string& theAreaFileName);
	void CalcZoomedAreaPosition(void);

	int itsUsedMapIndex;
	int itsUsedOverMapBitmapIndex; // mit‰ nimi karttaa k‰ytet‰‰n (-1 = ei mit‰‰n, 0=1. vektorissa olevaa jne.)
	checkedVector<Gdiplus::Bitmap*> itsMapBitmaps;// t‰h‰n l‰pin‰kyv‰‰n 'karttaan' on laitettu eri paikkojen sijainteja ja niiden nimi‰.
	checkedVector<Gdiplus::Bitmap*> itsOverMapBitmaps;// t‰h‰n l‰pin‰kyv‰‰n 'karttaan' on laitettu eri paikkojen sijainteja ja niiden nimi‰.
									// T‰m‰ Bitmap on tarkoitus haluttaessa piirt‰‰ oikean kartan p‰‰lle.
	boost::shared_ptr<NFmiArea> itsOriginalArea;
	boost::shared_ptr<NFmiArea> itsZoomedArea;
	NFmiRect itsZoomedAreaPosition;
    // Pit‰‰kˆ uusi karttapohja rakentaa syyst‰ tai toisesta
	bool fMakeNewBackgroundBitmap;
    // Jokin jossain karttan‰ytˆn rivin parametri layer rakenteissa on muuttunut ja niit‰ pit‰‰ p‰ivitt‰‰
    bool fUpdateMapViewDrawingLayers;
    // T‰m‰n avulla tiedet‰‰n onko karttaa oikeasti zoomattu, edell‰ olevaa fMakeNewBackgroundBitmap:‰ voidaan k‰ytt‰‰ muissakin tapauksissa
	bool fMapReallyChanged; 
	std::string itsAreaFileName;
	checkedVector<std::string> itsMapFileNames;
	checkedVector<int> itsMapDrawStyles;
	checkedVector<std::string> itsOverMapBitmapFileNames;
	checkedVector<int> itsOverMapBitmapDrawStyles;
    // Smartmetin kontrollipolku (-p optio) pit‰‰ laittaa talteen, koska muuten ohjelma ei aina
    // osaa lukea karttoja suhteellisesta polusta alkaen (t‰st‰ seurasi mustat ruudut)
    // T‰ss‰ oli aiemmin WorkingDirectory, mutta se ei toiminut varsinkaan kun k‰ytˆss‰ oli 
    // KV-projektien Dropbox konfiguraatiot ja VC++ debuggerin k‰yttˆ menossa.
    std::string itsControlPath; 

	// N‰m‰ swap-base ja swap-back area:t on SmartMetin pika zoomi swap ominaisuutta varten.
	// Eli halutessa k‰ytt‰j‰ voi tallettaa swap-base arean, johon palataan aina kun painetaan SPACE
	// acceleraattoria. Ja kun ollaan swap-basessa, seuraavan kerran kun painetaan SPACE:a, palataan
	// taas takaisin swap-back areaan (joka otetaan talteen aina kun on tehty 1. swap-komento).
	// CTRL+SPACE:lla talletetaan mik‰ tahansa zoomi swap-base:ksi. Mik‰ tahansa zoomi tuhoaa
	// swap-back:in, koska kun zoomataan ja painetaan SPACE:a, siit‰ tulee uusi swap-back-area.
	boost::shared_ptr<NFmiArea> itsSwapBaseArea; // defaulttina t‰h‰n otetaan itsOriginalArea, mutta t‰t‰ voidaan p‰ivitt‰‰ milloin vain CTRL+SPACE:llea ja SwapBase-metodilla.
	boost::shared_ptr<NFmiArea> itsSwapBackArea;
	// SwapMode m‰‰r‰‰ mit‰ tehd‰‰n seuraavksi, kun suoritetaan SwapArea-metodi.
	// Arvolla 0 tehd‰‰n perus swappi eli talletetaan nykyinen zoomi swap-back-areaan ja menn‰‰n swap-base-areaan.
	// Arvolla 1 palataan takaisin swap-base:sta swap-back:iin.
	// Aina kun tehd‰‰n mit‰ tahansa muita zoomeja, nollataan swapMode.
	int itsSwapMode;

    // Optimointia: kun stationviewhandler on tehnyt piirtolistan valmiiksi, se talletetaan dokumenttiin ja sen piirto on nopeaa kun kaikki konversiot on valmikksi tehty	
    std::list<NFmiPolyline*> itsDrawBorderPolyLineList;
    std::list<std::vector<NFmiPoint>> itsDrawBorderPolyLineListGdiplus;
    boost::shared_ptr<Imagine::NFmiPath> itsLandBorderPath; // t‰h‰n lasketaan itsOriginalArea:n sis‰‰n menev‰ path kerran (GenDocissa)
};

