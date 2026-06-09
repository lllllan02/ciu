---
title: 算法技巧
description: 二分查找、按位运算。
---

### 二分查找（Binary search）

二分查找在 **已排序** 的序列上工作：每次取中间元素与目标比较，根据大小关系丢弃左半或右半，直到找到或区间为空。每轮比较都能排除约一半元素，时间复杂度 $O(\log n)$；用循环实现时额外空间只需 $O(1)$。这是分治思想的典型应用，也是找边界、最小满足条件等许多算法题的基础模板。

- [x] [OI Wiki - 二分查找](https://oi-wiki.org/basic/binary/) (2026-06-01)
- [ ] [二分查找（视频）](https://www.youtube.com/watch?v=D5SrAga1pno)
- [ ] [二分查找（视频）](https://www.khanacademy.org/computing/computer-science/algorithms/binary-search/a/binary-search)
- [ ] [详情](https://www.topcoder.com/community/data-science/data-science-tutorials/binary-search/)
- [ ] [蓝图](https://leetcode.com/discuss/general-discussion/786126/python-powerful-ultimate-binary-search-template-solved-many-problems)
- [ ] [【复习】四分钟二分查找(视频)](https://youtu.be/fDKIpRe8GW4)

> - [x] [实现二分查找](/dsa/techniques/binary-search) (2026-06-02)

### 按位运算（Bitwise operations）

按位运算在底层表示、掩码、权限标志和性能优化里很常见；面试里常考 2 的幂、符号位与经典 bit trick。下面按「进制与速查 → 运算符 → 补码 → 技巧 → 置位与交换」顺序展开。

#### 2 的幂与速查

- [x] [OI Wiki - 进位制](https://oi-wiki.org/math/numeral-sys/base/) (2026-06-04)
- [ ] [Bits 速查表](https://github.com/jwasham/coding-interview-university/blob/main/extras/cheat%20sheets/bits-cheat-sheet.pdf) ── 你需要知道大量 2 的幂数值（从 2^1 到 2^16 及 2^32）

#### 位运算符

好好理解位操作符的含义：&、|、^、~、>>、<<

- [x] [菜鸟教程 - 位运算](https://www.runoob.com/w3cnote/bit-operation.html) (2026-06-03)
- [x] [OI Wiki - 位操作](https://oi-wiki.org/misc/bit/) (2026-06-03)
- [ ] [Wikipedia - 位操作](https://en.wikipedia.org/wiki/Bit_manipulation)
- [ ] [Wikipedia - 按位运算](https://en.wikipedia.org/wiki/Bitwise_operation)
- [ ] [字码（words）](https://en.wikipedia.org/wiki/Word_(computer_architecture))
- [ ] [位操作（视频）](https://www.youtube.com/watch?v=7jkIUgLC29I)
- [ ] [C 语言编程教程 2-10：按位运算（视频）](https://www.youtube.com/watch?v=d0AwjSpNXR0)
- [ ] [位元抚弄者（The Bit Twiddler）](http://bits.stephan-brumme.com/)
- [ ] [交互式位元抚弄者（The Bit Twiddler Interactive）](http://bits.stephan-brumme.com/interactive.html)
- [ ] [练习位操作](https://pconrad.github.io/old_pconrad_cs16/topics/bitOps/)
- [x] [绝对整型（Absolute Integer）](/dsa/techniques/bitwise-absolute-integer) (2026-06-03)

#### 补数与补码

- [x] [菜鸟教程 - 原码、反码、补码](https://www.runoob.com/w3cnote/sign-magnitude.html) (2026-06-04)
- [ ] [补数（1s Complement）](https://en.wikipedia.org/wiki/Ones%27_complement)
- [ ] [补码（2s Complement）](https://en.wikipedia.org/wiki/Two%27s_complement)
- [ ] [二进制：利 & 弊（为什么我们要使用补码）（视频）](https://www.youtube.com/watch?v=lKTsv6iVxV4)

#### 位运算技巧

- [ ] [Bithacks](https://graphics.stanford.edu/~seander/bithacks.html)
- [ ] [位操作技巧（Bit Hacks）（视频）](https://www.youtube.com/watch?v=ZusiKXcz_ac)

#### 计算置位（Set Bits）

- [ ] [计算一个字节中置位（Set Bits）的四种方式（视频）](https://youtu.be/Hzuzo9NJrlc)
- [ ] [计算比特位](https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan)
- [ ] [如何在一个 32 位的整型中计算置位（Set Bits）的数量](http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer)

#### 交换

- [ ] [交换（Swap）](http://bits.stephan-brumme.com/swap.html)
