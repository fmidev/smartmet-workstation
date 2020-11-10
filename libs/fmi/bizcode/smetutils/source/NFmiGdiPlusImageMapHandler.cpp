//////////////////////////////////////////////////////////////////////
// NFmiGdiPlusImageMapHandler.cpp: implementation for the
// NFmiGdiPlusImageMapHandler class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k‰‰nt‰j‰n varoitusta (liian pitk‰ nimi >255 merkki‰ joka johtuu 'puretuista' STL-template nimist‰)
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
#include "SpatialReference.h"
#include "catlog/catlog.h"
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
,itsCountryBorderGeometry()
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

// Huomioitavaa ett‰ copy-constructor ei tee t‰ydellist‰ kopioita.
// Kopiointiin k‰ytet‰‰n sijoitus operaattoria.
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
	, itsCountryBorderGeometry()
	, itsMapConfiguration()
{
	*this = other;
}

// Huomioitavaa ett‰ sijoitus operator ei tee t‰ydellist‰ kopioita.
// Kaikkea ei voi eik‰ saa kopioida, kommentit erikoistapauksista ja syist‰ erikseen.
NFmiGdiPlusImageMapHandler& NFmiGdiPlusImageMapHandler::operator=(const NFmiGdiPlusImageMapHandler& other)
{
	if(this != &other)
	{
		itsUsedMapIndex = other.itsUsedMapIndex;
		itsUsedOverMapBitmapIndex = other.itsUsedOverMapBitmapIndex;
		//itsMapBitmaps // bitmap:eja ei voi kopioida, t‰m‰n alustus metodin lopussa
		//itsOverMapBitmaps // bitmap:eja ei voi kopioida, t‰m‰n alustus metodin lopussa
		itsOriginalArea.reset(::MakeNewAreaClone(other.itsOriginalArea));
		itsZoomedArea.reset(::MakeNewAreaClone(other.itsZoomedArea));
		itsZoomedAreaPosition = other.itsZoomedAreaPosition;
		fMakeNewBackgroundBitmap = true; // Kopion j‰lkeen pakotetaan tekem‰‰n uusi karttapohja
		fUpdateMapViewDrawingLayers = true; // Kopion j‰lkeen pakotetaan tekem‰‰n piirtopintojen p‰ivitykset
		fMapReallyChanged = true; // Kopion j‰lkeen tehd‰‰n asiat niin kuin kartta-alue olisi todellakin muuttunut
		itsControlPath = other.itsControlPath;
		itsSwapBaseArea.reset(::MakeNewAreaClone(other.itsSwapBaseArea));
		itsSwapBackArea.reset(::MakeNewAreaClone(other.itsSwapBackArea));
		itsSwapMode = other.itsSwapMode;
		itsCountryBorderPolylineCache = other.itsCountryBorderPolylineCache;
		itsCountryBorderGeometry = other.itsCountryBorderGeometry;
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
		itsOriginalArea->SetGridSize(150, 150);
	}

	std::string logStr = "Area: ";
	logStr += itsOriginalArea->AreaFactoryProjStr();
	logStr += ", worldXY rect: ";
	auto worldXyRect = itsOriginalArea->WorldRect();
	logStr += std::to_string(worldXyRect.Left());
	logStr += ",";
	logStr += std::to_string(worldXyRect.Top());
	logStr += ",";
	logStr += std::to_string(worldXyRect.Right());
	logStr += ",";
	logStr += std::to_string(worldXyRect.Bottom());
	CatLog::logMessage(logStr, CatLog::Severity::Trace, CatLog::Category::Configuration, true);

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

bool NFmiGdiPlusImageMapHandler::Init(std::shared_ptr<NFmiMapConfiguration>& mapConfiguration)
{
	CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, __FUNCTION__);

	itsMapConfiguration = mapConfiguration;
	CreateMapAreaFromConfiguration();

	itsZoomedArea = boost::shared_ptr<NFmiArea>(itsOriginalArea->Clone());
	itsSwapBaseArea = boost::shared_ptr<NFmiArea>(itsOriginalArea->Clone());
	InitializeBitmapVectors();

	const auto& mapFileNames = itsMapConfiguration->MapFileNames();
	if(mapFileNames.size() > 0) // pakko lukea 1. image muistiin, ett‰ saadaan koko talteen
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
	// pit‰‰ alustaa 0-pointtereilla image taulukko.
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
	try
	{
		if(itsUsedMapIndex >= 0 && itsUsedMapIndex < static_cast<int>(itsMapBitmaps.size()))
		{
			if(!itsMapBitmaps[itsUsedMapIndex])
				itsMapBitmaps[itsUsedMapIndex] = CreateBitmapFromFile(itsMapConfiguration->MapFileNames()[itsUsedMapIndex]);
			return itsMapBitmaps[itsUsedMapIndex];
		}
	}
	catch(std::exception& e)
	{
		CatLog::logMessage(e.what(), CatLog::Severity::Error, CatLog::Category::Configuration, true);
	}
	return nullptr;
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
			if(newArea->TopRightLatLon() == itsZoomedArea->TopRightLatLon() &&  // t‰m‰ ehto yritt‰‰ testata, ovatko alueet samoja
			   newArea->BottomLeftLatLon() == itsZoomedArea->BottomLeftLatLon())
				return ;
		}
		if(NFmiQueryDataUtil::AreAreasSameKind(itsOriginalArea.get(), newArea.get()))
		{
			itsZoomedArea = boost::shared_ptr<NFmiArea>(newArea->Clone());
		}
		else
		{ 
			// pit‰‰ luoda newArea kulmia k‰ytt‰en uusi zoomattu alue original areaa k‰ytt‰en, 
			// n‰in smartmetin karttan‰yttˆ ei mene sekaisin ja piirto mahdollisesti hidastu jos on k‰ytetty skandi n‰yttˆmakroa euro smartmetissa.
			itsZoomedArea.reset(itsOriginalArea->CreateNewArea(newArea->BottomLeftLatLon(), newArea->TopRightLatLon()));
		}
		CalcZoomedAreaPosition();
        SetMakeNewBackgroundBitmap(true);
		MapReallyChanged(true); // t‰m‰ nollataan zoom-dialogissa!!!! ks. NFmiZoomView::Update -metodi
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
		// lasketaan myˆs t‰m‰ suhteellinen zoomattu alue
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

