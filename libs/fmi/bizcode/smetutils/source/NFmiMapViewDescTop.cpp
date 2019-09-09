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

#include <algorithm>
#include "stdafx.h"

#pragma warning( push )

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//template<typename T>
struct PointerDestroyer
{
	template<typename T>
	void operator()(T* thePtr)
	{delete thePtr;}
};

static std::string Point2String(const NFmiPoint &p)
{
    std::stringstream out;
    out << p.X() << "," << p.Y(); // tein oman tavan kirjoittaa NFmiPoint, koska en halunnut rivinvaihtoa loppuun
    return out.str();
}

static NFmiPoint String2Point(const std::string &str)
{
    std::stringstream in(str);
    double x = 0;
    double y = 0;
    char ch = 0; // pilkku pit‰‰ lukea v‰list‰
    in >> x >> ch >> y;
    return NFmiPoint(x, y);
}

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

	if(is.fail())
		throw std::runtime_error("NFmiMapViewDescTop::ViewMacroDipMapHelper::Read failed");
}


// ************************************************************************
// ******* NFmiMapViewDescTop -luokka *************************************
// ************************************************************************

NFmiMapViewDescTop::NFmiMapViewDescTop(void)
:itsSettingsBaseName()
,itsMapConfigurationSystem(0)
,itsProjectionCurvatureInfo(0)
,itsControlPath()
,itsSelectedMapIndexVM(1)
,itsGdiPlusImageMapHandlerList()
,itsMapViewCache(CtrlViewUtils::MaxViewGridYSize)
,fRedrawMapView(true)
,itsLandBorderColors()
,itsLandBorderColorIndex(0)
,itsLandBorderPenSize(1,1)
,fBorderDrawDirty(true)
,itsTimeControlViewTimes()
,itsClientViewXperYRatio(1.)
,itsRelativeMapRect(0.,0.,1.,0.9) // t‰m‰n suhteellisen osan n‰ytˆst‰ valtaa kartasto, loput menee aikakontrolli ikkunalle
,itsMapViewSizeInPixels(10, 10)
,fShowParamWindowView(true)
,itsDrawParamListVector(0)
,itsMapBlitDC(0)
,itsDrawOverMapMode(0)
,itsMapRowStartingIndex(1)
,itsCopyCDC(0)
,itsShowTimeOnMapMode(0)
,fShowTimeString(true)
,itsCurrentTime()
,itsViewGridSizeVM(1,1)
,itsViewGridSizeMax(CtrlViewUtils::MaxViewGridXSize, CtrlViewUtils::MaxViewGridYSize)
,fShowStationPlotVM(false)
,itsStationPointColorIndex(0)
,itsStationPointSize(1, 1)
,fShowMasksOnMapVM(false)
,itsTimeControlTimeStep(1)
,itsSpacingOutFactorVM(0)
,itsMapViewDisplayMode(CtrlViewUtils::MapViewMode::kNormal)
,itsActiveViewRow(1)
,fDescTopOn(false)
,fMapViewBitmapDirty(false)
,itsMapView(0)
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
,itsLandBorderMapBitmap(nullptr)
{
}


NFmiMapViewDescTop::NFmiMapViewDescTop(const std::string &theSettingsBaseName, NFmiMapConfigurationSystem *theMapConfigurationSystem, NFmiProjectionCurvatureInfo* theProjectionCurvatureInfo, const std::string &theControlPath)
:itsSettingsBaseName(theSettingsBaseName + "::")
,itsMapConfigurationSystem(theMapConfigurationSystem)
,itsProjectionCurvatureInfo(theProjectionCurvatureInfo)
,itsControlPath(theControlPath)
,itsSelectedMapIndexVM(1)
,itsGdiPlusImageMapHandlerList()
,itsMapViewCache(CtrlViewUtils::MaxViewGridYSize)
,fRedrawMapView(true)
,itsLandBorderColors()
,itsLandBorderColorIndex(0)
,itsLandBorderPenSize(1,1)
,fBorderDrawDirty(true)
,itsTimeControlViewTimes()
,itsClientViewXperYRatio(1.)
,itsRelativeMapRect(0.,0.,1.,0.9) // t‰m‰n suhteellisen osan n‰ytˆst‰ valtaa kartasto, loput menee aikakontrolli ikkunalle
,itsMapViewSizeInPixels(10, 10)
,fShowParamWindowView(true)
,itsDrawParamListVector(0)
,itsMapBlitDC(0)
,itsDrawOverMapMode(0)
,itsMapRowStartingIndex(1)
,itsCopyCDC(0)
,itsShowTimeOnMapMode(0)
,fShowTimeString(true)
,itsCurrentTime()
,itsViewGridSizeVM(1,1)
,itsViewGridSizeMax(CtrlViewUtils::MaxViewGridXSize, CtrlViewUtils::MaxViewGridYSize)
,fShowStationPlotVM(false)
,itsStationPointColorIndex(0)
,itsStationPointSize(1, 1)
,fShowMasksOnMapVM(false)
,itsTimeControlTimeStep(1)
,itsSpacingOutFactorVM(0)
,itsMapViewDisplayMode(CtrlViewUtils::MapViewMode::kNormal)
,itsActiveViewRow(1)
,fDescTopOn(false)
,fMapViewBitmapDirty(false)
,itsMapView(0)
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
,itsLandBorderMapBitmap(nullptr)
{
}

