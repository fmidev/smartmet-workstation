// ======================================================================
/*!
 * \file NFmiMapViewDescTop.cpp
 * \brief This class handles all the thing required to show individual map-views.
 * There will be several different mapviews in editor and GeneralDocument contains
 * vector of these objects.
 */
// ======================================================================

#pragma warning( push )
#pragma warning( disable : 4458 ) // mm. gdiplusheaders.h tekee l‰j‰n meille t‰ysin turhia "declaration hides class member" varoituksia

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

	// Muuttaa suhteellisen v‰ri kanava arvon (arvov‰li 0-1) 8-bit arvoksi (arvov‰li 0-255)
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
		// N‰m‰ stream manipulaattorit ovat voimassa vain yhden arvon tulostuksen ajan, joten jokaiselle v‰rikanavalle piti tehd‰ oma tulostusrivi
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

	std::string Point2String(const NFmiPoint& p)
	{
		std::stringstream out;
		out << p.X() << "," << p.Y(); // tein oman tavan kirjoittaa NFmiPoint, koska en halunnut rivinvaihtoa loppuun
		return out.str();
	}

	NFmiPoint String2Point(const std::string& str)
	{
		std::stringstream in(str);
		double x = 0;
		double y = 0;
		char ch = 0; // pilkku pit‰‰ lukea v‰list‰
		in >> x >> ch >> y;
		return NFmiPoint(x, y);
	}

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

// ************* ViewMacroDipMapHelper  -luokka *******************************************


NFmiMapViewDescTop::ViewMacroDipMapHelper::ViewMacroDipMapHelper() = default;


// ************************************************************************
// ******* NFmiMapViewDescTop::ViewMacroDipMapHelper -luokka **************
// ************************************************************************

// HUOM!! T‰m‰ laittaa kommentteja mukaan!
void NFmiMapViewDescTop::ViewMacroDipMapHelper::Write(std::ostream& os) const
{
	os << "// NFmiMapViewDescTop::ViewMacroDipMapHelper::Write..." << std::endl;

	os << "// UsedMapIndex + UsedOverMapDibIndex" << std::endl;
	os << itsUsedMapIndex << " " << itsUsedOverMapDibIndex << std::endl;

	os << "// itsZoomedAreaStr" << std::endl;
	os << itsZoomedAreaStr << std::endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.

	// 'double' muotoisten lis‰datojen lis‰ys
	extraData.Add(itsUsedCombinedModeMapIndex); // talletetaan 1. extra-datana k‰ytetty combined-mode karttaindeksi
	extraData.Add(itsUsedCombinedModeOverMapDibIndex); // talletetaan 2. extra-datana k‰ytetty combined-mode overlay-karttaindeksi

	// 'string' muotoisten lis‰datojen lis‰ys
	extraData.Add(itsBackgroundMacroReference); // talletetaan 1. extra-datana mahdollinen taustakartan macro-referenssi nimi
	extraData.Add(itsOverlayMacroReference); // talletetaan 2. extra-datana mahdollinen overlay-kartan macro-referenssi nimi

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiMapViewDescTop::ViewMacroDipMapHelper::Write failed");
}

// HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
// sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
void NFmiMapViewDescTop::ViewMacroDipMapHelper::Read(std::istream& is)
{
	is >> itsUsedMapIndex >> itsUsedOverMapDibIndex;

	is >> itsZoomedAreaStr;

	if(is.fail())
		throw std::runtime_error("NFmiMapViewDescTop::ViewMacroDipMapHelper::Read failed");

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	// 'double' muotoisten lis‰datojen poiminta, alustetaan ensin datat oletusarvoilla, ja sitten katsotaan onko ne talletettu
	itsUsedCombinedModeMapIndex = itsUsedMapIndex; // oletuksena normi karttaindeksi
	itsUsedCombinedModeOverMapDibIndex = itsUsedOverMapDibIndex; // oletuksena normi karttaindeksi
	if(extraData.itsDoubleValues.size() >= 1)
		itsUsedCombinedModeMapIndex = static_cast<int>(extraData.itsDoubleValues[0]); // luetaan 1. extra-datana combined-mode karttaindeksi
	if(extraData.itsDoubleValues.size() >= 2)
		itsUsedCombinedModeOverMapDibIndex = static_cast<int>(extraData.itsDoubleValues[1]); // luetaan 2. extra-datana combined-mode overlay karttaindeksi

	// 'string' muotoisten lis‰datojen poiminta, alustetaan ensin datat oletusarvoilla, ja sitten katsotaan onko ne talletettu
	itsBackgroundMacroReference.clear();
	if(extraData.itsStringValues.size() >= 1)
		itsBackgroundMacroReference = extraData.itsStringValues[0]; // luetaan 1. extra-datana taustakartan macro-referenssi nimi
	itsOverlayMacroReference.clear();
	if(extraData.itsStringValues.size() >= 2)
		itsOverlayMacroReference = extraData.itsStringValues[1]; // luetaan 2. extra-datana overlay-kartan macro-referenssi nimi

	if(is.fail())
		throw std::runtime_error("NFmiMapViewDescTop::ViewMacroDipMapHelper::Read failed");
}


