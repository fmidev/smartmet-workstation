// k5/merge1.cc
#include<algorithm>
#include<showseq.h>
#include<vector>

using namespace std;

int main()
{
    vector<int> v(16);                   // even number
    int middle = v.size()/2;
    for(int i = 0; i < middle; i++)
    {
        v[i]         = 2*i;              // even
        v[middle + i] = 2*i + 1;         // odd
    }
    showSequence(v);
    inplace_merge(v.begin(), v.begin() + middle, v.end());
    showSequence(v);
}
