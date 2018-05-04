// k5/set_algorithms.cc
#include<algorithm>
#include<set>
#include<showseq.h>

using namespace std;

int main ()
{
    int v1[] = {1, 2, 3, 4};
    int v2[] = {0, 1, 2, 3, 4, 5, 7, 99, 13};
    int v3[] = {-2, 5, 12, 7, 33};

    // initialize sets with the vector contents
    // default comparison object: less<int>()
    // (implicit automatic sorting)          
    set<int> s1(v1, v1 + 4);
    set<int> s2(v2, v2 + 8);
    set<int> s3(v3, v3 + 5);
    set<int> Result;          // empty set (s1, s2, s3 as above)

    if(includes(s2.begin(), s2.end(), s1.begin(), s1.end()))
    {
        showSequence(s1);              // 1 2 3 4
        cout << " is a subset of ";
        showSequence(s2);              // 0 1 2 3 4 5 7 99
    }

    set_union(s1.begin(), s1.end(),
              s3.begin(), s3.end(),
              inserter(Result, Result.begin()));      

    showSequence(s1);                  // 1 2 3 4
    cout << " united with ";
    showSequence(s3);                  // -2 5 7 12 33
    cout << " yields ";
    showSequence(Result);            // -2 1 2 3 4 5 7 12 33

    Result.clear();                  // empty the set
    set_intersection(s2.begin(), s2.end(),
                     s3.begin(), s3.end(),
                     inserter(Result, Result.begin()));

    showSequence(s2);                  // 0 1 2 3 4 5 7 99
    cout << " intersected with ";
    showSequence(s3);                  // -2 5 7 12 33
    cout << " yields ";
    showSequence(Result);            // 5 7

    Result.clear();
    set_difference(s2.begin(), s2.end(),
                   s1.begin(), s1.end(),
                   inserter(Result, Result.begin()));

    showSequence(s2);                  // 0 1 2 3 4 5 7 99
    cout << " minus ";
    showSequence(s1);                  // 1 2 3 4
    cout << " yields ";
    showSequence(Result);              // 0 5 7 99

    Result.clear();
    set_symmetric_difference(s2.begin(), s2.end(),
                             s3.begin(), s3.end(),
                 inserter(Result, Result.begin()));

    showSequence(s2);                  // 0 1 2 3 4 5 7 99
    cout << "  exclusive or ";
    showSequence(s3);                  // -2 5 7 12 33
    cout << "yields ";
    showSequence(Result);              // -2 0 1 2 3 4 12 33 99
}

