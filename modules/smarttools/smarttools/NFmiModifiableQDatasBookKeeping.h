#pragma once
// NFmiModifiableQDatasBookKeeping

#include <string>

class NFmiRect;
class NFmiBitMask;
class NFmiRawData;
class NFmiUndoableMultiLevelMask;
class NFmiUndoRedoQData;

class NFmiModifiableQDatasBookKeeping
{
 public:
  NFmiModifiableQDatasBookKeeping(unsigned long theMaskSize = 32);
  ~NFmiModifiableQDatasBookKeeping();
  void CopyClonedDatas(const NFmiModifiableQDatasBookKeeping &theOther);

  bool IsDirty() const { return *fDirty; };
  void Dirty(bool newState) { *fDirty = newState; };
  bool LoadedFromFile() { return *fLoadedFromFile; }
  void LoadedFromFile(bool loadedFromFile) { *fLoadedFromFile = loadedFromFile; }
  int MaskedCount(unsigned long theMaskType,
                  unsigned long theIndex,
                  const NFmiRect &theSearchArea,
                  unsigned long theGridXNumber,
                  unsigned long theGridYNumber);
  void InverseMask(unsigned long theMaskType);
  void MaskAllLocations(const bool &newState, unsigned long theMaskType);
  unsigned long LocationMaskedCount(unsigned long theMaskType);
  bool Mask(const NFmiBitMask &theMask, unsigned long theMaskType);
  const NFmiBitMask &Mask(unsigned long theMaskType) const;
  void MaskLocation(const bool &newState,
                    unsigned long theMaskType,
                    unsigned long theLocationIndex);
  void MaskType(unsigned long theMaskType);
  unsigned long MaskType();
  bool IsMasked(unsigned long theIndex) const;

  bool SnapShotData(const std::string &theAction, const NFmiRawData &theRawData);
  void RearrangeUndoTable();
  bool Undo();
  bool Redo();
  bool UndoData(NFmiRawData &theRawData, std::string &modificationDescription);
  bool RedoData(NFmiRawData &theRawData, std::string &modificationDescription);
  void UndoLevel(long theDepth, const NFmiRawData &theRawData);

  bool LocationSelectionSnapShot();                      // ota maskit talteen
  bool LocationSelectionUndo();                          // kysyy onko undo mahdollinen
  bool LocationSelectionRedo();                          // kysyy onko redo mahdollinen
  bool LocationSelectionUndoData();                      // suorittaa todellisen undon
  bool LocationSelectionRedoData();                      // suorittaa todellisen redon
  void LocationSelectionUndoLevel(int theNewUndoLevel);  // undolevel asetetaan tällä
 private:
  NFmiModifiableQDatasBookKeeping &operator=(
      const NFmiModifiableQDatasBookKeeping &theOther);  // ei toteuteta sijoitus operaatiota!!

  NFmiUndoableMultiLevelMask *itsAreaMask;
  NFmiUndoRedoQData *itsUndoRedoQData;
  bool *fLoadedFromFile;  // kertoo onko data ladattu tiedostosta vai työlistan mukaisesti
  bool *fDirty;           // onko dataa editoitu
};
