// {\em k6/mainset.cc}
// Example for sets with set algorithms
// alternatively for set (STL) or HSet(hash) implementation
#include<showseq.h>

// compiler switch (see text)
//#define STL_SET

#include<setalgo.h>

#ifdef STL_SET
#include<set>
#else
#include<hset.h>
#include<hashfun.h>
#endif

using namespace std;

int main()
{
// type definition according to selected implementation
#ifdef STL_SET
   typedef set<int> SET;  //  default comparison object: less<int>()
#else
    typedef HSet<int, HashFun<int> > SET;
#endif

    SET  Set1, Set2, Result;
    for(int i = 0; i < 10; i++) Set1.insert(i);
#ifdef _MSC_VER // Compiler Issue Q167748 PRB
    for(i = 7; i < 16; i++) Set2.insert(i);
#else
    for(int i = 7; i < 16; i++) Set2.insert(i);
#endif

    // display
    showSequence(Set1);
    showSequence(Set2);
    cout << "Subset:\n";
    cout << "Includes(Set1, Set2) = "
         << Includes(Set1, Set2) << endl;

    cout << "Includes(Set1, Set1) = "
         << Includes(Set1, Set1) << endl;

    cout << "Union:\n";
    Union(Set1, Set2, Result);
    showSequence(Result);

    cout << "Intersection:\n";
    Intersection(Set1, Set2, Result);
    showSequence(Result);

    cout << "Difference:\n";
    Difference(Set1, Set2, Result);
    showSequence(Result);

    cout << "Symmetric difference:\n";
    Symmetric_Difference(Set1, Set2, Result);
    showSequence(Result);

    cout << "Copy constructor:\n";
    SET newSet(Result);
    showSequence(newSet);

    cout << "Assignment:\n";
    Result = Set1;
    showSequence(Set1);
    showSequence(Result);
}

