#pragma once
//**********************************************************
// C++ Class Name : NFmiAreaMaskSectionInfo
// ---------------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Thur - Jun 20, 2002
//
// Sisältää listan areaMaskInfoja, esim. IF-lauseen maskihan voi olla vaikka:
// IF(T>2) tai IF(T>2 && P<1012) jne.
//**********************************************************

#include <boost/shared_ptr.hpp>
#include <vector>

class NFmiAreaMaskInfo;

class NFmiAreaMaskSectionInfo
{
 public:
  NFmiAreaMaskSectionInfo();
  ~NFmiAreaMaskSectionInfo();

  boost::shared_ptr<NFmiAreaMaskInfo> MaskInfo(int theIndex);
  void Add(boost::shared_ptr<NFmiAreaMaskInfo>& theMask);
  std::vector<boost::shared_ptr<NFmiAreaMaskInfo> >& GetAreaMaskInfoVector()
  {
    return itsAreaMaskInfoVector;
  }
  const std::string& GetCalculationText() { return itsCalculationText; }
  void SetCalculationText(const std::string& theText) { itsCalculationText = theText; }
 private:
  std::vector<boost::shared_ptr<NFmiAreaMaskInfo> > itsAreaMaskInfoVector;
  std::string itsCalculationText;  // originaali teksti, mistä tämä lasku on tulkittu
};
