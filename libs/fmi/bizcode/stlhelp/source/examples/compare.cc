// k1/a6/compare.cc  --  Demonstration of comparison objects
#include<functional>      //  less<T>
#include<iostream>
#include<cstdlib>         // abs()

struct absoluteLess
{
       bool operator()(int x, int y) const
       {
          return (abs(x) < abs(y));
       }
};

/* The following sorting routine no longer uses the < operator in the
   if-condition, but the comparison object whose  operator()(...) is
   called.*/

template<class T, class CompareType>
void bubble_sort(T* array, int Count,
                 const CompareType& Compare)
{
    for(int i = 0; i < Count; i++)
        for(int j = i+1; j < Count; j++)
        if (Compare(array[i], array[j])) // functor call
        {   // exchange
            const T temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
}

using namespace std;

// Auxiliary procedure for display
void Display(int *Array, int N)   
{
    for(int i = 0; i < N; i++)
    {   cout.width(7);
        cout << Array[i];
    }
    cout << endl;
}

int main()
{
    int Table[] = {55, -7, 3, -9, 2, -9198, -937, 1, 473};
    const int num = sizeof(Table)/sizeof(int);

    /* The comparison object normalCompare is of the standard class
       type less, which has been made known with #include<functional>.
       less compares with the < -operator. */

    // Variation 1
    less<int> normalCompare;
    bubble_sort(Table, num, normalCompare);
    cout << "sorted by size:" << endl;
    Display(Table, num);
       //  473 55  3  2  1  -7  -9   -937  -9198

    /* Alternatively, you can do without explicit creation of an
       object when the constructor is called in the argument list. */

    // Variation 2
    bubble_sort(Table, num, less<int>());

    /* The comparison object is of the user-defined type absoluteLess}
       which does not only use the < operator, but also the abs()
       function, and which in principle can be arbitrarily complex. It
       is a big advantage that the bubble_sort algorithm and its
       interface do not have to be changed. */

    cout << "sorted by absolute value:" << endl;
    bubble_sort(Table, num, absoluteLess());
    Display(Table, num);
       //   -9198 -937  473 55  -9 -7  3  2  1
}   // End of example
