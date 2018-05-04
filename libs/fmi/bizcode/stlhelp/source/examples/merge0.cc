// k5/merge0.cc
#include<algorithm>
#include<showseq.h>
#include<vector>
#include<iota.h>  

using namespace std;

int main()
{
    vector<int> v1(6);               // sequence 1
    iota(v1.begin(), v1.end(), 0);   // initialize
    showSequence(v1);                // display
                
    vector<int> v2(10);              // sequence 2
    iota(v2.begin(), v2.end(), 0);   // initialize
    showSequence(v2);                // display
                
    vector<int> result(v1.size()+v2.size()); // sequence 3

    merge(v1.begin(), v1.end(),      // merge
          v2.begin(), v2.end(),
          result.begin());
    showSequence(result);            // display
}
