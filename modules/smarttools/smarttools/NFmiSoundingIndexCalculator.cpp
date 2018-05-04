// ======================================================================
/*!
 * \file NFmiSoundingIndexCalculator.cpp
 *
 * Tämä luokka laskee erilaisia luotausi ndeksejä annettujen querinfojen avulla.
 * Mm. CAPE, CIN, LCL, BulkShear StormRelatedHellicity jne.
 */
// ======================================================================

#include "NFmiDrawParam.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiSoundingData.h"
#include "NFmiSoundingDataOpt1.h"
#include "NFmiSoundingFunctions.h"
#include "NFmiSoundingIndexCalculator.h"
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiGrid.h>
#include <newbase/NFmiQueryData.h>
#include <newbase/NFmiQueryDataUtil.h>
#include <newbase/NFmiValueString.h>

#ifndef BOOST_DISABLE_THREADS

#ifdef _MSC_VER
#pragma warning(disable : 4244)  // boost:in thread kirjastosta tulee ikävästi 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4244)  // laitetaan 4244 takaisin päälle, koska se on tärkeä (esim. double
                                 // -> int auto castaus varoitus)
#endif

#endif  // BOOST_DISABLE_THREADS

using namespace NFmiSoundingFunctions;

bool NFmiSoundingIndexCalculator::FillSoundingData(
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiSoundingData &theSoundingData,
    const NFmiMetTime &theTime,
    const NFmiLocation &theLocation,
    const boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo)
{
  if (theInfo)
  {
    if (theInfo->IsGrid())
      return theSoundingData.FillSoundingData(theInfo,
                                              theTime,
                                              theInfo->OriginTime(),
                                              theLocation.GetLocation(),
                                              theLocation.GetName(),
                                              theGroundDataInfo);
    else
      return theSoundingData.FillSoundingData(theInfo, theTime, theInfo->OriginTime(), theLocation);
  }
  return false;
}

bool NFmiSoundingIndexCalculator::FillSoundingDataOpt1(
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
    NFmiSoundingDataOpt1 &theSoundingData,
    const NFmiMetTime &theTime,
    const NFmiLocation &theLocation,
    const boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo)
{
  if (theInfo)
  {
    if (theInfo->IsGrid())
      return theSoundingData.FillSoundingData(theInfo,
                                              theTime,
                                              theInfo->OriginTime(),
                                              theLocation.GetLocation(),
                                              theLocation.GetName(),
                                              theGroundDataInfo);
    else
      return theSoundingData.FillSoundingData(theInfo, theTime, theInfo->OriginTime(), theLocation);
  }
  return false;
}

static bool FillSoundingData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                             NFmiSoundingData &theSoundingData,
                             const NFmiMetTime &theTime,
                             const NFmiPoint &theLatlon)
{
  static NFmiString bsName("bsname");
  if (theInfo)
  {
    if (theInfo->IsGrid())
      return theSoundingData.FillSoundingData(theInfo,
                                              theTime,
                                              theInfo->OriginTime(),
                                              theLatlon,
                                              bsName,
                                              boost::shared_ptr<NFmiFastQueryInfo>());
  }
  return false;
}

static bool FillSoundingDataOpt1(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                 NFmiSoundingDataOpt1 &theSoundingDataOpt1,
                                 const boost::shared_ptr<NFmiFastQueryInfo> &thePossibleGroundInfo,
                                 const NFmiMetTime &theTime,
                                 const NFmiPoint &theLatlon,
                                 bool useFastFill)
{
  static NFmiString bsName("bsname");
  if (theInfo)
  {
    if (theInfo->IsGrid())
      return theSoundingDataOpt1.FillSoundingData(theInfo,
                                                  theTime,
                                                  theInfo->OriginTime(),
                                                  theLatlon,
                                                  bsName,
                                                  thePossibleGroundInfo,
                                                  useFastFill);
  }
  return false;
}

bool NFmiSoundingIndexCalculator::IsSurfaceBasedSoundingIndex(
    FmiSoundingParameters theSoundingParameter)
{
  if (theSoundingParameter >= kSoundingParLCLSurBas &&
      theSoundingParameter <= kSoundingParCAPE_TT_SurBas)
    return true;
  else
    return false;
}

#if 0  // NEVER USED
static bool FillSurfaceValuesFromInfo(NFmiSmartInfo *theInfo, NFmiSoundingData &theSoundingData, const NFmiPoint &theLatLon, float &theT, float &theTd)
{
	theInfo->Param(kFmiTemperature);
	float T = theInfo->InterpolatedValue(theLatLon);
	theT = T;
	theInfo->Param(kFmiDewPoint);
	float Td = theInfo->InterpolatedValue(theLatLon);
	theTd = Td;
	if(T != kFloatMissing && Td != kFloatMissing)
	{
		theSoundingData.SetTandTdSurfaceValues(T, Td);
		return true;
	}
	else
		return false;
}
#endif

static void CheckIfStopped(NFmiStopFunctor *theStopFunctor)
{
  if (theStopFunctor && theStopFunctor->Stop())
    throw NFmiStopThreadException();
}

