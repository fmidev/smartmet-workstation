#pragma once

// HUOM! tämä ei ole nyt käytössä SmartMetDllLib:issä, koska en saanut sen avulla kuin haettua 
// kuvia http-rajapinnan yli ja talletettua levylle, mutta en voinut piirtää niitä näytölle.
// Tämä luokka vaatii myös että kääntäjän crl-optio on päällä, koska se käyttää C#-toimintoja.

#ifdef SMARTMETDLLLIB_BUILD
#define SMARTMETDLLLIB_USAGE __declspec(dllexport)
#else
#define SMARTMETDLLLIB_USAGE __declspec(dllimport)
#endif

class SMARTMETDLLLIB_USAGE CFmiImageFromWeb
{
public:
	CFmiImageFromWeb(void);
	~CFmiImageFromWeb(void);
	bool GetImage(void);
};
