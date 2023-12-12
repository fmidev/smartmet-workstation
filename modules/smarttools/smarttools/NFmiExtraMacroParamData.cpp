#include "NFmiExtraMacroParamData.h"
#include "NFmiInfoOrganizer.h"
#include <newbase/NFmiFastQueryInfo.h>

#include <boost/math/special_functions/round.hpp>

// *************************************************************
// *************  FindWantedInfoData defines *******************
// *************************************************************

FindWantedInfoData::FindWantedInfoData(boost::shared_ptr<NFmiFastQueryInfo> &foundInfo,
                                       const std::string &originalDataDescription,
                                       const std::set<ReasonForDataRejection> &rejectionReasons)
    : foundInfo_(foundInfo),
      originalDataDescription_(originalDataDescription),
      rejectionReasons_(rejectionReasons)
{
}

// *************************************************************
// *************  NFmiDefineWantedData defines *****************
// *************************************************************

NFmiDefineWantedData::NFmiDefineWantedData() = default;

NFmiDefineWantedData::NFmiDefineWantedData(NFmiInfoData::Type dataType,
                                           const NFmiParam &param,
                                           const std::string &originalDataString)
    : dataType_(dataType), param_(param), originalDataString_(originalDataString)
{
}

NFmiDefineWantedData::NFmiDefineWantedData(NFmiInfoData::Type dataType,
                                           const NFmiParam &param,
                                           const NFmiLevel *level,
                                           const std::string &originalDataString)
    : dataType_(dataType), param_(param), originalDataString_(originalDataString)
{
  levelPtr_.reset(level ? new NFmiLevel(*level) : nullptr);
}

NFmiDefineWantedData::NFmiDefineWantedData(const NFmiProducer &producer,
                                           FmiLevelType levelType,
                                           const std::string &originalDataString)
    : producer_(producer), levelType_(levelType), originalDataString_(originalDataString)
{
}

NFmiDefineWantedData::NFmiDefineWantedData(const NFmiProducer &producer,
                                           const NFmiParam &param,
                                           const NFmiLevel *level,
                                           const std::string &originalDataString,
                                           float offsetTimeInHours)
    : producer_(producer), param_(param), originalDataString_(originalDataString)
{
  levelPtr_.reset(level ? new NFmiLevel(*level) : nullptr);
  if(offsetTimeInHours > 0)
  {
    dataTriggerRelatedWaitForMinutes_ = boost::math::iround(offsetTimeInHours * 60.f);
  }
}

NFmiDefineWantedData::NFmiDefineWantedData(const NFmiDefineWantedData &other)
    : dataType_(other.dataType_),
      producer_(other.producer_),
      param_(other.param_),
      levelPtr_(),
      levelType_(other.levelType_),
      originalDataString_(other.originalDataString_),
      dataTriggerRelatedWaitForMinutes_(other.dataTriggerRelatedWaitForMinutes_)
{
  levelPtr_.reset(other.levelPtr_ ? new NFmiLevel(*other.levelPtr_) : nullptr);
}

NFmiDefineWantedData &NFmiDefineWantedData::operator=(const NFmiDefineWantedData &other)
{
  if (this != &other)
  {
    dataType_ = other.dataType_;
    producer_ = other.producer_;
    param_ = other.param_;
    levelPtr_.reset(other.levelPtr_ ? new NFmiLevel(*other.levelPtr_) : nullptr);
    levelType_ = other.levelType_;
    originalDataString_ = other.originalDataString_;
    dataTriggerRelatedWaitForMinutes_ = other.dataTriggerRelatedWaitForMinutes_;
  }
  return *this;
}