static void CalcAllSoundingIndexParamFields(boost::shared_ptr<NFmiFastQueryInfo> &theSourceInfo,
                                            boost::shared_ptr<NFmiFastQueryInfo> &theResultInfo,
                                            const boost::shared_ptr<NFmiFastQueryInfo> &thePossibleGroundInfo,
                                            bool useFastFill,
                                            NFmiStopFunctor *theStopFunctor)
{
  // bool fObsDataFound = false; // toistaiseksi ei käytössä
  // bool useAnalyzeData = false; // toistaiseksi ei käytössä

  NFmiSoundingDataOpt1 soundingDataOpt1;
  unsigned long counter = 0;
  for (theResultInfo->ResetLocation(); theResultInfo->NextLocation();)
  {
    try
    {
      // bool surfaceBaseStatus = false;
      if (useFastFill)
        theSourceInfo->LocationIndex(theResultInfo->LocationIndex());
      ::FillSoundingDataOpt1(theSourceInfo,
                             soundingDataOpt1,
                             thePossibleGroundInfo,
                             theResultInfo->Time(),
                             theResultInfo->LatLon(),
                             useFastFill);
      if (theSourceInfo->Grid() && !soundingDataOpt1.IsDataGood())
        continue;  // jos oltiin mallidatassa ja datassa oli tiettyjä puutteita, ei tehdä laskentoja

      for (theResultInfo->ResetParam(); theResultInfo->NextParam();)
      {
        counter++;
        if (counter % 20 == 0)
          ::CheckIfStopped(
              theStopFunctor);  // joka 20 hila/paramtrilla -pisteellä katsotaan, pitääkö lopettaa

        FmiSoundingParameters soundingParameter =
            static_cast<FmiSoundingParameters>(theResultInfo->Param().GetParamIdent());
        // bool surfaceBasedCalculation =
        // NFmiSoundingIndexCalculator::IsSurfaceBasedSoundingIndex(soundingParameter); // onko
        // surfacebased???

        // HUOM!!!! muista muuttaa luotaus-parametri pelkäksi surface arvoksi, koska loppu menee
        // itsestään sitten
        float valueOpt1 =
            NFmiSoundingIndexCalculator::CalcOpt1(soundingDataOpt1, soundingParameter);
        theResultInfo->FloatValue(valueOpt1);
      }
    }
    catch (NFmiStopThreadException &)
    {
      break;
    }
    catch (...)
    {
      // jouduin laittamaan try-catch blokin tänne, koska aina silloin tällöin jossain vaiheessa
      // lentää poikkeus, joka lopettaa laskennat
      // tässä ei tehdä mitään, mutta laskennat jatkuvat ainakin seuraavasta pisteestä...
    }
  }
}

static void CalculatePartOfSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theSourceInfo,
                                        boost::shared_ptr<NFmiFastQueryInfo> &theResultInfo,
                                        const boost::shared_ptr<NFmiFastQueryInfo> &thePossibleGroundInfo,
                                        unsigned long startTimeIndex,
                                        unsigned long endTimeIndex,
                                        bool useFastFill,
                                        NFmiStopFunctor *theStopFunctor,
                                        int index,
                                        bool fDoCerrReporting)
{
  try
  {
    if (theSourceInfo->IsGrid() == false || theResultInfo->IsGrid() == false)
      throw std::runtime_error(
          "Error in CalculatePartOfSoundingData, source or result data was non grid-data.");

    for (unsigned long i = startTimeIndex; i <= endTimeIndex; i++)
    {
      if (theResultInfo->TimeIndex(i))
      {
        if (useFastFill == false ||
            theSourceInfo->TimeIndex(
                theResultInfo->TimeIndex()))  // optimointia, molemmissa samat ajat!!!
        {
          if (fDoCerrReporting)
            std::cerr << "thread nro: " << index << " starting time step nro: " << i << std::endl;
          ::CheckIfStopped(theStopFunctor);
          ::CalcAllSoundingIndexParamFields(
              theSourceInfo, theResultInfo, thePossibleGroundInfo, useFastFill, theStopFunctor);
        }
      }
    }
  }
  catch (NFmiStopThreadException & /* stopException */)
  {
    // lopetetaan sitten kun on niin haluttu
    if (fDoCerrReporting)
      std::cerr << "thread nro: " << index << " stops because it was told to do so." << std::endl;
  }
  catch (std::exception &e)
  {
    // lopetetaan muutenkin, jos tulee poikkeus
    if (fDoCerrReporting)
      std::cerr << "thread nro: " << index << " stops because exception was thrown:\n" << e.what()
                << std::endl;
  }
  catch (...)
  {
    // lopetetaan muutenkin, jos tulee poikkeus
    if (fDoCerrReporting)
      std::cerr << "thread nro: " << index << " stops because unknown error." << std::endl;
  }
  if (fDoCerrReporting)
    std::cerr << "thread nro: " << index << " end here." << std::endl;
}

