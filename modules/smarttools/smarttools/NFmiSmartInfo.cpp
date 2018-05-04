
#include "NFmiSmartInfo.h"
#include "NFmiModifiableQDatasBookKeeping.h"
#include <newbase/NFmiQueryData.h>

NFmiSmartInfo::NFmiSmartInfo(void) : NFmiOwnerInfo(), itsQDataBookKeepingPtr()
{
}

NFmiSmartInfo::NFmiSmartInfo(NFmiQueryData *theOwnedData,
                             NFmiInfoData::Type theDataType,
                             const std::string &theDataFileName,
                             const std::string &theDataFilePattern)
    : NFmiOwnerInfo(theOwnedData, theDataType, theDataFileName, theDataFilePattern),
      itsQDataBookKeepingPtr()
{
  itsQDataBookKeepingPtr = boost::shared_ptr<NFmiModifiableQDatasBookKeeping>(
      new NFmiModifiableQDatasBookKeeping(SizeLocations()));
}

NFmiSmartInfo::NFmiSmartInfo(const NFmiOwnerInfo &theInfo)
    : NFmiOwnerInfo(theInfo), itsQDataBookKeepingPtr()
{
}

NFmiSmartInfo::NFmiSmartInfo(const NFmiSmartInfo &theInfo)
    : NFmiOwnerInfo(theInfo), itsQDataBookKeepingPtr(theInfo.itsQDataBookKeepingPtr)
{
}

NFmiSmartInfo::~NFmiSmartInfo(void)
{
}

NFmiSmartInfo &NFmiSmartInfo::operator=(const NFmiSmartInfo &theInfo)
{
  if (this != &theInfo)
  {
    NFmiOwnerInfo::operator=(theInfo);
    itsQDataBookKeepingPtr = theInfo.itsQDataBookKeepingPtr;
  }
  return *this;
}

NFmiSmartInfo *NFmiSmartInfo::Clone(void) const
{
  NFmiQueryData *cloneData = itsDataPtr.get()->Clone();  // datasta tehtävä tässä kopio!
  NFmiSmartInfo *copy = new NFmiSmartInfo(
      cloneData,
      itsDataType,
      itsDataFileName,
      itsDataFilePattern);       // tämä ei osaa tehdä kaikesta tarvittavasta datasta kopiota
  copy->CopyClonedDatas(*this);  // tässä laitetaan kaikki loput tarvittavat NFmiSmartInfo-data osat
                                 // kopioitavaksi clooniin.
  // Pitää asettaa iteraattorit myös kohdalleen
  copy->Param(Param());  // Tämä pitää asettaa parametrilla, pelkkä indeksin asetus ei riitä
                         // (aliparametri juttu)!!!
  copy->TimeIndex(TimeIndex());
  copy->LevelIndex(LevelIndex());
  copy->LocationIndex(LocationIndex());
  return copy;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiSmartInfo::CreateShallowCopyOfHighestInfo(
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  if (theInfo)
  {
    NFmiSmartInfo *smartInfo = dynamic_cast<NFmiSmartInfo *>(theInfo.get());
    if (smartInfo)
      return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiSmartInfo(*smartInfo));

    NFmiOwnerInfo *ownerInfo = dynamic_cast<NFmiOwnerInfo *>(theInfo.get());
    if (ownerInfo)
      return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiOwnerInfo(*ownerInfo));

    NFmiFastQueryInfo *fastInfo = dynamic_cast<NFmiFastQueryInfo *>(theInfo.get());
    if (fastInfo)
      return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*fastInfo));
  }

  return boost::shared_ptr<NFmiFastQueryInfo>();
}

void NFmiSmartInfo::CopyClonedDatas(const NFmiSmartInfo &theOther)
{
  if (this != &theOther)
  {
    itsQDataBookKeepingPtr->CopyClonedDatas(*(theOther.itsQDataBookKeepingPtr.get()));
  }
}

void NFmiSmartInfo::UndoLevel(long theDepth)  // theDepth kuvaa kuinka monta Undota voidaan tehdä.
{
  itsQDataBookKeepingPtr->UndoLevel(theDepth, *itsRefRawData);
}

bool NFmiSmartInfo::LocationSelectionSnapShot(void)
{
  return itsQDataBookKeepingPtr->LocationSelectionSnapShot();
}

bool NFmiSmartInfo::LocationSelectionUndo(void)
{
  return itsQDataBookKeepingPtr->LocationSelectionUndo();
}

