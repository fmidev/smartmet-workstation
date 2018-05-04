// ======================================================================
/*!
 * \file NFmiSmartToolUtil.cpp
 * \brief Implementation of class NFmiSmartToolUtil
 */
// ======================================================================
#ifdef _MSC_VER
#pragma warning(disable : 4786)  // poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255
                                 // merkkiä joka johtuu 'puretuista' STL-template nimistä)
#endif

#include "NFmiInfoOrganizer.h"
#include "NFmiSmartToolModifier.h"
#include "NFmiSmartToolUtil.h"
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiQueryData.h>
#include <newbase/NFmiStreamQueryData.h>

#ifndef UNIX
#include <direct.h>  // working directory juttuja varten
#else
#include <unistd.h>
#endif

NFmiQueryData *NFmiSmartToolUtil::ModifyData(
    const std::string &theMacroText,
    NFmiQueryData *theModifiedData,
    const checkedVector<std::string> *theHelperDataFileNames,
    bool createDrawParamFileIfNotExist,
    bool goThroughLevels,
    bool fMakeStaticIfOneTimeStepData)
{
  NFmiTimeDescriptor times(theModifiedData->Info()->TimeDescriptor());
  return ModifyData(theMacroText,
                    theModifiedData,
                    &times,
                    theHelperDataFileNames,
                    createDrawParamFileIfNotExist,
                    goThroughLevels,
                    fMakeStaticIfOneTimeStepData);
}

NFmiQueryData *NFmiSmartToolUtil::ModifyData(
    const std::string &theMacroText,
    NFmiQueryData *theModifiedData,
    NFmiTimeDescriptor *theTimes,
    const checkedVector<std::string> *theHelperDataFileNames,
    bool createDrawParamFileIfNotExist,
    bool goThroughLevels,
    bool fMakeStaticIfOneTimeStepData)
{
  NFmiInfoOrganizer dataBase;
  if (!InitDataBase(&dataBase,
                    theModifiedData,
                    theHelperDataFileNames,
                    createDrawParamFileIfNotExist,
                    fMakeStaticIfOneTimeStepData))
  {
    std::cerr << "Database initialization failed, aborting." << std::endl;
    return 0;
  }

  NFmiSmartToolModifier smartToolModifier(&dataBase);

  try  // ensin tulkitaan macro
  {
    smartToolModifier.InitSmartTool(theMacroText);
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 0;
  }

  boost::shared_ptr<NFmiFastQueryInfo> editedInfo = dataBase.FindInfo(NFmiInfoData::kEditable);
  try  // suoritetaan macro sitten
  {
    if (goThroughLevels == false)
      smartToolModifier.ModifyData_ver2(
          theTimes,
          false,
          false,
          0);  // false = ei tehdä muokkauksia vain valituille pisteille vaan kaikille pisteille
    else
    {
      for (editedInfo->ResetLevel(); editedInfo->NextLevel();)
      {
        if (editedInfo->SizeLevels() > 1)  // jos kyseessä on level-data, pitää läpikäytävä leveli
                                           // ottaa talteen, että smartToolModifier osaa luoda
                                           // siihen osoittavia fastInfoja.
        {
          boost::shared_ptr<NFmiLevel> theLevel(new NFmiLevel(*editedInfo->Level()));
          smartToolModifier.ModifiedLevel(theLevel);
        }

        smartToolModifier.ModifyData_ver2(
            theTimes,
            false,
            false,
            0);  // false = ei tehdä muokkauksia vain valituille pisteille vaan kaikille pisteille
                 // ::DoSmartToolModification(smartToolModifier, theTimes);
      }
    }
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 0;
  }

  NFmiQueryData *data = 0;
  if (editedInfo && editedInfo->RefQueryData())
    data = editedInfo->RefQueryData()->Clone();
  return data;
}

