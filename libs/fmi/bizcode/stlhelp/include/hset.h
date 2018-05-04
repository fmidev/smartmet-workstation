//  include/hset.h  (= hash set)
#ifndef HASHSET_H
#define hashset hashset

// implicit data structures
#include<vector>
#include<slist.h>

#include<algorithm>      // swap()
#include<setalgo.h>

using namespace std;

// hash set class
template<class Key, class hashFun>
class HSet
{
 public:
    typedef size_t size_type;
    typedef Key value_type;

    // define more readable denominations
    typedef slist<value_type> list_type;
    typedef vector<list_type*> vector_type;

    class iterator;
    typedef iterator const_iterator;
    friend class iterator;

    class iterator
    {
      friend class HSet<Key, hashFun>;
      private:
       typename list_type::iterator current;
       typedef forward_iterator_tag iterator_category;
       size_type Address;
       const vector_type *pVec;

      public:
       iterator()
       :  pVec(0)
       {}

       iterator(typename list_type::iterator LI,
                size_type A,  const vector_type *C)
       : current(LI), Address(A), pVec(C)
       {}

       operator const void* () const
       {
           return pVec;
       }  

       bool operator!() const
       {
           return pVec == 0;
       }

       const value_type& operator*() const
       {
          assert(pVec);
          return *current;
       }  
       /* There is no non-const function, because a *direct* change of
       an element cannot be allowed because of the necessary new
       address calculation. */

       iterator& operator++()
       {
          ++current;

          if(current == (*pVec)[Address]->end())
          {

            /* At this point, one address after the other is checked
               in the vector, until either a list entry is found or
               the end of the vector is reached. In the latter case,
               the iterator becomes invalid, because it can only move
               forward. In order to exclude further use, pVec is set
               to 0: */

             while(++Address < pVec->size())
                if((*pVec)[Address])
                {
                   current = (*pVec)[Address]->begin();
                   break;
                }

             if(Address == pVec->size()) // end of vector reached
                pVec = 0;
          }
          return *this;
       }

       iterator operator++(int)
       {
           iterator temp = *this;
           operator++();
           return temp;
       }

       /* The last two methods compare two HSet iterators. Two
          undefined or invalidated iterators are always considered as
          equal: */

       bool operator==(const iterator& x) const
       {
           return current == x.current
                || !pVec && !x.pVec;
       }  

       bool operator!=(const iterator& x) const
       {
          return !operator==(x);
       }
    }; // iterator

    /* With this, the nested class iterator is concluded, so
       that now the data and methods of the HSet class can follow:*/
  private:
    vector_type v;
    hashFun hf;
    size_type Count;

    /* Count is the number of stored pairs of keys and data, v is the
       vector whose elements are pointers to singly linked lists, and
       hf is the function object used for calculation of the hash
       address. */

    void construct(const HSet& S)
    {
        hf = S.hf;
        v = vector_type(S.v.size(),0);
        Count = 0;

        // begin(), end(), insert(): see below
        iterator t = S.begin();
        while(t != S.end())
           insert(*t++);
    }

    /* The construct() function is an auxiliary function which is
       called both in the copy constructor and in the assignment
       operator. It constructs an HSet object out of another one and
       has been extracted in order to avoid code duplication in copy
       constructor and assignment operator. */

  public:
    iterator begin() const
    {
        size_type adr = 0;
        while(adr < v.size())
        {
           if(!v[adr])    // found nothing?
             adr++;       // continue search
           else
             return iterator(v[adr]->begin(), adr, &v);
        }
        return iterator();
    }

    iterator end() const
    {
        return iterator();
    }

    HSet(hashFun f = hashFun())
    : v(f.tableSize(),0), hf(f), Count(0)
    {}

    HSet(const HSet& S)
    {
       construct(S);
    }

    ~HSet()
    {
        clear();                       // see below
    }

    HSet& operator=(const HSet& S)
    {
        if(this != &S)
        {
           clear();
           construct(S);
        }
        return *this;
    }

    /* clear() uses delete to call the destructor of each list
       referred to by a vector element. Subsequently, the vector
       element is marked as unoccupied.*/

    void clear()
    {
         for(size_t i = 0; i < v.size(); i++)
            if(v[i])                   // does list exist?
            {
                delete v[i];
                v[i] = 0;
            }
         Count = 0;
    }

    /* In the following find() and insert() functions, the sought
       address within the vector v is calculated directly by means of
       the hash function object. If the vector element contains a
       pointer to a list, the list is searched in find() by means of
       the list iterator temp until an element with the correct key is
       found or the list has been completely processed: */

