// FmiTMColorIndexDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTMColorIndexDlg.h"
#include "NFmiStringTools.h"
#include "NFmiValueString.h"
#include "NFmiDrawParam.h"
#include "FmiWin32Helpers.h"
#include "FmiModifyDrawParamDlg.h"
#include "NFmiColorSpaces.h"
#include "NFmiDictionaryFunction.h"
#include "CtrlViewWin32Functions.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewFunctions.h"

#include <fstream>
#include <utility>
#include <algorithm>
#include "CloneBitmap.h"


// ##### HUONOA KOODAUSTA #########
// t‰‰ll‰ on m‰‰ritelty dialogin koko m‰‰rityksi‰ globaalein vakioin, se on huomioitava,
// JOS aikoo muuttaa dialogin ulkon‰kˆ‰ ja kokoa!!!!
static const int gTransparentColorIndex = 2;
static const int gColorAreaGapX = 3;
static const int gColorAreaSizeX = 17;
static const int gColorAreaSizeY = 14;
static const int gNewControlsHeigth = 45; // lis‰sin uusia kontrolleja dialogin pohjaan, siit‰ koko t‰h‰n koska se vaikuttaa 3:een muuhun vakioon.
static const int gWindowTotalHeight = 730 + gNewControlsHeigth;
static const int gSelectedColorsAreaDiffFromBottom = 104 + gNewControlsHeigth;
static const int gButtonAreaHeight = 35 + gNewControlsHeigth;
//static const int gColorPaletteBottomY = 550;
static const int gColorTableRedSize = 8; // t‰m‰ on m‰‰ritelty myˆs NFmiIsoLineview.cpp:ss‰, ja n‰iden pit‰‰ olla synkanssa
static const int gColorTableGreenSize = 8; // t‰m‰ on m‰‰ritelty myˆs NFmiIsoLineview.cpp:ss‰, ja n‰iden pit‰‰ olla synkanssa
static const int gColorTableBlueSize = 8; // t‰m‰ on m‰‰ritelty myˆs NFmiIsoLineview.cpp:ss‰, ja n‰iden pit‰‰ olla synkanssa

static std::vector<std::pair<COLORREF, std::string>> gColorNameVector;

BOOL CFmiTMColorIndexDlg::fDoViewUpdatesMemory = true;

static int RgbToColorIndex(Matrix3D<std::pair<int, COLORREF> > &theUsedColorsCube, const NFmiColor& color)
{
	int k = static_cast<int>(std::round(color.Red()*(gColorTableRedSize-1)));
    int j = static_cast<int>(std::round(color.Green()*(gColorTableGreenSize - 1)));
    int i = static_cast<int>(std::round(color.Blue()*(gColorTableBlueSize - 1)));

	return theUsedColorsCube[k][j][i].first;
}

