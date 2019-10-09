

# ucoreOS操作系统实验——Lab1  

## 问题发现&改进

> 老师说让我们实验报告写犯的错de的bug还有吐的槽

- Qemu

  - 一开始安装qemu的时候没有弄清楚软链接硬链接的区别，但是报错的原因是没有仔细看命令的意思直接复制导致的

  - oslab上的qemu安装好进行实验会一直卡在Booting Service...，但找不出问题，所以从官网上下载最新版的归档安装才好

  - 安装qemu的时候我一直在踩坑就是各种无脑操作导致的qemu command not found

  - 大致安装流程：

    1. 在qemu.org下载最新归档文件，并按照官网给定安装方法安装，如图

    ![安装1](/Users/zhouchenfei/Desktop/OS截图/安装1.jpg)

    2. ![安装2](/Users/zhouchenfei/Desktop/OS截图/安装2.jpg)

    > 注意上面这个有几个命令要改，不能盲目cv，==cd到qemu文件夹里面==`configure --help`没必要执行(反正也不会看，看了也看不懂)，执行的话前面加`./` 下面的也是前面加`./` 最后软链接的时候直接`cd /usr/local/bin` 查看是不是已经有了许多qemu，如果是的话再执行下面这个软链接`sudo ln -s qemu-system-i386 qemu` 
    >
    > 直接cv铁定报错，还是要带着脑子看实验指导书

- 练习一是要学习看懂Makefile，但是Makefile其实和shell有很多相似之处，我就顺着注释往下看，有很多奇奇怪怪的地方，也查了各种option的含义，但是由于debug和grade生成与ucore.img无关所以没有仔细看，所以后面找运行报错都不知道上哪找(╥╯^╰╥)

- 练习五踩坑，注释里面除了最后一个输出换行使用的是cprintf前面都写的printf，我就全写的printf感觉差别影响不大，结果make qemu的时候就报错`did you mean cprintf`，查了一下发现cprintf的c是console的意思。cprintf适应窗口配合函数使用等等，总之就是配合硬件的非标准输出，而printf是标准输出是不支持console这边的

- 练习六的踩坑就是ubuntu系统上面无法很好的启动qemu加载ucore所以不能键盘中断回显，这一问题的原因未知，解决方法见下换Mac上使用qemu以及使用不显示图形化界面的qemu命令解决

- ![lab1-41](/Users/zhouchenfei/Desktop/OS截图/lab1-41.png)使用`make clean`解决，错误提示写着600>>510，结合练习一第二问可以得知此处是因为扇区大小超过了sign.c规定的510B的限制

- 挣扎了一下lab2后发现还是本机好用一些，于是又开始挣扎MacOS上安装配置使用qemu，homebrew下载之后运行make qemu报了一堆clang编译器gcc的错

  <img src="/Users/zhouchenfei/Desktop/OS截图/lab1-30.png" alt="lab1-30" style="zoom:20%;" />

  ​	search一番发现需要用i386-elf-gcc编译，于是又开始下载安装Macports，但是该应用是pkg安装所以需要先设置环境变量

  ​	`export PATH=/opt/local/bin:/opt/local/sbin:$PATH`

  ​	之后再`sudo port install i386-elf-gcc`(漫长的等待)即可

  ​	这次换了另一种错，于是换了一种安装办法——直接使用macports安装qemu，`sudo port install qemu`(依然漫长)之后就好了，再如法炮制创建软链接`sudo ln -s qemu-system-i386 /usr/local/bin/qemu`就能用了

- 还有一个坑到现在都未知原因，如果使用answer里面的Makefile可以正常进行实验，但是labcodes中的不行，由于我并未对Makefile进行过更改所以姑且不认为是我的过失，用meld对比只在gcc和gdb的一些设置上有问题，此外labcodes中的Makefile使用的都是`i386-ucore-elf` 而answer中的使用的都是`i386-elf`

  此外在Mac上会多出来使用clang/llvm编译器的设定也不知从何而来，而且直接使用Mac发送到ubuntu的Makefile也会报错

  ​	<img src="/Users/zhouchenfei/Library/Containers/com.tencent.qq/Data/Library/Caches/Images/DEF1DEF46B0F90A8D3B50227A48FABA0.png" alt="DEF1DEF46B0F90A8D3B50227A48FABA0" style="zoom: 50%;" />

