// k2/istring.cc
#include<iostream>
#include<fstream>
#include<iterator>
#include<string>

using namespace std;

int main( )
{
    // defining and opening of input file
    ifstream Source("istring.cc");
    istream_iterator<string> Pos(Source), End;

    /* The iterator End has no association with Source because all
       iterators of a type which indicate the past-end position are
       considered to be equal. */

    if(Pos == End)
       cout << "File not found!" << endl;
    else
       while(Pos != End)
       {
           cout << *Pos << endl;
           ++Pos;
       }
}
