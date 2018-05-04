#include "wmssupport/stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WmsSupport/GdiplusBitmapParser.h"

#include <fstream>

using namespace testing;
using namespace std;
using namespace Wms;

class BitmapParserParseBitMapFromString : public Test
{
public:
    BitmapParser parser;
    string correctBitmapStringPath = "D:\\projekti\\ver200_SmartMet_release_5_11\\src\\SmartMetWmsSupport\\WmsSupportTest\\data\\bitmap_string.txt";    
    string incorrectBitmapStringPath = "D:\\projekti\\ver200_SmartMet_release_5_11\\src\\SmartMetWmsSupport\\WmsSupportTest\\data\\incorrect_bitmap_string.txt";

    string readBitmapStringFrom(const string& bitmapStringPath)
    {
        ifstream src;
        src.open(bitmapStringPath, ios::binary);
        ostringstream ostrm;
        ostrm << src.rdbuf();
        return ostrm.str();
    }
};

TEST_F(BitmapParserParseBitMapFromString, doesNotThrowExceptionWithCorrectString)
{
    
    EXPECT_NO_THROW(parser.parseBitmapFromString(readBitmapStringFrom(correctBitmapStringPath)));
}

TEST_F(BitmapParserParseBitMapFromString, throwsExceptionWithIncorrectString)
{
    EXPECT_THROW(parser.parseBitmapFromString(readBitmapStringFrom(incorrectBitmapStringPath)), runtime_error);
}