bool NFmiDefineWantedData::operator==(const NFmiDefineWantedData &other) const
{
  if (dataType_ != other.dataType_)
  {
    return false;
  }
  if (producer_ != other.producer_)
  {
    return false;
  }
  if (param_ != other.param_)
  {
    return false;
  }
  if (levelPtr_ && other.levelPtr_)
  {
    // Molemmissa oli level pointteri, vertaillaan levelin arvoja sitten
    if (*levelPtr_ != *other.levelPtr_)
    {
      return false;
    }
  }
  else if (!levelPtr_ && !other.levelPtr_)
  {
    // Kummassakain oli level pointteri nullptr, ei tarvitse tehd‰ mit‰‰n t‰ss‰, mutta
    // piti varmistaa t‰m‰kin tapaus erikseen, jotta else haaraan menee ne tapaukset, miss‰
    // toisessa on level ja toisessa ei.
  }
  else
  {
    return false;
  }
  if (levelType_ != other.levelType_)
  {
    return false;
  }
  if (dataTriggerRelatedWaitForMinutes_ != other.dataTriggerRelatedWaitForMinutes_)
  {
    return false;
  }
  // Huom! originalDataString_ ei tarvitse olla sama (T_ec == par4_prod240), sille ei tarkastelua
  return true;
}

bool NFmiDefineWantedData::IsEditedData() const
{
  return dataType_ == NFmiInfoData::kEditable;
}

bool NFmiDefineWantedData::IsProducerLevelType() const
{
  return producer_.GetIdent() != 0 && levelType_ != kFmiNoLevelType;
}

bool NFmiDefineWantedData::IsParamProducerLevel() const
{
  return producer_.GetIdent() != 0 && param_.GetIdent() != 0;
}

const NFmiLevel *NFmiDefineWantedData::UsedLevel() const
{
  if (levelPtr_)
  {
    if (levelPtr_->GetIdent() != 0)
    {
      return levelPtr_.get();
    }
  }

  return nullptr;
}

bool NFmiDefineWantedData::IsInUse() const
{
  return IsEditedData() || IsProducerLevelType() || IsParamProducerLevel();
}

// *************************************************************
// ***************  MultiParamData defines *********************
// *************************************************************

MultiParamData::MultiParamData() = default;

MultiParamData::MultiParamData(const NFmiDefineWantedData &paramData)
    : possibleParamData_(paramData)
{
}

MultiParamData::MultiParamData(const std::string &originalParamString,
                               const std::string macroParamFullPath)
    : possibleOriginalMacroParamPath_(originalParamString),
      possibleMacroParamFullPath_(macroParamFullPath)
{
}

bool MultiParamData::IsInUse() const
{
  if (possibleParamData_.IsInUse())
    return true;
  if (!possibleMacroParamFullPath_.empty())
    return true;

  return false;
}

bool MultiParamData::IsMacroParamCase() const
{
  if (possibleParamData_.IsInUse())
    return false;
  if (!possibleMacroParamFullPath_.empty())
    return true;

  return false;
}

// *************************************************************
// *************  NFmiExtraMacroParamData defines **************
// *************************************************************

NFmiExtraMacroParamData::NFmiExtraMacroParamData()
    : itsWantedResolutionData(),
      itsResolutionMacroParamData(),
      itsCalculationPoints(),
      itsCalculationPointProducers(),
      itsSymbolTooltipFile(),
      itsMacroParamDescription()
{
}

void NFmiExtraMacroParamData::Clear()
{
  *this = NFmiExtraMacroParamData();
}

void NFmiExtraMacroParamData::FinalizeData(NFmiInfoOrganizer &theInfoOrganizer)
{
  if (itsGivenResolutionInKm != kFloatMissing)
  {
    InitializeResolutionData(theInfoOrganizer.MacroParamData()->Area(),
                             NFmiPoint(itsGivenResolutionInKm, itsGivenResolutionInKm));
  }
  else
  {
    InitializeDataBasedResolutionData(theInfoOrganizer);
  }

  InitializeFixedBaseDataInfo(theInfoOrganizer);
  InitializeMultiParamData(theInfoOrganizer);

  if (!itsCalculationPointProducers.empty())
  {
    AddCalculationPointsFromData(theInfoOrganizer, itsCalculationPointProducers);
  }

  InitializeRelativeObservationRange(theInfoOrganizer, itsObservationRadiusInKm);
}

bool NFmiExtraMacroParamData::UseSpecialResolution() const
{
  return itsResolutionMacroParamData != nullptr;
}