NFmiMapViewDescTop::~NFmiMapViewDescTop(void)
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

    if(itsLandBorderMapBitmap)
    {
        delete itsLandBorderMapBitmap;
        itsLandBorderMapBitmap = nullptr;
    }
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
    SelectedMapIndex(theMapViewWinRegistry.SelectedMapIndex()); // pit‰‰ tehd‰ j‰rkevyys tarkastelut
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

static void SetGridSize(const string &theKey, const NFmiPoint &theGridSize)
{
	string gridStr;
	gridStr += NFmiStringTools::Convert<int>(static_cast<int>(theGridSize.X()));
	gridStr += ",";
	gridStr += NFmiStringTools::Convert<int>(static_cast<int>(theGridSize.Y()));
	NFmiSettings::Set(theKey, gridStr, true);
}

template<typename T>
static T ReadValueFromSettings(const string &theBaseStr, const string &theKeyWord)
{
	string settingStr = theBaseStr;
	settingStr += theKeyWord;
	return NFmiSettings::Require<T>(settingStr.c_str());
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

template<typename T>
static void StoreValueToSettings(const string &theBaseStr, const string &theKeyWord, T theValue)
{
	string settingStr(theBaseStr);
	settingStr += theKeyWord;
	NFmiSettings::Set(settingStr.c_str(), NFmiStringTools::Convert<T>(theValue), true);
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

static void InitDrawParamListVector(NFmiPtrList<NFmiDrawParamList> **theList, int theSize)
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
	if(itsMapConfigurationSystem)
	{
		int ssize = static_cast<int>(itsMapConfigurationSystem->Size());
		for(int i=0; i<ssize; i++)
		{
			NFmiGdiPlusImageMapHandler* mHandler = CreateGdiPlusImageMapHandler(itsMapConfigurationSystem->GetMapConfiguration(i));
			if(mHandler)
				itsGdiPlusImageMapHandlerList.push_back(mHandler);
			else
				throw std::runtime_error("ERROR in NFmiMapViewDescTop::InitGdiPlusImageMapHandlerSystem - cannot create NFmiGdiPlusImageMapHandler.");
		}
	}
	else
		throw std::runtime_error("ERROR in NFmiMapViewDescTop::InitGdiPlusImageMapHandlerSystem - MapConfigurationSystem was null pointer.");
}

NFmiGdiPlusImageMapHandler* NFmiMapViewDescTop::CreateGdiPlusImageMapHandler(const NFmiMapConfiguration &theMapConfiguration)
{
	NFmiGdiPlusImageMapHandler* mHandler = new NFmiGdiPlusImageMapHandler;
	mHandler->UsedMapIndex(0);
    mHandler->ControlPath(std::string(itsControlPath));
	if(theMapConfiguration.ProjectionFileName() == "")
	{
		mHandler->OriginalArea(theMapConfiguration.Projection());
		mHandler->Init(theMapConfiguration.MapFileNames(), theMapConfiguration.MapDrawingStyles(), theMapConfiguration.OverMapDibFileNames(), theMapConfiguration.OverMapDibDrawingStyles());
	}
	else
	{
		mHandler->Init(theMapConfiguration.ProjectionFileName(), theMapConfiguration.MapFileNames(), theMapConfiguration.MapDrawingStyles(), theMapConfiguration.OverMapDibFileNames(), theMapConfiguration.OverMapDibDrawingStyles());
	}

	return mHandler;
}

void NFmiMapViewDescTop::SelectedMapIndex(unsigned int newValue)
{
	itsSelectedMapIndexVM = newValue;
    // Huom! ViewMakroissa itsGdiPlusImageMapHandlerList-otusta ei ole alustettu, joten jos sen size = 0, silloin ei tehd‰ indeksin tarkistusta ja korjausta
	if(itsGdiPlusImageMapHandlerList.size() > 0 && itsSelectedMapIndexVM >= itsGdiPlusImageMapHandlerList.size())
		itsSelectedMapIndexVM = static_cast<int>(itsGdiPlusImageMapHandlerList.size() - 1);
}

NFmiGdiPlusImageMapHandler* NFmiMapViewDescTop::MapHandler(void)
{
	if(itsSelectedMapIndexVM < itsGdiPlusImageMapHandlerList.size())
		return itsGdiPlusImageMapHandlerList[itsSelectedMapIndexVM];
	throw std::runtime_error("ERROR in NFmiMapViewDescTop::GdiPlusImageMapHandler - SelectedMapIndex was out of bounds, error in program or configurations.");
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
bool NFmiMapViewDescTop::ScrollViewRow(int theCount, int &theActiveViewRow)
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
    {
        theActiveViewRow = theActiveViewRow + (oldValue - itsMapRowStartingIndex);
        return true;
    }
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
	if(theRowIndex > itsMapRowStartingIndex + itsViewGridSizeVM.Y())
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

static double CalcTimeControlViewHeightInPixels(double thePixelsPerMMinX)
{
    NFmiPoint fontSizeInPixels = CtrlViewUtils::CalcTimeScaleFontSizeInPixels(thePixelsPerMMinX);
    return fontSizeInPixels.X() * 3.5; // kerrotaan n. tekstirivien lukum‰‰r‰ll‰
}

void NFmiMapViewDescTop::MapViewSizeInPixels(const NFmiPoint& newSize, bool fHideTimeControlView)
{
    itsMapViewSizeInPixels = newSize;

    // S‰‰det‰‰n smalla suhteellista osiota, mink‰ karttan‰yttˆ ottaa ja j‰tt‰‰ loput aikakontrolli-ikkunalle.
    int wantedTimeControlHeightInPixels = FmiRound(::CalcTimeControlViewHeightInPixels(GetGraphicalInfo().itsPixelsPerMM_x));
    if(fHideTimeControlView)
        wantedTimeControlHeightInPixels = 1; // laitetaan mm. printtauksen yhteydess‰, karttan‰yttˆjen tapauksessa, t‰h‰n korkeudeksi 1, jotta aikakontrolli-ikkuna olisi piilossa, mutta ei kuitenkaan 0:aa, josko siit‰ seuraisi jokin 0:lla jako tms.
    double mapVerticalPortion = (newSize.Y() - wantedTimeControlHeightInPixels) / newSize.Y();
    itsRelativeMapRect.Height(mapVerticalPortion);

    // lopuksi pit‰‰ viel‰ p‰ivitt‰‰ x-y suhde
    CalcClientViewXperYRatio(newSize);

    UpdateOneMapViewSize();
}

const NFmiRect& NFmiMapViewDescTop::RelativeMapRect(void)
{ 
    if(itsShowTimeOnMapMode < 2)
        return itsRelativeMapRect; 
    else
    {
		static const NFmiRect totalRelativeRect(0.,0.,1.,1.);
        return totalRelativeRect;
    }
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
		BorderDrawDirty(true); // ikkunan koko muuttuu tietyiss‰ tapauksissa, joten rajaviivat on laskettava uudestaan
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

const NFmiColor& NFmiMapViewDescTop::LandBorderColor(void)
{
	if(itsLandBorderColorIndex >= 0 && itsLandBorderColorIndex < static_cast<int>(itsLandBorderColors.size()))
		return itsLandBorderColors[itsLandBorderColorIndex];
	return itsLandBorderColors[0]; // virhetilanteessa palautetaan 1. v‰ri
}
void NFmiMapViewDescTop::ToggleLandBorderColor(void)
{
	itsLandBorderColorIndex++;
	if(itsLandBorderColorIndex >= static_cast<int>(itsLandBorderColors.size()))
		itsLandBorderColorIndex = -1; // kun menee ymp‰ri, laitetaan 'tyhj‰' v‰ri p‰‰lle
	BorderDrawDirty(true);
}

bool NFmiMapViewDescTop::DrawLandBorders(void)
{
	if(itsLandBorderColorIndex < 0)
		return false;
	return true;
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

	BorderDrawDirty(true);
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
		BorderDrawDirty(true);
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

void NFmiMapViewDescTop::InitForViewMacro(const NFmiMapViewDescTop &theOther, NFmiMapViewWinRegistry &theMapViewWinRegistry, bool getFromRegisty)
{
	itsLandBorderColorIndex = theOther.itsLandBorderColorIndex;
	fShowParamWindowView = theOther.fShowParamWindowView;

	itsLandBorderPenSize = theOther.itsLandBorderPenSize;

	itsRelativeMapRect = theOther.itsRelativeMapRect;

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
	itsActiveViewRow = theOther.itsActiveViewRow;

	fDescTopOn = theOther.fDescTopOn;
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
	os << itsLandBorderColorIndex << " " << fShowParamWindowView << std::endl;

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

	is >> itsLandBorderColorIndex >> fShowParamWindowView;

	is >> itsLandBorderPenSize;

	is >> itsRelativeMapRect;

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
    if(extraData.itsDoubleValues.size() >= 1)
        fLockToMainMapViewRow = extraData.itsDoubleValues[0] != 0; // luetaan 1. extra-datana parametri rivin lukitus (HUOM! ei voi static_cast:ata bool:iksi, koska VC++ tekee yst‰v‰llisen varoituksen hitaudesta)

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
	size_t ssize = itsGdiPlusImageMapHandlerList.size();
	for(size_t i = 0; i < ssize; i++)
	{
		NFmiGdiPlusImageMapHandler &mapHandler = *(itsGdiPlusImageMapHandlerList[i]);
		tmpData.itsUsedMapIndex = mapHandler.UsedMapIndex();
		tmpData.itsUsedOverMapDibIndex = mapHandler.OverMapBitmapIndex();
		tmpData.itsZoomedAreaStr = mapHandler.Area() ? mapHandler.Area()->AreaStr() : "";
		helperList.push_back(tmpData);
	}

	return helperList;
}

void NFmiMapViewDescTop::SetViewMacroDipMapHelperList(const std::vector<NFmiMapViewDescTop::ViewMacroDipMapHelper> &theData)
{
	size_t ssize1 = itsGdiPlusImageMapHandlerList.size();
	size_t ssize2 = theData.size();
	size_t usedSize = FmiMin(ssize1, ssize2);
	for(size_t i = 0; i < usedSize; i++)
	{
		const NFmiMapViewDescTop::ViewMacroDipMapHelper &tmpData = theData[i];
		NFmiGdiPlusImageMapHandler &mapHandler = *(itsGdiPlusImageMapHandlerList[i]);

		mapHandler.UsedMapIndex(tmpData.itsUsedMapIndex);
		mapHandler.OverMapBitmapIndex(tmpData.itsUsedOverMapDibIndex);

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
	ScrollViewRow(0, itsActiveViewRow);
}

// S‰‰det‰‰n kaikki aikaa liittyv‰t jutut parametrina annettuun aikaan, ett‰ SmartMet s‰‰tyy ladattuun CaseStudy-dataan mahdollisimman hyvin.
void NFmiMapViewDescTop::SetCaseStudyTimes(const NFmiMetTime &theCaseStudyTime)
{
	itsTimeControlViewTimes.SetNewStartTime(theCaseStudyTime);
	itsCurrentTime = theCaseStudyTime;
}

std::list<NFmiPolyline*>& NFmiMapViewDescTop::DrawBorderPolyLineList(void)
{
    return MapHandler()->DrawBorderPolyLineList();
}

void NFmiMapViewDescTop::DrawBorderPolyLineList(std::list<NFmiPolyline*> &newValue)
{
    MapHandler()->DrawBorderPolyLineList(newValue);
}

const std::list<std::vector<NFmiPoint>>& NFmiMapViewDescTop::DrawBorderPolyLineListGdiplus()
{ 
    return MapHandler()->DrawBorderPolyLineListGdiplus();
}
void NFmiMapViewDescTop::DrawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>> &newValue)
{ 
    MapHandler()->DrawBorderPolyLineListGdiplus(newValue);
}
void NFmiMapViewDescTop::DrawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>> &&newValue)
{ 
    MapHandler()->DrawBorderPolyLineListGdiplus(newValue);
}

void NFmiMapViewDescTop::SetLandBorderMapBitmap(Gdiplus::Bitmap *newBitmap)
{
    delete itsLandBorderMapBitmap;
    itsLandBorderMapBitmap = newBitmap;
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
