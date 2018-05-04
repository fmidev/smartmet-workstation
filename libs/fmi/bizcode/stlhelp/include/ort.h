// {\em cppbuch/include/ort.h}
#ifndef ort_h
#define ort_h ort_h
#include<string>
#include<cmath>                 // wegen {\tt sqrt()}
#include<iostream>

#include<cstdlib>
#include<cctype>
using namespace std;

class Ort
{
    public:
      // Typumwandlungskonstruktor
      // Format: 2 Folgen von Ziffern
      explicit Ort(const string& str)
      {
         unsigned int pos = 0;         // Position einer Ziffer
         for(int j = 0; j < 2; j++) // für jede Koordinate
         {
            // erste Ziffer suchen
            while(pos < str.size())
            {
               if(isdigit(str.at(pos)))
                 break;
               else ++pos;
            }
            assert(pos < str.size()); // Ziffer gefunden?
            // Zahl bilden
            int Koordinate = 0;
            while(pos < str.size() && isdigit(str.at(pos)))
            {
              Koordinate = 10*Koordinate + str.at(pos) - '0';
              ++pos;
            }
            switch(j)
            {
              case 0: xKoordinate = Koordinate; break;
              case 1: yKoordinate = Koordinate;
            }
         }
      }
      Ort(int einX = 0, int einY = 0)
      : xKoordinate(einX), yKoordinate(einY)
      {}

      int X() const { return xKoordinate;}
      int Y() const { return yKoordinate;}
      void aendern(int x, int y)
      {
          xKoordinate = x;
          yKoordinate = y;
      }
    private:
      int xKoordinate,
          yKoordinate;
};

// globale Funktionen

 // Berechnung der Entfernung zwischen zwei Orten
inline double Entfernung(const Ort &Ort1, const Ort &Ort2)
{
    double dx = static_cast<double>(Ort1.X() - Ort2.X());
    double dy = static_cast<double>(Ort1.Y() - Ort2.Y());
    return sqrt(dx*dx + dy*dy);
}


// Anzeige auf der Standardausgabe
inline void anzeigen(const Ort &O)
{
    cout << '(' << O.X() << ", " << O.Y() << ')';
}

// Gleichheitsoperator
inline bool operator==(const Ort &Ort1, const Ort &Ort2)
{
  return  (Ort1.X() == Ort2.X()
        && Ort1.Y() == Ort2.Y());
}

// Überladen des Ausgabeoperators zur Ausgabe eines Ortes
inline ostream& operator<<(ostream &os, const Ort &einOrt)
{
    os << " (" << einOrt.X() << ", " << einOrt.Y() << ')';
    return os;
}
#endif     //  ort\_h
