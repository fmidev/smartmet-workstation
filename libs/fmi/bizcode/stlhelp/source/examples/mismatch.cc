// k5/mismatch.cc
#include<algorithm>
#include<vector>
#include<set>
#include<showseq.h>

using namespace std;

int main()
{
    vector<int> v(8);

    for(register size_t i = 0; i < v.size(); i++)
        v[i] = 2*i;                 // sorted sequence

    set<int> s(v.begin(), v.end()); // initialize set} with v
    v[3] = 7;                       // insert mismatch

    showSequence(v);                // display
    showSequence(s);

    // comparison for match with iterator pair 'where'
    pair<vector<int>::iterator, set<int>::iterator>    
          where = mismatch(v.begin(), v.end(), s.begin());

    if(where.first == v.end())
       cout << "Match found." << endl;
    else
      cout << "The first mismatch ("
           << *where.first << " != "
           << *where.second
           << ") was found at position "
           << (where.first - v.begin())
           << "." << endl;
}
