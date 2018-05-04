// ======================================================================
/*!
 * \file
 * \brief Demonstration of NFmiFace usage
 *
 * The result is written to /tmp/NFmiFace.png and is displayed with display
 */
// ======================================================================

#include "NFmiFace.h"
#include "NFmiFreeType.h"
#include "NFmiImage.h"
#include "NFmiPath.h"
#include <newbase/NFmiFileSystem.h>

#include <iostream>

using namespace std;
using namespace Imagine;

void demo()
{
  const string tmpfile = "NFmiFaceExample.png";
  const int width = 500;
  const int height = 500;

  NFmiColorTools::Color white = NFmiColorTools::MakeColor(255,255,255);
  NFmiImage image(width,height,white);

  NFmiPath path;
  path.MoveTo(0,0);
  path.LineTo(width,height);
  path.MoveTo(width,0);
  path.LineTo(0,height);
  path.Stroke(image,NFmiColorTools::MakeColor(0,255,0));

#if 1
  const string font = "ttf/ArialNarrow.ttf";
  const int w = 0;
  const int h = 20;
  const int dh = -1;
#else  
  const string font = "misc/6x13.pcf.gz";
  const int w = 6;
  const int h = 13;
  const int dh = 0;
#endif

  const NFmiFreeType & ft = NFmiFreeType::Instance();
  ft.Draw(image,font,w,h,0,0,"NorthWest",kFmiAlignNorthWest);
  ft.Draw(image,font,w,h+dh,width/2,0,"North",kFmiAlignNorth);
  ft.Draw(image,font,w,h+2*dh,width,0,"NorthEast",kFmiAlignNorthEast);

  ft.Draw(image,font,w,h+3*dh,0,height/2,"West",kFmiAlignWest,NFmiColorTools::Black,NFmiColorTools::kFmiColorOnOpaque,true);
  ft.Draw(image,font,w,h+4*dh,width/2,height/2,"Center",kFmiAlignCenter,NFmiColorTools::Black,NFmiColorTools::kFmiColorOnOpaque,true);
  ft.Draw(image,font,w,h+5*dh,width,height/2,"East",kFmiAlignEast,NFmiColorTools::Black,NFmiColorTools::kFmiColorOnOpaque,true);

  NFmiColorTools::Color bg = NFmiColorTools::MakeColor(255,0,0);
  ft.Draw(image,font,w,h+6*dh,0,height,"SouthWest",kFmiAlignSouthWest,NFmiColorTools::Black,NFmiColorTools::kFmiColorOnOpaque,true,3,3,bg);
  ft.Draw(image,font,w,h+7*dh,width/2,height,"South",kFmiAlignSouth,NFmiColorTools::Black,NFmiColorTools::kFmiColorOnOpaque,true,3,3,bg);
  ft.Draw(image,font,w,h+8*dh,width,height,"SouthEast",kFmiAlignSouthEast,NFmiColorTools::Black,NFmiColorTools::kFmiColorOnOpaque,true,3,3,bg);
  
  image.WritePng(tmpfile);
  string cmd = "display "+tmpfile;
  system(cmd.c_str());
  NFmiFileSystem::RemoveFile(tmpfile);
}

int main()
{
  try
	{
	  demo();
	  return 0;
	}
  catch(exception & e)
	{
	  cerr << "Caught exception:" << endl << e.what() << endl;
	  return 1;
	}
}
