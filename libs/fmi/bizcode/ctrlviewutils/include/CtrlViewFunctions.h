#pragma once
#include "NFmiPoint.h"
#include "NFmiTimeBag.h"
#include <vector>
#include <boost/shared_ptr.hpp>

class NFmiColor;
class NFmiRect;
class NFmiDrawParam;
class CtrlViewDocumentInterface;
class NFmiProducerSystem;
class NFmiFastQueryInfo;

namespace CtrlViewUtils
{
    // SmartMetin karttanäyttö ruudukun maksimi koot x- ja y-suunnassa
    const int MaxViewGridXSize = 10;
    const int MaxViewGridYSize = 5; // tämä on myös viewcachen max rivikoko
    const unsigned int kDoAllMapViewDescTopIndex = 999;
    const int ci_string_not_found = -1;

   NFmiPoint CalcTimeScaleFontSizeInPixels(double thePixelsPerMMinX);
   // lat/lon-string helpers
   std::string GetLatLonValue2MinuteStr(double value, int decimals); // voi olla lat/lon arvo
   std::string GetLatitudeMinuteStr(double lat, int decimals); // desimaali jutut toimivat nyt vain niin että jos 0, ei desimaaleja, muuten yksi desimaali
   std::string GetLongitudeMinuteStr(double lon, int decimals);

   std::string MakeSizeString(const NFmiPoint &theSize);
   std::string MakeMapPortionPixelSizeStringForStatusbar(const NFmiPoint &theSize, bool longVersion);

   // Case-insensitive-find stringille, haettu webistä: http://stackoverflow.com/questions/3152241/case-insensitive-stdstring-find
   // templated version of my_equal so it could work with both char and wchar_t
   template<typename charT>
   struct case_insensitive_equal {
       case_insensitive_equal(const std::locale& loc) : loc_(loc) {}
       bool operator()(charT ch1, charT ch2) {
           return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
       }
   private:
       const std::locale& loc_;
   };

   // find substring (case insensitive)
   template<typename T>
   int ci_find_substr(const T& str1, const T& str2, const std::locale& loc = std::locale())
   {
       typename T::const_iterator it = std::search(str1.begin(), str1.end(), str2.begin(), str2.end(), case_insensitive_equal<typename T::value_type>(loc));
       if(it != str1.end()) 
           return static_cast<int>(it - str1.begin());
       else 
           return ci_string_not_found;
   }

   template<class T>
   static void RemoveNthElementFromVector(T &theVector, int theRemovedItemIndex)
   {
       if(theRemovedItemIndex >= 0 && theRemovedItemIndex < static_cast<int>(theVector.size()))
           theVector.erase(theVector.begin() + theRemovedItemIndex);
   }

   std::string ColorFloat2HexStr(float value);
   std::string Color2HtmlColorStr(const NFmiColor &theColor);

   bool AreVectorValuesInRisingOrder(const std::vector<float> &theValueVector);

   template<class T>
   std::string ToHex(const T &value, int minWidth = 0, char paddingchar = ' ')
   {
       std::ostringstream oss;
       if(!(oss << std::setw(minWidth) << std::setfill(paddingchar) << std::hex << value))
           throw std::exception("ToHex - Invalid argument");
       return oss.str();
   }

   NFmiPoint ConvertPointFromRect1ToRect2(const NFmiPoint &thePointInRect1, const NFmiRect &theRect1, const NFmiRect &theRect2);
   NFmiTimeBag GetAdjustedTimeBag(const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime, int theTimeStepInMinutes);

   std::string GetIdString(unsigned long theId);
   std::string GetProducerName(NFmiProducerSystem &theProducerSystem, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool fAddProdId, size_t theLongerProducerNameMaxCharCount);
   std::string GetParamNameString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const std::string &theNormalOrigTimeFormat, const std::string &theMinuteOrigTimeFormat, bool fCrossSectionInfoWanted, bool fAddIdInfos, bool fMakeTooltipXmlEncode, size_t theLongerProducerNameMaxCharCount, bool fTimeSerialViewCase, bool fShowModelOriginTime = true);
   NFmiColor GetParamTextColor(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface);
   std::string GetEditingDataString(const std::string &theNameStr, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiLanguage lang, const std::string &theOrigTimeFormat);
   std::string GetTotalMapViewStatusBarStr(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const NFmiPoint &theLatlon);
   boost::shared_ptr<NFmiFastQueryInfo> GetLatestLastTimeObservation(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, bool fCrossSectionInfoWanted);
   std::string GetArchiveOrigTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool fGetCurrentDataFromQ2Server, const std::string &theDictionaryTokenForOrigTimeFormat);
   std::string GetLatestObservationTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const std::string &theTimeFormat, bool fCrossSectionInfoWanted);
   std::string XmlEncode(const std::string &src);

   template<typename T>
   bool IsEqualEnough(T value1, T value2, T usedEpsilon)
   {
       if(::fabs(static_cast<double>(value1 - value2)) < usedEpsilon)
           return true;
       return false;
   }

} // namespace CtrlViewUtils
