# ucoreOS操作系统实验——Lab8

## 问题发现与改进

## 练习0

终于最后一次cv了真好。此外在`proc.c`中更改

```c
static struct proc_struct *alloc_proc(void) {
    // 初始化 PCB 下的 fs(进程相关的文件信息)
    proc->filesp = NULL;
}
int do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf) {
    // 使用 copy_files()函数复制父进程的fs到子进程中
    if (copy_files(clone_flags, proc) != 0) {
        goto bad_fork_cleanup_kstack;
    }
}
```

## 练习一

> **完成读文件操作的实现（需要编码）**
>
> 首先了解打开文件的处理流程，然后参考本实验后续的文件读写操作的过程分析，编写在sfs_inode.c中sfs_io_nolock读文件中数据的实现代码

打开文件的执行流程：

- 用户进程使用`read(fd, data, len);`读取磁盘上的文件
- 系统调用read->sys_read->syscall，进入内核态
- 通过ISR调用到sys_read内核函数，并进一步调用sysfile_read内核函数，进入文件系统抽象层
- 检查长度是否为0以及文件是否可读
- 分配buffer，即调用kmalloc分配4096字节的buffer空间，之后通过file_read每次读取buffer大小循环读文件，再通过调用copy_to_user函数将读到的内容拷贝到用户的内存空间中，直到指定len长度读取完毕，最后返回用户程序，用户程序收到读文件的内容
- 在file_read中，通过文件描述符查找到相应文件对应内存中的inode信息，然后转交给vop_read进行读取处理，实际上就是转交给sfs_read，调用sfs_io再进一步调用sfs_io_nolock

`sfs_io_nolock`函数功能是针对指定的文件（文件对应的内存中的inode信息已经给出），从指定偏移量进行指定长度的读或者写操作

- 一系列的边界检查，访问是否合法
- 将读写操作实用的函数指针同一，针对整块操作
- 之后是我们要完成的部分，根据操作不落在整块数据块的各种情况进行分别处理

![lab8-1](/Users/zhouchenfei/Desktop/OS截图/lab8-1.png)

### 问题回答

> 请在实验报告中给出设计实现”UNIX的PIPE机制“的概要设方案，鼓励给出详细设计方案

- 可以考虑在磁盘上保留一部分空间或是一个特定的文件作为pipe机制的缓冲区
  - 当某两个进程之间要求建立管道，假设进程A的标准输出是进程B的标准输入，就可以在这两个进程的PCB上新增成员变量来记录进程的这种属性，同时生成一个临时的文件，将其在进程A、B中打开
  - 当进程A使用标准输出进行write系统调用时，通过PCB中的变量得知，需要将这些标准输出的数据输出到先前创建的临时文件中
  - 当进程B使用标准输入的时候进行read系统调用的时候，通过PCB中的变量给出信息，需要从上述临时文件中读取数据

## 练习二

> 改写proc.c中的load_icode函数和其他相关函数，实现基于文件系统的执行程序机制。执行：make qemu。如果能看看到sh用户程序的执行界面，则基本成功了。如果在sh用户界面上可以执行”ls”,”hello”等其他放置在sfs文件系统中的其他执行程序，则可以认为本实验基本成功

这次要完成的主要是具有从磁盘读取可执行文件并加载到内存功能的`load_icode`，和lab5中的区别是lab5仅将原先就加载到了内核内存空间中的ELF可执行文件加载到用户内存空间中，并没有涉及磁盘读取，而且也没有考虑给需要执行的应用程度传递操作的可能性

那么根据lab5中的help_comment，`load_icode`实现流程：

- 给要执行的用户进程创建一个新的内存管理结构mm，因为原来的mm已经在`do_execve`中被释放了
- 创建用户内存空间的新的页目录表PDT
- 将磁盘上的ELF文件的TEXT/DATA/BSS段正确地加载到用户空间中
  - 从磁盘中读取elf文件的header
  - 根据elfheader中的信息，获取到磁盘上的program header
  - 对于每个program header
    - 为TEXT/DATA段在用户内存空间上的保存分配物理内存页，同时建立物理页和虚拟页的映射关系
    - 从磁盘上读取TEXT/DATA段，并且复制到用户内存空间上去；
    - 根据program header得知是否需要创建BBS段，如果是，则分配相应的内存空间，并且全部初始化成0，并且建立物理页和虚拟页的映射关系
  - 将用户栈的虚拟空间设置为合法，并且为栈顶部分先分配4个物理页，建立好映射关系
  - 切换到用户地址空间；
  - 设置好用户栈上的信息，即需要传递给执行程序的参数
  - 设置好中断帧

实现：
![lab8-2](/Users/zhouchenfei/Desktop/OS截图/lab8-2.png)

### 问题回答

> 请在实验报告中给出设计实现基于”UNIX的硬链接和软链接机制“的概要设方案，鼓励给出详细设计方案

保存在磁盘上的inode信息均包含一个nlinks变量用于表示当前文件的被链接的次数

- 如果在磁盘上创建文件A的软链接B，首先将B当成正常的文件创建inode，然后将TYPE域设置为链接，然后使用剩余的域中的一个，指向A的inode位置，然后再额外使用一个位来标记当前的链接是软链接还是硬链接

- 当读写操作等系统调用访问B时，判断B是否为一个链接，则实际是将对B指向的文件A（已知A的inode位置）进行操作

- 当删除一个软链接B的时候，直接将其在磁盘上的inode删掉即可

  

- 如果在磁盘上的文件A创建一个硬链接B，那么在按照软链接的方法创建完B之后，还需要将A中的被链接的计数加1；

- 访问硬链接的方式与访问软链接一致

- 当删除一个硬链接B的时候，除了需要删除掉B的inode之外，还需要将B指向的文件A的被链接计数减1，如果减到了0，则将A删除

## 实验结果

<img src="/Users/zhouchenfei/Desktop/OS截图/lab8-3.jpg" alt="lab8-3" style="zoom:33%;" />

ls：

![lab8-4](/Users/zhouchenfei/Desktop/OS截图/lab8-4.jpg)

hello：

![lab8-5](/Users/zhouchenfei/Desktop/OS截图/lab8-5.png)

