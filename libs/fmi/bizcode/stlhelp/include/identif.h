// k2/identify/identif.h
#ifndef IDENTIF_H
#define IDENTIF_H
#include<iostream>
#include<string>
using namespace std;

class Identifier
{
  public:
   const string& toString() const { return theIdentifier;}
   friend istream& operator>>(istream&, Identifier&);
  private:
   string theIdentifier;
};

inline bool operator==(const Identifier& N1, const Identifier& N2)
{
    return N1.toString() == N2.toString();
}

inline bool operator<(const Identifier& N1, const Identifier& N2)
{
    return N1.toString() < N2.toString();
}

ostream& operator<<(ostream&, const Identifier&);

#endif
