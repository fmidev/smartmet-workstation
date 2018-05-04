// k5/reverse.cc
#include<algorithm>
#include<showseq.h>
#include<vector>
#include<iota.h>

using namespace std;

int main()
{
    char s[] = "Madam";
    vector<char> vc(s, s + sizeof(s)-1); // -1 because of null byte
    showSequence(vc);   // Madam

    reverse(vc.begin(), vc.end());
    showSequence(vc);   // madaM

    vector<int> vi(10);
    iota(vi.begin(), vi.end(), 10);
    showSequence(vi);   // 10 11 12 13 14 15 16 17 18 19

    reverse(vi.begin(), vi.end());
    showSequence(vi);   //  19 18 17 16 15 14 13 12 11 10
}
