// k9/a4/sparse1.cc
#include<map>
#include<iostream>
using namespace std;

// matrix declaration
typedef map<long, double> doubleRow;
typedef map<long, doubleRow> SparseMatrix;

    /* The first index operator applied to a SparseMatrix returns a
       row on which the second index operator is applied, as shown in
       the program: */

int main()
{
    SparseMatrix M;         // see declaration above
    M[1][1] = 1.0;
    M[1000000][1000000] = 123456.7890;
    cout.setf(ios::fixed);
    cout.precision(6);

    cout << M[1][1] << endl;                    // 1.000000
    cout << M[1000000][1000000] << endl;        // 123456.789000
    cout << "M.size() :" << M.size() << endl;   // 2

    /* Unfortunately, this very simple form of a sparse matrix has a
       couple of `minor blemishes'. Access to a not yet defined
       element creates a new one: */

    cout << M[0][0] << endl;
    cout << "M.size() :" << M.size() << endl;   // 3

    /* This is not desirable, since the point is saving storage space.
       The next flaw is the uncontrolled access to undesired
       positions, once again with the effect of generating additional
       elements: */

    cout << M[-1][0] << endl;                   // index error
    cout << "M.size() :" << M.size() << endl;   // 4
}