NFmiQueryData *NFmiSmartToolUtil::ModifyData(const std::string &theMacroText,
                                             NFmiQueryData *theModifiedData,
                                             bool createDrawParamFileIfNotExist,
                                             bool fMakeStaticIfOneTimeStepData)
{
  NFmiTimeDescriptor times(theModifiedData->Info()->TimeDescriptor());
  return ModifyData(theMacroText,
                    theModifiedData,
                    &times,
                    createDrawParamFileIfNotExist,
                    fMakeStaticIfOneTimeStepData);
}

NFmiQueryData *NFmiSmartToolUtil::ModifyData(const std::string &theMacroText,
                                             NFmiQueryData *theModifiedData,
                                             NFmiTimeDescriptor *theTimes,
                                             bool createDrawParamFileIfNotExist,
                                             bool fMakeStaticIfOneTimeStepData)
{
  return ModifyData(theMacroText,
                    theModifiedData,
                    theTimes,
                    0,
                    createDrawParamFileIfNotExist,
                    false,
                    fMakeStaticIfOneTimeStepData);  // 0=tyhjä apudata filename-lista
}

std::string NFmiSmartToolUtil::GetWorkingDirectory(void)
{
#ifndef UNIX
  static char path[_MAX_PATH];
  int curdrive = ::_getdrive();
  ::_getdcwd(curdrive, path, _MAX_PATH);
  std::string workingDirectory(path);
  return workingDirectory;
#else
  static char path[4096];  // we assume 4096 is maximum buffer length
  if (!::getcwd(path, 4096))
    throw std::runtime_error("Error: Current path is too long (>4096)");
  return std::string(path);
#endif
}

bool NFmiSmartToolUtil::InitDataBase(NFmiInfoOrganizer *theDataBase,
                                     NFmiQueryData *theModifiedData,
                                     const checkedVector<std::string> *theHelperDataFileNames,
                                     bool createDrawParamFileIfNotExist,
                                     bool fMakeStaticIfOneTimeStepData)
{
  if (theDataBase)
  {
    theDataBase->WorkingDirectory(GetWorkingDirectory());
    theDataBase->Init(
        std::string(""), createDrawParamFileIfNotExist, false, false);  // tähän annetaan
                                                                        // drawparametrien lataus
                                                                        // polku, mutta niitä ei
    // käytetä tässä tapauksessa
    // false tarkoittaa että ei tehdä kopiota editoidusta datasta, tässä se on turhaa
    bool dataWasDeleted = false;
    theModifiedData->LatLonCache();  // lasketaan latlon-cache valmiiksi, koska muuten multi-thread
                                     // ympäristössä tulee sen kanssa ongelmia
    theDataBase->AddData(theModifiedData,
                         "xxxfileName",
                         "",
                         NFmiInfoData::kEditable,
                         0,
                         0,
                         0,
                         dataWasDeleted);  // 0=undolevel
    if (theHelperDataFileNames && theHelperDataFileNames->size())
      InitDataBaseHelperData(*theDataBase, *theHelperDataFileNames, fMakeStaticIfOneTimeStepData);
    return true;
  }
  return false;
}

bool NFmiSmartToolUtil::InitDataBaseHelperData(
    NFmiInfoOrganizer &theDataBase,
    const checkedVector<std::string> &theHelperDataFileNames,
    bool fMakeStaticIfOneTimeStepData)
{
  for (unsigned int i = 0; i < theHelperDataFileNames.size(); i++)
  {
    NFmiQueryData *helperdata = new NFmiQueryData(theHelperDataFileNames[i]);

    NFmiInfoData::Type dataType = NFmiInfoData::kViewable;
    if (helperdata->Info()->SizeTimes() == 1)
    {
      if (fMakeStaticIfOneTimeStepData || helperdata->Info()->Param(kFmiTopoGraf))
        dataType = NFmiInfoData::kStationary;
    }

    helperdata->LatLonCache();

    bool dataWasDeleted = false;

    theDataBase.AddData(helperdata,
                        theHelperDataFileNames[i],
                        theHelperDataFileNames[i],
                        dataType,
                        0,
                        0,
                        0,
                        dataWasDeleted);  // 0=undolevel
  }
  return true;
}
