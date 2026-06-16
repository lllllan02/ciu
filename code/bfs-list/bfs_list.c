#include <stdio.h>
#include <stdlib.h>

#define maxn 1001

typedef struct Edge {
    int v;
    struct Edge* next;
} Edge;

Edge* G[maxn];
int vis[maxn];
int q[maxn];
int head, tail;

void add_edge(int u, int v) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->v = v;
    e->next = G[u];
    G[u] = e;
}

void bfs(int s) {
    head = tail = 0;
    vis[s] = 1;
    q[tail++] = s;

    while (head < tail) {
        int u = q[head++];
        printf("to: %d\n", u);

        for (Edge* e = G[u]; e; e = e->next) {
            int v = e->v;
            if (!vis[v]) {
                vis[v] = 1;
                q[tail++] = v;
            }
        }
    }
}

int main() {
    setbuf(stdout, NULL);

    // 简单有向图: 1 -> 2 -> 3, 1 -> 4
    add_edge(1, 2);
    add_edge(1, 4);
    add_edge(2, 3);

    printf("BFS from node 1:\n");
    bfs(1);

    return 0;
}
