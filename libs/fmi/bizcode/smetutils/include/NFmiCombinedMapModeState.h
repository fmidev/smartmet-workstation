#pragma once

// T‰m‰ luokka tiet‰‰ eri moodien (lokaalit ja wms) karttojen lukum‰‰r‰t.
// Voidaan menn‰ seuraavaan tai edelliseen karttaan ottaen huomioon yhteinen karttojen lukum‰‰r‰.
// Voidaan asettaa suoraan haluttu yhteisindeksi.
// Voidaan kysy‰ osoittaako nykyhetken indeksi lokaali vai wms karttaan (isLocalMapInUse).
// Voidaan kysy‰ nykyhetken osoittaman kokonaisindeksin moodikohtainen (lokaali/wms) indeksi (currentMapSectionIndex metodi).
class NFmiCombinedMapModeState
{
    int localMapSize_ = 0;
    int wmsMapSize_ = 0;
    int totalMapSize_ = 0;
    // Nollasta alkava indeksi, joka osoittaa johonkin totalMapSize_ koon rajoittamaan karttaan.
    int combinedModeMapIndex_ = 0;
public:
    NFmiCombinedMapModeState();
    void initialize(int localMapSize, int wmsMapSize);

    void next();
    void previous();
    int combinedModeMapIndex() const;
    void combinedModeMapIndex(int index);
    bool isLocalMapInUse() const;
    int currentMapSectionIndex() const;

private:
    void checkIndexUnderFlow();
    void checkIndexOverFlow();
};
