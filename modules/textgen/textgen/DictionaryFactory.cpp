// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::DictionaryFactory
 */
// ======================================================================
/*!
 * \namespace TextGen::DictionaryFactory
 *
 * \brief Dictionary creation services
 *
 * The responsibility of the DictionaryFactory namespace is to provide
 * dictionary creation services.
 *
 * For example,
 * \code
 * boost::shared_ptr<Dictionary> dict1(DictionaryFactory::create("null"));
 * boost::shared_ptr<Dictionary> dict2(DictionaryFactory::create("basic"));
 * boost::shared_ptr<Dictionary> dict3(DictionaryFactory::create("mysql"));
 * boost::shared_ptr<Dictionary> dict3(DictionaryFactory::create("file"));
 * \endcode
 *
 */
// ======================================================================

#include "DictionaryFactory.h"
#include "NullDictionary.h"
#include "BasicDictionary.h"
#include "FileDictionary.h"
#include "MySQLDictionary.h"
#include "MySQLDictionaries.h"
#include <calculator/TextGenError.h>

using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Create a dictionary of given type
 *
 * \param theType The type of the dictionary (null|basic|mysql)
 * \return An uninitialized dictionary
 */
// ----------------------------------------------------------------------

Dictionary* DictionaryFactory::create(const std::string& theType)
{
  if (theType == "null") return new NullDictionary();
  if (theType == "basic") return new BasicDictionary();
  if (theType == "file") return new FileDictionary();
#ifdef UNIX
  if (theType == "mysql") return new MySQLDictionary();
  if (theType == "multimysql") return new MySQLDictionaries();
#endif // UNIX

  throw TextGenError("Error: Unknown dictionary type " + theType);
}

}  // namespace TextGen

// ======================================================================
