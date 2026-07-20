//////////////////////////////////////////////////////////////////////
// NFmiGdiPlusImageMapHandler.cpp: implementation for the
// NFmiGdiPlusImageMapHandler class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
// Removes n VC++ compiler warnings (name too long >255 characters, which is caused by 'expanded' STL template names)
#pragma warning(disable : 4786)
#endif

#include "NFmiGdiPlusImageMapHandler.h"
#include "NFmiFileSystem.h"
#include "NFmiAreaFactory.h"
#include "NFmiArea.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiPolyline.h"
#include "NFmiFileString.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiPathUtils.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "NFmiMapConfigurationSystem.h"

using namespace std;

namespace
{
	void clearBitmapVector(std::vector<Gdiplus::Bitmap*>& theBitmaps)
	{
		for(auto *bitmapPointer : theBitmaps)
		{
			delete bitmapPointer;
		}
		theBitmaps.clear();
	}
}

NFmiGdiPlusImageMapHandler::NFmiGdiPlusImageMapHandler()
:itsUsedMapIndex(0)
,itsUsedOverMapBitmapIndex(-1)
,itsMapBitmaps()
,itsOverMapBitmaps()
,itsOriginalArea()
,itsZoomedArea()
,itsZoomedAreaPosition()
,fMakeNewBackgroundBitmap(true)
,fUpdateMapViewDrawingLayers(true)
,fMapReallyChanged(true)
,itsControlPath()
,itsSwapBaseArea()
,itsSwapBackArea()
,itsSwapMode(0)
,itsCountryBorderPolylineCache()
,itsLandBorderPath()
,itsMapConfiguration()
{
}

static NFmiArea* MakeNewAreaClone(const boost::shared_ptr<NFmiArea>& areaPtr)
{
	if(areaPtr)
		return areaPtr->Clone();
	else
		return nullptr;
}

// Note that the copy constructor does not make a complete copy.
// The assignment operator is used for copying.
NFmiGdiPlusImageMapHandler::NFmiGdiPlusImageMapHandler(const NFmiGdiPlusImageMapHandler& other)
	:itsUsedMapIndex(0)
	, itsUsedOverMapBitmapIndex(0)
	, itsMapBitmaps()
	, itsOverMapBitmaps()
	, itsOriginalArea(nullptr)
	, itsZoomedArea(nullptr)
	, itsZoomedAreaPosition()
	, fMakeNewBackgroundBitmap(true)
	, fUpdateMapViewDrawingLayers(true)
	, fMapReallyChanged(true)
	, itsControlPath()
	, itsSwapBaseArea(nullptr)
	, itsSwapBackArea(nullptr)
	, itsSwapMode(0)
	, itsCountryBorderPolylineCache()
	, itsLandBorderPath()
	, itsMapConfiguration()
{
	*this = other;
}

// Note that the assignment operator does not make a complete copy.
// Not everything can or may be copied; see the comments about the special cases and reasons individually.
NFmiGdiPlusImageMapHandler& NFmiGdiPlusImageMapHandler::operator=(const NFmiGdiPlusImageMapHandler& other)
{
	if(this != &other)
	{
		itsUsedMapIndex = other.itsUsedMapIndex;
		itsUsedOverMapBitmapIndex = other.itsUsedOverMapBitmapIndex;
		//itsMapBitmaps // bitmaps cannot be copied, this is initialized at the end of the method
		//itsOverMapBitmaps // bitmaps cannot be copied, this is initialized at the end of the method
		itsOriginalArea.reset(::MakeNewAreaClone(other.itsOriginalArea));
		itsZoomedArea.reset(::MakeNewAreaClone(other.itsZoomedArea));
		itsZoomedAreaPosition = other.itsZoomedAreaPosition;
		// After a copy, force building a new base map
		fMakeNewBackgroundBitmap = true;
		// After a copy, force updating the drawing layers
		fUpdateMapViewDrawingLayers = true;
		// After a copy, do things as if the map area had really changed
		fMapReallyChanged = true;
		itsControlPath = other.itsControlPath;
		itsSwapBaseArea.reset(::MakeNewAreaClone(other.itsSwapBaseArea));
		itsSwapBackArea.reset(::MakeNewAreaClone(other.itsSwapBackArea));
		itsSwapMode = other.itsSwapMode;
		itsCountryBorderPolylineCache = other.itsCountryBorderPolylineCache;
		itsLandBorderPath = other.itsLandBorderPath;
		itsMapConfiguration = other.itsMapConfiguration;
		InitializeBitmapVectors();
	}
	return *this;
}

