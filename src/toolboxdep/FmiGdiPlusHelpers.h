#pragma once

#include "stdafx.h"
#include <gdiplus.h>
#pragma warning( push )
#pragma warning( disable : 4244 )
#include <vector>
// Undo the above warning disabling.
#pragma warning( pop )

#include "NFmiColor.h"

class CBitmap;
class SmartMetDocumentInterface;
class NFmiRect;
class NFmiToolBox;
class NFmiPoint;
class NFmiCtrlView;

namespace Gdiplus
{
	class Bitmap;
	class StringFormat;
	class PointF;
}

// CFmiGdiPlusHelpers

namespace CFmiGdiPlusHelpers
{
    bool SaveMfcBitmapToFile(const std::string &theCallingFunctionName, CBitmap *bm, SmartMetDocumentInterface *smartMetDocumentInterface, const NFmiRect *theRelativeOutputArea);
    bool SaveMfcBitmapToFile(const std::string &theCallingFunctionName, CBitmap *bm, const std::string &theFileName, const NFmiRect *theRelativeOutputArea, bool throwError = false);
    bool SafelySaveMfcBitmapToFile(const std::string &theCallingFunctionName, CBitmap *bm, const std::string &theFileName, const NFmiRect *theRelativeOutputArea, bool throwError = false);

    std::string StrToUtf8(const std::string &theString);
};


