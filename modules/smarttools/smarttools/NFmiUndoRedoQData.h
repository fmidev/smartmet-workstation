#pragma once

#include <deque>
#include <string>

class NFmiRawData;

class NFmiUndoRedoQData
{
 public:
  NFmiUndoRedoQData();
  ~NFmiUndoRedoQData();

  bool SnapShotData(const std::string& theAction, const NFmiRawData& theRawData);
  void RearrangeUndoTable();
  bool Undo();
  bool Redo();
  bool UndoData(NFmiRawData& theRawData, std::string& modificationDescription);
  bool RedoData(NFmiRawData& theRawData, std::string& modificationDescription);
  void UndoLevel(long theDepth, const NFmiRawData& theRawData);

 private:
  long* itsMaxUndoLevelPtr;
  long* itsMaxRedoLevelPtr;
  int* itsCurrentUndoLevelPtr;
  int* itsCurrentRedoLevelPtr;

  char** itsUndoTable;
  std::string* itsUndoTextTable;
};
