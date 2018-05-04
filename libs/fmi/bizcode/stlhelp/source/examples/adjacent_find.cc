// k5/adjacent_find.cc
#include<algorithm>
#include<vector>
#include<iostream>
using namespace std;

void display(int x) { cout << x << ' ';}


int main()
{
    vector<int> v(8);

    for(register size_t i = 0; i < v.size(); i++)
        v[i] = 2*i;            // even
    v[5] = 99;                 // two identical adjacent elements
    v[6] = 99;

    // display
    for_each(v.begin(), v.end(), display);
    cout << endl;

    // find identical neighbors
    vector<int>::iterator iter
        = adjacent_find(v.begin(), v.end());

    if(iter != v.end())
    {
        cout << "The first identical adjacent numbers ("
             << *iter
             << ") were found at position "
             << (iter - v.begin())
             << "." << endl;
    }
    else
      cout << "No identical adjacent numbers found."
           << endl;
}
