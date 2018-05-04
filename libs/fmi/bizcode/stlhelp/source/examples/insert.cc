// k3/iterator/insert.cc
// Insert iterator
#include<showseq.h>
#include<vector>
#include<iterator>

using namespace std;

int main()
{
    vector<int> aVector(5, 0);      // 5 zeros

    cout << "aVector.size() = "
         << aVector.size() << endl;  // {\sl 5}
    showSequence(aVector);           // {\sl 0 0 0 0 0}

    // insertion by means of the operations *, ++, =
    insert_iterator<vector<int> >
        aBeginInserter(aVector, aVector.begin());

    int i = 1;
    while(i < 3) *aBeginInserter++ = i++;
    // vector:  1 2 0 0 0 0 0
    // size() is now 7

    insert_iterator<vector<int> >
        aMiddleInserter(aVector, aVector.begin() +
                         aVector.size()/2);

    while(i < 6) *aMiddleInserter++   = i++;
    // vector:  1 2 0 3 4 5 0 0 0 0
    // size() is now 10

    insert_iterator<vector<int> >
        anEndInserter(aVector, aVector.end());
    while(i < 9) *anEndInserter++    = i++;

    cout << "aVector.size() = "
         << aVector.size() << endl;  //  13
    showSequence(aVector);           //  1 2 0 3 4 5 0 0 0 0 6 7 8
}
