#include<algorithm>
#include<graph.h>
#include<gr_input.h>

using namespace std;

int main()
{
    // integer weights
    Graph<string, int> V(false);        // undirected

    ReadGraph(V, "gra2.dat");

    V.check();                // display properties

    // display of vertices with successors
    cout << V;
}
