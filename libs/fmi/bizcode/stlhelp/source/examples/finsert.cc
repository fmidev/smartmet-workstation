// k3/iterator/finsert.ccc
// Insert iterators: front inserter
#include<showseq.h>
#include<list>
#include<iterator>

using namespace std;

int main()
{
    list<int> aList(5, 0);     // 5 zeros

    cout << "aList.size() = "
         << aList.size() << endl;

    showSequence(aList);

    front_insert_iterator<list<int> >
                              aFrontInserter(aList);

    // insertion by means of the operations *, ++, =
    int i = 1;
    while(i < 3)
          *aFrontInserter++ = i++;

    cout << "aList.size() = "
         << aList.size() << endl;

    showSequence(aList);
}