static void InitColorNameMap()
{
    if(gColorNameVector.empty())
    {
        gColorNameVector.push_back(std::make_pair(RGB(176, 23, 31), ::GetDictionaryString("indian red")));
        gColorNameVector.push_back(std::make_pair(RGB(220, 20, 60), ::GetDictionaryString("crimson")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 182, 193), ::GetDictionaryString("lightpink")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 174, 185), ::GetDictionaryString("lightpink 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 162, 173), ::GetDictionaryString("lightpink 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 140, 149), ::GetDictionaryString("lightpink 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 95, 101), ::GetDictionaryString("lightpink 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 192, 203), ::GetDictionaryString("pink")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 181, 197), ::GetDictionaryString("pink 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 169, 184), ::GetDictionaryString("pink 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 145, 158), ::GetDictionaryString("pink 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 99, 108), ::GetDictionaryString("pink 4")));
        gColorNameVector.push_back(std::make_pair(RGB(219, 112, 147), ::GetDictionaryString("palevioletred")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 130, 171), ::GetDictionaryString("palevioletred 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 121, 159), ::GetDictionaryString("palevioletred 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 104, 137), ::GetDictionaryString("palevioletred 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 71, 93), ::GetDictionaryString("palevioletred 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 240, 245), ::GetDictionaryString("lavenderblush 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 224, 229), ::GetDictionaryString("lavenderblush 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 193, 197), ::GetDictionaryString("lavenderblush 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 131, 134), ::GetDictionaryString("lavenderblush 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 62, 150), ::GetDictionaryString("violetred 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 58, 140), ::GetDictionaryString("violetred 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 50, 120), ::GetDictionaryString("violetred 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 34, 82), ::GetDictionaryString("violetred 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 105, 180), ::GetDictionaryString("hotpink")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 110, 180), ::GetDictionaryString("hotpink 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 106, 167), ::GetDictionaryString("hotpink 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 96, 144), ::GetDictionaryString("hotpink 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 58, 98), ::GetDictionaryString("hotpink 4")));
        gColorNameVector.push_back(std::make_pair(RGB(135, 38, 87), ::GetDictionaryString("raspberry")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 20, 147), ::GetDictionaryString("deeppink 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 18, 137), ::GetDictionaryString("deeppink 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 16, 118), ::GetDictionaryString("deeppink 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 10, 80), ::GetDictionaryString("deeppink 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 52, 179), ::GetDictionaryString("maroon 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 48, 167), ::GetDictionaryString("maroon 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 41, 144), ::GetDictionaryString("maroon 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 28, 98), ::GetDictionaryString("maroon 4")));
        gColorNameVector.push_back(std::make_pair(RGB(199, 21, 133), ::GetDictionaryString("mediumvioletred")));
        gColorNameVector.push_back(std::make_pair(RGB(208, 32, 144), ::GetDictionaryString("violetred")));
        gColorNameVector.push_back(std::make_pair(RGB(218, 112, 214), ::GetDictionaryString("orchid")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 131, 250), ::GetDictionaryString("orchid 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 122, 233), ::GetDictionaryString("orchid 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 105, 201), ::GetDictionaryString("orchid 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 71, 137), ::GetDictionaryString("orchid 4")));
        gColorNameVector.push_back(std::make_pair(RGB(216, 191, 216), ::GetDictionaryString("thistle")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 225, 255), ::GetDictionaryString("thistle 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 210, 238), ::GetDictionaryString("thistle 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 181, 205), ::GetDictionaryString("thistle 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 123, 139), ::GetDictionaryString("thistle 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 187, 255), ::GetDictionaryString("plum 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 174, 238), ::GetDictionaryString("plum 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 150, 205), ::GetDictionaryString("plum 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 102, 139), ::GetDictionaryString("plum 4")));
        gColorNameVector.push_back(std::make_pair(RGB(221, 160, 221), ::GetDictionaryString("plum")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 130, 238), ::GetDictionaryString("violet")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 0, 255), ::GetDictionaryString("magenta")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 0, 238), ::GetDictionaryString("magenta 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 0, 205), ::GetDictionaryString("magenta 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 0, 139), ::GetDictionaryString("magenta 4")));
        gColorNameVector.push_back(std::make_pair(RGB(128, 0, 128), ::GetDictionaryString("purple")));
        gColorNameVector.push_back(std::make_pair(RGB(186, 85, 211), ::GetDictionaryString("mediumorchid")));
        gColorNameVector.push_back(std::make_pair(RGB(224, 102, 255), ::GetDictionaryString("mediumorchid 1")));
        gColorNameVector.push_back(std::make_pair(RGB(209, 95, 238), ::GetDictionaryString("mediumorchid 2")));
        gColorNameVector.push_back(std::make_pair(RGB(180, 82, 205), ::GetDictionaryString("mediumorchid 3")));
        gColorNameVector.push_back(std::make_pair(RGB(122, 55, 139), ::GetDictionaryString("mediumorchid 4")));
        gColorNameVector.push_back(std::make_pair(RGB(148, 0, 211), ::GetDictionaryString("darkviolet")));
        gColorNameVector.push_back(std::make_pair(RGB(153, 50, 204), ::GetDictionaryString("darkorchid")));
        gColorNameVector.push_back(std::make_pair(RGB(191, 62, 255), ::GetDictionaryString("darkorchid 1")));
        gColorNameVector.push_back(std::make_pair(RGB(178, 58, 238), ::GetDictionaryString("darkorchid 2")));
        gColorNameVector.push_back(std::make_pair(RGB(154, 50, 205), ::GetDictionaryString("darkorchid 3")));
        gColorNameVector.push_back(std::make_pair(RGB(104, 34, 139), ::GetDictionaryString("darkorchid 4")));
        gColorNameVector.push_back(std::make_pair(RGB(75, 0, 130), ::GetDictionaryString("indigo")));
        gColorNameVector.push_back(std::make_pair(RGB(138, 43, 226), ::GetDictionaryString("blueviolet")));
        gColorNameVector.push_back(std::make_pair(RGB(155, 48, 255), ::GetDictionaryString("purple 1")));
        gColorNameVector.push_back(std::make_pair(RGB(145, 44, 238), ::GetDictionaryString("purple 2")));
        gColorNameVector.push_back(std::make_pair(RGB(125, 38, 205), ::GetDictionaryString("purple 3")));
        gColorNameVector.push_back(std::make_pair(RGB(85, 26, 139), ::GetDictionaryString("purple 4")));
        gColorNameVector.push_back(std::make_pair(RGB(147, 112, 219), ::GetDictionaryString("mediumpurple")));
        gColorNameVector.push_back(std::make_pair(RGB(171, 130, 255), ::GetDictionaryString("mediumpurple 1")));
        gColorNameVector.push_back(std::make_pair(RGB(159, 121, 238), ::GetDictionaryString("mediumpurple 2")));
        gColorNameVector.push_back(std::make_pair(RGB(137, 104, 205), ::GetDictionaryString("mediumpurple 3")));
        gColorNameVector.push_back(std::make_pair(RGB(93, 71, 139), ::GetDictionaryString("mediumpurple 4")));
        gColorNameVector.push_back(std::make_pair(RGB(72, 61, 139), ::GetDictionaryString("darkslateblue")));
        gColorNameVector.push_back(std::make_pair(RGB(132, 112, 255), ::GetDictionaryString("lightslateblue")));
        gColorNameVector.push_back(std::make_pair(RGB(123, 104, 238), ::GetDictionaryString("mediumslateblue")));
        gColorNameVector.push_back(std::make_pair(RGB(106, 90, 205), ::GetDictionaryString("slateblue")));
        gColorNameVector.push_back(std::make_pair(RGB(131, 111, 255), ::GetDictionaryString("slateblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(122, 103, 238), ::GetDictionaryString("slateblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(105, 89, 205), ::GetDictionaryString("slateblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(71, 60, 139), ::GetDictionaryString("slateblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(248, 248, 255), ::GetDictionaryString("ghostwhite")));
        gColorNameVector.push_back(std::make_pair(RGB(230, 230, 250), ::GetDictionaryString("lavender")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 0, 255), ::GetDictionaryString("blue")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 0, 238), ::GetDictionaryString("blue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 0, 205), ::GetDictionaryString("mediumblue")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 0, 139), ::GetDictionaryString("darkblue")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 0, 128), ::GetDictionaryString("navy")));
        gColorNameVector.push_back(std::make_pair(RGB(25, 25, 112), ::GetDictionaryString("midnightblue")));
        gColorNameVector.push_back(std::make_pair(RGB(61, 89, 171), ::GetDictionaryString("cobalt")));
        gColorNameVector.push_back(std::make_pair(RGB(65, 105, 225), ::GetDictionaryString("royalblue")));
        gColorNameVector.push_back(std::make_pair(RGB(72, 118, 255), ::GetDictionaryString("royalblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(67, 110, 238), ::GetDictionaryString("royalblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(58, 95, 205), ::GetDictionaryString("royalblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(39, 64, 139), ::GetDictionaryString("royalblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(100, 149, 237), ::GetDictionaryString("cornflowerblue")));
        gColorNameVector.push_back(std::make_pair(RGB(176, 196, 222), ::GetDictionaryString("lightsteelblue")));
        gColorNameVector.push_back(std::make_pair(RGB(202, 225, 255), ::GetDictionaryString("lightsteelblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(188, 210, 238), ::GetDictionaryString("lightsteelblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(162, 181, 205), ::GetDictionaryString("lightsteelblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(110, 123, 139), ::GetDictionaryString("lightsteelblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(119, 136, 153), ::GetDictionaryString("lightslategray")));
        gColorNameVector.push_back(std::make_pair(RGB(112, 128, 144), ::GetDictionaryString("slategray")));
        gColorNameVector.push_back(std::make_pair(RGB(198, 226, 255), ::GetDictionaryString("slategray 1")));
        gColorNameVector.push_back(std::make_pair(RGB(185, 211, 238), ::GetDictionaryString("slategray 2")));
        gColorNameVector.push_back(std::make_pair(RGB(159, 182, 205), ::GetDictionaryString("slategray 3")));
        gColorNameVector.push_back(std::make_pair(RGB(108, 123, 139), ::GetDictionaryString("slategray 4")));
        gColorNameVector.push_back(std::make_pair(RGB(30, 144, 255), ::GetDictionaryString("dodgerblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(28, 134, 238), ::GetDictionaryString("dodgerblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(24, 116, 205), ::GetDictionaryString("dodgerblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(16, 78, 139), ::GetDictionaryString("dodgerblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(240, 248, 255), ::GetDictionaryString("aliceblue")));
        gColorNameVector.push_back(std::make_pair(RGB(70, 130, 180), ::GetDictionaryString("steelblue")));
        gColorNameVector.push_back(std::make_pair(RGB(99, 184, 255), ::GetDictionaryString("steelblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(92, 172, 238), ::GetDictionaryString("steelblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(79, 148, 205), ::GetDictionaryString("steelblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(54, 100, 139), ::GetDictionaryString("steelblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(135, 206, 250), ::GetDictionaryString("lightskyblue")));
        gColorNameVector.push_back(std::make_pair(RGB(176, 226, 255), ::GetDictionaryString("lightskyblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(164, 211, 238), ::GetDictionaryString("lightskyblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(141, 182, 205), ::GetDictionaryString("lightskyblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(96, 123, 139), ::GetDictionaryString("lightskyblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(135, 206, 255), ::GetDictionaryString("skyblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(126, 192, 238), ::GetDictionaryString("skyblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(108, 166, 205), ::GetDictionaryString("skyblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(74, 112, 139), ::GetDictionaryString("skyblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(135, 206, 235), ::GetDictionaryString("skyblue")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 191, 255), ::GetDictionaryString("deepskyblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 178, 238), ::GetDictionaryString("deepskyblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 154, 205), ::GetDictionaryString("deepskyblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 104, 139), ::GetDictionaryString("deepskyblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(51, 161, 201), ::GetDictionaryString("peacock")));
        gColorNameVector.push_back(std::make_pair(RGB(173, 216, 230), ::GetDictionaryString("lightblue")));
        gColorNameVector.push_back(std::make_pair(RGB(191, 239, 255), ::GetDictionaryString("lightblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(178, 223, 238), ::GetDictionaryString("lightblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(154, 192, 205), ::GetDictionaryString("lightblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(104, 131, 139), ::GetDictionaryString("lightblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(176, 224, 230), ::GetDictionaryString("powderblue")));
        gColorNameVector.push_back(std::make_pair(RGB(152, 245, 255), ::GetDictionaryString("cadetblue 1")));
        gColorNameVector.push_back(std::make_pair(RGB(142, 229, 238), ::GetDictionaryString("cadetblue 2")));
        gColorNameVector.push_back(std::make_pair(RGB(122, 197, 205), ::GetDictionaryString("cadetblue 3")));
        gColorNameVector.push_back(std::make_pair(RGB(83, 134, 139), ::GetDictionaryString("cadetblue 4")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 245, 255), ::GetDictionaryString("turquoise 1")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 229, 238), ::GetDictionaryString("turquoise 2")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 197, 205), ::GetDictionaryString("turquoise 3")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 134, 139), ::GetDictionaryString("turquoise 4")));
        gColorNameVector.push_back(std::make_pair(RGB(95, 158, 160), ::GetDictionaryString("cadetblue")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 206, 209), ::GetDictionaryString("darkturquoise")));
        gColorNameVector.push_back(std::make_pair(RGB(240, 255, 255), ::GetDictionaryString("azure 1")));
        gColorNameVector.push_back(std::make_pair(RGB(224, 238, 238), ::GetDictionaryString("azure 2")));
        gColorNameVector.push_back(std::make_pair(RGB(193, 205, 205), ::GetDictionaryString("azure 3")));
        gColorNameVector.push_back(std::make_pair(RGB(131, 139, 139), ::GetDictionaryString("azure 4")));
        gColorNameVector.push_back(std::make_pair(RGB(224, 255, 255), ::GetDictionaryString("lightcyan 1")));
        gColorNameVector.push_back(std::make_pair(RGB(209, 238, 238), ::GetDictionaryString("lightcyan 2")));
        gColorNameVector.push_back(std::make_pair(RGB(180, 205, 205), ::GetDictionaryString("lightcyan 3")));
        gColorNameVector.push_back(std::make_pair(RGB(122, 139, 139), ::GetDictionaryString("lightcyan 4")));
        gColorNameVector.push_back(std::make_pair(RGB(187, 255, 255), ::GetDictionaryString("paleturquoise 1")));
        gColorNameVector.push_back(std::make_pair(RGB(174, 238, 238), ::GetDictionaryString("paleturquoise 2")));
        gColorNameVector.push_back(std::make_pair(RGB(150, 205, 205), ::GetDictionaryString("paleturquoise 3")));
        gColorNameVector.push_back(std::make_pair(RGB(102, 139, 139), ::GetDictionaryString("paleturquoise 4")));
        gColorNameVector.push_back(std::make_pair(RGB(47, 79, 79), ::GetDictionaryString("darkslategray")));
        gColorNameVector.push_back(std::make_pair(RGB(151, 255, 255), ::GetDictionaryString("darkslategray 1")));
        gColorNameVector.push_back(std::make_pair(RGB(141, 238, 238), ::GetDictionaryString("darkslategray 2")));
        gColorNameVector.push_back(std::make_pair(RGB(121, 205, 205), ::GetDictionaryString("darkslategray 3")));
        gColorNameVector.push_back(std::make_pair(RGB(82, 139, 139), ::GetDictionaryString("darkslategray 4")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 255, 255), ::GetDictionaryString("cyan")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 238, 238), ::GetDictionaryString("cyan 2")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 205, 205), ::GetDictionaryString("cyan 3")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 139, 139), ::GetDictionaryString("darkcyan")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 128, 128), ::GetDictionaryString("teal")));
        gColorNameVector.push_back(std::make_pair(RGB(72, 209, 204), ::GetDictionaryString("mediumturquoise")));
        gColorNameVector.push_back(std::make_pair(RGB(32, 178, 170), ::GetDictionaryString("lightseagreen")));
        gColorNameVector.push_back(std::make_pair(RGB(3, 168, 158), ::GetDictionaryString("manganeseblue")));
        gColorNameVector.push_back(std::make_pair(RGB(64, 224, 208), ::GetDictionaryString("turquoise")));
        gColorNameVector.push_back(std::make_pair(RGB(128, 138, 135), ::GetDictionaryString("coldgrey")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 199, 140), ::GetDictionaryString("turquoiseblue")));
        gColorNameVector.push_back(std::make_pair(RGB(127, 255, 212), ::GetDictionaryString("aquamarine 1")));
        gColorNameVector.push_back(std::make_pair(RGB(118, 238, 198), ::GetDictionaryString("aquamarine 2")));
        gColorNameVector.push_back(std::make_pair(RGB(102, 205, 170), ::GetDictionaryString("mediumaquamarine")));
        gColorNameVector.push_back(std::make_pair(RGB(69, 139, 116), ::GetDictionaryString("aquamarine 4")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 250, 154), ::GetDictionaryString("mediumspringgreen")));
        gColorNameVector.push_back(std::make_pair(RGB(245, 255, 250), ::GetDictionaryString("mintcream")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 255, 127), ::GetDictionaryString("springgreen")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 238, 118), ::GetDictionaryString("springgreen 1")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 205, 102), ::GetDictionaryString("springgreen 2")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 139, 69), ::GetDictionaryString("springgreen 3")));
        gColorNameVector.push_back(std::make_pair(RGB(60, 179, 113), ::GetDictionaryString("mediumseagreen")));
        gColorNameVector.push_back(std::make_pair(RGB(84, 255, 159), ::GetDictionaryString("seagreen 1")));
        gColorNameVector.push_back(std::make_pair(RGB(78, 238, 148), ::GetDictionaryString("seagreen 2")));
        gColorNameVector.push_back(std::make_pair(RGB(67, 205, 128), ::GetDictionaryString("seagreen 3")));
        gColorNameVector.push_back(std::make_pair(RGB(46, 139, 87), ::GetDictionaryString("seagreen 4")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 201, 87), ::GetDictionaryString("emeraldgreen")));
        gColorNameVector.push_back(std::make_pair(RGB(189, 252, 201), ::GetDictionaryString("mint")));
        gColorNameVector.push_back(std::make_pair(RGB(61, 145, 64), ::GetDictionaryString("cobaltgreen")));
        gColorNameVector.push_back(std::make_pair(RGB(240, 255, 240), ::GetDictionaryString("honeydew 1")));
        gColorNameVector.push_back(std::make_pair(RGB(224, 238, 224), ::GetDictionaryString("honeydew 2")));
        gColorNameVector.push_back(std::make_pair(RGB(193, 205, 193), ::GetDictionaryString("honeydew 3")));
        gColorNameVector.push_back(std::make_pair(RGB(131, 139, 131), ::GetDictionaryString("honeydew 4")));
        gColorNameVector.push_back(std::make_pair(RGB(143, 188, 143), ::GetDictionaryString("darkseagreen")));
        gColorNameVector.push_back(std::make_pair(RGB(193, 255, 193), ::GetDictionaryString("darkseagreen 1")));
        gColorNameVector.push_back(std::make_pair(RGB(180, 238, 180), ::GetDictionaryString("darkseagreen 2")));
        gColorNameVector.push_back(std::make_pair(RGB(155, 205, 155), ::GetDictionaryString("darkseagreen 3")));
        gColorNameVector.push_back(std::make_pair(RGB(105, 139, 105), ::GetDictionaryString("darkseagreen 4")));
        gColorNameVector.push_back(std::make_pair(RGB(152, 251, 152), ::GetDictionaryString("palegreen")));
        gColorNameVector.push_back(std::make_pair(RGB(154, 255, 154), ::GetDictionaryString("palegreen 1")));
        gColorNameVector.push_back(std::make_pair(RGB(144, 238, 144), ::GetDictionaryString("lightgreen")));
        gColorNameVector.push_back(std::make_pair(RGB(124, 205, 124), ::GetDictionaryString("palegreen 3")));
        gColorNameVector.push_back(std::make_pair(RGB(84, 139, 84), ::GetDictionaryString("palegreen 4")));
        gColorNameVector.push_back(std::make_pair(RGB(50, 205, 50), ::GetDictionaryString("limegreen")));
        gColorNameVector.push_back(std::make_pair(RGB(34, 139, 34), ::GetDictionaryString("forestgreen")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 255, 0), ::GetDictionaryString("lime")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 238, 0), ::GetDictionaryString("green 2")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 205, 0), ::GetDictionaryString("green 3")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 139, 0), ::GetDictionaryString("green 4")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 128, 0), ::GetDictionaryString("green")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 100, 0), ::GetDictionaryString("darkgreen")));
        gColorNameVector.push_back(std::make_pair(RGB(48, 128, 20), ::GetDictionaryString("sapgreen")));
        gColorNameVector.push_back(std::make_pair(RGB(124, 252, 0), ::GetDictionaryString("lawngreen")));
        gColorNameVector.push_back(std::make_pair(RGB(127, 255, 0), ::GetDictionaryString("chartreuse 1")));
        gColorNameVector.push_back(std::make_pair(RGB(118, 238, 0), ::GetDictionaryString("chartreuse 2")));
        gColorNameVector.push_back(std::make_pair(RGB(102, 205, 0), ::GetDictionaryString("chartreuse 3")));
        gColorNameVector.push_back(std::make_pair(RGB(69, 139, 0), ::GetDictionaryString("chartreuse 4")));
        gColorNameVector.push_back(std::make_pair(RGB(173, 255, 47), ::GetDictionaryString("greenyellow")));
        gColorNameVector.push_back(std::make_pair(RGB(202, 255, 112), ::GetDictionaryString("darkolivegreen 1")));
        gColorNameVector.push_back(std::make_pair(RGB(188, 238, 104), ::GetDictionaryString("darkolivegreen 2")));
        gColorNameVector.push_back(std::make_pair(RGB(162, 205, 90), ::GetDictionaryString("darkolivegreen 3")));
        gColorNameVector.push_back(std::make_pair(RGB(110, 139, 61), ::GetDictionaryString("darkolivegreen 4")));
        gColorNameVector.push_back(std::make_pair(RGB(85, 107, 47), ::GetDictionaryString("darkolivegreen")));
        gColorNameVector.push_back(std::make_pair(RGB(107, 142, 35), ::GetDictionaryString("olivedrab")));
        gColorNameVector.push_back(std::make_pair(RGB(192, 255, 62), ::GetDictionaryString("olivedrab 1")));
        gColorNameVector.push_back(std::make_pair(RGB(179, 238, 58), ::GetDictionaryString("olivedrab 2")));
        gColorNameVector.push_back(std::make_pair(RGB(154, 205, 50), ::GetDictionaryString("yellowgreen")));
        gColorNameVector.push_back(std::make_pair(RGB(105, 139, 34), ::GetDictionaryString("olivedrab 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 255, 240), ::GetDictionaryString("ivory")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 238, 224), ::GetDictionaryString("ivory 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 205, 193), ::GetDictionaryString("ivory 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 139, 131), ::GetDictionaryString("ivory 4")));
        gColorNameVector.push_back(std::make_pair(RGB(245, 245, 220), ::GetDictionaryString("beige")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 255, 224), ::GetDictionaryString("lightyellow")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 238, 209), ::GetDictionaryString("lightyellow 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 205, 180), ::GetDictionaryString("lightyellow 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 139, 122), ::GetDictionaryString("lightyellow 4")));
        gColorNameVector.push_back(std::make_pair(RGB(250, 250, 210), ::GetDictionaryString("lightgoldenrodyellow")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 255, 0), ::GetDictionaryString("yellow")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 238, 0), ::GetDictionaryString("yellow 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 205, 0), ::GetDictionaryString("yellow 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 139, 0), ::GetDictionaryString("yellow 4")));
        gColorNameVector.push_back(std::make_pair(RGB(128, 128, 105), ::GetDictionaryString("warmgrey")));
        gColorNameVector.push_back(std::make_pair(RGB(128, 128, 0), ::GetDictionaryString("olive")));
        gColorNameVector.push_back(std::make_pair(RGB(189, 183, 107), ::GetDictionaryString("darkkhaki")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 246, 143), ::GetDictionaryString("khaki 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 230, 133), ::GetDictionaryString("khaki 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 198, 115), ::GetDictionaryString("khaki 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 134, 78), ::GetDictionaryString("khaki 4")));
        gColorNameVector.push_back(std::make_pair(RGB(240, 230, 140), ::GetDictionaryString("khaki")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 232, 170), ::GetDictionaryString("palegoldenrod")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 250, 205), ::GetDictionaryString("lemonchiffon")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 233, 191), ::GetDictionaryString("lemonchiffon 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 201, 165), ::GetDictionaryString("lemonchiffon 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 137, 112), ::GetDictionaryString("lemonchiffon 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 236, 139), ::GetDictionaryString("lightgoldenrod 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 220, 130), ::GetDictionaryString("lightgoldenrod 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 190, 112), ::GetDictionaryString("lightgoldenrod 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 129, 76), ::GetDictionaryString("lightgoldenrod 4")));
        gColorNameVector.push_back(std::make_pair(RGB(227, 207, 87), ::GetDictionaryString("banana")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 215, 0), ::GetDictionaryString("gold")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 201, 0), ::GetDictionaryString("gold 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 173, 0), ::GetDictionaryString("gold 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 117, 0), ::GetDictionaryString("gold 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 248, 220), ::GetDictionaryString("cornsilk")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 232, 205), ::GetDictionaryString("cornsilk 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 200, 177), ::GetDictionaryString("cornsilk 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 136, 120), ::GetDictionaryString("cornsilk 4")));
        gColorNameVector.push_back(std::make_pair(RGB(218, 165, 32), ::GetDictionaryString("goldenrod")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 193, 37), ::GetDictionaryString("goldenrod 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 180, 34), ::GetDictionaryString("goldenrod 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 155, 29), ::GetDictionaryString("goldenrod 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 105, 20), ::GetDictionaryString("goldenrod 4")));
        gColorNameVector.push_back(std::make_pair(RGB(184, 134, 11), ::GetDictionaryString("darkgoldenrod")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 185, 15), ::GetDictionaryString("darkgoldenrod 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 173, 14), ::GetDictionaryString("darkgoldenrod 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 149, 12), ::GetDictionaryString("darkgoldenrod 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 101, 8), ::GetDictionaryString("darkgoldenrod 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 165, 0), ::GetDictionaryString("orange 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 154, 0), ::GetDictionaryString("orange 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 133, 0), ::GetDictionaryString("orange 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 90, 0), ::GetDictionaryString("orange 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 250, 240), ::GetDictionaryString("floralwhite")));
        gColorNameVector.push_back(std::make_pair(RGB(253, 245, 230), ::GetDictionaryString("oldlace")));
        gColorNameVector.push_back(std::make_pair(RGB(245, 222, 179), ::GetDictionaryString("wheat")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 231, 186), ::GetDictionaryString("wheat 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 216, 174), ::GetDictionaryString("wheat 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 186, 150), ::GetDictionaryString("wheat 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 126, 102), ::GetDictionaryString("wheat 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 228, 181), ::GetDictionaryString("moccasin")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 239, 213), ::GetDictionaryString("papayawhip")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 235, 205), ::GetDictionaryString("blanchedalmond")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 222, 173), ::GetDictionaryString("navajowhite")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 207, 161), ::GetDictionaryString("navajowhite 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 179, 139), ::GetDictionaryString("navajowhite 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 121, 94), ::GetDictionaryString("navajowhite 4")));
        gColorNameVector.push_back(std::make_pair(RGB(252, 230, 201), ::GetDictionaryString("eggshell")));
        gColorNameVector.push_back(std::make_pair(RGB(210, 180, 140), ::GetDictionaryString("tan")));
        gColorNameVector.push_back(std::make_pair(RGB(156, 102, 31), ::GetDictionaryString("brick")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 153, 18), ::GetDictionaryString("cadmiumyellow")));
        gColorNameVector.push_back(std::make_pair(RGB(250, 235, 215), ::GetDictionaryString("antiquewhite")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 239, 219), ::GetDictionaryString("antiquewhite 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 223, 204), ::GetDictionaryString("antiquewhite 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 192, 176), ::GetDictionaryString("antiquewhite 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 131, 120), ::GetDictionaryString("antiquewhite 4")));
        gColorNameVector.push_back(std::make_pair(RGB(222, 184, 135), ::GetDictionaryString("burlywood")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 211, 155), ::GetDictionaryString("burlywood 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 197, 145), ::GetDictionaryString("burlywood 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 170, 125), ::GetDictionaryString("burlywood 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 115, 85), ::GetDictionaryString("burlywood 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 228, 196), ::GetDictionaryString("bisque")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 213, 183), ::GetDictionaryString("bisque 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 183, 158), ::GetDictionaryString("bisque 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 125, 107), ::GetDictionaryString("bisque 4")));
        gColorNameVector.push_back(std::make_pair(RGB(227, 168, 105), ::GetDictionaryString("melon")));
        gColorNameVector.push_back(std::make_pair(RGB(237, 145, 33), ::GetDictionaryString("carrot")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 140, 0), ::GetDictionaryString("darkorange")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 127, 0), ::GetDictionaryString("darkorange 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 118, 0), ::GetDictionaryString("darkorange 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 102, 0), ::GetDictionaryString("darkorange 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 69, 0), ::GetDictionaryString("darkorange 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 128, 0), ::GetDictionaryString("orange")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 165, 79), ::GetDictionaryString("tan 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 154, 73), ::GetDictionaryString("tan 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 133, 63), ::GetDictionaryString("tan 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 90, 43), ::GetDictionaryString("tan 4")));
        gColorNameVector.push_back(std::make_pair(RGB(250, 240, 230), ::GetDictionaryString("linen")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 218, 185), ::GetDictionaryString("peachpuff 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 203, 173), ::GetDictionaryString("peachpuff 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 175, 149), ::GetDictionaryString("peachpuff 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 119, 101), ::GetDictionaryString("peachpuff 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 245, 238), ::GetDictionaryString("seashell 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 229, 222), ::GetDictionaryString("seashell 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 197, 191), ::GetDictionaryString("seashell 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 134, 130), ::GetDictionaryString("seashell 4")));
        gColorNameVector.push_back(std::make_pair(RGB(244, 164, 96), ::GetDictionaryString("sandybrown")));
        gColorNameVector.push_back(std::make_pair(RGB(199, 97, 20), ::GetDictionaryString("rawsienna")));
        gColorNameVector.push_back(std::make_pair(RGB(210, 105, 30), ::GetDictionaryString("chocolate")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 127, 36), ::GetDictionaryString("chocolate 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 118, 33), ::GetDictionaryString("chocolate 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 102, 29), ::GetDictionaryString("chocolate 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 69, 19), ::GetDictionaryString("saddlebrown")));
        gColorNameVector.push_back(std::make_pair(RGB(41, 36, 33), ::GetDictionaryString("ivoryblack")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 125, 64), ::GetDictionaryString("flesh")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 97, 3), ::GetDictionaryString("cadmiumorange")));
        gColorNameVector.push_back(std::make_pair(RGB(138, 54, 15), ::GetDictionaryString("burntsienna")));
        gColorNameVector.push_back(std::make_pair(RGB(160, 82, 45), ::GetDictionaryString("sienna")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 130, 71), ::GetDictionaryString("sienna 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 121, 66), ::GetDictionaryString("sienna 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 104, 57), ::GetDictionaryString("sienna 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 71, 38), ::GetDictionaryString("sienna 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 160, 122), ::GetDictionaryString("lightsalmon")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 149, 114), ::GetDictionaryString("lightsalmon 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 129, 98), ::GetDictionaryString("lightsalmon 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 87, 66), ::GetDictionaryString("lightsalmon 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 127, 80), ::GetDictionaryString("coral")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 69, 0), ::GetDictionaryString("orangered")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 64, 0), ::GetDictionaryString("orangered 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 55, 0), ::GetDictionaryString("orangered 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 37, 0), ::GetDictionaryString("orangered 4")));
        gColorNameVector.push_back(std::make_pair(RGB(94, 38, 18), ::GetDictionaryString("sepia")));
        gColorNameVector.push_back(std::make_pair(RGB(233, 150, 122), ::GetDictionaryString("darksalmon")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 140, 105), ::GetDictionaryString("salmon 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 130, 98), ::GetDictionaryString("salmon 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 112, 84), ::GetDictionaryString("salmon 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 76, 57), ::GetDictionaryString("salmon 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 114, 86), ::GetDictionaryString("coral 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 106, 80), ::GetDictionaryString("coral 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 91, 69), ::GetDictionaryString("coral 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 62, 47), ::GetDictionaryString("coral 4")));
        gColorNameVector.push_back(std::make_pair(RGB(138, 51, 36), ::GetDictionaryString("burntumber")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 99, 71), ::GetDictionaryString("tomato")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 92, 66), ::GetDictionaryString("tomato 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 79, 57), ::GetDictionaryString("tomato 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 54, 38), ::GetDictionaryString("tomato 4")));
        gColorNameVector.push_back(std::make_pair(RGB(250, 128, 114), ::GetDictionaryString("salmon")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 228, 225), ::GetDictionaryString("mistyrose")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 213, 210), ::GetDictionaryString("mistyrose 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 183, 181), ::GetDictionaryString("mistyrose 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 125, 123), ::GetDictionaryString("mistyrose 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 250, 250), ::GetDictionaryString("snow")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 233, 233), ::GetDictionaryString("snow 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 201, 201), ::GetDictionaryString("snow 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 137, 137), ::GetDictionaryString("snow 4")));
        gColorNameVector.push_back(std::make_pair(RGB(188, 143, 143), ::GetDictionaryString("rosybrown")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 193, 193), ::GetDictionaryString("rosybrown 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 180, 180), ::GetDictionaryString("rosybrown 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 155, 155), ::GetDictionaryString("rosybrown 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 105, 105), ::GetDictionaryString("rosybrown 4")));
        gColorNameVector.push_back(std::make_pair(RGB(240, 128, 128), ::GetDictionaryString("lightcoral")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 92, 92), ::GetDictionaryString("indianred")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 106, 106), ::GetDictionaryString("indianred 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 99, 99), ::GetDictionaryString("indianred 2")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 58, 58), ::GetDictionaryString("indianred 4")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 85, 85), ::GetDictionaryString("indianred 3")));
        gColorNameVector.push_back(std::make_pair(RGB(165, 42, 42), ::GetDictionaryString("brown")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 64, 64), ::GetDictionaryString("brown 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 59, 59), ::GetDictionaryString("brown 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 51, 51), ::GetDictionaryString("brown 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 35, 35), ::GetDictionaryString("brown 4")));
        gColorNameVector.push_back(std::make_pair(RGB(178, 34, 34), ::GetDictionaryString("firebrick")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 48, 48), ::GetDictionaryString("firebrick 1")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 44, 44), ::GetDictionaryString("firebrick 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 38, 38), ::GetDictionaryString("firebrick 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 26, 26), ::GetDictionaryString("firebrick 4")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 0, 0), ::GetDictionaryString("red")));
        gColorNameVector.push_back(std::make_pair(RGB(238, 0, 0), ::GetDictionaryString("red 2")));
        gColorNameVector.push_back(std::make_pair(RGB(205, 0, 0), ::GetDictionaryString("red 3")));
        gColorNameVector.push_back(std::make_pair(RGB(139, 0, 0), ::GetDictionaryString("darkred")));
        gColorNameVector.push_back(std::make_pair(RGB(128, 0, 0), ::GetDictionaryString("maroon")));
        gColorNameVector.push_back(std::make_pair(RGB(142, 56, 142), ::GetDictionaryString("sgi beet")));
        gColorNameVector.push_back(std::make_pair(RGB(113, 113, 198), ::GetDictionaryString("sgi slateblue")));
        gColorNameVector.push_back(std::make_pair(RGB(125, 158, 192), ::GetDictionaryString("sgi lightblue")));
        gColorNameVector.push_back(std::make_pair(RGB(56, 142, 142), ::GetDictionaryString("sgi teal")));
        gColorNameVector.push_back(std::make_pair(RGB(113, 198, 113), ::GetDictionaryString("sgi chartreuse")));
        gColorNameVector.push_back(std::make_pair(RGB(142, 142, 56), ::GetDictionaryString("sgi olivedrab")));
        gColorNameVector.push_back(std::make_pair(RGB(197, 193, 170), ::GetDictionaryString("sgi brightgray")));
        gColorNameVector.push_back(std::make_pair(RGB(198, 113, 113), ::GetDictionaryString("sgi salmon")));
        gColorNameVector.push_back(std::make_pair(RGB(85, 85, 85), ::GetDictionaryString("sgi darkgray")));
        gColorNameVector.push_back(std::make_pair(RGB(30, 30, 30), ::GetDictionaryString("sgi gray 12")));
        gColorNameVector.push_back(std::make_pair(RGB(40, 40, 40), ::GetDictionaryString("sgi gray 16")));
        gColorNameVector.push_back(std::make_pair(RGB(81, 81, 81), ::GetDictionaryString("sgi gray 32")));
        gColorNameVector.push_back(std::make_pair(RGB(91, 91, 91), ::GetDictionaryString("sgi gray 36")));
        gColorNameVector.push_back(std::make_pair(RGB(132, 132, 132), ::GetDictionaryString("sgi gray 52")));
        gColorNameVector.push_back(std::make_pair(RGB(142, 142, 142), ::GetDictionaryString("sgi gray 56")));
        gColorNameVector.push_back(std::make_pair(RGB(170, 170, 170), ::GetDictionaryString("sgi lightgray")));
        gColorNameVector.push_back(std::make_pair(RGB(183, 183, 183), ::GetDictionaryString("sgi gray 72")));
        gColorNameVector.push_back(std::make_pair(RGB(193, 193, 193), ::GetDictionaryString("sgi gray 76")));
        gColorNameVector.push_back(std::make_pair(RGB(234, 234, 234), ::GetDictionaryString("sgi gray 92")));
        gColorNameVector.push_back(std::make_pair(RGB(244, 244, 244), ::GetDictionaryString("sgi gray 96")));
        gColorNameVector.push_back(std::make_pair(RGB(255, 255, 255), ::GetDictionaryString("white")));
        gColorNameVector.push_back(std::make_pair(RGB(245, 245, 245), ::GetDictionaryString("gray 96")));
        gColorNameVector.push_back(std::make_pair(RGB(220, 220, 220), ::GetDictionaryString("gainsboro")));
        gColorNameVector.push_back(std::make_pair(RGB(211, 211, 211), ::GetDictionaryString("lightgrey")));
        gColorNameVector.push_back(std::make_pair(RGB(192, 192, 192), ::GetDictionaryString("silver")));
        gColorNameVector.push_back(std::make_pair(RGB(169, 169, 169), ::GetDictionaryString("darkgray")));
        gColorNameVector.push_back(std::make_pair(RGB(128, 128, 128), ::GetDictionaryString("gray")));
        gColorNameVector.push_back(std::make_pair(RGB(105, 105, 105), ::GetDictionaryString("dimgray")));
        gColorNameVector.push_back(std::make_pair(RGB(0, 0, 0), ::GetDictionaryString("black")));
        gColorNameVector.push_back(std::make_pair(RGB(252, 252, 252), ::GetDictionaryString("gray 99")));
        gColorNameVector.push_back(std::make_pair(RGB(250, 250, 250), ::GetDictionaryString("gray 98")));
        gColorNameVector.push_back(std::make_pair(RGB(247, 247, 247), ::GetDictionaryString("gray 97")));
        gColorNameVector.push_back(std::make_pair(RGB(245, 245, 245), ::GetDictionaryString("gray 96")));
        gColorNameVector.push_back(std::make_pair(RGB(242, 242, 242), ::GetDictionaryString("gray 95")));
        gColorNameVector.push_back(std::make_pair(RGB(240, 240, 240), ::GetDictionaryString("gray 94")));
        gColorNameVector.push_back(std::make_pair(RGB(237, 237, 237), ::GetDictionaryString("gray 93")));
        gColorNameVector.push_back(std::make_pair(RGB(235, 235, 235), ::GetDictionaryString("gray 92")));
        gColorNameVector.push_back(std::make_pair(RGB(232, 232, 232), ::GetDictionaryString("gray 91")));
        gColorNameVector.push_back(std::make_pair(RGB(229, 229, 229), ::GetDictionaryString("gray 90")));
        gColorNameVector.push_back(std::make_pair(RGB(227, 227, 227), ::GetDictionaryString("gray 89")));
        gColorNameVector.push_back(std::make_pair(RGB(224, 224, 224), ::GetDictionaryString("gray 88")));
        gColorNameVector.push_back(std::make_pair(RGB(222, 222, 222), ::GetDictionaryString("gray 87")));
        gColorNameVector.push_back(std::make_pair(RGB(219, 219, 219), ::GetDictionaryString("gray 86")));
        gColorNameVector.push_back(std::make_pair(RGB(217, 217, 217), ::GetDictionaryString("gray 85")));
        gColorNameVector.push_back(std::make_pair(RGB(214, 214, 214), ::GetDictionaryString("gray 84")));
        gColorNameVector.push_back(std::make_pair(RGB(212, 212, 212), ::GetDictionaryString("gray 83")));
        gColorNameVector.push_back(std::make_pair(RGB(209, 209, 209), ::GetDictionaryString("gray 82")));
        gColorNameVector.push_back(std::make_pair(RGB(207, 207, 207), ::GetDictionaryString("gray 81")));
        gColorNameVector.push_back(std::make_pair(RGB(204, 204, 204), ::GetDictionaryString("gray 80")));
        gColorNameVector.push_back(std::make_pair(RGB(201, 201, 201), ::GetDictionaryString("gray 79")));
        gColorNameVector.push_back(std::make_pair(RGB(199, 199, 199), ::GetDictionaryString("gray 78")));
        gColorNameVector.push_back(std::make_pair(RGB(196, 196, 196), ::GetDictionaryString("gray 77")));
        gColorNameVector.push_back(std::make_pair(RGB(194, 194, 194), ::GetDictionaryString("gray 76")));
        gColorNameVector.push_back(std::make_pair(RGB(191, 191, 191), ::GetDictionaryString("gray 75")));
        gColorNameVector.push_back(std::make_pair(RGB(189, 189, 189), ::GetDictionaryString("gray 74")));
        gColorNameVector.push_back(std::make_pair(RGB(186, 186, 186), ::GetDictionaryString("gray 73")));
        gColorNameVector.push_back(std::make_pair(RGB(184, 184, 184), ::GetDictionaryString("gray 72")));
        gColorNameVector.push_back(std::make_pair(RGB(181, 181, 181), ::GetDictionaryString("gray 71")));
        gColorNameVector.push_back(std::make_pair(RGB(179, 179, 179), ::GetDictionaryString("gray 70")));
        gColorNameVector.push_back(std::make_pair(RGB(176, 176, 176), ::GetDictionaryString("gray 69")));
        gColorNameVector.push_back(std::make_pair(RGB(173, 173, 173), ::GetDictionaryString("gray 68")));
        gColorNameVector.push_back(std::make_pair(RGB(171, 171, 171), ::GetDictionaryString("gray 67")));
        gColorNameVector.push_back(std::make_pair(RGB(168, 168, 168), ::GetDictionaryString("gray 66")));
        gColorNameVector.push_back(std::make_pair(RGB(166, 166, 166), ::GetDictionaryString("gray 65")));
        gColorNameVector.push_back(std::make_pair(RGB(163, 163, 163), ::GetDictionaryString("gray 64")));
        gColorNameVector.push_back(std::make_pair(RGB(161, 161, 161), ::GetDictionaryString("gray 63")));
        gColorNameVector.push_back(std::make_pair(RGB(158, 158, 158), ::GetDictionaryString("gray 62")));
        gColorNameVector.push_back(std::make_pair(RGB(156, 156, 156), ::GetDictionaryString("gray 61")));
        gColorNameVector.push_back(std::make_pair(RGB(153, 153, 153), ::GetDictionaryString("gray 60")));
        gColorNameVector.push_back(std::make_pair(RGB(150, 150, 150), ::GetDictionaryString("gray 59")));
        gColorNameVector.push_back(std::make_pair(RGB(148, 148, 148), ::GetDictionaryString("gray 58")));
        gColorNameVector.push_back(std::make_pair(RGB(145, 145, 145), ::GetDictionaryString("gray 57")));
        gColorNameVector.push_back(std::make_pair(RGB(143, 143, 143), ::GetDictionaryString("gray 56")));
        gColorNameVector.push_back(std::make_pair(RGB(140, 140, 140), ::GetDictionaryString("gray 55")));
        gColorNameVector.push_back(std::make_pair(RGB(138, 138, 138), ::GetDictionaryString("gray 54")));
        gColorNameVector.push_back(std::make_pair(RGB(135, 135, 135), ::GetDictionaryString("gray 53")));
        gColorNameVector.push_back(std::make_pair(RGB(133, 133, 133), ::GetDictionaryString("gray 52")));
        gColorNameVector.push_back(std::make_pair(RGB(130, 130, 130), ::GetDictionaryString("gray 51")));
        gColorNameVector.push_back(std::make_pair(RGB(127, 127, 127), ::GetDictionaryString("gray 50")));
        gColorNameVector.push_back(std::make_pair(RGB(125, 125, 125), ::GetDictionaryString("gray 49")));
        gColorNameVector.push_back(std::make_pair(RGB(122, 122, 122), ::GetDictionaryString("gray 48")));
        gColorNameVector.push_back(std::make_pair(RGB(120, 120, 120), ::GetDictionaryString("gray 47")));
        gColorNameVector.push_back(std::make_pair(RGB(117, 117, 117), ::GetDictionaryString("gray 46")));
        gColorNameVector.push_back(std::make_pair(RGB(115, 115, 115), ::GetDictionaryString("gray 45")));
        gColorNameVector.push_back(std::make_pair(RGB(112, 112, 112), ::GetDictionaryString("gray 44")));
        gColorNameVector.push_back(std::make_pair(RGB(110, 110, 110), ::GetDictionaryString("gray 43")));
        gColorNameVector.push_back(std::make_pair(RGB(107, 107, 107), ::GetDictionaryString("gray 42")));
        gColorNameVector.push_back(std::make_pair(RGB(105, 105, 105), ::GetDictionaryString("dimgray")));
        gColorNameVector.push_back(std::make_pair(RGB(102, 102, 102), ::GetDictionaryString("gray 40")));
        gColorNameVector.push_back(std::make_pair(RGB(99, 99, 99), ::GetDictionaryString("gray 39")));
        gColorNameVector.push_back(std::make_pair(RGB(97, 97, 97), ::GetDictionaryString("gray 38")));
        gColorNameVector.push_back(std::make_pair(RGB(94, 94, 94), ::GetDictionaryString("gray 37")));
        gColorNameVector.push_back(std::make_pair(RGB(92, 92, 92), ::GetDictionaryString("gray 36")));
        gColorNameVector.push_back(std::make_pair(RGB(89, 89, 89), ::GetDictionaryString("gray 35")));
        gColorNameVector.push_back(std::make_pair(RGB(87, 87, 87), ::GetDictionaryString("gray 34")));
        gColorNameVector.push_back(std::make_pair(RGB(84, 84, 84), ::GetDictionaryString("gray 33")));
        gColorNameVector.push_back(std::make_pair(RGB(82, 82, 82), ::GetDictionaryString("gray 32")));
        gColorNameVector.push_back(std::make_pair(RGB(79, 79, 79), ::GetDictionaryString("gray 31")));
        gColorNameVector.push_back(std::make_pair(RGB(77, 77, 77), ::GetDictionaryString("gray 30")));
        gColorNameVector.push_back(std::make_pair(RGB(74, 74, 74), ::GetDictionaryString("gray 29")));
        gColorNameVector.push_back(std::make_pair(RGB(71, 71, 71), ::GetDictionaryString("gray 28")));
        gColorNameVector.push_back(std::make_pair(RGB(69, 69, 69), ::GetDictionaryString("gray 27")));
        gColorNameVector.push_back(std::make_pair(RGB(66, 66, 66), ::GetDictionaryString("gray 26")));
        gColorNameVector.push_back(std::make_pair(RGB(64, 64, 64), ::GetDictionaryString("gray 25")));
        gColorNameVector.push_back(std::make_pair(RGB(61, 61, 61), ::GetDictionaryString("gray 24")));
        gColorNameVector.push_back(std::make_pair(RGB(59, 59, 59), ::GetDictionaryString("gray 23")));
        gColorNameVector.push_back(std::make_pair(RGB(56, 56, 56), ::GetDictionaryString("gray 22")));
        gColorNameVector.push_back(std::make_pair(RGB(54, 54, 54), ::GetDictionaryString("gray 21")));
        gColorNameVector.push_back(std::make_pair(RGB(51, 51, 51), ::GetDictionaryString("gray 20")));
        gColorNameVector.push_back(std::make_pair(RGB(48, 48, 48), ::GetDictionaryString("gray 19")));
        gColorNameVector.push_back(std::make_pair(RGB(46, 46, 46), ::GetDictionaryString("gray 18")));
        gColorNameVector.push_back(std::make_pair(RGB(43, 43, 43), ::GetDictionaryString("gray 17")));
        gColorNameVector.push_back(std::make_pair(RGB(41, 41, 41), ::GetDictionaryString("gray 16")));
        gColorNameVector.push_back(std::make_pair(RGB(38, 38, 38), ::GetDictionaryString("gray 15")));
        gColorNameVector.push_back(std::make_pair(RGB(36, 36, 36), ::GetDictionaryString("gray 14")));
        gColorNameVector.push_back(std::make_pair(RGB(33, 33, 33), ::GetDictionaryString("gray 13")));
        gColorNameVector.push_back(std::make_pair(RGB(31, 31, 31), ::GetDictionaryString("gray 12")));
        gColorNameVector.push_back(std::make_pair(RGB(28, 28, 28), ::GetDictionaryString("gray 11")));
        gColorNameVector.push_back(std::make_pair(RGB(26, 26, 26), ::GetDictionaryString("gray 10")));
        gColorNameVector.push_back(std::make_pair(RGB(23, 23, 23), ::GetDictionaryString("gray 9")));
        gColorNameVector.push_back(std::make_pair(RGB(20, 20, 20), ::GetDictionaryString("gray 8")));
        gColorNameVector.push_back(std::make_pair(RGB(18, 18, 18), ::GetDictionaryString("gray 7")));
        gColorNameVector.push_back(std::make_pair(RGB(15, 15, 15), ::GetDictionaryString("gray 6")));
        gColorNameVector.push_back(std::make_pair(RGB(13, 13, 13), ::GetDictionaryString("gray 5")));
        gColorNameVector.push_back(std::make_pair(RGB(10, 10, 10), ::GetDictionaryString("gray 4")));
        gColorNameVector.push_back(std::make_pair(RGB(8, 8, 8), ::GetDictionaryString("gray 3")));
        gColorNameVector.push_back(std::make_pair(RGB(5, 5, 5), ::GetDictionaryString("gray 2")));
        gColorNameVector.push_back(std::make_pair(RGB(3, 3, 3), ::GetDictionaryString("gray 1")));
    }
}

static std::string GetRGBText(COLORREF color)
{
    std::string str;
    int tmpValue = GetRValue(color);
    str += boost::lexical_cast<std::string>(tmpValue);
    str += ", ";
    tmpValue = GetGValue(color);
    str += boost::lexical_cast<std::string>(tmpValue);
    str += ", ";
    tmpValue = GetBValue(color);
    str += boost::lexical_cast<std::string>(tmpValue);
    return str;
}

static std::string GetColorInfoText(const ColorRectInfo &theColorInfo)
{
    std::string str = ::GetDictionaryString("Color");
    str += ": ";
    if(theColorInfo.itsColorIndex == gTransparentColorIndex)
        str += ::GetDictionaryString("transparent");
    else
    {
        str += ::GetRGBText(theColorInfo.itsColor);
        str += " (";
        str += ::GetDictionaryString("RGB");
        str += ")";

        COLORREF searchedColor = theColorInfo.itsColor;

        // Etsit‰‰n v‰rille kuvausta suurenevalla hakuhaarukalla
        for(int allowedDiff = 0; allowedDiff <= 20; allowedDiff += 5)
        {
            // V‰rin haku lambda funktio, joka sallii annetun suuruisen v‰rieron etsityiss‰ v‰reiss‰.
            auto colorEqualFunction = [searchedColor, allowedDiff](const std::pair<COLORREF, std::string> &valuePair)
            {
                int rDiff = std::abs(GetRValue(searchedColor) - GetRValue(valuePair.first));
                int gDiff = std::abs(GetGValue(searchedColor) - GetGValue(valuePair.first));
                int bDiff = std::abs(GetBValue(searchedColor) - GetBValue(valuePair.first));
                return (rDiff <= allowedDiff && gDiff <= allowedDiff && bDiff <= allowedDiff);
            };

            auto colorNameIter = std::find_if(gColorNameVector.begin(), gColorNameVector.end(), colorEqualFunction);
            if(colorNameIter != gColorNameVector.end())
            {
                str += "\n";
                if(allowedDiff == 0)
                {
                    str += "\t";
                    str += colorNameIter->second;
                }
                else
                {
                    str += "closest reference: ";
                    str += colorNameIter->second;
                    str += " (";
                    str += ::GetRGBText(colorNameIter->first);
                    str += ")";
                }
                break; // Lˆytyi kuvaus, lopetetaan etsint‰ looppi
            }
        }
    }
    return str;
}


/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
/////////////////////////////////////////////////////////////////////////////
// CFmiTMColorIndexDlg dialog


CFmiTMColorIndexDlg::CFmiTMColorIndexDlg(CFmiModifyDrawParamDlg *theModifyDrawParamDlg, const std::string &theTitleStr, const std::string theHelpStr, Matrix3D<std::pair<int, COLORREF> >* theColorsCube, boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool doIsolineModifications, CWnd* pParent)
: CDialog(CFmiTMColorIndexDlg::IDD, pParent)
, itsUsedColorsCube(*theColorsCube)
, itsTitleStr(theTitleStr)
, itsHelpStr(theHelpStr)
, itsColorRectVector()
, itsSelectedColorsRectVector()
, itsDrawParam(theDrawParam)
, itsSelectedColorIndex(-1)
, itsLastColorPaletteColorIndex(-1)
, itsMemoryBitmap(0)
, fDrawButtonArea(true)
, itsColorPaletteBottomY(0)
, fDoViewUpdates(TRUE)
, itsModifyDrawParamDlg(theModifyDrawParamDlg)
, fDoIsolineModifications(doIsolineModifications)
, itsSpecialClassesValuesStrU_(_T(""))
, fUseColorBlendingWithCustomContours(FALSE)
, itsLatestAcceptedSpecialClasses()
, itsLatestAcceptedContourGap()
, fSpecialClassesHaveInvalidValues(false)
, itsContourGab(0)
{
	static bool fDoViewUpdatesMemoryInitializedYet = false;
	if(fDoViewUpdatesMemoryInitializedYet == false)
	{
		fDoViewUpdatesMemory = fDoViewUpdates;
		fDoViewUpdatesMemoryInitializedYet = true;
	}
	else
		fDoViewUpdates = fDoViewUpdatesMemory;

	//{{AFX_DATA_INIT(CFmiTMColorIndexDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CFmiTMColorIndexDlg::~CFmiTMColorIndexDlg(void)
{
	CtrlView::DestroyBitmap(&itsMemoryBitmap);
}

void CFmiTMColorIndexDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiTMColorIndexDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Check(pDX, IDC_CHECK_INSTANT_VIEW_UPDATE, fDoViewUpdates);
	//}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_SPECIAL_CLASSES_VALUES, itsSpecialClassesValuesStrU_);
	DDX_Check(pDX, IDC_CHECK_USE_COLOR_BLENDING_WITH_CUSTOM_CONTOURS, fUseColorBlendingWithCustomContours);
	DDX_Text(pDX, IDC_CONTOUR_GAP, itsContourGab);
}


BEGIN_MESSAGE_MAP(CFmiTMColorIndexDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiTMColorIndexDlg)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_CHECK_INSTANT_VIEW_UPDATE, OnBnClickedCheckInstantViewUpdate)
	ON_WM_RBUTTONUP()
	ON_EN_CHANGE(IDC_SPECIAL_CLASSES_VALUES, &CFmiTMColorIndexDlg::OnEnChangeSpecialClassesValues)
	ON_EN_CHANGE(IDC_CONTOUR_GAP, &CFmiTMColorIndexDlg::OnEnChangeContourGap)
	ON_BN_CLICKED(IDC_CHECK_USE_COLOR_BLENDING_WITH_CUSTOM_CONTOURS, &CFmiTMColorIndexDlg::OnBnClickedCheckUseColorBlendingWithCustomContours)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(ID_BUTTOM_REMOVE_COLOR, &CFmiTMColorIndexDlg::OnBnClickedButtomRemoveColor)
    ON_NOTIFY(UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
    ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiTMColorIndexDlg message handlers

void CFmiTMColorIndexDlg::MakeViewUpdates(void)
{
	if(fDoViewUpdates)
	{
		MakeColorUpdatesTodrawParam();
		itsModifyDrawParamDlg->SkipReadingSpecialClassColorIndices(true);
		try
		{
			itsModifyDrawParamDlg->OnBnClickedModifyDrwParamRefresh();
		}
		catch(...)
		{
		}
		itsModifyDrawParamDlg->SkipReadingSpecialClassColorIndices(false);
	}
}

int CFmiTMColorIndexDlg::CalcCustomDrawAreaTop(void)
{
    CWnd *bottomControl = GetDlgItem(ID_BUTTOM_REMOVE_COLOR);
	CRect bottomControlRect;
	bottomControl->GetWindowRect(bottomControlRect);
    ScreenToClient(bottomControlRect);
	return bottomControlRect.bottom;
}

void CFmiTMColorIndexDlg::InitializeColorRects(void)
{
    itsColorRectVector.clear();

    int xsize = static_cast<int>(itsUsedColorsCube.Columns());
    int ysize = static_cast<int>(itsUsedColorsCube.Rows());
    int zsize = static_cast<int>(itsUsedColorsCube.zDIM());
    int totalIndex = 1;
    int shift = 18;
    int currentLocX = 5;
    int maxLocX = 5;
    itsColorPaletteBottomY = CalcCustomDrawAreaTop();
    int currentLocY = itsColorPaletteBottomY + 66;
    CRect colorRect;


    int continuousRowCount = 8;
    int currentYBase = currentLocY;
    int currentXBase = currentLocX;
    ColorRectInfo tmp;
    itsColorRectVector.push_back(tmp); // Laitetaan t‰ss‰ dummy arvo l‰pin‰kyv‰‰ varten, sen arvot t‰ytet‰‰n vasta metodin lopuksi

    for(int k = 0; k < zsize; k++)
    {
        for(int j = 0; j < ysize; j++)
        {
            for(int i = 0; i < xsize; i++)
            {
                tmp.itsColorIndex = itsUsedColorsCube[k][j][i].first;
                tmp.itsColor = itsUsedColorsCube[k][j][i].second;
                tmp.itsColorRect = CRect(currentLocX, currentLocY, currentLocX + gColorAreaSizeX, currentLocY + gColorAreaSizeY);
                itsColorRectVector.push_back(tmp);

                if(totalIndex % (continuousRowCount * 8 * 4) == 0)
                {
                    currentYBase += static_cast<int>(8.5 * (gColorAreaSizeY + shift));
                    currentLocY = currentYBase;
                    currentLocX = currentXBase;
                }
                else if(totalIndex % (continuousRowCount * 8) == 0)
                {
                    currentLocY = currentYBase;
                    currentLocX += static_cast<int>(1.5 * (gColorAreaSizeX + gColorAreaGapX));
                }
                else if(totalIndex % continuousRowCount == 0)
                {
                    currentLocY = currentYBase;
                    currentLocX += 1 * (gColorAreaSizeX + gColorAreaGapX);
                }
                else
                {
                    currentLocY += gColorAreaSizeY + shift;
                }
                totalIndex++;
                if(maxLocX < currentLocX)
                    maxLocX = currentLocX;
            }
        }
    }
    // Lopuksi pit‰‰ laittaa l‰pin‰kyv‰‰ ruutua varten laatikko, otetaan pohjaksi 1. ja viimeinen laatikko
    // ja siirret‰‰n laatikkoa niiden avulla oikeaan yl‰kulmaan.
    CRect firstColorRect(itsColorRectVector[1].itsColorRect);
    CRect lastColorRect(itsColorRectVector[itsColorRectVector.size() - 1].itsColorRect);
    tmp.itsColorIndex = gTransparentColorIndex;
    tmp.itsColor = 0x00000000; // t‰ll‰ v‰rill‰ ei ole v‰li‰, koska l‰pin‰kyvyys on se juttu
    int transY = 28;
    tmp.itsColorRect = CRect(lastColorRect.left, firstColorRect.top - transY, lastColorRect.right, firstColorRect.bottom - transY);
    itsColorRectVector[0] = tmp;

}

CRect CFmiTMColorIndexDlg::CalcSelectedColorRect(size_t theIndex)
{
	CRect aRect;
	aRect.top = itsColorPaletteBottomY + 17;
	aRect.left = static_cast<LONG>(gColorAreaGapX + gColorAreaSizeX*theIndex + gColorAreaGapX*theIndex);
	aRect.bottom = aRect.top + gColorAreaSizeY;
	aRect.right = aRect.left + gColorAreaSizeX;

	return aRect;
}

// Jos on valittuna jo jokin valituista v‰reist‰, laita annettu v‰ri-info siihen tilalle.
// Muuten lis‰‰ valittujen listaan annetun v‰rin, pit‰‰ lasketa uusi sijainti,
// eli sijainti laitetaan viimeiseksi valittujen v‰rien piirto paikkaan.
void CFmiTMColorIndexDlg::AddSelectedColor(const ColorRectInfo &theColorRectInfo)
{
	if(itsSelectedColorIndex >= 0)
	{
		CRect tmpRect = itsSelectedColorsRectVector[itsSelectedColorIndex].itsColorRect;
		itsSelectedColorsRectVector[itsSelectedColorIndex] = theColorRectInfo;
		itsSelectedColorsRectVector[itsSelectedColorIndex].itsColorRect = tmpRect;
	}
	else
	{
		// lasketaan uuden valitun v‰rin indeksi (indeksit alkaa 0:sta)
		size_t newIndex = itsSelectedColorsRectVector.size();
		CRect newRect(CalcSelectedColorRect(newIndex));
		ColorRectInfo tmp = theColorRectInfo;
		tmp.itsColorRect = newRect;
		itsSelectedColorsRectVector.push_back(tmp);
	}
}

// Sijoitetaan annettu v‰ri ennen valittua v‰ri‰, jolloin pit‰‰ siirt‰‰ kaikkia
// sen j‰lkeisi‰ v‰rej‰ eteenp‰in.
// Jos ei oltu valittu mit‰‰n, lis‰t‰‰n viimeiseksi.
void CFmiTMColorIndexDlg::InsertBeforeSelectedColor(const ColorRectInfo &theColorRectInfo)
{
	if(itsSelectedColorIndex >= 0)
	{
		MoveRemainingSelectedColorRectsForward(itsSelectedColorIndex);
		CRect tmpRect = itsSelectedColorsRectVector[itsSelectedColorIndex].itsColorRect;
		itsSelectedColorsRectVector[itsSelectedColorIndex] = theColorRectInfo;
		itsSelectedColorsRectVector[itsSelectedColorIndex].itsColorRect = tmpRect;
	}
	else
		AddSelectedColor(theColorRectInfo);
}

struct ColorIndexFinder
{
	ColorIndexFinder(int theColorIndex)
	:itsColorIndex(theColorIndex)
	{}

	bool operator()(const ColorRectInfo &theColorRectInfo)
	{
		return theColorRectInfo.itsColorIndex == itsColorIndex;
	}

	int itsColorIndex;
};

static std::vector<ColorRectInfo>::iterator FindColorRectWithColorIndex(std::vector<ColorRectInfo> &theColorRectVector, int theColorIndex)
{
	return std::find_if(theColorRectVector.begin(), theColorRectVector.end(), ColorIndexFinder(theColorIndex));
}

static std::string VectorValues2Str(const std::vector<float> &theValues)
{
	std::string str;
	try
	{
		size_t ssize = theValues.size();
		for(size_t i = 0; i < ssize; i++)
		{
			str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(theValues[i], 4);
			if(i < ssize - 1)
				str += ", ";
		}
	}
	catch(...)
	{
		// ei tehd‰ mit‰‰n ainakaan viel‰
	}
	return str;
}

static void TESTWriteColorMapToFile(std::vector<ColorRectInfo> &colors)
{
    std::ofstream out("D:\\colors.txt");
    if(out)
    {
        for(const auto &colorInfo : colors)
            out << ::GetColorInfoText(colorInfo) << std::endl;
    }
}

static const int COLOR_INDEX_VIEW_TOOLTIP_ID = 1234570;

BOOL CFmiTMColorIndexDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
    CFmiWin32Helpers::InitializeCPPTooltip(this, m_tooltip, COLOR_INDEX_VIEW_TOOLTIP_ID, 400);
    InitializeColorRects();
	SetWindowText(CA2T(itsTitleStr.c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_SPECIAL_CLASSES_STR, "Special classes (numbers in rising order!)");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_STEP_STR, "Step");
    CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTOM_REMOVE_COLOR, "Remove Color");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DRAW_PARAM_STEP_CONTOUR_STR, "Contour");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_INSTANT_VIEW_UPDATE, "Instant view update");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_COLOR_BLENDING_WITH_CUSTOM_CONTOURS, "Use color blending with contours");
    
	// Lis‰t‰‰n jo valitut v‰rit
	auto specialClassColorIndices = GetSpecialClassColorIndexies();
	for(size_t i=0; i<specialClassColorIndices.size(); i++)
	{
		std::vector<ColorRectInfo>::iterator it = ::FindColorRectWithColorIndex(itsColorRectVector, specialClassColorIndices[i]);
		if(it != itsColorRectVector.end())
		{
			AddSelectedColor(*it);
		}
	}

	itsLatestAcceptedSpecialClasses = GetSpecialClassValues();
    itsSpecialClassesValuesStrU_ = CA2T(::VectorValues2Str(itsLatestAcceptedSpecialClasses).c_str());
	itsContourGab = itsLatestAcceptedContourGap = itsDrawParam->ContourGab();
    // Luokan dataosan fUseColorBlendingWithCustomContours nimi on oikea 
    // k‰yttˆtarkoitus kyseiselle asetukselle (EI UseIsoLineGabWithCustomContours)
	fUseColorBlendingWithCustomContours = itsDrawParam->UseIsoLineGabWithCustomContours();
    ::InitColorNameMap();

//    ::TESTWriteColorMapToFile(itsColorRectVector);
    auto rect = CalcRealFinalDialogSize();
    MoveWindow(rect, TRUE);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

const std::vector<float>& CFmiTMColorIndexDlg::GetSpecialClassValues()
{
    if(fDoIsolineModifications)
        return itsDrawParam->SpecialIsoLineValues();
    else
        return itsDrawParam->SpecialContourValues();
}

void CFmiTMColorIndexDlg::SetSpecialClassValues(const std::vector<float>& classValues)
{
    if(fDoIsolineModifications)
        itsDrawParam->SetSpecialIsoLineValues(classValues);
    else
        itsDrawParam->SetSpecialContourValues(classValues);
}

const std::vector<int>& CFmiTMColorIndexDlg::GetSpecialClassColorIndexies()
{
    if(fDoIsolineModifications)
        return itsDrawParam->SpecialIsoLineColorIndexies();
    else
        return itsDrawParam->SpecialContourColorIndexies();
}

void CFmiTMColorIndexDlg::SetSpecialClassColorIndexies(const std::vector<int>& classColorIndexies)
{
    if(fDoIsolineModifications)
        itsDrawParam->SetSpecialIsoLineColorIndexies(classColorIndexies);
    else
        itsDrawParam->SetSpecialContourColorIndexies(classColorIndexies);
}

static bool IsGrayColor(COLORREF theColor)
{
    return (GetRValue(theColor) == GetGValue(theColor)) && (GetRValue(theColor) == GetBValue(theColor));
}

static CRect MakeSelectedAreaRect(const CRect theColorRect)
{
	CRect aRect(theColorRect);
	// suurennetaan laatikkoa, niin ett‰ se ymp‰rˆi valitun v‰rin laatikon ja indeksi tekstin
	aRect.top -= 3;
	aRect.bottom += 17;
	aRect.left -= 3;
	aRect.right += 3;
	return aRect;
}

static void DrawColorMarkerRectangle(CDC *theDC, std::vector<ColorRectInfo> &theColorRectVector, int theIndex, COLORREF theWantedColor, int theRectWidth)
{
    if(theIndex >= 0 && theRectWidth > 0 && theRectWidth < 50)
	{
		if(theIndex >= static_cast<int>(theColorRectVector.size()))
			throw std::runtime_error("Error in CFmiTMColorIndexDlg - DrawColorMarkerRectangle, selected color index too big, error in application...");
		else
		{
			ColorRectInfo tmp(theColorRectVector[theIndex]);
			CRect aRect = ::MakeSelectedAreaRect(tmp.itsColorRect);

			CBrush aBrush(theWantedColor);
			for(int i=0; i<theRectWidth; i++)
			{
				theDC->FrameRect(&aRect, &aBrush);
				aRect.InflateRect(-1, -1, -1, -1); // vanhassa GDI:ss‰ ei ole edes viivan paksuutta kun piirt‰‰ rect:i‰!!!! siksi pit‰‰ tehd‰ t‰ll‰inen looppi
			}
		}
	}
}

static void DrawColorRects(CDC *theDC, std::vector<ColorRectInfo> &theColorRectVector, COLORREF backgroundColor)
{
    for(size_t i = 0; i<theColorRectVector.size(); i++)
    {
        int colIndex = theColorRectVector[i].itsColorIndex;
        COLORREF col = theColorRectVector[i].itsColor;
        CRect posRect(theColorRectVector[i].itsColorRect);
        if(colIndex == gTransparentColorIndex)
        { // piirret‰‰n vain mustalle laatikon reunat ja teksti
            CBrush aBrush(static_cast<COLORREF>(0x00000000));
            theDC->FrameRect(&posRect, &aBrush);
            theDC->SetBkColor(backgroundColor);
            theDC->TextOut(posRect.left, posRect.bottom + 2, CString(CA2T(NFmiStringTools::Convert<int>(colIndex).c_str())));
        }
        else
        {
            if(::IsGrayColor(col))
                ::DrawColorMarkerRectangle(theDC, theColorRectVector, static_cast<int>(i), RGB(170, 170, 170), 2);
            theDC->FillSolidRect(posRect, col);
            theDC->SetBkColor(backgroundColor);
            theDC->TextOut(posRect.left, posRect.bottom + 2, CString(CA2T(NFmiStringTools::Convert<int>(colIndex).c_str())));
        }
    }
}

void CFmiTMColorIndexDlg::DrawSelectedColorMarker(CDC *theDC)
{
	::DrawColorMarkerRectangle(theDC, itsSelectedColorsRectVector, itsSelectedColorIndex, RGB(255, 0, 0), 2);
}

CRect CFmiTMColorIndexDlg::CalcBelowButtonsClientArea(void)
{
	CRect clientRect;
	GetClientRect(clientRect);
    clientRect.top += itsColorPaletteBottomY; // MFC kontrollien p‰‰lle ei piirret‰ p‰‰lle
	return clientRect;
}

void CFmiTMColorIndexDlg::DoDraw(CDC *theDC)
{
	COLORREF backGroundColor = GetSysColor(CTLCOLOR_DLG);

	if(itsColorRectVector.size())
	{
		CFont myFont14;
		myFont14.CreateFont(14
			   ,0 // width
			   ,0 // escapement (angle in 0.1 degrees)
			   ,1 // orientation (angle in 0.1 degrees)
			   ,FW_NORMAL // weight (FW_NORMAL, FW_BOLD jne.)
			   ,0 // italic
			   ,0 // underline
			   ,0 // strikeout
			   ,1 // char set
			   ,OUT_TT_PRECIS//040397 //291096/LW oli 1
			   ,1
			   ,PROOF_QUALITY
			   ,DEFAULT_PITCH // FF_DONTCARE  1 == FIXED_PITCH  // 29.9.1998/Marko&Persa
			   ,NULL); //040397/LW oli NULL
		CFont* oldFont = theDC->SelectObject(&myFont14);

		CRect clientRect;
		GetClientRect(clientRect);
        // En ymm‰rr‰ miksi GetClientRect ei saa oikeaa piirtoaluetta, joudun lis‰‰m‰‰n t‰h‰n alareunaan lis‰‰ tilaa, 
        // muuten punaiset v‰rivalinta laatikot eiv‰t p‰ivity oikein. Luulen ett‰ vika johtuu jotenkin 
        // DoDraw-metodin lopussa olevasta MoveWindow(rect, TRUE)-kohdasta. Pit‰isi tehd‰ koko dialogi uusiksi...
        clientRect.bottom += 82; 
		if(fDrawButtonArea == false)
			clientRect.bottom -= gButtonAreaHeight; // n‰in OK ja Cancel -painonappuloita ei piirret‰ p‰‰lle
		theDC->FillSolidRect(clientRect, backGroundColor);

		COLORREF back = 0x00b8b8b8;
		theDC->SetBkColor(back);
		theDC->TextOut(5, itsColorPaletteBottomY + 2, _TEXT("Selected colors:"));
        theDC->TextOut(4, itsColorPaletteBottomY + 48, CString(CA2T(itsHelpStr.c_str())));

		CFont myFont12;
		myFont12.CreateFont(12
			   ,0 // width
			   ,0 // escapement (angle in 0.1 degrees)
			   ,1 // orientation (angle in 0.1 degrees)
			   ,FW_NORMAL // weight (FW_NORMAL, FW_BOLD jne.)
			   ,0 // italic
			   ,0 // underline
			   ,0 // strikeout
			   ,1 // char set
			   ,OUT_TT_PRECIS//040397 //291096/LW oli 1
			   ,1
			   ,PROOF_QUALITY
			   ,DEFAULT_PITCH // FF_DONTCARE  1 == FIXED_PITCH  // 29.9.1998/Marko&Persa
			   ,NULL); //040397/LW oli NULL
		theDC->SelectObject(&myFont12);

		::DrawColorRects(theDC, itsSelectedColorsRectVector, back);
		::DrawColorRects(theDC, itsColorRectVector, back);
		DrawSelectedColorMarker(theDC);
        ::DrawColorMarkerRectangle(theDC, itsColorRectVector, itsLastColorPaletteColorIndex, RGB(255, 0, 0), 2);

	// piirret‰‰n viel‰ erottava apuviiva v‰ri paletin alle (valitun v‰rin deselectointi tapahtuu klikkaamalle viivan alapuoliseen alueeseen)
		CPen aPen(0, 1, back);
		CPen *oldPen = theDC->SelectObject(&aPen);
		theDC->MoveTo(clientRect.left, itsColorPaletteBottomY);
		theDC->LineTo(clientRect.right, itsColorPaletteBottomY);

        theDC->SelectObject(oldPen);

		theDC->SelectObject(oldFont);
	}
	else
	{
		COLORREF back = 0x00b8b8b8;
		theDC->SetBkColor(back);
		theDC->TextOut(0, 0, _TEXT("Colors were not initialized, error in application?"));
	}
}

CRect CFmiTMColorIndexDlg::CalcRealFinalDialogSize()
{
    CRect rect;
    GetWindowRect(&rect);
    CRect lastRect(itsColorRectVector[itsColorRectVector.size() - 1].itsColorRect); // oletetaan ett‰ viimeinen listalla oleva laatikko on myˆs oikeassa reunassa
    auto colorRectBasedRightCoordinate = rect.left + lastRect.right + 25;
    auto dialogControlBasedRightCoordinate = 0;
    auto *bottomRightMostPressButton = GetDlgItem(ID_BUTTOM_REMOVE_COLOR);
    if(bottomRightMostPressButton)
    {
        CRect controlRect;
        bottomRightMostPressButton->GetWindowRect(&controlRect);
        dialogControlBasedRightCoordinate = controlRect.right + 15;
    }
    if(colorRectBasedRightCoordinate > dialogControlBasedRightCoordinate)
        rect.right = colorRectBasedRightCoordinate;
    else
        rect.right = dialogControlBasedRightCoordinate;
    rect.bottom = rect.top + lastRect.bottom + 70;
    return rect;
}

void CFmiTMColorIndexDlg::OnPaint()
{
	static CSize lastClientSize;

	CPaintDC dc(this); // device context for painting

	// *** double buffer koodia ****
	CRect clientArea;
	GetClientRect(&clientArea);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	if(itsMemoryBitmap == 0 || lastClientSize != clientArea.Size())
	{
		lastClientSize = clientArea.Size();
		CtrlView::DestroyBitmap(&itsMemoryBitmap);
		CSize realBitmapSize(clientArea.Size());
		realBitmapSize.cy -= gButtonAreaHeight; // pit‰‰ tehd‰ pienempi bitmap, ett‰ painonappulat (ok+cancel) j‰‰ piirron alle
        CtrlView::MakeCombatibleBitmap(this, &itsMemoryBitmap);
		fDrawButtonArea = true;
	}
	else
		fDrawButtonArea = false;
	CBitmap *oldBitmap = dcMem.SelectObject(itsMemoryBitmap);
	// *** double beffer koodia ****

	DoDraw(&dcMem);

	// *** double beffer koodia ****
	dc.BitBlt(0
			 ,0
			 ,clientArea.Width()
			 ,clientArea.Height()
			 ,&dcMem
			 ,0
			 ,0
			 ,SRCCOPY);

	itsMemoryBitmap = dcMem.SelectObject(oldBitmap);
	dcMem.DeleteDC();
	// *** double beffer koodia ****

	// Do not call CDialog::OnPaint() for painting messages
}

void CFmiTMColorIndexDlg::MakeColorUpdatesTodrawParam(void)
{
	// Lis‰t‰‰n valitut v‰rit takaisin drawParamiin
	std::vector<int> specialClassColorIndices;
	for(size_t i=0; i<itsSelectedColorsRectVector.size(); i++)
		specialClassColorIndices.push_back(itsSelectedColorsRectVector[i].itsColorIndex);

	SetSpecialClassColorIndexies(specialClassColorIndices);

	// tehd‰‰n muutkin tarvittavat p‰ivitykset drawParamiin
	SetSpecialClassValues(itsLatestAcceptedSpecialClasses);
	itsDrawParam->ContourGab(itsLatestAcceptedContourGap);
	itsDrawParam->UseIsoLineGabWithCustomContours(fUseColorBlendingWithCustomContours == TRUE);

}

void CFmiTMColorIndexDlg::OnOK()
{
	MakeColorUpdatesTodrawParam();

	CDialog::OnOK();
}

struct ColorRectFinder
{
	ColorRectFinder(const CPoint &thePoint, bool useLargerArea)
	:itsPoint(thePoint)
	,fUseLargerArea(useLargerArea)
	{}

	bool operator()(const ColorRectInfo &theColorRectInfo)
	{
		if(fUseLargerArea)
		{
			return ::MakeSelectedAreaRect(theColorRectInfo.itsColorRect).PtInRect(itsPoint) == TRUE;
		}
		else
			return theColorRectInfo.itsColorRect.PtInRect(itsPoint) == TRUE;
	}

	CPoint itsPoint;
	bool fUseLargerArea; // etsint‰ saatetaan haluta isomman laatikon sis‰lt‰, eli mukana myˆs indeksi teksti alue
};

static std::vector<ColorRectInfo>::iterator FindColorRect(std::vector<ColorRectInfo> &theColorRectVector, CPoint thePoint, bool fUseLargerArea)
{
	return std::find_if(theColorRectVector.begin(), theColorRectVector.end(), ColorRectFinder(thePoint, fUseLargerArea));
}

void CFmiTMColorIndexDlg::SetSelectedColorIndex(const CPoint &thePoint)
{
	std::vector<ColorRectInfo>::iterator it = ::FindColorRect(itsSelectedColorsRectVector, thePoint, true);
	if(it != itsSelectedColorsRectVector.end())
	{
		itsSelectedColorIndex = static_cast<int>(std::distance(itsSelectedColorsRectVector.begin(), it));
		std::vector<ColorRectInfo>::iterator it2 = ::FindColorRectWithColorIndex(itsColorRectVector, (*it).itsColorIndex);
		if(it2 != itsColorRectVector.end())
			itsLastColorPaletteColorIndex = static_cast<int>(std::distance(itsColorRectVector.begin(), it2));
	}
	else
	{
		if(thePoint.y > itsColorPaletteBottomY) // hiiren klikkauksen pit‰‰ menn‰ v‰ripaletin alapuoliseen osaan ennenkuin deselectointi tapahtuu
			itsSelectedColorIndex = -1;
	}
}

void CFmiTMColorIndexDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	std::vector<ColorRectInfo>::iterator it = ::FindColorRect(itsColorRectVector, point, true);
	if(it != itsColorRectVector.end())
	{
		AddSelectedColor(*it);
		itsLastColorPaletteColorIndex = static_cast<int>(std::distance(itsColorRectVector.begin(), it));
	}
	else
		SetSelectedColorIndex(point); // jos ei osunut v‰ri palettiin, niin katso valittiinko joku valituista v‰reist‰

	MakeViewUpdates();
	InvalidateRect(CalcBelowButtonsClientArea(), FALSE); // n‰in painonapit eiv‰t v‰lky p‰ivityksess‰

	CDialog::OnLButtonUp(nFlags, point);
}

void CFmiTMColorIndexDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	std::vector<ColorRectInfo>::iterator it = ::FindColorRect(itsColorRectVector, point, true);
	if(it != itsColorRectVector.end())
	{
		size_t oldSize = itsColorRectVector.size();
		InsertBeforeSelectedColor(*it);
		if(itsSelectedColorIndex >= 0 && oldSize < itsColorRectVector.size())
			itsSelectedColorIndex++; // pidet‰‰n valittu v‰ri samana eli se on nyt yhden eteenp‰in
		itsLastColorPaletteColorIndex = static_cast<int>(std::distance(itsColorRectVector.begin(), it));
	}

	MakeViewUpdates();
	InvalidateRect(CalcBelowButtonsClientArea(), FALSE); // n‰in painonapit eiv‰t v‰lky p‰ivityksess‰

	CDialog::OnRButtonUp(nFlags, point);
}

template<class T>
static void RemoveNthElementFromVector(T &theVector, int theRemovedItemIndex)
{
	if(theRemovedItemIndex >= 0 && theRemovedItemIndex < static_cast<int>(theVector.size()))
		theVector.erase(theVector.begin() + theRemovedItemIndex);
}

// pit‰‰ keskelle listaa tehdyn deleten j‰lkeen siirt‰‰ listalla olleita colorRecteja 'taaksep‰in'
void CFmiTMColorIndexDlg::MoveRemainingSelectedColorRectsBackward(int theSelectedColorIndex)
{
	if(theSelectedColorIndex >= 0 && theSelectedColorIndex < static_cast<int>(itsSelectedColorsRectVector.size()))
	{
		for(size_t i = theSelectedColorIndex; i < itsSelectedColorsRectVector.size(); i++)
			itsSelectedColorsRectVector[i].itsColorRect = CalcSelectedColorRect(i);
	}
}


