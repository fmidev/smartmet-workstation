// k5/heap.cc
#include<algorithm>
#include<showseq.h>
#include<vector>
#include<iota.h>

using namespace std;

int main()
{
    vector<int> v(12);           // container for heap
    iota(v.begin(), v.end(), 0); // enter 0 .. 11 
    showSequence(v); // 0 1 2 3 4 5 6 7 8 9 10 11

    // create valid heap
    make_heap(v.begin(), v.end());
    showSequence(v); // 11 10 6 8 9 5 0 7 3 1 4 2
    // display and remove the two numbers 
    // with the highest priority:
    vector<int>::iterator last = v.end();
    cout << *v.begin() << endl;            // 11
    pop_heap(v.begin(), last--);

    cout << *v.begin() << endl;            // 10
    pop_heap(v.begin(), last--);

    /* Note that the end of the heap is no longer indicated by
       v.end(), but by the iterator last. The range between these two
       values is undefined with regard to the heap properties of v. */

    // enter an 'important number' (99)
    *last = 99;
    push_heap(v.begin(), ++last);


    // enter an `unimportant number' (-1)
    *last = -1;
    push_heap(v.begin(), ++last);

    // display of the complete heap
    // (no complete ordering, only heap condition!)
    showSequence(v);   
    // display of all numbers by priority:
    while(last != v.begin())
    {
       cout << *v.begin() << ' ';
       pop_heap(v.begin(), last--);
    }
    cout << endl;      
    // generate new valid heap of all elements
    make_heap(v.begin(), v.end());                  

    // and sort
    sort_heap(v.begin(), v.end());
    // display of the completely sorted sequence
    showSequence(v);   
}
