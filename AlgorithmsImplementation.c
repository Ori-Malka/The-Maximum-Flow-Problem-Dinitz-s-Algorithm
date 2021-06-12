#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*                           ~~Results~~
Graph #1 - |V|=10000, |E|=26941543, |SF|=152274, |BF|=4732325, |1F|=299;
Graph #2 - |V|=10000, |E|=402057, |SF|=25311, |BF|=737898;
Graph #3 - |V|=1000, |E|=389895, |SF|=351774, |BF|=11786632, |1F|=633;
Graph #4 - |V|=1000, |E|=7150, |SF|=2816, |BF|=85104;
*/

/*
* V=|V| - # of nodes. 
* s - source node.
* t - sink node.
*/

#define V 1000
#define s 0
#define t V-1

/*Change to create new networks*/
#define MAX_CAP 32768

/*Infinity*/
#define inf 2147483647

typedef enum Boolean {
    FALSE, TRUE
}Boolean;

enum Color {
    WHITE, RED, BLACK
};

typedef struct FlowEdge {
    int capacity;
    int flow;
}FlowEdge;

void CapacityBuild();
void GraphInit(char[]);
int residualCap(int u, int v);
int getMin(int x, int y);

void FF_Test(char[]);
void EK_Test(char[]);
void D_Test(char[]);

FlowEdge FlowNetwork[V][V];


/*~~~~~~~~~~~~~~~~~~~Ford_Fulkerson~~~~~~~~~~~~~~~~~~~*/

/*Simple DFS implementation to find a path from s to t*/
int visited[V];
int visitedT = 1;
int FF_DFS(int location, int flow) {
    int i;

    /*return when reached t*/
    if (location == t) 
        return flow;

    /*we save in visited[i] the order of i in the DFS exploration*/
    visited[location] = visitedT;

    for (i = 0; i < V; i++) {
        int cap = FlowNetwork[location][i].capacity;
        if (visited[i] != visitedT && cap > 0) {

            if (cap < flow)
                flow = cap;
            int FF_DFS_Flow = FF_DFS(i, flow);
            if (FF_DFS_Flow > 0) {
                FlowNetwork[location][i].capacity -= FF_DFS_Flow;
                FlowNetwork[i][location].capacity += FF_DFS_Flow;
                return FF_DFS_Flow;
            }
        }


    }
    return 0;
}