// ************************************************************************
// ******* NFmiMapViewDescTop -luokka *************************************
// ************************************************************************

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
,itsRelativeMapRect(0.,0.,1.,0.9) // t‰m‰n suhteellisen osan n‰ytˆst‰ valtaa kartasto, loput menee aikakontrolli ikkunalle
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
,itsRelativeMapRect(0.,0.,1.,0.9) // t‰m‰n suhteellisen osan n‰ytˆst‰ valtaa kartasto, loput menee aikakontrolli ikkunalle
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

// Huomioitavaa ett‰ copy-constructor ei tee t‰ydellist‰ kopioita.
// Varsinaiseen kopiointiin k‰ytet‰‰n sijoitus operatoria.
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
	, itsDrawParamListVector(new NFmiPtrList<NFmiDrawParamList>()) // Tehd‰‰n lopullinen kopiointi metodin rungossa
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
	, itsMapView(other.itsMapView) // MapView pointteri pit‰‰ kopioida t‰ss‰, se ei muutu ajon aikana
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

// Huomioitavaa ett‰ sijoitus operator ei tee t‰ydellist‰ kopioita.
// Kaikkea ei voi eik‰ saa kopioida, kommentit erikoistapauksista ja syist‰ erikseen.
NFmiMapViewDescTop& NFmiMapViewDescTop::operator=(const NFmiMapViewDescTop& other)
{
	if(this != &other)
	{
		itsSettingsBaseName = other.itsSettingsBaseName;
		itsMapViewDescTopIndex = other.itsMapViewDescTopIndex;
		itsMapConfigurationSystem = other.itsMapConfigurationSystem; // pointteri kopioidaan, ei omistusta kummallakaan
		itsProjectionCurvatureInfo = other.itsProjectionCurvatureInfo; // pointteri kopioidaan, ei omistusta kummallakaan
		itsControlPath = other.itsControlPath;
		itsGdiPlusImageMapHandlerList = std::move(::CopyMapHandlerVector(other.itsGdiPlusImageMapHandlerList));
		itsMapViewCache.MakeDirty(); // T‰m‰ bitmap cache pit‰‰ vain tyhjent‰‰
		fRedrawMapView = true; // Kopion j‰lkeen kaikki piirret‰‰n uusiksi
		itsLandBorderColors = other.itsLandBorderColors;
		itsLandBorderColorIndex = other.itsLandBorderColorIndex;
		itsLandBorderPenSize = other.itsLandBorderPenSize;
		itsTimeControlViewTimes = other.itsTimeControlViewTimes;
		itsClientViewXperYRatio = other.itsClientViewXperYRatio;
		RelativeMapRect(other.itsRelativeMapRect);
		itsMapViewSizeInPixels = other.itsMapViewSizeInPixels;
		itsParamWindowViewPosition = other.itsParamWindowViewPosition;
		CombinedMapHandlerInterface::copyDrawParamsList(other.itsDrawParamListVector, itsDrawParamListVector);
		itsMapBlitDC = nullptr; // T‰ll‰isiin MFC pointtereihin vain nullptr:‰‰
		itsDrawOverMapMode = other.itsDrawOverMapMode;
		itsMapRowStartingIndex = other.itsMapRowStartingIndex;
		itsCopyCDC = nullptr; // T‰ll‰isiin MFC pointtereihin vain nullptr:‰‰
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
		fMapViewBitmapDirty = true; // Kopioinnin j‰lkeen kaikki on 'likaista'
		itsMapView = other.itsMapView; // MapView pointteri pit‰‰ kopioida t‰ss‰, se ei muutu ajon aikana
		itsGraphicalInfo = other.itsGraphicalInfo;
		itsGridPointCache.Clear(); // Nollataan t‰m‰kin cache
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
		ClearBaseLandBorderMapBitmap(); // deletoi ja nollaa itsLandBorderMapBitmap:in
		itsSeparateCountryBorderBitmapCache = other.itsSeparateCountryBorderBitmapCache;
		itsTrueMapViewSizeInfo = other.itsTrueMapViewSizeInfo;
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
	InitMapViewDrawParamListVector(); // luo tyhj‰t listat
	AnimationDataRef().TimeStepInMinutes(static_cast<int>(::round(TimeControlTimeStep()*60))); // animaatiodataan pit‰‰ p‰ivitt‰‰ aikaresoluutio
    InitFromMapViewWinRegistry(theMapViewWinRegistry);
}

void NFmiMapViewDescTop::InitFromMapViewWinRegistry(NFmiMapViewWinRegistry &theMapViewWinRegistry)
{
    fShowMasksOnMapVM = theMapViewWinRegistry.ShowMasksOnMap();
    itsSpacingOutFactorVM = theMapViewWinRegistry.SpacingOutFactor();
	// SelectedMapIndex:ille pit‰‰ tehd‰ j‰rkevyys tarkastelut
    SelectedMapIndex(theMapViewWinRegistry.SelectedMapIndex());
	fShowStationPlotVM = theMapViewWinRegistry.ShowStationPlot();
    // stringi pit‰‰ muuttaa point-otukseksi ja lopuksi pit‰‰ tehd‰ j‰rkevyys tarkastelut
    ViewGridSize(::String2Point(theMapViewWinRegistry.ViewGridSizeStr()), nullptr);
}

void NFmiMapViewDescTop::StoreToMapViewWinRegistry(NFmiMapViewWinRegistry &theMapViewWinRegistry)
{
    // N‰it‰ s‰‰det‰‰n ja k‰ytet‰‰n suoraan Win-rekistereist‰, joten n‰iden arvoja ei saa laittaa sinne lopuksi takaisin.
//    theMapViewWinRegistry.ShowMasksOnMap(fShowMasksOnMapVM);
//    theMapViewWinRegistry.SpacingOutFactor(itsSpacingOutFactorVM);
//    theMapViewWinRegistry.SelectedMapIndex(itsSelectedMapIndexVM);
//    theMapViewWinRegistry.ShowStationPlot(fShowStationPlotVM);

    // T‰t‰ s‰‰det‰‰n vain t‰nne, joten loppu tulos on aika-ajoin laitettava takaisin Win-rekistereihin.
    theMapViewWinRegistry.ViewGridSizeStr(::Point2String(itsViewGridSizeVM));
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
    ShowTimeOnMapMode(itsShowTimeOnMapMode); // t‰m‰ asettaa myˆs fShowTimeString:in arvon oikeaan
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
	itsLandBorderColors.push_back(NFmiColor(0.0f, 0.0f, 0.0f)); // musta
	itsLandBorderColors.push_back(NFmiColor(1.0f, 1.0f, 1.0f)); // valkoinen
	itsLandBorderColors.push_back(NFmiColor(1.0f, 0.0f, 0.0f)); // punainen
	itsLandBorderColors.push_back(NFmiColor(0.0f, 0.0f, 1.0f)); // sininen
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
    // Huom! ViewMakroissa itsGdiPlusImageMapHandlerList-otusta ei ole alustettu, joten jos sen size = 0, silloin ei tehd‰ indeksin tarkistusta ja korjausta
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

// scrollaa n‰yttˆriveja halutun m‰‰r‰n (negatiivinen skrollaa ylˆs ja positiivinen count alas)
bool NFmiMapViewDescTop::ScrollViewRow(int theCount)
{
    int oldValue = itsMapRowStartingIndex;
    itsMapRowStartingIndex += theCount;

    int currentRowCount = CalcVisibleRowCount();
    int maxRowSize = static_cast<int>(itsViewGridSizeMax.X()*itsViewGridSizeMax.Y());
    // menn‰‰n aina reunojen 'yli'
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
        theMapViewWinRegistry->ViewGridSizeStr(::Point2String(itsViewGridSizeVM));

	// lasketaan sitten mik‰ on maksimi karttarivin alku indeksi (riippuu mak ruudukon koosta ja nyky hila ruudukosta)
	int maxStartIndex = CalcMaxRowStartingIndex();
	itsMapRowStartingIndex = FmiMin(maxStartIndex, itsMapRowStartingIndex);
	itsTrueMapViewSizeInfo.onViewGridSizeChange(itsViewGridSizeVM, IsTimeControlViewVisibleTotal());
}

void NFmiMapViewDescTop::MapRowStartingIndex(int newIndex)
{
	// lasketaan sitten mik‰ on maksimi karttarivin alku indeksi (riippuu mak ruudukon koosta ja nyky hila ruudukosta)
	int maxStartIndex = CalcMaxRowStartingIndex();
	itsMapRowStartingIndex = FmiMin(maxStartIndex, newIndex);
	MapViewDirty(false, false, true, true);
}

// N‰kyykˆ kyseinen karttarivi nyt karttan‰ytˆss‰. TheRowIndex on rivi indeksi, jonka arvo alkaa 1:st‰.
bool NFmiMapViewDescTop::IsVisibleRow(int theRowIndex)
{
	if(theRowIndex < itsMapRowStartingIndex)
		return false;
	if(theRowIndex > (itsMapRowStartingIndex + itsViewGridSizeVM.Y() - 1))
		return false;

	return true;
}

// MapViewDirty metodi tekee kaiken sen mit‰ ennen tekiv‰t sekavasti AreaViewDirty- ja 
// MapDirty -metodit yhdess‰. Niit‰ k‰ytettiin sekaisin eri tilanteissa ja yhdess‰ (sekaisin taas).
// Yksi pahimmista sekaannuksista aiheutti fAreaViewDirty muuttujan asetus, joka meni 
// MapDirty -metodin kautta ep‰suorasti false:ksi, jos mapDirty ja clearCache parametrit  olivat molemmat true.
// Uudessa metodissa ei p‰‰se tapahtumaan sekaannuksia.
// ======================================================
// Seuraavassa selitykset parametreille:
// ======================================================
// 1. makeNewBackgroundBitmap, jos true, on tarve laskea uusi zoomaus karttapohjalle tai karttapohja vaihtuu 
//    ja samalla piirt‰‰ karttapohja uudestaan cacheen.
// 2. clearMapViewBitmapCacheRows, jos true, tyhjennet‰‰n karttan‰yttˆjen kuva-cachet t‰ysin. Kuva cache:lla tarkoitetaan eri 
//    n‰yttˆriveill‰ olevat eri aika-askeleilla olevat karttapohjan ja parametri piirto-layereiden yhteistulos.
//    T‰h‰n cache kuvaan ei kuulu ns. DrawOverBitmaptThings, joilla piirret‰‰n erilaisia merkkej‰ karttan‰ytˆille.
// 3. redrawMapView parametrilla s‰‰det‰‰n piirret‰‰nkˆ karttan‰yttˆ uudestaan, vai k‰ytet‰‰nkˆ piirrossa suoraan
//    karttan‰yttˆ-luokkien (CSmartMetView ja CFmiExtraMapView) tallettamaa cache kuvaa. T‰m‰ yksitt‰inen cache kuva on 
//    siis vain kulloisellakin hetkell‰ k‰ytˆss‰ siksi ett‰ piti tehd‰ double-buffer kuva, jos esim. toinen ohjelma/n‰yttˆ 
//    siirret‰‰n SmartMet n‰yttˆjen yli ja niit‰ ei tarvitse muuten p‰ivitt‰‰.
// 4. updateMapViewDrawingLayers parametrilla varmistetaan ett‰ karttan‰yttˆluokat tekev‰t p‰ivityksi‰ piirto-layer rakenteisiin.
void NFmiMapViewDescTop::MapViewDirty(bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool updateMapViewDrawingLayers)
{
    SetRedrawMapView(redrawMapView);
    MapHandler()->SetMakeNewBackgroundBitmap(makeNewBackgroundBitmap);
    MapHandler()->SetUpdateMapViewDrawingLayers(updateMapViewDrawingLayers);
    if(clearMapViewBitmapCacheRows)
    {
        MapViewCache().MakeDirty(); // laitetaan cache halutessa likaiseksi
        if(makeNewBackgroundBitmap)
        {
            GridPointCache().Clear();
        }
    }
}

// t‰lle funktiolle annetaan client view:ss‰ 'kartastolle' k‰ytetty alue pikselein‰
// funktio laskee t‰m‰n koon ja k‰ytetyn kartta ruudukon avulla yhden ruudun x/y suhteen,
// jota sitten k‰ytet‰‰n mm. automaattizoomauksessa
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
	// Ei saa tehd‰ itsRelativeMapRect jos timeControlViewIsHidden on true, 
	// koska siit‰ tulee max-area laatikko eli (0,0-1,1). T‰m‰ on erikoistapaus ja ne hoidetaan
	// RelativeMapRect -metodeissa omalla tavalla. itsRelativeMapRect:in arvoksi ei saa tulla sit‰!
    if(!timeControlViewIsHidden)
	{
		// S‰‰det‰‰n samalla suhteellista osiota, mink‰ karttan‰yttˆ ottaa ja j‰tt‰‰ loput aikakontrolli-ikkunalle.
		int wantedTimeControlHeightInPixels = FmiRound(TrueMapViewSizeInfo::calculateTimeControlViewHeightInPixels(itsTrueMapViewSizeInfo.logicalPixelsPerMilliMeter().X()));
	    double mapVerticalPortion = (newSize.Y() - wantedTimeControlHeightInPixels) / newSize.Y();
		auto modifiedRelativeMapRect = itsRelativeMapRect;
		modifiedRelativeMapRect.Height(mapVerticalPortion);
		RelativeMapRect(modifiedRelativeMapRect);
	}

    // lopuksi pit‰‰ viel‰ p‰ivitt‰‰ x-y suhde
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
	// Ei sallita (0,0 - 1,1) rectin asetusta, joka on erikoistapaus
	if(theMapRect != g_TotalRelativeMapRect)
		itsRelativeMapRect = theMapRect; 
}

bool NFmiMapViewDescTop::IsTimeControlViewVisible() const
{
	return itsShowTimeOnMapMode < 2;
}

// nelj‰ tilaa:
// 0 = n‰yt‰ aikakontrolliikkuna+teksti
// 1=vain aik.kont.ikkuna
// 2=‰l‰ n‰yt‰ kumpaakaan
// 3= n‰yt‰ vain teksti
// palauttaa currentin tilan
int NFmiMapViewDescTop::ToggleShowTimeOnMapMode(void)
{
	itsShowTimeOnMapMode++;
	if(itsShowTimeOnMapMode > 3)
		itsShowTimeOnMapMode = 0;
	// Jos vaikka on luettu joku negatiivinen luku n‰yttˆmakrosta, se pit‰‰ korjata
	if(itsShowTimeOnMapMode < 0)
		itsShowTimeOnMapMode = 0;
	bool mapAreaDirty = false;
	switch(itsShowTimeOnMapMode)
	{
	case 0: // 0 = n‰yt‰ aikakontrolliikkuna+teksti
		fShowTimeString = true;
		mapAreaDirty = true;
		break;
	case 1: // 1=vain aik.kont.ikkuna
		fShowTimeString = false;
		break;
	case 2: // 2=‰l‰ n‰yt‰ kumpaakaan
		fShowTimeString = false;
		mapAreaDirty = true;
		break;
	case 3: // 3= n‰yt‰ vain teksti
		fShowTimeString = true;
		break;
	}
	MapViewDirty(mapAreaDirty, true, true, false);
	if(mapAreaDirty)
	{
		SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry); // ikkunan koko muuttuu tietyiss‰ tapauksissa, joten rajaviivat on laskettava uudestaan
	}
	// Kartta-alueen koot pit‰‰ myˆs p‰ivitt‰‰, jos aikakontrolli-ikkuna menee pois n‰kyvist‰ tai tulee taas n‰kyviin
	RecalculateMapViewSizeInPixels(CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->ApplicationWinRegistry().DrawObjectScaleFactor());
	return itsShowTimeOnMapMode;
}

