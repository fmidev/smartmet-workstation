// k5/rshuffle.cc
// Example for random_shuffle()
#include<algorithm>
#include<vector>
#include<showseq.h>
#include<iota.h>
#include<rand.h>

using namespace std;

int main()
{
    vector<int> v(12);
    iota(v.begin(), v.end(), 0);   // 0 1 2 3 4 5 6 7 8 9 10 11

    random_shuffle(v.begin(), v.end());
    showSequence(v);               // 4 8 5 11 7 1 3 9 6 2 10 0
    
    RAND aRAND;
    random_shuffle(v.begin(), v.end(), aRAND);
    showSequence(v);               // 8 1 2 6 11 0 5 4 10 3 9 7
}
