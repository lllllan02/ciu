#include <stdio.h>
#include <stdlib.h>

#define maxn 1001

typedef struct Edge {
    int v;
    struct Edge* next;
} Edge;

Edge* G[maxn];
int vis[maxn];

void add_edge(int u, int v) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->v = v;
    e->next = G[u];
    G[u] = e;
}

void dfs(int u) {
    vis[u] = 1;
    printf("to: %d\n", u);

    for (Edge* e = G[u]; e; e = e->next) {
        int v = e->v;
        if (!vis[v]) dfs(v);
    }
}

int main() {
    setbuf(stdout, NULL);

    // 简单有向图: 1 -> 2 -> 3, 1 -> 4
    add_edge(1, 2);
    add_edge(1, 4);
    add_edge(2, 3);

    printf("DFS from node 1:\n");
    dfs(1);

    return 0;
}