void NFmiMapViewDescTop::ShowTimeOnMapMode(int newValue)
{
	// k‰ytet‰‰n hyv‰ksi ToggleShowTimeOnMapMode-metodin
	// tekemi‰ tarkastuksia ja asetuksia.
	itsShowTimeOnMapMode = newValue-1;
	ToggleShowTimeOnMapMode();
}

void NFmiMapViewDescTop::TimeControlTimeStep(float newValue)
{
	itsTimeControlTimeStep = newValue;
	if(itsTimeControlTimeStep < 0.02f) // 0.02 on sama kuin yhden minuutin time steppi, mik‰ on pienin mahdollinen
		itsTimeControlTimeStep = 0.02f;
	itsAnimationData.TimeStepInMinutes(boost::math::iround(itsTimeControlTimeStep*60.f));
}

void NFmiMapViewDescTop::ToggleLandBorderColor(void)
{
	itsLandBorderColorIndex++;
	if(itsLandBorderColorIndex >= static_cast<int>(itsLandBorderColors.size()))
		itsLandBorderColorIndex = -1; // kun menee ymp‰ri, laitetaan 'tyhj‰' v‰ri p‰‰lle
	SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Cosmetic);
}

// N‰m‰ border layer piirtoon liittyv‰t metodit jotka ottavat separateBorderLayerDrawOptions parametrin
// toimivat seuraavalla periaatteella:
// Jos k‰ytt‰j‰ haluaa tietoja yleisest‰ border-draw asetuksista, on em. parametri nullptr.
// Jos se on nullptr:st‰ poikkeava, kyse on erillinen border-layer, jonka tiedot haetaan erikseen.
// *************************************************************************************************
const NFmiColor& NFmiMapViewDescTop::LandBorderColor(NFmiDrawParam* separateBorderLayerDrawOptions)
{
	if(separateBorderLayerDrawOptions)
		return separateBorderLayerDrawOptions->IsolineColor();

	if(itsLandBorderColorIndex >= 0 && itsLandBorderColorIndex < static_cast<int>(itsLandBorderColors.size()))
		return itsLandBorderColors[itsLandBorderColorIndex];
	return itsLandBorderColors[0]; // virhetilanteessa palautetaan 1. v‰ri
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
		return (bitmap == nullptr); // Jos ei lˆydy erillis-layer-cachesta kuvaa, on se 'likainen'
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
		// Annettu newBitmap otetaan t‰ss‰ omistukseen unique_ptr:iin
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
	return itsLandBorderColors[0]; // virhetilanteessa palautetaan 1. v‰ri
}

