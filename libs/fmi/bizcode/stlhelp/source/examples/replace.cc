// k5/replace.cc
#include<algorithm>
#include<showseq.h>
#include<string>
#include<vector>

using namespace std;

// unary predicate as functor
class Citrusfruit
{
    public:
       bool operator()(const string& a)
       {
            return  a == "lemon" 
                 || a == "orange"
                 || a == "lime";
       }
};

int main()
{
    vector<string> Fruitbasket(3), Crate(3);

    Fruitbasket[0] = "apple";
    Fruitbasket[1] = "orange";
    Fruitbasket[2] = "lemon";
    showSequence(Fruitbasket); // apple orange lemon

    cout << "replace: "
            "replace apple with quince:\n";
    replace(Fruitbasket.begin(), Fruitbasket.end(),
            string("apple"), string("quince"));
    showSequence(Fruitbasket); // quince orange lemon

    cout << "replace_if: "
            "replace citrus fruits with plums:\n";
    replace_if(Fruitbasket.begin(), Fruitbasket.end(),
            Citrusfruit(), string("plum"));
    showSequence(Fruitbasket); // quince plum plum

    cout << "replace_copy: "
            "copy and replace the plums "
            "with limes:\n";
    replace_copy(Fruitbasket.begin(), Fruitbasket.end(),
            Crate.begin(), string("plum"), string("lime"));
    showSequence(Crate);       // quince lime lime

    cout << "replace_copy_if: copy and replace "
            "the citrus fruits with tomatoes:\n";
    replace_copy_if(Crate.begin(), Crate.end(),
            Fruitbasket.begin(), Citrusfruit(), string("tomato"));
    showSequence(Fruitbasket); // quince tomato tomato
}
