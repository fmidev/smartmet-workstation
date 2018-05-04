#include<dynpq.h>
#include<showseq.h>

using namespace std;

int main()
{
    checkedVector<double> V(8);
    //... assign values to the elements V[i]}
    for(size_t i = 0; i< V.size(); i++)
        V[i] = 1.1230 + 0.1*i*(int(i)-4);

    showSequence(V);

    dynamic_priority_queue<double>    DPQ(V);

    // change value V[3]}; correct insertion
    // into DPQ is carried out automatically
    cout <<  "DPQ.changeKeyAt(3, -1.162);" << endl;
    DPQ.changeKeyAt(3, -1.162);
    showSequence(V);

    cout <<  "DPQ.changeKeyAt(7, 61.1611);" << endl;
    DPQ.changeKeyAt(7, 61.1611);
    showSequence(V);

    cout <<  "DPQ.changeKeyAt(0, 661.1615);" << endl;
    DPQ.changeKeyAt(0, 661.1615);
    showSequence(V);

    // outputting and emptying by order of priority
    while(!DPQ.empty())
    {
        int ix = DPQ.topIndex();
        double x = DPQ.topKey();
        cout << "ind: " << ix;
        cout << " key: " << x << endl;
        DPQ.pop();
    }
    // provoke error:
//    DPQ.changeKeyAt(0, -0.335);
}