void CFmiTMColorIndexDlg::MoveRemainingSelectedColorRectsForward(int theSelectedColorIndex)
{
	if(theSelectedColorIndex >= 0 && theSelectedColorIndex < static_cast<int>(itsSelectedColorsRectVector.size()))
	{
		itsSelectedColorsRectVector.push_back(itsSelectedColorsRectVector[itsSelectedColorsRectVector.size()-1]); // lis‰t‰‰n yksi uusi color per‰‰n
		for(size_t i = itsSelectedColorsRectVector.size() - 1; static_cast<int>(i) >= theSelectedColorIndex + 1; i--)
		{
			itsSelectedColorsRectVector[i] = itsSelectedColorsRectVector[i-1]; // siirr‰ v‰riinfo taaksep‰in
			itsSelectedColorsRectVector[i].itsColorRect = CalcSelectedColorRect(i); // laita sijainti kuntoon
		}
	}
}

void CFmiTMColorIndexDlg::DeleteSelectedColorRect(void)
{
	if(itsSelectedColorIndex >= 0)
	{
		::RemoveNthElementFromVector(itsSelectedColorsRectVector, itsSelectedColorIndex);
		MoveRemainingSelectedColorRectsBackward(itsSelectedColorIndex);
	}
	else
		::RemoveNthElementFromVector(itsSelectedColorsRectVector, static_cast<int>(itsSelectedColorsRectVector.size())-1);
	if(itsSelectedColorIndex >= static_cast<int>(itsSelectedColorsRectVector.size()))
		itsSelectedColorIndex = -1;

	if(itsSelectedColorsRectVector.empty())
		itsSelectedColorIndex = -1;
}

