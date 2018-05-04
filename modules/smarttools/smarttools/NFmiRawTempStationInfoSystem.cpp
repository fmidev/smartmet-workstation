// NFmiRawTempStationInfoSystem.cpp
//

#include "NFmiRawTempStationInfoSystem.h"
#include <newbase/NFmiCommentStripper.h>
#include <newbase/NFmiStation.h>
#include <newbase/NFmiLocationBag.h>
#include <fstream>

using namespace std;

static void ThrowLatLonProblemException(const std::string &theFunctionNameStr,
                                        const std::string &theProblemStr,
                                        const std::string &theLatLonStr,
                                        const std::string &theLineStr,
                                        const std::string &theInitFileName)
{
  std::string errStr;
  errStr += theFunctionNameStr;
  errStr += " - ";
  errStr += theProblemStr;
  errStr += ": ";
  errStr += theLatLonStr;
  errStr += "\non line:\n";
  errStr += theLineStr;
  errStr += "\nin file: ";
  errStr += theInitFileName;
  throw std::runtime_error(errStr);
}

// purkaa rivin
// 2963 23.5 60.8167 JOKIOINEN OBSERVATORIO
// osiin, ja rakentaan NFmiStation-otuksen, niin että 3. spacen jälkeen kaikki on nimeä.
static bool GetStationFromString(const std::string &theStationStr,
                                 const std::string &theInitFileName,
                                 NFmiStation &theStationOut)
{
  std::vector<std::string> strVector = NFmiStringTools::Split(theStationStr, " ");
  if (strVector.size() > 1 && strVector.size() < 4)
  {
    std::string errStr("GetStationFromString - station info had too few parts on line\n");
    errStr += theStationStr;
    errStr += "\n in file: ";
    errStr += theInitFileName;
    throw std::runtime_error(errStr);
  }
  else if (strVector.size() <= 1)
    return false;  // en ole varma splitteristä, mutta ignoorataan vain tyhjä/yhden osion rivit

  theStationOut.SetIdent(NFmiStringTools::Convert<unsigned long>(strVector[0]));
  theStationOut.SetLongitude(NFmiStringTools::Convert<double>(strVector[1]));
  theStationOut.SetLatitude(NFmiStringTools::Convert<double>(strVector[2]));
  std::string name;
  for (unsigned int i = 3; i < strVector.size(); i++)
  {
    if (i != 3)
      name += " ";
    name += strVector[i];
  }
  theStationOut.SetName(NFmiString(name));
  return true;
}

// poistaa alku+loppu spacet ja tabulaattorit
// sekä sanojen välistä muut paitsi spacet ja tabit paitsi yhden spacen.
// Jos sanojen välillä on ainoastaan tabulaattoreita, korvaa ne yhdellä spacella.
// <cr> ja <lf> ei poisteta eikä huomioida (paitsi alusta ja lopusta)!!
static std::string RemoveExtraSpaces(const std::string &theStr)
{
  std::string tmp(theStr);

  // trimmataan stringin alusta ja lopusta whitespacet pois
  NFmiStringTools::Trim(tmp, ' ');
  NFmiStringTools::Trim(tmp, '\t');
  NFmiStringTools::Trim(tmp, '\t');
  NFmiStringTools::Trim(tmp, '\n');
  std::string result;
  bool wordBreak = false;
  for (unsigned int i = 0; i < tmp.size(); i++)
  {
    if (tmp[i] == ' ' || tmp[i] == '\t')
    {
      if (wordBreak == false)
      {
        wordBreak = true;
        result.push_back(' ');  // laitetaan aina sanan perään yksi space
      }
    }
    else
    {
      wordBreak = false;
      result.push_back(tmp[i]);
    }
  }

  return result;
}

