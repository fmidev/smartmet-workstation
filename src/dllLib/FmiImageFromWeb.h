#pragma once

// HUOM! t�m� ei ole nyt k�yt�ss� SmartMetDllLib:iss�, koska en saanut sen avulla kuin haettua 
// kuvia http-rajapinnan yli ja talletettua levylle, mutta en voinut piirt�� niit� n�yt�lle.
// T�m� luokka vaatii my�s ett� k��nt�j�n crl-optio on p��ll�, koska se k�ytt�� C#-toimintoja.

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
