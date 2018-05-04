// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::FileDictionary
 */
// ======================================================================
/*!
 * \class TextGen::FileDictionary
 *
 * \brief Provides dictionary services
 *
 * The responsibility of the FileDictionary class is to provide natural
 * language text for the given keyword. Inserting new keyword-text pairs.
 *
 * The dictionary has an initialization method, which fetches the specified
 * language from the flatfile databases.
 *
 * Sample usage:
 * \code
 * using namespace TextGen;
 *
 * FileDictionary english;
 * english.init("en");
 *
 * cout << english.find("1-aamusta") << endl;
 * cout << english.find("paivan ylin lampotila") << endl;
 *
 * if(english.contains("1-aamusta"))
 *    cout << english.find("1-aamusta") << endl;
 * \endcode
 *
 * Note that find throws if the given keyword does not exist.
 *
 * The flatfile database location is stored externally in fmi.conf
 * used by newbase NFmiSettings class.
 *
 * The dictionary can be initialized multiple times. Each init
 * erases the language initialized earlier.
 */
// ----------------------------------------------------------------------

#include "FileDictionary.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiStringTools.h>

#include <fstream>
#include <map>

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Implementation hiding pimple
 *
 */
// ----------------------------------------------------------------------

class FileDictionary::Pimple
{
 public:
  Pimple() : itsInitialized(false), itsLanguage(), itsData() {}
  typedef std::map<std::string, std::string> StorageType;
  typedef StorageType::value_type value_type;

  bool itsInitialized;
  std::string itsLanguage;
  StorageType itsData;

};  // class Pimple

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 *
 * The destructor does nothing special.
 */
// ----------------------------------------------------------------------

FileDictionary::~FileDictionary() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * The constructor does nothing special.
 */
// ----------------------------------------------------------------------

FileDictionary::FileDictionary() : Dictionary(), itsPimple(new Pimple()) {}
// ----------------------------------------------------------------------
/*!
 * \brief Return the language
 *
 * An empty string is returned if no language has been initialized.
 *
 * \return The Language
 */
// ----------------------------------------------------------------------

const std::string& FileDictionary::language(void) const { return itsPimple->itsLanguage; }
// ----------------------------------------------------------------------
/*!
 * \brief Initialize with given language
 *
 * Initializing the FileDictionary involves connecting to the File
 * database containing the dictionary for the given language, and
 * reading all words defined in the dictionary to the internal containers.
 *
 * If any errors occur during the initialization, a TextGenError is thrown.
 *
 * Any language initialized earlier will be erased. Initializing the
 * same language twice is essentially a reload from the File database.
 *
 * \param theLanguage The ISO-code of the language to initialize
 */
// ----------------------------------------------------------------------

void FileDictionary::init(const std::string& theLanguage)
{
  // clear possible earlier language
  itsPimple->itsLanguage = theLanguage;
  itsPimple->itsInitialized = false;
  itsPimple->itsData.clear();

  // Establish the settings for TextGen

  std::string database = Settings::require_string("textgen::filedictionaries");
  // Read the file one line at a time

  std::string filename = database + '/' + theLanguage + ".txt";

  if (!NFmiFileSystem::FileExists(filename))
    throw TextGenError("Error: Could not find dictionary '" + filename + "'");

  std::ifstream in(filename.c_str());
  if (!in) throw TextGenError("Error: Could not open dictionary '" + filename + "' for reading");

  std::string line;
  while (getline(in, line))
  {
    std::vector<std::string> parts = NFmiStringTools::Split(line, "|");
    if (parts.size() != 2)
      throw TextGenError("Error: Dictionary '" + filename + "' contains invalid line '" + line +
                         "'");
    if (!parts[0].empty()) itsPimple->itsData.insert(Pimple::value_type(parts[0], parts[1]));
  }

  itsPimple->itsInitialized = true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the given phrase is in the dictionary
 *
 * \param theKey The key of the phrase
 * \return True if the phrase is in the dictionary.
 */
// ----------------------------------------------------------------------

bool FileDictionary::contains(const std::string& theKey) const
{
  return (itsPimple->itsData.find(theKey) != itsPimple->itsData.end());
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the phrase for the given key.
 *
 * This will throw a TextGenError if the phrase is not in
 * the dictionary, or if the dictionary has not been initialized.
 *
 * \param theKey The key of the phrase
 * \return The phrase
 */
// ----------------------------------------------------------------------

const std::string& FileDictionary::find(const std::string& theKey) const
{
  if (!itsPimple->itsInitialized)
    throw TextGenError("Error: FileDictionary::find() called before init()");
  Pimple::StorageType::const_iterator it = itsPimple->itsData.find(theKey);

  if (it != itsPimple->itsData.end()) return it->second;
  throw TextGenError("Error: FileDictionary::find(" + theKey + ") failed in language " +
                     itsPimple->itsLanguage);
}

// ----------------------------------------------------------------------
/*!
 * \brief Inserting a new phrase into the dictionary is disabled
 *
 * We do not wish to manage the dictionaries via C++, it is far too
 * risky. Therefore this method will always throw.
 *
 * \param theKey The key of the phrase
 * \param thePhrase The phrase
 */
// ----------------------------------------------------------------------

void FileDictionary::insert(const std::string& theKey, const std::string& thePhrase)
{
  throw TextGenError("Error: FileDictionary::insert() is not allowed");
}

// ----------------------------------------------------------------------
/*!
 * Return the size of the dictionary
 *
 * \return The size of the dictionary
 */
// ----------------------------------------------------------------------

FileDictionary::size_type FileDictionary::size(void) const { return itsPimple->itsData.size(); }
// ----------------------------------------------------------------------
/*!
 * Test if the dictionary is empty
 *
 * \return True if the dictionary is empty
 */
// ----------------------------------------------------------------------

bool FileDictionary::empty(void) const { return itsPimple->itsData.empty(); }
}  // namespace TextGen

// ======================================================================
