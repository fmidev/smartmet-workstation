#include "bzip2util.h"
#include "NFmiFileSystem.h"
#include "NFmiQueryData.h"
#include "NFmiFileString.h"

#include <fstream>

#ifdef _MSC_VER
#pragma warning (disable : 4244 4127) // boost:in kirjastosta tulee ik‰v‰sti varoituksia
#endif
	#include <boost/iostreams/filtering_stream.hpp>
	#include <boost/iostreams/filter/bzip2.hpp>
    #include <boost/iostreams/copy.hpp>
#ifdef _MSC_VER
#pragma warning (default : 4244 4127) // laitetaan varoitukset takaisin p‰‰lle
#endif


// Kopsasin apuluokan, joka osaa heitt‰‰ poikkeuksen, kun boost::iostreams::copy failaa. Luokka on kopsattu:
// http://stackoverflow.com/questions/20414577/boostiostreamscopy-sink-enospc-no-space-left-on-device-error-handling
struct safe_ofstream_sink
{
    typedef char char_type;
    typedef boost::iostreams::sink_tag category;

    std::ofstream& ofs;

    safe_ofstream_sink(std::ofstream& ofs) :
            ofs(ofs)
    {
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        ofs.write(s, n);
        if (!ofs)
            throw std::runtime_error("Failed writing to fstream");

        return n;
    }

private:
    safe_ofstream_sink& operator=(const safe_ofstream_sink &other); // disabloidaan sijoitus operator
};

bool CFmiBzip2Helpers::UnpackBzip2DataFile(const std::string& thePackedFileName, const std::string& theUnpackedFileName, bool fDeletePackedFileAfter)
{
	std::ifstream inFile(thePackedFileName.c_str(), std::ios_base::in | std::ios_base::binary);
    if(!inFile)
        throw std::runtime_error(std::string("Error in CFmiWin32Helpers::UnpackBzip2QueryDataFile: unable to open source file: ") + thePackedFileName);
	std::ofstream outFile(theUnpackedFileName.c_str(), std::ios_base::out | std::ios_base::binary);
    if(!outFile)
        throw std::runtime_error(std::string("Error in CFmiWin32Helpers::UnpackBzip2QueryDataFile: unable to open dest file: ") + theUnpackedFileName);

    boost::iostreams::filtering_streambuf<boost::iostreams::output> filters;
    filters.push(boost::iostreams::bzip2_decompressor());
    filters.push(safe_ofstream_sink(outFile));
    bool status = false;
    try
    {
        status = boost::iostreams::copy(inFile, filters) > 0; // jos copy palautta 0:sta poikkeavan luvu, homma onnistui
    }
    catch(std::exception &e)
    {
        if(fDeletePackedFileAfter)
            NFmiFileSystem::RemoveFile(thePackedFileName);

        std::string errStr("Error in CFmiWin32Helpers::UnpackBzip2QueryDataFile: unable to unpack file: ");
        errStr += thePackedFileName;
        errStr += " to file: ";
        errStr += theUnpackedFileName;
        errStr += " because: ";
        errStr += e.what();
        throw std::runtime_error(errStr);
    }
    catch(...)
    {
        if(fDeletePackedFileAfter)
            NFmiFileSystem::RemoveFile(thePackedFileName);

        std::string errStr("Unknown error in CFmiWin32Helpers::UnpackBzip2QueryDataFile: unable to unpack file: ");
        errStr += thePackedFileName;
        errStr += " to file: ";
        errStr += theUnpackedFileName;
        throw std::runtime_error(errStr);
    }
    if(fDeletePackedFileAfter)
        NFmiFileSystem::RemoveFile(thePackedFileName);
    return status;
}

// t‰st‰ pit‰isi tehd‰ DataHandler-plugin.
// Tukee bzip2 ja Z pakattuja tiedostoja
NFmiQueryData* CFmiBzip2Helpers::ReadQueryDataFromCompressedFile(const std::string& theFileName)
{
	bool useBZip2 = false;
	bool useZip = false;
	NFmiFileString fileString(theFileName);
	if(fileString.Extension() == NFmiString("bz2"))
		useBZip2 = true;
#ifdef FMI_USE_BOOST_ZLIB
	else if(fileString.Extension() == NFmiString("Z")) // eli ei normaali zip-tiedosto vaan Z-tiedosto!
		useZip = true;
#endif
	if(useBZip2 || useZip)
	{
		std::auto_ptr<NFmiQueryData> data(new NFmiQueryData());
		try
		{
			using namespace std;
			using namespace boost;
			using namespace boost::iostreams;

			ifstream file(theFileName.c_str(), ios_base::in | ios_base::binary);
			filtering_stream<input> in;
			if(useBZip2)
				in.push(bzip2_decompressor());
#ifdef FMI_USE_BOOST_ZLIB
			else if(useZip)
				in.push(zlib_decompressor());
#endif
			in.push(file);
			in >> *(data.get());
			return data.release();
		}
		catch(...)
		{
		}
	}
	return 0;
}

// siis lukee queryData tiedostosta pelk‰n infon siit‰ tiedoston alusta
NFmiQueryInfo* CFmiBzip2Helpers::ReadQueryInfoFromCompressedFile(const std::string& theFileName)
{
	bool useBZip2 = false;
	bool useZip = false;
	NFmiFileString fileString(theFileName);
	if(fileString.Extension() == NFmiString("bz2"))
		useBZip2 = true;
#ifdef FMI_USE_BOOST_ZLIB
	else if(fileString.Extension() == NFmiString("Z")) // eli ei normaali zip-tiedosto vaan Z-tiedosto!
		useZip = true;
#endif
	if(useBZip2 || useZip)
	{
		std::auto_ptr<NFmiQueryInfo> info(new NFmiQueryInfo());
		try
		{
			using namespace std;
			using namespace boost;
			using namespace boost::iostreams;

			ifstream file(theFileName.c_str(), ios_base::in | ios_base::binary);
			filtering_stream<input> in;
			if(useBZip2)
				in.push(bzip2_decompressor());
#ifdef FMI_USE_BOOST_ZLIB
			else if(useZip)
				in.push(zlib_decompressor());
#endif
			in.push(file);
			in >> *(info.get());
			return info.release();
		}
		catch(...)
		{
		}
	}
	return 0;
}

