// k5/permute.cc
#include<algorithm>
#include<showseq.h>
#include<vector>
#include<iota.h>

long factorial(unsigned n)
{
    long fac = 1;
    while(n > 1) fac *= n--;
    return fac;
}

using namespace std;

int main()
{
    vector<int> v(4);
    iota(v.begin(), v.end(), 0);     // 0 1 2 3
    long fac = factorial(v.size());

    for(int i = 0; i < fac; i++)
    {
        if(!prev_permutation(v.begin(), v.end()))
          cout << "Start of cycle:\n";
        showSequence(v);
    }
}
