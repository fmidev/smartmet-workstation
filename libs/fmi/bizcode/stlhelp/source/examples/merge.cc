// k3/list/merge.cc
#include<list>
#include<iostream>

using namespace std;

void displayIntList(const list<int> & L)   // auxiliary function
{
    list<int>::const_iterator I = L.begin();
    while(I != L.end())
         cout << *I++ << ' ';
    cout << " size() ="
         << L.size() << endl;
}

int main( )
{
    list<int> L1, L2;

    // fill lists with sorted numbers
    for(int i = 0; i < 10; i++)
    {
        L1.push_back(2*i);       // even numbers
        L2.push_back(2*i+1);     // odd numbers
    }

    displayIntList(L1); // 0 2 4 6 8 10 12 14 16 18  size() =10
    displayIntList(L2); // 1 3 5 7 9 11 13 15 17 19  size() =10

    L1.merge(L2);                // merge
    displayIntList(L1);
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19  size() =20
    displayIntList(L2); //  size() =0
}
