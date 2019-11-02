# ucoreOS操作系统实验——Lab2  

>  吐槽踩坑

- 由于以前直接merge踩过巨大的坑所以选择了手动merge，但是由于没仔细看init.c所以没有lab1练习六输出的时候感觉很奇怪就下了meld查看对比，发现好像并没有出错，于是找了半天原因，直到想起来去看一眼init.c才发现idt_init执行是在pmm_init之后的，而pmm并没有完成所以肯定不会有结果。其实也算不上坑，应该算是长了一个教训，应该完整看完实验内容不然东墙补完补西墙是不能长久的，以后应该再深入研究一下diff merge这些优秀的工具，毕竟手动出错概率高

- 一开始看完注释还是懵的，跟lab1详细到每一行代码的注释不一样，lab2有一大篇的全面介绍，之后就是靠自己优化first-fit算法了，虽然注释很详尽全面但还是不知道first-fit在干嘛，所以找到了mooc5.3连续内存分配中对三种算法的介绍，总算最后摸到一点头脑，而且lab2的代码乍一看已经挺完善的了，但仔细结合实验报告的要求一看其实不符合实验要求，有的是条件有问题，有的是思路不一致等待

- 练习二和三就更是一头雾水了，可能跟理论课还没有将相关内容以及计组全忘了有关，所以着重看了实验指导书附录内容和mooc6.3-4以及8.4-6，再根据get_pte和page_remove_pte的执行顺序以及相关注释逐句翻译注释并理解程序这么去写的意图就会理解的很好(但其实get_pte都没看懂是在干嘛，参考了各种Github大佬的报告以及好多遍的MOOC才get到，不过也跟我提前写了这次作业有关(内存什么的还没怎么讲))现在好了，讲完了再看好像就没那么难了

