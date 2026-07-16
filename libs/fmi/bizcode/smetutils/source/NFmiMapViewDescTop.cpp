// ======================================================================
/*!
 * \file NFmiMapViewDescTop.cpp
 * \brief This class handles all the thing required to show individual map-views.
 * There will be several different mapviews in editor and GeneralDocument contains
 * vector of these objects.
 */
// ======================================================================

#pragma warning( push )
// E.g. gdiplusheaders.h produces a heap of completely useless "declaration hides class member" warnings for us
#pragma warning( disable : 4458 )

#include "NFmiMapViewDescTop.h"
#include "NFmiMapConfigurationSystem.h"
#include "NFmiGdiPlusImageMapHandler.h"

#include "NFmiSettings.h"
#include "NFmiArea.h"
#include "NFmiAreaFactory.h"
#include "NFmiDataStoringHelpers.h"
#include "NFmiApplicationWinRegistry.h"
#include "CtrlViewFunctions.h"
#include "CtrlViewWin32Functions.h"
#include "CtrlViewDocumentInterface.h"
#include "CombinedMapHandlerInterface.h"
#include "NFmiCombinedMapModeState.h"
#include "catlog/catlog.h"
#include "CtrlViewTimeConsumptionReporter.h"

#include <algorithm>
#include "stdafx.h"

#pragma warning( push )

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef max
#undef min
#undef max
#endif

namespace
{
	const NFmiRect g_TotalRelativeMapRect(0., 0., 1., 1.);

	// Converts a relative color channel value (range 0-1) to an 8-bit value (range 0-255)
	int colorRelativeTo8bit(double colorValue)
	{
		int eightBitColor = boost::math::iround(colorValue * 255.);
		eightBitColor = std::min(255, std::max(0, eightBitColor));
		return eightBitColor;
	}

	void colorChannelToHex(std::ostream &out, int channelValue)
	{
		out << std::setfill('0') << std::setw(2) << std::hex << channelValue;
	}

	std::string colorToHex(int red, int green, int blue)
	{
		std::stringstream stream;
		// These stream manipulators are in effect only for the output of one value, so a separate output line had to be made for each color channel
		::colorChannelToHex(stream, red);
		::colorChannelToHex(stream, green);
		::colorChannelToHex(stream, blue);
		return stream.str();
	}

	std::string colorToHexaString(const NFmiColor& color)
	{
		int redColor = ::colorRelativeTo8bit(color.GetRed());
		int greenColor = ::colorRelativeTo8bit(color.GetGreen());
		int blueColor = ::colorRelativeTo8bit(color.GetBlue());
		return colorToHex(redColor, greenColor, blueColor);
	}

	struct PointerDestroyer
	{
		template<typename T>
		void operator()(T* thePtr)
		{
			delete thePtr;
		}
	};

	std::vector<NFmiGdiPlusImageMapHandler*> CopyMapHandlerVector(const std::vector<NFmiGdiPlusImageMapHandler*>& mapHandlerVector)
	{
		std::vector<NFmiGdiPlusImageMapHandler*> copiedVector;
		for(const auto* mapHandler : mapHandlerVector)
		{
			copiedVector.emplace_back(new NFmiGdiPlusImageMapHandler(*mapHandler));
		}
		return copiedVector;
	}

	void SetGridSize(const string& theKey, const NFmiPoint& theGridSize)
	{
		string gridStr;
		gridStr += NFmiStringTools::Convert<int>(static_cast<int>(theGridSize.X()));
		gridStr += ",";
		gridStr += NFmiStringTools::Convert<int>(static_cast<int>(theGridSize.Y()));
		NFmiSettings::Set(theKey, gridStr, true);
	}

	template<typename T>
	T ReadValueFromSettings(const string& theBaseStr, const string& theKeyWord)
	{
		string settingStr = theBaseStr;
		settingStr += theKeyWord;
		return NFmiSettings::Require<T>(settingStr.c_str());
	}

	template<typename T>
	void StoreValueToSettings(const string& theBaseStr, const string& theKeyWord, T theValue)
	{
		string settingStr(theBaseStr);
		settingStr += theKeyWord;
		NFmiSettings::Set(settingStr.c_str(), NFmiStringTools::Convert<T>(theValue), true);
	}

	void InitDrawParamListVector(NFmiPtrList<NFmiDrawParamList>** theList, int theSize)
	{
		if((*theList))
		{
			(*theList)->Clear(true);
			delete (*theList);
		}
		(*theList) = new NFmiPtrList<NFmiDrawParamList>;
		for(int i = 0; i < theSize; i++)
			(*theList)->AddEnd(new NFmiDrawParamList());
	}

}

// ************* ViewMacroDipMapHelper  class *******************************************


NFmiMapViewDescTop::ViewMacroDipMapHelper::ViewMacroDipMapHelper() = default;


// ************************************************************************
// ******* NFmiMapViewDescTop::ViewMacroDipMapHelper class **************
// ************************************************************************

// NOTE!! This includes comments!
void NFmiMapViewDescTop::ViewMacroDipMapHelper::Write(std::ostream& os) const
{
	os << "// NFmiMapViewDescTop::ViewMacroDipMapHelper::Write..." << std::endl;

	os << "// UsedMapIndex + UsedOverMapDibIndex" << std::endl;
	os << itsUsedMapIndex << " " << itsUsedOverMapDibIndex << std::endl;

	os << "// itsZoomedAreaStr" << std::endl;
	os << itsZoomedAreaStr << std::endl;

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.

	// Adding 'double' format extra data
	// Store as the 1st extra data the used combined-mode map index
	extraData.Add(itsUsedCombinedModeMapIndex);
	// Store as the 2nd extra data the used combined-mode overlay map index
	extraData.Add(itsUsedCombinedModeOverMapDibIndex);

	// Adding 'string' format extra data
	// Store as the 1st extra data the possible background map macro-reference name
	extraData.Add(itsBackgroundMacroReference);
	// Store as the 2nd extra data the possible overlay map macro-reference name
	extraData.Add(itsOverlayMacroReference);

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiMapViewDescTop::ViewMacroDipMapHelper::Write failed");
}

// NOTE!! before this class is read in from a file, remove the comments
// with the NFmiCommentStripper class, because comments are inserted
// when writing. So do not use this method directly, but the Init(filename) method!!!!
void NFmiMapViewDescTop::ViewMacroDipMapHelper::Read(std::istream& is)
{
	is >> itsUsedMapIndex >> itsUsedOverMapDibIndex;

	is >> itsZoomedAreaStr;

	if(is.fail())
		throw std::runtime_error("NFmiMapViewDescTop::ViewMacroDipMapHelper::Read failed");

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

	// Picking 'double' format extra data; first initialize the data with default values, then check whether they were stored
	// By default the normal map index
	itsUsedCombinedModeMapIndex = itsUsedMapIndex;
	// By default the normal map index
	itsUsedCombinedModeOverMapDibIndex = itsUsedOverMapDibIndex;
	if(extraData.itsDoubleValues.size() >= 1)
		// Read as the 1st extra data the combined-mode map index
		itsUsedCombinedModeMapIndex = static_cast<int>(extraData.itsDoubleValues[0]);
	if(extraData.itsDoubleValues.size() >= 2)
		// Read as the 2nd extra data the combined-mode overlay map index
		itsUsedCombinedModeOverMapDibIndex = static_cast<int>(extraData.itsDoubleValues[1]);

	// Picking 'string' format extra data; first initialize the data with default values, then check whether they were stored
	itsBackgroundMacroReference.clear();
	if(extraData.itsStringValues.size() >= 1)
		// Read as the 1st extra data the background map macro-reference name
		itsBackgroundMacroReference = extraData.itsStringValues[0];
	itsOverlayMacroReference.clear();
	if(extraData.itsStringValues.size() >= 2)
		// Read as the 2nd extra data the overlay map macro-reference name
		itsOverlayMacroReference = extraData.itsStringValues[1];

	if(is.fail())
		throw std::runtime_error("NFmiMapViewDescTop::ViewMacroDipMapHelper::Read failed");
}


// ************************************************************************
// ******* NFmiMapViewDescTop class *************************************
// ************************************************************************

const std::vector<float> NFmiMapViewDescTop::itsTimeBoxTextSizeAllowedFactors{ 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f, 2.0f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f };
const NFmiColor NFmiMapViewDescTop::TimeBoxFillColorDefault = NFmiColor(1, 1, 1, 0.4f);
const FmiDirection NFmiMapViewDescTop::TimeBoxLocationDefault = kBottomLeft;
const float NFmiMapViewDescTop::TimeBoxTextSizeFactorDefault = 1.f;

NFmiMapViewDescTop::NFmiMapViewDescTop()
:itsSettingsBaseName()
,itsMapViewDescTopIndex(0)
,itsControlPath()
,itsGdiPlusImageMapHandlerList()
,itsMapViewCache(CtrlViewUtils::MaxViewGridYSize)
,fRedrawMapView(true)
,itsLandBorderColors()
,itsLandBorderColorIndex(0)
,itsLandBorderPenSize(1,1)
,itsTimeControlViewTimes()
,itsClientViewXperYRatio(1.)
// The atlas occupies this relative portion of the view, the rest goes to the time control window
,itsRelativeMapRect(0.,0.,1.,0.9)
,itsMapViewSizeInPixels(10, 10)
,itsParamWindowViewPosition(kTopLeft)
,itsDrawOverMapMode(0)
,itsMapRowStartingIndex(1)
,itsShowTimeOnMapMode(0)
,fShowTimeString(true)
,itsCurrentTime()
,itsViewGridSizeMax(CtrlViewUtils::MaxViewGridXSize, CtrlViewUtils::MaxViewGridYSize)
,itsStationPointColorIndex(0)
,itsStationPointSize(1, 1)
,itsTimeControlTimeStep(1)
,itsMapViewDisplayMode(CtrlViewUtils::MapViewMode::kNormal)
,fDescTopOn(false)
,fMapViewBitmapDirty(false)
,itsGraphicalInfo()
,itsGridPointCache()
,itsAnimationData()
,fLockToMainMapViewTime(true)
,fLockToMainMapViewRow(false)
,fShowTrajectorsOnMap(true)
,fShowSoundingMarkersOnMap(true)
,fShowCrossSectionMarkersOnMap(true)
,fShowSelectedPointsOnMap(true)
,fShowControlPointsOnMap(true)
,fShowObsComparisonOnMap(true)
,fShowWarningMarkersOnMap(true)
,fShowMasksOnMapVM(false)
,itsSpacingOutFactorVM(0)
,itsSelectedMapIndexVM(1)
,fShowStationPlotVM(false)
,itsViewGridSizeVM(1,1)
,itsSeparateCountryBorderBitmapCache()
,itsTrueMapViewSizeInfo(0)
{
}