NFmiGdiPlusImageMapHandler::~NFmiGdiPlusImageMapHandler()
{
	::clearBitmapVector(itsMapBitmaps);
	::clearBitmapVector(itsOverMapBitmaps);
}

void NFmiGdiPlusImageMapHandler::CreateMapAreaFromConfiguration()
{
	const auto& projectionFileName = itsMapConfiguration->ProjectionFileName();
	if(projectionFileName.empty())
	{
		const auto& projectionString = itsMapConfiguration->Projection();
		CreateOriginalArea(projectionString);

		if(!itsOriginalArea)
		{
			string errMsg("NFmiGdiPlusImageMapHandler::CreateMapAreaFromConfiguration - unable to create the map area from settings with area string: ");
			errMsg += projectionString;
			throw runtime_error(errMsg);
		}
	}
	else
	{
		const auto& usedTotalAreaFilePath = PathUtils::makeFixedAbsolutePath(projectionFileName, itsControlPath);
		itsOriginalArea = ReadArea(usedTotalAreaFilePath);

		if(!itsOriginalArea)
		{
			string errMsg("NFmiGdiPlusImageMapHandler::CreateMapAreaFromConfiguration - unable to read the area file: ");
			errMsg += usedTotalAreaFilePath;
			errMsg += ", originally gives as: ";
			errMsg += projectionFileName;
			throw runtime_error(errMsg);
		}
	}
}

bool NFmiGdiPlusImageMapHandler::Init(std::shared_ptr<NFmiMapConfiguration>& mapConfiguration)
{
	CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, __FUNCTION__);

	itsMapConfiguration = mapConfiguration;
	CreateMapAreaFromConfiguration();

	itsZoomedArea = boost::shared_ptr<NFmiArea>(itsOriginalArea->Clone());
	itsSwapBaseArea = boost::shared_ptr<NFmiArea>(itsOriginalArea->Clone());
	InitializeBitmapVectors();

	const auto& mapFileNames = itsMapConfiguration->MapFileNames();
	// Must read the 1st image into memory, in order to obtain its size
	if(mapFileNames.size() > 0)
	{
		itsMapBitmaps[0] = CreateBitmapFromFile(mapFileNames[0]);
		if(itsMapBitmaps[0] == 0)
		{
			string errMsg("NFmiGdiPlusImageMapHandler::Init - unable to read the image file: \n");
			errMsg += mapFileNames[0];
			throw runtime_error(errMsg);
		}
		CalcZoomedAreaPosition();
	}

	return true;
}

void NFmiGdiPlusImageMapHandler::InitializeBitmapVectors()
{
	// The image array must be initialized with 0-pointers.
	const auto& mapFileNames = itsMapConfiguration->MapFileNames();
	for(auto mapIndex = 0ul; mapIndex < static_cast<int>(mapFileNames.size()); mapIndex++)
		itsMapBitmaps.push_back(nullptr);

	const auto& overMapDibFileNames = itsMapConfiguration->OverMapDibFileNames();
	for(auto mapIndex = 0ul; mapIndex < static_cast<int>(overMapDibFileNames.size()); mapIndex++)
		itsOverMapBitmaps.push_back(nullptr);
}

boost::shared_ptr<NFmiArea> NFmiGdiPlusImageMapHandler::ReadArea(const string& theAreaFileName)
{
	if(NFmiFileSystem::FileExists(theAreaFileName) == false)
		throw runtime_error(std::string("Error: NFmiGdiPlusImageMapHandler::ReadArea - file does not exist:\n") + theAreaFileName);
	string areaStr;
	NFmiFileSystem::ReadFile2String(theAreaFileName, areaStr);
	return NFmiAreaFactory::Create(areaStr);
}

Gdiplus::Bitmap* NFmiGdiPlusImageMapHandler::GetBitmap()
{
	if(itsUsedMapIndex >= 0 && itsUsedMapIndex < static_cast<int>(itsMapBitmaps.size()))
	{
		if(!itsMapBitmaps[itsUsedMapIndex])
			itsMapBitmaps[itsUsedMapIndex] = CreateBitmapFromFile(itsMapConfiguration->MapFileNames()[itsUsedMapIndex]);
		return itsMapBitmaps[itsUsedMapIndex];
	}
	return 0;
}

