//  include/hmap.h  (= hash map)
#ifndef HASHMAP_H
#define HASHMAP_H

// implicit data structures
#include<vector>
#include<slist.h>

using namespace std;

// hash map class
template<class Key, class T, class hashFun>
class HMap
{
 public:
    typedef size_t size_type;
    typedef pair<const Key,T> value_type;

    // define more readable denominations
    typedef slist<value_type> list_type;
    typedef vector<list_type*> vector_type;

    /* The template parameter Key stands for the type of the key; T
       stands for the class of data associated to a key; and hashFun
       is the placeholder for the data type of the function objects
       used for address calculation. Below, a function object for
       address calculation is proposed, but any other one can be used
       as well. In analogy to map, value_type is the type of the
       elements that are stored in an HMap object. value_type is a
       pair consisting of a constant key and the associated data. */

    class iterator;
    // maintain compatibility with the STL:
    typedef iterator const_iterator;
    friend class iterator;

    /* The nested class iterator closely cooperates with HMap, so that
       both are mutually declared as friend. iterator is only supposed
       to allow forward traversal and therefore its category is
       defined as the standard forward_iterator_tag. An iterator object
       allows you to visit all elements of an HMap object one after
       the other. Neither an order nor a sorting is defined for the
       elements. The visiting order of the iterator is given by the
       implicit data structure (see below, operator++()). */

    class iterator
    {
      friend class HMap<Key, T, hashFun>;
      private:
       typename list_type::iterator current;
       typedef forward_iterator_tag iterator_category;
       size_type Address;
       const vector_type *pVec;

      public:
       iterator()
       : pVec(0)
       {}

       iterator(typename list_type::iterator LI,
                size_type A,  const vector_type *C)
       : current(LI), Address(A), pVec(C)
       {}

       /* The following operators allow you to check an HMap iterator
          in the condition part of if or while as to whether it is at
          all defined: */

       operator const void* () const
       {
           return pVec;
       }  

       bool operator!() const
       {
           return pVec == 0;
       }

       /* The operator for dereferencing occurs both in the const
          variation and in the non-const variation. Thus,
          dereferencing of an undefined iterator is punished with a
          program abort, which is a clear message to you to check the
          program that uses the iterator. */

       const value_type& operator*() const
       {
          assert(pVec);
          return *current;
       }  

       value_type& operator*()
       {
          assert(pVec);
          return *current;
       }

       /* The non-const variation is required to be able to modify
          data independently from the key. Modification of the key
          must be excluded because it requires a new address
          calculation. Constancy is guaranteed by the const
          declaration in the type definition of value_type.

          How does the HMap iterator move from one element to the
          other with operator++()? First, current is incremented:
         */

       iterator& operator++()
       {
          ++current;

          /* If after this, current points to a list element, a
             reference to the iterator is returned (see below: return
             *this. Otherwise, the end of the list is reached. */

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

       /* The last two methods compare two HMap iterators. Two
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
       that now the data and methods of the HMap class can follow:*/
  private:
    vector_type v;
    hashFun hf;
    size_type Count;

    /* Count is the number of stored pairs of keys and data, v is the
       vector whose elements are pointers to singly linked lists, and
       hf is the function object used for calculation of the hash
       address. */

    void construct(const HMap& S)
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
       operator. It constructs an HMap object out of another one and
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

    HMap(hashFun f = hashFun())
    : v(f.tableSize(),0), hf(f), Count(0)
    {}

    HMap(const HMap& S)
    {
       construct(S);
    }

    ~HMap()
    {
        clear();                       // see below
    }

    HMap& operator=(const HMap& S)
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
          if((*temp).first == k)
            return iterator(temp,address,&v); //found
          else ++temp;

        return iterator();
    }

    /* A map stores pairs of keys and associated data, where the first
       element (first) is the key and the second element ( second)
       contains the data. find() returns an iterator which can be
       interpreted as a pointer to a pair. In order to obtain the data
       belonging to a key, the index operator can be called with the
       key as argument: */

    T& operator[](const Key& k)
    {
         return (*find(k)).second;
    }

    /* If the key does not exist, that is, if find() returns an end
       iterator, a run time error occurs while dereferencing! (See the
       dereferencing operator).
       As in the STL, insert() returns a pair whose first part
       consists of the iterator that points to the found position. The
       second part indicates whether the insertion has taken place or
       not. */

    pair<iterator, bool> insert(const value_type& P)
    {
        iterator temp = find(P.first);
        bool inserted = false;

        if(!temp) // not present
        {
            size_type address = hf(P.first);
            if(!v[address])
               v[address] = new list_type;
            v[address]->push_front(P);
            temp = find(P.first); // redefine temp
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
       map that have a given key. In an HMap, this can at most be one
       element, but in an HMultimap, several elements might be
       affected. */

    // suitable for HMap and HMultimap
    size_type erase(const Key& k)
    {
       size_type deleted_elements = 0; // Count
       // calculate address
       size_type address = hf(k);
       if(!v[address])
          return 0;         // not present

       typename list_type::iterator temp =  v[address]->begin();

       /* In the following loop, the list is searched. An iterator
          called pos is used to remember the current position for the
          deletion itself. */

       while(temp != v[address]->end())
       {
          if((*temp).first == k)
          {
             typename list_type::iterator pos = temp++;

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

    /* We present here a couple of very simple methods. As opposed to
       other containers, max_size() does not indicate the maximum
       number of elements that can be stored in an HMap container,
       which is only limited by the capacity of the lists, but the
       number of available hash table entries. This information is
       more sensible, because the efficiency of an HMap depends on the
       occupation range alpha, assuming a good hash function. The
       occupation rate can easily be determined: alpha =
       size()/max_size(). */

    size_type size()     const { return Count;}

    size_type max_size() const { return v.size();}

    bool empty()         const { return Count == 0;}

    void swap(HMap& s)
    {
       v.swap(s.v);
       std::swap(Count, s.Count);
       std::swap(hf, s.hf);        
    }
};


#endif   // File hmap

