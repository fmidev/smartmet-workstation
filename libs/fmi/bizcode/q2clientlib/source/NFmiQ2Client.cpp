//© Ilmatieteenlaitos/Marko.
//Original 6.3.2007
// 
// NFmiQ2Client luokan implementointi.
//---------------------------------------------------------- NFmiQ2Client.cpp

#ifdef _MSC_VER
#pragma warning(disable : 4996) // poistaa varoituksen "warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead"
#endif

#define _WIN32_WINNT 0x0600 // boost/asio.hpp include pyytää että _WIN32_WINNT määritettäisiin, nyt minimi platformi on tässä Windows Vista (600)

#include <boost/asio.hpp>

#include <fstream>

#include "NFmiQ2Client.h"
#include "catlog/catlog.h"

using boost::asio::ip::tcp;

using namespace std;

static std::istream& operator>>(std::istream& s, NFmiDataMatrix<float>& m)
{
	typedef NFmiDataMatrix<float>::size_type sz_type;
	sz_type rows = 0;
	sz_type columns = 0;

	char tmp,tmp2,tmp3;
	s >> columns >> tmp >> rows >> tmp2;
	if(columns == 0 || rows == 0) // luultavasti istreamissä olevassa stringissä oli virheilmoitus tai jotain roskaa ja nyt heitetään poikkeus
		throw std::runtime_error("Failed to read matrix.");
	m.Resize(columns, rows);
	for (sz_type j = 0; j < rows; j++)
	{
		for (sz_type i = 0; i < columns; i++)
		{
			if(s.peek() == ',')
			{
				m[i][j] = kFloatMissing; // tämä on mahd. turha sijoitus, mutta varmuuden vuoksi
				s >> tmp3; // q3-server palauttaa puuttuvan arvon tyhjänä, eli seuraava pilkku on jo kohdalla ja luetaan vain se
			}
			else
				s >> m[i][j] >> tmp3;
		}
	}
	return s;
}

static int GetIntegerFromCharArrayBits(char* charArray)
{
	int value = 0;
	char *valueCharPtr = reinterpret_cast<char *>(&value);
	valueCharPtr[0] = charArray[0];
	valueCharPtr[1] = charArray[1];
	valueCharPtr[2] = charArray[2];
	valueCharPtr[3] = charArray[3];
	return value;
}

template<typename T>
static inline float SpeedChangeScaledInt2Float(T value, float scale, float base, T missingValue)
{
	if(value == missingValue)
		return kFloatMissing;
	return value/scale + base;
}

NFmiQ2Client::NFmiQ2Client(void)
{
}

NFmiQ2Client::~NFmiQ2Client(void)
{
}