// asettaa zoomin puoleksi koko alueesta ja keskelle
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
		finalFileName += kFmiDirectorySeparator; // pakko lis‰t‰ editorin tyˆhakemisto suhteelliseen tiedostonimeen, koska oletus hakemisto on saattanut muuttua esim. makro tallennuksissa jne.
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
		Area(itsSwapBaseArea); // zoomataan swap-baseen
		itsSwapMode = 1; // t‰m‰ pit‰‰ asettaa Area-metodin j‰lkeen
	}
	else
	{
		Area(itsSwapBackArea); // zoomataan swap-back:iin takaisin, swap-mode asetetaan Area-metodissa
	}
}

void NFmiGdiPlusImageMapHandler::DrawBorderPolyLineList(std::list<NFmiPolyline*> &newValue)
{
	itsCountryBorderPolylineCache.drawBorderPolyLineList(newValue);
}

void NFmiGdiPlusImageMapHandler::PolyLineListGdiplusInPixelCoordinates(const std::list<std::vector<Gdiplus::PointF>>& newValue)
{ 
	itsCountryBorderPolylineCache.polyLineListGdiplusInPixelCoordinates(newValue);
}

void NFmiGdiPlusImageMapHandler::PolyLineListGdiplusInPixelCoordinates(const std::list<std::vector<Gdiplus::PointF>>&& newValue)
{ 
	itsCountryBorderPolylineCache.polyLineListGdiplusInPixelCoordinates(std::move(newValue));
}

// Laitoin karttojen likaus systeemit uusiksi monellakin tapaa:
// 1. Nimi muuttui toivottavasti kuvaavammaksi eli MakeNewBackgroundBitmap, eli se tarkoittaa
//    ett‰ seuraavalla piirrooskierroksella karttapohjat pit‰‰ piirt‰‰ uusiksi.
// 2. Ns. set-funktiolla voi lipun laittaa vain p‰‰lle, false arvo j‰tet‰‰n huomiotta. Vain clear-funktiolla
//    lipun tila voidaan nollata. T‰m‰ muutos siksi ett‰ eri paikoista voi tulla samalla likaus kerralla eri arvoja
//    eik‰ toisaalta tehdyst‰ false asetuksesta ei haluta nollata toisesta tehty‰ true asetusta.
// 3. Lippu siis nollataan clear-funktiolla, jota kutsutaan piirto toimintojen j‰lkeen.
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

// Sama kysely, asetus ja nollaus mekanismi kuin MakeNewBackgroundBitmap -lipullekin
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

// Uusi border-draw-dirty systeemi ei laita lippuja p‰‰lle, vaan tyhjent‰‰ tarvittavat cachet
// jotta seuraavalla piirto kerralla on pakko tehd‰ tˆit‰.
void NFmiGdiPlusImageMapHandler::SetBorderDrawDirtyState(CountryBorderDrawDirtyState newState)
{
	itsCountryBorderPolylineCache.setBorderDrawDirtyState(newState);
}

bool NFmiGdiPlusImageMapHandler::BorderDrawPolylinesDirty() const
{
	return itsCountryBorderPolylineCache.borderDrawPolylinesDirty();
}

bool NFmiGdiPlusImageMapHandler::PolyLineListGdiplusInPixelCoordinatesDirty() const
{
	return itsCountryBorderPolylineCache.polyLineListGdiplusInPixelCoordinatesDirty();
}

