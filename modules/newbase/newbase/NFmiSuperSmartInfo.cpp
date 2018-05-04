// ======================================================================
/*!
 * \file NFmiSuperSmartInfo.cpp
 * \brief Implementation of class NFmiSuperSmartInfo
 */
// ======================================================================
/*!
 * \class NFmiSuperSmartInfo
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiSuperSmartInfo.h"
#include "NFmiAreaMask.h"
#include "NFmiBitmapAreaMask.h"
#include "NFmiCalculator.h"
#include "NFmiDataModifierClasses.h"
#include "NFmiDataModifierDescriptor.h"
#include "NFmiGrid.h"
#include "NFmiParamDataModifier.h"
#include "NFmiRelativeDataIterator.h"

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiSuperSmartInfo::NFmiSuperSmartInfo()
    : NFmiFastQueryInfo(),
      itsFloatValueAccessLevel(kNormalAccess),
      fUseCalculator(false),
      fUseVarianceCalculator(false),
      fUseVariationCalculator(false),
      fUseTimeIntegrationCalculator(false),
      itsCalculator(nullptr),
      itsVariationCalculator(nullptr)  // laitetaan täällä tyhjät calculaattorit valmiiksi
      ,
      itsTimeIntegrationCalculator(nullptr),
      itsCurrentVarianceCalculator(
          new NFmiCalculator(static_cast<NFmiQueryInfo*>(nullptr),
                             nullptr))  // laitetaan täällä tyhjät calculaattorit valmiiksi
      ,
      itsDataModifierDescriptor(nullptr),
      itsAreaMask(nullptr),
      fUseAreaMask(false),
      fUseHelperBinaryMasks(false),
      itsCurrentHelperBinaryMask(nullptr),
      itsXYMaskBoundingBox(),
      itsHelperBinaryMaskList()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParamDescriptor Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theHPlaceDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 */
// ----------------------------------------------------------------------

NFmiSuperSmartInfo::NFmiSuperSmartInfo(const NFmiParamDescriptor& theParamDescriptor,
                                       const NFmiTimeDescriptor& theTimeDescriptor,
                                       const NFmiHPlaceDescriptor& theHPlaceDescriptor,
                                       const NFmiVPlaceDescriptor& theVPlaceDescriptor)
    : NFmiFastQueryInfo(
          theParamDescriptor, theTimeDescriptor, theHPlaceDescriptor, theVPlaceDescriptor),
      itsFloatValueAccessLevel(kNormalAccess),
      fUseCalculator(false),
      fUseVarianceCalculator(false),
      fUseVariationCalculator(false),
      fUseTimeIntegrationCalculator(false),
      itsCalculator(nullptr),
      itsVariationCalculator(nullptr)  // laitetaan täällä tyhjät calculaattorit valmiiksi
      ,
      itsTimeIntegrationCalculator(nullptr),
      itsCurrentVarianceCalculator(
          new NFmiCalculator(static_cast<NFmiQueryInfo*>(nullptr),
                             nullptr))  // laitetaan täällä tyhjät calculaattorit valmiiksi
      ,
      itsDataModifierDescriptor(nullptr),
      itsAreaMask(nullptr),
      fUseAreaMask(false),
      fUseHelperBinaryMasks(false),
      itsCurrentHelperBinaryMask(nullptr),
      itsXYMaskBoundingBox(),
      itsHelperBinaryMaskList()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param data Undocumented
 * \param theParamDescriptor Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theHPlaceDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 */
// ----------------------------------------------------------------------

NFmiSuperSmartInfo::NFmiSuperSmartInfo(NFmiQueryData* data,
                                       NFmiParamDescriptor* theParamDescriptor,
                                       NFmiTimeDescriptor* theTimeDescriptor,
                                       NFmiHPlaceDescriptor* theHPlaceDescriptor,
                                       NFmiVPlaceDescriptor* theVPlaceDescriptor)
    : NFmiFastQueryInfo(
          data, theParamDescriptor, theTimeDescriptor, theHPlaceDescriptor, theVPlaceDescriptor),
      itsFloatValueAccessLevel(kNormalAccess),
      fUseCalculator(false),
      fUseVarianceCalculator(false),
      fUseVariationCalculator(false),
      fUseTimeIntegrationCalculator(false),
      itsCalculator(nullptr),
      itsVariationCalculator(nullptr)  // laitetaan täällä tyhjät calculaattorit valmiiksi
      ,
      itsTimeIntegrationCalculator(nullptr),
      itsCurrentVarianceCalculator(
          new NFmiCalculator(static_cast<NFmiQueryInfo*>(nullptr),
                             nullptr))  // laitetaan täällä tyhjät calculaattorit valmiiksi
      ,
      itsDataModifierDescriptor(nullptr),
      itsAreaMask(nullptr),
      fUseAreaMask(false),
      fUseHelperBinaryMasks(false),
      itsCurrentHelperBinaryMask(nullptr),
      itsXYMaskBoundingBox(),
      itsHelperBinaryMaskList()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfo Undocumented
 */
// ----------------------------------------------------------------------

NFmiSuperSmartInfo::NFmiSuperSmartInfo(const NFmiQueryInfo& theInfo)
    : NFmiFastQueryInfo(theInfo),
      itsFloatValueAccessLevel(kNormalAccess),
      fUseCalculator(false),
      fUseVarianceCalculator(false),
      fUseVariationCalculator(false),
      fUseTimeIntegrationCalculator(false),
      itsCalculator(nullptr),
      itsVariationCalculator(nullptr)  // laitetaan täällä tyhjät calculaattorit valmiiksi
      ,
      itsTimeIntegrationCalculator(nullptr),
      itsCurrentVarianceCalculator(
          new NFmiCalculator(static_cast<NFmiQueryInfo*>(nullptr),
                             nullptr))  // laitetaan täällä tyhjät calculaattorit valmiiksi
      ,
      itsDataModifierDescriptor(nullptr),
      itsAreaMask(nullptr),
      fUseAreaMask(false),
      fUseHelperBinaryMasks(false),
      itsCurrentHelperBinaryMask(nullptr),
      itsXYMaskBoundingBox(),
      itsHelperBinaryMaskList()
{
}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theInfo The object being copied
 */