static bool MakeHttpCommand(const std::string &theServerStr, const std::string &thePortStr, const std::string &theParamStr, std::string &theHeaderStr, 
	std::string &theResponseStr, std::string &theErrorStr, bool fDoGet, std::string& scheme = std::string())
{
   theHeaderStr.clear();
   theResponseStr.clear();
   theErrorStr.clear();
 
   try
   {
	   {
		   boost::asio::io_service io_service;

		   // Get a list of endpoints corresponding to the server name.
		   tcp::resolver resolver(io_service);
		   tcp::resolver::query query(theServerStr, thePortStr.empty() ? "http" : thePortStr);

		   tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		   tcp::resolver::iterator end;

		   // Try each endpoint until we successfully establish a connection.
		   tcp::socket socket(io_service);
		   boost::system::error_code error = boost::asio::error::host_not_found;
		   while (error && endpoint_iterator != end)
		   {
			   socket.close();
			   socket.connect(*endpoint_iterator++, error);
		   }
		   if (error)
			   throw boost::system::system_error(error);

		   // Form the request. We specify the "Connection: close" header so that the
		   // server will close the socket after transmitting the response. This will
		   // allow us to treat all data up until the EOF as the content.
		   boost::asio::streambuf request;
		   std::ostream request_stream(&request);

		   request_stream << (fDoGet ? "GET " : "POST ") << theParamStr << " HTTP/1.1\r\n";
		   request_stream << "Host: " << theServerStr;
		   if(thePortStr.empty() == false)
			   request_stream << ":" << thePortStr;
		   request_stream << "\r\n";
		   request_stream << "Accept: */*\r\n";
		   request_stream << "Connection: close\r\n\r\n";

		   // Send the request.
		   boost::asio::write(socket, request);

		   // Read the response status line.
		   boost::asio::streambuf response;
		   boost::asio::read_until(socket, response, "\r\n");

		   // Check that response is OK.
		   std::istream response_stream(&response);
		   std::string http_version;
		   response_stream >> http_version;
		   unsigned int status_code;
		   response_stream >> status_code;
		   std::string status_message;
		   std::getline(response_stream, status_message);
		   if (!response_stream || http_version.substr(0, 5) != "HTTP/")
			   throw std::runtime_error("Invalid response");

		   // Read the response headers, which are terminated by a blank line.
		   boost::asio::read_until(socket, response, "\r\n\r\n");

		   // Process the response headers.
		   std::string header;
		   while (std::getline(response_stream, header) && header != "\r")
			   theHeaderStr += header + "\n";
		   theHeaderStr += "\n";

		   // Write whatever content we already have to output.
		   std::stringstream sstream;
		   if (response.size() > 0)
			   sstream << &response;

		   // Read until EOF, writing data to output as we go.
		   while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
			   sstream << &response;
		   theResponseStr += sstream.str();
		   
		   if (status_code != 200)
		   {
			   theErrorStr = "";
			   theErrorStr.swap(theResponseStr);
			   return false;
		   }

		   if (error != boost::asio::error::eof)
			   throw boost::system::system_error(error);
	   }

   }
   catch (std::exception& e)
   {
       theErrorStr = std::string("Error in MakeHttpGet: \n") + e.what();
       return false;
   }

   return true;
}

// Halutaan etsiä annetusta stringista annetusta lähtöpisteestä maksimissaan tarkastaen halutun määrän merrkejä ja etsien haluttua merkkiä.
static std::string::size_type FindSubStringWithInGivenChars(const std::string &theSearchedStr, std::string::size_type theStartPos, char theSearchedChar, size_t theCheckedCharCount)
{
	for(size_t i = 0; i < theCheckedCharCount; i++)
	{
		size_t checkedPos = theStartPos + i;
		if(checkedPos < theSearchedStr.size())
		{
			if(theSearchedStr[checkedPos] == theSearchedChar)
				return checkedPos;
		}
	}
	return std::string::npos;
}

static void GetMatrixFromString3(const std::string &theResponseStr, NFmiDataMatrix<float> &theDataOut, std::string &theExtraInfoStr)
{
	std::string::size_type pos1 = theResponseStr.find(";", 0); // FindChar3(chunk, 0, ';', chunk.itsMemory.size());
	if(pos1 == std::string::npos) // ei löytynyt ;-merkkiä, pakko olla joku virhe
		throw std::runtime_error(std::string("Error message from server:\n") + theResponseStr);
	else
	{ // tutkitaan löytyykö 1. ;-merkin jälkeen numero,numero; yhdistelmä, jos löytyy, on alussa extra infoa. Jos ei löydy, ei kannata
		// käydä koko jäljellä olevaa stringiä läpi etsimässä toista mahd. ;-merkkiä
		// Käytännössä etsin seuraavat max 40 merkkiä, löytyykö toista;-merkkiä
		std::string::size_type pos2 = ::FindSubStringWithInGivenChars(theResponseStr, pos1 + 1, ';', 40);
		
		if(pos2 != std::string::npos)
		{ // nyt löytyi toinenkin ;-merkki, mikä merkitsee sitä että on olemassa extrainfo-osuus.
			// Oletus: ei saa olla kuin kaksi ;-merkkiä vastauksessa!!!! Haluan vain estää että ei tarvitse käydä koko stringiä läpi aina, koska stringi voi olla pitkä
			theExtraInfoStr = std::string(theResponseStr.begin(), theResponseStr.begin()+pos1);
		}
		else
			pos1 = static_cast<std::string::size_type>(-1); // pitää säätää pos1:n arvo kun ei ollut extrainfo osiota
	}

	if(::isdigit(theResponseStr[pos1+1]))
	{
		std::istringstream iss(&theResponseStr[0] + pos1 + 1);
		iss >> theDataOut;
	}
	else
		throw std::runtime_error(std::string("Error message from server:\n") + theResponseStr);
}

