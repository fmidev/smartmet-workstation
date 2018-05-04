// k1/a4/mainstl2.cc
#include<algorithm>    // contains  find()
#include<iostream>
#include"slist.h"        // user-defined list class 

using namespace std;

int main()
{
    const int Count = 100;
    slist<int> aContainer;          // define the container

    /* Change of order because the container is filled from the front!
       This example differs from those in 1.3.4, because elements are
       inserted, i.e., the container is expanded as needed. */

    for(int i = Count; i >= 0; i--)  // fill the container with
       aContainer.push_front(2*i);   // even numbers

    int Number = 0;
    while(true)
    {
       cout << " enter required number (-1 = end):";
       cin >> Number;
       if(Number == -1)              // program abort
          break;

       // use of container methods:
       slist<int>::iterator Position =
           find(aContainer.begin(),
                aContainer.end(), Number);

       if (Position != aContainer.end())
          cout << "found at position "
               << (Position - aContainer.begin()) << endl;
       else cout << Number << " not found!" << endl;
    }
}
