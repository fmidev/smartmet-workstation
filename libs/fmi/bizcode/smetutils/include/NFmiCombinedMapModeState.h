#pragma once

#include <functional>

// T‰m‰ luokka tiet‰‰ eri moodien (lokaalit ja wms) karttojen lukum‰‰r‰t.
// Voidaan menn‰ seuraavaan tai edelliseen karttaan ottaen huomioon yhteinen karttojen lukum‰‰r‰.
// Voidaan asettaa suoraan haluttu yhteisindeksi.
// Voidaan kysy‰ osoittaako nykyhetken indeksi lokaali vai wms karttaan (isLocalMapInUse).
// Voidaan kysy‰ nykyhetken osoittaman kokonaisindeksin moodikohtainen (lokaali/wms) indeksi (currentMapSectionIndex metodi).
class NFmiCombinedMapModeState
{
    using LocalOnlyMapModeUsedFunction = std::function<bool()>;

    int localMapSize_ = 0;
    int wmsMapSize_ = 0;
    int totalMapSize_ = 0;
    // Nollasta alkava indeksi, joka osoittaa johonkin totalMapSize_ koon rajoittamaan karttaan.
    int combinedModeMapIndex_ = 0;
    // Nollasta alkava indeksi, joka osoittaa johonkin localMapSize_ koon rajoittamaan karttaan.
    // T‰t‰ on tarkoitus p‰ivitt‰‰ (combinedModeMapIndex_:in ohessa) aina kun ollaan vain lokaali moodissa ja kun combinedModeMapIndex_ 
    // on lokaalien indeksien rajoissa. T‰m‰n tarkoitus on olla lyhyt indeksi muisti, kun k‰ytt‰j‰ esim. hyppii local modesta combine moodiin ja takaisin.
    int lastUsedLocalModeMapIndex_ = 0;
    // T‰m‰n avulla kysyt‰‰n ollaanko lokaali vai combined-map moodissa.
    LocalOnlyMapModeUsedFunction localOnlyMapModeUsedFunction_;
public:
    NFmiCombinedMapModeState();
    void initialize(int localMapSize, int wmsMapSize, LocalOnlyMapModeUsedFunction &localOnlyMapModeUsedFunction);

    void next();
    void previous();
    int combinedModeMapIndex() const;
    void combinedModeMapIndex(int index);
    bool isLocalMapCurrentlyInUse() const;
    int currentMapSectionIndex() const;

private:
    void checkIndexUnderFlow();
    void checkIndexOverFlow();
    bool isLocalOnlyMapModeInUse() const;
    void mapIndexIsAboutToChangeChecks();
    bool isMapIndexInLocalSection() const;
    void updateLastUsedLocalModeMapIndex();
};
