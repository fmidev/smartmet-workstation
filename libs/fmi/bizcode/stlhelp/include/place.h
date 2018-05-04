// include/place.h
#ifndef PLACE_H
#define PLACE_H
#include<cmath>
#include<string>

using namespace std;

class Place
{
  public:
    Place() {};

    Place(long int ax, long int ay, const string& N = string(""))
    : x(ax), y(ay), Name(N)
    {}

    const string& readName() const { return Name;}
    unsigned long int X() const { return x;}
    unsigned long int Y() const { return y;}

    bool operator==(const Place& O) const
    {
        return x == O.x && y == O.y;
    }

    // for alphabetical ordering
    bool operator<(const Place& O) const
    {
        return Name < O.Name;
    }

  private:
    long int x, y;                    // coordinates
    string Name;                      // identifier
};

inline unsigned long int DistSquare(const Place& p, const Place& q)
{
   long int dx = p.X()-q.X();
   long int dy = p.Y()-q.Y();
   // (arithmetic overflow with large numbers is not checked)
   return dx*dx + dy*dy;
}

inline double Distance(const Place& p, const Place& q)
{
    return sqrt(double(DistSquare(p,q)));
}

inline ostream& operator<<(ostream& os, const Place& S)
{
    os << S.readName();
    return os;
}


#endif

