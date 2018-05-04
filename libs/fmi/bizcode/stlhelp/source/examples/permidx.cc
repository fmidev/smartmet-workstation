// k8/permidx.cc
// Program for generation of a permuted index
#include<iostream>
#include<vector>
#include<string>
#include<cstring>  // for strcmp()
#include<map>
#include<cctype> 

using namespace std;

struct StringCompare
{
   bool operator()(const char* a, const char* b) const
   {
       return strcmp(a,b) < 0;
   }
};

/* The class StringCompare is needed for the creation of a function
   object for the map container. No use is made of the well-known
   function strcmp()}, since in the coming C++ standard, application
   of the functions from string.h is regarded as a `C hangover' and is
   no longer recommended. However, if speed is needed, please use
   strcmp().*/

int main()
{
    vector<string> Title(3);
    vector<int> Page(Title.size());

    /* Normally, titles and page numbers would be read from a file,
       but for simplicity, in this example both are wired in.  */
    // read titles here. Only for the example:
    Title[0] = "Electronic Mail and POP";        Page[0] = 174;
    Title[1] = "Objects in the World Wide Web" ; Page[1] = 162;
    Title[2] = "Unix or WindowsNT?";             Page[2] =  12;

    typedef map<const char*, int, StringCompare> MAP;
    MAP aMap;

    /* All pointers to words that begin with an upper case letter are
       stored in the map container together with the page numbers of
       the titles. It is assumed that words are separated by spaces.
       An alternative could be not to store the pointers, but the
       words themselves as string objects. On average, however, this
       would require more memory, and a multimap container would have
       to be used, because the same words can occur in different
       titles. The pointers, in contrast, are unique. The same words
       in different titles have different addresses.*/

    for(register size_t i = 0; i < Title.size(); i++)
    {
        register size_t j = 0;

        do
        {
            const char *Word = Title[i].c_str() + j;
            if(isalpha(*Word) && isupper(*Word))
               aMap[Word] = i;             // entry

            // find next space
            while(j < Title[i].length()
               && Title[i][j] != ' ') j++;

            // find beginning of word
            while(j < Title[i].length()
               && !isalpha(Title[i][j])) j++; 
        } while(j < Title[i].length());
    }

    /* The map container is filled, now we need the output. As usual
       in such cases, the formatting requires more program lines than
       the rest.*/

    MAP::iterator I = aMap.begin();
    const int leftColumnWidth = 28,
              rightColumnWidth = 30;

    while(I != aMap.end())
    {
        // determine left column text
        // = 1st character of title no. (*I).second
        // up to the beginning of the search term 
        // which begins at (*I).first.
        const char *Begin = Title[(*I).second].c_str();
        const char *End = (*I).first;

        // and output with leading spaces
        cout.width(leftColumnWidth-(End-Begin));
        cout << " ";
        while(Begin != End)
              cout << *Begin++;      

        // output right column text
        cout << " ";    // highlight separation left/right
        cout.width(rightColumnWidth);
        cout.setf(ios::left, ios::adjustfield);     // ranged left
        cout << (*I).first;          

        cout.width(4);
        cout.setf(ios::right, ios::adjustfield);    // ranged right
        cout << Page[(*I).second]        // page number
             << endl;
        ++I;              // go to next entry
    }
}
