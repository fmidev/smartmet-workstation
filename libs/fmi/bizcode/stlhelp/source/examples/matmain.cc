// k9/a2/matmain.cc
// Examples for matrix as nested container
#include"matrix3d.h"   // includes matrix

using namespace std;

int main()
{
    Matrix<float> a(3,4);
    a.init(1.0);            // set all elements = 1
    cout << " Matrix a:\n" << a;

    cout << "\n Identity matrix:\n" << a.I();

    /* As with a simple C array, the index operator can be chained,
       but with the advantage that the index is checked for limits:*/

    Matrix<float> b(4,5);

    for (register size_t i=0; i< b.Rows(); i++)
        for (register size_t j=0; j< b.Columns(); j++)
            b[i][j] = 1+i+(j+1)/10.;     // index operator

    cout << "\n Matrix b:\n" << b;

    Matrix3D<float> M3(2,4,5);

#ifdef _MSC_VER // Compiler Issue Q167748 PRB
    for (i=0; i< M3.Rows(); i++)
#else
    for (register size_t i=0; i< M3.Rows(); i++)
#endif
        for (register size_t j=0; j< M3.Columns(); j++)
             for (register size_t k=0; k< M3.zDIM(); k++)
              // chained index operator
              M3[i][j][k] = 10*(i+1)+(j+1)+(k+1)/10.;

    cout << "\n 3D matrix:\n";
#ifdef _MSC_VER
    for (i=0; i< M3.Rows(); i++)
#else
    for (register size_t i=0; i< M3.Rows(); i++)
#endif
       cout << "Submatrix " << i
            << ":\n"
            << M3[i];
    // ... and so on
}

