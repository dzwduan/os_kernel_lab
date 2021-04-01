# 1 "pmboot.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "pmboot.S"







DA_C = 0x98
DA_32 = 0x4000
DA_DRW = 0x92

.text
.globl start
.code16
start:
 jmpl $0x0, $code

GDT_START:
Descriptor_DUMMY:.word 0x0&0xffff;.word 0x0&0xffff;.byte (0x0>>16)&0xff;.word ((0x0>>8)&0xf00)|(0x0&0x0f0ff);.byte ((0x0>>24)&0xff)
Descript_CODE32 :.word 0xffffffff&0xffff;.word 0x0&0xffff;.byte (0x0>>16)&0xff;.word ((0xffffffff>>8)&0xf00)|(DA_C+DA_32&0x0f0ff);.byte ((0x0>>24)&0xff)
Descriptor_VIDEO:.word 0x0ffff&0xffff;.word 0xb8000&0xffff;.byte (0xb8000>>16)&0xff;.word ((0x0ffff>>8)&0xf00)|(DA_DRW&0x0f0ff);.byte ((0xb8000>>24)&0xff)
GDT_END:

GdtPtr:
 .word (GDT_END-GDT_START)-1 # so does gdt
 .long GDT_START # This will be rewrite by code.
msg:
 .string "Hello world!"
code:
 mov %cs,%ax
 mov %ax,%ds
 mov %ax,%es
 mov %ax,%ss
 mov $0x8000,%sp

 mov $msg ,%ax
 mov %ax ,%bp
 mov $12 ,%cx
 mov $0x1301,%ax
 mov $0x000c,%bx
 mov $0 ,%dl

 int $0x10


 lgdt GdtPtr


 cli


 inb $0x92,%al
 or $0x02,%al
 outb %al,$0x92


 movl %cr0,%eax
 or $1,%eax
 movl %eax,%cr0



 ljmp $0x8,$(LABEL_SEG_CODE32)



LABEL_SEG_CODE32:
.align 32
.code32
 movw $0x10,%ax
 movw %ax,%gs
 movl $((80*11+79)*2),%edi
 movb $0x0c,%ah
 movb $'P',%al
 movw %ax,%gs:(%edi)

loop2:
 jmp loop2

.org 0x1fe, 0x90
.word 0xaa55