const NFmiPoint& NFmiMapViewDescTop::StationPointSize(void) const
{
	return itsStationPointSize;
}

void NFmiMapViewDescTop::ToggleStationPointColor(void)
{
	itsStationPointColorIndex++;
	if(itsStationPointColorIndex >= static_cast<int>(itsLandBorderColors.size()))
		itsStationPointColorIndex = 0; // kun menee ymp‰ri, laitetaan 1. v‰ri takaisin p‰‰lle
	if(fShowStationPlotVM)
		MapViewDirty(false, true, true, false); // laitetaan viela kaikki ajat likaisiksi cachesta
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
		MapViewDirty(false, true, true, false); // laitetaan viela kaikki ajat likaisiksi cachesta
}

// t‰m‰ asettaa uuden karttan‰ytˆn hilaruudukon koon.
// tekee tarvittavat 'likaukset' ja palauttaa true, jos
// n‰yttˆj‰ tarvitsee p‰ivitt‰‰, muuten false (eli ruudukko ei muuttunut).
bool NFmiMapViewDescTop::SetMapViewGrid(const NFmiPoint &newValue, NFmiMapViewWinRegistry *theMapViewWinRegistry)
{
	NFmiPoint oldSize(itsViewGridSizeVM);
	ViewGridSize(newValue, theMapViewWinRegistry);
	MapRowStartingIndex(MapRowStartingIndex()); // HUOM! pit‰‰ myˆs s‰‰t‰‰ aloitus rivi
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
    // lasketaan myˆs yhden kartta ruudun koko mm uudestaan
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

	// drawparamlistoje ei initialisoida t‰ss‰!!!!
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
		fShowMasksOnMapVM = theOther.fShowMasksOnMapVM; // talletetaan viewMacrosta arvo sek‰ lokaali arvoon ett‰ rekisteriin
		theMapViewWinRegistry.ShowMasksOnMap(theOther.fShowMasksOnMapVM);
		itsSpacingOutFactorVM = theOther.itsSpacingOutFactorVM; // talletetaan viewMacrosta arvo sek‰ lokaali arvoon ett‰ rekisteriin
		theMapViewWinRegistry.SpacingOutFactor(theOther.itsSpacingOutFactorVM);
		SelectedMapIndex(theOther.itsSelectedMapIndexVM); // t‰m‰ indeksi pit‰‰ tarkistaa, joten se laitetaan asetus-metodin l‰pi
		theMapViewWinRegistry.SelectedMapIndex(itsSelectedMapIndexVM);

		fShowStationPlotVM = theOther.fShowStationPlotVM;
		theMapViewWinRegistry.ShowStationPlot(theOther.fShowStationPlotVM);
		itsViewGridSizeVM = theOther.itsViewGridSizeVM;
		theMapViewWinRegistry.ViewGridSizeStr(::Point2String(theOther.itsViewGridSizeVM));
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

	if(theOther.itsTimeControlViewTimes.Size() > 1) // vanhoista makroista t‰h‰n tulee tyhj‰ aikabagi, joka pit‰‰ ignoorata
		itsTimeControlViewTimes = theOther.itsTimeControlViewTimes;
	itsAnimationData = theOther.itsAnimationData;
}

