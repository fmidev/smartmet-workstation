// ======================================================================
/*!
 * \file NFmiQueryInfoSatel.h
 * \brief Interface of class NFmiQueryInfoSatel
 */
// ======================================================================

#pragma once

#include "NFmiQueryInfo.h"

class NFmiQueryData;

//! Undocumented

class _FMI_DLL NFmiQueryInfoSatel : public NFmiQueryInfo
{
 public:
  ~NFmiQueryInfoSatel(void);
  NFmiQueryInfoSatel(void);
  NFmiQueryInfoSatel(const NFmiQueryInfoSatel& theInfo);

  NFmiQueryInfoSatel(const NFmiParamDescriptor& theParamDescriptor,
                     const NFmiTimeDescriptor& theTimeDescriptor,
                     const NFmiHPlaceDescriptor& theHPlaceDescriptor,
                     const NFmiVPlaceDescriptor& theVPlaceDescriptor = NFmiVPlaceDescriptor());

  NFmiQueryInfoSatel(NFmiQueryData* data,
                     NFmiParamDescriptor* theParamDescriptor = 0,
                     NFmiTimeDescriptor* theTimeDescriptor = 0,
                     NFmiHPlaceDescriptor* theHPlaceDescriptor = 0,
                     NFmiVPlaceDescriptor* theVPlaceDescriptor = 0);

  virtual NFmiQueryInfo& operator=(const NFmiQueryInfo& theInfo);

  NFmiQueryInfo* Clone(void) const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  virtual unsigned long ClassId(void) const;
  virtual const char* ClassName(void) const;

 private:
  NFmiQueryInfoSatel& operator=(const NFmiQueryInfoSatel& theInfo);

  void Destroy(void);
  NFmiString* itsSatelName;

};  // class NFMiQueryInfoSatel

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfoSatel::ClassId(void) const { return kNFmiSatelQueryInfo; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiQueryInfoSatel::ClassName(void) const { return "NFmiQueryInfoSatel"; }

// ======================================================================