void NFmiExtraMacroParamData::SetUsedAreaForData(boost::shared_ptr<NFmiFastQueryInfo> &theData,
                                                 const NFmiArea *theUsedArea)
{
  if (theData->Grid() && theUsedArea)
  {
    try
    {
      NFmiGrid grid(theUsedArea, theData->Grid()->XNumber(), theData->Grid()->YNumber());
      grid.Area()->SetXYArea(NFmiRect(0, 0, 1, 1));  // t‰ss‰ pit‰‰ laittaa xy-alue 0,0 - 1,1 :ksi,
                                                     // koska macroParam datat zoomataan sitten
                                                     // erikseen
      NFmiHPlaceDescriptor hplace(grid);
      theData->SetHPlaceDescriptor(hplace);
    }
    catch (...)
    {
      // ei tehd‰ mit‰‰n, otetaan vain poikkeukset kiinni
    }
  }
}

static void CalcUsedGridSize(const NFmiArea *usedArea,
                             int &gridSizeX,
                             int &gridSizeY,
                             const NFmiPoint &usedResolutionInKm)
{
  if (usedArea)
  {
    gridSizeX = boost::math::iround(usedArea->WorldXYWidth() / (usedResolutionInKm.X() * 1000.));
    gridSizeY = boost::math::iround(usedArea->WorldXYHeight() / (usedResolutionInKm.Y() * 1000.));
  }
}

void NFmiExtraMacroParamData::InitializeResolutionData(const NFmiArea *usedArea,
                                                       const NFmiPoint &usedResolutionInKm)
{
  int gridSizeX = 0;
  int gridSizeY = 0;
  ::CalcUsedGridSize(usedArea, gridSizeX, gridSizeY, usedResolutionInKm);

  itsResolutionMacroParamData =
      NFmiInfoOrganizer::CreateNewMacroParamData(gridSizeX, gridSizeY, NFmiInfoData::kMacroParam);
  // Pit‰‰ viel‰ s‰‰t‰‰ datan alue kartan zoomaus alueeseen. Se saadaan infoOrganizerin omasta
  // macroParamDatasta.
  SetUsedAreaForData(itsResolutionMacroParamData, usedArea);
}

void NFmiExtraMacroParamData::InitializeRelativeObservationRange(
    NFmiInfoOrganizer &theInfoOrganizer, float usedRangeInKm)
{
  if (usedRangeInKm == kFloatMissing)
  {
    itsObservationRadiusRelative = kFloatMissing;
  }
  else
  {
    const NFmiArea *usedArea = theInfoOrganizer.MacroParamData()->Area();
    double xRatio = (usedRangeInKm * 1000.) / usedArea->WorldXYWidth();
    double yRatio = (usedRangeInKm * 1000.) / usedArea->WorldXYHeight();
    itsObservationRadiusRelative = static_cast<float>((xRatio + yRatio) / 2.);
  }
}

void NFmiExtraMacroParamData::AdjustValueMatrixToMissing(
    const boost::shared_ptr<NFmiFastQueryInfo> &theData, NFmiDataMatrix<float> &theValueMatrix)
{
  if (theData)
  {
    const NFmiGrid *grid = theData->Grid();
    if (grid)
    {
      theValueMatrix.Resize(grid->XNumber(), grid->YNumber());
      theValueMatrix = kFloatMissing;
    }
  }
}

static bool IsPrimarySurfaceDataType(boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  NFmiInfoData::Type dataType = info->DataType();
  if (dataType == NFmiInfoData::kViewable || dataType == NFmiInfoData::kObservations ||
      dataType == NFmiInfoData::kKepaData || dataType == NFmiInfoData::kAnalyzeData)
    return true;
  else
    return false;
}

static bool IsPrimaryLevelDataType(boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  NFmiInfoData::Type dataType = info->DataType();
  if (dataType == NFmiInfoData::kViewable || dataType == NFmiInfoData::kHybridData ||
      dataType == NFmiInfoData::kSingleStationRadarData || dataType == NFmiInfoData::kAnalyzeData)
    return true;
  else
    return false;
}

