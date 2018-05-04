// include/myrandom.h
#ifndef MYRANDOM_H
#define MYRANDOM_H

#include<cstdlib>   // rand() and  RAND_MAX

using namespace std;

class Random
{
    public:
      Random(int b): Range(b) {}
      // returns an int-random number between 0 and Range -1
      int operator()() 
      {
          return (int)(((double)rand()*(Range))/(RAND_MAX+1.0));
      }
    private:
      int Range;
};


#endif

