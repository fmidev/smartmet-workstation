// ======================================================================
/*!
 * \file NFmiCalculatedAreaMask.cpp
 * \brief Implementation of class NFmiCalculatedAreaMask
 */
// ======================================================================
/*!
 * \class NFmiCalculatedAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================
/*
 * \class NFmiLatLonAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================
/*!
 * \class NFmiElevationAngleAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================
/*!
 * \class NFmiJulianDayAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================
/*!
 * \class NFmiLocalHourAreaMask
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiCalculatedAreaMask.h"
#include "NFmiArea.h"
#include "NFmiDataIdent.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiGrid.h"
#include "NFmiLocation.h"
#include "NFmiPoint.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiCalculatedAreaMask::~NFmiCalculatedAreaMask() { delete itsDataIdent; }
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiCalculatedAreaMask::NFmiCalculatedAreaMask() : NFmiAreaMaskImpl(), itsDataIdent(nullptr) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiCalculatedAreaMask::NFmiCalculatedAreaMask(const NFmiDataIdent& theParam,
                                               const NFmiCalculationCondition& theOperation)
    : NFmiAreaMaskImpl(theOperation, kCalculated, NFmiInfoData::kCalculatedValue),
      itsDataIdent(new NFmiDataIdent(theParam))
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiCalculatedAreaMask::NFmiCalculatedAreaMask(const NFmiCalculatedAreaMask& theMask)
    : NFmiAreaMaskImpl(theMask),
      itsDataIdent(theMask.itsDataIdent ? new NFmiDataIdent(*theMask.itsDataIdent) : nullptr)
{
}

NFmiAreaMask* NFmiCalculatedAreaMask::Clone() const { return new NFmiCalculatedAreaMask(*this); }

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiCalculatedAreaMask& NFmiCalculatedAreaMask::operator=(const NFmiCalculatedAreaMask& theMask)
{
  if (this != &theMask)
  {
    NFmiAreaMaskImpl::operator=(theMask);
    itsDataIdent = theMask.itsDataIdent ? new NFmiDataIdent(*theMask.itsDataIdent) : nullptr;
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCalculatedAreaMask::IsWantedParam(const NFmiDataIdent& theParam,
                                           const NFmiLevel* /* theLevel */) const
{
  if (itsDataIdent && (*itsDataIdent == theParam)) return true;
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiDataIdent* NFmiCalculatedAreaMask::DataIdent() const { return itsDataIdent; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiParam* NFmiCalculatedAreaMask::Param() const
{
  if (itsDataIdent) return itsDataIdent->GetParam();
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLatLonAreaMask::NFmiLatLonAreaMask() : NFmiCalculatedAreaMask() {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiLatLonAreaMask::NFmiLatLonAreaMask(const NFmiDataIdent& theParam,
                                       const NFmiCalculationCondition& theOperation)
    : NFmiCalculatedAreaMask(theParam, theOperation)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiLatLonAreaMask::NFmiLatLonAreaMask(const NFmiLatLonAreaMask& theMask)

    = default;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLatLonAreaMask::~NFmiLatLonAreaMask() = default;
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiLatLonAreaMask::Clone() const { return new NFmiLatLonAreaMask(*this); }
// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLatLonAreaMask::CalcValueFromLocation(const NFmiPoint& theLatLon) const
{
  if (itsDataIdent->GetParamIdent() == kFmiLatitude)
    return theLatLon.Y();
  else
    return theLatLon.X();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiLatLonAreaMask::MakeSubMaskString() const
{
  return NFmiString("NFmiLatLonAreaMask::MakeSubMaskString");
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiElevationAngleAreaMask::NFmiElevationAngleAreaMask() : NFmiLatLonAreaMask(), itsTime() {}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiElevationAngleAreaMask::NFmiElevationAngleAreaMask(const NFmiElevationAngleAreaMask& theMask)

    = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiElevationAngleAreaMask::NFmiElevationAngleAreaMask(const NFmiDataIdent& theParam,
                                                       const NFmiCalculationCondition& theOperation)
    : NFmiLatLonAreaMask(theParam, theOperation), itsTime()
{
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiElevationAngleAreaMask::~NFmiElevationAngleAreaMask() = default;
// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiElevationAngleAreaMask::Time(const NFmiMetTime& theTime)
{
  itsTime = theTime;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiElevationAngleAreaMask::Clone() const
{
  return new NFmiElevationAngleAreaMask(*this);
}

double NFmiElevationAngleAreaMask::Value(const NFmiCalculationParams& theCalculationParams,
                                         bool fUseTimeInterpolationAlways)
{
  if (fUseTimeInterpolationAlways)
    itsTime = theCalculationParams.itsTime;  // tämä on pakko tehdä kun käytetään esim.
                                             // SumT-tyyppisiä SmartTool-funktioita ja niiden
                                             // argumentteina on annettu tietyt argumentit jotka
                                             // ovat ajasta riippuvia.
  return CalcValueFromLocation(theCalculationParams.itsLatlon);
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiElevationAngleAreaMask::CalcValueFromLocation(const NFmiPoint& theLatLon) const
{
  NFmiLocation location(theLatLon);
  return location.ElevationAngle(itsTime);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiElevationAngleAreaMask::MakeSubMaskString() const
{
  return NFmiString("NFmiElevationAngleAreaMask::MakeSubMaskString");
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiJulianDayAreaMask::NFmiJulianDayAreaMask() : NFmiElevationAngleAreaMask(), itsJulianDay(0) {}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiJulianDayAreaMask::NFmiJulianDayAreaMask(const NFmiDataIdent& theParam,
                                             const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation), itsJulianDay(0)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiJulianDayAreaMask::NFmiJulianDayAreaMask(const NFmiJulianDayAreaMask& theMask)

    = default;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiJulianDayAreaMask::~NFmiJulianDayAreaMask() = default;
// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiJulianDayAreaMask::Time(const NFmiMetTime& theTime)
{
  NFmiElevationAngleAreaMask::Time(theTime);
  itsJulianDay = itsTime.GetJulianDay();
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiJulianDayAreaMask::Clone() const { return new NFmiJulianDayAreaMask(*this); }
// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented, unused
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiJulianDayAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  return itsJulianDay;  // tämä on jo laskettu Time-metodissa
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiJulianDayAreaMask::MakeSubMaskString() const
{
  return NFmiString("NFmiJulianDayAreaMask::MakeSubMaskString");
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLocalHourAreaMask::NFmiLocalHourAreaMask() : NFmiElevationAngleAreaMask() {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiLocalHourAreaMask::NFmiLocalHourAreaMask(const NFmiDataIdent& theParam,
                                             const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiLocalHourAreaMask::NFmiLocalHourAreaMask(const NFmiLocalHourAreaMask& theMask)

    = default;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLocalHourAreaMask::~NFmiLocalHourAreaMask() = default;
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiLocalHourAreaMask::Clone() const { return new NFmiLocalHourAreaMask(*this); }
// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLocalHourAreaMask::CalcValueFromLocation(const NFmiPoint& theLatLon) const
{
  NFmiTime locTime(itsTime.LocalTime(NFmiLocation(theLatLon)));
  return locTime.GetHour();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiLocalHourAreaMask::MakeSubMaskString() const
{
  return NFmiString("NFmiLocalHourAreaMask::MakeSubMaskString");
}

// ======================================================================

NFmiUtcHourAreaMask::~NFmiUtcHourAreaMask() = default;
NFmiUtcHourAreaMask::NFmiUtcHourAreaMask() : NFmiElevationAngleAreaMask() {}
NFmiUtcHourAreaMask::NFmiUtcHourAreaMask(const NFmiUtcHourAreaMask& theMask)

    = default;
NFmiUtcHourAreaMask::NFmiUtcHourAreaMask(const NFmiDataIdent& theParam,
                                         const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation)
{
}
NFmiAreaMask* NFmiUtcHourAreaMask::Clone() const { return new NFmiUtcHourAreaMask(*this); }
double NFmiUtcHourAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  return itsTime.GetHour();
}
const NFmiString NFmiUtcHourAreaMask::MakeSubMaskString() const
{
  return NFmiString("NFmiUtcHourAreaMask::MakeSubMaskString");
}

// ======================================================================

NFmiMinuteAreaMask::~NFmiMinuteAreaMask() = default;
NFmiMinuteAreaMask::NFmiMinuteAreaMask() : NFmiElevationAngleAreaMask() {}
NFmiMinuteAreaMask::NFmiMinuteAreaMask(const NFmiMinuteAreaMask& theMask)

    = default;
NFmiMinuteAreaMask::NFmiMinuteAreaMask(const NFmiDataIdent& theParam,
                                       const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation)
{
}
NFmiAreaMask* NFmiMinuteAreaMask::Clone() const { return new NFmiMinuteAreaMask(*this); }
double NFmiMinuteAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  return itsTime.GetMin();
}
const NFmiString NFmiMinuteAreaMask::MakeSubMaskString() const
{
  return NFmiString("NFmiMinuteAreaMask::MakeSubMaskString");
}

// ======================================================================

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask::NFmiForecastHourAreaMask(
    const boost::shared_ptr<NFmiFastQueryInfo>& theInfo)
    : NFmiElevationAngleAreaMask(), itsInfo(theInfo)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfo Undocumented
 * \param theParam Undocumented
 * \param theOperation Undocumented
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask::NFmiForecastHourAreaMask(
    const boost::shared_ptr<NFmiFastQueryInfo>& theInfo,
    const NFmiDataIdent& theParam,
    const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation), itsInfo(theInfo)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask::NFmiForecastHourAreaMask(const NFmiForecastHourAreaMask& theMask)
    : NFmiElevationAngleAreaMask(theMask), itsInfo(NFmiAreaMask::DoShallowCopy(theMask.itsInfo))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 *
 * \param theMask The other object being copied
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask& NFmiForecastHourAreaMask::operator=(
    const NFmiForecastHourAreaMask& theMask)
{
  if (this != &theMask)
  {
    NFmiElevationAngleAreaMask::operator=(theMask);
    itsInfo = NFmiAreaMask::DoShallowCopy(theMask.itsInfo);
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiForecastHourAreaMask::~NFmiForecastHourAreaMask() = default;
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiForecastHourAreaMask::Clone() const
{
  return new NFmiForecastHourAreaMask(*this);
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiForecastHourAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  return itsTime.DifferenceInHours(itsInfo->TimeDescriptor().FirstTime());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiForecastHourAreaMask::MakeSubMaskString() const
{
  return NFmiString("NFmiForecastHourAreaMask::MakeSubMaskString");
}

NFmiTimeStepAreaMask::~NFmiTimeStepAreaMask() = default;
NFmiTimeStepAreaMask::NFmiTimeStepAreaMask(const boost::shared_ptr<NFmiFastQueryInfo>& theInfo)
    : NFmiElevationAngleAreaMask(), itsInfo(theInfo)
{
}

NFmiTimeStepAreaMask::NFmiTimeStepAreaMask(const NFmiTimeStepAreaMask& theMask)
    : NFmiElevationAngleAreaMask(theMask), itsInfo(NFmiAreaMask::DoShallowCopy(theMask.itsInfo))
{
}

NFmiTimeStepAreaMask& NFmiTimeStepAreaMask::operator=(const NFmiTimeStepAreaMask& theMask)
{
  if (this != &theMask)
  {
    NFmiElevationAngleAreaMask::operator=(theMask);
    itsInfo = NFmiAreaMask::DoShallowCopy(theMask.itsInfo);
  }
  return *this;
}

NFmiTimeStepAreaMask::NFmiTimeStepAreaMask(const boost::shared_ptr<NFmiFastQueryInfo>& theInfo,
                                           const NFmiDataIdent& theParam,
                                           const NFmiCalculationCondition& theOperation)
    : NFmiElevationAngleAreaMask(theParam, theOperation), itsInfo(theInfo)
{
}

NFmiAreaMask* NFmiTimeStepAreaMask::Clone() const { return new NFmiTimeStepAreaMask(*this); }
double NFmiTimeStepAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  return (itsInfo->TimeResolution() /
          60.);  // palauttaa datan currentin ajan aikaresoluution tunneissa
}

const NFmiString NFmiTimeStepAreaMask::MakeSubMaskString() const
{
  return NFmiString("NFmiTimeStepAreaMask::MakeSubMaskString");
}

double NFmiTimeStepAreaMask::Value(const NFmiCalculationParams& theCalculationParams,
                                   bool fUseTimeInterpolationAlways)
{
  if (fUseTimeInterpolationAlways)
    itsInfo->Time(
        theCalculationParams.itsTime);  // asetetaan ensin aika kohdalleen ja sitten kysytään arvoa
  else
    itsInfo->TimeIndex(
        theCalculationParams
            .itsTimeIndex);  // asetetaan ensin aikaindeksi kohdalleen ja sitten kysytään arvoa
  return CalcValueFromLocation(
      theCalculationParams.itsLatlon);  // ollaan jo oikeassa ajassa, ei aikainterpolointia
}

// **********************************************************
// *****    NFmiGridSizeAreaMask    *************************
// **********************************************************

NFmiGridSizeAreaMask::~NFmiGridSizeAreaMask() = default;
NFmiGridSizeAreaMask::NFmiGridSizeAreaMask(const boost::shared_ptr<NFmiFastQueryInfo>& theInfo,
                                           bool calcXValue)
    : NFmiElevationAngleAreaMask(), itsInfo(theInfo), fCalcXValue(calcXValue)
{
}

NFmiGridSizeAreaMask::NFmiGridSizeAreaMask(const boost::shared_ptr<NFmiFastQueryInfo>& theInfo,
                                           const NFmiDataIdent& theParam,
                                           const NFmiCalculationCondition& theOperation,
                                           bool calcXValue)
    : NFmiElevationAngleAreaMask(theParam, theOperation), itsInfo(theInfo), fCalcXValue(calcXValue)
{
}

NFmiGridSizeAreaMask::NFmiGridSizeAreaMask(const NFmiGridSizeAreaMask& theMask)
    : NFmiElevationAngleAreaMask(theMask),
      itsInfo(NFmiAreaMask::DoShallowCopy(theMask.itsInfo)),
      fCalcXValue(theMask.fCalcXValue)
{
}

NFmiAreaMask* NFmiGridSizeAreaMask::Clone() const { return new NFmiGridSizeAreaMask(*this); }
double NFmiGridSizeAreaMask::Value(const NFmiCalculationParams& theCalculationParams,
                                   bool /* fUseTimeInterpolationAlways */)
{
  return CalcValueFromLocation(theCalculationParams.itsLatlon);  // ajalla ei ole väliä
}

double NFmiGridSizeAreaMask::CalcValueFromLocation(const NFmiPoint& /* theLatLon */) const
{
  if (itsInfo->IsGrid())
  {
    if (fCalcXValue)
      return itsInfo->Area()->WorldXYWidth() / (itsInfo->Grid()->XNumber() - 1);
    else
      return itsInfo->Area()->WorldXYHeight() / (itsInfo->Grid()->YNumber() - 1);
  }
  return kFloatMissing;

  /*
          // Tämä saattaisi olla maantieteellisesti oikea lasku tapa, mutta teen simppelin
     jakolaskun
          if(itsInfo->IsGrid() && itsInfo->Location(theLatLon))
          {
                  NFmiPoint gridPoint1(itsInfo->Grid()->GridPoint());
                  NFmiPoint gridPoint2(gridPoint1);
                  if(fCalcXValue)
                          gridPoint2.X((itsInfo->Grid()->XNumber() < gridPoint2.X()) ?
     gridPoint2.X()+1 : gridPoint2.X()-1);
                  else
                          gridPoint2.Y((itsInfo->Grid()->YNumber() < gridPoint2.Y()) ?
     gridPoint2.Y()+1 : gridPoint2.Y()-1);
                  NFmiPoint latlon1(itsInfo->Grid()->GridToLatLon(gridPoint1));
                  NFmiPoint latlon2(itsInfo->Grid()->GridToLatLon(gridPoint2));
                  NFmiLocation loc1(latlon1);
                  return loc1.Distance(latlon2);
          }
          return kFloatMissing;
  */
}

const NFmiString NFmiGridSizeAreaMask::MakeSubMaskString() const
{
  return NFmiString("NFmiTimeStepAreaMask::MakeSubMaskString");
}

// **********************************************************
// *****    NFmiGridSizeAreaMask    *************************
// **********************************************************
