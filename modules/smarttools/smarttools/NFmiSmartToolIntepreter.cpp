//**********************************************************
// C++ Class Name : NFmiSmartToolIntepreter
// ---------------------------------------------------------
//  Author         : pietarin
//  Creation Date  : 8.11. 2010
//
//**********************************************************
#ifdef _MSC_VER
#pragma warning(disable : 4786)  // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiSmartToolIntepreter.h"

#include "NFmiAreaMaskInfo.h"
#include "NFmiAreaMaskSectionInfo.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiPathUtils.h"
#include "NFmiProducerSystem.h"
#include "NFmiSimpleConditionInfo.h"
#include "NFmiSmartToolCalculationInfo.h"
#include "NFmiSmartToolCalculationSectionInfo.h"

#include <newbase/NFmiEnumConverter.h>
#include <newbase/NFmiFileString.h>
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiLevel.h>
#include <newbase/NFmiLevelType.h>
#include <newbase/NFmiPreProcessor.h>
#include <newbase/NFmiValueString.h>

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <boost/algorithm/string.hpp>

// RHEL7 std::regex is broken, must use boost instead
#ifdef UNIX
#include <boost/regex.hpp>
using boost::regex;
using my_regex_iterator = boost::regex_iterator<std::string::iterator>;
#else
#include <regex>
using std::regex;
using my_regex_iterator = std::regex_iterator<std::string::iterator>;
#endif

static const unsigned int gMesanProdId = 160;
static const std::vector<std::string> g_SimpleConditionCombinationWords{
    ">=", "<=", "!=", "==", "<>", "&&", "||", "->"};
static const std::map<std::string, NFmiAreaMask::CalculationOperator>
    g_SimpleConditionCalculationOperatorMap{{"+", NFmiAreaMask::Add},
                                            {"-", NFmiAreaMask::Sub},
                                            {"*", NFmiAreaMask::Mul},
                                            {"/", NFmiAreaMask::Div},
                                            {"%", NFmiAreaMask::Mod},
                                            {"^", NFmiAreaMask::Pow}};
static const std::map<std::string, NFmiAreaMask::BinaryOperator> g_SimpleConditionBinaryOperatorMap{
    {"and", NFmiAreaMask::kAnd},
    {"&&", NFmiAreaMask::kAnd},
    {"or", NFmiAreaMask::kOr},
    {"||", NFmiAreaMask::kOr},
    {"xor", NFmiAreaMask::kXor}};

using namespace std;

// globaali tarkistus luokka etsimään rivin lopetusta
struct EndOfLineSearcher
{
  template <typename T>
  bool operator()(T theChar)
  {  // tarkistetaan myös blokin loppu merkki '}' koska blokki-koodi voi olla seuraavanlaista "{T =
     // T + 1}" eli blokin loppu merkki samalla rivillä
    return (theChar == '\r' || theChar == '\n' || theChar == '}' || theChar == '{');
  }
};

// HUOM!!! vaarallinen luokka, pitää muistaa kutsua Clearia, muuten vuotaa!!!
NFmiSmartToolCalculationBlockInfoVector::NFmiSmartToolCalculationBlockInfoVector()
    : itsCalculationBlockInfos()
{
}

NFmiSmartToolCalculationBlockInfoVector::~NFmiSmartToolCalculationBlockInfoVector() {}
void NFmiSmartToolCalculationBlockInfoVector::Clear()
{
  itsCalculationBlockInfos.clear();
}
// Ottaa pointterin 'omistukseensa' eli pitää luoda ulkona new:llä ja antaa tänne
void NFmiSmartToolCalculationBlockInfoVector::Add(
    boost::shared_ptr<NFmiSmartToolCalculationBlockInfo> &theBlockInfo)
{
  itsCalculationBlockInfos.push_back(theBlockInfo);
}

void NFmiSmartToolCalculationBlockInfoVector::AddModifiedParams(
    std::map<int, std::string> &theModifiedParams)
{
  Iterator it = Begin();
  Iterator endIt = End();
  for (; it != endIt; ++it)
  {
    (*it)->AddModifiedParams(theModifiedParams);
  }
}

bool NFmiSmartToolCalculationBlockInfoVector::BlockWasEnclosedInBrackets() const
{
  if (itsCalculationBlockInfos.empty())
    return false;
  else if (itsCalculationBlockInfos.size() == 1)
    return itsCalculationBlockInfos.front()->BlockWasEnclosedInBrackets();
  else
  {
    return itsCalculationBlockInfos.front()->fStartingBracketFound &&
           itsCalculationBlockInfos.back()->fEndingBracketFound;
  }
}

// HUOM!!! vaarallinen luokka, pitää muistaa kutsua Clearia, muuten vuotaa!!!
NFmiSmartToolCalculationBlockInfo::NFmiSmartToolCalculationBlockInfo()
    : itsFirstCalculationSectionInfo(new NFmiSmartToolCalculationSectionInfo),
      itsIfAreaMaskSectionInfo(new NFmiAreaMaskSectionInfo),
      itsIfCalculationBlockInfos(),
      itsElseIfAreaMaskSectionInfo(new NFmiAreaMaskSectionInfo),
      itsElseIfCalculationBlockInfos(),
      itsElseCalculationBlockInfos(),
      itsLastCalculationSectionInfo(new NFmiSmartToolCalculationSectionInfo)
{
}

NFmiSmartToolCalculationBlockInfo::~NFmiSmartToolCalculationBlockInfo() {}

void NFmiSmartToolCalculationBlockInfo::Clear()
{
  if (itsIfCalculationBlockInfos)
    itsIfCalculationBlockInfos->Clear();
  if (itsElseIfCalculationBlockInfos)
    itsElseIfCalculationBlockInfos->Clear();
  if (itsElseCalculationBlockInfos)
    itsElseCalculationBlockInfos->Clear();
  fElseSectionExist = false;
}

// Lisätään set:iin kaikki parametrit, joita tässä calculationblokissa
// voidaan muokata. Talteen otetaan vain identti, koska muu ei
// kiinnosta (ainakaan nyt).
void NFmiSmartToolCalculationBlockInfo::AddModifiedParams(
    std::map<int, std::string> &theModifiedParams)
{
  if (itsFirstCalculationSectionInfo)  // eka section
    itsFirstCalculationSectionInfo->AddModifiedParams(theModifiedParams);
  if (itsIfCalculationBlockInfos)  // if section
    itsIfCalculationBlockInfos->AddModifiedParams(theModifiedParams);
  if (itsElseIfCalculationBlockInfos)  // elseif section
    itsElseIfCalculationBlockInfos->AddModifiedParams(theModifiedParams);
  if (itsElseCalculationBlockInfos)  // else section
    itsElseCalculationBlockInfos->AddModifiedParams(theModifiedParams);
  if (itsLastCalculationSectionInfo)  // vika section
    itsLastCalculationSectionInfo->AddModifiedParams(theModifiedParams);
}

// Luokan staattisten dataosien pakollinen alustus.
bool NFmiSmartToolIntepreter::fTokensInitialized = false;
NFmiSmartToolIntepreter::ParamMap NFmiSmartToolIntepreter::itsTokenParameterNamesAndIds;
NFmiSmartToolIntepreter::ProducerMap NFmiSmartToolIntepreter::itsTokenProducerNamesAndIds;
NFmiSmartToolIntepreter::ConstantMap NFmiSmartToolIntepreter::itsTokenConstants;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenConditionalCommands;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenIfCommands;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenElseIfCommands;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenElseCommands;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenCalculationBlockMarkers;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenMaskBlockMarkers;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenRampUpFunctions;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenRampDownFunctions;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenDoubleRampFunctions;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenRampFunctions;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenMacroParamIdentifiers;
std::vector<std::string> NFmiSmartToolIntepreter::itsTokenDeltaZIdentifiers;

NFmiSmartToolIntepreter::MaskOperMap NFmiSmartToolIntepreter::itsTokenMaskOperations;
NFmiSmartToolIntepreter::CalcOperMap NFmiSmartToolIntepreter::itsCalculationOperations;
NFmiSmartToolIntepreter::BinaOperMap NFmiSmartToolIntepreter::itsBinaryOperator;
NFmiSmartToolIntepreter::ParamMap NFmiSmartToolIntepreter::itsTokenStaticParameterNamesAndIds;
NFmiSmartToolIntepreter::ParamMap NFmiSmartToolIntepreter::itsTokenCalculatedParameterNamesAndIds;
NFmiSmartToolIntepreter::FunctionMap NFmiSmartToolIntepreter::itsTokenFunctions;
NFmiSmartToolIntepreter::FunctionMap NFmiSmartToolIntepreter::itsTokenThreeArgumentFunctions;
NFmiSmartToolIntepreter::FunctionMap NFmiSmartToolIntepreter::itsExtraInfoCommands;
NFmiSmartToolIntepreter::MetFunctionMap NFmiSmartToolIntepreter::itsTokenMetFunctions;
NFmiSmartToolIntepreter::VertFunctionMap NFmiSmartToolIntepreter::itsTokenVertFunctions;
NFmiSmartToolIntepreter::PeekFunctionMap NFmiSmartToolIntepreter::itsTokenPeekFunctions;
NFmiSmartToolIntepreter::MathFunctionMap NFmiSmartToolIntepreter::itsMathFunctions;
NFmiSmartToolIntepreter::ResolutionLevelTypesMap NFmiSmartToolIntepreter::itsResolutionLevelTypes;

std::string NFmiSmartToolIntepreter::itsBaseDelimiterChars = "+-*/%^=(){}<>&|!,";
std::string NFmiSmartToolIntepreter::itsFullDelimiterChars =
    NFmiSmartToolIntepreter::itsBaseDelimiterChars + " \t\n\r\0";
std::string NFmiSmartToolIntepreter::itsAbsoluteSmarttoolsBasePath = "";
std::string NFmiSmartToolIntepreter::itsAbsoluteMacroParamBasePath = "";

//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiSmartToolIntepreter::NFmiSmartToolIntepreter(NFmiProducerSystem *theProducerSystem,
                                                 NFmiProducerSystem *theObservationProducerSystem)
    : itsProducerSystem(theProducerSystem),
      itsSmartToolCalculationBlocks(),
      itsExtraMacroParamData(),
      fNormalAssigmentFound(false),
      fMacroParamFound(false),
      fMacroParamSkriptInProgress(false)
{
  NFmiSmartToolIntepreter::InitTokens(itsProducerSystem, theObservationProducerSystem);
}
NFmiSmartToolIntepreter::~NFmiSmartToolIntepreter()
{
  Clear();
}
//--------------------------------------------------------
// Interpret
//--------------------------------------------------------
// Tulkitsee annetun macro-tekstin. Erottelee eri calculationSectionit, mahdolliset ehto rakenteet
// ja niiden maskit ja rakentaa sen
// mukaiset maski ja calculation infot, että SmartToolModifier osaa rakentaa oikeat systeemit
// (areamaskit ja lasku-oliot).
// Jos macrossa virhe, heittää poikkeuksen.
// fThisIsMacroParamSkript -parametrin avulla voidaan tarkistaa, ettei ajeta tavallista sijoitusta
// macroParamin
// RESULT = ... sijoitusten sijaan. Tulkin pitää olla tietoinen 'moodista' että voi heittää
// poikkeuksen
// huomatessaan tälläisen tilanteen.
void NFmiSmartToolIntepreter::Interpret(const std::string &theMacroText,
                                        bool fThisIsMacroParamSkript)
{
  fMacroParamSkriptInProgress = fThisIsMacroParamSkript;
  Clear();
  // tyhjennetaan aluksi kaikki skripti muuttujat (Interpret metodia pitäisi 
  // kyllä oikeasti kutsua vain kerran olion elinaikana).
  itsTokenScriptVariableNames.clear();
  itsTokenScriptConstVariableNames.clear();  
  SetMacroTexts(theMacroText);
  InitCheckOut();

  // nollataan tulkinnan aluksi, minkä tyyppisiä parametri sijoituksia sijoituksia löytyy
  fNormalAssigmentFound = false;
  fMacroParamFound = false;

  bool fGoOn = true;
  int index = 0;
  while (fGoOn)
  {
    index++;
    NFmiSmartToolCalculationBlockInfo block;
    try
    {
      if (index > 500)
        throw runtime_error(::GetDictionaryString("SmartToolErrorTooManyBlocks"));
      fGoOn = CheckoutPossibleNextCalculationBlock(block, true);
      itsSmartToolCalculationBlocks.push_back(block);
      if (itsCheckOutTextStartPosition != itsStrippedMacroText.end() &&
          *itsCheckOutTextStartPosition == '}')  // jos ollaan blokin loppu merkissä, siirrytään sen
                                                 // yli ja jatketaan seuraavalle kierrokselle
        ++itsCheckOutTextStartPosition;
    }
    catch (...)
    {
      throw;
    }
  }
}

