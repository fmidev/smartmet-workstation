#pragma once
//**********************************************************
// C++ Class Name : NFmiSmartToolIntepreter
// ---------------------------------------------------------
//  Author         : pietarin
//  Creation Date  : 8.11. 2010
//
// Aluksi simppeli smarttool-tulkki. Macro voi olla muotoa:
//
// calculationSection1
// IF(mask1)
//   calculationSection2
// ELSEIF(mask2)
//   calculationSection3
// ELSE(mask3)
// {
//   calculationSection4
// }
// calculationSection5
//
// Eli Voi olla laskuja, jotka suoritetaan aina ensin (calculationSection1), sitten tulee
// haluttu määrä IF - ELSEIF - ELSE osioita, joihin jokaiseen liittyy jokin calculationSection.
// ELSEIF:jä voi olla useita, mutta IF ja ELSE osioita voi olla vain yksi kumpaakin
// ja niiden on oltava päissä (IF alussa ja ELSE lopussa).
// Lopuksi vielä on calculationSection, joka suoritetaan aina.
// Kaikki osiot sikä vapaa ehtoisia, ettei mitään tarvitse olla, mutta esim IF, ELSEIF ja ELSE:n
// jälkeen pitää tulla calculationSection.
//**********************************************************

#include "NFmiExtraMacroParamData.h"
#include <newbase/NFmiAreaMask.h>
#include <newbase/NFmiLevelType.h>
#include <newbase/NFmiParamBag.h>
#include <newbase/NFmiParameterName.h>
#include <newbase/NFmiProducer.h>
#include <newbase/NFmiProducerName.h>

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <map>
#include <set>

class NFmiSmartToolCalculationSectionInfo;
class NFmiAreaMaskSectionInfo;
class NFmiAreaMaskInfo;
class NFmiSmartToolCalculationInfo;
class NFmiSmartToolCalculationBlockInfo;
class NFmiProducerSystem;
class NFmiExtraMacroParamData;
class NFmiSimpleConditionPartInfo;
class NFmiSingleConditionInfo;

enum class SmarttoolsUserVariableType
{
    None,
    Var,
    Const
};

class NFmiSmartToolCalculationBlockInfoVector
{
 public:
  typedef std::vector<boost::shared_ptr<NFmiSmartToolCalculationBlockInfo> >::iterator Iterator;

  NFmiSmartToolCalculationBlockInfoVector();
  ~NFmiSmartToolCalculationBlockInfoVector();
  void Clear();
  void Add(boost::shared_ptr<NFmiSmartToolCalculationBlockInfo> &theBlockInfo);
  void AddModifiedParams(std::map<int, std::string> &theModifiedParams);
  Iterator Begin() { return itsCalculationBlockInfos.begin(); };
  Iterator End() { return itsCalculationBlockInfos.end(); };
  bool Empty() const { return itsCalculationBlockInfos.empty(); }
  bool BlockWasEnclosedInBrackets() const;

 private:
  // luokka ei omista vektorissa olevia otuksia, Clear pitää kutsua erikseen!!!
  std::vector<boost::shared_ptr<NFmiSmartToolCalculationBlockInfo> > itsCalculationBlockInfos;
};

class NFmiSmartToolCalculationBlockInfo
{
 public:
  NFmiSmartToolCalculationBlockInfo();
  ~NFmiSmartToolCalculationBlockInfo();
  void Clear();
  void AddModifiedParams(std::map<int, std::string> &theModifiedParams);

