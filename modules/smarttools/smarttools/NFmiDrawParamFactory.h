//**********************************************************
// C++ Class Name : NFmiDrawParamFactory
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiDrawParamFactory.h
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 2
//  - GD View Type      : Class Diagram
//  - GD View Name      : Markon ehdotus
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Thur - Jan 28, 1999
//
//
//  Description:
//
//  Change Log:
// 1999.08.26/Marko	Laitoin DrawParam:in (oikeastaan DataParam tarvitsee)
//					luontiin mukaan tiedon levelistä.
//
//**********************************************************
#pragma once

#include <boost/shared_ptr.hpp>

class NFmiDrawParam;
class NFmiLevel;
class NFmiDataIdent;

class NFmiDrawParamFactory
{
 public:
  NFmiDrawParamFactory(bool createDrawParamFileIfNotExist, bool onePressureLevelDrawParam);
  ~NFmiDrawParamFactory(void);
  boost::shared_ptr<NFmiDrawParam> CreateDrawParam(const NFmiDataIdent& theIdent,
                                                   const NFmiLevel* theLevel);
  boost::shared_ptr<NFmiDrawParam> CreateCrossSectionDrawParam(const NFmiDataIdent& theIdent);
  boost::shared_ptr<NFmiDrawParam> CreateEmptyInfoDrawParam(const NFmiDataIdent& theIdent);
  bool Init();
  const std::string& LoadDirectory(void) const { return itsLoadDirectory; };
  void LoadDirectory(const std::string& newValue) { itsLoadDirectory = newValue; };
 private:
  boost::shared_ptr<NFmiDrawParam> CreateDrawParam(boost::shared_ptr<NFmiDrawParam>& theDrawParam,
                                                   bool fDoCrossSection);
  std::string CreateFileName(boost::shared_ptr<NFmiDrawParam>& drawParam,
                             bool fCrossSectionCase = false);
  std::string itsLoadDirectory;
  bool fCreateDrawParamFileIfNotExist;  // esim. metkun editori luo drawparam-tiedostot,
                                        // mutta SmarToolFiltterin ei tarvitse.
  bool fOnePressureLevelDrawParam;
};

