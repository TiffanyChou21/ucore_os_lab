#include <defs.h>
#include <x86.h>
#include <elf.h>

/* *********************************************************************
 * This a dirt simple boot loader, whose sole job is to boot
 * an ELF kernel image from the first IDE hard disk.
 *
 * DISK LAYOUT
 *  * This program(bootasm.S and bootmain.c) is the bootloader.
 *    It should be stored in the first sector of the disk.
 *
 *  * The 2nd sector onward holds the kernel image.
 *
 *  * The kernel image must be in ELF format.
 *
 * BOOT UP STEPS
 *  * when the CPU boots it loads the BIOS into memory and executes it
 *
 *  * the BIOS intializes devices, sets of the interrupt routines, and
 *    reads the first sector of the boot device(e.g., hard-drive)
 *    into memory and jumps to it.
 *
 *  * Assuming this boot loader is stored in the first sector of the
 *    hard-drive, this code takes over...
 *
 *  * control starts in bootasm.S -- which sets up protected mode,
 *    and a stack so C code then run, then calls bootmain()
 *
 *  * bootmain() in this file takes over, reads in the kernel and jumps to it.
 * */

#define SECTSIZE        512
#define ELFHDR          ((struct elfhdr *)0x10000)      // scratch space

/* waitdisk - wait for disk ready */
static void
waitdisk(void) {
    while ((inb(0x1F7) & 0xC0) != 0x40)
        /* do nothing */;
}

/* readsect - read a single sector at @secno into @dst */
static void
readsect(void *dst, uint32_t secno) {
    // wait for disk to be ready
    waitdisk();// 等待磁盘free

    outb(0x1F2, 1);  // 往0X1F2地址中写入要读取的扇区数，由于此处需要读一个扇区，因此参数为1                       // count = 1
    outb(0x1F3, secno & 0xFF);// 输入LBA参数的0...7位；
    outb(0x1F4, (secno >> 8) & 0xFF);//8-15
    outb(0x1F5, (secno >> 16) & 0xFF);//16-23
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);//24-27
    outb(0x1F7, 0x20); // 向磁盘发出读命令0x20                     // cmd 0x20 - read sectors

    // wait for disk to be ready
    waitdisk();// 等待磁盘free

    // read a sector
    // 从数据端口0x1F0读取数据，/4此处是以4个字节为单位的，因为这个指令是以l(long)结尾
    insl(0x1F0, dst, SECTSIZE / 4);
}

/* *
 * readseg - read @count bytes at @offset from kernel into virtual address @va,
 * might copy more than asked.
 * 从内核的offset处读count个字节到虚拟地址va中。
 * 复制的内容可能比count个字节多。
 * */
static void
readseg(uintptr_t va, uint32_t count, uint32_t offset) {
    uintptr_t end_va = va + count;

    // round down to sector boundary向下舍入到扇区边
    va -= offset % SECTSIZE;

    // translate from bytes to sectors; kernel starts at sector 1
    // 从字节转换到扇区；kernel开始于扇区1
    uint32_t secno = (offset / SECTSIZE) + 1;

    // If this is too slow, we could read lots of sectors at a time.
    // We'd write more to memory than asked, but it doesn't matter --
    // 写到内存时会比请求的更多
    // we load in increasing order.以内存递增次序加载的˛
    for (; va < end_va; va += SECTSIZE, secno ++) {
        readsect((void *)va, secno);
    }
}

/* bootmain - the entry of bootloader */
void
bootmain(void) {
    // read the 1st page off disk
    // 从磁盘读出kernel映像的第一页，得到ELF头
    readseg((uintptr_t)ELFHDR, SECTSIZE * 8, 0);

    // is this a valid ELF?是否合法ELF
    if (ELFHDR->e_magic != ELF_MAGIC) {
        goto bad;
    }

    struct proghdr *ph, *eph;

    // load each program segment (ignores ph flags)先将描述表的头地址存在ph
    // ELF头部有描述ELF文件应加载到内存什么位置的描述表
    ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);
    eph = ph + ELFHDR->e_phnum;
    // 按照描述表将ELF文件中数据载入内存
    for (; ph < eph; ph ++) {
        readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);
    }

    // call the entry point from the ELF header
    // note: does not return
    ((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();

bad:
    outw(0x8A00, 0x8A00);
    outw(0x8A00, 0x8E00);

    /* do nothing */
    while (1);
}