void NFmiGdiPlusImageMapHandler::CreateOriginalArea(const std::string& theArea)
{
	itsOriginalArea = NFmiAreaFactory::Create(theArea);
}

void NFmiGdiPlusImageMapHandler::Area(const boost::shared_ptr<NFmiArea> &newArea)
{
	if(newArea)
	{
		if(itsZoomedArea)
		{
			if(itsZoomedArea == newArea)
				return ;
			// This condition tries to test whether the areas are the same
			if(newArea->TopRightLatLon() == itsZoomedArea->TopRightLatLon() &&
			   newArea->BottomLeftLatLon() == itsZoomedArea->BottomLeftLatLon())
				return ;
		}
		if(NFmiQueryDataUtil::AreAreasSameKind(itsOriginalArea.get(), newArea.get()))
		{
			itsZoomedArea = boost::shared_ptr<NFmiArea>(newArea->Clone());
		}
		else
		{
			// A new zoomed area must be created using the corners of newArea but using the original area (this way SmartMet's map view does not get confused and drawing possibly slowed down if a scandi view macro has been used in euro SmartMet)
			string origAreaStr = itsOriginalArea->AreaStr();
			string newAreaStr = newArea->AreaStr();
			string::size_type pos1 = origAreaStr.find(":");
			string::size_type pos2 = newAreaStr.find(":");
			if(pos1 != string::npos && pos2 != string::npos)
			{
				// Take the initial part (area type) from the original area
				string newZoomedAreaStr(origAreaStr.begin(), origAreaStr.begin()+pos1);
				// Take the corner points from the new area
				newZoomedAreaStr += string(newAreaStr.begin()+pos2, newAreaStr.end());
				boost::shared_ptr<NFmiArea> tmpArea = NFmiAreaFactory::Create(newZoomedAreaStr);
				if(tmpArea.get() == 0)
					// Something went wrong, do nothing
					return ;
				else
				{
					itsZoomedArea = tmpArea;
				}
			}
			else
				// Something went wrong, do nothing
				return ;
		}
		CalcZoomedAreaPosition();
        SetMakeNewBackgroundBitmap(true);
		// This is reset in the zoom dialog!!!! see the NFmiZoomView::Update method
		MapReallyChanged(true);
		itsSwapMode = 0;
	}
}

static NFmiRect CalcZoomedAbsolutRect(Gdiplus::Bitmap *theCurrentBitmap, const boost::shared_ptr<NFmiArea> &theOriginalArea, const boost::shared_ptr<NFmiArea> &theZoomedArea)
{
	if(theCurrentBitmap == 0)
	{
		string errMsg("NFmiGdiPlusImageMapHandler::CalcZoomedAbsolutRect - Cannot calculate, because given bitmap was 0-pointer, error in application.");
		throw runtime_error(errMsg);
	}
	else
	{
        NFmiRect relativeArea = theOriginalArea->XYArea(theZoomedArea.get());
		double top = relativeArea.Top();
		double bottom = relativeArea.Bottom();
		double left = relativeArea.Left();
		double right = relativeArea.Right();
		top *= theCurrentBitmap->GetHeight();
		bottom *= theCurrentBitmap->GetHeight();
		left *= theCurrentBitmap->GetWidth();
		right *= theCurrentBitmap->GetWidth();
		return NFmiRect(left, top, right, bottom);
	}
}


void NFmiGdiPlusImageMapHandler::CalcZoomedAreaPosition()
{
	if(itsOriginalArea && itsZoomedArea)
	{
		// Also compute this relative zoomed area
        itsZoomedAreaPosition = itsOriginalArea->XYArea(itsZoomedArea.get());
	}
}

boost::shared_ptr<NFmiArea> NFmiGdiPlusImageMapHandler::TotalArea()
{return itsOriginalArea;}

boost::shared_ptr<NFmiArea> NFmiGdiPlusImageMapHandler::Area()
{return itsZoomedArea;}

bool NFmiGdiPlusImageMapHandler::SetMaxArea()
{
	itsZoomedArea = boost::shared_ptr<NFmiArea>(itsOriginalArea->Clone());
	CalcZoomedAreaPosition();
    SetMakeNewBackgroundBitmap(true);
	return true;
}

