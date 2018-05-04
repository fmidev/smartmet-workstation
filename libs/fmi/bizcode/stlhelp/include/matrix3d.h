// k9/a2/matrix3d.h
#ifndef MATRIX3D_H
#define MATRIX3D_H

#include"matrix.h"

/* 3D matrix as vector of 2D matrices*/
template<class T>
class Matrix3D : public checkedVector<Matrix<T> >
{
#ifdef _MSC_VER
     typedef typename checkedVector< checkedVector< T > >::size_type size_type;
#endif
  protected:
    size_type rows,
              columns,
              zDim;       // 3rd dimension

  public:
    Matrix3D(size_type x = 0, size_type y = 0,
             size_type z = 0)
    : checkedVector<Matrix<T> >(x, Matrix<T>(y,z)),
      rows(x), columns(y), zDim(z)
    {}

    /* The constructor initializes the base class subobject, a
       checkedVector of length c, whose elements are matrices. Each
       element of this vector is initialized with a (y,z) matrix. */

    size_type Rows() const { return rows;}

    size_type Columns() const { return columns;}

    size_type zDIM() const { return zDim;}

    /* The other methods resemble those of the Matrix class. The
       init() method needs only one loop over the outermost dimension
       of the three-dimensional matrix, because operator[](i) is of
       type &Matrix<T> and therefore Matrix::init() is called for each
       two-dimensional submatrix: */

    void init(const T& Value)
    {
       for (size_type i = 0; i < rows; i++)
           operator[](i).init(Value);
    }

     void Resize(size_type newRows, size_type newColumns, size_type newZDim, const T& Value)
     {
		zDim = newZDim;
		resize(newZDim);
        for (size_type i = 0; i < zDim; i++)
            operator[](i).Resize(newRows, newColumns, Value);
     }

    // here, mathematical operators could follow ...
};



#endif

