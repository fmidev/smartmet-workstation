// k5/partialsum.cc
#include<numeric>
#include<vector>
#include<showseq.h>
#include<iota.h>

using namespace std;

int main()
{
    vector<long> v(10), ps(10);
    iota(v.begin(), v.end(), 1); // natural numbers

    cout << "vector            = ";
    showSequence(v);          // 1 2 3 4 5 6 7 8 9 10

    partial_sum(v.begin(), v.end(), ps.begin());
    cout << "Partial sums    = ";
    showSequence(ps);         // 1 3 6 10 15 21 28 36 45 55

    // Sequence of factorials
    cout << "Partial products = "; 
    partial_sum(v.begin(), v.end(), v.begin(), multiplies<long>());
               // 1 2 6 24 120 720 5040 40320 362880 3628800
    showSequence(v);
}
