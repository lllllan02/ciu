---
title: Tarjan 算法
order: 4
---

Tarjan 算法在 **一次 DFS** 中求出有向图的全部 **强连通分量（SCC）**：极大顶点集合，其中任意两点互相可达。缩点后得到 DAG，常用于 2-SAT、依赖分析、判环后的图压缩等。

本仓库实现基于 **邻接表 + DFS + 栈**，与拓扑排序等 C 实现风格一致。

**特性**：时间 $O(V + E)$ · 空间 $O(V)$ · 单次 DFS · 可顺带求缩点图

> 源码：[Tarjan 实现 (C)](https://github.com/lllllan02/ciu/tree/master/code/tarjan)

## 示例图

```
1 → 2 → 3 ↺ 1
      ↓
      4 ↔ 5
```

边集：$1 \to 2,\ 2 \to 3,\ 3 \to 1,\ 2 \to 4,\ 4 \to 5,\ 5 \to 4$。两个 SCC：$\{1,2,3\}$ 与 $\{4,5\}$。

## 核心概念

对每个顶点 $u$ 维护：

| 数组 | 含义 |
| :--- | :--- |
| `dfn[u]` | DFS 时间戳，首次访问顺序 |
| `low[u]` | $u$ 所在子树能追溯到的 **最小** `dfn`（经树边或栈内回边） |
| `in_stack[u]` | $u$ 是否在当前 DFS 栈中 |
| `scc[u]` | $u$ 所属 SCC 编号 |
| `size[k]` | 第 $k$ 个 SCC 的顶点数 |

另用栈 `stack[]` 记录当前 DFS 路径上的顶点。

## 思路

1. 从每个未访问顶点出发 DFS。进入 $u$ 时：`dfn[u] = low[u] = ++dfncnt`，$u$ 入栈。
2. 遍历每条出边 $u \to v$。处理前先记住：`low[u]` 回答一个问题 —— ==u 能不能和栈里更早访问的某个节点划进同一个 SCC？== 能的话，`low[u]` 就记下这些节点里最小的 `dfn`。下面按 $v$ 的三种状态分别处理：

   - **$v$ 未访问**（`!dfn[v]`）：$u \to v$ 是 **树边**。$u$ 还不知道 $v$ 子树里有没有回边，得先递归 `tarjan(v)` 把 $v$ 那边探完；回来后用 `low[u] = min(low[u], low[v])`。
   
    > 这里用的是 **`low[v]`**——$v$ 子树全部探完后汇总的结论：「我这边最远能连回多早的节点」。

   - **$v$ 已访问且在栈中**（`in_stack[v]`）：$u \to v$ 是 **回边**，不用进 $v$，当前这条边本身就是证据：$u$ 能 **直接** 连到栈里的 $v$。所以用 `low[u] = min(low[u], dfn[v])`
   
   > 记的是 **`dfn[v]`**——「$u$ 至少能连到 $v$ 这个祖先」。
   
   > [!TIP] dfn 和 low 的选择
   > 
   > 这里写 `dfn[v]` 而不是 `low[v]`，是 **不必** 借 `low[v]`，不是 **不能**：若 $v$ 自己还能连到更早的节点，那已经记在 `low[v]` 里，会沿 DFS 树在递归返回时 **往上传**；$u$ 最终照样能收到。回边处若硬写成 `low[v]`，结果通常也对（因为 `low[v] ≤ dfn[v]`），但语义变成「借祖先的汇总」，不如 `dfn[v]` 直白——这条边只证明「$u$ 能连到 $v$」。

   - **$v$ 已访问但不在栈中**（`!in_stack[v]`）：$v$ 已被 **收栈** 并划入某个 SCC。$u \to v$ 是 **交叉边 / 前向边**，指向已处理完的分量。$v$ 所在 SCC 与当前栈上的节点 **不可能** 再合并成更大的 SCC（否则 $v$ 不会先于它们出栈），这条边对 $u$ 的 SCC 判定 **没有贡献**，故 **不更新** `low[u]`。
3. **收栈**：若 `dfn[u] == low[u]`，说明 $u$ 是所在 SCC 的「根」——栈中从 $u$ 到栈顶的顶点构成一个 SCC，依次出栈并赋编号。

整张图对每个未访问顶点调用一次 `tarjan(i)` 即可。

## 邻接表

```c
typedef struct Edge {
    int v;
    struct Edge* next;
} Edge;

Edge* G[maxn];
```

建边时将新边头插到 `G[u]`，与拓扑排序等实现一致。

## 核心代码

```c
void tarjan(int u) {
    // 1. 首次访问：打时间戳，入栈
    dfn[u] = low[u] = ++dfncnt;
    stack[++top] = u;
    in_stack[u] = 1;

    for (Edge* e = G[u]; e; e = e->next) {
        int v = e->v;
        if (!dfn[v]) {
            tarjan(v);
            low[u] = min(low[u], low[v]);  // 树边：听子树汇报
        } else if (in_stack[v]) {
            low[u] = min(low[u], dfn[v]);  // 回边：u 能直接连到栈内祖先 v
        }
        // 已出栈：交叉边 / 前向边，不更新 low[u]
    }

    // 2. u 是所在 SCC 的根，收栈
    if (dfn[u] == low[u]) {
        scn++;
        do {
            int v = stack[top];
            in_stack[v] = 0;
            scc[v] = scn;
            size[scn]++;
        } while (stack[top--] != u);  // 弹出 [u, 栈顶] 构成一个 SCC
    }
}
```

外层对每个 `!dfn[i]` 的顶点调用 `tarjan(i)`。`dfn[u] == low[u]` 时，栈顶到 $u$ 的顶点同属一个 SCC。