static void CalculateSoundingDataOneTimeStepAtTime(
    boost::shared_ptr<NFmiFastQueryInfo> &theSourceInfo,
    boost::shared_ptr<NFmiFastQueryInfo> &theResultInfo,
    const boost::shared_ptr<NFmiFastQueryInfo> &thePossibleGroundInfo,
    NFmiTimeIndexCalculator &theTimeIndexCalculator,
    bool useFastFill,
    NFmiStopFunctor *theStopFunctor,
    int index,
    bool fDoCerrReporting)
{
  try
  {
    if (theSourceInfo->IsGrid() == false || theResultInfo->IsGrid() == false)
      throw std::runtime_error(
          "Error in CalculatePartOfSoundingData, source or result data was non grid-data.");

    unsigned long workedTimeIndex = 0;
    for (; theTimeIndexCalculator.GetCurrentTimeIndex(workedTimeIndex);)
    {
      if (theResultInfo->TimeIndex(workedTimeIndex))
      {
        if (useFastFill == false ||
            theSourceInfo->TimeIndex(
                theResultInfo->TimeIndex()))  // optimointia, molemmissa samat ajat!!!
        {
          if (fDoCerrReporting)
            std::cerr << "thread nro: " << index << " starting time step nro: " << workedTimeIndex
                      << std::endl;
          ::CheckIfStopped(theStopFunctor);
          ::CalcAllSoundingIndexParamFields(
              theSourceInfo, theResultInfo, thePossibleGroundInfo, useFastFill, theStopFunctor);
        }
      }
    }
  }
  catch (NFmiStopThreadException & /* stopException */)
  {
    // lopetetaan sitten kun on niin haluttu
    if (fDoCerrReporting)
      std::cerr << "thread nro: " << index << " stops because it was told to do so." << std::endl;
  }
  catch (std::exception &e)
  {
    // lopetetaan muutenkin, jos tulee poikkeus
    if (fDoCerrReporting)
      std::cerr << "thread nro: " << index << " stops because exception was thrown:\n" << e.what()
                << std::endl;
  }
  catch (...)
  {
    // lopetetaan muutenkin, jos tulee poikkeus
    if (fDoCerrReporting)
      std::cerr << "thread nro: " << index << " stops because unknown error." << std::endl;
  }
  if (fDoCerrReporting)
    std::cerr << "thread nro: " << index << " end here." << std::endl;
}

// Jos useFastFill on true, on datoilla sama hila ja aika descriptor rakenne
// theMaxThreadCount -parametrilla voidaan rajoittaa käytettävien threadien määrää. Jos sen arvo on
// <=0,
// ei rajoitusta ole ja käytetään koneen kaikkia coreja (paitsi jos fUseOnlyOneThread = true,
// jolloin
// käytetään vain yhtä threadia).
void NFmiSoundingIndexCalculator::CalculateWholeSoundingData(NFmiQueryData &theSourceData,
                                                             NFmiQueryData &theResultData,
                                                             NFmiQueryData *thePossibleGroundData,
                                                             bool useFastFill,
                                                             bool fDoCerrReporting,
                                                             NFmiStopFunctor *theStopFunctor,
                                                             bool fUseOnlyOneThread,
                                                             int theMaxThreadCount)
{
  NFmiSoundingFunctions::CalcDP(1, 56);  // tämä funktio pitää varmistaa että se on alustettu, koska
                                         // siellä on pari staattista muuttujaa, jotka
  // alustetaan ensimmäisellä kerralla ja multi-threaddaavassa jutussa se voisi olla ongelma.

  unsigned long timeSize = theResultData.Info()->SizeTimes();
  unsigned int usedThreadCount = boost::thread::hardware_concurrency();
  if (theMaxThreadCount > 0 && usedThreadCount > static_cast<unsigned int>(theMaxThreadCount))
    usedThreadCount = static_cast<unsigned int>(theMaxThreadCount);  // jos on haluttu säätää maksim
  // threadien määrää, säädetään
  // maksimia jos usedThreadCount
  // olisi muuten ylittänyt sen.

  usedThreadCount = NFmiQueryDataUtil::CalcOptimalThreadCount(usedThreadCount, timeSize);

  if (fUseOnlyOneThread || usedThreadCount < 2)
  {  // jos aikoja oli alle kaksi, lasketaan data yhdessä funktiossa

    if (fDoCerrReporting)
      std::cerr << "making data in single thread" << std::endl;
    boost::shared_ptr<NFmiFastQueryInfo> sourceInfo(new NFmiFastQueryInfo(&theSourceData));
    boost::shared_ptr<NFmiFastQueryInfo> resultInfo(new NFmiFastQueryInfo(&theResultData));
    boost::shared_ptr<NFmiFastQueryInfo> possibleGroundInfo(thePossibleGroundData ? new NFmiFastQueryInfo(thePossibleGroundData) : nullptr);
    ::CalculatePartOfSoundingData(
        sourceInfo, resultInfo, possibleGroundInfo, 0, timeSize - 1, useFastFill, theStopFunctor, 1, fDoCerrReporting);
  }
  else
  {
    if (fDoCerrReporting)
      std::cerr << "making data in multiple threads" << std::endl;

    theSourceData.LatLonCache();  // Ennen multi-thread laskuja pitää varmistaa että kunkin datan
                                  // (source + result) latlon-cache on alustettu, muutern tulee
                                  // ongelmia.
    theResultData.LatLonCache();
    if(thePossibleGroundData)
        thePossibleGroundData->LatLonCache();

    // pakko luoda dynaamisesti eri threadeille tarvittavat kopiot source ja target datoista
    std::vector<boost::shared_ptr<NFmiFastQueryInfo> > resultInfos(usedThreadCount);
    std::vector<boost::shared_ptr<NFmiFastQueryInfo> > sourceInfos(usedThreadCount);
    std::vector<boost::shared_ptr<NFmiFastQueryInfo> > possibleGroundInfos(usedThreadCount);
    for (unsigned int i = 0; i < usedThreadCount; i++)
    {
      resultInfos[i] = boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(&theResultData));
      sourceInfos[i] = boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(&theSourceData));
      if(thePossibleGroundData)
          possibleGroundInfos[i] = boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(thePossibleGroundData));
    }

    NFmiTimeIndexCalculator timeIndexCalculator(0, timeSize - 1);
    boost::thread_group calcParts;
    for (unsigned int i = 0; i < usedThreadCount; i++)
      calcParts.add_thread(new boost::thread(::CalculateSoundingDataOneTimeStepAtTime,
                                             sourceInfos[i],
                                             resultInfos[i],
                                             possibleGroundInfos[i],
                                             boost::ref(timeIndexCalculator),
                                             useFastFill,
                                             theStopFunctor,
                                             i + 1,
                                             fDoCerrReporting));
    calcParts.join_all();  // odotetaan että threadit lopettavat

    if (fDoCerrReporting)
      std::cerr << "all threads ended" << std::endl;
  }
}