- 基本实现了实验要求的所有练习以后使用`make grade`出满分以后，满心欢喜的以为顺利完成了实验，结果`make qemu`之后出现了如下结果

  ![lab2-11](file:///Users/zhouchenfei/Desktop/OS%E6%88%AA%E5%9B%BE/lab2-11.png?lastModify=1572702691)

  terminal告诉我了报错原因和位置，这其实是一个本身就用的验证机制，按理说不会出现问题，而且更奇怪的是就在这个assert为空之前才有代码定义了将之设为NULL，所以是真的不知道是出了什么故障，所以又换ubuntu跑了一遍，发现报错的原因又跑到了pmm_init的下一个函数，于是通过gdb单步调试找到了问题，是保存文件的时候出现了故障，原本应该保存的`make grade`后正确的版本并没有保存，保留的是将`struct Page *p = le2page(le, page_link);`这一简单初始化直接写成NULL的版本，所以在后续的assert中才会出现验证为NULL的情况，所以又从Mac换回了ubuntu(太菜了不配使用Mac)

## 练习0

> 本实验依赖实验1。请把你做的实验1的代码填入本实验中代码中有“LAB1”的注释相应部分。提示：可采用diff和patch工具进行半自动的合并（merge），也可用一些图形化的比较/merge工具来手动合并，比如meld，eclipse中的diff/merge工具，understand中的diff/merge工具等

​	手动复制merge，由于pmm_init()尚未完成所以练习六结果没有输出，等所有代码都输出以后即可看到练习六输出，merge以后查看结果，发现其实lab2并不仅仅是拷贝了lab1的成果，比如在bootloader执行kern_init之前lab2会先行调用entry.S中的kern_entry，它为执行kern_init设置堆栈，而且临时建立了一个段映射关系，这些都为之后建立分页机制的过程做一个准备，完成这些之后，才调用kern_init。

##  练习一

> **实现 first-fit 连续物理内存分配算法（需要编程）**
>
> 在实现first fit 内存分配算法的回收函数时，要考虑地址连续的空闲块之间的合并操作。提示:在建立空闲页块链表时，需要按照空闲页块起始地址来排序，形成一个有序的链表。可能会修改default_pmm.c中的default_init，default_init_memmap，default_alloc_pages， default_free_pages等相关函数。请仔细查看和理解default_pmm.c中的注释

​	阅读default_pmm.c中的注释，需要改写default_init，default_init_memmap，default_alloc_pages, default_free_pages几个函数

​	查看FFMA算法部分注释将代码分为以下几个部分：

#### Prepare

​	了解list.h中的*list_entry*结构并且会使用与该结构相关的函数

```c
//list.h
struct list_entry {//一个简单的双链表
    struct list_entry *prev, *next;//两个指针父子节点
};
typedef struct list_entry list_entry_t;//重命名
list_init() //初始化一个新的list_entry
list_add(),list_add_after(),list_add_after()//添加一个新的表项
list_del()//从表中删除一个表项
list_del_init()//从表中删除并重定义一个表项
list_empty()//判断链表是否为空
list_next()list_prev()//获取链表的前一项和后一项
```

​	另一种复杂的办法是将普通链表结构转化为页，有相关宏le2page在memlayout.h中

```c
//memlayout.h
struct Page {//Page结构定义
    int ref;        // page frame's reference counter
    uint32_t flags; // array of flags that describe the status of the page frame
    unsigned int property;// the num of free block, used in first fit pm manager
    list_entry_t page_link;// free list link
};
// le2page宏定义
#define le2page(le, member) to_struct((le), struct Page, member)
```

#### default_init

​	该函数可被直接使用，用以初始化free_list和将nr_free置0

<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-1.png" alt="lab2-1" style="zoom:40%;" />

​	free_area_t结构定义在memlayout.h中

<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-2.png" alt="lab2-2" style="zoom: 40%;" />

#### default_init_memmap

1. 首先初始化每个在memelayout.h中定义的page
   - p→flags应被置为PG_property，表示该页是可用的以及使用p->page_link将页链接到free_list里面，最后更正nr_free的数目
   - 根据注释提示将default_init_memmap改写成如下，主要变化是①list_add变为list_add_before和SetPageProperty一起被移到循环内，表示后续的连续空页要被设为保留页然后链接成一个双向链表

<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-3.png" alt="lab2-3" style="zoom:50%;" />

​	整个default_init_memmap函数的作用是传入base页地址及生成物理页的个数n，将物理页初始化设为保留页后与base连接，由于base既是空页也是首页所以将其property设为n，nr_free空页数设为n(虽然写了好多，但其实只改了最后一句，双向链表，所以头指针的前一个就是最后一个)

#### default_alloc_pages

​	该函数作用是在free list中找到一个大小≥n的块重新分配其大小并返回这个分配后的块地址

​	<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-4.png" alt="lab2-4" style="zoom:40%;" />

​		代码有所变动的地方在于检查不为空后，根据注释判断p→property的大小，≥n要对两个标志位修改并且＞n的要将块分隔，如果不符合则要返回NULL

#### default_free_pages

​	该函数重新将页链接到free list，也可以是将小的空块合并到大块中<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-5.png" alt="lab2-5" style="zoom:50%;" />

​	代码基本都做了改动，主要是合并部分的判断和操作，因为原代码是从头找到尾，找是否有free块相邻的块，但其实根据first fit，只要照着第一个判断出来然后合并就可以了

#### 回答问题

​	实验中的first-fit算法使用链表进行查找，时间复杂度为O(N)，可以使用树状结构，尽管alloc的过程变成DFS后复杂度仍然是O(N)，但free过程可以使用二分查找，复杂度为O(logn)，此外还可以使用mooc5.3中的best-fit和worst-fit算法

## 练习二

> **实现寻找虚拟地址对应的页表项（需要编程）**
>
> 通过设置页表和对应的页表项，可建立虚拟内存地址和物理内存地址的对应关系。其中的get_pte函数是设置页表项环节中的一个重要步骤。此函数找到一个虚地址对应的二级页表项的内核虚地址，如果此二级页表项不存在，则分配一个包含此项的二级页表。本练习需要补全get_pte函数 in kern/mm/pmm.c，实现其功能。

​	借助mooc8.5虚拟页式存储以及实验附录，理清get_pte函数的调用及功能，再借助注释完成编程

​	阅读注释给出的宏及其定义

```c
//宏 OR 函数
PDX(la) // 返回虚拟地址la的页目录索引 /kern/mm/mmu.h
KADDR(pa) //返回物理地址pa对应的虚拟地址 /kern/mm/pmm.h
set_page_ref(page,1) //设置此页被引用一次  /kern/mm/pmm.h
page2pa(page) //得到page管理的那一页的物理地址  /kern/mm/pmm.h
struct Page * alloc_page()  //分配一页出来 /kern/mm/pmm.h
memset(void * s, char c, size_t n)   //设置s指向地址的前面n个字节为‘c’
//define
PTE_P 0x001 //表示物理内存页存在
PTE_W 0x002 //表示物理内存页内容可写
PTE_U 0x004 //表示可以读取对应地址的物理内存页内容
```

​	可以看到mmu.h中注释如下

<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-6.png" alt="lab2-6" style="zoom:40%;" />

​	表示页式管理将32位的线性地址拆分为三部分：Directory、Table和Offset，ucore页式管理通过一个二级的页表实现。一级页表存放在高10位中，二级页表存放于中间10位中，最后的12位表示偏移量，据此可以证明，页大小为4KB（2的12次方，4096）

​	Directory为一级页表，`PDX(la)`可以获取Directory；Table为二级页表，`PTX(la)`可以获取Table

​	寻找虚拟地址对应的页表项的主要步骤：

​	①在Directory里找，存在即直接返回；不存在且create为0则返回NULL

​	②成功获得一个page，清空，并把引用次数+1(设置为1)，并在Directory中建立该项并返回

​	get_pte的功能是根据基址返回pte虚拟线性地址，如果pte不存在就分配页，其实现如下：

<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-7.png" alt="lab2-7" style="zoom:33%;" />

​	得到最后return的pte虚拟线性地址的过程为找到Directory对应项中的Table地址，转为虚拟地址，再根据线性地址的offset找到对应页表

#### 回答问题

> - 请描述页目录项（Page Directory Entry）和页表项（Page Table Entry）中每个组成部分的含义以及对ucore而言的潜在用处。
> - 如果ucore执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？

1. <img src="/Users/zhouchenfei/Desktop/OS截图/lab2-8.png" alt="lab2-8" style="zoom:30%;" />

| 组成部分  | 含义                                       | 地址  |
| --------- | ------------------------------------------ | ----- |
| PTE_P     | 表示物理内存页是否存在                     | 0     |
| PTE_W     | 表示物理内存页是否可写                     | 1     |
| PTE_U     | 表示物理内存页是否可被用户获取(需要特权级) | 2     |
| PTE_PWT   | 表示物理内存页是否直写(write-through)      | 3     |
| PTE_PCD   | 禁用缓存                                   | 4     |
| PTE_A     | 表示物理内存页是否被使用过                 | 5     |
| PTE_D     | 脏页                                       | 6     |
| PTE_PS    | 设置物理内存页的大小                       | 7     |
| PTE_MBZ   | must be zero恒为0                          | 8     |
| PTE_AVAIL | 可以给OS设置和使用                         | 11    |
|           | 高20位                                     | 12-31 |

​	PDE和PTE的高20位类似，都是用来表示其指向的物理页的物理地址，0-9位如上，其中9-11位保留给OS，PDE和PTE的不同在于：①PDE的第7位用于设置page大小0表示4KB，而PTE的第7位恒为0；②PDE第6位恒为0，PTE第6位表示脏位即是否需要在swap out的时候写回外存；③PDE的第3位如上第4位设置为1则表示不对该页进行缓存，PTE的3-4位恒为0；

​	可以发现无论是PTE还是TDE，都具有着一些保留的位供OS使用，也就是说ucore可以利用这些位来完成一些其他的内存管理相关的算法，比如可以在这些位里保存最近一段时间内该页的被访问的次数（仅能表示0-7次），这些保留位有利于OS进行功能的拓展

2.  硬件需要完成：
   - 将引发页访问异常的线性地址保存在cr2寄存器中
   - 在中断栈中依次压入EFLAGS，CS, EIP，以及页访问异常码error code，如果page fault是发生在用户态，则还需要先压入ss和esp，并且切换到内核栈
   - 根据IDT查询到对应page fault的ISR跳转到对应ISR执行，之后由软件进行对Page  Fault 的处理

## 练习三

> **释放某虚地址所在的页并取消对应二级页表项的映射（需要编程）**
>
> 当释放一个包含某虚地址的物理内存页时，需要让对应此物理内存页的管理数据结构Page做相关的清除处理，使得此物理内存页成为空闲；另外还需把表示虚地址与物理地址对应关系的二级页表项清除。请仔细查看和理解page_remove_pte函数中的注释。为此，需要补全在 kern/mm/pmm.c中的page_remove_pte函数。

​	page_remove_pte函数的功能是释放与la对应线性地址相关的Page，并clear与之相关的pte无效

​	阅读注释给出的宏及其定义

```c
//宏 OR 函数
struct Page *page pte2page(*ptep) // //从ptep值中获取相应的页面 /kern/mm/pmm.h
free_page//释放一个页  /kern/mm/pmm.h
page_ref_dec(page)//减少该页的引用次数，返回剩下的引用次数 /kern/mm/pmm.h
tlb_invalidate(pde_t *pgdir, uintptr_t la)//当修改的页表目前正在被进程使用时，使之无效  /kern/mm/pmm.h
//define
PTE_P 0x001 //表示物理内存页存在
```

<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-9.png" alt="lab2-9" style="zoom:33%;" />

#### 回答问题

> 1. 数据结构Page的全局变量（其实是一个数组）的每一项与页表中的页目录项和页表项有无对应关系？如果有，其对应关系是啥？
> 2. 如果希望虚拟地址与物理地址相等，则需要如何修改lab2，完成此事？ **鼓励通过编程来具体完成这个问题**

1. 存在对应关系：页表项中存放着对应的物理页的物理地址，可以通过这个物理地址来获取对应的Page数组的对应项，具体做法为将物理地址除以一个页的大小，然后乘上一个Page结构的大小获得偏移量，使用偏移量加上Page数组的基地址即可得到对应Page项的地址

2. 阅读实验参考书“系统执行中地址映射的四个阶段”

   物理地址和虚拟地址之间存在偏移量

   物理地址 + KERNBASE = 虚拟地址

   所以，KERNBASE = 0时，物理地址 = 虚拟地址，把memlayout.h中改为

![lab2-10](/Users/zhouchenfei/Desktop/OS截图/lab2-10.png)

​	代码更改完毕后分别执行`make grade` `make qemu  ` 得到下列结果

<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-13.jpg" alt="lab2-11" style="zoom:30%;" />

<img src="/Users/zhouchenfei/Desktop/OS截图/lab2-12.png" alt="lab2-11" style="zoom:40%;" />

## Challenge 1 伙伴系统分配算法

​	首先参考[伙伴分配器的极简实现](http://coolshell.cn/articles/10427.html)以及[wuwenbin](https://github.com/wuwenbin)的代码

> 伙伴分配的实质就是一种特殊的“分离适配”，即将内存按2的幂进行划分，相当于分离出若干个块大小一致的空闲链表，搜索该链表并给出同需求最佳匹配的大小。

​	整个算法大体上可以表示为：

> **分配内存：**
> 寻找大小合适的内存块（大于等于所需大小并且最接近2的幂，比如需要27，实际分配32）
>
> 1. 如果找到了，分配给应用程序。
> 2. 如果没找到，分出合适的内存块。
>    1. 对半分离出高于所需大小的空闲内存块
>    2. .如果分到最低限度，分配这个大小。
>    3. 回溯到步骤1（寻找合适大小的块）
>    4. 重复该步骤直到一个合适的块
>
> **释放内存：**
>
> 释放该内存块
>
> 1. 寻找相邻的块，看其是否释放了。
> 2. 如果相邻块也释放了，合并这两个块，重复上述步骤直到遇上未释放的相邻块，或者达到最高上限（即所有内存都释放了）。
>
> > 再结合OSMOOC的示意图可以很好的理解伙伴系统的算法实现思路

​	**分配器的整体思想**

> 通过一个数组形式的完全二叉树来监控管理内存，二叉树的节点用于标记相应内存块的使用状态，高层节点对应大的块，低层节点对应小的块，在分配和释放中我们就通过这些节点的标记属性来进行块的分离合并。如图所示，假设总大小为16单位的内存，我们就建立一个深度为5的满二叉树，根节点从数组下标[0]开始，监控大小16的块；它的左右孩子节点下标[1\]\[2]，监控大小8的块；第三层节点下标[3\]\[6]监控大小4的块……依此类推。

![lab2-14](/Users/zhouchenfei/Desktop/OS截图/lab2-14.jpeg)

​	最后在[wuwenbin](https://github.com/wuwenbin/buddy2)代码的基础上更改处适用于ucore的buddy system

​	参照default_pmm.h结合buddy2.h实现buddy.h

![lab2-15](/Users/zhouchenfei/Desktop/OS截图/lab2-15.png)

​	参照default_pmm.cbuddy2.c实现buddy.h并在其中修改写成check函数，代码如下



​	`make qemu`结果如下

​	![lab2-16](/Users/zhouchenfei/Desktop/OS截图/lab2-16.png)

​	

## Challenge 2 任意大小的内存单元slub分配算法

> 太复杂了没看懂也看不下去了，所以大概可能做完所有的还有空会回来看看👀吧