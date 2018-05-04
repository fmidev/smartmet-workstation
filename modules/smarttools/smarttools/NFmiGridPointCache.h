//© Ilmatieteenlaitos/Marko.
// Original 09.10.2006
//
// Luokka pitää MetEditoriin imagine contourauksessa käytettyjen
// xy-pisteiden cachea.
//---------------------------------------------------------- NFmiGridPointCache.h

#pragma once

#include <newbase/NFmiDataMatrix.h>
#include <newbase/NFmiPoint.h>

#include <map>

class NFmiGrid;
class NFmiArea;

class NFmiGridPointCache
{
 public:
  struct Data
  {
    NFmiPoint itsOffSet;  // koska lasketut hilat lasketaan (usean ruudukon karttanäytössä) eri
                          // kohdissa 0,0 - 1,1 maailmaa, pitää
    // 'originaalin' hilan offset origoon laittaa talteen, että myöhemmin voidaan laskea haluttuja
    // offsetteja contoureille
    NFmiDataMatrix<NFmiPoint> itsPoints;
  };

  typedef std::map<std::string, Data> pointMap;

  NFmiGridPointCache(void) : itsPointCache() {}
  ~NFmiGridPointCache(void) {}
  void Add(const std::string &theGridStr, const NFmiGridPointCache::Data &theData);
  pointMap::iterator Find(const std::string &theGridStr) { return itsPointCache.find(theGridStr); };
  pointMap::iterator End(void) { return itsPointCache.end(); };
  void Clear(void) { itsPointCache.clear(); };
  static const std::string MakeGridCacheStr(const NFmiGrid &theGrid);
  static const std::string MakeGridCacheStr(const NFmiArea &theArea, size_t xCount, size_t yCount);

 private:
  pointMap itsPointCache;
};

