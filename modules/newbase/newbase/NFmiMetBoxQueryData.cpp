// ======================================================================
/*!
 * \file NFmiMetBoxQueryData.cpp
 * \brief Implementation of class NFmiMetBoxQueryData
 */
// ======================================================================
/*!
 * \class NFmiMetBoxQueryData
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiMetBoxQueryData.h"
#include "NFmiStationBag.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiMetBoxQueryData::~NFmiMetBoxQueryData()
{
  if (itsMetBox) delete itsMetBox;
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiMetBoxQueryData::NFmiMetBoxQueryData() : NFmiQueryData(), itsMetBox(nullptr) {}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theData The object being copied
 */
// ----------------------------------------------------------------------

NFmiMetBoxQueryData::NFmiMetBoxQueryData(const NFmiMetBoxQueryData& theData)

    = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfo Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetBoxQueryData::NFmiMetBoxQueryData(NFmiQueryInfo& theInfo)
    : NFmiQueryData(theInfo), itsMetBox(nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiMetBox* NFmiMetBoxQueryData::Value()
{
  Reset();
  NextTime();
  NextLocation();
  NextParam();

  // Set TimeBag Descritor
  TMetTime theFirstTime(ValidTime());
  unsigned long theNumber = TimeDesc()->Size();
  unsigned int i;
  for (i = 1; i < theNumber; i++)
  {
    NextTime();
  }
  TMetTime theLastTime(ValidTime());
  unsigned long theResolution = 180;
  NFmiTimeBag* theTimeBag = new NFmiTimeBag(theFirstTime, theLastTime, theResolution);

  // Set StationBag Descritor
  theNumber = HPlaceDesc()->Size();
  auto* theStation = new NFmiStation[theNumber];
#if 0
  for(i=0;i<theNumber;i++)
	{
	  // Mika: Ei toimi
	  // theStation[i] = Location();
	  assert(0);
	  // NextLocation();
	}
#endif
  auto* theStationBag = new NFmiStationBag(theStation, theNumber);

  // Set ParamBag Descritor
  theNumber = ParamDesc()->Size();
  auto* theParam = new NFmiDataIdent[theNumber];
  for (i = 0; i < theNumber; i++)
  {
    theParam[i] = Param();
    NextParam();
  }
  auto* theParamBag = new NFmiParamBag(theParam, theNumber);

  itsMetBox = new NFmiMetBox(*theTimeBag, *theStationBag, *theParamBag);
  NFmiMetBoxIterator theBoxIterator(itsMetBox);

  Reset();
  theBoxIterator.Reset();
  while (NextTime())
  {
    theBoxIterator.NextTime();
    ResetLocation();
    theBoxIterator.ResetStation();

    while (NextLocation())
    {
      theBoxIterator.NextStation();
      ResetParam();
      theBoxIterator.ResetParam();
      while (NextParam())
      {
        theBoxIterator.NextParam();
        (*itsMetBox)[theBoxIterator.CalcBoxIndex()] = FloatValue();
      }
    }
  }

  delete theTimeBag;
  delete theStationBag;
  delete theParamBag;

  return itsMetBox;
}

// ======================================================================
