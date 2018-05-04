// k5/innerproduct.cc
#include<numeric>
#include<vector>
#include<cmath>
#include<iota.h>
#include<iostream>

// functor for calculating the square of a difference (see below)
template<class T>
struct difference_square
{
    T operator()(const T& x, const T& y)
    {
        T d = x - y;
        return d*d;
    }
};

using namespace std;

int main()
{
    int Dimension = 4;
    const int ONE = 1;
    vector<int> v(Dimension,ONE);

    cout << "Length of vector v = "
         << sqrt((double) inner_product(v.begin(), v.end(),
                                        v.begin(), 0))
         << endl;

    /* In order to show the application of other mathematical
       operators, the following part of the example calculates the
       distance between two points. */

    // 2 points p1 and p2
    vector<double> p1(Dimension,1.0),    // unit vector
                   p2(Dimension);

    iota(p2.begin(), p2.end(), 1.0);     // arbitrary vector

    cout << "Distance between p1 and p2 = "
         << sqrt((double) inner_product(p1.begin(), p1.end(),
                               p2.begin(), 0,
               plus<int>(), difference_square<int>()))
         << endl;
}
