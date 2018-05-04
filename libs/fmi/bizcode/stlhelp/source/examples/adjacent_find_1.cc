// k5/adjacent_find_1.cc
#include<algorithm>
#include<vector>
#include<iostream>
using namespace std;

void display(int x) {  cout << x << ' ';}

class doubled
{
    public:
       bool operator()(int a, int b) { return (b == 2*a);}
};


int main()
{
    vector<int> v(8);

    for(register size_t i = 0; i < v.size(); i++)
        v[i] = i*i;
    v[6] = 2 * v[5];       // twice as large successor

    // display
    for_each(v.begin(), v.end(), display);
    cout << endl;

    // search for twice as large successor
    vector<int>::iterator iter
        = adjacent_find(v.begin(), v.end(), doubled());

    if(iter != v.end())
    {
        cout << "The first number ("
             << *iter
             << ") with a twice as large successor"
                " was found at position "
             << (iter - v.begin())
             << "." << endl;
    }
    else cout << "No number with twice as large "
                 "successor found." << endl;
}
