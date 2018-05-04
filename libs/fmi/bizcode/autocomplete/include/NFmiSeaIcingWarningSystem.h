#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "WarningSymbolInfo.h"
#include "xmlliteutils/XMLite.h"

#include <set>

class NFmiArea;
class NFmiStopFunctor;

class NFmiSeaIcingWarningMessage
{
public:
	NFmiSeaIcingWarningMessage(void);
	~NFmiSeaIcingWarningMessage(void);

	void InitializeFromFormNode(LPXNode theNode);
	void Clear(void);

	const NFmiMetTime& Time(void) const {return itsTime;}
	void Time(const NFmiMetTime &newValue) {itsTime = newValue;}
	const std::string& MessageStr(void) const {return itsMessageStr;}
	void MessageStr(const std::string &newValue) {itsMessageStr = newValue;}
	const NFmiPoint& LatlonPoint(void) const {return itsLatlonPoint;}
	void LatlonPoint(const NFmiPoint &newValue) {itsLatlonPoint = newValue;}
	const std::string& TotalMessageStr(void) const {return itsTotalMessageStr;}
	void TotalMessageStr(const std::string &newValue) {itsTotalMessageStr = newValue;}

	const std::string& IdStr(void) const {return itsIdStr;}
	void IdStr(const std::string &newValue) {itsIdStr = newValue;}
	const std::string& SprayStr(void) const {return itsSprayStr;}
	void SprayStr(const std::string &newValue) {itsSprayStr = newValue;}
	const std::string& Intensity1Str(void) const {return itsIntensity1Str;}
	void Intensity1Str(const std::string &newValue) {itsIntensity1Str = newValue;}
	const std::string& Intensity2Str(void) const {return itsIntensity2Str;}
	void Intensity2Str(const std::string &newValue) {itsIntensity2Str = newValue;}
	const std::string& AirKJStr(void) const {return itsAirKJStr;}
	void AirKJStr(const std::string &newValue) {itsAirKJStr = newValue;}
	const std::string& VisKMStr(void) const {return itsVisKMStr;}
	void VisKMStr(const std::string &newValue) {itsVisKMStr = newValue;}
	const std::string& WSStr(void) const {return itsWSStr;}
	void WSStr(const std::string &newValue) {itsWSStr = newValue;}
	const std::string& IceSpeedStr(void) const {return itsIceSpeedStr;}
	void IceSpeedStr(const std::string &newValue) {itsIceSpeedStr = newValue;}
	const std::string& TwStr(void) const {return itsTwStr;}
	void TwStr(const std::string &newValue) {itsTwStr = newValue;}
	const std::string& TemperatureStr(void) const {return itsTemperatureStr;}
	void TemperatureStr(const std::string &newValue) {itsTemperatureStr = newValue;}
	const std::string& WaveStr(void) const {return itsWaveStr;}
	void WaveStr(const std::string &newValue) {itsWaveStr = newValue;}

private:

	NFmiMetTime itsTime; // esim.  <pvm>2008-01-02</pvm><utc>23:00</utc>
	std::string itsMessageStr; // <viesti>Hiljennetty nopeutta heti ja jäätyminen jatkuu edelleen Selkämerellä ja jatkuu todennäköisesti ensi yön.Lisatietoja saa tali@eslshipping.fi</viesti>

	std::string itsIdStr; // <tunnus>230935000/OJJP/Eira</tunnus>
	std::string itsSprayStr; // <parskej>1</parskej>
	std::string itsIntensity1Str; // <intens1/>
	std::string itsIntensity2Str; // <intens2>101</intens2>
	std::string itsAirKJStr; // <ilmakj>0</ilmakj>
	std::string itsVisKMStr; // <naky_km>20.00</naky_km>
	std::string itsWSStr; // <tuuli>9.00</tuuli>
	std::string itsIceSpeedStr; // <jaa_nop>IA</jaa_nop>
	std::string itsTwStr; // <merivesi>6.00</merivesi>
	std::string itsTemperatureStr; // <ilma>-4.00</ilma>
	std::string itsWaveStr; // <aallokko>1.50</aallokko>

	NFmiPoint itsLatlonPoint; // otetaan <KoordX>3356592.9</KoordX> ja <KoordY>6775576.18</KoordY> nodeista
	std::string itsTotalMessageStr; // koko message xml löpinä
};
inline bool operator<(const NFmiSeaIcingWarningMessage &first, const NFmiSeaIcingWarningMessage &second)
{
	if(first.Time() < second.Time())
		return true;
	return false;
}

