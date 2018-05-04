// k5/rotate.cc
#include<showseq.h>
#include<algorithm>
#include<vector>
#include<iota.h>

using namespace std;

int main()
{
    vector<int> v(10);
    iota(v.begin(), v.end(), 0);

    for(register size_t shift = 1; shift < 3; shift++)
    {
       cout << "Rotation by " << shift << endl;
       for(size_t i = 0; i < v.size()/shift; i++)
       {
          showSequence(v);
          rotate(v.begin(), v.begin() + shift, v.end());
       }
    }
}
