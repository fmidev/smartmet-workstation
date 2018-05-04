// Dijkstar - Algorithm with graph from file
#include<showseq.h>
#include<checkvec.h>
#include<gra_algo.h>
#include<gr_input.h>

using namespace std;

int main()
{
   // With edge weights of type int
    Graph<string,int> G(false);             // undirected
    ReadGraph(G, "gra2.dat");  // contains minor errors

    G.check(cout); // error check and correction

    checkedVector<int> Dist, Predec;

    int start=0;

    Dijkstra(G, Dist, Predec, start);

    cout << "shortest way to "
         << G[start].first << ":\n";

    cout << "predecessor of:     is:  "
            "distance to [indices in ()]:\n";

    for(size_t i = 0; i < Predec.size(); ++i)
    {
        cout <<  "    " << G[i].first
             << '(' << i << ')';
        cout << "         ";
        if(Predec[i] < 0)
           cout << "-"   //  no predecessor of starting point
                << '(' << Predec[i] << ')';
        else
           cout << G[Predec[i]].first
                << '(' << Predec[i] << ')';
        cout.width(9);
        cout << Dist[i] << endl;
    }
}
