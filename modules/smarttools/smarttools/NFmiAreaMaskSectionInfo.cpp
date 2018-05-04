//**********************************************************
// C++ Class Name : NFmiAreaMaskSectionInfo
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: G:/siirto/marko/oc/NFmiAreaMaskSectionInfo.cpp
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 4.0 )
//  - GD System Name    : aSmartTools
//  - GD View Type      : Class Diagram
//  - GD View Name      : smarttools 1
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Thur - Jun 20, 2002
//
//  Change Log     :
//
//**********************************************************
#ifdef _MSC_VER
#pragma warning(disable : 4786)  // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiAreaMaskSectionInfo.h"
#include "NFmiAreaMaskInfo.h"
//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiAreaMaskSectionInfo::NFmiAreaMaskSectionInfo()
{
}
NFmiAreaMaskSectionInfo::~NFmiAreaMaskSectionInfo()
{
}

boost::shared_ptr<NFmiAreaMaskInfo> NFmiAreaMaskSectionInfo::MaskInfo(int theIndex)
{
  if (theIndex >= 0 && static_cast<unsigned int>(theIndex) < itsAreaMaskInfoVector.size())
    return itsAreaMaskInfoVector[theIndex];
  return boost::shared_ptr<NFmiAreaMaskInfo>();
}
void NFmiAreaMaskSectionInfo::Add(boost::shared_ptr<NFmiAreaMaskInfo> &theMask)
{
  itsAreaMaskInfoVector.push_back(theMask);
}
