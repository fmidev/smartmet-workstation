//**********************************************************
// C++ Class Name : NFmiMultiLevelMask
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiMultiLevelMask.cpp
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 2
//  - GD View Type      : Class Diagram
//  - GD View Name      : Markon ehdotus
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Tues - Feb 9, 1999
//
//
//  Description:
//   Luokka joka pitää sisällään usean tason maskeja.
//   Kaikki toiminnat
//   riippuvat aina itsMaskState:n arvosta. Tässä
//   vaiheessa luokka
//   toteutetaan ei dynaamisesti, luomalla käyttöön
//   kolme tasoinen
//   maski, jota tarvitaan MetEditorissa. Myöhemmin
//   luokan voisi tehdä
//   siten, että eri bitmaskit laitetaan listaan.
//
//
//  Change Log:
//
//**********************************************************
#include "NFmiMultiLevelMask.h"
#include "NFmiMetEditorTypes.h"
#include <newbase/NFmiRect.h>
#include <cassert>

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------

NFmiMultiLevelMask::NFmiMultiLevelMask(unsigned long theSize)
    : itsSelectionMask(theSize),
      itsDisplayedMask(theSize),
      itsMaskType(NFmiMetEditorTypes::kFmiNoMask),
      itsMaskSize(theSize)
{
}

NFmiMultiLevelMask::NFmiMultiLevelMask(const NFmiMultiLevelMask& theMask)
    : itsSelectionMask(theMask.itsSelectionMask),
      itsDisplayedMask(theMask.itsDisplayedMask),
      itsMaskType(theMask.itsMaskType),
      itsMaskSize(theMask.itsMaskSize)
{
}

NFmiMultiLevelMask& NFmiMultiLevelMask::operator=(const NFmiMultiLevelMask& theMask)
{
  itsSelectionMask = theMask.itsSelectionMask;
  itsDisplayedMask = theMask.itsDisplayedMask;
  itsMaskType = theMask.itsMaskType;
  itsMaskSize = theMask.itsMaskSize;

  return *this;
}

NFmiMultiLevelMask::~NFmiMultiLevelMask(void)
{
}

//--------------------------------------------------------
// MaskSize
//--------------------------------------------------------
const unsigned long& NFmiMultiLevelMask::MaskSize(void) const
{
  return itsMaskSize;
}

