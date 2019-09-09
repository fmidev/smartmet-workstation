//© Ilmatieteenlaitos/Marko.
// Original 5.9.2019
// 
// Luokka hanskaa color contourien yhteyteen piirrett‰vien v‰rilegendojen 
// todellisia arvoja. Ominaisuudet saadaan annetusta DrawParam objektista.
//---------------------------------------------------------- NFmiColorContourLegendValues.h

#pragma once

#include "NFmiColor.h"
#include "boost/shared_ptr.hpp"
#include <string>
#include <vector>

class NFmiDrawParam;
class NFmiFastQueryInfo;

class NFmiColorContourLegendValues
{
    // Luokka rajoja on N kappaletta, esim. 1, 2, 5, 10, 20, 50, 100 (eli t‰ss‰ 7 kpl)
    std::vector<float> classLimitValues_;
    // classLimitValues_ -arvojen vastaavat teksti muodot, joissa on desimaalien m‰‰r‰t laskettu jotenkin fiksusti
    std::vector<std::string> classLimitTexts_;
    // Jos classLimitValues_ -arvoja on 7 kpl, pit‰‰ v‰ri luokkia olla 7 + 1 eli 8 kpl, koska 
    // joku v‰ri vastaa kaikkia jotka menee yli isoimman arvon ja joku toinen v‰ri vastaa kaikkia jotka ovat alle 
    // pienimm‰n luokka-arvon. Jos v‰ri indeksi on ns. hollow-color, k‰sitell‰‰n sit‰ n‰kym‰ttˆm‰n‰ v‰rin‰.
    std::vector<NFmiColor> classColors_;
    // T‰h‰n laitetaan legenda-laatikkoa varten nimi, jonka ainakin alku piirret‰‰n laatikon yl‰osaan.
    std::string name_;
    bool useLegend_ = false;
    bool isStationData_ = false;
public:
    NFmiColorContourLegendValues() = default;
    NFmiColorContourLegendValues(const boost::shared_ptr<NFmiDrawParam>& drawParam, boost::shared_ptr<NFmiFastQueryInfo>& info);
    void init(const boost::shared_ptr<NFmiDrawParam> &drawParam, boost::shared_ptr<NFmiFastQueryInfo> &info);
    bool useLegend() const { return useLegend_; }


    const std::vector<float>& classLimitValues() const { return classLimitValues_; }
    const std::vector<std::string>& classLimitTexts() const { return  classLimitTexts_; }
    const std::vector<NFmiColor>& classColors() const { return  classColors_; }
    const std::string& name() const { return  name_; }
private:
    void clear();
    bool isStationDataType(const boost::shared_ptr<NFmiDrawParam>& drawParam, boost::shared_ptr<NFmiFastQueryInfo>& info);
    void FillSimpleColorContourValues(const boost::shared_ptr<NFmiDrawParam>& drawParam);
    void FillCustomColorContourValues(const boost::shared_ptr<NFmiDrawParam>& drawParam);
    void FillClassLimitTextsVector();
    void FinalizeFillingValues();
};


