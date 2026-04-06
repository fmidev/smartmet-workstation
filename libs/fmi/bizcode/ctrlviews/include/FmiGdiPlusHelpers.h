#pragma once

#ifndef UNIX

#include <string>

class CBitmap;
class SmartMetDocumentInterface;
class NFmiRect;

// CFmiGdiPlusHelpers

namespace CFmiGdiPlusHelpers
{
    bool SaveMfcBitmapToFile(const std::string &theCallingFunctionName, CBitmap *bm, SmartMetDocumentInterface *smartMetDocumentInterface, const NFmiRect *theRelativeOutputArea);
    bool SaveMfcBitmapToFile(const std::string &theCallingFunctionName, CBitmap *bm, const std::string &theFileName, const NFmiRect *theRelativeOutputArea, bool throwError = false);
    bool SafelySaveMfcBitmapToFile(const std::string &theCallingFunctionName, CBitmap *bm, const std::string &theFileName, const NFmiRect *theRelativeOutputArea, bool throwError = false);
    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
};



#else  // UNIX - Windows-only header; Linux implementation will use Qt
#endif // UNIX
