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
#include "CombinedMapHandlerInterface.h"
#include "NFmiCountryBorderPolylineCache.h"

class NFmiArea;
class NFmiPolyline;
class OGRGeometry;
namespace Gdiplus
{
	class Bitmap;
}

class NFmiGdiPlusImageMapHandler
{
public:

	NFmiGdiPlusImageMapHandler();
	NFmiGdiPlusImageMapHandler(const NFmiGdiPlusImageMapHandler& other);
	NFmiGdiPlusImageMapHandler& operator=(const NFmiGdiPlusImageMapHandler &other);
	virtual ~NFmiGdiPlusImageMapHandler();
	Gdiplus::Bitmap* GetBitmap();
	int GetDrawStyle();
	const std::string& GetBitmapFileName();
	const std::string GetBitmapAbsoluteFileName();
	Gdiplus::Bitmap* GetOverMapBitmap();
	int GetOverMapDrawStyle();
	const std::string& GetOverMapBitmapFileName();
	const std::string GetOverMapBitmapAbsoluteFileName();
	bool Init(const std::string& theAreaFileName, const std::vector<std::string> &theMapFileNames, const std::vector<int> &theMapDrawStyles, const std::vector<std::string> &theOverMapBitmapFileNames, const std::vector<int> &theOverMapBitmapDrawStyles);
	bool Init(const std::vector<std::string> &theMapFileNames, const std::vector<int> &theMapDrawStyles, const std::vector<std::string> &theOverMapBitmapFileNames, const std::vector<int> &theOverMapBitmapDrawStyles);
	void Clear();

	void OriginalArea(const std::string& theArea);
	void Area(const boost::shared_ptr<NFmiArea> &newArea);
	NFmiRect ZoomedAbsolutRect();
	NFmiRect ZoomedAbsolutRectOverMap();
	NFmiRect TotalAbsolutRect();
	NFmiRect TotalAbsolutRectOverMap();
	boost::shared_ptr<NFmiArea> TotalArea();
	boost::shared_ptr<NFmiArea> Area();
	bool SetMaxArea();
	bool SetHalfArea(); // asettaa zoomin puoleksi koko alueesta ja keskelle
	// aspectratio muodossa x/y
	double BitmapAspectRatio();
	double BitmapAspectRatioOverMap();
	const NFmiRect& Position();
    bool MakeNewBackgroundBitmap() const;
    void SetMakeNewBackgroundBitmap(bool newState);
    void ClearMakeNewBackgroundBitmap();
    bool UpdateMapViewDrawingLayers() const;
    void SetUpdateMapViewDrawingLayers(bool newState);
    void ClearUpdateMapViewDrawingLayers();

	int UsedMapIndex(){return itsUsedMapIndex;};
	void UsedMapIndex(int theIndex);
	int MapSize() const { return static_cast<int>(itsMapBitmaps.size()); }
	int OverMapBitmapIndex() const {return itsUsedOverMapBitmapIndex;}
	void OverMapBitmapIndex(int newValue);
	int OverMapSize() const { return static_cast<int>(itsOverMapBitmaps.size()); }
	bool ShowOverMap();
	void NextMap();
	void PreviousMap();
	void NextOverMap();
	void PreviousOverMap();
	const std::string& ControlPath() const {return itsControlPath;}
	void ControlPath(const std::string& newValue) {itsControlPath = newValue;}
	void MakeSwapBaseArea();
	void SwapArea();
	bool MapReallyChanged() const {return fMapReallyChanged;}
	void MapReallyChanged(bool newValue) {fMapReallyChanged = newValue;}
	std::list<NFmiPolyline*>& DrawBorderPolyLineList();
	void DrawBorderPolyLineList(std::list<NFmiPolyline*> &newValue);
	const std::list<std::vector<NFmiPoint>>& DrawBorderPolyLineListGdiplus();
	void DrawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>>& newValue);
	void DrawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>>&& newValue);
	void SetBorderDrawDirtyState(CountryBorderDrawDirtyState newState);
	bool BorderDrawPolylinesDirty() const;
	bool BorderDrawPolylinesGdiplusDirty() const;
	std::shared_ptr<OGRGeometry> CountryBorderGeometry();
	void CountryBorderGeometry(std::shared_ptr<OGRGeometry> &theGeometry);

private:
	Gdiplus::Bitmap* CreateBitmapFromFile(const std::string &theFileName);
	boost::shared_ptr<NFmiArea> ReadArea(const std::string& theAreaFileName);
	void CalcZoomedAreaPosition();
	void InitializeBitmapVectors();

	int itsUsedMapIndex;
	// Mit‰ nimi karttaa k‰ytet‰‰n (-1 = ei mit‰‰n, 0=1. vektorissa olevaa jne.)
	int itsUsedOverMapBitmapIndex; 
	// T‰h‰n l‰pin‰kyv‰‰n 'karttaan' on laitettu eri paikkojen sijainteja ja niiden nimi‰.
	std::vector<Gdiplus::Bitmap*> itsMapBitmaps;
	// T‰h‰n l‰pin‰kyv‰‰n 'karttaan' on laitettu eri paikkojen sijainteja ja niiden nimi‰.
	// T‰m‰ Bitmap on tarkoitus haluttaessa piirt‰‰ oikean kartan p‰‰lle.
	std::vector<Gdiplus::Bitmap*> itsOverMapBitmaps;
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
	std::vector<std::string> itsMapFileNames;
	std::vector<int> itsMapDrawStyles;
	std::vector<std::string> itsOverMapBitmapFileNames;
	std::vector<int> itsOverMapBitmapDrawStyles;
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

	NFmiCountryBorderPolylineCache itsCountryBorderPolylineCache;
	// T‰h‰n lasketaan itsOriginalArea:n sis‰‰n menev‰ maiden rajapolut shape datasta kerran
	std::shared_ptr<OGRGeometry> itsCountryBorderGeometry;
};