int Ford_Fulkerson() {


    for (int max_flow = 0; ; ) {
        //finding an augmenting path
        int flow = FF_DFS(s, inf);
        visitedT++;

        max_flow += flow;

        //No more augmenting paths 
        if (flow == 0)
            return max_flow;
    }

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~Edmonds-Karp~~~~~~~~~~~~~~~~~~~~*/
int color[V]; //array for the BFS implementation
int p[V];  //array to store the augmenting path

/*~~Simple Queue implementation~~*/
int head, tail;
int q[V];

void Enqueue(int x) {
    q[tail] = x;
    tail++;
    color[x] = RED;
}

int Dequeue() {
    int x = q[head];
    head++;
    color[x] = BLACK;
    return x;
}
/*~~~~~~~~~*/

/*
* This BFS is extended because its -
* implemented to find an augmenting path -
* and not just the shortest path from s to t.
* 
*/

Boolean EK_BFS() {
    int i, u, v;

    /*Queue initialization*/
    head = 0;
    tail = 0;
    /*~~~~~~~~~~~~~~~~~~~~*/

    /*EK_BFS initialization*/
    for (i = 0; i < V; i++) {
        color[i] = WHITE;
        p[i] = -1;
    }

    Enqueue(s);
    /*~~~~~~~~~~~~~~~~~~~~~*/

    while (head != tail) {
        u = Dequeue();
        for (v = 0; v < V; v++) {

            //color[v]->white and (u, v) is not saturated

            if (color[v] == WHITE && (residualCap(u, v) > 0)) {
                p[v] = u;
                Enqueue(v);
            }
        }
    }
    return color[t] == BLACK;
    //if the color of t is black it means that there are a path from s to t this is why BFS return TRUE.
}

int Edmonds_Karp() {
    int u, max_flow = 0;

    while (EK_BFS()) {
        int bottle_neck_val = inf;

        //find the bottle neck value of the augmenting path P;
        //u=t->....->s; 
        for (u = t; p[u] >= 0; u = p[u]) {

            bottle_neck_val = getMin(bottle_neck_val, residualCap(p[u], u));
        }

        // increment the flow with the bottle neck val;
        for (u = t; p[u] >= 0; u = p[u]) {
            FlowNetwork[p[u]][u].flow += bottle_neck_val;
            FlowNetwork[u][p[u]].flow -= bottle_neck_val;
        }
        max_flow += bottle_neck_val;
    }

    return max_flow;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~Dinitz's Algorithm~~~~~~~~~~~~~~~~*/
int level[V];
//similar implementation to the EK_BFS but this one helps us to build the level graph.
//I could implement D_BFS & EK_BFS together but i choose to do it separately so it will be easier to understand.
Boolean D_BFS() {
    int i, u, v;
    /*Queue initialization*/
    head = 0;
    tail = 0;

    /*D_BFS initialization*/
    for (i = 0; i < V; i++) {
        color[i] = WHITE;
        level[i] = -1;
    }
    level[s] = 0;
    Enqueue(s);

   
    while (head != tail) {
        u = Dequeue();
        for (v = 0; v < V; v++) {
            if (level[v]==-1&&(residualCap(u, v) > 0)) {
                //color[v] == WHITE its the same like checking level[v]==-1
                level[v] = level[u] + 1;
                Enqueue(v);
            }
        }
    }

    return level[t] != -1; 
}
//this DFS implementation returns the blocking flow.
int D_DFS(int location, int* next, int flow) {

    if (location == t) return flow;

    for (; next[location] < V; next[location]++) {
        int cap = residualCap(location, next[location]);
        if (cap > 0 && level[next[location]] == level[location] + 1) {

            int bottle_neck_val = D_DFS(next[location], next, min(flow, cap));
            if (bottle_neck_val > 0) {

                FlowNetwork[location][next[location]].flow += bottle_neck_val;
                FlowNetwork[next[location]][location].flow -= bottle_neck_val;
                return bottle_neck_val;
            }
        }
    }
    return 0;
}

int Dinitz() {
    /* next[i] indicates the next unused edge index in the adjacency array for node i */
    /* This implementation suggested by Shimon Even and Alon itai to remove dead-ends
       and implement the 'clean' routine on-the-fly */
    int i, next[V], max_flow = 0;

    
    //D_BFS its an implementation of building the level graph
    while (D_BFS()) {

        //initialize next array.
        for (i = 0; i < V; i++) {
            next[i] = 0;
        }

       //while there is a blocking flow sum the flow and re-build the level graph
        for (int f = D_DFS(s, next, inf); f != 0; f = D_DFS(s, next, inf)) {
            max_flow += f;
        }
    }

    return max_flow; //no more paths from s,t 
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~augmentingPathNum~~~~~~~~~~~~~~~~~~~*/

int augmentingPathNum() {
        int u, counter = 0;

        while (EK_BFS()) {
            //remove the augmenting path P edges from the graph.
            //u=t->....->s; 
      
            for (u = t; p[u] >= 0; u = p[u]) {

                FlowNetwork[p[u]][u].capacity = 0;
                FlowNetwork[p[u]][u].flow =0;
            }
            counter++;
        }

        return counter;
    }


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int main() {
    srand(time(NULL));

    /* ~~~~~~~~~~ Augmenting path counting trial ~~~~~~~~~
    GraphInit("G1BC.txt");
    printf("\nThe Number of Augmenting paths in phase 3 of the trial is:%d\n" ,augmentingPathNum());

    GraphInit("G3BC.txt");
    printf("\nThe Number of Augmenting paths in phase 8 of the trial is:%d\n", augmentingPathNum());
    */

    //Adjust V to 10K OR 1K
    switch (V) {

    //Phases 1-5
    case(10000):
      
        EK_Test("G11C.txt");
        D_Test("G11C.txt");

        EK_Test("G1SC.txt");
        D_Test("G1SC.txt");

        EK_Test("G1BC.txt");
        D_Test("G1BC.txt");
       
        EK_Test("G2SC.txt");
        D_Test("G2SC.txt");

        EK_Test("G2BC.txt");
        D_Test("G2BC.txt");
       
        break;

    //Phases 6-10
    case(1000):
     
        FF_Test("G31C.txt");
        EK_Test("G31C.txt");
        D_Test("G31C.txt");

        FF_Test("G3SC.txt");
        EK_Test("G3SC.txt");
        D_Test("G3SC.txt");
        
        FF_Test("G3BC.txt");
        EK_Test("G3BC.txt");
        D_Test("G3BC.txt");

        FF_Test("G4SC.txt");
        EK_Test("G4SC.txt");
        D_Test("G4SC.txt");
        
        FF_Test("G4BC.txt");
        EK_Test("G4BC.txt");
        D_Test("G4BC.txt");
       
        break;

   

    default:
        break;
    }

    return 0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Utility~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*Function to generate a Flow-Network text file from a regular graph text file*/
void CapacityBuild() {
    int n, m, i, j;
    char c;

    /*Opens Graph #*/
    FILE* input = fopen("G#.txt", "r");
    if (input == NULL) {
        printf("Error Reading File!");
        exit(1);
    }

    /*X ={S=1-1023, B=1-32767, 1=1-1};*/
    FILE* out = fopen("G#XC.txt", "w+");
    if (out == NULL) {
        printf("Error Reading File!");
        fclose(input);
        exit(1);
    }

    /*read |V|=n, |E|=m*/
    fscanf_s(input, "%d %d", &n, &m);

    fprintf(out, "%d %d", n, m);

    /*Creates Capacity Matrix - G#XC*/
    int t_cap = 0;
    for (i = 0; i < V; i++) {
        for (j = 0; j < V; j++) {
            t_cap = 0;
            c = getc(input);

            /*to avoid \n's */
            if (c == '\n')
                break;

            /*create an edge (u,v) with t_cap capacity*/
            if (c == '1') {
                t_cap = rand() % MAX_CAP;
                //to avoid zero capacity
                while (!t_cap)
                    t_cap = rand() % MAX_CAP;
            }
            fprintf(out, "%d ", t_cap);
        }
        fprintf(out, "\n");
    }

    fclose(out);
    fclose(input);
}
/*Function that initialize a FlowEdge matrix from the flow-network text file we created using CapacityBuild*/
void GraphInit(char name[]) {

    int c, i, j, n, m;
    FILE* input = fopen(name, "r");

    if (input == NULL) {
        printf("Error Reading File!");
        exit(1);
    }

    fscanf_s(input, "%d %d", &n, &m);
  
    printf("\n%s: |V|=%d, |E|=%d.",name ,n, m);
  
    for (i = 0; i < V; i++) {
        for (j = 0; j < V; j++) {
            fscanf_s(input, "%d", &c);
            FlowNetwork[i][j].capacity = c;
            FlowNetwork[i][j].flow = 0;
        }
    }

    fclose(input);

}
/*Function that returns the residualCap of an edge*/
int residualCap(int u, int v) {
    return FlowNetwork[u][v].capacity - FlowNetwork[u][v].flow;
}

/*Function that returns the minimum out of two integers*/
int getMin(int x, int y) {
    return x < y ? x : y;
}

/*
* Test creators. 
* measuring time in seconds and calling the 
* given functions using the given network name.
*/




void FF_Test(char name[]) {
    int max_flow;
    clock_t start, end;
    float cpu_time_used;

    GraphInit(name);

    start = clock();
    max_flow=Ford_Fulkerson();
    end = clock();
    cpu_time_used = ((float)(end - start)) / CLOCKS_PER_SEC;

    printf("\nThe Maximum flow is: %d.\nFord-Fulkerson's time is: %lf seconds.\n", max_flow, cpu_time_used);

}


void EK_Test(char name[]) {
    int max_flow;
    clock_t start, end;
    float cpu_time_used;

    GraphInit(name);

    start = clock();
    max_flow = Edmonds_Karp();
    end = clock();
    cpu_time_used = ((float)(end - start)) / CLOCKS_PER_SEC;

    printf("\nThe Maximum flow is: %d.\nEdmonds-Karp's time is: %lf seconds.\n", max_flow, cpu_time_used);
}

void D_Test(char name[]) {
    int max_flow = 0;
    clock_t start, end;
    float cpu_time_used;

    GraphInit(name);

    start = clock();
    max_flow = Dinitz();
    end = clock();
    cpu_time_used = ((float)(end - start)) / CLOCKS_PER_SEC;

    printf("\nThe Maximum flow is: %d.\nDinitz's time is: %lf seconds.\n", max_flow, cpu_time_used);

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