- 由于在Mac上运行`make debug`和`make grade`出现问题重新研究Makefile发现如果运行下图红框中命令`make qemu-nox`即可无图形化显示qemu，也即可直接在terminal里面直接实现按键回显 

  ![lab1-40](/Users/zhouchenfei/Desktop/OS截图/lab1-40.png)

- 最后，尝试了很久在Mac上运行debug和grade根据报错信息以及艰难的英文提问使用Google和overstackflow终于找到了报错原因的gnome-terminal的安装包，这个在ubuntu上是预装的，Mac上找源码就找了许久

  [源码]: http://www.finkproject.org/download/srcdist.php

  于是放弃，重新回到ubuntu(在听取老师建议把gnome-terminal改成Terminal也不行后)

- 总的来说踩了一大堆的坑，尤其是前期配环境，后面有指导书贴心的提示并把代码注释看完(注释很明晰)再连蒙带猜就可以比较好的完成练习，实验指导书说开头前两个是最难的，这个lab1也让我深刻的认识到“不自己挖挖坑又怎么能体会到OS有多南呢”，但是填完坑之后的喜悦也是独一无二的

- (10.9检查完作业的更新)还是实验指导书没有认真看完👀有很多感觉跟代码关系不大的内容就没有一步步的仔细去看，所以有两个问题被学姐问住了QAQ

  > 1. ![lab-42](/Users/zhouchenfei/Desktop/OS截图/lab-42.png)
  >
  >    为什么这里要使用ljmp？毕竟下一条指令就是protcseg
  >
  > 2. <img src="/Users/zhouchenfei/Desktop/OS截图/lab-43.png" alt="lab-43" style="zoom:33%;" />为什么有的有pushl $0有的没有

  1. 一开始我回答的是毕竟变成了32位，所以应该是需要用ljmp把这个寄存器也给改成32位吧(应该)

     由于及其不确定回来后又看了看实验指导书并没有找到直接的解释，但是在“BIOS启动过程”这一章节中有对第一条长指令的类似解释，所以(*这次是合情推理*)可以认为：使用一个长跳转指令，将cs修改为32位段寄存器，并且跳转到protcseg这一32位代码入口处，此时CPU进入32位模式(学姐不告诉标答所以只能推测)

  2. 最开始看过实验书知道有32个系统预留寄存器，所以就尝试猜测说没有`pushl $0`的是因为已经被系统使用了

     回来再翻实验书，这次这个是标答没错了![lab1-44](/Users/zhouchenfei/Desktop/OS截图/lab1-44.png)

     <img src="/Users/zhouchenfei/Desktop/OS截图/lab1-45.png" alt="lab1-45" style="zoom:33%;" />

     可以看到在**lab1中对中断的处理实现**章节中有图一的内容结合vector.c部分代码(图二)可以得知是因为8-14还有17是直接压入了error code其他的是自动压入0以及其对应中断号，这个error code与异常号相关，在产生中断的时候起到作用

- 所以还是要好好看完所有的实验指导书还有MOOC再开始做实验啊TwT

## 练习一

> 1. 操作系统镜像文件ucore.img是如何一步一步生成的？(需要比较详细地解释Makefile中每一条相关命令和命令参数的含义，以及说明命令导致的结果)
> 2. 一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？

1. 

![lab1-2](/Users/zhouchenfei/Desktop/OS截图/lab1-2.png)

​	**上图为Makefile中与ucore.img生成相关的代码，下图是使用make "v="后执行Makefile的输出**

![lab1-1](/Users/zhouchenfei/Desktop/OS截图/lab1-1.png)

