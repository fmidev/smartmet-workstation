// k5/nth.cc  Example for nth_element
#include<algorithm>
#include<deque>
#include<showseq.h>
#include<myrandom.h>
#include<functional>

using namespace std;

int main()
{
    deque<int> d(15);
    generate(d.begin(), d.end(), Random(1000));
    showSequence(d);
    // 10 3 335 33 355 217 536 195 700 949 274 444 108 698 564

    deque<int>::iterator nth = d.begin();
    nth_element(d.begin(), nth, d.end());

    cout << "smallest element:"
         << (*nth)                     // 3
         << endl;

    /* The standard comparison object greater causes the sequence to
       be reversed. In this case, the greatest element stands at the
       first position: */

    // here still is nth == d.begin().
    nth_element(d.begin(), nth, d.end(), greater<int>());

    cout << "greatest element  :"
         << (*nth)                     // 949
         << endl;

    // With the < -operator, the greatest element stands at the end:

    nth = d.end();
    nth--;              // now points to the last element
    nth_element(d.begin(), nth, d.end());

    cout << "greatest element  :"
         << (*nth)                     // 949
         << endl;


    // assumption for median value: d.size() is odd
    nth = d.begin() + d.size()/2;
    nth_element(d.begin(), nth, d.end());   

    cout << "Median value       :"
         << (*nth)                     // 335
         << endl;
}
