// map1.cc     Example for map
#include<map>
#include<string>
#include<iostream>

using namespace std;

// two typedefs for abbreviations
typedef map<long, string>   MapType;      // comparison object: less<long>()
typedef MapType::value_type ValuePair;

int main()
{
    MapType aMap;

    aMap.insert(ValuePair(836361136, "Andrew"));
    aMap.insert(ValuePair(274635328, "Berni"));
    aMap.insert(ValuePair(260736622, "John"));
    aMap.insert(ValuePair(720002287, "Karen"));
    aMap.insert(ValuePair(138373498, "Thomas"));
    aMap.insert(ValuePair(135353630, "William"));
    // insertion of Xaviera is not executed, because
    // the key already exists.
    aMap.insert(ValuePair(720002287, "Xaviera"));

    /* Due to the underlying implementation, the output of the names
       is sorted by numbers: */

    cout << "Output:\n";
    MapType::iterator iter = aMap.begin();
    while(iter != aMap.end())
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
    iter = aMap.find(Number);         // O(log N), see text

    if(iter != aMap.end())
        cout << (*iter).second << ' ' // O(1)
             << aMap[Number]          // O(log N)
             << endl;
    else cout << "Not found!" << endl;
}
