#pragma once

#include <string>


class NFmiQueryData;
class NFmiQueryInfo;

namespace CFmiBzip2Helpers
{
    // Mink� tahansa pakatun tiedoston purku toiseen
	bool UnpackBzip2DataFile(const std::string& thePackedFileName, const std::string& theUnpackedFileName, bool fDeletePackedFileAfter);
	// Querydatojen ja queryInfojen luku pakatuista tiedostoista.
	NFmiQueryData* ReadQueryDataFromCompressedFile(const std::string& theFileName);
	NFmiQueryInfo* ReadQueryInfoFromCompressedFile(const std::string& theFileName); // siis lukee queryData tiedostosta pelk�n infon siit� tiedoston alusta
};