// HUOM!! Tämä NFmiSeaIcingWarningSystem-luokka on lähes sama kuin
// NFmiWarningCenterSystem-luokka. Toteutin nyt homman copy-paste + "pikku viilausta"
// -meiningillä kiireen takia. Pitäisi toteuttaa jotenkin geneerisemmin, jos aikaa.
class NFmiSeaIcingWarningSystem
{
public:
	NFmiSeaIcingWarningSystem(void);
	~NFmiSeaIcingWarningSystem(void);
	void InitializeFromSettings(const std::string &theSettingsKeyBaseStr);
	void Init(const NFmiSeaIcingWarningSystem &theData, bool doFullInit = false);
	void StoreSettings(const std::string &theSettingsKeyBaseStr);
	bool ViewVisible(void) const {return fViewVisible;}
	void ViewVisible(bool newState) {fViewVisible = newState;}
	bool IsHidden(void) const {return fIsHidden;}
	void IsHidden(bool newState) {fIsHidden = newState;}
	std::vector<NFmiSeaIcingWarningMessage*> GetWantedWarningMessages(const NFmiMetTime &theLastTime, int theStepInMinutes, const boost::shared_ptr<NFmiArea> &theArea);
    bool DoMessageSearching() const;

	const std::string& MessageFilePattern(void) const {return itsMessageFilePattern;}
	void MessageFilePattern(const std::string &newValue) {itsMessageFilePattern = newValue;}
	int UpdateTimeStepInMinutes(void) const;
	void UpdateTimeStepInMinutes(int newValue) {itsUpdateTimeStepInMinutes = newValue;}
	std::set<NFmiSeaIcingWarningMessage>& GetMessages(void){return itsMessages;}
	void Add(std::set<NFmiSeaIcingWarningMessage> &theNewMessages, time_t theTimeStamp);
	const std::string& LastErrorWhenReadingMessages(void) const {return itsLastErrorWhenReadingMessages;}
    HakeLegacySupport::WarningSymbolInfo& DefaultSymbolInfo(void) {return itsDefaultSymbolInfo;}
	void DefaultSymbolInfo(const HakeLegacySupport::WarningSymbolInfo &newValue) {itsDefaultSymbolInfo = newValue;}
	const std::string& ErrorFilePath(void) const {return itsErrorFilePath;}
	void ErrorFilePath(const std::string &newValue) {itsErrorFilePath = newValue;}
	time_t LatestReadFileTimeStamp(void) const {return itsLatestReadFileTimeStamp;}
	void LatestReadFileTimeStamp(time_t newValue) {itsLatestReadFileTimeStamp = newValue;}

	std::set<NFmiSeaIcingWarningMessage>::iterator Find(const NFmiMetTime &theStartTime);
	void Clear(bool fClearOnlyMessages);
	bool CheckForNewMessages(NFmiStopFunctor *theStopFunctor);

	// HUOM!! Tämä laittaa kommentteja mukaan!
/* // #### Read/Write toteutus vasta myöhemmin ####
	void Write(std::ostream& os) const;
	// HUOM!! ennen kuin tämä luokka luetaan sisään tiedostosta, poista kommentit
	// NFmiCommentStripper-luokalla, koska kirjoitettaessa kommentteja laitetaan
	// sekaan. Eli älä käytä suoraan tätä metodia, vaan Init(filename)-metodia!!!!
	void Read(std::istream& is);
*/

private:
	void DecodeMessages(const std::string &theXMLMessageStr, const std::string &theFileName);
	void MakeErrorPath(void);

	std::set<NFmiSeaIcingWarningMessage> itsMessages;
	std::string itsMessageFilePattern; // tämän avulla luetaan halyviestejä jostain esim. p:\data\haly\*_haly.xml
	time_t itsLatestReadFileTimeStamp; // tähän asetetaan aikaleima, joka on myöhäisimmän tiedoston aikaleima
									// Kun editori päivittää halyviesti listoja, se tarkistaa tämän avulla,
									// että ei lueta jo luettuja viestejä uudestaan.
	int itsUpdateTimeStepInMinutes; // kuinka usein tarkastetaan, onko tullut uusia sanomia. Jos <= 0, ei tehdä tarkistuksia ollenkaan
	bool fViewVisible;
    HakeLegacySupport::WarningSymbolInfo itsDefaultSymbolInfo; // jos halutaan katsoa kaikkia mahdollisia vatoituksia, käytetään silloin piirrossta näitä asetuksia
	std::string itsLastErrorWhenReadingMessages; // tähän talletetaan mahdolliset virheilmoitukset joita tulee luettaessa virheellisiä viestejä
	std::string itsErrorFilePath; // tähän hakemistoon siirretään virheelliset varoitukset
	bool fIsHidden; // jos toiminto on piilotettu konfiguraatioilla, laitetaan palauttamaan negatiivista timesteppiä, että SmartMet
};