- \#Line1 :=是覆盖前值的赋值，即使用UCOREIMG这个变量表示$(call totarget,ucore.img)
- \#Line2 表示需要两个依赖文件: kernel和bootblock
- \#Line3-5 $V$是Makefile开始时定义的全局变量 $V:=@$ @代表的是生成的目标\$(UCOREIMG) dd是拷贝的同时转换 即
  - 指定10000个block并将/dev/zero下面的块文件拷贝到\$@中($V$代表的\$(UCOREIMG))
  - 将\$(bootblock)的内容拷贝到\$(UCOREIMG)中，conv=notrunc表示不截短输出文件
  - 同上将将\$(kernel)的内容拷贝到\$(UCOREIMG)中，seek=1表示从输出文件跳过一个块后再开始复制

- \#Line6 将生成的目标文件ucore.img传入create_target

  **以上是ucore.img的生成主体，输出结果如*上图红框部分*,与之相关的还有kernel和bootblock的生成**

![](/Users/zhouchenfei/Desktop/OS截图/lab1-5.png)

​	上图的作用是生成kernel，第一行是定义的kernel变量是调用call将kernel放入totarget——即将`bin/`前缀加到kernel中，二三行是添加kernel的依赖文件tools/kernel.ld和KOBJS变量，即将kernel的包含库放入read_packet，第四行中$\$@$表示$\$(kernel)$ 即给kernel所指向的内容做链接，第五行在gcc下执行链接操作并生成目标文件，第六七行完成-o的编译，最后将kernel传入create_target，最终得到了`elf-i386`的内核文件，**输出结果如图二蓝框部分，可以看出除了`elf-i386`，在生成kernel的过程中.c文件被编译链接成了.o文件**

![lab1-6](/Users/zhouchenfei/Desktop/OS截图/lab1-6.png)

​	最后是bootblock的生成，前三行是编译bootblock所必须的bootasm.o、bootmain.o、sign.o的生成，第七八行表示bootasm.o的生成和拷贝到bootblock中，最后将生成的bootblock传入create_target

​	**输出如图二绿框部分，同时蓝框最后一句是编译sign.c文件(用于生成一个符合规范的硬盘主引导扇区)**

​	生成sign的代码

![lab1-1.1](/Users/zhouchenfei/Desktop/OS截图/lab1-1.1.png)

其它部分：

​	 <img src="/Users/zhouchenfei/Desktop/OS截图/lab1-3.png" alt="lab1-1.1" style="zoom:33%;" /> 对各种常量的初始化

​	<img src="/Users/zhouchenfei/Desktop/OS截图/lab1-34.png" alt="lab1-1.1" style="zoom:20%;" /> 之后是对gcc前缀正确性的判断

​	<img src="/Users/zhouchenfei/Desktop/OS截图/lab1-35.png" alt="lab1-1.1" style="zoom:20%;" />	同上对qemu的正确性判断

​	接下来的部分定义了各种编译命令以及编译选项

​	<img src="/Users/zhouchenfei/Desktop/OS截图/lab1-36.png" alt="lab1-36" style="zoom:33%;" />

​	包含function.mk文件并进行设置

​	<img src="/Users/zhouchenfei/Desktop/OS截图/lab1-37.png" alt="lab1-37" style="zoom:33%;" />

​	余下部分用来完成clean以及grade等，故不做分析

