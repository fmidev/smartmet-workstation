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
	// Sets the zoom to half of the whole area and to the center
	bool SetHalfArea();
	// Aspect ratio in the form x/y
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
	// Which named map is used (-1 = none, 0 = the 1st one in the vector, etc.)
	int itsUsedOverMapBitmapIndex;
	// This transparent 'map' has been filled with the locations of different places and their names.
	std::vector<Gdiplus::Bitmap*> itsMapBitmaps;
	// This transparent 'map' has been filled with the locations of different places and their names.
	// This Bitmap is meant to be drawn on top of the actual map when desired.
	std::vector<Gdiplus::Bitmap*> itsOverMapBitmaps;
	boost::shared_ptr<NFmiArea> itsOriginalArea;
	boost::shared_ptr<NFmiArea> itsZoomedArea;
	NFmiRect itsZoomedAreaPosition;
    // Whether a new base map needs to be built for one reason or another
	bool fMakeNewBackgroundBitmap;
    // Something in some map view row's parameter layer structures has changed and they need to be updated
    bool fUpdateMapViewDrawingLayers;
    // This tells whether the map has really been zoomed; the above fMakeNewBackgroundBitmap can also be used in other cases
	bool fMapReallyChanged;
    // Smartmet's control path (-p option) must be stored, because otherwise the program cannot always
    // read maps starting from a relative path (this resulted in black cells)
    // This used to be WorkingDirectory, but that did not work, especially when
    // the Dropbox configurations of international projects and the VC++ debugger were in use.
    std::string itsControlPath;

	// These swap-base and swap-back areas are for SmartMet's quick zoom swap feature.
	// I.e. if desired, the user can store a swap-base area, which is returned to whenever the SPACE
	// accelerator is pressed. And when in swap-base, the next time SPACE is pressed, we return
	// back to the swap-back area (which is stored whenever the 1st swap command has been made).
	// With CTRL+SPACE any zoom is stored as the swap-base. Any zoom destroys
	// the swap-back, because when you zoom and press SPACE, it becomes the new swap-back-area.
	// By default itsOriginalArea is taken here, but this can be updated at any time with CTRL+SPACE and the SwapBase method.
	boost::shared_ptr<NFmiArea> itsSwapBaseArea;
	boost::shared_ptr<NFmiArea> itsSwapBackArea;
	// SwapMode determines what is done next when the SwapArea method is executed.
	// With value 0 a basic swap is done, i.e. the current zoom is stored to the swap-back-area and we move to the swap-base-area.
	// With value 1 we return from swap-base back to swap-back.
	// Whenever any other zooms are done, swapMode is reset.
	int itsSwapMode;

	NFmiCountryBorderPolylineCache itsCountryBorderPolylineCache;
	// Here the path that goes inside itsOriginalArea is computed once (in GenDoc)
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

