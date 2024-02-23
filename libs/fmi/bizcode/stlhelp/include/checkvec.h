// include/checkvec : vector class with checked limits
#ifndef CHECKVEC_H 
#define CHECKVEC_H
#include<cassert>
#include<vector>

//using namespace std;

template<class T>
class checkedVector : public std::vector<T>   // inherit from std::vector<T>
{
   public:
       // Accessing std::vector<T> types in C++17 must be done more directly
       using typename std::vector<T>::size_type;
       using typename std::vector<T>::difference_type;
       using typename std::vector<T>::iterator;
       using typename std::vector<T>::reference;
       using typename std::vector<T>::const_reference;

      // type names like iterator etc. are also inherited
      checkedVector()
      {}

      checkedVector(size_type n, const T& value = T())
      : std::vector<T>(n, value)
      {}

      checkedVector(iterator i, iterator j)
      : std::vector<T>(i, j)
      {}

      reference operator[](difference_type index)
      {
         assert(index >=0 
             && index < static_cast<difference_type>(size()));
         return vector<T>::operator[](index);
      }

      const_reference operator[](difference_type index) const
      {
         assert(index >=0 
             && index < static_cast<difference_type>(size()));
         return vector<T>::operator[](index);
      }
};



#endif