NFmiMapViewDescTop::NFmiMapViewDescTop(const std::string &theSettingsBaseName, NFmiMapConfigurationSystem *theMapConfigurationSystem, NFmiProjectionCurvatureInfo* theProjectionCurvatureInfo, const std::string &theControlPath, int theMapViewDescTopIndex)
:itsSettingsBaseName(theSettingsBaseName + "::")
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,itsMapConfigurationSystem(theMapConfigurationSystem)
,itsProjectionCurvatureInfo(theProjectionCurvatureInfo)
,itsControlPath(theControlPath)
,itsGdiPlusImageMapHandlerList()
,itsMapViewCache(CtrlViewUtils::MaxViewGridXSize * CtrlViewUtils::MaxViewGridYSize)
,fRedrawMapView(true)
,itsLandBorderColors()
,itsLandBorderColorIndex(0)
,itsLandBorderPenSize(1,1)
,itsTimeControlViewTimes()
,itsClientViewXperYRatio(1.)
// The atlas occupies this relative portion of the view, the rest goes to the time control window
,itsRelativeMapRect(0.,0.,1.,0.9)
,itsMapViewSizeInPixels(10, 10)
,itsParamWindowViewPosition(kTopLeft)
,itsDrawOverMapMode(0)
,itsMapRowStartingIndex(1)
,itsShowTimeOnMapMode(0)
,fShowTimeString(true)
,itsCurrentTime()
,itsViewGridSizeMax(CtrlViewUtils::MaxViewGridXSize, CtrlViewUtils::MaxViewGridYSize)
,itsStationPointColorIndex(0)
,itsStationPointSize(1, 1)
,itsTimeControlTimeStep(1)
,itsMapViewDisplayMode(CtrlViewUtils::MapViewMode::kNormal)
,fDescTopOn(false)
,fMapViewBitmapDirty(false)
,itsGraphicalInfo()
,itsGridPointCache()
,itsAnimationData()
,fLockToMainMapViewTime(true)
,fLockToMainMapViewRow(false)
,fShowTrajectorsOnMap(true)
,fShowSoundingMarkersOnMap(true)
,fShowCrossSectionMarkersOnMap(true)
,fShowSelectedPointsOnMap(true)
,fShowControlPointsOnMap(true)
,fShowObsComparisonOnMap(true)
,fShowWarningMarkersOnMap(true)
,fShowMasksOnMapVM(false)
,itsSpacingOutFactorVM(0)
,itsSelectedMapIndexVM(1)
,fShowStationPlotVM(false)
,itsViewGridSizeVM(1,1)
,itsSeparateCountryBorderBitmapCache()
,itsTrueMapViewSizeInfo(theMapViewDescTopIndex)
{
}

// Note that the copy constructor does not make a complete copy.
// The assignment operator is used for the actual copying.
NFmiMapViewDescTop::NFmiMapViewDescTop(const NFmiMapViewDescTop& other)
	:itsSettingsBaseName()
	, itsMapViewDescTopIndex(0)
	, itsControlPath()
	, itsGdiPlusImageMapHandlerList()
	, itsMapViewCache(CtrlViewUtils::MaxViewGridXSize* CtrlViewUtils::MaxViewGridYSize)
	, fRedrawMapView(true)
	, itsLandBorderColors()
	, itsLandBorderColorIndex(0)
	, itsLandBorderPenSize()
	, itsTimeControlViewTimes()
	, itsClientViewXperYRatio(1)
	, itsRelativeMapRect()
	, itsMapViewSizeInPixels()
	, itsParamWindowViewPosition(kTopLeft)
	// The final copy is done in the method body
	, itsDrawParamListVector(new NFmiPtrList<NFmiDrawParamList>())
	, itsDrawOverMapMode(0)
	, itsMapRowStartingIndex(0)
	, itsShowTimeOnMapMode(0)
	, fShowTimeString(true)
	, itsCurrentTime()
	, itsViewGridSizeMax()
	, itsStationPointColorIndex(0)
	, itsStationPointSize()
	, itsTimeControlTimeStep(1)
	, itsMapViewDisplayMode(CtrlViewUtils::MapViewMode::kNormal)
	, fDescTopOn(true)
	, fMapViewBitmapDirty(true)
	// The MapView pointer must be copied here, it does not change during runtime
	, itsMapView(other.itsMapView)
	, itsGraphicalInfo()
	, itsGridPointCache()
	, itsAnimationData()
	, fLockToMainMapViewTime(false)
	, fLockToMainMapViewRow(false)
	, fShowTrajectorsOnMap(false)
	, fShowSoundingMarkersOnMap(false)
	, fShowCrossSectionMarkersOnMap(false)
	, fShowSelectedPointsOnMap(false)
	, fShowControlPointsOnMap(false)
	, fShowObsComparisonOnMap(false)
	, fShowWarningMarkersOnMap(false)
	, fShowMasksOnMapVM(false)
	, itsSpacingOutFactorVM(0)
	, itsSelectedMapIndexVM(0)
	, fShowStationPlotVM(false)
	, itsViewGridSizeVM()
	, itsSeparateCountryBorderBitmapCache()
	, itsTrueMapViewSizeInfo(0)
{
	*this = other;
}

// Note that the assignment operator does not make a complete copy.
// Not everything can or may be copied; see the comments about the special cases and reasons individually.
NFmiMapViewDescTop& NFmiMapViewDescTop::operator=(const NFmiMapViewDescTop& other)
{
	if(this != &other)
	{
		itsSettingsBaseName = other.itsSettingsBaseName;
		itsMapViewDescTopIndex = other.itsMapViewDescTopIndex;
		// The pointer is copied, neither owns it
		itsMapConfigurationSystem = other.itsMapConfigurationSystem;
		// The pointer is copied, neither owns it
		itsProjectionCurvatureInfo = other.itsProjectionCurvatureInfo;
		itsControlPath = other.itsControlPath;
		itsGdiPlusImageMapHandlerList = std::move(::CopyMapHandlerVector(other.itsGdiPlusImageMapHandlerList));
		// This bitmap cache just needs to be cleared
		itsMapViewCache.MakeDirty();
		// After a copy everything is redrawn
		fRedrawMapView = true;
		itsLandBorderColors = other.itsLandBorderColors;
		itsLandBorderColorIndex = other.itsLandBorderColorIndex;
		itsLandBorderPenSize = other.itsLandBorderPenSize;
		itsTimeControlViewTimes = other.itsTimeControlViewTimes;
		itsClientViewXperYRatio = other.itsClientViewXperYRatio;
		RelativeMapRect(other.itsRelativeMapRect);
		itsMapViewSizeInPixels = other.itsMapViewSizeInPixels;
		itsParamWindowViewPosition = other.itsParamWindowViewPosition;
		CombinedMapHandlerInterface::copyDrawParamsList(other.itsDrawParamListVector, itsDrawParamListVector);
		// For such MFC pointers, only nullptr
		itsMapBlitDC = nullptr;
		itsDrawOverMapMode = other.itsDrawOverMapMode;
		itsMapRowStartingIndex = other.itsMapRowStartingIndex;
		// For such MFC pointers, only nullptr
		itsCopyCDC = nullptr;
		itsShowTimeOnMapMode = other.itsShowTimeOnMapMode;
		fShowTimeString = other.fShowTimeString;
		itsCurrentTime = other.itsCurrentTime;
		itsViewGridSizeMax = other.itsViewGridSizeMax;
		itsStationPointColorIndex = other.itsStationPointColorIndex;
		itsStationPointSize = other.itsStationPointSize;
		itsTimeControlTimeStep = other.itsTimeControlTimeStep;
		itsMapViewDisplayMode = other.itsMapViewDisplayMode;
		itsAbsoluteActiveViewRow = other.itsAbsoluteActiveViewRow;
		fDescTopOn = other.fDescTopOn;
		// After copying, everything is 'dirty'
		fMapViewBitmapDirty = true;
		// The MapView pointer must be copied here, it does not change during runtime
		itsMapView = other.itsMapView;
		itsGraphicalInfo = other.itsGraphicalInfo;
		// Reset this cache too
		itsGridPointCache.Clear();
		itsAnimationData = other.itsAnimationData;
		fLockToMainMapViewTime = other.fLockToMainMapViewTime;
		fLockToMainMapViewRow = other.fLockToMainMapViewRow;
		fShowTrajectorsOnMap = other.fShowTrajectorsOnMap;
		fShowSoundingMarkersOnMap = other.fShowSoundingMarkersOnMap;
		fShowCrossSectionMarkersOnMap = other.fShowCrossSectionMarkersOnMap;
		fShowSelectedPointsOnMap = other.fShowSelectedPointsOnMap;
		fShowControlPointsOnMap = other.fShowControlPointsOnMap;
		fShowObsComparisonOnMap = other.fShowObsComparisonOnMap;
		fShowWarningMarkersOnMap = other.fShowWarningMarkersOnMap;
		fShowMasksOnMapVM = other.fShowMasksOnMapVM;
		itsSpacingOutFactorVM = other.itsSpacingOutFactorVM;
		itsSelectedMapIndexVM = other.itsSelectedMapIndexVM;
		fShowStationPlotVM = other.fShowStationPlotVM;
		itsViewGridSizeVM = other.itsViewGridSizeVM;
		// Deletoi ja nollaa itsLandBorderMapBitmap:in
		ClearBaseLandBorderMapBitmap();
		itsSeparateCountryBorderBitmapCache = other.itsSeparateCountryBorderBitmapCache;
		itsTrueMapViewSizeInfo = other.itsTrueMapViewSizeInfo;
		itsTimeBoxLocationVM = other.itsTimeBoxLocationVM;
		itsTimeBoxTextSizeFactorVM = other.itsTimeBoxTextSizeFactorVM;
		itsTimeBoxFillColorVM = other.itsTimeBoxFillColorVM;
	}
	return *this;
}

NFmiMapViewDescTop::~NFmiMapViewDescTop()
{
	Clear();
}

