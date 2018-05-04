// choose map-implementation here by deleting or inserting
// comment '//' in the next line:
#define STL_map
#include"../sparmat.h"
#include"stowatch.h"
using namespace std;

#ifdef STL_map
char Modus[] = "STL_map";
#else
char Modus[] = "hash-MAP";
#endif

int main()
{
    sparseMatrix<double, long> M(1000000,1000000);
    cout << Modus << " is used here\n";
    cout << "sizeof(int)   : " << sizeof(int) << endl;
    cout << "sizeof(size_t): " << sizeof(size_t) << endl;
    cout << "sizeof(ptrdiff_t): " << sizeof(ptrdiff_t) << endl
         << "container.max_size(): " 
         << (unsigned long)M.max_size() << endl;
    long count = 1,
         assignments = 131072L;
    cout << assignments << "  assignments" << endl;
    // compute speed of computer for scaling
    Stopwatch cl; 
    cl.start();
    long icount=0L; double d=0.0;
    while(cl.duration() < 1.0);       // get starting point
    while(cl.duration() < 2.0)     
    { 
      ++icount; d += 1.0/(double)icount;  // do something
    } 

  while(count < 65537L)
  {
    double scaling = (double)icount*0.0001*(double)assignments/count;

    Stopwatch aClock;
    long i = 423427L;  // arbitrary index
    // generate matrix so that later on only the access is measured,
    // not the creation
       for(long j=777500L; j < 777500L+count; j++)
       {
          M[i][j] = double(j)+double(i)/1e6;
       }

    aClock.start();
    for(long k = 0; k < scaling; k++)
       for(long j=777500L; j < 777500L+count; j++)
       {
          M[i][j] = double(j)+double(i)/1e6;
       }
    aClock.stop();
    cout << "Number of non-zero elements = ";
    cout.width(8);
    cout << M.size() << ": "
         << aClock.duration()/(scaling*count)*1e6
         << " uS per operation" << endl;
    count *=2;
  }
}

