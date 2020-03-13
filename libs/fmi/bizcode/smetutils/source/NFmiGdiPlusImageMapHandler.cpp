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

using namespace std;

//template<typename T>
struct PointerDestroyer
{
	template<typename T>
	void operator()(T* thePtr)
	{delete thePtr;}
};

NFmiGdiPlusImageMapHandler::NFmiGdiPlusImageMapHandler(void)
:itsUsedMapIndex(0)
,itsUsedOverMapBitmapIndex(-1)
,itsMapBitmaps()
,itsOverMapBitmaps()
,itsOriginalArea()
,itsZoomedArea()
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
,itsDrawBorderPolyLineList()
,itsDrawBorderPolyLineListGdiplus()
{
}

NFmiGdiPlusImageMapHandler::~NFmiGdiPlusImageMapHandler(void)
{
	Clear();
}

void NFmiGdiPlusImageMapHandler::Clear(void)
{
	ClearBitmapVector(itsMapBitmaps);
	ClearBitmapVector(itsOverMapBitmaps);

	itsMapFileNames.clear();
	itsMapDrawStyles.clear();
	itsOverMapBitmapFileNames.clear();
	itsOverMapBitmapDrawStyles.clear();
	std::for_each(itsDrawBorderPolyLineList.begin(), itsDrawBorderPolyLineList.end(), PointerDestroyer());
}