//--------------------------------------------------------
// Mask
//--------------------------------------------------------
//   Asettaa indeksin osoittaman kohdan maskin
//   newState:n mukaiseksi.
//   Se mikä/mitkä maskit asetetaan , riippuu
//   itsMaskType:n arvosta.
void NFmiMultiLevelMask::Mask(unsigned long theIndex, bool theNewState)
{
  if ((itsMaskType & NFmiMetEditorTypes::kFmiNoMask) == NFmiMetEditorTypes::kFmiNoMask)
    return;
  if ((itsMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
      NFmiMetEditorTypes::kFmiSelectionMask)
    itsSelectionMask.Mask(theIndex, theNewState);
  if ((itsMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
      NFmiMetEditorTypes::kFmiDisplayedMask)
    itsDisplayedMask.Mask(theIndex, theNewState);
  return;
}
//--------------------------------------------------------
// IsMasked
//--------------------------------------------------------

//   Palauttaa indeksin osoittaman kohdan, itsMaskType:n
//   osoittamien
//   maskien arvon. Jos kFmiNoMask 'päällä', palauttaa
//   aina true.
//   Tarkista indeksin koko ja jos se ei sovi,
//   palauta false.
//
bool NFmiMultiLevelMask::IsMasked(unsigned long theIndex) const
{
  if ((itsMaskType & NFmiMetEditorTypes::kFmiNoMask) == NFmiMetEditorTypes::kFmiNoMask)
    return true;
  if ((itsMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
      NFmiMetEditorTypes::kFmiSelectionMask)
    if (!itsSelectionMask.IsMasked(theIndex))
      return false;
  if ((itsMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
      NFmiMetEditorTypes::kFmiDisplayedMask)
    if (!itsDisplayedMask.IsMasked(theIndex))
      return false;
  if ((itsMaskType & 0xffffffff) == 0)  // Jos mitään maaskityyppiä ei ole valittu.
    return false;                       // Tämä on käyttäjän aiheuttama virhetila.
  return true;
}
//--------------------------------------------------------
// Mask
//--------------------------------------------------------
//   Asettaa vain theMaskTypen:n osoittamat maskit
//   .
void NFmiMultiLevelMask::Mask(unsigned long theIndex, bool theNewState, unsigned long theMaskType)
{
  if ((theMaskType & NFmiMetEditorTypes::kFmiNoMask) == NFmiMetEditorTypes::kFmiNoMask)
    return;
  if ((theMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
      NFmiMetEditorTypes::kFmiSelectionMask)
    itsSelectionMask.Mask(theIndex, theNewState);
  if ((theMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
      NFmiMetEditorTypes::kFmiDisplayedMask)
    itsDisplayedMask.Mask(theIndex, theNewState);
  return;
}
//--------------------------------------------------------
// IsMasked
//--------------------------------------------------------
bool NFmiMultiLevelMask::IsMasked(unsigned long theIndex, unsigned long theMaskType) const
{
  if ((theMaskType & NFmiMetEditorTypes::kFmiNoMask) == NFmiMetEditorTypes::kFmiNoMask)
    return true;
  if ((theMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
      NFmiMetEditorTypes::kFmiSelectionMask)
    if (!itsSelectionMask.IsMasked(theIndex))
      return false;
  if ((theMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
      NFmiMetEditorTypes::kFmiDisplayedMask)
    if (!itsDisplayedMask.IsMasked(theIndex))
      return false;
  if ((theMaskType & 0xffffffff) == 0)
    return false;  // Tämä on käyttäjän aiheuttama virhetila.
  return true;
}
//--------------------------------------------------------
// MaskAll
//--------------------------------------------------------

//   Maskaa kaikki maskit itsMaskType:n osoittaman
//   arvon mukaisesti
//   eli asettaa kaikkien itsMaskTypen osoittamien
//   NFmiBitMask:ien
//   kaikki arvot newStaten mukaisiksi.
void NFmiMultiLevelMask::MaskAll(bool theNewState)
{
  if ((itsMaskType & NFmiMetEditorTypes::kFmiNoMask) == NFmiMetEditorTypes::kFmiNoMask)
    return;
  if ((itsMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
      NFmiMetEditorTypes::kFmiSelectionMask)
    itsSelectionMask.Mask(theNewState);
  if ((itsMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
      NFmiMetEditorTypes::kFmiDisplayedMask)
    itsDisplayedMask.Mask(theNewState);
  return;
}
//--------------------------------------------------------
// MaskAll
//--------------------------------------------------------
void NFmiMultiLevelMask::MaskAll(bool theNewState, unsigned long theMaskType)
{
  if ((theMaskType & NFmiMetEditorTypes::kFmiNoMask) == NFmiMetEditorTypes::kFmiNoMask)
    return;
  if ((theMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
      NFmiMetEditorTypes::kFmiSelectionMask)
    itsSelectionMask.Mask(theNewState);
  if ((theMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
      NFmiMetEditorTypes::kFmiDisplayedMask)
    itsDisplayedMask.Mask(theNewState);
  return;
}

void NFmiMultiLevelMask::InverseMask(unsigned long theMaskType)
{
  if ((theMaskType & NFmiMetEditorTypes::kFmiNoMask) == NFmiMetEditorTypes::kFmiNoMask)
    return;
  if ((theMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
      NFmiMetEditorTypes::kFmiSelectionMask)
    itsSelectionMask.InverseMask();
  if ((theMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
      NFmiMetEditorTypes::kFmiDisplayedMask)
    itsDisplayedMask.InverseMask();
}

//--------------------------------------------------------
// Mask
//--------------------------------------------------------

//   Kopioi annetun maskin suoraan itsMaskTypen
//   osoittamiin maskeihin.
//   Jos aneetun maskin ja itsMaskSize:n koot
//   eivät täsmää, epäonnistuu
//   koko operaatio.
bool NFmiMultiLevelMask::Mask(const NFmiBitMask& theMask)
{
  if ((itsMaskType & NFmiMetEditorTypes::kFmiNoMask) == NFmiMetEditorTypes::kFmiNoMask)
    return true;
  if (itsMaskSize == static_cast<unsigned long>(theMask.Size()))
  {
    if ((itsMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
        NFmiMetEditorTypes::kFmiSelectionMask)
      itsSelectionMask = theMask;
    if ((itsMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
        NFmiMetEditorTypes::kFmiDisplayedMask)
      itsDisplayedMask = theMask;
    return true;
  }
  else
    return false;
}
//--------------------------------------------------------
// Mask
//--------------------------------------------------------

//   Kopioi maskin vain theMaskType:n osoittamiin
//   maskeihin.
bool NFmiMultiLevelMask::Mask(const NFmiBitMask& theMask, unsigned long theMaskType)
{
  if ((theMaskType & NFmiMetEditorTypes::kFmiNoMask) == NFmiMetEditorTypes::kFmiNoMask)
    return true;
  if (itsMaskSize == static_cast<unsigned long>(theMask.Size()))
  {
    if ((theMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
        NFmiMetEditorTypes::kFmiSelectionMask)
      itsSelectionMask = theMask;
    if ((theMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
        NFmiMetEditorTypes::kFmiDisplayedMask)
      itsDisplayedMask = theMask;
    return true;
  }
  else
    return false;
}
//--------------------------------------------------------
// Mask
//--------------------------------------------------------
const NFmiBitMask& NFmiMultiLevelMask::Mask(unsigned long theMaskType) const
{
  if ((theMaskType & NFmiMetEditorTypes::kFmiSelectionMask) ==
      NFmiMetEditorTypes::kFmiSelectionMask)
    return itsSelectionMask;
  if ((theMaskType & NFmiMetEditorTypes::kFmiDisplayedMask) ==
      NFmiMetEditorTypes::kFmiDisplayedMask)
    return itsDisplayedMask;
  return itsSelectionMask;  // ???????
}

unsigned long NFmiMultiLevelMask::MaskedCount(unsigned long theMaskType)
{
  if (theMaskType == NFmiMetEditorTypes::kFmiNoMask)
    return itsMaskSize;
  if (theMaskType == NFmiMetEditorTypes::kFmiSelectionMask)
    return itsSelectionMask.MaskedCount();
  if (theMaskType == NFmiMetEditorTypes::kFmiDisplayedMask)
    return itsDisplayedMask.MaskedCount();

  return 0;
}

// Laskee kuinka monta pistettä on maskattu annetun laatikon alueella, jonka vasen alakulma
// annetun indeksin kohdalla.
int NFmiMultiLevelMask::MaskedCount(unsigned long theMaskType,
                                    unsigned long theIndex,
                                    const NFmiRect& theSearchArea,
                                    unsigned int theXGridSize,
                                    unsigned int theYGridSize)
{
  assert(theXGridSize);
  unsigned long indexYPos = theIndex / theXGridSize;
  unsigned long indexXPos = theIndex % theXGridSize;
  unsigned long count = 0;
  unsigned long endRow = static_cast<unsigned long>(indexYPos + theSearchArea.Bottom());
  unsigned long startRow = static_cast<unsigned long>(indexYPos + theSearchArea.Top());
  unsigned long startColumn = static_cast<unsigned long>(indexXPos + theSearchArea.Left());
  unsigned long endColumn = static_cast<unsigned long>(indexXPos + theSearchArea.Right());

  for (unsigned long i = startRow; i <= endRow; i++)
  {
    for (unsigned long j = startColumn; j <= endColumn; j++)
    {
      if (i >= theYGridSize)
        count++;
      else if (j >= theXGridSize)
        count++;
      else
      {
        unsigned long index = i * theXGridSize + j;
        if (IsMasked(index, theMaskType))
          count++;
      }
    }
  }

  return count;
}
