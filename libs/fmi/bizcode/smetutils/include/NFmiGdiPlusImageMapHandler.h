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
#include "CombinedMapHandlerInterface.h"
#include "NFmiCountryBorderPolylineCache.h"

class NFmiArea;
class NFmiPolyline;
class NFmiMapConfiguration;
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
	bool Init(std::shared_ptr<NFmiMapConfiguration>& mapConfiguration);

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
	int SwapMode() const { return itsSwapMode; }
	void SwapMode(int newValue) { itsSwapMode = newValue; }
	bool MapReallyChanged() const {return fMapReallyChanged;}
	void MapReallyChanged(bool newValue) {fMapReallyChanged = newValue;}
	std::list<NFmiPolyline*>& DrawBorderPolyLineList();
	void DrawBorderPolyLineList(std::list<NFmiPolyline*> &newValue);
	const std::list<std::vector<NFmiPoint>>& DrawBorderPolyLineListGdiplus();
	void DrawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>>& newValue);
	void DrawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>>&& newValue);
	boost::shared_ptr<Imagine::NFmiPath> LandBorderPath();
	void LandBorderPath(boost::shared_ptr<Imagine::NFmiPath>& thePath);
	void SetBorderDrawDirtyState(CountryBorderDrawDirtyState newState);
	bool BorderDrawPolylinesDirty() const;
	bool BorderDrawPolylinesGdiplusDirty() const;

private:
	Gdiplus::Bitmap* CreateBitmapFromFile(const std::string &theFileName);
	boost::shared_ptr<NFmiArea> ReadArea(const std::string& theAreaFileName);
	void CalcZoomedAreaPosition();
	void InitializeBitmapVectors();
	void CreateOriginalArea(const std::string& theArea);
	void CreateMapAreaFromConfiguration();

	int itsUsedMapIndex;
	// Mitä nimi karttaa käytetään (-1 = ei mitään, 0=1. vektorissa olevaa jne.)
	int itsUsedOverMapBitmapIndex; 
	// Tähän läpinäkyvään 'karttaan' on laitettu eri paikkojen sijainteja ja niiden nimiä.
	std::vector<Gdiplus::Bitmap*> itsMapBitmaps;
	// Tähän läpinäkyvään 'karttaan' on laitettu eri paikkojen sijainteja ja niiden nimiä.
	// Tämä Bitmap on tarkoitus haluttaessa piirtää oikean kartan päälle.
	std::vector<Gdiplus::Bitmap*> itsOverMapBitmaps;
	boost::shared_ptr<NFmiArea> itsOriginalArea;
	boost::shared_ptr<NFmiArea> itsZoomedArea;
	NFmiRect itsZoomedAreaPosition;
    // Pitääkö uusi karttapohja rakentaa syystä tai toisesta
	bool fMakeNewBackgroundBitmap;
    // Jokin jossain karttanäytön rivin parametri layer rakenteissa on muuttunut ja niitä pitää päivittää
    bool fUpdateMapViewDrawingLayers;
    // Tämän avulla tiedetään onko karttaa oikeasti zoomattu, edellä olevaa fMakeNewBackgroundBitmap:ä voidaan käyttää muissakin tapauksissa
	bool fMapReallyChanged; 
    // Smartmetin kontrollipolku (-p optio) pitää laittaa talteen, koska muuten ohjelma ei aina
    // osaa lukea karttoja suhteellisesta polusta alkaen (tästä seurasi mustat ruudut)
    // Tässä oli aiemmin WorkingDirectory, mutta se ei toiminut varsinkaan kun käytössä oli 
    // KV-projektien Dropbox konfiguraatiot ja VC++ debuggerin käyttö menossa.
    std::string itsControlPath; 

	// Nämä swap-base ja swap-back area:t on SmartMetin pika zoomi swap ominaisuutta varten.
	// Eli halutessa käyttäjä voi tallettaa swap-base arean, johon palataan aina kun painetaan SPACE
	// acceleraattoria. Ja kun ollaan swap-basessa, seuraavan kerran kun painetaan SPACE:a, palataan
	// taas takaisin swap-back areaan (joka otetaan talteen aina kun on tehty 1. swap-komento).
	// CTRL+SPACE:lla talletetaan mikä tahansa zoomi swap-base:ksi. Mikä tahansa zoomi tuhoaa
	// swap-back:in, koska kun zoomataan ja painetaan SPACE:a, siitä tulee uusi swap-back-area.
	boost::shared_ptr<NFmiArea> itsSwapBaseArea; // defaulttina tähän otetaan itsOriginalArea, mutta tätä voidaan päivittää milloin vain CTRL+SPACE:llea ja SwapBase-metodilla.
	boost::shared_ptr<NFmiArea> itsSwapBackArea;
	// SwapMode määrää mitä tehdään seuraavksi, kun suoritetaan SwapArea-metodi.
	// Arvolla 0 tehdään perus swappi eli talletetaan nykyinen zoomi swap-back-areaan ja mennään swap-base-areaan.
	// Arvolla 1 palataan takaisin swap-base:sta swap-back:iin.
	// Aina kun tehdään mitä tahansa muita zoomeja, nollataan swapMode.
	int itsSwapMode;

	NFmiCountryBorderPolylineCache itsCountryBorderPolylineCache;
	// tähän lasketaan itsOriginalArea:n sisään menevä path kerran (GenDocissa)
	boost::shared_ptr<Imagine::NFmiPath> itsLandBorderPath;
	std::shared_ptr<NFmiMapConfiguration> itsMapConfiguration;

public:
	void SelectBackgroundMapFromViewMacro(const std::string& referenceName, int mapLayerIndex);
	void SelectOverlayMapFromViewMacro(const std::string& referenceName, int mapLayerIndex);
	bool SelectBackgroundMapFromGui(const std::string& name);
	bool SelectOverlayMapFromGui(const std::string& name);
	std::string GetCurrentGuiMapLayerText(bool backgroundMap);
	std::shared_ptr<NFmiMapConfiguration>& GetMapConfiguration() { return itsMapConfiguration; }
	const std::shared_ptr<NFmiMapConfiguration>& GetMapConfiguration() const { return itsMapConfiguration; }
};

