---
title: 图
order: 5
---

图用来表示节点（顶点）与边之间的关系，是计算机科学中许多问题的自然建模方式；本节内容较多，建议先掌握表示法与遍历，再深入最短路与最小生成树。

## 核心概念

图 $G = (V, E)$ 由**顶点集** $V$ 与**边集** $E$ 组成，可**有向**或**无向**、**带权**或**无权**。常见存储方式：**邻接矩阵**用 $V \times V$ 数组存边，查边 $O(1)$、适合稠密图；**邻接表**为每个顶点维护邻居列表，空间 $O(V + E)$、适合稀疏图。实现上常用数组 + 链表/动态数组，或用「对象 + 指针」表达节点与边的引用关系。

- 对象和指针
- [x] [邻接表](https://baike.baidu.com/item/%E9%82%BB%E6%8E%A5%E8%A1%A8/9796152) (2026-06-15)
- [x] [邻接矩阵](https://baike.baidu.com/item/%E9%82%BB%E6%8E%A5%E7%9F%A9%E9%98%B5/9796080) (2026-06-15)
- 邻接图

## 深度优先搜索

**深度优先搜索（DFS）** 从起点出发，沿一条路径尽可能深入，走不通时回溯，直到访问完所有可达顶点。常用**递归**或**显式栈**实现，配合 `visited` 标记避免重复访问。DFS 是拓扑排序、强连通分量、连通性判定等算法的基础，遍历一张含 $V$ 个顶点、$E$ 条边的图时间复杂度为 $O(V + E)$。

- [x] [OI Wiki - 深度优先搜索](https://oi-wiki.org/graph/dfs/) (2026-06-15)

> - [x] [实现 DFS 邻接表 (递归)](https://github.com/lllllan02/ciu/tree/master/code/dfs-list)
> - [ ] DFS 邻接表 (栈迭代)
> - [ ] DFS 邻接矩阵 (递归)
> - [ ] DFS 邻接矩阵 (栈迭代)

## 广度优先搜索

**广度优先搜索（BFS）** 从起点出发，先访问距离为 1 的邻居，再访问距离为 2 的邻居，依此类推，天然按**层**扩展。用**队列**维护待访问顶点；在**无权图**上，BFS 首次到达某顶点时的路径即为最短路径。时间复杂度同样为 $O(V + E)$。

- [x] [OI Wiki - 广度优先搜索](https://oi-wiki.org/graph/bfs/) (2026-06-16)

> - [x] [实现 BFS 邻接表](https://github.com/lllllan02/ciu/tree/master/code/bfs-list) (2026-06-16)
> - [ ] BFS 邻接矩阵

## 拓扑排序

**拓扑排序**将**有向无环图（DAG）** 的顶点排成线性序列，使得每条边 $u \to v$ 中 $u$ 都出现在 $v$ 之前。典型应用包括任务调度、课程先修关系、编译依赖解析。常见做法：**DFS 逆后序**（深度优先完成后入栈再弹出）与 **Kahn 算法**（反复取入度为 0 的顶点并删边），二者时间均为 $O(V + E)$；若无法排出完整序列，说明图中存在环。

- [x] [OI Wiki - 拓扑排序 DFS 算法](https://oi-wiki.org/graph/topo/#dfs-%E7%AE%97%E6%B3%95) (2026-06-15)
- [x] [OI Wiki - 拓扑排序 kahn 算法](https://oi-wiki.org/graph/topo/#kahn-%E7%AE%97%E6%B3%95) (2026-06-16)
- [x] [拓扑排序](https://blog.csdn.net/lisonglisonglisong/article/details/45543451) (2026-06-16)

> - [x] [实现拓扑排序](/dsa/graphs/topo-sort) (2026-06-16)

## 最短路径

**最短路径**问题求图中两点（或单源到各点）之间权值之和最小的路径。根据图是否带权、是否存在负权边、是否需要多源结果，选用不同算法：无权图用 BFS；单源非负权用 Dijkstra；单源可含负权用 Bellman-Ford；全源最短路径用 Floyd。

### Floyd 算法

**Floyd 算法**（Floyd-Warshall）基于**动态规划**，依次尝试以每个顶点 $k$ 为中转，更新任意两点 $i, j$ 的最短距离。适用于**多源最短路**及带负权（无负权环）的稠密图，时间 $O(V^3)$、空间 $O(V^2)$，实现简洁，适合顶点数较少（通常 $V \le 500$）的场景。

- [x] [OI Wiki - 最短路径 Floyd 算法](https://oi-wiki.org/graph/shortest-path/#floyd-%E7%AE%97%E6%B3%95) (2026-06-16)

> - [x] [实现 Floyd 算法](https://github.com/lllllan02/ciu/tree/master/code/floyd) (2026-06-16)

### Bellman-Ford 算法

**Bellman-Ford 算法**解决**单源最短路径**，**允许负权边**，并能检测**负权环**（沿环走一圈总权值为负，最短路无定义）。核心是对全部边反复**松弛** $V - 1$ 轮；队列优化版 SPFA 平均更快，最坏仍可能退化到 $O(VE)$。

- [x] [Bellman-Ford 算法](https://www.geeksforgeeks.org/bellman-ford-algorithm-dp-23/) (2026-06-16)

> - [x] [实现 Bellman-Ford 算法](/dsa/graphs/bellman-ford) (2026-06-16)

### Dijkstra 算法

**Dijkstra 算法**解决**单源最短路径**，要求边权**非负**。贪心地每次「定型」当前距离最小的未处理顶点，并对其出边松弛；朴素版 $O(V^2)$，配合**小根堆**可优化至 $O((V + E) \log V)$，是工程中最常用的最短路算法之一。

- [x] [OI Wiki - 最短路径 Dijkstra 算法](https://oi-wiki.org/graph/shortest-path/#dijkstra-%E7%AE%97%E6%B3%95) (2026-06-15)

> - [x] [实现 Dijkstra 算法](/dsa/graphs/dijkstra) (2026-06-16)

## 强连通分量

**强连通分量（SCC）** 是有向图中极大强连通子图：子图内任意两点互相可达。缩点后将 SCC 视为一个「超级顶点」，原图变为 DAG，便于分析依赖与层次结构。常见求法有 **Tarjan** 与 **Kosaraju**，均为一次 DFS 思路，时间 $O(V + E)$。

- [x] [百度百科 - 强连通分量](https://baike.baidu.com/item/%E5%BC%BA%E8%BF%9E%E9%80%9A%E5%88%86%E9%87%8F/7448759) (2026-06-15)

### Tarjan 算法

**Tarjan 算法**在一次 DFS 中用 **dfn**（发现时间）与 **low**（能回溯到的最早 dfn）判定 SCC：当 `dfn[u] == low[u]` 时，$u$ 为某个 SCC 的根，弹出栈中该分量全部顶点。只需一次深度优先搜索，常数较小，是竞赛与工程中常用的 SCC 模板。

- [x] [OI Wiki - Tarjan 算法](https://oi-wiki.org/graph/scc/#tarjan-%E7%AE%97%E6%B3%95) (2026-06-15)

> - [x] [实现 Tarjan 算法](/dsa/graphs/tarjan) (2026-06-16)

## 最小生成树

**最小生成树（MST）** 是连通**无向带权图**的一棵生成树，使所有边的权值之和最小。只有连通无向图才有 MST；不连通时每个连通分量各有一棵。经典算法 **Kruskal**（按边权排序 + 并查集）与 **Prim**（类似 Dijkstra 的贪心扩点），时间均可达 $O(E \log V)$。

### Kruskal 算法

**Kruskal 算法**将边按权值**升序**排序，依次尝试加入当前边：若两端点不在同一连通分量（用**并查集**判定），则加入生成树。适合**稀疏图**，实现直观，与边数相关的排序与并查集操作主导复杂度。

- [x] [OI Wiki - 最小生成树 Kruskal 算法](https://oi-wiki.org/graph/mst/#kruskal-%E7%AE%97%E6%B3%95) (2026-06-16)

### Prim 算法

**Prim 算法**从任意顶点出发，每次选择连接「已在树中」与「未在树中」顶点之间的**最小权边**，将新顶点并入生成树，直至覆盖全部顶点。配合**优先队列**效率更高，在**稠密图**上常优于 Kruskal。

- [x] [OI Wiki - 最小生成树 Prim 算法](https://oi-wiki.org/graph/mst/#prim-%E7%AE%97%E6%B3%95) (2026-06-15)

> - [x] [实现 Prim 算法](https://github.com/lllllan02/ciu/tree/master/code/prim) (2026-06-16)

## 视频课程

以下为 MIT、Skiena 等公开课程中的图论相关视频，可作为阅读与实现的补充；建议配合上文各节概念与实现对照学习。

MIT 课程

- [广度优先搜索](https://www.youtube.com/watch?v=s-CYnVz-uh4&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=13)
- [深度优先搜索](https://www.youtube.com/watch?v=AfSk24UTFS8&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=14)

Skiena 课程

- [ ] [CSE373 2012 - 课程 11 - 图的数据结构（视频）](https://www.youtube.com/watch?v=OiXxhDrFruw&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b&index=11)
- [ ] [CSE373 2012 - 课程 12 - 广度优先搜索（视频）](https://www.youtube.com/watch?v=g5vF8jscteo&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b&index=12)
- [ ] [CSE373 2012 - 课程 13 - 图的算法（视频）](https://www.youtube.com/watch?v=S23W6eTcqdY&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b&index=13)
- [ ] [CSE373 2012 - 课程 14 - 图的算法 (1)（视频）](https://www.youtube.com/watch?v=WitPBKGV0HY&index=14&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b)
- [ ] [CSE373 2012 - 课程 15 - 图的算法 (2)（视频）](https://www.youtube.com/watch?v=ia1L30l7OIg&index=15&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b)
- [ ] [CSE373 2012 - 课程 16 - 图的算法 (3)（视频）](https://www.youtube.com/watch?v=jgDOQq6iWy8&index=16&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b)

复习与经典算法

- [ ] [6.006 单源最短路径问题（视频）](https://www.youtube.com/watch?v=Aa2sqUhIn-E&index=15&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [6.006 Dijkstra 算法（视频）](https://www.youtube.com/watch?v=NSHizBK9JD8&t=1731s&ab_channel=MITOpenCourseWare)
- [ ] [6.006 Bellman-Ford 算法（视频）](https://www.youtube.com/watch?v=f9cVS_URPc0&ab_channel=MITOpenCourseWare)
- [ ] [6.006 加速 Dijkstra 算法（视频）](https://www.youtube.com/watch?v=CHvQ3q_gJ7E&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=18)
- [ ] [Aduni：图算法 I - 拓扑排序，最小生成树，Prim 算法 - 讲座 6（视频）](https://www.youtube.com/watch?v=i_AQT_XfvD8&index=6&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm)
- [ ] [Aduni：图算法 II - DFS，BFS，Kruskal 算法，Union Find 数据结构 - 讲座 7（视频）](https://www.youtube.com/watch?v=ufj5_bppBsA&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=7)
- [ ] [Aduni：图算法 III：最短路径 - 讲座 8（视频）](https://www.youtube.com/watch?v=DiedsPsMKXc&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=8)
- [ ] [Aduni：图算法 IV：几何算法入门 - 讲座 9（视频）](https://www.youtube.com/watch?v=XIAQRlNkJAw&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=9)
- [ ] [CS 61B 2014：加权图（视频）](https://archive.org/details/ucberkeley_webcast_zFbq8vOZ_0k)
- [ ] [贪婪算法：最小生成树（视频）](https://www.youtube.com/watch?v=tKwnms5iRBU&index=16&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)
- [ ] [强连通分量 Kosaraju 算法图算法（视频）](https://www.youtube.com/watch?v=RpgcYiky7uw)
- [ ] [[复习] 最短路径算法（播放列表）16 分钟（视频）](https://www.youtube.com/playlist?list=PL9xmBV_5YoZO-Y-H3xIC9DGSfVYJng9Yw)
- [ ] [[复习] 最小生成树（播放列表）4 分钟（视频）](https://www.youtube.com/playlist?list=PL9xmBV_5YoZObEi3Hf6lmyW-CBfs7nkOV)

Coursera 课程

- [ ] [图的算法（视频）](https://www.coursera.org/learn/algorithms-on-graphs/home/welcome)
