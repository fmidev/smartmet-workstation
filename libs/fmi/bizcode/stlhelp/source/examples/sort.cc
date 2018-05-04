// k5/sort.cc
#include<algorithm>
#include<vector>
#include<showseq.h>
#include<rand.h>

bool integer_less(double x, double y)
{   return long(x) < long(y);}

using namespace std;

int main()
{
    vector<double> v(17);
    RAND aChance;

    // initialize vector with random values, with
    // many values having the same integer part:

    register size_t i;
    for(i = 0; i < v.size(); i++)
    {
        v[i] =  aChance(3)
               + double(aChance(100)/1000.0);
    }

    random_shuffle(v.begin(), v.end(), aChance);

    vector<double> unstable = v,       // auxiliary vectors
                     stable = v;

    cout << "Sequence             :\n";
    showSequence(v);
     // 1.032 1.081 0.042 0.069 0.016 2.065 0.03 0.09
     // 2.022 1.07 0.086 0.073 0.045 1.042 1.077 2.097 1.098

    // sorting with < operator:
    stable_sort(stable.begin(), stable.end());
    cout << "\n no difference, because double number "
            "is used as key\n";
    cout << "stable sorting   :\n";
    showSequence(stable);
     // 0.016 0.03 0.042 0.045 0.069 0.073 0.086 0.09
     // 1.032 1.042 1.07 1.077 1.081 1.098 2.022 2.065 2.097

    sort(unstable.begin(), unstable.end());
    cout << "unstable sorting :\n";
    showSequence(unstable);
    // 0.016 0.03 0.042 0.045 0.069 0.073 0.086 0.09
    // 1.032 1.042 1.07 1.077 1.081 1.098 2.022 2.065 2.097

    //  sorting with function instead of < operator:
    unstable = v;
    stable = v;
    cout << "\n differences, because only the int part "
            "is used as key\n";

    stable_sort(stable.begin(), stable.end(),integer_less);
    cout << "stable sorting (integer key)  :\n";
    showSequence(stable);
     // 0.042 0.069 0.016 0.03 0.09 0.086 0.073 0.045
     // 1.032 1.081 1.07 1.042 1.077 1.098 2.065 2.022 2.097

    sort(unstable.begin(), unstable.end(), integer_less);
    cout << "unstable sorting (integer key):\n";
    showSequence(unstable);
     // 0.042 0.069 0.016 0.03 0.09 0.045 0.073 0.086
     // 1.098 1.077 1.042 1.07 1.081 1.032 2.022 2.065 2.097
}
