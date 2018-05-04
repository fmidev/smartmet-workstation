#include<algorithm>
#include<graph.h>
#include<gr_input.h>

using namespace std;

int main()
{
    // no edge weighting, therefore type Empty:
    Graph<string, Empty> V(true);        // directed

    ReadGraph(V, "gra1.dat");

    V.check();                // display properties

    // display of vertices with successors
    cout << V;

    // next: undirected graph
    Graph<string, Empty> VU(false);    // undirected

    ReadGraph(VU, "gra1u.dat");

    VU.check();                // display properties

    // display of vertices with successors
    cout << VU;
}