float NFmiSoundingIndexCalculator::CalcOpt1(NFmiSoundingDataOpt1 &theSoundingDataOpt1,
                                            FmiSoundingParameters theParam)
{
  double value = kFloatMissing;
  double xxxxValue = kFloatMissing;  // tämä on ns. hukka parametri, koska jotkut parametrit
                                     // syntyvät sivutuotteena ja tähän sijoitetaan aina se ei
                                     // haluttu parametri
  switch (theParam)
  {
    // **** 1. yksinkertaiset indeksit, tarvitaan vain soundingdata ***
    case kSoundingParSHOW:
      value = theSoundingDataOpt1.CalcSHOWIndex();
      break;
    case kSoundingParLIFT:
      value = theSoundingDataOpt1.CalcLIFTIndex();
      break;
    case kSoundingParKINX:
      value = theSoundingDataOpt1.CalcKINXIndex();
      break;
    case kSoundingParCTOT:
      value = theSoundingDataOpt1.CalcCTOTIndex();
      break;
    case kSoundingParVTOT:
      value = theSoundingDataOpt1.CalcVTOTIndex();
      break;
    case kSoundingParTOTL:
      value = theSoundingDataOpt1.CalcTOTLIndex();
      break;

    // **** 2. indeksit joissa tarvitaan myös pintakerros lasku tyyppi soundingdatan lisäksi ja
    // mahd. korkeus parametri ***
    // **** surface ****
    case kSoundingParLCLSur:
    case kSoundingParLCLSurBas:
      value = theSoundingDataOpt1.CalcLCLIndex(kLCLCalcSurface);
      break;
    case kSoundingParCAPESur:
    case kSoundingParCAPESurBas:
      value = theSoundingDataOpt1.CalcCAPE500Index(kLCLCalcSurface);
      break;
    case kSoundingParCAPE0_3kmSur:
    case kSoundingParCAPE0_3kmSurBas:
      value = theSoundingDataOpt1.CalcCAPE500Index(kLCLCalcSurface, 3000);
      break;
    case kSoundingParCAPE_TT_Sur:
    case kSoundingParCAPE_TT_SurBas:
      value = theSoundingDataOpt1.CalcCAPE_TT_Index(kLCLCalcSurface, -10, -40);
      break;
    case kSoundingParCINSur:
    case kSoundingParCINSurBas:
      value = theSoundingDataOpt1.CalcCINIndex(kLCLCalcSurface);
      break;
    case kSoundingParLCLHeightSur:
    case kSoundingParLCLHeightSurBas:
      value = theSoundingDataOpt1.CalcLCLHeightIndex(kLCLCalcSurface);
      break;

    // **** 500 m mixing ****
    case kSoundingParLCL500m:
      value = theSoundingDataOpt1.CalcLCLIndex(kLCLCalc500m2);
      break;
    case kSoundingParCAPE500m:
      value = theSoundingDataOpt1.CalcCAPE500Index(kLCLCalc500m2);
      break;
    case kSoundingParCAPE0_3km500m:
      value = theSoundingDataOpt1.CalcCAPE500Index(kLCLCalc500m2, 3000);
      break;
    case kSoundingParCAPE_TT_500m:
      value = theSoundingDataOpt1.CalcCAPE_TT_Index(kLCLCalc500m2, -10, -40);
      break;
    case kSoundingParCIN500m:
      value = theSoundingDataOpt1.CalcCINIndex(kLCLCalc500m2);
      break;
    case kSoundingParLCLHeight500m:
      value = theSoundingDataOpt1.CalcLCLHeightIndex(kLCLCalc500m2);
      break;

    // **** most unstable case ****
    case kSoundingParLCLMostUn:
      value = theSoundingDataOpt1.CalcLCLIndex(kLCLCalcMostUnstable);
      break;
    case kSoundingParCAPEMostUn:
      value = theSoundingDataOpt1.CalcCAPE500Index(kLCLCalcMostUnstable);
      break;
    case kSoundingParCAPE0_3kmMostUn:
      value = theSoundingDataOpt1.CalcCAPE500Index(kLCLCalcMostUnstable, 3000);
      break;
    case kSoundingParCAPE_TT_MostUn:
      value = theSoundingDataOpt1.CalcCAPE_TT_Index(kLCLCalcMostUnstable, -10, -40);
      break;
    case kSoundingParCINMostUn:
      value = theSoundingDataOpt1.CalcCINIndex(kLCLCalcMostUnstable);
      break;
    case kSoundingParLCLHeightMostUn:
      value = theSoundingDataOpt1.CalcLCLHeightIndex(kLCLCalcMostUnstable);
      break;

    // **** 3. indeksit jotka lasketaan jonkun muun indeksin yhteydessä, tarvitaan myös
    // mahdollisesti pintakerros lasku tyyppi ja soundingdata ***
    case kSoundingParLFCSur:
    case kSoundingParLFCSurBas:
      value = theSoundingDataOpt1.CalcLFCIndex(kLCLCalcSurface, xxxxValue);
      break;
    case kSoundingParELSur:
    case kSoundingParELSurBas:
      xxxxValue = theSoundingDataOpt1.CalcLFCIndex(kLCLCalcSurface, value);
      break;
    case kSoundingParLFCHeightSur:
    case kSoundingParLFCHeightSurBas:
      value = theSoundingDataOpt1.CalcLFCHeightIndex(kLCLCalcSurface, xxxxValue);
      break;
    case kSoundingParELHeightSur:
    case kSoundingParELHeightSurBas:
      xxxxValue = theSoundingDataOpt1.CalcLFCHeightIndex(kLCLCalcSurface, value);
      break;

    case kSoundingParLFC500m:
      value = theSoundingDataOpt1.CalcLFCIndex(kLCLCalc500m2, xxxxValue);
      break;
    case kSoundingParEL500m:
      xxxxValue = theSoundingDataOpt1.CalcLFCIndex(kLCLCalc500m2, value);
      break;
    case kSoundingParLFCHeight500m:
      value = theSoundingDataOpt1.CalcLFCHeightIndex(kLCLCalc500m2, xxxxValue);
      break;
    case kSoundingParELHeight500m:
      xxxxValue = theSoundingDataOpt1.CalcLFCHeightIndex(kLCLCalc500m2, value);
      break;

    case kSoundingParLFCMostUn:
      value = theSoundingDataOpt1.CalcLFCIndex(kLCLCalcMostUnstable, xxxxValue);
      break;
    case kSoundingParELMostUn:
      xxxxValue = theSoundingDataOpt1.CalcLFCIndex(kLCLCalcMostUnstable, value);
      break;
    case kSoundingParLFCHeightMostUn:
      value = theSoundingDataOpt1.CalcLFCHeightIndex(kLCLCalcMostUnstable, xxxxValue);
      break;
    case kSoundingParELHeightMostUn:
      xxxxValue = theSoundingDataOpt1.CalcLFCHeightIndex(kLCLCalcMostUnstable, value);
      break;

    // **** 4. indeksit joiden laskuissa tarvitaan korkeus parametreja ja soundingdata ***
    case kSoundingParBS0_6km:
      value = theSoundingDataOpt1.CalcBulkShearIndex(0, 6);
      break;
    case kSoundingParBS0_1km:
      value = theSoundingDataOpt1.CalcBulkShearIndex(0, 1);
      break;
    case kSoundingParSRH0_3km:
      value = theSoundingDataOpt1.CalcSRHIndex(0, 3);
      break;
    case kSoundingParSRH0_1km:
      value = theSoundingDataOpt1.CalcSRHIndex(0, 1);
      break;
    case kSoundingParWS1500m:
      value = theSoundingDataOpt1.CalcWSatHeightIndex(1500);
      break;
    case kSoundingParThetaE0_3km:
      value = theSoundingDataOpt1.CalcThetaEDiffIndex(0, 3);
      break;
    case kSoundingParGDI:
      value = theSoundingDataOpt1.CalcGDI();
      break;
    case kSoundingParNone:
      value = kFloatMissing;
      break;
  }

  return static_cast<float>(value);
}