template<typename T>
static void GetMatrixFromBinaryData(char* dataChars, size_t dataCharsSize, NFmiDataMatrix<float> &theDataOut, float scale, int rows, int columns, float base, int theMissValue)
{
	T missValue = static_cast<T>(theMissValue);
	if(dataCharsSize % sizeof(T) != 0) // jos ei ollut tasajakoinen, tulosdatassa jotain outoa
		throw std::runtime_error(std::string("Error message from server:\n") + dataChars);

	size_t arraySize = dataCharsSize / sizeof(T);
	if(arraySize != static_cast<size_t>(rows*columns))
		throw std::runtime_error("Error: binary data array size doesn't macth with row*column size");

	size_t index = 0;
	T *dataArray = reinterpret_cast<T*>(dataChars);
	theDataOut.Resize(columns, rows);
	for(int j=0; j<rows; j++)
	{
		for(int i=0; i<columns; i++)
		{
			theDataOut[i][j] = SpeedChangeScaledInt2Float<T>(dataArray[index++], scale, base, missValue);
		}
	}
}

static void GetMatrixFromBinaryData3(const std::string &theBinaryResponseStr, NFmiDataMatrix<float> &theDataOut, std::string &theExtraInfoStr)
{
	// alussa on integer numerona extrainfo-osuuden koko
	char* extraInfoChars = const_cast<char*>(theBinaryResponseStr.c_str());
	int intExtraSize = ::GetIntegerFromCharArrayBits(extraInfoChars);
	if(intExtraSize)
	{ // löytyi tavaraa, luetaan ne sstringiin talteen
		if(intExtraSize > static_cast<int>(theBinaryResponseStr.size()) - 4)
			throw std::runtime_error("Error: ExtraInfo size was too big, probably the server responce format is not the wanted one.");

		extraInfoChars += 4; // siirretään ensin osoitin oikeaan paikkaan
		theExtraInfoStr = std::string(extraInfoChars, extraInfoChars+intExtraSize);
	}

	char* dataChars = const_cast<char*>(theBinaryResponseStr.c_str() + 4 + intExtraSize); // matriisi data löytyy integer luvun ja extrainfon jälkeen
	size_t dataCharsSize = theBinaryResponseStr.size() - (4 + intExtraSize);

	// matriisin ns. header osio on 6 integer lukua
	const int kHeaderSizeInChars = 6*sizeof(int);
	if(kHeaderSizeInChars > dataCharsSize)
		throw std::runtime_error("Error: there was not enough room for header data in received data chunk.");

	int *intHeaderArray = reinterpret_cast<int*>(dataChars);
	int intHeaderIndex = 0;

	int outputData_typeID = intHeaderArray[intHeaderIndex++];
	float scale = static_cast<float>(intHeaderArray[intHeaderIndex++]);
	int rows = intHeaderArray[intHeaderIndex++];
	int columns = intHeaderArray[intHeaderIndex++];
	int intBase = intHeaderArray[intHeaderIndex++];
	float base = SpeedChangeScaledInt2Float<int>(intBase, scale, 0, 9999999);
	int missValue = intHeaderArray[intHeaderIndex++];
	if(outputData_typeID == 1)
		GetMatrixFromBinaryData<short>(dataChars+kHeaderSizeInChars, dataCharsSize-kHeaderSizeInChars, theDataOut, scale, rows, columns, base, missValue);
	else if(outputData_typeID == 2)
		GetMatrixFromBinaryData<int>(dataChars+kHeaderSizeInChars, dataCharsSize-kHeaderSizeInChars, theDataOut, scale, rows, columns, base, missValue);
	else
		throw std::runtime_error("Error: unsupported outputDatatypeID, only short and int is supported");
}