bool NFmiGdiPlusImageMapHandler::Init(const std::string& theAreaFileName, const checkedVector<std::string> &theMapFileNames, const checkedVector<int> &theMapDrawStyles, const checkedVector<std::string> &theOverMapBitmapFileNames, const checkedVector<int> &theOverMapBitmapDrawStyles)
{
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
	int i=0;
	// pit‰‰ alustaa 0-pointtereilla image taulukko.
	for(i=0; i < static_cast<int>(itsMapFileNames.size()); i++)
		itsMapBitmaps.push_back(nullptr);
	for(i=0; i < static_cast<int>(itsOverMapBitmapFileNames.size()); i++)
		itsOverMapBitmaps.push_back(nullptr);

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

boost::shared_ptr<NFmiArea> NFmiGdiPlusImageMapHandler::ReadArea(const string& theAreaFileName)
{
	if(NFmiFileSystem::FileExists(theAreaFileName) == false)
		throw runtime_error(std::string("Error: NFmiGdiPlusImageMapHandler::ReadArea - file does not exist:\n") + theAreaFileName);
	string areaStr;
	NFmiFileSystem::ReadFile2String(theAreaFileName, areaStr);
	return NFmiAreaFactory::Create(areaStr);
}

Gdiplus::Bitmap* NFmiGdiPlusImageMapHandler::GetBitmap(void)
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


void NFmiGdiPlusImageMapHandler::CalcZoomedAreaPosition(void)
{
	if(itsOriginalArea && itsZoomedArea)
	{
		// lasketaan myˆs t‰m‰ suhteellinen zoomattu alue
        itsZoomedAreaPosition = itsOriginalArea->XYArea(itsZoomedArea.get());
	}
}

boost::shared_ptr<NFmiArea> NFmiGdiPlusImageMapHandler::TotalArea(void)
{return itsOriginalArea;}

boost::shared_ptr<NFmiArea> NFmiGdiPlusImageMapHandler::Area(void)
{return itsZoomedArea;}

bool NFmiGdiPlusImageMapHandler::SetMaxArea(void)
{
	itsZoomedArea = boost::shared_ptr<NFmiArea>(itsOriginalArea->Clone());
	CalcZoomedAreaPosition();
    SetMakeNewBackgroundBitmap(true);
	return true;
}

// asettaa zoomin puoleksi koko alueesta ja keskelle
bool NFmiGdiPlusImageMapHandler::SetHalfArea(void)
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

Gdiplus::Bitmap* NFmiGdiPlusImageMapHandler::GetOverMapBitmap(void)
{
	if(itsUsedOverMapBitmapIndex >= 0 && itsUsedOverMapBitmapIndex < static_cast<int>(itsOverMapBitmaps.size()))
	{
		if(!itsOverMapBitmaps[itsUsedOverMapBitmapIndex])
			itsOverMapBitmaps[itsUsedOverMapBitmapIndex] = CreateBitmapFromFile(itsOverMapBitmapFileNames[itsUsedOverMapBitmapIndex]);
		return itsOverMapBitmaps[itsUsedOverMapBitmapIndex];
	}
	return 0;
}

bool NFmiGdiPlusImageMapHandler::ShowOverMap(void)
{
	if(itsUsedOverMapBitmapIndex >= 0 && itsUsedOverMapBitmapIndex < static_cast<int>(itsOverMapBitmaps.size()))
		return true;
	else
		return false;
}

void NFmiGdiPlusImageMapHandler::NextOverMap(void)
{
	int oldIndex = itsUsedOverMapBitmapIndex;
	itsUsedOverMapBitmapIndex++;
	if(itsUsedOverMapBitmapIndex >= static_cast<long>(itsOverMapBitmaps.size()))
		itsUsedOverMapBitmapIndex = -1;
	if(oldIndex != itsUsedOverMapBitmapIndex)
		EmptyBitmapVector(itsOverMapBitmaps);
}

void NFmiGdiPlusImageMapHandler::PreviousOverMap(void)
{
	int oldIndex = itsUsedOverMapBitmapIndex;
	itsUsedOverMapBitmapIndex--;
	if(itsUsedOverMapBitmapIndex < -1)
		itsUsedOverMapBitmapIndex = static_cast<long>(itsOverMapBitmaps.size()-1);
	if(oldIndex != itsUsedOverMapBitmapIndex)
		EmptyBitmapVector(itsOverMapBitmaps);
}

void NFmiGdiPlusImageMapHandler::ClearBitmapVector(checkedVector<Gdiplus::Bitmap*>& theBitmaps)
{
	EmptyBitmapVector(theBitmaps);
	theBitmaps.clear();
}

const NFmiRect& NFmiGdiPlusImageMapHandler::Position(void)
{
	return itsZoomedAreaPosition;
}

void NFmiGdiPlusImageMapHandler::NextMap(void)
{
	int oldIndex = itsUsedMapIndex;
	itsUsedMapIndex++;
	if(itsUsedMapIndex >= static_cast<long>(itsMapBitmaps.size()))
		itsUsedMapIndex = 0;
	if(oldIndex != itsUsedMapIndex)
		EmptyBitmapVector(itsMapBitmaps);
}

void NFmiGdiPlusImageMapHandler::PreviousMap(void)
{
	int oldIndex = itsUsedMapIndex;
	itsUsedMapIndex--;
	if(itsUsedMapIndex < 0)
		itsUsedMapIndex = static_cast<long>(itsMapBitmaps.size()-1);
	if(oldIndex != itsUsedMapIndex)
		EmptyBitmapVector(itsMapBitmaps);
}

// deletoi ja laittaa 0-pointterit vektoriin
void NFmiGdiPlusImageMapHandler::EmptyBitmapVector(checkedVector<Gdiplus::Bitmap*>& theBitmaps)
{
	for(unsigned int i=0; i < theBitmaps.size(); i++)
	{
		delete theBitmaps[i];
		theBitmaps[i] = 0;
	}
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
		EmptyBitmapVector(itsMapBitmaps);
	}
}

static BOOL IsImageGDIPLUSValid(CString filePathU_)
{
    Gdiplus::Bitmap image(filePathU_.AllocSysString());

	if( image.GetFlags() == Gdiplus::ImageFlagsNone )
		return FALSE;
	else
		return TRUE;
}

Gdiplus::Bitmap* NFmiGdiPlusImageMapHandler::CreateBitmapFromFile(const std::string &theFileName)
{
	return CtrlView::CreateBitmapFromFile(itsControlPath, theFileName);
}

