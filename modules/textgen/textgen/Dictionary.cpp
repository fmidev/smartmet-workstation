// ======================================================================
/*!
 * \file
 * \brief Implementation of abstract class TextGen::Dictionary
 */
// ======================================================================
/*!
 * \class TextGen::Dictionary
 *
 * \brief Provides dictionary services
 *
 * The responsibility of the Dictionary class is to provide natural
 * language text for the given keyword, and to accept inserting
 * new keyword-text pairs.
 *
 * Dictionary is an abstract interface. Implementations of the interface
 * are provided for example by the BasicDictionary class.
 *
 * Note that the design intentionally does not provide a read method,
 * those are to be implemented externally using the add method. Also,
 * no write method is provided, all dictionary management is to be
 * done externally using real GUI tools.
 *
 */
// ----------------------------------------------------------------------

#include "Dictionary.h"
