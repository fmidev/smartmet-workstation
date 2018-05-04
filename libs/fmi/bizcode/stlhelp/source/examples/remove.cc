// k5/remove.cc
#include<iostream>
#include<algorithm>
#include<vector>
#include<string>
#include<iota.h>

bool isVowel(char c)
{
    char vowel[] = "aeiouAEIOU";
    for(register size_t i = 0; i < sizeof(vowel); i++)
       if(c == vowel[i]) return true;
    return false;
}

using namespace std;

int main()
{
    vector<char> v(26);
    // generate alphabet in lower case letters:
    iota(v.begin(), v.end(), 'a');
    ostream_iterator<char> Output(cout, "");
    copy(v.begin(), v.end(), Output);
    cout << endl;

    /* Here, the sequence is not displayed by means of showSequence(),
       because not all values between begin() and end() are to be
       shown, but only the significant ones (iterator last). */

    cout << "remove 't': ";
    vector<char>::iterator last =
                             remove(v.begin(), v.end(), 't');

    // last = new end after shifting
    // v.end() remains unchanged
    copy(v.begin(), last, Output);
      // abcdefghijklmnopqrsuvwxyz   (t is missing)
    cout << endl;

    last = remove_if(v.begin(), last, isVowel);
    cout << "only consonants left: ";
    copy(v.begin(), last, Output);
      // bcdfghjklmnpqrsvwxyz
    cout << endl;

    cout << "complete sequence up to end() with "
            " meaningless rest elements: ";
    copy(v.begin(), v.end(), Output);
      // bcdfghjklmnpqrsvwxyzvwxyzz
    cout << endl;
}
