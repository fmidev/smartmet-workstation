// include/hashfun.h
// Standard function object
#ifndef HASH_FUNCTION_H
#define HASH_FUNCTION_H

using namespace std;

template<class T>
class HashFun
{
   public:
    HashFun(long prime=1009) : tabSize(prime) {}
    long operator()(T p) const
    {
       return long(p) % tabSize;
    }
    long tableSize() const { return tabSize;}
   private:
    long tabSize;
};


#endif

