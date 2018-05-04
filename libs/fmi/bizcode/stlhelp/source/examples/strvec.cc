//  k3/vector/strvec.cc
// example for string-vector containers
#include<vector>
#include<iostream>
#include<string>

using namespace std;

int main()
{
    // a string vector of 4 elements
    vector<string> stringVec(4);
    stringVec[0] = "First";
    stringVec[1] = "Second";
    stringVec[2] = "Third";
    stringVec[3] = "Fourth";

    // vector increases size on demand
    stringVec.insert(stringVec.end(), string("Last"));
    cout << "size() = "
         << stringVec.size() << endl;           // 5

    // delete the element 'Second'
    vector<string>::iterator I = stringVec.begin();
    ++I;                                        // 2nd position
    cout << "erase: "
         << *I << endl;
    stringVec.erase(I);    // delete Second
    cout << "size() = "
         << stringVec.size() << endl;           // 4
    for(I = stringVec.begin(); I != stringVec.end(); ++I)
       cout << *I << endl;

    cout << "backwards with reverse_iterator:" << endl;
    vector<string>::reverse_iterator revI;

    for(revI = stringVec.rbegin();
        revI != stringVec.rend(); ++revI)
           cout << *revI << endl;
}