// Sets the zoom to half of the whole area and to the center
bool NFmiGdiPlusImageMapHandler::SetHalfArea()
{
	NFmiRect halfRect(0,0,0.5,0.5);
	halfRect.Center(NFmiPoint(0.5,0.5));
	boost::shared_ptr<NFmiArea> area(itsOriginalArea->CreateNewArea(halfRect));
	if(!area)
		return false;
	else
	{
		itsZoomedArea = area;
		CalcZoomedAreaPosition();
        SetMakeNewBackgroundBitmap(true);
		return true;
	}
}

Gdiplus::Bitmap* NFmiGdiPlusImageMapHandler::GetOverMapBitmap()
{
	if(itsUsedOverMapBitmapIndex >= 0 && itsUsedOverMapBitmapIndex < static_cast<int>(itsOverMapBitmaps.size()))
	{
		if(!itsOverMapBitmaps[itsUsedOverMapBitmapIndex])
			itsOverMapBitmaps[itsUsedOverMapBitmapIndex] = CreateBitmapFromFile(itsMapConfiguration->OverMapDibFileNames()[itsUsedOverMapBitmapIndex]);
		return itsOverMapBitmaps[itsUsedOverMapBitmapIndex];
	}
	return 0;
}

bool NFmiGdiPlusImageMapHandler::ShowOverMap()
{
	if(itsUsedOverMapBitmapIndex >= 0 && itsUsedOverMapBitmapIndex < static_cast<int>(itsOverMapBitmaps.size()))
		return true;
	else
		return false;
}

void NFmiGdiPlusImageMapHandler::NextOverMap()
{
	int oldIndex = itsUsedOverMapBitmapIndex;
	itsUsedOverMapBitmapIndex++;
	if(itsUsedOverMapBitmapIndex >= static_cast<long>(itsOverMapBitmaps.size()))
		itsUsedOverMapBitmapIndex = -1;
}

void NFmiGdiPlusImageMapHandler::PreviousOverMap()
{
	int oldIndex = itsUsedOverMapBitmapIndex;
	itsUsedOverMapBitmapIndex--;
	if(itsUsedOverMapBitmapIndex < -1)
		itsUsedOverMapBitmapIndex = static_cast<long>(itsOverMapBitmaps.size()-1);
}

void NFmiGdiPlusImageMapHandler::OverMapBitmapIndex(int newValue)
{
	if(itsUsedOverMapBitmapIndex != newValue && itsOverMapBitmaps.size() > 0)
	{
		itsUsedOverMapBitmapIndex = newValue;
		if(itsUsedOverMapBitmapIndex < 0)
			itsUsedOverMapBitmapIndex = -1;
		else if(itsUsedOverMapBitmapIndex >= static_cast<int>(itsOverMapBitmaps.size()))
			itsUsedOverMapBitmapIndex = static_cast<int>(itsOverMapBitmaps.size() - 1);
	}
}

void NFmiGdiPlusImageMapHandler::NextMap()
{
	int oldIndex = itsUsedMapIndex;
	itsUsedMapIndex++;
	if(itsUsedMapIndex >= static_cast<long>(itsMapBitmaps.size()))
		itsUsedMapIndex = 0;
}

void NFmiGdiPlusImageMapHandler::PreviousMap()
{
	int oldIndex = itsUsedMapIndex;
	itsUsedMapIndex--;
	if(itsUsedMapIndex < 0)
		itsUsedMapIndex = static_cast<long>(itsMapBitmaps.size()-1);
}

void NFmiGdiPlusImageMapHandler::UsedMapIndex(int theIndex)
{
	if(itsUsedMapIndex != theIndex && itsMapBitmaps.size() > 0)
	{
		itsUsedMapIndex = theIndex;
		if(itsUsedMapIndex < 0)
			itsUsedMapIndex = 0;
		else if(itsUsedMapIndex >= static_cast<int>(itsMapBitmaps.size()))
			itsUsedMapIndex = static_cast<int>(itsMapBitmaps.size()-1);
	}
}

const NFmiRect& NFmiGdiPlusImageMapHandler::Position()
{
	return itsZoomedAreaPosition;
}

Gdiplus::Bitmap* NFmiGdiPlusImageMapHandler::CreateBitmapFromFile(const std::string &theFileName)
{
	return CtrlView::CreateBitmapFromFile(itsControlPath, theFileName);
}

int NFmiGdiPlusImageMapHandler::GetDrawStyle()
{
	const auto& mapDrawingStyles = itsMapConfiguration->MapDrawingStyles();
	if(itsUsedMapIndex >= 0 && itsUsedMapIndex < static_cast<int>(mapDrawingStyles.size()))
	{
		return mapDrawingStyles[itsUsedMapIndex];
	}
	return 0;
}

