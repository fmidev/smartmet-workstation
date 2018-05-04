#include "FmiSmarttoolsTabDlgHelpers.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

SmartToolMacroData::SmartToolMacroData()
    :itsOriginalName()
    , itsModifiedTime()
    , fIsDirectory(false)
    , itsDisplaydedName()
    , itsGridControlRowNumber(-1)
    , itsDirectoryContent()
    , itsTotalSize(0)
    , itsTreePatternArray()
{
}

SmartToolMacroData::SmartToolMacroData(SmartToolMacroData &&other)
    :itsOriginalName(std::move(other.itsOriginalName))
    , itsModifiedTime(std::move(other.itsModifiedTime))
    , fIsDirectory(std::move(other.fIsDirectory))
    , itsDisplaydedName(std::move(other.itsDisplaydedName))
    , itsGridControlRowNumber(std::move(other.itsGridControlRowNumber))
    , itsDirectoryContent(std::move(other.itsDirectoryContent))
    , itsTotalSize(std::move(other.itsTotalSize))
    , itsTreePatternArray(std::move(other.itsTreePatternArray))
{
}

SmartToolMacroData& SmartToolMacroData::operator=(SmartToolMacroData &&other)
{
    if(this != &other)
    {
        itsOriginalName = std::move(other.itsOriginalName);
        itsModifiedTime = std::move(other.itsModifiedTime);
        fIsDirectory = std::move(other.fIsDirectory);
        itsDisplaydedName = std::move(other.itsDisplaydedName);
        itsGridControlRowNumber = std::move(other.itsGridControlRowNumber);
        itsDirectoryContent = std::move(other.itsDirectoryContent);
        itsTotalSize = std::move(other.itsTotalSize);
        itsTreePatternArray = std::move(other.itsTreePatternArray);
    }
    return *this;
}

bool SmartToolMacroData::operator<(const SmartToolMacroData &other) const
{
    if(fIsDirectory != other.fIsDirectory)
        return fIsDirectory == true; // Jos toinen on hakemisto ja toinen ei, on hakemisto ensin pienuusj‰rjestyksess‰
    else
        return  boost::algorithm::ilexicographical_compare(itsDisplaydedName, other.itsDisplaydedName); // Muuten j‰rjestys displayed nimen mukaisesti cas insensitiivisesti
}

// Onko annetussa polussa hakemisto osiota alussa
static bool HasParentPath(const std::string &theMacroPath)
{
    boost::filesystem::path p = theMacroPath;
    return p.has_parent_path();
}

static std::string GetFileStem(const std::string &theMacroPath)
{
    boost::filesystem::path p = theMacroPath;
    return p.stem().string();
}

// Pilkotaan polku osiin eli esim. "marko/apuhakis/macro1.st" => "marko" ja "apuhakis/macro1.st"
static std::pair<std::string, std::string> SplitFromBaseParentPath(const std::string &theMacroPath)
{
    size_t pos = theMacroPath.find_first_of("\\/"); // Oletus: theMacroPath alussa ei voi olla kenoviivoja
    if(pos != std::string::npos)
        return std::make_pair(std::string(theMacroPath.begin(), theMacroPath.begin() + pos), std::string(theMacroPath.begin() + pos + 1, theMacroPath.end()));

    throw std::runtime_error("Not a valid path given in SplitFromBaseParentPath -function");
}

static std::time_t GetLastWriteTime(const std::string &theFullPath)
{
    boost::system::error_code er;
    return boost::filesystem::last_write_time(theFullPath, er);
}

static std::string MakeAbsolutePath(const std::string &theRelativePath, const std::string &theAbsoluteRootPath)
{
    boost::filesystem::path directoryPath = theAbsoluteRootPath;
    directoryPath.append(theRelativePath);
    return directoryPath.string();
}

static std::string MakeAbsoluteDirectoryPath(const std::string &theOriginalRelativeMacroPath, const std::string &theAbsoluteRootPath)
{
    boost::filesystem::path relativeMacroPath = theOriginalRelativeMacroPath;
    boost::filesystem::path directoryPath = theAbsoluteRootPath;
    directoryPath.append(relativeMacroPath.parent_path().string());
    return directoryPath.string();
}