void NFmiMapViewDescTop::Clear(void)
{
	std::for_each(itsGdiPlusImageMapHandlerList.begin(), itsGdiPlusImageMapHandlerList.end(), PointerDestroyer());

	if(itsDrawParamListVector)
	{
		itsDrawParamListVector->Clear(true);
		delete itsDrawParamListVector;
		itsDrawParamListVector = nullptr;
	}
	ClearBaseLandBorderMapBitmap();
	itsSeparateCountryBorderBitmapCache.clearCache();
}

boost::shared_ptr<Imagine::NFmiPath> NFmiMapViewDescTop::LandBorderPath(void)
{
    return MapHandler()->LandBorderPath();
}

void NFmiMapViewDescTop::Init(NFmiMapViewWinRegistry &theMapViewWinRegistry)
{
	Clear();
	InitGdiPlusImageMapHandlerSystem();
	InitLandBorderDrawingSystem();
	InitMapViewDescTopFromSettings();
	// Creates empty lists
	InitMapViewDrawParamListVector();
	// The time resolution must be updated to the animation data
	AnimationDataRef().TimeStepInMinutes(static_cast<int>(::round(TimeControlTimeStep()*60)));
    InitFromMapViewWinRegistry(theMapViewWinRegistry);
}

void NFmiMapViewDescTop::InitFromMapViewWinRegistry(NFmiMapViewWinRegistry &theMapViewWinRegistry)
{
    fShowMasksOnMapVM = theMapViewWinRegistry.ShowMasksOnMap();
    itsSpacingOutFactorVM = theMapViewWinRegistry.SpacingOutFactor();
	// SelectedMapIndex must undergo sanity checks
    SelectedMapIndex(theMapViewWinRegistry.SelectedMapIndex());
	fShowStationPlotVM = theMapViewWinRegistry.ShowStationPlot();
    // The string must be converted to a point and finally sanity checks must be done
    ViewGridSize(CtrlViewUtils::String2Point(theMapViewWinRegistry.ViewGridSizeStr()), nullptr);
	TimeBoxLocation(theMapViewWinRegistry.TimeBoxLocation());
	TimeBoxTextSizeFactor(theMapViewWinRegistry.TimeBoxTextSizeFactor());
	itsTimeBoxFillColorVM = theMapViewWinRegistry.TimeBoxFillColor();
}

void NFmiMapViewDescTop::StoreToMapViewWinRegistry(NFmiMapViewWinRegistry &theMapViewWinRegistry)
{
    // These are adjusted and used directly from the Win registry, so their values must not be put back there at the end.
//    theMapViewWinRegistry.ShowMasksOnMap(fShowMasksOnMapVM);
//    theMapViewWinRegistry.SpacingOutFactor(itsSpacingOutFactorVM);
//    theMapViewWinRegistry.SelectedMapIndex(itsSelectedMapIndexVM);
//    theMapViewWinRegistry.ShowStationPlot(fShowStationPlotVM);

    // This is adjusted only here, so the end result must periodically be put back into the Win registry.
    theMapViewWinRegistry.ViewGridSizeStr(CtrlViewUtils::Point2String(itsViewGridSizeVM));
	theMapViewWinRegistry.TimeBoxLocation(itsTimeBoxLocationVM);
	theMapViewWinRegistry.TimeBoxTextSizeFactor(itsTimeBoxTextSizeFactorVM);
	theMapViewWinRegistry.TimeBoxFillColor(itsTimeBoxFillColorVM);
}


void NFmiMapViewDescTop::InitMapViewDescTopFromSettings(void)
{
	SetSelectedMapsFromSettings();

	itsLandBorderColorIndex = ReadValueFromSettings<int>(itsSettingsBaseName, "LandBorderColorIndex");

	int penSize = ReadValueFromSettings<int>(itsSettingsBaseName, "LandBorderPenSize");
	itsLandBorderPenSize = NFmiPoint(penSize, penSize);

	itsStationPointColorIndex = ReadValueFromSettings<int>(itsSettingsBaseName, "StationPointColorIndex");
	penSize = ReadValueFromSettings<int>(itsSettingsBaseName, "StationPointSize");
	itsStationPointSize = NFmiPoint(penSize, penSize);

	itsMapViewDisplayMode = static_cast<CtrlViewUtils::MapViewMode>(ReadValueFromSettings<int>(itsSettingsBaseName, "MapViewDisplayMode"));
	itsTimeControlTimeStep = ReadValueFromSettings<float>(itsSettingsBaseName, "TimeControlTimeStep") / 60.f;

	itsShowTimeOnMapMode = ReadValueFromSettings<int>(itsSettingsBaseName, "ShowTimeOnMapMode");
    // This also sets fShowTimeString to the correct value
    ShowTimeOnMapMode(itsShowTimeOnMapMode);
	itsDrawOverMapMode = ReadValueFromSettings<int>(itsSettingsBaseName, "DrawOverMapMode");
	fLockToMainMapViewTime = ReadValueFromSettings<bool>(itsSettingsBaseName, "LockToMainMapViewTime");
    fLockToMainMapViewRow = NFmiSettings::Optional<bool>(itsSettingsBaseName + "LockToMainMapViewRow", false);
    fShowTrajectorsOnMap = ReadValueFromSettings<bool>(itsSettingsBaseName, "ShowTrajectorsOnMap");
	fShowSoundingMarkersOnMap = ReadValueFromSettings<bool>(itsSettingsBaseName, "ShowSoundingMarkersOnMap");
	fShowCrossSectionMarkersOnMap = ReadValueFromSettings<bool>(itsSettingsBaseName, "ShowCrossSectionMarkersOnMap");
	fShowSelectedPointsOnMap = ReadValueFromSettings<bool>(itsSettingsBaseName, "ShowSelectedPointsOnMap");
	fShowControlPointsOnMap = ReadValueFromSettings<bool>(itsSettingsBaseName, "ShowControlPointsOnMap");
	fShowObsComparisonOnMap = ReadValueFromSettings<bool>(itsSettingsBaseName, "ShowObsComparisonOnMap");
	fShowWarningMarkersOnMap = ReadValueFromSettings<bool>(itsSettingsBaseName, "ShowWarningMessageMarkersOnMap");
}

void NFmiMapViewDescTop::StoreMapViewDescTopToSettings(void)
{
	StoreHandlerSelectedMapsToSettings();

	StoreValueToSettings<int>(itsSettingsBaseName, "LandBorderColorIndex", itsLandBorderColorIndex);
	StoreValueToSettings<int>(itsSettingsBaseName, "LandBorderPenSize", static_cast<int>(itsLandBorderPenSize.X()));
	StoreValueToSettings<int>(itsSettingsBaseName, "StationPointColorIndex", itsStationPointColorIndex);
	StoreValueToSettings<int>(itsSettingsBaseName, "StationPointSize", static_cast<int>(itsStationPointSize.X()));
	StoreValueToSettings<int>(itsSettingsBaseName, "MapViewDisplayMode", static_cast<int>(itsMapViewDisplayMode));

	StoreValueToSettings<float>(itsSettingsBaseName, "TimeControlTimeStep", itsTimeControlTimeStep * 60.f);

	StoreValueToSettings<int>(itsSettingsBaseName, "ShowTimeOnMapMode", itsShowTimeOnMapMode);
	StoreValueToSettings<int>(itsSettingsBaseName, "DrawOverMapMode", itsDrawOverMapMode);
    StoreValueToSettings<bool>(itsSettingsBaseName, "LockToMainMapViewTime", fLockToMainMapViewTime);
    StoreValueToSettings<bool>(itsSettingsBaseName, "LockToMainMapViewRow", fLockToMainMapViewRow);
	StoreValueToSettings<bool>(itsSettingsBaseName, "ShowTrajectorsOnMap", fShowTrajectorsOnMap);
	StoreValueToSettings<bool>(itsSettingsBaseName, "ShowSoundingMarkersOnMap", fShowSoundingMarkersOnMap);
	StoreValueToSettings<bool>(itsSettingsBaseName, "ShowCrossSectionMarkersOnMap", fShowCrossSectionMarkersOnMap);
	StoreValueToSettings<bool>(itsSettingsBaseName, "ShowSelectedPointsOnMap", fShowSelectedPointsOnMap);
	StoreValueToSettings<bool>(itsSettingsBaseName, "ShowControlPointsOnMap", fShowControlPointsOnMap);
	StoreValueToSettings<bool>(itsSettingsBaseName, "ShowObsComparisonOnMap", fShowObsComparisonOnMap);
	StoreValueToSettings<bool>(itsSettingsBaseName, "ShowWarningMessageMarkersOnMap", fShowWarningMarkersOnMap);
}

void NFmiMapViewDescTop::InitMapViewDrawParamListVector(void)
{
	::InitDrawParamListVector(&itsDrawParamListVector, static_cast<int>(itsViewGridSizeMax.X()*itsViewGridSizeMax.Y()));
}

void NFmiMapViewDescTop::InitLandBorderDrawingSystem(void)
{
	itsLandBorderColors.clear();
	// Black
	itsLandBorderColors.push_back(NFmiColor(0.0f, 0.0f, 0.0f));
	// White
	itsLandBorderColors.push_back(NFmiColor(1.0f, 1.0f, 1.0f));
	// Red
	itsLandBorderColors.push_back(NFmiColor(1.0f, 0.0f, 0.0f));
	// Blue
	itsLandBorderColors.push_back(NFmiColor(0.0f, 0.0f, 1.0f));
}

void NFmiMapViewDescTop::InitGdiPlusImageMapHandlerSystem(void)
{
	CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, __FUNCTION__);

	if(itsMapConfigurationSystem)
	{
		for(size_t mapAreaIndex = 0; mapAreaIndex < itsMapConfigurationSystem->Size(); mapAreaIndex++)
		{
			NFmiGdiPlusImageMapHandler* mHandler = CreateGdiPlusImageMapHandler(itsMapConfigurationSystem->GetMapConfiguration(mapAreaIndex));
			if(mHandler)
				itsGdiPlusImageMapHandlerList.push_back(mHandler);
			else
				throw std::runtime_error("ERROR in NFmiMapViewDescTop::InitGdiPlusImageMapHandlerSystem - cannot create NFmiGdiPlusImageMapHandler.");
		}
	}
	else
		throw std::runtime_error("ERROR in NFmiMapViewDescTop::InitGdiPlusImageMapHandlerSystem - MapConfigurationSystem was null pointer.");
}