static boost::shared_ptr<NFmiFastQueryInfo> FindWantedInfo(
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &theInfos,
    FmiLevelType theLevelType,
    std::set<ReasonForDataRejection> &rejectionReasonsOut,
    bool allowStationData)
{
  // T‰h‰n laitetaan talteen ei prim‰‰ri datatyyppi varmuuden varalle
  boost::shared_ptr<NFmiFastQueryInfo> backupData;
  bool searchSingleLevelData = (theLevelType == kFmiMeanSeaLevel);
  for (size_t i = 0; i < theInfos.size(); i++)
  {
    boost::shared_ptr<NFmiFastQueryInfo> &info = theInfos[i];
    if (allowStationData || info->Grid())
    {
      if (searchSingleLevelData)
      {
        if (info->SizeLevels() == 1)
        {
          if (::IsPrimarySurfaceDataType(info))
            return info;  // Palautetaan surface tapauksessa 1. yksi tasoinen 'prim‰‰ri' data
          else
            backupData = info;
        }
        else
          rejectionReasonsOut.insert(ReasonForDataRejection::WrongLevelStructure);
      }
      else
      {
        if (theLevelType == info->LevelType())
        {
          if (::IsPrimaryLevelDataType(info))
            return info;  // Palautetaan level tapauksessa 1. 'prim‰‰ri' data
          else
            backupData = info;
        }
        else
          rejectionReasonsOut.insert(ReasonForDataRejection::WrongLevelType);
      }
    }
    else
      rejectionReasonsOut.insert(ReasonForDataRejection::NoGridData);
  }
  return backupData;
}

static boost::shared_ptr<NFmiFastQueryInfo> FindWantedInfo(
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &theInfos,
    const NFmiParam &param,
    const NFmiLevel *level,
    std::set<ReasonForDataRejection> &rejectionReasonsOut,
    bool allowStationData)
{
  // T‰h‰n laitetaan talteen ei prim‰‰ri datatyyppi varmuuden varalle
  boost::shared_ptr<NFmiFastQueryInfo> backupData;
  bool searchSingleLevelData = (level == nullptr);
  for (auto &info : theInfos)
  {
    if (allowStationData || info->Grid())
    {
      if (info->Param(param))
      {
        if (searchSingleLevelData)
        {
          if (info->SizeLevels() == 1)
          {
            if (::IsPrimarySurfaceDataType(info))
              return info;  // Palautetaan surface tapauksessa 1. yksi tasoinen 'prim‰‰ri' data
            else
              backupData = info;
          }
          else
            rejectionReasonsOut.insert(ReasonForDataRejection::WrongLevelStructure);
        }
        else
        {
          if (info->Level(*level))
          {
            if (::IsPrimaryLevelDataType(info))
              return info;  // Palautetaan level tapauksessa 1. 'prim‰‰ri' data
            else
              backupData = info;
          }
          else
            rejectionReasonsOut.insert(ReasonForDataRejection::NoLevel);
        }
      }
      else
        rejectionReasonsOut.insert(ReasonForDataRejection::NoParameter);
    }
    else
      rejectionReasonsOut.insert(ReasonForDataRejection::NoGridData);
  }
  return backupData;
}

static std::string GetProducerInfoForResolutionError(const NFmiProducer &theProducer,
                                                     FmiLevelType theLevelType)
{
  std::string str = theProducer.GetName().CharPtr();
  str += " ";
  if (theLevelType == kFmiMeanSeaLevel)
    str += "surface";
  else if (theLevelType == kFmiPressureLevel)
    str += "pressure";
  else if (theLevelType == kFmiHybridLevel)
    str += "hybrid";
  else if (theLevelType == kFmiHeight)
    str += "height";

  return str;
}

// Oletus theInfo on hilamuotoista dataa, eli silt‰ lˆytyy area.
// Lasketaan hilan x- ja y-resoluutioiden arvot kilometreissa.
static NFmiPoint CalcDataBasedResolutionInKm(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  double resolutionX = theInfo->Area()->WorldXYWidth() / theInfo->GridXNumber();
  double resolutionY = theInfo->Area()->WorldXYHeight() / theInfo->GridYNumber();
  return NFmiPoint(resolutionX / 1000., resolutionY / 1000.);
}

