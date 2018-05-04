#pragma once

#include <deque>

class NFmiRawData;

class NFmiUndoRedoQData
{
 public:
  NFmiUndoRedoQData(void);
  ~NFmiUndoRedoQData(void);

  bool SnapShotData(const std::string& theAction,
                    const NFmiRawData& theRawData);
  void RearrangeUndoTable(void);
  bool Undo(void);
  bool Redo(void);
  bool UndoData(NFmiRawData& theRawData, std::string &modificationDescription);
  bool RedoData(NFmiRawData& theRawData, std::string &modificationDescription);
  void UndoLevel(long theDepth, const NFmiRawData& theRawData);

 private:
  long* itsMaxUndoLevelPtr;
  long* itsMaxRedoLevelPtr;
  int* itsCurrentUndoLevelPtr;
  int* itsCurrentRedoLevelPtr;

  char** itsUndoTable;
  std::string* itsUndoTextTable;
};
