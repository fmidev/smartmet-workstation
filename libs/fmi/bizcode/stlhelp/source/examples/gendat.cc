// generate file with random numbers
#include<cstdlib>
#include<fstream>
#include<iterator>
#include <iostream>

using namespace std;

int main()
{
    long howmany;
    cout << "howmany numbers shall be generated? ";
    cin >> howmany;
    ofstream dest("random.dat");
    ostream_iterator<long> Output(dest, "\n");
    for(long i = 0; i < howmany; i++)
          *Output++ = long(rand()) * long(rand());
}

