
/*!
 * Tämä on Editorin käyttämä sana kirja funktio.
 * Kieli versiot stringeihin tulevat täältä.
 */

#include "NFmiDictionaryFunction.h"

// HUOM! Tämä on kopio NFmiEditMapGeneralDataDoc-luokan metodista, kun en voinut antaa tänne
// dokumenttia
std::string GetDictionaryString(const char *theMagicWord)
{
  const std::string baseWords = "MetEditor::Dictionary::";

  std::string usedMagicWord(theMagicWord);
  NFmiStringTools::ReplaceAll(usedMagicWord, "\t", "\\t");  // pitää muuttaa mahdollinen
  // tabulaattori merkkisarjaksi, missä on
  // kenoviiva ja t peräkkäin, koska
  // tabulaattori on siten sanakirjassa
  std::string finalMagicWord(baseWords);
  finalMagicWord += usedMagicWord;
  std::string dictionarySentense =
      NFmiSettings::Optional<std::string>(finalMagicWord.c_str(), std::string(theMagicWord));
  NFmiStringTools::ReplaceAll(
      dictionarySentense,
      "\\t",
      "\t");  // pitää muuttaa mahdolliset "\\t"-merkkijonot takaisin tabulaattoreiksi
  return dictionarySentense;
}