int NFmiGdiPlusImageMapHandler::GetOverMapDrawStyle()
{
	const auto& overMapBitmapDrawStyles = itsMapConfiguration->OverMapDibDrawingStyles();
	if(itsUsedOverMapBitmapIndex >= 0 && itsUsedOverMapBitmapIndex < static_cast<int>(overMapBitmapDrawStyles.size()))
	{
		return overMapBitmapDrawStyles[itsUsedOverMapBitmapIndex];
	}
	return 0;
}

const std::string& NFmiGdiPlusImageMapHandler::GetBitmapFileName()
{
	const auto& mapFileNames = itsMapConfiguration->MapFileNames();
	if(itsUsedMapIndex >= 0 && itsUsedMapIndex < static_cast<int>(mapFileNames.size()))
	{
		return mapFileNames[itsUsedMapIndex];
	}
	else
	{
		static const std::string dummy;
		return dummy;
	}
}

const std::string& NFmiGdiPlusImageMapHandler::GetOverMapBitmapFileName()
{
	const auto& overMapDibFileNames = itsMapConfiguration->OverMapDibFileNames();
	if(itsUsedOverMapBitmapIndex >= 0 && itsUsedOverMapBitmapIndex < static_cast<int>(overMapDibFileNames.size()))
	{
		return overMapDibFileNames[itsUsedOverMapBitmapIndex];
	}
	else
	{
		static const std::string dummy;
		return dummy;
	}
}

static std::string MakeAbsoluteFileName(const std::string &theFileName, const std::string &thePath)
{
	if(theFileName.empty())
	{
		std::string errMsg("MakeAbsoluteFileName - given image file name was empty.");
		throw std::runtime_error(errMsg);
	}
	std::string finalFileName;
	if(!thePath.empty())
	{
		if(NFmiFileSystem::DirectoryExists(thePath) == false)
		{
			std::string errMsg(std::string("MakeAbsoluteFileName - given path/directory doesn't exist:\n") + thePath);
			throw std::runtime_error(errMsg);
		}
		finalFileName = thePath;
		// The editor's working directory must be added to the relative file name, because the default directory may have changed e.g. during macro saves etc.
		finalFileName += kFmiDirectorySeparator;
	}
	finalFileName += theFileName;
	return finalFileName;
}

const std::string NFmiGdiPlusImageMapHandler::GetBitmapAbsoluteFileName()
{
	return ::MakeAbsoluteFileName(GetBitmapFileName(), ControlPath());
}

const std::string NFmiGdiPlusImageMapHandler::GetOverMapBitmapAbsoluteFileName()
{
	return ::MakeAbsoluteFileName(GetOverMapBitmapFileName(), ControlPath());
}

static NFmiRect CalcTotalAbsolutRect(Gdiplus::Bitmap *theBitmap)
{
	if(theBitmap)
	{
		NFmiRect aRect(0, 0, theBitmap->GetWidth(), theBitmap->GetHeight());
		return aRect;
	}
	else
	{
		string errMsg("NFmiGdiPlusImageMapHandler - CalcTotalAbsolutRect - Cannot calculate map images total pixel size, cause bitmap was 0-pointer, error in application.");
		throw runtime_error(errMsg);
	}
}

NFmiRect NFmiGdiPlusImageMapHandler::TotalAbsolutRect()
{
    return ::CalcTotalAbsolutRect(GetBitmap());
}

NFmiRect NFmiGdiPlusImageMapHandler::TotalAbsolutRectOverMap()
{
    return ::CalcTotalAbsolutRect(GetOverMapBitmap());
}

NFmiRect NFmiGdiPlusImageMapHandler::ZoomedAbsolutRect()
{
    return ::CalcZoomedAbsolutRect(GetBitmap(), itsOriginalArea, itsZoomedArea);
}

NFmiRect NFmiGdiPlusImageMapHandler::ZoomedAbsolutRectOverMap()
{
    return ::CalcZoomedAbsolutRect(GetOverMapBitmap(), itsOriginalArea, itsZoomedArea);
}