std::list<NFmiPolyline*>& NFmiGdiPlusImageMapHandler::DrawBorderPolyLineList()
{
	return itsCountryBorderPolylineCache.drawBorderPolyLineList();
}

const std::list<std::vector<Gdiplus::PointF>>& NFmiGdiPlusImageMapHandler::PolyLineListGdiplusInPixelCoordinates()
{
	return itsCountryBorderPolylineCache.polyLineListGdiplusInPixelCoordinates();
}

std::shared_ptr<OGRGeometry> NFmiGdiPlusImageMapHandler::CountryBorderGeometry()
{
	return itsCountryBorderGeometry;
}

void NFmiGdiPlusImageMapHandler::CountryBorderGeometry(std::shared_ptr<OGRGeometry>& theGeometry)
{
	itsCountryBorderGeometry = theGeometry;
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

// xxxFromViewMacro metodien hakujen priorisointi:
// 1. Jos annettu referenceName lˆytyy xxxMacroReferenceNames vektorista, k‰ytet‰‰n sen indeksi‰
// 2. Muuten k‰ytet‰‰n suoraan annettua mapLayerIndex:ia
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

// xxxFromGui metodien hakujen priorisointi:
// 1. Etsit‰‰n lˆytyykˆ name:a xxxMapDescriptiveNames vektorista
// 2. Etsit‰‰n lˆytyykˆ name:a xxxMapMacroReferenceNames vektorista
// 3. Etsit‰‰n lˆytyykˆ name:a xxxMapFileNameBasedReferenceNames vektorista
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

static const std::string& GetLayerTextFromVector(int layerIndex, const std::vector<std::string>& layerNames)
{
	if(layerIndex >= 0 && layerIndex < layerNames.size())
		return layerNames[layerIndex];
	else
	{
		static const std::string emptyString;
		return emptyString;
	}
}

static std::string GetBestDescriptiveMapLayerText(int layerIndex, const std::vector<std::string>& descriptiveNames, const std::vector<std::string>& macroReferenceNames, const std::string& layerFileName)
{
	// 1. Jos lˆytyy ei-puuttuva descriptiveName, k‰ytet‰‰n sit‰.
	std::string descriptiveMapLayerText = ::GetLayerTextFromVector(layerIndex, descriptiveNames);
	if(descriptiveMapLayerText.empty())
	{
		// 2. Jos lˆytyy ei-puuttuva macroReferenceName, k‰ytet‰‰n sit‰.
		descriptiveMapLayerText = ::GetLayerTextFromVector(layerIndex, macroReferenceNames);
		if(descriptiveMapLayerText.empty())
		{
			// 3. Muutoin tehd‰‰n nimi kuvan tiedostonimest‰
			descriptiveMapLayerText = PathUtils::getFilename(layerFileName);
		}
	}
	return descriptiveMapLayerText;
}

std::string NFmiGdiPlusImageMapHandler::GetCurrentGuiMapLayerText(bool backgroundMap)
{
	return GetWantedGuiMapLayerText(backgroundMap, backgroundMap ? itsUsedMapIndex : itsUsedOverMapBitmapIndex);
}

// Priorisointi kun tehd‰‰n map-layer nimej‰ Gui:lle:
// 1. Descriptive name
// 2. Macro-reference name
// 3. V‰‰nnet‰‰n sopiva nimi bitmapin tiedosto nimest‰
std::string NFmiGdiPlusImageMapHandler::GetWantedGuiMapLayerText(bool backgroundMap, int wantedLayerIndex)
{
	// Jos ei ole valittuna overlay kerrosta, on indeksi negatiivinen ja silloin palautetaan tyhj‰‰
	if(wantedLayerIndex < 0)
		return "";

	if(backgroundMap)
		return ::GetBestDescriptiveMapLayerText(wantedLayerIndex, itsMapConfiguration->BackgroundMapDescriptiveNames(), itsMapConfiguration->BackgroundMapMacroReferenceNames(), GetBitmapFileName());
	else
		return ::GetBestDescriptiveMapLayerText(wantedLayerIndex, itsMapConfiguration->OverlayMapDescriptiveNames(), itsMapConfiguration->OverlayMapMacroReferenceNames(), GetOverMapBitmapFileName());
}

std::string NFmiGdiPlusImageMapHandler::GetCurrentMacroReferenceName(bool backgroundMap)
{
	return GetWantedMacroReferenceName(backgroundMap, backgroundMap ? itsUsedMapIndex : itsUsedOverMapBitmapIndex);
}

std::string NFmiGdiPlusImageMapHandler::GetWantedMacroReferenceName(bool backgroundMap, int wantedLayerIndex)
{
	if(backgroundMap)
		return ::GetLayerTextFromVector(wantedLayerIndex, itsMapConfiguration->BackgroundMapMacroReferenceNames());
	else
		return ::GetLayerTextFromVector(wantedLayerIndex, itsMapConfiguration->OverlayMapMacroReferenceNames());
}