> 相关命令参数
>
> 1. gcc 
>    - -I：添加包含目录
>    - -fno-builtin：只接受以“_builtin”开头的名称的内建函数
>    - -Wall：开启全部警告提示
>    - -ggdb：生成GDB需要的调试信息
>    - -m32：为32位环境生成代码，int、long和指针都是32位
>    - -gstab：生成stab格式的调试信息，仅用于gdb
>    - -nostdinc：不扫描标准系统头文件，只在-I指令指定的目录中扫描
>    - -fno-stack-protector：生成用于检查栈溢出的额外代码，如果发生错误，则打印错误信息并退出
>    - -c：编译源文件但不进行链接
>    - -o：结果的输出文件
> 2. ld
>    - -m elf_i386：使用elf_i386模拟器
>    - -nostdlib：只查找命令行中明确给出的库目录，不查找链接器脚本中给出的（即使链接器脚本是在命令行中给出的）
>    - -T tools/kernel.ld：将tools/kernel.ld作为链接器脚本
>    - -o bin/kernel：输出到bin/kernel文件
> 3. obj相关
>    - -Os：对输出文件大小进行优化，开启全部不增加代码大小的-O2优化
>    - -g：以操作系统原生格式输出调试信息，gdb可以处理这一信息
>    - -O2：进行大部分不以空间换时间的优化
> 4. bootblock
>    - -N：将文字和数据部分置为可读写，不将数据section置为与页对齐， 不链接共享库
>    - -e start：将start符号置为程序起始点
>    - -Ttext 0x7C00：链接时将".bss"、".data"或".text"置于绝对地址0x7C00处

2. 由`make "V="`的输出可以看出由sign.c编译出的sign.o规范了bootblock.o生成了题目描述的主引导扇区，查看sign.c

   ![](/Users/zhouchenfei/Desktop/OS截图/lab1-7.png)

   由上图画横线部分可得**一个磁盘主引导扇区有512Bytes**,通过红框部分可以得知文件内容不超过510Bytes，最后两个Bytes是0x55和0xAA

## 练习二

> 1. 从CPU加电后执行的第一条指令开始，单步跟踪BIOS的执行。
>2. 在初始化位置0x7c00设置实地址断点,测试断点正常。
> 3. 从0x7c00开始跟踪代码运行,将单步跟踪反汇编得到的代码与bootasm.S和 bootblock.asm进行比较。
> 4. 自己找一个bootloader或内核中的代码位置，设置断点并进行测试。

1. 首先单步跟踪，将lab1/tools/gdbinit改为

   ```bash
   set architecture i8086
   target remote : 1234   #远程
   ```

   之后在lab1中执行make debug，并在gdb中输入`si`即可单步跟踪BIOS

   可以看到第一条指令PC为0xFFF0，内存地址为0xFFFF0

   使用`x /i $pc`可以查看BIOS代码

   ![lab1-20](/Users/zhouchenfei/Desktop/OS截图/lab1-20.png)

2. 3.  将lab1/tools/gdbinit改为

   ```shell
   file obj/bootblock.o
   set architecture i8086
   target remote : 1234
   b *0x7c00
   continue
   x /i $pc  
   ```

   ![lab1-8](/Users/zhouchenfei/Desktop/OS截图/lab1-8.png)

   如图，断点测试正常，逐行执行(`si`)发现断点处0x7c00为运行BootLoader的起始地址，反汇编bootasm.S以后除了注释全部删去**其他与bootblock.asm一致**

4. 将lab1/tools/gdbinit改为

```shell
file bin/kernel
set architecture i8086
target remote : 1234
b kernel_init
continue
x /i $pc  
```

![lab1-9](/Users/zhouchenfei/Desktop/OS截图/lab1-9.png)

​	断点设置正常

## 练习三

> 分析bootloader进入保护模式的过程

​	bootloader中从实模式进到保护模式的代码保存在lab1/boot/bootasm.S中，使用x86汇编语言编写。

​	参考实验指导书的*保护模式和分段机制*部分以及bootasm.S，了解到通过修改A20地址线及初始化GDT表可以达成实模式到保护模式的转换。

​	入口为start

​	![lab1-10](/Users/zhouchenfei/Desktop/OS截图/lab1-10.png)	

​	一开始从`%cs=0 $pc=0x7c00`此时bootloader被加载到0x7c00处，进入实模式后关闭中断，清除EFLAGS的DF位，并将AX、SS、ES、DS置0

​	![lab1-11](/Users/zhouchenfei/Desktop/OS截图/lab1-11.png)

​	由于未开启A20的时候可访问内存不足1MB，所以之后开启A20地址线(将A20线置于高电位1)，达到全部32个地址线可用，共有4GB的内存空间，之后是关闭“回卷”机制：