  // luokka ei omista näitä, Clear pitää kutsua erikseen!!!
  boost::shared_ptr<NFmiSmartToolCalculationSectionInfo> itsFirstCalculationSectionInfo;
  boost::shared_ptr<NFmiAreaMaskSectionInfo> itsIfAreaMaskSectionInfo;
  boost::shared_ptr<NFmiSmartToolCalculationBlockInfoVector> itsIfCalculationBlockInfos;
  boost::shared_ptr<NFmiAreaMaskSectionInfo> itsElseIfAreaMaskSectionInfo;
  boost::shared_ptr<NFmiSmartToolCalculationBlockInfoVector> itsElseIfCalculationBlockInfos;
  bool fElseSectionExist;
  boost::shared_ptr<NFmiSmartToolCalculationBlockInfoVector> itsElseCalculationBlockInfos;
  boost::shared_ptr<NFmiSmartToolCalculationSectionInfo> itsLastCalculationSectionInfo;
  bool fStartingBracketFound = false;
  bool fEndingBracketFound = false;
  bool BlockWasEnclosedInBrackets() const { return fStartingBracketFound && fEndingBracketFound; }
};

class NFmiSmartToolIntepreter
{
 public:
  typedef std::pair<FmiProducerName, NFmiInfoData::Type> ProducerIdTypePair;
  typedef std::pair<NFmiProducer, NFmiInfoData::Type> ProducerTypePair;
  typedef std::map<std::string, ProducerIdTypePair> ProducerMap;
  typedef std::map<std::string, double> ConstantMap;  // esim. MISS 32700 tai PI 3.14159
  typedef std::map<std::string, FmiParameterName> ParamMap;
  // Vert(ikaali)Funktioihin talletetaan:
  // 1. Funktion nimi std::map:in key
  // 2. Funktio tyyppi-1 (avg,max,find, jne.)
  // 3. Tyyppi2 (esim. VertP, VertZ, AreaCircle, ProbOver, etc.)
  // 4. Funktion argumenttien lukumäärä (int). Jos funktiolle sallitaan simple-condition (ks. kohta
  // 6), sitä ei lasketa argumentiksi.
  // 5. Funktion oikea 'muoto' stringinä joka pitää sisällään esim. grad-funktion tapauksessa
  // "grad(param)"
  // 6. Funktion simple-condition sääntö eli ei sallita/sallitaan/pakollinen simple-condition teksti
  // ("WS_ec > 10") funktiolle loppuun, esim. area_min(T_ec, 50, -3, 0 , "WS_ec > 10")
  typedef boost::tuple<NFmiAreaMask::FunctionType,
                       NFmiAreaMask::FunctionType,
                       int,
                       std::string,
                       NFmiAreaMask::SimpleConditionRule>
      VertFunctionMapValue;
  // Vertikaali funktiot. Näillä funktioilla käsitellään queryData-olioita eli pyydetään erilaisia
  // arvoja siitä (esim. vertp_max(WS_hir, p1, p2) hakee hirlamin maksimi tuulen nopeuden p1 ja p2
  // painepintojen väliltä).
  typedef std::map<std::string, VertFunctionMapValue> VertFunctionMap;
  typedef std::map<std::string, NFmiAreaMask::FunctionType> FunctionMap;
  typedef std::map<std::string, FmiLevelType> ResolutionLevelTypesMap;

  void Interpret(const std::string &theMacroText, bool fThisIsMacroParamSkript = false);

  NFmiSmartToolIntepreter(NFmiProducerSystem *theProducerSystem,
                          NFmiProducerSystem *theObservationProducerSystem = 0);
  ~NFmiSmartToolIntepreter();

  void Clear();
  const std::string &GetMacroText() const { return itsMacroText; }
  const std::string &GetStrippedMacroText() const { return itsStrippedMacroText; }
  const std::string &IncludeDirectory() const { return itsIncludeDirectory; }
  void IncludeDirectory(const std::string &newValue) { itsIncludeDirectory = newValue; }
  std::vector<NFmiSmartToolCalculationBlockInfo> &SmartToolCalculationBlocks()
  {
    return itsSmartToolCalculationBlocks;
  }
  NFmiParamBag ModifiedParams();
  NFmiParam GetParamFromString(const std::string &theParamText);
  // kun intepreter on tulkinnut smarttool-tekstin, voidaan kysyä, onko kyseinen makro ns.
  // macroParam-skripti eli sisältääkö se RESULT = ??? tapaista tekstiä
  bool IsInterpretedSkriptMacroParam();
  NFmiExtraMacroParamData &ExtraMacroParamData() { return itsExtraMacroParamData; }