// kun joku skripti on tulkittu Interpret-metodissa, on tuotettu
// laskenta-lausekkeet itsSmartToolCalculationBlocks, joista tämä
// metodi käy katsomassa, mitä parametreja ollaan mahd. muokkaamassa
// eli T = ? jne. tarkoittaa että lämpötilaa ollaan ainakin
// mahdollisesti muokkaamassa.
NFmiParamBag NFmiSmartToolIntepreter::ModifiedParams()
{
  std::map<int, std::string> modifiedParams;
  std::vector<NFmiSmartToolCalculationBlockInfo>::size_type ssize =
      itsSmartToolCalculationBlocks.size();
  std::vector<NFmiSmartToolCalculationBlockInfo>::size_type i = 0;
  for (; i < ssize; i++)
  {
    itsSmartToolCalculationBlocks[i].AddModifiedParams(modifiedParams);
  }
  NFmiEnumConverter converter;
  NFmiParamBag params;
  std::map<int, std::string>::iterator it = modifiedParams.begin();
  std::map<int, std::string>::iterator endIt = modifiedParams.end();
  for (; it != endIt; ++it)
  {
    params.Add(NFmiDataIdent(NFmiParam((*it).first,
                                       // converter.ToString((*it).first),
                                       (*it).second,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
  }
  params.SetActivities(true);
  return params;
}

bool NFmiSmartToolIntepreter::CheckoutPossibleNextCalculationBlockVector(
    boost::shared_ptr<NFmiSmartToolCalculationBlockInfoVector> &theBlockVector)
{
  int safetyIndex = 0;
  boost::shared_ptr<NFmiSmartToolCalculationBlockInfo> block(
      new NFmiSmartToolCalculationBlockInfo());
  try
  {
    for (; CheckoutPossibleNextCalculationBlock(*(block.get()), false, safetyIndex); safetyIndex++)
    {
      theBlockVector->Add(block);
      if (safetyIndex > 500)
        throw runtime_error(::GetDictionaryString("SmartToolErrorTooManyBlocks"));

      if (itsCheckOutTextStartPosition != itsStrippedMacroText.end() &&
          *itsCheckOutTextStartPosition == '}')  // jos ollaan loppu merkissä, siirrytään sen yli ja
                                                 // jatketaan seuraavalle kierrokselle
      {
        block->fEndingBracketFound = true;
        ++itsCheckOutTextStartPosition;
        break;  // lopetetaan blokki vektorin luku tähän kun loppu merkki tuli vastaan
      }
      block = boost::shared_ptr<NFmiSmartToolCalculationBlockInfo>(
          new NFmiSmartToolCalculationBlockInfo());
    }
  }
  catch (...)
  {
    throw;
  }
  return !theBlockVector->Empty();
}

// Jos on joku ehdollinen lauseke (if, elseif, else), sitä pitää seurata blokki, joka alkaa ja
// loppuu kaarisulkuihin IF(x > y) { ... } Tämä funktio tekee tarkastelut ja heittää poikkeuksia
// selvennyksineen, jos ehdot ei toteudu.
static void DoConditionalBlockBracketChecks(
    std::string conditionalName,
    boost::shared_ptr<NFmiAreaMaskSectionInfo> &conditionalAreaMaskSectionInfo,
    boost::shared_ptr<NFmiSmartToolCalculationBlockInfoVector> &conditionalCalculationBlockInfos,
    bool elseCase)
{
  if (!((conditionalAreaMaskSectionInfo || elseCase) && conditionalCalculationBlockInfos))
    throw std::runtime_error(
        std::string(
            "Unknown logical error (?) in smarttool intepreter when doing conditional clause block "
            "bracket checks with ") +
        conditionalName);
  else
  {
    if (!conditionalCalculationBlockInfos->BlockWasEnclosedInBrackets())
    {
      std::string errorMessage = conditionalName;
      errorMessage += " \"";
      errorMessage += elseCase ? "ELSE" : conditionalAreaMaskSectionInfo->GetCalculationText();
      errorMessage += "\" didn't have correctly the block markers (bracers '{' and '}') like:\n";
      errorMessage += elseCase ? "ELSE" : "IF(x > y)";
      errorMessage += "\n{ T = T + 1 }";
      throw std::runtime_error(errorMessage);
    }
  }
}

// paluu arvo tarkoittaa, jatketaanko tekstin läpikäymistä vielä, vai ollaanko tultu jo loppuun.
bool NFmiSmartToolIntepreter::CheckoutPossibleNextCalculationBlock(
    NFmiSmartToolCalculationBlockInfo &theBlock, bool fFirstLevelCheckout, int theBlockIndex)
{
  bool fWasBlockMarksFound = false;
  CheckoutPossibleNextCalculationSection(theBlock.itsFirstCalculationSectionInfo,
                                         fWasBlockMarksFound);
  theBlock.fStartingBracketFound = fWasBlockMarksFound;
  if (fFirstLevelCheckout || (fWasBlockMarksFound && theBlockIndex == 0) ||
      theBlockIndex > 0)  // vain 1. tason kyselyssä jatketaan tai jos blokki merkit löytyivät {}
  // eli IF()-lauseen jälkeen pitää olla {}-blokki muuten ei oteta kuin 1. calc-sektio kun ollaan
  // extraktoimassa if, else if tai else -blokkia
  // tai jos useita blokkeja if-lauseen sisällä, jatketaan myös
  {
    if (CheckoutPossibleIfClauseSection(theBlock.itsIfAreaMaskSectionInfo))
    {
      // blokit voidaan luoda  vasta täällä eikä konstruktorissa, koska muuten konstruktori joutuisi
      // iki-looppiin
      theBlock.itsIfCalculationBlockInfos =
          boost::shared_ptr<NFmiSmartToolCalculationBlockInfoVector>(
              new NFmiSmartToolCalculationBlockInfoVector());
      CheckoutPossibleNextCalculationBlockVector(theBlock.itsIfCalculationBlockInfos);
      ::DoConditionalBlockBracketChecks("IF clause",
                                        theBlock.itsIfAreaMaskSectionInfo,
                                        theBlock.itsIfCalculationBlockInfos,
                                        false);
      if (CheckoutPossibleElseIfClauseSection(theBlock.itsElseIfAreaMaskSectionInfo))
      {
        theBlock.itsElseIfCalculationBlockInfos =
            boost::shared_ptr<NFmiSmartToolCalculationBlockInfoVector>(
                new NFmiSmartToolCalculationBlockInfoVector());
        CheckoutPossibleNextCalculationBlockVector(theBlock.itsElseIfCalculationBlockInfos);
        ::DoConditionalBlockBracketChecks("ELSEIF clause",
                                          theBlock.itsElseIfAreaMaskSectionInfo,
                                          theBlock.itsElseIfCalculationBlockInfos,
                                          false);
      }
      if ((theBlock.fElseSectionExist = CheckoutPossibleElseClauseSection()) == true)
      {
        theBlock.itsElseCalculationBlockInfos =
            boost::shared_ptr<NFmiSmartToolCalculationBlockInfoVector>(
                new NFmiSmartToolCalculationBlockInfoVector());
        CheckoutPossibleNextCalculationBlockVector(theBlock.itsElseCalculationBlockInfos);
        // Else tapauksessa annetaan vain joku AreaMaskSectionInfo (eli tässä elseif versio siitä)
        ::DoConditionalBlockBracketChecks("ELSE clause",
                                          theBlock.itsElseIfAreaMaskSectionInfo,
                                          theBlock.itsElseCalculationBlockInfos,
                                          true);
      }
    }
    if (!fWasBlockMarksFound)  // jos 1. checkoutiss ei törmätty blokin alkumerkkiin '{' voidaan
                               // kokeilla löytyykö tästä lasku-sektiota
      CheckoutPossibleNextCalculationSection(theBlock.itsLastCalculationSectionInfo,
                                             fWasBlockMarksFound);
  }
  if (itsCheckOutTextStartPosition == itsStrippedMacroText.end())
    return false;
  return true;
}

void NFmiSmartToolIntepreter::InitCheckOut()
{
  itsCheckOutSectionText = "";
  itsCheckOutTextStartPosition = itsStrippedMacroText.begin();
  itsCheckOutTextEndPosition = itsStrippedMacroText.begin();  // tällä ei vielä väliä
  fContinueCurrentSectionCheckOut = true;
}

static std::string::iterator EatWhiteSpaces(std::string::iterator &it,
                                            const std::string::const_iterator &endIter)
{
  if (it == endIter)
    return it;

  while (std::isspace(*it))
  {
    ++it;
    if (it == endIter)
      break;
  };
  return it;
}

// Irroitetaan mahdollisia laskuoperaatio rivejä tulkkausta varten.
// laskun pitää olla yhdellä rivillä ja ne ovat muotoa:
// T = T + 1
// Eli pitää olla joku sijoitus johonkin parametriin.
// Palauttaa true jos löytyi jotai lasku-operaatioita.
// Päivittää myös luokan sisäisiä iteraattoreita macro-tekstiin.
bool NFmiSmartToolIntepreter::ExtractPossibleNextCalculationSection(bool &fWasBlockMarksFound)
{
  fWasBlockMarksFound = false;
  itsCheckOutSectionText = "";
  string nextLine;
  std::string::iterator eolPos = itsCheckOutTextStartPosition;
  eolPos = EatWhiteSpaces(eolPos, itsStrippedMacroText.end());
  itsCheckOutTextStartPosition = itsCheckOutTextEndPosition = eolPos;
  if (eolPos != itsStrippedMacroText.end() &&
      *eolPos == '{')  // jos blokin alkumerkki löytyi, haetaan sen loppua
  {
    eolPos = std::find(itsCheckOutTextStartPosition, itsStrippedMacroText.end(), '}');
    if (eolPos == itsStrippedMacroText.end())
      throw runtime_error(::GetDictionaryString("SmartToolErrorEndOfBlockNotFound"));
    else
    {
      fWasBlockMarksFound = true;
      ++itsCheckOutTextStartPosition;  // hypätään alkumerkin ohi
      eolPos = itsCheckOutTextStartPosition;
    }
  }

  {
    do
    {
      eolPos = EatWhiteSpaces(eolPos, itsStrippedMacroText.end());
      // en ole varma vielä näistä iteraattoreista, mitkä ovat tarpeellisisa ja mitkä turhia.
      itsCheckOutTextStartPosition = itsCheckOutTextEndPosition = eolPos;
      itsCheckOutSectionText += nextLine;
      if (eolPos != itsStrippedMacroText.end() &&
          (*eolPos == '{' || *eolPos == '}'))  // jos seuraavan blokin alkumerkki tai loppumerkki
                                               // löytyi, lopetetaan tämä blokki tähän
      {
        if (*eolPos == '{')  // jos löytyy alkumerkki, ilmoitetaan siitä ulos täältä
          fWasBlockMarksFound = true;
        if (itsCheckOutSectionText.empty())
          return false;
        else
          return true;
      }

      eolPos = std::find_if(
          itsCheckOutTextStartPosition, itsStrippedMacroText.end(), EndOfLineSearcher());

      nextLine = string(itsCheckOutTextStartPosition, eolPos);
      nextLine += '\n';
      if (eolPos != itsStrippedMacroText.end() && (*eolPos == '\n' || *eolPos == '\r'))
        ++eolPos;
    } while (IsPossibleCalculationLine(nextLine));
  }
  if (itsCheckOutSectionText.empty())
    return false;
  return true;
}

// aluksi yksinkertainen laskurivi tarkistus:
// 1. Ei saa olla ehto-lause sanoja (IF,ELSEIF jne.).
// 2. Pitää olla sijoitus-operaatio eli '='
bool NFmiSmartToolIntepreter::IsPossibleCalculationLine(const std::string &theTextLine)
{
  if (StartsWithAnyWholeWord(theTextLine, itsTokenConditionalCommands))
    return false;
  if (theTextLine.find(string("=")) != string::npos)
    return true;

  if (std::find_if(theTextLine.begin(), theTextLine.end(), std::not1(std::ptr_fun(::isspace))) !=
      theTextLine.end())
  {
    // Riviltä löytyi sanoja ja niiden välissä space, ehtolauseet on jo tarkastettu edellä,
    // vaikuttaa siltä että rivin lausekkeesta siis puuttuu '=' -operaattori.
    throw runtime_error(
        ::GetDictionaryString("Calculation line seems to be missing the '=' operator") + ": \n" +
        theTextLine);
  }
  return false;
}

// Yksinkertainen IF(mask) -rivin tarkistus:
// 1. aluksi pitää olla IF
// 2. ei saa olla ELSE/ELSEIF sanoja
// 3. Pitää olla ensin '('- ja sitten ')' -merkit
bool NFmiSmartToolIntepreter::IsPossibleIfConditionLine(const std::string &theTextLine)
{
  if (!FindAnyFromText(theTextLine, itsTokenIfCommands))
    return false;
  if (FindAnyFromText(theTextLine, itsTokenElseIfCommands))
    return false;
  if (FindAnyFromText(theTextLine, itsTokenElseCommands))
    return false;
  if ((theTextLine.find(string("(")) != string::npos) &&
      (theTextLine.find(string(")")) != string::npos))
    return true;
  return false;
}

// Yksinkertainen ELSEIF(mask) -rivin tarkistus:
// 1. aluksi pitää olla ELSEIF
// 2. ei saa olla IF/ELSE sanoja
// 3. Pitää olla ensin '('- ja sitten ')' -merkit
bool NFmiSmartToolIntepreter::IsPossibleElseIfConditionLine(const std::string &theTextLine)
{
  if (!FindAnyFromText(theTextLine, itsTokenElseIfCommands))
    return false;
  if ((theTextLine.find(string("(")) != string::npos) &&
      (theTextLine.find(string(")")) != string::npos))
    return true;
  return false;
}

// Yksinkertainen ELSE -rivin tarkistus:
// 1. saa olla vain else-sana
bool NFmiSmartToolIntepreter::IsPossibleElseConditionLine(const std::string &theTextLine)
{
  stringstream sstream(theTextLine);
  string tmp;
  sstream >> tmp;
  if (!FindAnyFromText(tmp, itsTokenElseCommands))
    return false;
  tmp = "";  // nollataan tämä, koska MSVC++7.1 ei sijoita jostain syystä mitään kun ollaan tultu
             // loppuun (muilla kääntäjillä on sijoitettu tyhjä tmp-stringiin)
  sstream >> tmp;
  if (tmp.empty())
    return true;
  else
    throw runtime_error(::GetDictionaryString("SmartToolErrorIllegalElseLine") + ": \n" +
                        theTextLine);
}

static bool IsWordContinuing(char ch)
{
  if (isalnum(ch) || ch == '_')
    return true;
  return false;
}

// Pitää olla kokonainen sana eli juuri ennen sanaa ei saa olla kirjaimia,numeroita tai _-merkkiä,
// eikä heti sen jälkeenkään.
bool NFmiSmartToolIntepreter::FindAnyFromText(const std::string &theText,
                                              const std::vector<std::string> &theSearchedItems)
{
  int size = static_cast<int>(theSearchedItems.size());
  for (int i = 0; i < size; i++)
  {
    string::size_type pos = string::npos;
    if ((pos = theText.find(theSearchedItems[i])) != string::npos)
    {
      if (pos > 0)
      {
        char ch1 = theText[pos - 1];
        if (IsWordContinuing(ch1))
          continue;
      }
      if (pos + theSearchedItems[i].size() < theText.size())
      {
        char ch2 = theText[pos + theSearchedItems[i].size()];
        if (IsWordContinuing(ch2))
          continue;
      }
      return true;
    }
  }
  return false;
}

// Jos annettu teksti alkaa jollain kokonaisella theSearchedWords sanalla, palauta
// true, muuten false. Vertailut tehdään case-insensitiivisesti.
bool NFmiSmartToolIntepreter::StartsWithAnyWholeWord(
    const std::string &theText, const std::vector<std::string> &theSearchedWords)
{
  std::stringstream out(theText);
  // Otetaan stringista ensimmaiset yhtenaiset merkit, ilman alun whitespaceja.
  std::string firstLumpOfCharacters;
  out >> firstLumpOfCharacters;
  for (const auto &checkedWord : theSearchedWords)
  {
    if (boost::algorithm::istarts_with(firstLumpOfCharacters, checkedWord))
    {
      if (checkedWord.size() == firstLumpOfCharacters.size())
      {
        return true;
      }
      else
      {
        if (!::IsWordContinuing(firstLumpOfCharacters[checkedWord.size()]))
        {
          return true;
        }
      }
    }
  }
  return false;
}

// palauttaa true, jos if-lause löytyi
bool NFmiSmartToolIntepreter::CheckoutPossibleIfClauseSection(
    boost::shared_ptr<NFmiAreaMaskSectionInfo> &theAreaMaskSectionInfo)
{
  if (ExtractPossibleIfClauseSection())
    return InterpretMaskSection(itsCheckOutSectionText, theAreaMaskSectionInfo);
  return false;
}

// IF-lause koostuu sanasta IF ja suluista, jotka sulkevat vähintään yhden mahd. useita ehtoja
// joita yhdistetään erilaisilla binääri operaatioilla. Koko jutun pitää olla yhdellä rivillä.
// Esim:
// IF(T<1)
// IF(T<1 && P>1012)
// IF(T<1 && P>1012 || RH>=95)
bool NFmiSmartToolIntepreter::ExtractPossibleIfClauseSection()
{
  itsCheckOutSectionText = "";
  std::string::iterator eolPos = itsCheckOutTextStartPosition;
  eolPos = EatWhiteSpaces(eolPos, itsStrippedMacroText.end());
  // en ole varma vielä näistä iteraattoreista, mitkä ovat tarpeellisisa ja mitkä turhia.
  itsCheckOutTextStartPosition = itsCheckOutTextEndPosition = eolPos;

  // HUOM!!! NT4 ja W2000 ero:
  // Dialogista tulevasta stringissä rivin vaihto on W2000:ssa 13 (=CR) ja NT4:ssa 10 (=LF)
  // Pitää Testata molemmat tapaukset!!!!!!
  eolPos = std::find_if(itsCheckOutTextStartPosition,
                        itsStrippedMacroText.end(),
                        EndOfLineSearcher());  // W2000:ssa etsitään CR:ää
  string nextLine = string(itsCheckOutTextStartPosition, eolPos);
  if (IsPossibleIfConditionLine(nextLine))
  {
    itsCheckOutSectionText += nextLine;
    itsCheckOutTextStartPosition = eolPos;  // asetetaan positio seuraavia tarkasteluja varten
    return true;
  }
  return false;
}

// En ymmärrä miten tämän saa toimimaan, miten saa käytettyä mem_fun-adapteria.
// VC:ssa näyttää olevan taas oma systeemi ja nyt mem_fun ei toimi
// standardin mukaisesti. En saa kutsua toimiaan.
template <typename memfunction>
bool NFmiSmartToolIntepreter::ExtractPossibleConditionalClauseSection(
    memfunction conditionalChecker)
{
  itsCheckOutSectionText = "";
  std::string::iterator eolPos = itsCheckOutTextStartPosition;
  eolPos = EatWhiteSpaces(eolPos, itsStrippedMacroText.end());
  // en ole varma vielä näistä iteraattoreista, mitkä ovat tarpeellisisa ja mitkä turhia.
  itsCheckOutTextStartPosition = itsCheckOutTextEndPosition = eolPos;

  // HUOM!!! NT4 ja W2000 ero:
  // Dialogista tulevasta stringissä rivin vaihto on W2000:ssa 13 (=CR) ja NT4:ssa 10 (=LF)
  // Pitää Testata molemmat tapaukset!!!!!!
  eolPos = std::find_if(itsCheckOutTextStartPosition,
                        itsStrippedMacroText.end(),
                        EndOfLineSearcher());  // W2000:ssa etsitään CR:ää
  string nextLine = string(itsCheckOutTextStartPosition, eolPos);
  if (conditionalChecker(nextLine))
  {
    itsCheckOutSectionText += nextLine;
    itsCheckOutTextStartPosition = eolPos;  // asetetaan positio seuraavia tarkasteluja varten
    return true;
  }
  return false;
}

// En saanut toimimaan mem_fun adapteri-systeemiä, joten joudun kopioimaan koodia
// ExtractPossibleIfClauseSection-metodista. Olisin halunnut käyttää yleistä
// funktiota, jolle annetaan parametrina yksi erottava metodi.
// palauttaa true, jos ifelse-lause löytyi
bool NFmiSmartToolIntepreter::CheckoutPossibleElseIfClauseSection(
    boost::shared_ptr<NFmiAreaMaskSectionInfo> &theAreaMaskSectionInfo)
{
  if (ExtractPossibleElseIfClauseSection())
    return InterpretMaskSection(itsCheckOutSectionText, theAreaMaskSectionInfo);
  return false;
}

bool NFmiSmartToolIntepreter::ExtractPossibleElseIfClauseSection()
{
  itsCheckOutSectionText = "";
  std::string::iterator eolPos = itsCheckOutTextStartPosition;
  eolPos = EatWhiteSpaces(eolPos, itsStrippedMacroText.end());
  // en ole varma vielä näistä iteraattoreista, mitkä ovat tarpeellisisa ja mitkä turhia.
  itsCheckOutTextStartPosition = itsCheckOutTextEndPosition = eolPos;

  // HUOM!!! NT4 ja W2000 ero:
  // Dialogista tulevasta stringissä rivin vaihto on W2000:ssa 13 (=CR) ja NT4:ssa 10 (=LF)
  // Pitää Testata molemmat tapaukset!!!!!!
  eolPos = std::find_if(itsCheckOutTextStartPosition,
                        itsStrippedMacroText.end(),
                        EndOfLineSearcher());  // W2000:ssa etsitään CR:ää
  string nextLine = string(itsCheckOutTextStartPosition, eolPos);
  if (IsPossibleElseIfConditionLine(nextLine))
  {
    itsCheckOutSectionText += nextLine;
    itsCheckOutTextStartPosition = eolPos;  // asetetaan positio seuraavia tarkasteluja varten
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::CheckoutPossibleElseClauseSection()
{
  itsCheckOutSectionText = "";
  std::string::iterator eolPos = itsCheckOutTextStartPosition;
  eolPos = EatWhiteSpaces(eolPos, itsStrippedMacroText.end());
  // en ole varma vielä näistä iteraattoreista, mitkä ovat tarpeellisisa ja mitkä turhia.
  itsCheckOutTextStartPosition = itsCheckOutTextEndPosition = eolPos;

  // HUOM!!! NT4 ja W2000 ero:
  // Dialogista tulevasta stringissä rivin vaihto on W2000:ssa 13 (=CR) ja NT4:ssa 10 (=LF)
  // Pitää Testata molemmat tapaukset!!!!!!
  eolPos = std::find_if(itsCheckOutTextStartPosition,
                        itsStrippedMacroText.end(),
                        EndOfLineSearcher());  // W2000:ssa etsitään CR:ää
  string nextLine = string(itsCheckOutTextStartPosition, eolPos);
  if (IsPossibleElseConditionLine(nextLine))
  {
    itsCheckOutSectionText += nextLine;
    itsCheckOutTextStartPosition = eolPos;  // asetetaan positio seuraavia tarkasteluja varten
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::CheckoutPossibleNextCalculationSection(
    boost::shared_ptr<NFmiSmartToolCalculationSectionInfo> &theSectionInfo,
    bool &fWasBlockMarksFound)
{
  if (ExtractPossibleNextCalculationSection(fWasBlockMarksFound))
    return InterpretCalculationSection(itsCheckOutSectionText, theSectionInfo);
  return false;
}

void NFmiSmartToolIntepreter::SetMacroTexts(const std::string &theMacroText)
{
  itsMacroText = theMacroText;
  NFmiPreProcessor commentStripper;
  commentStripper.SetIncluding("#include", itsIncludeDirectory);
  commentStripper.SetString(theMacroText);
  if (commentStripper.Strip())
  {
    itsStrippedMacroText = commentStripper.GetString();
  }
  else
  {
    throw runtime_error(::GetDictionaryString("SmartToolErrorCommentRemovalFailed") + ":\n" +
                        commentStripper.GetMessage());
  }
}
//--------------------------------------------------------
// InterpretMaskSection
//--------------------------------------------------------
// Koko Section on yhdellä rivillä!!!
// Esim:
// IF(T<1)
// IF(T<1 && P>1012)
// IF(T<1 && P>1012 || RH>=95)
bool NFmiSmartToolIntepreter::InterpretMaskSection(
    const std::string &theMaskSectorText,
    boost::shared_ptr<NFmiAreaMaskSectionInfo> &theAreaMaskSectionInfo)
{
  theAreaMaskSectionInfo->SetCalculationText(theMaskSectorText);
  string maskText(theMaskSectorText);
  exp_ptr = maskText.begin();
  exp_end = maskText.end();

  string tmp;
  if (GetToken())  // luetaan komento tähän esim. if, else jne
  {
    tmp = token;
    if (FindAnyFromText(tmp, itsTokenConditionalCommands))
    {
      if (GetToken())  // luetaan komento tähän esim. if, else jne
      {
        tmp = token;
        if (tmp == "(")  // pitää löytyä alku sulku
        {
          string::size_type startPos = exp_ptr - maskText.begin();
          string::size_type endPos = maskText.rfind(string(")"));
          string::iterator it1 = maskText.begin() + startPos;
          string::iterator it2 = maskText.begin() + endPos;
          if (endPos != string::npos)  // pitää löytyä lopetus sulku
          {
            string finalText(it1, it2);
            return InterpretMasks(finalText, theAreaMaskSectionInfo);
          }
        }
      }
    }
  }
  throw runtime_error(::GetDictionaryString("SmartToolErrorIllegalConditional") + ":\n" + maskText);
}

// tässä on enää ehtolauseen sulkujen sisältävä oleva teksti esim.
// T<1
// T<1 && P>1012
// T<1 && P>1012 || RH>=95
// HUOM!!!! Tämä vuotaa exceptionin yhteydessä, pino ei tuhoa AreaMaskInfoja!!!!! (korjaa)
bool NFmiSmartToolIntepreter::InterpretMasks(
    std::string &theMaskSectionText,
    boost::shared_ptr<NFmiAreaMaskSectionInfo> &theAreaMaskSectionInfo)
{
  string maskText(theMaskSectionText);
  exp_ptr = maskText.begin();
  exp_end = maskText.end();

  string tmp;
  for (; GetToken();)
  {
    tmp = token;  // luetaan muuttuja/vakio/funktio tai mikä lie
    boost::shared_ptr<NFmiAreaMaskInfo> maskInfo(new NFmiAreaMaskInfo(theMaskSectionText));
    InterpretToken(tmp, maskInfo);
    theAreaMaskSectionInfo->Add(maskInfo);
  }

  // minimissään erilaisia lasku elementtejä pitää olla vahintäin 3 (esim. T > 15)
  if (theAreaMaskSectionInfo->GetAreaMaskInfoVector().size() >= 3)
    return true;
  throw runtime_error(::GetDictionaryString("SmartToolErrorConditionalWasNotComplete") + ":\n" +
                      theMaskSectionText);
}

// Vain tyhjä luokka erikoispoikkeusta varten.
// Kun luetaan Extra info rivejä, otetaan niiden tiedot NFmiExtraMacroParamData
// luokan olioon talteen ja siirrytään seuraavaan riviin ilman normaaleja toimintoja.
class ExtraInfoMacroLineException
{
};

//--------------------------------------------------------
// InterpretCalculationSection
//--------------------------------------------------------
// Jokainen rivi tekstissä on mahdollinen laskuoperaatio esim.
// T = T + 1
// Jokaiselta riviltä pitää siis lötyä muuttuja johon sijoitetaan ja jotain laskuja
// palauttaa true, jos löytyi laskuja ja false jos ei.
bool NFmiSmartToolIntepreter::InterpretCalculationSection(
    std::string &theCalculationSectiontext,
    boost::shared_ptr<NFmiSmartToolCalculationSectionInfo> &theSectionInfo)
{
  std::string::iterator pos = theCalculationSectiontext.begin();
  std::string::iterator end = theCalculationSectiontext.end();
  do
  {
    string nextLine = ExtractNextLine(theCalculationSectiontext, pos, &end);
    try
    {
      if (!nextLine.empty() && !ConsistOnlyWhiteSpaces(nextLine))
      {
        boost::shared_ptr<NFmiSmartToolCalculationInfo> calculationInfo =
            InterpretCalculationLine(nextLine);
        if (calculationInfo)
          theSectionInfo->AddCalculationInfo(calculationInfo);
      }
    }
    catch (ExtraInfoMacroLineException &)
    {
    }

    if (end != theCalculationSectiontext.end())  // jos ei tarkistusta, menee yli lopusta
      pos = ++end;
  } while (end != theCalculationSectiontext.end());

  return (!theSectionInfo->GetCalculationInfos().empty());
}

bool NFmiSmartToolIntepreter::ConsistOnlyWhiteSpaces(const std::string &theText)
{
  static const string someSpaces(" \t\r\n");
  if (theText.find_first_not_of(someSpaces) == string::npos)
    return true;
  return false;
}

std::string NFmiSmartToolIntepreter::ExtractNextLine(std::string &theText,
                                                     std::string::iterator theStartPos,
                                                     std::string::iterator *theEndPos)
{
  *theEndPos = std::find_if(theStartPos, theText.end(), EndOfLineSearcher());

  string str(theStartPos, *theEndPos);
  return str;
}

// Rivissä on mahdollinen laskuoperaatio esim.
// T = T + 1
// Riviltä pitää siis lötyä muuttuja johon sijoitetaan ja jotain laskuja
boost::shared_ptr<NFmiSmartToolCalculationInfo> NFmiSmartToolIntepreter::InterpretCalculationLine(
    const std::string &theCalculationLineText)
{
  itsCalculationLineText = theCalculationLineText;
  boost::shared_ptr<NFmiSmartToolCalculationInfo> calculationInfo(
      new NFmiSmartToolCalculationInfo());
  calculationInfo->SetCalculationText(itsCalculationLineText);

  exp_ptr = itsCalculationLineText.begin();
  exp_end = itsCalculationLineText.end();

  string tmp;
  if (GetToken())  // luetaan muuttuja johon sijoitetaan esim. T
  {
    tmp = token;
    auto newVariableType = DoUserVariableChecks(tmp);
    boost::shared_ptr<NFmiAreaMaskInfo> assignedVariable(
        new NFmiAreaMaskInfo(itsCalculationLineText));
    InterpretVariable(
        tmp,
        assignedVariable, newVariableType);

    // Jos rivillä on alustettu named-constant-variable tyyppinen otus, 
    // lopetetaan rivin tulkinta siihen ja palautetaan nullptr, koska
    // kyseistä lauseketta ei varsinaisesti saa käyttää itse laskuissa.
    if (assignedVariable->GetOperationType() == NFmiAreaMask::Constant)
    {
      if (newVariableType == SmarttoolsUserVariableType::None)
      {
        // Jos kyse oli ei uuden const-muuttujan luonnista ja
        // on yritetty sijoittaa arvoa const:iin, pitää tehdä virheilmoitus.
        throw runtime_error(::GetDictionaryString("Can't reassign value to named const variable") + ":\n" +
                            itsCalculationLineText);
      }
      return nullptr;
    }
    NFmiInfoData::Type dType = assignedVariable->GetDataType();
    if (!(dType == NFmiInfoData::kEditable || dType == NFmiInfoData::kScriptVariableData ||
          dType == NFmiInfoData::kAnyData || dType == NFmiInfoData::kMacroParam))
      throw runtime_error(::GetDictionaryString("SmartToolErrorAssignmentError") + ":\n" +
                          itsCalculationLineText);
    calculationInfo->SetResultDataInfo(assignedVariable);

    GetToken();  // luetaan sijoitus operaattori =
    if (string(token) != string("="))
      throw runtime_error(::GetDictionaryString("SmartToolErrorNoAssignmentOperator") + ":\n" +
                          itsCalculationLineText);
    for (; GetToken();)
    {
      tmp = token;  // luetaan muuttuja/vakio/funktio tai mikä lie
      boost::shared_ptr<NFmiAreaMaskInfo> variableInfo(
          new NFmiAreaMaskInfo(itsCalculationLineText));
      InterpretToken(tmp, variableInfo);
      AddVariableToCalculation(calculationInfo, variableInfo);
    }

    CheckMustHaveSimpleConditionFunctions(calculationInfo);
    if (calculationInfo->GetCalculationOperandInfoVector().empty())
      return boost::shared_ptr<NFmiSmartToolCalculationInfo>();
  }
  calculationInfo->CheckIfAllowMissingValueAssignment();
  if (calculationInfo->GetResultDataInfo()->GetDataType() == NFmiInfoData::kMacroParam)
    fMacroParamFound = true;
  else if (calculationInfo->GetResultDataInfo()->GetDataType() !=
           NFmiInfoData::kScriptVariableData)  // skripti muuttuja voi olla kummassakin tapauksessa
                                               // mukana
  {
    fNormalAssigmentFound = true;
    if (fMacroParamSkriptInProgress)
      throw runtime_error(::GetDictionaryString("SmartToolErrorMacroParamAssignmentError1") + "\n" +
                          ::GetDictionaryString("SmartToolErrorMacroParamAssignmentError2") + "\n" +
                          ::GetDictionaryString("SmartToolErrorThatWontWorkEnding"));
  }
  // tarkistetaan saman tien, onko sijoituksia tehty molempiin tyyppeihin ja heitetään poikkeus jos
  // on
  if (fMacroParamFound && fNormalAssigmentFound)
    throw runtime_error(::GetDictionaryString("SmartToolErrorMacroParamAssignmentError3") + "\n" +
                        ::GetDictionaryString("SmartToolErrorMacroParamAssignmentError4") + "\n" +
                        ::GetDictionaryString("SmartToolErrorThatWontWorkEnding"));

  return calculationInfo;
}

SmarttoolsUserVariableType NFmiSmartToolIntepreter::DoUserVariableChecks(
    std::string &variableNameInOut)
{
  if (boost::iequals(variableNameInOut, "var"))
  {
    // Ollaan alustamassa uutta skripti muuttujaa, luetaan nimi talteen
    GetToken();  
    variableNameInOut = token;
    return SmarttoolsUserVariableType::Var;
  }
  else if (boost::iequals(variableNameInOut, "const"))
  {
    // Ollaan alustamassa uutta skripti vakiomuuttujaa, luetaan nimi talteen
    GetToken();
    variableNameInOut = token;
    return SmarttoolsUserVariableType::Const;
  }

  return SmarttoolsUserVariableType::None;
}

void NFmiSmartToolIntepreter::CheckMustHaveSimpleConditionFunctions(
    boost::shared_ptr<NFmiSmartToolCalculationInfo> &theCalculationInfo)
{
  auto &areaMaskVector = theCalculationInfo->GetCalculationOperandInfoVector();
  for (const auto &areaMask : areaMaskVector)
  {
    if (areaMask->SimpleConditionRule() == NFmiAreaMask::SimpleConditionRule::MustHave)
    {
      if (!areaMask->SimpleConditionInfo())
      {
        std::string errorString =
            "Function, that must have simple-condition (e.g. \"T_ec > 0\") as last parameter, was "
            "missing from line:\n";
        errorString += areaMask->GetOrigLineText();
        throw std::runtime_error(errorString);
      }
    }
  }
}

void NFmiSmartToolIntepreter::AddVariableToCalculation(
    boost::shared_ptr<NFmiSmartToolCalculationInfo> &theCalculationInfo,
    boost::shared_ptr<NFmiAreaMaskInfo> &theVariableInfo)
{
  if (theVariableInfo->GetOperationType() == NFmiAreaMask::SimpleConditionCalculation)
    AddSimpleCalculationToCallingAreaMask(theCalculationInfo, theVariableInfo);
  else
    theCalculationInfo->AddCalculationInfo(theVariableInfo);
}

// Oletus: theSimpleCalculationAreaMask:in OperationType on jo todettu
// NFmiAreaMask::SimpleConditionCalculation:iksi.
void NFmiSmartToolIntepreter::AddSimpleCalculationToCallingAreaMask(
    boost::shared_ptr<NFmiSmartToolCalculationInfo> &theCalculationInfo,
    const boost::shared_ptr<NFmiAreaMaskInfo> &theSimpleCalculationAreaMask)
{
  auto &calculationOperandVector = theCalculationInfo->GetCalculationOperandInfoVector();
  // 1. Etsi se areaMask calculationOperandVector:ista (lopusta alkua kohden), johon annettu
  // theSimpleCalculationAreaMask liittyy ja lisää se siihen. theSimpleCalculationAreaMask:ia ei
  // siis liitetä normaaliin theCalculationInfo:n laskulistaan.
  auto areaMaskWithSimpleConditionIter = std::find_if(
#ifndef UNIX
      calculationOperandVector.rbegin(),
                   calculationOperandVector.rend(),
      [](const auto &areaMask)
      {
#else
      calculationOperandVector.rbegin(),
      calculationOperandVector.rend(),
      [](const boost::shared_ptr<NFmiAreaMaskInfo> &areaMask)
      {
#endif
        return areaMask->AllowSimpleCondition();
      });
  if (areaMaskWithSimpleConditionIter != calculationOperandVector.rend())
  {
    auto info =
        theSimpleCalculationAreaMask->SimpleConditionInfo();  // must be l-value for next call
    (*areaMaskWithSimpleConditionIter)->SimpleConditionInfo(info);
  }
  else
  {
    // 2. Jos em. areaMaskia ei löydy, heitä poikkeus, joka keroo virheestä.
    std::string errorString =
        "Simple-condition (e.g. \"x > y\") was found, but there were no function that was related "
        "to it, error with the line:\n";
    errorString += theSimpleCalculationAreaMask->GetOrigLineText();
    throw std::runtime_error(errorString);
  }

  // 3. Lisäksi poista theCalculationInfo:n viimeinen operaatio jos se on pilkku
  // (NFmiAreaMask::CommaOperator), koska se erotteli tätä SimpleConditionCalculation:ion otusta.
  auto commaAreaMaskIter = calculationOperandVector.crbegin();
  if (commaAreaMaskIter != calculationOperandVector.rend() &&
      (*commaAreaMaskIter)->GetOperationType() == NFmiAreaMask::CommaOperator)
  {
    // Can't erase element with reverse-iterators, pop_back removes the last element from vector
    calculationOperandVector.pop_back();
  }
  else
  {
    // Jos calculationOperandVector:in viimeinen areaMask ei ole CommaOperator, heitä virheestä
    // kertova poikkeus
    std::string errorString =
        "Simple-condition (e.g. \"x > y\") was found, but the last operand before it was not the "
        "comma separator ','  error in line:\n";
    errorString += theSimpleCalculationAreaMask->GetOrigLineText();
    throw std::runtime_error(errorString);
  }
}

// GetToken ja IsDelim otettu H. Schilbertin  C++: the Complete Refeference third ed.
// jouduin muuttamaan niitä tähän ympäristöön.
// Obtain the next token. Palauttaa true, jos sellainen saatiin, jos ollaan lopussa, palauttaa
// false.
bool NFmiSmartToolIntepreter::GetToken()
{
  char *temp;

  tok_type = NOTYPE;
  temp = token;
  *temp = '\0';

  if (exp_ptr == exp_end)
    return false;  // at end of expression

  while (exp_ptr < exp_end && std::isspace(*exp_ptr))
    ++exp_ptr;  // skip over white space
  if (exp_ptr == exp_end)
    return false;  // at end of expression

  // HUOM! tässä delimiter testissä pitää käyttää ns. base-delimiter listaa.
  if (NFmiSmartToolIntepreter::IsBaseDelimiter(*exp_ptr))
  {
    tok_type = DELIMITER;
    // advance to next char
    *temp++ = *exp_ptr++;
    if (exp_ptr < exp_end)
    {
      if (*exp_ptr == '>' ||
          *exp_ptr ==
              '=')  // tässä halutaan ottaa huomioon >=, <=, !=, <> ja == vertailu operaattorit
      {
        *temp++ = *exp_ptr++;
      }
      if (*exp_ptr == '&')  // tässä halutaan ottaa huomioon && vertailu operaattori
      {
        *temp++ = *exp_ptr++;
      }
      if (*exp_ptr == '|')  // tässä halutaan ottaa huomioon || vertailu operaattori
      {
        *temp++ = *exp_ptr++;
      }
    }
  }
  else if (isalpha(*exp_ptr))
  {
    while (!IsDelim(*exp_ptr))
    {
      *temp++ = *exp_ptr++;
      if (exp_ptr == exp_end)
        break;              // at end of expression
      if (*exp_ptr == '[')  // Ollaan tultu kohtaan missa annetaan malliajo eli esim. T_HIR[-1], nyt
                            // jatketaan kunnes löytyy lopetus merkki eli ']'
      {
        static const std::string modelRunMarkerError =
            "Unable to find model-run end marker ']' from given formula. If you start something "
            "with '[' marker, remember to end it too.";
        SearchUntil(exp_ptr, temp, ']', modelRunMarkerError);
        tok_type = VARIABLE;
        return true;
      }
    }
    tok_type = VARIABLE;
  }
  else if (isdigit(*exp_ptr))
  {
    while (!IsDelim(*exp_ptr))
    {
      *temp++ = *exp_ptr++;
      if (exp_ptr == exp_end)
        break;  // at end of expression
    }
    tok_type = NUMBER;
  }
  else if (*exp_ptr == '"')
  {
    static const std::string stringLiteralError =
        "Unable to find string liter end marker '\"' from given formula. If you start something "
        "with '\"' marker, remember to end it too.";
    SearchUntil(exp_ptr, temp, '"', stringLiteralError);
    tok_type = STRING_LITERAL;
    return true;
  }
  else
  {
    // Tässä on joku outo merkki nyt vastassa, heitetään poikkeus
    throw std::runtime_error(
        std::string("Strange character prevents intepreting following clause: ") +
        std::string(exp_ptr, exp_end));
  }

  *temp = '\0';
  return true;
}

// Tämä on tosi kämäistä pointteri virittely koodia, mikä on vanhaa perua. En uskalla nyt laittaa
// koko smarttool-kielen tulkintaa
// uusiksi, joten joudun virittelemään aika pahasti tätä koodia tässä.
// Nykyään en tekisi tulkki-koodia missään tapauksessä näin.
// theTempCharPtr -osoittaa oikeasti token-dataosan currenttiin char:iin. Se pitää sulkea tässä
// (asettaa 0 viimeiseen kohtaan).
// Lisäksi tämän kutsun jälkeen ei saa lopettaa NFmiSmartToolIntepreter::GetToken -metodia
// normaalisti, koska tuo pointteri
// ei osoitakaan enää sinne minne oltiin edetty funktiossa, vaan sitä aiempaan tilaan. Kokeilin
// antaa pointterin pointterin, mutta
// sitten syntaksi meni jo niin kinkkiseksi että luovutin.
void NFmiSmartToolIntepreter::SearchUntil(std::string::iterator &theExp_ptr,
                                          char *theTempCharPtr,
                                          char theSearchedCh,
                                          const std::string &theErrorStr)
{
  // Pakko siirtää pointereita yksi eteen, jos esim. alku ja loppu merkki ovat samoja ja ollaan
  // vielä alkumerkin kohdalla
  if (*theExp_ptr == theSearchedCh)
    *theTempCharPtr++ = *theExp_ptr++;

  while (theExp_ptr != exp_end && *theExp_ptr != theSearchedCh)
  {
    *theTempCharPtr++ = *theExp_ptr++;
  }

  if (theExp_ptr != exp_end && *theExp_ptr == theSearchedCh)
  {
    *theTempCharPtr++ = *theExp_ptr++;
    *theTempCharPtr = 0;
  }
  else
    throw runtime_error(::GetDictionaryString(theErrorStr.c_str()));
}

// Return true if c is a delimiter.
bool NFmiSmartToolIntepreter::IsDelim(char c)
{  // HUOM! tässä delimiter testissä pitää käyttää kaikkia erotin merkkejä
  if (NFmiSmartToolIntepreter::IsDelimiter(c))
    return true;
  else
    return false;
}

bool NFmiSmartToolIntepreter::IsBaseDelimiter(char c)
{
  if (strchr(NFmiSmartToolIntepreter::itsBaseDelimiterChars.c_str(), c))
    return true;
  else
    return false;
}

bool NFmiSmartToolIntepreter::IsDelimiter(char c)
{
  if (strchr(NFmiSmartToolIntepreter::itsFullDelimiterChars.c_str(), c))
    return true;
  else
    return false;
}

const std::string &NFmiSmartToolIntepreter::GetBaseDelimiterChars()
{
  return NFmiSmartToolIntepreter::itsBaseDelimiterChars;
}

const std::string &NFmiSmartToolIntepreter::GetFullDelimiterChars()
{
  return NFmiSmartToolIntepreter::itsFullDelimiterChars;
}

NFmiAreaMask::CalculationOperator NFmiSmartToolIntepreter::InterpretCalculationOperator(
    const std::string &theOperatorText)
{
  if (theOperatorText == string(""))
    return NFmiAreaMask::NotOperation;
  else if (theOperatorText == "+")
    return NFmiAreaMask::Add;
  else if (theOperatorText == "-")
    return NFmiAreaMask::Sub;
  else if (theOperatorText == "/")
    return NFmiAreaMask::Div;
  else if (theOperatorText == "*")
    return NFmiAreaMask::Mul;
  else if (theOperatorText == "^")
    return NFmiAreaMask::Pow;
  else if (theOperatorText == "%")
    return NFmiAreaMask::Mod;
  else
    throw runtime_error(::GetDictionaryString("SmartToolErrorCalculationOperatorError") + ": " +
                        theOperatorText);
}

void NFmiSmartToolIntepreter::InterpretToken(const std::string &theTokenText,
                                             boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  switch (tok_type)
  {
    case DELIMITER:
      InterpretDelimiter(theTokenText, theMaskInfo);
      break;
    case VARIABLE:
    case NUMBER:
      InterpretVariable(theTokenText, theMaskInfo);
      break;
    case STRING_LITERAL:
      InterpretStringLiteral(theTokenText, theMaskInfo);
      break;
    default:
      throw runtime_error(::GetDictionaryString("SmartToolErrorStrangeWord") + ": " + theTokenText);
  }
}

// HUOM!!!! Muuta käyttämään itsCalculationOperations-mappia!!!!!!!!!!
void NFmiSmartToolIntepreter::InterpretDelimiter(const std::string &theDelimText,
                                                 boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  theMaskInfo->SetMaskText(theDelimText);
  if (theDelimText == string(""))
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::NoType);
    return;
  }

  CalcOperMap::iterator it = itsCalculationOperations.find(theDelimText);
  if (it != itsCalculationOperations.end())
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::Operator);
    theMaskInfo->SetCalculationOperator((*it).second);
    return;
  }

  MaskOperMap::iterator it2 = itsTokenMaskOperations.find(theDelimText);
  if (it2 != itsTokenMaskOperations.end())
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::Comparison);
    NFmiCalculationCondition maskCondition((*it2).second, 0);
    theMaskInfo->SetMaskCondition(maskCondition);
    return;
  }

  BinaOperMap::iterator it3 = itsBinaryOperator.find(theDelimText);
  if (it3 != itsBinaryOperator.end())
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::BinaryOperatorType);
    theMaskInfo->SetBinaryOperator((*it3).second);
    return;
  }

  if (theDelimText == "(")
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::StartParenthesis);
  }
  else if (theDelimText == ")")
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::EndParenthesis);
  }
  else if (theDelimText == ",")
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::CommaOperator);
  }
  else
    throw runtime_error(::GetDictionaryString("SmartToolErrorCalculationOperatorError") + ": " +
                        theDelimText);
}