static void ChangeSelectedColorNotPlace(std::vector<ColorRectInfo> &theSelectedColorVector, int theSelectedColorIndex, std::vector<ColorRectInfo> &theColorVector, int theLastColorPaletteColorIndex)
{
	if(theSelectedColorIndex >= 0 && theLastColorPaletteColorIndex > 0 && theSelectedColorIndex < static_cast<int>(theSelectedColorVector.size()) && theLastColorPaletteColorIndex < static_cast<int>(theColorVector.size()))
	{
		CRect tmpRect(theSelectedColorVector[theSelectedColorIndex].itsColorRect);
		theSelectedColorVector[theSelectedColorIndex] = theColorVector[theLastColorPaletteColorIndex];
		theSelectedColorVector[theSelectedColorIndex].itsColorRect = tmpRect;
	}
}

static int MakeColorShadeAndGetItsColorIndex(Matrix3D<std::pair<int, COLORREF> > &theUsedColorsCube, const ColorRectInfo &theColorInfo, short zDelta)
{
/*
	static const NFmiColor sWhiteColor(1.f, 1.f, 1.f);
	static const NFmiColor sBlackColor(0.f, 0.f, 0.f);
	NFmiColor aColor(CFmiWin32Helpers::ColorRef2Color(theColorInfo.itsColor));
	if(zDelta > 0)
		aColor.Mix(sWhiteColor, 0.2f);
	else
		aColor.Mix(sBlackColor, 0.2f);
*/
	NFmiColor aColor(CtrlView::ColorRef2Color(theColorInfo.itsColor));
	NFmiColor modifiedColor = NFmiColorSpaces::GetBrighterColor(aColor, (zDelta > 0) ? 20 : -20);

	return RgbToColorIndex(theUsedColorsCube, modifiedColor);
}