void NFmiExtraMacroParamData::UseDataForResolutionCalculations(
    const NFmiArea *usedArea,
    boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    const std::string &dataDescriptionForErrorMessage)
{
  if (theInfo)
  {
    if (theInfo->IsGrid())
    {
      itsDataBasedResolutionInKm = CalcDataBasedResolutionInKm(theInfo);
      InitializeResolutionData(usedArea, itsDataBasedResolutionInKm);
    }
    else
    {
      throw std::runtime_error(std::string("Wanted 'resolution' data didn't have grid (") +
                               dataDescriptionForErrorMessage + ")");
    }
  }
  else
  {
    throw std::runtime_error(std::string("Could find wanted 'resolution' data for ") +
                             dataDescriptionForErrorMessage);
  }
}

bool NFmiExtraMacroParamData::IsMultiParamCase() const
{
  if (!itsMultiParamTooltipFile.empty() && itsMultiParam2.IsInUse())
    return true;
  else
    return false;
}

static std::string MakeMessageOfRejectionreasons(
    const std::set<ReasonForDataRejection> &rejectionReasons)
{
  if (rejectionReasons.empty())
  {
    return "unknown reason, error in program logic?";
  }

  auto wrongLevelTypeIter = rejectionReasons.find(ReasonForDataRejection::WrongLevelType);
  if (wrongLevelTypeIter != rejectionReasons.end())
  {
    return "no data with correct level type";
  }

  auto wrongLevelIter = rejectionReasons.find(ReasonForDataRejection::NoLevel);
  if (wrongLevelIter != rejectionReasons.end())
  {
    return "no data with correct level";
  }

  auto wrongLevelStructureIter = rejectionReasons.find(ReasonForDataRejection::WrongLevelStructure);
  if (wrongLevelStructureIter != rejectionReasons.end())
  {
    return "no correct single level data";
  }

  auto noParamIter = rejectionReasons.find(ReasonForDataRejection::NoParameter);
  if (noParamIter != rejectionReasons.end())
  {
    return "no correct parameter data";
  }

  auto noGridDataIter = rejectionReasons.find(ReasonForDataRejection::NoGridData);
  if (noGridDataIter != rejectionReasons.end())
  {
    return "no grid data found";
  }

  return "data not found, error in program logic?";
}

static std::string MakeFindWantedInfoErrorMessage(const std::string &operationName,
                                                  const FindWantedInfoData &findWantedInfoData)
{
  if (!findWantedInfoData.foundInfo_)
  {
    std::string message = "Couldn't find wanted '";
    message += operationName;
    message += "' data for ";
    message += findWantedInfoData.originalDataDescription_;
    message += ": ";
    message += ::MakeMessageOfRejectionreasons(findWantedInfoData.rejectionReasons_);
    return message;
  }
  return "";
}

void NFmiExtraMacroParamData::InitializeDataBasedResolutionData(NFmiInfoOrganizer &theInfoOrganizer)
{
  if (itsWantedResolutionData.IsInUse())
  {
    auto findWantedInfoData = FindWantedInfo(theInfoOrganizer, itsWantedResolutionData);
    if (findWantedInfoData.foundInfo_)
    {
      UseDataForResolutionCalculations(theInfoOrganizer.MacroParamData()->Area(),
                                       findWantedInfoData.foundInfo_,
                                       findWantedInfoData.originalDataDescription_);
    }
    else
    {
      throw std::runtime_error(::MakeFindWantedInfoErrorMessage("resolution", findWantedInfoData));
    }
  }
}

void NFmiExtraMacroParamData::InitializeFixedBaseDataInfo(NFmiInfoOrganizer &theInfoOrganizer)
{
  if (itsWantedFixedBaseData.IsInUse())
  {
    auto findWantedInfoData = FindWantedInfo(theInfoOrganizer, itsWantedFixedBaseData);
    if (findWantedInfoData.foundInfo_)
    {
      itFixedBaseDataInfo = findWantedInfoData.foundInfo_;
    }
    else
    {
      throw std::runtime_error(
          ::MakeFindWantedInfoErrorMessage("FixedBaseData", findWantedInfoData));
    }
  }
}

void NFmiExtraMacroParamData::InitializeMultiParamData(NFmiInfoOrganizer &theInfoOrganizer)
{
  InitializeMultiParamData(theInfoOrganizer, itsMultiParam2);
  InitializeMultiParamData(theInfoOrganizer, itsMultiParam3);
}

