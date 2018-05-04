// k5/find_if.cc
// there are two possibilities to find odd numbers
// with find_if, see below (uncomment next line)
//#define SECOND_POSSIBILITY

#include<algorithm>
#include<vector>
#include<iostream>

#ifdef SECOND_POSSIBILITY
#include<functional>  
#else
class odd
{
    public:
       // odd argument yields true}
       bool operator()(int x) { return x % 2;}
};
#endif

using namespace std;

void display(int x) {  cout << x << ' ';}


int main()
{
    vector<int> v(8);

    for(register size_t i = 0; i < v.size(); i++)
        v[i] = 2*i;                     // all even
    v[5] = 99;                          // an odd number

    // display
    for_each(v.begin(), v.end(), display);
    cout << endl;


    // search for odd number
    vector<int>::iterator iter
#ifdef SECOND_POSSIBILITY
        = find_if(v.begin(), v.end(), bind2nd(modulus<int>(),2));
#else
        = find_if(v.begin(), v.end(), odd());
#endif

    if(iter != v.end())
    {
        cout << "The first odd number ("
             << *iter
             << ") was found at position "
             << (iter - v.begin())
             << "." << endl;
    }
    else cout << "No odd number found." << endl;
}