BOOL CFmiTMColorIndexDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect windowRect;
	GetWindowRect(&windowRect);
	if(windowRect.PtInRect(pt)) // pt ja windowRect ovat molemmat absoluuttisessa n‰yttˆmaailmassa
	{
		if(itsLastColorPaletteColorIndex >= 0)
		{
			if(nFlags & MK_CONTROL && itsSelectedColorIndex >= 0)
			{ // Vaalennetaan tai tummennetaan v‰ri‰ jos CTRL pohjassa
				int shadedColorIndex = ::MakeColorShadeAndGetItsColorIndex(itsUsedColorsCube, itsSelectedColorsRectVector[itsSelectedColorIndex], zDelta);

				std::vector<ColorRectInfo>::iterator it = ::FindColorRectWithColorIndex(itsColorRectVector, shadedColorIndex);
				if(it != itsColorRectVector.end())
				{
					AddSelectedColor(*it);
				}
			}
			else
			{
				if(zDelta > 0)
					itsLastColorPaletteColorIndex++;
				else
					itsLastColorPaletteColorIndex--;
				if(itsLastColorPaletteColorIndex < 0)
					itsLastColorPaletteColorIndex = static_cast<int>(itsColorRectVector.size() - 1);
				if(itsLastColorPaletteColorIndex >= static_cast<int>(itsColorRectVector.size()))
					itsLastColorPaletteColorIndex = 0;

				::ChangeSelectedColorNotPlace(itsSelectedColorsRectVector, itsSelectedColorIndex, itsColorRectVector, itsLastColorPaletteColorIndex);
			}
			MakeViewUpdates();
			InvalidateRect(CalcBelowButtonsClientArea(), FALSE); // n‰in painonapit eiv‰t v‰lky p‰ivityksess‰
		}
	}

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CFmiTMColorIndexDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// jos kontrolli-nappi pohjassa liikutaan hiiren kursorin kanssa v‰ri paletin p‰‰ll‰,
	// vaihdetaan lastcolor indeksi‰ ja mahd. valitun v‰rin v‰ri‰
	if(nFlags & MK_CONTROL)
	{
		std::vector<ColorRectInfo>::iterator it = ::FindColorRect(itsColorRectVector, point, true);
		if(it != itsColorRectVector.end())
		{
			itsLastColorPaletteColorIndex = static_cast<int>(std::distance(itsColorRectVector.begin(), it));
			::ChangeSelectedColorNotPlace(itsSelectedColorsRectVector, itsSelectedColorIndex, itsColorRectVector, itsLastColorPaletteColorIndex);
			MakeViewUpdates();
			InvalidateRect(CalcBelowButtonsClientArea(), FALSE); // n‰in painonapit eiv‰t v‰lky p‰ivityksess‰
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CFmiTMColorIndexDlg::OnBnClickedCheckInstantViewUpdate()
{
	UpdateData(TRUE);
	fDoViewUpdatesMemory = fDoViewUpdates;
}

void CFmiTMColorIndexDlg::OnEnChangeSpecialClassesValues()
{
	UpdateData(TRUE);

	fSpecialClassesHaveInvalidValues = false;
	try
	{
        std::string tmp = CT2A(itsSpecialClassesValuesStrU_);
		std::vector<float> classValues = NFmiStringTools::Split<std::vector<float> >(tmp, ",");
		if(CtrlViewUtils::AreVectorValuesInRisingOrder(classValues) == false)
			throw std::runtime_error("xxx");
		else
			itsLatestAcceptedSpecialClasses = classValues;
	}
	catch(...)
	{
		fSpecialClassesHaveInvalidValues = true; // vain asetetaan t‰m‰ lippu p‰‰lle jos tulee mit‰‰n ongelmia
	}
	CWnd *win = GetDlgItem(IDC_STATIC_DRAW_PARAM_SPECIAL_CLASSES_STR);
	if(win)
		win->Invalidate(FALSE);
	MakeViewUpdates();
}

void CFmiTMColorIndexDlg::OnEnChangeContourGap()
{
	UpdateData(TRUE);

	if(itsContourGab > 0)
		itsLatestAcceptedContourGap = itsContourGab;

	MakeViewUpdates();
}

void CFmiTMColorIndexDlg::OnBnClickedCheckUseColorBlendingWithCustomContours()
{
	UpdateData(TRUE);

	MakeViewUpdates();
}

HBRUSH CFmiTMColorIndexDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_DRAW_PARAM_SPECIAL_CLASSES_STR)
	{
		if(this->fSpecialClassesHaveInvalidValues)
			pDC->SetTextColor(RGB(255, 0, 0)); // virhetilanteissa erikois luokka edit boxin labeli v‰ritet‰‰n punaiseksi
		else
			pDC->SetTextColor(RGB(0, 0, 0));
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

void CFmiTMColorIndexDlg::OnBnClickedButtomRemoveColor()
{
	DeleteSelectedColorRect();
	MakeViewUpdates();
	InvalidateRect(CalcBelowButtonsClientArea(), FALSE); // n‰in painonapit eiv‰t v‰lky p‰ivityksess‰
}


BOOL CFmiTMColorIndexDlg::PreTranslateMessage(MSG* pMsg)
{
    m_tooltip.RelayEvent(pMsg);

    return CDialog::PreTranslateMessage(pMsg);
}

static std::string FindColorText(const CPoint &point, std::vector<ColorRectInfo> &theColorVector)
{
    std::vector<ColorRectInfo>::iterator it = ::FindColorRect(theColorVector, point, true);
    if(it != theColorVector.end())
        return ::GetColorInfoText(*it);
    else
        return "";
}

std::string CFmiTMColorIndexDlg::ComposeToolTipText(const CPoint &point)
{
    std::string str = ::FindColorText(point, itsColorRectVector);
    if(str.empty())
        str = ::FindColorText(point, itsSelectedColorsRectVector);
    return str;
}

void CFmiTMColorIndexDlg::NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result)
{
    *result = 0;
    NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

    if(pNotify->ti->nIDTool == COLOR_INDEX_VIEW_TOOLTIP_ID)
    {
        CPoint pt = *pNotify->pt;
        ScreenToClient(&pt);
        pNotify->ti->sTooltip = CA2T(ComposeToolTipText(pt).c_str());
    }
}

void CFmiTMColorIndexDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    CRect rect;
    GetClientRect(rect);
    m_tooltip.SetToolRect(this, COLOR_INDEX_VIEW_TOOLTIP_ID, rect);
}
