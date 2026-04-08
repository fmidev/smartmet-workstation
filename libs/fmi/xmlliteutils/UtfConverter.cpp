#include "xmlliteutils/stdafx.h"
#include "xmlliteutils/UtfConverter.h"
#include "xmlliteutils/ConvertUTF.h"
#ifdef UNIX
#include <stdexcept>
#endif

namespace UtfConverter
{

	std::wstring FromUtf8(const std::string& utf8string)
	{
		size_t widesize = utf8string.length();
		if (sizeof(wchar_t) == 2)
		{
			std::wstring resultstring;
			resultstring.resize(widesize+1, L'\0');
			const UTF8* sourcestart = reinterpret_cast<const UTF8*>(utf8string.c_str());
			const UTF8* sourceend = sourcestart + widesize;
			UTF16* targetstart = reinterpret_cast<UTF16*>(&resultstring[0]);
			UTF16* targetend = targetstart + widesize;
			ConversionResult res = ConvertUTF8toUTF16(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
			if (res != conversionOK)
			{
#ifdef UNIX
				throw std::runtime_error("La falla!");
#else
				throw std::exception("La falla!");
#endif
			}
			*targetstart = 0;
			return resultstring;
		}
		else if (sizeof(wchar_t) == 4)
		{
			std::wstring resultstring;
			resultstring.resize(widesize+1, L'\0');
			const UTF8* sourcestart = reinterpret_cast<const UTF8*>(utf8string.c_str());
			const UTF8* sourceend = sourcestart + widesize;
			UTF32* targetstart = reinterpret_cast<UTF32*>(&resultstring[0]);
			UTF32* targetend = targetstart + widesize;
			ConversionResult res = ConvertUTF8toUTF32(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
			if (res != conversionOK)
			{
#ifdef UNIX
				throw std::runtime_error("La falla!");
#else
				throw std::exception("La falla!");
#endif
			}
			*targetstart = 0;
			return resultstring;
		}
		else
		{
#ifdef UNIX
			throw std::runtime_error("La falla!");
#else
			throw std::exception("La falla!");
#endif
		}
		return L"";
	}

	std::string ToUtf8(const std::wstring& widestring)
	{
		size_t widesize = widestring.length();

		if (sizeof(wchar_t) == 2)
		{
			size_t utf8size = 3 * widesize + 1;
			std::string resultstring;
			resultstring.resize(utf8size, '\0');
			const UTF16* sourcestart = reinterpret_cast<const UTF16*>(widestring.c_str());
			const UTF16* sourceend = sourcestart + widesize;
			UTF8* targetstart = reinterpret_cast<UTF8*>(&resultstring[0]);
			UTF8* targetend = targetstart + utf8size;
			ConversionResult res = ConvertUTF16toUTF8(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
			if (res != conversionOK)
			{
#ifdef UNIX
				throw std::runtime_error("La falla!");
#else
				throw std::exception("La falla!");
#endif
			}
			*targetstart = 0;
			return resultstring;
		}
		else if (sizeof(wchar_t) == 4)
		{
			size_t utf8size = 4 * widesize + 1;
			std::string resultstring;
			resultstring.resize(utf8size, '\0');
			const UTF32* sourcestart = reinterpret_cast<const UTF32*>(widestring.c_str());
			const UTF32* sourceend = sourcestart + widesize;
			UTF8* targetstart = reinterpret_cast<UTF8*>(&resultstring[0]);
			UTF8* targetend = targetstart + utf8size;
			ConversionResult res = ConvertUTF32toUTF8(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
			if (res != conversionOK)
			{
#ifdef UNIX
				throw std::runtime_error("La falla!");
#else
				throw std::exception("La falla!");
#endif
			}
			*targetstart = 0;
			return resultstring;
		}
		else
		{
#ifdef UNIX
			throw std::runtime_error("La falla!");
#else
			throw std::exception("La falla!");
#endif
		}
		return "";
	}

    std::string ConvertUtf_8ToString(const std::string &theUtf8Str)
    {
        std::wstring wstr = UtfConverter::FromUtf8(theUtf8Str);
#ifdef UNIX
        // On Linux, convert wstring to UTF-8 string using the ToUtf8 helper
        std::string asciiStr = UtfConverter::ToUtf8(wstr);
#else
        std::string asciiStr = CT2A(CString(wstr.c_str()));
#endif
        return asciiStr;
    }

}
