// k9/a3/divmat.cc
#include"matrices.h"
#include<iostream>
using namespace std;

int main()
{
    fixMatrix<CMatrix<float,int> > MC(5,7);
    cout << " CMatrix " << endl;

    // fill rectangle
    for(int i = 0; i < MC.Rows(); i++)
      for(int j = 0; j < MC.Columns(); j++)
          // application of operator()()}:
          MC(i,j) = i + float(j/100.);

    // display rectangle
#ifdef _MSC_VER // Compiler Issue Q167748 PRB
    for(i = 0; i < MC.Rows(); i++)
#else
    for(int i = 0; i < MC.Rows(); i++)
#endif
    {
       for(int j = 0; j < MC.Columns(); j++)
          cout << MC(i,j) << ' ';
       cout << endl;
    }

// Example of a symmetric matrix
    fixMatrix<symmMatrix<float, int> > MD(5,5);
    cout << "\n symmMatrix " << endl; 

    // fill triangle
#ifdef _MSC_VER // Compiler Issue Q167748 PRB
    for(i = 0; i < MD.Rows(); i++)
#else
    for(int i = 0; i < MD.Rows(); i++)
#endif
      for(int j = i; j < MD.Columns(); j++)
          MD(i,j) = i + float(j/100.);

    // output square
#ifdef _MSC_VER // Compiler Issue Q167748 PRB
    for(i = 0; i < MD.Rows(); i++)
#else
    for(int i = 0; i < MD.Rows(); i++)
#endif
    {
       for(int j = 0; j < MD.Columns(); j++)
          cout << MD(i,j) << ' ';
       cout << endl;
    }
}