// Lukee initialisointi tiedostosta asema tiedot, heittää poikkeuksen jos ongelmia.
// Asema tiedoston formaatti on seuraava (kommentit sallittuja, mutta asema rivit
// ilman kommentti merkkejä):
//
// // id  lon  lat      name (with spaces to the end of line)
// 2963 23.5 60.8167 JOKIOINEN OBSERVATORIO
// 3005 -1.18333 60.1333 LERWICK
void NFmiRawTempStationInfoSystem::Init(const std::string &theInitFileName)
{
  itsInitLogMessage = "";
  itsLocations = NFmiHPlaceDescriptor();  // tyhjennetään ensin asemalista
  NFmiLocationBag locBag;

  if (theInitFileName.empty())
    throw std::runtime_error("NFmiRawTempStationInfoSystem::Init - empty settings filename given.");

  NFmiCommentStripper stripComments;
  if (stripComments.ReadAndStripFile(theInitFileName))
  {
#ifdef OLDGCC
    std::istrstream in(stripComments.GetString().c_str());
#else
    std::stringstream in(stripComments.GetString());
#endif

    const int maxBufferSize = 1024 + 1;  // kuinka pitkä yhden rivin maksimissaan oletetaan olevan
    std::string buffer;
    NFmiStation station;
    int i = 0;
    int counter = 0;
    do
    {
      buffer.resize(maxBufferSize);
      in.getline(&buffer[0], maxBufferSize);

      int realSize = static_cast<int>(strlen(buffer.c_str()));
      buffer.resize(realSize);
      buffer = ::RemoveExtraSpaces(buffer);
      if (::GetStationFromString(buffer, theInitFileName, station))
      {
        counter++;
        locBag.AddLocation(station, true);
      }
      i++;
    } while (in.good());
    itsLocations = NFmiHPlaceDescriptor(locBag);  // tyhjennetään ensin asemalista
    itsInitLogMessage = "Initializing sounding station data went OK, from file: ";
    itsInitLogMessage += theInitFileName;
    if (itsLocations.Size() == 0)
      itsInitLogMessage += "\nWarning: 0 sounding stations found.";
    else
      itsInitLogMessage += std::string("\nInfo: ") +
                           NFmiStringTools::Convert<int>(itsLocations.Size()) +
                           " sounding stations found.";
    int diff = counter - itsLocations.Size();
    if (diff > 0)
    {
      itsInitLogMessage += "\nWarning: there were probably ";
      itsInitLogMessage += NFmiStringTools::Convert<int>(diff);
      itsInitLogMessage +=
          " 'extra' stations in file, meaning that there were more station lines than different "
          "station were found, use unique stations.";
    }
  }
  else
    throw std::runtime_error(
        std::string("NFmiRawTempStationInfoSystem::Init - trouble reading file: ") +
        theInitFileName);
}

// esim. 70-56N (ilmeisesti myös sekunnit voi olla mukana, jolloin olisi 70-56-24N)
// Station Latitude 		DD-MM-SSH where DD is degrees, MM is minutes, SS is seconds and H is
// N
// for northern hemisphere or S for southern hemisphere. The seconds value is omitted for those
// stations where the seconds value is unknown.
// tai
// esim. 008-40W (ilmeisesti myös sekunnit voi olla mukana, jolloin olisi esim. 008-40-15W)
// Station Longitude 		DDD-MM-SSH where DDD is degrees, MM is minutes, SS is seconds and H
// is E for eastern hemisphere or W for western hemisphere. The seconds value is omitted for those
// stations where the seconds value is unknown.
static double GetLatOrLonFromString(const std::string &theLatOrLonStr,
                                    const std::string &theLineStr,
                                    const std::string &theInitFileName,
                                    char posMark,
                                    char negMark)
{
  std::vector<std::string> strVector = NFmiStringTools::Split(theLatOrLonStr, "-");
  if (strVector.size() < 2 || strVector.size() > 3)
    ThrowLatLonProblemException("GetLatOrLonFromString",
                                "Station lat/lon had illegal form",
                                theLatOrLonStr,
                                theLineStr,
                                theInitFileName);

  int degrees = NFmiStringTools::Convert<int>(strVector[0]);
  int minutes = 0;
  int seconds = 0;
  bool posSign = true;
  if (strVector.size() == 2)
  {
    char ch = static_cast<char>(
        ::toupper(strVector[1].operator[](strVector[1].size() - 1)));  // tähän N, S, W tai E
    if (ch == posMark)
      posSign = true;
    else if (ch == negMark)
      posSign = false;
    else
      ThrowLatLonProblemException("GetLatOrLonFromString",
                                  "The station's lat/lon string didn't have N, S, W or E at end",
                                  theLatOrLonStr,
                                  theLineStr,
                                  theInitFileName);
    strVector[1].resize(strVector[1].size() -
                        1);  // lyhennetään tätä stringiä, niin että viimeinen merkki jää pois
  }
  else if (strVector.size() == 3)
  {
    char ch = static_cast<char>(
        ::toupper(strVector[2].operator[](strVector[2].size() - 1)));  // tähän N, S, W tai E
    if (ch == posMark)
      posSign = true;
    else if (ch == negMark)
      posSign = false;
    else
      ThrowLatLonProblemException("GetLatOrLonFromString",
                                  "The station's lat/lon string didn't have N, S, W or E at end",
                                  theLatOrLonStr,
                                  theLineStr,
                                  theInitFileName);
    strVector[2].resize(strVector[2].size() -
                        1);  // lyhennetään tätä stringiä, niin että viimeinen merkki jää pois
    seconds = NFmiStringTools::Convert<int>(strVector[2]);
  }
  minutes = NFmiStringTools::Convert<int>(strVector[1]);
  double value = degrees;
  value += minutes / 60.;
  value += seconds / 3600.;
  if (posSign == false)
    value = -value;
  return value;
}