NFmiGdiPlusImageMapHandler* NFmiMapViewDescTop::CreateGdiPlusImageMapHandler(std::shared_ptr<NFmiMapConfiguration> &theMapConfiguration)
{
	NFmiGdiPlusImageMapHandler* mHandler = new NFmiGdiPlusImageMapHandler;
	mHandler->UsedMapIndex(0);
    mHandler->ControlPath(std::string(itsControlPath));
	mHandler->Init(theMapConfiguration);
	return mHandler;
}

void NFmiMapViewDescTop::SelectedMapIndex(unsigned int newValue)
{
	itsSelectedMapIndexVM = newValue;
    // Note! In view macros the itsGdiPlusImageMapHandlerList is not initialized, so if its size = 0, then no index check and correction is done
	if(itsGdiPlusImageMapHandlerList.size() > 0 && itsSelectedMapIndexVM >= itsGdiPlusImageMapHandlerList.size())
		itsSelectedMapIndexVM = static_cast<int>(itsGdiPlusImageMapHandlerList.size() - 1);
}

NFmiGdiPlusImageMapHandler* NFmiMapViewDescTop::MapHandler(void) const
{
	if(itsSelectedMapIndexVM < itsGdiPlusImageMapHandlerList.size())
		return itsGdiPlusImageMapHandlerList[itsSelectedMapIndexVM];
	throw std::runtime_error("ERROR in NFmiMapViewDescTop::GdiPlusImageMapHandler - SelectedMapIndex was out of bounds, error in program or configurations.");
}

NFmiGdiPlusImageMapHandler* NFmiMapViewDescTop::MapHandler(unsigned int mapAreaIndex) const
{
	if(mapAreaIndex < itsGdiPlusImageMapHandlerList.size())
		return itsGdiPlusImageMapHandlerList[mapAreaIndex];
	throw std::runtime_error("ERROR in NFmiMapViewDescTop::GdiPlusImageMapHandler - given MapIndex was out of bounds, error in program or configurations.");
}

int NFmiMapViewDescTop::CalcVisibleRowCount() const
{
    if(itsMapViewDisplayMode == CtrlViewUtils::MapViewMode::kNormal)
        return static_cast<int>(itsViewGridSizeVM.Y());
    else if(itsMapViewDisplayMode == CtrlViewUtils::MapViewMode::kOneTime)
        return static_cast<int>(itsViewGridSizeVM.X()*itsViewGridSizeVM.Y());
    else
        return 1;
}

// Scrolls the view rows by the desired amount (negative scrolls up and positive count down)
bool NFmiMapViewDescTop::ScrollViewRow(int theCount)
{
    int oldValue = itsMapRowStartingIndex;
    itsMapRowStartingIndex += theCount;

    int currentRowCount = CalcVisibleRowCount();
    int maxRowSize = static_cast<int>(itsViewGridSizeMax.X()*itsViewGridSizeMax.Y());
    // Always go 'over' the edges
    if(itsMapRowStartingIndex < 1)
        itsMapRowStartingIndex = maxRowSize - currentRowCount + 1;
    else if(itsMapRowStartingIndex > maxRowSize - currentRowCount + 1)
        itsMapRowStartingIndex = 1;

    if(oldValue == itsMapRowStartingIndex)
        return false;
    else
        return true;
}

int NFmiMapViewDescTop::CalcMaxRowStartingIndex() const
{
    return static_cast<int>(itsViewGridSizeMax.X() * itsViewGridSizeMax.Y() - CalcVisibleRowCount() + 1);
}

void NFmiMapViewDescTop::ViewGridSize(const NFmiPoint& newSize, NFmiMapViewWinRegistry *theMapViewWinRegistry)
{
	itsViewGridSizeVM = newSize;
	if(itsViewGridSizeVM.X() < 1.)
		itsViewGridSizeVM.X(1);
	if(itsViewGridSizeVM.Y() < 1.)
		itsViewGridSizeVM.Y(1);
	if(itsViewGridSizeVM.X() > itsViewGridSizeMax.X())
		itsViewGridSizeVM.X(itsViewGridSizeMax.X());
	if(itsViewGridSizeVM.Y() > itsViewGridSizeMax.Y())
		itsViewGridSizeVM.Y(itsViewGridSizeMax.Y());

    if(theMapViewWinRegistry)
        theMapViewWinRegistry->ViewGridSizeStr(CtrlViewUtils::Point2String(itsViewGridSizeVM));

	// Then calculate the maximum map row start index (depends on the max grid size and the current grid)
	int maxStartIndex = CalcMaxRowStartingIndex();
	itsMapRowStartingIndex = FmiMin(maxStartIndex, itsMapRowStartingIndex);
	itsTrueMapViewSizeInfo.onViewGridSizeChange(itsViewGridSizeVM, IsTimeControlViewVisibleTotal());
}

void NFmiMapViewDescTop::MapRowStartingIndex(int newIndex)
{
	// Then calculate the maximum map row start index (depends on the max grid size and the current grid)
	int maxStartIndex = CalcMaxRowStartingIndex();
	itsMapRowStartingIndex = FmiMin(maxStartIndex, newIndex);
	MapViewDirty(false, false, true, true);
}

// Whether the given map row is now visible in the map view. TheRowIndex is a row index whose value starts from 1.
bool NFmiMapViewDescTop::IsVisibleRow(int theRowIndex)
{
	if(theRowIndex < itsMapRowStartingIndex)
		return false;
	if(theRowIndex > (itsMapRowStartingIndex + itsViewGridSizeVM.Y() - 1))
		return false;

	return true;
}

long NFmiMapViewDescTop::TimeControlTimeStepInMinutes() const
{
	return boost::math::iround(TimeControlTimeStep() * 60.f);
}

// The MapViewDirty method does everything that the AreaViewDirty and
// MapDirty methods used to do together in a confusing way. They were used inconsistently in different situations and together (again inconsistently).
// One of the worst confusions was caused by setting the fAreaViewDirty variable, which indirectly went
// false via the MapDirty method if the mapDirty and clearCache parameters were both true.
// In the new method no such confusion can occur.
// ======================================================
// Below are the explanations of the parameters:
// ======================================================
// 1. makeNewBackgroundBitmap, if true, there is a need to compute a new zoom for the base map or the base map changes,
//    and at the same time redraw the base map into the cache.
// 2. clearMapViewBitmapCacheRows, if true, the map views' image caches are cleared completely. The image cache means the
//    combined result of the base map and the parameter draw layers on the different view rows at different time steps.
//    This cache image does not include the so-called DrawOverBitmapThings, which draw various markers onto the map views.
// 3. the redrawMapView parameter controls whether the map view is redrawn, or whether the drawing uses directly
//    the cache image stored by the map view classes (CSmartMetView and CFmiExtraMapView). This single cache image is
//    thus only in use at any given moment because a double-buffer image had to be made, if e.g. another program/window
//    is moved over the SmartMet views and they otherwise do not need to be updated.
// 4. the updateMapViewDrawingLayers parameter ensures that the map view classes make updates to the draw-layer structures.
void NFmiMapViewDescTop::MapViewDirty(bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool updateMapViewDrawingLayers)
{
    SetRedrawMapView(redrawMapView);
    MapHandler()->SetMakeNewBackgroundBitmap(makeNewBackgroundBitmap);
    MapHandler()->SetUpdateMapViewDrawingLayers(updateMapViewDrawingLayers);
    if(clearMapViewBitmapCacheRows)
    {
        // Laitetaan cache halutessa likaiseksi
        MapViewCache().MakeDirty();
        if(makeNewBackgroundBitmap)
        {
            GridPointCache().Clear();
        }
    }
}

// This function is given the area used for the 'atlas' in the client view, in pixels
// the function computes, using this size and the used map grid, the x/y ratio of one cell,
// which is then used e.g. in auto-zooming
void NFmiMapViewDescTop::CalcClientViewXperYRatio(const NFmiPoint& theViewSize)
{
	itsClientViewXperYRatio = (RelativeMapRect().Width() * theViewSize.X() / itsViewGridSizeVM.X()) /
							  (RelativeMapRect().Height() * theViewSize.Y() / itsViewGridSizeVM.Y());
}

void NFmiMapViewDescTop::MapViewSizeInPixels(const NFmiPoint& newSize, CDC* pDC, double theDrawObjectScaleFactor, bool fHideTimeControlView)
{
    itsMapViewSizeInPixels = newSize;
	auto timeControlViewIsHidden = fHideTimeControlView || !IsTimeControlViewVisible();

	itsTrueMapViewSizeInfo.onSize(newSize, pDC, itsViewGridSizeVM, !timeControlViewIsHidden, theDrawObjectScaleFactor);
	// Must not set itsRelativeMapRect if timeControlViewIsHidden is true,
	// because it would become the max-area box, i.e. (0,0-1,1). This is a special case and those are handled
	// in their own way in the RelativeMapRect methods. itsRelativeMapRect must not get that value!
    if(!timeControlViewIsHidden)
	{
		// At the same time adjust the relative portion that the map view takes, leaving the rest to the time control window.
		int wantedTimeControlHeightInPixels = FmiRound(TrueMapViewSizeInfo::calculateTimeControlViewHeightInPixels(itsTrueMapViewSizeInfo.logicalPixelsPerMilliMeter().X()));
	    double mapVerticalPortion = (newSize.Y() - wantedTimeControlHeightInPixels) / newSize.Y();
		auto modifiedRelativeMapRect = itsRelativeMapRect;
		modifiedRelativeMapRect.Height(mapVerticalPortion);
		RelativeMapRect(modifiedRelativeMapRect);
	}

    // Finally the x-y ratio must also be updated
    CalcClientViewXperYRatio(newSize);

    UpdateOneMapViewSize();
}

void NFmiMapViewDescTop::RecalculateMapViewSizeInPixels(double theDrawObjectScaleFactor)
{
	auto timeControlViewIsHidden = !IsTimeControlViewVisibleTotal();
	MapViewSizeInPixels(itsMapViewSizeInPixels, nullptr, theDrawObjectScaleFactor, timeControlViewIsHidden);
}

const NFmiRect& NFmiMapViewDescTop::RelativeMapRect(void)
{ 
    if(IsTimeControlViewVisibleTotal())
        return itsRelativeMapRect; 
    else
    {
        return g_TotalRelativeMapRect;
    }
}

void NFmiMapViewDescTop::RelativeMapRect(const NFmiRect& theMapRect) 
{ 
	// Setting the (0,0 - 1,1) rect is not allowed, that is a special case
	if(theMapRect != g_TotalRelativeMapRect)
		itsRelativeMapRect = theMapRect; 
}

