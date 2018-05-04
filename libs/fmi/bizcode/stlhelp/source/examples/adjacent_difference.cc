// k5/adjacent_difference.cc
#include<numeric>
#include<vector>
#include<iota.h>
#include<showseq.h>

using namespace std;

int main()
{
    vector<long> v(10), ad(10);
    iota(v.begin(), v.end(), 0);

    cout << "vector          = ";
    showSequence(v);                // 0 1 2 3 4 5 6 7 8 9

    cout << "Differences     = ";
    adjacent_difference(v.begin(), v.end(), ad.begin());
    showSequence(ad);               // 0 1 1 1 1 1 1 1 1 1

    // Fibonacci numbers
    vector<int> fib(16);
    fib[0] = 1;                     // initial value
    /* One initial value is sufficient here because the first value is
       written to position 1 (see formula  e_i =... in the book
       above and the result iterator which is shifted by one
       position). Therefore, after the first step of the algorithm,
       fib[1]} equals 1. */

    cout << "Fibonacci numbers  = "; // see below
    adjacent_difference(fib.begin(), fib.end()-1,
                  (fib.begin()+1), plus<int>());

    showSequence(fib);
    // 1 1 2 3 5 8 13 21 34 55 89 144 233 377 610 987
}
