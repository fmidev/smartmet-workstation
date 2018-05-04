// include/rand.h
#ifndef RAND_H
#define RAND_H

class RAND
{
    public:
      RAND() : r(1) {}
      int operator()(int X)
      // returns an int-pseudo random number between 0 and X-1
      // period: 2048
      {
          r = (125 * r) % 8192;
          return int(double(r)/8192.0*(X));
      }
    private:
      long int r;
};


#endif