void SmartToolMacroData::InsertMacro(const std::string &theMacroPath, const std::string &theOriginalMacroPath, const std::string &theAbsoluteRootPath)
{
    if(fIsDirectory) // vain hakemistoihin voi lis‰t‰ mit‰‰n
    {
        if(::HasParentPath(theMacroPath))
        { // Pilkotaan polku osiin eli esim. "marko/apuhakis/macro1.st" => "marko" ja "apuhakis/macro1.st"
            std::pair<std::string, std::string> pathData = ::SplitFromBaseParentPath(theMacroPath);
            SmartToolMacroData &macroDirectory = FindOrCreateDirectory(pathData.first, theOriginalMacroPath, theAbsoluteRootPath);
            macroDirectory.InsertMacro(pathData.second, theOriginalMacroPath, theAbsoluteRootPath); // Rekursio hoitaa puurakenteen
        }
        else
        { // Lis‰t‰‰n makro t‰h‰n hakemistoon
            // Huom! Lis‰ys vector:iin tehd‰‰n ensin move:lla, ja lis‰ttyyn dataan pyydet‰‰n referenssi, jota aletaan muokkaamaan. T‰m‰ tehty, koska luokassa k‰ytetty unique_ptr:‰‰.
            itsDirectoryContent.push_back(std::move(SmartToolMacroData()));
            SmartToolMacroData &data = itsDirectoryContent.back();
            data.fIsDirectory = false;
            data.itsModifiedTime = NFmiTime(::GetLastWriteTime(::MakeAbsolutePath(theOriginalMacroPath, theAbsoluteRootPath)));
            data.itsOriginalName = theOriginalMacroPath;
            data.itsDisplaydedName = ::GetFileStem(theMacroPath);
        }
    }
}

static std::string MakeDisplayedDirectoryName(const std::string &theDirectoryName)
{
    std::string displayedName = "<";
    displayedName += theDirectoryName + ">";
    return displayedName;
}

SmartToolMacroData& SmartToolMacroData::AddNewDirectory(const std::string &theDirectoryName, const std::string &theOriginalMacroPath, const std::string &theAbsoluteRootPath)
{
    // Huom! Lis‰ys vector:iin tehd‰‰n ensin move:lla, ja lis‰ttyyn dataan pyydet‰‰n referenssi, jota aletaan muokkaamaan. T‰m‰ tehty, koska luokassa k‰ytetty unique_ptr:‰‰.
    itsDirectoryContent.push_back(std::move(SmartToolMacroData()));
    SmartToolMacroData &data = itsDirectoryContent.back();
    data.fIsDirectory = true;
    data.itsModifiedTime = NFmiTime(::GetLastWriteTime(::MakeAbsoluteDirectoryPath(theOriginalMacroPath, theAbsoluteRootPath)));
    data.itsOriginalName = theDirectoryName;
    data.itsDisplaydedName = ::MakeDisplayedDirectoryName(theDirectoryName);
    return data;
}

// Etsii lˆytyykˆ annettua hakemisto nime‰ itsDirectoryContent:ista ja palauttaa sen iteraattorin.
// Jos ei lˆydy, luo se ja paluta sen iteraattori.
SmartToolMacroData& SmartToolMacroData::FindOrCreateDirectory(const std::string &theDirectoryName, const std::string &theOriginalMacroPath, const std::string &theAbsoluteRootPath)
{
    auto iter = std::find_if(itsDirectoryContent.begin(), itsDirectoryContent.end(), [&](const SmartToolMacroData &macroData){return macroData.itsOriginalName == theDirectoryName; });
    if(iter != itsDirectoryContent.end())
        return *iter;
    else
    { // Luodaan uudelle hakemistolle data
        return AddNewDirectory(theDirectoryName, theOriginalMacroPath, theAbsoluteRootPath);
    }
}