float NFmiSoundingIndexCalculator::Calc(NFmiSoundingData &theSoundingData,
                                        FmiSoundingParameters theParam)
{
  double value = kFloatMissing;
  double xxxxValue = kFloatMissing;  // tämä on ns. hukka parametri, koska jotkut parametrit
                                     // syntyvät sivutuotteena ja tähän sijoitetaan aina se ei
                                     // haluttu parametri
  switch (theParam)
  {
    // **** 1. yksinkertaiset indeksit, tarvitaan vain soundingdata ***
    case kSoundingParSHOW:
      value = theSoundingData.CalcSHOWIndex();
      break;
    case kSoundingParLIFT:
      value = theSoundingData.CalcLIFTIndex();
      break;
    case kSoundingParKINX:
      value = theSoundingData.CalcKINXIndex();
      break;
    case kSoundingParCTOT:
      value = theSoundingData.CalcCTOTIndex();
      break;
    case kSoundingParVTOT:
      value = theSoundingData.CalcVTOTIndex();
      break;
    case kSoundingParTOTL:
      value = theSoundingData.CalcTOTLIndex();
      break;

    // **** 2. indeksit joissa tarvitaan myös pintakerros lasku tyyppi soundingdatan lisäksi ja
    // mahd. korkeus parametri ***
    // **** surface ****
    case kSoundingParLCLSur:
    case kSoundingParLCLSurBas:
      value = theSoundingData.CalcLCLIndex(kLCLCalcSurface);
      break;
    case kSoundingParCAPESur:
    case kSoundingParCAPESurBas:
      value = theSoundingData.CalcCAPE500Index(kLCLCalcSurface);
      break;
    case kSoundingParCAPE0_3kmSur:
    case kSoundingParCAPE0_3kmSurBas:
      value = theSoundingData.CalcCAPE500Index(kLCLCalcSurface, 3000);
      break;
    case kSoundingParCAPE_TT_Sur:
    case kSoundingParCAPE_TT_SurBas:
      value = theSoundingData.CalcCAPE_TT_Index(kLCLCalcSurface, -10, -40);
      break;
    case kSoundingParCINSur:
    case kSoundingParCINSurBas:
      value = theSoundingData.CalcCINIndex(kLCLCalcSurface);
      break;
    case kSoundingParLCLHeightSur:
    case kSoundingParLCLHeightSurBas:
      value = theSoundingData.CalcLCLHeightIndex(kLCLCalcSurface);
      break;

    // **** 500 m mixing ****
    case kSoundingParLCL500m:
      value = theSoundingData.CalcLCLIndex(kLCLCalc500m2);
      break;
    case kSoundingParCAPE500m:
      value = theSoundingData.CalcCAPE500Index(kLCLCalc500m2);
      break;
    case kSoundingParCAPE0_3km500m:
      value = theSoundingData.CalcCAPE500Index(kLCLCalc500m2, 3000);
      break;
    case kSoundingParCAPE_TT_500m:
      value = theSoundingData.CalcCAPE_TT_Index(kLCLCalc500m2, -10, -40);
      break;
    case kSoundingParCIN500m:
      value = theSoundingData.CalcCINIndex(kLCLCalc500m2);
      break;
    case kSoundingParLCLHeight500m:
      value = theSoundingData.CalcLCLHeightIndex(kLCLCalc500m2);
      break;

    // **** most unstable case ****
    case kSoundingParLCLMostUn:
      value = theSoundingData.CalcLCLIndex(kLCLCalcMostUnstable);
      break;
    case kSoundingParCAPEMostUn:
      value = theSoundingData.CalcCAPE500Index(kLCLCalcMostUnstable);
      break;
    case kSoundingParCAPE0_3kmMostUn:
      value = theSoundingData.CalcCAPE500Index(kLCLCalcMostUnstable, 3000);
      break;
    case kSoundingParCAPE_TT_MostUn:
      value = theSoundingData.CalcCAPE_TT_Index(kLCLCalcMostUnstable, -10, -40);
      break;
    case kSoundingParCINMostUn:
      value = theSoundingData.CalcCINIndex(kLCLCalcMostUnstable);
      break;
    case kSoundingParLCLHeightMostUn:
      value = theSoundingData.CalcLCLHeightIndex(kLCLCalcMostUnstable);
      break;

    // **** 3. indeksit jotka lasketaan jonkun muun indeksin yhteydessä, tarvitaan myös
    // mahdollisesti pintakerros lasku tyyppi ja soundingdata ***
    case kSoundingParLFCSur:
    case kSoundingParLFCSurBas:
      value = theSoundingData.CalcLFCIndex(kLCLCalcSurface, xxxxValue);
      break;
    case kSoundingParELSur:
    case kSoundingParELSurBas:
      xxxxValue = theSoundingData.CalcLFCIndex(kLCLCalcSurface, value);
      break;
    case kSoundingParLFCHeightSur:
    case kSoundingParLFCHeightSurBas:
      value = theSoundingData.CalcLFCHeightIndex(kLCLCalcSurface, xxxxValue);
      break;
    case kSoundingParELHeightSur:
    case kSoundingParELHeightSurBas:
      xxxxValue = theSoundingData.CalcLFCHeightIndex(kLCLCalcSurface, value);
      break;

    case kSoundingParLFC500m:
      value = theSoundingData.CalcLFCIndex(kLCLCalc500m2, xxxxValue);
      break;
    case kSoundingParEL500m:
      xxxxValue = theSoundingData.CalcLFCIndex(kLCLCalc500m2, value);
      break;
    case kSoundingParLFCHeight500m:
      value = theSoundingData.CalcLFCHeightIndex(kLCLCalc500m2, xxxxValue);
      break;
    case kSoundingParELHeight500m:
      xxxxValue = theSoundingData.CalcLFCHeightIndex(kLCLCalc500m2, value);
      break;

    case kSoundingParLFCMostUn:
      value = theSoundingData.CalcLFCIndex(kLCLCalcMostUnstable, xxxxValue);
      break;
    case kSoundingParELMostUn:
      xxxxValue = theSoundingData.CalcLFCIndex(kLCLCalcMostUnstable, value);
      break;
    case kSoundingParLFCHeightMostUn:
      value = theSoundingData.CalcLFCHeightIndex(kLCLCalcMostUnstable, xxxxValue);
      break;
    case kSoundingParELHeightMostUn:
      xxxxValue = theSoundingData.CalcLFCHeightIndex(kLCLCalcMostUnstable, value);
      break;

    // **** 4. indeksit joiden laskuissa tarvitaan korkeus parametreja ja soundingdata ***
    case kSoundingParBS0_6km:
      value = theSoundingData.CalcBulkShearIndex(0, 6);
      break;
    case kSoundingParBS0_1km:
      value = theSoundingData.CalcBulkShearIndex(0, 1);
      break;
    case kSoundingParSRH0_3km:
      value = theSoundingData.CalcSRHIndex(0, 3);
      break;
    case kSoundingParSRH0_1km:
      value = theSoundingData.CalcSRHIndex(0, 1);
      break;
    case kSoundingParWS1500m:
      value = theSoundingData.CalcWSatHeightIndex(1500);
      break;
    case kSoundingParThetaE0_3km:
      value = theSoundingData.CalcThetaEDiffIndex(0, 3);
      break;
    case kSoundingParNone:
      value = kFloatMissing;
      break;
  }

  return static_cast<float>(value);
}

