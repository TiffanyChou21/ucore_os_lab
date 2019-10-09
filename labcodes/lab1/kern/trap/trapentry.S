#include <memlayout.h>

# vectors.S sends all traps here.
.text
.globl __alltraps
__alltraps:
    # push registers to build a trap frame
    # therefore make the stack look like a struct trapframe
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushal

    # load GD_KDATA into %ds and %es to set up data segments for kernel
    movl $GD_KDATA, %eax
    movw %ax, %ds
    movw %ax, %es

    # push %esp to pass a pointer to the trapframe as an argument to trap()
    pushl %esp

    # call trap(tf), where tf=%esp
    call trap

    # pop the pushed stack pointer
    popl %esp

    # return falls through to trapret...
.globl __trapret
__trapret:
    # restore registers from stack
    popal

    # restore %ds, %es, %fs and %gs
    popl %gs
    popl %fs
    popl %es
    popl %ds

    # get rid of the trap number and error code
    addl $0x8, %esp
    iret


#challenge2
.globl __move_down_stack2 
# 整个栈向下平移2字节用以保存esp ss
__move_down_stack2:
    pushl %ebp
    movl %esp, %ebp

    pushl %ebx
    pushl %esi
    pushl %edi

    movl 8(%ebp), %ebx # ebx用来保存当前trapframe的最高地址
    movl 12(%ebp), %edi #分别减8得到真正的值
    subl $8, -4(%edi) # 修改esp的值使其与平移过后的栈保持一致
    movl %esp, %eax

    cmpl %eax, %ebx
    jle loop_end

loop_start:
    movb (%eax), %cl
    movb %cl, -8(%eax)
    addl $1, %eax
    cmpl %eax, %ebx
    jg loop_start

loop_end: 
    subl $8, %esp 
    subl $8, %ebp 
    
    movl %ebp, %eax
    cmpl %eax, %ebx
    jle ebp_loop_end

ebp_loop_begin:
    movl (%eax), %ecx

    cmpl $0, %ecx
    je ebp_loop_end
    cmpl %ecx, %ebx
    jle ebp_loop_end
    subl $8, %ecx
    movl %ecx, (%eax)
    movl %ecx, %eax
    jmp ebp_loop_begin

ebp_loop_end:

    popl %edi
    popl %esi
    popl %ebx

    popl %ebp
    ret 

.globl __move_up_stack2
# 整个栈向上平移2字节用以保存esp ss
__move_up_stack2:
    pushl %ebp 
    movl %esp, %ebp

    pushl %ebx
    pushl %edi
    pushl %esi

#把所有的tf_end以下的copy到用户栈 
    movl 8(%ebp), %eax
    subl $1, %eax
    movl 16(%ebp), %ebx # ebx存用户栈指针
    
    cmpl %eax, %esp
    jg copy_loop_end

copy_loop_begin:
    subl $1, %ebx
    movb (%eax), %cl
    movb %cl, (%ebx)

    subl $1, %eax
    cmpl %eax, %esp
    jle copy_loop_begin

copy_loop_end:

#将ebp中所有的值都修正
    movl %ebp, %eax
    cmpl %eax, 8(%ebp)
    jle fix_ebp_loop_end

fix_ebp_loop_begin:
    movl %eax, %edi
    subl 8(%ebp), %edi
    addl 16(%ebp), %edi # edi <=> eax

    cmpl (%eax), %esp 
    jle normal_condition
    movl (%eax), %esi
    movl %esi, (%edi)
    jmp fix_ebp_loop_end

normal_condition:
    movl (%eax), %esi
    subl 8(%ebp), %esi
    addl 16(%ebp), %esi
    movl %esi, (%edi)
    movl (%eax), %eax
    jmp fix_ebp_loop_begin

fix_ebp_loop_end:

    movl 12(%ebp), %eax
    subl $4, %eax

    movl %eax, %edi
    subl 8(%ebp), %edi
    addl 16(%ebp), %edi

    movl (%eax), %esi
    subl 8(%ebp), %esi
    addl 16(%ebp), %esi

    movl %esi, (%edi)

    movl 12(%ebp), %eax
    subl 8(%ebp), %eax
    addl 16(%ebp), %eax

# switch to user stack
    movl %ebx, %esp
    movl %ebp, %esi
    subl 8(%ebp), %esi
    addl 16(%ebp), %esi
    movl %esi, %ebp

    popl %esi
    popl %edi
    popl %ebx

    popl %ebp
    ret

