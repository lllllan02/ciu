---
title: 图
order: 5
---

图用来表示节点（顶点）与边之间的关系，是计算机科学中许多问题的自然建模方式；本节内容较多，建议先掌握表示法与遍历，再深入最短路与最小生成树。

## 核心概念

- 对象和指针
- [x] [邻接表](https://baike.baidu.com/item/%E9%82%BB%E6%8E%A5%E8%A1%A8/9796152) (2026-06-15)
- [x] [邻接矩阵](https://baike.baidu.com/item/%E9%82%BB%E6%8E%A5%E7%9F%A9%E9%98%B5/9796080) (2026-06-15)
- 邻接图

## 深度优先搜索

- [x] [OI Wiki - 深度优先搜索](https://oi-wiki.org/graph/dfs/) (2026-06-15)

> - [x] [实现 DFS 邻接表 (递归)](https://github.com/lllllan02/ciu/tree/master/code/dfs-list)
> - [ ] DFS 邻接表 (栈迭代)
> - [ ] DFS 邻接矩阵 (递归)
> - [ ] DFS 邻接矩阵 (栈迭代)

### 拓扑排序

- [x] [OI Wiki - 拓扑排序 DFS 算法](https://oi-wiki.org/graph/topo/#dfs-%E7%AE%97%E6%B3%95) (2026-06-15)

### 强连通分量

- [x] [百度百科 - 强连通分量](https://baike.baidu.com/item/%E5%BC%BA%E8%BF%9E%E9%80%9A%E5%88%86%E9%87%8F/7448759) (2026-06-15)
- [x] [OI Wiki - Tarjan 算法](https://oi-wiki.org/graph/scc/#tarjan-%E7%AE%97%E6%B3%95) (2026-06-15)

## 最短路径 

### Dijkstra 算法

- [x] [OI Wiki - 最短路径 Dijkstra 算法](https://oi-wiki.org/graph/shortest-path/#dijkstra-%E7%AE%97%E6%B3%95) (2026-06-15)

> - [x] [实现 Dijkstra 算法](/dsa/graphs/dijkstra) (2026-06-15) 

## 最小生成树 

### Prim 算法

- [x] [OI Wiki - 最小生成树 Prim 算法](https://oi-wiki.org/graph/mst/#prim-%E7%AE%97%E6%B3%95) (2026-06-15)

## 动手实现

- [ ] BFS 邻接表
- [ ] BFS 邻接矩阵
- [ ] 最小生成树
- 基于 DFS 的算法 (根据上文 Aduni 的视频):
    - [ ] 检查环 (我们会先检查是否有环存在以便做拓扑排序)
    - [ ] 拓扑排序
    - [ ] 计算图中的连通分支
    - [ ] 列出强连通分量
    - [ ] 检查双向图

## 视频课程

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
