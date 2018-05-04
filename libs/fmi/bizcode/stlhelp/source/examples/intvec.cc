//  k3/vector/intvec.cc
// example for int-vector container
#include<vector>
#include<iostream>

using namespace std;

int main()
{
    // an int vector of 10 elements
    vector<int> intV(10);
    register size_t i;
    for(i = 0; i < intV.size(); i++)
       intV[i] = i;                 // fill vector, random access

    // vector increases on demand
    intV.insert(intV.end(), 100);   // append the number 100

    // use as array
    for(i = 0; i < intV.size(); i++)
       cout << intV[i] << endl;

    // use with an iterator
    vector<int>::iterator I;

    for(I = intV.begin(); I != intV.end(); ++I)
       cout << *I << endl;
    
    vector<int> newV(20, 0); // all elements are 0
    
    cout << " newV = ";

    for(i = 0; i < newV.size(); i++)
       cout << newV[i] << ' ';

    // swap() is a very fast method for
    // swapping two vectors
    newV.swap(intV);

    cout << "\n newV after swapping = ";
    for(i = 0; i < newV.size(); i++)
       cout << newV[i] << ' ';     // old contents of intV

    cout << "\n\n intV        = ";
    for(i = 0; i < intV.size(); i++)
       cout << intV[i] << ' ';     // old contents of newV
    cout << endl;
}
