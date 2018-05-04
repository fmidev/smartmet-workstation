// k4/div_adt.cc
#include<stack> 
#include<queue>
#include<deque>
#include<list>
#include<vector>
#include<iostream>

using namespace std;

int main()
{
    queue<int, list<int> > aQueue; // list container

    int numbers[] = {1, 5, 6, 0, 9, 1, 8, 7, 2};
    const int count = sizeof(numbers)/sizeof(int);

    cout << "Put numbers into the queue:" << endl;

    for(int i = 0; i < count; i++)
    {
         cout.width(6); cout << numbers[i];
         aQueue.push(numbers[i]);
    }

    stack<int> aStack;             //  default container

    cout << "\n\n Read numbers from the queue (same "
            "order)\n and put them into the stack:"
         << endl;

    while(!aQueue.empty())
    {
         int Z = aQueue.front();  // read value
         cout.width(6); cout << Z;
         aQueue.pop();            // delete value
         aStack.push(Z);
    }
    // ... (to be continued)

    priority_queue<int, vector<int>, greater<int> > aPrioQ;
    // {\tt greater}: small elements first (= high priority)
    // {\tt less}: large elements first

    cout << "\n\n Read numbers from the stack "
            "(reverse order!)\n"
            " and put them into the priority queue:"  << endl;

    while(!aStack.empty())
    {
         int Z = aStack.top();      // read value
         cout.width(6); cout << Z;  // display
         aStack.pop();              // delete value
         aPrioQ.push(Z);
    }

    cout << "\n\n Read numbers from the priority queue "
            "(sorted order!)" << endl;

    while(!aPrioQ.empty())
    {
         int Z = aPrioQ.top();       // read value
         cout.width(6); cout << Z;   // display
         aPrioQ.pop();               // delete value
    }
    cout << endl;
}