// HUOM!! T‰m‰ laittaa kommentteja mukaan!
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

	// itsDrawParamListVector; // T‰m‰n talletus tehd‰‰n erikseen n‰yttˆmakroluokassa
	// itsExtraDrawParamListVector; // T‰m‰n talletus tehd‰‰n erikseen n‰yttˆmakroluokassa

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
    // N‰yttˆmakrojen taakse/eteenp‰in yhteen sopivuuden nimiss‰, t‰m‰ pit‰‰ tallettaa falsena
    bool legacy_fShowRedCrossOverMapIfNotEditedDataTime = false;
	os << fShowSelectedPointsOnMap << " " << fShowControlPointsOnMap << " " << fShowObsComparisonOnMap << " " << fShowWarningMarkersOnMap << " " << legacy_fShowRedCrossOverMapIfNotEditedDataTime << std::endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.

    // 'double' muotoisten lis‰datojen lis‰ys
	extraData.Add(static_cast<double>(fLockToMainMapViewRow)); // talletetaan 1. extra-datana parametri rivin lukitus
	extraData.Add(static_cast<double>(itsParamWindowViewPosition)); // talletetaan 2. extra-datana parametri-laatikon sijainti

    // string muotoisten lis‰datojen lis‰ys
    std::stringstream extraDataStream;
    itsAnimationData.Write(extraDataStream);
	extraData.Add(extraDataStream.str()); // talletetaan 1. extra-datana animaatioon liittyv‰ data yhten‰ stringin‰
	std::string timeBagStr = NFmiDataStoringHelpers::GetTimeBagOffSetStr(usedViewMacroTime, *(TimeControlViewTimes().ValidTimeBag()));
	extraData.Add(timeBagStr); // lis‰t‰‰n 2. extra-datana aikaikkunan timebagi offsettina currenttiin aikaan

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiMapViewDescTop::Write failed");
}

