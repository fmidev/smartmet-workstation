// k7/maph.cpp
// Example for a map with hash map
#include<string>
#include<iostream>
using namespace std;

// compiler switch (see text)
//#define STL_map

#ifdef STL_map
 #include<map>
 typedef map<long, string> MapType;      // comparison object: less<long>()
#else
 #include<hmap.h>
 #include<hashfun.h>
 using namespace std;
 typedef HMap<long, string, HashFun<long> > MapType;
#endif

typedef MapType::value_type ValuePair;

int main()
{
   // main() equals ..k4/map1.cpp
    MapType Map;

    Map.insert(ValuePair(836361136, "Andrew"));
    Map.insert(ValuePair(274635328, "Berni"));
    Map.insert(ValuePair(260736622, "John"));
    Map.insert(ValuePair(720002287, "Karen"));
    Map.insert(ValuePair(138373498, "Thomas"));
    Map.insert(ValuePair(135353630, "William"));
    // insertion of Xaviera is not executed, because
    // the key} already exists.
    Map.insert(ValuePair(720002287, "Xaviera"));

    /* Dependent to the underlying implementation,
       the output of the names maybe sorted by numbers
       (STL map) or not (hash map):*/
       
    cout << "Output:\n";
    MapType::iterator iter = Map.begin();
    while(iter != Map.end())
    {
          cout << (*iter).first << ':'     // number
               << (*iter).second           // name
               << endl;
          ++iter;
    }

    cout << "Output of the name after entering the number\n"
         << "Number: ";
    long Number;
    cin >> Number;
    iter = Map.find(Number);    // O(log N), see text

    if(iter != Map.end())
        cout << (*iter).second << ' ' // O(1)
             << Map[Number]     // O(log N)
             << endl;
    else cout << "Not found!" << endl;
}
