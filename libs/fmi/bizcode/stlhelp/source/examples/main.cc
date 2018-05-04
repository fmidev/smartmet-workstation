// k9/a4/main.cpp
#include<iostream>
#define STL_map
#include"sparmat.h"  // class sparseMatrix, see below

using namespace std;

// example of a very big sparse matrix
int main()
{
    // ValueType double, IndexType long
    sparseMatrix<double, long> M(10000000,10000000);

    // Documentation
#ifdef STL_map
    cout << "STL map based ";
#else
    cout << "Hash map based ";
#endif    
    cout << "matrix with "
         << M.Rows()                  // 10000000
         << " rows and "
         << M.Columns()               // 10000000
         << " columns" << endl;


    // occupy some elements
    M[999998][777777]   = 999998.7777770;
    M[1][8035354]       = 123456789.33970;
    M[1002336][0]       = 444444444.1111;
    M[5000000][4900123] = 0.00000027251;


    // display of two elements
    cout.setf(ios::fixed|ios::showpoint);
    cout.precision(8);
    cout << "M[1002336][0]       = "
         <<  M[1002336][0]     << endl;


    cout << "M[5000000][4900123] = "
         <<  M[5000000][4900123]  << endl;
    cout << "Number of non-zero elements = "
         << M.size() << endl;


    cout << "max. number of non-zero elements = "
         << M.max_size() << endl;
    cout << "Output all non-zero elements via iterators\n";
    sparseMatrix<double, long>::iterator
            temp = M.begin();


    while(temp != M.end())
    {
        cout << "M["   << M.Index1(temp)    // i
             << "]["   << M.Index2(temp)    // j
             << "] = " << M.Value(temp)      // value
             << endl;
        ++temp;
    }
    cout << "check assignment M[i][j] = M[k][m]\n";
    M[2][3] = 3.0;
    cout << "M[2][3] = " << M[2][3] << " (expected: 3.0)" << endl;
    M[2][3] = M[5][5];
    cout << "M[2][3] = " << M[2][3] << " (expected: 0)" << endl;
    M[1][0] = M[5][5];
    cout << "M[1][0] = " << M[1][0] << " (expected: 0)" << endl;
    M[7][7] =    M[999998][777777]  ;
    cout << "M[7][7] = " << M[7][7] << " (expected: 999998.7777770)" << endl;
    M[9][17] =    M[7][7]  ;
    cout << "M[9][17] = " << M[9][17] << " (expected: 999998.7777770)" << endl;

   // .....
}