static void GetDataMatrixData(const std::string &theResponseStr, bool fUseBinaryData, CompressionType /* theUsedCompression */ , NFmiDataMatrix<float> &theDataMatrixOut, string &theExtraInfoStrOut, bool logDataRequest)
{
	if(theResponseStr.size() > 0)
	{
		// tarvittaessa pura chunk:in pakattu stringi ja laita se takaisin chunkille uuden pituuden kera (tuhoa vanha)
/*
		if(theUsedCompression == kComprZip) 
			::UncompressMemoryChunkZip3(theMemoryChunk);
		else if(theUsedCompression == kComprBZ2)
			::UncompressMemoryChunkBZ2_3(theMemoryChunk);
*/
		// tässä tarkistetaan, löytyykö error sanaa stringin alusta, jolloin tiedetään että 
		// paluuna tuli virheviesti.
		if(theResponseStr.size() > 5)
		{
			std::string firstLetters;
			firstLetters.resize(5); // tehdään tilaa mahdolliselle "error" sanalle
			std::copy(theResponseStr.begin(), theResponseStr.begin()+5, firstLetters.begin());
			NFmiStringTools::LowerCase(firstLetters);

			// Q2-serveri palautti "error" sanan virhetilanteissa.
			// q3-serveri asettaa http-header flagin päälle virheen merkiksi.
			if(firstLetters == std::string("error"))
			{
				std::string errorStr(theResponseStr.begin(), theResponseStr.end());
				if(logDataRequest)
					CatLog::logMessage(errorStr, CatLog::Severity::Error, CatLog::Category::NetRequest);

				throw std::runtime_error(errorStr);
			}
		}

		if(fUseBinaryData)
			GetMatrixFromBinaryData3(theResponseStr, theDataMatrixOut, theExtraInfoStrOut);
		else
			GetMatrixFromString3(theResponseStr, theDataMatrixOut, theExtraInfoStrOut);
	}
	else
		throw std::runtime_error("ERROR: response received from server is empty.");
}

// Splits url in two or three parts: server address part, param part and used port part.
// From sample url:
// "http://brainstormgw.fmi.fi/q2?requestType=grid&paramId=4&validTime=NOW+6&producerId=230"
// server address part is "brainstormgw.fmi.fi"
// and param part is "/q2?requestType=grid&paramId=4&validTime=NOW+6&producerId=230"
// and port part is empty

// From sample url:
// "http://brainstormgw.fmi.fi:8089/q2?requestType=grid&paramId=4&validTime=NOW+6&producerId=230"
// server address part is "brainstormgw.fmi.fi"
// and param part is "/q2?requestType=grid&paramId=4&validTime=NOW+6&producerId=230"
// and port part is "8089"

