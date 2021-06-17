//© Ilmatieteenlaitos/Marko.
//Original 6.10.2009
//
// Luokka tietää brainstorm autocomplete-pluginin toiminnosta.
// Luojkan avulla sieltä haetaan paikkahaku tietoa paikannimien 
// alun perusteella.
//---------------------------------------------------------- NFmiAutoComplete.h

#pragma once

#include "NFmiPoint.h"
#include "NFmiColor.h"
#include <vector>

class NFmiQ2Client;

// olisin laittanut tämän NFmiAutoComplete-luokan sisään, mutta
// tällöin sitä ei voisi etekäteen esitellä C++-kielessä. 
class NFmiACLocationInfo
{
public:
	static NFmiPoint gMissingLatlon;

	NFmiACLocationInfo(void);

	int itsId;
	std::wstring itsName;
	std::wstring itsCountry;
	std::wstring itsFeature;
	std::wstring itsArea;
	NFmiPoint itsLatlon;
	std::wstring itsTimeZone;
};

class NFmiAutoComplete
{
public:

	NFmiAutoComplete(void);
	~NFmiAutoComplete(void);

	void InitFromSettings(const std::string &theBaseNameSpace);
//	void StoreToSettings(void); // tätä ei ole ehkä edes tarkoitus toteuttaa

	std::vector<NFmiACLocationInfo> DoAutoComplete(NFmiQ2Client &theHttpClient, const std::string &theWord, bool logEvents);
	const std::vector<NFmiACLocationInfo>& AutoCompleteResults(void) const {return itsResults;};
	void AutoCompleteResults(const std::vector<NFmiACLocationInfo> &newValue) {itsResults = newValue;}
	bool AutoCompleteDialogOn(void) const {return fAutoCompleteDialogOn;}
	void AutoCompleteDialogOn(bool newValue) {fAutoCompleteDialogOn = newValue;}

	// teen aluksi vain get-metodeja, lisään setterit tarvittaessa
	bool Use(void) const {return fUse;}

	const NFmiColor& NameTextColor (void) const {return itsNameTextColor;}
	const NFmiColor& RectangleFillColor(void) const {return itsRectangleFillColor;}
	const NFmiColor& RectangleFrameColor(void) const {return itsRectangleFrameColor;}
	const NFmiColor& RectangleOutOfMapFillColor(void) const {return itsRectangleOutOfMapFillColor;}
	const NFmiColor& RectangleOutOfMapFrameColor(void) const {return itsRectangleOutOfMapFrameColor;}
	const NFmiColor& RectangleSelectedFillColor(void) const {return itsRectangleSelectedFillColor;}
	const NFmiColor& RectangleSelectedFrameColor(void) const {return itsRectangleSelectedFrameColor;}
	const NFmiColor& MarkerFrameColor(void) const {return itsMarkerFrameColor;}
	const NFmiColor& MarkerFillColor(void) const {return itsMarkerFillColor;}
	const NFmiColor& OutOfMapArroyHeadColor(void) const {return itsOutOfMapArroyHeadColor;}

	double OutOfMapArroyHeadInMM(void) const {return itsOutOfMapArroyHeadInMM;}
	double MarkerSizeInMM(void) const {return itsMarkerSizeInMM;}
	double FontSizeInMM(void) const {return itsFontSizeInMM;}
	double RectangleFramePenSizeInMM(void) const {return itsRectangleFramePenSizeInMM;}
	double ConnectingLinePenSizeInMM(void) const {return itsConnectingLinePenSizeInMM;}

	std::string FontName(void) const {return itsFontName;}

private:
	std::wstring GetAutoCompleteRespond(NFmiQ2Client &theHttpClient, const std::string &theWord, bool logEvents);
	void doColorAlphaFixes();

	bool fAutoCompleteDialogOn; // jos dialogi on päällä, tämä on true, muuten false
	std::vector<NFmiACLocationInfo> itsResults; // tänne voidaan tallettaa erikseen tuloksia

	std::string itsBaseUrl;
	int itsMaxResults;

	bool fUse; // onko haku systeemi ollenkaan käytössä vai ei
	std::vector<std::string> itsKeyWordList; // tämä lista sisältää ne keyword-sanat, joilla voidaan säätää kriteerejä, joilla hakukoneesta haetaan paikkoja
	NFmiColor itsNameTextColor;
	NFmiColor itsRectangleFillColor; // millä värillä piirretään laatikko karttanäytölle, mukana myös alpha!
	NFmiColor itsRectangleFrameColor;
	NFmiColor itsRectangleOutOfMapFillColor; // millä värillä piirretään laatikko karttanäytölle, jos kyseinen paikka ei ole kartalla
	NFmiColor itsRectangleOutOfMapFrameColor;
	NFmiColor itsRectangleSelectedFillColor; // kun drop listasta on valittu joku, piirretään kyseinen lokaatio merkki laatikko tällä värillä
	NFmiColor itsRectangleSelectedFrameColor;
	NFmiColor itsMarkerFrameColor;
	NFmiColor itsMarkerFillColor;
	NFmiColor itsOutOfMapArroyHeadColor;
	double itsOutOfMapArroyHeadInMM; // lähinnä kolmio, joka osoittaa tekstilaatikosta kohti kartan ulkopuolella olevaa kohdettaan
	double itsMarkerSizeInMM;
	double itsFontSizeInMM;
	double itsRectangleFramePenSizeInMM;
	double itsConnectingLinePenSizeInMM;

	std::string itsFontName;
    // invertColorAlphaValues_:in pitää olla true, kunnes SmartMet versio 5.13.15.0 on vanhin 
    // käytössä olevista versioista ja silloin tämä voidaan muuttaa false:ksi ja muuttaa 
    // color_contour_legend_settings.conf tiedoston väriasetukset niin että full opacity 
    // on 0.f ja full transparency on 1.f.
    bool invertColorAlphaValues_ = true;

	std::string itsBaseNameSpace;
};

