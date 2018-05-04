//**********************************************************
// C++ Class Name : NFmiMultiLevelMask
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiMultiLevelMask.h
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
#pragma once

#include <newbase/NFmiBitMask.h>

class NFmiRect;

class NFmiMultiLevelMask
{
 public:
  NFmiMultiLevelMask(unsigned long theSize = 32);
  NFmiMultiLevelMask(const NFmiMultiLevelMask& theMask);
  NFmiMultiLevelMask& operator=(const NFmiMultiLevelMask& theMask);
  ~NFmiMultiLevelMask(void);
  unsigned long MaskType(void) const { return itsMaskType; }
  void MaskType(unsigned long theMaskType) { itsMaskType = theMaskType; }
  const unsigned long& MaskSize(void) const;
  void Mask(unsigned long theIndex, bool theNewState);
  bool IsMasked(unsigned long theIndex) const;
  void Mask(unsigned long theIndex, bool theNewState, unsigned long theMaskType);
  bool IsMasked(unsigned long theIndex, unsigned long theMaskType) const;
  void MaskAll(bool theNewState);
  void MaskAll(bool theNewState, unsigned long theMaskType);
  void InverseMask(unsigned long theMaskType);
  bool Mask(const NFmiBitMask& theMask);
  bool Mask(const NFmiBitMask& theMask, unsigned long theMaskType);
  const NFmiBitMask& Mask(unsigned long theMaskType) const;
  void MaskSize(unsigned long theMaskSize){};
  unsigned long MaskedCount(unsigned long theMaskType);
  int MaskedCount(unsigned long theMaskType,
                  unsigned long theIndex,
                  const NFmiRect& theSearchArea,
                  unsigned int theXGridSize,
                  unsigned int theYGridSize);

 private:
  NFmiBitMask itsSelectionMask;
  NFmiBitMask itsDisplayedMask;  // Koskee vain aikasarjaikkunaa
  unsigned long itsMaskType;
  unsigned long itsMaskSize;  //    NFmiBitmask:ien koko.
};

