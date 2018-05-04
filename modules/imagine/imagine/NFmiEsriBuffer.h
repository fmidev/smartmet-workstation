// ======================================================================
//
// Utilities for reading/ writing values from/to a character buffer
//
// ======================================================================

#pragma once

#include <string>

namespace Imagine
{
namespace NFmiEsriBuffer
{
bool IsCpuLittleEndian(void);

int BigEndianInt(const std::string& theBuffer, int thePos);
int LittleEndianInt(const std::string& theBuffer, int thePos);
int BigEndianShort(const std::string& theBuffer, int thePos);
int LittleEndianShort(const std::string& theBuffer, int thePos);
double LittleEndianDouble(const std::string& theBuffer, int thePos);

const std::string BigEndianInt(int theValue);
const std::string LittleEndianInt(int theValue);
const std::string LittleEndianDouble(double theValue);
const std::string LittleEndianShort(int theValue);

bool EsriRead(std::istream& is, std::string& theString, unsigned int theLength);
}
}  // namespace Imagine


// ======================================================================
