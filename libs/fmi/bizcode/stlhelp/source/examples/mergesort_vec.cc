// k5/mergesort_vec.cc  Simple example for mergesort()
#include<algorithm>
#include<showseq.h>
#include<vector>
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


int main()
{
    vector<int> v(20), buffer(20);
    Random whatAChance(1000);

    generate(v.begin(), v.end(), whatAChance);
    showSequence(v);     // random numbers

    // sort and display
    mergesort(v.begin(), v.end(), buffer.begin());
    showSequence(v);     // sorted Sequence
}
