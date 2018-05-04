// NFmiModifiableQDatasBookKeeping

#include "NFmiModifiableQDatasBookKeeping.h"
#include "NFmiUndoableMultiLevelMask.h"
#include "NFmiMultiLevelMask.h"
#include "NFmiUndoRedoQData.h"

NFmiModifiableQDatasBookKeeping::NFmiModifiableQDatasBookKeeping(unsigned long theMaskSize)
    : itsAreaMask(theMaskSize ? new NFmiUndoableMultiLevelMask(theMaskSize) : 0),
      itsUndoRedoQData(0),
      fLoadedFromFile(new bool(false)),
      fDirty(new bool(false))
{
}

NFmiModifiableQDatasBookKeeping::~NFmiModifiableQDatasBookKeeping(void)
{
  delete itsAreaMask;
  delete itsUndoRedoQData;
  delete fLoadedFromFile;
  delete fDirty;
}

void NFmiModifiableQDatasBookKeeping::CopyClonedDatas(
    const NFmiModifiableQDatasBookKeeping& theOther)
{
  if (this != &theOther)
  {
    // HUOM! Tässä kopioidaan dataa vain this:issa oleviin ei 0-pointtereihin.
    // Eli ei luoda uusia olioita tässä, kuten ehkä normaalisti sijoitus operaatiossa tehtäisiin.
    if (itsUndoRedoQData)
      *itsUndoRedoQData = *(theOther.itsUndoRedoQData);
    if (itsAreaMask)
      *itsAreaMask = *(theOther.itsAreaMask);
    if (fLoadedFromFile)
      *fLoadedFromFile = *(theOther.fLoadedFromFile);
    if (fDirty)
      *fDirty = *(theOther.fDirty);
  }
}

int NFmiModifiableQDatasBookKeeping::MaskedCount(unsigned long theMaskType,
                                                 unsigned long theIndex,
                                                 const NFmiRect& theSearchArea,
                                                 unsigned long theGridXNumber,
                                                 unsigned long theGridYNumber)
{
  return (*itsAreaMask)
      ->MaskedCount(theMaskType, theIndex, theSearchArea, theGridXNumber, theGridYNumber);
}

void NFmiModifiableQDatasBookKeeping::InverseMask(unsigned long theMaskType)
{
  (*itsAreaMask)->InverseMask(theMaskType);
}

void NFmiModifiableQDatasBookKeeping::MaskAllLocations(const bool& newState,
                                                       unsigned long theMaskType)
{
  (*itsAreaMask)->MaskAll(newState, theMaskType);
}

unsigned long NFmiModifiableQDatasBookKeeping::LocationMaskedCount(unsigned long theMaskType)
{
  return (*itsAreaMask)->MaskedCount(theMaskType);
}

bool NFmiModifiableQDatasBookKeeping::Mask(const NFmiBitMask& theMask, unsigned long theMaskType)
{
  if (itsAreaMask)
    return itsAreaMask->Mask(theMask, theMaskType);

  return false;
}

const NFmiBitMask& NFmiModifiableQDatasBookKeeping::Mask(unsigned long theMaskType) const
{
  if (itsAreaMask)
    return itsAreaMask->Mask(theMaskType);

  throw std::runtime_error("Error in application - NFmiSmartInfo::Mask has no mask.");
}

void NFmiModifiableQDatasBookKeeping::MaskLocation(const bool& newState,
                                                   unsigned long theMaskType,
                                                   unsigned long theLocationIndex)
{
  (*itsAreaMask)->Mask(theLocationIndex, newState, theMaskType);
}

void NFmiModifiableQDatasBookKeeping::MaskType(unsigned long theMaskType)
{
  (*itsAreaMask)->MaskType(theMaskType);
}

unsigned long NFmiModifiableQDatasBookKeeping::MaskType(void)
{
  return (*itsAreaMask)->MaskType();
}

bool NFmiModifiableQDatasBookKeeping::IsMasked(unsigned long theIndex) const
{
  return (*itsAreaMask)->IsMasked(theIndex);
}

bool NFmiModifiableQDatasBookKeeping::SnapShotData(
    const std::string& theAction,
    const NFmiRawData& theRawData)
{
  if (itsUndoRedoQData)
    return itsUndoRedoQData->SnapShotData(theAction, theRawData);
  else
    return false;
}

bool NFmiModifiableQDatasBookKeeping::Undo(void)
{
  if (itsUndoRedoQData)
    return itsUndoRedoQData->Undo();
  return false;
}

bool NFmiModifiableQDatasBookKeeping::Redo(void)
{
  if (itsUndoRedoQData)
    return itsUndoRedoQData->Redo();
  return false;
}

bool NFmiModifiableQDatasBookKeeping::UndoData(NFmiRawData& theRawData, std::string &modificationDescription)
{
  if (itsUndoRedoQData)
  {
    if (itsUndoRedoQData->UndoData(theRawData, modificationDescription))
    {
      *fDirty = true;  // 18.1.2002/Marko lisäsin datan likauksen.
      return true;
    }
  }
  return false;
}

bool NFmiModifiableQDatasBookKeeping::RedoData(NFmiRawData& theRawData, std::string &modificationDescription)
{
  if (itsUndoRedoQData)
  {
    if (itsUndoRedoQData->RedoData(theRawData, modificationDescription))
    {
      *fDirty = true;  // 18.1.2002/Marko lisäsin datan likauksen.
      return true;
    }
  }
  return false;
}

void NFmiModifiableQDatasBookKeeping::UndoLevel(long theDepth, const NFmiRawData& theRawData)
{
  if (theDepth == 0)
  {
    if (itsUndoRedoQData != 0)
    {
      delete itsUndoRedoQData;
      itsUndoRedoQData = 0;
    }
  }
  else
  {
    if (itsUndoRedoQData == 0)
      itsUndoRedoQData = new NFmiUndoRedoQData;

    itsUndoRedoQData->UndoLevel(theDepth, theRawData);
  }
}

bool NFmiModifiableQDatasBookKeeping::LocationSelectionSnapShot(void)
{
  return itsAreaMask->SnapShotData();
}

bool NFmiModifiableQDatasBookKeeping::LocationSelectionUndo(void)
{
  return itsAreaMask->Undo();
}

bool NFmiModifiableQDatasBookKeeping::LocationSelectionRedo(void)
{
  return itsAreaMask->Redo();
}

bool NFmiModifiableQDatasBookKeeping::LocationSelectionUndoData(void)
{
  return itsAreaMask->UndoData();
}

bool NFmiModifiableQDatasBookKeeping::LocationSelectionRedoData(void)
{
  return itsAreaMask->RedoData();
}

void NFmiModifiableQDatasBookKeeping::LocationSelectionUndoLevel(int theNewUndoLevel)
{
  itsAreaMask->UndoLevel(theNewUndoLevel);
}