// ***********************************************************************
// *****************  NFmiSilamStationList  *************************
// ***********************************************************************

// täältä heitetään aina poikkeus kun on virhe tai halutaan skipata rivi
static NFmiSilamStationList::Station GetSilamStationFromString(const std::string &lineStr)
{
  if (lineStr.empty())
    throw std::runtime_error("GetSilamStationFromString - empty line string.");  // viestillä ei ole
                                                                                 // oikeastaan
  // väliä, lähinnä
  // kommentti

  if (lineStr[0] == '!')
    throw std::runtime_error("GetSilamStationFromString - comment line string.");  // viestillä ei
                                                                                   // ole oikeastaan
  // väliä, lähinnä
  // kommentti

  std::stringstream in(lineStr);
  NFmiSilamStationList::Station station;

  double value = 0;
  in >> value;  // latitude
  if (in.fail())
    throw std::runtime_error(
        "GetSilamStationFromString - error in line string.");  // viestillä ei ole oikeastaan väliä,
                                                               // lähinnä kommentti
  std::string str;
  in >> str;  // latitude sign
  if (in.fail())
    throw std::runtime_error(
        "GetSilamStationFromString - error in line string.");  // viestillä ei ole oikeastaan väliä,
                                                               // lähinnä kommentti

  station.itsLatlon.Y(value);  // latitude signilla ei ilmeisesti merkitystä, ainakin longitude sign
                               // on merkityksetön

  in >> value;  // longtitude
  if (in.fail())
    throw std::runtime_error(
        "GetSilamStationFromString - error in line string.");  // viestillä ei ole oikeastaan väliä,
                                                               // lähinnä kommentti
  in >> str;                                                   // longtitude sign
  if (in.fail())
    throw std::runtime_error(
        "GetSilamStationFromString - error in line string.");  // viestillä ei ole oikeastaan väliä,
                                                               // lähinnä kommentti

  station.itsLatlon.X(value);  // longtitude signilla ei ilmeisesti merkitystä, ainakin longitude
                               // sign on merkityksetön

  // sitten luetaan maa (joka voi olla useassa osassa)
  do
  {
    in >> str;
    if (in.fail())
      throw std::runtime_error(
          "GetSilamStationFromString - error in line string.");  // viestillä ei ole oikeastaan
                                                                 // väliä, lähinnä kommentti
    if (str.size() >= 2 &&
        ((str[0] == 'O' && str[1] == 'T') ||
         (str[0] == 'P' && str[1] == 'L')))  // tyyppi ilmeisesti alkaa OT tai PL alulla
      break;
    station.itsCountry += str;
  } while (true);

  if (in.fail())
    throw std::runtime_error(
        "GetSilamStationFromString - error in line string.");  // viestillä ei ole oikeastaan väliä,
                                                               // lähinnä kommentti

  station.itsType = str;

  int maxBufferSize = 1024 + 1;  // kuinka pitkä yhden rivin maksimissaan oletetaan olevan
  std::string buffer;
  buffer.resize(maxBufferSize);
  in.getline(&buffer[0], maxBufferSize);
  size_t length2 = ::strlen(&buffer[0]);
  buffer.resize(length2);
  station.itsInfo = buffer;

  return station;
}

