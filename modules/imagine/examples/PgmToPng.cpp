// ======================================================================
/*!
 * \brief Convert PGM to PNG
 */
// ======================================================================

#include "NFmiImage.h"
#include <iostream>
#include <string>

using namespace Imagine;
using namespace std;

int main(int argc, const char * argv[])
{
  if(argc != 3)
	{
	  cerr << "Usage: PgmToPng <pngfile> <pgmfile>" << endl;
	  exit(1);
	}

  string infile = argv[1];
  string outfile = argv[2];

  NFmiImage img(infile);
  img.WritePng(outfile);
}
