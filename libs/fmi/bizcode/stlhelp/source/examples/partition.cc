// k5/partition.cc
#include<algorithm>
#include<vector>
#include<showseq.h>
#include<iota.h>
#include<rand.h>

using namespace std;

int main()
{
    vector<int> v(12);
    iota(v.begin(), v.end(), -6);
    RAND aRAND;
    random_shuffle(v.begin(), v.end(), aRAND);

    vector<int> unstable = v,
                  stable = v;

    partition(unstable.begin(), unstable.end(), 
              bind2nd(less<int>(),0));
    stable_partition(stable.begin(), stable.end(),
              bind2nd(less<int>(),0));

    cout << "Partition into negative and positive elements\n";
    cout << "sequence             :";
    showSequence(v);        // -5 -1 3 2 -3 5 -4 -6 4 0 1 -2

    cout << "stable partition   :";
    showSequence(stable);   // -5 -1 -3 -4 -6 -2 3 2 5 4 0 1

    cout << "unstable partition :";
    // the negative elements are no longer 
    // in their original order
    showSequence(unstable); // -5 -1 -2 -6 -3 -4 5 2 4 0 1 3
}