// ----------------------------------------------------------------------

NFmiSuperSmartInfo::NFmiSuperSmartInfo(const NFmiSuperSmartInfo& theInfo)
    : NFmiFastQueryInfo(theInfo),
      itsFloatValueAccessLevel(theInfo.itsFloatValueAccessLevel),
      fUseCalculator(theInfo.fUseCalculator),
      fUseVarianceCalculator(theInfo.fUseVarianceCalculator),
      fUseVariationCalculator(theInfo.fUseVariationCalculator),
      fUseTimeIntegrationCalculator(theInfo.fUseTimeIntegrationCalculator),
      itsCalculator(theInfo.itsCalculator),
      itsVariationCalculator(
          theInfo.itsVariationCalculator)  // laitetaan täällä tyhjät calculaattorit valmiiksi
      ,
      itsTimeIntegrationCalculator(nullptr)
      // HUOM!!!! TÄMÄ PITÄÄ HOITAA SITTEN KUNTOON (itsCurrentVarianceCalculator)
      //,itsCurrentVarianceCalculator(new NFmiCalculator((NFmiQueryInfo *)0, 0)) // laitetaan täällä
      // tyhjät calculaattorit valmiiksi
      ,
      itsCurrentVarianceCalculator(nullptr),
      itsDataModifierDescriptor(theInfo.itsDataModifierDescriptor),
      itsAreaMask(theInfo.itsAreaMask),
      fUseAreaMask(theInfo.fUseAreaMask),
      fUseHelperBinaryMasks(theInfo.fUseHelperBinaryMasks),
      itsCurrentHelperBinaryMask(
          nullptr)  // tämä alustetaan (virheellisesti) CopyHelperBinaryMasksAndInfo:issa
      ,
      itsXYMaskBoundingBox()  // tämä alustetaan (virheellisesti) CopyHelperBinaryMasksAndInfo:issa
      ,
      itsHelperBinaryMaskList()  // tämä alustetaan (virheellisesti)
                                 // CopyHelperBinaryMasksAndInfo:issa
{
  //	InitHelperBinaryMasks(); // tämä on väärin kopi-konstruktorissa, mutta
  //  en jaksa tehdä todellista kopiota itsHelperBinaryMaskList:ta ja muista
  CopyHelperBinaryMasksAndInfo(theInfo);
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiSuperSmartInfo::~NFmiSuperSmartInfo() { Destroy(); }
// ----------------------------------------------------------------------
/*!
 * Kopioi helperbinarymaskit omaan käyttöönsä
 *
 * \param theSSInfo Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::CopyHelperBinaryMasksAndInfo(const NFmiSuperSmartInfo& theSSInfo)
{
  fUseHelperBinaryMasks = theSSInfo.fUseHelperBinaryMasks;
  itsXYMaskBoundingBox = theSSInfo.itsXYMaskBoundingBox;

  const checkedVector<NFmiBitmapAreaMask*>& helperBinaryMaskList =
      theSSInfo.itsHelperBinaryMaskList;
  size_t size = helperBinaryMaskList.size();
  itsHelperBinaryMaskList.resize(size);

  // poikkeustapauksessa voi olla 0-pointteri, pitää tarkistaa kloonattaessa
  for (size_t i = 0; i < size; i++)
    if (helperBinaryMaskList[i])
      itsHelperBinaryMaskList[i] =
          static_cast<NFmiBitmapAreaMask*>(helperBinaryMaskList[i]->Clone());
  SetCurrentHelperBinaryMask();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::FirstLocation()
{
  // asettaa 1. maskatun locationin
  ResetLocation();
  return NextLocation();
}

// ----------------------------------------------------------------------
/*!
 * NextLocation hyppää seuraavaa maskattuun locationiin
 * (jos ei maskia, kaikki on maskattu!)
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::NextLocation()
{
  if (!fUseAreaMask) return NFmiFastQueryInfo::NextLocation();

  bool status = false;
  do
    status = NFmiFastQueryInfo::NextLocation();
  while (status && !IsLocationMasked(LocationIndex()));
  return status;
}

// ----------------------------------------------------------------------
/*!
 * PreviousLocation hyppää edelliseen maskattuun locationiin
 * (jos ei maskia, kaikki on maskattu!)
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::PreviousLocation()
{
  if (!fUseAreaMask) return NFmiFastQueryInfo::PreviousLocation();

  bool status = false;
  do
    status = NFmiFastQueryInfo::PreviousLocation();
  while (status && IsLocationMasked(LocationIndex()));
  return status;
}

// ----------------------------------------------------------------------
/*!
 * HUOM! ei etsi lähintä maskattua paikkaa vielä vaan vain paikan
 * (pitäisikö toteuttaa?!?!?!?)
 *
 * \param theLocation Undocumented
 * \param theMaxDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::NearestLocation(const NFmiLocation& theLocation, double theMaxDistance)
{
  return NFmiFastQueryInfo::NearestLocation(theLocation, theMaxDistance);
}

// ----------------------------------------------------------------------
/*!
 * HUOM! ei etsi lähintä maskattua paikkaa vielä vaan vain paikan
 * (pitäisikö toteuttaa?!?!?!?)
 *
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::NearestPoint(const NFmiPoint& theLatLonPoint)
{
  return NFmiFastQueryInfo::NearestPoint(theLatLonPoint);
}

// ----------------------------------------------------------------------
/*!
 * HUOM! En tiedä pitäisikö ottaa huomioon vain maskatut paikat?!?!?
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiSuperSmartInfo::SizeLocations() const
{
  return NFmiFastQueryInfo::SizeLocations();
}

// ----------------------------------------------------------------------
/*!
 * HUOM! En tiedä pitäisikö ottaa huomioon vain maskatut paikat?!?!?
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiSuperSmartInfo::SizeActiveLocations() const
{
  return NFmiFastQueryInfo::SizeActiveLocations();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiSuperSmartInfo::FloatValue() const
{
  // Käyttäjä on etukäteen määrännyt millä tasolla haluaa dataa
  // määrämällä itsFloatValueAccessLevel.
  // HUOM!! tätä ei kannata optimoida switch-lauseella, koska
  // eniten päädytään kuitenkin normalaccessiin!!!

  // 'normal' == raw-access eli FastInfo:n vastaava
  if (itsFloatValueAccessLevel == kNormalAccess) return RawFloatValue();

  // pyydetään varianssia
  if (itsFloatValueAccessLevel == kVarianceAccess) return VarianceFloatValue();

  // pyydetään aikaintegraatiota
  if (itsFloatValueAccessLevel == kTimeIntegrationAccess) return TimeIntegrationFloatValue();

  // epävarmuus laatikko laskut
  if (itsFloatValueAccessLevel == kVariationAccess) return VariationFloatValue();

  // 'integraattori'/calculaattori (korkeimman tason kutsu)
  if (itsFloatValueAccessLevel == kCalculatorAccess) return CalculationFloatValue();

  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam* NFmiSuperSmartInfo::CombinedValue()
{
// Käyttäjä on etukäteen määrännyt millä tasolla haluaa dataa
// määrämällä itsFloatValueAccessLevel.
// kommenteissa on oikea käyntijärjestys. Jos joku lisää jonkun
// haaran käyttöön (esim. TimeIntegrationCombinedValue:n), pitää
// se kommentoida esiin että 'vesiputous' toimisi oikeassa järjestyksessä.

#if 0
  if(itsFloatValueAccessLevel == kNormalAccess)
	return 0;//RawFloatValue();
  if(itsFloatValueAccessLevel == kVarianceAccess)
	return 0;//VarianceCombinedValue();
  if(itsFloatValueAccessLevel == kTimeIntegrationAccess)
	return 0;//TimeIntegrationCombinedValue();
#endif

  if (itsFloatValueAccessLevel == kVariationAccess) return VariationCombinedValue();
  if (itsFloatValueAccessLevel == kCalculatorAccess) return CalculationCombinedValue();
  return nullptr;

#if 0
  if(itsFloatValueAccessLevel == kCalculatorAccess)
	return CalculationCombinedValue();
  if(itsFloatValueAccessLevel == kVariationAccess)
	return VariationCombinedValue();
  if(itsFloatValueAccessLevel == kTimeIntegrationAccess)
	return 0;//TimeIntegrationCombinedValue();
  if(itsFloatValueAccessLevel == kVarianceAccess)
	return 0;//VarianceCombinedValue();
  return 0;//RawFloatValue();

#endif
}

// ----------------------------------------------------------------------
/*!
 * \param doVariationCalculation Undocumented
 * \param doCalculation Undocumented
 * \param doTimeIntegration Undocumented
 * \param doVarianceCalculation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiSuperSmartInfo::FloatValue(bool doVariationCalculation,
                                     bool doCalculation,
                                     bool doTimeIntegration,
                                     bool doVarianceCalculation)
{
  bool oldUseCalc = fUseCalculator;
  bool oldUseVari = fUseVariationCalculator;
  bool oldUseTimeIntegration = fUseTimeIntegrationCalculator;
  bool oldUseVariance = fUseVarianceCalculator;
  DataAccessLevel oldAccessLevel = itsFloatValueAccessLevel;

  itsFloatValueAccessLevel = kCalculatorAccess;

  fUseCalculator = doCalculation;
  fUseVariationCalculator = doVariationCalculation;
  fUseTimeIntegrationCalculator = doTimeIntegration;
  fUseVarianceCalculator = doVarianceCalculation;

  float value = FloatValue();

  fUseCalculator = oldUseCalc;
  fUseVariationCalculator = oldUseVari;
  fUseTimeIntegrationCalculator = oldUseTimeIntegration;
  fUseVarianceCalculator = oldUseVariance;
  itsFloatValueAccessLevel = oldAccessLevel;
  return value;
}

// ----------------------------------------------------------------------
/*!
 * \param doVariationCalculation Undocumented
 * \param doCalculation Undocumented
 * \param doTimeIntegration Undocumented
 * \param doVarianceCalculation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam* NFmiSuperSmartInfo::CombinedValue(bool doVariationCalculation,
                                                     bool doCalculation,
                                                     bool doTimeIntegration,
                                                     bool doVarianceCalculation)
{
  bool oldUseCalc = fUseCalculator;
  bool oldUseVari = fUseVariationCalculator;
  bool oldUseTimeIntegration = fUseTimeIntegrationCalculator;
  bool oldUseVariance = fUseVarianceCalculator;
  DataAccessLevel oldAccessLevel = itsFloatValueAccessLevel;
  itsFloatValueAccessLevel = kCalculatorAccess;

  fUseCalculator = doCalculation;
  fUseVariationCalculator = doVariationCalculation;
  fUseTimeIntegrationCalculator = doTimeIntegration;
  fUseVarianceCalculator = doVarianceCalculation;

  NFmiCombinedParam* value = nullptr;
  value = CombinedValue();

  fUseCalculator = oldUseCalc;
  fUseVariationCalculator = oldUseVari;
  fUseTimeIntegrationCalculator = oldUseTimeIntegration;
  fUseVarianceCalculator = oldUseVariance;
  itsFloatValueAccessLevel = oldAccessLevel;
  return value;
}

// ----------------------------------------------------------------------
/*!
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiSuperSmartInfo::PeekLocationValue(int theXOffset, int theYOffset) const
{
  // Peek...Value()-metodit pitää tehdä niin, että alkuperäiset
  // indeksit otetaan talteen, että tilanne voidaan palauttaa
  // datan kyselyn jälkeen.
  // Eri access-tasoisia Calculaattoreita varten paikka pitää
  // asettaa ensin kohdalleen, että Calculaattorit toimisivat oikein.

  unsigned long oldLocationIndex = LocationIndex();
  int wantedIndex = CalcPeekLocationIndex(oldLocationIndex, theXOffset, theYOffset);
  if (!const_cast<NFmiSuperSmartInfo*>(this)->LocationIndex(wantedIndex)) return kFloatMissing;

  float value = FloatValue();  // FloatValue hoitaa nyt 'peekkauksen'
  const_cast<NFmiSuperSmartInfo*>(this)->LocationIndex(
      oldLocationIndex);  // aseta vanhat indeksit paikoilleen eli info vanhaan 'asentoon'
  return value;
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiSuperSmartInfo::PeekTimeValue(int theTimeOffset)
{
  // Peek...Value()-metodit pitää tehdä niin, että alkuperäiset
  // indeksit otetaan talteen, että tilanne voidaan palauttaa datan
  // kyselyn jälkeen. Eri access-tasoisia Calculaattoreita varten
  // paikka pitää asettaa ensin kohdalleen, että Calculaattorit
  // toimisivat oikein.

  unsigned long oldTimeIndex = TimeIndex();

  // jos epäonnistui, palauta missing arvo, ei tarvitse asettaa oldTimeIndex:iä
  if (!TimeIndex(oldTimeIndex + theTimeOffset))
  {
    TimeIndex(oldTimeIndex);  // aseta vanhat indeksit paikoilleen
    return kFloatMissing;
  }
  float value = FloatValue();  // FloatValue hoitaa nyt 'peekkauksen'
  TimeIndex(oldTimeIndex);     // aseta vanhat indeksit paikoilleen
  return value;
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeOffset Undocumented
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiSuperSmartInfo::PeekValue(int theTimeOffset, int theXOffset, int theYOffset)
{
  // Peek...Value()-metodit pitää tehdä niin, että alkuperäiset
  // indeksit otetaan talteen,että tilanne voidaan palauttaa datan
  // kyselyn jälkeen. Eri access-tasoisia Calculaattoreita varten
  // paikka pitää asettaa ensin kohdalleen, että Calculaattorit
  // toimisivat oikein.

  unsigned long oldLocationIndex = LocationIndex();
  unsigned long oldTimeIndex = TimeIndex();
  int wantedIndex = CalcPeekLocationIndex(oldLocationIndex, theXOffset, theYOffset);
  bool status1 = LocationIndex(wantedIndex);
  bool status2 = TimeIndex(oldTimeIndex + theTimeOffset);
  float value =
      (status1 && status2) ? FloatValue() : kFloatMissing;  // FloatValue hoitaa nyt 'peekkauksen'
  LocationIndex(oldLocationIndex);  // aseta vanhat indeksit paikoilleen eli info vanhaan 'asentoon'
  TimeIndex(oldTimeIndex);          // aseta vanhat indeksit paikoilleen eli info vanhaan 'asentoon'
  return value;
}

// ----------------------------------------------------------------------
/*!
 * \param currentIndex Undocumented
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiSuperSmartInfo::CalcPeekLocationIndex(unsigned long currentIndex,
                                                        int theXOffset,
                                                        int theYOffset) const
{
  if (!IsGrid()) return (currentIndex + theXOffset);

  int currentXIndex = (currentIndex % itsGridXNumber) + theXOffset;
  int currentYIndex = (currentIndex / itsGridXNumber) + theYOffset;

  // voiko tämän seuraavan tarkistuksen poistaa, kun indeksi
  // tarkistetaan kuitenkin Index-metodissa??

  // x- ja y-indeksien pitää pysyä gridin sisällä offsettien kera!
  if (currentXIndex >= 0 && currentYIndex >= 0 && currentXIndex < int(itsGridXNumber) &&
      currentYIndex < int(itsGridYNumber))
    return (currentYIndex * itsGridXNumber + currentXIndex);

  // palauttaa -1 indeksin, jos peek menee hilaruudukon ulkopuolelle
  return static_cast<unsigned long>(-1);
}

// ----------------------------------------------------------------------
/*!
 * Laskee kurrentista location indeksistä (1-ulotteinen) kaksi
 * ulotteiseksi x ja y-indeksiksi. Indeksit alkavat 0:sta, virhetilanteessa
 * x ja y on -1:iä
 *
 * \param theIndex Undocumented
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::LocationIndex2XYPosition(unsigned long theIndex,
                                                  int* theXOffset,
                                                  int* theYOffset)
{
  if (theIndex > itsLocationSize)
  {
    *theXOffset = -1;
    *theYOffset = -1;
    return false;
  }
  else if (IsGrid())
  {
    *theXOffset = (theIndex % itsGridXNumber);
    *theYOffset = (theIndex / itsGridXNumber);
  }
  else
  {
    *theXOffset = theIndex;
    *theYOffset = -1;
  }
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return boost::shared_ptr
 */
// ----------------------------------------------------------------------

NFmiQueryInfo* NFmiSuperSmartInfo::Clone() const { return new NFmiSuperSmartInfo(*this); }
// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiSuperSmartInfo::Write(std::ostream& file) const
{
  NFmiFastQueryInfo::Write(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream& NFmiSuperSmartInfo::Read(std::istream& file)
{
  NFmiFastQueryInfo::Read(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Assignment operaor
 *
 * \param theInfo The object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiSuperSmartInfo& NFmiSuperSmartInfo::operator=(const NFmiSuperSmartInfo& theInfo)
{
  if (this != &theInfo)
  {
    NFmiFastQueryInfo::operator=(theInfo);

    itsFloatValueAccessLevel = theInfo.itsFloatValueAccessLevel;
    fUseCalculator = theInfo.fUseCalculator;
    fUseVarianceCalculator = theInfo.fUseVarianceCalculator;
    fUseVariationCalculator = theInfo.fUseVariationCalculator;
    fUseTimeIntegrationCalculator = theInfo.fUseTimeIntegrationCalculator;
    itsCalculator = theInfo.itsCalculator;
    itsVariationCalculator = theInfo.itsVariationCalculator;

    // HUOM!!!! TÄMÄ PITÄÄ HOITAA SITTEN KUNTOON
    // itsCurrentVarianceCalculator = theInfo.itsCurrentVarianceCalculator;

    itsDataModifierDescriptor = theInfo.itsDataModifierDescriptor;
    itsAreaMask = theInfo.itsAreaMask;
    fUseAreaMask = theInfo.fUseAreaMask;

    // tämä alustetaan (virheellisesti) CopyHelperBinaryMasksAndInfo:issa
    itsCurrentHelperBinaryMask = nullptr;
    CopyHelperBinaryMasksAndInfo(theInfo);
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Tuhoa sisäiset dynaamiset muuttujat
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::Destroy()
{
  delete itsCurrentVarianceCalculator;
  ClearHelperBinaryMasks();

  NFmiFastQueryInfo::Destroy();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

size_t NFmiSuperSmartInfo::Index() const
{
  // tämä saattaa muuttua, jos datan järjestystä muutetaan
  // (locationit sisimmäiseen looppiin!)

  return NFmiFastQueryInfo::Index();
}

// ----------------------------------------------------------------------
/*!
 * \param theLocationIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::IsLocationMasked(unsigned long theLocationIndex) const
{
  if (fUseAreaMask && itsAreaMask)
  {
    if (fUseHelperBinaryMasks && itsCurrentHelperBinaryMask)
      return itsCurrentHelperBinaryMask->IsMasked(theLocationIndex);
    else
      return itsAreaMask->IsMasked(HPlaceDescriptor().LatLon(theLocationIndex));
  }

  return true;  // jos ei maskia käytössä, on maski aina päällä!!!
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiSuperSmartInfo::VarianceFloatValue() const
{
  if (fUseVarianceCalculator && itsCurrentVarianceCalculator)
  {
    // accessleveliä pitää säätää tässä väliaikaisesti, että
    // calculator pääsee käsiksi 'alemman' tason dataan
    DataAccessLevel oldAccessLevel = itsFloatValueAccessLevel;
    itsFloatValueAccessLevel = kNormalAccess;

    auto value = static_cast<float>(itsCurrentVarianceCalculator->FloatValue());

    // palautetaan accesslevel
    itsFloatValueAccessLevel = oldAccessLevel;
    return value;
  }

  return RawFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiSuperSmartInfo::VariationFloatValue() const
{
  if (fUseVariationCalculator && itsVariationCalculator)
  {
    // accessleveliä pitää säätää tässä väliaikaisesti, että
    // calculator pääsee käsiksi 'alemman' tason dataan
    DataAccessLevel oldAccessLevel = itsFloatValueAccessLevel;
    itsFloatValueAccessLevel = kTimeIntegrationAccess;

    auto value = static_cast<float>(itsVariationCalculator->FloatValue());

    // palautetaan accesslevel
    itsFloatValueAccessLevel = oldAccessLevel;
    return value;
  }

  // mielestäni pitää kutsua seuraavaa tasoa, jos tältä tasolta
  // ei löydy calculaattoria
  return TimeIntegrationFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiSuperSmartInfo::TimeIntegrationFloatValue() const
{
  if (fUseTimeIntegrationCalculator && itsTimeIntegrationCalculator)
  {
    // accessleveliä pitää säätää tässä väliaikaisesti, että
    // calculator pääsee käsiksi 'alemman' tason dataan
    DataAccessLevel oldAccessLevel = itsFloatValueAccessLevel;
    itsFloatValueAccessLevel = kVarianceAccess;

    auto value = static_cast<float>(itsTimeIntegrationCalculator->FloatValue());

    itsFloatValueAccessLevel = oldAccessLevel;
    return value;
  }

  // mielestäni pitää kutsua seuraavaa tasoa, jos tältä tasolta
  // ei löydy calculaattoria
  return VarianceFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam* NFmiSuperSmartInfo::VariationCombinedValue()
{
  if (fUseVariationCalculator && itsVariationCalculator)
  {
    // accessleveliä pitää säätää tässä väliaikaisesti, että
    // calculator pääsee käsiksi 'alemman' tason dataan
    DataAccessLevel oldAccessLevel = itsFloatValueAccessLevel;
    itsFloatValueAccessLevel = kVarianceAccess;

    NFmiCombinedParam* value = itsVariationCalculator->CombinedValue();

    itsFloatValueAccessLevel = oldAccessLevel;
    return value;
  }

  // mielestäni pitää kutsua seuraavaa tasoa, jos tältä tasolta
  // ei löydy calculaattoria

  return nullptr;  // VarianceFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam* NFmiSuperSmartInfo::CalculationCombinedValue()
{
  if (fUseCalculator && itsCalculator)
  {
    // accessleveliä pitää säätää tässä väliaikaisesti, että
    // calculator pääsee käsiksi 'alemman' tason dataan
    DataAccessLevel oldAccessLevel = itsFloatValueAccessLevel;
    itsFloatValueAccessLevel = kVariationAccess;

    NFmiCombinedParam* value = itsCalculator->CombinedValue();

    itsFloatValueAccessLevel = oldAccessLevel;
    return value;
  }

  // mielestäni pitää kutsua seuraavaa tasoa, jos tältä tasolta
  // ei löydy calculaattoria

  return VariationCombinedValue();  // VarianceFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiSuperSmartInfo::CalculationFloatValue() const
{
  if (fUseCalculator && itsCalculator)
  {
    // accessleveliä pitää säätää tässä väliaikaisesti, että
    // calculator pääsee käsiksi 'alemman' tason dataan
    DataAccessLevel oldAccessLevel = itsFloatValueAccessLevel;
    itsFloatValueAccessLevel = kVariationAccess;

    auto value = static_cast<float>(itsCalculator->FloatValue());

    itsFloatValueAccessLevel = oldAccessLevel;
    return value;
  }

  // mielestäni pitää kutsua seuraavaa tasoa, jos tältä tasolta
  // ei löydy calculaattoria

  return VariationFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \param theCalculator Undocumented
 * \param useCalculator Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::SetCalculator(NFmiCalculator* theCalculator, bool useCalculator)
{
  itsCalculator = theCalculator;
  if (theCalculator)
    fUseCalculator = useCalculator;
  else  // jos 0-pointteri, otetaan pois käytöstä!!!
    fUseCalculator = false;
}

// ----------------------------------------------------------------------
/*!
 * \param theDescriptor Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::SetDataModifierDescriptor(NFmiDataModifierDescriptor* theDescriptor)
{
  itsDataModifierDescriptor = theDescriptor;  // HUOM!! Ei omista, pitäisikö???
  UpdateVarianceCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::UseVarianceCalculator(bool value)
{
  fUseVarianceCalculator = value;
  UpdateVarianceCalculator();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::FirstParam(bool fIgnoreSubParam)
{
  bool status = NFmiFastQueryInfo::FirstParam(fIgnoreSubParam);
  UpdateVarianceCalculator();
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::NextParam(bool fIgnoreSubParam)
{
  bool status = NFmiFastQueryInfo::NextParam(fIgnoreSubParam);
  UpdateVarianceCalculator();
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::PreviousParam(bool fIgnoreSubParam)
{
  bool status = NFmiFastQueryInfo::PreviousParam(fIgnoreSubParam);
  UpdateVarianceCalculator();
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::LastParam(bool fIgnoreSubParam)
{
  bool status = NFmiFastQueryInfo::LastParam(fIgnoreSubParam);
  UpdateVarianceCalculator();
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::Param(const NFmiParam& theParam)
{
  bool status = NFmiFastQueryInfo::Param(theParam);
  UpdateVarianceCalculator();
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::Param(const NFmiParam& theParam, const NFmiParam& theSubParam)
{
  bool status = NFmiFastQueryInfo::Param(theParam, theSubParam);
  UpdateVarianceCalculator();
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::Param(const NFmiDataIdent& theDataIdent)
{
  bool status = NFmiFastQueryInfo::Param(theDataIdent);
  UpdateVarianceCalculator();
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::FirstTime()
{
  // Ajansiirto-funktioissa pitää päivittää myös maskin aikaa
  unsigned long oldTimeIndex = itsTimeIndex;
  bool status = NFmiFastQueryInfo::FirstTime();
  TimeChanged(oldTimeIndex);
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::NextTime()
{
  unsigned long oldTimeIndex = itsTimeIndex;
  bool status = NFmiFastQueryInfo::NextTime();
  TimeChanged(oldTimeIndex);
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::LastTime()
{
  unsigned long oldTimeIndex = itsTimeIndex;
  bool status = NFmiFastQueryInfo::LastTime();
  TimeChanged(oldTimeIndex);
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::PreviousTime()
{
  unsigned long oldTimeIndex = itsTimeIndex;
  bool status = NFmiFastQueryInfo::PreviousTime();
  TimeChanged(oldTimeIndex);
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::Time(const NFmiMetTime& theTime)
{
  unsigned long oldTimeIndex = itsTimeIndex;
  // TimeChanged metodia ei saa kutsua, jos asetus feilaa
  bool status = NFmiFastQueryInfo::Time(theTime);
  if (status) TimeChanged(oldTimeIndex);
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::TimeIndex(unsigned long theIndex)
{
  unsigned long oldTimeIndex = itsTimeIndex;
  bool status = NFmiFastQueryInfo::TimeIndex(theIndex);
  TimeChanged(oldTimeIndex);
  return status;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------
void NFmiSuperSmartInfo::UpdateVarianceCalculator()
{
  if (itsDataModifierDescriptor)
  {
    const NFmiDataIdent& param = NFmiFastQueryInfo::Param();
    const NFmiLevel* level = Level();
    itsCurrentVarianceCalculator->SetData(this);
    NFmiParamDataModifier* modifier = itsDataModifierDescriptor->VarianceModifier(param, level);
    itsCurrentVarianceCalculator->SetDataModifier(modifier);
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::UpdateAreaMaskTime()
{
  if (fUseAreaMask && itsAreaMask)
  {
    SetCurrentHelperBinaryMask();
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::SetCurrentHelperBinaryMask()
{
  itsCurrentHelperBinaryMask = HelperBinaryMask(CalcAreaUnCertainty());
}

// ----------------------------------------------------------------------
/*!
 * \param theUsedVariationFactor
 * \result Undocumented
 */
// ----------------------------------------------------------------------

NFmiBitmapAreaMask* NFmiSuperSmartInfo::HelperBinaryMask(int theUsedVariationFactor)
{
  size_t size = itsHelperBinaryMaskList.size();
  if (theUsedVariationFactor < 0 || static_cast<std::size_t>(theUsedVariationFactor) >= size)
    return nullptr;
  else
    return itsHelperBinaryMaskList[theUsedVariationFactor];
}

// ----------------------------------------------------------------------
/*!
 * \param newStatus Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::UseHelperBinaryMasks(bool newStatus)
{
  fUseHelperBinaryMasks = newStatus;
  InitHelperBinaryMasks();  // pitää varmuuden vuoksi aina alustaa (voisi optimoida)
}

// ----------------------------------------------------------------------
/*!
 * Alustaa kaikki tarvittavat harvennusapu maskit
 * (eli kaikille epävarmuuksille 0-n).
 *
 * \return  Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::InitHelperBinaryMasks()
{
  if (fUseHelperBinaryMasks)
  {
    ClearHelperBinaryMasks();
    CalcXYMaskBoundingBox();
    // int start = AreaUnCertaintyStart();
    int end = AreaUnCertaintyEnd();
    itsHelperBinaryMaskList.resize(end + 1);
    for (int i = 0; i <= end; i++)
      itsHelperBinaryMaskList[i] = CreateHelperBinaryMask(i);
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theUsedVariationFactor Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiBitmapAreaMask* NFmiSuperSmartInfo::CreateHelperBinaryMask(int theUsedVariationFactor)
{
  if (theUsedVariationFactor == 0) return CreateZeroVariationHelperBinaryMask();

  auto* helperMask =
      new NFmiBitmapAreaMask(itsGridXNumber, itsGridYNumber, Area(), &Param(), Level());
  if (!helperMask) return nullptr;

  int moveByX = theUsedVariationFactor * 2 + 1;
  int moveByY = theUsedVariationFactor * 2 + 1;
  auto left = static_cast<int>(itsXYMaskBoundingBox.Left());
  auto right = static_cast<int>(itsXYMaskBoundingBox.Right());

  int moveXFromStart = 0;
  int moveXFactor = (right - left) % (moveByX);
  bool specialXCase = ((moveByX > 3) && ((moveByX - moveXFactor) ==
                                         1));  // jos totta, tihennetään harvennushilaa yhdellä
  if (specialXCase)
    moveByX--;
  else if (moveXFactor > 1)
    moveXFromStart = moveXFactor / 2;
  auto top = static_cast<int>(itsXYMaskBoundingBox.Top());
  auto bottom = static_cast<int>(itsXYMaskBoundingBox.Bottom());
  int moveYFromStart = 0;
  int moveYFactor = (bottom - top) % (moveByY);
  bool specialYCase = ((moveByY > 3) && ((moveByY - moveYFactor) == 1));
  if (specialYCase)
    moveByY--;
  else if (moveYFactor > 1)
    moveYFromStart = moveYFactor / 2;
  helperMask->Mask((bottom + top) / 2 * itsGridXNumber + (right + left) / 2, true);
  for (int j = top + moveYFromStart; j <= bottom; j += moveByY)
  {
    for (int i = left + moveXFromStart; i <= right; i += moveByX)
    {
      int idx = j * itsGridXNumber + i;
      helperMask->Mask(idx, true);
    }
  }
  return helperMask;
}

// ----------------------------------------------------------------------
/*!
 * Tämä luo itseasiassa maskin muotoisen binäärimaskin, kun ei ole
 * epävarmuutta mukana
 */
// ----------------------------------------------------------------------

NFmiBitmapAreaMask* NFmiSuperSmartInfo::CreateZeroVariationHelperBinaryMask()
{
  NFmiAreaMask* mask = AreaMask();
  auto* helperMask = new NFmiBitmapAreaMask;
  if (mask && mask->Info() && helperMask)
  {
    unsigned int oldLocationIndex = LocationIndex();
    bool oldUseAreaMaskStatus = UseAreaMask();
    // laitetaan maski pois päältä, niin bitmapmaski voi tehdä hommansa
    UseAreaMask(false);

    helperMask->Init(mask->Info(), AreaMask()->Condition());

    LocationIndex(oldLocationIndex);
    UseAreaMask(oldUseAreaMaskStatus);

    return helperMask;
  }
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * Tuhoaa maskit
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::ClearHelperBinaryMasks()
{
  size_t size = itsHelperBinaryMaskList.size();
  for (size_t i = 0; i < size; i++)
    delete itsHelperBinaryMaskList[i];
  itsHelperBinaryMaskList.clear();
}

// ----------------------------------------------------------------------
/*!
 * Laskee käytetyn maskin xy-indeksi-boundingboxin
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::CalcXYMaskBoundingBox()
{
  unsigned int oldLocationIndex = LocationIndex();
  bool oldUseAreaMaskStatus = UseAreaMask();
  // laitetaan maski päälle, niin liikutaan vain maskatuissa paikoissa
  UseAreaMask(true);
  bool oldUseHelperBinaryMasksState = fUseHelperBinaryMasks;
  fUseHelperBinaryMasks = false;
  NFmiDataModifierMinMax xMinMax;
  NFmiDataModifierMinMax yMinMax;
  int xOffset = -1, yOffset = -1;
  for (ResetLocation(); NextLocation();)
  {
    // tämän pitäisi olla turha testi
    if (LocationIndex2XYPosition(LocationIndex(), &xOffset, &yOffset))
    {
      xMinMax.Calculate(static_cast<float>(xOffset));
      yMinMax.Calculate(static_cast<float>(yOffset));
    }
  }
  itsXYMaskBoundingBox =
      NFmiRect(xMinMax.MinValue(), yMinMax.MaxValue(), xMinMax.MaxValue(), yMinMax.MinValue());

  LocationIndex(oldLocationIndex);
  UseAreaMask(oldUseAreaMaskStatus);
  fUseHelperBinaryMasks = oldUseHelperBinaryMasksState;
}

// ----------------------------------------------------------------------
/*!
 * Testi funktio. Tekee stringin tietylle epävarmuudelle tehdystä
 * harvennusmaskista
 *
 * Sisältää seuraavaa:
 *
 *  - n x m merkkiä (eli kuvaa hilaa)
 *  - '-' = ei maskia, ei harvennuspistettä
 *  - 'a' = maski, ei harvennuspistettä
 *  - '@' = maski ja harvennuspiste
 *  - 'C' = ei maskia, harvennuspiste
 *
 * \param theUsedVariationFactor
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiString NFmiSuperSmartInfo::HelperBinaryMaskTestString(int theUsedVariationFactor)
{
  NFmiString returnStr;
  NFmiAreaMask* mask = AreaMask();
  NFmiBitmapAreaMask* helperMask = HelperBinaryMask(theUsedVariationFactor);
  if (mask && helperMask)
  {
    unsigned int oldLocationIndex = LocationIndex();
    bool oldUseAreaMaskStatus = UseAreaMask();
    UseAreaMask(false);  // laitetaan maski pois päältä, niin liikutaan kaikissa paikoissa

    int locIndex = 0;
    int i = 0;
    string rowStr;
    checkedVector<string> lines;
    int x = 0, y = 0;
    for (ResetLocation(); NextLocation();)
    {
      locIndex = LocationIndex();
      bool status1 = mask->IsMasked(locIndex);
      bool status2 = helperMask->IsMasked(locIndex);
      if (status1 && status2)
        rowStr += "@";
      else if (status1)
        rowStr += "a";
      else if (status2)
        rowStr += "C";
      else
      {
        if (LocationIndex2XYPosition(locIndex, &x, &y))
        {
          if ((x == itsXYMaskBoundingBox.Left() || x == itsXYMaskBoundingBox.Right()) &&
              (y <= itsXYMaskBoundingBox.Bottom() && y >= itsXYMaskBoundingBox.Top()))
            rowStr += "|";
          else if ((x >= itsXYMaskBoundingBox.Left() && x <= itsXYMaskBoundingBox.Right()) &&
                   (y == itsXYMaskBoundingBox.Bottom() || y == itsXYMaskBoundingBox.Top()))
            rowStr += "-";
          else
            rowStr += ",";
        }
        else
          rowStr += ",";
      }
      i++;
      if (i % itsGridXNumber == 0)
      {
        rowStr += "\n";
        lines.push_back(rowStr);
        rowStr = "";
      }
    }
    rowStr += "\n";  // lisätään vielä viimeinen rivi vectoriin
    lines.push_back(rowStr);

    LocationIndex(oldLocationIndex);
    UseAreaMask(oldUseAreaMaskStatus);
    for (i = lines.size() - 1; i >= 0;
         i--)  // laitetaan rivit käänteisessä järjestyksessä, että 'kuva' tulee oikein päin
      returnStr += lines[i];
  }
  else
    returnStr += "Ei ollut maskia tai harvennusmaskia käytettävissä";

  return returnStr;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::UpdateHelperBinaryMasks()
{
  InitHelperBinaryMasks();
  SetCurrentHelperBinaryMask();
}

// ----------------------------------------------------------------------
/*!
 * Kutsu tätä aina kun aika mahdollisesti muuttuu SSInfossa!!!!
 * Pitää ottaa vanha timeindex talteen ennen muutosta ja antaa parametrina.
 *
 * \param theOldTimeIndex Undocumented
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::TimeChanged(unsigned long theOldTimeIndex)
{
  if (theOldTimeIndex != itsTimeIndex)
    UpdateAreaMaskTime();  // hoitaa myös SetCurrentHelperBinaryMask-kutsun
  if (itsVariationCalculator)
  {
    int uncertainty = CalcAreaUnCertainty();
    itsVariationCalculator->DataIterator()->SetDimensions(
        uncertainty, uncertainty, 0);  // dt toistaiseksi 0
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiSuperSmartInfo::InitEmptyAreaMask()
{
  NFmiDataIdent dummyParam;
  NFmiLevel dummyLevel;
  delete itsAreaMask;
  itsAreaMask = new NFmiBitmapAreaMask(*Grid(), &dummyParam, &dummyLevel);
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \param newValue Undocumented
 * \param fClearFirst Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiSuperSmartInfo::SetNearestPointMask(const NFmiPoint& theLatLonPoint,
                                             bool newValue,
                                             bool fClearFirst)
{
  if (itsAreaMask)
  {
    unsigned long oldIndex = LocationIndex();
    bool status = Location(theLatLonPoint);
    if (status)
    {
      if (fClearFirst) itsAreaMask->SetAll(false);
      itsAreaMask->Mask(LocationIndex(), newValue);
    }
    LocationIndex(oldIndex);
    return status;
  }
  return false;
}

// ======================================================================
