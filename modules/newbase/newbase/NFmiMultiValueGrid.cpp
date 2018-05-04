// ======================================================================
/*!
 * \file NFmiMultiValueGrid.cpp
 * \brief Implementation of class NFmiMultiValueGrid
 */
// ======================================================================
/*!
 * \class NFmiMultiValueGrid
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiMultiValueGrid.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 *
 * \todo Don't bother setting
 */
// ----------------------------------------------------------------------

NFmiMultiValuedGrid::~NFmiMultiValuedGrid()
{
  if (itsParamDescriptor) delete itsParamDescriptor;
  itsParamDescriptor = nullptr;
  if (itsTimeDescriptor) delete itsTimeDescriptor;
  itsTimeDescriptor = nullptr;
  if (itsVPlaceDescriptor) delete itsVPlaceDescriptor;
  itsVPlaceDescriptor = nullptr;
  if (itsMultiData) delete itsMultiData;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theGrid Undocumented
 * \param theParamDescriptor Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 */
// ----------------------------------------------------------------------

NFmiMultiValuedGrid::NFmiMultiValuedGrid(const NFmiGrid &theGrid,
                                         const NFmiParamDescriptor &theParamDescriptor,
                                         const NFmiTimeDescriptor &theTimeDescriptor,
                                         const NFmiVPlaceDescriptor &theVPlaceDescriptor)
    : NFmiGrid(theGrid), itsMultiData(nullptr), itsCurrentDataStatus(0)
{
  itsParamDescriptor = new NFmiParamDescriptor(theParamDescriptor);
  itsTimeDescriptor = new NFmiTimeDescriptor(theTimeDescriptor);
  itsHPlaceDescriptor = nullptr;
  itsVPlaceDescriptor = new NFmiVPlaceDescriptor(theVPlaceDescriptor);
  itsRefRawData = nullptr;
  Init();
}

// ----------------------------------------------------------------------
/*!
 * Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMultiValuedGrid::Init()
{
  itsMultiData = new NFmiDataPool();
  if (!itsMultiData->Init(NFmiGridBase::Size() * NFmiQueryInfo::Size()))
  {
    std::cout << "Out of memory. Execution failed.";
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Remove unnecessary last return statement
 */
// ----------------------------------------------------------------------

bool NFmiMultiValuedGrid::FirstValue()
{
  Save();
  NFmiQueryInfo::First();
  return Edit();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMultiValuedGrid::Save()
{
  float *sourceAddress = nullptr;

  if (!itsMultiData) return false;

  // Mika: unsigned int
  // if(NFmiQueryInfo::Index() < 0 || NFmiQueryInfo::Index() >= NFmiQueryInfo::Size())

  if (NFmiQueryInfo::Index() >= NFmiQueryInfo::Size()) return false;
  if (!itsData->FloatValueAddress(0, &sourceAddress)) return false;

  itsMultiData->Index(itsCurrentDataStatus);
  return itsMultiData->MemCopy(NFmiGridBase::Size(), sourceAddress);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMultiValuedGrid::Edit()
{
  float *sourceAddress = nullptr;

  if (!itsMultiData) return false;
  // Mika: Index() is unsigned int
  // if(NFmiQueryInfo::Index() < 0 || NFmiQueryInfo::Index() >= NFmiQueryInfo::Size())

  if (NFmiQueryInfo::Index() >= NFmiQueryInfo::Size()) return false;
  if (!itsMultiData->FloatValueAddress(NFmiQueryInfo::Index() * NFmiGridBase::Size(),
                                       &sourceAddress))
    return false;

  itsData->First();
  return itsData->MemCopy(NFmiGridBase::Size(), sourceAddress);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

void NFmiMultiValuedGrid::UpDate()
{
  Save();
  itsCurrentDataStatus = NFmiQueryInfo::Index() * NFmiGridBase::Size();
  Edit();
}

// ----------------------------------------------------------------------
/*!
 * \param dx Undocumented
 * \param dy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiMultiValuedGrid::FloatValue(long dx, long dy)
{
  UpDate();
  return static_cast<float>(NFmiGrid::FloatValue(dx, dy));
}

// ----------------------------------------------------------------------
/*!
 * \param data Undcoumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMultiValuedGrid::FloatValue(float data)
{
  UpDate();
  return NFmiGrid::FloatValue(data);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiMultiValuedGrid::InterpolateToLatLonPoint(double newLon, double newLat, double &theValue)
{
  UpDate();
  return NFmiGrid::InterpolateToLatLonPoint(newLon, newLat, theValue);
}

// ======================================================================