bool NFmiMapViewDescTop::IsTimeControlViewVisible() const
{
	return itsShowTimeOnMapMode < 2;
}

// Four states:
// 0 = show time control window+text
// 1 = only time control window
// 2 = show neither
// 3 = show only text
// returns the current state
int NFmiMapViewDescTop::ToggleShowTimeOnMapMode(void)
{
	itsShowTimeOnMapMode++;
	if(itsShowTimeOnMapMode > 3)
		itsShowTimeOnMapMode = 0;
	// If e.g. some negative number has been read from a view macro, it must be corrected
	if(itsShowTimeOnMapMode < 0)
		itsShowTimeOnMapMode = 0;
	bool mapAreaDirty = false;
	switch(itsShowTimeOnMapMode)
	{
	case 0: // 0 = show time control window+text
		fShowTimeString = true;
		mapAreaDirty = true;
		break;
	case 1: // 1 = only time control window
		fShowTimeString = false;
		break;
	case 2: // 2 = show neither
		fShowTimeString = false;
		mapAreaDirty = true;
		break;
	case 3: // 3 = show only text
		fShowTimeString = true;
		break;
	}
	MapViewDirty(mapAreaDirty, true, true, false);
	if(mapAreaDirty)
	{
		// The window size changes in certain cases, so the border lines must be recalculated
		SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
	}
	// The map area sizes must also be updated if the time control window goes out of view or becomes visible again
	RecalculateMapViewSizeInPixels(CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->ApplicationWinRegistry().DrawObjectScaleFactor());
	return itsShowTimeOnMapMode;
}

void NFmiMapViewDescTop::ShowTimeOnMapMode(int newValue)
{
	// Make use of the checks and settings done by the
	// ToggleShowTimeOnMapMode method.
	itsShowTimeOnMapMode = newValue-1;
	ToggleShowTimeOnMapMode();
}

void NFmiMapViewDescTop::TimeControlTimeStep(float newValue)
{
	itsTimeControlTimeStep = newValue;
	// 0.02 is the same as a one-minute time step, which is the smallest possible
	if(itsTimeControlTimeStep < 0.02f)
		itsTimeControlTimeStep = 0.02f;
	itsAnimationData.TimeStepInMinutes(boost::math::iround(itsTimeControlTimeStep*60.f));
}

void NFmiMapViewDescTop::ToggleLandBorderColor(void)
{
	itsLandBorderColorIndex++;
	if(itsLandBorderColorIndex >= static_cast<int>(itsLandBorderColors.size()))
		// When it wraps around, turn on the 'empty' color
		itsLandBorderColorIndex = -1;
	SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Cosmetic);
}

// These border layer drawing methods that take the separateBorderLayerDrawOptions parameter
// work on the following principle:
// If the user wants information about the general border-draw settings, that parameter is nullptr.
// If it differs from nullptr, it concerns a separate border-layer whose information is fetched separately.
// *************************************************************************************************
const NFmiColor& NFmiMapViewDescTop::LandBorderColor(NFmiDrawParam* separateBorderLayerDrawOptions)
{
	if(separateBorderLayerDrawOptions)
		return separateBorderLayerDrawOptions->IsolineColor();

	if(itsLandBorderColorIndex >= 0 && itsLandBorderColorIndex < static_cast<int>(itsLandBorderColors.size()))
		return itsLandBorderColors[itsLandBorderColorIndex];
	// In an error situation return the 1st color
	return itsLandBorderColors[0];
}

bool NFmiMapViewDescTop::DrawLandBorders(NFmiDrawParam* separateBorderLayerDrawOptions)
{
	if(separateBorderLayerDrawOptions)
		return CombinedMapHandlerInterface::IsBorderLayerDrawn(separateBorderLayerDrawOptions);

	if(itsLandBorderColorIndex < 0)
		return false;
	return true;
}

int NFmiMapViewDescTop::LandBorderPenSize(NFmiDrawParam* separateBorderLayerDrawOptions) 
{ 
	if(separateBorderLayerDrawOptions)
		return NFmiMapViewDescTop::GetSeparateBorderLayerLineWidthInPixels(*separateBorderLayerDrawOptions);
		
	return static_cast<int>(itsLandBorderPenSize.X());
}

bool NFmiMapViewDescTop::BorderDrawBitmapDirty(NFmiDrawParam* separateBorderLayerDrawOptions) const
{ 
	if(separateBorderLayerDrawOptions)
	{
		auto cacheKey = NFmiMapViewDescTop::MakeSeparateBorderLayerCacheKey(*separateBorderLayerDrawOptions);
		const auto* bitmap = itsSeparateCountryBorderBitmapCache.getCacheBitmap(cacheKey);
		// If no image is found in the separate-layer cache, it is 'dirty'
		return (bitmap == nullptr);
	}
	else
		return (itsLandBorderMapBitmap == nullptr);
}

bool NFmiMapViewDescTop::BorderDrawPolylinesDirty() const
{
	return MapHandler()->BorderDrawPolylinesDirty();
}

bool NFmiMapViewDescTop::BorderDrawPolylinesGdiplusDirty() const
{
	return MapHandler()->BorderDrawPolylinesGdiplusDirty();
}

Gdiplus::Bitmap* NFmiMapViewDescTop::LandBorderMapBitmap(NFmiDrawParam* separateBorderLayerDrawOptions) const
{ 
	if(separateBorderLayerDrawOptions)
	{
		auto cacheKey = NFmiMapViewDescTop::MakeSeparateBorderLayerCacheKey(*separateBorderLayerDrawOptions);
		return itsSeparateCountryBorderBitmapCache.getCacheBitmap(cacheKey);
	}
	else
		return itsLandBorderMapBitmap; 
}

void NFmiMapViewDescTop::SetLandBorderMapBitmap(Gdiplus::Bitmap* newBitmap, NFmiDrawParam* separateBorderLayerDrawOptions)
{
	if(separateBorderLayerDrawOptions)
	{
		auto cacheKey = NFmiMapViewDescTop::MakeSeparateBorderLayerCacheKey(*separateBorderLayerDrawOptions);
		// The given newBitmap is taken into ownership here in a unique_ptr
		itsSeparateCountryBorderBitmapCache.insertCacheBitmap(cacheKey, std::unique_ptr<Gdiplus::Bitmap>(newBitmap));
	}
	else
	{
		delete itsLandBorderMapBitmap;
		itsLandBorderMapBitmap = newBitmap;
	}
}

// *************************************************************************************************

std::string NFmiMapViewDescTop::MakeSeparateBorderLayerCacheKey(const NFmiDrawParam& borderLayerDrawOptions)
{
	int usedLineWidthInPixels = GetSeparateBorderLayerLineWidthInPixels(borderLayerDrawOptions);
	auto usedBorderDrawColor = borderLayerDrawOptions.FrameColor();
	return NFmiMapViewDescTop::MakeSeparateBorderLayerCacheKey(usedLineWidthInPixels, usedBorderDrawColor);
}

int NFmiMapViewDescTop::GetSeparateBorderLayerLineWidthInPixels(const NFmiDrawParam& borderLayerDrawOptions)
{
	int usedLineWidthInPixels = boost::math::iround(borderLayerDrawOptions.SimpleIsoLineWidth());
	usedLineWidthInPixels = std::min(3, std::max(0, usedLineWidthInPixels));
	return usedLineWidthInPixels;
}

std::string NFmiMapViewDescTop::MakeSeparateBorderLayerCacheKey(int lineWidthInPixels, const NFmiColor& color)
{
	std::string keyString = std::to_string(lineWidthInPixels);
	keyString += "_";
	keyString += ::colorToHexaString(color);
	return keyString;
}


void NFmiMapViewDescTop::ToggleLandBorderPenSize(void)
{
	static const double maxPenSize = 3;
	if(itsLandBorderPenSize.X() >= maxPenSize)
		itsLandBorderPenSize.X(1);
	else
		itsLandBorderPenSize.X(itsLandBorderPenSize.X() + 1);

	if(itsLandBorderPenSize.Y() >= maxPenSize)
		itsLandBorderPenSize.Y(1);
	else
		itsLandBorderPenSize.Y(itsLandBorderPenSize.Y() + 1);

	SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Cosmetic);
}

const NFmiColor& NFmiMapViewDescTop::StationPointColor(void) const
{
	if(itsStationPointColorIndex >= 0 && itsStationPointColorIndex < static_cast<int>(itsLandBorderColors.size()))
		return itsLandBorderColors[itsStationPointColorIndex];
	// In an error situation return the 1st color
	return itsLandBorderColors[0];
}

const NFmiPoint& NFmiMapViewDescTop::StationPointSize(void) const
{
	return itsStationPointSize;
}

void NFmiMapViewDescTop::ToggleStationPointColor(void)
{
	itsStationPointColorIndex++;
	if(itsStationPointColorIndex >= static_cast<int>(itsLandBorderColors.size()))
		// When it wraps around, turn the 1st color back on
		itsStationPointColorIndex = 0;
	if(fShowStationPlotVM)
		// Also mark all times dirty from the cache
		MapViewDirty(false, true, true, false);
}

void NFmiMapViewDescTop::ToggleStationPointSize(void)
{
	const double maxSize = 4;
	if(itsStationPointSize.X() >= maxSize)
		itsStationPointSize.X(1);
	else
		itsStationPointSize.X(itsStationPointSize.X() + 1);

	if(itsStationPointSize.Y() >= maxSize)
		itsStationPointSize.Y(1);
	else
		itsStationPointSize.Y(itsStationPointSize.Y() + 1);

	if(fShowStationPlotVM)
		// Also mark all times dirty from the cache
		MapViewDirty(false, true, true, false);
}

// This sets the new grid size of the map view.
// makes the necessary 'dirtyings' and returns true if
// the views need to be updated, otherwise false (i.e. the grid did not change).
bool NFmiMapViewDescTop::SetMapViewGrid(const NFmiPoint &newValue, NFmiMapViewWinRegistry *theMapViewWinRegistry)
{
	NFmiPoint oldSize(itsViewGridSizeVM);
	ViewGridSize(newValue, theMapViewWinRegistry);
	// NOTE! the start row must also be adjusted
	MapRowStartingIndex(MapRowStartingIndex());
	if(oldSize != itsViewGridSizeVM)
	{
		CRect aRect;
		itsMapView->GetClientRect(aRect);
		CalcClientViewXperYRatio(NFmiPoint(aRect.Width(), aRect.Height()));
        UpdateOneMapViewSize();

		MapViewDirty(true, true, true, false);
		SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
		return true;
	}
	return false;
}

