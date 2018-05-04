// k3/iterator/binsert.cc
// Insert iterators : back insert
#include<showseq.h>
#include<vector>
#include<iterator>

using namespace std;

int main()
{
    vector<int> aVector(5, 0);      // 5 zeros
    cout << "aVector.size() = "
         << aVector.size() << endl; // 5
    showSequence(aVector);          // 0 0 0 0 0

    back_insert_iterator<vector<int> >
                      aBackInserter(aVector);   

    // insertion by means of the operations *, ++, =
    int i = 1;
    while(i < 3)
          *aBackInserter++ = i++;

    cout << "aVector.size() = "
         << aVector.size() << endl;  // 7

    showSequence(aVector);           // 0 0 0 0 0 1 2
}

