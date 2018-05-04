// k1/a3.4/maintstl.cc
// variation 4: STL algorithm 
#include<algorithm>
// ... rest as variation 3, but without find() template
#include<iostream>
#include<vector>  // STL

// new type name 'Iterator' for 'pointer to  int'
typedef int* Iterator;

using namespace std;

int main()
{
    const int Count = 100;
    vector<int> aContainer(Count);   // define container
    for(int i = 0; i < Count; i++)   // fill container with
        aContainer[i] = 2*i;         // even numbers

    int Number = 0;
    while(1)
    {
       cout << " enter required number (-1 = end):";
       cin >> Number;
       if(Number == -1)               // program abort
          break;

       // use of container methods:
       Iterator position =
          find(aContainer.begin(),
               aContainer.end(), Number);

       if (position != aContainer.end())
          cout << "found at position "
               << (position - aContainer.begin()) << endl;
       else cout << Number << " not found!" << endl;
    }
}
