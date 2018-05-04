#include<gra_algo.h>         // contains Dijkstra()
#include<gra_util.h>         // auxiliary functions

using namespace std;

int main()
{
    size_t Count = 100;
    Graph<Place,float> G(false);              // undirected
    create_vertex_set(G, Count, 12800, 9000); // range

    connectNeighbors(G);

    /* The Dijkstra() function must be passed the graph, a vector of
       distances and a vector of the predecessors, which are modified
       by the algorithm. The distance type must match the edge
       parameter type of the graph. */
       
    vector<float> Dist;
    vector<int> Pred;

    int start = 0;               // starting point 0

    Dijkstra(G, Dist, Pred, start);

    /* The last argument is the starting point which can be any vertex
       between no. 0 and no. (G.size()-1). After the call, the
       distance vector contains the length of the shortest paths from
       each point to the starting point. Dist[k] is the length of the
       shortest possible path from vertex no. k to vertex no. 0.
       Dist[StartingPoint] is 0 by definition. */
       
    // output
    cout << "shortest path to "
         << G[start].first << ":\n";

    cout << "predecessor of:  is:  "
            "distance to [indices in ()]:\n";

    for(size_t i = 0; i < Pred.size(); ++i)
    {
        cout <<  G[i].first
             << '(' << i << ')';
        cout << "    ";

        if(Pred[i] < 0)
           cout << "-"          // no predecessor of starting point
                << '(' << Pred[i] << ')';
        else
           cout << G[Pred[i]].first
                << '(' << Pred[i] << ')';

        cout.width(9);
        cout << Dist[i] << endl;
    }
}
