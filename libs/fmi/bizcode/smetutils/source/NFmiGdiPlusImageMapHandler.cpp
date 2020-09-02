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
#include "CtrlViewTimeConsumptionReporter.h"

using namespace std;

namespace
{
	void clearBitmapVector(checkedVector<Gdiplus::Bitmap*>& theBitmaps)
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
,itsAreaFileName()
,itsMapFileNames()
,itsMapDrawStyles()
,itsOverMapBitmapFileNames()
,itsOverMapBitmapDrawStyles()
,itsControlPath()
,itsSwapBaseArea()
,itsSwapBackArea()
,itsSwapMode(0)
,itsCountryBorderPolylineCache()
,itsLandBorderPath()
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
	, itsAreaFileName()
	, itsMapFileNames()
	, itsMapDrawStyles()
	, itsOverMapBitmapFileNames()
	, itsOverMapBitmapDrawStyles()
	, itsControlPath()
	, itsSwapBaseArea(nullptr)
	, itsSwapBackArea(nullptr)
	, itsSwapMode(0)
	, itsCountryBorderPolylineCache()
	, itsLandBorderPath()
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
		itsAreaFileName = other.itsAreaFileName;
		itsMapFileNames = other.itsMapFileNames;
		itsMapDrawStyles = other.itsMapDrawStyles;
		itsOverMapBitmapFileNames = other.itsOverMapBitmapFileNames;
		itsOverMapBitmapDrawStyles = other.itsOverMapBitmapDrawStyles;
		itsControlPath = other.itsControlPath;
		itsSwapBaseArea.reset(::MakeNewAreaClone(other.itsSwapBaseArea));
		itsSwapBackArea.reset(::MakeNewAreaClone(other.itsSwapBackArea));
		itsSwapMode = other.itsSwapMode;
		itsCountryBorderPolylineCache = other.itsCountryBorderPolylineCache;
		itsLandBorderPath = other.itsLandBorderPath;
		InitializeBitmapVectors();
	}
	return *this;
}

NFmiGdiPlusImageMapHandler::~NFmiGdiPlusImageMapHandler()
{
	Clear();
}

void NFmiGdiPlusImageMapHandler::Clear()
{
	::clearBitmapVector(itsMapBitmaps);
	::clearBitmapVector(itsOverMapBitmaps);
	itsMapFileNames.clear();
	itsMapDrawStyles.clear();
	itsOverMapBitmapFileNames.clear();
	itsOverMapBitmapDrawStyles.clear();
	itsCountryBorderPolylineCache.clearCache();
}

bool NFmiGdiPlusImageMapHandler::Init(const std::string& theAreaFileName, const checkedVector<std::string> &theMapFileNames, const checkedVector<int> &theMapDrawStyles, const checkedVector<std::string> &theOverMapBitmapFileNames, const checkedVector<int> &theOverMapBitmapDrawStyles)
{
	CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, __FUNCTION__);

	itsAreaFileName = PathUtils::makeFixedAbsolutePath(theAreaFileName, itsControlPath);

	itsOriginalArea = ReadArea(itsAreaFileName);
	if(!itsOriginalArea)
	{
		string errMsg("NFmiGdiPlusImageMapHandler::Init - ei saanut luettua area-tiedostoa: \n");
		errMsg += itsAreaFileName;
		errMsg += ", originally gives as: ";
		errMsg += theAreaFileName;
		throw runtime_error(errMsg);
	}

	return Init(theMapFileNames, theMapDrawStyles, theOverMapBitmapFileNames, theOverMapBitmapDrawStyles);
}

bool NFmiGdiPlusImageMapHandler::Init(const checkedVector<std::string> &theMapFileNames, const checkedVector<int> &theMapDrawStyles, const checkedVector<std::string> &theOverMapBitmapFileNames, const checkedVector<int> &theOverMapBitmapDrawStyles)
{
	itsMapFileNames = theMapFileNames;
	itsMapDrawStyles = theMapDrawStyles;
	itsOverMapBitmapFileNames = theOverMapBitmapFileNames;
	itsOverMapBitmapDrawStyles = theOverMapBitmapDrawStyles;

	itsZoomedArea = boost::shared_ptr<NFmiArea>(itsOriginalArea->Clone());
	itsSwapBaseArea = boost::shared_ptr<NFmiArea>(itsOriginalArea->Clone());
	InitializeBitmapVectors();

	if(itsMapFileNames.size() > 0) // pakko lukea 1. image muistiin, ett‰ saadaan koko talteen
	{
		itsMapBitmaps[0] = CreateBitmapFromFile(itsMapFileNames[0]);
		if(itsMapBitmaps[0] == 0)
		{
			string errMsg("NFmiGdiPlusImageMapHandler::Init - ei saanut luettua kuva-tiedostoa: \n");
			errMsg += itsMapFileNames[0];
			throw runtime_error(errMsg);
		}
		CalcZoomedAreaPosition();
	}

	return true;
}

