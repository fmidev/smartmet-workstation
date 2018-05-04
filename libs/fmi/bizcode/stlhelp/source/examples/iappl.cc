// k3/iterator/iappl.cc
#include<iostream>
#include<list>
#include<vector>
#include<iterator>

using namespace std;

// calling implementation
template<class Iterator>
void showLastElements(Iterator last, 
#ifdef _MSC_VER
                         iterator_traits<Iterator>::distance_type n)
#else
                         iterator_traits<Iterator>::difference_type n)
#endif
{
    showLastElements(last, n, iterator_traits<Iterator>::iterator_category());
}

   /*  This
   function now calls the corresponding overloaded variation, where
   the selection at compile time is carried out by the parameter
   iterator_category() whose type corresponds to an iterator tag.
   Therefore, the third parameter is an iterator tag object
   constructed by calling its default constructor.
   */
using namespace std;

// first overloaded function
template<class Iterator, class Distance>
void showLastElements(Iterator last, Distance n,
                      bidirectional_iterator_tag)
{
    Iterator temp = last;
    advance(temp, -n);
    while(temp != last)
    {
       cout << *temp << ' ';
       ++temp;
    }
    cout << endl;
}

/* The bidirectional iterator does not allow random access and
   therefore no iterator arithmetic. Only the operators ++ and -- are
   allowed for moving. Therefore, advance() is used to go back n steps
   and then display the remaining elements. A random access iterator
   allows arithmetic, which makes the implementation of this case
   slightly easier: */

// second overloaded function
template<class Iterator, class Distance>
void showLastElements(Iterator last, Distance n,
                      random_access_iterator_tag)
{
    Iterator first = last - n;      // Arithmetik
    while(first != last) cout << *first++ << ' ';
    cout << endl;
}

int main( )
{
    list<int> L;                        // list
    for(int i=0; i < 10; i++) L.push_back(i);

    // call of 1st implementation
    showLastElements(L.end(), 5L);   // 5 long
 
    vector<int> v(10);                  // vector
#ifdef _MSC_VER
    for(i = 0; i < 10; i++) v[i] = i;
#else
    for(int i = 0; i < 10; i++) v[i] = i;
#endif
    
#ifndef _MSC_VER
    // call of 2nd implementation
    showLastElements(v.end(), 5);    // 5 int
#endif
}
