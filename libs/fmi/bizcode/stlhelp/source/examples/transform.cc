// k5/transform.cc
#include<algorithm>
#include<showseq.h>
#include<string>
#include<vector>

using namespace std;

string uppercase(string s)     // unary operation as function
{
    for(register size_t i = 0; i < s.length(); i++)
        if(s[i] >= 'a' && s[i] <= 'z')
           s[i] -= 'a'-'A';
    return s;
}

class join                // binary operation as functor
{
    public:
       string operator()(const string& a, const string& b)
       {
            return a + " and " + b;
       }
};

int main()
{
    vector<string> Gals(3), Guys(3),
                   Couples(3);   // there must be enough space
    Gals[0] = "Annabella";
    Gals[1] = "Scheherazade";
    Gals[2] = "Xaviera";

    Guys[0]  = "Bogey";
    Guys[1]  = "Amadeus";
    Guys[2]  = "Wladimir";

    transform(Guys.begin(), Guys.end(),
              Guys.begin(),   // Target == Source
              uppercase);

    transform(Gals.begin(), Gals.end(),
              Guys.begin(), Couples.begin(),
              join());

    showSequence(Couples, "\n");
}
