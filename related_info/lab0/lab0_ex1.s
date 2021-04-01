	.file	"lab0_ex1.c"
	.text
	.globl	count
	.data
	.align 4
	.type	count, @object
	.size	count, 4
count:
	.long	1
	.globl	value
	.align 4
	.type	value, @object
	.size	value, 4
value:
	.long	1
	.comm	buf,40,32
	.text
	.globl	main
	.type	main, @function
;COMMON--------------未初始化的全局变量
main:
.LFB0:
	.cfi_startproc
	endbr32
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	.cfi_offset 7, -12
	.cfi_offset 6, -16
	.cfi_offset 3, -20
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	count@GOTOFF(%eax), %edx
	movl	value@GOTOFF(%eax), %ebx
	movl	buf@GOT(%eax), %eax
	movl	(%eax), %esi
	movl	%edx, %ecx
	movl	%ebx, %eax
	movl	%esi, %edi
	;stosl stores the value of %eax at the address pointed to by %(e)di, and increases %e(di) by the size of %eax.
	;	"cld \n\t"
    ;   "rep \n\t"					     
    ;   "stosl"
	;:
	;: "c" (count), "a" (value) , "D" (buf[0])
	
#APP
# 6 "lab0_ex1.c" 1
	cld 
	rep 
	stosl
# 0 "" 2
#NO_APP
	nop
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
.LFB1:
	.cfi_startproc
	movl	(%esp), %eax
	ret
	.cfi_endproc
.LFE1:
	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 4
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 4
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 4
4:
