// k8/crossref.cc : program for printing cross-references
#include<fstream>
#include<string>
#include<list>
#include<cctype>
#include<showseq.h>
#include<map>

/* In order to avoid different sorting of upper case and lower case
letters, the class Compare is used which converts all strings to be
compared into lower case, since a corresponding function is not
provided in the string class:*/

using namespace std;

struct Compare
{
   bool operator()(string a, string b) const
   {
     for(size_t i=0; i< a.length(); i++) a[i]=tolower(a[i]);
#ifdef _MSC_VER // Compiler Issue Q167748 PRB
     for(i=0; i< b.length(); i++) b[i]=tolower(b[i]);
#else
     for(size_t i=0; i< b.length(); i++) b[i]=tolower(b[i]);
#endif
     return a < b;
   }
};

int main( )
{
    // This program generates its own cross-reference list.
    ifstream Source("crossref.cc");

    typedef map<string, list<int>, Compare > MAP;
    MAP CrossRef;

    char c;
    int LineNo = 1;

    while(Source)
    {
       // find beginning of identifier
       c = '\0';

       while(Source && !(isalpha(c) || '_' == c))
       {
             Source.get(c);
             if(c == '\n') LineNo++;
       }

       string aKey(1,c);
       // collect rest of identifier
       while(Source && (isalnum(c) || '_' == c))
       {
          Source.get(c);
          if(isalnum(c) || '_' == c)
            aKey += c;
       }
       Source.putback(c);   // back to input stream

       if(c)
           CrossRef[aKey].push_back(LineNo);  // entry
    }

    /* Putting the line number in the list utilizes the fact
       that the MAP::operator[]() returns a reference to the entry,
       even if this has still to be created because the key does not
       yet exist. The entry for the key aKey is a list. Since the line
       numbers are inserted with push_back(), they are in the correct
       order from the very beginning. The output of the cross
       reference list profits by the sorted storage. The first element
       of a value pair is the identifier (key), the second element is
       the list which is output by means of the known template. */

    MAP::iterator iter = CrossRef.begin();

    while(iter != CrossRef.end())
    {
        cout << (*iter).first;           // identifier
        cout.width(20 - (*iter).first.length());
        cout << ": ";
        showSequence((*iter++).second);  // line numbers
    }
}
