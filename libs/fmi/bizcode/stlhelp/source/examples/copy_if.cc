// k5/copy_if.cc
#include<iostream>
#include<vector>
#include<functional>
#include<showseq.h>
#include<iota.h>

template <class Iterator1, class Iterator2, class Predicate>
Iterator2 copy_if(Iterator1 iter, Iterator1 sourceEnd,
             Iterator2 destination, Predicate Pred) {
  while(iter != sourceEnd) {
    if(Pred(*iter))
        *destination++ = *iter;
    ++iter;
  }
  return destination;
}

int main()
{
  typedef vector<int> Container;
  Container V(20);
  iota(V.begin(), V.end(), 1);
  showSequence(V);

  // create empty Container:
  Container C;
  // append all elements > 10
  copy_if(V.begin(), V.end(),
	  back_inserter(C),
          bind2nd(greater<int>(),10));

  showSequence(C);
}
