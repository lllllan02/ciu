#include <stdio.h>
#include <stdlib.h>

#define maxn 1001

typedef struct Edge {
    int v;
    struct Edge* next;
} Edge;

Edge* G[maxn];
int deg[maxn];   // 入度
int q[maxn];
int head, tail;
int topo[maxn];
int tp;

void add_edge(int u, int v) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->v = v;
    e->next = G[u];
    G[u] = e;
    deg[v]++;
}

// Kahn 算法：返回 0 表示存在环
int topo_sort_bfs(int n) {
    head = tail = 0;
    tp = 0;

    for (int i = 1; i <= n; i++) {
        if (deg[i] == 0) q[tail++] = i;
    }

    while (head < tail) {
        int u = q[head++];
        topo[tp++] = u;

        for (Edge* e = G[u]; e; e = e->next) {
            int v = e->v;
            if (--deg[v] == 0) q[tail++] = v;
        }
    }

    return tp == n;
}

int main() {
    setbuf(stdout, NULL);

    // DAG: 1->2, 1->3, 2->4, 3->4, 4->5
    add_edge(1, 2);
    add_edge(1, 3);
    add_edge(2, 4);
    add_edge(3, 4);
    add_edge(4, 5);

    if (topo_sort_bfs(5)) {
        printf("Topological order (BFS / Kahn):\n");
        for (int i = 0; i < tp; i++) {
            printf("  %d\n", topo[i]);
        }
    } else {
        printf("Cycle detected, no topological order\n");
    }

    return 0;
}
