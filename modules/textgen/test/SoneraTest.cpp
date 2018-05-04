#include <regression/tframe.h>
#include "Dictionary.h"
#include "DictionaryFactory.h"
#include "Phrase.h"
#include <calculator/Settings.h>

#include <boost/lexical_cast.hpp>

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace boost;

namespace SoneraTest
{
shared_ptr<TextGen::Dictionary> dict;

void require(const string& theKey, const string& theValue)
{
  using namespace TextGen;
  Phrase p(theKey);
  const string result = p.realize(*dict);
  if (result != theValue)
  {
    const string msg = ("'" + theKey + "'" + " = '" + p.realize(*dict) + "' is wrong, should be '" +
                        theValue + "'");
    TEST_FAILED(msg.c_str());
  }
}

void require(const string& theKey) { require(theKey, theKey); }
void phrases()
{
  require("o'clock", "");
  require("pohjanmaa:lle", "402");
  require("ahvenanmaa:lle", "308");

  require("1-aamusta", "200");
  require("1-aamuun", "201");
  require("1-alkaen", "202");
  require("1-illasta", "203");
  require("1-iltaan", "204");
  require("1-na", "205");
  require("1-puoleiseksi tuuleksi", "206");
  require("1-puoleista tuulta", "207");
  require("1-suunnasta", "208");
  require("1-suuntaan", "209");
  require("1-tuuleksi", "210");
  require("1-tuulta", "211");
  require("1-vastaisena yona", "212");
  require("2-aamusta", "213");
  require("2-aamuun", "214");
  require("2-alkaen", "215");
  require("2-illasta", "216");
  require("2-iltaan", "217");
  require("2-na", "218");
  require("2-puoleiseksi tuuleksi", "219");
  require("2-puoleista tuulta", "220");
  require("2-suunnasta", "221");
  require("2-suuntaan", "222");
  require("2-tuuleksi", "223");
  require("2-tuulta", "224");
  require("2-vastaisena yona", "225");
  require("3-aamusta", "226");
  require("3-aamuun", "227");
  require("3-alkaen", "228");
  require("3-illasta", "229");
  require("3-iltaan", "230");
  require("3-na", "231");
  require("3-puoleiseksi tuuleksi", "232");
  require("3-puoleista tuulta", "233");
  require("3-suunnasta", "234");
  require("3-suuntaan", "235");
  require("3-tuuleksi", "236");
  require("3-tuulta", "237");
  require("3-vastaisena yona", "238");
  require("4-aamusta", "239");
  require("4-aamuun", "240");
  require("4-alkaen", "241");
  require("4-illasta", "242");
  require("4-iltaan", "243");
  require("4-na", "244");
  require("4-puoleiseksi tuuleksi", "245");
  require("4-puoleista tuulta", "246");
  require("4-suunnasta", "247");
  require("4-suuntaan", "248");
  require("4-tuuleksi", "249");
  require("4-tuulta", "250");
  require("4-vastaisena yona", "251");
  require("5-aamusta", "252");
  require("5-aamuun", "253");
  require("5-alkaen", "254");
  require("5-illasta", "255");
  require("5-iltaan", "256");
  require("5-na", "257");
  require("5-puoleiseksi tuuleksi", "258");
  require("5-puoleista tuulta", "259");
  require("5-suunnasta", "260");
  require("5-suuntaan", "261");
  require("5-tuuleksi", "262");
  require("5-tuulta", "263");
  require("5-vastaisena yona", "264");
  require("5-vuorokauden saa", "265");
  require("6-aamusta", "266");
  require("6-aamuun", "267");
  require("6-alkaen", "268");
  require("6-illasta", "269");
  require("6-iltaan", "270");
  require("6-na", "271");
  require("6-puoleiseksi tuuleksi", "272");
  require("6-puoleista tuulta", "273");
  require("6-suunnasta", "274");
  require("6-suuntaan", "275");
  require("6-tuuleksi", "276");
  require("6-tuulta", "277");
  require("6-vastaisena yona", "278");
  require("7-aamusta", "279");
  require("7-aamuun", "280");
  require("7-alkaen", "281");
  require("7-illasta", "282");
  require("7-iltaan", "283");
  require("7-na", "284");
  require("7-puoleiseksi tuuleksi", "285");
  require("7-puoleista tuulta", "286");
  require("7-suunnasta", "287");
  require("7-suuntaan", "288");
  require("7-tuuleksi", "289");
  require("7-tuulta", "290");
  require("7-vastaisena yona", "291");
  require("8-puoleiseksi tuuleksi", "292");
  require("8-puoleista tuulta", "293");
  require("8-suunnasta", "294");
  require("8-suuntaan", "295");
  require("8-tuuleksi", "296");
  require("8-tuulta", "297");
  require("aamulla", "298");
  require("aamupaivalla", "299");
  require("aamupaivasta alkaen", "300");
  require("aamusta alkaen", "301");
  require("aamuyolla", "302");
  require("aamuyosta alkaen", "303");
  require("ajoittain lumisadetta", "304");
  require("ajoittain sadetta", "305");
  require("ajoittain sateista", "306");
  require("ajoittain ukkosta", "307");
  require("aland:lle", "308");
  require("alempi", "309");
  require("alin suhteellinen kosteus", "310");
  require("alkavan", "311");
  require("alueen yli", "312");
  require("aluksi", "313");
  require("ankaran hallan todennakoisyys", "314");
  require("astetta", "315");
  require("asti", "316");
  require("edelleen", "317");
  require("enimmakseen", "318");
  require("ensi yona", "319");
  require("etela-karjala:lle", "320");
  require("etela-pohjanmaa:lle", "321");
  require("etela-savo:lle", "322");
  require("hallan todennakoisyys", "323");
  require("hame:lle", "324");
  require("heikkenevaa", "325");
  require("heikkoa sadetta", "326");
  require("heikkoa", "327");
  require("heikoksi", "328");
  require("hieman alempi", "329");
  require("hieman korkeampi", "330");
  require("hieman pienempi", "331");
  require("hieman suurempi", "332");
  require("hirmumyrskyksi", "333");
  require("hirmumyrskya", "334");
  require("huomattavasti alempi", "335");
  require("huomattavasti korkeampi", "336");
  require("huomattavasti lampimampi", "337");
  require("huomattavasti pienempi", "338");
  require("huomattavasti suurempi", "339");
  require("huomenna", "340");
  require("huomisaamuna", "341");
  require("illalla", "342");
  require("illasta alkaen", "343");
  require("iltapaivalla", "344");
  require("iltapaivasta alkaen", "345");
  require("iltayolla", "346");
  require("iltayosta alkaen", "347");
  require("kainuu:lle", "348");
  require("kello", "349");
  require("keski-pohjanmaa:lle", "350");
  require("keski-suomi:lle", "351");
  require("keskilampotila", "352");
  require("keskimaarainen alin lampotila", "353");
  require("keskimaarainen ylin lampotila", "354");
  require("keskipaivalla", "355");
  require("keskipaivasta alkaen", "356");
  require("keskiyolla", "357");
  require("keskiyosta alkaen", "358");
  require("kohtalaiseksi", "359");
  require("kohtalaista", "360");
  require("koillismaa:lle", "361");
  require("korkeampi", "362");
  require("korkeapaine", "363");
  require("korkeapaineen alue", "364");
  require("kosteammaksi", "365");
  require("kovaa", "366");
  require("kovaksi", "367");
  require("kuivemmaksi", "368");
  require("kymenlaakso:lle", "369");
  require("laaja", "370");
  require("lansi-lappi:lle", "371");
  require("leviaa", "372");
  require("liikkuu", "373");
  require("loittonee", "374");
  require("lopuksi", "375");
  require("lumi-", "376");
  require("lumikuuroja", "377");
  require("lumisadetta", "378");
  require("lahestyy", "379");
  require("lampenee", "380");
  require("lampimampi", "381");
  require("lampotila", "382");
  require("m/s", "383");
  require("mahdollisesti ukkosta", "384");
  require("matalapaine", "385");
  require("matalapaineen alue", "386");
  require("melko selkeaa", "387");
  require("metria sekunnissa", "388");
  require("miinus", "389");
  require("millimetria", "991,390");
  require("monin paikoin enemman", "391");
  require("monin paikoin", "392");
  require("muuttuu", "393");
  require("myrskyksi", "394");
  require("myrskya", "395");
  require("navakaksi", "396");
  require("navakkaa", "397");
  require("noin", "398,991");
  require("nyland:lle", "399");
  require("odotettavissa", "400");
  require("on", "401,992");
  require("osterbotten:lle", "402");
  require("paijat-hame:lle", "403");
  require("paikoin enemman", "404");
  require("paikoin", "405");
  require("pienempi", "406");
  require("pilvistyvaa", "407");
  require("pilvista", "408");
  require("pirkanmaa:lle", "409");
  require("pohjois-karjala:lle", "410");
  require("pohjois-lappi:lle", "411");
  require("pohjois-pohjanmaa:lle", "412");
  require("pohjois-savo:lle", "413");
  require("poutaa", "414");
  require("puolipilvista", "415");
  require("paivien ylin lampotila", "416");
  require("paivalla", "417");
  require("paivalampotila", "418");
  require("paivan ylin lampotila", "419");
  require("rannikolla", "420");
  require("runsasta sadetta", "421");
  require("ranta-", "422");
  require("rantasadetta", "423");
  require("sadealue", "424");
  require("sadesumma", "425");
  require("sadetta", "426");
  require("sakeaa lumisadetta", "427");
  require("sakeita lumikuuroja", "428");
  require("sama", "429");
  require("satakunta:lle", "430");
  require("selkenevaa", "431");
  require("selkeaa", "432");
  require("seuraava yo", "433");
  require("seuraavana paivana", "434");
  require("seuraavana yona", "435");
  require("suunnaltaan vaihtelevaa", "436");
  require("suunnilleen sama", "437");
  require("suurempi", "438");
  require("syvenee", "439");
  require("saa on epavakaista", "440");
  require("saa", "441");
  require("saaennuste", "442");
  require("tai", "443");
  require("tuuli heikkenee", "444");
  require("tuuli tyyntyy", "445");
  require("tuuli voimistuu", "446");
  require("tuuli", "447");
  require("tuulta", "448");
  require("tyyneksi", "449");
  require("tyynta", "450");
  require("tanaan", "451");
  require("uusimaa:lle", "452");
  require("vahvistuu", "453");
  require("vaihtelevaa pilvisyytta", "454");
  require("varsinais-suomi:lle", "455");
  require("vesi-", "456");
  require("vesisadetta", "457");
  require("viilenee", "458");
  require("viiva", "459,991");
  require("voimakkaita kuuroja", "460");
  require("voimistuvaa", "461");
  require("vahaisia kuuroja", "462");
  require("vahaista lumisadetta", "463");
  require("yli", "464");
  require("ylihuomenna", "465");
  require("yolla", "466");
  require("yolampotila", "467");
  require("yon alin lampotila", "468");
  require("yosta alkaen", "469");
  require("oiden alin lampotila", "470");
  require("prosenttia", "471");
  require("ajoittain", "472");
  require("1-na kello", "473,991");
  require("2-na kello", "474,991");
  require("3-na kello", "475,991");
  require("4-na kello", "476,991");
  require("5-na kello", "477,991");
  require("6-na kello", "478,991");
  require("7-na kello", "479,991");
  require("ei suuria muutoksia saassa", "480");
  require("saa selkenee", "481");
  require("saa muuttuu epavakaisemmaksi", "482");
  require("sateinen saa jatkuu", "483");
  require("saa muuttuu sateisemmaksi", "484");
  require("epavakainen saa jatkuu", "485");
  require("saa lauhtuu", "486");
  require("saa on enimmakseen poutaista", "487");
  require("saa poutaantuu", "488");
  require("saa muuttuu helteiseksi", "489");
  require("saa lampenee", "490");
  require("saa viilenee", "491");
  require("saa muuttuu koleaksi", "492");
  require("saa kylmenee", "493");
  require("helteinen saa jatkuu", "494");
  require("lammin saa jatkuu", "495");
  require("viilea saa jatkuu", "496");
  require("kolea saa jatkuu", "497");
  require("kylma saa jatkuu", "498");
  require("virtaa", "499");
  require("tilapaisesti", "500");
  require("lamminta", "501");
  require("kylmaa", "502");
  require("viileaa", "503");
  require("viileampaa", "504");
  require("lampimampaa", "505");
  require("kylmempaa", "506");
  require("hyvin", "507");
  require("vahan", "508");
  require("ilmaa", "509");
  require("Suomeen", "510");
  require("maan", "511");
  require("etelaosiin", "512");
  require("keskiosiin", "513");
  require("pohjoisosiin", "514");
  require("itaosiin", "515");
  require("lansiosiin", "516");
  require("etela-", "517");
  require("keski-", "518");
  require("pohjois-", "519");
  require("ita-", "520");
  require("lansi-", "521");
  require("kuivempaa", "522");
  require("kosteampaa", "523");
  require("sadealueita", "524");
  require("Suomen yli", "525");
  require("Etela-Suomen yli", "526");
  require("Keski-Suomen yli", "527");
  require("Pohjois-Suomen yli", "528");
  require("Ita-Suomen yli", "529");
  require("Lansi-Suomen yli", "530");
  require("Lounais-Suomen yli", "531");
  require("Kaakkois-Suomen yli", "532");
  require("uusi", "533");
  require("saapuu", "534");
  require("alue", "535");
  require("sateineen", "536");
  require("sadealueineen", "537");
  require("hajanaisine sadealueineen", "538");
  require("sateen todennakoisyys", "539");
  require("kuurosateen todennakoisyys", "540");
  require("ukkosen todennakoisyys", "541");
  require("kuuluu", "542");
  require("pakkanen jatkuu", "543");
  require("pakkanen kiristyy", "544");
  require("pakkanen heikkenee", "545");
  require("ja", "546");

  // New additions (14) with same pronounciation:
  require("astetta (2)", "315");
  require("astetta (0)", "315");
  require("astetta (n...4)", "315");
  require("astetta (n...3)", "315");
  require("astetta (n...2)", "315");
  require("astetta (n...1)", "315");
  require("astetta (n...0)", "315");
  require("astetta (m...n)", "315");
  require("astetta (n)", "315");
  require("astetta (4)", "315");
  require("astetta (3)", "315");
  require("astetta (mod 10=1)", "315");
  require("astetta (n...(mod 10=1))", "315");
  require("astetta (1)", "315");

  // extras:
  // pohjanmaa:lle == osterbotten::lle
  // ahvenanmaa:lle == aland::lle
  // o'clock = <empty>

  const int extras = 3 + 14;
  if (dict->size() != 546 - 200 + 1 + extras)
    TEST_FAILED("Sonera dictionary should have 350 phrases, not " +
                boost::lexical_cast<std::string>(dict->size()));

  TEST_PASSED();
}

//! The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void) { TEST(phrases); }
};  // class tests

}  // namespace SoneraTest

int main(void)
{
  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  cout << endl << "SoneraPhrases requirements" << endl << "==========================" << endl;

  SoneraTest::dict.reset(TextGen::DictionaryFactory::create("mysql"));
  SoneraTest::dict->init("sonera");

  SoneraTest::tests t;
  return t.run();
}
