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
int total;

void add_edge(int u, int v, int w) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->v = v;
    e->w = w;
    e->next = G[u];
    G[u] = e;

    e = (Edge*)malloc(sizeof(Edge));
    e->v = u;
    e->w = w;
    e->next = G[v];
    G[v] = e;
}

void prim(int s, int n) {
    memset(dis, inf, (n + 1) * sizeof(int));
    dis[s] = 0;
    total = 0;

    for (int i = 1; i <= n; i++) {
        int u = 0, mind = inf;

        for (int j = 1; j <= n; j++) {
            if (!vis[j] && dis[j] < mind) {
                u = j;
                mind = dis[j];
            }
        }

        vis[u] = 1;
        total += dis[u];

        for (Edge* e = G[u]; e; e = e->next) {
            int v = e->v, w = e->w;
            if (!vis[v] && w < dis[v]) {
                dis[v] = w;
            }
        }
    }
}

int main() {
    setbuf(stdout, NULL);

    // 简单无向图:
    //   1 --2-- 2 --1-- 3 --2-- 4
    //   |                            |
    //   5                            |
    //   +----------------------------+
    // MST 边: 2-3(1), 3-4(2), 1-2(2), 总权 5
    add_edge(1, 2, 2);
    add_edge(1, 4, 5);
    add_edge(2, 3, 1);
    add_edge(3, 4, 2);

    prim(1, 4);

    printf("MST total weight: %d\n", total);
    printf("Edge weight to each node in MST:\n");
    for (int i = 1; i <= 4; i++) {
        printf("  node %d: %d\n", i, dis[i]);
    }

    return 0;
}