void NFmiMapViewDescTop::UpdateOneMapViewSize()
{
    // Also recompute the size of one map cell in mm
    NFmiPoint oneMapViewSizeInPixels = ActualMapBitmapSizeInPixels();
    itsGraphicalInfo.itsViewWidthInMM = oneMapViewSizeInPixels.X() / itsGraphicalInfo.itsPixelsPerMM_x;
    itsGraphicalInfo.itsViewHeightInMM = oneMapViewSizeInPixels.Y() / itsGraphicalInfo.itsPixelsPerMM_y;
	itsTrueMapViewSizeInfo.onViewGridSizeChange(itsViewGridSizeVM, IsTimeControlViewVisibleTotal());
}

void NFmiMapViewDescTop::SetSelectedMapsFromSettings(void)
{
	for(size_t i=0; i<itsGdiPlusImageMapHandlerList.size(); i++)
		SetGdiPlusImageMapHandlerSelectedMaps(*itsGdiPlusImageMapHandlerList[i], static_cast<int>(i));
}

void NFmiMapViewDescTop::SetGdiPlusImageMapHandlerSelectedMaps(NFmiGdiPlusImageMapHandler &theGdiPlusImageMapHandler, int mapHandlerIndex)
{
	string usedMapIndexKey(itsSettingsBaseName);
	usedMapIndexKey += "SelectedMapImage";
	usedMapIndexKey += NFmiStringTools::Convert<int>(mapHandlerIndex);

	int mapIndex = NFmiSettings::Require<int>(usedMapIndexKey.c_str());
	if(mapIndex < 0)
		mapIndex = 0;
	theGdiPlusImageMapHandler.UsedMapIndex(mapIndex);

	string usedOverlayIndexKey(itsSettingsBaseName);
	usedOverlayIndexKey += "SelectedMapOverlayImage";
	usedOverlayIndexKey += NFmiStringTools::Convert<int>(mapHandlerIndex);

	int overlayIndex = NFmiSettings::Require<int>(usedOverlayIndexKey.c_str());
	if(overlayIndex < -1)
		overlayIndex = -1;
	theGdiPlusImageMapHandler.OverMapBitmapIndex(overlayIndex);
}

void NFmiMapViewDescTop::StoreHandlerSelectedMapsToSettings(void)
{
	for(size_t i=0; i<itsGdiPlusImageMapHandlerList.size(); i++)
		StoreHandlerSelectedMapsToSettings(*itsGdiPlusImageMapHandlerList[i], static_cast<int>(i));
}

void NFmiMapViewDescTop::StoreHandlerSelectedMapsToSettings(NFmiGdiPlusImageMapHandler &theGdiPlusImageMapHandler, int mapHandlerIndex)
{
	string usedMapIndexKey(itsSettingsBaseName);
	usedMapIndexKey += "SelectedMapImage";
	usedMapIndexKey += NFmiStringTools::Convert<int>(mapHandlerIndex);
	int mapIndex = theGdiPlusImageMapHandler.UsedMapIndex();
	NFmiSettings::Set(usedMapIndexKey.c_str(), NFmiStringTools::Convert<int>(mapIndex).c_str(), true);

	string usedOverlayIndexKey(itsSettingsBaseName);
	usedOverlayIndexKey += "SelectedMapOverlayImage";
	usedOverlayIndexKey += NFmiStringTools::Convert<int>(mapHandlerIndex);
	int overlayIndex = theGdiPlusImageMapHandler.OverMapBitmapIndex();
	NFmiSettings::Set(usedOverlayIndexKey.c_str(), NFmiStringTools::Convert<int>(overlayIndex).c_str(), true);
}


void NFmiMapViewDescTop::InitStationPointDrawingSystem(void)
{
	itsStationPointColorIndex = NFmiSettings::Require<int>("MetEditor::StationPointColorIndex");
	int stationPointSize = NFmiSettings::Require<int>("MetEditor::StationPointSize");
	itsStationPointSize = NFmiPoint(stationPointSize, stationPointSize);
}

void NFmiMapViewDescTop::ToggleMapViewDisplayMode(void)
{
    if(itsMapViewDisplayMode == CtrlViewUtils::MapViewMode::kNormal)
        itsMapViewDisplayMode = CtrlViewUtils::MapViewMode::kOneTime;
    else if(itsMapViewDisplayMode == CtrlViewUtils::MapViewMode::kOneTime)
        itsMapViewDisplayMode = CtrlViewUtils::MapViewMode::kRunningTime;
    else
        itsMapViewDisplayMode = CtrlViewUtils::MapViewMode::kNormal;

	MapViewDirty(false, true, true, false);
	ViewGridSize(itsViewGridSizeVM, nullptr);
}

void NFmiMapViewDescTop::InitForViewMacro(const NFmiMapViewDescTop& theOther, NFmiMapViewWinRegistry& theMapViewWinRegistry, bool getFromRegisty, bool disableWindowManipulations)
{
	itsLandBorderColorIndex = theOther.itsLandBorderColorIndex;
	itsParamWindowViewPosition = theOther.itsParamWindowViewPosition;

	itsLandBorderPenSize = theOther.itsLandBorderPenSize;

	RelativeMapRect(theOther.itsRelativeMapRect);

	// The drawparam lists are not initialized here!!!!
//	itsDrawParamListVector = theOther.itsDrawParamListVector;
//	itsExtraDrawParamListVector = theOther.itsExtraDrawParamListVector;

	itsDrawOverMapMode = theOther.itsDrawOverMapMode;
	itsMapRowStartingIndex = theOther.itsMapRowStartingIndex;
	itsShowTimeOnMapMode = theOther.itsShowTimeOnMapMode;
	fShowTimeString = theOther.fShowTimeString;

	itsCurrentTime = theOther.itsCurrentTime;

	itsStationPointColorIndex = theOther.itsStationPointColorIndex;

	itsStationPointSize = theOther.itsStationPointSize;

	if(getFromRegisty)
		InitFromMapViewWinRegistry(theMapViewWinRegistry);
	else
	{
		// Store the value from the view macro to both the local value and the registry
		fShowMasksOnMapVM = theOther.fShowMasksOnMapVM;
		theMapViewWinRegistry.ShowMasksOnMap(theOther.fShowMasksOnMapVM);
		// Store the value from the view macro to both the local value and the registry
		itsSpacingOutFactorVM = theOther.itsSpacingOutFactorVM;
		theMapViewWinRegistry.SpacingOutFactor(theOther.itsSpacingOutFactorVM);
		// This index must be checked, so it goes through the setter method
		SelectedMapIndex(theOther.itsSelectedMapIndexVM);
		theMapViewWinRegistry.SelectedMapIndex(itsSelectedMapIndexVM);

		fShowStationPlotVM = theOther.fShowStationPlotVM;
		theMapViewWinRegistry.ShowStationPlot(theOther.fShowStationPlotVM);
		itsViewGridSizeVM = theOther.itsViewGridSizeVM;
		theMapViewWinRegistry.ViewGridSizeStr(CtrlViewUtils::Point2String(theOther.itsViewGridSizeVM));
		TimeBoxLocation(theOther.itsTimeBoxLocationVM);
		TimeBoxTextSizeFactor(theOther.itsTimeBoxTextSizeFactorVM);
		TimeBoxFillColor(theOther.itsTimeBoxFillColorVM);
	}
	itsTimeControlTimeStep = theOther.itsTimeControlTimeStep;
	itsMapViewDisplayMode = theOther.itsMapViewDisplayMode;
	itsAbsoluteActiveViewRow = theOther.itsAbsoluteActiveViewRow;

	if(!disableWindowManipulations)
	{
		fDescTopOn = theOther.fDescTopOn;
	}
	fLockToMainMapViewTime = theOther.fLockToMainMapViewTime;
	fLockToMainMapViewRow = theOther.fLockToMainMapViewRow;
	fShowTrajectorsOnMap = theOther.fShowTrajectorsOnMap;
	fShowSoundingMarkersOnMap = theOther.fShowSoundingMarkersOnMap;
	fShowCrossSectionMarkersOnMap = theOther.fShowCrossSectionMarkersOnMap;

	fShowSelectedPointsOnMap = theOther.fShowSelectedPointsOnMap;
	fShowControlPointsOnMap = theOther.fShowControlPointsOnMap;
	fShowObsComparisonOnMap = theOther.fShowObsComparisonOnMap;
	fShowWarningMarkersOnMap = theOther.fShowWarningMarkersOnMap;

	// From old macros this gets an empty time bag, which must be ignored
	if(theOther.itsTimeControlViewTimes.Size() > 1)
		itsTimeControlViewTimes = theOther.itsTimeControlViewTimes;
	itsAnimationData = theOther.itsAnimationData;
}

