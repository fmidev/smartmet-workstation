// ======================================================================
/*!
 *
 * C++ Class Name : NFmiDrawParamFactory
 * ---------------------------------------------------------
 * Filetype: (SOURCE)
 * Filepath: D:/projekti/GDPro/GDTemp/NFmiDrawParamFactory.cpp
 *
 *
 * GDPro Properties
 * ---------------------------------------------------
 *  - GD Symbol Type    : CLD_Class
 *  - GD Method         : UML ( 2.1.4 )
 *  - GD System Name    : Met-editor Plan 2
 *  - GD View Type      : Class Diagram
 *  - GD View Name      : Markon ehdotus
 * ---------------------------------------------------
 *  Author         : pietarin
 *  Creation Date  : Thur - Jan 28, 1999
 *
 *
 *  Description:
 *
 *  Change Log:
 * 1999.08.26/Marko	Laitoin DrawParam:in (oikeastaan DataParam tarvitsee)
 *					luontiin mukaan tiedon levelistä.
 *
 */
// ======================================================================

#include "NFmiDrawParamFactory.h"
#include "NFmiDrawParam.h"

#include <newbase/NFmiValueString.h>
#include <newbase/NFmiFileSystem.h>

#include <cassert>

static void InitDrawParam(boost::shared_ptr<NFmiDrawParam>& theDrawParam,
                          const std::string& theFileName,
                          bool createDrawParamFileIfNotExist)
{
  if (theDrawParam)
  {
    if (!theDrawParam->Init(theFileName))
    {
      // laitetaan tiedoston nimi kuitenkin talteen drawparamiin ja yritetään luoda tiedosto jos
      // tarpeen
      theDrawParam->InitFileName(theFileName);
      if (createDrawParamFileIfNotExist)
      {
        if (NFmiFileSystem::FileExists(theFileName) == false)
        {  // jos tosiaan tiedostoa ei ollut, luodaan sellainen.
           // Mutta jos luku ei onnistunut, ei luoda sitä, koska kyseessä saattoi olla jokin
           // hetkellinen luku häiriö
           // ja omituisesti tiuhtissa olevat perustiedostot ovat aina silloin tällöin
           // 'resetoituneet'.
          if (!theDrawParam->StoreData(theFileName))
          {
            // tiedostoa ei voitu luoda, mitä pitäisi tehdä?
          }
        }
      }
    }
  }
}

//--------------------------------------------------------
// NFmiDrawParamFactory(void)
//--------------------------------------------------------
NFmiDrawParamFactory::NFmiDrawParamFactory(bool createDrawParamFileIfNotExist,
                                           bool onePressureLevelDrawParam)
    : itsLoadDirectory(""),
      fCreateDrawParamFileIfNotExist(createDrawParamFileIfNotExist),
      fOnePressureLevelDrawParam(onePressureLevelDrawParam)
{
}
//--------------------------------------------------------
// ~FmiDrawParamFactory(void)
//--------------------------------------------------------
NFmiDrawParamFactory::~NFmiDrawParamFactory(void)
{
}

//--------------------------------------------------------
// DrawParam
//--------------------------------------------------------
boost::shared_ptr<NFmiDrawParam> NFmiDrawParamFactory::CreateDrawParam(
    const NFmiDataIdent& theIdent, const NFmiLevel* theLevel)

//  Tässä metodissa valitaan sääparametrin theParam perusteella piirtoa
//  varten sopiva drawParam. Kostruktorin NFmiDrawParam vaatima pointteri
//  NFmiDataParam* dataParam saadaan attribuutilta itsDataParamFactory.
//  Huomaa, että palautettava drawParam = new NFmiDrawParam, joten drawParam
//  pitää muistaa tuhota ulkopuolella.

{
  //	NFmiDrawParam* drawParam = new NFmiDrawParam(theInfo, theIdent, 1); // 1 = priority
  // 7.1.2002/Marko Muutin dataidentin alustuksen niin, että se otetaan annetusta
  // infosta, jolloin se on aina oikein. Info on aina asetettu halutun parametrin
  // kohdalle, kun se tulee tänne.
  boost::shared_ptr<NFmiDrawParam> drawParam = boost::shared_ptr<NFmiDrawParam>(
      new NFmiDrawParam(theIdent, theLevel ? *theLevel : NFmiLevel(), 1));  // 1 = priority
  return CreateDrawParam(drawParam, false);
}

