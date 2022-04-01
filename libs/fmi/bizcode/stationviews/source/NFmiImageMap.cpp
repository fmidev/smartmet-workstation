//© Ilmatieteenlaitos/Marko Pietarinen
//  Original 19.02.2016
//
//
//-------------------------------------------------------------------- NFmiImageMap.cpp
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255 merkkiä joka johtuu 'puretuista' STL-template nimistä)
#endif

#include "NFmiImageMap.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiFileSystem.h"
#include "NFmiStringTools.h"
#include "catlog/catlog.h"

#include <list>

using namespace std;
using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Tähän luokkaan luetaan Mirwa analyyseissä käytettyjä kuvia.
// Kuvat mapataan taikasanan taakse std::map:iin.
// Kullekin kuvalle on kaksi kokoa, yksi ruudulle ja toinen printterille.
NFmiImageMap::ImageHolder::ImageHolder(void)
    :itsImage(0)
    ,itsImageFile()
{}

NFmiImageMap::ImageHolder::ImageHolder(const std::string &theImageFile)
    :itsImage(0)
    ,itsImageFile(theImageFile)
{}

void NFmiImageMap::ImageHolder::ReadImage(void)
{
    delete itsImage;
    itsImage = 0;
    try
    {
        itsImage = CtrlView::CreateBitmapFromFile(itsImageFile);
    }
    catch(std::exception &e)
    {
        std::string errorMessage("Error while reading image (continuing): ");
        errorMessage += e.what();
        CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration);
    }
}

NFmiImageMap::ImagesHolder::ImagesHolder(void)
    :itsScreenImageHolder()
    ,itsPrinterImageHolder()
{}

NFmiImageMap::ImagesHolder::ImagesHolder(const std::string &theScreenImageFile, const std::string &thePrintedImageFile)
    :itsScreenImageHolder(theScreenImageFile)
    ,itsPrinterImageHolder(thePrintedImageFile)
{}

void NFmiImageMap::ImagesHolder::ReadImages(void)
{
    itsScreenImageHolder.ReadImage();
    itsPrinterImageHolder.ReadImage();
}

NFmiImageMap::NFmiImageMap(void)
:itsImageBaseFolder()
,itsSymbolMap()
,fInitialized(false)
,fOperational(false)
{
}

// initialisoinnissa luetaan bitmapit tiedostoista, kutsu vasta kun itsBitmapFolder-dataosa on asetettu
void NFmiImageMap::Initialize(const std::string &theImageBaseFolder, const std::string &theInitFile)  // HUOM! heittää poikkeuksia epäonnistuessaan
{
    fInitialized = true;
    itsSymbolMap.clear();
    itsImageBaseFolder = theImageBaseFolder;
    if(!NFmiFileSystem::DirectoryExists(itsImageBaseFolder))
        throw std::runtime_error(std::string("Error in MirwaSymbolMap::Initialize - the given base imagefolder doesn't exist:\n'") + theImageBaseFolder + "'");

    // init tiedoston nimen voi antaa tyhjänä, jolloin ei tehdä mitään.
    if(!theInitFile.empty())
    {
	    std::string fileContent;
	    if(NFmiFileSystem::ReadFile2String(theInitFile, fileContent))
	    {
		    std::stringstream in(fileContent);

		    const int maxBufferSize = 1024; // kuinka pitkä yhden rivin maksimissaan oletetaan olevan
		    std::string buffer;
		    int i = 0;
		    int counter = 0;
		    do
		    {
			    buffer.resize(maxBufferSize);
			    in.getline(&buffer[0], maxBufferSize);
			    size_t realSize = strlen(buffer.c_str());
			    buffer.resize(realSize);
                std::pair<std::string, ImagesHolder> imageHolderMapItem = GetImageHolderInfo(buffer, theInitFile);
                if(!imageHolderMapItem.first.empty())
                {
                    imageHolderMapItem.second.ReadImages();
                    itsSymbolMap.insert(imageHolderMapItem);
                }
        	}while(in.good());
        }
        else
            throw std::runtime_error(std::string("Error in MirwaSymbolMap::Initialize - reading failed from file:\n") + theInitFile);
    }
    fOperational = true;
}

Gdiplus::Bitmap* NFmiImageMap::GetSymbolImage(const std::string &theSymbolCodeStr, bool fPrinted)
{
    if(fOperational)
    {
        std::map<std::string, ImagesHolder>::iterator it = itsSymbolMap.find(theSymbolCodeStr);
        if(it != itsSymbolMap.end())
        {
            if(fPrinted)
                return it->second.itsPrinterImageHolder.itsImage;
            else
                return it->second.itsScreenImageHolder.itsImage;
        }
    }
    return 0;
}

std::pair<std::string, NFmiImageMap::ImagesHolder> NFmiImageMap::GetImageHolderInfo(const std::string &theConfFileLine, const std::string &theConfFileName)
{
    std::string line(theConfFileLine);
    NFmiStringTools::TrimAll(line);
    if(!line.empty())
    {
        if(line.size() > 0 && line[0] == '#')
            return std::pair<std::string, ImagesHolder>(); // rivi oli #-kommentti
        if(line.size() > 1 && line[0] == '/' && line[1] == '/')
            return std::pair<std::string, ImagesHolder>(); // rivi oli //-kommentti

        std::vector<std::string> parts = NFmiStringTools::Split(line, ";");
        if(parts.size() == 3)
        {
            std::string file1 = parts[1];
            file1 = NFmiFileSystem::MakeAbsolutePath(file1, itsImageBaseFolder);
            std::string file2 = parts[2];
            file2 = NFmiFileSystem::MakeAbsolutePath(file2, itsImageBaseFolder);
            ImagesHolder imageHolder(file1, file2);
            return make_pair(parts[0], imageHolder);
        }
        else if(parts.size() == 2 || parts.size() > 3) // tehdään poikkeus, jos riviltä on löytynyt ';'-merkkejä, mutta ei tasan kahta kappaletta
        {
            std::string errStr("Error in MirwaSymbolMap::GetImageHolderInfo - malformatted line:\n");
            errStr += theConfFileLine;
            errStr += "\nIn configuration file: ";
            errStr += theConfFileName;
            errStr += "\nThere should be three string values separated by two semicolons (;)";
            errStr += "\nLike following: codeword;imagefile1;imagefile2";
            throw std::runtime_error(errStr);
        }
    }
    return std::pair<std::string, ImagesHolder>(); // rivi oli tyhjä
}

// 1. Jos ei printata (= ei tarvita välttämättä isoa/paras laatuista kuvaa), katsotaan löytyykö pienempi kuva
// 2. Jos löytyy ja sen pikseli koko vastaa haluttua kuvan koko, palautetaan se.
// 3. Kaikissa muissa tapauksissa yritetään palauttaa iso kuva (löytyi se tai ei)
Gdiplus::Bitmap* NFmiImageMap::GetRightSizeImage(double drawedSymbolSizeInPixels, bool isPrinting, const std::string &theSymbolCode)
{
    if(!isPrinting)
    {
        Gdiplus::Bitmap *smallerSymbolImage = GetSymbolImage(theSymbolCode, false); // false = hae pienempää kuvaa
        if(smallerSymbolImage)
        {
            if(smallerSymbolImage->GetHeight()*1.2 >= drawedSymbolSizeInPixels) // Jos pienempi kuvista on tarpeeksi lähellä haluttua piirtokoko tai pienempi, käytetään pientä kuvaa
                return smallerSymbolImage;
        }
    }
    // Kokeillaan löytyykö isompaa kuvaa true parametrilla
    return GetSymbolImage(theSymbolCode, true);
}
