// k5/swap.cc
#include<algorithm>
#include<vector>
#include<showseq.h>
#include<iota.h>

using namespace std;

int main()
{
    vector<int> v(17);
    iota(v.begin(), v.end(), 10);
    showSequence(v);
      // 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26


    cout << "Swap elements v[3] and v[5]:\n";
    swap(v[3], v[5]);                    // swap
    showSequence(v);
      // 10 11 12 15 14 13 16 17 18 19 20 21 22 23 24 25 26

    cout << "swap first and last elements"
            " via iterator:\n";
    vector<int>::iterator first = v.begin(),
                          last  = v.end();
    last--;

    iter_swap(first, last);          // swap
    showSequence(v);
      // 26 11 12 15 14 13 16 17 18 19 20 21 22 23 24 25 10

    int oneThird = v.size()/3;
    cout << "swap about the first and last thirds "
         << "(" << oneThird << " Positions):\n";
    last = v.begin();
    advance(last, oneThird);        // end of first third
    vector<int>::iterator target = v.end();
    advance(target, -oneThird);          // beginning of second third

    swap_ranges(first, last, target);  // swap
    showSequence(v);
      // 22 23 24 25 10 13 16 17 18 19 20 21 26 11 12 15 14
}
