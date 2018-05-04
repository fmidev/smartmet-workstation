// ======================================================================
/*!
 * \file NFmiSettings.cpp
 * \brief Definition of NFmiSettings methods
 */
// ======================================================================

#include "NFmiSettings.h"

namespace NFmiSettings
{
void Clear() { NFmiSettingsImpl::Instance().Clear(); }
bool Init() { return NFmiSettingsImpl::Instance().Init(); }
bool Read(const std::string& theFile) { return NFmiSettingsImpl::Instance().Read(theFile); }
void Save() { NFmiSettingsImpl::Instance().Save(); }
void Set(const std::string& theName, const std::string& theValue, bool modifyOnlyExisting)
{
  NFmiSettingsImpl::Instance().Set(theName, theValue, modifyOnlyExisting);
}

bool IsSet(const std::string& theName) { return NFmiSettingsImpl::Instance().IsSet(theName); }
std::string ToString() { return NFmiSettingsImpl::Instance().ToString(); }
}

// ======================================================================
