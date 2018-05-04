// k7/mainseto.cc
#include<showseq.h>
#include<hset.h>
#include<hashfun.h>
using namespace std;

int main()
{
    int i;
    typedef HSet<int, HashFun<int> > SET;
    SET  Set1, Set2, Result;
    for(i = 0; i < 10; i++) Set1.insert(i);
    for(i = 7; i < 16; i++) Set2.insert(i);

    showSequence(Set1);    // 0 1 2 3 4 5 6 7 8 9
    showSequence(Set2);    // 7 8 9 10 11 12 13 14 15

    cout << "Union:\n";
    Result = Set1;
    Result += Set2;

    showSequence(Result);  // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

    cout << "Intersection:\n";
    Result = Set1;
    Result *= Set2;

    showSequence(Result);  // 7 8 9

    cout << "Union:\n";
    Result = Set1 + Set2;

    showSequence(Result);  // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

    cout << "Intersection:\n";
    Result = Set1 * Set2;
    showSequence(Result);  // 7 8 9

    cout << "Difference:\n";
    Result = Set1 - Set2;

    showSequence(Result);  // 0 1 2 3 4 5 6

    cout << "Symmetric difference:\n";
    Result = Set1 ^ Set2;

    showSequence(Result);  // 0 1 2 3 4 5 6 10 11 12 13 14 15
}
