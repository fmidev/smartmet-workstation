// k5/fill.cc
#include<algorithm>
#include<vector>
#include<showseq.h>

using namespace std;

int main()
{
    vector<double> v(8);
    // initialize all values with 9.23
    fill(v.begin(), v.end(), 9.23);

    showSequence(v);

    /* fill() expects the specification of the number of elements
       in the sequence which are to be initialized with a value and
       returns an iterator to the end of the range. Here, the first
       half of the sequence is changed: */

    // initialize the first half with 1.01
    vector<double>::iterator iter =
       fill_n(v.begin(), v.size()/2, 1.01);

    showSequence(v);
    cout << "iter is in position = "
         << (iter - v.begin())
         << ", *iter = " << *iter << endl;
}
