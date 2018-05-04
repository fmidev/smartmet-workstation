//**********************************************************
// C++ Class Name : NFmiDrawParamList
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiDrawParamList.h
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
//  Creation Date  : Thur - Jan 28, 1999
//
//
//  Description:
//   Voi olla että tässä riittää suoraan uuden
//   template lista luokan ilmentymä.
//
//
//  Change Log:
//
//  22.02.-99 lisättiin bool fDirtyList, joka saa arvon
//  metodeissa Add, Remove ja Clear true. Oletusarvo on false.
// Changed 26.08.1999/Marko		Lisäsin uuden Find()-metodin.
//
//**********************************************************
#pragma once

#include "NFmiSortedPtrList.h"
#include <newbase/NFmiDataMatrix.h>
#include <newbase/NFmiInfoData.h>
#include "NFmiDrawParam.h"
#include <boost/shared_ptr.hpp>
#include <list>

#ifdef _MSC_VER
#pragma warning(disable : 4512)  // poistaa VC++ kääntäjän C4512 "assignment operator could not be
                                 // generated" varoituksen
#endif

class NFmiMetTime;
class NFmiLevel;
class NFmiProducer;
class NFmiLevel;
class NFmiDataIdent;

class NFmiDrawParamList
{
 public:
  typedef boost::shared_ptr<NFmiDrawParam> DataType;
  typedef std::list<DataType> ListType;
  typedef ListType::iterator IterType;

  NFmiDrawParamList(void);
  ~NFmiDrawParamList(void);

  boost::shared_ptr<NFmiDrawParam> Current(void);
  void CopyList(NFmiDrawParamList& theList, bool clearFirst);
  bool Reset(void);
  bool Next(void);
  void Clear(void);
  void Clear(const NFmiProducer& theProducer,
             checkedVector<int>& theParamIdsNotRemoved,
             NFmiLevel* theLevel = 0);
  void Clear(const NFmiProducer& theProducer,
             std::list<std::pair<int, NFmiLevel> >& theParamIdsAndLevelsNotRemoved);
  bool Add(boost::shared_ptr<NFmiDrawParam>& theParam);
  bool Add(boost::shared_ptr<NFmiDrawParam>& theParam, unsigned long theIndex);
  void BorrowParams(NFmiDrawParamList& theList);
  void ClearBorrowedParams(void);
  bool Remove(void);
  bool Index(unsigned long index);
  //	bool Find(NFmiDrawParam* item);

  void HideAllParams(bool newState);
  void DisableEditing(void);
  void DeactivateAll(void);
  bool Find(const NFmiDataIdent& theParam,
            const NFmiLevel* theLevel,
            NFmiInfoData::Type theDataType,
            bool fUseOnlyParamId = false,
            bool fIgnoreLevelInfo = false);

  void Update(void);
  bool IsDirty(void) { return fDirtyList; };
  void Dirty(bool fDirty) { fDirtyList = fDirty; };
  unsigned long NumberOfItems(void) { return static_cast<unsigned long>(itsList.size()); };
  bool RemoveMacroParam(const std::string& theName);
  bool MoveActiveParam(int theMovement);
  bool MoveParam(int theMovedParamIndex, int theMoveToPosition);
  int FindActive(void);
  int FindEdited(void);
  bool HasBorrowedParams(void) const { return fHasBorrowedParams; }
  void HasBorrowedParams(bool newValue) { fHasBorrowedParams = newValue; }
  void ActivateOnlyOne(void);

 private:
  void Swap(int index1, int index2);

  bool fBeforeFirstItem;  // muutin luokan käyttämään std:list:ia ja halusin kuitenkin että luokka
                          // toimii kuten ennen,
  // joten tällä varmistan että Reset() -kutsun jälkeen ollaan tilassa, mista ei vielä saa ulos
  // drawParamia, vaan vasta yhden Next-kutsun jälkeen saa 1. listan olion.
  ListType itsList;
  IterType itsIter;
  //	NFmiSortedPtrList < NFmiDrawParam > itsList;
  //	NFmiPtrList < NFmiDrawParam > :: Iterator itsIter;
  bool fDirtyList;
  bool fHasBorrowedParams;  // onko tällä listalla 'lainattuja' parametreja
};

