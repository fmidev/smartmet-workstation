#include "GdiplusBitmapParser.h"

#include <Objidl.h>

using namespace std;

namespace BitmapHandler
{
    namespace
    {
        bool canBeParsed(const std::string& bitmapAsString)
        {
            static const auto pngMagic = std::string("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A");
            const auto& magic = bitmapAsString.substr(0, 8);
            return magic == pngMagic;
        }

        void checkIStreamOpen(HRESULT hr, IStream* iStream)
        {
            if(hr != S_OK || !iStream)
            {
                throw runtime_error("BitmapParser: Opening IStream failed.");
            }
        }

        void checkWrite(HRESULT hr, IStream* iStream)
        {
            if(hr != S_OK)
            {
                iStream->Release();
                throw runtime_error("BitmapParser: Write failed.");
            }
        }

        void checkBitmapCreate(const Gdiplus::Bitmap& bitmap, IStream* iStream)
        {
            if(bitmap.GetLastStatus() != Gdiplus::Status::Ok)
            {
                iStream->Release();
                throw runtime_error("BitmapParser: Bitmap creation failed.");
            }
        }

        shared_ptr<Gdiplus::Bitmap> createBitmapFromIStream(IStream* iStream)
        {
            auto sharedBitmap = make_shared<Gdiplus::Bitmap>(iStream);
            iStream->Release();
            if(sharedBitmap->GetLastStatus() != Gdiplus::Status::Ok)
            {
                throw runtime_error("BitmapParser: Bitmap creation failed.");
            }
            return sharedBitmap;
        }

        void writeStringToIStream(const string &bitmapAsString, IStream* iStream)
        {
            auto hr = iStream->Write(bitmapAsString.data(), ULONG(bitmapAsString.size()), nullptr);
            iStream->Release();
            if(hr != S_OK)
            {
                throw runtime_error("BitmapParser: Write failed.");
            }
        }

        void openIStream(IStream* iStream)
        {
            auto hr = ::CreateStreamOnHGlobal(nullptr, TRUE, &iStream);
            if(hr != S_OK || !iStream)
            {
                throw runtime_error("BitmapParser: Opening IStream failed.");
            }
        }
    }

    GdiplusBitmapParser::GdiplusBitmapParser()
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, nullptr);
    }

    GdiplusBitmapParser::~GdiplusBitmapParser()
    {
        Gdiplus::GdiplusShutdown(gdiplusToken_);
    }

    NFmiImageHolder GdiplusBitmapParser::parse(const string &bitmapAsString)
    {
        if (canBeParsed(bitmapAsString))
        {
            IStream* iStream = nullptr;
            auto hr = ::CreateStreamOnHGlobal(nullptr, TRUE, &iStream);
            checkIStreamOpen(hr, iStream);
            hr = iStream->Write(bitmapAsString.data(), ULONG(bitmapAsString.size()), nullptr);
            checkWrite(hr, iStream);

            auto bitmap = make_shared<Gdiplus::Bitmap>(iStream);
            checkBitmapCreate(*bitmap, iStream);
            auto holder = make_shared<NFmiImageData>();
            holder->mImage = bitmap;
            holder->mState = NFmiImageData::kOk;
            return holder;
        }
        else
        {
            return nullptr;
        }
    }
}