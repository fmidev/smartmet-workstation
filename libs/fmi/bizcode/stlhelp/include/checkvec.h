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
		 using std::vector;
         assert(index >=0 
             && index < static_cast<difference_type>(size()));
         return vector<T>::operator[](index);
      }

      const_reference operator[](difference_type index) const
      {
		 using std::vector;
         assert(index >=0 
             && index < static_cast<difference_type>(size()));
         return vector<T>::operator[](index);
      }
};



#endif