// Tulkitsee annetun muuttujan. Hoitelee myös tuottajan, jos se on annettu, esim.
// T T_HIR jne.
// Voi olla myös vakio tai funktio systeemi.
void NFmiSmartToolIntepreter::InterpretVariable(const std::string &theVariableText,
                                                boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
                                                SmarttoolsUserVariableType theNewVariableType)
{
  theMaskInfo->SetMaskText(theVariableText);
  string paramNameOnly;
  string levelNameOnly;
  string producerNameOnly;
  bool levelExist = false;
  bool producerExist = false;
  // 1 = viimeisin malliajo data
  // 0 = viimeisin data, mutta niin että jos on tullut hir-pintadatasta 06 utc ajo ja painepinta
  // datasta viimeisin on 00 utc, tällöin painepinta datan 0 viimeisin ajo on tyhjää
  // (explisiittisesti on haluttu 06 viimeisintä malliajoa) -1 - -n on edellisiä malliajoja -1 on
  // siis edellinen ja -2 on viimeksi edellinen jne.
  int modelRunIndex = 1;
  // Datalle voidaan tehdä myös aikasiirtoja
  float timeOffsetInHours = 0;

  // tutkitaan ensin onko mahdollisesti variable-muuttuja, jolloin voimme sallia _-merkin käytön
  // muuttujissa
  if (InterpretPossibleScriptVariable(theVariableText, theMaskInfo, theNewVariableType))
    return;

  NFmiSmartToolIntepreter::CheckVariableString(theVariableText,
                                               paramNameOnly,
                                               levelExist,
                                               levelNameOnly,
                                               producerExist,
                                               producerNameOnly,
                                               modelRunIndex,
                                               timeOffsetInHours);

  bool origWanted = NFmiSmartToolIntepreter::IsProducerOrig(producerNameOnly);

  if (InterpretVariableCheckTokens(theVariableText,
                                   theMaskInfo,
                                   origWanted,
                                   levelExist,
                                   producerExist,
                                   paramNameOnly,
                                   levelNameOnly,
                                   producerNameOnly,
                                   modelRunIndex,
                                   timeOffsetInHours))
  {
    if (theNewVariableType != SmarttoolsUserVariableType::None)
      throw runtime_error(::GetDictionaryString("SmartToolErrorTokenWordUsedAsVariable") + ": " +
                          theVariableText);
    return;
  }

  throw runtime_error(::GetDictionaryString("SmartToolErrorStrangeVariable") + ": " +
                      theVariableText);
}

void NFmiSmartToolIntepreter::InterpretStringLiteral(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  if (!InterpretSimpleCondition(theVariableText, theMaskInfo))
  {
    std::string errorString = ::GetDictionaryString("Unable to decipher given string-literal");
    errorString += ": ";
    errorString += theVariableText;
    throw runtime_error(errorString);
  }
}

// Tyhjät sanat ja space/tabulaattori alkuiset sanat eivät kelpaa
static bool IsGoodSimpleconditionWord(const std::string &word)
{
  if (!word.empty())
  {
    auto firstChar = word[0];
    if (firstChar != ' ' && firstChar != '\t')
      return true;
  }
  return false;
}

static std::vector<std::string> SplitSimpleConditionTextToWordsKeepingDelimiters(
    std::string theSimpleConditionText)
{
  // Following characters are wanted to be as delimiters with simple-conditions: "\t <>=!+-*/^%"
  // But you need to escape '-' and '^' characters in regex so this becomes following:
  std::string delimiterCharactersWithRegexEscapes = "\t <>=!+\\-*/\\^%&|";
  // matches delimiters or consecutive non-delimiters
  regex reg(std::string("([") + delimiterCharactersWithRegexEscapes + "]|[^" +
                 delimiterCharactersWithRegexEscapes + "]+)");
  my_regex_iterator rit(theSimpleConditionText.begin(), theSimpleConditionText.end(), reg);
  my_regex_iterator rend;
  std::vector<std::string> words;
  for (; rit != rend; ++rit)
  {
    auto word = rit->str();
    if (::IsGoodSimpleconditionWord(word))
      words.push_back(rit->str());
  }
  return words;
}

// Have to combine certain words together to make real simple condition words.
// E.g. ">" and "=" has been splitted earlier and now if those two are consecutive words, they have
// to be combined. We are looking to combine following words: >=, <=, !=, ==, <> Presumes that word
// in given basicWords vector can't be empty.
static std::vector<std::string> MakeSimpleConditionBasicWordCombinations(
    const std::vector<std::string> &basicWords)
{
  if (basicWords.size() <= 1)
    return basicWords;

  std::vector<std::string> finalWords;
  bool wordCombinationHappened = false;
  for (size_t wordIndex = 0; wordIndex < basicWords.size() - 1; wordIndex++)
  {
    const auto &firstWord = basicWords[wordIndex];
    const auto &secondWord = basicWords[wordIndex + 1];
    auto firstChar1 = firstWord.at(0);
    auto firstChar2 = secondWord.at(0);
    for (auto &combinationWord : g_SimpleConditionCombinationWords)
    {
      if (firstChar1 == combinationWord[0] && firstChar2 == combinationWord[1])
      {
        // Let's combine consecutive words
        finalWords.push_back(firstWord + secondWord);
        // Must also forward index over the second word
        wordIndex++;
        wordCombinationHappened = true;
        break;
      }
    }
    if (!wordCombinationHappened)
      finalWords.push_back(firstWord);
    wordCombinationHappened = false;
  }
  // Jos kahden viimeisen sana kohdalla ei tapahtunut yhdistelyä, pitää vielä viimeinen sana lisätä
  // lopulliseen listaan
  if (!wordCombinationHappened)
    finalWords.push_back(basicWords.back());

  return finalWords;
}

static bool FindCharacter(const std::string &word, char ch)
{
  auto bracketIter = word.find(ch);
  if (bracketIter != std::string::npos)
    return true;
  else
    return false;
}

// Etsi kaksi eri merkkiä sanasta, ja niiden pitää olla annetussa järjestyksessä eli ch1 ensin ja
// sitten ch2.
static bool FindCharacters(const std::string &word, char ch1, char ch2)
{
  auto position1 = word.find(ch1);
  if (position1 != std::string::npos)
  {
    auto position2 = word.find(ch2, position1 + 1);
    if (position2 != std::string::npos)
    {
      return true;
    }
  }

  return false;
}

// Esim. "T_ec[-3h] > 12" simple-condition jakautuu seuraaviin sanoihin:
// "T_ec[", "-", "3h", "]", ">" ja "12"
// Tässä pitää yhdistää sanoja niin että hakasulkeisiin liittyvät muuttujaan liittyvät sanat
// yhdistetään seuraavasti: "T_ec[-3h]", ">" ja "12"
static std::vector<std::string> CombineBracketVariables(const std::string originalVariableText,
                                                        const std::vector<std::string> &basicWords)
{
  if (basicWords.size() <= 1)
    return basicWords;

  std::vector<std::string> finalWords;
  for (size_t wordIndex = 0; wordIndex < basicWords.size(); wordIndex++)
  {
    const auto &word = basicWords[wordIndex];
    // Hakasulku muuttujan hakasulkumerkit voivat olla mukana yhdessä sanassa, tai koostua useasta
    // erillisestä sanasta
    if (::FindCharacters(word, '[', ']'))
    {
      finalWords.push_back(word);
    }
    else if (!::FindCharacter(word, '['))
    {
      finalWords.push_back(word);
    }
    else
    {
      bool endbracketFound = false;
      auto bracketVariable = word;
      for (size_t bracketWordIndex = wordIndex + 1; bracketWordIndex < basicWords.size();
           bracketWordIndex++)
      {
        const auto &bracketWord = basicWords[bracketWordIndex];
        if (::FindCharacter(bracketWord, '['))
        {
          std::string errorText =
              "Found second consecutive starting bracket '[' from given single-condition: ";
          errorText += originalVariableText;
          throw std::runtime_error(errorText);
        }

        bracketVariable += bracketWord;
        if (::FindCharacter(bracketWord, ']'))
        {
          endbracketFound = true;
          wordIndex = bracketWordIndex;
          break;
        }
      }
      if (endbracketFound)
        finalWords.push_back(bracketVariable);
      else
      {
        std::string errorText = "Unable to find end bracket ']' from given single-condition: ";
        errorText += originalVariableText;
        throw std::runtime_error(errorText);
      }
    }
  }
  return finalWords;
}

static std::vector<std::string> GetSimpleConditionWordsFromVariableText(
    const std::string &theOrigSimpleConditionText)
{
  std::string simpleConditionText = theOrigSimpleConditionText;
  // 1. Trim "-characters from original text's both ends
  boost::trim_left_if(simpleConditionText, boost::is_any_of("\""));
  boost::trim_right_if(simpleConditionText, boost::is_any_of("\""));
  // 2. Split condition to basic words
  std::vector<std::string> basicWords =
      ::SplitSimpleConditionTextToWordsKeepingDelimiters(simpleConditionText);
  basicWords = ::MakeSimpleConditionBasicWordCombinations(basicWords);
  return ::CombineBracketVariables(theOrigSimpleConditionText, basicWords);
}

// Handles following kind of string literals:
// 1. "part1 > part2" , which is the simplest condition
// 2. "part1 < part2 < part3" , which is the range condition
// Parts consist of following:
// 1. One parameter/variable/constant e.g. "T_ec", "x", "5"
// 2. First parameter/variable/constant, calculationOperator, second parameter/variable/constant
//    e.g. "T_ec - x" or "5.2 * P_ec", etc.
//    calculationOperator can be following: +, -, *, /, %, ^
bool NFmiSmartToolIntepreter::InterpretSimpleCondition(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  std::vector<std::string> words = ::GetSimpleConditionWordsFromVariableText(theVariableText);
  if (words.size() >= 3)
    return InterpretSimpleCondition(theVariableText, words, theMaskInfo);
  else
    return false;
}

static FmiMaskOperation GetMaskOperation(const std::string &operandString)
{
  if (operandString == "=" || operandString == "==")
    return kFmiMaskEqual;
  else if (operandString == ">")
    return kFmiMaskGreaterThan;
  else if (operandString == "<")
    return kFmiMaskLessThan;
  else if (operandString == ">=")
    return kFmiMaskGreaterOrEqualThan;
  else if (operandString == "<=")
    return kFmiMaskLessOrEqualThan;
  else if (operandString == "!=" || operandString == "<>")
    return kFmiMaskNotEqual;
  else if (operandString == "->")
    return kFmiMaskContinuousEqual;
  else
    return kFmiNoMaskOperation;
}

static bool CheckSimpleConditionMaskInfo(const boost::shared_ptr<NFmiAreaMaskInfo> &mask)
{
  if (mask)
  {
    auto maskType = mask->GetOperationType();
    if (maskType == NFmiAreaMask::InfoVariable || maskType == NFmiAreaMask::Variable ||
        maskType == NFmiAreaMask::Constant)
    {
      return true;
    }
  }
  return false;
}

// To make range operands to work logically they both must be to same 'direction',
// meaning if operand1 is > or >=, then operand2 must be > or >= (just greater part must be same).
// And if operand1 is < or <= then operand2 must be < or <=.
static bool CheckSimpleConditionRangeOperands(const std::string &theVariableText,
                                              FmiMaskOperation operand1,
                                              FmiMaskOperation operand2)
{
  if (operand1 == kFmiMaskGreaterThan || operand1 == kFmiMaskGreaterOrEqualThan)
  {
    if (operand2 == kFmiMaskGreaterThan || operand2 == kFmiMaskGreaterOrEqualThan)
    {
      return true;
    }
  }
  else if (operand1 == kFmiMaskLessThan || operand1 == kFmiMaskLessOrEqualThan)
  {
    if (operand2 == kFmiMaskLessThan || operand2 == kFmiMaskLessOrEqualThan)
    {
      return true;
    }
  }

  std::string errorstring = ::GetDictionaryString(
      "With simple-condition range case first and second operands must both be </<= or >/>=. In "
      "following string-literal: ");
  errorstring += theVariableText;
  throw std::runtime_error(errorstring);
}

// If currentWord is '-' or '+' and nextWord in words is constant number, you have to combine them
// and advance index.
static std::string GetPossibleSignedConstant(const std::string &currentWord,
                                             const std::vector<std::string> &words,
                                             size_t &startingWordIndexInOut)
{
  if (currentWord.size() == 1 && (currentWord[0] == '-' || currentWord[0] == '+'))
  {
    auto nextWordIndex = startingWordIndexInOut + 1;
    if (nextWordIndex < words.size())
    {
      try
      {
        const auto &nextWord = words[nextWordIndex];
        double testValue = boost::lexical_cast<double>(nextWord);
        startingWordIndexInOut = nextWordIndex;
        return currentWord + nextWord;
      }
      catch (...)
      {
      }
    }
  }

  return currentWord;
}

// Presumes that words constainer contains atleast so many items that the startingWordIndex
// parameter points to.
boost::shared_ptr<NFmiSimpleConditionPartInfo> NFmiSmartToolIntepreter::GetNextSimpleConditionPart(
    const std::string &theVariableText,
    const std::vector<std::string> &words,
    size_t &startingWordIndexInOut)
{
  boost::shared_ptr<NFmiAreaMaskInfo> mask1(new NFmiAreaMaskInfo(theVariableText));
  // 1. word must be param/variable/constant
  auto firstParamWord = words[startingWordIndexInOut];
  firstParamWord = ::GetPossibleSignedConstant(firstParamWord, words, startingWordIndexInOut);
  InterpretVariable(firstParamWord, mask1);
  if (!::CheckSimpleConditionMaskInfo(mask1))
  {
    throw std::runtime_error(
        std::string("Simple condition was illegal so that first part of condition didn't have "
                    "param/variable/constant with word: ") +
        firstParamWord + "\nIn variable text: " + theVariableText);
  }

  boost::shared_ptr<NFmiAreaMaskInfo> mask2;
  NFmiAreaMask::CalculationOperator calculationOperator = NFmiAreaMask::NotOperation;
  startingWordIndexInOut++;
  if (startingWordIndexInOut < words.size())
  {
    const auto &calculationOperatorWord = words[startingWordIndexInOut];
    auto calculationOperatorIter =
        g_SimpleConditionCalculationOperatorMap.find(calculationOperatorWord);
    if (calculationOperatorIter != g_SimpleConditionCalculationOperatorMap.end())
    {
      calculationOperator = calculationOperatorIter->second;
      // Because there were calculation operator, there must be another parameter
      startingWordIndexInOut++;
      if (startingWordIndexInOut < words.size())
      {
        auto secondParamWord = words[startingWordIndexInOut];
        secondParamWord =
            ::GetPossibleSignedConstant(secondParamWord, words, startingWordIndexInOut);
        mask2 = boost::shared_ptr<NFmiAreaMaskInfo>(new NFmiAreaMaskInfo(theVariableText));
        InterpretVariable(secondParamWord, mask2);
        if (!::CheckSimpleConditionMaskInfo(mask2))
        {
          throw std::runtime_error(
              std::string("Simple condition was illegal so that second part of condition didn't "
                          "have param/variable/constant with word: ") +
              secondParamWord + "\nIn variable text: " + theVariableText);
        }
        startingWordIndexInOut++;
      }
      else
      {
        throw std::runtime_error(
            std::string("Simple condition was illegal so that it had parameter and calculation "
                        "operator but second parameter was missing\nIn variable text: " +
                        theVariableText));
      }
    }
  }
  return boost::shared_ptr<NFmiSimpleConditionPartInfo>(
      new NFmiSimpleConditionPartInfo(mask1, calculationOperator, mask2));
}

boost::shared_ptr<NFmiSingleConditionInfo> NFmiSmartToolIntepreter::GetNextSingleCondition(
    const std::string &theVariableText,
    const std::vector<std::string> &words,
    size_t &startingWordIndexInOut)
{
  boost::shared_ptr<NFmiSimpleConditionPartInfo> part1 =
      GetNextSimpleConditionPart(theVariableText, words, startingWordIndexInOut);
  if (!(startingWordIndexInOut < words.size()))
    throw std::runtime_error(
        std::string("Simple condition was illegal so that it had 1. part ok, but no following "
                    "condition operand\nIn variable text: " +
                    theVariableText));

  FmiMaskOperation operand1 = ::GetMaskOperation(words[startingWordIndexInOut++]);
  if (operand1 == kFmiNoMaskOperation)
    throw std::runtime_error(
        std::string("Simple condition was illegal so that 1. condition operand was unknown\nIn "
                    "variable text: " +
                    theVariableText));

  if (!(startingWordIndexInOut < words.size()))
    throw std::runtime_error(
        std::string("Simple condition was illegal so that it had 1. part and it's condition "
                    "operand ok, but no following calculation part\nIn variable text: " +
                    theVariableText));

  boost::shared_ptr<NFmiSimpleConditionPartInfo> part2 =
      GetNextSimpleConditionPart(theVariableText, words, startingWordIndexInOut);
  FmiMaskOperation operand2 = kFmiNoMaskOperation;
  boost::shared_ptr<NFmiSimpleConditionPartInfo> part3;
  if (startingWordIndexInOut < words.size())
  {
    operand2 = ::GetMaskOperation(words[startingWordIndexInOut]);
    if (operand2 != kFmiNoMaskOperation)
    {
      startingWordIndexInOut++;
      if (!(startingWordIndexInOut < words.size()))
        throw std::runtime_error(
            std::string("Simple condition was illegal so that it had 2. part and it's condition "
                        "operand ok, but no following calculation part\nIn variable text: " +
                        theVariableText));
      part3 = GetNextSimpleConditionPart(theVariableText, words, startingWordIndexInOut);

      ::CheckSimpleConditionRangeOperands(theVariableText, operand1, operand2);
    }
  }

  // Make SimpleConditionInfo and give it to theMaskInfo parameter
  boost::shared_ptr<NFmiSingleConditionInfo> singleConditionInfo(
      new NFmiSingleConditionInfo(part1, operand1, part2, operand2, part3));
  return singleConditionInfo;
}

// Premise: there are at least 3 words and they all have been 'cleaned' properly.
bool NFmiSmartToolIntepreter::InterpretSimpleCondition(
    const std::string &theVariableText,
    const std::vector<std::string> &words,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  size_t startingWordIndex = 0;
  boost::shared_ptr<NFmiSingleConditionInfo> singleConditionInfo1 =
      GetNextSingleCondition(theVariableText, words, startingWordIndex);
  NFmiAreaMask::BinaryOperator conditionOperator = NFmiAreaMask::kNoValue;
  boost::shared_ptr<NFmiSingleConditionInfo> singleConditionInfo2;
  if (startingWordIndex < words.size())
  {
    auto binaryOperatorWord = words[startingWordIndex++];
    boost::algorithm::to_lower(binaryOperatorWord);
    auto binaryOperatorIter = g_SimpleConditionBinaryOperatorMap.find(binaryOperatorWord);
    if (binaryOperatorIter == g_SimpleConditionBinaryOperatorMap.end())
      throw std::runtime_error(
          std::string("Simple condition was illegal so that binary operator (between 1. and 2. "
                      "part) was unknown\nIn variable text: " +
                      theVariableText));
    conditionOperator = binaryOperatorIter->second;
    singleConditionInfo2 = GetNextSingleCondition(theVariableText, words, startingWordIndex);

    if (startingWordIndex < words.size())
      throw std::runtime_error(
          std::string("Simple condition was illegal so that after 2. single-condition there were "
                      "more items left in the expression\nIn variable text: " +
                      theVariableText));
  }

  // Make SimpleConditionInfo and give it to theMaskInfo parameter
  boost::shared_ptr<NFmiSimpleConditionInfo> simpleConditionInfo(
      new NFmiSimpleConditionInfo(singleConditionInfo1, conditionOperator, singleConditionInfo2));
  theMaskInfo->SimpleConditionInfo(simpleConditionInfo);
  theMaskInfo->SetOperationType(NFmiAreaMask::SimpleConditionCalculation);

  return true;
}

bool NFmiSmartToolIntepreter::InterpretVariableForChecking(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  theMaskInfo->SetMaskText(theVariableText);
  string paramNameOnly;
  string levelNameOnly;
  string producerNameOnly;
  bool levelExist = false;
  bool producerExist = false;
  int modelRunIndex = 1;
  float timeOffsetInHours = 0;

  try
  {
    NFmiSmartToolIntepreter::CheckVariableString(theVariableText,
                                                 paramNameOnly,
                                                 levelExist,
                                                 levelNameOnly,
                                                 producerExist,
                                                 producerNameOnly,
                                                 modelRunIndex,
                                                 timeOffsetInHours);

    bool origWanted = NFmiSmartToolIntepreter::IsProducerOrig(producerNameOnly);

    if (InterpretVariableOnlyCheck(theVariableText,
                                   theMaskInfo,
                                   origWanted,
                                   levelExist,
                                   producerExist,
                                   paramNameOnly,
                                   levelNameOnly,
                                   producerNameOnly,
                                   modelRunIndex,
                                   timeOffsetInHours))
      return true;
    else
      return false;
  }
  catch (...)
  {
  }

  return false;
}

bool NFmiSmartToolIntepreter::InterpretPossibleScriptConstVariable(
    const std::string &theVariableText,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
    SmarttoolsUserVariableType theNewVariableType)
{
  auto isNewScriptConstVariable = theNewVariableType == SmarttoolsUserVariableType::Const;
  auto it = itsTokenScriptConstVariableNames.find(theVariableText);
  if (it != itsTokenScriptConstVariableNames.end() && isNewScriptConstVariable)
  {
    // const x käytetty uudestaan, esitellään nimetyt vakiomuuttujat vain kerran
    throw runtime_error(
        ::GetDictionaryString("Can't introduce named constant variable more than once") + ": " +
        theVariableText);
  }
  else if (it != itsTokenScriptConstVariableNames.end())
  {
    // Nimettyä vakiomuuttujaa x käytetty uudestaan
    theMaskInfo->SetOperationType(NFmiAreaMask::Constant);
    NFmiCalculationCondition calcCond(kFmiMaskEqual, it->second);
    theMaskInfo->SetMaskCondition(calcCond);
    return true;
  }
  else if (isNewScriptConstVariable)
  {
    // named-const-variable:n alustus pitää tulkata tässä loppuun asti,
    // koska kyseistä lauseketta ei käytetä muuhun kuin nimetyn vakion
    // alustuksessa ja niitä käytetään myöhemmin suoraan eri lausekkeissa.
    GetToken();
    string assignOperator = token;
    if (assignOperator == string("="))
    {
      GetToken();
      string valueStr = token;
      if (valueStr == string("-") || valueStr == string("+"))
      {
        // nagatiivisen tai positiivisen luvun kanssa pitää lukea vielä luvun loppuosa
        GetToken();
        valueStr += token;
      }

      if (IsVariableConstantValue(valueStr, theMaskInfo))
      {
        itsTokenScriptConstVariableNames.insert(ScriptConstVariableMap::value_type(
            theVariableText, theMaskInfo->GetMaskCondition().LowerLimit()));
        if (!GetToken())
        {
          // Sijoituksen jälkeen ei rivillä saisi olla mitään muuta, siksi vielä testattiin
          // GetToken:ia
          return true;
        }
      }
    }

    // const x = ? ei voitu alustaa nimetyn vakion arvoa
    throw runtime_error(
        ::GetDictionaryString("Can't initialize named constant variable's value with '") +
        itsCalculationLineText + "'");
  }
  return false;
}

