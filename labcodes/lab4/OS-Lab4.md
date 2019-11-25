# ucoreOS操作系统实验——Lab4

## 问题发现与改进

- 这次像lab3一样的打patch出现了问题，然后使用git提交版本号打也出现了小的问题，然后通过labcodes_answer打patch就成功了。。并不知道为什么，可能answer的patch和Makefile就是对的吧\_(:з」∠)_不过反正这个代码是为了make grade通过，所以不是我以前写了一大堆注释的不简洁的代码，肯定效果更好，不会因为多的乱七八糟的注释而变得混乱

- 第一次运行`make qemu`以后得到了理想的输出，但是和示例输出不一样的是，我多了中断的堆栈信息

  ![lab4-3](/Users/zhouchenfei/Desktop/OS截图/lab4-3.jpg)

  ![lab4-4](/Users/zhouchenfei/Desktop/OS截图/lab4-4.png)

  然后尝试了一下`make grade`发现只有90分，显示是少了check_slab的部分

  但是我明明就有![lab4-5](/Users/zhouchenfei/Desktop/OS截图/lab4-5.jpg)

  又看了看grade.sh，估计是固定检查`make qemu`的输出是否有`函数 succeeded !`这一句，大概是语句写错了

  找了半天都没找到哪里有check_slab(甚至都想去改grade.sh了。。)，最后在kern/mm/kmalloc.c里面找到了，然后改成succeeded!就对了，这都什么莫名其妙的错。。。

## 练习0

> 见问题改进

## 练习一

> alloc_proc函数（位于kern/process/proc.c中）负责分配并返回一个新的struct proc_struct结构，用于存储新建立的内核线程的管理信息。ucore需要对这个结构进行最基本的初始化，你需要完成这个初始化过程。
>
> > 需要初始化的proc_struct结构中的成员变量至少包括：state/pid/runs/kstack/need_resched/parent/mm/context/tf/cr3/flags/name。

###关键数据结构

根据提示先行查看proc结构的成员变量都是什么以及功能是什么

```c
//kern/process/proc.h   ❇︎表示本lab比较重要
struct proc_struct {
    enum proc_state state;//进程所处的状态 未初始化、sleep、Zoombie、运行  ❇︎
    int pid;              //进程ID                                    ❇︎
    int runs;             //运行时间
    uintptr_t kstack;     //内核栈，记录了分配给该线程的内核栈的位置。对内核线程是运行时的程序使用的栈；而对是发生特权级改变的时候使保存被打断的硬件信息用的栈          ❇︎
    volatile bool need_resched;// 对于释放CPU时是否需要调度的值？
    struct proc_struct *parent;// 父进程
    struct mm_struct *mm;// 内存管理的信息，包括内存映射列表、页表指针等等，这里其实不用考虑换页
    struct context context;  // 进程的上下文，用于进程切换，通用寄存器等     ❇︎
    struct trapframe *tf;  //中断帧的指针 指向当前中断状态                 ❇︎
    uintptr_t cr3;  // 保存页表的物理地址PDT 进程切换的时候方便直接使用lcr3实现页表切换
    uint32_t flags;    //标志位
    char name[PROC_NAME_LEN + 1]; //进程名
    list_entry_t list_link;   // 进程链表 
    list_entry_t hash_link;   // 哈希？
};
```

###实现

再根据proc.c里面的help_comment可以很容易实现练习一

除了特定的几个属性其它都赋0/NULL即可

<img src="/Users/zhouchenfei/Desktop/OS截图/lab4-1.png" alt="lab4-1" style="zoom:33%;" />

### 回答问题

> 请说明proc_struct中`struct context context`和`struct trapframe *tf`成员变量含义和在本实验中的作用是啥？（提示通过看代码和编程调试可以判断出来）

- **context**：进程上下文,用于在上下文切换时保存当前通用寄存器(除%eax)及eip的值

  除了为了简化切换模式而省略掉的返回寄存器%eax(可以在栈上对应找到)，保存其它所有通用寄存器以及eip的值

```c
struct context {
    uint32_t eip;
    uint32_t esp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
};
```

​	参考proc_run()和copy_thread()

- **tf**：中断帧，调度往往发生在时钟中断的时候，所以调度执行进程的时候，需要进行中断返回

  tf变量的作用在于在构造出了新的线程的时候，如果要将控制权交给这个线程，是使用中断返回的方式进行的，因此需要构造出一个伪造的中断返回现场，即trapframe，使得可以正确地将控制权转交给新的线程

  具体切换到新的线程的做法为，调用switch_to(switch.S)函数，然后在该函数中进行函数返回，直接跳转到forkret函数，最终进入中断返回函数__trapret，之后便可以根据tf中构造的中断返回地址切换到新的线程

## 练习二

> 创建一个内核线程需要分配和设置好很多资源。kernel_thread函数通过调用**do_fork**函数完成具体内核线程的创建工作。do_kernel函数会调用alloc_proc函数来分配并初始化一个进程控制块，但alloc_proc只是找到了一小块内存用以记录进程的必要信息，并没有实际分配这些资源。ucore一般通过do_fork实际创建新的内核线程。do_fork的作用是，创建当前内核线程的一个副本，它们的执行上下文、代码、数据都一样，但是存储位置不同。在这个过程中，需要给新内核线程分配资源，并且复制原进程的状态。你需要完成在kern/process/proc.c中的do_fork函数中的处理过程。它的大致执行步骤包括：
>
> - 调用alloc_proc，首先获得一块用户信息块。
> - 为进程分配一个内核栈。
> - 复制原进程的内存管理信息到新进程（但内核线程不必做此事）
> - 复制原进程上下文到新进程
> - 将新进程添加到进程列表
> - 唤醒新进程
> - 返回新进程号

