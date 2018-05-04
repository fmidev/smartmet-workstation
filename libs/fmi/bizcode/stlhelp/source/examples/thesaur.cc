// k8/thesaur.cc : program for the output of terms
// related to a given concept
#include<fstream>
#include<vector>
#include<string>
#include<slist.h>
#include<cctype>
#include<algorithm>
#include<iostream>

using namespace std;

struct indirectCompare
{
    indirectCompare(const vector<string>& v) : V(v) {}

    bool operator()( int x,  int y) const
    {
         return V[x] < V[y];
    }

    bool operator()( int x, const string& a) const
    {
         return V[x] < a;
    }

    const vector<string>& V;
};

/* The class indirectCompare compares the corresponding values in the
vector V for passed indices, and the reference is initialized during
construction of the object. The second overloaded function operator
directly compares a value with a vector element whose index is given.
*/

void readRoget(vector<string>& Words,
               vector<slist<int> >& lists)
{
    ifstream Source("roget.dat");
    assert(Source);

    // read thesaurus
    const int maxbuf = 200;
    char buf[maxbuf];
    char c;
    register size_t i;

    while(Source.get(c))
    {
       if(c == '*')                // skip line
          Source.ignore(1000,'\n');
       else
         if(isdigit(c))
         {
            Source.putback(c);
            Source >> i;                      // current no.
            Source.getline(buf, maxbuf, ':');   // word
            Words[--i] = buf;

            // read linenumbers, if there are some,
            // while ignoring backslash
            while(Source.peek() != '\n')
            {
                int j;
                Source >> j;
                lists[i].push_front(--j);
                if(Source.peek() == '\\')
                   Source.ignore(1000,'\n');
            } 
         }
    } 
}

int main( )
{
    const int Maxi = 1022;  // number of entries in roget.dat

    vector<string> Words(Maxi);
    vector<slist<int> > relatedWords(Maxi);
    vector<int> Index(Maxi);

    // read thesaurus file
    readRoget(Words,relatedWords);

    // build index vector
    for(size_t i = 0; i < Index.size(); i++)
       Index[i] = i;

    indirectCompare aComparison(Words);    // functor

    sort(Index.begin(), Index.end(), aComparison);

    /* The index vector now indicates the ordering, so that
       Words[Index[0]]} is the first term according to the
       alphabetical sorting order. This creates the precondition for
       binary search. */

    cout << "Search term? ";
    string SearchTerm;
    getline(cin, SearchTerm);

    // binary search
    vector<int>::iterator TableEnd,
               where = lower_bound(Index.begin(), Index.end(),
                                SearchTerm, aComparison);

    /* If the iterator where points to the end of the table, the term
       was not found. Otherwise, a check must be made as to whether
       the found term matches the search term in its first characters.
       This does not have to be the case, because lower_bound() only
       returns a position which is suitable for sorted insertion. */

    bool found = true;         // hypothesis to be checked
    if(where == TableEnd)
       found = false;
    else
         // next possible entry is <= search term
         // do they match?
    {
        register size_t i = 0;
        while(i < Words[*where].length()
             && i < SearchTerm.length()
             && found)
           if(Words[*where][i] != SearchTerm[i])
              found = false;
           else ++i;
    }

    /* If the term is found, the list of references, provided that
       references exist, is 'scoured' with the iterator here, and the
       corresponding terms are displayed on screen. */

    if(found)
    {
       cout << "found   : "
            << Words[*where] << endl
            << "related words:\n";

       slist<int>::iterator atEnd,
             here = relatedWords[*where].begin();

       if(here == atEnd)
          cout << "not found\n";
       else
          while(here != atEnd)
               cout << '\t' << Words[*here++] << endl;
    }
    else  cout << "not found\n";
}
