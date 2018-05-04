// reverse_iterator.cc Reverse-Iterator
#include<iostream>
#include<vector>
#include<showseq.h>

int main()
{
  vector<int> V(10);
  for(size_t i=0; i< V.size();i++) V[i]=i;
  showSequence(V);
  cout << "reverse:" << endl;
  vector<int>::reverse_iterator RI=V.rbegin();
  while(RI != V.rend())
  {  cout << (*RI) << "  ";
     ++RI;
  }
}
