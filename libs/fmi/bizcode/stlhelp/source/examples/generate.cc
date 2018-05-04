// k5/generate.cc
#include<algorithm>         
#include<vector>
#include<showseq.h>
#include<myrandom.h>                   // see ...

int PowerOfTwo()    // double value, but begin with 1
{
    static int Value = 1;
    return (Value *= 2)/2;  
}

using namespace std;

int main()
{
    vector<int> v(12);

    Random whatAChance(1000);
    generate(v.begin(), v.end(), whatAChance);
    showSequence(v); // 10 3 335 33 355 217 536 195 700 949 274 444

    generate_n(v.begin(), 10, PowerOfTwo);  // only 10 out of 12!
    showSequence(v); // 1 2 4 8 16 32 64 128 256 512   274 444
}
