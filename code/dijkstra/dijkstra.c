#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define maxn 1001
#define inf 0x3f3f3f3f

typedef struct Edge {
    int v, w;
    struct Edge* next;
} Edge;

Edge* G[maxn];
int dis[maxn];
int vis[maxn];

void add_edge(int u, int v, int w) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->v = v;
    e->w = w;
    e->next = G[u];
    G[u] = e;
}

void dijkstra(int s, int n) {
    memset(dis, inf, (n + 1) * sizeof(int));
    dis[s] = 0;

    for (int i = 1; i <= n; i++) {
        int u = 0, mind = inf;

        for (int j = 1; j <= n; j++) {
            if (!vis[j] && dis[j] < mind) {
                u = j;
                mind = dis[j];
            }
        }

        vis[u] = 1;

        for (Edge* e = G[u]; e; e = e->next) {
            int v = e->v, w = e->w;
            if (dis[u] + w < dis[v]) {
                dis[v] = dis[u] + w;
            }
        }
    }
}

int main() {
    setbuf(stdout, NULL);

    // 简单图: 1 -2-> 2 -1-> 3, 1 -5-> 3
    // 从 1 到 3 最短距离为 3 (经 2)
    add_edge(1, 2, 2);
    add_edge(1, 3, 5);
    add_edge(2, 3, 1);

    dijkstra(1, 3);

    printf("Shortest distances from node 1:\n");
    for (int i = 1; i <= 3; i++) {
        printf("  to %d: %d\n", i, dis[i]);
    }

    return 0;
}