1. 等待8042控制器input buffer为空

 	2. `Write 8042 Output Port （P2）`发送P2命令到input buffer
 	3. 等待8042input buffer为空
 	4. 将8042 Output Port（P2）得到字节的第2位置1，写入input buffer

​	![lab1-12](/Users/zhouchenfei/Desktop/OS截图/lab1-12.png)

​	横线处是载入GDT表(全局描述符表)，如下图可以看到在代码的最后已经将一个简单的GDT表及其描述符静态储存在引导区中

​	![lab1-13](/Users/zhouchenfei/Desktop/OS截图/lab1-13.png)	

​	红框框出的部分是在A20开启，GDT载入后进入保护模式的代码——**通过将`%cr0`寄存器的第0(PE)位置为1**

![lab1-14](/Users/zhouchenfei/Desktop/OS截图/lab1-14.png)

​		之后长跳转到proctcseg这一32位代码入口处，重置DS、ES、FS、GS、SS段寄存器并初始化栈的frame pointer和stack pointer，最后`call bootmain` 转到bootmain进行操作系统内核的加载，即完成从实模式到保护模式的转换

## 练习四

> 分析bootloader加载ELF格式的OS的过程

​	借助实验指导书的*硬盘访问概述*和*ELF文件格式概述*以及bootmain.c文件可知读一个扇区的流程是:①等待磁盘准备好②往0x1F2到0X1F6中设置读取扇区需要的参数，包括读取扇区的数量以及LBA参数③等待磁盘完成读取操作④从数据端口0X1F0读取出数据到指定内存中

​	bootmain.c开头的注释表示它的作用是从硬盘中启动ELF的OS内核映像，该程序是BootLoader且应该被存储在磁盘的第一个扇区，第二个扇区存储的是kernel image(必须是ELF格式)；

​	boot up的步骤是：CPU启动时吧BIOS读入内存并执行，BIOS对设备初始化并设置中断程序，并将硬盘的第一个扇区读入内存并跳转；然后执行bootmain.S，开启保护模式，并设置好栈，最后调用bootmain()；bootmain()将kernel读入内存并跳转

​	其中waitdisk函数用以实现①和③<img src="/Users/zhouchenfei/Desktop/OS截图/lab1-38.png" alt="lab1-38" style="zoom:33%;" />该函数的作用是连续不断地从0x1F7地址读取磁盘的状态，直到磁盘free

​	bootmain.c中readsect函数基本功能为读取一个磁盘扇区

​	<img src="/Users/zhouchenfei/Desktop/OS截图/lab1-15.png" style="zoom:33%;" />

​	之后的`readseg`函数对`readsect`函数进行了完善，可以读取任意长度的内容

![lab1-16](/Users/zhouchenfei/Desktop/OS截图/lab1-16.png)

​	在elf.h文件中定义了ELF文件的组织形式![lab1-24](/Users/zhouchenfei/Desktop/OS截图/lab1-24.png)

​	`bootmain`函数呈现了bootloader加载ELF格式的OS的过程：

​	![lab1-25](/Users/zhouchenfei/Desktop/OS截图/lab1-25.png)

1. 读取8个扇区(1页4kB)大小的ELF头部存储到定义好的ELFHDR地址0x10000
2. 通过存储在头部的e_magic判断是否为合法的ELF文件
3. 从ELF头中获取程序头位置，从中获得每段信息并将ELF文件中的数据载入内存
4. 找到OS的入口(e_entry)然后使用函数调用的方式跳转到该地址上去

## 练习五

> 实现函数调用堆栈跟踪函数 

​	根据*函数堆栈*这一节内容可以得到如下函数堆栈结构：

​	![lab1-17](/Users/zhouchenfei/Desktop/OS截图/lab1-17.png)

> `ebp` 基址指针寄存器 `eip` 堆栈指针寄存器(栈顶) 

​	代码参考`print_stackframe`的注释写成如下

​	![lab1-18](/Users/zhouchenfei/Desktop/OS截图/lab1-18.png)