float NFmiSoundingIndexCalculator::Calc(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                        const NFmiPoint &theLatlon,
                                        const NFmiMetTime &theTime,
                                        FmiSoundingParameters theParam)
{
  NFmiSoundingData soundingData;
  if (::FillSoundingData(theInfo, soundingData, theTime, theLatlon))
    return Calc(soundingData, theParam);
  return kFloatMissing;
}

static const NFmiParamDescriptor &GetSoundingIndexParams(void)
{
  static bool firstTime = true;
  static NFmiParamDescriptor soundingParams;
  if (firstTime)
  {
    firstTime = false;
    NFmiParamBag parBag;

    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParSHOW,
                                       "SHOW",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLIFT,
                                       "LIFT",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParKINX,
                                       "KINX",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCTOT,
                                       "CTOT",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParVTOT,
                                       "VTOT",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParTOTL,
                                       "TOTL",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));

    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLCLSur,
                                       "LCL (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLFCSur,
                                       "LFC (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParELSur,
                                       "EL (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLCLHeightSur,
                                       "LCL height (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLFCHeightSur,
                                       "LFC height (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParELHeightSur,
                                       "EL height (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPESur,
                                       "CAPE (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPE0_3kmSur,
                                       "CAPE 0-3km (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPE_TT_Sur,
                                       "CAPE -10-40 (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCINSur,
                                       "CIN (sur)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));

    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLCL500m,
                                       "LCL (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLFC500m,
                                       "LFC (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParEL500m,
                                       "EL (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLCLHeight500m,
                                       "LCL height (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLFCHeight500m,
                                       "LFC height (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParELHeight500m,
                                       "EL height (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPE500m,
                                       "CAPE (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPE0_3km500m,
                                       "CAPE 0-3km (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPE_TT_500m,
                                       "CAPE -10-40 (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCIN500m,
                                       "CIN (500m)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));

    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLCLMostUn,
                                       "LCL (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLFCMostUn,
                                       "LFC (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParELMostUn,
                                       "EL (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLCLHeightMostUn,
                                       "LCL height (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLFCHeightMostUn,
                                       "LFC height (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParELHeightMostUn,
                                       "EL height (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));

    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPEMostUn,
                                       "CAPE (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));

    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPE0_3kmMostUn,
                                       "CAPE 0-3km (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPE_TT_MostUn,
                                       "CAPE -10-40 (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCINMostUn,
                                       "CIN (mu)",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));

    /* xxxx jää kommenttiin
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLCLSurBas, "LCL (obs-bas)",
       kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLFCSurBas, "LFC (obs-bas)",
       kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParELSurBas, "EL (obs-bas)",
       kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLCLHeightSurBas, "LCL height
       (obs-bas)", kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParLFCHeightSurBas, "LFC height
       (obs-bas)", kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParELHeightSurBas, "EL height
       (obs-bas)", kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPESurBas, "CAPE (obs-bas)",
       kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPE0_3kmSurBas, "CAPE 0-3km
       (obs-bas)", kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCAPE_TT_SurBas, "CAPE -10-40
       (obs-bas)", kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
                    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParCINSurBas, "CIN (obs-bas)",
       kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly)));
    */

    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParBS0_6km,
                                       "BS 0-6km",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParBS0_1km,
                                       "BS 0-1km",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParSRH0_3km,
                                       "SRH 0-3km",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParSRH0_1km,
                                       "SRH 0-1km",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParWS1500m,
                                       "WS 1500m",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParThetaE0_3km,
                                       "ThetaE 0-3km",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));
    parBag.Add(NFmiDataIdent(NFmiParam(kSoundingParGDI,
                                       "GDI",
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       kFloatMissing,
                                       "%.1f",
                                       kLinearly)));

    soundingParams = NFmiParamDescriptor(parBag);
  }

  return soundingParams;
}

