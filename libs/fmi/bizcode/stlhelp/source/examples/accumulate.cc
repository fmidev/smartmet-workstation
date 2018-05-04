// k5/accumulate.cc
#include<iota.h>
#include<numeric>
#include<vector>
#include<iostream>

using namespace std;

int main()
{
    vector<int> v(10);
    iota(v.begin(), v.end(), 1);

    cout << "Sum = "
         << accumulate(v.begin(), v.end(), 0)    // 55
         << endl;

    cout << "Product = "
         << accumulate(v.begin(), v.end(), 1L,
                       multiplies<long>())            // 3628800
         << endl;
}
