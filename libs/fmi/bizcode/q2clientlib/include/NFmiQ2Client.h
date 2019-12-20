//© Ilmatieteenlaitos/Marko.
//Original 6.3.2007
// 
// Luokka joka on yhteydess‰ q2server:iin. Tarvitsee boost::asio-kirjastoa 
// http-juttuihin. Tarvitsee zlib- ja bzip2- (libbz2) kirjastoja 
// pakkauksen hoitamiseen (pakkauksen purku ei nyt k‰ytˆss‰).
//---------------------------------------------------------- NFmiQ2Client.h

#ifndef __NFMIQ2CLIENT_H__
#define __NFMIQ2CLIENT_H__

#include "NFmiDataMatrix.h"

enum CompressionType
{
	kComprNone,
	kComprZip,
	kComprBZ2,
};

class NFmiQ2Client
{
public:
	NFmiQ2Client(void);
	~NFmiQ2Client(void);

	// Hakee dataa q2serverilt‰. Palauttaa datan datamatrix-luokassa ja 
	// mahdollisen extra-datan theExtraInfoStrOut-parametrissa stringina.
	// Jos haluat nopeuttaa hakua, aseta fUseBinaryData = true.
	// Jos haluat pakata, laita theUsedCompression = kComprZip tai kComprBZ2, t‰m‰ pienent‰‰
	// datan siirtoa verkon yli mutta ei v‰ltt‰m‰tt‰ nopeuta hakua.
	// HUOM! Jos datan haussa tulee mink‰‰nlaisia ongelmia, metodi heitt‰‰ runtime_error-poikkeuksen.
    void GetDataFromQ2Server(const std::string &theURLStr,
        const std::string &theParamsStr,
        bool fUseBinaryData,
        CompressionType theUsedCompression,
        NFmiDataMatrix<float> &theDataMatrixOut,
        std::string &theExtraInfoStrOut,
        bool logDataRequest);

	// Tekee http-rajapinnan l‰pi kutsun ja palauttaa vastauksen stringin‰.
	static void MakeHTTPRequest(const std::string& theUrlStr, std::string& theResponseStrOut, bool fDoGet);

private:
};

#endif // __NFMIQ2CLIENT_H__