static NFmiQueryInfo MakeSoundingIndexInfo(NFmiQueryData &theSourceData,
                                           const std::string &theProducerName)
{
  NFmiFastQueryInfo fInfo(&theSourceData);

  NFmiParamDescriptor params = ::GetSoundingIndexParams();
  NFmiProducer usedProducer = *fInfo.Producer();
  if (theProducerName.empty() == false)
    usedProducer.SetName(theProducerName);
  else
  {
    NFmiString prodName = usedProducer.GetName();
    prodName += " (sounding index)";
    usedProducer.SetName(prodName);
  }
  params.SetProducer(usedProducer);  // tuottaja pitää asettaa oikeaksi

  NFmiQueryInfo info(
      params, fInfo.TimeDescriptor(), fInfo.HPlaceDescriptor());  // default vplaceDesc riittää kun
                                                                  // dataa lasketaan vain yhteen
                                                                  // tasoon
  return info;
}

boost::shared_ptr<NFmiQueryData> NFmiSoundingIndexCalculator::CreateNewSoundingIndexData(
    const std::string &theSourceFileFilter,
    const std::string &theProducerName,
    bool fDoCerrReporting,
    NFmiStopFunctor *theStopFunctor,
    bool fUseOnlyOneThread,
    int theMaxThreadCount)
{
    return NFmiSoundingIndexCalculator::CreateNewSoundingIndexData(
        theSourceFileFilter,
        theProducerName,
        "",
        fDoCerrReporting,
        theStopFunctor,
        fUseOnlyOneThread,
        theMaxThreadCount);
}

