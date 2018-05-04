// k1/a3.4/maint1.cc
// variation 2: algorithm as template
template<class Iteratortype, class T>
Iteratortype find(Iteratortype begin, Iteratortype end,
                  const T& Value)
{
    while(begin != end        // iterator comparison
          && *begin != Value) // dereferencing and object comparison
         ++begin;             // next position
    return begin;
}

// rest as before:
#include<iostream>

// new type name {\tt Iterator} for `pointer to {\tt int}'
typedef int* Iterator;

using namespace std;

int main()
{
    const int Count = 100;
    int aContainer[Count];          // define container

    Iterator begin = aContainer;    // pointer to the beginning

    //  position after the last element
    Iterator end = aContainer + Count;

    // fill container with even numbers
    for(int i = 0; i < Count; i++)
       aContainer[i] = 2*i;

    int Number = 0;
    while(1)
    {
       cout << " enter required number (-1 = end):";
       cin >> Number;
       if(Number == -1)                 // program abort
          break;

       Iterator position = find(begin, end, Number);

       if (position != end)
          cout << "found at position "
               << (position - begin) << endl;
       else cout << Number << " not found!" << endl;
    }
}

