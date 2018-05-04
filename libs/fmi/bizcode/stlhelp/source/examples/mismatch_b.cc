// k5/mismatch_b.cc
#include<algorithm>
#include<vector>
#include<iostream>
#include<cctype>
            
using namespace std;

class myCharCompare // tolerates upper/lower case spelling
{
    public:
      bool operator()(char x, char y)
      {
          // convert to lower case if needed
          return tolower(x) == tolower(y);
      }
};
            
int main()
{
    char Text1[] = "Algorithms and Data Structures";
    char Text2[] = "Algorithms and data Struktures"; // 2 errors

    // copy texts into vector (-1 because of null byte)
    vector<char> v1(Text1, Text1 + sizeof(Text1)-1);
    vector<char> v2(Text2, Text2 + sizeof(Text2)-1);

    // compare with iterator pair 'where'
    pair<vector<char>::iterator, vector<char>::iterator>
        where = mismatch(v1.begin(), v1.end(), v2.begin());
    /* Of course, direct use of the c-strings is also possible:
     pair<const char*, const char*>
        where = mismatch(Text1, Text1 + sizeof(Text1)-1, Text2);
     But here we want to use begin() and end().
    */

    if(where.first != v1.end())
    {
        cout << Text1 << endl << Text2 << endl;
        cout.width(1 + where.first - v1.begin());

        cout << "^";
        cout << "  first mismatch" << endl;
    }

    // compare with predicate
    where = mismatch(v1.begin(), v1.end(), v2.begin(),
                  myCharCompare());

    if(where.first != v1.end())
    {
        cout << Text1 << endl << Text2 << endl;
        cout.width(1 + where.first - v1.begin());
        cout << "^";
        cout << "  first mismatch at\n"
                "tolerance of upper/lower case spelling"
             << endl;
    }
}
