#ifndef MATRICES_H
#define MATRICES_H

#include<cassert>             // used in subclasses
#include<vector>

using namespace std;

template<class VType, class IType>
class MatrixSuperClass
{
  public:
    // public type definitions
    typedef VType ValueType;
    typedef IType IndexType;
    // define vector as container type
    typedef vector<ValueType> ContainerType;

    IndexType Rows()  const { return rows;}

    IndexType Columns() const { return columns;}

  protected:
    MatrixSuperClass(IndexType z, IndexType s, ContainerType &Cont)
    :  C(Cont), rows(z), columns(s)
    {}

    ContainerType &C;
  private:
    IndexType rows, columns;
};

template<class ValueType, class IndexType>
class CMatrix : public MatrixSuperClass<ValueType, IndexType>
{
  public:
   CMatrix(IndexType r, IndexType c, ContainerType& C)
   : MatrixSuperClass<ValueType,IndexType>(r,c,C)
   {}

   // The size of the vector can easily be calculated:
   IndexType howmany() const
   {
       return Rows()*Columns();
   }

    /* The position of an element with the indices r and c is
       calculated in the where() method. Checking of index limits in
       the vector container is only possible to a limited extent,
       because the check could only be carried out against the entire
       length (Rows times Columns). Therefore, a std::vector is not
       sufficient, and the index check is carried out directly inside
       the where() method. */

   ValueType& where(IndexType r, IndexType c) const
   {
      assert(r < Rows() && c < Columns());
      return C[r * Columns() + c];
   }
};

template<class ValueType, class IndexType>
class FortranMatrix : public MatrixSuperClass<ValueType, IndexType>
{
  public:
   FortranMatrix(IndexType r, IndexType c, ContainerType& C)
   : MatrixSuperClass<ValueType, IndexType>(r,c,C)
   {}

   IndexType howmany() const
   {
       return Rows()*Columns();
   }

    /* In the address calculation, rows and columns are exchanged in
       contrast to the CMatrix class: */

   ValueType& where(IndexType r, IndexType c) const
   {
      assert(r < Rows() && c < Columns());
      return C[c * Rows() + r];
   }
};

template<class ValueType, class IndexType>
class symmMatrix : public MatrixSuperClass<ValueType, IndexType>
{
  public:
   symmMatrix(IndexType r, IndexType c, ContainerType& C)
   : MatrixSuperClass<ValueType, IndexType>(r,c,C)
   {
      assert(r == c);   // matrix must be quadratic
   }

   // reduced memory consumption thanks to symmetry
   IndexType howmany() const
   {
       return Rows()*(Rows()+1)/2;
   }

   // The symmetry is exploited
   ValueType& where(IndexType r, IndexType c) const
   {
      assert(r < Rows() && c < Columns());
      if (r <= c) return C[r + c*(c+1)/2];
      else        return C[c + r*(r+1)/2];
   }
};


template<class MatrixType>
class fixMatrix
{
  public:
    typedef typename MatrixType::ValueType ValueType;
    typedef typename MatrixType::IndexType IndexType;
    typedef typename MatrixType::ContainerType ContainerType;

    fixMatrix(IndexType z, IndexType s)
    : theMatrix(z,s,C), C(theMatrix.howmany())
    {}

    IndexType Rows()  const { return theMatrix.Rows();}

    IndexType Columns() const { return theMatrix.Columns();}

    ValueType& operator()(IndexType z, IndexType s)
    {
       return theMatrix.where(z,s);
    }
    // ... further methods and operators

  private:
    MatrixType theMatrix;        // determines memory layout
    ContainerType C;             // container C
};


#endif