​	基本上一行对应一行注释，注释很明晰，理清楚函数堆栈的构造就很容易改写出

​	![lab1-19](/Users/zhouchenfei/Desktop/OS截图/lab1-19.png)

​	最后一行对应的是第一个使用堆栈的函数，即bootmain.c中的bootmain(根据debug_info此时ebp对应地址的值为0)，ebp对应bootmain的栈帧，eip为调用kern_init后的地址，因为kern_init没有参数所以args后面是bootloader的二进制代码，\<unknown\>后面跟的是eip-1的值

##  练习六

> 完善中断初始化和处理
>
> 1. 中断描述符表（也可简称为保护模式下的中断向量表）中一个表项占多少字节？其中哪几位代表中断处理代码的入口？
> 2. 请编程完善kern/trap/trap.c中对中断向量表进行初始化的函数idt_init。在idt_init函数中，依次对所有中断入口进行初始化。使用mmu.h中的SETGATE宏，填充idt数组内容。每个中断的入口由tools/vectors.c生成，使用trap.c中声明的vectors数组即可。
> 3. 请编程完善trap.c中的中断处理函数trap，在对时钟中断进行处理的部分填写trap函数中处理时钟中断的部分，使操作系统每遇到100次时钟中断后，调用print_ticks子程序，向屏幕上打印一行文字”100 ticks”。

1. ![lab1-26](/Users/zhouchenfei/Desktop/OS截图/lab1-26.png)

   IDT中每个表项占8个字节，由图，16-31位是处理代码入口地址的段选择子，0-15和48-63拼接成offset位移，使用段选择子在GDT中查找到相应段的base address，加上offset就是中断处理代码的入口

2. 3. 参考注释以及必须文件写成

   ![lab1-21](/Users/zhouchenfei/Desktop/OS截图/lab1-21.png)

   ![lab1-22](/Users/zhouchenfei/Desktop/OS截图/lab1-22.png)

   得到输出如下

   > kbd的字符回显ubuntu18不显示，xubuntu用同一代码可以，Mac也可以

   ![lab1-23](/Users/zhouchenfei/Desktop/OS截图/lab1-23.png)

## 练习七(Challenge)

> 1. 增加一用户态函数（可执行一特定系统调用：获得时钟计数值）
>
> 2. 用键盘实现用户模式内核模式切换。具体目标是：“键盘输入3时切换到用户模式，键盘输入0时切换到内核模式”。 基本思路是借鉴软中断(syscall功能)的代码，并且把trap.c中软中断处理的设置语句拿过来。

1. 根据lab1/kern/init.c中的注释提示可知要实现这一功能需要完成init.c中的lab1_switch_to_user()和lab1_switch_to_kernel()函数以及lab1/kern/trap/trap.c中的 T_SWITCH_TOU和T_SWITCH_TOK函数

<img src="/Users/zhouchenfei/Desktop/OS截图/lab1-27.png" alt="lab1-27" style="zoom:33%;" />

![lab1-28](/Users/zhouchenfei/Desktop/OS截图/lab1-28.png)

![lab1-29](/Users/zhouchenfei/Desktop/OS截图/lab1-29.png)

2. 主要考虑在ISR中在修改trapframe的同时对栈进行更进一步的伪造，比如在从内核态返回到用户态的时候，在trapframe里额外插入原本不存在的ss和esp，在用户态返回到内核态的时候，将trapframe中的esp和ss删去等

   1. 利用IRQ_OFFSET+IRQ_KBD中断号进行相应的处理，在case中加入栈的信息(ss和esp)并在kern/trap/trapentry.S中对trapframe的结构进行处理

   最终结果如图示

   <img src="/Users/zhouchenfei/Desktop/OS截图/lab1-31.png" alt="lab1-31" style="zoom:30%;" /><img src="/Users/zhouchenfei/Desktop/OS截图/lab1-32.png" alt="lab1-32" style="zoom:30%;" />

   最后执行`make grade`得到分数![lab1-39](/Users/zhouchenfei/Desktop/OS截图/lab1-39.png)