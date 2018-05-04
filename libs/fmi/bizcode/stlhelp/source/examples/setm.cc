// k4/setm.cc  Example for sets
#include<set>
#include<showseq.h>

using namespace std;

int main()
{
    set<int> aSet;  //  default comparison object: less<int>()

    for(int i = 0; i < 10; i++) aSet.insert(i);
#ifdef _MSC_VER // Compiler Issue Q167748 PRB
    for(i = 0; i < 10; i++) aSet.insert(i); // no effect
#else
    for(int i = 0; i < 10; i++) aSet.insert(i); // no effect
#endif
    showSequence(aSet);                     // 0 1 2 3 4 5 6 7 8 9

    /* The display shows that the elements of the set really occur
       exactly once. In the next part of the program, elements are
       deleted. In the first variation, first the element is sought in
       order to delete it with the found iterator. In the second
       variation, deletion is carried out via the specified key. */

    cout << "Deletion by iterator\n"
            "Delete which element? (0..9)" ;
#ifndef _MSC_VER // Compiler Issue Q167748 PRB
    int i;
#endif
    cin >> i;
    set<int>::iterator iter = aSet.find(i);
    if(iter == aSet.end())
       cout << i << " not found!\n";
    else
    {
        cout << "The element " << i            // 1
             << " exists " << aSet.count(i) << " times." << endl;
        aSet.erase(iter);
        cout << i << " deleted!\n";
        cout << "The element " << i            // 0
             << " exists " << aSet.count(i) << " times." << endl;
    }
    showSequence(aSet);

    /* The count() method yields either 0 or 1. Thus, it is an
       indicator as to whether an element is present in the set. */

    cout << "Deletion by value\n"
            "Delete which element? (0..9)" ;
    cin >> i;
    int Count = aSet.erase(i);
    if(Count == 0)
       cout << i << " not found!\n";
    showSequence(aSet);

    /* A further set NumberSet is not initialized with a loop, but by
       specifying the range to be inserted in the constructor.
       Suitable iterators for int values are pointers of int* type.
       The name of a C array can be interpreted as a constant pointer
       to the beginning of the array. When the number of array
       elements is added to this pointer, the result is a pointer that
       points to the position after the last array element. Both
       pointers can be used as iterators for initialization of a set:
       */

    cout << "call constructor with iterator range\n";

    // 2 and 1 twice!
    int Array[] = { 1, 2, 2, 3, 4, 9, 13, 1, 0, 5};
    Count = sizeof(Array)/sizeof(Array[0]);

    set<int> NumberSet(Array, Array + Count);
    showSequence(NumberSet);    // 0 1 2 3 4 5 9 13
}