void SmartToolMacroData::SortMacros()
{
    // Sortataan ensin oma sis‰ltˆ
    std::sort(itsDirectoryContent.begin(), itsDirectoryContent.end());
    for(auto &macroData : itsDirectoryContent)
    {
        if(macroData.fIsDirectory)
            macroData.SortMacros(); // sortataan sitten viel‰ kaikki kansiot erikseen (jotka sorttaavat omansa rekurion loppuun asti)
    }
}

bool SmartToolMacroData::IsRoot() const
{
    if(fIsDirectory)
        return itsDisplaydedName.empty();

    return false;
}

static void FillTreeLevelInfo(const SmartToolMacroData::ContainerType &theMacroDirectoryData, SmartToolMacroData::TreePatternTypePtr &theTreeLevelData, unsigned char theTreeLevel)
{
    for(const auto &macroData : theMacroDirectoryData)
    {
        theTreeLevelData->push_back(theTreeLevel);
        if(macroData.fIsDirectory)
            ::FillTreeLevelInfo(macroData.itsDirectoryContent, theTreeLevelData, theTreeLevel + 1);
    }
}

void SmartToolMacroData::MakeTreePatternArray()
{
    if(IsRoot())
    {
        itsTreePatternArray = std::make_unique<TreePatternType>();
        unsigned char treeLevel = 1;
        // Makrot n‰ytt‰v‰ grid-control t‰ytet‰‰n seuraavasti:
        // 1. Kaikki hakemistot ja niiden alihakemistot on jo sortattu.
        // 2. Aloita listan 1. makrosta/hakemistosta ja lis‰‰ siihen liittyv‰ tree-level
        // 3. Jos kyse oli hakemistosta, k‰y sen sis‰ltˆ l‰pi, mutta kasvata niille tree-levelin arvoa yhdell‰.
        ::FillTreeLevelInfo(itsDirectoryContent, itsTreePatternArray, treeLevel);

        CalcTotalTreeSize();
    }
}

static int CalcDirectorySize(const SmartToolMacroData::ContainerType &theMacroDirectoryData)
{
    int totalSize = static_cast<int>(theMacroDirectoryData.size());
    for(const auto &macroData : theMacroDirectoryData)
    {
        if(macroData.fIsDirectory)
            totalSize += ::CalcDirectorySize(macroData.itsDirectoryContent);
    }
    return totalSize;
}

void SmartToolMacroData::CalcTotalTreeSize()
{
    itsTotalSize = ::CalcDirectorySize(itsDirectoryContent);;
}

void SmartToolMacroData::Reset()
{
    itsOriginalName.clear();
    itsModifiedTime = NFmiMetTime::gMissingTime;
    fIsDirectory = false;
    itsDisplaydedName.clear();
    itsDirectoryContent.clear();
    if(itsTreePatternArray)
        itsTreePatternArray->clear();
}

static SmartToolMacroData* FindMacro(SmartToolMacroData::ContainerType &theDirectoryContent, int theGridControlRowIndex)
{
    for(auto &macroData : theDirectoryContent)
    {
        if(macroData.itsGridControlRowNumber == theGridControlRowIndex)
            return &macroData;
        if(macroData.fIsDirectory)
        {
            SmartToolMacroData *foundMacroData = ::FindMacro(macroData.itsDirectoryContent, theGridControlRowIndex);
            if(foundMacroData)
                return foundMacroData;
        }
    }

    return nullptr; // Ei lˆytynyt makroa t‰st‰ hakemistosta
}

// Kun makro listaa on klikattu, tiedet‰‰n mik‰ rivi on valittuna. Etsi kyseist‰ rivi‰ vastaava macroData.
// Palauttaa nullptr:n jos annettu rivi-indeksi ei osu mihink‰‰n.
SmartToolMacroData* SmartToolMacroData::FindMacro(int theGridControlRowIndex, int theFixedRowCount)
{
    if(theGridControlRowIndex >= theFixedRowCount)
    {
        return ::FindMacro(itsDirectoryContent, theGridControlRowIndex);
    }

    return nullptr; // Ei lˆytynyt makroa
}
