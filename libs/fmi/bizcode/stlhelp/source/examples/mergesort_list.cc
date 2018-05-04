// k5/mergesort_list.cc  // with list instead of vector
#include<algorithm>
#include<showseq.h>
#include<list>
#include<myrandom.h>

using namespace std;

template<class InputIterator, class OutputIterator>
void mergesort(InputIterator first,
               InputIterator last,
               OutputIterator result)
{
    iterator_traits<InputIterator>::difference_type 
                        n    = distance(first, last),
                        Half = n/2;
    InputIterator Middle = first;
    advance(Middle, Half);

    if(Half > 1)            // sort left half, if needed
        mergesort(first, Middle, result);  // recursion

    if(n - Half > 1)        // sort right half if needed
    {
        OutputIterator result2 = result;
        advance(result2, Half);
        mergesort(Middle, last, result2);  // recursion
    }

    // merge both halves and copy back the result
    OutputIterator End =
          merge(first, Middle, Middle, last, result);
    copy(result, End, first);
}


int main()       // with list instead of vector
{
    list<int> v;
    for(register size_t i = 0; i < 20; i++)
        v.push_front(0);            // create space
    Random whatAChance(1000);
    generate(v.begin(), v.end(), whatAChance);
    showSequence(v);     // random numbers

    list<int> buffer = v;
    mergesort(v.begin(), v.end(), buffer.begin());
    showSequence(v);     // sorted sequence
}
