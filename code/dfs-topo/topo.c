#include <stdio.h>
#include <stdlib.h>

#define maxn 1001

typedef struct Edge {
    int v;
    struct Edge* next;
} Edge;

Edge* G[maxn];
int vis[maxn];   // 0 未访问, 1 正在访问, 2 已完成
int topo[maxn];
int tp;

void add_edge(int u, int v) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->v = v;
    e->next = G[u];
    G[u] = e;
}

// 返回 0 表示存在环
int dfs(int u) {
    vis[u] = 1;

    for (Edge* e = G[u]; e; e = e->next) {
        int v = e->v;
        if (vis[v] == 1) return 0;
        if (vis[v] == 0 && !dfs(v)) return 0;
    }

    vis[u] = 2;
    topo[tp++] = u;
    return 1;
}

// 返回 0 表示存在环
int topo_sort_dfs(int n) {
    tp = 0;
    for (int i = 1; i <= n; i++) {
        if (vis[i] == 0 && !dfs(i)) return 0;
    }
    return 1;
}

int main() {
    setbuf(stdout, NULL);

    // DAG: 1->2, 1->3, 2->4, 3->4, 4->5
    add_edge(1, 2);
    add_edge(1, 3);
    add_edge(2, 4);
    add_edge(3, 4);
    add_edge(4, 5);

    if (topo_sort_dfs(5)) {
        printf("Topological order (DFS):\n");
        for (int i = tp - 1; i >= 0; i--) {
            printf("  %d\n", topo[i]);
        }
    } else {
        printf("Cycle detected, no topological order\n");
    }

    return 0;
}
