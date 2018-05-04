// k1/a3.4/maint2.cc
// variation 3 : a container as STL template 
#include<iostream>
#include<vector>                    // STL

using namespace std;

// new type name 'Iterator', normally equal to 'pointer to {\tt int}'
typedef vector<int>::iterator Iterator;

// algorithm as template
template<class Iteratortype, class T>
Iteratortype find(Iteratortype begin, Iteratortype end,
                  const T& Value)
{
    while(begin != end              // iterator comparison
          && *begin != Value)       // object comparison
         ++begin;                   // next position
    return begin;
}

int main()
{
    const int Count = 100;
    vector<int> aContainer(Count);   // define container
    for(int i = 0; i < Count; i++)   // fill container with
        aContainer[i] = 2*i;         // even numbers

    int Number = 0;
    while(Number != -1)
    {
       cout << " enter required number (-1 = end):";
       cin >> Number;
       if(Number != -1)  
       {
         // use global find() defined above
         Iterator position =
            ::find(aContainer.begin(), // use of container methods:
                 aContainer.end(), Number);

         if (position != aContainer.end())
            cout << "found at position "
                 << (position - aContainer.begin()) << endl;
         else cout << Number << " not found!" << endl;
       }
    }
}
