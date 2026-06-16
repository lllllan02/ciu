#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define maxn 1001
#define inf 0x3f3f3f3f

typedef struct Edge {
    int v, w;
    struct Edge* next;
} Edge;

Edge* G[maxn];

int queue[maxn];
int head, tail;

int dis[maxn];
int cnt[maxn];
int vis[maxn];

void add_edge(int u, int v, int w) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->v = v;
    e->w = w;
    e->next = G[u];
    G[u] = e;
}

// 返回 false 表示存在负权环
bool spfa(int s, int n) {
    memset(dis, inf, (n + 1) * sizeof(int));
    memset(cnt, 0, (n + 1) * sizeof(int));
    memset(vis, 0, (n + 1) * sizeof(int));
    head = tail = 0;

    dis[s] = 0;
    vis[s] = 1;
    queue[tail++] = s;

    while (head < tail) {
        int u = queue[head++];
        vis[u] = 0;

        for (Edge* e = G[u]; e; e = e->next) {
            int v = e->v, w = e->w;
            if (dis[u] + w < dis[v]) {
                dis[v] = dis[u] + w;
                cnt[v] = cnt[u] + 1;

                if (cnt[v] >= n) return false;

                if (!vis[v]) {
                    queue[tail++] = v;
                    vis[v] = 1;
                }
            }
        }
    }

    return true;
}

int main() {
    setbuf(stdout, NULL);

    // 含负权边: 1 -1-> 2 -2-> 3, 1 -4-> 3
    // 从 1 到 3 最短距离为 -1 (经 2)，Dijkstra 无法处理
    add_edge(1, 2, 1);
    add_edge(2, 3, -2);
    add_edge(1, 3, 4);

    if (spfa(1, 3)) {
        printf("Shortest distances from node 1 (SPFA):\n");
        for (int i = 1; i <= 3; i++) {
            printf("  to %d: %d\n", i, dis[i]);
        }
    } else {
        printf("Negative cycle detected\n");
    }

    return 0;
}
