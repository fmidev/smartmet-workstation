// k5/binarysearch.cc
// Example for binary_search} and related algorithms
#include<algorithm>
#include<list>
#include<string>
#include<showseq.h>

using namespace std;

int main()
{
    list<string> Places;
    Places.push_front("Bremen");
    Places.push_front("Paris");
    Places.push_front("Milan");
    Places.push_front("Hamburg");
    Places.sort();                   // important precondition
    showSequence(Places);

    string Town;
    cout << "Search/insert which town? ";
    cin >> Town;

    if(binary_search(Places.begin(), Places.end(), Town))
       cout << Town << " exists\n";
    else
       cout << Town << " does not yet exist\n";

    // insertion at the correct position
    cout << Town << " is inserted:\n";
    list<string>::iterator i =
        lower_bound(Places.begin(), Places.end(), Town);
    Places.insert(i, Town);
    showSequence(Places);

    // range of identical values
    pair<list<string>::iterator, list<string>::iterator> 
     p = equal_range(Places.begin(), Places.end(), Town);

    // The two iterators of the pair p limit the range
    // in which Town occurs:
    list<string>::difference_type n = distance(p.first, p.second);
    cout << Town << " is contained " << n
         << " times in the list\n";
}
