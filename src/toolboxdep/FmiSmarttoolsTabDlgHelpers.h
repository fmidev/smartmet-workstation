#pragma once

#include "NFmiMetTime.h"

#include <functional>

class NFmiSmartToolInfo;

// T‰m‰n avulla rakennetaan puurakenne smarttool- ja macroParam -dialogien grid-kontrolliin.
// T‰m‰ rakennetaan speed-search:i‰ varten rakennetusta listasta, jossa on kaikki juuresta alkaen olevat
// macrot ja hakemistot ja niiden alihakemistoissa olevat jutut.
struct SmartToolMacroData
{
    typedef std::vector<SmartToolMacroData> ContainerType;
    typedef std::vector<unsigned char> TreePatternType;
    typedef std::unique_ptr<TreePatternType> TreePatternTypePtr; // En ehk‰ tarvitsisi t‰t‰ unique_ptr -systeemi‰, mutta tein sen, jotta saisin kokemusta siit‰ ett‰ miten luokka tehd‰‰n, jos sill‰ t‰ll‰inen move-only rakenne mukana.

    SmartToolMacroData();
    SmartToolMacroData(SmartToolMacroData &&other); // Tarvitaan std::unique_ptr:in takia (Huom! Move constructorissa other -parametri ei saa olla const!)
    SmartToolMacroData& operator=(SmartToolMacroData &&other); // Tarvitaan std::unique_ptr:in takia (Huom! Move sijoitus operaattorissa other -parametri ei saa olla const!)
    bool operator<(const SmartToolMacroData &other) const;
    void InsertMacro(const std::string &theMacroPath, const std::string &theOriginalMacroPath, const std::string &theAbsoluteRootPath);
    SmartToolMacroData& FindOrCreateDirectory(const std::string &theDirectoryName, const std::string &theOriginalMacroPath, const std::string &theAbsoluteRootPath);
    SmartToolMacroData& AddNewDirectory(const std::string &theDirectoryName, const std::string &theOriginalMacroPath, const std::string &theAbsoluteRootPath);
    void SortMacros();
    bool IsRoot() const;
    void MakeTreePatternArray();
    void Reset();
    void CalcTotalTreeSize();
    SmartToolMacroData* FindMacro(int theGridControlRowIndex, int theFixedRowCount);

    SmartToolMacroData(const SmartToolMacroData&) = delete; // Tarvitaan std::unique_ptr:in takia
    SmartToolMacroData& operator=(const SmartToolMacroData&) = delete; // Tarvitaan std::unique_ptr:in takia


    // 1. Jos makrosta kyse, on t‰ss‰ macro polkuineen juuresta asti esim. "macro1.st" (juuressa) tai "marko\\apuhakis\\macro2.st" (marko\apuhakis -kansiossa). 
    // 2. Jos kyse normaalista hakemistosta, on t‰ss‰ normaalin hakemiston nimi esim. "kansio1" (itsDisplaydedName:ssa on "<kansio1>")
    // 3. Jos t‰m‰ on tyhj‰, ollaan juurikansiossa.
    std::string itsOriginalName;
    NFmiTime itsModifiedTime; // Milloin makroa tai hakista on muutettu
    bool fIsDirectory; // Onko kyseess‰ hakemisto vai makro
    std::string itsDisplaydedName; // Mik‰ nimi n‰ytet‰‰n grid-kontrollissa Name -sarakkeessa. Jos on seuraava makro "marko\\apuhakis\\macro2.st", sen displayed name on "macro2". Jos kyseess‰ kansio "marko\\apuhakis", siihen tulee "<apuhakis>"
    int itsGridControlRowNumber; // Laitetaan talteen, mille grid-controllin riville kukin macro on laitettu
    ContainerType itsDirectoryContent; // Jos kyse oli hakemistossa, t‰nne on laitettu kaikki hakemistossa olevat macrot ja niiden hakemistot. Esim. juurihakemisto sis‰lt‰‰ kaiken alihakemistojhen sis‰llˆn t‰‰ll‰.
    int itsTotalSize; // Kaikkien hakemistojen ja niiss‰ olevien makrojen yhteism‰‰r‰ koko puurakenteessa. T‰m‰ lasketaan kun tehd‰‰n rootin MakeTreePatternArray -kutsu (oletetaan ett‰ silloin rakenne on valmis).

    // Huom! itsTreePatternArray lasketaan vain juureen!
    // T‰t‰ on grid-controlliin laitettavan puurakenteen syvyys rakenne eli juuressa olevat jutut ovat tasolla 1, juuressa olevien alihakemistojen jutut on tasolla 2 jne.
    TreePatternTypePtr itsTreePatternArray;
};