// Lukee Silam tyyppisestä asemalista tiedostosta asema tiedot, heittää poikkeuksen jos ongelmia.
// Asema tiedoston formaatti on seuraava (!-merkki aloittaa kommentti rivin):
//
// !    latitude        longitude        valtio                    tyyppi               info
// !--------------------------------------------------------------------------------------------------------------------
//
// 40.600   N   44.350   E   ARMENIA   PL_PWR   Ararat, Armenia teho 2x376 MW, Shut down 1989
// 48.300   N   15.800   E   AUSTRIA   PL   Zwentendorf, Tullnerfeld 1
void NFmiSilamStationList::Init(const std::string &theInitFileName)
{
  itsInitLogMessage = "";
  itsLocations.clear();  // tyhjennetään ensin asemalista

  if (theInitFileName.empty())
    throw std::runtime_error("NFmiSilamStationList::Init - empty settings filename given.");

  ifstream in(theInitFileName.c_str(), std::ios::binary);
  if (in)
  {
    const int maxBufferSize = 1024 + 1;  // kuinka pitkä yhden rivin maksimissaan oletetaan olevan
    std::string buffer;
    Station station;
    do
    {
      buffer.resize(maxBufferSize);
      in.getline(&buffer[0], maxBufferSize);

      int realSize = static_cast<int>(strlen(buffer.c_str()));
      buffer.resize(realSize);
      buffer = ::RemoveExtraSpaces(buffer);

      try
      {
        station = ::GetSilamStationFromString(buffer);
        itsLocations.push_back(station);
      }
      catch (...)
      {
        // ei tehdä mitään, jatketaan seuraavalla rivillä niin kauan kuin rivejä riittää
      }

    } while (in.good());
    itsInitLogMessage = "Initializing silam station data went OK, from file: ";
    itsInitLogMessage += theInitFileName;
  }
  else
    throw std::runtime_error(std::string("NFmiSilamStationList::Init - trouble reading file: ") +
                             theInitFileName);
}

void NFmiSilamStationList::Clear(void)
{
  itsLocations.clear();
}

// ****************   NFmiWmoStationLookUpSystem  *************************************

NFmiWmoStationLookUpSystem::NFmiWmoStationLookUpSystem(void) : itsStations(), itsInitLogMessage()
{
}

const NFmiWmoStation &NFmiWmoStationLookUpSystem::GetStation(long theWmoId)
{
  const static NFmiWmoStation dummy;
  std::map<long, NFmiWmoStation>::const_iterator it = itsStations.find(theWmoId);
  if (it == itsStations.end())
    return dummy;
  else
    return it->second;
}

// purkaa rivin
// 01;001;ENJA;Jan Mayen;;Norway;6;70-56N;008-40W;70-56N;008-40W;10;9;P
// osiin, ja rakentaan NFmiWmoStation-otuksen.
static bool GetWmoStationFromString(const std::string &theStationStr,
                                    const std::string &theInitFileName,
                                    NFmiWmoStation &theStationOut)
{
  std::vector<std::string> strVector = NFmiStringTools::Split(theStationStr, ";");
  if (strVector.size() > 1 && strVector.size() < 13)
    throw std::runtime_error(std::string("Wmo-station error with line: ") + theStationStr);
  else if (strVector.size() <= 1)
    return false;  // ignoorataan vain tyhjä/yhden osion rivit

  theStationOut.itsIcaoStr = strVector[2];
  theStationOut.itsWmoId = NFmiStringTools::Convert<long>(strVector[0]) * 1000 +
                           NFmiStringTools::Convert<long>(strVector[1]);
  double lat = GetLatOrLonFromString(strVector[7], theStationStr, theInitFileName, 'N', 'S');
  double lon = GetLatOrLonFromString(strVector[8], theStationStr, theInitFileName, 'E', 'W');
  theStationOut.itsLatlon = NFmiPoint(lon, lat);
  theStationOut.itsName = strVector[3];
  theStationOut.itsState = strVector[4];
  theStationOut.itsCountry = strVector[5];
  return true;
}