    iterator find(const Key& k) const
    {
        size_type address = hf(k);     // calculate address
        if(!v[address])
           return iterator();          // not existent
        typename list_type::iterator temp =  v[address]->begin();

        // find k in the list
        while(temp != v[address]->end())
          if(*temp == k)
            return iterator(temp,address,&v); //found
          else ++temp;

        return iterator();
    }

/* Subscript operator only in map, but not in set
    T& operator[](const Key& k)
*/

    /* If the key does not exist, that is, if find() returns an end
       iterator, a run time error occurs while dereferencing! (See the
       dereferencing operator).
       As in the STL, insert() returns a pair whose first part
       consists of the iterator that points to the found position. The
       second part indicates whether the insertion has taken place or
       not. */

    pair<iterator, bool> insert(const value_type& P)
    {
        iterator temp = find(P);
        bool inserted = false;

        if(!temp) // not present
        {
            size_type address = hf(P);
            if(!v[address])
               v[address] = new list_type;
            v[address]->push_front(P);
            temp = find(P); // redefine temp
            inserted = true;
            Count++;
        }
        return make_pair(temp, inserted);
    }

    /* After the insertion, temp is redefined, because the iterator at
       first does not point to an existing element. The known
       auxiliary function makepair() generates a pair object to be
       returned. */

    void erase(iterator q)
    { 
        /* If the iterator is defined at all, the element function
           erase() of the associated list is called. Subsequently, the
           list is deleted, provided it is now empty, and the vector
           element to which the list is attached, is set to 0. */

       if(q.pVec)             // defined?
       {
          v[q.Address]->erase(q.current);

          if(v[q.Address]->empty())
          {
             delete v[q.Address];
             v[q.Address] = 0;
          }
          Count--;
       }
    }

    /* Sometimes, one would probably like to delete all elements of a
       map that have a given key. In an HSet, this can at most be one
       element, but in an HMultiset, several elements might be
       affected. */

    // suitable for HSet and HMultiset
    size_type erase(const Key& k)
    {
       size_type deleted_elements = 0; // Count
       // calculate address
       size_type address = hf(k);
       if(!v[address])
          return 0;         // not present

       list_type::iterator temp =  v[address]->begin();

       /* In the following loop, the list is searched. An iterator
          called pos is used to remember the current position for the
          deletion itself. */

       while(temp != v[address]->end())
       {
          if(*temp == k)
          {
             list_type::iterator pos = temp++;

             v[address]->erase(pos);
             // pos is now undefined

             Count--;
             deleted_elements++;
          }
          else ++temp;
       }

       /* The temporary iterator temp is advanced in both branches of
          the if instruction. The operation ++ cannot be extracted in
          order to save the else, because temp would then be identical
          with pos which is undefined after the deletion, and a
          defined ++ operation would no longer be possible. */

       // delete hash table entry if needed
       if(v[address]->empty())
       {
          delete v[address];
          v[address] = 0;
       }
       return deleted_elements;
    }

    size_type size()     const { return Count;}

    size_type max_size() const { return v.size();}

    bool empty()         const { return Count == 0;}

    void swap(HSet& s)
    {
       v.swap(s.v);
       std::swap(Count, s.Count);
       std::swap(hf, s.hf);        
    }

     // from here operators for set operations
     HSet& operator+=(const HSet& S)     // union
     {
        // Union from setalgo is not used,
        // so no copy of *this is generated
        typename HSet::iterator i = S.begin();
        while(i != S.end()) insert(*i++);
        return *this;
     }

     HSet& operator*=(const HSet& S)     // intersection
     {
        Intersection(*this, S, *this);
        return *this;
     }

     HSet& operator-=(const HSet& S)     // difference
     {
        Difference(*this, S, *this);
        return *this;
     }

     HSet& operator^=(const HSet& S)     // symm. difference
     {
        Symmetric_Difference(*this, S, *this);
        return *this;
     }

     /* The binary operators are not global in order to avoid
     name clashes with others (no namespace use)
     */
     HSet operator+(const HSet& S)     // union
     {
        HSet result = *this;
        return result += S;
     }

     HSet operator*(const HSet& S)     // intersection
     {
        HSet result = *this;
        return result *= S;
     }


     HSet operator-(const HSet& S)     // difference
     {
        HSet result = *this;
        return result -= S;
     }

     HSet operator^(const HSet& S)     // symm. difference
     {
        HSet result = *this;
        return result ^= S;
     }


};


#endif   // File hset

