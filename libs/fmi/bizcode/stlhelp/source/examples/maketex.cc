// Dijkstra - algorithm with generated graph
#include<gra_algo.h>
#include<gra_util.h>
#include<cmath>
#include<cstdlib>
using namespace std;

int main( int argc, char** argv )
{
    unsigned int Count = 100;
    Graph<Place,float> G(false);             // undirected
    create_vertex_set(G, Count, 12800, 9000); // range
    connectNeighbors(G);

    createTeXfile("graph.pic", G, 0.01, 120, 90);

    // generate MetaPost file for graph
    createMPfile( "graph.mp", G, 0.01 );

    vector<float> DistVec;
    vector<int> Predecessors;

    int start = 0;
    Dijkstra(G, DistVec, Predecessors, start);

    int where = ( argc >= 2 )
         ? min( int( Count - 1 ), atoi( argv[ 1 ] ) ) : 63;
 
    // generate MetaPost file for shortest path
    writeMPpath( "graph1.mp", G, Predecessors, 0.01, where );
  

    cout << "\n\nAttachment to tex-file to emphasize the shortest path from  63 to 0.\n"
            "Please, insert output of this program into graph.pic directly\n"
            "before \\end{picture} if you want.\n\n"
            "If you use MetaPost (recommended), than put the shortest\n"
            "path from graph1.mp into graph.mp (see also readme.tex).\n\n";

    cout.setf(ios::fixed);
    cout.precision(2);
    while(where !=0)
    {  double x1,x2,y1,y2,dx,dy;
      //  cout << "from " << where << " to " << Predecessors[where] << ": ";
        x1 = 0.01*G[where].first.X();  // mm
        y1 = 0.01*G[where].first.Y();
        where = Predecessors[where];
        x2 = 0.01*G[where].first.X();
        y2 = 0.01*G[where].first.Y();
        dx = x2-x1; dy = y2-y1;
        double dist = sqrt(dx*dx+dy*dy);
        int wdh = int(5*dist);
        dx = dx/wdh; dy = dy/wdh;
        cout << "\\multiput(" << x1 << "," << y1 << ")("
              << dx << "," << dy << "){" << wdh
              << "}{\\circle*{0.5}}\n";
    }
}    