void NFmiGdiPlusImageMapHandler::InitializeBitmapVectors()
{
	// pit‰‰ alustaa 0-pointtereilla image taulukko.
	for(auto mapIndex = 0ul; mapIndex < static_cast<int>(itsMapFileNames.size()); mapIndex++)
		itsMapBitmaps.push_back(nullptr);
	for(auto mapIndex = 0ul; mapIndex < static_cast<int>(itsOverMapBitmapFileNames.size()); mapIndex++)
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
			itsMapBitmaps[itsUsedMapIndex] = CreateBitmapFromFile(itsMapFileNames[itsUsedMapIndex]);
		return itsMapBitmaps[itsUsedMapIndex];
	}
	return 0;
}

void NFmiGdiPlusImageMapHandler::OriginalArea(const std::string& theArea)
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
		{ // pit‰‰ luoda newArea kulmia k‰ytt‰en uusi zoomattu alue original areaa k‰ytt‰en (n‰in smartmetin karttan‰yttˆ ei mene sekaisin ja piirto mahdollisesti hidastu jos on k‰ytetty skandi n‰yttˆmakroa euro smartmetissa)
			string origAreaStr = itsOriginalArea->AreaStr();
			string newAreaStr = newArea->AreaStr();
			string::size_type pos1 = origAreaStr.find(":");
			string::size_type pos2 = newAreaStr.find(":");
			if(pos1 != string::npos && pos2 != string::npos)
			{
				string newZoomedAreaStr(origAreaStr.begin(), origAreaStr.begin()+pos1); // otetaan alkuosio (area-tyyppi) originaali areasta
				newZoomedAreaStr += string(newAreaStr.begin()+pos2, newAreaStr.end()); // otetaan kulmapisteet uudesta alueesta
				boost::shared_ptr<NFmiArea> tmpArea = NFmiAreaFactory::Create(newZoomedAreaStr);
				if(tmpArea.get() == 0)
					return ; // jokin meni pieleen, ei tehd‰ mit‰‰n
				else
				{
					itsZoomedArea = tmpArea;
				}
			}
			else
				return ; // jokin meni pieleen, ei tehd‰ mit‰‰n
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
			itsOverMapBitmaps[itsUsedOverMapBitmapIndex] = CreateBitmapFromFile(itsOverMapBitmapFileNames[itsUsedOverMapBitmapIndex]);
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
	if(itsUsedMapIndex >= 0 && itsUsedMapIndex < static_cast<int>(itsMapDrawStyles.size()))
	{
		return itsMapDrawStyles[itsUsedMapIndex];
	}
	return 0;
}

int NFmiGdiPlusImageMapHandler::GetOverMapDrawStyle()
{
	if(itsUsedOverMapBitmapIndex >= 0 && itsUsedOverMapBitmapIndex < static_cast<int>(itsOverMapBitmapDrawStyles.size()))
	{
		return itsOverMapBitmapDrawStyles[itsUsedOverMapBitmapIndex];
	}
	return 0;
}

const std::string& NFmiGdiPlusImageMapHandler::GetBitmapFileName()
{
	static const std::string dummy;
	if(itsUsedMapIndex >= 0 && itsUsedMapIndex < static_cast<int>(itsMapDrawStyles.size()))
	{
		return itsMapFileNames[itsUsedMapIndex];
	}
	return dummy;
}

const std::string& NFmiGdiPlusImageMapHandler::GetOverMapBitmapFileName()
{
	static const std::string dummy;
	if(itsUsedOverMapBitmapIndex >= 0 && itsUsedOverMapBitmapIndex < static_cast<int>(itsOverMapBitmapDrawStyles.size()))
	{
		return itsOverMapBitmapFileNames[itsUsedOverMapBitmapIndex];
	}
	return dummy;
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

void NFmiGdiPlusImageMapHandler::DrawBorderPolyLineListGdiplus(const std::list<std::vector<NFmiPoint>>& newValue)
{ 
	itsCountryBorderPolylineCache.drawBorderPolyLineListGdiplus(newValue);
}

void NFmiGdiPlusImageMapHandler::DrawBorderPolyLineListGdiplus(std::list<std::vector<NFmiPoint>>&& newValue)
{ 
	itsCountryBorderPolylineCache.drawBorderPolyLineListGdiplus(std::move(newValue));
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