bool NFmiSmartToolIntepreter::InterpretPossibleScriptVariable(
    const std::string &theVariableText,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
    SmarttoolsUserVariableType theNewVariableType)
{
  if (InterpretPossibleScriptConstVariable(theVariableText,
                                           theMaskInfo,
                                           theNewVariableType))
  {
    return true;
  }

  auto isNewScriptVariable = theNewVariableType == SmarttoolsUserVariableType::Var;
  ScriptVariableMap::iterator it = itsTokenScriptVariableNames.find(theVariableText);
  if (it != itsTokenScriptVariableNames.end() && isNewScriptVariable)
  {
    // var x käytetty uudestaan, esitellään muuttujat vain kerran
    throw runtime_error(::GetDictionaryString("SmartToolErrorScriptVariableSecondTime") + ": " +
                        theVariableText);
  }
  else if (it != itsTokenScriptVariableNames.end())  
  {
    // muuttujaa x käytetty uudestaan
    NFmiParam param((*it).second,
                    (*it).first,
                    kFloatMissing,
                    kFloatMissing,
                    kFloatMissing,
                    kFloatMissing,
                    "%.1f",
                    kLinearly);
    NFmiProducer producer;  // tällä ei ole väliä
    NFmiDataIdent dataIdent(param, producer);
    theMaskInfo->SetOperationType(NFmiAreaMask::InfoVariable);
    theMaskInfo->SetDataIdent(dataIdent);
    theMaskInfo->SetDataType(NFmiInfoData::kScriptVariableData);
    // tämä ei todellakaan ole default tuottajan dataa
    // (tämä vaikuttaa siihen mm. että tehdäänkö datasta
    // kopioita tietyissä tilanteissa)
    theMaskInfo->SetUseDefaultProducer(false);  
    return true;
  }
  else if (isNewScriptVariable)
  {
    // var x, eli 1. alustus
    NFmiParam param(itsScriptVariableParamIdCounter,
                    theVariableText,
                    kFloatMissing,
                    kFloatMissing,
                    kFloatMissing,
                    kFloatMissing,
                    "%.1f",
                    kLinearly);
    itsTokenScriptVariableNames.insert(
        ScriptVariableMap::value_type(theVariableText, itsScriptVariableParamIdCounter));
    itsScriptVariableParamIdCounter++;  // kasvatetaan seuraavaa uutta muutujaa varten
    NFmiProducer producer;              // Tuottajalla ei ole väliä
    NFmiDataIdent dataIdent(param, producer);
    theMaskInfo->SetOperationType(NFmiAreaMask::InfoVariable);
    theMaskInfo->SetDataIdent(dataIdent);
    theMaskInfo->SetDataType(NFmiInfoData::kScriptVariableData);
    // tämä ei todellakaan ole default tuottajan dataa
    // (tämä vaikuttaa siihen mm. että tehdäänkö datasta
    // kopioita tietyissä tilanteissa)
    theMaskInfo->SetUseDefaultProducer(false);
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::InterpretVariableCheckTokens(
    const std::string &theVariableText,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
    bool fOrigWanted,
    bool fLevelExist,
    bool fProducerExist,
    const std::string &theParamNameOnly,
    const std::string &theLevelNameOnly,
    const std::string &theProducerNameOnly,
    int theModelRunIndex,
    float theTimeOffsetInHours)
{
  if (NFmiSmartToolIntepreter::InterpretVariableOnlyCheck(theVariableText,
                                                          theMaskInfo,
                                                          fOrigWanted,
                                                          fLevelExist,
                                                          fProducerExist,
                                                          theParamNameOnly,
                                                          theLevelNameOnly,
                                                          theProducerNameOnly,
                                                          theModelRunIndex,
                                                          theTimeOffsetInHours))
    return true;
  else
  {
    if (IsVariableThreeArgumentFunction(theVariableText, theMaskInfo))
      return true;

    if (IsVariableFunction(theVariableText, theMaskInfo))
      return true;

    if (IsVariableMathFunction(theVariableText, theMaskInfo))
      return true;

    if (IsVariableRampFunction(theVariableText, theMaskInfo))
      return true;

    if (IsVariableMacroParam(theVariableText, theMaskInfo))
      return true;

    if (IsVariableDeltaZ(theVariableText, theMaskInfo))
      return true;

    if (IsVariableBinaryOperator(theVariableText,
                                 theMaskInfo))  // tämä on and ja or tapausten käsittelyyn
      return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::InterpretVariableOnlyCheck(
    const std::string &theVariableText,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
    bool fOrigWanted,
    bool fLevelExist,
    bool fProducerExist,
    const std::string &theParamNameOnly,
    const std::string &theLevelNameOnly,
    const std::string &theProducerNameOnly,
    int theModelRunIndex,
    float theTimeOffsetInHours)
{
  if (fLevelExist && fProducerExist)  // kokeillaan ensin, löytyykö param+level+producer
  {
    if (NFmiSmartToolIntepreter::FindParamAndLevelAndProducerAndSetMaskInfo(
            theParamNameOnly,
            theLevelNameOnly,
            theProducerNameOnly,
            NFmiAreaMask::InfoVariable,
            fOrigWanted ? NFmiInfoData::kCopyOfEdited : NFmiInfoData::kViewable,
            theMaskInfo,
            theModelRunIndex,
            theTimeOffsetInHours))
      return true;
  }
  else if (fLevelExist)  // kokeillaan sitten, löytyykö param+level
  {
    // Jos tuottajaa ei ole mainittu, oletetaan, että kyseessä on editoitava parametri.
    if (NFmiSmartToolIntepreter::FindParamAndLevelAndSetMaskInfo(theParamNameOnly,
                                                                 theLevelNameOnly,
                                                                 NFmiAreaMask::InfoVariable,
                                                                 NFmiInfoData::kEditable,
                                                                 theMaskInfo,
                                                                 theModelRunIndex,
                                                                 theTimeOffsetInHours))
      return true;
  }
  else if (fProducerExist)  // kokeillaan sitten, löytyykö param+producer
  {
    if (NFmiSmartToolIntepreter::FindParamAndProducerAndSetMaskInfo(
            theParamNameOnly,
            theProducerNameOnly,
            NFmiAreaMask::InfoVariable,
            fOrigWanted ? NFmiInfoData::kCopyOfEdited : NFmiInfoData::kViewable,
            theMaskInfo,
            theModelRunIndex,
            theTimeOffsetInHours))
      return true;
  }

  if (NFmiSmartToolIntepreter::FindParamAndSetMaskInfo(theVariableText,
                                                       itsTokenParameterNamesAndIds,
                                                       NFmiAreaMask::InfoVariable,
                                                       NFmiInfoData::kEditable,
                                                       theMaskInfo,
                                                       theModelRunIndex,
                                                       theTimeOffsetInHours))
    return true;

  if (NFmiSmartToolIntepreter::FindParamAndSetMaskInfo(theVariableText,
                                                       itsTokenStaticParameterNamesAndIds,
                                                       NFmiAreaMask::InfoVariable,
                                                       NFmiInfoData::kStationary,
                                                       theMaskInfo,
                                                       theModelRunIndex,
                                                       theTimeOffsetInHours))
    return true;

  if (NFmiSmartToolIntepreter::FindParamAndSetMaskInfo(theVariableText,
                                                       itsTokenCalculatedParameterNamesAndIds,
                                                       NFmiAreaMask::CalculatedVariable,
                                                       NFmiInfoData::kCalculatedValue,
                                                       theMaskInfo,
                                                       theModelRunIndex,
                                                       theTimeOffsetInHours))
    return true;

  if (NFmiSmartToolIntepreter::IsVariableConstantValue(theVariableText, theMaskInfo))
    return true;

  return false;
}

bool NFmiSmartToolIntepreter::IsProducerOrig(std::string &theProducerText)
{
  // Normalize the type name
  string name(theProducerText);
  transform(name.begin(), name.end(), name.begin(), ::tolower);
  if (name == "orig")
    return true;
  return false;
}

bool NFmiSmartToolIntepreter::IsFunctionNameWithUnderScore(const std::string &theVariableText)
{
  std::string tmpVariableStr = theVariableText;
  // HUOM! Mm. uusissa vertikaali-funktioissa on '_'-merkkejä, jotka pitää ignoorata jossain
  // paikoissa koodia
  VertFunctionMap::iterator it =
      itsTokenVertFunctions.find(NFmiStringTools::LowerCase(tmpVariableStr));
  if (it != itsTokenVertFunctions.end())
    return true;  // Kyse oli jostain uudesta vertikaali funktiosta

  return false;
}

// tämä heittää poikkeuksen virheilmoituksineen (auttavamman), jos tämä muuttuja muistuttaa uusia
// alaviiva -funktioita
void NFmiSmartToolIntepreter::CheckIfVariableResemblesVerticalFunction(
    const std::string &theVariableText)
{
  std::string tmpVariableStr(theVariableText);
  NFmiStringTools::LowerCase(tmpVariableStr);
  // jos theVariableText:istä löytyy vert-sana, tehdään vertikaali funktioihin liittyvä
  // virheilmoitus
  string::size_type pos = tmpVariableStr.find(string("vert"));
  if (pos != string::npos)
  {
    std::string errorStr(::GetDictionaryString("Given variable or function"));
    errorStr += " '";
    errorStr += theVariableText;
    errorStr += "' ";
    errorStr += ::GetDictionaryString("resembles vertical functions.");
    errorStr += "\n";
    errorStr += ::GetDictionaryString("If it was, there was a typo.");
    errorStr += " ";
    errorStr += ::GetDictionaryString("Try something like: vertp_max or vertz_findh.");
    throw std::runtime_error(errorStr);
  }
}

// Bracket has either model-run-index (e.g. "T_ec[-1]") or time-offset-in-hours (e.g. "T_ec[-1.5h]")
static void IntepretBracketString(const std::string &theVariableText,
                                  std::string theBracketText,
                                  int &theModelRunIndex,
                                  float &theTimeOffsetInHours)
{
  std::string trimmedText = NFmiStringTools::TrimR(theBracketText, ']');
  if (trimmedText.empty())
    throw std::runtime_error(std::string("Empty brackets in variable:") + theVariableText);
  auto lastCharacter = trimmedText[trimmedText.size() - 1];
  if (lastCharacter == 'h' || lastCharacter == 'H')
  {
    trimmedText.resize(trimmedText.size() - 1);
    theTimeOffsetInHours = NFmiStringTools::Convert<float>(trimmedText);
  }
  else
    theModelRunIndex = NFmiStringTools::Convert<int>(trimmedText);
}

// Saa parametrina kokonaisen parametri stringin, joka voi sisältää
// parametrin lisäksi myös levelin ja producerin.
// Teksti voi siis olla vaikka: T, T_850, T_Ec, T_850_Ec
// Eri osat asetetaan parametreina annetuille stringeille ja exist
// parametrit asetetaan myös vastaavasti, paitsi itse paramStringille
// ei ole muutujaa, koska se on pakollinen.
void NFmiSmartToolIntepreter::CheckVariableString(const std::string &theVariableText,
                                                  std::string &theParamText,
                                                  bool &fLevelExist,
                                                  std::string &theLevelText,
                                                  bool &fProducerExist,
                                                  std::string &theProducerText,
                                                  int &theModelRunIndex,
                                                  float &theTimeOffsetInHours)
{
  if (NFmiSmartToolIntepreter::IsFunctionNameWithUnderScore(theVariableText))
    return;  // Kyse oli jostain uudesta funktiosta, joissa on alaviiva, eikä tehdä nyt tässä mitään

  theParamText = theLevelText = theProducerText = "";
  fLevelExist = fProducerExist = false;

  std::vector<std::string> variableParts = NFmiStringTools::Split(theVariableText, "_");
  if (variableParts.empty())
    throw std::runtime_error(std::string(
        "Error with checked variable text, seems that it was empty (internal error?). If you see "
        "this, send message to developers with the problematic smarttool formula..."));

  std::vector<std::string> lastParamParts = NFmiStringTools::Split(
      variableParts[variableParts.size() - 1], "[");  // viimeisen parametri osion yhteydessä voi
                                                      // olla [-1] -osio, missä on kerrottu haluttu
                                                      // malliajo
  if (lastParamParts.size() > 2)
  {
    std::string errStr = ::GetDictionaryString("Error with variable text");
    errStr += ": ";
    errStr += theVariableText;
    errStr += "\n";
    errStr += ::GetDictionaryString("Too many '[' markers.");
    throw runtime_error(errStr);
  }
  else if (lastParamParts.size() == 2)
  {
    // pitää laittaa viimeiseen osioon malliajo-tiedosta 'riisuttu' teksti osio
    variableParts[variableParts.size() - 1] = lastParamParts[0];
    ::IntepretBracketString(
        theVariableText, lastParamParts[1], theModelRunIndex, theTimeOffsetInHours);
  }

  theParamText = variableParts[0];  // parametri teksti on aina 1. osiossa

  if (variableParts.size() >= 2)
  {
    std::string secondPartStr = variableParts[1];
    if (NFmiSmartToolIntepreter::IsPossiblyLevelItem(secondPartStr))
    {
      fLevelExist = true;
      theLevelText = secondPartStr;
    }
    else if (NFmiSmartToolIntepreter::IsPossiblyProducerItem(secondPartStr,
                                                             itsTokenProducerNamesAndIds))
    {
      fProducerExist = true;
      theProducerText = secondPartStr;
    }
    else
    {
      NFmiSmartToolIntepreter::CheckIfVariableResemblesVerticalFunction(
          theVariableText);  // tämä heittää poikkeuksen
                             // virheilmoituksineen
                             // (auttavamman), jos tämä
                             // muuttuja muistuttaa uusia
                             // alaviiva -funktioita
      // muuten tee standardi virheilmoitus
      throw runtime_error(::GetDictionaryString("SmartToolErrorVariableWithUndescore") + ":\n" +
                          theVariableText);
    }
  }

  if (variableParts.size() >= 3)
  {
    std::string thirdPartStr = variableParts[2];
    if (NFmiSmartToolIntepreter::IsPossiblyLevelItem(thirdPartStr))
    {
      if (fLevelExist == false)
      {
        fLevelExist = true;
        theLevelText = thirdPartStr;
      }
      else
        throw runtime_error(::GetDictionaryString("SmartToolErrorVariableWithTwoLevels") + ":\n" +
                            theVariableText);
    }
    else if (NFmiSmartToolIntepreter::IsPossiblyProducerItem(thirdPartStr,
                                                             itsTokenProducerNamesAndIds))
    {
      if (fProducerExist == false)
      {
        fProducerExist = true;
        theProducerText = thirdPartStr;
      }
      else
        throw runtime_error(::GetDictionaryString("SmartToolErrorVariableWithTwoProducers") +
                            ":\n" + theVariableText);
    }
    else
      throw runtime_error(::GetDictionaryString("SmartToolErrorVariableNoLevelOrProducer") + ":\n" +
                          theVariableText);
  }
}

// HUOM! tekee lower case tarkasteluja, joten theMap pitää myös alustaa lower case stringeillä
template <typename mapType>
bool IsInMap(mapType &theMap, const std::string &theSearchedItem)
{
  std::string lowerCaseItem = theSearchedItem;
  NFmiStringTools::LowerCase(lowerCaseItem);
  typename mapType::iterator it = theMap.find(lowerCaseItem);
  if (it != theMap.end())
    return true;
  return false;
}

bool NFmiSmartToolIntepreter::IsPossiblyProducerItem(const std::string &theText,
                                                     ProducerMap &theMap)
{
  if (::IsInMap(theMap, theText))
    return true;
  else if (NFmiSmartToolIntepreter::IsWantedStart(theText, "prod"))
    return true;
  return false;
}
bool NFmiSmartToolIntepreter::IsPossiblyLevelItem(const std::string &theText)
{
  try
  {
    // jos numeroksi muunto onnistuu, oletetaan että kyseessä on level
    NFmiStringTools::Convert<int>(theText);
    return true;
  }
  catch (...)
  {
  }

  if (NFmiSmartToolIntepreter::IsWantedStart(theText, "lev"))
    return true;
  else if (NFmiSmartToolIntepreter::IsWantedStart(theText, "fl"))
    return true;
  else if (NFmiSmartToolIntepreter::IsWantedStart(theText, "z"))
    return true;
  return false;
}

bool NFmiSmartToolIntepreter::ExtractParamAndLevel(const std::string &theVariableText,
                                                   std::string *theParamNameOnly,
                                                   std::string *theLevelNameOnly)
{
  string::size_type pos = theVariableText.find(string("_"));
  if (pos != string::npos)
  {
    *theParamNameOnly = string(theVariableText.begin(), theVariableText.begin() + pos);
    pos++;
    *theLevelNameOnly = string(theVariableText.begin() + pos, theVariableText.end());
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::FindParamAndLevelAndSetMaskInfo(
    const std::string &theVariableText,
    const std::string &theLevelText,
    NFmiAreaMask::CalculationOperationType theOperType,
    NFmiInfoData::Type theDataType,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
    int theModelRunIndex,
    float theTimeOffsetInHours)
{
  if (NFmiSmartToolIntepreter::FindParamAndSetMaskInfo(theVariableText,
                                                       itsTokenParameterNamesAndIds,
                                                       theOperType,
                                                       theDataType,
                                                       theMaskInfo,
                                                       theModelRunIndex,
                                                       theTimeOffsetInHours))
  {
    NFmiLevel level(NFmiSmartToolIntepreter::GetPossibleLevelInfo(theLevelText, theDataType));
    theMaskInfo->SetLevel(&level);
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::FindParamAndProducerAndSetMaskInfo(
    const std::string &theVariableText,
    const std::string &theProducerText,
    NFmiAreaMask::CalculationOperationType theOperType,
    NFmiInfoData::Type theDataType,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
    int theModelRunIndex,
    float theTimeOffsetInHours)
{
  auto producerDataTypePair = NFmiSmartToolIntepreter::GetPossibleProducerInfo(theProducerText);
  theDataType = producerDataTypePair.second;
  if (NFmiSmartToolIntepreter::FindParamAndSetMaskInfo(theVariableText,
                                                       itsTokenParameterNamesAndIds,
                                                       theOperType,
                                                       theDataType,
                                                       theMaskInfo,
                                                       producerDataTypePair.first,
                                                       theModelRunIndex,
                                                       theTimeOffsetInHours))
    return true;
  return false;
}

bool NFmiSmartToolIntepreter::FindParamAndLevelAndProducerAndSetMaskInfo(
    const std::string &theVariableText,
    const std::string &theLevelText,
    const std::string &theProducerText,
    NFmiAreaMask::CalculationOperationType theOperType,
    NFmiInfoData::Type theDataType,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
    int theModelRunIndex,
    float theTimeOffsetInHours)
{
  auto producerDataTypePair = NFmiSmartToolIntepreter::GetPossibleProducerInfo(theProducerText);
  theDataType = producerDataTypePair.second;
  if (NFmiSmartToolIntepreter::FindParamAndSetMaskInfo(theVariableText,
                                                       itsTokenParameterNamesAndIds,
                                                       theOperType,
                                                       theDataType,
                                                       theMaskInfo,
                                                       producerDataTypePair.first,
                                                       theModelRunIndex,
                                                       theTimeOffsetInHours))
  {
    NFmiLevel level(NFmiSmartToolIntepreter::GetPossibleLevelInfo(theLevelText, theDataType));
    theMaskInfo->SetLevel(&level);
    if (level.LevelType() == kFmiHybridLevel)
      theMaskInfo->SetDataType(NFmiInfoData::kHybridData);
    else if (FmiParameterName(level.LevelType()) == kFmiPressure)
      theMaskInfo->SetDataType(NFmiInfoData::kViewable);
    return true;
  }
  return false;
}

// Level tekstin (850, 500 jne.) luodaan NFmiLevel-otus.
NFmiLevel NFmiSmartToolIntepreter::GetPossibleLevelInfo(const std::string &theLevelText,
                                                        NFmiInfoData::Type theDataType)
{
  NFmiLevel level;

  try
  {
    // jos numeroksi muunto onnistuu, oletetaan että kyseessä on level
    float value = NFmiStringTools::Convert<float>(theLevelText);
    level = NFmiLevel(kFmiPressureLevel, theLevelText, value);
    return level;
  }
  catch (...)
  {
  }

  if (!NFmiSmartToolIntepreter::GetLevelFromVariableById(theLevelText, level, theDataType))
    throw runtime_error(::GetDictionaryString("SmartToolErrorLevelInfoFailed") + ":\n" +
                        theLevelText);
  return level;
}

// Tuottaja tekstin (Ec, Hir jne.) luodaan NFmiProducer-otus.
NFmiSmartToolIntepreter::ProducerTypePair NFmiSmartToolIntepreter::GetPossibleProducerInfo(
    const std::string &theProducerText)
{
  NFmiProducer producer;
  std::string lowerCaseProdName(theProducerText);
  NFmiStringTools::LowerCase(lowerCaseProdName);
  auto it = itsTokenProducerNamesAndIds.find(lowerCaseProdName);
  if (it != itsTokenProducerNamesAndIds.end())
  {
    producer = NFmiProducer(it->second.first, it->first);
    return std::make_pair(producer, it->second.second);
  }
  else if (!NFmiSmartToolIntepreter::GetProducerFromVariableById(theProducerText, producer))
    throw runtime_error(::GetDictionaryString("SmartToolErrorProducerInfoFailed") + ":\n" +
                        theProducerText);
  return std::make_pair(producer, NFmiInfoData::kViewable);
}

bool NFmiSmartToolIntepreter::IsInterpretedSkriptMacroParam()
{
  return (fNormalAssigmentFound == false) && (fMacroParamFound == true);
}

bool NFmiSmartToolIntepreter::GetParamFromVariable(const std::string &theVariableText,
                                                   ParamMap &theParamMap,
                                                   NFmiParam &theParam,
                                                   bool &fUseWildDataType)
{
  std::string tmp(theVariableText);
  ParamMap::iterator it = theParamMap.find(NFmiStringTools::LowerCase(tmp));
  if (it == theParamMap.end())
  {
    if (NFmiSmartToolIntepreter::GetParamFromVariableById(theVariableText, theParam))
      fUseWildDataType =
          true;  // paridta käytettäessä pitää asettaa data tyyppi 'villiksi' toistaiseksi
    else
      return false;
  }
  else
    theParam = NFmiParam((*it).second,
                         (*it).first,
                         kFloatMissing,
                         kFloatMissing,
                         kFloatMissing,
                         kFloatMissing,
                         "%.1f",
                         kLinearly);
  return true;
}

// tutkii alkaako annettu sana "par"-osiolla ja sitä seuraavalla numerolla
// esim. par4 tai PAR114 jne.
bool NFmiSmartToolIntepreter::GetParamFromVariableById(const std::string &theVariableText,
                                                       NFmiParam &theParam)
{
  if (NFmiSmartToolIntepreter::IsWantedStart(theVariableText, "par"))
  {
    NFmiValueString numericPart(theVariableText.substr(3));
    if (numericPart.IsNumeric())
    {
      theParam = NFmiParam(static_cast<long>(numericPart),
                           theVariableText,
                           kFloatMissing,
                           kFloatMissing,
                           kFloatMissing,
                           kFloatMissing,
                           "%.1f",
                           kLinearly);
      return true;
    }
  }
  return false;
}

// tutkii alkaako annettu sana "prod"-osiolla ja sitä seuraavalla numerolla
// esim. prod230 tai PROD1 jne.
bool NFmiSmartToolIntepreter::GetProducerFromVariableById(const std::string &theVariableText,
                                                          NFmiProducer &theProducer)
{
  if (NFmiSmartToolIntepreter::IsWantedStart(theVariableText, "prod"))
  {
    NFmiValueString numericPart(theVariableText.substr(4));
    if (numericPart.IsNumeric())
    {
      theProducer = NFmiProducer(static_cast<long>(numericPart), theVariableText);
      return true;
    }
  }
  return false;
}

static bool HandleVariableLevelInfo(const std::string &variableText,
                                    NFmiLevel &level,
                                    const std::string &searchedLevelTextStart,
                                    FmiLevelType wantedLevelType)
{
  if (NFmiSmartToolIntepreter::IsWantedStart(variableText, searchedLevelTextStart))
  {
    NFmiValueString numericPart(variableText.substr(searchedLevelTextStart.size()));
    if (numericPart.IsNumeric())
    {
      FmiLevelType levelType = wantedLevelType;
      level = NFmiLevel(levelType, variableText, static_cast<float>(numericPart));
      return true;
    }
  }
  return false;
}

// tutkii alkaako annettu sana "lev"-osiolla ja sitä seuraavalla numerolla
// esim. par100 tai LEV850 jne.
bool NFmiSmartToolIntepreter::GetLevelFromVariableById(const std::string &theVariableText,
                                                       NFmiLevel &theLevel,
                                                       NFmiInfoData::Type /* theDataType */)
{
  if (::HandleVariableLevelInfo(theVariableText, theLevel, "lev", kFmiHybridLevel))
  {
    // jos käyttäjä on antanut esim. T_ec_lev45, tällöin halutaan hybrid level 45 ei painepinta 45.
    return true;
  }
  else if (::HandleVariableLevelInfo(theVariableText, theLevel, "fl", kFmiFlightLevel))
  {
    // jos käyttäjä on antanut esim. T_ec_fl200, tällöin halutaan flight level 200.
    return true;
  }
  else if (::HandleVariableLevelInfo(theVariableText, theLevel, "z", kFmiHeight))
  {
    // jos käyttäjä on antanut esim. T_ec_z1500, tällöin halutaan arvot korkeudelta 1500 metriä.
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsWantedStart(const std::string &theText,
                                            const std::string &theWantedStart)
{
  string name(theText.substr(0, theWantedStart.size()));
  transform(name.begin(), name.end(), name.begin(), ::tolower);
  if (name == theWantedStart)
    return true;
  return false;
}

void NFmiSmartToolIntepreter::SetAbsoluteBasePaths(const std::string &theAbsoluteSmarttoolsBasePath,
                                                   const std::string &theAbsoluteMacroParamBasePath)
{
  itsAbsoluteSmarttoolsBasePath = theAbsoluteSmarttoolsBasePath;
  itsAbsoluteMacroParamBasePath = theAbsoluteMacroParamBasePath;
}

bool NFmiSmartToolIntepreter::FindParamAndSetMaskInfo(
    const std::string &theVariableText,
    ParamMap &theParamMap,
    NFmiAreaMask::CalculationOperationType theOperType,
    NFmiInfoData::Type theDataType,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
    int theModelRunIndex,
    float theTimeOffsetInHours)
{
  NFmiParam param;
  bool fUseWildDataType = false;
  if (NFmiSmartToolIntepreter::GetParamFromVariable(
          theVariableText, theParamMap, param, fUseWildDataType))
  {
    NFmiProducer producer;
    NFmiDataIdent dataIdent(param, producer);
    theMaskInfo->SetOperationType(theOperType);
    theMaskInfo->SetDataIdent(dataIdent);
    theMaskInfo->SetUseDefaultProducer(true);
    theMaskInfo->ModelRunIndex(theModelRunIndex);
    theMaskInfo->TimeOffsetInHours(theTimeOffsetInHours);
    // HUOM! anydata-tyyppi sallitaan vain kun tarkastellaan ei editoitavia parametreja, tämä
    // siksi että ei haluta esim. par165 menevän läpi smarttool-kielessä, koska se on ec:n meren
    // aallon pituusparametri ja ilman tuottajaa haluataan vain tukea editoitua dataa ja topo-dataa
    if (fUseWildDataType && theDataType != NFmiInfoData::kEditable)
      theMaskInfo->SetDataType(NFmiInfoData::kAnyData);
    else
      theMaskInfo->SetDataType(theDataType);
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::FindParamAndSetMaskInfo(
    const std::string &theVariableText,
    ParamMap &theParamMap,
    NFmiAreaMask::CalculationOperationType theOperType,
    NFmiInfoData::Type theDataType,
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo,
    const NFmiProducer &theProducer,
    int theModelRunIndex,
    float theTimeOffsetInHours)
{
  NFmiParam param;
  bool fUseWildDataType = false;
  if (NFmiSmartToolIntepreter::GetParamFromVariable(
          theVariableText, theParamMap, param, fUseWildDataType))
  {
    NFmiProducer usedProducer(theProducer);
    if (usedProducer.GetIdent() == NFmiInfoData::kFmiSpEcmwf3Vrk)
    {  // ikävää koodia, mutta tein kikka vitosen että saan EC:n 3vrk datat mukaan smarttool-kieleen
      // ja laitoin koodin tähän funktioon, että se hanskaa samalla sekä pinta että painepinta datat
      theDataType = NFmiInfoData::kModelHelpData;  // EC:n 3 vrk datat tyyppi asetettava, että
                                                   // ohittaa normaali ec datan
      usedProducer = NFmiProducer(kFmiMTAECMWF, "Ec3vrk");
    }

    NFmiDataIdent dataIdent(param, usedProducer);
    theMaskInfo->SetOperationType(theOperType);
    theMaskInfo->SetDataIdent(dataIdent);
    theMaskInfo->SetUseDefaultProducer(false);
    theMaskInfo->ModelRunIndex(theModelRunIndex);
    theMaskInfo->TimeOffsetInHours(theTimeOffsetInHours);
    if (fUseWildDataType)
    {
      if (usedProducer.GetIdent() == 0)  // tämä on viritys, jos on annettu esim. "par180_ec"
                                         // parametri, menee se any-data kategoriaan, ellei ei
                                         // tarkisteta onko oikeasti annettu tuottajakin
        theMaskInfo->SetDataType(NFmiInfoData::kAnyData);
      else
        theMaskInfo->SetDataType(theDataType);
    }
    else if (usedProducer.GetIdent() == 999)  // tämä 999 on viritys, mutta se on määrätty
                                              // helpdatainfo.dat tiedostossa kepa-datan feikki id
      // numeroksi. Oikeaa id:tä ei voi käyttää, koska se on
      // sama kuin editoitavalla datalla.
      theMaskInfo->SetDataType(NFmiInfoData::kKepaData);
    else
      theMaskInfo->SetDataType(theDataType);
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsVariableConstantValue(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  NFmiValueString valueString(theVariableText);
  if (valueString.IsNumeric())
  {
    double value = static_cast<double>(valueString);
    theMaskInfo->SetOperationType(NFmiAreaMask::Constant);
    NFmiCalculationCondition calcCond(kFmiMaskEqual, value);
    theMaskInfo->SetMaskCondition(calcCond);
    return true;
  }

  // sitten katsotaan onko se esim. MISS tai PI tms esi määritelty vakio
  std::string tmp(theVariableText);
  ConstantMap::iterator it = itsTokenConstants.find(NFmiStringTools::LowerCase(tmp));
  if (it != itsTokenConstants.end())
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::Constant);
    NFmiCalculationCondition calcCond(kFmiMaskEqual, (*it).second);
    theMaskInfo->SetMaskCondition(calcCond);
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsVariableMacroParam(const std::string &theVariableText,
                                                   boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  if (FindAnyFromText(theVariableText, itsTokenMacroParamIdentifiers))
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::InfoVariable);
    theMaskInfo->SetDataType(NFmiInfoData::kMacroParam);
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsVariableDeltaZ(const std::string &theVariableText,
                                               boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  if (FindAnyFromText(theVariableText, itsTokenDeltaZIdentifiers))
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::DeltaZFunction);
    return true;
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsVariableMathFunction(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  std::string tmp(theVariableText);
  MathFunctionMap::iterator it = itsMathFunctions.find(NFmiStringTools::LowerCase(tmp));
  if (it != itsMathFunctions.end())
  {
    if (GetToken())
    {
      tmp = token;             // luetaan muuttuja/vakio/funktio tai mikä lie
      if (tmp == string("("))  // etsitään fuktion aloitus sulkua (lopetus sulku tulee sitten
                               // aikanaan, välissä voi olla mitä vain!)
      {
        theMaskInfo->SetOperationType(NFmiAreaMask::MathFunctionStart);
        theMaskInfo->SetMathFunctionType((*it).second);
        return true;
      }
    }
    throw runtime_error(::GetDictionaryString("SmartToolErrorMathFunctionParams") + ": " +
                        theVariableText);
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsVariableThreeArgumentFunction(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  // sitten katsotaan onko jokin integraatio funktioista
  std::string tmp(theVariableText);
  FunctionMap::iterator it = itsTokenThreeArgumentFunctions.find(NFmiStringTools::LowerCase(tmp));
  if (it != itsTokenThreeArgumentFunctions.end())
  {
    int functionUsed = 1;  // 1 = T-funktio, 2 = Z-funktio ja 3 = H-funktio
    if (theVariableText.find('z') != string::npos || theVariableText.find('Z') != string::npos)
      functionUsed = 2;
    else if (theVariableText.find('h') != string::npos || theVariableText.find('H') != string::npos)
      functionUsed = 3;
    theMaskInfo->SetFunctionType((*it).second);  // min, max jne. asetus
    theMaskInfo->IntegrationFunctionType(functionUsed);
    if (GetToken())
    {
      tmp = token;             // luetaan muuttuja/vakio/funktio tai mikä lie
      if (tmp == string("("))  // etsitään fuktion aloitus sulkua (lopetus sulku tulee sitten
                               // aikanaan, välissä voi olla mitä vain!)
      {
        theMaskInfo->SetOperationType(NFmiAreaMask::ThreeArgumentFunctionStart);
        return true;
      }
    }
    throw runtime_error(::GetDictionaryString("SmartToolErrorTimeFunctionParams") + ":\n" +
                        theVariableText);
  }
  return false;
}

// Nämä ovat muotoa aika-'integroinnin' yhteydessä:
// MIN(T -1 1)
// eli 1. funktion nimi, sulku auki, parametri, aloitus aikaoffset, lopetus aikaoffset ja lopuksi
// suliku kiinni.
// Tai alue-'integroinnin' yhteydessä:
// MIN(T -1 -1 1 1)
// eli 1. funktion nimi, sulku auki, parametri, aloitus x ja y paikkaoffset, lopetus x ja y
// paikkaoffset ja lopuksi suliku kiinni.
bool NFmiSmartToolIntepreter::IsVariableFunction(const std::string &theVariableText,
                                                 boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  // katsotaan onko jokin peek-funktioista
  if (IsVariablePeekFunction(theVariableText, theMaskInfo))
    return true;
  if (IsVariableMetFunction(theVariableText, theMaskInfo))
    return true;
  if (IsVariableVertFunction(theVariableText, theMaskInfo))
    return true;
  if (IsVariableExtraInfoCommand(theVariableText))
    throw ExtraInfoMacroLineException();

  // sitten katsotaan onko jokin integraatio funktioista
  std::string tmp(theVariableText);
  FunctionMap::iterator it = itsTokenFunctions.find(NFmiStringTools::LowerCase(tmp));
  if (it != itsTokenFunctions.end())
  {
    theMaskInfo->SetFunctionType((*it).second);
    tmp = "";
    std::vector<pair<string, types> > tokens;
    int i;
    for (i = 0; i < 7 && GetToken(); i++)  // maksimissaan 7 kertaa
    {
      tmp = token;  // luetaan muuttuja/vakio/funktio tai mikä lie
      tmp = HandlePossibleUnaryMarkers(tmp);
      tokens.push_back(std::make_pair(tmp, tok_type));
      if (tmp == string(")"))  // etsitään lopetus sulkua
        break;
    }
    // ensin tutkitaan, onko kyseessä aikaintegrointi eli 5 'tokenia'
    if (i == 4 && (tokens[0].first == string("(")) && (tokens[4].first == string(")")))
    {
      if (tokens[1].second == VARIABLE && tokens[2].second == NUMBER && tokens[3].second == NUMBER)
      {
        InterpretVariable(tokens[1].first, theMaskInfo);
        if (theMaskInfo->GetOperationType() == NFmiAreaMask::InfoVariable)
        {
          theMaskInfo->SetOperationType(NFmiAreaMask::FunctionTimeIntergration);
          NFmiValueString valueString1(tokens[2].first);
          double value1 = static_cast<double>(valueString1);
          NFmiValueString valueString2(tokens[3].first);
          double value2 = static_cast<double>(valueString2);
          theMaskInfo->SetOffsetPoint1(NFmiPoint(value1, value2));
          return true;
        }
      }
    }
    // sitten tutkitaan, onko kyseessä alueintegrointi eli 7 'tokenia'
    else if (i == 6 && (tokens[0].first == string("(")) && (tokens[6].first == string(")")))
    {
      if (tokens[1].second == VARIABLE && tokens[2].second == NUMBER &&
          tokens[3].second == NUMBER && tokens[4].second == NUMBER && tokens[5].second == NUMBER)
      {
        InterpretVariable(tokens[1].first, theMaskInfo);
        if (theMaskInfo->GetOperationType() == NFmiAreaMask::InfoVariable)
        {
          theMaskInfo->SetOperationType(NFmiAreaMask::FunctionAreaIntergration);
          NFmiValueString valueString1(tokens[2].first);
          double value1 = static_cast<double>(valueString1);
          NFmiValueString valueString2(tokens[3].first);
          double value2 = static_cast<double>(valueString2);
          theMaskInfo->SetOffsetPoint1(NFmiPoint(value1, value2));
          NFmiValueString valueString3(tokens[4].first);
          double value3 = static_cast<double>(valueString3);
          NFmiValueString valueString4(tokens[5].first);
          double value4 = static_cast<double>(valueString4);
          theMaskInfo->SetOffsetPoint2(NFmiPoint(value3, value4));
          return true;
        }
      }
    }
    throw runtime_error(::GetDictionaryString("SmartToolErrorIntegrationFunctionParams") + ": " +
                        theVariableText);
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsVariablePeekFunction(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  std::string aVariableText(theVariableText);
  PeekFunctionMap::iterator it = itsTokenPeekFunctions.find(
      NFmiStringTools::LowerCase(aVariableText));  // tässä tarkastellaan case insensitiivisesti
  if (it != itsTokenPeekFunctions.end())
  {
    string tmp;
    std::vector<pair<string, types> > tokens;
    int i;
    for (i = 0; i < 5 && GetToken(); i++)  // maksimissaan 5 kertaa
    {
      tmp = token;  // luetaan muuttuja/vakio/funktio tai mikä lie
      tmp = HandlePossibleUnaryMarkers(tmp);
      tokens.push_back(std::make_pair(tmp, tok_type));
      if (tmp == string(")"))  // etsitään lopetus sulkua
        break;
    }
    // ensin tutkitaan, onko kyseessä peekxy eli yhteensä 5 'tokenia' peekxy(T 1 1), huom! sulut on
    // mukana tokeneissa
    if (i == 4 && (tokens[0].first == string("(")) && (tokens[4].first == string(")")))
    {
      if (tokens[1].second == VARIABLE && tokens[2].second == NUMBER && tokens[3].second == NUMBER)
      {
        InterpretVariable(tokens[1].first, theMaskInfo);
        if (theMaskInfo->GetOperationType() == NFmiAreaMask::InfoVariable)
        {
          theMaskInfo->SetOperationType((*it).second);
          NFmiValueString valueString1(tokens[2].first);
          double value1 = static_cast<double>(valueString1);
          NFmiValueString valueString2(tokens[3].first);
          double value2 = static_cast<double>(valueString2);
          theMaskInfo->SetOffsetPoint1(NFmiPoint(value1, value2));
          return true;
        }
      }
    }
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsVariableMetFunction(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  std::string aVariableText(theVariableText);
  MetFunctionMap::iterator it = itsTokenMetFunctions.find(
      NFmiStringTools::LowerCase(aVariableText));  // tässä tarkastellaan case insensitiivisesti
  if (it != itsTokenMetFunctions.end())
  {
    string tmp;
    std::vector<pair<string, types> > tokens;
    int argumentCount =
        (*it).second.get<2>();  // näin monta argumenttia on odotettavissa tälle funktio tyypille
    int tokenCount = argumentCount + (argumentCount - 1) +
                     2;  // ja siinä on argumentCount-1 pilkkua välissä!  + 2 on alku ja loppu sulut
    for (int i = 0; i < tokenCount && GetToken(); i++)
    {
      tmp = token;  // luetaan muuttuja/vakio/funktio tai mikä lie
      tmp = HandlePossibleUnaryMarkers(tmp);
      // pitää tarkistaa että on oikeissa kohdissa pilkut, mutta ne on ohitettava vain sitten
      if (tmp == string(","))  // tarkistetaan pilkut
      {
        bool isIndexPar = i % 2 == 0;  // onko indeksi parillinen
        bool overNonPossibleStartPos =
            i > 1;  // onko indeksi edennyt ohi alun, missä ei voi olla pilkkuja
        bool underNonPossibleEndPos = i < tokenCount - 2;  // onko indeksi vielä alle lopun sen
        // kohdan, minkä jälkeen ei voi olla enää
        // pilkkuja
        if ((isIndexPar && overNonPossibleStartPos && underNonPossibleEndPos) == false)
          break;  // jotain vikaa oli lausekkeessa
      }
      tokens.push_back(std::make_pair(tmp, tok_type));
    }
    // ensin tutkitaan, oliko funktiossa oikea määrä argumentteja esim. grad(T_Hir), missä on
    // tokeneita 1 + 2 (sulut)
    if (static_cast<long>(tokens.size()) == argumentCount + 2 && (tokens[0].first == string("(")) &&
        (tokens[tokens.size() - 1].first == string(")")))
    {
      if (tokens[1].second == VARIABLE)  // tehdään tämä toimimaan ensin grad-tapaus  // &&
                                         // tokens[2].second == NUMBER && tokens[3].second ==
                                         // NUMBER)
      {
        InterpretVariable(tokens[1].first, theMaskInfo);
        if (theMaskInfo->GetOperationType() == NFmiAreaMask::InfoVariable)
        {
          theMaskInfo->SetOperationType(NFmiAreaMask::MetFunction);
          theMaskInfo->SetFunctionType((*it).second.get<0>());
          theMaskInfo->MetFunctionDirection((*it).second.get<1>());
          return true;
        }
      }
    }
    else
    {
      std::string errorStr(::GetDictionaryString("Function"));
      errorStr += " '";
      errorStr += theVariableText;
      errorStr += "' ";
      errorStr += ::GetDictionaryString("error in line");
      errorStr += ": ";
      errorStr += theMaskInfo->GetOrigLineText();
      errorStr += "\n";
      errorStr += ::GetDictionaryString("Proper form for this function is");
      errorStr += ": ";
      errorStr += (*it).second.get<3>();
      throw std::runtime_error(errorStr);
    }
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsVariableVertFunction(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  std::string aVariableText(theVariableText);
  VertFunctionMap::iterator it = itsTokenVertFunctions.find(
      NFmiStringTools::LowerCase(aVariableText));  // tässä tarkastellaan case insensitiivisesti
  if (it != itsTokenVertFunctions.end())
  {
    // 1. Vertikaali funktioiden tapauksessa tarkistetaan tässä, että on '('-merkki, sitten yksi
    // muuttuja ja pilkku, muuten saa lausekkeita ja muita miten haluaa...
    // 2. Ensimmäinen parametri pitää olla pelkistetty yksi parametri (esim. WS_Hir), ei lauseketta
    // esim. WS_Hir - WS_EC (ainakaan vielä), jotta
    //	  voidaan käydä dataa läpi level-by-level tyyliin eikä kuten vanhoilla vertikaali
    // funktioilla, missä oli keinotekoiset stepit, joita käytiin läpi (juuri siksi että parametrin
    // sijasta saattoi olla lauseke.)
    // 3. Lopuissa parametreissa voi olla laskuja, muuttujia tai lausekkeita.
    GetToken();
    string tmp = token;
    if (tmp == string("("))
    {
      GetToken();
      string firstVariableStr = token;
      GetToken();
      tmp = token;
      if (tmp == string(","))
      {
        InterpretVariable(firstVariableStr, theMaskInfo);
        if (theMaskInfo->GetOperationType() == NFmiAreaMask::InfoVariable)
        {
          theMaskInfo->SetOperationType(NFmiAreaMask::VertFunctionStart);
          theMaskInfo->SetFunctionType((*it).second.get<0>());
          theMaskInfo->SetSecondaryFunctionType((*it).second.get<1>());
          theMaskInfo->FunctionArgumentCount((*it).second.get<2>());
          theMaskInfo->SimpleConditionRule((*it).second.get<4>());
          ExtractPossibleSecondaryParameterInfo(theMaskInfo);
          return true;
        }
      }
    }

    // Jos löytyi Vertikaali-Funktio, mutta ehdot eivät täyttyneet, tehdään virheilmoitus.
    std::string errorStr(::GetDictionaryString("Function"));
    errorStr += " '";
    errorStr += theVariableText;
    errorStr += "' ";
    errorStr += ::GetDictionaryString("error in line");
    errorStr += ": ";
    errorStr += theMaskInfo->GetOrigLineText();
    errorStr += "\n";
    errorStr += ::GetDictionaryString("Proper form for this function is");
    errorStr += ": ";
    errorStr += (*it).second.get<3>();
    throw std::runtime_error(errorStr);
  }
  return false;
}

static bool HasSecondaryParameter(boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  auto functionType = theMaskInfo->GetSecondaryFunctionType();
  return (functionType == NFmiAreaMask::SecondParamFromExtremeTime);
}

void NFmiSmartToolIntepreter::ExtractPossibleSecondaryParameterInfo(
    boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  if (HasSecondaryParameter(theMaskInfo))
  {
    // Nyt pitää löytyä sekundaari parametri ja sen perässä vielä pilkku
    GetToken();
    string secondaryVariableStr = token;
    GetToken();
    auto commaStr = token;
    if (commaStr == string(","))
    {
      boost::shared_ptr<NFmiAreaMaskInfo> secondaryParamMaskInfo(new NFmiAreaMaskInfo());
      InterpretVariable(secondaryVariableStr, secondaryParamMaskInfo);
      if (secondaryParamMaskInfo->GetOperationType() == NFmiAreaMask::InfoVariable)
      {
        theMaskInfo->SetSecondaryParam(secondaryParamMaskInfo->GetDataIdent());
        theMaskInfo->SetSecondaryParamLevel(secondaryParamMaskInfo->GetLevel());
        theMaskInfo->SetSecondaryParamDataType(secondaryParamMaskInfo->GetDataType());
        theMaskInfo->SetSecondaryParamUseDefaultProducer(
            secondaryParamMaskInfo->GetUseDefaultProducer());
        // NFmiSmartToolIntepreter::InitTokens metodissa kun itsTokenVertFunctions
        // alustetaan argumenttien lukumäärällä, yhtenäisyyden takia laitoin siellä
        // argumenttien määräksi sen mitä funktiolla annettiin, mutta koska sekundaari
        // parametri argumenttia ei käsitellä normaalisti ja se ohitetaan itse laskuissa
        //  niin siksi tässä kohtaa pitää niiden funktioiden argumentti määrää pienentää yhdellä.
        theMaskInfo->FunctionArgumentCount(theMaskInfo->FunctionArgumentCount() - 1);
        return;
      }
    }

    // Jos ei löytynyt oikein tulkittavaa sekundaari parametria ja pilkkua sen perässä,
    // heitetään poikkeus, jotta käyttäjälle saadaan virheilmoitus.
    std::string errorMessage =
        "There should be an acceptable secondary parameter in 2nd place in parameter list with "
        "function ";
    errorMessage += theMaskInfo->GetMaskText();
    throw std::runtime_error(errorMessage);
  }
}

static std::pair<bool, NFmiDefineWantedData> CheckForProducerLevelType(
    const std::string &possibleProducerPart,
    const std::string &possibleLevelTypePart,
    const std::string &originalDataString,
    const NFmiSmartToolIntepreter::ResolutionLevelTypesMap &resolutionLevelTypes)
{
  auto producer = NFmiSmartToolIntepreter::GetPossibleProducerInfo(possibleProducerPart).first;
  auto iter = resolutionLevelTypes.find(possibleLevelTypePart);
  if (iter != resolutionLevelTypes.end())
  {
    return std::make_pair(true, NFmiDefineWantedData(producer, iter->second, originalDataString));
  }
  return std::make_pair(false, NFmiDefineWantedData());
}

std::pair<bool, NFmiDefineWantedData> NFmiSmartToolIntepreter::CheckForVariableDataType(
    const std::string &originalDataVariableString)
{
  string paramNameOnly;
  string levelNameOnly;
  string producerNameOnly;
  bool levelExist = false;
  bool producerExist = false;
  int modelRunIndex = 1;
  float timeOffsetInHours = 0;

  NFmiSmartToolIntepreter::CheckVariableString(originalDataVariableString,
                                               paramNameOnly,
                                               levelExist,
                                               levelNameOnly,
                                               producerExist,
                                               producerNameOnly,
                                               modelRunIndex,
                                               timeOffsetInHours);

  boost::shared_ptr<NFmiAreaMaskInfo> maskInfo(new NFmiAreaMaskInfo());
  bool origWanted = false;
  if (NFmiSmartToolIntepreter::InterpretVariableOnlyCheck(originalDataVariableString,
                                                          maskInfo,
                                                          origWanted,
                                                          levelExist,
                                                          producerExist,
                                                          paramNameOnly,
                                                          levelNameOnly,
                                                          producerNameOnly,
                                                          modelRunIndex,
                                                          timeOffsetInHours))
  {
    if (!producerExist)
    {
      // Jos ei tuottajaa, kyse on sitten editoidusta datasta
      if (levelExist)
      {
        return std::make_pair(true,
                              NFmiDefineWantedData(NFmiInfoData::kEditable,
                                                   *maskInfo->GetDataIdent().GetParam(),
                                                   maskInfo->GetLevel(),
                                                   originalDataVariableString));
      }
      else
      {
        return std::make_pair(true,
                              NFmiDefineWantedData(NFmiInfoData::kEditable,
                                                   *maskInfo->GetDataIdent().GetParam(),
                                                   originalDataVariableString));
      }
    }
    else
    {
      return std::make_pair(true,
                            NFmiDefineWantedData(*maskInfo->GetDataIdent().GetProducer(),
                                                 *maskInfo->GetDataIdent().GetParam(),
                                                 maskInfo->GetLevel(),
                                                 originalDataVariableString,
                                                 maskInfo->TimeOffsetInHours()));
    }
  }
  return std::make_pair(false, NFmiDefineWantedData());
}

bool NFmiSmartToolIntepreter::ExtractResolutionInfo()
{
  GetToken();
  string assignOperator = token;
  if (assignOperator == string("="))
  {
    GetToken();
    string resolutionStr = token;

    try
    {
      itsExtraMacroParamData.GivenResolutionInKm(NFmiStringTools::Convert<float>(resolutionStr));
      return true;
    }
    catch (...)
    {
      // saattaa heittää poikkeuksia, mutta ei fataalia, jatketaan sitten taas erilaisten
      // asioiden tutkimista
    }

    auto variableDataInfo = GetPossibleVariableDataInfo(resolutionStr);
    if (variableDataInfo.first)
    {
      itsExtraMacroParamData.WantedResolutionData(variableDataInfo.second);
      return true;
    }
  }

  // Jos löytyi resolution -lauseke, mutta muuten ehdot eivät täyttyneet, tehdään virheilmoitus.
  std::string errorStr(::GetDictionaryString("Given 'resolution' operation was illegal"));
  errorStr += "\n";
  errorStr += ::GetDictionaryString("Try something like following");
  errorStr += ":\n";
  errorStr += ::GetDictionaryString("resolution = 25");
  errorStr += "\n" + ::GetDictionaryString("OR") + "\n";
  errorStr += ::GetDictionaryString("resolution = ec_surface");
  errorStr += "\n" + ::GetDictionaryString("OR") + "\n";
  errorStr +=
      ::GetDictionaryString("resolution = T_ec (ec surface data with Temperature parameter in it)");
  errorStr += "\n";
  errorStr += ::GetDictionaryString(
      "resolution = par4_prod240_850 (data which has par4 (Temperature) from producer with id 240 "
      "with pressure level 850 hPa)");
  throw std::runtime_error(errorStr);
}

std::pair<bool, NFmiDefineWantedData> NFmiSmartToolIntepreter::GetPossibleVariableDataInfo(
    const std::string &originalResolutionStr)
{
  std::string resolutionStr = originalResolutionStr;
  NFmiStringTools::LowerCase(resolutionStr);

  // Editoitu data on poikkeus, joka hanskataan ensin
  if (resolutionStr == std::string("edited"))
  {
    return std::make_pair(
        true,
        NFmiDefineWantedData(
            NFmiInfoData::kEditable, NFmiParam(kFmiBadParameter), originalResolutionStr));
  }

  vector<string> resolutionParts = NFmiStringTools::Split(resolutionStr, "_");

  try
  {
    // Tutkitaan ensin producer_leveltype (esim. ec_surface) type case
    if (resolutionParts.size() == 2)
    {
      auto producerLevelTypeInfo = ::CheckForProducerLevelType(
          resolutionParts[0], resolutionParts[1], originalResolutionStr, itsResolutionLevelTypes);
      if (producerLevelTypeInfo.first)
      {
        return std::make_pair(true, producerLevelTypeInfo.second);
      }
    }
  }
  catch (...)
  {
    // saattaa heittää poikkeuksia, mutta ei fataalia, jatketaan sitten taas erilaisten asioiden
    // tutkimista
  }

  // Tutkitaan lopuksi param_producer_possiblelevel (esim. T_ec tai par43_prod221 tai T_meps_850)
  // type case
  auto variableDataInfo = CheckForVariableDataType(resolutionStr);
  if (variableDataInfo.first)
  {
    auto &wantedResolutionData = variableDataInfo.second;
    wantedResolutionData.originalDataString_ = originalResolutionStr;
    return std::make_pair(true, wantedResolutionData);
  }

  return std::make_pair(false, NFmiDefineWantedData());
}

bool NFmiSmartToolIntepreter::ExtractFixedBaseData()
{
  GetToken();
  string assignOperator = token;
  if (assignOperator == string("="))
  {
    GetToken();
    string resolutionStr = token;

    auto variableDataInfo = GetPossibleVariableDataInfo(resolutionStr);
    if (variableDataInfo.first)
    {
      itsExtraMacroParamData.WantedFixedBaseData(variableDataInfo.second);
      return true;
    }
  }

  // Jos löytyi resolution -lauseke, mutta muuten ehdot eivät täyttyneet, tehdään virheilmoitus.
  std::string errorStr(::GetDictionaryString("Given 'FixedBaseData' operation was illegal"));
  errorStr += "\n";
  errorStr += ::GetDictionaryString("Try something like following");
  errorStr += ":\n";
  errorStr += ::GetDictionaryString("FixedBaseData = ec_surface");
  errorStr += "\n" + ::GetDictionaryString("OR") + "\n";
  errorStr += ::GetDictionaryString(
      "FixedBaseData = T_ec (ec surface data with Temperature parameter in it)");
  errorStr += "\n";
  errorStr += ::GetDictionaryString(
      "FixedBaseData = par4_prod240_850 (data which has par4 (Temperature) from producer with id "
      "240 "
      "with pressure level 850 hPa)");
  throw std::runtime_error(errorStr);
}

bool NFmiSmartToolIntepreter::ExtractMultiParam(NFmiAreaMask::FunctionType multiParamId)
{
  GetToken();
  string assignOperator = token;
  if (assignOperator == string("="))
  {
    GetToken();
    string parameterStr = token;

    // Katsotaan ensin onko kyse jostain parametrista
    auto variableDataInfo = std::make_pair(false, NFmiDefineWantedData());
    try
    {
      // Tehdään tämä try-catch blokissa, koska jos annettu macroParam tiedoston nimi, missä '_'
      // merkkejä, lentää poikkeus tässä muuttujatarkastelussa, ja sen jälkeen halutaan tutkia että
      // onko kyse macroParam tiedoston nimestä.
      variableDataInfo = GetPossibleVariableDataInfo(parameterStr);
    }
    catch (...)
    {
    }

    if (variableDataInfo.first)
    {
      if (multiParamId == NFmiAreaMask::MultiParam2)
      {
        itsExtraMacroParamData.MultiParam2(MultiParamData(variableDataInfo.second));
      }
      else
      {
        itsExtraMacroParamData.MultiParam3(MultiParamData(variableDataInfo.second));
      }
      return true;
    }
    else
    {
      // Muuten oletetaan että kyse on polusta haluttuun macroParam skriptiin, jota käytetään
      // parametrina
      auto absolutePathToMacroParam =
          PathUtils::getAbsoluteFilePath(parameterStr, GetUsedAbsoluteBasePath());
      // Pitää varmistaa että macroParamin polku tehdään oikein, eli ei saa olla .st päätettä,
      // eikä saa olla ilman .dpa päätettä.
      NFmiFileString fileString(absolutePathToMacroParam);
      fileString.Extension("dpa");
      absolutePathToMacroParam = fileString;

      if (NFmiFileSystem::FileExists(absolutePathToMacroParam))
      {
        if (multiParamId == NFmiAreaMask::MultiParam2)
        {
          itsExtraMacroParamData.MultiParam2(
              MultiParamData(parameterStr, absolutePathToMacroParam));
        }
        else
        {
          itsExtraMacroParamData.MultiParam3(
              MultiParamData(parameterStr, absolutePathToMacroParam));
        }
        return true;
      }
    }
  }

  std::string multiParamName =
      (multiParamId == NFmiAreaMask::MultiParam2) ? "MultiParam2" : "MultiParam3";

  // Jos löytyi MultiParam2/3 -lauseke, mutta muuten ehdot eivät täyttyneet, tehdään virheilmoitus.
  std::string errorStr = "Given '" + multiParamName + "' operation was illegal";
  errorStr += "\n";
  errorStr += ::GetDictionaryString("Try something like following");
  errorStr += ":\n";
  errorStr += multiParamName + " = T_ec (Ecmwf's surface data Temperature parameter)";
  errorStr += "\n";
  errorStr += multiParamName +
              " = par4_prod240_850 (par4 (Temperature) from producer with id "
              "240 "
              "from pressure level 850 hPa)";
  errorStr += "\nOR\n";
  errorStr += multiParamName + " = path_to_macroParam";
  throw std::runtime_error(errorStr);
}

// Numero voi koostua kahdesta tokenista, merkistä ja itse numerosta.
// Tämä metodi varmistaa että se ottaa kokonaisen numeron stringin.
std::string NFmiSmartToolIntepreter::GetWholeNumberFromTokens()
{
  GetToken();
  string numberStr = token;
  if (numberStr == "-" || numberStr == "+")
  {
    GetToken();
    numberStr += token;
  }
  return numberStr;
}

const std::string gCalculationPointErrorStart =
    "\"CalculationPoint = lat,lon\" operation was given illegal";

bool NFmiSmartToolIntepreter::ExtractCalculationPointInfo()
{
  // Haluttu laskenta piste kerrotaan seuraavanlaisilla lausekkeilla
  // calculationpoint = 60.1,24.9
  // Laskentapisteet otetaan talteen itsExtraMacroParamData -olioon.

  GetToken();
  string assignOperator = token;
  if (assignOperator == string("="))
  {
    string latitudeStr = GetWholeNumberFromTokens();

    // Kokeillaan onko annettu tuottaja, jonka datasta asemat otetaan (pitää hanskata poikkeukset,
    // että voidaan tarvittaessa jatkaa)
    try
    {
      if (itsExtraMacroParamData.AddCalculationPointProducer(
              NFmiSmartToolIntepreter::GetPossibleProducerInfo(latitudeStr).first))
      {
        return true;
      }
    }
    catch (...)
    {
    }

    GetToken();
    string commaOperator = token;
    if (commaOperator == string(","))
    {
      string longitudeStr = GetWholeNumberFromTokens();
      try
      {
        double latitude = NFmiStringTools::Convert<double>(latitudeStr);
        double longitude = NFmiStringTools::Convert<double>(longitudeStr);
        if (latitude >= -90 && latitude <= 90)
        {
          if (longitude >= -180 && longitude <= 360)
          {
            NFmiPoint latlon(longitude, latitude);
            itsExtraMacroParamData.AddCalculationPoint(latlon);
            return true;
          }
          else
            throw std::runtime_error(gCalculationPointErrorStart +
                                     " lon value.\nValue must be between -180 and 360 degrees.");
        }
        else
          throw std::runtime_error(gCalculationPointErrorStart +
                                   " lat value.\nValue must be between -90 and 90 degrees.");
      }
      catch (std::exception &e)
      {
        std::string errorStr = gCalculationPointErrorStart + " lat/lon point:\n";
        errorStr += e.what();
        throw std::runtime_error(errorStr);
      }
    }
  }

  std::string errorStr = gCalculationPointErrorStart + " values, try something like this:\n";
  errorStr += "\"CalculationPoint = 60.1,24.9\"";
  errorStr += " or ";
  errorStr += "\"CalculationPoint = synop\\metar\\other_producer\"";
  throw std::runtime_error(errorStr);
}

bool NFmiSmartToolIntepreter::ExtractObservationRadiusInfo()
{
  // Jos skriptistä on löytynyt 'ObservationRadius = xxx'
  GetToken();
  string assignOperator = token;
  if (assignOperator == string("="))
  {
    string obsRadiusStr = GetWholeNumberFromTokens();
    try
    {
      float obsRadiusInKm = NFmiStringTools::Convert<float>(obsRadiusStr);
      itsExtraMacroParamData.ObservationRadiusInKm(obsRadiusInKm);
      return true;
    }
    catch (...)
    {
    }
  }

  std::string errorStr = "Given ObservationRadius -clause was illegal, try something like this:\n";
  errorStr += "\"ObservationRadius = 20 \\\\ [km]\"";
  throw std::runtime_error(errorStr);
}

bool NFmiSmartToolIntepreter::ExtractWorkingThreadCount()
{
  // Jos skriptistä on löytynyt 'WorkingThreadCount = xxx'
  GetToken();
  string assignOperator = token;
  if (assignOperator == string("="))
  {
    string workingThreadCountStr = GetWholeNumberFromTokens();
    try
    {
      int workingThreadCount = NFmiStringTools::Convert<int>(workingThreadCountStr);
      itsExtraMacroParamData.WorkingThreadCount(workingThreadCount);
      return true;
    }
    catch (...)
    {
    }
  }

  std::string errorStr = "Given WorkingThreadCount -clause was illegal, try something like this:\n";
  errorStr += "\"WorkingThreadCount = 2\"";
  throw std::runtime_error(errorStr);
}

const std::string &NFmiSmartToolIntepreter::GetUsedAbsoluteBasePath() const
{
  if (fMacroParamSkriptInProgress)
    return itsAbsoluteMacroParamBasePath;
  else
    return itsAbsoluteSmarttoolsBasePath;
}

// Smarttools kielessä voidaan antaa polkuja #include lausekkeilla ja SymbolTooltipFile = path
// lausekkeilla. 1) Tämä funktio muuttaa suhteelliset polut absoluuttisiksi, esim:
//    path1\path2\file => absoluteBasePath\path1\path2\file
// 2) Siivoaa poluista .. -notaatio siirtymät ja muuttaa ne vain absoluuttisiksi, esim.
//    ..\..\myscripts\myscript.st => D:\macrot\myscripts\myscript.st
// , jos basepath (macroparameille) oli vaikka D:\macrot\fmi\macroparams
std::string NFmiSmartToolIntepreter::FixGivenSmarttoolsScriptPath(
    const std::string &thePathInScript) const
{
  auto absolutePathInscript =
      PathUtils::getAbsoluteFilePath(thePathInScript, GetUsedAbsoluteBasePath());
  return PathUtils::simplifyWindowsPath(absolutePathInscript);
}

bool NFmiSmartToolIntepreter::ExtractSymbolTooltipFile(bool multiParamCase)
{
  // Jos skriptistä on löytynyt 'SymbolTooltipFile = path_to_file' tai 'MultiParamTooltipFile =
  // path_to_file'
  std::string tooltipFileType = multiParamCase ? "MultiParamTooltipFile" : "SymbolTooltipFile";
  GetToken();
  string assignOperator = token;
  if (assignOperator == string("="))
  {
    // Haetaan teksti rivin loppuun asti poluksi (polussa voi olla space:ja)
    string pathToSymbolFile = std::string(exp_ptr, exp_end);
    // otetään edessä ja mahdolliset perässä olevat spacet pois
    NFmiStringTools::Trim(pathToSymbolFile);
    pathToSymbolFile = FixGivenSmarttoolsScriptPath(pathToSymbolFile);
    if (NFmiFileSystem::FileExists(pathToSymbolFile))
    {
      if (multiParamCase)
      {
        itsExtraMacroParamData.MultiParamTooltipFile(pathToSymbolFile);
      }
      else
      {
        itsExtraMacroParamData.SymbolTooltipFile(pathToSymbolFile);
      }
      return true;
    }
    else
    {
      std::string errorStr = "Given " + tooltipFileType + " doesn't exist: ";
      errorStr += pathToSymbolFile;
      throw std::runtime_error(errorStr);
    }
  }

  std::string errorStr =
      "Given " + tooltipFileType + " -clause was illegal, try something like this:\n";
  errorStr += "\"" + tooltipFileType + " = path_to_file\"";
  throw std::runtime_error(errorStr);
}

bool NFmiSmartToolIntepreter::ExtractMacroParamDescription()
{
  // Jos skriptistä on löytynyt 'MacroParamDescription = description-text'
  GetToken();
  std::string assignOperator = token;
  if (assignOperator == string("="))
  {
    // Haetaan teksti rivin loppuun asti description:iksi
    std::string descriptionText = std::string(exp_ptr, exp_end);
    // otetään edessä ja mahdolliset perässä olevat spacet pois
    NFmiStringTools::Trim(descriptionText);
    itsExtraMacroParamData.MacroParamDescription(descriptionText);
    return true;
  }

  std::string errorStr =
      "Given MacroParamDescription -clause was illegal, try something like this:\n";
  errorStr += "\"MacroParamDescription = description-texts\"";
  throw std::runtime_error(errorStr);
}

bool NFmiSmartToolIntepreter::ExtractCalculationType()
{
  // Jos skriptistä on löytynyt esim. 'CalculationType = index'
  GetToken();
  std::string assignOperator = token;
  if (assignOperator == string("="))
  {
    // Haetaan teksti rivin loppuun asti description:iksi
    std::string calculationTypeText = std::string(exp_ptr, exp_end);
    // otetään edessä ja mahdolliset perässä olevat spacet pois
    NFmiStringTools::Trim(calculationTypeText);
    if (boost::iequals(calculationTypeText, "index"))
    {
      itsExtraMacroParamData.CalculationType(MacroParamCalculationType::Index);
      return true;
    }
  }

  std::string errorStr = "Given CalculationType -clause was illegal, try something like this:\n";
  errorStr += "\"CalculationType = index\"";
  throw std::runtime_error(errorStr);
}

bool NFmiSmartToolIntepreter::IsVariableExtraInfoCommand(const std::string &theVariableText)
{
  std::string aVariableText(theVariableText);
  NFmiStringTools::LowerCase(aVariableText);  // Tässä tarkastellaan case insensitiivisesti
  FunctionMap::iterator it = itsExtraInfoCommands.find(aVariableText);
  if (it != itsExtraInfoCommands.end())
  {
    if (it->second == NFmiAreaMask::Resolution)
      return ExtractResolutionInfo();
    else if (it->second == NFmiAreaMask::CalculationPoint)
      return ExtractCalculationPointInfo();
    else if (it->second == NFmiAreaMask::ObservationRadius)
      return ExtractObservationRadiusInfo();
    else if (it->second == NFmiAreaMask::SymbolTooltipFile)
      return ExtractSymbolTooltipFile(false);
    else if (it->second == NFmiAreaMask::MacroParamDescription)
      return ExtractMacroParamDescription();
    else if (it->second == NFmiAreaMask::CalculationType)
      return ExtractCalculationType();
    else if (it->second == NFmiAreaMask::WorkingThreadCount)
      return ExtractWorkingThreadCount();
    else if (it->second == NFmiAreaMask::FixedBaseData)
      return ExtractFixedBaseData();
    else if (it->second == NFmiAreaMask::MultiParamTooltipFile)
      return ExtractSymbolTooltipFile(true);
    else if (it->second == NFmiAreaMask::MultiParam2 || it->second == NFmiAreaMask::MultiParam3)
      return ExtractMultiParam(it->second);
  }
  return false;
}

std::string NFmiSmartToolIntepreter::HandlePossibleUnaryMarkers(const std::string &theCurrentString)
{
  string returnStr(theCurrentString);
  if (returnStr == string("-"))
  {
    GetToken();
    returnStr += token;  // lisätään '-'-etumerkki ja seuraava token ja katsotaan mitä syntyy
  }
  if (returnStr == string("+"))
    GetToken();  // +-merkki ohitetaan merkityksettömänä
  return returnStr;
}

// Nämä ovat muotoa:
// RU(T 1 3)
// eli 1. maskin/funktion nimi, sulku auki, parametri, alkuarvo, loppuarvo ja lopuksi suliku kiinni.
bool NFmiSmartToolIntepreter::IsVariableRampFunction(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  if (FindAnyFromText(theVariableText, itsTokenRampFunctions))
  {
    string tmp;
    std::vector<pair<string, types> > tokens;
    int i;
    for (i = 0; i < 5 && GetToken(); i++)
    {
      tmp = token;  // luetaan muuttuja/vakio/funktio tai mikä lie
      tmp = HandlePossibleUnaryMarkers(tmp);
      tokens.push_back(std::make_pair(tmp, tok_type));
      if (tmp == string(")"))  // etsitään lopetus sulkua
        break;
    }
    if (i == 4 && (tokens[0].first == string("(")) && (tokens[4].first == string(")")))
    {
      if (tokens[1].second == VARIABLE && tokens[2].second == NUMBER && tokens[3].second == NUMBER)
      {
        InterpretVariable(tokens[1].first, theMaskInfo);
        NFmiAreaMask::CalculationOperationType type = theMaskInfo->GetOperationType();
        if (type == NFmiAreaMask::InfoVariable || type == NFmiAreaMask::CalculatedVariable)
        {
          theMaskInfo->SetOperationType(NFmiAreaMask::RampFunction);
          NFmiValueString valueString1(tokens[2].first);
          double value1 = static_cast<double>(valueString1);
          NFmiValueString valueString2(tokens[3].first);
          double value2 = static_cast<double>(valueString2);
          FmiMaskOperation maskOper = kFmiNoMaskOperation;
          if (FindAnyFromText(theVariableText, itsTokenRampUpFunctions))
            maskOper = kFmiMaskRisingRamp;
          else if (FindAnyFromText(theVariableText, itsTokenRampDownFunctions))
            maskOper = kFmiMaskLoweringRamp;
          else if (FindAnyFromText(theVariableText, itsTokenDoubleRampFunctions))
            maskOper = kFmiMaskDoubleRamp;
          NFmiCalculationCondition condition(maskOper, value1, value2);
          theMaskInfo->SetMaskCondition(condition);

          return true;
        }
      }
    }
    throw runtime_error(::GetDictionaryString("SmartToolErrorRampFunctionParams") + ": " +
                        theVariableText);
  }
  return false;
}

bool NFmiSmartToolIntepreter::IsVariableBinaryOperator(
    const std::string &theVariableText, boost::shared_ptr<NFmiAreaMaskInfo> &theMaskInfo)
{
  BinaOperMap::iterator it = itsBinaryOperator.find(theVariableText);
  if (it != itsBinaryOperator.end())
  {
    theMaskInfo->SetOperationType(NFmiAreaMask::BinaryOperatorType);
    theMaskInfo->SetBinaryOperator((*it).second);
    return true;
  }
  return false;
}

NFmiParam NFmiSmartToolIntepreter::GetParamFromString(const std::string &theParamText)
{
  NFmiParam param;

  std::string tmp(theParamText);
  ParamMap::iterator it = itsTokenParameterNamesAndIds.find(NFmiStringTools::LowerCase(tmp));
  if (it == itsTokenParameterNamesAndIds.end())
  {
    if (!NFmiSmartToolIntepreter::GetParamFromVariableById(theParamText, param))
      throw runtime_error(::GetDictionaryString("SmartToolErrorWantedParam1") + " '" +
                          theParamText + "' " +
                          ::GetDictionaryString("SmartToolErrorWantedParam2"));
  }
  else
    param = NFmiParam((*it).second,
                      (*it).first,
                      kFloatMissing,
                      kFloatMissing,
                      kFloatMissing,
                      kFloatMissing,
                      "%.1f",
                      kLinearly);
  return param;
}

//--------------------------------------------------------
// ExtractFirstCalculationSection
//--------------------------------------------------------
// Metodi saa parametrina stripatun macro-tekstin ja etsii siitä 1. calculation sectionin.
// Jos löytyy, palauta sen positio, jos ei löydy, palauta string.end().
// Parametrina tulee aloitus positio (merkitsee enemmän muissa vastaavissa metodeissa).
// Tee myös seuraavat metodit:
// ExtractNextCalculationSection
// ExtractIfMaskSection
// ExtractNextElseIfMaskSection
// ExtractElseCalculationSection

std::string::const_iterator NFmiSmartToolIntepreter::ExtractFirstCalculationSection(
    const std::string &theMacroText, std::string::iterator theStartPosition)
{
  return theMacroText.end();
}

void NFmiSmartToolIntepreter::Clear()
{
  int size = static_cast<int>(itsSmartToolCalculationBlocks.size());
  for (int i = 0; i < size; i++)
    itsSmartToolCalculationBlocks[i].Clear();
}

void NFmiSmartToolIntepreter::InitProducerTokens(NFmiProducerSystem *theProducerSystem,
                                                 NFmiInfoData::Type theDefaultDataType)
{
  if (theProducerSystem)
  {
    // Tuottaja listaa täydennetään ProducerSystemin tuottajilla
    int modelCount = static_cast<int>(theProducerSystem->Producers().size());
    for (int i = 0; i < modelCount; i++)
    {
      NFmiProducerInfo &prodInfo = theProducerSystem->Producer(i + 1);
      for (size_t j = 0; j < prodInfo.ShortNameCount(); j++)
      {
        std::string prodName(prodInfo.ShortName(j));
        NFmiStringTools::LowerCase(prodName);  // pitää muuttaa lower case:en!!!
        itsTokenProducerNamesAndIds.insert(
            std::make_pair(prodName,
                           std::make_pair(static_cast<FmiProducerName>(prodInfo.ProducerId()),
                                          theDefaultDataType)));
      }
    }
  }
}

// Look also used ERA interim fractile params setup from NFmiTimeSerialView.cpp
// Just before namespace ModelClimatology starts.
static void InitEraInterimParams(NFmiSmartToolIntepreter::ParamMap &paramMap)
{
  // Cape
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef100"), kFmiCAPEF100));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef99"), kFmiCAPEF99));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef975"), kFmiCAPEF97_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef95"), kFmiCAPEF95));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef875"), kFmiCAPEF87_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef50"), kFmiCAPEF50));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef125"), kFmiCAPEF12_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef5"), kFmiCAPEF5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef025"), kFmiCAPEF2_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef1"), kFmiCAPEF1));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("capef0"), kFmiCAPEF0));

  // Td (Dew point)
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf100"), kFmiDewPointF100));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf99"), kFmiDewPointF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf975"), kFmiDewPointF97_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf95"), kFmiDewPointF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf875"), kFmiDewPointF87_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf50"), kFmiDewPointF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf125"), kFmiDewPointF12_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf5"), kFmiDewPointF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf025"), kFmiDewPointF2_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf1"), kFmiDewPointF1));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tdf0"), kFmiDewPointF0));

  // Ice cover
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf100"), kFmiIceCoverF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf99"), kFmiIceCoverF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf975"), kFmiIceCoverF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf95"), kFmiIceCoverF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf875"), kFmiIceCoverF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf50"), kFmiIceCoverF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf125"), kFmiIceCoverF12_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf5"), kFmiIceCoverF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf025"), kFmiIceCoverF2_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf1"), kFmiIceCoverF1));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("icecoverf0"), kFmiIceCoverF0));

  // Maximum temperature
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf100"),
                                                                kFmiMaximumTemperatureF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf99"), kFmiMaximumTemperatureF99));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf975"),
                                                                kFmiMaximumTemperatureF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf95"), kFmiMaximumTemperatureF95));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf875"),
                                                                kFmiMaximumTemperatureF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf50"), kFmiMaximumTemperatureF50));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf125"),
                                                                kFmiMaximumTemperatureF12_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf5"), kFmiMaximumTemperatureF5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf025"),
                                                                kFmiMaximumTemperatureF2_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf1"), kFmiMaximumTemperatureF1));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tmaxf0"), kFmiMaximumTemperatureF0));

  // Minimum temperature
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf100"),
                                                                kFmiMinimumTemperatureF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf99"), kFmiMinimumTemperatureF99));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf975"),
                                                                kFmiMinimumTemperatureF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf95"), kFmiMinimumTemperatureF95));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf875"),
                                                                kFmiMinimumTemperatureF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf50"), kFmiMinimumTemperatureF50));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf125"),
                                                                kFmiMinimumTemperatureF12_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf5"), kFmiMinimumTemperatureF5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf025"),
                                                                kFmiMinimumTemperatureF2_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf1"), kFmiMinimumTemperatureF1));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tminf0"), kFmiMinimumTemperatureF0));

  // Precipitation 3h
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf100"), kFmiPrecipitation3hF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf99"), kFmiPrecipitation3hF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf975"), kFmiPrecipitation3hF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf95"), kFmiPrecipitation3hF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf875"), kFmiPrecipitation3hF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf50"), kFmiPrecipitation3hF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf125"), kFmiPrecipitation3hF12_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf5"), kFmiPrecipitation3hF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf025"), kFmiPrecipitation3hF2_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf1"), kFmiPrecipitation3hF1));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("rr3hf0"), kFmiPrecipitation3hF0));

  // Pressure
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("pf100"), kFmiPressureF100));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("pf99"), kFmiPressureF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("pf975"), kFmiPressureF97_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("pf95"), kFmiPressureF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("pf875"), kFmiPressureF87_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("pf50"), kFmiPressureF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("pf125"), kFmiPressureF12_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("pf5"), kFmiPressureF5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("pf025"), kFmiPressureF2_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("pf1"), kFmiPressureF1));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("pf0"), kFmiPressureF0));

  // Snow depth
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf100"), kFmiSnowDepthF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf99"), kFmiSnowDepthF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf975"), kFmiSnowDepthF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf95"), kFmiSnowDepthF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf875"), kFmiSnowDepthF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf50"), kFmiSnowDepthF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf125"), kFmiSnowDepthF12_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf5"), kFmiSnowDepthF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf025"), kFmiSnowDepthF2_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf1"), kFmiSnowDepthF1));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("snowdepthf0"), kFmiSnowDepthF0));

  // Temperature
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tf100"), kFmiTemperatureF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tf99"), kFmiTemperatureF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tf975"), kFmiTemperatureF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tf95"), kFmiTemperatureF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tf875"), kFmiTemperatureF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tf50"), kFmiTemperatureF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tf125"), kFmiTemperatureF12_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tf5"), kFmiTemperatureF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tf025"), kFmiTemperatureF2_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tf1"), kFmiTemperatureF1));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("tf0"), kFmiTemperatureF0));

  // Temperature sea
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf100"), kFmiTemperatureSeaF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf99"), kFmiTemperatureSeaF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf975"), kFmiTemperatureSeaF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf95"), kFmiTemperatureSeaF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf875"), kFmiTemperatureSeaF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf50"), kFmiTemperatureSeaF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf125"), kFmiTemperatureSeaF12_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf5"), kFmiTemperatureSeaF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf025"), kFmiTemperatureSeaF2_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf1"), kFmiTemperatureSeaF1));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tseaf0"), kFmiTemperatureSeaF0));

  // Total cloud cover (N)
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf100"), kFmiTotalCloudCoverF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf99"), kFmiTotalCloudCoverF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf975"), kFmiTotalCloudCoverF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf95"), kFmiTotalCloudCoverF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf875"), kFmiTotalCloudCoverF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf50"), kFmiTotalCloudCoverF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf125"), kFmiTotalCloudCoverF12_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf5"), kFmiTotalCloudCoverF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf025"), kFmiTotalCloudCoverF2_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf1"), kFmiTotalCloudCoverF1));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("nf0"), kFmiTotalCloudCoverF0));

  // Total Column Water (TCW)
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf100"), kFmiTotalColumnWaterF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf99"), kFmiTotalColumnWaterF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf975"), kFmiTotalColumnWaterF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf95"), kFmiTotalColumnWaterF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf875"), kFmiTotalColumnWaterF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf50"), kFmiTotalColumnWaterF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf125"), kFmiTotalColumnWaterF12_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf5"), kFmiTotalColumnWaterF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf025"), kFmiTotalColumnWaterF2_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf1"), kFmiTotalColumnWaterF1));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("tcwf0"), kFmiTotalColumnWaterF0));

  // Wind gust
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf100"), kFmiWindGustF100));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf99"), kFmiWindGustF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf975"), kFmiWindGustF97_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf95"), kFmiWindGustF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf875"), kFmiWindGustF87_5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf50"), kFmiWindGustF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf125"), kFmiWindGustF12_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf5"), kFmiWindGustF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf025"), kFmiWindGustF2_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf1"), kFmiWindGustF1));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("gustf0"), kFmiWindGustF0));

  // Wind speed
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf100"), kFmiWindSpeedF100));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf99"), kFmiWindSpeedF99));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf975"), kFmiWindSpeedF97_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf95"), kFmiWindSpeedF95));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf875"), kFmiWindSpeedF87_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf50"), kFmiWindSpeedF50));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf125"), kFmiWindSpeedF12_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf5"), kFmiWindSpeedF5));
  paramMap.insert(
      NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf025"), kFmiWindSpeedF2_5));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf1"), kFmiWindSpeedF1));
  paramMap.insert(NFmiSmartToolIntepreter::ParamMap::value_type(string("wsf0"), kFmiWindSpeedF0));
}

