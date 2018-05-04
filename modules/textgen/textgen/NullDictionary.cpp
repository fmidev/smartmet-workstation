// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::NullDictionary
 */
// ======================================================================
/*!
 * \class TextGen::NullDictionary
 *
 * \brief Provides a dummy dictionary service
 *
 * The responsibility of the NullDictionary class is to serve as
 * a default null dictionary so that conditional testing can be
 * avoided.
 *
 * The intended use is withing a singleton providing global access
 * to dictionary services. Since the singleton must be initialized
 * with some dictionary implementation, we must either have a boolean
 * indicating a successful init, or a null dictionary serving as
 * the default dictionary.
 *
 * NullDictionary will throw a TextGenError when accessing data.
 *
 * For example, a singleton providing global dictionary services
 * could have an attribute
 * \code
 *   boost::shared_ptr<Dictionary>
 * \endcode
 * and could have a constructor and methods like
 * \code
 * GlobalDictionary::GlobalDictionary()
 * {
 *    itsDictionary.reset(new NullDictionary());
 * }
 *
 * GlobalDictionary::find(const std::string & theKey) const
 * {
 *   return itsDictionary->find(theKey);
 * }
 * \endcode
 * instead of
 * \code
 * GlobalDictionary::GlobalDictionary()
 *   : itsDictionary()
 * {
 * }
 *
 * GlobalDictionary::find(const std::string & theKey) const
 * {
 *   if(itsDictionary.get())
 *      return itsDictionary->find(theKey);
 *   else
 *      throw TextGenError("Dictionary not initialized");
 * }
 * \endcode
 * That is, we avoid having to test the validity of the pointer
 * whenever we call any method in the stored Dictionary pointer.
 */
// ----------------------------------------------------------------------

#include "NullDictionary.h"
#include <calculator/TextGenError.h>

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Return the phrase for the given key.
 *
 * This will always throw a TextGenError.
 *
 * \param theKey The key of the phrase
 * \return Nothing, always throws
 */
// ----------------------------------------------------------------------

const std::string& NullDictionary::find(const std::string& theKey) const
{
  throw TextGenError("NullDictionary find is disabled");
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert a new phrase into the dictionary
 *
 * Always throws.
 *
 * \param theKey The key of the phrase
 * \param thePhrase The phrase
 */
// ----------------------------------------------------------------------

void NullDictionary::insert(const std::string& theKey, const std::string& thePhrase)
{
  throw TextGenError("NullDictionary insert is disabled");
}

}  // namespace TextGen

// ======================================================================
