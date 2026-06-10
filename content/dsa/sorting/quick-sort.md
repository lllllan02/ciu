---
title: 快速排序
order: 3
---

> 源码: https://github.com/lllllan02/ciu/tree/master/code/quick-sort
>
> 参考: [菜鸟教程 - 快速排序](https://www.runoob.com/w3cnote/quick-sort-2.html)

快速排序（Quick Sort）是一种 **分治** 比较排序算法：选取 **基准（pivot）**，将数组 **分区** 为「小于基准」「等于基准」「大于基准」三部分，再对两侧子区间继续排序。平均时间复杂度 $O(n \log n)$，最坏 $O(n^2)$；标准实现 **不稳定**，适合随机访问的数组。

本实现采用 **迭代 + 显式栈** 代替递归，避免深层递归栈开销；基准取当前区间 **中点元素值**，分区时双指针向中间扫描并交换。

## API

| 函数 | 说明 |
| :--- | :--- |
| `quick_sort(arr, len)` | 对 `arr[0..len-1]` 升序排序 |

## 算法步骤

整体流程：用栈保存待排序区间，每次弹出一个区间做分区，再将两侧未就绪的子区间压栈，直到栈空。

1. **初始化**：将 `[0, len - 1]` 压入栈；栈空间预分配 `len` 个 `Range`。
2. **弹出区间**：若 `start >= end`（长度 ≤ 1），跳过。
3. **选基准**：`mid = arr[(start + end) / 2]`，取中点 **元素值** 作为 pivot。
4. **分区**：`left` 从 `start`、`right` 从 `end` 向中间扫描：
   - `arr[left] < mid` 时 `left++`；
   - `arr[right] > mid` 时 `right--`；
   - 若 `left <= right`，交换 `arr[left]` 与 `arr[right]`，然后 `left++`、`right--`。
5. **入栈子区间**：若 `left < end`，压入 `[left, end]`；若 `right > start`，压入 `[start, right]`。

分区完成后，`[start, right]` 内元素均 ≤ pivot，`[left, end]` 内元素均 ≥ pivot；两侧子区间不含已就位的 pivot 段，避免重复处理。

### 分区（双指针）

```c
int left = range.start, right = range.end;
int mid = arr[(range.start + range.end) / 2];
while (left <= right) {
    while (arr[left] < mid) left++;
    while (arr[right] > mid) right--;

    if (left <= right) {
        swap(arr + left, arr + right);
        left++, right--;
    }
}

if (left < range.end) ranges[index++] = new_range(left, range.end);
if (right > range.start) ranges[index++] = new_range(range.start, right);
```

### 迭代主循环

```c
Range *ranges = malloc(len * sizeof(Range));
int index = 0;
ranges[index++] = new_range(0, len - 1);

while (index) {
    Range range = ranges[--index];
    if (range.start >= range.end) continue;

    // 分区（见上）
}
```

## 与递归快排的对比

| | 递归快排 | 本实现（迭代） |
| :--- | :--- | :--- |
| 调用方式 | 递归 `[start, j]`、`[i, end]` | 显式栈保存 `Range` |
| 栈空间 | $O(\log n)$ 平均，最坏 $O(n)$ 系统栈 | $O(n)$ 堆上预分配 |
| 基准选取 | 常见：首/尾/随机 | 区间中点元素值 |
| 分区方式 | 双指针交换 | 同上，等价思路 |

## 复杂度分析

设数组长度为 $n$。

| 项目 | 平均 | 最坏 | 说明 |
| :--- | :--- | :--- | :--- |
| **时间** | $O(n \log n)$ | $O(n^2)$ | 每次分区 $O(k)$；最坏为每次只缩小 1 个元素（如已有序 + 固定基准） |
| **额外空间** | $O(\log n)$ | $O(n)$ | 迭代栈最多同时保存 $O(\log n)$ 个区间；预分配 $O(n)$ 缓冲区 |
| **稳定性** | — | 不稳定 | 交换可能打乱相等元素的相对顺序 |

中点基准在随机或一般数据上表现较好；若需保证最坏 $O(n \log n)$，可改用随机基准或三数取中，或改用归并排序。

## 测试

```bash
cd code/quick-sort
gcc -Wall -Wextra -std=c11 -o main main.c
./main
```

测试覆盖：基本乱序、已有序、逆序、重复元素，以及空数组、单元素、含负数等边界情况。
