#pragma once
/*!
 *  \file NFmiSmartToolInfo.h
 *  C++ Class Name : NFmiSmartToolInfo \par
 *  ---------------------------------------------------------
 *  Luokka pitää sisällään SmartTool:ia ja SmartToolDialogia
 *  koskevia tietoja.
 *
 */

#include <string>
#include <vector>

class NFmiSmartToolInfo
{
 public:
  NFmiSmartToolInfo();
  ~NFmiSmartToolInfo();
  bool Init(const std::string& theLoadDirectory);

  bool LoadScript(const std::string& theScriptName);
  bool SpeedLoadScript(const std::string& theScriptName);
  bool SaveScript(const std::string& theFullScriptPath);
  bool RemoveScript(const std::string& theScriptName);
  bool LoadDBChecker();
  bool SaveDBChecker();

  const std::string& CurrentScript() const { return itsCurrentScript; }
  const std::string& CurrentScriptName() const { return itsCurrentScriptName; }
  const std::string& LoadDirectory() const { return itsLoadDirectory; }
  const std::string& RootLoadDirectory() const { return itsRootLoadDirectory; }
  const std::string& DBCheckerFileName() const { return itsDBCheckerFileName; }
  const std::string& DBCheckerText() const { return itsDBCheckerText; }
  bool MakeDBCheckAtSend() const { return fMakeDBCheckAtSend; }
  bool IsThereDBCheckScript() const { return fIsThereDBCheckScript; }
  void CurrentScript(const std::string& newValue)
  {
    itsCurrentScript = newValue;
    SaveSettings();
  }
  void CurrentScriptName(const std::string& newValue)
  {
    itsCurrentScriptName = newValue;
    SaveSettings();
  }
  void LoadDirectory(const std::string& newValue, bool fSaveSettings);
  void SetCurrentLoadDirectory(const std::string& newValue);  // tässä ei aseteta root-directoria
  void DBCheckerFileName(const std::string& newValue) { itsDBCheckerFileName = newValue; }
  void DBCheckerText(const std::string& newValue) { itsDBCheckerText = newValue; }
  void MakeDBCheckAtSend(bool newValue)
  {
    fMakeDBCheckAtSend = newValue;
    SaveSettings();
  }
  void IsThereDBCheckScript(bool newValue) { fIsThereDBCheckScript = newValue; }
  bool ScriptExist(const std::string& theScriptName);  // löytyykö tämän nimistä skriptiä
  std::vector<std::string> GetScriptNames(
      void);  // hae lista olemassa olevista skripteistä (pelkkä nimi lista)
  const std::string& ScriptFileExtension() const { return itsScriptFileExtension; }
  void ScriptFileExtension(const std::string& newValue) { itsScriptFileExtension = newValue; }
  std::string GetFullScriptFileName(const std::string& theScriptName);
  std::string GetRelativeLoadPath() const;

 private:
  bool WriteScript2File(const std::string& theFileName, const std::string& theScript);
  bool LoadSettings();
  bool SaveSettings();

  std::string itsCurrentScript;        //! Dialogissa oleva scripti.
  std::string itsScriptFileExtension;  //! smarttool-macrot talletetaan tiedostoihin tällä
                                       //! extensiolla (.st).
  std::string itsCurrentScriptName;    //! (talleta init-fileen) Dialogissa olevan scriptin polku ja
                                       //! tiedostonnimi kokonaisuudessaan.
  std::string itsLoadDirectory;        //! (talleta init-fileen) Viimeisin hakemisto, mistä on
                                       //! ladattu/talletettu smarttool scripti.
  std::string itsRootLoadDirectory;  //! tämän avulla otetaan kansiot käyttöön, jos ollaan tässä, ei
                                     //! listata parent-hakemistoa
  std::string itsDBCheckerFileName;  //! Tiedoston nimi (polkuineen), mistä ladataan DBChecker
                                     //! smarttool scripti.
  std::string itsDBCheckerText;      //! Itse DBChecker smarttool scripti.
  bool fMakeDBCheckAtSend;  //! (talleta init-fileen) Tehdäänkö tarkistus scripti automaattisesti,
                            //! kun dataa lähetetään tietokantaan.
  bool fIsThereDBCheckScript;  //! Onko ylipäätään ollenkaan oletus tiedostossa tallessa DBChecker
                               //! scriptiä.
};
