---
title: Dijkstra 算法
order: 3
---

Dijkstra 算法解决 **单源最短路径** 问题：给定带权有向图与起点 $s$，求 $s$ 到每个顶点的最短距离。要求边权 **非负**；若存在负权边，应改用 [Bellman-Ford](/dsa/graphs/bellman-ford)。

本仓库提供两种实现——**朴素选点法** 与 **优先队列（小根堆）法**，示例图相同，结果一致。

**特性**：朴素版时间 $O(V^2)$ · 堆优化 $O((V + E) \log V)$ · 仅适用于非负权图

> 源码：[朴素实现 (C)](https://github.com/lllllan02/ciu/tree/master/code/dijkstra)、[优先队列实现 (C++)](https://github.com/lllllan02/ciu/tree/master/code/dijkstra-pq)

## 示例图

两份实现使用同一张图便于对比：

```
1 -2-> 2 -1-> 3
1 -5-> 3
```

边集：$1 \to 2$（权 2）、$1 \to 3$（权 5）、$2 \to 3$（权 1）。从 1 到 3 的最短路径为 $1 \to 2 \to 3$，距离为 3。

## 公共思路

两种实现共享同一贪心框架：

1. 维护 `dis[]`：当前已知的从起点到各顶点的最短距离，初始 `dis[s] = 0`，其余为 $\infty$。
2. 每次 **确定** 一个尚未处理、且 `dis` 最小的顶点 $u$（即「当前最近且未定型」的顶点）。
3. **松弛** $u$ 的出边：对每个邻居 $v$，想一想「先到 $u$（已是最短），再走 $u \to v$ 这条边」会不会更近；若 `dis[u] + w` 比当前 `dis[v]` 更小，就把 `dis[v]` 改成这个更短的距离。
4. 重复直到所有可达顶点都被定型。

区别在于第 2 步如何找 $u$：朴素版线性扫描全部顶点；堆优化版用小根堆按 `dis` 取最小。

## 朴素实现（C）

### 邻接表

```c
typedef struct Edge {
    int v, w;
    struct Edge* next;
} Edge;

Edge* G[maxn];
```

建边时将新边头插到 `G[u]`，与拓扑排序等 C 实现风格一致。

### 核心代码

用 `vis[]` 标记已定型顶点；每轮在未访问顶点中线性找 `dis` 最小者：

```c
void dijkstra(int s, int n) {
    memset(dis, inf, (n + 1) * sizeof(int));
    dis[s] = 0;

    for (int i = 1; i <= n; i++) {  // 共定型 n 个顶点
        // 1. 在未定型顶点中找 dis 最小者
        int u = 0, mind = inf;

        for (int j = 1; j <= n; j++) {
            if (!vis[j] && dis[j] < mind) {
                u = j;
                mind = dis[j];
            }
        }

        vis[u] = 1;  // 定型：此后 dis[u] 不再变化

        // 2. 绕道 u 尝试缩短到各邻居的距离
        for (Edge* e = G[u]; e; e = e->next) {
            int v = e->v, w = e->w;
            if (dis[u] + w < dis[v]) {
                dis[v] = dis[u] + w;
            }
        }
    }
}
```

外层循环 $n$ 次，内层扫描 $O(V)$，松弛总 $O(E)$，故时间 $O(V^2 + E)$；稠密图 $E \approx V^2$ 时即为 $O(V^2)$。

### 运行结果

```
Shortest distances from node 1:
  to 1: 0
  to 2: 2
  to 3: 3
```

## 优先队列实现（C++）

### 邻接表

```cpp
vector<pair<int, int>> G[maxn];  // (终点, 边权)
```

### 核心代码

用 `priority_queue` 配合 `greater<>` 实现小根堆，堆中存 `(距离, 顶点)`。同一顶点可能被多次入堆；弹出时若 `d > dis[u]` 说明是 **过期记录**，直接跳过（惰性删除）：

```cpp
void dijkstra(int s, int n) {
    fill(dis + 1, dis + n + 1, inf);
    dis[s] = 0;

    // 小根堆，存 (当前距离, 顶点)
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.emplace(0, s);

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dis[u]) continue;  // 过期记录：已有更短路径到达 u

        // 绕道 u 尝试缩短到各邻居的距离
        for (auto [v, w] : G[u]) {
            if (dis[u] + w < dis[v]) {
                dis[v] = dis[u] + w;
                pq.emplace(dis[v], v);  // 入堆；同一顶点的旧记录靠上一行跳过
            }
        }
    }
}
```

每次堆操作 $O(\log V)$，最多 $O(E)$ 次入堆，总时间 $O((V + E) \log V)$。稀疏图上明显优于朴素版。

### 运行结果

```
Shortest distances from node 1 (priority queue):
  to 1: 0
  to 2: 2
  to 3: 3
```

## 两种实现对比

| | 朴素选点 (C) | 优先队列 (C++) |
| :--- | :--- | :--- |
| **选点方式** | 每轮线性扫描 `vis` 外最小 `dis` | 小根堆取当前最小 `dis` |
| **辅助结构** | `vis[]` | `priority_queue`，无需 `vis` |
| **过期处理** | 每顶点只定型一次 | 惰性删除：`d > dis[u]` 时跳过 |
| **时间复杂度** | $O(V^2 + E)$ | $O((V + E) \log V)$ |
| **适用场景** | 稠密图、$V$ 较小、无堆依赖 | 稀疏图、$E \ll V^2$、竞赛/工程常用 |

实际选型时：顶点数不大或图较稠密，朴素版代码更短、常数更小；边数远小于 $V^2$ 时，堆优化版渐近更优。本仓库将朴素版保留为 C 邻接表实现，堆优化版单独放在 C++ 目录，便于直接复用 STL。
