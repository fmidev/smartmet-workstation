#include "NFmiFixedDrawParamSystem.h"
#include "NFmiStringTools.h"
#include "NFmiFileSystem.h"
#include "NFmiDrawParam.h"

// T‰m‰ funktio muuttaa annetut polut seuraavaan muotoon:
// 1. kaikki polkuerottimet ovat yksinkertaisia /- tai \-merkkej‰ (riippuu OS:‰st‰)
// 2. kaikki polut loppuvat /- tai \-merkkiin
static std::string ConvertPathToWantedFormat(const std::string &thePath, bool fAddDeliminorAtEnd)
{
    const char wantedDeliminor = kFmiDirectorySeparator;
    std::string formattedPath(thePath);
    NFmiStringTools::ReplaceChars(formattedPath, '\\', wantedDeliminor); // Muutetaan 'v‰‰r‰‰n' suuntaan olevat kenoviivat kaikkialla (win/lin/mac) sopiviksi
    NFmiStringTools::ReplaceAll(formattedPath, "//", "/"); // jos on tupla kenoja, muutetaan ne yksinkertaisiksi
    if(fAddDeliminorAtEnd && !(formattedPath.empty()) && formattedPath[formattedPath.size() - 1] != wantedDeliminor)
        formattedPath += wantedDeliminor; // Lis‰t‰‰n tarvittaessa loppuun kenoviiva
    return formattedPath;
}

// Konvertoi polun haluttuun formaattiin ja tarkistaa ett‰ se ei ole tyhj‰ ja 
// ett‰ sen on absoluuttinen polku.
static std::string DoPathChecksAndConversions(const std::string &thePath, const std::string &theCallingFunctionName, bool fAddDeliminorAtEnd, bool fCheckAbsolute)
{
    std::string path = ::ConvertPathToWantedFormat(thePath, fAddDeliminorAtEnd);
    if(path.empty())
        throw std::runtime_error(std::string("Error in ") + theCallingFunctionName + ": given theRootFolderPath was empty, error in program.");
    if(fCheckAbsolute && !NFmiFileSystem::IsAbsolutePath(path))
        throw std::runtime_error(std::string("Error in ") + theCallingFunctionName + ": given theRootFolderPath was not absolute path, error in program.");
    return path;
}

// **************************************************
// ***** NFmiFixedDrawParamFolder *******************
// **************************************************

NFmiFixedDrawParamFolder::NFmiFixedDrawParamFolder()
:itsFolderPath()
,itsSubFolderName()
,itsSubFolders()
,itsDrawParams()
{
}

NFmiFixedDrawParamFolder::~NFmiFixedDrawParamFolder()
{
}

static const std::string gDrawParamFileExtension = ".dpa";

void NFmiFixedDrawParamFolder::Initialize(const std::string &theFolderPath, std::string theSubFolderName)
{
    itsFolderPath = ::DoPathChecksAndConversions(theFolderPath, __FUNCTION__, true, true);
    itsSubFolderName = theSubFolderName;

    // Luetaan ensin lapsihakemistot
    std::list<std::string> directories = NFmiFileSystem::Directories(itsFolderPath);
    for(auto directory : directories)
    {
        if(directory == "." || directory == "..")
            continue; // ohitetaan currentti ja emohakemisto

        NFmiFixedDrawParamFolder subFolder;
        subFolder.Initialize(itsFolderPath + directory, directory);
        itsSubFolders.push_back(subFolder);
    }

    // Luetaan sitten t‰ss‰ hakemistossa olevat drawParamit
    std::string filePattern = itsFolderPath;
    filePattern += "*" + gDrawParamFileExtension;
    std::list<std::string> drawParamFiles = NFmiFileSystem::PatternFiles(filePattern);
    for(auto fileName : drawParamFiles)
    {
        std::string finalFileName = itsFolderPath + fileName;
        std::shared_ptr<NFmiDrawParam> drawParam = std::make_shared<NFmiDrawParam>();
        if(drawParam->Init(finalFileName)) // vain onnistuneesti luetut drawParamit lis‰t‰t‰‰n listaan
                itsDrawParams.push_back(drawParam);
    }
}

// Lis‰t‰‰n folderissa olleiden alihakemistojen sis‰ltˆ ensin listaan, 
// lopuksi t‰ss‰ hakemistossa oleet drawParamit.
void NFmiFixedDrawParamFolder::AddToFlatList(std::vector<std::shared_ptr<NFmiDrawParam>> &theFlatDrawParamList)
{
    for(auto childFolder : itsSubFolders)
    {
        childFolder.AddToFlatList(theFlatDrawParamList);
    }

    for(auto drawParam : itsDrawParams)
        theFlatDrawParamList.push_back(drawParam);
}

