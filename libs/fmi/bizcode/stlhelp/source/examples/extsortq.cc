// k10/extsortq.cc
// Sorting of a large file
#include<functional>   // greater<>
#include <iostream>
#include"extsortq.h"            // with priority-queue

using namespace std;

int main()
{
//   less<long> Comparison;               // descending
    greater<long> Comparison;             // ascending
    istream_iterator<long> suitable_iterator;

    cout << externalSorting(
                suitable_iterator,  // type of file
                "random.dat",       // file name
                "\n",               // separator
                Comparison)         // sorting criterion
         << " sorting runs" << endl;
}