### 相关宏即函数定义

```c
alloc_proc//proc.c刚完成的 分配一个进程
setup_kstack//proc.c给线程内核栈分配一个大小为KSTACKPAGE(2Page 8KB)的页
copy_mm//proc.c 根据clone_flags对虚拟内存空间进行拷贝 如果和CLONE_VM(pmm.h)一致则共享否则赋值
copy_thread//proc.c 拷贝设置tf以及context(返回eip和栈顶指针esp)中堆栈的信息
hash_proc//proc.c  把进程加到哈希表里
get_pid//proc.c  为新进程创建一个pid
wakup_proc//sched.c  通过将状态置为runable达到唤醒进程的目的
```

### 实现

同样照着comment实现

<img src="/Users/zhouchenfei/Desktop/OS截图/lab4-2.png" alt="lab4-2" style="zoom:33%;" />

### 回答问题

> 请说明ucore是否做到给每个新fork的线程一个唯一的id？请说明你的分析和理由。

能做到

查看`get_pid`代码

<img src="/Users/zhouchenfei/Desktop/OS截图/lab4-8.png" alt="lab4-8" style="zoom:33%;" />

- 在该函数中使用了两个静态局部变量`next_safe`和`last_pid`，在每次进入`get_pid`函数的时候，这两个变量的数值之间的取值均是合法(尚未使用)的`pid`，如果有严格的`next_safe > last_pid + 1`，就可以直接取`last_pid + 1`作为新的`pid`(`last_pid`就是上一次分配的`PID`)

- 如果`next_safe > last_pid + 1`不成立，则在循环中通过`if (proc->pid == last_pid)`确保不存在任何进程的`pid`与`last_pid`相同，再通过`if (proc->pid > last_pid && next_safe > proc->pid)`保证了不存在任何已经存在的`pid`满足：`last_pid<pid<next_safe`，这样就保证最后能够找到一个满足条件的区间，来获得合法的`pid`

## 练习三

> 请在实验报告中简要说明你对proc_run函数的分析。并回答如下问题：
>
> - 在本实验的执行过程中，创建且运行了几个内核线程？
> - 语句`local_intr_save(intr_flag);....local_intr_restore(intr_flag);`在这里有何作用?请说明理由

`proc_run`函数的作用是让线程在CPU上运行起来,即将CPU的控制权交给指定线程

### 执行过程

```c
void proc_run(struct proc_struct *proc) {
    if (proc != current) {// 判断指定线程是否正在运行
        bool intr_flag;
        struct proc_struct *prev = current, *next = proc;
        local_intr_save(intr_flag);//sync.h 关闭中断
        {
            current = proc;    //current->将要执行的process
            load_esp0(next->kstack + KSTACKSIZE);//pmm.c 设置TSS
            lcr3(next->cr3);// libs/x86.h 内联编译 修改当前cr3为需要运行线程的页目录表PDT
            switch_to(&(prev->context), &(next->context));//切换到新的线程
        }
        local_intr_restore(intr_flag);//恢复中断
    }
}
```

- 保存FL_IF(中断标志位intr_flag)并禁止中断
- 将current指针指向将要执行的进程，设置`任务状态段ts`中特权态0下的栈顶指针`esp0`为将要执行线程(next)的内核栈栈顶，即next->kstack + KSTACKSIZE
- 加载新的页表，设置CR3寄存器的值为`next->cr3`(由于lab4都是内核进程，所以这一步其实没用)
- 调用switch_to进行切换
- 当执行proc_run的进程恢复执行之后，恢复FL_IF

### 回答问题

> 查看init_proc及运行结果可得

- 两个：idleproc(pid=0)和initproc(pid=1)
  - idleproc  最初的内核线程，在完成新的内核线程的创建以及各种初始化工作之后，进入死循环不断寻找可以调度的任务执行
  - initproc  用于打印"Hello World"的线程
- 该语句作用是关闭中断，使得在这个语句块内的执行内容不会被中断打断，是一个原子操作
- 在进程切换过程需要避免中断干扰以免产生不必要的错误，所以在切换进程期间将FL_IF(中断标志位)保存并禁止中断，等到进程切换完毕之后再将FL_IF恢复

## 实验结果

完成之后运行`make qemu`和`make grade`可以得到如下结果

​	![lab4-6](/Users/zhouchenfei/Desktop/OS截图/lab4-6.jpg)

![lab4-7](/Users/zhouchenfei/Desktop/OS截图/lab4-7.jpg)

## Challenge

> 这不是本实验的内容，其实是上一次实验内存的扩展，但考虑到现在的slab算法比较复杂，有必要实现一个比较简单的任意大小内存分配算法。可参考本实验中的slab如何调用基于页的内存分配算法（注意，不是要你关注slab的具体实现）来实现first-fit/best-fit/worst-fit/buddy等支持任意大小的内存分配算法。。

看到是slab相关其实我就不想做。。。

参考kern/mm/kmalloc.c中关于SLOB和SLAB的解释以及Linux的实现文档

大概看懂了ucore是怎么做的，但要让我移植到first fit/best fit上一时半会还想不出来，所以就先空着，如果后面lab都比较顺利的话回来看看SLAB的实现

