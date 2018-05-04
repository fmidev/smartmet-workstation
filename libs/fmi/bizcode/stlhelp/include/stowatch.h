// stowatch.h           stopwatch class
#ifndef stowatch_h
#define stowatch_h            

#include <sys/types.h>
#include <sys/timeb.h>
#include <iostream>

class Stopwatch
{
      timeb starttime, stoptime;
      enum watchstate { undefined, halted, running} state;
    public:
      Stopwatch() { state = undefined;}
      void start();
      void stop();
      double duration();
};
#endif
// end of stowatch.h