static double CalcBitmapAspectRatio(Gdiplus::Bitmap *theBitmap)
{
	if(theBitmap)
		return theBitmap->GetWidth() / static_cast<double>(theBitmap->GetHeight());
	else
	{
		string errMsg("NFmiGdiPlusImageMapHandler - CalcBitmapAspectRatio - Cannot calculate aspect ratio, bitmap is 0-pointer, error in application.");
		throw runtime_error(errMsg);
	}
}

double NFmiGdiPlusImageMapHandler::BitmapAspectRatio()
{
	return ::CalcBitmapAspectRatio(GetBitmap());
}

double NFmiGdiPlusImageMapHandler::BitmapAspectRatioOverMap()
{
    return ::CalcBitmapAspectRatio(GetOverMapBitmap());
}

void NFmiGdiPlusImageMapHandler::MakeSwapBaseArea()
{
	itsSwapBaseArea = boost::shared_ptr<NFmiArea>(itsZoomedArea->Clone());
	itsSwapMode = 0;
}

void NFmiGdiPlusImageMapHandler::SwapArea()
{
	if(itsSwapMode == 0)
	{
		itsSwapBackArea = boost::shared_ptr<NFmiArea>(itsZoomedArea->Clone());
		// Zoom to the swap-base
		Area(itsSwapBaseArea);
		// This must be set after the Area method
		itsSwapMode = 1;
	}
	else
	{
		// Zoom back to the swap-back, swap-mode is set in the Area method
		Area(itsSwapBackArea);
	}
}

void NFmiGdiPlusImageMapHandler::DrawBorderPolyLineList(std::list<NFmiPolyline*> &newValue)
{
	itsCountryBorderPolylineCache.drawBorderPolyLineList(newValue);
}

void NFmiGdiPlusImageMapHandler::DrawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>>& newValue)
{ 
	itsCountryBorderPolylineCache.drawBorderPolyLineListGdiplus(newValue);
}

void NFmiGdiPlusImageMapHandler::DrawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>>&& newValue)
{ 
	itsCountryBorderPolylineCache.drawBorderPolyLineListGdiplus(std::move(newValue));
}

// I reworked the map dirtying systems in several ways:
// 1. The name changed, hopefully to a more descriptive one, i.e. MakeNewBackgroundBitmap, meaning
//    that on the next drawing round the base maps must be redrawn.
// 2. The so-called set function can only turn the flag on; a false value is ignored. Only the clear function
//    can reset the flag's state. This change is because different places may provide different values in the same dirtying round,
//    and on the other hand a false setting made in one place should not reset a true setting made in another.
// 3. The flag is thus reset with the clear function, which is called after the drawing operations.
bool NFmiGdiPlusImageMapHandler::MakeNewBackgroundBitmap() const 
{ 
    return fMakeNewBackgroundBitmap; 
}

void NFmiGdiPlusImageMapHandler::SetMakeNewBackgroundBitmap(bool newState) 
{ 
    if(newState)
        fMakeNewBackgroundBitmap = newState; 
}

void NFmiGdiPlusImageMapHandler::ClearMakeNewBackgroundBitmap() 
{ 
    fMakeNewBackgroundBitmap = false; 
}

// The same query, set and reset mechanism as for the MakeNewBackgroundBitmap flag
bool NFmiGdiPlusImageMapHandler::UpdateMapViewDrawingLayers() const
{
    return fUpdateMapViewDrawingLayers;
}

void NFmiGdiPlusImageMapHandler::SetUpdateMapViewDrawingLayers(bool newState)
{
    if(newState)
        fUpdateMapViewDrawingLayers = newState;
}

void NFmiGdiPlusImageMapHandler::ClearUpdateMapViewDrawingLayers()
{
    fUpdateMapViewDrawingLayers = false;
}

// The new border-draw-dirty system does not set flags on, but clears the necessary caches
// so that on the next drawing round work has to be done.
void NFmiGdiPlusImageMapHandler::SetBorderDrawDirtyState(CountryBorderDrawDirtyState newState)
{
	itsCountryBorderPolylineCache.setBorderDrawDirtyState(newState);
}

bool NFmiGdiPlusImageMapHandler::BorderDrawPolylinesDirty() const
{
	return itsCountryBorderPolylineCache.borderDrawPolylinesDirty();
}

bool NFmiGdiPlusImageMapHandler::BorderDrawPolylinesGdiplusDirty() const
{
	return itsCountryBorderPolylineCache.borderDrawPolylinesGdiplusDirty();
}

std::list<NFmiPolyline*>& NFmiGdiPlusImageMapHandler::DrawBorderPolyLineList()
{
	return itsCountryBorderPolylineCache.drawBorderPolyLineList();
}