// NOTE!! This includes comments!
void NFmiMapViewDescTop::Write(std::ostream& os) const
{
	os << "// NFmiMapViewDescTop::Write..." << std::endl;

	os << "// SelectedMapIndex" << std::endl;
	os << itsSelectedMapIndexVM << std::endl;

	os << "// LandBorderColorIndex + ShowParamWindowView" << std::endl;
	auto showParamWindowView = (itsParamWindowViewPosition != kNoDirection);
	os << itsLandBorderColorIndex << " " << showParamWindowView << std::endl;

	os << "// LandBorderPenSize" << std::endl;
	os << itsLandBorderPenSize;

	os << "// RelativeMapRect" << std::endl;
	os << itsRelativeMapRect;

	// This is stored separately in the view macro class
	// itsDrawParamListVector;
	// This is stored separately in the view macro class
	// itsExtraDrawParamListVector;

	os << "// DrawOverMapMode + MapRowStartingIndex + ShowTimeOnMapMode + ShowTimeString" << std::endl;
	os << itsDrawOverMapMode << " " << itsMapRowStartingIndex << " " << itsShowTimeOnMapMode << " " << fShowTimeString << std::endl;

	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	os << "// selected time with utc hour + minute + day shift to current day" << std::endl;
	NFmiDataStoringHelpers::WriteTimeWithOffsets(usedViewMacroTime, itsCurrentTime, os);

	os << "// ViewGridSize" << std::endl;
    os << itsViewGridSizeVM;

	os << "// ShowStationPlot + StationPointColorIndex" << std::endl;
	os << fShowStationPlotVM << " " << itsStationPointColorIndex << std::endl;

	os << "// StationPointSize" << std::endl;
	os << itsStationPointSize;

	os << "// ShowMasksOnMap + TimeControlTimeStep + SpacingOutFactor + MapViewDisplayMode" << std::endl;
	os << fShowMasksOnMapVM << " " << itsTimeControlTimeStep << " " << itsSpacingOutFactorVM << " " << static_cast<int>(itsMapViewDisplayMode) << std::endl;

	os << "// DescTopOn + LockToMainMapViewTime + ShowTrajectorsOnMap + ShowSoundingMarkersOnMap + ShowCrossSectionMarkersOnMap" << std::endl;
	os << fDescTopOn << " " << fLockToMainMapViewTime << " " << fShowTrajectorsOnMap << " " << fShowSoundingMarkersOnMap << " " << fShowCrossSectionMarkersOnMap << std::endl;

	os << "// ShowSelectedPointsOnMap + ShowControlPointsOnMap + ShowObsComparisonOnMap + ShowWarningMarkersOnMap + ShowRedCrossOverMapIfNotEditedDataTime" << std::endl;
    // For backward/forward compatibility of view macros, this must be stored as false
    bool legacy_fShowRedCrossOverMapIfNotEditedDataTime = false;
	os << fShowSelectedPointsOnMap << " " << fShowControlPointsOnMap << " " << fShowObsComparisonOnMap << " " << fShowWarningMarkersOnMap << " " << legacy_fShowRedCrossOverMapIfNotEditedDataTime << std::endl;

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.

    // Adding 'double' format extra data
	// Store as the 1st extra data the parameter row lock
	extraData.Add(static_cast<double>(fLockToMainMapViewRow));
	// Store as the 2nd extra data the parameter box position
	extraData.Add(static_cast<double>(itsParamWindowViewPosition));
	// Store as the 3rd extra data the time box position
	extraData.Add(static_cast<double>(itsTimeBoxLocationVM));
	// Store as the 4th extra data the time box text size factor
	extraData.Add(static_cast<double>(itsTimeBoxTextSizeFactorVM));

    // Adding 'string' format extra data
    std::stringstream extraDataStream;
    itsAnimationData.Write(extraDataStream);
	// Store as the 1st extra data the animation-related data as a single string
	extraData.Add(extraDataStream.str());
	std::string timeBagStr = NFmiDataStoringHelpers::GetTimeBagOffSetStr(usedViewMacroTime, *(TimeControlViewTimes().ValidTimeBag()));
	// Add as the 2nd extra data the time window's timebag as an offset to the current time
	extraData.Add(timeBagStr);
	// Add as the 3rd extra data the map's time legend background color as text
	extraData.Add(NFmiDrawParam::Color2String(itsTimeBoxFillColorVM));

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiMapViewDescTop::Write failed");
}

// NOTE!! before this class is read in from a file, remove the comments
// with the NFmiCommentStripper class, because comments are inserted
// when writing. So do not use this method directly, but the Init(filename) method!!!!
void NFmiMapViewDescTop::Read(std::istream& is)
{
	is >> itsSelectedMapIndexVM;

	bool showParamWindowView = true;
	is >> itsLandBorderColorIndex >> showParamWindowView;

	is >> itsLandBorderPenSize;
	NFmiRect tmpMapRect;
	is >> tmpMapRect;
	RelativeMapRect(tmpMapRect);

	// This is stored separately in the view macro class
	// itsDrawParamListVector;
	// This is stored separately in the view macro class
	// itsExtraDrawParamListVector;

	is >> itsDrawOverMapMode >> itsMapRowStartingIndex >> itsShowTimeOnMapMode >> fShowTimeString;

	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	NFmiDataStoringHelpers::ReadTimeWithOffsets(usedViewMacroTime, itsCurrentTime, is);

    is >> itsViewGridSizeVM;

	is >> fShowStationPlotVM >> itsStationPointColorIndex;

	is >> itsStationPointSize;

    int displayModeTmp = 0;
    is >> fShowMasksOnMapVM >> itsTimeControlTimeStep >> itsSpacingOutFactorVM >> displayModeTmp;
    itsMapViewDisplayMode = static_cast<CtrlViewUtils::MapViewMode>(displayModeTmp);

	is >> fDescTopOn >> fLockToMainMapViewTime >> fShowTrajectorsOnMap >> fShowSoundingMarkersOnMap >> fShowCrossSectionMarkersOnMap;

    // For backward/forward compatibility of view macros, this must be read
    bool legacy_fShowRedCrossOverMapIfNotEditedDataTime = false;
    is >> fShowSelectedPointsOnMap >> fShowControlPointsOnMap >> fShowObsComparisonOnMap >> fShowWarningMarkersOnMap >> legacy_fShowRedCrossOverMapIfNotEditedDataTime;

	if(is.fail())
		throw std::runtime_error("NFmiMapViewDescTop::Read failed");

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

    // Picking 'double' format extra data

	// Read as the 1st extra data the parameter row lock; the default for fLockToMainMapViewRow is false
	fLockToMainMapViewRow = false;
    if(extraData.itsDoubleValues.size() >= 1)
        // (NOTE! cannot static_cast to bool, because VC++ gives a friendly warning about slowness)
        fLockToMainMapViewRow = extraData.itsDoubleValues[0] != 0;

	// Read as the 2nd extra data the parameter box position; the default for itsParamWindowViewPosition potentially depends
	// on the original setting read into the showParamWindowView variable of a view macro made with an old version.
	itsParamWindowViewPosition = showParamWindowView ? kTopLeft : kNoDirection;
	if(extraData.itsDoubleValues.size() >= 2)
		itsParamWindowViewPosition = static_cast<FmiDirection>(extraData.itsDoubleValues[1]);

	// Read as the 3rd extra data the time box position
	itsTimeBoxLocationVM = kBottomLeft;
	if(extraData.itsDoubleValues.size() >= 3)
		TimeBoxLocation(static_cast<FmiDirection>(extraData.itsDoubleValues[2]));

	// Read as the 4th extra data the time box text size factor
	itsTimeBoxTextSizeFactorVM = 1;
	if(extraData.itsDoubleValues.size() >= 4)
		TimeBoxTextSizeFactor(static_cast<float>(extraData.itsDoubleValues[3]));

    // Picking 'string' format extra data
    if(extraData.itsStringValues.size() >= 1)
	{// read as the 1st extra data the animation-related data as a single string
		if(is.fail())
			throw std::runtime_error("NFmiMapViewDescTop::Read failed");
		std::stringstream extraDataStream;
		extraDataStream.str(extraData.itsStringValues[0]);
		itsAnimationData.Read(extraDataStream);
	}
	if(extraData.itsStringValues.size() >= 2)
	{// read as the 2nd extra data the time window's timebag as an offset to the current time
		if(is.fail())
			throw std::runtime_error("NFmiMapViewDescTop::Read failed");
		NFmiTimeBag times = NFmiDataStoringHelpers::GetTimeBagOffSetFromStr(usedViewMacroTime, extraData.itsStringValues[1]);
		TimeControlViewTimes(NFmiTimeDescriptor(times.FirstTime(), times));
	}
	itsTimeBoxFillColorVM = TimeBoxFillColorDefault;
	if(extraData.itsStringValues.size() >= 3)
	{
		// Read as the 3rd extra data the time legend background coloring
		if(is.fail())
			throw std::runtime_error("NFmiMapViewDescTop::Read failed");
		itsTimeBoxFillColorVM = NFmiDrawParam::String2Color(extraData.itsStringValues[2]);
	}

	if(is.fail())
		throw std::runtime_error("NFmiMapViewDescTop::Read failed");
}

std::vector<NFmiMapViewDescTop::ViewMacroDipMapHelper> NFmiMapViewDescTop::GetViewMacroDipMapHelperList(void)
{
	std::vector<NFmiMapViewDescTop::ViewMacroDipMapHelper> helperList;

	NFmiMapViewDescTop::ViewMacroDipMapHelper tmpData;
	for(unsigned int mapAreaIndex = 0; mapAreaIndex < itsGdiPlusImageMapHandlerList.size(); mapAreaIndex++)
	{
		NFmiGdiPlusImageMapHandler &mapHandler = *(itsGdiPlusImageMapHandlerList[mapAreaIndex]);
		tmpData.itsUsedMapIndex = mapHandler.UsedMapIndex();
		tmpData.itsUsedOverMapDibIndex = mapHandler.OverMapBitmapIndex();
		auto& combinedMapHandler = CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->GetCombinedMapHandlerInterface();
		tmpData.itsUsedCombinedModeMapIndex = combinedMapHandler.getCombinedMapModeState(itsMapViewDescTopIndex, mapAreaIndex).combinedModeMapIndex();
		tmpData.itsUsedCombinedModeOverMapDibIndex = combinedMapHandler.getCombinedOverlayMapModeState(itsMapViewDescTopIndex, mapAreaIndex).combinedModeMapIndex();
		tmpData.itsZoomedAreaStr = mapHandler.Area() ? mapHandler.Area()->AreaStr() : "";
		auto macroReferenceNamePair = combinedMapHandler.getMacroReferenceNamesForViewMacro(itsMapViewDescTopIndex, mapAreaIndex);
		tmpData.itsBackgroundMacroReference = macroReferenceNamePair.first;
		tmpData.itsOverlayMacroReference = macroReferenceNamePair.second;
		helperList.push_back(tmpData);
	}

	return helperList;
}

void NFmiMapViewDescTop::SetViewMacroDipMapHelperList(const std::vector<NFmiMapViewDescTop::ViewMacroDipMapHelper> &theData)
{
	auto ssize1 = static_cast<unsigned int>(itsGdiPlusImageMapHandlerList.size());
	auto ssize2 = static_cast<unsigned int>(theData.size());
	unsigned int usedSize = std::min(ssize1, ssize2);
	for(unsigned int mapAreaIndex = 0; mapAreaIndex < usedSize; mapAreaIndex++)
	{
		const NFmiMapViewDescTop::ViewMacroDipMapHelper &tmpData = theData[mapAreaIndex];
		NFmiGdiPlusImageMapHandler &mapHandler = *(itsGdiPlusImageMapHandlerList[mapAreaIndex]);

		mapHandler.UsedMapIndex(tmpData.itsUsedMapIndex);
		mapHandler.OverMapBitmapIndex(tmpData.itsUsedOverMapDibIndex);

		auto& combinedMapHandler = CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->GetCombinedMapHandlerInterface();
		combinedMapHandler.selectCombinedMapModeIndices(itsMapViewDescTopIndex, mapAreaIndex, tmpData.itsUsedCombinedModeMapIndex, tmpData.itsUsedCombinedModeOverMapDibIndex);
		combinedMapHandler.selectMapLayersByMacroReferenceNamesFromViewMacro(itsMapViewDescTopIndex, mapAreaIndex, tmpData.itsBackgroundMacroReference, tmpData.itsOverlayMacroReference);

		mapHandler.Area(NFmiAreaFactory::Create(static_cast<char*>(tmpData.itsZoomedAreaStr)));
	}
}