static bool SplitUrlStr(const std::string &theUrlStrIn, std::string &theServerAddressStrOut, std::string &theParamStrOut, std::string &thePortStrOut, std::string& scheme = std::string())
{
	std::string tmpUrlStr = theUrlStrIn;

	std::string::size_type pos3 = tmpUrlStr.find(":");
	if (pos3 != std::string::npos)
		scheme = std::string(theUrlStrIn.begin(), theUrlStrIn.begin() + pos3);

	std::string::size_type currentPos = tmpUrlStr.find("//"); // etsitään, löytyykö tupla kenoa
	if(currentPos != std::string::npos) // jos löytyi, poistetaan alku osa koodista 
		tmpUrlStr = std::string(tmpUrlStr.begin()+currentPos+2, tmpUrlStr.end());

	std::string::size_type pos1 = tmpUrlStr.find("/"); // etsitään, 1. keno
	std::string::size_type pos2 = tmpUrlStr.find(":"); // etsitään, 1. :-merkki
	if(pos1 != std::string::npos)
	{
		bool portPartFound = (pos2 != std::string::npos && pos2 < pos1);
		if(portPartFound)
		{
			theServerAddressStrOut = std::string(tmpUrlStr.begin(), tmpUrlStr.begin()+pos2);
			thePortStrOut = std::string(tmpUrlStr.begin()+pos2+1, tmpUrlStr.begin()+pos1);
			theParamStrOut = std::string(tmpUrlStr.begin()+pos1, tmpUrlStr.end());
		}
		else
		{
			theServerAddressStrOut = std::string(tmpUrlStr.begin(), tmpUrlStr.begin()+pos1);
			thePortStrOut = "";
			theParamStrOut = std::string(tmpUrlStr.begin()+pos1, tmpUrlStr.end());
		}
		return true;
	}
	return false;
}

void NFmiQ2Client::GetDataFromQ2Server(const std::string &theURLStr, const std::string &theParamsStr, bool fUseBinaryData, CompressionType theUsedCompression, NFmiDataMatrix<float> &theDataMatrixOut, string &theExtraInfoStrOut, bool logDataRequest)
{
	// alustetaan vielä parametri stringiä annetuilla optioilla
	string finalParamsStr(theParamsStr);
	if(fUseBinaryData)
        finalParamsStr += "&output=bin";
/*
	if(theUsedCompression == kComprZip)
		finalUrlStr += "&compress=zip";
	else if(theUsedCompression == kComprBZ2)
		finalUrlStr += "&compress=bz2";
*/

    string finalUrlStr = theURLStr + "?" + finalParamsStr;

	if(logDataRequest)
	{
		std::string logMsg("The Q2/Q3 url string was:\n");
		logMsg += NFmiStringTools::UrlDecode(finalUrlStr); // Puretaan lokitukseen url encodaus, jotta haku näyttää ihmismäiseltä
        CatLog::logMessage(logMsg, CatLog::Severity::Debug, CatLog::Category::NetRequest);
	}

	std::string serverAddressStr;
	std::string paramsStr;
	std::string portStr;
	if(::SplitUrlStr(finalUrlStr, serverAddressStr, paramsStr, portStr))
	{
		std::string headerStr;
		std::string responseStr;
		std::string errorStr;
		if(::MakeHttpCommand(serverAddressStr, portStr, paramsStr, headerStr, responseStr, errorStr, true))
			::GetDataMatrixData(responseStr, fUseBinaryData, theUsedCompression, theDataMatrixOut, theExtraInfoStrOut, logDataRequest);
		else
			throw std::runtime_error(errorStr);
	}
	else
		throw std::runtime_error(std::string("Error in NFmiQ2Client::GetDataFromQ2Server: given url was illegal:\n") + theURLStr);
}

void NFmiQ2Client::MakeHTTPRequest(const std::string &theUrlStr, std::string &theResponseStrOut, bool fDoGet)
{
	std::string serverAddressStr;
	std::string paramsStr;
	std::string portStr;
	std::string scheme;

	if(::SplitUrlStr(theUrlStr, serverAddressStr, paramsStr, portStr, scheme))
	{
		std::string headerStr;
		std::string errorStr;
		::MakeHttpCommand(serverAddressStr, portStr, paramsStr, headerStr, theResponseStrOut, errorStr, fDoGet, scheme);
	}
	else
		throw std::runtime_error(std::string("Error in NFmiQ2Client::MakeHTTPRequest: given url was illegal:\n") + theUrlStr);
}
