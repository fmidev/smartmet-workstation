// stowatch.cc       
#include"stowatch.h"
#include<cassert>

void Stopwatch::start()
{     ftime(&starttime);
      state        = running;
}

void Stopwatch::stop()
{     ftime(&stoptime);
      state        = halted;
}

double Stopwatch::duration()
{     if(state == undefined)
      assert(!("Stopwatch not started!\n"));
      if (state == running)
          ftime(&stoptime);
      return double(stoptime.time - starttime.time)
           + double(stoptime.millitm
           - starttime.millitm)/1000.0;
}

