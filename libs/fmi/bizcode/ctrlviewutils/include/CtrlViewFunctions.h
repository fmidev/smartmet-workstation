#pragma once
#include "NFmiPoint.h"
#include "NFmiTimeBag.h"
#include "NFmiInfoData.h"
#include "NFmiParameterName.h"
#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>

class NFmiColor;
class NFmiRect;
class NFmiDrawParam;
class CtrlViewDocumentInterface;
class NFmiProducerSystem;
class NFmiFastQueryInfo;
class NFmiMacroParamSystem;
namespace CatLog
{
    enum class Severity;
    enum class Category;
}

namespace CtrlViewUtils
{
    // SmartMetin karttanäyttö ruudukun maksimi koot x- ja y-suunnassa
    const int MaxViewGridXSize = 10;
    const int MaxViewGridYSize = 5; // tämä on myös viewcachen max rivikoko
    const unsigned int kDoAllMapViewDescTopIndex = 999;
    const int MaxMapViewTooltipWidthInPixels = 900;
    const char ParameterStringHighlightCharacter = '*';

   NFmiPoint CalcTimeScaleFontSizeInPixels(double thePixelsPerMMinX);
   // lat/lon-string helpers
   std::string GetLatLonValue2MinuteStr(double value, int decimals); // voi olla lat/lon arvo
   std::string GetLatitudeMinuteStr(double lat, int decimals); // desimaali jutut toimivat nyt vain niin että jos 0, ei desimaaleja, muuten yksi desimaali
   std::string GetLongitudeMinuteStr(double lon, int decimals);

   std::string MakeSizeString(const NFmiPoint &theSize);
   std::string MakeMapPortionPixelSizeStringForStatusbar(const NFmiPoint &theSize, bool longVersion);


   template<class T>
   static void RemoveNthElementFromVector(T &theVector, int theRemovedItemIndex)
   {
       if(theRemovedItemIndex >= 0 && theRemovedItemIndex < static_cast<int>(theVector.size()))
           theVector.erase(theVector.begin() + theRemovedItemIndex);
   }

   bool AreVectorValuesInRisingOrder(const std::vector<float> &theValueVector);

   NFmiPoint ConvertPointFromRect1ToRect2(const NFmiPoint &thePointInRect1, const NFmiRect &theRect1, const NFmiRect &theRect2);
   NFmiTimeBag GetAdjustedTimeBag(const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime, int theTimeStepInMinutes);

