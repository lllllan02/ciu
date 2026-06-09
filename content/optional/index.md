---
title: 选修扩展
description: 书籍、系统设计、高级主题与课程资源（可选）。
---

> 下面的内容都是可选的。通过学习这些内容，你将会得到更多的有关 CS 的概念，并将为所有的软件工程工作做更好的准备。


## 额外书籍

你可以从以下的书单挑选你有兴趣的主题来研读。

- [UNIX 环境高级编程](https://www.amazon.com/dp/013937681X)
    - 老，但却很棒
- [Linux 命令行大全](https://www.amazon.com/dp/1593273894/)
    - 现代选择
- [TCP-IP 详解系列](https://en.wikipedia.org/wiki/TCP/IP_Illustrated)
- [Head First 设计模式](https://www.amazon.com/gp/product/0596007124/)
    - 设计模式入门介绍
- [设计模式：可复用面向对象软件的基础](https://www.amazon.com/Design-Patterns-Elements-Reusable-Object-Oriented/dp/0201633612)
    - 也被称为“四人帮”（Gang of Four(GOF)）
    - 经典设计模式书籍
- [算法设计手冊（Skiena）](http://www.amazon.com/Algorithm-Design-Manual-Steven-Skiena/dp/1849967202)
    - 作为复习以及问题辨别
    - 这本书中算法的部分难度已经超过面试会出现的
    - 本书分为两个部分:
        - 数据结构和算法课本
            - 优点:
                - 跟其他算法课本一样是个很棒的复习素材
                - 包含作者以往解决工业及学术上问题的经验的故事
                - 含 C 语言代码示例
            - 缺点:
                - 某些地方跟《算法导论》（CLRS）一样艰深，但在某些主题，算法导论或许是更好的选择。
                - 第 7、8、9 章有点难以消化，因为某些地方并没有解释得很清楚，或者根本上我就是个学渣
                - 别会错意了，我很喜欢 Skiena 的教学方法以及他的风格。
        - 算法目录:
            - 这个部分是买这本书的最大原因
            - 我即将着手进行这部分，一旦完成这部分我会再更新上来
    - 可以在 kindle 上租
    - 解答：
        - [解答](https://web.archive.org/web/20150404194210/http://www.algorithm.cs.sunysb.edu/algowiki/index.php/The_Algorithms_Design_Manual_(Second_Edition))
    - [勘误表](http://www3.cs.stonybrook.edu/~skiena/algorist/book/errata)
- [算法](http://jeffe.cs.illinois.edu/teaching/algorithms/) (Jeff Erickson)
- [编程卓越之道（第一卷）：深入理解计算机](https://www.amazon.com/Write-Great-Code-Understanding-Machine/dp/1593270038)
    - 该书于 2004 年出版，虽然有些过时，但是对于简单了解计算机而言，这是一个了不起的资源
    - 作者发明了[高阶组合语言 HLA](https://en.wikipedia.org/wiki/High_Level_Assembly)，所以提到，并且举了一些 HLA 的例子。里面没有用到很多，但都是很棒的组合语言的例子。
    - 这些章节值得阅读，为你提供良好的基础：
        - 第 2 章──数字表示
        - 第 3 章──二进制算术和位运算
        - 第 4 章──浮点表示
        - 第 5 章──字符表示
        - 第 6 章──内存组织和访问
        - 第 7 章──组合数据类型和内存对象
        - 第 9 章──CPU 体系结构
        - 第 10 章──指令集架构
        - 第 11 章──内存体系结构和组织
- [算法导论](https://www.amazon.com/Introduction-Algorithms-fourth-Thomas-Cormen/dp/026204630X)
    - **重要提示**：读这本书的价值有限。本书很好地回顾了算法和数据结构，但不会教你如何编写良好的代码。你必须能够有效地编写一个不错的解决方案
    - 又称 CLR，有时是 CLRS，因为 Stein 最后才加入
- [计算机体系结构，第六版：定量方法](https://www.amazon.com/dp/0128119055)
    - 对于更丰富、更时新（2017 年）但较长的处理方式

## 系统设计、可扩展性和数据处理

**如果您有 4 年以上的工作经验，可以预期会遇到系统设计问题。**

- 可扩展性和系统设计是一个非常广泛的主题，涵盖了许多内容和资源，
因为在设计一个可以扩展的软件/硬件系统时需要考虑很多因素。
预计需要花费相当多的时间来学习这方面的知识。
- 考虑要点：
    - 可扩展性
        - 将大数据集归纳为单一值
        - 将一个数据集转换为另一个数据集
        - 处理海量数据
    - 系统设计
        - 功能集
        - 接口
        - 类层次结构
        - 在特定约束下设计系统
        - 简单性和鲁棒性
        - 权衡
        - 性能分析和优化
- [ ] **从这里开始**: [The System Design Primer](https://github.com/donnemartin/system-design-primer)
- [ ] [HiredInTech 的系统设计](http://www.hiredintech.com/system-design/)
- [ ] [如何准备回答技术面试中的设计问题？](https://www.quora.com/How-do-I-prepare-to-answer-design-questions-in-a-technical-interview?redirected_qid=1500023)
- [ ] [通过 8 个步骤掌握系统设计面试](https://javascript.plainenglish.io/8-steps-guide-to-ace-a-system-design-interview-7a5a797f4d7d)
- [ ] [数据库规范化 - 第一范式、第二范式、第三范式和第四范式（视频）](https://www.youtube.com/watch?v=UrYLYV7WSHM)
- [ ] [系统设计面试 ](https://github.com/checkcheckzz/system-design-interview) - 这个资源有很多内容。浏览文章和示例。我列出了一些示例在下面。
- [ ] [如何在系统设计面试中脱颖而出](https://web.archive.org/web/20120716060051/http://www.palantir.com/2011/10/how-to-rock-a-systems-design-interview/)
- [ ] [每个人都应该了解的数字](http://everythingisdata.wordpress.com/2009/10/17/numbers-everyone-should-know/)
- [ ] [进行上下文切换需要多长时间？](http://blog.tsunanet.net/2010/11/how-long-does-it-take-to-make-context.html)
- [ ] [跨数据中心的事务（视频）](https://www.youtube.com/watch?v=srOgpXECblk)
- [ ] [CAP 定理的简明英文介绍](http://ksat.me/a-plain-english-introduction-to-cap-theorem)
- [ ] [MIT 6.824：分布式系统，2020 年春季（20 个视频）](https://www.youtube.com/watch?v=cQP8WApzIQQ&list=PLrw6a1wE39_tb2fErI4-WkMbsvGQk9_UB)
- [ ] 共识算法：
    - [ ] Paxos - [Paxos 协议 - Computerphile（视频）](https://www.youtube.com/watch?v=s8JqcZtvnsM)
    - [ ] Raft - [Raft 分布式共识算法简介（视频）](https://www.youtube.com/watch?v=P9Ydif5_qvE)
        - [ ] [易于理解的论文](https://raft.github.io/)
        - [ ] [信息图](http://thesecretlivesofdata.com/raft/)
- [ ] [一致性哈希](http://www.tom-e-white.com/2007/11/consistent-hashing.html)
- [ ] [NoSQL 模式](http://horicky.blogspot.com/2009/11/nosql-patterns.html)
- [ ] 可扩展性：
    - 您不需要掌握所有这些内容，只需选择一些您感兴趣的。
    - [ ] [优秀的概述（视频）](https://www.youtube.com/watch?v=-W9F__D3oY4)
    - [ ] 短系列：
        - [克隆](http://www.lecloud.net/post/7295452622/scalability-for-dummies-part-1-clones)
        - [数据库](http://www.lecloud.net/post/7994751381/scalability-for-dummies-part-2-database)
        - [缓存](http://www.lecloud.net/post/9246290032/scalability-for-dummies-part-3-cache)
        - [异步性](http://www.lecloud.net/post/9699762917/scalability-for-dummies-part-4-asynchronism)
    - [ ] [可扩展的 Web 架构和分布式系统](http://www.aosabook.org/en/distsys.html)
    - [ ] [分布式计算的谬误解释](https://pages.cs.wisc.edu/~zuyu/files/fallacies.pdf)
    - [ ] [Jeff Dean - 在 Google 构建软件系统以及吸取的教训（视频）](https://www.youtube.com/watch?v=modXC5IWTJI)
    - [ ] [架构师为规模而设计的介绍](http://lethain.com/introduction-to-architecting-systems-for-scale/)
    - [ ] [缩放移动游戏以面向全球受众使用 App Engine 和 Cloud Datastore（视频）](https://www.youtube.com/watch?v=9nWyWwY2Onc)
    - [ ] [谷歌是如何进行面向全球基础设施的大规模工程的（视频）](https://www.youtube.com/watch?v=H4vMcD7zKM0)
    - [ ] [算法的重要性](https://www.topcoder.com/thrive/articles/The%20Importance%20of%20Algorithms)
    - [ ] [分片](http://highscalability.com/blog/2009/8/6/an-unorthodox-approach-to-database-design-the-coming-of-the.html)
    - [ ] [针对长期目标的工程 - Astrid Atkinson 主题演讲（视频）](https://www.youtube.com/watch?v=p0jGmgIrf_M&list=PLRXxvay_m8gqVlExPC5DG3TGWJTaBgqSA&index=4)
    - [ ] [在 30 分钟内了解 YouTube 7 年的可扩展性经验](http://highscalability.com/blog/2012/3/26/7-years-of-youtube-scalability-lessons-in-30-minutes.html)
        - [视频](https://www.youtube.com/watch?v=G-lGCC4KKok)
    - [ ] [PayPal 如何使用仅 8 台 VM 每天处理数十亿次交易](http://highscalability.com/blog/2016/8/15/how-paypal-scaled-to-billions-of-transactions-daily-using-ju.html)
    - [ ] [如何在大型数据集中去重](https://blog.clevertap.com/how-to-remove-duplicates-in-large-datasets/)
    - [ ] [通过 Jon Cowie 深入了解 Etsy 的规模和工程文化（视频）](https://www.youtube.com/watch?v=3vV4YiqKm1o)
    - [ ] [Amazon 是如何转向自己的微服务架构的](http://thenewstack.io/led-amazon-microservices-architecture/)
    - [ ] [压缩还是不压缩，这是 Uber 面临的问题](https://eng.uber.com/trip-data-squeeze/)
    - [ ] [何时应使用近似查询处理？](http://highscalability.com/blog/2016/2/25/when-should-approximate-query-processing-be-used.html)
    - [ ] [谷歌从单一数据中心到故障转移再到本地多家数据中心架构的转变]( http://highscalability.com/blog/2016/2/23/googles-transition-from-single-datacenter-to-failover-to-a-n.html)
    - [ ] [为每天处理数百万请求的图像优化技术](http://highscalability.com/blog/2016/6/15/the-image-optimization-technology-that-serves-millions-of-re.html)
    - [ ] [Patreon 架构简介](http://highscalability.com/blog/2016/2/1/a-patreon-architecture-short.html)
    - [ ] [如何在 Instagram 庞大的推荐引擎中决定您将看到谁？](http://highscalability.com/blog/2016/1/27/tinder-how-does-one-of-the-largest-recommendation-engines-de.html)
    - [ ] [现代缓存设计](http://highscalability.com/blog/2016/1/25/design-of-a-modern-cache.html)
    - [ ] [在 Facebook 规模下进行直播视频流](http://highscalability.com/blog/2016/1/13/live-video-streaming-at-facebook-scale.html)
    - [ ] [在亚马逊 AWS 上如何扩展到 1100 万以上的用户](http://highscalability.com/blog/2016/1/11/a-beginners-guide-to-scaling-to-11-million-users-on-amazons.html)
    - [ ] [全面了解 Netflix 整个堆栈](http://highscalability.com/blog/2015/11/9/a-360-degree-view-of-the-entire-netflix-stack.html)
    - [ ] [延迟无处不在，而且它会让您丧失销售机会 - 如何应对](http://highscalability.com/latency-everywhere-and-it-costs-you-sales-how-crush-it)
    - [ ] [Instagram 的动力：数百个实例，几十种技术](http://instagram-engineering.tumblr.com/post/13649370142/what-powers-instagram-hundreds-of-instances)
    - [ ] [Salesforce 架构 - 如何处理每天 13 亿次交易](http://highscalability.com/blog/2013/9/23/salesforce-architecture-how-they-handle-13-billion-transacti.html)
    - [ ] [ESPN 规模上的架构 - 每秒操作 10 万次“嘟嘟噜嘟嘟噜”](http://highscalability.com/blog/2013/11/4/espns-architecture-at-scale-operating-at-100000-duh-nuh-nuhs.html)
    - [ ]  在下面的“消息、序列化和队列系统”部分查看一些将服务连接在一起的技术信息
    - [ ] Twitter:
        - [O'Reilly MySQL CE 2011: Jeremy Cole, "Big and Small Data at @Twitter" (视频)](https://www.youtube.com/watch?v=5cKTP36HVgI)
        - [时间轴扩展](https://www.infoq.com/presentations/Twitter-Timeline-Scalability)
    - 欲知更多信息，请参阅[Video Series](/optional#视频系列) 部分中的“Mining Massive Datasets”视频系列
- [ ] 练习系统设计过程：以下是一些建议您在纸上尝试的想法，每个想法都有一些关于如何在现实世界中处理的文档：
    - 复习: [The System Design Primer](https://github.com/donnemartin/system-design-primer)
    - [HiredInTech 的系统设计](http://www.hiredintech.com/system-design/)
    - [速查表](https://github.com/jwasham/coding-interview-university/blob/main/extras/cheat%20sheets/system-design.pdf)
    - 流程：
        1. 理解问题和范围：
            - 定义用例，与面试官的帮助
            - 提出额外的功能
            - 移除面试官认为超出范围的项目
            - 假设需要高可用性，并将其添加为用例
        2. 考虑限制：
            - 询问每月有多少个请求
            - 询问每秒有多少个请求（他们可能会主动提供或让您计算）
            - 估计读取与写入的百分比
            - 保持估计时考虑 80/20 法则
            - 每秒写入多少数据
            - 在 5 年内所需的总存储量
            - 每秒读取多少数据
        3. 抽象设计：
            - 层（服务、数据、缓存）
            - 基础架构：负载均衡、消息传递
            - 驱动服务的任何关键算法的粗略概述
            - 考虑瓶颈并确定解决方案
    - 练习：
        - [设计一个随机唯一 ID 生成系统](https://blog.twitter.com/2010/announcing-snowflake)
        - [设计一个键值数据库](http://www.slideshare.net/dvirsky/introduction-to-redis)
        - [设计一个图片分享系统](http://highscalability.com/blog/2011/12/6/instagram-architecture-14-million-users-terabytes-of-photos.html)
        - [设计一个推荐系统](http://ijcai13.org/files/tutorial_slides/td3.pdf)
        - [设计一个 URL 缩短系统：来自上面的复制](http://www.hiredintech.com/system-design/the-system-design-process/)
        - [设计一个缓存系统](https://web.archive.org/web/20220217064329/https://adayinthelifeof.nl/2011/02/06/memcache-internals/)

## 附加学习

我把它们加进来是为了让你成为更全方位的软件工程师，并且留意一些技术以及算法，让你拥有更大的工具箱。

## 编译器
- [编译器的工作方式，约 1 分钟（视频）](https://www.youtube.com/watch?v=IhC7sdYe-Jg)
- [Harvard CS50-编译器（视频）](https://www.youtube.com/watch?v=CSZLNYF4Klo)
- [C ++（视频）](https://www.youtube.com/watch?v=twodd1KFfGk)
- [了解编译器优化（C ++）（视频）](https://www.youtube.com/watch?v=FnGCDLhaxKU)

## Emacs and vi(m)
- 熟悉基于 unix 的代码编辑器
- vi(m):
    - [使用 vim 进行编辑 01 - 安装, 设置和模式 (视频)](https://www.youtube.com/watch?v=5givLEMcINQ&index=1&list=PL13bz4SHGmRxlZVmWQ9DvXo1fEg4UdGkr)
    - [VIM 的冒险之旅](http://vim-adventures.com/)
    - 4 个视频集:
        - [vi/vim 编辑器 - 课程 1](https://www.youtube.com/watch?v=SI8TeVMX8pk)
        - [vi/vim 编辑器 - 课程 2](https://www.youtube.com/watch?v=F3OO7ZIOaJE)
        - [vi/vim 编辑器 - 课程 4](https://www.youtube.com/watch?v=1lYD5gwgZIA)
        - [vi/vim 编辑器 - 课程 3](https://www.youtube.com/watch?v=ZYEccA_nMaI)
    - [使用 Vi 而不是 Emacs](http://www.cs.yale.edu/homes/aspnes/classes/223/notes.html#Using_Vi_instead_of_Emacs)
- emacs:
    - [基础 Emacs 教程 (视频)](https://www.youtube.com/watch?v=hbmV1bnQ-i0)
    - 3 个视频集:
        - [Emacs 教程 (初学者) -第 1 部分- 文件命令, 剪切/复制/粘贴,  自定义命令](https://www.youtube.com/watch?v=ujODL7MD04Q)
        - [Emacs 教程 (初学者 -第 2 部分- Buffer 管理, 搜索, M-x grep 和 rgrep 模式](https://www.youtube.com/watch?v=XWpsRupJ4II)
        - [Emacs 教程 (初学者 -第 3 部分- 表达式, 声明, ~/.emacs 文件和包机制](https://www.youtube.com/watch?v=paSgzPso-yc)
    - [Evil 模式: 或许, 我是怎样对 Emacs 路人转粉的 (视频)](https://www.youtube.com/watch?v=JWD1Fpdd4Pc)
    - [使用 Emacs 开发 C 程序](http://www.cs.yale.edu/homes/aspnes/classes/223/notes.html#Writing_C_programs_with_Emacs)
	- [Emacs 绝对初学者指南（David Wilson 的视频）](https://www.youtube.com/watch?v=48JlgiBpw_I&t=0s)
	- [Emacs 绝对初学者指南（David Wilson 批注）](https://systemcrafters.net/emacs-essentials/absolute-beginners-guide-to-emacs/)

## Unix 命令行工具
- 下列内容包含优秀工具
- bash
- cat
- grep
- sed
- awk
- curl or wget
- sort
- tr
- uniq
- [strace](https://en.wikipedia.org/wiki/Strace)
- [tcpdump](https://danielmiessler.com/study/tcpdump/)

## 信息论 (视频)
- [Khan Academy 可汗学院](https://www.khanacademy.org/computing/computer-science/informationtheory)
- 更多有关马尔可夫的内容:
    - [马尔可夫内容生成（Core Markov Text Generation）](https://www.coursera.org/learn/data-structures-optimizing-performance/lecture/waxgx/core-markov-text-generation)
    - [Core Implementing Markov Text Generation 马尔可夫内容生成实现](https://www.coursera.org/learn/data-structures-optimizing-performance/lecture/gZhiC/core-implementing-markov-text-generation)
    - [一个马尔可夫内容生成器的项目（Project = Markov Text Generation Walk Through）](https://www.coursera.org/learn/data-structures-optimizing-performance/lecture/EUjrq/project-markov-text-generation-walk-through)
- 关于更多信息，请参照下方 MIT 6.050J 信息和系统复杂度的内容。

## 奇偶校验位 & 汉明码 (视频)
- [入门](https://www.youtube.com/watch?v=q-3BctoUpHE)
- [奇偶校验位](https://www.youtube.com/watch?v=DdMcAUlxh1M)
- 汉明码(Hamming Code):
    - [发现错误](https://www.youtube.com/watch?v=1A_NcXxdoCc)
    - [修正错误](https://www.youtube.com/watch?v=JAMLuxdHH8o)
- [检查错误](https://www.youtube.com/watch?v=wbH2VxzmoZk)

## 系统熵值（Entropy）
- 请参考下方视频
- 观看之前，请先确定观看了信息论的视频
- [信息理论, 克劳德·香农, 熵值, 系统冗余, 数据比特压缩 (视频)](https://youtu.be/JnJq3Py0dyM?t=176)

## 密码学
- 请参考下方视频
- 观看之前，请先确定观看了信息论的视频
- [可汗学院](https://www.khanacademy.org/computing/computer-science/密码学)
- [密码学: 哈希函数](https://www.youtube.com/watch?v=KqqOXndnvic&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp&index=30)
- [密码学: 加密](https://www.youtube.com/watch?v=9TNI2wHmaeI&index=31&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)

## 压缩
- 观看之前，请先确定观看了信息论的视频
- Computerphile (视频)：
    - [压缩](https://www.youtube.com/watch?v=Lto-ajuqW3w)
    - [压缩熵值](https://www.youtube.com/watch?v=M5c_RFKVkko)
    - [由上而下的树 (霍夫曼编码树)](https://www.youtube.com/watch?v=umTbivyJoiI)
    - [额外比特 - 霍夫曼编码树](https://www.youtube.com/watch?v=DV8efuB3h2g)
    - [优雅的压缩数据 (无损数据压缩方法)](https://www.youtube.com/watch?v=goOa3DGezUA)
    - [Text Compression Meets Probabilities](https://www.youtube.com/watch?v=cCDCfoHTsaU)
- [数据压缩的艺术](https://www.youtube.com/playlist?list=PLOU2XLYxmsIJGErt5rrCqaSGTMyyqNt2H)
- [(可选) 谷歌开发者：GZIP 还差远了呢!](https://www.youtube.com/watch?v=whGwm0Lky2s)

## 计算机安全
- [MIT（23 个视频）](https://www.youtube.com/playlist?list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)
    - [威胁模型：入门](https://www.youtube.com/watch?v=GqmQg-cszw4&index=1&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)
    - [控制劫持攻击](https://www.youtube.com/watch?v=6bwzNg5qQ0o&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh&index=2)
    - [缓冲区溢出漏洞攻击和防御](https://www.youtube.com/watch?v=drQyrzRoRiA&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh&index=3)
    - [优先权区分](https://www.youtube.com/watch?v=6SIJmoE9L9g&index=4&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)
    - [能力](https://www.youtube.com/watch?v=8VqTSY-11F4&index=5&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)
    - [在沙盒中运行原生代码](https://www.youtube.com/watch?v=VEV74hwASeU&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh&index=6)
    - [网络安全模型](https://www.youtube.com/watch?v=chkFBigodIw&index=7&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)
    - [网络安全应用](https://www.youtube.com/watch?v=EBQIGy1ROLY&index=8&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)
    - [标志化执行](https://www.youtube.com/watch?v=yRVZPvHYHzw&index=9&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)
    - [网络安全](https://www.youtube.com/watch?v=SIEVvk3NVuk&index=11&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)
    - [网络协议](https://www.youtube.com/watch?v=QOtA76ga_fY&index=12&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)
    - [旁路攻击](https://www.youtube.com/watch?v=PuVMkSEcPiI&index=15&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)

## 垃圾回收
- [ ] [Python 中的垃圾回收 (视频)](https://www.youtube.com/watch?v=iHVs_HkjdmI)
- [ ] [深度解析：论垃圾回收在 JAVA 中的重要性](https://www.infoq.com/presentations/garbage-collection-benefits)
- [ ] [深度解析：论垃圾回收在 Python 中的重要性(视频)](https://www.youtube.com/watch?v=P-8Z0-MhdQs&list=PLdzf4Clw0VbOEWOS_sLhT_9zaiQDrS5AR&index=3)

## 并行编程
- [ ] [Coursera (Scala)](https://www.coursera.org/learn/parprog1/home/week/1)
- [ ] [用于高性能并行计算的高效 Python（视频）](https://www.youtube.com/watch?v=uY85GkaYzBk)

## 消息传递，序列化和队列系统
- [Thrift](https://thrift.apache.org/)
    - [教程](http://thrift-tutorial.readthedocs.io/en/latest/intro.html)
- [协议缓冲](https://developers.google.com/protocol-buffers/)
    - [教程](https://developers.google.com/protocol-buffers/docs/tutorials)
- [gRPC](http://www.grpc.io/)
    - [gRPC 对于 JAVA 开发者的入门教程（视频）](https://www.youtube.com/watch?v=5tmPvSe7xXQ&list=PLcTqM9n_dieN0k1nSeN36Z_ppKnvMJoly&index=1)
- [Redis](http://redis.io/)
    - [教程](http://try.redis.io/)
- [Amazon 的 SQS 系统 (队列)](https://aws.amazon.com/sqs/)
- [Amazon 的 SNS 系统 (pub-sub)](https://aws.amazon.com/sns/)
- [RabbitMQ](https://www.rabbitmq.com/)
    - [入门教程](https://www.rabbitmq.com/getstarted.html)
- [Celery](http://www.celeryproject.org/)
    - [Celery 入门](http://docs.celeryproject.org/en/latest/getting-started/first-steps-with-celery.html)
- [ZeroMQ](http://zeromq.org/)
    - [入门教程](http://zeromq.org/intro:read-the-manual)
- [ActiveMQ](http://activemq.apache.org/)
- [Kafka](http://kafka.apache.org/documentation.html#introduction)
- [MessagePack](http://msgpack.org/index.html)
- [Avro](https://avro.apache.org/)

## A*搜索算法
- [A 搜索算法](https://en.wikipedia.org/wiki/A*_search_algorithm)
- [A* 路径搜索（E01：算法解释）（视频）](https://www.youtube.com/watch?v=-L-WgKMFuhE)

## 快速傅里叶变换
- [傅立叶变换的交互式指南](https://betterexplained.com/articles/an-interactive-guide-to-the-fourier-transform/)
- [什么是傅立叶变换？论傅立叶变换的用途](http://www.askamathematician.com/2012/09/q-what-is-a-fourier-transform-what-is-it-used-for/)
- [什么是傅立叶变换？ (视频)](https://www.youtube.com/watch?v=Xxut2PN-V8Q)
- [分而治之：FFT（视频）](https://www.youtube.com/watch?v=iTMn0Kt18tg&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp&index=4)
- [FTT 是什么](http://jakevdp.github.io/blog/2013/08/28/understanding-the-fft/)

## 布隆过滤器
- 给定布隆过滤器 m 比特位和 k 个哈希函数，插入和成员检测都会是 O(k)。
- [布隆过滤器（视频）](https://www.youtube.com/watch?v=-SuTGoFYjZs)
- [布隆过滤器 | 数据挖掘 | Stanford University（视频）](https://www.youtube.com/watch?v=qBTdukbzc78)
- [教程](http://billmill.org/bloomfilter-tutorial/)
- [如何写一个布隆过滤器应用](http://blog.michaelschmatz.com/2016/04/11/how-to-write-a-bloom-filter-cpp/)

## HyperLogLog
- [如何仅使用 1.5KB 内存计算十亿个不同的对象](http://highscalability.com/blog/2012/4/5/big-data-counting-how-to-count-a-billion-distinct-objects-us.html)

## 局部敏感哈希
- 用于确定文件的相似性
- MD5 或 SHA 的反义词，用于确定 2 个文档/字符串是否完全相同
- [Simhashing（希望如此）变得简单](http://ferd.ca/simhashing-hopefully-made-simple.html)

## van Emde Boas 树
- [分而治之：van Emde Boas 树 (视频)](https://www.youtube.com/watch?v=hmReJCupbNU&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp&index=6)
- [MIT 课堂笔记](https://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-046j-design-and-analysis-of-algorithms-spring-2012/lecture-notes/MIT6_046JS12_lec15.pdf)

## 增强数据结构
- [CS 61B 第 39 课: 增强数据结构](https://youtu.be/zksIj9O8_jc?list=PL4BBB74C7D2A1049C&t=950)

## 平衡查找树（Balanced search trees）
- 掌握至少一种平衡查找树（并懂得如何实现）：
- “在各种平衡查找树当中，AVL 树和 2-3 树已经成为了过去，而红黑树（red-black trees）看似变得越来越受人青睐。
这种令人特别感兴趣的数据结构，亦称伸展树（splay tree）。
它可以自我管理，且会使用轮换来移除任何访问过根节点的键。” —— Skiena
- 因此，在各种各样的平衡查找树当中，我选择了伸展树来实现。
  虽然，通过我的阅读，我发现在面试中并不会被要求实现一棵平衡查找树。
  但是，为了胜人一筹，我们还是应该看看如何去实现。在阅读了大量关于红黑树的代码后，
  我才发现伸展树的实现确实会使得各方面更为高效。
    - 伸展树：插入、查找、删除函数的实现，而如果你最终实现了红黑树，那么请尝试一下：
    - 跳过删除函数，直接实现搜索和插入功能
- 我希望能阅读到更多关于 B 树的资料，因为它也被广泛地应用到大型的数据集当中。
- [自平衡二叉查找树](https://en.wikipedia.org/wiki/Self-balancing_binary_search_tree)

- **AVL 树**
    - 实际中：
    我能告诉你的是，该种树并无太多的用途，但我能看到有用的地方在哪里：
    AVL 树是另一种平衡查找树结构。其可支持时间复杂度为 O(log n) 的查询、插入及删除。
    它比红黑树严格意义上更为平衡，从而导致插入和删除更慢，但遍历却更快。正因如此，才彰显其结构的魅力。
    只需要构建一次，就可以在不重新构造的情况下读取，
    适合于实现诸如语言字典（或程序字典，如一个汇编程序或解释程序的操作码）。
    - [MIT AVL 树 / AVL 树的排序（视频）](https://www.youtube.com/watch?v=FNeL18KsWPc&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=6)
    - [AVL 树（视频）](https://www.coursera.org/learn/data-structures/lecture/Qq5E0/avl-trees)
    - [AVL 树的实现（视频）](https://www.coursera.org/learn/data-structures/lecture/PKEBC/avl-tree-implementation)
    - [分离与合并](https://www.coursera.org/learn/data-structures/lecture/22BgE/split-and-merge)
    - [[Review] AVL Trees (playlist) in 19 minutes (video)](https://www.youtube.com/playlist?list=PL9xmBV_5YoZOUFgdIeOPuH6cfSnNRMau-)

- **伸展树**
    - 实际中：
    伸展树一般用于缓存、内存分配者、路由器、垃圾回收者、数据压缩、ropes
    （字符串的一种替代品，用于存储长串的文本字符）、
    Windows NT（虚拟内存、网络及文件系统）等的实现。
    - [CS 61B：伸展树（Splay trees）（视频）](https://www.youtube.com/watch?v=Najzh1rYQTo&index=23&list=PL-XXv-cvA_iAlnI-BQr9hjqADPBtujFJd)
    - MIT 教程：伸展树（Splay trees）：
        - 该教程会过于学术，但请观看到最后的 10 分钟以确保掌握。
        - [视频](https://www.youtube.com/watch?v=QnPl_Y6EqMo)

- **红黑树**
    - 这些是 2-3 棵树的翻译（请参见下文）。
    - 实际中：红黑树提供了在最坏情况下插入操作、删除操作和查找操作的时间保证。
    这些时间值的保障不仅对时间敏感型应用有用，例如实时应用，
    还对在其他数据结构中块的构建非常有用，
    而这些数据结构都提供了最坏情况下的保障；
    例如，许多用于计算几何学的数据结构都可以基于红黑树，
    而目前 Linux 内核所采用的完全公平调度器（the Completely Fair Scheduler）也使用到了该种树。
    在 Java 8 中，Collection HashMap 也从原本用 Linked List 实现，
    储存特定元素的哈希码，改为用红黑树实现。
    - [Aduni —— 算法 —— 课程 4（该链接直接跳到开始部分）（视频）](https://youtu.be/1W3x0f_RmUo?list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&t=3871)
    - [Aduni —— 算法 —— 课程 5（视频）](https://www.youtube.com/watch?v=hm2GHwyKF1o&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=5)
    - [黑树（Black Tree）](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree)
    - [二分查找及红黑树的介绍](https://www.topcoder.com/community/data-science/data-science-tutorials/an-introduction-to-binary-search-and-red-black-trees/)
    - [[Review] Red-Black Trees (playlist) in 30 minutes (video)](https://www.youtube.com/playlist?list=PL9xmBV_5YoZNqDI8qfOZgzbqahCUmUEin)

- **2-3 查找树**
    - 实际中：
    2-3 树的元素插入非常快速，但却有着查询慢的代价（因为相比较 AVL 树来说，其高度更高）。
    - 你会很少用到 2-3 树。这是因为，其实现过程中涉及到不同类型的节点。因此，人们更多地会选择红黑树。
    - [2-3 树的直感与定义（视频）](https://www.youtube.com/watch?v=C3SsdUqasD4&list=PLA5Lqm4uh9Bbq-E0ZnqTIa8LRaL77ica6&index=2)
    - [2-3 树的二元观点](https://www.youtube.com/watch?v=iYvBtGKsqSg&index=3&list=PLA5Lqm4uh9Bbq-E0ZnqTIa8LRaL77ica6)
    - [2-3 树（学生叙述）（视频）](https://www.youtube.com/watch?v=TOb1tuEZ2X4&index=5&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)

- **2-3-4 树 (亦称 2-4 树)**
    - 实际中：
    对于每一棵 2-4 树，都有着对应的红黑树来存储同样顺序的数据元素。
    在 2-4 树上进行插入及删除操作等同于在红黑树上进行颜色翻转及轮换。
    这使得 2-4 树成为一种用于掌握红黑树背后逻辑的重要工具。
    这就是为什么许多算法引导文章都会在介绍红黑树之前，先介绍 2-4 树，尽管**2-4 树在实际中并不经常使用**。
    - [CS 61B Lecture 26：平衡查找树（视频）](https://www.youtube.com/watch?v=zqrqYXkth6Q&index=26&list=PL4BBB74C7D2A1049C)
    - [自底向上的 2-4 树（视频）](https://www.youtube.com/watch?v=DQdMYevEyE4&index=4&list=PLA5Lqm4uh9Bbq-E0ZnqTIa8LRaL77ica6)
    - [自顶向下的 2-4 树（视频）](https://www.youtube.com/watch?v=2679VQ26Fp4&list=PLA5Lqm4uh9Bbq-E0ZnqTIa8LRaL77ica6&index=5)

- **N 叉树（K 叉树、M 叉树）**
    - 注意：N 或 K 指的是分支系数（即树的最大分支数）：
    - 二叉树是一种分支系数为 2 的树
    - 2-3 树是一种分支系数为 3 的树
    - [K 叉树](https://en.wikipedia.org/wiki/K-ary_tree)

- **B 树**
    - 有趣的是：为啥叫 B 仍然是一个神秘。因为 B 可代表波音（Boeing）、平衡（Balanced）或 Bayer（联合创造者）
    - 实际中：
    B 树会被广泛适用于数据库中，而现代大多数的文件系统都会使用到这种树（或变种）。
    除了运用在数据库中，B 树也会被用于文件系统以快速访问一个文件的任意块。
    但存在着一个基本的问题，
    那就是如何将文件块 i 转换成一个硬盘块（或一个柱面-磁头-扇区）上的地址。
    - [B 树](https://en.wikipedia.org/wiki/B-tree)
    - [B 树数据结构](http://btechsmartclass.com/data_structures/b-trees.html)
    - [B 树的介绍（视频）](https://www.youtube.com/watch?v=I22wEC1tTGo&list=PLA5Lqm4uh9Bbq-E0ZnqTIa8LRaL77ica6&index=6)
    - [B 树的定义及其插入操作（视频）](https://www.youtube.com/watch?v=s3bCdZGrgpA&index=7&list=PLA5Lqm4uh9Bbq-E0ZnqTIa8LRaL77ica6)
    - [B 树的删除操作（视频）](https://www.youtube.com/watch?v=svfnVhJOfMc&index=8&list=PLA5Lqm4uh9Bbq-E0ZnqTIa8LRaL77ica6)
    - [MIT 6.851 —— 内存层次模块（Memory Hierarchy Models）（视频）](https://www.youtube.com/watch?v=V3omVLzI0WE&index=7&list=PLUl4u3cNGP61hsJNdULdudlRL493b-XZf)
        - 覆盖有高速缓存参数无关型（cache-oblivious）B 树和非常有趣的数据结构
        - 头 37 分钟讲述的很专业，或许可以跳过（B 指块的大小、即缓存行的大小）
    - [[Review] B-Trees (playlist) in 26 minutes (video)](https://www.youtube.com/playlist?list=PL9xmBV_5YoZNFPPv98DjTdD9X6UI9KMHz)


## k-D 树
- 非常适合在矩形或更高维度的对象中查找点数
- 最适合 k 近邻
- [kNN K-d 树算法（视频）](https://www.youtube.com/watch?v=Y4ZgLlDfKDg)

## 跳表
- "有一种非常迷幻的数据类型" - Skiena
- [随机化: 跳表 (视频)](https://www.youtube.com/watch?v=2g9OSRKJuzM&index=10&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)
- [更生动详细的解释](https://en.wikipedia.org/wiki/Skip_list)

## 网络流
- [5 分钟简析 Ford-Fulkerson──一步步示例 (视频)](https://www.youtube.com/watch?v=v1VgJmkEJW0)
- [Ford-Fulkerson 算法 (视频)](https://www.youtube.com/watch?v=v1VgJmkEJW0)
- [网络流 (视频)](https://www.youtube.com/watch?v=2vhN4Ice5jI)

## 不相交集 & 联合查找
- [UCB 61B - 不相交集；排序 & 选择(视频)](https://www.youtube.com/watch?v=MAEGXTwmUsI&list=PL-XXv-cvA_iAlnI-BQr9hjqADPBtujFJd&index=21)
- [Sedgewick 算法──Union-Find（6 视频）](https://www.coursera.org/learn/algorithms-part1/home/week/1)

## 快速处理的数学
- [整数运算, Karatsuba 乘法 (视频)](https://www.youtube.com/watch?v=eCaXlAaN2uE&index=11&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [中国剩余定理 (在密码学中的使用) (视频)](https://www.youtube.com/watch?v=ru7mWZJlRQg)

## 树堆 (Treap)
- 一个二叉搜索树和一个堆的组合
- [树堆](https://en.wikipedia.org/wiki/Treap)
- [数据结构：树堆的讲解（视频）](https://www.youtube.com/watch?v=6podLUYinH8)
- [集合操作的应用(Applications in set operations)](https://www.cs.cmu.edu/~scandal/papers/treaps-spaa98.pdf)

## 线性规划（Linear Programming）（视频）
- [线性规划](https://www.youtube.com/watch?v=M4K6HYLHREQ)
- [寻找最小成本](https://www.youtube.com/watch?v=2ACJ9ewUC6U)
- [寻找最大值](https://www.youtube.com/watch?v=8AA_81xI3ik)
- [用 Python 解决线性方程式──单纯形算法](https://www.youtube.com/watch?v=44pAWI7v5Zk)

## 几何：凸包（Geometry, Convex hull）（视频）
- [Graph Alg. IV: 几何算法介绍 - 第 9 课](https://youtu.be/XIAQRlNkJAw?list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&t=3164)
- [Graham & Jarvis: 几何算法 - 第 10 课](https://www.youtube.com/watch?v=J5aJEcOr6Eo&index=10&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm)
- [分而治之: 凸包, 中值查找](https://www.youtube.com/watch?v=EzeYI7p9MjU&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp&index=2)

## 离散数学
- [计算机科学 70，001 - 2015 年春季 - 离散数学与概率论](http://www.infocobuild.com/education/audio-video-courses/computer-science/cs70-spring2015-berkeley.html)
- [离散数学由 Shai Simonson（19 个视频）](https://www.youtube.com/playlist?list=PLWX710qNZo_sNlSWRMVIh6kfTjolNaZ8t)
- [离散数学由印度理工学院罗帕尔分校 NPTEL 提供](https://nptel.ac.in/courses/106/106/106106183/)

## 一些主题的额外内容

我添加了这些内容来加强上面已经提出的一些观点，但是不想把它们放在上面，因为那样会太多。
对于一个主题来说，过度处理很容易。
你希望在本世纪被雇佣吗？

- **SOLID**
- [ ] [Bob Martin SOLID Principles of Object Oriented and Agile Design (视频)](https://www.youtube.com/watch?v=TMuno5RZNeE)
- [ ] S - [单一职责原则 | 每个对象负责一个单一职责](http://www.oodesign.com/single-responsibility-principle.html) | [Single responsibility to each Object](http://www.javacodegeeks.com/2011/11/solid-single-responsibility-principle.html)
    - [更多解释](https://docs.google.com/open?id=0ByOwmqah_nuGNHEtcU5OekdDMkk)
- [ ] O - [开闭原则](http://www.oodesign.com/open-close-principle.html)  | [在生产级别上，对象应准备好进行扩展，但不进行修改](https://en.wikipedia.org/wiki/Open/closed_principle)
    - [更多解释](http://docs.google.com/a/cleancoder.com/viewer?a=v&pid=explorer&chrome=true&srcid=0BwhCYaYDn8EgN2M5MTkwM2EtNWFkZC00ZTI3LWFjZTUtNTFhZGZiYmUzODc1&hl=en)
- [ ] L - [里氏替换原则](http://www.oodesign.com/liskov-s-substitution-principle.html) | [基类和派生类遵循‘是一个’原则](http://stackoverflow.com/questions/56860/what-is-the-liskov-substitution-principle)
    - [更多解释](http://docs.google.com/a/cleancoder.com/viewer?a=v&pid=explorer&chrome=true&srcid=0BwhCYaYDn8EgNzAzZjA5ZmItNjU3NS00MzQ5LTkwYjMtMDJhNDU5ZTM0MTlh&hl=en)
- [ ] I - [接口隔离原则](http://www.oodesign.com/interface-segregation-principle.html) | 客户端不应被强制实现不使用的接口
    - [5 分钟内的接口隔离原则（视频）](https://www.youtube.com/watch?v=3CtAfl7aXAQ)
    - [更多解释](http://docs.google.com/a/cleancoder.com/viewer?a=v&pid=explorer&chrome=true&srcid=0BwhCYaYDn8EgOTViYjJhYzMtMzYxMC00MzFjLWJjMzYtOGJiMDc5N2JkYmJi&hl=en)
- [ ] D -[依赖反转原则](http://www.oodesign.com/dependency-inversion-principle.html) | 在对象的组合中减少依赖
    - [为何依赖反转原则如此重要](http://stackoverflow.com/questions/62539/what-is-the-dependency-inversion-principle-and-why-is-it-important)
    - [更多解释](http://docs.google.com/a/cleancoder.com/viewer?a=v&pid=explorer&chrome=true&srcid=0BwhCYaYDn8EgMjdlMWIzNGUtZTQ0NC00ZjQ5LTkwYzQtZjRhMDRlNTQ3ZGMz&hl=en)


- **Union-Find**
- [概览](https://www.coursera.org/learn/data-structures/lecture/JssSY/overview)
- [初级实践](https://www.coursera.org/learn/data-structures/lecture/EM5D0/naive-implementations)
- [树状结构](https://www.coursera.org/learn/data-structures/lecture/Mxu0w/trees)
- [合并树状结构](https://www.coursera.org/learn/data-structures/lecture/qb4c2/union-by-rank)
- [路径压缩](https://www.coursera.org/learn/data-structures/lecture/Q9CVI/path-compression)
- [分析选项](https://www.coursera.org/learn/data-structures/lecture/GQQLN/analysis-optional)

- **动态规划的更多内容** (视频)
- [6.006: 动态规划 I: 斐波那契数列, 最短路径](https://www.youtube.com/watch?v=OQ5jsbhAv_M&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=19)
- [6.006: 动态规划 II: 文本匹配, 二十一点/黑杰克](https://www.youtube.com/watch?v=ENyox7kNKeY&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=20)
- [6.006: 动态规划 III: 最优加括号方式, 最小编辑距离, 背包问题](https://www.youtube.com/watch?v=ocZMDMZwhCY&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=21)
- [6.006: 动态规划 IV: 吉他指法，拓扑，超级马里奥.](https://www.youtube.com/watch?v=tp4_UXaVyx8&index=22&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [6.046: 动态规划: 动态规划进阶](https://www.youtube.com/watch?v=Tw1k46ywN6E&index=14&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)
- [6.046: 动态规划: 所有点对最短路径](https://www.youtube.com/watch?v=NzgFUwOaoIw&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp&index=15)
- [6.046: 动态规划: 更多示例](https://www.youtube.com/watch?v=krZI60lKPek&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp&index=12)

- **图形处理进阶** (视频)
- [异步分布式算法: 对称性破缺，最小生成树](https://www.youtube.com/watch?v=mUBmcbbJNf4&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp&index=27)
- [异步分布式算法: 最小生成树](https://www.youtube.com/watch?v=kQ-UQAzcnzA&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp&index=28)

- MIT **概率论** (过于数学，进度缓慢，但这对于数学的东西却是必要之恶) (视频):
- [MIT 6.042J - 概率论概述](https://www.youtube.com/watch?v=SmFwFdESMHI&index=18&list=PLB7540DEDD482705B)
- [MIT 6.042J - 条件概率 Probability](https://www.youtube.com/watch?v=E6FbvM-FGZ8&index=19&list=PLB7540DEDD482705B)
- [MIT 6.042J - 独立](https://www.youtube.com/watch?v=l1BCv3qqW4A&index=20&list=PLB7540DEDD482705B)
- [MIT 6.042J - 随机变量](https://www.youtube.com/watch?v=MOfhhFaQdjw&list=PLB7540DEDD482705B&index=21)
- [MIT 6.042J - 期望 I](https://www.youtube.com/watch?v=gGlMSe7uEkA&index=22&list=PLB7540DEDD482705B)
- [MIT 6.042J - 期望 II](https://www.youtube.com/watch?v=oI9fMUqgfxY&index=23&list=PLB7540DEDD482705B)
- [MIT 6.042J - 大偏差](https://www.youtube.com/watch?v=q4mwO2qS2z4&index=24&list=PLB7540DEDD482705B)
- [MIT 6.042J - 随机游走](https://www.youtube.com/watch?v=56iFMY8QW2k&list=PLB7540DEDD482705B&index=25)

- [Simonson: 近似算法 (视频)](https://www.youtube.com/watch?v=oDniZCmNmNw&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=19)

- **字符串匹配**
- Rabin-Karp（视频）
    - [Rabin Karps 算法](https://www.coursera.org/learn/data-structures/lecture/c0Qkw/rabin-karps-algorithm)
    - [预计算](https://www.coursera.org/learn/data-structures/lecture/nYrc8/optimization-precomputation)
    - [优化：实施和分析](https://www.coursera.org/learn/data-structures/lecture/h4ZLc/optimization-implementation-and-analysis)
    - [表翻倍，Karp-Rabin](https://www.youtube.com/watch?v=BRO7mVIFt08&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=9)
    - [滚动哈希，摊销分析](https://www.youtube.com/watch?v=w6nuXg0BISo&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=32)
- Knuth-Morris-Pratt (KMP)：
    - [Knuth-Morris-Pratt（KMP）字符串匹配算法](https://www.youtube.com/watch?v=5i7oKodCRJo)
- Boyer–Moore 字符串搜索算法
    - [Boyer–Moore 字符串搜索算法](https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm)
    - [高级字符串搜索 Boyer-Moore-Horspool 算法（视频）](https://www.youtube.com/watch?v=QDZpzctPf10)
- [Coursera：字符串算法](https://www.coursera.org/learn/algorithms-on-strings/home/week/1)
    - 刚开始时很棒，但是当它超过 KMP 时，它变得比需要复杂得多
    - 很好的字典树解释
    - 可以跳过

- **排序**

- 斯坦福大学关于排序算法的视频:
    - [课程 15 | 编程抽象（视频）](https://www.youtube.com/watch?v=ENp00xylP7c&index=15&list=PLFE6E58F856038C69)
    - [课程 16 | 编程抽象（视频）](https://www.youtube.com/watch?v=y4M9IVgrVKo&index=16&list=PLFE6E58F856038C69)
- Shai Simonson 视频，[Aduni.org](http://www.aduni.org/):
    - [算法 - 排序 - 第二讲（视频）](https://www.youtube.com/watch?v=odNJmw5TOEE&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=2)
    - [算法 - 排序 2 - 第三讲（视频）](https://www.youtube.com/watch?v=hj8YKFTFKEE&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=3)
- Steven Skiena 关于排序的视频:
    - [CSE373 2020 - 归并排序/快速排序（视频）](https://www.youtube.com/watch?v=jUf-UQ3a0kg&list=PLOtl7M3yp-DX6ic0HGT0PUX_wiNmkWkXx&index=8)
    - [CSE373 2020 - 线性排序（视频）](https://www.youtube.com/watch?v=0ksyQKmre84&list=PLOtl7M3yp-DX6ic0HGT0PUX_wiNmkWkXx&index=9)

- NAND 到 Tetris: [从第一原理构建现代计算机](https://www.coursera.org/learn/build-a-computer)

## 视频系列

坐下来，尽情享受。

- [个人的动态规划问题列表 (都是短视频)](https://www.youtube.com/playlist?list=PLrmLmBdmIlpsHaNTPP_jHHDx_os9ItYXr)

- [x86 架构，汇编，应用程序 (11 个视频)](https://www.youtube.com/playlist?list=PL038BE01D3BAEFDB0)

- [MIT 18.06 线性代数，2005 年春季 (35 个视频)](https://www.youtube.com/playlist?list=PLE7DDD91010BC51F8)

- [绝妙的 MIT 微积分：单变量微积分](https://www.youtube.com/playlist?list=PL3B08AE665AB9002A)

- [Skiena 讲座来自《算法设计手册》- CSE373 2020 - 算法分析（26 个视频）](https://www.youtube.com/watch?v=22hwcnXIGgk&list=PLOtl7M3yp-DX6ic0HGT0PUX_wiNmkWkXx&index=1)

- [UC Berkeley 61B (2014 年春季): 数据结构 (25 个视频)](https://www.youtube.com/watch?v=mFPmKGIrQs4&list=PL-XXv-cvA_iAlnI-BQr9hjqADPBtujFJd)

- [UC Berkeley 61B (2006 年秋季): 数据结构 (39 个视频)](https://www.youtube.com/playlist?list=PL4BBB74C7D2A1049C)

- [UC Berkeley 61C：计算机结构 (26 个视频)](https://www.youtube.com/watch?v=gJJeUFyuvvg&list=PL-XXv-cvA_iCl2-D-FS5mk0jFF6cYSJs_)

- [OOSE：使用 UML 和 Java 进行软件开发 (21 个视频)](https://www.youtube.com/playlist?list=PLJ9pm_Rc9HesnkwKlal_buSIHA-jTZMpO)

- [MIT 6.004: 计算结构 (49 视频)](https://www.youtube.com/playlist?list=PLrRW1w6CGAcXbMtDFj205vALOGmiRc82-)

- [卡內基梅隆大学 - 计算机架构讲座 (39 个视频)](https://www.youtube.com/playlist?list=PL5PHm2jkkXmi5CxxI7b3JCL1TWybTDtKq)

- [MIT 6.006：算法介绍 (47 个视频)](https://www.youtube.com/watch?v=HtSuA80QTyo&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&nohtml5=False)

- [MIT 6.033：计算机系统工程 (22 个视频)](https://www.youtube.com/watch?v=zm2VP0kHl1M&list=PL6535748F59DCA484)

- [MIT 6.034：人工智能，2010 年秋季 (30 个视频)](https://www.youtube.com/playlist?list=PLUl4u3cNGP63gFHB6xb-kVBiQHYe_4hSi)

- [MIT 6.042J：计算机科学数学, 2010 年秋季 (25 个视频)](https://www.youtube.com/watch?v=L3LMbpZIKhQ&list=PLB7540DEDD482705B)

- [MIT 6.046：算法设计与分析 (34 个视频)](https://www.youtube.com/watch?v=2P-yW7LQr08&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)

- [MIT 6.050J：信息和熵, 2008 年春季 (19 个视频)](https://www.youtube.com/watch?v=phxsQrZQupo&list=PL_2Bwul6T-A7OldmhGODImZL8KEVE38X7)

- [MIT 6.851：高等数据结构 (22 个视频)](https://www.youtube.com/watch?v=T0yzrZL1py0&list=PLUl4u3cNGP61hsJNdULdudlRL493b-XZf&index=1)

- [MIT 6.854: 高等算法, 2016 年春季 (24 个视频)](https://www.youtube.com/playlist?list=PL6ogFv-ieghdoGKGg2Bik3Gl1glBTEu8c)

- [Harvard COMPSCI 224：高级算法（25 个视频）](https://www.youtube.com/playlist?list=PL2SOU6wwxB0uP4rJgf5ayhHWgw7akUWSf)

- [MIT 6.858：计算机系统安全, 2014 年秋季](https://www.youtube.com/watch?v=GqmQg-cszw4&index=1&list=PLUl4u3cNGP62K2DjQLRxDNRi0z2IRWnNh)

- [斯坦福: 编程范例 (27 个视频)](https://www.youtube.com/playlist?list=PL9D558D49CA734A02)

- [密码学导论，Christof Paar](https://www.youtube.com/playlist?list=PL6N5qY2nvvJE8X75VkXglSrVhLv1tVcfy)
    - [课程网站以及幻灯片和问题集](http://www.crypto-textbook.com/)

- [大数据 - 斯坦福大学 (94 个视频)](https://www.youtube.com/playlist?list=PLLssT5z_DsK9JDLcT8T62VtzwyW9LNepV)

- [图论，Sarada Herke（67 个视频）](https://www.youtube.com/user/DrSaradaHerke/playlists?shelf_id=5&view=50&sort=dd)

## 计算机科学课程

- [在线 CS 课程目录](https://github.com/open-source-society/computer-science)
- [CS 课程目录 (一些是在线讲座)](https://github.com/prakhar1989/awesome-courses)

## 算法实现

- [普林斯顿大学的多算法实现](https://algs4.cs.princeton.edu/code)

## 论文

- [喜欢经典的论文？](https://www.cs.cmu.edu/~crary/819-f09/)
- [1978: 通信顺序处理](http://spinroot.com/courses/summer/Papers/hoare_1978.pdf)
    - [Go 实现](https://godoc.org/github.com/thomas11/csp)
- [2003: The Google 文件系统](http://static.googleusercontent.com/media/research.google.com/en//archive/gfs-sosp2003.pdf)
    - 2012 年被 Colossus 取代了
- [2004: MapReduce: Simplified Data Processing on Large Clusters](http://static.googleusercontent.com/media/research.google.com/en//archive/mapreduce-osdi04.pdf)
    - 大多被云数据流取代了?
- [2006 年：Bigtable：结构化数据的分布式存储系统](https://static.googleusercontent.com/media/research.google.com/en//archive/bigtable-osdi06.pdf)
- [2006 年：针对松散耦合的分布式系统的 Chubby Lock 服务](https://research.google.com/archive/chubby-osdi06.pdf)
- [2007 年：Dynamo：亚马逊的高可用键值存储](http://s3.amazonaws.com/AllThingsDistributed/sosp/amazon-dynamo-sosp2007.pdf)
    - Dynamo 论文启动了 NoSQL 革命
- [2007: 每个程序员都应该知道的内存知识 (非常长，作者建议跳过某些章节来阅读)](https://www.akkadia.org/drepper/cpumemory.pdf)
- 2012: AddressSanitizer: 快速的内存访问检查器:
    - [论文](http://static.googleusercontent.com/media/research.google.com/en//pubs/archive/37752.pdf)
    - [视频](https://www.usenix.org/conference/atc12/technical-sessions/presentation/serebryany)
- 2013: Spanner: Google 的分布式数据库:
    - [论文](http://static.googleusercontent.com/media/research.google.com/en//archive/spanner-osdi2012.pdf)
    - [视频](https://www.usenix.org/node/170855)
- [2015: Google 的持续流水线](http://static.googleusercontent.com/media/research.google.com/en//pubs/archive/43790.pdf)
- [2015: 大规模高可用性：构建 Google 广告数据基础设施](https://static.googleusercontent.com/media/research.google.com/en//pubs/archive/44686.pdf)
- [2015: 开发人员如何搜索代码：一个案例研究](http://static.googleusercontent.com/media/research.google.com/en//pubs/archive/43835.pdf)
- 更多论文： [1,000 篇论文](https://github.com/0voice/computer_expert_paper)

---