bool NFmiSmartInfo::LocationSelectionRedo(void)
{
  return itsQDataBookKeepingPtr->LocationSelectionRedo();
}

bool NFmiSmartInfo::LocationSelectionUndoData(void)
{
  return itsQDataBookKeepingPtr->LocationSelectionUndoData();
}

bool NFmiSmartInfo::LocationSelectionRedoData(void)
{
  return itsQDataBookKeepingPtr->LocationSelectionRedoData();
}

void NFmiSmartInfo::LocationSelectionUndoLevel(int theNewUndoLevel)
{
  itsQDataBookKeepingPtr->LocationSelectionUndoLevel(theNewUndoLevel);
}

bool NFmiSmartInfo::LoadedFromFile(void)
{
  return itsQDataBookKeepingPtr->LoadedFromFile();
}

void NFmiSmartInfo::LoadedFromFile(bool loadedFromFile)
{
  itsQDataBookKeepingPtr->LoadedFromFile(loadedFromFile);
}

//   Siirtää 'iteraattorin' osoittamaan seuraavaa
//   maskattua paikkaa.
//   Kutsutaan emon Next()-metodia ja katsotaan,
//   mikä itsAreaMask-olio
//   palauttaa. Jos Maski on true, palautetaan
//   true, muuten kutsutaan
//   taas emon Next:iä ja jatketaan kunnes emon
//   Next palauttaa falsen.
bool NFmiSmartInfo::NextLocation()
{
  bool returnVal = false;

  while ((returnVal == false) && NFmiFastQueryInfo::NextLocation())
  {
    returnVal = itsQDataBookKeepingPtr->IsMasked(LocationIndex());
  }
  return returnVal;
}

bool NFmiSmartInfo::SnapShotData(const std::string &theAction)
{
  return itsQDataBookKeepingPtr->SnapShotData(
      theAction, *itsRefRawData);
}

bool NFmiSmartInfo::Undo(void)
{
  return itsQDataBookKeepingPtr->Undo();
}

bool NFmiSmartInfo::Redo(void)
{
  return itsQDataBookKeepingPtr->Redo();
}

bool NFmiSmartInfo::UndoData(std::string &modificationDescription)
{
  return itsQDataBookKeepingPtr->UndoData(*itsRefRawData, modificationDescription);
}

bool NFmiSmartInfo::RedoData(std::string &modificationDescription)
{
  return itsQDataBookKeepingPtr->RedoData(*itsRefRawData, modificationDescription);
}

bool NFmiSmartInfo::IsDirty(void) const
{
  return itsQDataBookKeepingPtr->IsDirty();
}

void NFmiSmartInfo::Dirty(bool newState)
{
  itsQDataBookKeepingPtr->Dirty(newState);
}

int NFmiSmartInfo::MaskedCount(unsigned long theMaskType,
                               unsigned long theIndex,
                               const NFmiRect &theSearchArea)
{
  return itsQDataBookKeepingPtr->MaskedCount(
      theMaskType, theIndex, theSearchArea, itsGridXNumber, itsGridYNumber);
}

void NFmiSmartInfo::InverseMask(unsigned long theMaskType)
{
  itsQDataBookKeepingPtr->InverseMask(theMaskType);
}

void NFmiSmartInfo::MaskAllLocations(const bool &newState, unsigned long theMaskType)
{
  itsQDataBookKeepingPtr->MaskAllLocations(newState, theMaskType);
}

unsigned long NFmiSmartInfo::LocationMaskedCount(unsigned long theMaskType)
{
  return itsQDataBookKeepingPtr->LocationMaskedCount(theMaskType);
}

bool NFmiSmartInfo::Mask(const NFmiBitMask &theMask, unsigned long theMaskType)
{
  return itsQDataBookKeepingPtr->Mask(theMask, theMaskType);
}

const NFmiBitMask &NFmiSmartInfo::Mask(unsigned long theMaskType) const
{
  return itsQDataBookKeepingPtr->Mask(theMaskType);
}

void NFmiSmartInfo::MaskLocation(const bool &newState, unsigned long theMaskType)
{
  itsQDataBookKeepingPtr->MaskLocation(newState, theMaskType, LocationIndex());
}

void NFmiSmartInfo::MaskType(unsigned long theMaskType)
{
  itsQDataBookKeepingPtr->MaskType(theMaskType);
}

unsigned long NFmiSmartInfo::MaskType(void)
{
  return itsQDataBookKeepingPtr->MaskType();
}