// Lukee initialisointi tiedostosta asema tiedot, jos ongelmia, ja on annettu lokitus systeemi,
// lokittaa virheitä.
// Lukee asema tiedot jotka saadaan webistä paikasta http://weather.noaa.gov/data/nsd_bbsss.txt
// Asema tiedoston formaatti on seuraava (kommentit sallittuja, mutta asema rivit
// ilman kommentti merkkejä):
//
// Keyed by Index (Block and Station) Number
// This index table from http://weather.noaa.gov/tg/site.shtml
// and the station table from http://weather.noaa.gov/data/nsd_bbsss.txt
// Last Updated
// All stations that have an index number assigned by the WMO are available in this file. The ASCII
// file contains the following fields in the order specified, delimited by semi-colons. Fields that
// are empty will be coded as no characters (adjacent delimiters) except where otherwise noted.
//
// Block Number 		2 digits representing the WMO-assigned block.
// Station Number  		3 digits representing the WMO-assigned station.
// ICAO Location Indicator 	4 alphanumeric characters, not all stations in this file have an
// assigned location indicator. The value "----" is used for stations that do not have an assigned
// location indicator.
// Place Name 			Common name of station location.
// State 			2 character abbreviation (included for stations located in the
// United
// States
// only).
// Country Name 		Country name is ISO short English form.
// WMO Region 			digits 1 through 6 representing the corresponding WMO region, 7
// stands
// for the WMO Antarctic region.
// Station Latitude 		DD-MM-SSH where DD is degrees, MM is minutes, SS is seconds and H is
// N
// for northern hemisphere or S for southern hemisphere. The seconds value is omitted for those
// stations where the seconds value is unknown.
// Station Longitude 		DDD-MM-SSH where DDD is degrees, MM is minutes, SS is seconds and H
// is E for eastern hemisphere or W for western hemisphere. The seconds value is omitted for those
// stations where the seconds value is unknown.
// Upper Air Latitude 		DD-MM-SSH where DD is degrees, MM is minutes, SS is seconds and H is
// N for northern hemisphere or S for southern hemisphere. The seconds value is omitted for those
// stations where the seconds value is unknown.
// Upper Air Longitude 		DDD-MM-SSH where DDD is degrees, MM is minutes, SS is seconds and H
// is E for eastern hemisphere or W for western hemisphere. The seconds value is omitted for those
// stations where the seconds value is unknown.
// Station Elevation (Ha) 	The station elevation in meters. Value is omitted if unknown.
// Upper Air Elevation (Hp) 	The upper air elevation in meters. Value is omitted if unknown.
// RBSN indicator 		P if station is defined by the WMO as belonging to the Regional
// Basic
// Synoptic Network, omitted otherwise.

// 00;000;PABL;Buckland, Buckland Airport;AK;United States;4;65-58-56N;161-09-07W;;;7;;
// 01;001;ENJA;Jan Mayen;;Norway;6;70-56N;008-40W;70-56N;008-40W;10;9;P
// 01;002;----;Grahuken;;Norway;6;79-47N;014-28E;;;;15;
// 01;003;----;Hornsund;;Norway;6;77-00N;015-30E;;;10;12;
// 01;004;ENAS;Ny-Alesund Ii;;Norway;6;78-55N;011-56E;78-55N;011-56E;8;8;
// 01;006;----;Edgeoya;;Norway;6;78-14N;022-47E;;;;14;
// 01;007;----;Ny Alesund;;Norway;6;78-55N;011-56E;;;;12;
// 01;008;ENSB;Svalbard Lufthavn;;Norway;6;78-15N;015-28E;;;2;29;P
// 01;009;----;Phippsoya;;Norway;6;80-41N;020-52E;;;4;;
// 01;010;ENAN;Andoya;;Norway;6;69-18N;016-09E;69-18N;016-09E;1;14;P
// 01;011;----;Kvitoya;;Norway;6;80-06N;031-24E;80-06N;031-24E;16;16;
// 01;015;----;Hekkingen Fyr;;Norway;6;69-36N;017-50E;;;14;;
// 01;016;----;Senja-Grasmyrskogen;;Norway;6;69-17N;017-46E;;;50;;
// 01;023;ENDU;Bardufoss;;Norway;6;69-04N;018-32E;;;7;79;P
// jne....

