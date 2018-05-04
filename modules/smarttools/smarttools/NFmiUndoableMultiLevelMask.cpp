//**********************************************************
// C++ Class Name : NFmiUndoableMultiLevelMask
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiUndoableMultiLevelMask.cpp
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 3
//  - GD View Type      : Class Diagram
//  - GD View Name      : undoable multilevelmask
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Tues - Jun 22, 1999
//
//
//  Description:
//   Halusin tehdä NFmiMultiLevelMask:iin undo/redo
//   mahdollisuuden. Tein NFmiMultiLevelMask-
//   smartpointerin, jolla on multilevelmask ja
//   lisäksi undo/redo-lista, jossa on multilevelmask-pointereita.
//
//
//  Change Log:
//
//**********************************************************
#include "NFmiUndoableMultiLevelMask.h"
#include "NFmiMultiLevelMask.h"
#include <stdexcept>

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiUndoableMultiLevelMask::NFmiUndoableMultiLevelMask(unsigned long theSize)
    : itsMultiLevelMask(0),
      itsUndoList(),
      itsMaxUndoLevel(0),
      itsMaxRedoLevel(0),
      itsCurrentUndoLevel(0),
      itsCurrentRedoLevel(0)
{
  itsMultiLevelMask = new NFmiMultiLevelMask(theSize);
}
NFmiUndoableMultiLevelMask::NFmiUndoableMultiLevelMask(const NFmiUndoableMultiLevelMask& theMask)
    : itsMultiLevelMask(
          theMask.itsMultiLevelMask ? new NFmiMultiLevelMask(*theMask.itsMultiLevelMask) : 0),
      itsUndoList()  // undolistaa ei kopioida!!!!!
      ,
      itsMaxUndoLevel(0)  // eikä mitään undo/redo juttuihin liittyvää!!!!!
      ,
      itsMaxRedoLevel(0),
      itsCurrentUndoLevel(0),
      itsCurrentRedoLevel(0)
{
}
NFmiUndoableMultiLevelMask& NFmiUndoableMultiLevelMask::operator=(
    const NFmiUndoableMultiLevelMask& theMask)
{
  delete itsMultiLevelMask;
  itsMultiLevelMask = 0;
  itsMultiLevelMask =
      theMask.itsMultiLevelMask ? new NFmiMultiLevelMask(*theMask.itsMultiLevelMask) : 0;
  //	itsUndoList // undolistaa ei kopioida!!!!!
  itsMaxUndoLevel = 0;  // eikä mitään undo/redo juttuihin liittyvää!!!!!
  itsMaxRedoLevel = 0;
  itsCurrentUndoLevel = 0;
  itsCurrentRedoLevel = 0;

  return *this;
}
NFmiUndoableMultiLevelMask::~NFmiUndoableMultiLevelMask(void)
{
  delete itsMultiLevelMask;
  itsUndoList.Clear(true);
}
//--------------------------------------------------------
// SnapShotData
//--------------------------------------------------------
bool NFmiUndoableMultiLevelMask::SnapShotData(void)
{
  if (!itsMultiLevelMask || itsMaxUndoLevel <= 0)
    return false;

  if (itsCurrentUndoLevel == itsMaxUndoLevel - 1)
    RearrangeUndoTable();

  itsCurrentUndoLevel++;
  NFmiPtrList<NFmiMultiLevelMask>::Iterator it = itsUndoList.Index(itsCurrentUndoLevel);
  if (it.CurrentPtr() == 0)
    return false;  // ei pitäisi mennä tähän, exceptionin paikka!
  it.Current() = *itsMultiLevelMask;
  itsCurrentRedoLevel = itsCurrentUndoLevel;
  itsMaxRedoLevel = itsCurrentRedoLevel;

  return true;
}
//--------------------------------------------------------
// RearrangeUndoTable
//--------------------------------------------------------

