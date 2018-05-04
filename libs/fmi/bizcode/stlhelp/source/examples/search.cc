// k5/search.cc
#include<algorithm>
#include<vector>
#include<iostream>
#include<cstdlib>
#include<showseq.h>

using namespace std;

class AbsIntCompare //  ignore signs
{
    public:
      bool operator()(int x, int  y)
      {
          return abs(x) == abs(y);
      }
};

int main()
{
    vector<int> v1(12);
    for(size_t i = 0; i < v1.size(); i++)
        v1[i] = i;        // 0 1 2 3 4 5 6 7 8 9 10 11 12
    cout << "v1 = ";
    showSequence(v1);

    vector<int> v2(4);
    for(size_t i = 0; i < v2.size(); i++)
        v2[i] = i + 5;    // 5 6 7 8
    cout << "v2 = ";
    showSequence(v2);

    //  search for substructure v2 in v1
    vector<int>::iterator
        where = search(v1.begin(), v1.end(),
                       v2.begin(), v2.end());

    // if the sequence v2 does not begin with 5, but with a number >= 10,
    //  the else branch of the if condition is executed.

    if(where != v1.end())
    {
        cout << "  v2 is contained in v1 from position "
             << (where - v1.begin())
             << " onward." << endl;
    }
    else
        cout << "  v2 is not contained in v1."
             << endl;

    // put negative numbers into v2
    for(size_t i = 0; i < v2.size(); i++)
        v2[i] = -(i + 5); // -5 -6 -7 -8
    cout << "v2 = ";
    showSequence(v2);

    //  search for substructure v2 in v1, ignore signs
    where = search(v1.begin(), v1.end(),
                   v2.begin(), v2.end(),
                   AbsIntCompare());

    if(where != v1.end())
    {
        cout << "  v2 is contained in v1 from position "
             << (where - v1.begin())
             << " onward (signs are ignored)."
             << endl;
    }
    else
        cout << "  v2 is not contained in v1."
             << endl;
}