const std::string NFmiSoundingIndexCalculator::itsReadCompatibleGroundData_functionName = "ReadCompatibleGroundData";

// Jos thePossibleGroundDataFileFilter ei ole tyhjä, vaaditaan että löytyy data, jossa on sama tuottaja kuin theSourceData:ssa ja niissä sama origin aika.
// Jos pinta dataa ei vaadita, palautetaan tyhjää. Jos löytyy, palautetaan ladattu data. Jos pintadata vaaditaan, mutta sitä ei löydy, heitetään poikkeus.
static boost::shared_ptr<NFmiQueryData> ReadCompatibleGroundData(const std::string &thePossibleGroundDataFileFilter, boost::shared_ptr<NFmiQueryData> &theSourceData, bool fDoCerrReporting)
{
    if(thePossibleGroundDataFileFilter.empty())
        return boost::shared_ptr<NFmiQueryData>();
    else
    {
        boost::shared_ptr<NFmiQueryData> groundData(NFmiQueryDataUtil::ReadNewestData(thePossibleGroundDataFileFilter));
        if(!groundData)
            throw std::runtime_error(NFmiSoundingIndexCalculator::itsReadCompatibleGroundData_functionName + ": Cannot read required ground data " + thePossibleGroundDataFileFilter);
        else
        {
            if(theSourceData->Info()->Producer()->GetIdent() != groundData->Info()->Producer()->GetIdent())
                throw std::runtime_error(NFmiSoundingIndexCalculator::itsReadCompatibleGroundData_functionName + ": Sounding index source data had different producer id than in ground data " + thePossibleGroundDataFileFilter);
            else
            {
                if(theSourceData->OriginTime() != groundData->OriginTime())
                    throw std::runtime_error(NFmiSoundingIndexCalculator::itsReadCompatibleGroundData_functionName + ": Sounding index source data had different origin time than in ground data " + thePossibleGroundDataFileFilter);
                else
                {
                    if(!groundData->Info()->Param(kFmiPressureAtStationLevel))
                        throw std::runtime_error(NFmiSoundingIndexCalculator::itsReadCompatibleGroundData_functionName + ": Required ground data didn't have the needed PressureAtStationLevel parameter " + thePossibleGroundDataFileFilter);
                }
            }
        }
        return groundData;
    }
}

boost::shared_ptr<NFmiQueryData> NFmiSoundingIndexCalculator::CreateNewSoundingIndexData(
    const std::string &theSourceFileFilter,
    const std::string &theProducerName,
    const std::string &thePossibleGroundDataFileFilter,
    bool fDoCerrReporting,
    NFmiStopFunctor *theStopFunctor,
    bool fUseOnlyOneThread,
    int theMaxThreadCount)
{
    // 1. lue uusin pohjadata käyttöön
    boost::shared_ptr<NFmiQueryData> sourceData(
        NFmiQueryDataUtil::ReadNewestData(theSourceFileFilter));
    if(sourceData == 0)
        throw std::runtime_error("Error in CreateNewSoundingIndexData, cannot read source data.");
    else
    {
        if(fDoCerrReporting)
            std::cerr << "read qd-file: " << theSourceFileFilter << std::endl;
    }

    boost::shared_ptr<NFmiQueryData> possibleGroundData = ::ReadCompatibleGroundData(thePossibleGroundDataFileFilter, sourceData, fDoCerrReporting);

    return NFmiSoundingIndexCalculator::CreateNewSoundingIndexData(sourceData,
        possibleGroundData,
        theProducerName,
        fDoCerrReporting,
        theStopFunctor,
        fUseOnlyOneThread,
        theMaxThreadCount);
}

boost::shared_ptr<NFmiQueryData> NFmiSoundingIndexCalculator::CreateNewSoundingIndexData(
    boost::shared_ptr<NFmiQueryData> sourceData,
    boost::shared_ptr<NFmiQueryData> possibleGroundData,
    const std::string &theProducerName,
    bool fDoCerrReporting,
    NFmiStopFunctor *theStopFunctor,
    bool fUseOnlyOneThread,
    int theMaxThreadCount)
{
  // 1. luo sen avulla uusi qinfo pohjaksi
  NFmiQueryInfo soundingIndexInfo = ::MakeSoundingIndexInfo(*sourceData, theProducerName);
  // 2. luo uusi qdata
  boost::shared_ptr<NFmiQueryData> data(NFmiQueryDataUtil::CreateEmptyData(soundingIndexInfo));
  if (data == 0)
    throw std::runtime_error("Error in CreateNewSoundingIndexData, could not create result data.");
  // 4. täytä qdata
  NFmiSoundingIndexCalculator::CalculateWholeSoundingData(*sourceData.get(),
                                                          *data.get(),
                                                          possibleGroundData.get(),
                                                          true,
                                                          fDoCerrReporting,
                                                          theStopFunctor,
                                                          fUseOnlyOneThread,
                                                          theMaxThreadCount);

  if (theStopFunctor && theStopFunctor->Stop())
    return boost::shared_ptr<NFmiQueryData>();  // Jos Esim. SmartMetia ollaan sulkemassa ja laskut
                                                // on lopetettu kesken, ei haluta tallettaa kesken
                                                // jääneitä laskuja tiedostoon, eli palautetaan
                                                // tässä tyhjä smart-pointer
  else
    return data;
}
