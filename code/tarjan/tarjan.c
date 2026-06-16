#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define maxn 1001

typedef struct Edge {
    int v;
    struct Edge* next;
} Edge;

Edge* G[maxn];

int dfncnt;
int dfn[maxn];
int low[maxn];

int top;
int stack[maxn];
int in_stack[maxn];

int scn;
int scc[maxn];
int size[maxn];

int min(int a, int b) { return a < b ? a : b; }

void add_edge(int u, int v) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->v = v;
    e->next = G[u];
    G[u] = e;
}

void tarjan(int u) {
    dfn[u] = low[u] = ++dfncnt;
    stack[++top] = u;
    in_stack[u] = 1;

    for (Edge* e = G[u]; e; e = e->next) {
        int v = e->v;
        if (!dfn[v]) {
            tarjan(v);
            low[u] = min(low[u], low[v]);
        } else if (in_stack[v]) {
            low[u] = min(low[u], dfn[v]);
        }
    }

    if (dfn[u] == low[u]) {
        scn++;

        do {
            int v = stack[top];

            in_stack[v] = 0;
            scc[v] = scn;
            size[scn]++;
        } while (stack[top--] != u);
    }
}

void tarjan_scc(int n) {
    dfncnt = 0;
    top = 0;
    scn = 0;
    memset(dfn, 0, (n + 1) * sizeof(int));
    memset(low, 0, (n + 1) * sizeof(int));
    memset(in_stack, 0, (n + 1) * sizeof(int));
    memset(scc, 0, (n + 1) * sizeof(int));
    memset(size, 0, (n + 1) * sizeof(int));

    for (int i = 1; i <= n; i++) {
        if (!dfn[i]) tarjan(i);
    }
}

int main() {
    setbuf(stdout, NULL);

    // 1->2->3->1 (SCC {1,2,3}), 2->4, 4->5->4 (SCC {4,5})
    add_edge(1, 2);
    add_edge(2, 3);
    add_edge(3, 1);
    add_edge(2, 4);
    add_edge(4, 5);
    add_edge(5, 4);

    int n = 5;
    tarjan_scc(n);

    printf("Strongly connected components:\n");
    for (int i = 1; i <= scn; i++) {
        printf("  SCC %d (size %d):", i, size[i]);
        for (int u = 1; u <= n; u++) {
            if (scc[u] == i) printf(" %d", u);
        }
        printf("\n");
    }

    return 0;
}