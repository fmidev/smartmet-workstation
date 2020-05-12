// File k9/a2/matrix.h
#ifndef MATRIX_H
#define MATRIX_H

#include<checkvec.h>    // checked vector
#include<iostream>

// ********************* pientä käyttöohjetta **********************************
// HUOM!! voit käyttää seuraavia notaatioita mm. sijoittaessa arvoa tai 
// lukiessasia sitä:

// Matrix<float> matrix(rowCount, columnCount);
// float arvo = matrix[rowInd][columnInd];
// matrix[rowInd][columnInd] = arvo;
// matrix.Resize(newRowCount, newColumnCount); // Huom! Resize isolla kirjaimella!
// ********************* pientä käyttöohjetta **********************************


//using namespace std;

/* matrix as vector of vectors*/
template<class T>
class Matrix : public std::vector<std::vector<T> >
{
#ifdef _MSC_VER
public:
     typedef typename std::vector< std::vector< T > >::size_type size_type;
#endif

   protected:
     size_type rows,
               columns;

   public:
     Matrix(size_type x = 0, size_type y = 0)
     : std::vector<std::vector<T> >(x,
             std::vector<T>(y)), rows(x), columns(y)
     {}

    /* Thus, the Matrix class inherits from the std::vector class,
       with the data type of the vector elements now being described
       by a std::vector<T> template. With this, the matrix is a
       nested container that exploits the combination of templates
       with inheritance. The constructor initializes the implicit
       subobject of the base class type (std::vector<
       std::vector<T> >) with the correct size x. Exactly as with
       the standard vector container, the second parameter of the
       constructor specifies with which value each vector element is
       to be initialized. Here, the value is nothing else but a vector
       of type std::vector<T> and length y.*/

     size_type Rows() const {return rows; }

     size_type Columns() const {return columns; }

     void init(const T& Value)
     {
        for (size_type i = 0; i < rows; i++)
           for (size_type j = 0; j < columns ; j++)
               operator[](i)[j] = Value; // i.e. (*this)[i][j]}
     }


//     void Resize(size_type newColumns, size_type newRows, const T& Value)
     void Resize(size_type newRows, size_type newColumns, const T& Value)
     {
		columns = newColumns;
		rows = newRows;
		resize(rows);
        for (size_type i = 0; i < rows; i++)
            operator[](i).resize(columns, Value);
     }

	 void RemoveColumn(size_type theColumnIndex)
     {
        for (size_type i = 0; i < rows; i++)
            operator[](i).erase(&operator[](i)[theColumnIndex]);
		columns--;
     }

	 void RemoveRow(size_type theRowIndex)
     {
		iterator it = begin();
		it += theRowIndex;
		erase(it);
		rows--;
     }

    /* The index operator operator[]() is inherited from
       std::vector. Applied to i, it supplies a reference to the ith
       element of the (base class subobject) vector. This element is
       itself a vector of type std::vector<T>. It is again applied
       to the index operator, this time with the value j, which
       returns a reference to an object of type T, which is then
       assigned the value. */

     // create identity matrix
     Matrix<T>& I()    
     {
        for (size_type i = 0; i < rows; i++)
           for (size_type j = 0; j < columns ; j++)
               operator[](i)[j] = (i==j) ? T(1) : T(0);
        return *this;
     }

     // here, mathematical operators could follow ...
};   // class Matrix}

template<class T>
inline std::ostream& operator<<(std::ostream& s, const Matrix<T>& m )
{
    typedef Matrix<T>::size_type size_type;

    for (size_type i = 0; i < m.Rows(); i++)
    { 
       s << std::endl << i <<" :  ";
       for (size_type j = 0; j < m.Columns(); j++)
            s << m[i][j] <<" ";
    }

    s << std::endl;

    return s;
}





#endif

