// ======================================================================
/*!
 * \file NFmiSmartToolUtil.h
 * \brief Interface of class NFmiSmartToolUtil
 */
// ======================================================================
/*!
 * \class NFmiSmartToolUtil
 *
 * Tämä luokka tekee halutut smarttool-operaatiot halutuille datoille.
 * Toiminta samanlainen kuin NFmiQueryDataUtil-luokalla, mutta muutokset
 * tehdään erilaisilla scripteillä. Muokkaus datoihin tapahtuu luokan static
 * funktioilla.
 */
// ======================================================================
#pragma once

#include <newbase/NFmiDataMatrix.h>
#include <string>

class NFmiQueryData;
class NFmiTimeDescriptor;
class NFmiInfoOrganizer;

class NFmiSmartToolUtil
{
 public:
  static NFmiQueryData *ModifyData(const std::string &theMacroText,
                                   NFmiQueryData *theModifiedData,
                                   bool createDrawParamFileIfNotExist,
                                   bool fMakeStaticIfOneTimeStepData);
  static NFmiQueryData *ModifyData(const std::string &theMacroText,
                                   NFmiQueryData *theModifiedData,
                                   NFmiTimeDescriptor *theTimes,
                                   bool createDrawParamFileIfNotExist,
                                   bool fMakeStaticIfOneTimeStepData);
  static NFmiQueryData *ModifyData(const std::string &theMacroText,
                                   NFmiQueryData *theModifiedData,
                                   const checkedVector<std::string> *theHelperDataFileNames,
                                   bool createDrawParamFileIfNotExist,
                                   bool goThroughLevels,
                                   bool fMakeStaticIfOneTimeStepData);
  static NFmiQueryData *ModifyData(const std::string &theMacroText,
                                   NFmiQueryData *theModifiedData,
                                   NFmiTimeDescriptor *theTimes,
                                   const checkedVector<std::string> *theHelperDataFileNames,
                                   bool createDrawParamFileIfNotExist,
                                   bool goThroughLevels,
                                   bool fMakeStaticIfOneTimeStepData);

 private:
  static bool InitDataBase(NFmiInfoOrganizer *theDataBase,
                           NFmiQueryData *theModifiedData,
                           const checkedVector<std::string> *theHelperDataFileNames,
                           bool createDrawParamFileIfNotExist,
                           bool fMakeStaticIfOneTimeStepData);
  static bool InitDataBaseHelperData(NFmiInfoOrganizer &theDataBase,
                                     const checkedVector<std::string> &theHelperDataFileNames,
                                     bool fMakeStaticIfOneTimeStepData);
  static std::string GetWorkingDirectory(void);
};
