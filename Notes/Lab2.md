# Lab2 物理内存

## 问题汇总

* 理解基于段页式内存地址的转换机制 ？
* 理解页表的建立和使用方法 ？
* 理解物理内存的管理方法 ？

 - 你的first fit算法是否有进一步的改进空间
 - 请描述页目录项（Page Directory Entry）和页表项（Page Table Entry）中每个组成部分的含义以及对ucore而言的潜在用处。
 - 如果ucore执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？
 - 数据结构Page的全局变量（其实是一个数组）的每一项与页表中的页目录项和页表项有无对应关系？如果有，其对应关系是啥？
 - 如果希望虚拟地址与物理地址相等，则需要如何修改lab2，完成此事？ **鼓励通过编程来具体完成这个问题** 

- 管理页级物理内存空间所需的Page结构的内存空间从哪里开始，占多大空间？
- 空闲内存空间的起始地址在哪里？
- 如何在建立页表的过程中维护全局段描述符表（GDT）和页表的关系，确保ucore能够在各个时间段上都能正常寻址？
- 对于哪些物理内存空间需要建立页映射关系？
- 具体的页映射关系是什么？
- 页目录表的起始地址设置在哪里？
- 页表的起始地址设置在哪里，需要多大空间？
- 如何设置页目录表项的内容？
- 如何设置页表项的内容？



**扩展练习Challenge：buddy system（伙伴系统）分配算法（需要编程）**

Buddy System算法把系统中的可用存储空间划分为存储块(Block)来进行管理, 每个存储块的大小必须是2的n次幂(Pow(2, n)), 即1, 2, 4, 8, 16, 32, 64, 128...

 -  参考[伙伴分配器的一个极简实现](http://coolshell.cn/articles/10427.html)， 在ucore中实现buddy system分配算法，要求有比较充分的测试用例说明实现的正确性，需要有设计文档。

**扩展练习Challenge：任意大小的内存单元slub分配算法（需要编程）**

slub算法，实现两层架构的高效内存单元分配，第一层是基于页大小的内存分配，第二层是在第一层基础上实现基于任意大小的内存分配。可简化实现，能够体现其主体思想即可。

 - 参考[linux的slub分配算法/](http://www.ibm.com/developerworks/cn/linux/l-cn-slub/)，在ucore中实现slub分配算法。要求有比较充分的测试用例说明实现的正确性，需要有设计文档。

 - 显示界面怎么理解？

   ```
   chenyu$ make qemu
   (THU.CST) os is loading ...
   
   Special kernel symbols:
     entry  0xc010002c (phys)
     etext  0xc010537f (phys)
     edata  0xc01169b8 (phys)
     end    0xc01178dc (phys)
   Kernel executable memory footprint: 95KB
   memory managment: default_pmm_manager
   e820map:
     memory: 0009f400, [00000000, 0009f3ff], type = 1.
     memory: 00000c00, [0009f400, 0009ffff], type = 2.
     memory: 00010000, [000f0000, 000fffff], type = 2.
     memory: 07efd000, [00100000, 07ffcfff], type = 1.
     memory: 00003000, [07ffd000, 07ffffff], type = 2.
     memory: 00040000, [fffc0000, ffffffff], type = 2.
   check_alloc_page() succeeded!
   check_pgdir() succeeded!
   check_boot_pgdir() succeeded!
   -------------------- BEGIN --------------------
   PDE(0e0) c0000000-f8000000 38000000 urw
     |-- PTE(38000) c0000000-f8000000 38000000 -rw
   PDE(001) fac00000-fb000000 00400000 -rw
     |-- PTE(000e0) faf00000-fafe0000 000e0000 urw
     |-- PTE(00001) fafeb000-fafec000 00001000 -rw
   --------------------- END ---------------------
   ++ setup timer interrupts
   100 ticks
   100 ticks
   ……
   ```

- 了解如何发现系统中的物理内存；然后了解如何建立对物理内存的初步管理，即了解连续物理内存管理；最后了解页表相关的操作，即如何建立页表来实现虚拟内存到物理内存之间的映射，对段页式内存管理机制有一个比较全面的了解。本实验里面实现的内存管理还是非常基本的，并没有涉及到对实际机器的优化，比如针对 cache 的优化等

- 



## 阅读的文件

* boot/bootasm.S：增加了对计算机系统中物理内存布局的探测功能；
* kern/init/entry.S：根据临时段表重新暂时建立好新的段空间，为进行分页做好准备。
* kern/mm/default\_pmm.[ch]：提供基本的基于链表方法的物理内存管理（分配单位为页，即4096字节）；
* kern/mm/pmm.[ch]：pmm.h定义物理内存管理类框架struct pmm\_manager，基于此通用框架可以实现不同的物理内存管理策略和算法(default\_pmm.[ch]实现了一个基于此框架的简单物理内存管理策略)；
  pmm.c包含了对此物理内存管理类框架的访问，以及与建立、修改、访问页表相关的各种函数实现。
* kern/sync/sync.h：为确保内存管理修改相关数据时不被中断打断，提供两个功能，一个是保存eflag寄存器中的中断屏蔽位信息并屏蔽中断的功能，另一个是根据保存的中断屏蔽位信息来使能中断的功能；（可不用细看）
* libs/list.h：定义了通用双向链表结构以及相关的查找、插入等基本操作，这是建立基于链表方法的物理内存管理（以及其他内核功能）的基础。其他有类似双向链表需求的内核功能模块可直接使用list.h中定义的函数。
* libs/atomic.h：定义了对一个变量进行读写的原子操作，确保相关操作不被中断打断。（可不用细看）
* tools/kernel.ld：ld形成执行文件的地址所用到的链接脚本。修改了ucore的起始入口和代码段的起始地址。相关细节可参看附录C。