// luodaan drawparam crossSectionDataa varten. Huom käytetyt tiedostonimet
// poikkeavat muista drawparamien tiedostonimistä.
// Eli esim. DrawParam_4_CrossSection.dpa
boost::shared_ptr<NFmiDrawParam> NFmiDrawParamFactory::CreateCrossSectionDrawParam(
    const NFmiDataIdent& theIdent)
{
  boost::shared_ptr<NFmiDrawParam> drawParam = boost::shared_ptr<NFmiDrawParam>(
      new NFmiDrawParam(theIdent, NFmiLevel(), 1));  // 1 = priority
  return CreateDrawParam(drawParam, true);
}

boost::shared_ptr<NFmiDrawParam> NFmiDrawParamFactory::CreateEmptyInfoDrawParam(
    const NFmiDataIdent& theIdent)
{
  boost::shared_ptr<NFmiDrawParam> drawParam(new NFmiDrawParam());
  if (drawParam)
    drawParam->Param(theIdent);
  return CreateDrawParam(drawParam, false);
}

// Halusin laittaa yhteen metodiin kaiken drawParam-alustuksen ja virhetilanteiden käsittelyn, koska
// muuten olisi tullut melkein kolme identtistä metodi runkoa. Tämä on tosin ruma funktio, koska
// tämä mahdollisesti tuhoaa annetun DrawParamin tai sitten palautta sen jatkokäsittelyjä varten.
// Joten
// HUOM!!!! Eli parametrina annettua theDrawParam:ia pitää tarkastaa tämän kutsun jälkeen, koska se
// on
// saatettu tuhota!!!!!
// Rumaa koodia, oikea tapa olisi ehkä tehdä nuo kolme metodia (CreateDrawParam,
// CreateCrossSectionDrawParam,
// CreateEmptyInfoDrawParam) yhdeksi metodiksi, joka hoitaa kaiken kerralla.

// HUOM!!!! share_ptr käyttö muutti tätä funktiota ja erillisiä tuhoamisia ei tarvitse tehdä.
boost::shared_ptr<NFmiDrawParam> NFmiDrawParamFactory::CreateDrawParam(
    boost::shared_ptr<NFmiDrawParam>& theDrawParam, bool fDoCrossSection)
{
  try
  {
    if (theDrawParam)
    {
      std::string fileName = CreateFileName(theDrawParam, fDoCrossSection);
      InitDrawParam(theDrawParam, fileName, fCreateDrawParamFileIfNotExist);
    }
  }
  catch (...)
  {
    theDrawParam = boost::shared_ptr<NFmiDrawParam>();
  }
  return theDrawParam;
}

//--------------------------------------------------------
// Init
//--------------------------------------------------------
bool NFmiDrawParamFactory::Init()
{
  return true;
}

//--------------------------------------------------------
// CreateFileName, private
//--------------------------------------------------------
std::string NFmiDrawParamFactory::CreateFileName(boost::shared_ptr<NFmiDrawParam>& drawParam,
                                                 bool fCrossSectionCase)
{
  std::string fileName(itsLoadDirectory);
  if (!itsLoadDirectory.empty())
    fileName += kFmiDirectorySeparator;  // laitetaan varmuuden vuoksi keno, jos asetetun polun
                                         // lopussa ei ole kenoa
  fileName += "DrawParam_";

  if (drawParam)
  {
    unsigned long paramId = drawParam->Param().GetParam()->GetIdent();

    fileName += NFmiStringTools::Convert(paramId);
    if (fCrossSectionCase)
      fileName += "_CrossSection";
    else
    {
      if (drawParam && drawParam->Level().LevelType() == kFmiPressureLevel)
      {  // jos leveleitä on useita, niillä on omat tiedostot
        if (fOnePressureLevelDrawParam)
        {
          fileName += "_pressure";
        }
        else
        {
          fileName += "_level_";
          NFmiLevel& level = drawParam->Level();
          unsigned long levelTypeId = level.LevelTypeId();
          fileName += NFmiStringTools::Convert(levelTypeId);
          fileName += "_";
          fileName += NFmiStringTools::Convert(level.LevelValue());
        }
      }
      else if (drawParam && drawParam->Level().LevelType() == kFmiHybridLevel)
      {  // hybrideillä on ain yksi piirtotapa
        fileName += "_hybrid";
      }
      else if (drawParam &&
               drawParam->Level().LevelType() == kFmiSoundingLevel)  // tämä on luotaus dataa
      {  // luotaus parametreilla on ain yksi piirtotapa kartalla
        fileName += "_temp";
      }
      else
      {
        // normaali parametrille ei taida olla tiedosto nimessä mitään ekstraa
      }
    }
    fileName += ".dpa";
  }
  return fileName;
}
