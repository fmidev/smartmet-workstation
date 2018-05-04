// k5/cpy.cc
#include<algorithm>
#include<vector>
#include<showseq.h>
#include<iota.h>

using namespace std;

int main()
{
  const int zero=0;   // wg. GNU 2.8 extra Variable
    vector<int> v1(7),
                v2(7, zero);                     // 7 zeros
    iota(v1.begin(), v1.end(),0);             // result:

    showSequence(v1);       // 0 1 2 3 4 5 6
    showSequence(v2);       // 0 0 0 0 0 0 0

    /* In the copy process from v1 to v2 itself, the beginning of the
       target range is marked by v2.begin(). */

    // copy v1 to v2
    copy(v1.begin(), v1.end(), v2.begin());
    showSequence(v2);       // 0 1 2 3 4 5 6

    /* In order to show the variety of the iterator principle, we will
       now use the algorithm copy() with a special iterator instead of
       the algorithm for_each() algorithm. This iterator is defined as
       Ostream iterator (see exercise) which can display int} numbers
       on the standard output. The copy() algorithm has no
       difficulties with this (in practice, it doesn't give a hoot!).
     */

    // copy v1 to cout, separator *
    ostream_iterator<int> Output(cout, "*");
    copy(v1.begin(), v1.end(), Output);      // 0*1*2*3*4*5*6*
    cout << endl;

    /* Now, a range inside v1 is copied to a different position which
       lies inside v1. The range is chosen in such a way that source
       and target ranges overlap. The first four numbers are copied.
     */

    // overlapping ranges:
    vector<int>::iterator last = v1.begin();
    advance(last, 4);                       // 4 steps forward
    copy_backward(v1.begin(), last, v1.end());
    copy(v1.begin(), v1.end(), Output);     // 0*1*2*0*1*2*3*
    cout << endl;
}
