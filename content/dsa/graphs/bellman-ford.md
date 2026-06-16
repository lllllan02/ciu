---
title: Bellman-Ford 算法
order: 2
---

Bellman-Ford 算法同样解决 **单源最短路径** 问题，但 **允许负权边**，并能检测 **负权环**（从某点出发沿环走一圈总权值为负，最短路无定义）。[Dijkstra](/dsa/graphs/dijkstra) 的贪心「定型」在负权边下会失效，此时应改用 Bellman-Ford 或其队列优化版 SPFA。

本仓库实现采用 **SPFA**（Shortest Path Faster Algorithm）：只对可能被更新的顶点入队松弛，平均快于朴素 Bellman-Ford，最坏仍可能退化。

**特性**：朴素 Bellman-Ford 时间 $O(VE)$ · SPFA 平均 $O(kE)$、最坏 $O(VE)$ · 可处理负权边 · 可检测负权环

> 源码：[SPFA 实现 (C)](https://github.com/lllllan02/ciu/tree/master/code/bellmanford)

## 示例图

实现使用一张含 **负权边** 的图，Dijkstra 无法正确处理：

```
1 -1-> 2 -2-> 3
1 -4-> 3
```

边集：$1 \to 2$（权 1）、$2 \to 3$（权 $-2$）、$1 \to 3$（权 4）。从 1 到 3 的最短路径为 $1 \to 2 \to 3$，距离为 $1 + (-2) = -1$，优于直达的 4。

## 朴素 Bellman-Ford

### 思路

基于 **松弛**（与 Dijkstra 相同），但不做「选最小 dis 定型」，而是反复尝试缩短距离：

1. 初始化 `dis[s] = 0`，其余为 $\infty$。
2. 重复 $V - 1$ 轮：每轮遍历 **全部边** $u \to v$，若 `dis[u] + w < dis[v]` 则更新 `dis[v]`。
3. 直觉：不含负权环时，最短路最多经过 $V - 1$ 条边；每轮至少确定一条边的最短贡献。
4. **第 $V$ 轮** 再扫一遍：若仍能松弛，说明存在可无限绕行的负权环。

朴素版通常用 **边集** 存图，便于逐条遍历。

### 核心代码

```c
typedef struct {
    int u, v, w;
} E;

E edges[maxn];
int ecnt;

// 返回 false 表示存在负权环
bool bellman_ford(int s, int n) {
    memset(dis, inf, (n + 1) * sizeof(int));
    dis[s] = 0;

    // 1. 最多 V-1 轮全局松弛
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < ecnt; j++) {
            int u = edges[j].u, v = edges[j].v, w = edges[j].w;
            if (dis[u] + w < dis[v]) {
                dis[v] = dis[u] + w;
            }
        }
    }

    // 2. 第 V 轮仍能松弛 → 负权环
    for (int j = 0; j < ecnt; j++) {
        int u = edges[j].u, v = edges[j].v, w = edges[j].w;
        if (dis[u] + w < dis[v]) return false;
    }

    return true;
}
```

时间 $O(VE)$，实现简单，适合边数不多或需要严格最坏界保证的场景。

## SPFA

### 思路

观察朴素版：若某轮松弛中 `dis[v]` 未变，则下一轮从以 $v$ 为起点的边出发也不会产生新更新。SPFA 据此 **只把被更新过的顶点入队**，从队列取出后再松弛其出边：

1. 起点入队，`dis[s] = 0`。
2. 出队 $u$，标记「不在队中」；遍历 $u$ 的每条出边 $u \to v$。
3. 若绕道 $u$ 能缩短 `dis[v]`，则更新并入队（若 $v$ 尚不在队中）。
4. **负权环检测**：记录每个顶点被成功松弛的次数 `cnt[v]`。最短路最多 $V - 1$ 条边，故正常时 `cnt[v] < n`；若 `cnt[v] >= n`，说明 $v$ 被反复缩短，存在负权环。

### 邻接表

```c
typedef struct Edge {
    int v, w;
    struct Edge* next;
} Edge;

Edge* G[maxn];
```

与 Dijkstra、拓扑排序等 C 实现风格一致。

### 核心代码

```c
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

                if (cnt[v] >= n) return false;  // 松弛次数过多，负权环

                if (!vis[v]) {
                    queue[tail++] = v;
                    vis[v] = 1;
                }
            }
        }
    }

    return true;
}
```

此处 `vis[v]` 表示 **$v$ 是否在队列中**，与 Dijkstra 中「已定型」含义不同。

### 运行结果

```
Shortest distances from node 1 (SPFA):
  to 1: 0
  to 2: 1
  to 3: -1
```

## 与 Dijkstra 对比

| | Bellman-Ford / SPFA | Dijkstra |
| :--- | :--- | :--- |
| **负权边** | 支持 | 不支持 |
| **负权环** | 可检测 | 不适用 |
| **核心策略** | 反复松弛，不提前定型 | 贪心定型最小 `dis` |
| **时间复杂度** | 朴素 $O(VE)$；SPFA 平均较快、最坏 $O(VE)$ | 朴素 $O(V^2)$；堆优化 $O((V+E)\log V)$ |
| **典型用途** | 负权图、差分约束、小规模图 | 非负权图、稀疏大图 |

实际选型时：边权全非负，优先 Dijkstra；出现负权边或需要判负权环，用 Bellman-Ford 或 SPFA。竞赛中 SPFA 常用，但需注意特殊数据下可能超时，此时回退朴素 Bellman-Ford 更稳妥。
