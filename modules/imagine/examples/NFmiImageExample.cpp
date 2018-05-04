// ======================================================================
/*!
 * \file
 * \brief Demonstration of NFmiImage usage
 *
 * The result is written to /tmp/NFmiImage.png and is displayed with display
 */
// ======================================================================

#include "NFmiImage.h"
#include "NFmiPath.h"
#include <newbase/NFmiFileSystem.h>

#include <iostream>

using namespace std;
using namespace Imagine;

void demo()
{
  const string tmpfile = "NFmiImageExample.png";
  const int width = 500;
  const int height = 500;

  NFmiColorTools::Color white = NFmiColorTools::MakeColor(255,255,255);
  NFmiImage image(width,height,white);

  NFmiPath path;
  path.MoveTo(20,20);
  path.LineTo(width-20,height-20);
  path.LineTo(width-20,20);
  path.LineTo(20,100);
  path.LineTo(200,100);
  path.Stroke(image,3,NFmiColorTools::MakeColor(0,255,0));

  
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
