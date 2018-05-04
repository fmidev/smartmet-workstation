// k3/iterator/ityp.cc   determination of the iterator type
#include<string>
#include<fstream>
#include<vector>
#include<iterator>
#include<iostream>

using namespace std;

// template for getting the type (iterator-tag) of an iterator
template<class Iterator>
iterator_traits<Iterator>::iterator_category
get_iteratortype(const Iterator&)
{
   return iterator_traits<Iterator>::iterator_category();
}

// overloaded functions
void whichIterator(const input_iterator_tag&)
{
    cout << "Input iterator!" << endl;
}

void whichIterator(const output_iterator_tag&)
{
    cout << "Output iterator!" << endl;
}

void whichIterator(const forward_iterator_tag&)
{
    cout << "Forward iterator!" << endl;
}

void whichIterator(const random_access_iterator_tag&)
{
    cout << "Random access iterator!" << endl;
}


// application
int main( )
{
#ifndef _MSC_VER
    // In case of basic data types we *have* to use
    // the iterator_traits template
    int *ip;                      // random access iterator
    // display of iterator type
    whichIterator(get_iteratortype(ip));
    whichIterator(iterator_traits<int*>::iterator_category());
#endif

    // define a file object for reading
    // (actual file is not required here)
    ifstream Source;
    // an istream_iterator is an input iterator
    istream_iterator<string> IPos(Source);
    // display of iterator type
    whichIterator(get_iteratortype(IPos));  // or alternatively:
    whichIterator(iterator_traits<istream_iterator<string> >
            ::iterator_category());

   // define a file object for writing
    ofstream Destination;
    // an ostream_iterator is an output iterator
    ostream_iterator<string> OPos(Destination);
    // display of iterator type
    whichIterator(get_iteratortype(OPos));  // or alternatively:
    whichIterator(iterator_traits<ostream_iterator<string> >
            ::iterator_category());

#ifndef _MSC_VER
    vector<int> v(10);
    // display of iterator type
    whichIterator(get_iteratortype(v.begin()));  // or some other iterator
    whichIterator(iterator_traits<vector<int>::iterator>
            ::iterator_category());
#endif
}
