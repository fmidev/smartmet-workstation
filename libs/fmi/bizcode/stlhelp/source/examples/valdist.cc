// k3/iterator/valdist.cc
// Determination of value and distance types
#include<showseq.h>
#include<list>
#include<vector>
#include<iterator>

using namespace std;

template<class BidirectionalIterator>
void revert(BidirectionalIterator first,
            BidirectionalIterator last)
{
    revert(first, last, typename iterator_traits<BidirectionalIterator>
                                  ::iterator_category());

}

/* Reverting the order means that one element must be intermediately
   stored. For this, its type must be known. Following the well-proven
   scheme, the function calls the suitable implementation for the
   iterator type:
*/

template<class BidirectionalIterator>
void revert(BidirectionalIterator first,
            BidirectionalIterator last,
            bidirectional_iterator_tag)
{
    // Use of the difference type to calculate
    // the number of exchanges. The difference type
    // is derived from the iterator type:
    iterator_traits<BidirectionalIterator>::difference_type
         n = distance(first, last); 
    --n;

    while(n > 0)
    {
        // The value type is also derived from the iterator type:
         iterator_traits<BidirectionalIterator>::value_type
            temp = *first;      
        *first++ = *--last;
        *last = temp;
        n -= 2;
    }
}

// The second implementation uses arithmetic to compute
// the distance, which is possible only with
// random access iterators:

template<class RandomAccessIterator>
void revert(RandomAccessIterator first,
            RandomAccessIterator last,
            random_access_iterator_tag)
{
    // Use of the difference type to calculate
    // the number of exchanges. The difference type
    // is derived from the iterator type:
    iterator_traits<RandomAccessIterator>::difference_type
      n = last -first -1; // arithmetic! 

    while(n > 0)
    {
        // The value type is also derived from the iterator type:
         iterator_traits<RandomAccessIterator>::value_type
            temp = *first;      
        *first++ = *--last;
        *last = temp;
        n -= 2;
    }
}



/* At first sight, one could think that the algorithm could do without
   the distance type when comparing iterators and stop when first
   becomes >=  last. However, this assumption only holds when a >
   relation is defined for the iterator type at all. For a vector,
   where two pointers point to a continuous memory area, this is no
   problem. It is, however, impossible for containers of a different
   kind, such as lists or binary trees. */

int main()
{
    list<int> L;
    for(int i=0; i < 10; i++) L.push_back(i);
    revert(L.begin(), L.end());
    showSequence(L);

    vector<double> V(10);
    for(int i = 0; i < 10; i++) V[i] = i/10.;
    revert(V.begin(), V.end());
    showSequence(V);
}