bool NFmiFixedDrawParamFolder::Empty() const
{
    if(!itsDrawParams.empty())
        return false;

    for(auto childFolder : itsSubFolders)
    {
        if(!childFolder.Empty())
            return false;
    }
    return true;
}

// **************************************************
// ***** NFmiFixedDrawParamFolder *******************
// **************************************************


// **************************************************
// ***** NFmiFixedDrawParamSystem *******************
// **************************************************

NFmiFixedDrawParamSystem::NFmiFixedDrawParamSystem()
:itsRootFolder()
,itsFlatDrawParamList()
,itsCurrentFlatListSelection(-1)
{
}


NFmiFixedDrawParamSystem::~NFmiFixedDrawParamSystem()
{
}

void NFmiFixedDrawParamSystem::Initialize(const std::string &theRootFolderPath)
{
    Clear();
    itsRootFolder.Initialize(theRootFolderPath, ""); // rootille annetaan tyhj‰n‰ theSubFolderName -parametri
    MakeFlatList();
}

void NFmiFixedDrawParamSystem::Clear()
{
    itsCurrentFlatListSelection = -1;
    itsRootFolder = NFmiFixedDrawParamFolder();
    itsFlatDrawParamList.clear();
}

void NFmiFixedDrawParamSystem::MakeFlatList()
{
    itsFlatDrawParamList.clear();
    itsRootFolder.AddToFlatList(itsFlatDrawParamList);
}

const std::shared_ptr<NFmiDrawParam>& NFmiFixedDrawParamSystem::GetDrawParam(const std::string &theDrawParamInitName) const
{
    auto foundIter = std::find_if(itsFlatDrawParamList.begin(), itsFlatDrawParamList.end(), [&](const std::shared_ptr<NFmiDrawParam> &drawParam){return drawParam->InitFileName() == theDrawParamInitName; });
    if(foundIter != itsFlatDrawParamList.end())
    {
        itsCurrentFlatListSelection = static_cast<int>(foundIter - itsFlatDrawParamList.begin());
        return *foundIter;
    }
    else
    {
        static std::shared_ptr<NFmiDrawParam> dummy;
        return dummy;
    }
}

// T‰m‰ theRelativeDrawParamPath sis‰lt‰‰ siis suhteellisen polun, miss‰ on vajaa tiedosto nimi (ilman dpa -p‰‰tett‰).
// Pit‰‰ etsi‰ puusta sellainen drawParam, mik‰ sopii annettuun polkuun alihakemisto alihakemistolta.
const std::shared_ptr<NFmiDrawParam>& NFmiFixedDrawParamSystem::GetRelativePathDrawParam(const std::string &theRelativeDrawParamPath) const
{
    try
    {
        std::string totalDrawParamPath = itsRootFolder.FolderPath() + ::DoPathChecksAndConversions(theRelativeDrawParamPath, __FUNCTION__, false, false) + gDrawParamFileExtension;
        return GetDrawParam(totalDrawParamPath);
    }
    catch(...)
    {
        static std::shared_ptr<NFmiDrawParam> dummy;
        return dummy;
    }
}

const std::shared_ptr<NFmiDrawParam>& NFmiFixedDrawParamSystem::GetCurrentDrawParam() const
{
    if(itsFlatDrawParamList.size() && itsCurrentFlatListSelection >= 0 && itsCurrentFlatListSelection < itsFlatDrawParamList.size())
    {
        return itsFlatDrawParamList[itsCurrentFlatListSelection];
    }
    else
    {
        static std::shared_ptr<NFmiDrawParam> dummy;
        return dummy;
    }
}

void NFmiFixedDrawParamSystem::Next()
{
    if(itsFlatDrawParamList.size())
    {
        if(itsCurrentFlatListSelection < 0)
            itsCurrentFlatListSelection = 0; // alustamaton arvo asetetaan osoittamaan 1. drawParamia
        else
        {
            itsCurrentFlatListSelection++;
            if(itsCurrentFlatListSelection >= static_cast<int>(itsFlatDrawParamList.size()))
                itsCurrentFlatListSelection = 0; // yli mennyt indeksi asetetaan osoittamaan 1. drawParamia
        }
    }
}

void NFmiFixedDrawParamSystem::Previous()
{
    if(itsFlatDrawParamList.size())
    {
        if(itsCurrentFlatListSelection < 0)
            itsCurrentFlatListSelection = 0; // alustamaton arvo asetetaan osoittamaan 1. drawParamia
        else
        {
            itsCurrentFlatListSelection--;
            if(itsCurrentFlatListSelection < 0)
                itsCurrentFlatListSelection = static_cast<int>(itsFlatDrawParamList.size() - 1); // ali mennyt indeksi asetetaan osoittamaan viimeist‰ drawParamia
        }
    }
}

// **************************************************
// ***** NFmiFixedDrawParamSystem *******************
// **************************************************