void NFmiExtraMacroParamData::InitializeMultiParamData(NFmiInfoOrganizer &theInfoOrganizer,
                                                       MultiParamData &multiParamData)
{
  if (multiParamData.IsInUse())
  {
    if (multiParamData.IsMacroParamCase())
      return;

    auto &wantedMultiParamData = multiParamData.possibleParamData();
    auto findWantedInfoData = FindWantedInfo(theInfoOrganizer, wantedMultiParamData);
    if (findWantedInfoData.foundInfo_)
    {
      wantedMultiParamData.dataType_ = findWantedInfoData.foundInfo_->DataType();
    }
    else
    {
      throw std::runtime_error(
          ::MakeFindWantedInfoErrorMessage("MultiParamData", findWantedInfoData));
    }
  }
}

FindWantedInfoData NFmiExtraMacroParamData::FindWantedInfo(NFmiInfoOrganizer &theInfoOrganizer,
                                                           const NFmiDefineWantedData &wantedData,
                                                           bool allowStationData)
{
  std::set<ReasonForDataRejection> rejectionReasons;
  boost::shared_ptr<NFmiFastQueryInfo> info;
  if (wantedData.IsEditedData())
  {
    auto editedInfo = theInfoOrganizer.FindInfo(NFmiInfoData::kEditable);
    if (editedInfo && (allowStationData || editedInfo->IsGrid()))
      info = editedInfo;
    else
      rejectionReasons.insert(ReasonForDataRejection::NoGridData);
  }
  else if (wantedData.IsProducerLevelType())
  {
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> infos =
        theInfoOrganizer.GetInfos(wantedData.producer_.GetIdent());
    auto levelType = wantedData.levelType_;
    info = ::FindWantedInfo(infos, levelType, rejectionReasons, allowStationData);
  }
  else if (wantedData.IsParamProducerLevel())
  {
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> infos =
        theInfoOrganizer.GetInfos(wantedData.producer_.GetIdent());
    info = ::FindWantedInfo(
        infos, wantedData.param_, wantedData.UsedLevel(), rejectionReasons, allowStationData);
  }
  return FindWantedInfoData(info, wantedData.originalDataString_, rejectionReasons);
}

static void AddCalculationPoints(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                 const NFmiArea *theArea,
                                 std::vector<NFmiPoint> &theCalculationPoints)
{
  if (theInfo && theArea)
  {
    // Infon pit‰‰ olla asema dataa.
    // Siin‰ ei saa olla datassa paikkatietoa, kuten SHIP ja BOUY datoissa on,
    // koska t‰llˆin niiden paikka muuttuu ajan mukana.
    if (!theInfo->Grid() && !theInfo->HasLatlonInfoInData())
    {
      for (theInfo->ResetLocation(); theInfo->NextLocation();)
      {
        const NFmiPoint &latlon = theInfo->LatLonFast();
        if (theArea->IsInside(latlon))
        {
          theCalculationPoints.push_back(latlon);
        }
      }
    }
  }
}

static void AddCalculationPoints(std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &theInfos,
                                 const NFmiArea *theArea,
                                 std::vector<NFmiPoint> &theCalculationPoints)
{
  for (size_t i = 0; i < theInfos.size(); i++)
  {
    ::AddCalculationPoints(theInfos[i], theArea, theCalculationPoints);
  }
}

void NFmiExtraMacroParamData::AddCalculationPointsFromData(
    NFmiInfoOrganizer &theInfoOrganizer, const std::vector<NFmiProducer> &theProducers)
{
  for (const auto &producer : theProducers)
  {
    std::vector<boost::shared_ptr<NFmiFastQueryInfo>> infos =
        theInfoOrganizer.GetInfos(producer.GetIdent());
    const NFmiArea *usedArea = theInfoOrganizer.MacroParamData()->Area();

    ::AddCalculationPoints(infos, usedArea, itsCalculationPoints);
  }
}

bool NFmiExtraMacroParamData::AddCalculationPointProducer(const NFmiProducer &theProducer)
{
  if (theProducer.GetIdent() != 0)
  {
    itsCalculationPointProducers.push_back(theProducer);
    return true;
  }
  return false;
}