void NFmiWmoStationLookUpSystem::Init(const std::string &theInitFileName, int theStationCountHint)
{
  // We ignore theStationCountHint, the data structure is now a map

  itsInitLogMessage = "";

  if (theInitFileName.empty())
    throw std::runtime_error("NFmiWmoStationLookUpSystem::Init - empty settings filename given.");

  NFmiCommentStripper stripComments(
      false);  // ei stripata # eli pound kommentteja, koska stripperissä on bugi
  if (stripComments.ReadAndStripFile(theInitFileName))
  {
#ifdef OLDGCC
    std::istrstream in(stripComments.GetString().c_str());
#else
    std::stringstream in(stripComments.GetString());
#endif

    std::string warningSectionStr;
    const int maxBufferSize = 1024 + 1;  // kuinka pitkä yhden rivin maksimissaan oletetaan olevan
    std::string buffer;
    NFmiWmoStation station;
    int counter = 0;
    int warningCounter = 0;
    int foundStationCounter = 0;
    do
    {
      buffer.resize(maxBufferSize);
      in.getline(&buffer[0], maxBufferSize);
      size_t realSize = strlen(buffer.c_str());
      buffer.resize(realSize);
      buffer = ::RemoveExtraSpaces(buffer);
      bool getStationStatus = true;
      try
      {
        getStationStatus = ::GetWmoStationFromString(buffer, theInitFileName, station);
      }
      catch (std::exception &e)
      {
        warningSectionStr += e.what();
        warningSectionStr += "\n";
        warningCounter++;
      }

      if (getStationStatus)
      {
        counter++;
        itsStations[station.itsWmoId] = station;
      }
    } while (in.good());

    itsInitLogMessage = "Initializing NFmiWmoStationLookUpSystem data went OK, from file: ";
    itsInitLogMessage += theInitFileName;
    if (foundStationCounter == 0)
      itsInitLogMessage += "\nWarning: 0 sounding stations found.";
    else
      itsInitLogMessage += std::string("\nInfo: ") +
                           NFmiStringTools::Convert<int>(foundStationCounter) +
                           " wmo stations found.";
    if (warningCounter > 0)
    {
      itsInitLogMessage += "\nFollowing warning were issued:\n";
      itsInitLogMessage += warningSectionStr;
    }
  }
  else
    throw std::runtime_error(
        std::string("NFmiWmoStationLookUpSystem::Init - trouble reading file: ") + theInitFileName);
}

// ****************   NFmiWmoStationLookUpSystem  *************************************

