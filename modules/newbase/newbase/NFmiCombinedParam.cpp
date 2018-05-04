// ======================================================================
/*!
 * \file NFmiCombinedParam.cpp
 * \brief Implementation of class NFmiCombinedParam
 */
// ======================================================================
/*!
 * \class NFmiCombinedParam
 *
 * Undocumented
 *
 * \todo Should define operator= explicitly since pointers are involved
 */
// ======================================================================

#include "NFmiCombinedParam.h"
#include "NFmiDataModifierCombi.h"
#include "NFmiParamBag.h"
#include "NFmiParamDataModifierList.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiCombinedParam::~NFmiCombinedParam()
{
  if (itsSubParams)
  {
    int size = itsSubParams->GetSize();
    for (int i = 0; i < size; i++)
      delete itsIntegrators[i];
  }
  delete[] itsIntegrators;
  delete itsSubParams;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam::NFmiCombinedParam(double theInfoVersion)
    : itsSubParams(nullptr),
      itsIntegrators(nullptr),
      fIntegrationMode(false),
      fIntegrationReady(false),
      fIntegrationStarted(),
      fDataOk(true),
      fAutoUpdate(true),
      itsInfoVersion(theInfoVersion)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theParam The other object being copied
 */
// ----------------------------------------------------------------------

NFmiCombinedParam::NFmiCombinedParam(const NFmiCombinedParam &theParam)
    : itsSubParams(theParam.itsSubParams ? new NFmiParamBag(*theParam.itsSubParams) : nullptr),
      itsIntegrators(nullptr)  // Tässä pitäisi käyttää ParamModifierListin copy construktoria, vaan
                               // ei ole vielä
      ,
      fIntegrationMode(theParam.fIntegrationMode),
      fIntegrationReady(theParam.fIntegrationReady),
      fIntegrationStarted(theParam.fIntegrationStarted),
      fDataOk(theParam.fDataOk),
      fAutoUpdate(theParam.fAutoUpdate),
      itsInfoVersion(theParam.InfoVersion())
{
  int size = itsSubParams ? itsSubParams->GetSize() : 0;
  if (size)
  {
    itsIntegrators = new NFmiDataModifierCombi *[size];
    for (int i = 0; i < size; i++)
      itsIntegrators[i] = new NFmiDataModifierCombi(*theParam.itsIntegrators[i]);
  }
  //	InitIntegration();//Tämä pitäisi tehdä kopioimalla
  //	CreateIntegrators();
}

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 *
 * \param theParam The other object being copied
 */
// ----------------------------------------------------------------------

NFmiCombinedParam &NFmiCombinedParam::operator=(const NFmiCombinedParam &theParam)
{
  if (this != &theParam)
  {
    itsSubParams = (theParam.itsSubParams ? new NFmiParamBag(*theParam.itsSubParams) : nullptr);
    itsIntegrators =
        nullptr;  // Tässä pitäisi käyttää ParamModifierListin operator=, vaan ei ole vielä
    fIntegrationMode = theParam.fIntegrationMode;
    fIntegrationReady = theParam.fIntegrationReady;
    fIntegrationStarted = theParam.fIntegrationStarted;
    fDataOk = theParam.fDataOk;
    fAutoUpdate = theParam.fAutoUpdate;
    itsInfoVersion = theParam.InfoVersion();

    int size = itsSubParams ? itsSubParams->GetSize() : 0;
    if (size)
    {
      itsIntegrators = new NFmiDataModifierCombi *[size];
      for (int i = 0; i < size; i++)
        itsIntegrators[i] = new NFmiDataModifierCombi(*theParam.itsIntegrators[i]);
    }
    //	InitIntegration();//Tämä pitäisi tehdä kopioimalla
    //	CreateIntegrators();
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCombinedParam::TransformFromFloatValue(float theValue)
{
  return LongValue(ConvertFloatToLong(theValue));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiCombinedParam::TransformedFloatValue()
{
  union converter {
    unsigned long ulongvalue;
    float floatvalue;
  };

  converter tmp;
  tmp.ulongvalue = LongValue();
  return tmp.floatvalue;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiCombinedParam::ConvertFloatToLong(float theValue)
{
  // MSVC60:ssa on bugi ja ylempi rivi (if(theValue == kFloatMissing)) ei toimi oikein
  // TotalWind-arvoille,
  // jotka tulkitaan jotenkin NaN:eiksi ja ylemmässä versiossa tulee true vertailussa. Alempi joka
  // on käytännössä
  // sama toimii oikein molemmissa kääntäjä versioissa (MSVC60 ja MSVC71).

  //  if(theValue == kFloatMissing)
  if (theValue == 32700.0F) return kTCombinedWeatherMissing;

  union converter {
    unsigned long ulongvalue;
    float floatvalue;
  };

  converter tmp;
  tmp.floatvalue = theValue;
  return tmp.ulongvalue;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::InitIntegration()
{
  CreateSubParams();
  CreateIntegrators();
  fIntegrationMode = true;
  fIntegrationStarted = false;
  fIntegrationReady = false;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::ClearIntegration()
{
  fIntegrationMode = true;
  fIntegrationStarted = false;
  fIntegrationReady = false;
  if (itsSubParams)
  {
    for (unsigned int idx = 0; idx < itsSubParams->GetSize(); idx++)
    {
      NFmiDataModifier *integrator = GetSubIntegrator(idx);
      if (integrator)
      {
        integrator->Clear();
      }
    }
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::EndIntegration()
{
  if (!fIntegrationMode) return;

  if (fIntegrationStarted && itsSubParams)
  {
    itsSubParams->Reset();
    int idx = 0;
    while (itsSubParams->Next())
    {
      // Mika 08.11.2001: No huh-huh?!
      // NFmiDataModifierCombi *integrator =
      // GetSubIntegrator(index);itsIntegrators[index];//FindSubParamIntegrator(itsSubParams->CurrentParam());
      NFmiDataModifierCombi *integrator = GetSubIntegrator(idx);
      if (integrator)
      {
        //				SubValue(integrator->CalculationResult(),
        // itsSubParams->CurrentParam());
        /////Persa poisti koska yrittää toisella tavalla tuloksia
        ///				SubValue(integrator->CalcResult(kFmiMean),
        /// itsSubParams->CurrentParam());
      }
      idx++;
    }
  }
  //	DeleteIntegrators();
  //	fIntegrationMode = false;
  fIntegrationStarted = false;
  fIntegrationReady = true;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::Integrate(float theValue)
{
  if (!fIntegrationMode) return;
  fIntegrationStarted = true;

  if (itsSubParams)
  {
    NFmiCombinedParam *addNewValue = CreateNew(theValue);
    itsSubParams->Reset();
    int idx = 0;
    while (itsSubParams->Next())
    {
      NFmiDataModifier *integrator = GetSubIntegrator(
          idx);  // itsIntegrators[index];//FindSubParamIntegrator(itsSubParams->CurrentParam());
      if (integrator)
      {
        integrator->Calculate(
            static_cast<float>(addNewValue->RawSubValue(itsSubParams->CurrentParam())));
      }
      idx++;
    }
    delete addNewValue;
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::CreateIntegrators() {}
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::DeleteIntegrators() {}
// ----------------------------------------------------------------------
/*!
 * \param theName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombi *NFmiCombinedParam::FindSubParamIntegrator(FmiParameterName theName)
{
  int idx = FindSubParamIntegratorIndex(theName);
  if (idx >= 0)
  {
    return itsIntegrators[idx];
  }
  else
    return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \param theName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

int NFmiCombinedParam::FindSubParamIntegratorIndex(FmiParameterName theName)
{
  if (itsIntegrators && itsSubParams)
  {
    int i = 0;
    for (itsSubParams->Reset(); itsSubParams->Next();)
    {
      if (itsSubParams->CurrentParam() == theName)
      {
        return i;
      }
      i++;
    }
  }
  return -1;
}

// ----------------------------------------------------------------------
/*!
 * \param theSubParamName Undocumented
 * \param theModifier Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCombinedParam::SetSubIntegrator(FmiParameterName theSubParamName,
                                         NFmiDataModifierCombi *theModifier)
{
  int idx = FindSubParamIntegratorIndex(theSubParamName);
  if (idx >= 0)
  {
    itsIntegrators[idx] = theModifier;
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theInitValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam *NFmiCombinedParam::CreateNew(float /* theInitValue */) { return nullptr; }
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::CreateSubParams() { itsSubParams = nullptr; }
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombi *NFmiCombinedParam::GetSubIntegrator(unsigned long theIndex)
{
  if (theIndex < itsSubParams->GetSize())
    return itsIntegrators[theIndex];
  else
    return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \param theSubParamName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierCombi *NFmiCombinedParam::SubIntegrator(FmiParameterName theSubParamName)
{
  int idx = FindSubParamIntegratorIndex(theSubParamName);
  if (idx >= 0)
    return GetSubIntegrator(idx);
  else
    return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 */
// ----------------------------------------------------------------------

void NFmiCombinedParam::CrossCheck(FmiParameterName /* theParam */) {}
// ======================================================================