// HUOM!! ennen kuin t‰m‰ luokka luetaan sis‰‰n tiedostosta, poista kommentit
// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
// sekaan. Eli ‰l‰ k‰yt‰ suoraan t‰t‰ metodia, vaan Init(filename)-metodia!!!!
void NFmiMapViewDescTop::Read(std::istream& is)
{
	is >> itsSelectedMapIndexVM;

	bool showParamWindowView = true;
	is >> itsLandBorderColorIndex >> showParamWindowView;

	is >> itsLandBorderPenSize;
	NFmiRect tmpMapRect;
	is >> tmpMapRect;
	RelativeMapRect(tmpMapRect);

	// itsDrawParamListVector; // T‰m‰n talletus tehd‰‰n erikseen n‰yttˆmakroluokassa
	// itsExtraDrawParamListVector; // T‰m‰n talletus tehd‰‰n erikseen n‰yttˆmakroluokassa

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

    // N‰yttˆmakrojen taakse/eteenp‰in yhteen sopivuuden nimiss‰, t‰m‰ pit‰‰ lukea
    bool legacy_fShowRedCrossOverMapIfNotEditedDataTime = false;
    is >> fShowSelectedPointsOnMap >> fShowControlPointsOnMap >> fShowObsComparisonOnMap >> fShowWarningMarkersOnMap >> legacy_fShowRedCrossOverMapIfNotEditedDataTime;

	if(is.fail())
		throw std::runtime_error("NFmiMapViewDescTop::Read failed");

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

    // 'double' muotoisten lis‰datojen poiminta

	// Luetaan 1. extra-datana parametri rivin lukitus, oletusarvona fLockToMainMapViewRow:ille on false
	fLockToMainMapViewRow = false;
    if(extraData.itsDoubleValues.size() >= 1)
        fLockToMainMapViewRow = extraData.itsDoubleValues[0] != 0; // (HUOM! ei voi static_cast:ata bool:iksi, koska VC++ tekee yst‰v‰llisen varoituksen hitaudesta)

	// Luetaan 2. extra-datana parametri parametri-laatikon sijainti, oletusarvona itsParamWindowViewPosition:ille riippuu potentiaalisesti 
	// vanhalla versiolla tehdyn n‰yttˆmakron showParamWindowView muuttujaan luetusta originaali asetuksesta.
	itsParamWindowViewPosition = showParamWindowView ? kTopLeft : kNoDirection;
	if(extraData.itsDoubleValues.size() >= 2)
		itsParamWindowViewPosition = static_cast<FmiDirection>(extraData.itsDoubleValues[1]);

    // string muotoisten lis‰datojen poiminta
    if(extraData.itsStringValues.size() >= 1)
	{// luetaan 1. extra-datana animaatioon liittyv‰ data yhten‰ stringin‰
		if(is.fail())
			throw std::runtime_error("NFmiMapViewDescTop::Read failed");
		std::stringstream extraDataStream;
		extraDataStream.str(extraData.itsStringValues[0]);
		itsAnimationData.Read(extraDataStream);
	}
	if(extraData.itsStringValues.size() >= 2)
	{// luetaan 2. extra-datana aikaikkunan timebagi offsettina currenttiin aikaan
		if(is.fail())
			throw std::runtime_error("NFmiMapViewDescTop::Read failed");
		NFmiTimeBag times = NFmiDataStoringHelpers::GetTimeBagOffSetFromStr(usedViewMacroTime, extraData.itsStringValues[1]);
		TimeControlViewTimes(NFmiTimeDescriptor(times.FirstTime(), times));
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

// laskee n‰yttˆruudukon yhden ruudun koon pikseleiss‰
NFmiPoint NFmiMapViewDescTop::ActualMapBitmapSizeInPixels(void)
{
	NFmiPoint returnValue(MapViewSizeInPixels().X() * RelativeMapRect().Width() / itsViewGridSizeVM.X()
						 ,MapViewSizeInPixels().Y() * RelativeMapRect().Height() / itsViewGridSizeVM.Y());
	return returnValue;
}

void NFmiMapViewDescTop::TimeControlViewTimes(const NFmiTimeDescriptor &newTimeDescriptor)
{
	if(newTimeDescriptor.UseTimeList())
	{ // pit‰‰ tehd‰ timebag tyyppinen timedescriptori, jos annettu oli timelist tyyppinen
        int usedTimeStepInMinutes = boost::math::iround(itsTimeControlTimeStep * 60);
		if(usedTimeStepInMinutes > 60)
			usedTimeStepInMinutes = 60; // maksimissaan kuitenkin 60 mkinuutin aikasteppi
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
        if(diffInHours > 2*24*366) // ei yli kahden vuoden aikaikkunaa t‰h‰n, kyse on luultavasti virheest‰???
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

	// Kutsutaan lopuksi viel‰ skrollaus-metodia 0-siirrolla. T‰m‰ asettaa n‰yttˆrivit niin, 
	// kuin ne voivat kulloisessakin moodissa olla. Esim. Jos ollaan ensin yksi-aika-moodissa rivill‰ 35
	// ja siirryt‰‰n yksi-aika-per-sarake eli normaali moodiiin, silloin ei voida olla rivill‰ 35, vaan 
	// aloitus rivi‰ pit‰‰ s‰‰t‰‰, niin ett‰ se mahtuu 5 ensimm‰iseen riviin riippuen ruudukon koosta.
	ScrollViewRow(0);
}

// S‰‰det‰‰n kaikki aikaa liittyv‰t jutut parametrina annettuun aikaan, ett‰ SmartMet s‰‰tyy ladattuun CaseStudy-dataan mahdollisimman hyvin.
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


// Laitoin karttojen likaus systeemit uusiksi monellakin tapaa:
// 1. Nimi muuttui toivottavasti kuvaavammaksi eli RedrawMapView, eli se tarkoittaa
//    ett‰ seuraavalla piirrooskierroksella n‰yttˆ pit‰‰ piirt‰‰ uusiksi, eik‰ saa vain k‰ytt‰‰ double buffer bitmap:ia.
// 2. Ns. set-funktiolla voi lipun laittaa vain p‰‰lle, false arvo j‰tet‰‰n huomiotta. Vain clear-funktiolla
//    lipun tila voidaan nollata. T‰m‰ muutos siksi ett‰ eri paikoista voi tulla samalla likaus kerralla eri arvoja
//    eik‰ toisaalta tehdyst‰ false asetuksesta ei haluta nollata toisesta tehty‰ true asetusta.
// 3. Lippu siis nollataan clear-funktiolla, jota kutsutaan piirto toimintojen j‰lkeen.
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

bool NFmiMapViewDescTop::IsTimeControlViewVisibleTotal() const 
{ 
	return !fPrintingModeOn && IsTimeControlViewVisible(); 
}