// Calculates the size of one cell of the view grid in pixels
NFmiPoint NFmiMapViewDescTop::ActualMapBitmapSizeInPixels(void)
{
	NFmiPoint returnValue(MapViewSizeInPixels().X() * RelativeMapRect().Width() / itsViewGridSizeVM.X()
						 ,MapViewSizeInPixels().Y() * RelativeMapRect().Height() / itsViewGridSizeVM.Y());
	return returnValue;
}

void NFmiMapViewDescTop::TimeControlViewTimes(const NFmiTimeDescriptor &newTimeDescriptor)
{
	if(newTimeDescriptor.UseTimeList())
	{
		// Must make a timebag-type time descriptor if the given one was timelist-type
        int usedTimeStepInMinutes = boost::math::iround(itsTimeControlTimeStep * 60);
		if(usedTimeStepInMinutes > 60)
			// But at most a 60-minute time step
			usedTimeStepInMinutes = 60;
		NFmiMetTime newStartTime = newTimeDescriptor.FirstTime();
		newStartTime.SetTimeStep(usedTimeStepInMinutes, true);
		NFmiMetTime newEndTime = newTimeDescriptor.LastTime();
		newEndTime.SetTimeStep(usedTimeStepInMinutes, true);
		if(newStartTime >= newEndTime)
		{
			newEndTime = newStartTime;
			newEndTime.NextMetTime();
		}
        int diffInHours = newEndTime.DifferenceInHours(newStartTime);
        // No more than a two-year time window here, this is probably an error???
        if(diffInHours > 2*24*366)
		{
			newEndTime = newStartTime;
			newEndTime.NextMetTime();
		}
		NFmiTimeDescriptor timeDesc(newTimeDescriptor.OriginTime(), NFmiTimeBag(newStartTime, newEndTime, usedTimeStepInMinutes));
		itsTimeControlViewTimes = timeDesc;
	}
	else
		itsTimeControlViewTimes = newTimeDescriptor;
}

int NFmiMapViewDescTop::CalcPrintingPageShiftInMinutes(void)
{
	NFmiPoint gridSize = itsViewGridSizeVM;
    int usedTimeStepInMinutes = boost::math::iround(TimeControlTimeStep() * 60.f);
	int printingPageShiftInMinutes = usedTimeStepInMinutes;
	if(itsMapViewDisplayMode == CtrlViewUtils::MapViewMode::kNormal)
		printingPageShiftInMinutes = static_cast<int>(usedTimeStepInMinutes * gridSize.X());
	else if(itsMapViewDisplayMode == CtrlViewUtils::MapViewMode::kOneTime)
		printingPageShiftInMinutes = usedTimeStepInMinutes;
	else if(itsMapViewDisplayMode == CtrlViewUtils::MapViewMode::kRunningTime)
		printingPageShiftInMinutes = static_cast<int>(usedTimeStepInMinutes * gridSize.X() * gridSize.Y());

	return printingPageShiftInMinutes;
}

void NFmiMapViewDescTop::MapViewDisplayMode(CtrlViewUtils::MapViewMode newValue)
{
	itsMapViewDisplayMode = newValue;

	// Finally call the scroll method with a 0 shift. This sets the view rows to what
	// they can be in the current mode. E.g. if we are first in one-time mode on row 35
	// and switch to one-time-per-column, i.e. normal mode, then we cannot be on row 35, but
	// the start row must be adjusted so that it fits within the first 5 rows depending on the grid size.
	ScrollViewRow(0);
}

// Adjust all time-related things to the time given as a parameter, so that SmartMet adapts as well as possible to the loaded CaseStudy data.
void NFmiMapViewDescTop::SetCaseStudyTimes(const NFmiMetTime &theCaseStudyTime)
{
	itsTimeControlViewTimes.SetNewStartTime(theCaseStudyTime);
	itsCurrentTime = theCaseStudyTime;
}

std::list<NFmiPolyline*>& NFmiMapViewDescTop::DrawBorderPolyLineList()
{
	return MapHandler()->DrawBorderPolyLineList();
}

void NFmiMapViewDescTop::DrawBorderPolyLineList(std::list<NFmiPolyline*>& newPolyline)
{
	MapHandler()->DrawBorderPolyLineList(newPolyline);
}

const std::list<std::vector<NFmiPoint>>& NFmiMapViewDescTop::DrawBorderPolyLineListGdiplus()
{
	return MapHandler()->DrawBorderPolyLineListGdiplus();
}

void NFmiMapViewDescTop::DrawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>>& newPolylines)
{
	MapHandler()->DrawBorderPolyLineListGdiplus(newPolylines);
}

void NFmiMapViewDescTop::DrawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>>&& newPolylines)
{
	MapHandler()->DrawBorderPolyLineListGdiplus(newPolylines);
}

void NFmiMapViewDescTop::SetBorderDrawDirtyState(CountryBorderDrawDirtyState newState, NFmiDrawParam* separateBorderLayerDrawOptions)
{
	std::string cacheKey = separateBorderLayerDrawOptions ? MakeSeparateBorderLayerCacheKey(*separateBorderLayerDrawOptions) : "";
	SetBorderDrawDirtyState(newState, cacheKey);
}

void NFmiMapViewDescTop::SetBorderDrawDirtyState(CountryBorderDrawDirtyState newState, const std::string& cacheKey)
{
	MapHandler()->SetBorderDrawDirtyState(newState);
	if(newState != CountryBorderDrawDirtyState::None)
		ClearBaseLandBorderMapBitmap();

	itsSeparateCountryBorderBitmapCache.setBorderDrawDirtyState(newState, cacheKey);
}

void NFmiMapViewDescTop::ClearBaseLandBorderMapBitmap()
{
	delete itsLandBorderMapBitmap;
	itsLandBorderMapBitmap = nullptr;
}

const Gdiplus::Bitmap* NFmiMapViewDescTop::GetSeparateBorderLayerCacheBitmap(const std::string& cacheKeyString)
{
	return itsSeparateCountryBorderBitmapCache.getCacheBitmap(cacheKeyString);
}

void NFmiMapViewDescTop::InsertSeparateBorderLayerCacheBitmap(const std::string& cacheKeyString, std::unique_ptr<Gdiplus::Bitmap>&& cacheBitmap)
{
	itsSeparateCountryBorderBitmapCache.insertCacheBitmap(cacheKeyString, std::move(cacheBitmap));
}

std::string NFmiMapViewDescTop::GetCurrentGuiMapLayerText(bool backgroundMap)
{
	return MapHandler()->GetCurrentGuiMapLayerText(backgroundMap);
}


// I reworked the map dirtying systems in several ways:
// 1. The name changed, hopefully to a more descriptive one, i.e. RedrawMapView, meaning
//    that on the next drawing round the view must be redrawn, and the double buffer bitmap must not just be used.
// 2. The so-called set function can only turn the flag on; a false value is ignored. Only the clear function
//    can reset the flag's state. This change is because different places may provide different values in the same dirtying round,
//    and on the other hand a false setting made in one place should not reset a true setting made in another.
// 3. The flag is thus reset with the clear function, which is called after the drawing operations.
bool NFmiMapViewDescTop::RedrawMapView(void) const
{ 
    return fRedrawMapView; 
}

void NFmiMapViewDescTop::SetRedrawMapView(bool newValue) 
{ 
    if(newValue)
        fRedrawMapView = newValue; 
}

void NFmiMapViewDescTop::ClearRedrawMapView() 
{ 
    fRedrawMapView = false; 
}

double NFmiMapViewDescTop::SingleMapViewHeightInMilliMeters() const
{
	return itsTrueMapViewSizeInfo.singleMapSizeInMM().Y();
}

void NFmiMapViewDescTop::ParamWindowViewPositionChange(bool forward)
{
	itsParamWindowViewPosition = CtrlViewUtils::CalcFollowingParamWindowViewPosition(itsParamWindowViewPosition, forward);
}

void NFmiMapViewDescTop::TimeBoxPositionChange()
{
	TimeBoxLocation(CtrlViewUtils::MoveTimeBoxPositionForward(itsTimeBoxLocationVM));
}

bool NFmiMapViewDescTop::IsTimeControlViewVisibleTotal() const 
{ 
	return !fPrintingModeOn && IsTimeControlViewVisible(); 
}

void NFmiMapViewDescTop::TimeBoxLocation(FmiDirection newLocation)
{
	switch(newLocation)
	{
	case kBottomLeft:
	case kBottomCenter:
	case kBottomRight:
	case kTopLeft:
	case kTopCenter:
	case kTopRight:
	{
		itsTimeBoxLocationVM = newLocation;
		break;
	}
	default:
		itsTimeBoxLocationVM = kBottomLeft;
		break;
	}
}

void NFmiMapViewDescTop::TimeBoxTextSizeFactor(float newFactor) 
{
	itsTimeBoxTextSizeFactorVM = CtrlViewUtils::GetClosestValueFromContainer(newFactor, itsTimeBoxTextSizeAllowedFactors);
}

void NFmiMapViewDescTop::SetTimeBoxFillColor(NFmiColor newColorNotAlpha)
{
	// First set the alpha on the new color
	newColorNotAlpha.Alpha(itsTimeBoxFillColorVM.Alpha());
	// Only then replace the old color entirely with the new one
	itsTimeBoxFillColorVM = newColorNotAlpha;
}

void NFmiMapViewDescTop::SetTimeBoxFillColorAlpha(float newAlpha)
{
	if(newAlpha < 0)
		newAlpha = 0;
	if(newAlpha > 1)
		newAlpha = 1;
	itsTimeBoxFillColorVM.Alpha(newAlpha);
}