const std::list<std::vector<NFmiPoint>>& NFmiGdiPlusImageMapHandler::DrawBorderPolyLineListGdiplus()
{
	return itsCountryBorderPolylineCache.drawBorderPolyLineListGdiplus();
}

boost::shared_ptr<Imagine::NFmiPath> NFmiGdiPlusImageMapHandler::LandBorderPath()
{
	return itsLandBorderPath;
}

void NFmiGdiPlusImageMapHandler::LandBorderPath(boost::shared_ptr<Imagine::NFmiPath>& thePath)
{
	itsLandBorderPath = thePath;
}

static int FindMapLayerTextFromVector(const std::string& layerName, const std::vector<std::string>& layerNames)
{
	auto iter = std::find(layerNames.begin(), layerNames.end(), layerName);
	if(iter == layerNames.end())
		return -1;
	else
	{
		return static_cast<int>(std::distance(layerNames.begin(), iter));
	}
}

// Prioritization of the searches in the xxxFromViewMacro methods:
// 1. If the given referenceName is found in the xxxMacroReferenceNames vector, use its index
// 2. Otherwise use the given mapLayerIndex directly
void NFmiGdiPlusImageMapHandler::SelectBackgroundMapFromViewMacro(const std::string& referenceName, int mapLayerIndex)
{
	auto referenceNameIndex = ::FindMapLayerTextFromVector(referenceName, itsMapConfiguration->BackgroundMapMacroReferenceNames());
	if(referenceNameIndex >= 0)
		UsedMapIndex(referenceNameIndex);
	else
		UsedMapIndex(mapLayerIndex);
}

void NFmiGdiPlusImageMapHandler::SelectOverlayMapFromViewMacro(const std::string& referenceName, int mapLayerIndex)
{
	auto referenceNameIndex = ::FindMapLayerTextFromVector(referenceName, itsMapConfiguration->OverlayMapMacroReferenceNames());
	if(referenceNameIndex >= 0)
		OverMapBitmapIndex(referenceNameIndex);
	else
		OverMapBitmapIndex(mapLayerIndex);
}

// Prioritization of the searches in the xxxFromGui methods:
// 1. Search whether name is found in the xxxMapDescriptiveNames vector
// 2. Search whether name is found in the xxxMapMacroReferenceNames vector
// 3. Search whether name is found in the xxxMapFileNameBasedReferenceNames vector
bool NFmiGdiPlusImageMapHandler::SelectBackgroundMapFromGui(const std::string& name)
{
	auto foundMapLayerIndex = ::FindMapLayerTextFromVector(name, itsMapConfiguration->BackgroundMapDescriptiveNames());
	if(foundMapLayerIndex < 0)
		foundMapLayerIndex = ::FindMapLayerTextFromVector(name, itsMapConfiguration->BackgroundMapMacroReferenceNames());
	if(foundMapLayerIndex < 0)
		foundMapLayerIndex = ::FindMapLayerTextFromVector(name, itsMapConfiguration->BackgroundMapFileNameBasedGuiNames());

	if(foundMapLayerIndex < 0)
		return false;
	else
	{
		UsedMapIndex(foundMapLayerIndex);
		return true;
	}
}

bool NFmiGdiPlusImageMapHandler::SelectOverlayMapFromGui(const std::string& name)
{
	auto foundOverlayMapLayerIndex = ::FindMapLayerTextFromVector(name, itsMapConfiguration->OverlayMapDescriptiveNames());
	if(foundOverlayMapLayerIndex < 0)
		foundOverlayMapLayerIndex = ::FindMapLayerTextFromVector(name, itsMapConfiguration->OverlayMapMacroReferenceNames());
	if(foundOverlayMapLayerIndex < 0)
		foundOverlayMapLayerIndex = ::FindMapLayerTextFromVector(name, itsMapConfiguration->OverlayMapFileNameBasedGuiNames());

	if(foundOverlayMapLayerIndex < 0)
		return false;
	else
	{
		OverMapBitmapIndex(foundOverlayMapLayerIndex);
		return true;
	}
}

std::string NFmiGdiPlusImageMapHandler::GetCurrentGuiMapLayerText(bool backgroundMap)
{
	return itsMapConfiguration->GetBestGuiUsedMapLayerName(static_cast<size_t>(itsUsedMapIndex), backgroundMap);
}