  // Näitä static funktioita on tarkoitus käyttää sekä tässä luokassa että sen ulkopuolella.
  static bool IsBaseDelimiter(char c);
  static bool IsDelimiter(char c);
  static const std::string &GetBaseDelimiterChars();
  static const std::string &GetFullDelimiterChars();
  // Tätä käytetään tutkimaan pitääkö annettu theVariableText sisällään parametrin tuottaja/level
  // tietoineen. Tarkoitus käyttää tutkimaan että kun tulee uutta dataa, että pitääkö jotain näyttöä
  // päivittää, jos siinä on macroParam layereitä.
  static bool InterpretVariableForChecking(const std::string &theVariableText,
                                           boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  static VertFunctionMap &GetTokenVertFunctions()
  {
    return NFmiSmartToolIntepreter::itsTokenVertFunctions;
  }
  static FunctionMap &GetTokenThreeArgumentFunctions() { return itsTokenThreeArgumentFunctions; }
  static bool IsWantedStart(const std::string &theText, const std::string &theWantedStart);
  static void SetAbsoluteBasePaths(const std::string &theAbsoluteSmarttoolsBasePath,
                                   const std::string &theAbsoluteMacroParamBasePath);
  static ProducerTypePair GetPossibleProducerInfo(const std::string &theProducerText);
  static std::pair<bool, NFmiDefineWantedData> CheckForVariableDataType(
      const std::string &originalDataVariableString);

 private:
  bool CheckoutPossibleNextCalculationBlockVector(
      boost::shared_ptr<NFmiSmartToolCalculationBlockInfoVector> &theBlockVector);
  bool CheckoutPossibleNextCalculationBlock(NFmiSmartToolCalculationBlockInfo &theBlock,
                                            bool fFirstLevelCheckout,
                                            int theBlockIndex = -1);
  std::string HandlePossibleUnaryMarkers(const std::string &theCurrentString);
  static NFmiLevel GetPossibleLevelInfo(const std::string &theLevelText,
                                        NFmiInfoData::Type theDataType);
  static bool IsProducerOrig(std::string &theProducerText);
  static bool FindParamAndLevelAndSetMaskInfo(const std::string &theVariableText,
                                              const std::string &theLevelText,
                                              NFmiAreaMask::CalculationOperationType theOperType,
                                              NFmiInfoData::Type theDataType,
                                              boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                                              int theModelRunIndex,
                                              float theTimeOffsetInHours);
  static bool FindParamAndProducerAndSetMaskInfo(const std::string &theVariableText,
                                                 const std::string &theProducerText,
                                                 NFmiAreaMask::CalculationOperationType theOperType,
                                                 NFmiInfoData::Type theDataType,
                                                 boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                                                 int theModelRunIndex,
                                                 float theTimeOffsetInHours);
  static bool FindParamAndLevelAndProducerAndSetMaskInfo(
      const std::string &theVariableText,
      const std::string &theLevelText,
      const std::string &theProducerText,
      NFmiAreaMask::CalculationOperationType theOperType,
      NFmiInfoData::Type theDataType,
      boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
      int theModelRunIndex,
      float theTimeOffsetInHours);
  bool ExtractParamAndLevel(const std::string &theVariableText,
                            std::string *theParamNameOnly,
                            std::string *theLevelNameOnly);
  bool IsVariableMacroParam(const std::string &theVariableText,
                            boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool IsVariableDeltaZ(const std::string &theVariableText,
                        boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool IsVariableExtraInfoCommand(const std::string &theVariableText);
  bool IsVariableRampFunction(const std::string &theVariableText,
                              boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  static bool FindParamAndSetMaskInfo(const std::string &theVariableText,
                                      ParamMap &theParamMap,
                                      NFmiAreaMask::CalculationOperationType theOperType,
                                      NFmiInfoData::Type theDataType,
                                      boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                                      int theModelRunIndex,
                                      float theTimeOffsetInHours);
  static bool FindParamAndSetMaskInfo(const std::string &theVariableText,
                                      ParamMap &theParamMap,
                                      NFmiAreaMask::CalculationOperationType theOperType,
                                      NFmiInfoData::Type theDataType,
                                      boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                                      const NFmiProducer &theProducer,
                                      int theModelRunIndex,
                                      float theTimeOffsetInHours);
  void InterpretDelimiter(const std::string &theDelimText,
                          boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  void InterpretToken(const std::string &theTokenText,
                      boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool InterpretMaskSection(const std::string &theMaskSectorText,
                            boost::shared_ptr<NFmiAreaMaskSectionInfo> &theAreaMaskSectionInfo);
  bool InterpretMasks(std::string &theMaskSectionText,
                      boost::shared_ptr<NFmiAreaMaskSectionInfo> &theAreaMaskSectionInfo);
  bool InterpretCalculationSection(
      std::string &theCalculationSectionText,
      boost::shared_ptr<NFmiSmartToolCalculationSectionInfo> &theSectionInfo);
  boost::shared_ptr<NFmiSmartToolCalculationInfo> InterpretCalculationLine(
      const std::string &theCalculationLineText);

  std::string::const_iterator ExtractFirstCalculationSection(
      const std::string &theMacroText, std::string::iterator theStartPosition);

  void InitCheckOut();
  static bool IsPossiblyLevelItem(const std::string &theText);
  static bool IsPossiblyProducerItem(const std::string &theText, ProducerMap &theMap);
  static bool GetProducerFromVariableById(const std::string &theVariableText,
                                          NFmiProducer &theProducer);
  static bool GetLevelFromVariableById(const std::string &theVariableText,
                                       NFmiLevel &theLevel,
                                       NFmiInfoData::Type theDataType);
  static bool GetParamFromVariable(const std::string &theVariableText,
                                   ParamMap &theParamMap,
                                   NFmiParam &theParam,
                                   bool &fUseWildDataType);
  static bool GetParamFromVariableById(const std::string &theVariableText, NFmiParam &theParam);
  bool CheckoutPossibleIfClauseSection(
      boost::shared_ptr<NFmiAreaMaskSectionInfo> &theAreaMaskSectionInfo);
  bool CheckoutPossibleElseIfClauseSection(
      boost::shared_ptr<NFmiAreaMaskSectionInfo> &theAreaMaskSectionInfo);
  bool CheckoutPossibleElseClauseSection();
  bool CheckoutPossibleNextCalculationSection(
      boost::shared_ptr<NFmiSmartToolCalculationSectionInfo> &theSectionInfo,
      bool &fWasBlockMarksFound);
  bool ExtractPossibleNextCalculationSection(bool &fWasBlockMarksFound);
  bool ExtractPossibleIfClauseSection();
  bool ExtractPossibleElseIfClauseSection();
  template <typename memfunction>
  bool ExtractPossibleConditionalClauseSection(memfunction conditionalChecker);
  bool IsPossibleCalculationLine(const std::string &theTextLine);
  bool IsPossibleIfConditionLine(const std::string &theTextLine);
  bool IsPossibleElseIfConditionLine(const std::string &theTextLine);
  bool IsPossibleElseConditionLine(const std::string &theTextLine);
  bool FindAnyFromText(const std::string &theText,
                       const std::vector<std::string> &theSearchedItems);
  bool StartsWithAnyWholeWord(const std::string &theText,
                              const std::vector<std::string> &theSearchedWords);
  bool ConsistOnlyWhiteSpaces(const std::string &theText);
  bool IsVariableBinaryOperator(const std::string &theVariableText,
                                boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  NFmiAreaMask::CalculationOperator InterpretCalculationOperator(
      const std::string &theOperatorText);
  void InterpretVariable(const std::string &theVariableText,
                         boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                         SmarttoolsUserVariableType theNewVariableType = SmarttoolsUserVariableType::None);
  void InterpretStringLiteral(const std::string &theVariableText,
                              boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool InterpretSimpleCondition(const std::string &theVariableText,
                                boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool InterpretSimpleCondition(const std::string &theVariableText,
                                const std::vector<std::string> &words,
                                boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  boost::shared_ptr<NFmiSimpleConditionPartInfo> GetNextSimpleConditionPart(
      const std::string &theVariableText,
      const std::vector<std::string> &words,
      size_t &startingWordIndexInOut);
  boost::shared_ptr<NFmiSingleConditionInfo> GetNextSingleCondition(
      const std::string &theVariableText,
      const std::vector<std::string> &words,
      size_t &startingWordIndexInOut);

  bool InterpretVariableCheckTokens(const std::string &theVariableText,
                                    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                                    bool fOrigWanted,
                                    bool fLevelExist,
                                    bool fProducerExist,
                                    const std::string &theParamNameOnly,
                                    const std::string &theLevelNameOnly,
                                    const std::string &theProducerNameOnly,
                                    int theModelRunIndex,
                                    float theTimeOffsetInHours);
  static bool InterpretVariableOnlyCheck(const std::string &theVariableText,
                                         boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                                         bool fOrigWanted,
                                         bool fLevelExist,
                                         bool fProducerExist,
                                         const std::string &theParamNameOnly,
                                         const std::string &theLevelNameOnly,
                                         const std::string &theProducerNameOnly,
                                         int theModelRunIndex,
                                         float theTimeOffsetInHours);

  bool InterpretPossibleScriptVariable(const std::string &theVariableText,
                                       boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                                       SmarttoolsUserVariableType theNewVariableType);
  bool InterpretPossibleScriptConstVariable(const std::string &theVariableText,
                                       boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                                       SmarttoolsUserVariableType theNewVariableType);
  static void CheckVariableString(const std::string &theVariableText,
                                  std::string &theParamText,
                                  bool &fLevelExist,
                                  std::string &theLevelText,
                                  bool &fProducerExist,
                                  std::string &theProducerText,
                                  int &theModelRunIndex,
                                  float &theTimeOffsetInHours);
  static bool IsVariableConstantValue(const std::string &theVariableText,
                                      boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  std::string ExtractNextLine(std::string &theText,
                              std::string::iterator theStartPos,
                              std::string::iterator *theEndPos);
  bool IsVariableThreeArgumentFunction(const std::string &theVariableText,
                                       boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool IsVariableFunction(const std::string &theVariableText,
                          boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool IsVariablePeekFunction(const std::string &theVariableText,
                              boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool IsVariableMetFunction(const std::string &theVariableText,
                             boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool IsVariableVertFunction(const std::string &theVariableText,
                              boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  void ExtractPossibleSecondaryParameterInfo(boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  bool IsVariableMathFunction(const std::string &theVariableText,
                              boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo);
  void SearchUntil(std::string::iterator &theExp_ptr,
                   char *theTempCharPtr,
                   char theSearchedCh,
                   const std::string &theErrorStr);
  static bool IsFunctionNameWithUnderScore(const std::string &theVariableText);
  static void CheckIfVariableResemblesVerticalFunction(const std::string &theVariableText);
  bool ExtractResolutionInfo();
  bool ExtractCalculationPointInfo();
  bool ExtractObservationRadiusInfo();
  bool ExtractSymbolTooltipFile(bool multiParamCase);
  bool ExtractMacroParamDescription();
  bool ExtractCalculationType();
  bool ExtractWorkingThreadCount();
  bool ExtractFixedBaseData();
  bool ExtractMultiParam(NFmiAreaMask::FunctionType multiParamId);
  std::string GetWholeNumberFromTokens();
  void CheckMustHaveSimpleConditionFunctions(
      boost::shared_ptr<NFmiSmartToolCalculationInfo> &theCalculationInfo);
  void AddVariableToCalculation(boost::shared_ptr<NFmiSmartToolCalculationInfo> &theCalculationInfo,
                                boost::shared_ptr<NFmiAreaMaskInfo> &theVariableInfo);
  void AddSimpleCalculationToCallingAreaMask(
      boost::shared_ptr<NFmiSmartToolCalculationInfo> &theCalculationInfo,
      const boost::shared_ptr<NFmiAreaMaskInfo> &theSimpleCalculationAreaMask);
  std::pair<bool, NFmiDefineWantedData> GetPossibleVariableDataInfo(
      const std::string &originalResolutionStr);
  const std::string &GetUsedAbsoluteBasePath() const;
  std::string FixGivenSmarttoolsScriptPath(const std::string &thePathInScript) const;
  SmarttoolsUserVariableType DoUserVariableChecks(std::string &variableNameOut);

  NFmiProducerSystem *itsProducerSystem;               // ei omista
  std::string itsCheckOutSectionText;                  // esim. if-sectionin koko teksti
  std::string::iterator itsCheckOutTextStartPosition;  // sen hetkinen tekstiosan alkupiste
  std::string::iterator itsCheckOutTextEndPosition;    // sen hetkinen tekstiosan alkupiste
  bool fContinueCurrentSectionCheckOut;  // jatketaanko sen hetkisen sectionin tutkimista, vai onko
                                         // selvää, ettei esim. else-sectionia ole olemassa

  void SetMacroTexts(const std::string &theMacroText);
  std::string itsMacroText;
  std::string itsStrippedMacroText;
  std::string itsIncludeDirectory;  // mistä ladataan mahd. include filet

  std::vector<NFmiSmartToolCalculationBlockInfo> itsSmartToolCalculationBlocks;
  int itsMaxCalculationSectionCount;
  NFmiExtraMacroParamData itsExtraMacroParamData;

  static void InitTokens(NFmiProducerSystem *theProducerSystem,
                         NFmiProducerSystem *theObservationProducerSystem);
  static void InitProducerTokens(NFmiProducerSystem *theProducerSystem,
                                 NFmiInfoData::Type theDefaultDataType);
  static bool fTokensInitialized;
  static ParamMap itsTokenParameterNamesAndIds;
  // Tietyt tuottajat laitetaan tanne nimen, tuottaja-id:n ja oletus datatyypin kanssa.
  static ProducerMap itsTokenProducerNamesAndIds;
  static ConstantMap itsTokenConstants;
  static std::vector<std::string> itsTokenConditionalCommands;
  static std::vector<std::string> itsTokenIfCommands;
  static std::vector<std::string> itsTokenElseIfCommands;
  static std::vector<std::string> itsTokenElseCommands;
  static std::vector<std::string> itsTokenCalculationBlockMarkers;
  static std::vector<std::string> itsTokenMaskBlockMarkers;
  static std::vector<std::string> itsTokenRampUpFunctions;
  static std::vector<std::string> itsTokenRampDownFunctions;
  static std::vector<std::string> itsTokenDoubleRampFunctions;
  static std::vector<std::string> itsTokenRampFunctions;
  static std::vector<std::string> itsTokenMacroParamIdentifiers;  // tänne listataan result jne.
                                                                  // sanat joita käytetään
                                                                  // makrojen visualisoinnissa
  static std::vector<std::string> itsTokenDeltaZIdentifiers;  // tänne listataan deltaz 'funktiot'

  typedef std::map<std::string, FmiMaskOperation> MaskOperMap;
  static MaskOperMap itsTokenMaskOperations;
  typedef std::map<std::string, NFmiAreaMask::CalculationOperator> CalcOperMap;
  static CalcOperMap itsCalculationOperations;
  typedef std::map<std::string, NFmiAreaMask::BinaryOperator> BinaOperMap;
  static BinaOperMap itsBinaryOperator;
  static ParamMap itsTokenStaticParameterNamesAndIds;
  static ParamMap itsTokenCalculatedParameterNamesAndIds;  // mm. lat, lon ja elevAngle

  static FunctionMap itsTokenFunctions;
  static FunctionMap itsTokenThreeArgumentFunctions;
  static FunctionMap itsExtraInfoCommands;  // Tänne mm. resolution- ja calculationpoint -jutut

  typedef boost::tuple<NFmiAreaMask::FunctionType,
                       NFmiAreaMask::MetFunctionDirection,
                       int,
                       std::string>
      MetFunctionMapValue;  // MetFunktioihin talletetaan 'taika'-sanan lisäksi Funktio tyyppi ja
  // funktion argumenttien lukumäärä ja funktion oikea 'muoto' stringinä,
  // joka pitää sisällään esim. grad-funktion tapauksessa "grad(param)"
  typedef std::map<std::string, MetFunctionMapValue> MetFunctionMap;  // 'Meteorologiset' funktiot.
                                                                      // Näillä funktioilla
                                                                      // käsitellään
                                                                      // queryData-olioita eli
                                                                      // pyydetään erilaisia
  // arvoja siitä (esim. advektiota Adv(T_Hir)).
  static MetFunctionMap itsTokenMetFunctions;

  static VertFunctionMap itsTokenVertFunctions;

  typedef std::map<std::string, NFmiAreaMask::CalculationOperationType> PeekFunctionMap;
  static PeekFunctionMap itsTokenPeekFunctions;

  typedef std::map<std::string, NFmiAreaMask::MathFunctionType> MathFunctionMap;
  static MathFunctionMap itsMathFunctions;

  static ResolutionLevelTypesMap itsResolutionLevelTypes;

  typedef std::map<std::string, int> ScriptVariableMap;
  // skriptissä varatut muuttujat (var x = ?) talletetaan tänne, 
  // että voidaan tarkistaa niiden olemassa olo
  ScriptVariableMap itsTokenScriptVariableNames;
  // pitää keksia muutujille id, joten tehdää juokseva counter normaali 
  // ja macroParam sijoituksia halutaan seurata, että ei tapahdu vahinkoja 
  // eli niitä olisi sekaisin, jolloin seuramukset ovat vahingollisia.
  // Alustetaan tämä jollain isolla 'random' id numerolla, jotta ei
  // mene päällekkäin oikeiden parametri id numeroiden kanssa.
  int itsScriptVariableParamIdCounter = 918273645;
  typedef std::map<std::string, double> ScriptConstVariableMap;
  // skriptissä varatut nimetyt vakiomuuttujat (const x = ?) talletetaan tänne,
  // että voidaan tarkistaa niiden olemassa olo
  ScriptConstVariableMap itsTokenScriptConstVariableNames;
  // loytyykö skriptistä normaaleja sijoituksia esim. T = ???
  bool fNormalAssigmentFound;  
  // loytyykö skriptistä ns. macroParameri sijoituksia eli RESULT = ?????
  bool fMacroParamFound;  
  // Tieto siitä tulkitaanko macroParam-skriptiä vai tavallista
  // skriptiä. Poikkeus heitetään jos macrpParam-skripti päällä,
  // mutta tehdään tavallinen sijoitus
  bool fMacroParamSkriptInProgress;
  // Kulloisenkin tulkattavan rivin sisältö
  std::string itsCalculationLineText;
  // GetToken ja IsDelim otettu H. Schilbertin  C++: the Complete Refeference third ed.
  // jouduin muuttamaan niitä vähän sopimaan tähän ympäristöön.
  bool GetToken();
  bool IsDelim(char c);
  // Ed. funktiot käyttävät seuraavia muuttujia:
  enum types
  {
    NOTYPE = 0,
    DELIMITER = 1,
    VARIABLE,
    NUMBER,
    STRING_LITERAL
  };
  std::string::iterator exp_ptr;  // points to the expression
  std::string::iterator exp_end;
  char token[128];  // holds current token
  types tok_type;   // holds token's type

  static std::string itsBaseDelimiterChars;
  static std::string itsFullDelimiterChars;
  // Ainakin SymbolTooltipFile:ä määriteltäessä on hyvä olla tiedossa SmartMetin käytössä olevat
  // perushakemistot, jotta voidaan täydentää #include ja SymbolTooltipFile polut absoluuttisiksi.
  static std::string itsAbsoluteSmarttoolsBasePath;
  static std::string itsAbsoluteMacroParamBasePath;
};