int NFmiGdiPlusImageMapHandler::GetDrawStyle(void)
{
	if(itsUsedMapIndex >= 0 && itsUsedMapIndex < static_cast<int>(itsMapDrawStyles.size()))
	{
		return itsMapDrawStyles[itsUsedMapIndex];
	}
	return 0;
}

int NFmiGdiPlusImageMapHandler::GetOverMapDrawStyle(void)
{
	if(itsUsedOverMapBitmapIndex >= 0 && itsUsedOverMapBitmapIndex < static_cast<int>(itsOverMapBitmapDrawStyles.size()))
	{
		return itsOverMapBitmapDrawStyles[itsUsedOverMapBitmapIndex];
	}
	return 0;
}

const std::string& NFmiGdiPlusImageMapHandler::GetBitmapFileName(void)
{
	static const std::string dummy;
	if(itsUsedMapIndex >= 0 && itsUsedMapIndex < static_cast<int>(itsMapDrawStyles.size()))
	{
		return itsMapFileNames[itsUsedMapIndex];
	}
	return dummy;
}

const std::string& NFmiGdiPlusImageMapHandler::GetOverMapBitmapFileName(void)
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

const std::string NFmiGdiPlusImageMapHandler::GetBitmapAbsoluteFileName(void)
{
	return ::MakeAbsoluteFileName(GetBitmapFileName(), ControlPath());
}

const std::string NFmiGdiPlusImageMapHandler::GetOverMapBitmapAbsoluteFileName(void)
{
	return ::MakeAbsoluteFileName(GetOverMapBitmapFileName(), ControlPath());
}

void NFmiGdiPlusImageMapHandler::OverMapBitmapIndex(int newValue)
{
	if(itsUsedOverMapBitmapIndex != newValue && itsOverMapBitmaps.size() > 0)
	{
		itsUsedOverMapBitmapIndex = newValue;
		if(itsUsedOverMapBitmapIndex < 0)
			itsUsedOverMapBitmapIndex = -1;
		else if(itsUsedOverMapBitmapIndex >= static_cast<int>(itsOverMapBitmaps.size()))
			itsUsedOverMapBitmapIndex = static_cast<int>(itsOverMapBitmaps.size()-1);
		EmptyBitmapVector(itsOverMapBitmaps);
	}
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

NFmiRect NFmiGdiPlusImageMapHandler::TotalAbsolutRect(void)
{
    return ::CalcTotalAbsolutRect(GetBitmap());
}

NFmiRect NFmiGdiPlusImageMapHandler::TotalAbsolutRectOverMap(void)
{
    return ::CalcTotalAbsolutRect(GetOverMapBitmap());
}

NFmiRect NFmiGdiPlusImageMapHandler::ZoomedAbsolutRect(void)
{
    return ::CalcZoomedAbsolutRect(GetBitmap(), itsOriginalArea, itsZoomedArea);
}

NFmiRect NFmiGdiPlusImageMapHandler::ZoomedAbsolutRectOverMap(void)
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

double NFmiGdiPlusImageMapHandler::BitmapAspectRatio(void)
{
	return ::CalcBitmapAspectRatio(GetBitmap());
}

double NFmiGdiPlusImageMapHandler::BitmapAspectRatioOverMap(void)
{
    return ::CalcBitmapAspectRatio(GetOverMapBitmap());
}

void NFmiGdiPlusImageMapHandler::MakeSwapBaseArea(void)
{
	itsSwapBaseArea = boost::shared_ptr<NFmiArea>(itsZoomedArea->Clone());
	itsSwapMode = 0;
}

void NFmiGdiPlusImageMapHandler::SwapArea(void)
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
	// tuhotaan ensin vanhan listan sis‰ltˆ
	std::for_each(itsDrawBorderPolyLineList.begin(), itsDrawBorderPolyLineList.end(), PointerDestroyer());
	// kopioidaan uuden listan sis‰ltˆ
	itsDrawBorderPolyLineList = newValue;
	// tyhjennet‰‰n parametrina annettu lista Mutta ei tuhota polylinej‰ sen sis‰ll‰, koska ne j‰‰v‰t dokumenttiin talteen
	newValue.clear();
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