void NFmiSmartToolIntepreter::InitTokens(NFmiProducerSystem *theProducerSystem,
                                         NFmiProducerSystem *theObservationProducerSystem)
{
  if (!NFmiSmartToolIntepreter::fTokensInitialized)
  {
    NFmiSmartToolIntepreter::fTokensInitialized = true;

    // clang-format off

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("t"), kFmiTemperature));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("p"), kFmiPressure));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("rh"), kFmiHumidity));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("tpot"), kFmiPotentialTemperature));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("thetaw"), kFmiPseudoAdiabaticPotentialTemperature));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("kind"), kFmiKIndex));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("td"), kFmiDewPoint));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("dp"), kFmiDewPoint));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lrad"), kFmiRadiationLW));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("srad"), kFmiRadiationGlobal));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("ws"), kFmiWindSpeedMS));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wd"), kFmiWindDirection));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("u"), kFmiWindUMS));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("v"), kFmiWindVMS));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wvec"), kFmiWindVectorMS));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("n"), kFmiTotalCloudCover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cl"), kFmiLowCloudCover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cm"), kFmiMediumCloudCover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("ch"), kFmiHighCloudCover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("rr"), kFmiPrecipitation1h));

#ifdef USE_POTENTIAL_VALUES_IN_EDITING
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("pref"), kFmiPotentialPrecipitationForm));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("pret"), kFmiPotentialPrecipitationType));
    // Viittaukset vanhoihin parametreihin, jotka saattavat olla käytössä mm. ulkomailla
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("pref2"), kFmiPrecipitationForm));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("pret2"), kFmiPrecipitationType));
#else
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("pref"), kFmiPrecipitationForm));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("pret"), kFmiPrecipitationType));
#endif
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("thund"), kFmiProbabilityThunderstorm));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fog"), kFmiFogIntensity));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("hsade"), kFmiWeatherSymbol1));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("hessaa"), kFmiWeatherSymbol3));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("w"), kFmiVerticalVelocityMMS));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("z"), kFmiGeomHeight));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("geom"), kFmiGeomHeight));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("geop"), kFmiGeopHeight));

    // laitetaan kokoelma parametritkin kieleen, että niitä voi sijoittaa kerralla yhdellä komennolla
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wind"), kFmiTotalWindMS));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("weather"), kFmiWeatherAndCloudiness));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("rrcon"), kFmiPrecipitationConv));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("rrlar"), kFmiPrecipitationLarge));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape"), kFmiCAPE));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cin"), kFmiCIN));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("tke"), kFmiTurbulentKineticEnergy));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("icing"), kFmiIcing));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl1base"), kFmi_FL_1_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl1top"), kFmi_FL_1_Top));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl1cover"), kFmi_FL_1_Cover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl1cloudtype"), kFmi_FL_1_CloudType));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl2base"), kFmi_FL_2_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl2top"), kFmi_FL_2_Top));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl2cover"), kFmi_FL_2_Cover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl2cloudtype"), kFmi_FL_2_CloudType));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl3base"), kFmi_FL_3_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl3top"), kFmi_FL_3_Top));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl3cover"), kFmi_FL_3_Cover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl3cloudtype"), kFmi_FL_3_CloudType));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl4base"), kFmi_FL_4_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl4top"), kFmi_FL_4_Top));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl4cover"), kFmi_FL_4_Cover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl4cloudtype"), kFmi_FL_4_CloudType));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl5base"), kFmi_FL_5_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl5top"), kFmi_FL_5_Top));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl5cover"), kFmi_FL_5_Cover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl5cloudtype"), kFmi_FL_5_CloudType));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl6base"), kFmi_FL_6_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl6top"), kFmi_FL_6_Top));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl6cover"), kFmi_FL_6_Cover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl6cloudtype"), kFmi_FL_6_CloudType));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl7base"), kFmi_FL_7_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl7top"), kFmi_FL_7_Top));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl7cover"), kFmi_FL_7_Cover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl7cloudtype"), kFmi_FL_7_CloudType));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl8base"), kFmi_FL_8_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl8top"), kFmi_FL_8_Top));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl8cover"), kFmi_FL_8_Cover));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("fl8cloudtype"), kFmi_FL_8_CloudType));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("flcbbase"), kFmi_FL_Cb_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("flcbcover"), kFmi_FL_Cb_Cover));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("flminbase"), kFmi_FL_Min_Base));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("flmaxbase"), kFmi_FL_Max_Base));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("avivis"), kFmiAviationVisibility));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("vervis"), kFmiVerticalVisibility));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("pop"), kFmiPoP));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("mist"), kFmiMist));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("pseudosatel"), kFmiRadiationNetTopAtmLW));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("vis"), kFmiVisibility));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("gust"), kFmiHourlyMaximumGust));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wsmax"), kFmiHourlyMaximumWindSpeed));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("boundarylh"), kFmiMixedLayerDepth));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("snowfall"), kFmiSnowAccumulation));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cloudwater"), kFmiCloudWater));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cloudice"), static_cast<FmiParameterName>(277)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("stationp"), kFmiPressureAtStationLevel));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("zeroh"), kFmiFreezingLevel));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("snowdepth"), kFmiWaterEquivalentOfSnow));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("icecover"), kFmiIceCover));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wso1"), kFmiProbabilityOfWindLimit1));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wso2"), kFmiProbabilityOfWindLimit2));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wso3"), kFmiProbabilityOfWindLimit3));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wso4"), kFmiProbabilityOfWindLimit4));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wso5"), kFmiProbabilityOfWindLimit5));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("wso6"), kFmiProbabilityOfWindLimit6));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("icing"), kFmiIcing));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("qnh"), static_cast<FmiParameterName>(1207)));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("messagetype"), kFmiHakeMessageType));

    // ****** sounding index funktiot  HUOM! ne käsitellään case insensitiveinä!! *************************
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lclsur"), static_cast<FmiParameterName>(kSoundingParLCLSur)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lfcsur"), static_cast<FmiParameterName>(kSoundingParLFCSur)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("elsur"), static_cast<FmiParameterName>(kSoundingParELSur)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lclheightsur"), static_cast<FmiParameterName>(kSoundingParLCLHeightSur)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lfcheightsur"), static_cast<FmiParameterName>(kSoundingParLFCHeightSur)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("elheightsur"), static_cast<FmiParameterName>(kSoundingParELHeightSur)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("capesur"), static_cast<FmiParameterName>(kSoundingParCAPESur)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape03kmsur"), static_cast<FmiParameterName>(kSoundingParCAPE0_3kmSur)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape1040sur"), static_cast<FmiParameterName>(kSoundingParCAPE_TT_Sur)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cinsur"), static_cast<FmiParameterName>(kSoundingParCINSur)));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lcl500m"), static_cast<FmiParameterName>(kSoundingParLCL500m)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lfc500m"), static_cast<FmiParameterName>(kSoundingParLFC500m)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("el500m"), static_cast<FmiParameterName>(kSoundingParEL500m)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lclheight500m"), static_cast<FmiParameterName>(kSoundingParLCLHeight500m)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lfcheight500m"), static_cast<FmiParameterName>(kSoundingParLFCHeight500m)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("elheight500m"), static_cast<FmiParameterName>(kSoundingParELHeight500m)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape500m"), static_cast<FmiParameterName>(kSoundingParCAPE500m)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape03km500m"), static_cast<FmiParameterName>(kSoundingParCAPE0_3km500m)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape1040500m"), static_cast<FmiParameterName>(kSoundingParCAPE_TT_500m)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cin500m"), static_cast<FmiParameterName>(kSoundingParCIN500m)));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lclmostun"), static_cast<FmiParameterName>(kSoundingParLCLMostUn)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lfcmostun"), static_cast<FmiParameterName>(kSoundingParLFCMostUn)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("elmostun"), static_cast<FmiParameterName>(kSoundingParELMostUn)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lclheightmostun"), static_cast<FmiParameterName>(kSoundingParLCLHeightMostUn)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lfcheightmostun"), static_cast<FmiParameterName>(kSoundingParLFCHeightMostUn)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("elheightmostun"), static_cast<FmiParameterName>(kSoundingParELHeightMostUn)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("capemostun"), static_cast<FmiParameterName>(kSoundingParCAPEMostUn)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape03kmmostun"), static_cast<FmiParameterName>(kSoundingParCAPE0_3kmMostUn)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape1040mostun"), static_cast<FmiParameterName>(kSoundingParCAPE_TT_MostUn)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cinmostun"), static_cast<FmiParameterName>(kSoundingParCINMostUn)));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lclobsbas"), static_cast<FmiParameterName>(kSoundingParLCLSurBas)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lfcobsbas"), static_cast<FmiParameterName>(kSoundingParLFCSurBas)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("elobsbas"), static_cast<FmiParameterName>(kSoundingParELSurBas)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lclheightobsbas"), static_cast<FmiParameterName>(kSoundingParLCLHeightSurBas)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lfcheightobsbas"), static_cast<FmiParameterName>(kSoundingParLFCHeightSurBas)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("elheightobsbas"), static_cast<FmiParameterName>(kSoundingParELHeightSurBas)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("capeobsbas"), static_cast<FmiParameterName>(kSoundingParCAPESurBas)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape03kmobsbas"), static_cast<FmiParameterName>(kSoundingParCAPE0_3kmSurBas)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cape1040obsbas"), static_cast<FmiParameterName>(kSoundingParCAPE_TT_SurBas)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("cinobsbas"), static_cast<FmiParameterName>(kSoundingParCINSurBas)));

    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("show"), static_cast<FmiParameterName>(kSoundingParSHOW)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("lift"), static_cast<FmiParameterName>(kSoundingParLIFT)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("kinx"), static_cast<FmiParameterName>(kSoundingParKINX)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("ctot"), static_cast<FmiParameterName>(kSoundingParCTOT)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("vtot"), static_cast<FmiParameterName>(kSoundingParVTOT)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("totl"), static_cast<FmiParameterName>(kSoundingParTOTL)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("bulkshear06km"), static_cast<FmiParameterName>(kSoundingParBS0_6km)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("bulkshear01km"), static_cast<FmiParameterName>(kSoundingParBS0_1km)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("srh03km"), static_cast<FmiParameterName>(kSoundingParSRH0_3km)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("srh01km"), static_cast<FmiParameterName>(kSoundingParSRH0_1km)));
    itsTokenParameterNamesAndIds.insert(ParamMap::value_type(string("thetae03km"), static_cast<FmiParameterName>(kSoundingParThetaE0_3km)));
    // ****** sounding index funktiot *************************
    ::InitEraInterimParams(itsTokenParameterNamesAndIds);

    itsTokenStaticParameterNamesAndIds.insert(ParamMap::value_type(string("topo"), kFmiTopoGraf));
    itsTokenStaticParameterNamesAndIds.insert(ParamMap::value_type(string("slope"), kFmiTopoSlope));
    itsTokenStaticParameterNamesAndIds.insert(ParamMap::value_type(string("slopedir"), kFmiTopoAzimuth));
    itsTokenStaticParameterNamesAndIds.insert(ParamMap::value_type(string("distsea"), kFmiTopoDistanceToSea));
    itsTokenStaticParameterNamesAndIds.insert(ParamMap::value_type(string("dirsea"), kFmiTopoDirectionToSea));
    itsTokenStaticParameterNamesAndIds.insert(ParamMap::value_type(string("distland"), kFmiTopoDistanceToLand));
    itsTokenStaticParameterNamesAndIds.insert(ParamMap::value_type(string("dirland"), kFmiTopoDirectionToLand));
    itsTokenStaticParameterNamesAndIds.insert(ParamMap::value_type(string("landseemask"), kFmiLandSeaMask));
    itsTokenStaticParameterNamesAndIds.insert(ParamMap::value_type(string("reltopo"), kFmiTopoRelativeHeight));

    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("lat"), kFmiLatitude));
    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("lon"), kFmiLongitude));
    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("eangle"), kFmiElevationAngle));
    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("jday"), kFmiDay));  // julian day oikeasti
    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("lhour"), kFmiHour));  // local hour oikeasti
    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("utchour"), kFmiSecond));  // utc hour käyttää secondia, koska ei ollut omaa parametria utc hourille enkä lisää sellaista
    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("fhour"), kFmiForecastPeriod));  // forecast hour pikaviritys forperiodia käytetty, koska ei ollut valmista parametria
    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("minute"), kFmiMinute));  // kyseisen ajanhetken minuutit, aina samat 0-59 riippumatta oliko kyse lokaali, utc tai forecast timesta
    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("timestep"), kFmiDeltaTime));  // TIMESTEP eli timestep palauttaa datan currentin ajan aika stepin tunneissa

    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("gridsizex"), kFmiLastParameter));  // hilan x suuntainen koko metreissä (muokattavan datan tai macroParam hilan koko)
    itsTokenCalculatedParameterNamesAndIds.insert(ParamMap::value_type(string("gridsizey"), static_cast<FmiParameterName>(kFmiLastParameter + 1)));  // hilan y suuntainen koko metreissä (muokattavan datan tai macroParam hilan koko)

    // Alustetaan ensin tuottaja listaan muut tarvittavat tuottajat, Huom! nimi pienellä, koska
    // tehdään case insensitiivejä tarkasteluja!!
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("met"), std::make_pair(static_cast<FmiProducerName>(999), NFmiInfoData::kKepaData)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("orig"), std::make_pair(kFmiMETEOR, NFmiInfoData::kCopyOfEdited)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("anal"), std::make_pair(static_cast<FmiProducerName>(gMesanProdId), NFmiInfoData::kAnalyzeData)));  // analyysi mesan tuottaja
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("ana"), std::make_pair(static_cast<FmiProducerName>(gMesanProdId), NFmiInfoData::kAnalyzeData)));  // analyysi mesan tuottaja
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("help"), std::make_pair(static_cast<FmiProducerName>(NFmiProducerSystem::gHelpEditorDataProdId), NFmiInfoData::kEditingHelpData)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("ec3vrk"), std::make_pair(static_cast<FmiProducerName>(NFmiInfoData::kFmiSpEcmwf3Vrk), NFmiInfoData::kModelHelpData)));

    // havainto datoja
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("synop"), std::make_pair(kFmiSYNOP, NFmiInfoData::kObservations)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("synopx"), std::make_pair(static_cast<FmiProducerName>(NFmiInfoData::kFmiSpSynoXProducer), NFmiInfoData::kObservations)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("metar"), std::make_pair(kFmiMETAR, NFmiInfoData::kObservations)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("wxt"), std::make_pair(kFmiTestBed, NFmiInfoData::kObservations)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("road"), std::make_pair(static_cast<FmiProducerName>(20013), NFmiInfoData::kObservations))); // kFmiRoadObs));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("temp"), std::make_pair(kFmiTEMP, NFmiInfoData::kObservations)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("temp"), std::make_pair(kFmiBufrTEMP, NFmiInfoData::kObservations)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("nrd"), std::make_pair(kFmiRADARNRD, NFmiInfoData::kObservations)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("hake"), std::make_pair(kFmiHakeMessages, NFmiInfoData::kObservations)));
    itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("kaha"), std::make_pair(kFmiKaHaMessages, NFmiInfoData::kObservations)));

    if(theProducerSystem)
    {
        NFmiSmartToolIntepreter::InitProducerTokens(theProducerSystem, NFmiInfoData::kViewable);
        NFmiSmartToolIntepreter::InitProducerTokens(theObservationProducerSystem, NFmiInfoData::kObservations);
    }
    else
    {
        // tästä pitäisi varoittaa ja heittää poikkeus, mutta aina ei voi alustaa smarttool-systeemiä
        // producersystemillä (esim. SmarttoolFilter ei tiedä moisesta mitään!!)
        //			throw std::runtime_error("NFmiSmartToolIntepreter::InitTokens - Was
        // not initialized correctly, ProducerSystem missing, error in program, report it!");

        // joten pakko alustaa tämä tässä tapauksessa sitten jollain hardcode tuottajilla
        itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("hir"), std::make_pair(kFmiMTAHIRLAM, NFmiInfoData::kViewable)));
        itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("ec"), std::make_pair(kFmiMTAECMWF, NFmiInfoData::kViewable)));
        itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("mbe"), std::make_pair(kFmiMTAHIRMESO, NFmiInfoData::kViewable)));
        itsTokenProducerNamesAndIds.insert(ProducerMap::value_type(string("ala"), std::make_pair(static_cast<FmiProducerName>(555), NFmiInfoData::kViewable)));  // aladdinille laitetaan pikaviritys 555
    }

    itsTokenConstants.insert(ConstantMap::value_type(string("miss"), kFloatMissing));
    itsTokenConstants.insert(ConstantMap::value_type(string("pi"), 3.14159265358979));

    itsTokenIfCommands.push_back(string("IF"));
    itsTokenIfCommands.push_back(string("if"));
    itsTokenIfCommands.push_back(string("If"));

    itsTokenElseIfCommands.push_back(string("ELSEIF"));
    itsTokenElseIfCommands.push_back(string("elseif"));
    itsTokenElseIfCommands.push_back(string("ElseIf"));

    itsTokenElseCommands.push_back(string("ELSE"));
    itsTokenElseCommands.push_back(string("else"));
    itsTokenElseCommands.push_back(string("Else"));

    // tehdaan yhtenainen ehto komento joukko erilaisiin tarkastuksiin
    itsTokenConditionalCommands.insert(itsTokenConditionalCommands.end(), itsTokenIfCommands.begin(), itsTokenIfCommands.end());
    itsTokenConditionalCommands.insert(itsTokenConditionalCommands.end(), itsTokenElseIfCommands.begin(), itsTokenElseIfCommands.end());
    itsTokenConditionalCommands.insert(itsTokenConditionalCommands.end(), itsTokenElseCommands.begin(), itsTokenElseCommands.end());

    itsTokenMaskOperations.insert(MaskOperMap::value_type(string("="), kFmiMaskEqual));
    itsTokenMaskOperations.insert(MaskOperMap::value_type(string("=="), kFmiMaskEqual));
    itsTokenMaskOperations.insert(MaskOperMap::value_type(string("!="), kFmiMaskNotEqual));
    itsTokenMaskOperations.insert(MaskOperMap::value_type(string("<>"), kFmiMaskNotEqual));
    itsTokenMaskOperations.insert(MaskOperMap::value_type(string(">"), kFmiMaskGreaterThan));
    itsTokenMaskOperations.insert(MaskOperMap::value_type(string("<"), kFmiMaskLessThan));
    itsTokenMaskOperations.insert(MaskOperMap::value_type(string(">="), kFmiMaskGreaterOrEqualThan));
    itsTokenMaskOperations.insert(MaskOperMap::value_type(string("<="), kFmiMaskLessOrEqualThan));

    itsBinaryOperator.insert(BinaOperMap::value_type(string("&&"), NFmiAreaMask::kAnd));
    itsBinaryOperator.insert(BinaOperMap::value_type(string("And"), NFmiAreaMask::kAnd));
    itsBinaryOperator.insert(BinaOperMap::value_type(string("and"), NFmiAreaMask::kAnd));
    itsBinaryOperator.insert(BinaOperMap::value_type(string("AND"), NFmiAreaMask::kAnd));
    itsBinaryOperator.insert(BinaOperMap::value_type(string("||"), NFmiAreaMask::kOr));
    itsBinaryOperator.insert(BinaOperMap::value_type(string("Or"), NFmiAreaMask::kOr));
    itsBinaryOperator.insert(BinaOperMap::value_type(string("or"), NFmiAreaMask::kOr));
    itsBinaryOperator.insert(BinaOperMap::value_type(string("OR"), NFmiAreaMask::kOr));

    itsCalculationOperations.insert(CalcOperMap::value_type(string("+"), NFmiAreaMask::Add));
    itsCalculationOperations.insert(CalcOperMap::value_type(string("-"), NFmiAreaMask::Sub));
    itsCalculationOperations.insert(CalcOperMap::value_type(string("/"), NFmiAreaMask::Div));
    itsCalculationOperations.insert(CalcOperMap::value_type(string("*"), NFmiAreaMask::Mul));
    itsCalculationOperations.insert(CalcOperMap::value_type(string("^"), NFmiAreaMask::Pow));
    itsCalculationOperations.insert(CalcOperMap::value_type(string("%"), NFmiAreaMask::Mod));

    itsTokenCalculationBlockMarkers.push_back(string("{"));
    itsTokenCalculationBlockMarkers.push_back(string("}"));

    itsTokenMaskBlockMarkers.push_back(string("("));
    itsTokenMaskBlockMarkers.push_back(string(")"));

    itsTokenFunctions.insert(FunctionMap::value_type(string("avg"), NFmiAreaMask::Avg));
    itsTokenFunctions.insert(FunctionMap::value_type(string("min"), NFmiAreaMask::Min));
    itsTokenFunctions.insert(FunctionMap::value_type(string("max"), NFmiAreaMask::Max));
    itsTokenFunctions.insert(FunctionMap::value_type(string("sum"), NFmiAreaMask::Sum));
    itsTokenFunctions.insert(FunctionMap::value_type(string("med"), NFmiAreaMask::Med));
    itsTokenFunctions.insert(FunctionMap::value_type(string("modavg"), NFmiAreaMask::ModAvg));
    itsTokenFunctions.insert(FunctionMap::value_type(string("modmin"), NFmiAreaMask::ModMin));
    itsTokenFunctions.insert(FunctionMap::value_type(string("modmax"), NFmiAreaMask::ModMax));

    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("sumt"), NFmiAreaMask::Sum));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("maxt"), NFmiAreaMask::Max));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("mint"), NFmiAreaMask::Min));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("avgt"), NFmiAreaMask::Avg));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("medt"), NFmiAreaMask::Med));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("modavgt"), NFmiAreaMask::ModAvg));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("modmint"), NFmiAreaMask::ModMin));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("modmaxt"), NFmiAreaMask::ModMax));

    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("sumz"), NFmiAreaMask::Sum));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("maxz"), NFmiAreaMask::Max));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("minz"), NFmiAreaMask::Min));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("avgz"), NFmiAreaMask::Avg));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("medz"), NFmiAreaMask::Med));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("modavgz"), NFmiAreaMask::ModAvg));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("modminz"), NFmiAreaMask::ModMin));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("modmaxz"), NFmiAreaMask::ModMax));

    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("maxh"), NFmiAreaMask::Max));
    itsTokenThreeArgumentFunctions.insert(FunctionMap::value_type(string("minh"), NFmiAreaMask::Min));

    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("grad"), MetFunctionMapValue(NFmiAreaMask::Grad, NFmiAreaMask::DirectionXandY, 1, "grad(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("div"), MetFunctionMapValue(NFmiAreaMask::Divergence, NFmiAreaMask::DirectionXandY, 1, "div(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("adv"), MetFunctionMapValue(NFmiAreaMask::Adv, NFmiAreaMask::DirectionXandY, 1, "adv(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("lap"), MetFunctionMapValue(NFmiAreaMask::Lap, NFmiAreaMask::DirectionXandY, 1, "lap(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("rot"), MetFunctionMapValue(NFmiAreaMask::Rot, NFmiAreaMask::DirectionXandY, 1, "rot(wind)")));  // vain totalwind

    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("gradx"), MetFunctionMapValue(NFmiAreaMask::Grad, NFmiAreaMask::DirectionX, 1, "gradx(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("divx"), MetFunctionMapValue(NFmiAreaMask::Divergence, NFmiAreaMask::DirectionX, 1, "divx(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("advx"), MetFunctionMapValue(NFmiAreaMask::Adv, NFmiAreaMask::DirectionX, 1, "advx(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("lapx"), MetFunctionMapValue(NFmiAreaMask::Lap, NFmiAreaMask::DirectionX, 1, "lapx(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("rotx"), MetFunctionMapValue(NFmiAreaMask::Rot, NFmiAreaMask::DirectionX, 1, "rotx(wind)")));  // vain totalwind

    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("grady"), MetFunctionMapValue(NFmiAreaMask::Grad, NFmiAreaMask::DirectionY, 1, "grady(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("divy"), MetFunctionMapValue(NFmiAreaMask::Divergence, NFmiAreaMask::DirectionY, 1, "divy(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("advy"), MetFunctionMapValue(NFmiAreaMask::Adv, NFmiAreaMask::DirectionY, 1, "advy(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("lapy"), MetFunctionMapValue(NFmiAreaMask::Lap, NFmiAreaMask::DirectionY, 1, "lapy(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("roty"), MetFunctionMapValue(NFmiAreaMask::Rot, NFmiAreaMask::DirectionY, 1, "roty(wind)")));  // vain totalwind

    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("grad2"), MetFunctionMapValue(NFmiAreaMask::Grad2, NFmiAreaMask::DirectionXandY, 1, "grad2(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("div2"), MetFunctionMapValue(NFmiAreaMask::Divergence2, NFmiAreaMask::DirectionXandY, 1, "div2(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("adv2"), MetFunctionMapValue(NFmiAreaMask::Adv2, NFmiAreaMask::DirectionXandY, 1, "adv2(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("lap2"), MetFunctionMapValue(NFmiAreaMask::Lap2, NFmiAreaMask::DirectionXandY, 1, "lap2(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("rot2"), MetFunctionMapValue(NFmiAreaMask::Rot2, NFmiAreaMask::DirectionXandY, 1, "rot2(wind)")));  // vain totalwind

    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("grad2x"), MetFunctionMapValue(NFmiAreaMask::Grad2, NFmiAreaMask::DirectionX, 1, "grad2x(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("div2x"), MetFunctionMapValue(NFmiAreaMask::Divergence2, NFmiAreaMask::DirectionX, 1, "div2x(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("adv2x"), MetFunctionMapValue(NFmiAreaMask::Adv2, NFmiAreaMask::DirectionX, 1, "adv2x(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("lap2x"), MetFunctionMapValue(NFmiAreaMask::Lap2, NFmiAreaMask::DirectionX, 1, "lap2x(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("rot2x"), MetFunctionMapValue(NFmiAreaMask::Rot2, NFmiAreaMask::DirectionX, 1, "rot2x(wind)")));  // vain totalwind

    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("grad2y"), MetFunctionMapValue(NFmiAreaMask::Grad2, NFmiAreaMask::DirectionY, 1, "grad2y(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("div2y"), MetFunctionMapValue(NFmiAreaMask::Divergence2, NFmiAreaMask::DirectionY, 1, "div2y(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("adv2y"), MetFunctionMapValue(NFmiAreaMask::Adv2, NFmiAreaMask::DirectionY, 1, "adv2y(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("lap2y"), MetFunctionMapValue(NFmiAreaMask::Lap2, NFmiAreaMask::DirectionY, 1, "lap2y(param)")));
    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("rot2y"), MetFunctionMapValue(NFmiAreaMask::Rot2, NFmiAreaMask::DirectionY, 1, "rot2y(wind)")));  // vain totalwind

    itsTokenMetFunctions.insert(MetFunctionMap::value_type(string("latestvalue"), MetFunctionMapValue(NFmiAreaMask::LatestValue, NFmiAreaMask::DirectionXandY, 1, string("latestvalue(par)"))));

    // tässä on vertikaaliset-funktiot vertp-funktiot eli näitä operoidaan aina painepinnoilla [hPa]
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::VertP, 3, string("vertp_max(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::VertP, 3, string("vertp_min(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::VertP, 3, string("vertp_avg(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::VertP, 3, string("vertp_sum(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::VertP, 3, string("vertp_med(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::VertP, 3, string("vertp_modavg(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::VertP, 3, string("vertp_modmin(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::VertP, 3, string("vertp_modmax(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_get"), VertFunctionMapValue(NFmiAreaMask::Get, NFmiAreaMask::VertP, 2, string("vertp_get(par, p)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findh"), VertFunctionMapValue(NFmiAreaMask::FindH,NFmiAreaMask::VertP, 5, string("vertp_findh(par, p1, p2, value, nth)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findc"), VertFunctionMapValue(NFmiAreaMask::FindC, NFmiAreaMask::VertP, 4, string("vertp_findc(par, p1, p2, value)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_maxh"), VertFunctionMapValue(NFmiAreaMask::MaxH, NFmiAreaMask::VertP, 3, string("vertp_maxh(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_minh"), VertFunctionMapValue(NFmiAreaMask::MinH, NFmiAreaMask::VertP, 3, string("vertp_minh(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_grad"),VertFunctionMapValue(NFmiAreaMask::Grad, NFmiAreaMask::VertP, 3, string("vertp_grad(par, p1, p2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findh_cond"), VertFunctionMapValue(NFmiAreaMask::FindHeightCond, NFmiAreaMask::VertP, 4, string("vertp_findh_cond(par, p1, p2, nth, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findc_cond"), VertFunctionMapValue(NFmiAreaMask::FindCountCond, NFmiAreaMask::VertP, 3, string("vertp_findc_cond(par, p1, p2, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_peek"),VertFunctionMapValue(NFmiAreaMask::PeekZ, NFmiAreaMask::VertP, 2, string("vertp_peek(par, deltaP)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));

    // vertfl-funktiot eli näitä operoidaan aina lentopinnoilla flight-level [hft]
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::VertFL, 3, string("vertfl_max(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::VertFL, 3, string("vertfl_min(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::VertFL, 3, string("vertfl_avg(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::VertFL, 3, string("vertfl_sum(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::VertFL, 3, string("vertfl_med(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::VertFL, 3, string("vertfl_modavg(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::VertFL, 3, string("vertfl_modmin(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::VertFL, 3, string("vertfl_modmax(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_get"), VertFunctionMapValue(NFmiAreaMask::Get, NFmiAreaMask::VertFL, 2, string("vertfl_get(par, fl)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findh"), VertFunctionMapValue(NFmiAreaMask::FindH, NFmiAreaMask::VertFL,5, string("vertfl_findh(par, fl1, fl2, value, nth)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findc"), VertFunctionMapValue(NFmiAreaMask::FindC, NFmiAreaMask::VertFL, 4, string("vertfl_findc(par, fl1, fl2, value)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_maxh"), VertFunctionMapValue(NFmiAreaMask::MaxH, NFmiAreaMask::VertFL, 3, string("vertfl_maxh(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_minh"), VertFunctionMapValue(NFmiAreaMask::MinH, NFmiAreaMask::VertFL, 3, string("vertfl_minh(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_grad"), VertFunctionMapValue(NFmiAreaMask::Grad, NFmiAreaMask::VertFL, 3, string("vertfl_grad(par, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findh_cond"), VertFunctionMapValue(NFmiAreaMask::FindHeightCond, NFmiAreaMask::VertFL, 4, string("vertfl_findh_cond(par, fl1, fl2, nth, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findc_cond"), VertFunctionMapValue(NFmiAreaMask::FindCountCond, NFmiAreaMask::VertFL, 3, string("vertfl_findc_cond(par, fl1, fl2, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_peek"),VertFunctionMapValue(NFmiAreaMask::PeekZ, NFmiAreaMask::VertFL, 2, string("vertfl_peek(par, deltaFL)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));

    // vertz-funktiot eli näitä operoidaan aina metrisillä korkeuksilla [m]
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::VertZ, 3, string("vertz_max(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::VertZ, 3, string("vertz_min(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::VertZ, 3, string("vertz_avg(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::VertZ, 3, string("vertz_sum(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::VertZ, 3, string("vertz_med(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::VertZ, 3, string("vertz_modavg(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::VertZ, 3, string("vertz_modmin(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::VertZ, 3, string("vertz_modmax(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_get"), VertFunctionMapValue(NFmiAreaMask::Get, NFmiAreaMask::VertZ, 2, string("vertz_get(par, z)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findh"), VertFunctionMapValue(NFmiAreaMask::FindH, NFmiAreaMask::VertZ, 5, string("vertz_findh(par, z1, z2, value, nth)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findc"), VertFunctionMapValue(NFmiAreaMask::FindC, NFmiAreaMask::VertZ, 4, string("vertz_findc(par, z1, z2, value)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_maxh"), VertFunctionMapValue(NFmiAreaMask::MaxH, NFmiAreaMask::VertZ, 3, string("vertz_maxh(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_minh"), VertFunctionMapValue(NFmiAreaMask::MinH, NFmiAreaMask::VertZ, 3, string("vertz_minh(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_grad"), VertFunctionMapValue(NFmiAreaMask::Grad, NFmiAreaMask::VertZ, 3, string("vertz_grad(par, z1, z2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findh_cond"), VertFunctionMapValue(NFmiAreaMask::FindHeightCond, NFmiAreaMask::VertZ, 4, string("vertz_findh_cond(par, z1, z2, nth, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findc_cond"), VertFunctionMapValue(NFmiAreaMask::FindCountCond, NFmiAreaMask::VertZ, 3, string("vertz_findc_cond(par, z1, z2, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_peek"),VertFunctionMapValue(NFmiAreaMask::PeekZ, NFmiAreaMask::VertZ, 2, string("vertz_peek(par, deltaZ)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));

    // vertlev-funktiot eli näitä operoidaan aina mallipintadatan hybrid-level arvoilla esim. hirlamissa arvot ovat 60 - 1
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::VertHyb, 3, string("vertlev_max(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::VertHyb, 3, string("vertlev_min(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::VertHyb, 3, string("vertlev_avg(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::VertHyb, 3, string("vertlev_sum(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::VertHyb, 3, string("vertlev_med(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::VertHyb, 3, string("vertlev_modavg(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::VertHyb, 3, string("vertlev_modmin(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::VertHyb, 3, string("vertlev_modmax(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_get"), VertFunctionMapValue(NFmiAreaMask::Get, NFmiAreaMask::VertHyb, 2, string("vertlev_get(par, hyb)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findh"), VertFunctionMapValue(NFmiAreaMask::FindH, NFmiAreaMask::VertHyb, 5, string("vertlev_findh(par, hyb1, hyb2, value, nth)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findc"), VertFunctionMapValue(NFmiAreaMask::FindC, NFmiAreaMask::VertHyb, 4, string("vertlev_findc(par, hyb1, hyb2, value)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_maxh"), VertFunctionMapValue(NFmiAreaMask::MaxH, NFmiAreaMask::VertHyb, 3, string("vertlev_maxh(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_minh"), VertFunctionMapValue(NFmiAreaMask::MinH, NFmiAreaMask::VertHyb, 3, string("vertlev_minh(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_grad"), VertFunctionMapValue(NFmiAreaMask::Grad, NFmiAreaMask::VertHyb, 3, string("vertlev_grad(par, hyb1, hyb2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findh_cond"), VertFunctionMapValue(NFmiAreaMask::FindHeightCond, NFmiAreaMask::VertHyb, 4, string("vertlev_findh_cond(par, p1, p2, nth, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findc_cond"), VertFunctionMapValue(NFmiAreaMask::FindCountCond, NFmiAreaMask::VertHyb, 3, string("vertlev_findc_cond(par, p1, p2, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));

    // Kaikki vert-conditional funktiot yhdessä nipussa.
    // Niillä etsitään korkeutta mistä alkaen jokin ehto on voimassa.
    // Jätetty pois tarkoituksella yhtäsuuruus ehdot, koska niitä voi etsiä vertXXX_findh -funktioilla.
    // VertP -osio
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findh_over"), VertFunctionMapValue(NFmiAreaMask::ProbOver, NFmiAreaMask::VertP, 4, string("vertp_findh_over(par, p1, p2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findh_overeq"), VertFunctionMapValue(NFmiAreaMask::ProbOverEq, NFmiAreaMask::VertP, 4, string("vertp_findh_overeq(par, p1, p2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findh_under"), VertFunctionMapValue(NFmiAreaMask::ProbUnder, NFmiAreaMask::VertP, 4, string("vertp_findh_under(par, p1, p2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findh_undereq"), VertFunctionMapValue(NFmiAreaMask::ProbUnderEq, NFmiAreaMask::VertP, 4, string("vertp_findh_undereq(par, p1, p2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findh_between"), VertFunctionMapValue(NFmiAreaMask::ProbBetween, NFmiAreaMask::VertP, 5, string("vertp_findh_between(par, p1, p2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertp_findh_betweeneq"), VertFunctionMapValue(NFmiAreaMask::ProbBetweenEq, NFmiAreaMask::VertP, 5, string("vertp_findh_betweeneq(par, p1, p2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    // VertFL -osio
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findh_over"), VertFunctionMapValue(NFmiAreaMask::ProbOver, NFmiAreaMask::VertFL, 4, string("vertfl_findh_over(par, fl1, fl2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findh_overeq"), VertFunctionMapValue(NFmiAreaMask::ProbOverEq, NFmiAreaMask::VertFL, 4, string("vertfl_findh_overeq(par, fl1, fl2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findh_under"), VertFunctionMapValue(NFmiAreaMask::ProbUnder, NFmiAreaMask::VertFL, 4, string("vertfl_findh_under(par, fl1, fl2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findh_undereq"), VertFunctionMapValue(NFmiAreaMask::ProbUnderEq, NFmiAreaMask::VertFL, 4, string("vertfl_findh_undereq(par, fl1, fl2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findh_between"), VertFunctionMapValue(NFmiAreaMask::ProbBetween, NFmiAreaMask::VertFL, 5, string("vertfl_findh_between(par, fl1, fl2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertfl_findh_betweeneq"), VertFunctionMapValue(NFmiAreaMask::ProbBetweenEq, NFmiAreaMask::VertFL, 5, string("vertfl_findh_betweeneq(par, z1, z2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    // VertZ -osio
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findh_over"), VertFunctionMapValue(NFmiAreaMask::ProbOver, NFmiAreaMask::VertZ, 4, string("vertz_findh_over(par, z1, z2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findh_overeq"), VertFunctionMapValue(NFmiAreaMask::ProbOverEq, NFmiAreaMask::VertZ, 4, string("vertz_findh_overeq(par, z1, z2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findh_under"), VertFunctionMapValue(NFmiAreaMask::ProbUnder, NFmiAreaMask::VertZ, 4, string("vertz_findh_under(par, z1, z2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findh_undereq"), VertFunctionMapValue(NFmiAreaMask::ProbUnderEq, NFmiAreaMask::VertZ, 4, string("vertz_findh_undereq(par, z1, z2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findh_between"), VertFunctionMapValue(NFmiAreaMask::ProbBetween, NFmiAreaMask::VertZ, 5, string("vertz_findh_between(par, z1, z2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertz_findh_betweeneq"), VertFunctionMapValue(NFmiAreaMask::ProbBetweenEq, NFmiAreaMask::VertZ, 5, string("vertz_findh_betweeneq(par, z1, z2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    // VertLev -osio
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findh_over"), VertFunctionMapValue(NFmiAreaMask::ProbOver, NFmiAreaMask::VertHyb, 4, string("vertlev_findh_over(par, lev1, lev2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findh_overeq"), VertFunctionMapValue(NFmiAreaMask::ProbOverEq, NFmiAreaMask::VertHyb, 4, string("vertlev_findh_overeq(par, lev1, lev2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findh_under"), VertFunctionMapValue(NFmiAreaMask::ProbUnder, NFmiAreaMask::VertHyb, 4, string("vertlev_findh_under(par, lev1, lev2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findh_undereq"), VertFunctionMapValue(NFmiAreaMask::ProbUnderEq, NFmiAreaMask::VertHyb, 4, string("vertlev_findh_undereq(par, lev1, lev2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findh_between"), VertFunctionMapValue(NFmiAreaMask::ProbBetween, NFmiAreaMask::VertHyb, 5, string("vertlev_findh_between(par, lev1, lev2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("vertlev_findh_betweeneq"), VertFunctionMapValue(NFmiAreaMask::ProbBetweenEq, NFmiAreaMask::VertHyb, 5, string("vertlev_findh_betweeneq(par, lev1, lev2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));


    // Probability-laskenta (laatikko eli rect) vertlev-funktiot eli nämä on laitettu tähän, koska
    // tämän funktion parametrien käsittely sopii tn-laskuille
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probrect_over"), VertFunctionMapValue(NFmiAreaMask::ProbOver, NFmiAreaMask::AreaRect, 5, string("probrect_over(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probrect_overeq"), VertFunctionMapValue(NFmiAreaMask::ProbOverEq, NFmiAreaMask::AreaRect, 5, string("probrect_overeq(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probrect_under"), VertFunctionMapValue(NFmiAreaMask::ProbUnder, NFmiAreaMask::AreaRect, 5, string("probrect_under(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probrect_undereq"), VertFunctionMapValue(NFmiAreaMask::ProbUnderEq, NFmiAreaMask::AreaRect, 5, string("probrect_undereq(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probrect_equal"), VertFunctionMapValue(NFmiAreaMask::ProbEqual, NFmiAreaMask::AreaRect, 5, string("probrect_equal(par, radius_km, time_offset1, time_offset2, value)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probrect_notequal"), VertFunctionMapValue(NFmiAreaMask::ProbNotEqual, NFmiAreaMask::AreaRect, 5, string("probrect_notequal(par, radius_km, time_offset1, time_offset2, value)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probrect_between"), VertFunctionMapValue(NFmiAreaMask::ProbBetween, NFmiAreaMask::AreaRect, 6, string("probrect_between(par, radius_km, time_offset1, time_offset2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probrect_betweeneq"), VertFunctionMapValue(NFmiAreaMask::ProbBetweenEq, NFmiAreaMask::AreaRect, 6, string("probrect_betweeneq(par, radius_km, time_offset1, time_offset2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));

    // Probability-laskenta (ympyrä eli circle) vertlev-funktiot eli nämä on laitettu tähän, koska
    // tämän funktion parametrien käsittely sopii tn-laskuille
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probcircle_over"), VertFunctionMapValue(NFmiAreaMask::ProbOver, NFmiAreaMask::AreaCircle, 5, string("probcircle_over(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probcircle_overeq"), VertFunctionMapValue(NFmiAreaMask::ProbOverEq, NFmiAreaMask::AreaCircle, 5, string("probcircle_overeq(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probcircle_under"), VertFunctionMapValue(NFmiAreaMask::ProbUnder, NFmiAreaMask::AreaCircle, 5, string("probcircle_under(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probcircle_undereq"), VertFunctionMapValue(NFmiAreaMask::ProbUnderEq, NFmiAreaMask::AreaCircle, 5, string("probcircle_undereq(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probcircle_equal"), VertFunctionMapValue(NFmiAreaMask::ProbEqual, NFmiAreaMask::AreaCircle, 5, string("probcircle_equal(par, radius_km, time_offset1, time_offset2, value)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probcircle_notequal"), VertFunctionMapValue(NFmiAreaMask::ProbNotEqual, NFmiAreaMask::AreaCircle, 5, string("probcircle_equal(par, radius_km, time_offset1, time_offset2, value)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probcircle_between"), VertFunctionMapValue(NFmiAreaMask::ProbBetween, NFmiAreaMask::AreaCircle, 6, string("probcircle_between(par, radius_km, time_offset1, time_offset2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("probcircle_betweeneq"), VertFunctionMapValue(NFmiAreaMask::ProbBetweenEq, NFmiAreaMask::AreaCircle, 6, string("probcircle_betweeneq(par, radius_km, time_offset1, time_offset2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));

    // Esiintymä funktio eli kuinka monta kertaa joku ehto halutulle parametrille pitää paikkaansa
    // (halutulla aikavälillä ja halutun säteisellä alueella).
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("occurrence_over"), VertFunctionMapValue(NFmiAreaMask::ProbOver, NFmiAreaMask::Occurrence, 5, string("occurrence_over(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("occurrence_overeq"), VertFunctionMapValue(NFmiAreaMask::ProbOverEq, NFmiAreaMask::Occurrence, 5, string("occurrence_overeq(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("occurrence_under"), VertFunctionMapValue(NFmiAreaMask::ProbUnder, NFmiAreaMask::Occurrence, 5, string("occurrence_under(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("occurrence_undereq"), VertFunctionMapValue(NFmiAreaMask::ProbUnderEq, NFmiAreaMask::Occurrence, 5, string("occurrence_undereq(par, radius_km, time_offset1, time_offset2, limit)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("occurrence_equal"), VertFunctionMapValue(NFmiAreaMask::ProbEqual, NFmiAreaMask::Occurrence, 5, string("occurrence_equal(par, radius_km, time_offset1, time_offset2, value)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("occurrence_notequal"), VertFunctionMapValue(NFmiAreaMask::ProbNotEqual, NFmiAreaMask::Occurrence, 5, string("occurrence_notequal(par, radius_km, time_offset1, time_offset2, value)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("occurrence_between"), VertFunctionMapValue(NFmiAreaMask::ProbBetween, NFmiAreaMask::Occurrence, 6, string("occurrence_between(par, radius_km, time_offset1, time_offset2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("occurrence_betweeneq"), VertFunctionMapValue(NFmiAreaMask::ProbBetweenEq, NFmiAreaMask::Occurrence, 6, string("occurrence_betweeneq(par, radius_km, time_offset1, time_offset2, limit1, limit2)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));
    // Esiintymä funktio otetaan suoraan simple-condition ehdoista
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("occurrence"), VertFunctionMapValue(NFmiAreaMask::ProbSimpleCondition, NFmiAreaMask::Occurrence2, 4, string("occurrence(par, radius_km, time_offset1, time_offset2, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));

    // Hae asemadatasta lähin arvo funktionaalisuus
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("closestvalue"), VertFunctionMapValue(NFmiAreaMask::ClosestObsTimeOffset, NFmiAreaMask::ClosestObsValue, 2, string("closestvalue(par, timeoffset)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));

    // Hae datasta arvo aika offsetin [h] avulla
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("peekt"), VertFunctionMapValue(NFmiAreaMask::PeekT, NFmiAreaMask::PeekT, 2, string("peekt(par, timeoffset)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));

    // time_*-funktiot laskevat halutun operaation läpi halutun aikahaarukan. Laskut käydään läpi
    // datan omassa aikaresoluutiossa, eli tässä ei ole aikainterpolaatioita kuten esim. maxt
    // vastaavissa funktioissa.
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("time_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::TimeRange, 3, string("time_max(par, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("time_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::TimeRange, 3, string("time_min(par, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("time_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::TimeRange, 3, string("time_avg(par, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("time_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::TimeRange, 3, string("time_sum(par, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("time_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::TimeRange, 3, string("time_med(par, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("time_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::TimeRange, 3, string("time_modavg(par, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("time_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::TimeRange, 3, string("time_modmin(par, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("time_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::TimeRange, 3, string("time_modmax(par, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));

    // Haetaan par1:lle max/min arvoa ja siitä paikasta ja ajanhetkestä palautetaan par2:n arvo.
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("second_param_from_min_time"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::SecondParamFromExtremeTime, 4, string("second_param_from_min_time(par1, par2, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("second_param_from_max_time"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::SecondParamFromExtremeTime, 4, string("second_param_from_max_time(par1, par2, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    
    // Tässä on time-range vertikaaliset-funktiot, jotka operoivat datan omassa aika ja level
    // resoluutiossa.
    // Esim. hae maksimi arvo 2h aikavälillä 1000 ja 500 hPa väliltä.
    // =================================================================
    // timevertp-funktiot eli näitä operoidaan aina painepinnoilla [hPa]
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertp_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::TimeVertP, 5, string("timevertp_max(par, timeoffset1, timeoffset2, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertp_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::TimeVertP, 5, string("timevertp_min(par, timeoffset1, timeoffset2, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertp_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::TimeVertP, 5, string("timevertp_avg(par, timeoffset1, timeoffset2, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertp_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::TimeVertP, 5, string("timevertp_sum(par, timeoffset1, timeoffset2, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertp_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::TimeVertP, 5, string("timevertp_med(par, timeoffset1, timeoffset2, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertp_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::TimeVertP, 5, string("timevertp_modavg(par, timeoffset1, timeoffset2, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertp_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::TimeVertP, 5, string("timevertp_modmin(par, timeoffset1, timeoffset2, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertp_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::TimeVertP, 5, string("timevertp_modmax(par, timeoffset1, timeoffset2, p1, p2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));

    // timevertfl-funktiot eli näitä operoidaan aina flight level pinnoilla [hft]
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertfl_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::TimeVertFL, 5, string("timevertfl_max(par, timeoffset1, timeoffset2, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertfl_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::TimeVertFL, 5, string("timevertfl_min(par, timeoffset1, timeoffset2, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertfl_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::TimeVertFL, 5, string("timevertfl_avg(par, timeoffset1, timeoffset2, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertfl_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::TimeVertFL, 5, string("timevertfl_sum(par, timeoffset1, timeoffset2, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertfl_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::TimeVertFL, 5, string("timevertfl_med(par, timeoffset1, timeoffset2, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertfl_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::TimeVertFL, 5, string("timevertfl_modavg(par, timeoffset1, timeoffset2, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertfl_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::TimeVertFL, 5, string("timevertfl_modmin(par, timeoffset1, timeoffset2, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertfl_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::TimeVertFL, 5, string("timevertfl_modmax(par, timeoffset1, timeoffset2, fl1, fl2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));

    // timevertz-funktiot eli näitä operoidaan aina metrisillä korkeuksilla [m]
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertz_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::TimeVertZ, 5, string("timevertz_max(par, timeoffset1, timeoffset2, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertz_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::TimeVertZ, 5, string("timevertz_min(par, timeoffset1, timeoffset2, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertz_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::TimeVertZ, 5, string("timevertz_avg(par, timeoffset1, timeoffset2, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertz_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::TimeVertZ, 5, string("timevertz_sum(par, timeoffset1, timeoffset2, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertz_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::TimeVertZ, 5, string("timevertz_med(par, timeoffset1, timeoffset2, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertz_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::TimeVertZ, 5, string("timevertz_modavg(par, timeoffset1, timeoffset2, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertz_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::TimeVertZ, 5, string("timevertz_modmin(par, timeoffset1, timeoffset2, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertz_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::TimeVertZ, 5, string("timevertz_modmax(par, timeoffset1, timeoffset2, z1, z2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));

    // timevertlev-funktiot eli näitä operoidaan aina mallipintadatan hybrid-level arvoilla esim.
    // hirlamissa arvot ovat 60 - 1
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertlev_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::TimeVertHyb, 5, string("timevertlev_max(par, timeoffset1, timeoffset2, lev1, lev2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertlev_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::TimeVertHyb, 5, string("timevertlev_min(par, timeoffset1, timeoffset2, lev1, lev2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertlev_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::TimeVertHyb, 5, string("timevertlev_avg(par, timeoffset1, timeoffset2, lev1, lev2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertlev_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::TimeVertHyb, 5, string("timevertlev_sum(par, timeoffset1, timeoffset2, lev1, lev2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertlev_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::TimeVertHyb, 5, string("timevertlev_med(par, timeoffset1, timeoffset2, lev1, lev2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertlev_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::TimeVertHyb, 5, string("timevertlev_modavg(par, timeoffset1, timeoffset2, lev1, lev2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertlev_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::TimeVertHyb, 5, string("timevertlev_modmin(par, timeoffset1, timeoffset2, lev1, lev2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("timevertlev_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::TimeVertHyb, 5, string("timevertlev_modmax(par, timeoffset1, timeoffset2, lev1, lev2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));

    // area-kokooma funktioperhe, jotka toimivat annetun ympyrän säteellä
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("area_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::AreaCircle, 4, string("area_min(par, radius_km, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("area_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::AreaCircle, 4, string("area_max(par, radius_km, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("area_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::AreaCircle, 4, string("area_avg(par, radius_km, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("area_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::AreaCircle, 4, string("area_sum(par, radius_km, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("area_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::AreaCircle, 4, string("area_med(par, radius_km, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("area_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::AreaCircle, 4, string("area_modavg(par, radius_km, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("area_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::AreaCircle, 4, string("area_modmin(par, radius_km, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("area_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::AreaCircle, 4, string("area_modmax(par, radius_km, time_offset1, time_offset2)"), NFmiAreaMask::SimpleConditionRule::Allowed)));

    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("area_prob"), VertFunctionMapValue(NFmiAreaMask::ProbSimpleCondition, NFmiAreaMask::AreaCircle, 4, string("area_prob(par, radius_km, time_offset1, time_offset2, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));

    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("previousfulldays_min"), VertFunctionMapValue(NFmiAreaMask::Min, NFmiAreaMask::PreviousFullDays, 2, string("previousfulldays_min(par, day_count)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("previousfulldays_max"), VertFunctionMapValue(NFmiAreaMask::Max, NFmiAreaMask::PreviousFullDays, 2, string("previousfulldays_max(par, day_count)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("previousfulldays_avg"), VertFunctionMapValue(NFmiAreaMask::Avg, NFmiAreaMask::PreviousFullDays, 2, string("previousfulldays_avg(par, day_count)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("previousfulldays_sum"), VertFunctionMapValue(NFmiAreaMask::Sum, NFmiAreaMask::PreviousFullDays, 2, string("previousfulldays_sum(par, day_count)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("previousfulldays_med"), VertFunctionMapValue(NFmiAreaMask::Med, NFmiAreaMask::PreviousFullDays, 2, string("previousfulldays_med(par, day_count)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("previousfulldays_modavg"), VertFunctionMapValue(NFmiAreaMask::ModAvg, NFmiAreaMask::PreviousFullDays, 2, string("previousfulldays_modavg(par, day_count)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("previousfulldays_modmin"), VertFunctionMapValue(NFmiAreaMask::ModMin, NFmiAreaMask::PreviousFullDays, 2, string("previousfulldays_modmin(par, day_count)"), NFmiAreaMask::SimpleConditionRule::Allowed)));
    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("previousfulldays_modmax"), VertFunctionMapValue(NFmiAreaMask::ModMax, NFmiAreaMask::PreviousFullDays, 2, string("previousfulldays_modmax(par, day_count)"), NFmiAreaMask::SimpleConditionRule::Allowed)));

    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("duration"), VertFunctionMapValue(NFmiAreaMask::TimeDuration, NFmiAreaMask::TimeDuration, 3, string("duration(par, seek_time_in_hours, use_cumulative_calculation, \"x > y\")"), NFmiAreaMask::SimpleConditionRule::MustHave)));

    itsTokenVertFunctions.insert(VertFunctionMap::value_type(string("local_extremes"), VertFunctionMapValue(NFmiAreaMask::LocalExtremes, NFmiAreaMask::LocalExtremes, 4, string("local_extremes(par, search_radius_in_km, min_value, max_value)"), NFmiAreaMask::SimpleConditionRule::NotAllowed)));

    itsTokenPeekFunctions.insert(std::make_pair(string("peekxy"), NFmiAreaMask::FunctionPeekXY));
    itsTokenPeekFunctions.insert(std::make_pair(string("peekxy2"), NFmiAreaMask::FunctionPeekXY2));
    itsTokenPeekFunctions.insert(std::make_pair(string("peekxy3"), NFmiAreaMask::FunctionPeekXY3));

    itsTokenRampUpFunctions.push_back(string("RU"));
    itsTokenRampUpFunctions.push_back(string("Ru"));
    itsTokenRampUpFunctions.push_back(string("ru"));
    itsTokenRampDownFunctions.push_back(string("RD"));
    itsTokenRampDownFunctions.push_back(string("Rd"));
    itsTokenRampDownFunctions.push_back(string("rd"));
    itsTokenDoubleRampFunctions.push_back(string("DD"));
    itsTokenDoubleRampFunctions.push_back(string("Dd"));
    itsTokenDoubleRampFunctions.push_back(string("dd"));

    itsTokenRampFunctions.insert(itsTokenRampFunctions.end(), itsTokenRampUpFunctions.begin(), itsTokenRampUpFunctions.end());
    itsTokenRampFunctions.insert(itsTokenRampFunctions.end(), itsTokenRampDownFunctions.begin(), itsTokenRampDownFunctions.end());
    itsTokenRampFunctions.insert(itsTokenRampFunctions.end(), itsTokenDoubleRampFunctions.begin(), itsTokenDoubleRampFunctions.end());

    itsTokenMacroParamIdentifiers.push_back(string("result"));
    itsTokenMacroParamIdentifiers.push_back(string("Result"));
    itsTokenMacroParamIdentifiers.push_back(string("RESULT"));

    itsTokenDeltaZIdentifiers.push_back(string("deltaz"));
    itsTokenDeltaZIdentifiers.push_back(string("DeltaZ"));
    itsTokenDeltaZIdentifiers.push_back(string("DELTAZ"));

    itsMathFunctions.insert(MathFunctionMap::value_type(string("exp"), NFmiAreaMask::Exp));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("sqrt"), NFmiAreaMask::Sqrt));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("ln"), NFmiAreaMask::Log));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("lg"), NFmiAreaMask::Log10));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("sin"), NFmiAreaMask::Sin));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("cos"), NFmiAreaMask::Cos));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("tan"), NFmiAreaMask::Tan));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("sinh"), NFmiAreaMask::Sinh));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("cosh"), NFmiAreaMask::Cosh));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("tanh"), NFmiAreaMask::Tanh));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("asin"), NFmiAreaMask::Asin));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("acos"), NFmiAreaMask::Acos));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("atan"), NFmiAreaMask::Atan));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("ceil"), NFmiAreaMask::Ceil));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("floor"), NFmiAreaMask::Floor));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("round"), NFmiAreaMask::Round));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("abs"), NFmiAreaMask::Abs));
    itsMathFunctions.insert(MathFunctionMap::value_type(string("rand"), NFmiAreaMask::Rand));

    itsExtraInfoCommands.insert(FunctionMap::value_type(string("resolution"), NFmiAreaMask::Resolution));
    itsExtraInfoCommands.insert(FunctionMap::value_type(string("calculationpoint"), NFmiAreaMask::CalculationPoint));
    itsExtraInfoCommands.insert(FunctionMap::value_type(string("observationradius"), NFmiAreaMask::ObservationRadius));
    itsExtraInfoCommands.insert(FunctionMap::value_type(string("symboltooltipfile"), NFmiAreaMask::SymbolTooltipFile));
    itsExtraInfoCommands.insert(FunctionMap::value_type(string("macroparamdescription"), NFmiAreaMask::MacroParamDescription));
    itsExtraInfoCommands.insert(FunctionMap::value_type(string("calculationtype"), NFmiAreaMask::CalculationType));
    itsExtraInfoCommands.insert(FunctionMap::value_type(string("workingthreadcount"), NFmiAreaMask::WorkingThreadCount));
    itsExtraInfoCommands.insert(FunctionMap::value_type(string("fixedbasedata"), NFmiAreaMask::FixedBaseData));

    itsExtraInfoCommands.insert(FunctionMap::value_type(string("multiparamtooltipfile"), NFmiAreaMask::MultiParamTooltipFile));
    itsExtraInfoCommands.insert(FunctionMap::value_type(string("multiparam2"), NFmiAreaMask::MultiParam2));
    itsExtraInfoCommands.insert(FunctionMap::value_type(string("multiparam3"), NFmiAreaMask::MultiParam3));

    itsResolutionLevelTypes.insert(ResolutionLevelTypesMap::value_type(string("surface"), kFmiMeanSeaLevel));
    itsResolutionLevelTypes.insert(ResolutionLevelTypesMap::value_type(string("pressure"), kFmiPressureLevel));
    itsResolutionLevelTypes.insert(ResolutionLevelTypesMap::value_type(string("hybrid"), kFmiHybridLevel));
    itsResolutionLevelTypes.insert(ResolutionLevelTypesMap::value_type(string("height"), kFmiHeight));

    // clang-format on
  }
}
