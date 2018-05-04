// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::IdGenerator
 */
// ======================================================================

#include "IdGenerator.h"

#ifdef FMI_MULTITHREAD
#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>
#include <boost/interprocess/sync/upgradable_lock.hpp>

typedef boost::interprocess::interprocess_upgradable_mutex MutexType;
typedef boost::interprocess::upgradable_lock<MutexType> WriteLock;

static MutexType mymutex;
#endif

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Return a new ID
 *
 * \return The generated ID
 */
// ----------------------------------------------------------------------

#ifdef FMI_MULTITHREAD

long IdGenerator::generate()
{
  WriteLock lock(mymutex);
  static long id;
  ++id;
  return id;
}
#else
long IdGenerator::generate()
{
  static long id;
  return ++id;
}
#endif

}  // namespace TextGen

// ======================================================================