/*

AF	AFG	004	AFGHANISTAN
AL	ALB	008	ALBANIA
DZ	DZA	012	ALGERIA
AS	ASM	016	AMERICAN SAMOA
AD	AND	020	ANDORRA
AO	AGO	024	ANGOLA
AI	AIA	660	ANGUILLA
AQ	ATA	010	ANTARCTICA
AG	ATG	028	ANTIGUA AND BARBUDA
AR	ARG	032	ARGENTINA
AM	ARM	051	ARMENIA
AW	ABW	533	ARUBA
AU	AUS	036	AUSTRALIA
AT	AUT	040	AUSTRIA
AZ	AZE	031	AZERBAIJAN
BS	BHS	044	BAHAMAS
BH	BHR	048	BAHRAIN
BD	BGD	050	BANGLADESH
BB	BRB	052	BARBADOS
BY	BLR	112	BELARUS
BE	BEL	056	BELGIUM
BZ	BLZ	084	BELIZE
BJ	BEN	204	BENIN
BM	BMU	060	BERMUDA
BT	BTN	064	BHUTAN
BO	BOL	068	BOLIVIA
BA	BIH	070	BOSNIA AND HERZEGOWINA
BW	BWA	072	BOTSWANA
BV	BVT	074	BOUVET ISLAND
BR	BRA	076	BRAZIL
IO	IOT	086	BRITISH INDIAN OCEAN TERRITORY
BN	BRN	096	BRUNEI DARUSSALAM
BG	BGR	100	BULGARIA
BF	BFA	854	BURKINA FASO
BI	BDI	108	BURUNDI
KH	KHM	116	CAMBODIA
CM	CMR	120	CAMEROON
CA	CAN	124	CANADA
CV	CPV	132	CAPE VERDE
KY	CYM	136	CAYMAN ISLANDS
CF	CAF	140	CENTRAL AFRICAN REPUBLIC
TD	TCD	148	CHAD
CL	CHL	152	CHILE
CN	CHN	156	CHINA
CX	CXR	162	CHRISTMAS ISLAND
CC	CCK	166	COCOS (KEELING) ISLANDS
CO	COL	170	COLOMBIA
KM	COM	174	COMOROS
CG	COG	178	CONGO
CD	COD	180	CONGO, THE DRC
CK	COK	184	COOK ISLANDS
CR	CRI	188	COSTA RICA
CI	CIV	384	COTE D'IVOIRE
HR	HRV	191	CROATIA (local name: Hrvatska)
CU	CUB	192	CUBA
CY	CYP	196	CYPRUS
CZ	CZE	203	CZECH REPUBLIC
DK	DNK	208	DENMARK
DJ	DJI	262	DJIBOUTI
DM	DMA	212	DOMINICA
DO	DOM	214	DOMINICAN REPUBLIC
TP	TMP	626	EAST TIMOR
EC	ECU	218	ECUADOR
EG	EGY	818	EGYPT
SV	SLV	222	EL SALVADOR
GQ	GNQ	226	EQUATORIAL GUINEA
ER	ERI	232	ERITREA
EE	EST	233	ESTONIA
ET	ETH	231	ETHIOPIA
FK	FLK	238	FALKLAND ISLANDS (MALVINAS)
FO	FRO	234	FAROE ISLANDS
FJ	FJI	242	FIJI
FI	FIN	246	FINLAND
FR	FRA	250	FRANCE
FX	FXX	249	FRANCE, METROPOLITAN
GF	GUF	254	FRENCH GUIANA
PF	PYF	258	FRENCH POLYNESIA
TF	ATF	260	FRENCH SOUTHERN TERRITORIES
GA	GAB	266	GABON
GM	GMB	270	GAMBIA
GE	GEO	268	GEORGIA
DE	DEU	276	GERMANY
GH	GHA	288	GHANA
GI	GIB	292	GIBRALTAR
GR	GRC	300	GREECE
GL	GRL	304	GREENLAND
GD	GRD	308	GRENADA
GP	GLP	312	GUADELOUPE
GU	GUM	316	GUAM
GT	GTM	320	GUATEMALA
GN	GIN	324	GUINEA
GW	GNB	624	GUINEA-BISSAU
GY	GUY	328	GUYANA
HT	HTI	332	HAITI
HM	HMD	334	HEARD AND MC DONALD ISLANDS
VA	VAT	336	HOLY SEE (VATICAN CITY STATE)
HN	HND	340	HONDURAS
HK	HKG	344	HONG KONG
HU	HUN	348	HUNGARY
IS	ISL	352	ICELAND
IN	IND	356	INDIA
ID	IDN	360	INDONESIA
IR	IRN	364	IRAN (ISLAMIC REPUBLIC OF)
IQ	IRQ	368	IRAQ
IE	IRL	372	IRELAND
IL	ISR	376	ISRAEL
IT	ITA	380	ITALY
JM	JAM	388	JAMAICA
JP	JPN	392	JAPAN
JO	JOR	400	JORDAN
KZ	KAZ	398	KAZAKHSTAN
KE	KEN	404	KENYA
KI	KIR	296	KIRIBATI
KP	PRK	408	KOREA, D.P.R.O.
KR	KOR	410	KOREA, REPUBLIC OF
KW	KWT	414	KUWAIT
KG	KGZ	417	KYRGYZSTAN
LA	LAO	418	LAOS
LV	LVA	428	LATVIA
LB	LBN	422	LEBANON
LS	LSO	426	LESOTHO
LR	LBR	430	LIBERIA
LY	LBY	434	LIBYAN ARAB JAMAHIRIYA
LI	LIE	438	LIECHTENSTEIN
LT	LTU	440	LITHUANIA
LU	LUX	442	LUXEMBOURG
MO	MAC	446	MACAU
MK	MKD	807	MACEDONIA
MG	MDG	450	MADAGASCAR
MW	MWI	454	MALAWI
MY	MYS	458	MALAYSIA
MV	MDV	462	MALDIVES
ML	MLI	466	MALI
MT	MLT	470	MALTA
MH	MHL	584	MARSHALL ISLANDS
MQ	MTQ	474	MARTINIQUE
MR	MRT	478	MAURITANIA
MU	MUS	480	MAURITIUS
YT	MYT	175	MAYOTTE
MX	MEX	484	MEXICO
FM	FSM	583	MICRONESIA, FEDERATED STATES OF
MD	MDA	498	MOLDOVA, REPUBLIC OF
MC	MCO	492	MONACO
MN	MNG	496	MONGOLIA
MS	MSR	500	MONTSERRAT
MA	MAR	504	MOROCCO
MZ	MOZ	508	MOZAMBIQUE
MM	MMR	104	MYANMAR (Burma)
NA	NAM	516	NAMIBIA
NR	NRU	520	NAURU
NP	NPL	524	NEPAL
NL	NLD	528	NETHERLANDS
AN	ANT	530	NETHERLANDS ANTILLES
NC	NCL	540	NEW CALEDONIA
NZ	NZL	554	NEW ZEALAND
NI	NIC	558	NICARAGUA
NE	NER	562	NIGER
NG	NGA	566	NIGERIA
NU	NIU	570	NIUE
NF	NFK	574	NORFOLK ISLAND
MP	MNP	580	NORTHERN MARIANA ISLANDS
NO	NOR	578	NORWAY
OM	OMN	512	OMAN
PK	PAK	586	PAKISTAN
PW	PLW	585	PALAU
PA	PAN	591	PANAMA
PG	PNG	598	PAPUA NEW GUINEA
PY	PRY	600	PARAGUAY
PE	PER	604	PERU
PH	PHL	608	PHILIPPINES
PN	PCN	612	PITCAIRN
PL	POL	616	POLAND
PT	PRT	620	PORTUGAL
PR	PRI	630	PUERTO RICO
QA	QAT	634	QATAR
RE	REU	638	REUNION
RO	ROM	642	ROMANIA
RU	RUS	643	RUSSIAN FEDERATION
RW	RWA	646	RWANDA
KN	KNA	659	SAINT KITTS AND NEVIS
LC	LCA	662	SAINT LUCIA
VC	VCT	670	SAINT VINCENT AND THE GRENADINES
WS	WSM	882	SAMOA
SM	SMR	674	SAN MARINO
ST	STP	678	SAO TOME AND PRINCIPE
SA	SAU	682	SAUDI ARABIA
SN	SEN	686	SENEGAL
SC	SYC	690	SEYCHELLES
SL	SLE	694	SIERRA LEONE
SG	SGP	702	SINGAPORE
SK	SVK	703	SLOVAKIA (Slovak Republic)
SI	SVN	705	SLOVENIA
SB	SLB	090	SOLOMON ISLANDS
SO	SOM	706	SOMALIA
ZA	ZAF	710	SOUTH AFRICA
GS	SGS	239	SOUTH GEORGIA AND SOUTH S.S.
ES	ESP	724	SPAIN
LK	LKA	144	SRI LANKA
SH	SHN	654	ST. HELENA
PM	SPM	666	ST. PIERRE AND MIQUELON
SD	SDN	736	SUDAN
SR	SUR	740	SURINAME
SJ	SJM	744	SVALBARD AND JAN MAYEN ISLANDS
SZ	SWZ	748	SWAZILAND
SE	SWE	752	SWEDEN
CH	CHE	756	SWITZERLAND
SY	SYR	760	SYRIAN ARAB REPUBLIC
TW	TWN	158	TAIWAN, PROVINCE OF CHINA
TJ	TJK	762	TAJIKISTAN
TZ	TZA	834	TANZANIA, UNITED REPUBLIC OF
TH	THA	764	THAILAND
TG	TGO	768	TOGO
TK	TKL	772	TOKELAU
TO	TON	776	TONGA
TT	TTO	780	TRINIDAD AND TOBAGO
TN	TUN	788	TUNISIA
TR	TUR	792	TURKEY
TM	TKM	795	TURKMENISTAN
TC	TCA	796	TURKS AND CAICOS ISLANDS
TV	TUV	798	TUVALU
UG	UGA	800	UGANDA
UA	UKR	804	UKRAINE
AE	ARE	784	UNITED ARAB EMIRATES
GB	GBR	826	UNITED KINGDOM
US	USA	840	UNITED STATES
UM	UMI	581	U.S. MINOR ISLANDS
UY	URY	858	URUGUAY
UZ	UZB	860	UZBEKISTAN
VU	VUT	548	VANUATU
VE	VEN	862	VENEZUELA
VN	VNM	704	VIET NAM
VG	VGB	092	VIRGIN ISLANDS (BRITISH)
VI	VIR	850	VIRGIN ISLANDS (U.S.)
WF	WLF	876	WALLIS AND FUTUNA ISLANDS
EH	ESH	732	WESTERN SAHARA
YE	YEM	887	YEMEN
YU	YUG	891	Yugoslavia (Serbia and Montenegro)
ZM	ZMB	894	ZAMBIA
ZW 	ZWE	716	ZIMBABWE

*/
