#pragma once
#include "stdafx.h"
#include "BitmapParser.h"
#include <string>
#include <memory>

namespace BitmapHandler
{
    class GdiplusBitmapParser : public BitmapParser
    {
        ULONG_PTR gdiplusToken_;
    public:
        GdiplusBitmapParser();
        ~GdiplusBitmapParser();

        NFmiImageHolder parse(const std::string &bitmapAsString) final;
    };
}

