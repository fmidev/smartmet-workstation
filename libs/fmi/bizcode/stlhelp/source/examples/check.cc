// check sequence in file random.dat (after sorting)
#include<cstdlib>
#include<fstream>
#include<iterator>
#include <iostream>

using namespace std;

int main()
{
    ifstream Source("random.dat");
    istream_iterator<long> numberIterator(Source),
                           theEnd, temp = numberIterator;
    bool ok = true;
    enum {undef, up, down} theSequence = undef;

    while(numberIterator != theEnd) 
    {
//        cout << *numberIterator << ' ' << *temp << endl;
        if(   (theSequence == up && *numberIterator < *temp)
           || (theSequence == down  && *temp < *numberIterator))
        {
            cerr << "Error: " << *numberIterator << endl;
            ok = false;
        }
        temp = numberIterator;
        numberIterator++;
        if(theSequence == undef)
            if(*numberIterator < *temp)
            {   theSequence = down;
                cout << "descending sequence, ";
            }
            else
            {   theSequence = up;
                cout << "ascending sequence, ";
            }
    }
    if(ok) cout << "check ok\n";
    else cout << "found error in sequence!\n";
}

