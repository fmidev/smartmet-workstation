#pragma once
//**********************************************************
// C++ Class Name : NFmiSmartToolCalculationInfo
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Thur - Jun 20, 2002
//
//  Change Log     :
//
// Luokka sisältää calculationSectionista yhden laskurivin esim.
//
// T = T + 1
//
// Lasku rivi sisältää aina TulosParametrin ja sijoituksen. Lisäksi se sisältää joukon
// operandeja (n kpl) ja operaatioita (n-1 kpl).
//**********************************************************

#include <boost/shared_ptr.hpp>
#include <vector>

class NFmiAreaMaskInfo;

class NFmiSmartToolCalculationInfo
{
 public:
  NFmiSmartToolCalculationInfo();
  ~NFmiSmartToolCalculationInfo();

  void SetResultDataInfo(boost::shared_ptr<NFmiAreaMaskInfo>& value) { itsResultDataInfo = value; }
  boost::shared_ptr<NFmiAreaMaskInfo> GetResultDataInfo() { return itsResultDataInfo; }
  void AddCalculationInfo(boost::shared_ptr<NFmiAreaMaskInfo>& theAreaMaskInfo);
  std::vector<boost::shared_ptr<NFmiAreaMaskInfo> >& GetCalculationOperandInfoVector()
  {
    return itsCalculationOperandInfoVector;
  }
  const std::string& GetCalculationText() { return itsCalculationText; }
  void SetCalculationText(const std::string& theText) { itsCalculationText = theText; }
  void CheckIfAllowMissingValueAssignment();
  bool AllowMissingValueAssignment() { return fAllowMissingValueAssignment; };

 private:
  // HUOM!! Tämä erillinen ResultInfo-systeemi oli huono ratkaisu, laita ne mielluummin
  // osaksi laskentaketjua (itsCalculationOperandInfoVector:iin).
  // Silloin voi mm. ottaa tämän luokan käyttöön NFmiAreaMaskSectionInfo-luokan sijasta.
  boost::shared_ptr<NFmiAreaMaskInfo> itsResultDataInfo;  // omistaa+tuhoaa
  std::vector<boost::shared_ptr<NFmiAreaMaskInfo> >
      itsCalculationOperandInfoVector;  // omistaa+tuhoaa
  std::string itsCalculationText;       // originaali teksti, mistä tämä lasku on tulkittu
  bool fAllowMissingValueAssignment;
};
