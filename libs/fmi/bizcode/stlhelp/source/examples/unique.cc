// k5/unique.cc
#include<iostream>
#include<algorithm>
#include<vector>

using namespace std;

int main()
{
    vector<int> v(20);
    // sequence with identical adjacent elements
    for(register size_t i = 0; i < v.size(); i++)
        v[i] = i/3;

    ostream_iterator<int> Output(cout, " ");
    copy(v.begin(), v.end(), Output); 
                             // 0 0 0 1 1 1 2 2 2 3 3 3 4 4 4 5 5 5 6 6
    cout << endl;

    vector<int>::iterator last = unique(v.begin(), v.end());
    copy(v.begin(), last, Output); // 0 1 2 3 4 5 6
    cout << endl;
}