   std::string GetIdString(unsigned long theId);
   std::string GetProducerName(NFmiProducerSystem &theProducerSystem, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool fAddProdId, size_t theLongerProducerNameMaxCharCount);
   std::string GetParamNameString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fCrossSectionInfoWanted, bool fAddIdInfos, bool fMakeTooltipXmlEncode, size_t theLongerProducerNameMaxCharCount, bool fTimeSerialViewCase, bool doNewDataHighlight, bool fShowModelOriginTime, boost::shared_ptr<NFmiFastQueryInfo> possibleInfo);
   NFmiColor GetParamTextColor(NFmiInfoData::Type dataType, bool useArchiveModelData);
   std::string GetEditingDataString(const std::string &theNameStr, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiLanguage lang, const std::string &theOrigTimeFormat);
   std::string GetTotalMapViewStatusBarStr(CtrlViewDocumentInterface* theCtrlViewDocumentInterface, const NFmiPoint& theLatlon);
   std::string GetFixedLatlonStr(const NFmiPoint &theLatlon);
   std::string Point2String(const NFmiPoint& p);
   NFmiPoint String2Point(const std::string& str);

   boost::shared_ptr<NFmiFastQueryInfo> GetLatestLastTimeObservation(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, bool fCrossSectionInfoWanted);
   std::string GetArchiveOrigTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool fGetCurrentDataFromQ2Server, const std::string &theDictionaryTokenForOrigTimeFormat);
   std::string GetLatestObservationTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const std::string &theTimeFormat, bool fCrossSectionInfoWanted);
   std::string XmlEncode(const std::string &src);
   FmiDirection CalcFollowingParamWindowViewPosition(FmiDirection currentPosition, bool forward);
   FmiDirection MoveTimeBoxPositionForward(FmiDirection currentPosition);
   bool IsConsideredAsNewData(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, int modelRunIndex, bool isMacroParam);

   template<typename T>
   bool IsEqualEnough(T value1, T value2, T usedEpsilon)
   {
       if(::fabs(static_cast<double>(value1 - value2)) < usedEpsilon)
           return true;
       return false;
   }

   using FileNameWithTimeList = std::list<std::pair<std::string, std::time_t>>;
   FileNameWithTimeList TimeSortFiles(FileNameWithTimeList filesWithTimesCopy, bool descending = true);
   double CalcAngle(const NFmiPoint& point1, const NFmiPoint& point2);
   double CalcAngle(double x, double y);
   bool IsModularParameter(FmiParameterName theParam);
   std::string GetMacroParamFormula(NFmiMacroParamSystem& macroParamSystem, const boost::shared_ptr<NFmiDrawParam>& theDrawParam);
   std::string MakeMacroParamRelatedFinalErrorMessage(const std::string& baseMessage, const std::exception* exceptionPtr, boost::shared_ptr<NFmiDrawParam>& theDrawParam, const std::string& macroParamSystemRootPath);
   void SetMacroParamErrorMessage(const std::string& errorText, CtrlViewDocumentInterface& ctrlViewDocumentInterface, std::string* possibleTooltipErrorTextOut = nullptr);
   void DeleteFilesWithPattern(const std::string& directoryPath, const std::string& fileNamePattern, std::list<std::string>* deletedFileNamesOut = nullptr);
   void DeleteFilesWithPattern(const std::string& filePathPattern, std::list<std::string>* deletedFileNamesOut = nullptr);
   void DeleteFilesWithPatternAndLog(const std::string& filePathPattern, const std::string &logMessageStart, CatLog::Severity severity, CatLog::Category category);
   std::string GetFileExtension(const std::string& filePath);
   std::string GetParentPath(const std::string& filePath);

   // Haetaan sortatusta container:ista value:ta lähimmän arvon elementin iteraattori.
   // Koodi haettu: https://stackoverflow.com/questions/698520/search-for-nearest-value-in-an-array-of-doubles-in-c
   template <typename BidirectionalIterator, typename T>
   BidirectionalIterator GetClosestValue(BidirectionalIterator first,
       BidirectionalIterator last,
       const T& value)
   {
       BidirectionalIterator before = std::lower_bound(first, last, value);

       if(before == first) return first;
       if(before == last)  return --last; // iterator must be bidirectional

       BidirectionalIterator after = before;
       --before;

       return (*after - value) < (value - *before) ? after : before;
   }

   // Haetaan sortatusta container:ista value:ta lähimmän arvon elementin indeksi.
   // Koodi haettu: https://stackoverflow.com/questions/698520/search-for-nearest-value-in-an-array-of-doubles-in-c
   template <typename BidirectionalIterator, typename T>
   std::size_t GetClosestIndex(BidirectionalIterator first,
       BidirectionalIterator last,
       const T& value)
   {
       return std::distance(first, GetClosestValue(first, last, value));
   }

   template <typename ValueType, typename Container>
   size_t GetClosestValueIndex(ValueType value, const Container& container)
   {
       return GetClosestIndex(container.begin(), container.end(), value);
   }

   template <typename ValueType, typename Container>
   ValueType GetClosestValueFromContainer(ValueType value, const Container& container)
   {
       return container.at(GetClosestValueIndex(value, container));
   }

   // Tälle annettu container ei sää sisältää std::string olioita, koska sille ei ole std::to_string funktiota.
   template<typename Container>
   std::string MakeCommaSeparatedStringFromValues(const Container& container)
   {
       // Use std::accumulate to concatenate strings with comma separation, also use std::to_string for each object in container
       std::string result = std::accumulate(container.begin(), container.end(), std::string{},
           [](const std::string& accumulated, const auto& nextValue) 
           {
               auto nextString = std::to_string(nextValue);
               return accumulated.empty() ? nextString : accumulated + ", " + nextString;
           }
       );
       return result;
   }

   // Tässä on viimein tapa miten tehdään template funtio, joka saa jonkun stl containerin, joka sisältää tietyn tyyppisiä olioita.
   template <template<typename...> class Container, typename... Args>
   std::string MakeCommaSeparatedStringFromStrings(const Container<std::string, Args...>& container)
   {
       // Use std::accumulate to concatenate strings with comma separation, also use std::to_string for each object in container
       std::string result = std::accumulate(container.begin(), container.end(), std::string{},
           [](const std::string& accumulated, const auto& nextString)
           {
               return accumulated.empty() ? nextString : accumulated + ", " + nextString;
           }
       );
       return result;
   }

} // namespace CtrlViewUtils
