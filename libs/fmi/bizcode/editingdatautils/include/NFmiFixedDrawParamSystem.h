#pragma once

#include <vector>
#include <memory>
#include <string>

class NFmiDrawParam;

// Luokka sis‰lt‰‰ hakemiston lapsihakemistoineen ja niiden kaikki NFmiDrawParam -oliot.
class NFmiFixedDrawParamFolder
{
public:
    NFmiFixedDrawParamFolder();
    ~NFmiFixedDrawParamFolder();

    void Initialize(const std::string &theFolderPath, std::string theSubFolderName);
    void AddToFlatList(std::vector<std::shared_ptr<NFmiDrawParam>> &theFlatDrawParamList);
    bool Empty() const;
    const std::vector<NFmiFixedDrawParamFolder>& SubFolders() const { return itsSubFolders; }
    const std::vector<std::shared_ptr<NFmiDrawParam>>& DrawParams() const { return itsDrawParams; }
    const std::string& SubFolderName() const { return itsSubFolderName; }
    const std::string& FolderPath() const { return itsFolderPath; }

private:
    std::string itsFolderPath; // T‰ss‰ on koko polku mukana esim. D:/smartmet/fixeddrawparams/isolines
    std::string itsSubFolderName; // t‰ss‰ on t‰m‰n hakemiston nimi, viimeinen osio itsFolderPath:ist‰ (paitsi jos kyse on rootFolderista)
    std::vector<NFmiFixedDrawParamFolder> itsSubFolders; // T‰ss‰ on kaikki alihakemistot puurakenteena sis‰lt‰en niiden kaikki NFmiDrawParam -oliot.
    std::vector<std::shared_ptr<NFmiDrawParam>> itsDrawParams; // T‰ss‰ on kaikki t‰ss‰ hakemistossa olevat NFmiDrawParam -oliot.
};


// Luokka pit‰‰ kirjaa visualisoinnin piirto-ominaisuuksien (NFmiDrawParam -ilmentymi‰) tehdasasetuksista.
// SmartMetilla on hakemisto, miss‰ on joukko esim‰‰riteltyj‰ piirto-ominaisuuksia, joita k‰ytt‰j‰ voi helposti
// ottaa k‰yttˆˆn eri parametreille.
class NFmiFixedDrawParamSystem
{
public:
    NFmiFixedDrawParamSystem();
    ~NFmiFixedDrawParamSystem();

    void Initialize(const std::string &theRootFolderPath);
    const NFmiFixedDrawParamFolder& RootFolder() const { return itsRootFolder; }
    const std::shared_ptr<NFmiDrawParam>& GetDrawParam(const std::string &theDrawParamInitName) const;
    const std::shared_ptr<NFmiDrawParam>& GetRelativePathDrawParam(const std::string &theRelativeDrawParamPath) const;
    const std::shared_ptr<NFmiDrawParam>& GetCurrentDrawParam() const;
    void Next();
    void Previous();
    const std::vector<std::shared_ptr<NFmiDrawParam>>& FlatDrawParamList() const { return itsFlatDrawParamList; }

private:
    void Clear();
    void MakeFlatList();

    NFmiFixedDrawParamFolder itsRootFolder; // T‰ss‰ on juurihakemisto puurakenteena, joka sis‰lt‰‰ kaikki alihakemisot ja niiden NFmiDrawParam -oliot.
                                            // HUOM! Pit‰‰ olla absoluuttinen polku!
    std::vector<std::shared_ptr<NFmiDrawParam>> itsFlatDrawParamList; // T‰ss‰ on kaikkien hakemistojen sis‰lt‰m‰t NFmiDrawParam -oliot ns. flat-listana.
    mutable int itsCurrentFlatListSelection; // T‰h‰n talletetaan indeksi osoittamaan viimeksi valitun drawParamin paikkaa. Next/Previous -metodit vaikuttavat t‰h‰n. 
                                    // T‰m‰n avulla voidaan tehd‰ hiiren rulla + CTRL + SHIFT toiminto, jolla voidaan vaihtaa valitulle parametrille lennossa seuraavaan/edelliseen FixedDrawParam asetukseen.
                                    // Arvo -1 on alustamaton, jolloin seuraava/edellinen tulee osoittamaan indeksi 0:aan.
};