//   Kun Snapshot:illa mennään listan yli, järjestetään
//   lista niin, että siirretään listan ensimmäinen
//   alkio viimeiseksi ja asetetaan listan osoittimille
//   uudet arvot. Eli jos undo-level olisi 5 ja
//   tehdään kuudes peräkkäinen Snapshot, pitää
//   uudelle snapshotille tehdä tilaa listan alusta.
//
void NFmiUndoableMultiLevelMask::RearrangeUndoTable(void)
{
  NFmiPtrList<NFmiMultiLevelMask>::Iterator it = itsUndoList.Index(1);  // 1 = 1. paikka listassa
  if (it.CurrentPtr() == 0)
    return;  // ei pitäisi mennä tähän, exceptionin paikka!

  NFmiMultiLevelMask* tempMask = it.CurrentPtr();
  it.Remove(false);  // false = ei tuhoa dataa
  itsUndoList.AddEnd(tempMask);

  itsCurrentUndoLevel--;
  itsCurrentRedoLevel--;
}
//--------------------------------------------------------
// Undo
//--------------------------------------------------------
bool NFmiUndoableMultiLevelMask::Undo(void)
{
  if (itsCurrentUndoLevel < 1)
    return false;
  else
    return true;
}
//--------------------------------------------------------
// Redo
//--------------------------------------------------------
bool NFmiUndoableMultiLevelMask::Redo(void)
{
  if (itsCurrentRedoLevel == itsCurrentUndoLevel ||
      (itsCurrentRedoLevel == itsCurrentUndoLevel + 1))
    return false;
  else
    return true;
}

//--------------------------------------------------------
// UndoData
//--------------------------------------------------------
bool NFmiUndoableMultiLevelMask::UndoData(void)
{
  if (!itsMultiLevelMask || itsCurrentUndoLevel < 1)
    return false;
  if (itsCurrentUndoLevel == itsCurrentRedoLevel)
  {
    SnapShotData();         // "Ottaa kuvan" undo-toimintoa edeltäneestä tilanteesta,
    itsCurrentUndoLevel--;  // jos siihen halutaankin myöhemmin palata redo:lla.
  }

  NFmiPtrList<NFmiMultiLevelMask>::Iterator it = itsUndoList.Index(itsCurrentUndoLevel);
  if (it.CurrentPtr() == 0)
    return false;  // ei pitäisi mennä tähän, exceptionin paikka!
  *itsMultiLevelMask = *it.CurrentPtr();
  itsCurrentUndoLevel--;
  itsCurrentRedoLevel = itsCurrentUndoLevel + 2;
  return true;
}
//--------------------------------------------------------
// RedoData
//--------------------------------------------------------
bool NFmiUndoableMultiLevelMask::RedoData(void)
{
  if (itsCurrentUndoLevel == itsCurrentRedoLevel ||
      (itsCurrentUndoLevel + 1 == itsCurrentRedoLevel))
    return false;
  else
  {
    NFmiPtrList<NFmiMultiLevelMask>::Iterator it = itsUndoList.Index(itsCurrentRedoLevel);
    if (it.CurrentPtr() == 0)
      return false;  // ei pitäisi mennä tähän, exceptionin paikka!
    *itsMultiLevelMask = *it.CurrentPtr();

    itsCurrentUndoLevel++;
    if (itsCurrentRedoLevel + 1 <= itsMaxRedoLevel)
      itsCurrentRedoLevel++;
  }
  return true;
}
//--------------------------------------------------------
// UndoLevel
//--------------------------------------------------------
void NFmiUndoableMultiLevelMask::UndoLevel(int theNewUndoLevel)
{  // jos undo leveliä vaihdetaan kesken kaiken, tuhotaan vanha undo-tieto

  // undolistaan laitetaan level+1:lle tilaa, koska kun tehdään esim. 5 kertaa muutos,
  // ja sitten tehdään undo, pitää ottaa talteen nykyhetkestä 6.s 'kuva', että voidaan
  // palata takaisin nykyhetkeen (ja silti voidaan tehdä 5 kertaa undo peräkkäin).
  itsMaxUndoLevel = theNewUndoLevel + 1;
  itsMaxRedoLevel = 0;      // ??
  itsCurrentUndoLevel = 0;  // ??
  itsCurrentRedoLevel = 0;  // ??

  itsUndoList.Clear(true);
  for (int i = 0; i < itsMaxUndoLevel; i++)
    itsUndoList.AddEnd(new NFmiMultiLevelMask(itsMultiLevelMask->MaskSize()));
}

// Tämä asettaa halutun maskin, mutta ei tee undo/redo valmisteluja tai muuta.
bool NFmiUndoableMultiLevelMask::Mask(const NFmiBitMask& theMask, unsigned long theMaskType)
{
  if (itsMultiLevelMask)
    return itsMultiLevelMask->Mask(theMask, theMaskType);

  return false;
}

const NFmiBitMask& NFmiUndoableMultiLevelMask::Mask(unsigned long theMaskType) const
{
  if (itsMultiLevelMask)
    return itsMultiLevelMask->Mask(theMaskType);

  throw std::runtime_error("Error in application - NFmiUndoableMultiLevelMask::Mask has no mask.");
}
